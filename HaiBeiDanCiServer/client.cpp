#include "client.h"

Client::Client(QTcpSocket &tcpSocket) :
    m_tcpSocket(tcpSocket)
{

}
