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
    ptrClientHandler->setPeerAddress(m_tcpSocket->peerAddress());
    int consecutiveHeartbeat = 0;
    while (1)
    {
        if (consecutiveHeartbeat > MySettings::maximumConsecutiveHeartbeat())
        {
            // no data/request from the client for a while
            break;
        }

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
        //int handleResult = ptrClientHandler->handleMessage(msg);
        int handleResult = ptrClientHandler->processMessage(msg);
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
        }
        else if (handleResult == 1)
        {
        }
        else if (handleResult == -1)
        {
            // check if it's the message to change the client handler
            if (receivedMsgHeader.code() == ServerClientProtocol::RequestPromoteToManager)
            {
                ptrClientHandler = new HBDCManagerHandler(*this);
                ptrClientHandler->setPeerAddress(m_tcpSocket->peerAddress());
                sendResponsePromoteToManager(msg);
                qDebug() << "promoted the handler to manager\n";
                handleResult = 0;
            }
            else
            {
                ptrClientHandler->handleUnknownMessage(msg);

                // treat unknown message as heartbeat, so client will be disconnected
                // when there're too many unknown messages
                consecutiveHeartbeat ++;
            }
        }
        else if (handleResult == -2)
        {
            // client said good bye!
            break;
        }
        else if (handleResult == -3)
        {
            // message does have the expected token id
        }
        else
        {
            break;
        }

        logMessage(msg, handleResult);
        qDebug() << "consecutiveHeartbeat:" << consecutiveHeartbeat << endl;
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

void ClientWaiter::sendSimpleMessage(const QByteArray &msgToReply, qint32 msgCode)
{
    MessageHeader receivedMsgHeader(msgToReply);
    MessageHeader responseHeader(msgCode, receivedMsgHeader.sequenceNumber());
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << responseHeader;
    sendMessage(block);
}

// should be changed later
void ClientWaiter::sendMessage(const QByteArray &msg, bool needCompress, bool /*now*/)
{
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

    m_tcpSocket->write(block);
}

QByteArray ClientWaiter::readMessage()
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

void ClientWaiter::sendResponsePromoteToManager(const QByteArray &msg)
{
    sendSimpleMessage(msg, ServerClientProtocol::ResponsePromoteToManager);
}

void ClientWaiter::logMessage(const QByteArray &msg, int handleResult)
{
    MessageHeader receivedMsgHeader(msg);
    qDebug() << QDateTime::currentDateTime().toString() << "received message with header:";
    qDebug("%s", receivedMsgHeader.toString().toUtf8().constData());
    qDebug() << "handle result:" << handleResult;
}
