#include "clientwaiter.h"
#include "../HaiBeiDanCi/mysettings.h"

#include <QtNetwork>

ClientWaiter::ClientWaiter(qintptr socketDescriptor, QObject *parent)
    : QThread(parent), m_socketDescriptor(socketDescriptor)
{
}

void ClientWaiter::run()
{
    m_tcpSocket = new QTcpSocket(nullptr);

    // it's required to create the database connection
    // as we need to query database to like WordBook::getAllWords()
    WordDB::prepareDatabaseForThisThread();

    if (!m_tcpSocket->setSocketDescriptor(m_socketDescriptor)) {
        emit error(m_tcpSocket->error());
        return;
    }

    qDebug("%s:%d connected", m_tcpSocket->peerAddress().toString().toLatin1().constData(), m_tcpSocket->peerPort());

    int consecutiveHeartbeat = 0;
    while (1)
    {
        if (consecutiveHeartbeat > MySettings::maximumConsecutiveHeartbeat())
        {
            // no data/request from the client for a while
            break;
        }

        qDebug() << "consecutiveHeartbeat:" << consecutiveHeartbeat;

        // tries to read a message
        QByteArray msg = readMessage();
        if (msg.isEmpty() == true)
        {
            // can't get a message, we need more data
            if (waitForMoreData() == false)
            {
                consecutiveHeartbeat ++;
            }
            continue;
        }

        // we have a message here, process it
        MessageHeader receivedMsgHeader(msg);
        int handleResult = handleMessage(msg);
        if (handleResult == 0)
        {
            if (receivedMsgHeader.code() == ServerClientProtocol::RequestNoOperation)
            {
                consecutiveHeartbeat ++;
            }
            else
            {
                consecutiveHeartbeat = 0;
            }
            // successfully processed the message
            qDebug() << QDateTime::currentDateTime().toString() << "successfully handled message with header: ";
            qDebug("%s", receivedMsgHeader.toString().toUtf8().constData());
            continue;
        }
        else if (handleResult == 1)
        {
            qDebug() << QDateTime::currentDateTime().toString() << "failed to handle message with header: ";
            qDebug("%s", receivedMsgHeader.toString().toUtf8().constData());

            if (waitForMoreData() == false)
            {
                consecutiveHeartbeat ++;
            }
        }
        else if (handleResult == -1)
        {
            qDebug() << QDateTime::currentDateTime().toString() << "unknown message with header: ";
            qDebug("%s", receivedMsgHeader.toString().toUtf8().constData());

            // the message is discarded automatically, continue trying to get the next message
        }
        else if (handleResult == -2)
        {
            qDebug() << QDateTime::currentDateTime().toString() << "client said bye: ";
            qDebug("%s", receivedMsgHeader.toString().toUtf8().constData());
            break;
        }
        else
        {
            qCritical("unexpected result when handling message: %s", receivedMsgHeader.toString().toUtf8().constData());
            break;
        }
    }

    disconnectPeer();
}

/**
 * @brief ClientWaiter::waitForMoreData
 * @return
 * true if there's new data
 * false timeout, no new data
 */
bool ClientWaiter::waitForMoreData()
{
    if (m_tcpSocket == nullptr)
    {
        return false;
    }

    // wait for 30 seconds (the default) for simplicity
    if (m_tcpSocket->waitForReadyRead() == false)
    {
        return false;
    }
    else
    {
        // new message from client
        return true;
    }
}

void ClientWaiter::disconnectPeer()
{
    if (m_tcpSocket == nullptr)
    {
        return;
    }

    m_tcpSocket->disconnectFromHost();
    if (m_tcpSocket->state() != QAbstractSocket::UnconnectedState)
    {
        m_tcpSocket->waitForDisconnected();
    }
    qDebug() << "disconnected.";
}

void ClientWaiter::sendResponseUnknownRequest(const QByteArray &msg)
{
    MessageHeader receivedMsgHeader(msg);
    MessageHeader responseHeader(ServerClientProtocol::ResponseUnknownRequest, receivedMsgHeader.sequenceNumber());
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << responseHeader;
    sendMessage(block);
}

