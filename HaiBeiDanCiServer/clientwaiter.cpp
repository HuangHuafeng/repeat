#include "clientwaiter.h"
#include "../HaiBeiDanCi/word.h"

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

    qDebug("%s:%d connected", tcpSocket.peerAddress().toString().toLatin1().constData(), tcpSocket.peerPort());


    /*
    auto w = Word::getWord("over");
    if (w.get() != nullptr)
    {
        //out << w->getDefinition();
        QString reply = "ABC";
        out << reply;
        tcpSocket.write(block);
    }
    */

    auto counter = 0;
    while (1)
    {
        if (tcpSocket.waitForReadyRead() == false)
        {
            break;
        }

        int reqOpcode;
        QString reqPara;
        QDataStream in(&tcpSocket);
        in.startTransaction();
        in >> reqOpcode >> reqPara;
        in.commitTransaction();
        qDebug() << reqOpcode << reqPara;

        //auto data = tcpSocket.readAll();
        //qDebug() << data.constData();

        QByteArray block;
        QDataStream out(&block, QIODevice::WriteOnly);
        //out.setVersion(QDataStream::Qt_5_11);
        //out.setVersion(QDataStream::Qt_4_0);

        counter ++;
        int opcode = counter;
        int responseResult = 0;
        QString response = "Hello " + QString::number(counter);
        out << opcode << responseResult << response;
        tcpSocket.write(block);

        qDebug() << block;
    }


    tcpSocket.disconnectFromHost();
    if (tcpSocket.state() != QAbstractSocket::UnconnectedState)
    {
        tcpSocket.waitForDisconnected();
    }
    qDebug() << "disconnected.";
}
