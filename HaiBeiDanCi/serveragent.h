#ifndef SERVERAGENT_H
#define SERVERAGENT_H

#include <QObject>
#include <QtNetwork>
#include <QVector>
#include <QString>

class ServerAgent : public QObject
{
    Q_OBJECT

public:
    explicit ServerAgent(QObject *parent = nullptr);
    virtual ~ServerAgent();

    void connectToServer(const QString &hostName, quint16 port);
    void sendRequestGetAllBooks();
    void sendRequestGetWordsOfBook(QString bookName);

signals:
    void responseUnknownRequest();
    void responseGetAllBooks(QList<QString> books);
    void responseGetWordsOfBook(QString bookName, QVector<QString> wordList);
    void disconnected();

private slots:
    void onConnected();
    void onDisconnected();
    void onError(QAbstractSocket::SocketError socketError);
    void onReadyRead();
    void onStateChanged(QAbstractSocket::SocketState socketState);

private:
    QTcpSocket m_tcpSocket;

    int readMessageCode();
    bool handleMessage(int messageCode);
    bool handleResponseGetAllBooks();
    bool handleResponseGetWordsOfBook();
    bool handleResponseUnknownRequest();
};

#endif // SERVERAGENT_H
