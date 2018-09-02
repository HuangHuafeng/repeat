#include "serveragent.h"
#include "serverclientprotocol.h"
#include "mysettings.h"

ServerAgent * ServerAgent::m_serveragent = nullptr;

ServerAgent::ServerAgent(const QString &hostName, quint16 port, QObject *parent) : QObject(parent),
    m_serverHostName(hostName),
    m_serverPort(port),
    m_tcpSocket(nullptr),
    m_messageTimer(this),
    m_timerServerHeartBeat(this)
{
    connect(&m_timerServerHeartBeat, SIGNAL(timeout()), this, SLOT(onServerHeartBeat()));
    //connect(&m_downloadTimer, SIGNAL(timeout()), this, SLOT(sendDownloadRequestsToServer()));

    //connect(&m_messageTimer, SIGNAL(timeout()), this, SLOT(onSendMessage()));
    connect(&m_messageTimer, SIGNAL(timeout()), this, SLOT(onSendMessageSmart()));

    connect(this, SIGNAL(internalBookDataDownloaded(QString)), this, SLOT(onInternalBookDataDownloaded(QString)));
    connect(this, SIGNAL(internalFileDataDownloaded(QString, bool)), this, SLOT(onInternalFileDataDownloaded(QString, bool)));
    // the following should NOT be in connectToServer()
    //connect(this, SIGNAL(internalBookDataDownloaded(QString)), this, SLOT(onInternalBookDataDownloaded(QString)), Qt::ConnectionType::QueuedConnection);
    //connect(this, SIGNAL(internalFileDataDownloaded(QString, bool)), this, SLOT(onInternalFileDataDownloaded(QString, bool)), Qt::ConnectionType::QueuedConnection);
}

ServerAgent::~ServerAgent()
{
}

ServerAgent * ServerAgent::instance()
{
    if (m_serveragent == nullptr)
    {
        QString serverHostName = MySettings::serverHostName();
        quint16 serverPort = MySettings::serverPort();
        m_serveragent = new ServerAgent(serverHostName, serverPort);
    }

    return m_serveragent;
}

/**
 * @brief ServerAgent::onReadyRead
 * CORRECTION:
 * it seems size of message is NOT an issue. Although it should be taken care of.
 * We should be careful/aware that message is NOT always available as a whole, so we
 * need to wait for more data in case we failed to read the message contents.
 */
void ServerAgent::onReadyRead()
{
    static sptr<MessageHeader> currentMessage;
    static uint readMessageHeaderConsecutiveFailure = 0;

    do {
        if (currentMessage.get() == nullptr)
        {
            // last message processed completed, it's time for a new message
            currentMessage = readMessageHeader();
        }

        if (currentMessage.get() != nullptr)
        {
            // we have a message, try to process it
            readMessageHeaderConsecutiveFailure = 0;
            int handleResult = handleMessage(*currentMessage);
            if (handleResult == 0)
            {
                qDebug("successfully handled message: %s", currentMessage->toString().toUtf8().constData());

                // if it's not a heartbeat message, update the counters
                if (currentMessage->code() != ServerClientProtocol::ResponseNoOperation)
                {
                    m_lastResponded = currentMessage->respondsTo();
                    if (m_messagesSent < m_lastResponded)
                    {
                        // this can happen if the user cancels the downloading
                        m_messagesSent = m_lastResponded;
                    }
                }

                // successfully processed the message
                currentMessage = sptr<MessageHeader>();
            }
            else if (handleResult == 1)
            {
                qDebug("failed to handle message: %s", currentMessage->toString().toUtf8().constData());

                // failed to process the message, probably means the content of the message is NOT fully available
                // so quit the loop and contine in next call of onReadyRead()
                break;
            }
            else
            {
                qDebug("unknown message: %s", currentMessage->toString().toUtf8().constData());

                // discard the message and continue trying to get the next message
                currentMessage = sptr<MessageHeader>();
            }
        }
        else
        {
            // no messages available, quit the loop
            readMessageHeaderConsecutiveFailure ++;
            if (readMessageHeaderConsecutiveFailure > (sizeof(MessageHeader) / sizeof(qint32)))
            {
                qDebug() << "unable to get a message, waiting for data from the server.";
                readMessageHeaderConsecutiveFailure = 0;
                break;
            }
        }
    } while (1);
}

