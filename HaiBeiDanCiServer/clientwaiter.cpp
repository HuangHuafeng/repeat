#include "clientwaiter.h"
#include "../HaiBeiDanCi/mysettings.h"
#include "hbdcapphandler.h"
#include "hbdcmanagerhandler.h"

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

    sptr<ClientHandler> ptrClientHandler = new HBDCAppHandler(*this);
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
                if (m_tcpSocket->state() != QAbstractSocket::ConnectedState)
                {
                    break;
                }
            }
            continue;
        }

        // we have a message here, process it
        MessageHeader receivedMsgHeader(msg);
        int handleResult = ptrClientHandler->handleMessage(msg);
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
            continue;
        }
        else if (handleResult == -1)
        {
            // check if it's the message to change the client handler
            if (receivedMsgHeader.code() == ServerClientProtocol::RequestPromoteToManager)
            {
                ptrClientHandler = new HBDCManagerHandler(*this);
                sendResponsePromoteToManager(msg);
                qDebug() << QDateTime::currentDateTime().toString() << "successfully handled message with header: ";
                qDebug("%s", receivedMsgHeader.toString().toUtf8().constData());
                qDebug() << "promoted the handler to manager";
            }
            else
            {
                qDebug() << QDateTime::currentDateTime().toString() << "unknown message with header: ";
                qDebug("%s", receivedMsgHeader.toString().toUtf8().constData());
                ptrClientHandler->handleUnknownMessage(msg);

                // the message is discarded automatically, continue trying to get the next message
            }
        }
        else if (handleResult == -2)
        {
            qDebug() << QDateTime::currentDateTime().toString() << "client said bye: ";
            qDebug("%s", receivedMsgHeader.toString().toUtf8().constData());
            break;
        }
        else
        {
            qCritical("unexpected result when handling message with header: %s", receivedMsgHeader.toString().toUtf8().constData());
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

void ClientWaiter::sendResponsePromoteToManager(const QByteArray &msg)
{
    MessageHeader receivedMsgHeader(msg);
    MessageHeader responseHeader(ServerClientProtocol::ResponsePromoteToManager, receivedMsgHeader.sequenceNumber());
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << responseHeader;
    sendMessage(block);
}