// should be changed later
void ClientWaiter::sendMessage(QByteArray msg, bool /*now*/)
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << msg;
    m_tcpSocket->write(block);
}

QByteArray ClientWaiter::readMessage()
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
        return QByteArray();
    }
}

sptr<MessageHeader> ClientWaiter::readMessageHeader()
{
    if (m_tcpSocket == nullptr)
    {
        return sptr<MessageHeader>();
    }

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
        return sptr<MessageHeader>();
    }
}

void ClientWaiter::handleUnknownMessage(const QByteArray &msg)
{
    sendResponseUnknownRequest(msg);
}

int ClientWaiter::handleMessage(const QByteArray &msg)
{
    MessageHeader receivedMsgHeader(msg);
    bool handleResult = false;
    bool unknowMessage = false;
    bool clientSaidBye = false;
    switch (receivedMsgHeader.code()) {
    case ServerClientProtocol::RequestNoOperation:
        handleResult = handleRequestNoOperation(msg);
        break;

    case ServerClientProtocol::RequestGetAllBooks:
        handleResult = handleRequestGetAllBooks(msg);
        break;

    case ServerClientProtocol::RequestGetAWord:
        handleResult = handleRequestGetAWord(msg);
        break;

    case ServerClientProtocol::RequestGetABook:
        handleResult = handleRequestGetABook(msg);
        break;

    case ServerClientProtocol::RequestGetBookWordList:
        handleResult = handleRequestGetBookWordList(msg);
        break;

    case ServerClientProtocol::RequestGetFile:
        handleResult = handleRequestGetFile(msg);
        break;

    case ServerClientProtocol::RequestGetWordsOfBookFinished:
        handleResult = handleRequestGetWordsOfBookFinished(msg);
        break;

    case ServerClientProtocol::RequestBye:
        clientSaidBye = true;
        break;

    default:
        handleUnknownMessage(msg);
        unknowMessage = true;
        break;

    }

    if (clientSaidBye == true)
    {
        return -2;
    }

    if (unknowMessage == true)
    {
        return -1;
    }

    if (handleResult == true)
    {
        return 0;

    }
    else
    {
        return 1;
    }

}

bool ClientWaiter::handleRequestNoOperation(const QByteArray &msg)
{
    qDebug() << "Heartbeat received from the client";
    sendResponseNoOperation(msg);
    return true;
}

bool ClientWaiter::handleRequestGetAllBooks(const QByteArray &msg)
{
    funcTracker ft("handleRequestGetAllBooks()");

    auto books = WordBook::getAllBooks();
    sendResponseGetAllBooks(msg, books);
    return true;
}

void ClientWaiter::sendResponseNoOperation(const QByteArray &msg)
{
    MessageHeader receivedMsgHeader(msg);
    MessageHeader responseHeader(ServerClientProtocol::ResponseNoOperation, receivedMsgHeader.sequenceNumber());
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << responseHeader;
    sendMessage(block);
}

void ClientWaiter::sendResponseGetAllBooks(const QByteArray &msg, const QList<QString> &books)
{
    MessageHeader receivedMsgHeader(msg);
    MessageHeader responseHeader(ServerClientProtocol::ResponseGetAllBooks, receivedMsgHeader.sequenceNumber());
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << responseHeader << books;
    sendMessage(block);
}

bool ClientWaiter::handleRequestGetAWord(const QByteArray &msg)
{
    funcTracker ft("handleRequestGetAWord()");

    // read the spelling of the word
    QDataStream in(msg);
    MessageHeader receivedMsgHeader(-1, -1, -1);
    QString spelling;
    in.startTransaction();
    in >> receivedMsgHeader >> spelling;
    if (in.commitTransaction() == false)
    {
        // in this case, the transaction is restored by commitTransaction()
        qDebug() << "failed to get book name in handleRequestGetABook()";
        return false;
    }

    auto word = Word::getWord(spelling);
    if (word.get() == nullptr)
    {
        return false;
    }

    sendResponseGetAWord(msg, *word);

    return true;
}

void ClientWaiter::sendResponseGetAWord(const QByteArray &msg, const Word &word)
{
    MessageHeader receivedMsgHeader(msg);
    MessageHeader responseHeader(ServerClientProtocol::ResponseGetAWord, receivedMsgHeader.sequenceNumber());

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << responseHeader << word;
    sendMessage(block);
}