void ServerAgent::onConnected()
{
    qDebug() << "onConnected()";

    // renew the counters by a temporary message
    MessageHeader toBeDiscarded;
    m_messagesSent = toBeDiscarded.sequenceNumber();
    m_lastResponded = toBeDiscarded.sequenceNumber();

    // start heartbeat timer
    m_timerServerHeartBeat.start(1000 * MySettings::heartbeatIntervalInSeconds());
    // start sending message
    m_messageTimer.start(MySettings::downloadIntervalInMilliseconds());
}

void ServerAgent::onDisconnected()
{
    m_messageTimer.stop();
    m_timerServerHeartBeat.stop();
    m_tcpSocket->deleteLater();
    m_tcpSocket = nullptr;
}

void ServerAgent::onError(QAbstractSocket::SocketError socketError)
{
    qDebug() << "onError()" << socketError;
}

void ServerAgent::onStateChanged(QAbstractSocket::SocketState socketState)
{
    qDebug() << "onStateChanged()" << socketState;
}

sptr<MessageHeader> ServerAgent::readMessageHeader()
{
    sptr<MessageHeader> mh = new MessageHeader(-1, -1, -1);
    QDataStream in(m_tcpSocket);
    in.startTransaction();
    in >> *mh;
    if (in.commitTransaction() == true)
    {
        return mh;
    }
    else
    {
        // in this case, the transaction is restored by commitTransaction()
        //qInfo("failed to read message code in readMessageCode(), probably no data from peer");
        return sptr<MessageHeader>();
    }
}

void ServerAgent::onServerHeartBeat()
{
    sendRequestNoOperation();
}

void ServerAgent::onInternalBookDataDownloaded(QString bookName)
{
    QElapsedTimer t;
    t.start();

    Word::v2StoreMultipleWordFromServer(m_mapWords);
    m_mapWords.clear();
    completeBookDownload(bookName);

    qDebug() << "Used" << t.elapsed() << "ms in onInternalBookDataDownloaded()";
}

int ServerAgent::handleMessage(const MessageHeader &msgHeader)
{
    if (m_tcpSocket == nullptr)
    {
        return -1;
    }

    bool handleResult = false;
    bool unknowMessage = false;
    switch (msgHeader.code()) {
    case ServerClientProtocol::ResponseNoOperation:
        handleResult = handleResponseNoOperation(msgHeader);
        break;

    case ServerClientProtocol::ResponseGetAllBooks:
        handleResult = handleResponseGetAllBooks(msgHeader);
        break;

    case ServerClientProtocol::ResponseGetAWord:
        handleResult = handleResponseGetAWord(msgHeader);
        break;

    case ServerClientProtocol::ResponseGetABook:
        handleResult = handleResponseGetABook(msgHeader);
        break;

    case ServerClientProtocol::ResponseGetBookWordList:
        handleResult = handleResponseGetBookWordList(msgHeader);
        break;

    case ServerClientProtocol::ResponseBookWordListAllSent:
        handleResult = handleResponseBookWordListAllSent(msgHeader);
        break;

    case ServerClientProtocol::ResponseGetFile:
        handleResult = handleResponseGetFile(msgHeader);
        break;

    case ServerClientProtocol::ResponseGetFileFinished:
        handleResult = handleResponseGetFileFinished(msgHeader);
        break;

    case ServerClientProtocol::ResponseGetWordsOfBookFinished:
        handleResult = handleResponseGetWordsOfBookFinished(msgHeader);
        break;

    case ServerClientProtocol::ResponseUnknownRequest:
        handleResult = handleResponseUnknownRequest(msgHeader);
        break;

    default:
        handleUnknownMessage(msgHeader);
        unknowMessage = true;
        break;

    }

    int retVal;
    if (unknowMessage == true)
    {
        retVal = -1;
    }
    else
    {
        if (handleResult == true)
        {
            retVal = 0;
        }
        else
        {
            retVal = 1;
        }
    }

    return retVal;
}

