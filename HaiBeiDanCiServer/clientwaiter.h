#ifndef CLIENTWAITER_H
#define CLIENTWAITER_H

#include "../HaiBeiDanCi/word.h"
#include "../HaiBeiDanCi/worddb.h"
#include "../HaiBeiDanCi/serverclientprotocol.h"
#include "../HaiBeiDanCi/wordbook.h"

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

public:
    void sendMessage(const QByteArray &msg, bool needCompress = false, bool = false);

private:
    qintptr m_socketDescriptor;
    QTcpSocket *m_tcpSocket;

    void disconnectPeer();
    bool waitForMoreData();

    QByteArray readMessage();
    void sendResponsePromoteToManager(const QByteArray &msg);

    void logMessage(const QByteArray &msg, int handleResult);
};

#endif // CLIENTWAITER_H
