#include "svragt.h"
#include "serverclientprotocol.h"
#include "mysettings.h"

SvrAgt::SvrAgt(const QString &hostName, quint16 port, QObject *parent) : QObject(parent),
    m_serverHostName(hostName),
    m_serverPort(port),
    m_tcpSocket(nullptr),
    m_messageTimer(this),
    m_timerServerHeartBeat(this)
{
    connect(&m_timerServerHeartBeat, SIGNAL(timeout()), this, SLOT(onServerHeartBeat()));
    connect(&m_messageTimer, SIGNAL(timeout()), this, SLOT(onSendMessageSmart()));
}

SvrAgt::~SvrAgt()
{
}

void SvrAgt::onReadyRead()
{
    // we should have a loop here because multiple messages may arrive at the same time
    do {
        QByteArray msg = readMessage();
        if (msg.isEmpty() == true)
        {
            // can't get a message, we need more data
            break;
        }

        // we have a message, try to process it
        MessageHeader receivedMsgHeader(msg);
        int handleResult = handleMessage(msg);
        if (handleResult == 0)
        {
            qDebug("successfully handled message with header: %s", receivedMsgHeader.toString().toUtf8().constData());

            // if it's not a heartbeat message, update the counters
            if (receivedMsgHeader.code() != ServerClientProtocol::ResponseNoOperation)
            {
                m_lastResponded = receivedMsgHeader.respondsTo();
                if (m_messagesSent < m_lastResponded)
                {
                    // this can happen if the user cancels the downloading
                    m_messagesSent = m_lastResponded;
                }
            }
        }
        else if (handleResult == 1)
        {
            qDebug("failed to handle message with header: %s", receivedMsgHeader.toString().toUtf8().constData());

            // failed to process the message, this should not happen at current implementation, just discard the message
        }
        else
        {
            qDebug("unknown message with header: %s", receivedMsgHeader.toString().toUtf8().constData());

            // discard the message and continue trying to get the next message
        }
    } while (1);
}

void SvrAgt::onConnected()
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

void SvrAgt::onDisconnected()
{
    m_messageTimer.stop();
    m_timerServerHeartBeat.stop();
    m_tcpSocket->deleteLater();
    m_tcpSocket = nullptr;
}

void SvrAgt::onError(QAbstractSocket::SocketError socketError)
{
    qDebug() << "onError()" << socketError;
}

void SvrAgt::onStateChanged(QAbstractSocket::SocketState socketState)
{
    qDebug() << "onStateChanged()" << socketState;
}

QByteArray SvrAgt::readMessage()
{
    if (m_tcpSocket == nullptr)
    {
        return QByteArray();
    }

    QByteArray msg;
    QDataStream in(m_tcpSocket);
    in.startTransaction();
    in >> msg;
    if (in.commitTransaction() == true)
    {
        return msg;
    }
    else
    {
        // in this case, the transaction is restored by commitTransaction()
        return QByteArray();
    }
}

void SvrAgt::onServerHeartBeat()
{
    sendRequestNoOperation();
}