bool ServerAgent::handleResponseNoOperation(const MessageHeader &msgHeader)
{
    // avoid unused parameter warning
    msgHeader.toString();

    return true;
}

bool ServerAgent::handleUnknownMessage(const MessageHeader &msgHeader)
{
    // avoid unused parameter warning
    msgHeader.toString();

    return true;
}

bool ServerAgent::handleResponseGetAllBooks(const MessageHeader &msgHeader)
{
    // avoid unused parameter warning
    msgHeader.toString();

    QDataStream in(m_tcpSocket);
    QList<QString> books;
    in.startTransaction();
    in >> books;
    if (in.commitTransaction() == false)
    {
        // in this case, the transaction is restored by commitTransaction()
        qDebug() << "failed to read books in handleResponseGetAllBooks()";
        return false;
    }

    m_booksInServer = books;
    emit(bookListReady(books));

    return true;
}

bool ServerAgent::handleResponseBookWordListAllSent(const MessageHeader &msgHeader)
{
    // avoid unused parameter warning
    msgHeader.toString();

    QDataStream in(m_tcpSocket);
    QString bookName;
    in.startTransaction();
    in >> bookName;
    if (in.commitTransaction() == false)
    {
        // in this case, the transaction is restored by commitTransaction()
        qDebug() << "failed to read book name in handleResponseBookWordListAllSent()";
        return false;
    }

    // we've received the words of the book, get the definition of these words
    downloadWordsOfBook(bookName);

    return true;
}

bool ServerAgent::handleResponseGetBookWordList(const MessageHeader &msgHeader)
{
    // avoid unused parameter warning
    msgHeader.toString();

    QString bookName;
    QVector<QString> wordList;
    QDataStream in(m_tcpSocket);
    in.startTransaction();
    in >> bookName >> wordList;
    if (in.commitTransaction() == false)
    {
        // in this case, the transaction is restored by commitTransaction()
        qDebug() << "failed to read words of the book in handleResponseGetWordsOfBook()";
        return false;
    }

    bookName = bookName.replace(ServerClientProtocol::partPrefixReplaceRegExp(), "");
    auto currentList = m_mapBooksWordList.value(bookName);
    auto newList = currentList + wordList;
    m_mapBooksWordList.insert(bookName, newList);

    return true;
}

void ServerAgent::onInternalFileDataDownloaded(QString fileName, bool succeeded)
{
    //funcTracker ft("onInternalFileDataDownloaded()");
    if (m_filesToDownload.value(fileName) == WaitingDataFromServer)
    {
        bool ok = succeeded;
        DownloadStatus saveResult = DownloadFailed; // failed
        if (succeeded == true)
        {
            if (saveFileFromServer(fileName) == true)
            {
                saveResult = DownloadSucceeded; // successfully saved
            }
            else
            {
                saveResult = DownloadFailed; // failed
                ok = false;
            }
        }

        m_filesToDownload.insert(fileName, saveResult);
        emit(fileDownloaded(fileName, ok));
    }
    else
    {
        m_mapFileContent.remove(fileName);  // remove the content since it's cancelled
    }

    float percentage = getProgressPercentage(m_filesToDownload);
    emit(downloadProgress(percentage));

    if (percentage >= 1.0f)
    {
        // don't clear it, no meaning!
        //m_filesToDownload.clear();
    }
}

