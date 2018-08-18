#include "hbdcserver.h"
#include "clientwaiter.h"

#include <QRandomGenerator>

#include <stdlib.h>

HBDCServer::HBDCServer(QObject *parent)
    : QTcpServer(parent)
{
}

void HBDCServer::incomingConnection(qintptr socketDescriptor)
{
    ClientWaiter *thread = new ClientWaiter(socketDescriptor, this);
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    thread->start();
}