int SvrAgt::handleMessage(const QByteArray &msg)
{
    MessageHeader receivedMsgHeader(msg);
    bool handleResult = false;
    bool unknowMessage = false;
    switch (receivedMsgHeader.code()) {
    case ServerClientProtocol::ResponseNoOperation:
        handleResult = handleResponseNoOperation(msg);
        break;

    case ServerClientProtocol::ResponseGetAllBooks:
        handleResult = handleResponseGetAllBooks(msg);
        break;

    case ServerClientProtocol::ResponseGetAWord:
        handleResult = handleResponseGetAWord(msg);
        break;

    case ServerClientProtocol::ResponseGetABook:
        handleResult = handleResponseGetABook(msg);
        break;

    case ServerClientProtocol::ResponseGetBookWordList:
        handleResult = handleResponseGetBookWordList(msg);
        break;

    case ServerClientProtocol::ResponseBookWordListAllSent:
        handleResult = handleResponseBookWordListAllSent(msg);
        break;

    case ServerClientProtocol::ResponseGetFile:
        handleResult = handleResponseGetFile(msg);
        break;

    case ServerClientProtocol::ResponseGetFileFinished:
        handleResult = handleResponseGetFileFinished(msg);
        break;

    case ServerClientProtocol::ResponseGetWordsOfBookFinished:
        handleResult = handleResponseGetWordsOfBookFinished(msg);
        break;

    case ServerClientProtocol::ResponseUnknownRequest:
        handleResult = handleResponseUnknownRequest(msg);
        break;

    default:
        handleUnknownMessage(msg);
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

bool SvrAgt::handleResponseNoOperation(const QByteArray &msg)
{
    qDebug() << msg;

    return true;
}

bool SvrAgt::handleUnknownMessage(const QByteArray &msg)
{
    qDebug() << msg;

    return true;
}

bool SvrAgt::handleResponseGetAllBooks(const QByteArray &msg)
{
    QDataStream in(msg);
    MessageHeader receivedMsgHeader(-1, -1, -1);
    QList<QString> books;
    in.startTransaction();
    in >> receivedMsgHeader >> books;
    if (in.commitTransaction() == false)
    {
        qCritical() << "failed to read books in handleResponseGetAllBooks()";
        return false;
    }

    emit(bookListReady(books));

    return true;
}

bool SvrAgt::handleResponseBookWordListAllSent(const QByteArray &msg)
{
    QDataStream in(msg);
    MessageHeader receivedMsgHeader(-1, -1, -1);
    QString bookName;
    in.startTransaction();
    in >> receivedMsgHeader >> bookName;
    if (in.commitTransaction() == false)
    {
        qCritical() << "failed to read book name in handleResponseBookWordListAllSent()";
        return false;
    }

    emit(bookWordListReceived(bookName, m_mapBooksWordList.value(bookName)));
    m_mapBooksWordList.remove(bookName);

    return true;
}

bool SvrAgt::handleResponseGetBookWordList(const QByteArray &msg)
{
    QDataStream in(msg);
    MessageHeader receivedMsgHeader(-1, -1, -1);
    QString bookName;
    QVector<QString> wordList;
    in.startTransaction();
    in >> receivedMsgHeader >> bookName >> wordList;
    if (in.commitTransaction() == false)
    {
        qCritical() << "failed to read words of the book in handleResponseGetBookWordList()";
        return false;
    }

    bookName = bookName.replace(ServerClientProtocol::partPrefixReplaceRegExp(), "");
    auto currentList = m_mapBooksWordList.value(bookName);
    auto newList = currentList + wordList;
    m_mapBooksWordList.insert(bookName, newList);

    return true;
}

bool SvrAgt::handleResponseGetAWord(const QByteArray &msg)
{
    QDataStream in(msg);
    MessageHeader receivedMsgHeader(-1, -1, -1);
    Word word;
    in.startTransaction();
    in >> receivedMsgHeader >> word;
    if (in.commitTransaction() == false)
    {
        qCritical() << "failed to read words of the book in handleResponseGetAWord()";
        return false;
    }

    // store the word
    QString spelling = word.getSpelling();
    sptr<Word> newWord = new Word(word);

    if (m_wordsToDownload.value(spelling) == WaitingDataFromServer)
    {
        m_wordsToDownload.insert(spelling, DownloadSucceeded);
        emit(wordDownloaded(newWord));
    }

    updateAndEmitProgress();

    return true;
}

bool SvrAgt::handleResponseGetWordsOfBookFinished(const QByteArray &msg)
{
    QDataStream in(msg);
    MessageHeader receivedMsgHeader(-1, -1, -1);
    QString bookName;
    in.startTransaction();
    in >> receivedMsgHeader >> bookName;
    if (in.commitTransaction() == false)
    {
        qCritical() << "failed to read the book name in handleResponseGetWordsOfBookFinished()";
        return false;
    }

    // it's possible that the user cancel the downloading, but we still got this message
    // so we check if there's cancel from the user
    int numberOfCancelledWords = m_wordsToDownload.keys(DownloadCancelled).size();
    if (numberOfCancelledWords == 0)
    {
        // downloading the words finished, the book data is ready
        emit(getWordsOfBookFinished(bookName));
    }

    return true;
}

bool SvrAgt::handleResponseGetABook(const QByteArray &msg)
{
    QDataStream in(msg);
    MessageHeader receivedMsgHeader(-1, -1, -1);
    WordBook book;
    in.startTransaction();
    in >> receivedMsgHeader >> book;
    if (in.commitTransaction() == false)
    {
        qCritical() << "failed to read words of the book in handleResponseGetABook()";
        return false;
    }

    sptr<WordBook> newBook = new WordBook(book);
    emit(bookDownloaded(newBook));

    return true;
}

bool SvrAgt::handleResponseGetFile(const QByteArray &msg)
{
    QDataStream in(msg);
    MessageHeader receivedMsgHeader(-1, -1, -1);
    QString fileName;
    char *data;
    uint len;
    in.startTransaction();
    in >> receivedMsgHeader >> fileName;
    in.readBytes(data, len);
    if (in.commitTransaction() == false)
    {
        qCritical() << "failed to read words of the book in handleResponseGetFile()";
        return false;
    }

    fileName = fileName.replace(ServerClientProtocol::partPrefixReplaceRegExp(), "");
    if (m_mapFileContent.contains(fileName) == true)
    {
        auto currentContent = m_mapFileContent.value(fileName);
        auto newContent = currentContent + QByteArray(data, static_cast<int>(len));
        m_mapFileContent.insert(fileName, newContent);
    }
    else
    {
        m_mapFileContent.insert(fileName, QByteArray(data, static_cast<int>(len)));
    }

    return true;
}

bool SvrAgt::handleResponseGetFileFinished(const QByteArray &msg)
{
    QDataStream in(msg);
    MessageHeader receivedMsgHeader(-1, -1, -1);
    QString fileName;
    bool succeeded;
    in.startTransaction();
    in >> receivedMsgHeader >> fileName >> succeeded;
    if (in.commitTransaction() == false)
    {
        qCritical() << "failed to read books in handleResponseAllDataSentForRequestGetFile()";
        return false;
    }

    if (m_filesToDownload.contains(fileName) == true
            && m_filesToDownload.value(fileName) == WaitingDataFromServer)
    {
        if (succeeded == true)
        {
            m_filesToDownload.insert(fileName, DownloadSucceeded);
        }
        else
        {
            m_filesToDownload.insert(fileName, DownloadFailed);
        }
        emit(fileDownloaded(fileName, m_filesToDownload.value(fileName), m_mapFileContent.value(fileName)));
    }
    else
    {
        // file download is cancelled when (m_filesToDownload.contains(fileName) == false)
    }

    // remove the file content to release the memory, helpful?
    m_mapFileContent.remove(fileName);

    // don't remove the file from m_filesToDownload to keep the progress calculation accurate
    //m_filesToDownload.remove(fileName);

    updateAndEmitProgress();

    return true;
}

bool SvrAgt::handleResponseUnknownRequest(const QByteArray &msg)
{
    QDataStream in(msg);
    MessageHeader receivedMsgHeader(-1, -1, -1);
    int requestCode;
    in.startTransaction();
    in >> receivedMsgHeader >> requestCode;
    if (in.commitTransaction() == false)
    {
        qCritical() << "failed to read request code in handleResponseUnknownRequest()";
        return false;
    }

    return true;
}

void SvrAgt::connectToServer()
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

    m_tcpSocket->connectToHost(m_serverHostName, m_serverPort);

    // it seems waitForConnected() helps to get/trigger the error ConnectionRefusedError
    if (m_tcpSocket->waitForConnected() == false)
    {
        qDebug() << "waitForConnected() failed, error:" << m_tcpSocket->error();
    }
}


/**
 * @brief SvrAgt::sendRequestNoOperation
 * this is used as heartbeat at this moment, so it sends the message directly
 */
void SvrAgt::sendRequestNoOperation()
{
    connectToServer();
    MessageHeader msgHeader(ServerClientProtocol::RequestNoOperation);

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << msgHeader;
    sendMessage(block, true);
}

void SvrAgt::sendRequestBye()
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
    sendMessage(block, true);
}

