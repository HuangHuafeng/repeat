#include "servercommunicator.h"

ServerCommunicator * ServerCommunicator::m_sc = nullptr;

ServerCommunicator * ServerCommunicator::instance()
{
    if (m_sc == nullptr)
    {
        m_sc = new ServerCommunicator();
    }

    return m_sc;
}

ServerCommunicator::ServerCommunicator(QString hostName, quint16 port, QObject *parent) :
    QObject(parent),
    m_serverHostName(hostName),
    m_serverPort(port),
    m_tcpSocket(nullptr),
    m_messageTimer(this),
    m_timerServerHeartBeat(this)
{
    if (m_serverHostName.isEmpty() == true)
    {
        m_serverHostName = MySettings::serverHostName();
    }

    if (m_serverPort == 0)
    {
        m_serverPort = 61027;
    }

    connect(&m_timerServerHeartBeat, SIGNAL(timeout()), this, SLOT(onServerHeartBeat()));
    connect(&m_messageTimer, SIGNAL(timeout()), this, SLOT(onSendMessageSmart()));
}

ServerCommunicator::~ServerCommunicator()
{
    qDebug() << "ServerCommunicator::~ServerCommunicator() called";
}

void ServerCommunicator::onReadyRead()
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
            qDebug("successfully handled message with header: %s, message size: %d bytes", receivedMsgHeader.toString().toUtf8().constData(), msg.size());

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

void ServerCommunicator::onConnected()
{
    // renew the counters by a temporary message
    MessageHeader toBeDiscarded;
    m_messagesSent = toBeDiscarded.sequenceNumber();
    m_lastResponded = toBeDiscarded.sequenceNumber();

    // start heartbeat timer
    m_timerServerHeartBeat.start(1000 * MySettings::heartbeatIntervalInSeconds());
    // start sending message
    m_messageTimer.start(MySettings::downloadIntervalInMilliseconds());

    emit(serverConnected());
}

void ServerCommunicator::onDisconnected()
{
    m_messageTimer.stop();
    m_timerServerHeartBeat.stop();
    m_messages.clear();
    m_tcpSocket->deleteLater();
    m_tcpSocket = nullptr;
}

void ServerCommunicator::onError(QAbstractSocket::SocketError socketError)
{
    qDebug() << "onError()" << socketError;
    m_tcpSocket->deleteLater();
    m_tcpSocket = nullptr;
}

void ServerCommunicator::onStateChanged(QAbstractSocket::SocketState socketState)
{
    qDebug() << "onStateChanged()" << socketState;
}

QByteArray ServerCommunicator::readMessage()
{
    Q_ASSERT(m_tcpSocket != nullptr);

    bool compressed;
    QByteArray msg;
    QDataStream in(m_tcpSocket);
    in.startTransaction();
    in >> compressed >> msg;
    if (in.commitTransaction() == true)
    {
        if (compressed == true)
        {
            qDebug() << "received compressed message with size" << msg.size() << "bytes.";
            return qUncompress(msg);
        }
        else
        {
            return msg;
        }
    }
    else
    {
        // in this case, the transaction is restored by commitTransaction()
        return QByteArray();
    }
}

void ServerCommunicator::onServerHeartBeat()
{
    sendRequestNoOperation();
}