bool ServerAgent::handleResponseGetAWord(const MessageHeader &msgHeader)
{
    // avoid unused parameter warning
    msgHeader.toString();

    Word word;
    QDataStream in(m_tcpSocket);
    in.startTransaction();
    in >> word;
    if (in.commitTransaction() == false)
    {
        // in this case, the transaction is restored by commitTransaction()
        qDebug() << "failed to read words of the book in handleResponseGetAWord()";
        return false;
    }

    // store the word
    QString spelling = word.getSpelling();
    sptr<Word> newWord = new Word(word);
    m_mapWords.insert(spelling, newWord); // just insert it as the word should NOT exist at this moment

    if (m_wordsToDownload.value(spelling) == WaitingDataFromServer)
    {
        m_wordsToDownload.insert(spelling, DownloadSucceeded);
        emit(wordDownloaded(spelling));
    }

    float percentage = getProgressPercentage(m_wordsToDownload);
    emit(downloadProgress(percentage));

    if (percentage >= 1.0f)
    {
        // we should NOT clear it here, as we need it in handleResponseGetWordsOfBookFinished()
        //m_wordsToDownload.clear();
    }

    return true;
}

float ServerAgent::getProgressPercentage(const QMap<QString, DownloadStatus> mapToDownload)
{
    int finished = mapToDownload.keys(DownloadSucceeded).size()
            + mapToDownload.keys(DownloadFailed).size()
            + mapToDownload.keys(DownloadCancelled).size();
    int total = mapToDownload.size();

    return finished * 1.0f / total;
}

bool ServerAgent::handleResponseGetWordsOfBookFinished(const MessageHeader &msgHeader)
{
    // avoid unused parameter warning
    msgHeader.toString();

    QString bookName;
    QDataStream in(m_tcpSocket);
    in.startTransaction();
    in >> bookName;
    if (in.commitTransaction() == false)
    {
        // in this case, the transaction is restored by commitTransaction()
        qDebug() << "failed to read the book name in handleResponseGetWordsOfBookFinished()";
        return false;
    }

    // it's possible that the user cancel the downloading, but we still got this message
    // so we check if there's cancel from the user
    int numberOfCancelledWords = m_wordsToDownload.keys(DownloadCancelled).size();
    if (numberOfCancelledWords == 0)
    {
        // downloading the words finished, the book data is ready
        emit(internalBookDataDownloaded(bookName));
        // m_mapWords would be cleared after saving the words in OnInternalBookDataDownloaded()
        qDebug() << bookName << "downloaded in handleResponseGetWordsOfBookFinished()";
    }
    else
    {
        qDebug() << bookName << "download cancelled in handleResponseGetWordsOfBookFinished()";
        m_mapWords.clear();
    }

    // don't clear it. It will be cleared once it's going to be used again in downloadWordsOfBook()
    //m_wordsToDownload.clear();

    return true;
}

void ServerAgent::completeBookDownload(QString bookName)
{
    auto book = m_mapBooks.value(bookName);
    if (book.get() == nullptr)
    {
        // the book does NOT exist in m_mapBooks?! it should NOT be the case!
        return;
    }

    auto wordList = m_mapBooksWordList.value(bookName);
    WordBook::storeBookFromServer(book, wordList);

    emit(bookDownloaded(bookName));
}

bool ServerAgent::saveFileFromServer(QString fileName)
{
    QString localFile = MySettings::dataDirectory() + "/" + fileName;
    QFile toSave(localFile);

    QString folder = localFile.section('/', 0, -2);
    QDir::current().mkpath(folder);

    if (toSave.open(QIODevice::WriteOnly) == false)
    {
        qInfo("Could not open %s for writing: %s", localFile.toUtf8().constData(), toSave.errorString().toUtf8().constData());
        return false;
    }

    QByteArray content = m_mapFileContent.value(fileName);
    qDebug() << "saving" << fileName << "size" << content.size();
    toSave.write(content.constData(), content.size());
    m_mapFileContent.remove(fileName);  // remove the content since it's now saved to the disk

    return true;
}

