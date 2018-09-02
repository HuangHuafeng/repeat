#ifndef CLIENT_H
#define CLIENT_H

#include <QTcpSocket>

class Client
{
public:
    Client(QTcpSocket &tcpSocket);

private:
    QTcpSocket &m_tcpSocket;
};

#endif // CLIENT_H
