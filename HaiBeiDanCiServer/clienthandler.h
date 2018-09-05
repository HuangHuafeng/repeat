#ifndef CLIENTHANDLER_H
#define CLIENTHANDLER_H

#include "clientwaiter.h"

#include <QByteArray>

class ClientHandler
{
private:
    ClientWaiter &m_clientWaiter;

    bool handleRequestNoOperation(const QByteArray &msg);

    void sendResponseUnknownRequest(const QByteArray &msg);
    void sendResponseNoOperation(const QByteArray &msg);

protected:
    void sendSimpleMessage(const QByteArray &msgToReply, qint32 msgCode);

public:
    ClientHandler(ClientWaiter &clientWaiter);
    virtual ~ClientHandler();

    void sendMessage(QByteArray msg);
    virtual int handleMessage(const QByteArray &msg);
    void handleUnknownMessage(const QByteArray &msg);
};

#endif // CLIENTHANDLER_H