bool ServerAgent::handleResponseGetABook(const MessageHeader &msgHeader)
{
    // avoid unused parameter warning
    msgHeader.toString();

    WordBook book;
    QDataStream in(m_tcpSocket);
    in.startTransaction();
    in >> book;
    if (in.commitTransaction() == false)
    {
        // in this case, the transaction is restored by commitTransaction()
        qDebug() << "failed to read words of the book in handleResponseGetABook()";
        return false;
    }

    // store the book temporary in m_mapBooks, it will be saved to local database in completeBookDownload()
    sptr<WordBook> newBook = new WordBook(book);
    m_mapBooks.insert(book.getName(), newBook);
    //qDebug() << book.getId() << book.getName() << book.getIntroduction();

    return true;
}

bool ServerAgent::handleResponseGetFile(const MessageHeader &msgHeader)
{
    // avoid unused parameter warning
    msgHeader.toString();

    QString fileName;
    char *data;
    uint len;
    QDataStream in(m_tcpSocket);
    in.startTransaction();
    in >> fileName;
    in.readBytes(data, len);
    if (in.commitTransaction() == false)
    {
        // in this case, the transaction is restored by commitTransaction()
        qDebug() << "failed to read words of the book in handleResponseGetFile()";
        return false;
    }

    //qDebug() << "recevied data of" << fileName << len;
    fileName = fileName.replace(ServerClientProtocol::partPrefixReplaceRegExp(), "");
    auto currentContent = m_mapFileContent.value(fileName);
    auto newContent = currentContent + QByteArray(data, static_cast<int>(len));
    m_mapFileContent.insert(fileName, newContent);

    return true;
}

bool ServerAgent::handleResponseGetFileFinished(const MessageHeader &msgHeader)
{
    // avoid unused parameter warning
    msgHeader.toString();

    //funcTracker ft("handleResponseAllDataSentForRequestGetFile()");
    QDataStream in(m_tcpSocket);
    QString fileName;
    bool succeeded;
    in.startTransaction();
    in >> fileName >> succeeded;
    if (in.commitTransaction() == false)
    {
        // in this case, the transaction is restored by commitTransaction()
        qDebug() << "failed to read books in handleResponseAllDataSentForRequestGetFile()";
        return false;
    }

    emit(internalFileDataDownloaded(fileName, succeeded));

    return true;
}

bool ServerAgent::handleResponseUnknownRequest(const MessageHeader &msgHeader)
{
    // avoid unused parameter warning
    msgHeader.toString();

    QDataStream in(m_tcpSocket);
    int requestCode;
    in.startTransaction();
    in >> requestCode;
    if (in.commitTransaction() == false)
    {
        // in this case, the transaction is restored by commitTransaction()
        qDebug() << "failed to read books in handleResponseGetAllBooks()";
        return false;
    }

    qDebug() << "the server responed that failed to handle request with code" << requestCode;

    return true;
}

void ServerAgent::connectToServer()
{
    if (m_tcpSocket != nullptr)
    {
        return;
    }

    m_tcpSocket = new QTcpSocket(this);
    connect(m_tcpSocket, SIGNAL(connected()), this, SLOT(onConnected()));
    connect(m_tcpSocket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    connect(m_tcpSocket, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
    connect(m_tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)));
    connect(m_tcpSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(onStateChanged(QAbstractSocket::SocketState)));

    //connect(this, SIGNAL(responseGetABook(const WordBook &)), this, SLOT(onResponseGetABook(const WordBook &)));
    //connect(this, SIGNAL(responseGetWordsOfBook(QString, QVector<QString>)), this, SLOT(onResponseGetWordsOfBook(QString, QVector<QString>)));
    //connect(this, SIGNAL(responseGetAWord(const Word &)), this, SLOT(onResponseGetAWord(const Word &)));
    //connect(this, SIGNAL(responseGetAWord(const Word &)), this, SLOT(onResponseGetAWord(const Word &)));
    //connect(this, SIGNAL(responseGetAWord(const Word &)), this, SLOT(onResponseGetAWord(const Word &)));

    m_tcpSocket->connectToHost(m_serverHostName, m_serverPort);

    // it seems waitForConnected() helps to get/trigger the error ConnectionRefusedError
    if (m_tcpSocket->waitForConnected() == false)
    {
        qDebug() << "waitForConnected() failed" << m_tcpSocket->error();
    }
}