int ServerCommunicator::handleMessage(const QByteArray &msg)
{
    MessageHeader receivedMsgHeader(msg);
    bool handleResult = false;
    bool unknowMessage = false;
    switch (receivedMsgHeader.code()) {
    case ServerClientProtocol::ResponseOK:
        handleResult = handleResponseOK(msg);
        break;

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

    case ServerClientProtocol::ResponseGetFile:
        handleResult = handleResponseGetFile(msg);
        break;

    case ServerClientProtocol::ResponseGetFileFinished:
        handleResult = handleResponseGetFileFinished(msg);
        break;

    case ServerClientProtocol::ResponseUnknownRequest:
        handleResult = handleResponseUnknownRequest(msg);
        break;

    case ServerClientProtocol::ResponseRegister:
        handleResult = handleResponseRegister(msg);
        break;

    case ServerClientProtocol::ResponseLogin:
        handleResult = handleResponseLogin(msg);
        break;

    case ServerClientProtocol::ResponseInvalidTokenId:
        handleResult = handleResponseInvalidTokenId(msg);
        break;

    case ServerClientProtocol::ResponseLogout:
        handleResult = handleResponseLogout(msg);
        break;

    case ServerClientProtocol::ResponseAppVersion:
        handleResult = handleResponseAppVersion(msg);
        break;

    case ServerClientProtocol::ResponseUpgraderVersion:
        handleResult = handleResponseUpgraderVersion(msg);
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

bool ServerCommunicator::handleResponseOK(const QByteArray &msg)
{
    Q_ASSERT(msg.size() >= 0);

    return true;
}

bool ServerCommunicator::handleResponseInvalidTokenId(const QByteArray &msg)
{
    Q_ASSERT(msg.size() >= 0);

    return true;
}

bool ServerCommunicator::handleResponseNoOperation(const QByteArray &msg)
{
    Q_ASSERT(msg.size() >= 0);

    return true;
}

bool ServerCommunicator::handleUnknownMessage(const QByteArray &msg)
{
    Q_ASSERT(msg.size() >= 0);

    return true;
}

bool ServerCommunicator::handleResponseGetAllBooks(const QByteArray &msg)
{
    QDataStream in(msg);
    MessageHeader receivedMsgHeader = MessageHeader::invalidMessageHeader;
    QList<QString> books;
    in.startTransaction();
    in >> receivedMsgHeader >> books;
    if (in.commitTransaction() == false)
    {
        qCritical() << "failed to read books in handleResponseGetAllBooks()";
        return false;
    }

    emit(bookListDownloaded(books));

    return true;
}

bool ServerCommunicator::handleResponseGetBookWordList(const QByteArray &msg)
{
    QDataStream in(msg);
    MessageHeader receivedMsgHeader = MessageHeader::invalidMessageHeader;
    QString bookName;
    QVector<QString> wordList;
    bool listComplete;
    in.startTransaction();
    in >> receivedMsgHeader >> bookName >> wordList >> listComplete;
    if (in.commitTransaction() == false)
    {
        qCritical() << "failed to read words of the book in handleResponseGetBookWordList()";
        return false;
    }

    auto currentList = m_mapBooksWordList.value(bookName);
    if (currentList == nullptr)
    {
        currentList = new QVector<QString>;
        m_mapBooksWordList.insert(bookName, currentList);
    }
    currentList->append(wordList);

    if (listComplete == true)
    {
        // we've got the full list
        emit(bookWordListReceived(bookName, *currentList));
        delete currentList;
        m_mapBooksWordList.remove(bookName);
    }

    return true;
}

bool ServerCommunicator::handleResponseGetAWord(const QByteArray &msg)
{
    QDataStream in(msg);
    MessageHeader receivedMsgHeader = MessageHeader::invalidMessageHeader;
    Word word;
    in.startTransaction();
    in >> receivedMsgHeader >> word;
    if (in.commitTransaction() == false)
    {
        qCritical() << "failed to read words of the book in handleResponseGetAWord()";
        return false;
    }

    if (m_wordsInDownloading.value(word.getSpelling(), DownloadCancelled) == WaitingDataFromServer)
    {
        emit(wordDownloaded(word));
    }
    else
    {
        // word download is cancelled
        qDebug() << "downloaing" << word.getSpelling() << "canceled in ServerCommunicator::handleResponseGetAWord()";
    }

    m_wordsInDownloading.remove(word.getSpelling());

    return true;
}

bool ServerCommunicator::handleResponseRegister(const QByteArray &msg)
{
    QDataStream in(msg);
    MessageHeader receivedMsgHeader = MessageHeader::invalidMessageHeader;
    qint32 result;
    ApplicationUser user = ApplicationUser::invalidUser;
    in.startTransaction();
    in >> receivedMsgHeader >> result >> user;
    if (in.commitTransaction() == false)
    {
        qCritical() << "failed to read info in handleResponseRegister()";
        return false;
    }

    emit(registerResult(result, user));

    return true;
}

bool ServerCommunicator::handleResponseLogin(const QByteArray &msg)
{
    QDataStream in(msg);
    MessageHeader receivedMsgHeader = MessageHeader::invalidMessageHeader;
    qint32 result;
    ApplicationUser user = ApplicationUser::invalidUser;
    Token token = Token::invalidToken;
    in.startTransaction();
    in >> receivedMsgHeader >> result >> user >> token;
    if (in.commitTransaction() == false)
    {
        qCritical() << "failed to read info in handleResponseLogin()";
        return false;
    }

    emit(loginResult(result, user, token));

    return true;
}

bool ServerCommunicator::handleResponseLogout(const QByteArray &msg)
{
    QDataStream in(msg);
    MessageHeader receivedMsgHeader = MessageHeader::invalidMessageHeader;
    qint32 result;
    QString name;
    in.startTransaction();
    in >> receivedMsgHeader >> result >> name;
    if (in.commitTransaction() == false)
    {
        qCritical() << "failed to read info in handleResponseLogout()";
        return false;
    }

    emit(logoutResult(result, name));

    return true;
}

bool ServerCommunicator::handleResponseGetABook(const QByteArray &msg)
{
    QDataStream in(msg);
    MessageHeader receivedMsgHeader = MessageHeader::invalidMessageHeader;
    WordBook book;
    in.startTransaction();
    in >> receivedMsgHeader >> book;
    if (in.commitTransaction() == false)
    {
        qCritical() << "failed to read words of the book in handleResponseGetABook()";
        return false;
    }

    emit(bookDownloaded(book));

    return true;
}

bool ServerCommunicator::handleResponseGetFile(const QByteArray &msg)
{
    QDataStream in(msg);
    MessageHeader receivedMsgHeader = MessageHeader::invalidMessageHeader;
    QString fileName;
    char *data;
    uint len, sentBytes, totalBytes;
    in.startTransaction();
    in >> receivedMsgHeader >> fileName;
    in.readBytes(data, len);
    in >> sentBytes >> totalBytes;
    if (in.commitTransaction() == false)
    {
        qCritical() << "failed to read words of the book in handleResponseGetFile()";
        return false;
    }

    if (m_filesInDownloading.value(fileName, DownloadCancelled) == WaitingDataFromServer)
    {
        // the downloading is NOT cancelled
        QMap<const char *, uint> newBlock;
        newBlock.insert(data, len);
        auto contentBlocks = m_mapFileContentBlocks.value(fileName);
        if (contentBlocks == nullptr)
        {
            contentBlocks = new QVector<QMap<const char *, uint>>;
            m_mapFileContentBlocks.insert(fileName, contentBlocks);
        }
        contentBlocks->append(newBlock);

        // signal the progress of the current file
        emit(fileDownloadProgress(fileName, sentBytes * 1.0f / totalBytes));
    }
    else
    {
        delete [] data;
        qDebug() << "downloading is cancelled, disarding the received data!";
    }

    return true;
}

bool ServerCommunicator::handleResponseGetFileFinished(const QByteArray &msg)
{
    QDataStream in(msg);
    MessageHeader receivedMsgHeader = MessageHeader::invalidMessageHeader;
    QString fileName;
    bool succeeded;
    in.startTransaction();
    in >> receivedMsgHeader >> fileName >> succeeded;
    if (in.commitTransaction() == false)
    {
        qCritical() << "failed to read info in handleResponseGetFileFinished()";
        return false;
    }

    if (m_filesInDownloading.value(fileName, DownloadCancelled) == WaitingDataFromServer)
    {
        DownloadStatus result = succeeded ? DownloadSucceeded : DownloadFailed;
        emit(fileDownloaded(fileName, result, m_mapFileContentBlocks.value(fileName)));
    }
    else
    {
        // file download is cancelled
        qDebug() << "downloaing" << fileName << "canceled in ServerCommunicator::handleResponseGetFileFinished()";
    }

    discardFileContent(fileName);
    m_filesInDownloading.remove(fileName);

    return true;
}

void ServerCommunicator::discardFileContent(QString fileName)
{
    QVector<QMap<const char *, uint>> *fileContentBlocks = m_mapFileContentBlocks.value(fileName);
    if (fileContentBlocks != nullptr)
    {
        for (int i = 0;i < fileContentBlocks->size();i ++)
        {
            auto currentBlock = fileContentBlocks->at(i);
            // free the memory allocated in.readBytes() in handleRequestUploadAFile()
            delete [] currentBlock.firstKey();
        }

        // free the memory allocated in handleRequestUploadAFile()
        delete fileContentBlocks;
    }

    // clear the file in the map
    m_mapFileContentBlocks.remove(fileName);
}

bool ServerCommunicator::handleResponseUnknownRequest(const QByteArray &msg)
{
    QDataStream in(msg);
    MessageHeader receivedMsgHeader = MessageHeader::invalidMessageHeader;
    int requestCode;
    in.startTransaction();
    in >> receivedMsgHeader >> requestCode;
    if (in.commitTransaction() == false)
    {
        qCritical() << "failed to read request code in handleResponseUnknownRequest()";
        return false;
    }

    qCritical("server replied that it doesn't know the message with code: %d", requestCode);

    return true;
}

void ServerCommunicator::connectToServer()
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
        qDebug() << "waitForConnected() failed!";
    }
}

