#include "clienthandler.h"

ClientHandler::ClientHandler(ClientWaiter &clientWaiter) : m_clientWaiter(clientWaiter)
{
}

ClientHandler::~ClientHandler()
{
}

void ClientHandler::sendMessage(QByteArray msg)
{
    m_clientWaiter.sendMessage(msg);
}

int ClientHandler::handleMessage(const QByteArray &msg)
{
    MessageHeader receivedMsgHeader(msg);
    bool handleResult = false;
    bool unknowMessage = false;
    bool clientSaidBye = false;
    switch (receivedMsgHeader.code()) {
    case ServerClientProtocol::RequestNoOperation:
        handleResult = handleRequestNoOperation(msg);
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

bool ClientHandler::handleRequestNoOperation(const QByteArray &msg)
{
    qDebug() << "Heartbeat received from the client";
    sendResponseNoOperation(msg);
    return true;
}

void ClientHandler::handleUnknownMessage(const QByteArray &msg)
{
    sendResponseUnknownRequest(msg);
}

void ClientHandler::sendResponseUnknownRequest(const QByteArray &msg)
{
    MessageHeader receivedMsgHeader(msg);
    MessageHeader responseHeader(ServerClientProtocol::ResponseUnknownRequest, receivedMsgHeader.sequenceNumber());
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << responseHeader;
    sendMessage(block);
}

void ClientHandler::sendResponseNoOperation(const QByteArray &msg)
{
    MessageHeader receivedMsgHeader(msg);
    MessageHeader responseHeader(ServerClientProtocol::ResponseNoOperation, receivedMsgHeader.sequenceNumber());
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << responseHeader;
    sendMessage(block);
}