/**
 * @brief ServerAgent::sendRequestNoOperation
 * this is used as heartbeat at this moment, so it sends the message directly
 */
void ServerAgent::sendRequestNoOperation()
{
    connectToServer();
    MessageHeader msgHeader(ServerClientProtocol::RequestNoOperation);

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << msgHeader;
    m_tcpSocket->write(block);
    //m_messages.append(block);
}

void ServerAgent::sendRequestBye()
{
    if (m_tcpSocket == nullptr)
    {
        // if it's NOT connected, no need to say good bye
        return;
    }

    MessageHeader msgHeader(ServerClientProtocol::RequestBye);

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << msgHeader;
    m_tcpSocket->write(block);
    //m_messages.append(block);
}

void ServerAgent::sendRequestGetAllBooks()
{
    connectToServer();
    MessageHeader msgHeader(ServerClientProtocol::RequestGetAllBooks);

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << msgHeader;
     //m_tcpSocket->write(block);
    m_messages.append(block);
}

void ServerAgent::sendRequestGetBookWordList(QString bookName)
{
    connectToServer();
    MessageHeader msgHeader(ServerClientProtocol::RequestGetBookWordList);

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << msgHeader << bookName;
     //m_tcpSocket->write(block);
    m_messages.append(block);
}

void ServerAgent::sendRequestGetWordsOfBookFinished(QString bookName)
{
    connectToServer();
    MessageHeader msgHeader(ServerClientProtocol::RequestGetWordsOfBookFinished);

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << msgHeader << bookName;
     //m_tcpSocket->write(block);
    m_messages.append(block);
}

void ServerAgent::sendRequestGetAWord(QString spelling)
{
    connectToServer();
    MessageHeader msgHeader(ServerClientProtocol::RequestGetAWord);

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << msgHeader << spelling;
    //m_tcpSocket->write(block);
    m_messages.append(block);
}

void ServerAgent::sendRequestGetABook(QString bookName)
{
    connectToServer();
    MessageHeader msgHeader(ServerClientProtocol::RequestGetABook);

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << msgHeader << bookName;
     //m_tcpSocket->write(block);
    m_messages.append(block);
}

void ServerAgent::sendRequestGetFile(QString fileName)
{
    connectToServer();
    MessageHeader msgHeader(ServerClientProtocol::RequestGetFile);

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << msgHeader << fileName;
     //m_tcpSocket->write(block);
    m_messages.append(block);
}

void ServerAgent::downloadBook(QString bookName)
{
    if (WordBook::getBook(bookName).get() != nullptr)
    {
        // the book exists locally, nothing to do
        return;
    }
    else
    {
        sendRequestGetABook(bookName);
        sendRequestGetBookWordList(bookName);
    }
}

/**
 * @brief ServerAgent::downloadFile
 * @param fileName
 * assumes teh file does not exist, overwrite it if it exists!
 */
void ServerAgent::downloadFile(QString fileName)
{
    if (m_filesToDownload.contains(fileName) == false)
    {
        m_filesToDownload.insert(fileName, WaitingDataFromServer);  // mark it as request has been sent
        sendRequestGetFile(fileName);
    }
    else
    {
        // it's already in the list, so we don't touch it to spoil the state
    }
}

bool ServerAgent::fileExistsLocally(QString fileName)
{
    const QString dd = MySettings::dataDirectory() + "/";
    return QFile::exists(dd + fileName);
}