void ServerCommunicator::sendSimpleMessage(qint32 msgCode, bool now)
{
    MessageHeader msgHeader(msgCode);

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << msgHeader;
    sendMessage(block, false, now);
}

void ServerCommunicator::sendRequestRegister(const ApplicationUser &user)
{
    MessageHeader msgHeader(ServerClientProtocol::RequestRegister);

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << msgHeader << user;
    sendMessage(block);
}

void ServerCommunicator::sendRequestLogin(const ApplicationUser &user)
{
    MessageHeader msgHeader(ServerClientProtocol::RequestLogin);

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << msgHeader << user;
    sendMessage(block);
}

void ServerCommunicator::sendRequestLogout(QString name)
{
    MessageHeader msgHeader(ServerClientProtocol::RequestLogout);

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << msgHeader << name;
    sendMessage(block);
}

/**
 * @brief ServerCommunicator::sendRequestNoOperation
 * this is used as heartbeat at this moment, so it sends the message directly
 */
void ServerCommunicator::sendRequestNoOperation()
{
    sendSimpleMessage(ServerClientProtocol::RequestNoOperation, true);
}

void ServerCommunicator::sendRequestBye()
{
    if (m_tcpSocket == nullptr)
    {
        // if it's NOT connected, no need to say good bye
        return;
    }

    sendSimpleMessage(ServerClientProtocol::RequestBye, true);
}

