#include "clientwaiter.h"
#include "../HaiBeiDanCi/word.h"
#include "../HaiBeiDanCi/worddb.h"
#include "../HaiBeiDanCi/serverclientprotocol.h"
#include "../HaiBeiDanCi/wordbook.h"

#include <QtNetwork>

ClientWaiter::ClientWaiter(qintptr socketDescriptor, QObject *parent)
    : QThread(parent), m_socketDescriptor(socketDescriptor)
{
}

void ClientWaiter::run()
{
    m_tcpSocket = new QTcpSocket(nullptr);
    if (m_tcpSocket.get() == nullptr)
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
        if (m_tcpSocket->waitForReadyRead() == false)
        {
            break;
        }

        int messageCode = readMessageCode();
        if (messageCode == 0)
        {
            continue;
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
    if (m_tcpSocket.get() == nullptr)
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
    if (m_tcpSocket.get() == nullptr)
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
    if (m_tcpSocket.get() == nullptr)
    {
        return 0;
    }

    int messageCode;
    QDataStream in(m_tcpSocket.get());
    in.startTransaction();
    in >> messageCode;
    if (in.commitTransaction() == true)
    {
        return messageCode;
    }
    else
    {
        // in this case, the transaction is restored by commitTransaction()
        qDebug() << "failed to read message code in readMessageCode()";
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

    default:
        qDebug() << "got unknown message with code" << messageCode << "in handleMessage()";
        break;

    }

    return handleResult;
}

bool ClientWaiter::handleRequestGetAllBooks()
{
    if (m_tcpSocket.get() == nullptr)
    {
        return false;
    }

    auto books = WordBook::getAllBooks();
    int responseCode = ServerClientProtocol::ResponseGetAllBooks;

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << responseCode << books;
    m_tcpSocket->write(block);

    return true;
}

bool ClientWaiter::handleRequestGetWordsOfBook()
{
    if (m_tcpSocket.get() == nullptr)
    {
        return false;
    }

    // read the name of the book
    QDataStream in(m_tcpSocket.get());
    QString bookName;
    in.startTransaction();
    in >> bookName;
    if (in.commitTransaction() == false)
    {
        // in this case, the transaction is restored by commitTransaction()
        qDebug() << "failed to get book name in handleRequestGetWordsOfBook()";
        return false;
    }

    auto book = WordBook::getBook(bookName);
    if (book.get() == nullptr)
    {
        return false;
    }

    QVector<QString> wordList = book->getAllWords();
    int responseCode = ServerClientProtocol::ResponseGetWordsOfBook;

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << responseCode << bookName << wordList;
    m_tcpSocket->write(block);

    return true;
}
