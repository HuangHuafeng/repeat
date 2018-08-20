#ifndef SERVERAGENT_H
#define SERVERAGENT_H

#include "wordbook.h"

#include <QObject>
#include <QtNetwork>
#include <QVector>
#include <QString>

/**
 * @brief The ServerAgent class
 * ServerAgent communicates to the server and provide/seal the objects in the server
 */
class ServerAgent : public QObject
{
    Q_OBJECT

public:
    explicit ServerAgent(QObject *parent = nullptr);
    virtual ~ServerAgent();

    void connectToServer(const QString &hostName, quint16 port);
    void sendRequestGetAllBooks();
    void sendRequestGetWordsOfBook(QString bookName);
    void sendRequestGetAWord(QString spelling);
    void sendRequestGetABook(QString bookName);

    void downloadBook(QString bookName);
    void getBookList();

signals:
    void responseUnknownRequest();
    void responseGetAllBooks(QList<QString> books);
    void responseGetWordsOfBook(QString bookName, QVector<QString> wordList);
    void disconnected();
    void responseGetAWord(Word word);
    void responseGetABook(WordBook book);
    void responseAllDataSent(int messageCode);
    void requestCompleted(int messageCode);

    void bookDownloaded(QString bookName);
    void bookListReady(const QList<QString> books);

private slots:
    void onConnected();
    void onDisconnected();
    void onError(QAbstractSocket::SocketError socketError);
    void onReadyRead();
    void onStateChanged(QAbstractSocket::SocketState socketState);

private:
    QTcpSocket m_tcpSocket;

    int readMessageCode();
    int handleMessage(int messageCode);
    void handleUnknownMessage(int messageCode);
    bool handleResponseGetAllBooks();
    bool handleResponseGetWordsOfBook();
    bool handleResponseUnknownRequest();
    bool handleResponseGetABook();
    bool handleResponseGetAWord();
    bool handleResponseAllDataSent();
};

#endif // SERVERAGENT_H