void ServerCommunicator::sendRequestGetAllBooks()
{
    sendSimpleMessage(ServerClientProtocol::RequestGetAllBooks);
}

void ServerCommunicator::sendRequestGetBookWordList(QString bookName)
{
    MessageHeader msgHeader(ServerClientProtocol::RequestGetBookWordList);

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << msgHeader << bookName;
    sendMessage(block);
}

void ServerCommunicator::downloadWord(QString spelling)
{
    m_wordsInDownloading.insert(spelling, WaitingDataFromServer);
    sendRequestGetAWord(spelling);
}

void ServerCommunicator::sendRequestGetAWord(QString spelling)
{
    MessageHeader msgHeader(ServerClientProtocol::RequestGetAWord);

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << msgHeader << spelling;
    sendMessage(block);
}

void ServerCommunicator::sendRequestGetABook(QString bookName)
{
    MessageHeader msgHeader(ServerClientProtocol::RequestGetABook);

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << msgHeader << bookName;
    sendMessage(block);
}

void ServerCommunicator::downloadFile(QString fileName, bool appFile)
{
    m_filesInDownloading.insert(fileName, WaitingDataFromServer);
    if (appFile == true)
    {
        sendRequestGetAppFile(fileName);
    }
    else
    {
        sendRequestGetFile(fileName);
    }
}

