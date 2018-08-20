#include "clientwaiter.h"

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

    WordDB::prepareDatabaseForThisThread();

    qDebug() << "run() start";
    if (!m_tcpSocket->setSocketDescriptor(m_socketDescriptor)) {
        emit error(m_tcpSocket->error());
        return;
    }

    qDebug("%s:%d connected", m_tcpSocket->peerAddress().toString().toLatin1().constData(), m_tcpSocket->peerPort());

    auto counter = 0;
    while (1)
    {
        int messageCode = readMessageCode();
        if (messageCode == 0)
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

        if (handleMessage(messageCode) == true)
        {
            qDebug() << "successfully handled message with code" << messageCode;
            continue;
        }
        else
        {
            qDebug() << "failed to handle message with code" << messageCode;
            failedToHandleMessage(messageCode);

            counter ++;
            if (counter > 10)
            {
                qDebug() << "failed times more than 10, disconnect!";
                break;
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

void ClientWaiter::failedToHandleMessage(int messageCode)
{
    if (m_tcpSocket == nullptr)
    {
        return;
    }

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    int responseCode = ServerClientProtocol::ResponseFailedToRequest;
    out << responseCode << messageCode;
    m_tcpSocket->write(block);
}

int ClientWaiter::readMessageCode()
{
    funcTracker ft("readMessageCode()");

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
        qInfo("failed to read message code in readMessageCode(), probably no data from peer");
        return 0;
    }
}

bool ClientWaiter::handleMessage(int messageCode)
{
    bool handleResult = false;
    switch (messageCode) {
    case ServerClientProtocol::RequestBye:
    case ServerClientProtocol::RequestNoOperation:
        handleResult = true;
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

    case ServerClientProtocol::RequestGetABook:
        handleResult = handleRequestGetABook();
        break;

    default:
        qDebug() << "got unknown message with code" << messageCode << "in handleMessage()";
        break;

    }

    return handleResult;
}

bool ClientWaiter::handleRequestGetAllBooks()
{
    funcTracker ft("handleRequestGetAllBooks()");

    if (m_tcpSocket == nullptr)
    {
        return false;
    }

    auto books = WordBook::getAllBooks();
    sendResponseGetAllBooks(books);
    return true;
}

void ClientWaiter::sendResponseGetAllBooks(const QList<QString> &books)
{
    int responseCode = ServerClientProtocol::ResponseGetAllBooks;

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << responseCode << books;
    m_tcpSocket->write(block);
}

bool ClientWaiter::handleRequestGetAWord()
{
    funcTracker ft("handleRequestGetAWord()");

    if (m_tcpSocket == nullptr)
    {
        return false;
    }

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

bool ClientWaiter::handleRequestGetABook()
{
    funcTracker ft("handleRequestGetABook()");

    if (m_tcpSocket == nullptr)
    {
        return false;
    }

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

    if (m_tcpSocket == nullptr)
    {
        return false;
    }

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
    int pos = 0;
    do
    {
        QVector<QString> subList = wordList.mid(pos, MaximumWordsInAMessage);
        sendResponseGetWordsOfBook(bookName, subList);
        pos += MaximumWordsInAMessage;
    } while (pos < wordList.size());
}

void ClientWaiter::sendResponseGetWordsOfBook(const QString bookName, const QVector<QString> &wordList)
{
    if (wordList.size() > MaximumWordsInAMessage)
    {
        return;
    }

    int responseCode = ServerClientProtocol::ResponseGetWordsOfBook;

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << responseCode << bookName << wordList;
    m_tcpSocket->write(block);
}