void ClientWaiter::sendResponseBookWordListAllSent(const QByteArray &msg, const QString bookName)
{
    MessageHeader receivedMsgHeader(msg);
    MessageHeader responseHeader(ServerClientProtocol::ResponseBookWordListAllSent, receivedMsgHeader.sequenceNumber());

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << responseHeader << bookName;
    sendMessage(block);
}

void ClientWaiter::sendResponseGetWordsOfBookFinished(const QByteArray &msg, const QString bookName)
{
    MessageHeader receivedMsgHeader(msg);
    MessageHeader responseHeader(ServerClientProtocol::ResponseGetWordsOfBookFinished, receivedMsgHeader.sequenceNumber());

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << responseHeader << bookName;
    sendMessage(block);
}

void ClientWaiter::sendResponseGetFileFinished(const QByteArray &msg, const QString fileName, bool succeeded)
{
    MessageHeader receivedMsgHeader(msg);
    MessageHeader responseHeader(ServerClientProtocol::ResponseGetFileFinished, receivedMsgHeader.sequenceNumber());

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << responseHeader << fileName << succeeded;
    sendMessage(block);
}

bool ClientWaiter::handleRequestGetABook(const QByteArray &msg)
{
    funcTracker ft("handleRequestGetABook()");

    // read the name of the book
    MessageHeader receivedMsgHeader(-1, -1, -1);
    QDataStream in(msg);
    QString bookName;
    in.startTransaction();
    in >> receivedMsgHeader >> bookName;
    if (in.commitTransaction() == false)
    {
        // in this case, the transaction is restored by commitTransaction()
        qDebug() << "failed to get book name in handleRequestGetABook()";
        return false;
    }

    auto book = WordBook::getBook(bookName);
    if (book.get() == nullptr)
    {
        return false;
    }

    sendResponseGetABook(msg, *book);

    return true;
}

bool ClientWaiter::handleRequestGetFile(const QByteArray &msg)
{
    funcTracker ft("handleRequestGetFile()");

    // read the name of the book
    QDataStream in(msg);
    MessageHeader receivedMsgHeader(-1, -1, -1);
    QString fileName;
    in.startTransaction();
    in >> receivedMsgHeader >> fileName;
    if (in.commitTransaction() == false)
    {
        // in this case, the transaction is restored by commitTransaction()
        qDebug() << "failed to get file name in handleRequestGetFile()";
        return false;
    }

    bool succeeded = sendFile(msg, fileName);
    sendResponseGetFileFinished(msg, fileName, succeeded);

    // the message has been processed, so return true regardless if succeeded or not
    return true;
}

void ClientWaiter::sendResponseGetABook(const QByteArray &msg, const WordBook &book)
{
    MessageHeader receivedMsgHeader(msg);
    MessageHeader responseHeader(ServerClientProtocol::ResponseGetABook, receivedMsgHeader.sequenceNumber());

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << responseHeader << book;
    sendMessage(block);
}

bool ClientWaiter::handleRequestGetBookWordList(const QByteArray &msg)
{
    funcTracker ft("handleRequestGetWordsOfBook()");

    // read the name of the book
    QDataStream in(msg);
    MessageHeader receivedMsgHeader(-1, -1, -1);
    QString bookName;
    in.startTransaction();
    in >> receivedMsgHeader >> bookName;
    if (in.commitTransaction() == false)
    {
        // in this case, the transaction is restored by commitTransaction()
        qDebug() << "failed to get book name in handleRequestGetWordsOfBook()";
        return false;
    }

    sendBookWordList(msg, bookName);

    return true;
}

bool ClientWaiter::handleRequestGetWordsOfBookFinished(const QByteArray &msg)
{
    funcTracker ft("handleRequestGetWordsOfBook()");

    // read the name of the book
    QDataStream in(msg);
    MessageHeader receivedMsgHeader(-1, -1, -1);
    QString bookName;
    in.startTransaction();
    in >> receivedMsgHeader >> bookName;
    if (in.commitTransaction() == false)
    {
        // in this case, the transaction is restored by commitTransaction()
        qDebug() << "failed to get book name in handleRequestGetWordsOfBook()";
        return false;
    }

    sendResponseGetWordsOfBookFinished(msg, bookName);

    return true;
}