void ServerCommunicator::cancelDownloading()
{
    // clear the messages, so no downloading request will be sent to the server
    // it is OK to clear all messages, as normally downloading requests are waiting in the queue
    m_messages.clear();

    // clear the current downloading files/words, so when data of these files/words are received
    // from the server, the data will be discarded
    m_filesInDownloading.clear();
    m_wordsInDownloading.clear();
}

void ServerCommunicator::sendRequestGetFile(QString fileName)
{
    MessageHeader msgHeader(ServerClientProtocol::RequestGetFile);

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << msgHeader << fileName;
    sendMessage(block);
}

void ServerCommunicator::sendRequestGetAppFile(QString fileName)
{
    MessageHeader msgHeader(ServerClientProtocol::RequestGetAppFile);

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << msgHeader << fileName;
    sendMessage(block);
}

void ServerCommunicator::disconnectServer()
{
    sendRequestBye();
}

void ServerCommunicator::sendMessage(const QByteArray &msg, bool needCompress, bool now)
{
    connectToServer();
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);

    if (needCompress == true)
    {
        out << true << qCompress(msg);
    }
    else
    {
        out << false << msg;
    }

    if (now == false)
    {
        m_messages.append(block);
    }
    else
    {
        m_tcpSocket->write(block);
    }
}

void ServerCommunicator::onSendMessageSmart()
{
    int requestsForARound = MySettings::numberOfRequestInEveryDownloadRound();
    Q_ASSERT(m_messagesSent - m_lastResponded < requestsForARound * 2);
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

void ServerCommunicator::sendTheFirstMessage()
{
    Q_ASSERT(m_messages.isEmpty() == false);
    m_tcpSocket->write(m_messages.at(0));
    m_messages.pop_front();
    m_messagesSent ++;
}

bool ServerCommunicator::handleResponseAppVersion(const QByteArray &msg)
{
    QDataStream in(msg);
    MessageHeader receivedMsgHeader = MessageHeader::invalidMessageHeader;
    ReleaseInfo appRelaseInfo, appLibRelaseInfo;
    in.startTransaction();
    in >> receivedMsgHeader >> appRelaseInfo >> appLibRelaseInfo;
    if (in.commitTransaction() == false)
    {
        qCritical() << "failed to read info in handleResponseAppVersion()";
        return false;
    }

    emit(appVersion(appRelaseInfo, appLibRelaseInfo));

    return true;
}

bool ServerCommunicator::handleResponseUpgraderVersion(const QByteArray &msg)
{
    QDataStream in(msg);
    MessageHeader receivedMsgHeader = MessageHeader::invalidMessageHeader;
    ReleaseInfo upgraderRelaseInfo, upgraderLibRelaseInfo;
    in.startTransaction();
    in >> receivedMsgHeader >> upgraderRelaseInfo >> upgraderLibRelaseInfo;
    if (in.commitTransaction() == false)
    {
        qCritical() << "failed to read info in handleResponseUpgraderVersion()";
        return false;
    }

    emit(upgraderVersion(upgraderRelaseInfo, upgraderLibRelaseInfo));

    return true;
}

void ServerCommunicator::sendRequestAppVersion(QString platform)
{
    MessageHeader msgHeader(ServerClientProtocol::RequestAppVersion);

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << msgHeader << platform;
    sendMessage(block);
}

void ServerCommunicator::sendRequestUpgraderVersion(QString platform)
{
    MessageHeader msgHeader(ServerClientProtocol::RequestUpgraderVersion);

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << msgHeader << platform;
    sendMessage(block);
}

