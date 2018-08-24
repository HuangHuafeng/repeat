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
    connect(&m_messageTimer, SIGNAL(timeout()), this, SLOT(onSendMessage()));

    m_messageTimer.start(MySettings::downloadIntervalInMilliseconds());

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
        m_serveragent = new ServerAgent("huafengsmac");
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
    static int currentMessage = 0;

    do {
        if (currentMessage == 0)
        {
            // last message processed completed, it's time for a new message
            currentMessage = readMessageCode();
        }

        if (currentMessage != 0)
        {
            // we have a message, try to process it
            int handleResult = handleMessage(currentMessage);
            if (handleResult == 0)
            {
                // successfully processed the message
                //qDebug() << "successfully handled message with code" << currentMessage;
                currentMessage = 0;
            }
            else if (handleResult == 1)
            {
                qDebug() << "failed to handle message with code" << currentMessage;

                // failed to process the message, probably means the content of the message is NOT fully available
                // so quit the loop and contine in next call of onReadyRead()
                break;
            }
            else
            {
                // handleResult == -1, unknown message!
                // discard the message and continue trying to get the next message
                currentMessage = 0;
            }
        }
        else
        {
            // no messages available, quit the loop
            break;
        }
    } while (1);
}

void ServerAgent::onConnected()
{
    qDebug() << "onConnected()";

    // start heartbeat timer
    m_timerServerHeartBeat.start(1000 * MySettings::heartbeatIntervalInSeconds());
}

