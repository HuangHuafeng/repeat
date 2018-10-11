#include "svragt.h"
#include "../HaiBeiDanCi/serverclientprotocol.h"
#include "../HaiBeiDanCi/mysettings.h"

SvrAgt::SvrAgt(const QString &hostName, quint16 port, QObject *parent) : QObject(parent),
    m_serverHostName(hostName),
    m_serverPort(port),
    m_tcpSocket(nullptr),
    m_messageTimer(this),
    m_timerServerHeartBeat(this)
{
    connect(&m_timerServerHeartBeat, SIGNAL(timeout()), this, SLOT(onServerHeartBeat()));
    //connect(&m_timerServerHeartBeat, &QTimer::timeout, [] () {qDebug() << "lambda called";});
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

    emit(serverConnected());
}

void SvrAgt::onDisconnected()
{
    m_messageTimer.stop();
    m_timerServerHeartBeat.stop();
    m_messages.clear();
    m_tcpSocket->deleteLater();
    m_tcpSocket = nullptr;
}

void SvrAgt::onError(QAbstractSocket::SocketError socketError)
{
    qDebug() << "onError()" << socketError;
    m_tcpSocket->deleteLater();
    m_tcpSocket = nullptr;
}

void SvrAgt::onStateChanged(QAbstractSocket::SocketState socketState)
{
    qDebug() << "onStateChanged()" << socketState;
}

QByteArray SvrAgt::readMessage()
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

    case ServerClientProtocol::ResponseGetWordsOfBookFinished:
        handleResult = handleResponseGetWordsOfBookFinished(msg);
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

bool SvrAgt::handleResponseOK(const QByteArray &msg)
{
    Q_ASSERT(msg.size() >= 0);

    return true;
}

bool SvrAgt::handleResponseInvalidTokenId(const QByteArray &msg)
{
    Q_ASSERT(msg.size() >= 0);

    return true;
}

bool SvrAgt::handleResponseNoOperation(const QByteArray &msg)
{
    Q_ASSERT(msg.size() >= 0);

    return true;
}

bool SvrAgt::handleUnknownMessage(const QByteArray &msg)
{
    Q_ASSERT(msg.size() >= 0);

    return true;
}

bool SvrAgt::handleResponseGetAllBooks(const QByteArray &msg)
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

    emit(bookListReady(books));

    return true;
}

bool SvrAgt::handleResponseGetBookWordList(const QByteArray &msg)
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
    auto newList = currentList + wordList;
    m_mapBooksWordList.insert(bookName, newList);

    if (listComplete == true)
    {
        // we've got the full list
        emit(bookWordListReceived(bookName, m_mapBooksWordList.value(bookName)));
        m_mapBooksWordList.remove(bookName);
    }

    return true;
}

bool SvrAgt::handleResponseGetAWord(const QByteArray &msg)
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
    MessageHeader receivedMsgHeader = MessageHeader::invalidMessageHeader;
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

bool SvrAgt::handleResponseRegister(const QByteArray &msg)
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

bool SvrAgt::handleResponseLogin(const QByteArray &msg)
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

bool SvrAgt::handleResponseLogout(const QByteArray &msg)
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

bool SvrAgt::handleResponseGetABook(const QByteArray &msg)
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

    sptr<WordBook> newBook = new WordBook(book);
    emit(bookDownloaded(newBook));

    return true;
}

bool SvrAgt::handleResponseGetFile(const QByteArray &msg)
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

    if (m_filesToDownload.contains(fileName) == true
            && m_filesToDownload.value(fileName) == WaitingDataFromServer)
    {
        // the downloading is not cancelled
        QMap<const char *, uint> newBlock;
        newBlock.insert(data, len);
        auto contentBlocks = m_mapFileContentBlocks.value(fileName);
        if (contentBlocks == nullptr)
        {
            contentBlocks = new QVector<QMap<const char *, uint>>;
            m_mapFileContentBlocks.insert(fileName, contentBlocks);
        }
        contentBlocks->append(newBlock);

        if (sentBytes <= totalBytes)
        {
            emit(downloadProgress((m_downloaded + sentBytes * 1.0f / totalBytes) / m_toDownload));
        }
    }
    else
    {
        delete [] data;
        qDebug() << "downloading is cancelled, disarding the received data!";
    }

    return true;
}

