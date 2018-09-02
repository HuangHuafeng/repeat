#ifndef SERVERAGENT_H
#define SERVERAGENT_H

#include "wordbook.h"
#include "../golddict/sptr.hh"
#include "serverclientprotocol.h"

#include <QObject>
#include <QtNetwork>
#include <QVector>
#include <QString>
#include <QTimer>

/**
 * @brief The ServerAgent class
 * ServerAgent communicates to the server and provide/seal the objects in the server
 */
class ServerAgent : public QObject
{
    Q_OBJECT

public:
    typedef enum {
        WaitingDataFromServer = 1,
        DownloadSucceeded = 2,
        DownloadFailed = 3,
        DownloadCancelled = 4,
    } DownloadStatus;

    virtual ~ServerAgent();

    static ServerAgent * instance();

    void getBookList();
    void downloadBook(QString bookName);
    void downloadFile(QString fileName);
    bool fileExistsLocally(QString fileName);
    const QMap<QString, ServerAgent::DownloadStatus> &downloadMultipleFiles(QSet<QString> files);
    void cancelDownloading();
    void disconnectServer();

signals:
    void bookListReady(const QList<QString> books);
    void bookDownloaded(QString bookName);
    void wordDownloaded(QString spelling);
    void downloadProgress(float percentage);
    void fileDownloaded(QString fileName, bool succeeded);

    // this signal is sent after data related to a book is downloaded from the server
    // the agent should connect to this signal to save the book as saving a book may time-consuming
    void internalBookDataDownloaded(QString bookName);
    void internalFileDataDownloaded(QString fileName, bool succeeded);
    void internalAWordDataDownloaded();

private slots:
    void onConnected();
    void onDisconnected();
    void onError(QAbstractSocket::SocketError socketError);
    void onReadyRead();
    void onStateChanged(QAbstractSocket::SocketState socketState);

    void onServerHeartBeat();
    void onSendMessage();
    void onSendMessageSmart();
    void onInternalBookDataDownloaded(QString bookName);
    void onInternalFileDataDownloaded(QString fileName, bool succeeded);

private:
    explicit ServerAgent(const QString &hostName, quint16 port = 61027, QObject *parent = nullptr);
    static ServerAgent *m_serveragent;

    QString m_serverHostName;
    quint16 m_serverPort;
    QTcpSocket *m_tcpSocket;

    QList<QString> m_booksInServer;
    QMap<QString, sptr<WordBook>> m_mapBooks;
    QMap<QString, QVector<QString>> m_mapBooksWordList;
    QMap<QString, sptr<Word>> m_mapWords;
    QMap<QString, QByteArray>  m_mapFileContent;

    QMap<QString, DownloadStatus> m_wordsToDownload;
    QMap<QString, DownloadStatus> m_filesToDownload;
    QTimer m_messageTimer;

    QTimer m_timerServerHeartBeat;

    QVector<QByteArray> m_messages;
    int m_lastResponded;
    int m_messagesSent;

    void completeBookDownload(QString bookName);
    bool saveFileFromServer(QString fileName);

    // these handle*() functions should be fast as the socket is busy (receving data)
    // if they take long time, it can cause slow the socket to buffer data
    // thus result unnecessary failure in read transition
    QByteArray readMessage();
    int handleMessage(const QByteArray &msg);
    bool handleResponseNoOperation(const QByteArray &msg);
    bool handleUnknownMessage(const QByteArray &msg);
    bool handleResponseGetAllBooks(const QByteArray &msg);
    bool handleResponseGetBookWordList(const QByteArray &msg);
    bool handleResponseBookWordListAllSent(const QByteArray &msg);
    bool handleResponseUnknownRequest(const QByteArray &msg);
    bool handleResponseGetABook(const QByteArray &msg);
    bool handleResponseGetAWord(const QByteArray &msg);
    bool handleResponseGetFile(const QByteArray &msg);
    bool handleResponseGetFileFinished(const QByteArray &msg);
    bool handleResponseGetWordsOfBookFinished(const QByteArray &msg);

    void connectToServer();
    void sendRequestNoOperation();
    void sendRequestGetAllBooks();
    void sendRequestGetBookWordList(QString bookName);
    void sendRequestGetWordsOfBookFinished(QString bookName);
    void sendRequestGetAWord(QString spelling);
    void sendRequestGetABook(QString bookName);
    void sendRequestGetWords(QString bookName, QVector<QString> wordList);
    void sendRequestGetWordsWithSmallMessages(QString bookName, QVector<QString> wordList);
    void sendRequestGetFile(QString fileName);
    void sendRequestBye();

    void downloadWordsOfBook(QString bookName);
    void requestFiles();
    void requestWords();

    void sendMessage(const QByteArray &msg, bool now = false);
    void sendTheFirstMessage();
    float getProgressPercentage(const QMap<QString, DownloadStatus> mapToDownload);
    void cancelDownloadingWords();
    void cancelDownloadingFiles();
};

#endif // SERVERAGENT_H