void ClientWaiter::sendBookWordList(const QByteArray &msg, const QString bookName)
{
    auto book = WordBook::getBook(bookName);
    if (book == nullptr)
    {
        return;
    }

    QVector<QString> wordList = book->getAllWords();
    int total = wordList.size();
    int pos = 0;
    int counter = 0;
    while (pos < total)
    {
        counter ++;
        const QString partName = ServerClientProtocol::partPrefix(counter) + bookName;
        QVector<QString> subList = wordList.mid(pos, ServerClientProtocol::MaximumWordsInAMessage);
        sendResponseGetBookWordList(msg, partName, subList);
        pos += subList.size();
    }

    sendResponseBookWordListAllSent(msg, bookName);
}

bool ClientWaiter::okToSendFile(const QString fileName)
{
    // only allow files in folder media
    if (fileName.startsWith("media", Qt::CaseInsensitive) == false)
    {
        return false;
    }

    // only allow mp3/png/jpg/css/js
    QString ext = fileName.section('.', -1);
    if (ext.compare("mp3", Qt::CaseInsensitive) != 0
            && ext.compare("png", Qt::CaseInsensitive) != 0
            && ext.compare("jpg", Qt::CaseInsensitive) != 0
            && ext.compare("css", Qt::CaseInsensitive) != 0
            && ext.compare("js", Qt::CaseInsensitive) != 0)
    {
        return false;
    }

    // don't allow to move upper level folder
    if (fileName.contains("..") == true)
    {
        return false;
    }

    return true;
}

bool ClientWaiter::sendFile(const QByteArray &msg, const QString fileName)
{
    // check if the file is OK to send, we cannot expose everything on the sever!!!
    if (okToSendFile(fileName) != true)
    {
        qDebug() << "cannot send file" << fileName << "because it violates the security policy!";
        return false;
    }

    QString localFile = MySettings::dataDirectory() + "/" + fileName;
    qDebug() << "send file" << localFile;

    QFile toSend(localFile);
    if (toSend.open(QIODevice::ReadOnly | QIODevice::ExistingOnly) == false)
    {
        qDebug() << "cannot open file" << fileName << "because" << toSend.errorString();
        return false;
    }

    const int fileSize = static_cast<int>(toSend.size());
    int sentBytes = 0;
    int counter = 0;
    bool succeeded = true;
    QDataStream fileDS(&toSend);
    char *buf = new char[ServerClientProtocol::MaximumBytesForFileTransfer + 1];
    while (sentBytes < fileSize)
    {
        auto readBytes = fileDS.readRawData(buf, ServerClientProtocol::MaximumBytesForFileTransfer);
        if (readBytes == -1)
        {
            succeeded = false;
            break;
        }

        counter ++;
        const QString partName = ServerClientProtocol::partPrefix(counter) + fileName;
        sendResponseGetFile(msg, partName, buf, static_cast<uint>(readBytes));
        sentBytes += readBytes;
        qDebug() << "send" << readBytes << "bytes of total" << fileSize;
    }

    delete[] buf;

    return succeeded;
}

void ClientWaiter::sendResponseGetFile(const QByteArray &msg, const QString fileName, const char *s, uint len)
{
    MessageHeader receivedMsgHeader(msg);
    MessageHeader responseHeader(ServerClientProtocol::ResponseGetFile, receivedMsgHeader.sequenceNumber());

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << responseHeader << fileName;
    out.writeBytes(s, len);
    sendMessage(block);
}

void ClientWaiter::sendResponseGetBookWordList(const QByteArray &msg, const QString bookName, const QVector<QString> &wordList)
{
    if (wordList.size() > ServerClientProtocol::MaximumWordsInAMessage)
    {
        return;
    }

    MessageHeader receivedMsgHeader(msg);
    MessageHeader responseHeader(ServerClientProtocol::ResponseGetBookWordList, receivedMsgHeader.sequenceNumber());

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << responseHeader << bookName << wordList;
    sendMessage(block);
}