void SvrAgt::sendRequestGetAllBooks()
{
    connectToServer();
    MessageHeader msgHeader(ServerClientProtocol::RequestGetAllBooks);

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << msgHeader;
    sendMessage(block);
}

void SvrAgt::sendRequestGetBookWordList(QString bookName)
{
    connectToServer();
    MessageHeader msgHeader(ServerClientProtocol::RequestGetBookWordList);

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << msgHeader << bookName;
    sendMessage(block);
}

void SvrAgt::sendRequestGetWordsOfBookFinished(QString bookName)
{
    connectToServer();
    MessageHeader msgHeader(ServerClientProtocol::RequestGetWordsOfBookFinished);

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << msgHeader << bookName;
    sendMessage(block);
}

void SvrAgt::sendRequestGetAWord(QString spelling)
{
    connectToServer();
    MessageHeader msgHeader(ServerClientProtocol::RequestGetAWord);

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << msgHeader << spelling;
    sendMessage(block);
}

void SvrAgt::sendRequestGetABook(QString bookName)
{
    connectToServer();
    MessageHeader msgHeader(ServerClientProtocol::RequestGetABook);

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << msgHeader << bookName;
    sendMessage(block);
}

void SvrAgt::sendRequestGetFile(QString fileName)
{
    connectToServer();
    MessageHeader msgHeader(ServerClientProtocol::RequestGetFile);

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << msgHeader << fileName;
    sendMessage(block);
}

