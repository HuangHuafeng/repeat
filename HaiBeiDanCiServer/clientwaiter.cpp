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
    if (m_tcpSocket == nullptr)
    {
        return;
    }

    // it's required to create the database connection
    // as we need to query database to like WordBook::getAllWords()
    WordDB::prepareDatabaseForThisThread();

    qDebug() << "run() start";
    if (!m_tcpSocket->setSocketDescriptor(m_socketDescriptor)) {
        emit error(m_tcpSocket->error());
        return;
    }

    qDebug("%s:%d connected", m_tcpSocket->peerAddress().toString().toLatin1().constData(), m_tcpSocket->peerPort());

    int currentMessage = 0;
    while (1)
    {
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
                qDebug() << "successfully handled message with code" << currentMessage << endl;
                currentMessage = 0;
                continue;
            }
            else if (handleResult == 1)
            {
                qDebug() << "failed to handle message with code" << currentMessage;

                // failed to process the message, probably means the content of the message is NOT fully available
                // so quit the loop and contine in next call of onReadyRead()
                if (m_tcpSocket->waitForReadyRead() == false)
                {
                    // time out, then we stop waiting for message from the client
                    break;
                }
                else
                {
                    // new message from client, continue to handle the message
                    continue;
                }
            }
            else
            {
                // handleResult == -1, unknown message!
                // discard the message and continue trying to get the next message
                sendResponseUnknownRequest(currentMessage);
                currentMessage = 0;
                continue;
            }
        }
        else
        {
            // if we cannot read a msssage code, it means there's no data
            // so we wait for 30 seconds by default, we can add a setting later
            if (m_tcpSocket->waitForReadyRead() == false)
            {
                // time out, then we stop waiting for message from the client
                break;
            }
            else
            {
                // new message from client, continue to handle the message
                continue;
            }
        }
    }

    disconnectPeer();
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

void ClientWaiter::sendResponseUnknownRequest(int messageCode)
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    int responseCode = ServerClientProtocol::ResponseUnknownRequest;
    out << responseCode << messageCode;
    m_tcpSocket->write(block);
}

int ClientWaiter::readMessageCode()
{
    if (m_tcpSocket == nullptr)
    {
        return 0;
    }

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

int ClientWaiter::handleMessage(int messageCode)
{
    if (m_tcpSocket == nullptr)
    {
        return -1;
    }

    bool handleResult = false;
    bool unknowMessage = false;
    switch (messageCode) {
    case ServerClientProtocol::RequestBye:
        handleResult = true;
        break;

    case ServerClientProtocol::RequestNoOperation:
        handleResult = handleRequestNoOperation();
        break;

    case ServerClientProtocol::RequestGetAllBooks:
        handleResult = handleRequestGetAllBooks();
        break;

    case ServerClientProtocol::RequestGetWordsOfBook:
        handleResult = handleRequestGetWordsOfBook();
        break;

    case ServerClientProtocol::RequestGetAWord:
        handleResult = handleRequestGetAWord();
        break;

    case ServerClientProtocol::RequestGetWords:
        handleResult = handleRequestGetWords();
        break;

    case ServerClientProtocol::RequestGetABook:
        handleResult = handleRequestGetABook();
        break;

    case ServerClientProtocol::RequestGetFile:
        handleResult = handleRequestGetFile();
        break;

    default:
        qDebug() << "got unknown message with code" << messageCode << "in handleMessage()";
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

bool ClientWaiter::handleRequestNoOperation()
{
    qDebug() << "Heartbeat received from the client";
    sendResponseNoOperation();
    return true;
}

bool ClientWaiter::handleRequestGetAllBooks()
{
    funcTracker ft("handleRequestGetAllBooks()");

    auto books = WordBook::getAllBooks();
    sendResponseGetAllBooks(books);
    return true;
}

void ClientWaiter::sendResponseNoOperation()
{
    int responseCode = ServerClientProtocol::ResponseNoOperation;

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << responseCode;
    m_tcpSocket->write(block);
}

void ClientWaiter::sendResponseGetAllBooks(const QList<QString> &books)
{
    int responseCode = ServerClientProtocol::ResponseGetAllBooks;

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << responseCode << books;
    m_tcpSocket->write(block);
}

bool ClientWaiter::handleRequestGetWords()
{
    funcTracker ft("handleRequestGetWords()");

    // read the spelling of the word
    QDataStream in(m_tcpSocket);
    QString bookName;
    QVector<QString> wordList;
    in.startTransaction();
    in >> bookName >> wordList;
    if (in.commitTransaction() == false)
    {
        // in this case, the transaction is restored by commitTransaction()
        qDebug() << "failed to get book name in handleRequestGetWords()";
        return false;
    }

    for (int i = 0;i < wordList.size();i ++)
    {
        auto word = Word::getWord(wordList.at(i));
        if (word.get() != nullptr)
        {
            sendResponseGetAWord(*word);
        }
    }

    sendResponseAllDataSentForRequestGetWords(bookName);

    return true;
}

bool ClientWaiter::handleRequestGetAWord()
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

    sendResponseGetAWord(*word);

    return true;
}

void ClientWaiter::sendResponseGetAWord(const Word &word)
{
    int responseCode = ServerClientProtocol::ResponseGetAWord;

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << responseCode << word;
    m_tcpSocket->write(block);
}

void ClientWaiter::sendResponseAllDataSent(int messageCode)
{
    int responseCode = ServerClientProtocol::ResponseAllDataSent;

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << responseCode << messageCode;
    m_tcpSocket->write(block);
}

void ClientWaiter::sendResponseAllDataSentForRequestGetWordsOfBook(const QString bookName)
{
    int responseCode = ServerClientProtocol::ResponseAllDataSent;
    int messageCode = ServerClientProtocol::RequestGetWordsOfBook;

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << responseCode << messageCode << bookName;
    m_tcpSocket->write(block);
}

void ClientWaiter::sendResponseAllDataSentForRequestGetFile(const QString fileName, bool succeeded)
{
    int responseCode = ServerClientProtocol::ResponseAllDataSent;
    int messageCode = ServerClientProtocol::RequestGetFile;

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << responseCode << messageCode << fileName << succeeded;
    m_tcpSocket->write(block);
}

void ClientWaiter::sendResponseAllDataSentForRequestGetWords(const QString bookName)
{
    int responseCode = ServerClientProtocol::ResponseAllDataSent;
    int messageCode = ServerClientProtocol::RequestGetWords;

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << responseCode << messageCode << bookName;
    m_tcpSocket->write(block);
}

bool ClientWaiter::handleRequestGetABook()
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

    sendResponseGetABook(*book);

    return true;
}

bool ClientWaiter::handleRequestGetFile()
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

    bool succeeded = sendFile(fileName);
    sendResponseAllDataSentForRequestGetFile(fileName, succeeded);

    // the message has been processed, so return true regardless if succeeded or not
    return true;
}

