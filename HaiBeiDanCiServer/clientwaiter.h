#ifndef CLIENTWAITER_H
#define CLIENTWAITER_H

#include <QThread>
#include <QTcpSocket>

//! [0]
class ClientWaiter : public QThread
{
    Q_OBJECT

public:
    ClientWaiter(qintptr socketDescriptor, QObject *parent);

    void run() override;

signals:
    void error(QTcpSocket::SocketError socketError);

private:
    qintptr m_socketDescriptor;
    QTcpSocket *m_tcpSocket;

    void disconnectPeer();
    void failedToHandleMessage(int messageCode);

    int readMessageCode();
    bool handleMessage(int messageCode);

    bool handleRequestGetAllBooks();
    bool handleRequestGetWordsOfBook();
    bool handleRequestGetAWord();
    bool handleRequestGetABook();

};

#endif // CLIENTWAITER_H