bool SvrAgt::handleResponseGetFileFinished(const QByteArray &msg)
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
        emit(fileDownloaded(fileName, m_filesToDownload.value(fileName), m_mapFileContentBlocks.value(fileName)));
    }
    else
    {
        // file download is cancelled when (m_filesToDownload.contains(fileName) == false)
    }

    discardFileContent(fileName);

    // don't remove the file from m_filesToDownload
    // we may need some statistics later
    // we should remove it!!! Otherwise a failed file cannot be downloaded again!
    m_filesToDownload.remove(fileName);

    updateAndEmitProgress();

    return true;
}

void SvrAgt::discardFileContent(QString fileName)
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

bool SvrAgt::handleResponseUnknownRequest(const QByteArray &msg)
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
        qDebug() << "waitForConnected() failed!";
    }
}

void SvrAgt::sendSimpleMessage(qint32 msgCode, bool now)
{
    MessageHeader msgHeader(msgCode);

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << msgHeader;
    sendMessage(block, false, now);
}

void SvrAgt::sendRequestRegister(const ApplicationUser &user)
{
    MessageHeader msgHeader(ServerClientProtocol::RequestRegister);

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << msgHeader << user;
    sendMessage(block);
}

void SvrAgt::sendRequestLogin(const ApplicationUser &user)
{
    MessageHeader msgHeader(ServerClientProtocol::RequestLogin);

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << msgHeader << user;
    sendMessage(block);
}

void SvrAgt::sendRequestLogout(QString name)
{
    MessageHeader msgHeader(ServerClientProtocol::RequestLogout);

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << msgHeader << name;
    sendMessage(block);
}

/**
 * @brief SvrAgt::sendRequestNoOperation
 * this is used as heartbeat at this moment, so it sends the message directly
 */
void SvrAgt::sendRequestNoOperation()
{
    sendSimpleMessage(ServerClientProtocol::RequestNoOperation, true);
}

void SvrAgt::sendRequestBye()
{
    if (m_tcpSocket == nullptr)
    {
        // if it's NOT connected, no need to say good bye
        return;
    }

    sendSimpleMessage(ServerClientProtocol::RequestBye, true);
}

void SvrAgt::sendRequestGetAllBooks()
{
    sendSimpleMessage(ServerClientProtocol::RequestGetAllBooks);
}

void SvrAgt::sendRequestGetBookWordList(QString bookName)
{
    MessageHeader msgHeader(ServerClientProtocol::RequestGetBookWordList);

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << msgHeader << bookName;
    sendMessage(block);
}

void SvrAgt::sendRequestGetWordsOfBookFinished(QString bookName)
{
    MessageHeader msgHeader(ServerClientProtocol::RequestGetWordsOfBookFinished);

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << msgHeader << bookName;
    sendMessage(block);
}

void SvrAgt::sendRequestGetAWord(QString spelling)
{
    MessageHeader msgHeader(ServerClientProtocol::RequestGetAWord);

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << msgHeader << spelling;
    sendMessage(block);
}

void SvrAgt::sendRequestGetABook(QString bookName)
{
    MessageHeader msgHeader(ServerClientProtocol::RequestGetABook);

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << msgHeader << bookName;
    sendMessage(block);
}

void SvrAgt::sendRequestGetFile(QString fileName)
{
    MessageHeader msgHeader(ServerClientProtocol::RequestGetFile);

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << msgHeader << fileName;
    sendMessage(block);
}

void SvrAgt::sendRequestGetAppFile(QString fileName)
{
    MessageHeader msgHeader(ServerClientProtocol::RequestGetAppFile);

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << msgHeader << fileName;
    sendMessage(block);
}

void SvrAgt::sendRequestGetUpgrader(QString fileName)
{
    MessageHeader msgHeader(ServerClientProtocol::RequestGetUpgrader);

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << msgHeader << fileName;
    sendMessage(block);
}

/**
 * @brief SvrAgt::downloadFile
 * @param fileName
 * assumes the file does not exist, overwrite it if it exists!
 */
void SvrAgt::downloadFile(QString fileName)
{
    Q_ASSERT(m_filesToDownload.isEmpty() == true);
    Q_ASSERT(m_mapFileContentBlocks.isEmpty() == true);
    if (m_filesToDownload.contains(fileName) == false)
    {
        m_filesToDownload.insert(fileName, WaitingDataFromServer);  // mark it as request has been sent
        sendRequestGetFile(fileName);
    }

    m_toDownload = m_filesToDownload.size();
    m_downloaded = 0;
}