void ClientWaiter::sendResponseGetABook(const WordBook &book)
{
    int responseCode = ServerClientProtocol::ResponseGetABook;

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << responseCode << book;
    m_tcpSocket->write(block);
}

bool ClientWaiter::handleRequestGetWordsOfBook()
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

    sendWordsOfBook(bookName);

    return true;
}

void ClientWaiter::sendWordsOfBook(const QString bookName)
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
    while (pos + ServerClientProtocol::MaximumWordsInAMessage < total)
    {
        counter ++;
        const QString partName = ServerClientProtocol::partPrefix(counter) + bookName;
        QVector<QString> subList = wordList.mid(pos, ServerClientProtocol::MaximumWordsInAMessage);
        sendResponseGetWordsOfBook(partName, subList);
        pos += ServerClientProtocol::MaximumWordsInAMessage;
    }

    QVector<QString> leftWords = wordList.mid(pos, ServerClientProtocol::MaximumWordsInAMessage);
    sendResponseGetWordsOfBook(bookName, leftWords);

    sendResponseAllDataSentForRequestGetWordsOfBook(bookName);
}

bool ClientWaiter::sendFile(const QString fileName)
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
        //char buf[] = {0};
        auto readBytes = fileDS.readRawData(buf, ServerClientProtocol::MaximumBytesForFileTransfer);
        if (readBytes == -1)
        {
            succeeded = false;
            break;
        }

        counter ++;
        const QString partName = ServerClientProtocol::partPrefix(counter) + fileName;
        sendResponseGetFile(partName, buf, static_cast<uint>(readBytes));
        sentBytes += readBytes;
        qDebug() << "send" << readBytes << "bytes of total" << fileSize;
    }

    delete[] buf;

    return succeeded;
}

void ClientWaiter::sendResponseGetFile(const QString fileName, const char *s, uint len)
{
    int responseCode = ServerClientProtocol::ResponseGetFile;

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << responseCode << fileName;
    out.writeBytes(s, len);
    m_tcpSocket->write(block);
}

void ClientWaiter::sendResponseGetWordsOfBook(const QString bookName, const QVector<QString> &wordList)
{
    if (wordList.size() > ServerClientProtocol::MaximumWordsInAMessage)
    {
        return;
    }

    int responseCode = ServerClientProtocol::ResponseGetWordsOfBook;

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << responseCode << bookName << wordList;
    m_tcpSocket->write(block);
}
