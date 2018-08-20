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
    void responseUnknownRequest();
    void responseGetAllBooks(QList<QString> books);
    void responseGetWordsOfBook(QString bookName, QVector<QString> wordList);
    void disconnected();
    void responseGetAWord(const Word &word);
    void responseGetABook(const WordBook &book);
    void responseAllDataSent(int messageCode);
    void requestCompleted(int messageCode);

    // the following signals are more useful
    void bookDownloaded(QString bookName);
    void bookListReady(const QList<QString> books);

private slots:
    void onConnected();
    void onDisconnected();
    void onError(QAbstractSocket::SocketError socketError);
    void onReadyRead();
    void onStateChanged(QAbstractSocket::SocketState socketState);

    void onResponseGetABook(const WordBook &book);
    void onResponseGetWordsOfBook(QString bookName, QVector<QString> wordList);
    void onResponseGetAWord(const Word &word);

private:
    explicit ServerAgent(const QString &hostName, quint16 port = 61027, QObject *parent = nullptr);
    static ServerAgent *m_serveragent;

    QString m_serverHostName;
    quint16 m_serverPort;
    QTcpSocket *m_tcpSocket;

    QList<QString> m_books;
    QMap<QString, bool> m_mapBooksStatus;
    QMap<QString, sptr<WordBook>> m_mapBooks;
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
};

#endif // SERVERAGENT_H
