#ifndef SVRAGT_H
#define SVRAGT_H

#include "wordbook.h"
#include "../golddict/sptr.hh"
#include "serverclientprotocol.h"
#include "applicationuser.h"
#include "token.h"

#include <QObject>
#include <QtNetwork>
#include <QVector>
#include <QString>
#include <QTimer>

/**
 * @brief The SvrAgt class
 * SvrAgt talks to the server with the basic messages
 * Any data received from the server is discarded after emitting signals
 *
 */
class SvrAgt : public QObject
{
    Q_OBJECT

public:
    typedef enum {
        WaitingDataFromServer = 1,
        DownloadSucceeded = 2,
        DownloadFailed = 3,
        DownloadCancelled = 4,
    } DownloadStatus;

    explicit SvrAgt(const QString &hostName, quint16 port = 61027, QObject *parent = nullptr);
    virtual ~SvrAgt();

    void downloadWords(const QVector<QString> &wordList);
    void downloadMultipleFiles(QSet<QString> files);
    void cancelDownloading();

    void downloadFile(QString fileName);
    void downloadAppFile(QString fileName);
    void downloadUpgrader(QString fileName);

    void sendRequestNoOperation();
    void sendRequestGetAllBooks();
    void sendRequestGetBookWordList(QString bookName);
    void sendRequestGetWordsOfBookFinished(QString bookName);
    void sendRequestGetAWord(QString spelling);
    void sendRequestGetABook(QString bookName);
    void sendRequestGetFile(QString fileName);
    void sendRequestGetAppFile(QString fileName);
    void sendRequestGetUpgrader(QString fileName);
    void sendRequestBye();

    void sendRequestRegister(const ApplicationUser &user);
    void sendRequestLogin(const ApplicationUser &user);
    void sendRequestLogout(QString name);

    void sendRequestAppVersion(QString platform);
    void sendRequestUpgraderVersion(QString platform);

signals:
    void bookListReady(const QList<QString> &books);
    void bookDownloaded(sptr<WordBook> book);
    void wordDownloaded(sptr<Word> word);
    void downloadProgress(float percentage);
    void fileDownloaded(QString fileName, SvrAgt::DownloadStatus result, const QVector<QMap<const char *, uint>> *fileContentBlocks);

    // getWordsOfBookFinished() is signalled when the message is echoed from the server
    // it's used like a check point that responses of all previous resquests are received
    // from the server
    void getWordsOfBookFinished(QString bookName);

    // bookWordListReceived() is signalled when the word list of a book has been received
    // from handleResponseBookWordListAllSent()
    void bookWordListReceived(QString bookName, const QVector<QString> &wordList);

    void serverConnected();

    void registerResult(qint32 result, const ApplicationUser &user);
    void loginResult(qint32 result, const ApplicationUser &user, const Token &token);
    void logoutResult(qint32 result, QString name);

    void appVersion(ReleaseInfo appReleaseInfo, ReleaseInfo appLibReleaseInfo);
    void upgraderVersion(ReleaseInfo upgraderReleaseInfo, ReleaseInfo upgraderLibReleaseInfo);

private slots:
    void onConnected();
    void onDisconnected();
    void onError(QAbstractSocket::SocketError socketError);
    void onReadyRead();
    void onStateChanged(QAbstractSocket::SocketState socketState);

    void onServerHeartBeat();
    void onSendMessageSmart();

private:
    QString m_serverHostName;
    quint16 m_serverPort;
    QTcpSocket *m_tcpSocket;

    QMap<QString, DownloadStatus> m_wordsToDownload;
    QMap<QString, DownloadStatus> m_filesToDownload;
    int m_toDownload;
    int m_downloaded;

    QMap<QString, QVector<QString>> m_mapBooksWordList;
    QMap<QString, QVector<QMap<const char *, uint>> *> m_mapFileContentBlocks;

    QTimer m_messageTimer;
    QTimer m_timerServerHeartBeat;

    QVector<QByteArray> m_messages;
    int m_lastResponded;
    int m_messagesSent;

    QByteArray readMessage();
    void sendTheFirstMessage();

    void connectToServer();
    void disconnectServer();
    void discardFileContent(QString fileName);

protected:
    virtual int handleMessage(const QByteArray &msg);
    bool handleResponseOK(const QByteArray &msg);
    bool handleResponseNoOperation(const QByteArray &msg);
    bool handleUnknownMessage(const QByteArray &msg);
    bool handleResponseGetAllBooks(const QByteArray &msg);
    bool handleResponseGetBookWordList(const QByteArray &msg);
    bool handleResponseUnknownRequest(const QByteArray &msg);
    bool handleResponseGetABook(const QByteArray &msg);
    bool handleResponseGetAWord(const QByteArray &msg);
    bool handleResponseGetFile(const QByteArray &msg);
    bool handleResponseGetFileFinished(const QByteArray &msg);
    bool handleResponseGetWordsOfBookFinished(const QByteArray &msg);
    bool handleResponseRegister(const QByteArray &msg);
    bool handleResponseLogin(const QByteArray &msg);
    bool handleResponseInvalidTokenId(const QByteArray &msg);
    bool handleResponseLogout(const QByteArray &msg);
    bool handleResponseAppVersion(const QByteArray &msg);
    bool handleResponseUpgraderVersion(const QByteArray &msg);

    void sendMessage(const QByteArray &msg, bool needCompress = false, bool now = false);
    void updateAndEmitProgress();
    void cancelDownloadingWords();
    void cancelDownloadingFiles();

    void sendSimpleMessage(qint32 msgCode, bool now = false);
};

#endif // SVRAGT_H