const QMap<QString, ServerAgent::DownloadStatus> &ServerAgent::downloadMultipleFiles(QSet<QString> files)
{
    m_filesToDownload.clear();
    m_mapFileContent.clear();
    QSet<QString>::const_iterator it = files.constBegin();
    while (it != files.constEnd())
    {
        QString fileName = *it;
        // ServerAgent downloads the file regardless the existence of the file!!!
        //if (QFile::exists(dd + fileName) == false)
        {
            downloadFile(fileName);
        }
        it ++;
    }

    return m_filesToDownload;
}

void ServerAgent::cancelDownloading()
{
    cancelDownloadingFiles();
    cancelDownloadingWords();

    // just discard the messages!
    m_messages.clear();
}

void ServerAgent::disconnectServer()
{
    sendRequestBye();
}

void ServerAgent::cancelDownloadingFiles()
{
    auto filesLeft = m_filesToDownload.keys(WaitingDataFromServer);
    for (int i = 0;i < filesLeft.size();i ++)
    {
        //m_filesToDownload.remove(filesLeft.at(i));
        m_filesToDownload.insert(filesLeft.at(i), DownloadCancelled);
    }

    // the files downloaded have already been saved to disk, so no actions
}

void ServerAgent::cancelDownloadingWords()
{
    auto wordsLeft = m_wordsToDownload.keys(WaitingDataFromServer);
    for (int i = 0;i < wordsLeft.size();i ++)
    {
        //m_filesToDownload.remove(filesLeft.at(i));
        m_wordsToDownload.insert(wordsLeft.at(i), DownloadCancelled);
    }

    // the downloaded words are in m_mapWords, so clear them
    m_mapWords.clear();
}

void ServerAgent::onSendMessageSmart()
{
    int requestsForARound = MySettings::numberOfRequestInEveryDownloadRound();
    if ((m_messagesSent - m_lastResponded) > (requestsForARound / 5))
    {
        qDebug() << "waiting: m_messagesSent" << m_messagesSent << "m_lastResponded" << m_lastResponded << "requestsForARound" << requestsForARound;

        // we are waiting for the messages to be processed
        return;
    }

    while (requestsForARound > 0)
    {
        sendTheFirstMessage();
        requestsForARound --;
    }
}

void ServerAgent::onSendMessage()
{
    int requestsForARound = MySettings::numberOfRequestInEveryDownloadRound();
    while (requestsForARound > 0)
    {
        sendTheFirstMessage();
        requestsForARound --;
    }
    //qDebug() << "MySettings::downloadIntervalInMilliseconds() is" << MySettings::downloadIntervalInMilliseconds();
}

void ServerAgent::sendTheFirstMessage()
{
    if (m_messages.isEmpty() == true)
    {
        return;
    }

    m_tcpSocket->write(m_messages.at(0));
    m_messages.pop_front();
    m_messagesSent ++;
}

void ServerAgent::downloadWordsOfBook(QString bookName)
{
    m_wordsToDownload.clear();
    auto wordList = m_mapBooksWordList.value(bookName);
    // get the list of words which are not available locally
    for (int i = 0;i < wordList.size();i ++)
    {
        auto spelling = wordList.at(i);

        if (Word::getWord(spelling).get() == nullptr && m_wordsToDownload.contains(spelling) == false)
        {
            m_wordsToDownload.insert(spelling, WaitingDataFromServer);  // mark it as request has been sent
            sendRequestGetAWord(spelling);
        }
    }

    if (m_wordsToDownload.isEmpty() == true)
    {
        // no need to download any word, download of the book completes here
        emit(internalBookDataDownloaded(bookName));
    }
    else
    {
        // send the message "RequestGetWordsOfBookFinished"
        // so we will know downloading words finished when the server replied this message
        sendRequestGetWordsOfBookFinished(bookName);
    }
}

void ServerAgent::getBookList()
{
    if (m_booksInServer.size() == 0)
    {
        // no book yet, send message to server to get books
        sendRequestGetAllBooks();
    }
    else
    {
        // books are already available
        emit(bookListReady(m_booksInServer));
    }
}