void SvrAgt::downloadAppFile(QString fileName)
{
    //Q_ASSERT(m_filesToDownload.isEmpty() == true);
    //Q_ASSERT(m_mapFileContentBlocks.isEmpty() == true);
    if (m_filesToDownload.contains(fileName) == false)
    {
        m_filesToDownload.insert(fileName, WaitingDataFromServer);  // mark it as request has been sent
        sendRequestGetAppFile(fileName);
        m_toDownload ++;
    }

    //m_toDownload = m_filesToDownload.size();
    //m_downloaded = 0;
}

void SvrAgt::downloadUpgrader(QString fileName)
{
    Q_ASSERT(m_filesToDownload.isEmpty() == true);
    Q_ASSERT(m_mapFileContentBlocks.isEmpty() == true);
    if (m_filesToDownload.contains(fileName) == false)
    {
        m_filesToDownload.insert(fileName, WaitingDataFromServer);  // mark it as request has been sent
        sendRequestGetUpgrader(fileName);
    }

    m_toDownload = m_filesToDownload.size();
    m_downloaded = 0;
}

void SvrAgt::downloadMultipleFiles(QSet<QString> files)
{
    Q_ASSERT(m_filesToDownload.isEmpty() == true);
    Q_ASSERT(m_mapFileContentBlocks.isEmpty() == true);

    int counter = 0;
    int requestsForARound = MySettings::numberOfRequestInEveryDownloadRound();
    QSet<QString>::const_iterator it = files.constBegin();
    while (it != files.constEnd())
    {
        QString fileName = *it;
        if (m_filesToDownload.contains(fileName) == false)
        {
            m_filesToDownload.insert(fileName, WaitingDataFromServer);  // mark it as request has been sent
            sendRequestGetFile(fileName);
        }
        it ++;

        if (counter ++ % requestsForARound == 0)
        {
            // process events so we don't make the app unresponsive
            QCoreApplication::processEvents(QEventLoop::ExcludeSocketNotifiers);
        }
    }

    m_toDownload = m_filesToDownload.size();
    m_downloaded = 0;
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

void SvrAgt::sendMessage(const QByteArray &msg, bool needCompress, bool now)
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

void SvrAgt::updateAndEmitProgress()
{
    Q_ASSERT(m_toDownload != 0);
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
    auto fileList = m_mapFileContentBlocks.keys();
    for (int i = 0;i < fileList.size();i ++)
    {
        discardFileContent(fileList.at(i));
    }
    m_mapFileContentBlocks.clear();
}

void SvrAgt::onSendMessageSmart()
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

void SvrAgt::sendTheFirstMessage()
{
    Q_ASSERT(m_messages.isEmpty() == false);
    m_tcpSocket->write(m_messages.at(0));
    m_messages.pop_front();
    m_messagesSent ++;
}

void SvrAgt::downloadWords(const QVector<QString> &wordList)
{
    // clear m_wordsToDownload as previous download must finished, NO!!!!
    //m_wordsToDownload.clear();
    // don't clear this as it breaks multiple downloading. In other words,
    // it's possible that there are books in downloading at this moment!

    int requestsForARound = MySettings::numberOfRequestInEveryDownloadRound();
    // send message to download the words
    for (int i = 0;i < wordList.size();i ++)
    {
        auto spelling = wordList.at(i);
        if (m_wordsToDownload.contains(spelling) == false)
        {
            m_wordsToDownload.insert(spelling, WaitingDataFromServer);  // mark it as request has been sent
            sendRequestGetAWord(spelling);
        }

        if (i % requestsForARound == 0)
        {
            // process events so we don't make the app unresponsive
            QCoreApplication::processEvents(QEventLoop::ExcludeSocketNotifiers);
        }
    }

    m_toDownload = m_wordsToDownload.size();
    m_downloaded = 0;
}

bool SvrAgt::handleResponseAppVersion(const QByteArray &msg)
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

bool SvrAgt::handleResponseUpgraderVersion(const QByteArray &msg)
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

void SvrAgt::sendRequestAppVersion(QString platform)
{
    MessageHeader msgHeader(ServerClientProtocol::RequestAppVersion);

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << msgHeader << platform;
    sendMessage(block);
}

void SvrAgt::sendRequestUpgraderVersion(QString platform)
{
    MessageHeader msgHeader(ServerClientProtocol::RequestUpgraderVersion);

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << msgHeader << platform;
    sendMessage(block);
}
