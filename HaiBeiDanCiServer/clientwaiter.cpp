#include "clientwaiter.h"

#include <QtNetwork>

ClientWaiter::ClientWaiter(qintptr socketDescriptor, QObject *parent)
    : QThread(parent), m_socketDescriptor(socketDescriptor)
{
}

void ClientWaiter::run()
{
    QTcpSocket tcpSocket;

    if (!tcpSocket.setSocketDescriptor(m_socketDescriptor)) {
        emit error(tcpSocket.error());
        return;
    }

    QString text = "Hello, it's " + QDateTime::currentDateTime().toString() + "\n";
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    //out.setVersion(QDataStream::Qt_4_0);
    out << text;

    tcpSocket.write(block);
    //tcpSocket.write(text.toStdString().c_str());
    tcpSocket.disconnectFromHost();
    tcpSocket.waitForDisconnected();
}