void ServerAgent::onDisconnected()
{
    // stop heartbeat timer
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

int ServerAgent::readMessageCode()
{
    int messageCode;
    QDataStream in(m_tcpSocket);
    in.startTransaction();
    in >> messageCode;
    if (in.commitTransaction() == true)
    {
        return messageCode;
    }
    else
    {
        // in this case, the transaction is restored by commitTransaction()
        //qInfo("failed to read message code in readMessageCode(), probably no data from peer");
        return 0;
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

int ServerAgent::handleMessage(int messageCode)
{
    if (m_tcpSocket == nullptr)
    {
        return -1;
    }

    bool handleResult = false;
    bool unknowMessage = false;
    switch (messageCode) {
    case ServerClientProtocol::ResponseNoOperation:
        handleResult = handleResponseNoOperation();
        break;
    case ServerClientProtocol::ResponseUnknownRequest:
        handleResult = handleResponseUnknownRequest();
        break;

    case ServerClientProtocol::ResponseGetAllBooks:
        handleResult = handleResponseGetAllBooks();
        break;

    case ServerClientProtocol::ResponseGetWordsOfBook:
        handleResult = handleResponseGetWordsOfBook();
        break;

    case ServerClientProtocol::ResponseGetAWord:
        handleResult = handleResponseGetAWord();
        break;

    case ServerClientProtocol::ResponseGetABook:
        handleResult = handleResponseGetABook();
        break;

    case ServerClientProtocol::ResponseAllDataSent:
        handleResult = handleResponseAllDataSent();
        break;

    case ServerClientProtocol::ResponseGetFile:
        handleResult = handleResponseGetFile();
        break;

    case ServerClientProtocol::ResponseGetWordsOfBookFinished:
        handleResult = handleResponseGetWordsOfBookFinished();
        break;

    default:
        handleUnknownMessage(messageCode);
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

bool ServerAgent::handleResponseNoOperation()
{
    qDebug() << "Heartbeat response received from the server";

    return true;
}

bool ServerAgent::handleUnknownMessage(int messageCode)
{
    qDebug() << "got unknown message with code" << messageCode;

    return true;
}

bool ServerAgent::handleResponseGetAllBooks()
{
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

bool ServerAgent::handleResponseGetWordsOfBook()
{
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

bool ServerAgent::handleResponseGetAWord()
{
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

bool ServerAgent::handleResponseAllDataSent()
{
    QDataStream in(m_tcpSocket);
    int messageCode;
    in.startTransaction();
    in >> messageCode;
    if (in.commitTransaction() == false)
    {
        // in this case, the transaction is restored by commitTransaction()
        qDebug() << "failed to read books in handleResponseGetAllBooks()";
        return false;
    }

    bool handleResult = false;
    switch (messageCode) {
    case ServerClientProtocol::RequestGetWordsOfBook:
        handleResult = handleResponseAllDataSentForRequestGetWordsOfBook();
        break;

    case ServerClientProtocol::RequestGetWords:
        handleResult = handleResponseAllDataSentForRequestGetWords();
        break;

    case ServerClientProtocol::RequestGetFile:
        handleResult = handleResponseAllDataSentForRequestGetFile();
        break;

    default:
        qDebug() << "unhandled message code" << messageCode << "in handleResponseAllDataSent()";
        handleResult = false;
        break;
    }

    return true;
}

bool ServerAgent::handleResponseGetWordsOfBookFinished()
{
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

bool ServerAgent::handleResponseAllDataSentForRequestGetWordsOfBook()
{
    QDataStream in(m_tcpSocket);
    QString bookName;
    in.startTransaction();
    in >> bookName;
    if (in.commitTransaction() == false)
    {
        // in this case, the transaction is restored by commitTransaction()
        qDebug() << "failed to read books in handleResponseAllDataSentForRequestGetWordsOfBook()";
        return false;
    }

    // we've received the words of the book, get the definition of these words
    downloadWordsOfBook(bookName);

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

bool ServerAgent::handleResponseAllDataSentForRequestGetWords()
{
    QDataStream in(m_tcpSocket);
    QString bookName;
    in.startTransaction();
    in >> bookName;
    if (in.commitTransaction() == false)
    {
        // in this case, the transaction is restored by commitTransaction()
        qDebug() << "failed to read books in handleResponseAllDataSentForRequestGetWordsOfBook()";
        return false;
    }

    if (bookName.startsWith(ServerClientProtocol::partPrefix()) == false)
    {
        emit(internalBookDataDownloaded(bookName));
    }
    else
    {
        qDebug() << "received part" << bookName;
    }

    return true;
}

bool ServerAgent::handleResponseAllDataSentForRequestGetFile()
{
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

    if (fileName.startsWith(ServerClientProtocol::partPrefix()) == false)
    {
        emit(internalFileDataDownloaded(fileName, succeeded));
    }
    else
    {
        qDebug() << "received part" << fileName;
    }

    return true;
}

bool ServerAgent::handleResponseGetABook()
{
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

bool ServerAgent::handleResponseGetFile()
{
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

bool ServerAgent::handleResponseUnknownRequest()
{
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
    int messageCode = ServerClientProtocol::RequestNoOperation;

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << messageCode;
    m_tcpSocket->write(block);
    //m_messages.append(block);
}

void ServerAgent::sendRequestGetAllBooks()
{
    connectToServer();
    int messageCode = ServerClientProtocol::RequestGetAllBooks;

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << messageCode;
     //m_tcpSocket->write(block);
    m_messages.append(block);
}

void ServerAgent::sendRequestGetWordsOfBook(QString bookName)
{
    connectToServer();
    int messageCode = ServerClientProtocol::RequestGetWordsOfBook;

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << messageCode << bookName;
     //m_tcpSocket->write(block);
    m_messages.append(block);
}

void ServerAgent::sendRequestGetWordsOfBookFinished(QString bookName)
{
    connectToServer();
    int messageCode = ServerClientProtocol::RequestGetWordsOfBookFinished;

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << messageCode << bookName;
     //m_tcpSocket->write(block);
    m_messages.append(block);
}

void ServerAgent::sendRequestGetAWord(QString spelling)
{
    connectToServer();
    int messageCode = ServerClientProtocol::RequestGetAWord;

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << messageCode << spelling;
    //m_tcpSocket->write(block);
    m_messages.append(block);
}

void ServerAgent::sendRequestGetABook(QString bookName)
{
    connectToServer();
    int messageCode = ServerClientProtocol::RequestGetABook;

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << messageCode << bookName;
     //m_tcpSocket->write(block);
    m_messages.append(block);
}

void ServerAgent::sendRequestGetWords(QString bookName, QVector<QString> wordList)
{
    if (wordList.size() > ServerClientProtocol::MaximumWordsInAMessage)
    {
        return;
    }

    connectToServer();
    int messageCode = ServerClientProtocol::RequestGetWords;

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << messageCode << bookName << wordList;
     //m_tcpSocket->write(block);
    m_messages.append(block);
}

void ServerAgent::sendRequestGetWordsWithSmallMessages(QString bookName, QVector<QString> wordList)
{
    int total = wordList.size();
    int pos = 0;
    int counter = 0;
    while (pos + ServerClientProtocol::MaximumWordsInAMessage < total)
    {
        counter ++;
        QVector<QString> subList = wordList.mid(pos, ServerClientProtocol::MaximumWordsInAMessage);
        const QString partName = ServerClientProtocol::partPrefix(counter) + bookName;
        sendRequestGetWords(partName, subList);
        pos += ServerClientProtocol::MaximumWordsInAMessage;
    };

    QVector<QString> leftWords = wordList.mid(pos, ServerClientProtocol::MaximumWordsInAMessage);
    sendRequestGetWords(bookName, leftWords);
}

void ServerAgent::sendRequestGetFile(QString fileName)
{
    connectToServer();
    int messageCode = ServerClientProtocol::RequestGetFile;

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << messageCode << fileName;
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
        sendRequestGetWordsOfBook(bookName);
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

const QMap<QString, ServerAgent::DownloadStatus> &ServerAgent::downloadMultipleFiles(QList<QString> fileList)
{
    m_filesToDownload.clear();
    m_mapFileContent.clear();
    const QString dd = MySettings::dataDirectory() + "/";
    for (int i = 0;i < fileList.size();i ++)
    {
        QString fileName = fileList.at(i);
        if (QFile::exists(dd + fileName) == false)
        {
            downloadFile(fileName);
        }
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
}

/*
void ServerAgent::sendDownloadRequestsToServer()
{
    requestFiles();
    requestWords();

    if (m_filesToDownload.isEmpty() == true && m_wordsToDownload.isEmpty() == true)
    {
        // download finished
        m_downloadTimer.stop();
        qDebug() << "nothing to be downloaded, stopped the timer";
    }
}

void ServerAgent::requestFiles()
{
    if (m_filesToDownload.isEmpty() == true)
    {
        // if there's no files to download, return
        return;
    }

    int requestsForARound = MySettings::numberOfRequestInEveryDownloadRound();
    // check how many files are requested by not received
    int requestsInProcessing = m_filesToDownload.keys(1).size();
    if (requestsInProcessing > requestsForARound)
    {
        // we are still waiting requests to be replied from the server, so skip this round
        return;
    }

    auto filesToDownload = m_filesToDownload.keys(0);
    for (int i = 0;i < filesToDownload.size() && requestsForARound > 0;i ++)
    {
        QString fileName = filesToDownload.at(i);
        m_filesToDownload.insert(fileName, 1);  // mark it as request has been sent
        sendRequestGetFile(fileName);
        requestsForARound --;
    }
}

void ServerAgent::requestWords()
{
    if (m_wordsToDownload.isEmpty() == true)
    {
        // if there's no word to download, return
        return;
    }

    int requestsForARound = MySettings::numberOfRequestInEveryDownloadRound();
    // check how many files are requested by not received
    int requestsInProcessing = m_wordsToDownload.keys(1).size();
    if (requestsInProcessing > requestsForARound)
    {
        // we are still waiting requests to be replied from the server, so skip this round
        return;
    }

    auto wordsToDownload = m_wordsToDownload.keys(0);
    for (int i = 0;i < wordsToDownload.size() && requestsForARound > 0;i ++)
    {
        QString spelling = wordsToDownload.at(i);
        m_wordsToDownload.insert(spelling, 1);  // mark it as request has been sent
        sendRequestGetAWord(spelling);
        requestsForARound --;
    }
}
*/

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

