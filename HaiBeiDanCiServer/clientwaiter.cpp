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
    static sptr<MessageHeader> currentMessage;
    static uint readMessageHeaderConsecutiveFailure = 0;
    while (1)
    {
        if (consecutiveHeartbeat > MySettings::maximumConsecutiveHeartbeat())
        {
            // no data/request from the client for a while
            break;
        }

        qDebug() << "consecutiveHeartbeat:" << consecutiveHeartbeat;

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
                if (currentMessage->code() == ServerClientProtocol::RequestNoOperation)
                {
                    consecutiveHeartbeat ++;
                }
                else
                {
                    consecutiveHeartbeat = 0;
                }
                // successfully processed the message
                qDebug() << QDateTime::currentDateTime().toString() << "successfully handled message:";
                qDebug("%s", currentMessage->toString().toUtf8().constData());
                // successfully processed the message
                currentMessage = sptr<MessageHeader>();
                continue;
            }
            else if (handleResult == 1)
            {
                qDebug() << QDateTime::currentDateTime().toString() << "failed to handle message:";
                qDebug("%s", currentMessage->toString().toUtf8().constData());

                if (waitForMoreData() == false)
                {
                    consecutiveHeartbeat ++;
                }
            }
            else if (handleResult == -1)
            {
                qDebug() << QDateTime::currentDateTime().toString() << "unknown message:";
                qDebug("%s", currentMessage->toString().toUtf8().constData());

                // discard the message and continue trying to get the next message
                currentMessage = sptr<MessageHeader>();
            }
            else if (handleResult == -2)
            {
                // client said bye
                break;
            }
            else
            {
                qCritical("unexpected result when handling message: %s", currentMessage->toString().toUtf8().constData());
                break;
            }
        }
        else
        {
            if (waitForMoreData() == false)
            {
                consecutiveHeartbeat ++;
            }
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

void ClientWaiter::sendResponseUnknownRequest(const MessageHeader &msgHeader)
{
    MessageHeader responseHeader(ServerClientProtocol::ResponseUnknownRequest, msgHeader.sequenceNumber());
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << responseHeader;
    m_tcpSocket->write(block);
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

void ClientWaiter::handleUnknownMessage(const MessageHeader &msgHeader)
{
    sendResponseUnknownRequest(msgHeader);
}

int ClientWaiter::handleMessage(const MessageHeader &msgHeader)
{
    if (m_tcpSocket == nullptr)
    {
        return -1;
    }

    bool handleResult = false;
    bool unknowMessage = false;
    bool clientSaidBye = false;
    switch (msgHeader.code()) {
    case ServerClientProtocol::RequestNoOperation:
        handleResult = handleRequestNoOperation(msgHeader);
        break;

    case ServerClientProtocol::RequestGetAllBooks:
        handleResult = handleRequestGetAllBooks(msgHeader);
        break;

    case ServerClientProtocol::RequestGetAWord:
        handleResult = handleRequestGetAWord(msgHeader);
        break;

    case ServerClientProtocol::RequestGetABook:
        handleResult = handleRequestGetABook(msgHeader);
        break;

    case ServerClientProtocol::RequestGetBookWordList:
        handleResult = handleRequestGetBookWordList(msgHeader);
        break;

    case ServerClientProtocol::RequestGetFile:
        handleResult = handleRequestGetFile(msgHeader);
        break;

    case ServerClientProtocol::RequestGetWordsOfBookFinished:
        handleResult = handleRequestGetWordsOfBookFinished(msgHeader);
        break;

    case ServerClientProtocol::RequestBye:
        clientSaidBye = true;
        break;

    default:
        handleUnknownMessage(msgHeader);
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

bool ClientWaiter::handleRequestNoOperation(const MessageHeader &msgHeader)
{
    qDebug() << "Heartbeat received from the client";
    sendResponseNoOperation(msgHeader);
    return true;
}

bool ClientWaiter::handleRequestGetAllBooks(const MessageHeader &msgHeader)
{
    funcTracker ft("handleRequestGetAllBooks()");

    auto books = WordBook::getAllBooks();
    sendResponseGetAllBooks(msgHeader, books);
    return true;
}

void ClientWaiter::sendResponseNoOperation(const MessageHeader &msgHeader)
{
    MessageHeader responseHeader(ServerClientProtocol::ResponseNoOperation, msgHeader.sequenceNumber());
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << responseHeader;
    m_tcpSocket->write(block);
}

void ClientWaiter::sendResponseGetAllBooks(const MessageHeader &msgHeader, const QList<QString> &books)
{
    MessageHeader responseHeader(ServerClientProtocol::ResponseGetAllBooks, msgHeader.sequenceNumber());
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << responseHeader << books;
    m_tcpSocket->write(block);
}

bool ClientWaiter::handleRequestGetAWord(const MessageHeader &msgHeader)
{
    funcTracker ft("handleRequestGetAWord()");

    // read the spelling of the word
    QDataStream in(m_tcpSocket);
    QString spelling;
    in.startTransaction();
    in >> spelling;
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

    sendResponseGetAWord(msgHeader, *word);

    return true;
}

void ClientWaiter::sendResponseGetAWord(const MessageHeader &msgHeader, const Word &word)
{
    MessageHeader responseHeader(ServerClientProtocol::ResponseGetAWord, msgHeader.sequenceNumber());

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << responseHeader << word;
    m_tcpSocket->write(block);
}

void ClientWaiter::sendResponseBookWordListAllSent(const MessageHeader &msgHeader, const QString bookName)
{
    MessageHeader responseHeader(ServerClientProtocol::ResponseBookWordListAllSent, msgHeader.sequenceNumber());

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << responseHeader << bookName;
    m_tcpSocket->write(block);
}

void ClientWaiter::sendResponseGetWordsOfBookFinished(const MessageHeader &msgHeader, const QString bookName)
{
    MessageHeader responseHeader(ServerClientProtocol::ResponseGetWordsOfBookFinished, msgHeader.sequenceNumber());

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << responseHeader << bookName;
    m_tcpSocket->write(block);
}

void ClientWaiter::sendResponseGetFileFinished(const MessageHeader &msgHeader, const QString fileName, bool succeeded)
{
    MessageHeader responseHeader(ServerClientProtocol::ResponseGetFileFinished, msgHeader.sequenceNumber());

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << responseHeader << fileName << succeeded;
    m_tcpSocket->write(block);
}

bool ClientWaiter::handleRequestGetABook(const MessageHeader &msgHeader)
{
    funcTracker ft("handleRequestGetABook()");

    // read the name of the book
    QDataStream in(m_tcpSocket);
    QString bookName;
    in.startTransaction();
    in >> bookName;
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

    sendResponseGetABook(msgHeader, *book);

    return true;
}

bool ClientWaiter::handleRequestGetFile(const MessageHeader &msgHeader)
{
    funcTracker ft("handleRequestGetFile()");

    // read the name of the book
    QDataStream in(m_tcpSocket);
    QString fileName;
    in.startTransaction();
    in >> fileName;
    if (in.commitTransaction() == false)
    {
        // in this case, the transaction is restored by commitTransaction()
        qDebug() << "failed to get file name in handleRequestGetFile()";
        return false;
    }

    bool succeeded = sendFile(msgHeader, fileName);
    sendResponseGetFileFinished(msgHeader, fileName, succeeded);

    // the message has been processed, so return true regardless if succeeded or not
    return true;
}

void ClientWaiter::sendResponseGetABook(const MessageHeader &msgHeader, const WordBook &book)
{
    MessageHeader responseHeader(ServerClientProtocol::ResponseGetABook, msgHeader.sequenceNumber());

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << responseHeader << book;
    m_tcpSocket->write(block);
}

bool ClientWaiter::handleRequestGetBookWordList(const MessageHeader &msgHeader)
{
    funcTracker ft("handleRequestGetWordsOfBook()");

    // read the name of the book
    QDataStream in(m_tcpSocket);
    QString bookName;
    in.startTransaction();
    in >> bookName;
    if (in.commitTransaction() == false)
    {
        // in this case, the transaction is restored by commitTransaction()
        qDebug() << "failed to get book name in handleRequestGetWordsOfBook()";
        return false;
    }

    sendBookWordList(msgHeader, bookName);

    return true;
}

bool ClientWaiter::handleRequestGetWordsOfBookFinished(const MessageHeader &msgHeader)
{
    funcTracker ft("handleRequestGetWordsOfBook()");

    // read the name of the book
    QDataStream in(m_tcpSocket);
    QString bookName;
    in.startTransaction();
    in >> bookName;
    if (in.commitTransaction() == false)
    {
        // in this case, the transaction is restored by commitTransaction()
        qDebug() << "failed to get book name in handleRequestGetWordsOfBook()";
        return false;
    }

    sendResponseGetWordsOfBookFinished(msgHeader, bookName);

    return true;
}

void ClientWaiter::sendBookWordList(const MessageHeader &msgHeader, const QString bookName)
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
        sendResponseGetBookWordList(msgHeader, partName, subList);
        pos += subList.size();
    }

    sendResponseBookWordListAllSent(msgHeader, bookName);
}

bool ClientWaiter::sendFile(const MessageHeader &msgHeader, const QString fileName)
{
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
        sendResponseGetFile(msgHeader, partName, buf, static_cast<uint>(readBytes));
        sentBytes += readBytes;
        qDebug() << "send" << readBytes << "bytes of total" << fileSize;
    }

    delete[] buf;

    return succeeded;
}

void ClientWaiter::sendResponseGetFile(const MessageHeader &msgHeader, const QString fileName, const char *s, uint len)
{
    MessageHeader responseHeader(ServerClientProtocol::ResponseGetFile, msgHeader.sequenceNumber());

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << responseHeader << fileName;
    out.writeBytes(s, len);
    m_tcpSocket->write(block);
}

void ClientWaiter::sendResponseGetBookWordList(const MessageHeader &msgHeader, const QString bookName, const QVector<QString> &wordList)
{
    if (wordList.size() > ServerClientProtocol::MaximumWordsInAMessage)
    {
        return;
    }

    MessageHeader responseHeader(ServerClientProtocol::ResponseGetBookWordList, msgHeader.sequenceNumber());

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << responseHeader << bookName << wordList;
    m_tcpSocket->write(block);
}
