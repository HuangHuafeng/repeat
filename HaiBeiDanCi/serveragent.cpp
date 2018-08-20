#include "serveragent.h"
#include "serverclientprotocol.h"

ServerAgent::ServerAgent(QObject *parent) : QObject(parent), m_tcpSocket(nullptr)
{
    connect(&m_tcpSocket, SIGNAL(connected()), this, SLOT(onConnected()));
    connect(&m_tcpSocket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    connect(&m_tcpSocket, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
    connect(&m_tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)));
    connect(&m_tcpSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(onStateChanged(QAbstractSocket::SocketState)));
}

ServerAgent::~ServerAgent()
{
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
            if (handleMessage(currentMessage) == true)
            {
                // successfully processed the message
                qDebug() << "successfully handled message with code" << currentMessage;
                currentMessage = 0;
            }
            else
            {
                qDebug() << "failed to handle message with code" << currentMessage;

                // failed to process the message, probably means the content of the message is NOT fully available
                // so quit the loop and contine in next call of onReadyRead()
                break;
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
}

void ServerAgent::onDisconnected()
{
    qDebug() << "onDisconnected()";
    emit(disconnected());
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
    QDataStream in(&m_tcpSocket);
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

bool ServerAgent::handleMessage(int messageCode)
{
    bool handleResult = false;
    switch (messageCode) {
    case ServerClientProtocol::ResponseFailedToRequest:
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

    default:
        //aaaqDebug() << "got unknown message with code" << messageCode << "in handleMessage()";
        handleResult = false;
        break;

    }

    return handleResult;
}

bool ServerAgent::handleResponseGetAllBooks()
{
    QDataStream in(&m_tcpSocket);
    QList<QString> books;
    in.startTransaction();
    in >> books;
    if (in.commitTransaction() == false)
    {
        // in this case, the transaction is restored by commitTransaction()
        qDebug() << "failed to read books in handleResponseGetAllBooks()";
        return false;
    }

    //qDebug() << books;
    emit(responseGetAllBooks(books));

    return true;
}

bool ServerAgent::handleResponseGetWordsOfBook()
{
    QString bookName;
    QVector<QString> wordList;
    QDataStream in(&m_tcpSocket);
    in.startTransaction();
    in >> bookName >> wordList;
    if (in.commitTransaction() == false)
    {
        // in this case, the transaction is restored by commitTransaction()
        qDebug() << "failed to read words of the book in handleResponseGetWordsOfBook()";
        return false;
    }

    emit(responseGetWordsOfBook(bookName, wordList));

    return true;
}

bool ServerAgent::handleResponseGetAWord()
{
    Word word;
    QDataStream in(&m_tcpSocket);
    in.startTransaction();
    in >> word;
    if (in.commitTransaction() == false)
    {
        // in this case, the transaction is restored by commitTransaction()
        qDebug() << "failed to read words of the book in handleResponseGetAWord()";
        return false;
    }

    emit(responseGetAWord(word));

    qDebug() << word.getId() << word.getSpelling() << word.getDefinition();

    return true;
}

bool ServerAgent::handleResponseGetABook()
{
    WordBook book;
    QDataStream in(&m_tcpSocket);
    in.startTransaction();
    in >> book;
    if (in.commitTransaction() == false)
    {
        // in this case, the transaction is restored by commitTransaction()
        qDebug() << "failed to read words of the book in handleResponseGetWordsOfBook()";
        return false;
    }

    emit(responseGetABook(book));

    qDebug() << book.getId() << book.getName() << book.getIntroduction();

    return true;
}

bool ServerAgent::handleResponseUnknownRequest()
{
    QDataStream in(&m_tcpSocket);
    int requestCode;
    in.startTransaction();
    in >> requestCode;
    if (in.commitTransaction() == false)
    {
        // in this case, the transaction is restored by commitTransaction()
        qDebug() << "failed to read books in handleResponseGetAllBooks()";
        return false;
    }

    emit(responseUnknownRequest());
    qDebug() << "the server responed that failed to handle request with code" << requestCode;

    return true;
}

void ServerAgent::connectToServer(const QString &hostName, quint16 port)
{
    m_tcpSocket.connectToHost(hostName, port);

    // it seems waitForConnected() helps to get/trigger the error ConnectionRefusedError
    if (m_tcpSocket.waitForConnected() == false)
    {
        qDebug() << "waitForConnected() failed" << m_tcpSocket.error();
    }
}

void ServerAgent::sendRequestGetAllBooks()
{
    int messageCode = ServerClientProtocol::RequestGetAllBooks;

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << messageCode;
    m_tcpSocket.write(block);
}

void ServerAgent::sendRequestGetWordsOfBook(QString bookName)
{
    int messageCode = ServerClientProtocol::RequestGetWordsOfBook;

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << messageCode << bookName;
    m_tcpSocket.write(block);
}

void ServerAgent::sendRequestGetAWord(QString spelling)
{
    int messageCode = ServerClientProtocol::RequestGetAWord;

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << messageCode << spelling;
    m_tcpSocket.write(block);
}

void ServerAgent::sendRequestGetABook(QString bookName)
{
    int messageCode = ServerClientProtocol::RequestGetABook;

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << messageCode << bookName;
    m_tcpSocket.write(block);
}