/**
 * @brief SvrAgt::downloadFile
 * @param fileName
 * assumes teh file does not exist, overwrite it if it exists!
 */
void SvrAgt::downloadFile(QString fileName)
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

const QMap<QString, SvrAgt::DownloadStatus> &SvrAgt::downloadMultipleFiles(QSet<QString> files)
{
    m_filesToDownload.clear();
    m_mapFileContent.clear();

    QSet<QString>::const_iterator it = files.constBegin();
    while (it != files.constEnd())
    {
        QString fileName = *it;
        downloadFile(fileName);
        it ++;
    }

    m_toDownload = m_filesToDownload.size();
    m_downloaded = 0;

    return m_filesToDownload;
}

void SvrAgt::cancelDownloading()
{
    cancelDownloadingFiles();
    cancelDownloadingWords();

    // just discard the messages!
    m_messages.clear();
}

void SvrAgt::disconnectServer()
{
    sendRequestBye();
}

void SvrAgt::sendMessage(const QByteArray &msg, bool now)
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << msg;

    if (now == false)
    {
        m_messages.append(block);
    }
    else
    {
        m_tcpSocket->write(block);
    }
}

void SvrAgt::updateAndEmitProgress()
{
    m_downloaded ++;
    emit(downloadProgress(m_downloaded * 1.0f / m_toDownload));
}

void SvrAgt::cancelDownloadingWords()
{
    m_wordsToDownload.clear();
}

void SvrAgt::cancelDownloadingFiles()
{
    m_filesToDownload.clear();
    m_mapFileContent.clear();
}

void SvrAgt::onSendMessageSmart()
{
    int requestsForARound = MySettings::numberOfRequestInEveryDownloadRound();
    if ((m_messagesSent - m_lastResponded) > (requestsForARound / 5))
    {
        qDebug() << "waiting: m_messagesSent" << m_messagesSent << "m_lastResponded" << m_lastResponded << "requestsForARound" << requestsForARound;

        // we are waiting for the messages to be processed
        return;
    }

    while (requestsForARound > 0 && m_messages.isEmpty() == false)
    {
        sendTheFirstMessage();
        requestsForARound --;
    }
}

void SvrAgt::sendTheFirstMessage()
{
    Q_ASSERT(m_messages.isEmpty() == false);
    m_tcpSocket->write(m_messages.at(0));
    m_messages.pop_front();
    m_messagesSent ++;
}


void SvrAgt::getBookList()
{
    sendRequestGetAllBooks();
}

void SvrAgt::downloadWords(const QVector<QString> &wordList)
{
    // clear m_wordsToDownload as previous download must finished
    m_wordsToDownload.clear();

    // send message to download the words
    for (int i = 0;i < wordList.size();i ++)
    {
        auto spelling = wordList.at(i);
        if (m_wordsToDownload.contains(spelling) == false)
        {
            m_wordsToDownload.insert(spelling, WaitingDataFromServer);  // mark it as request has been sent
            sendRequestGetAWord(spelling);
        }
    }

    m_toDownload = m_wordsToDownload.size();
    m_downloaded = 0;
}