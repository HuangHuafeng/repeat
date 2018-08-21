#ifndef SERVERAGENT_H
#define SERVERAGENT_H

#include "wordbook.h"
#include "../golddict/sptr.hh"

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
    virtual ~ServerAgent();

    static ServerAgent * instance();

    void downloadBook(QString bookName);
    void getBookList();

signals:
    void bookListReady(const QList<QString> books);
    void bookDownloaded(QString bookName);
    void wordDownloaded(QString spelling);
    void downloadProgress(float percentage);

private slots:
    void onConnected();
    void onDisconnected();
    void onError(QAbstractSocket::SocketError socketError);
    void onReadyRead();
    void onStateChanged(QAbstractSocket::SocketState socketState);

private:
    explicit ServerAgent(const QString &hostName, quint16 port = 61027, QObject *parent = nullptr);
    static ServerAgent *m_serveragent;

    QString m_serverHostName;
    quint16 m_serverPort;
    QTcpSocket *m_tcpSocket;

    int m_numberOfWordsToDownload;
    int m_numberOfWordsDownloaded;

    QMap<QString, sptr<WordBook>> m_mapBooks;
    QMap<QString, bool> m_mapBooksStatus;
    QMap<QString, QVector<QString>> m_mapBooksWordList;

    QMap<QString, sptr<Word>> m_mapWords;

    int readMessageCode();
    int handleMessage(int messageCode);
    bool handleUnknownMessage(int messageCode);
    bool handleResponseGetAllBooks();
    bool handleResponseGetWordsOfBook();
    bool handleResponseUnknownRequest();
    bool handleResponseGetABook();
    bool handleResponseGetAWord();
    bool handleResponseAllDataSent();
    bool handleResponseAllDataSentForRequestGetWordsOfBook();
    bool handleResponseAllDataSentForRequestGetWords();

    void connectToServer();
    void sendRequestGetAllBooks();
    void sendRequestGetWordsOfBook(QString bookName);
    void sendRequestGetAWord(QString spelling);
    void sendRequestGetABook(QString bookName);
    void sendRequestGetWords(QString bookName, QVector<QString> wordList);
    void sendRequestGetWordsWithSmallMessages(QString bookName, QVector<QString> wordList);

    void requestWords(QString bookName, QVector<QString> wordList);
};

#endif // SERVERAGENT_H
