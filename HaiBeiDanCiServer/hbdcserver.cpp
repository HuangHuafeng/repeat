#include "hbdcserver.h"
#include "clientwaiter.h"
#include "tokenmanager.h"

#include <QTimer>

#include <stdlib.h>

HBDCServer::HBDCServer(QObject *parent)
    : QTcpServer(parent),
      m_deleteScheduler(this)
{
    connect(&m_deleteScheduler, SIGNAL(timeout()), this, SLOT(onScheduleDelete()));
    m_deleteScheduler.start(MySettings::tokenLifeInSeconds() * 500);    // use 500, so schedule is 1/2 of token life, it's fast enough
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

void HBDCServer::onScheduleDelete()
{
    auto tm = TokenManager::instance();
    auto tokenIdList = tm->popUnscheduledTokens();
    for (int i = 0;i < tokenIdList.size();i ++)
    {
        // create a timer to remove the token
        auto id = tokenIdList.at(i);
        auto token = tm->getToken(id);
        if (token.get() == nullptr)
        {
            // the token is already deleted
            continue;
        }

        auto createTime =token->createTime();
        auto timeToDelete = token->lifeInSeconds() - createTime.secsTo(QDateTime::currentDateTime());
        QTimer *timer = new QTimer(this);
        timer->singleShot(timeToDelete * 1000, [id, tm, timer] () {
            tm->destroyToken(id);
            timer->deleteLater();
        });
    }
}
