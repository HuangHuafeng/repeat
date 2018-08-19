#include "serveragent.h"
#include "serverclientprotocol.h"

ServerAgent::ServerAgent(QObject *parent) : QObject(parent), m_tcpSocket(this)
{
    connect(&m_tcpSocket, SIGNAL(connected()), this, SLOT(onConnected()));
    connect(&m_tcpSocket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    connect(&m_tcpSocket, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
    connect(&m_tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)));
    connect(&m_tcpSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(onStateChanged(QAbstractSocket::SocketState)));

    m_tcpSocket.connectToHost("huafengsmac", 61027);

    // it seems waitForConnected() helps to get/trigger the error ConnectionRefusedError
    if (m_tcpSocket.waitForConnected() == false)
    {
        qDebug() << "waitForConnected() failed" << m_tcpSocket.error();
    }
}


void ServerAgent::onReadyRead()
{
    int messageCode = readMessageCode();
    if (messageCode != 0)
    {
        if (handleMessage(messageCode) == true)
        {
            qDebug() << "successfully handled message with code" << messageCode;
        }
        else
        {
            qDebug() << "failed to handle message with code" << messageCode;
        }
    }
}

void ServerAgent::onConnected()
{
    qDebug() << "onConnected()";
}

void ServerAgent::onDisconnected()
{
    qDebug() << "onDisconnected()";
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
        qDebug() << "failed to read message code in readMessageCode()";
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

    default:
        qDebug() << "got unknown message with code" << messageCode << "in handleMessage()";
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
    /*
    qDebug() << "got words of book" << bookName;
    for (int i = 0;i < wordList.size();i ++)
    {
        qDebug() << wordList.at(i);
    }
    */

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
