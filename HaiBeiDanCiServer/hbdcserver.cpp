#include "hbdcserver.h"
#include "clientwaiter.h"

#include <QRandomGenerator>

#include <stdlib.h>

HBDCServer::HBDCServer(QObject *parent)
    : QTcpServer(parent)
{
}


bool HBDCServer::listen(const QHostAddress &address, quint16 port)
{
    if (port == 0)
    {
        port = 61027;
    }

    return QTcpServer::listen(address, port);
}

void HBDCServer::incomingConnection(qintptr socketDescriptor)
{
    ClientWaiter *thread = new ClientWaiter(socketDescriptor, nullptr);
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    thread->start();
}

