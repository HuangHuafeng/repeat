#ifndef SERVERCOMMUNICATOR_H
#define SERVERCOMMUNICATOR_H

#include "wordbook.h"
#include "serverclientprotocol.h"
#include "applicationuser.h"
#include "token.h"

#include <QObject>
#include <QtNetwork>
#include <QVector>
#include <QString>
#include <QTimer>

/**
 * @brief The ServerCommunicator class
 * ServerCommunicator talks to the server with messages
 * It is intended to be used by other objects.
 * Any data received from the server is discarded after emitting signals.
 * So the objects which recieve the signal should store the data if needed.
 */
class ServerCommunicator : public QObject
{
    Q_OBJECT

public:
    typedef enum {
        WaitingDataFromServer = 1,
        DownloadSucceeded = 2,
        DownloadFailed = 3,
        DownloadCancelled = 4,
    } DownloadStatus;

    static ServerCommunicator * instance();
    explicit ServerCommunicator(QString hostName = QString(), quint16 port = 0, QObject *parent = nullptr);
    virtual ~ServerCommunicator();

    void downloadFile(QString fileName, bool appFile = false);
    void downloadWord(QString spelling);
    void cancelDownloading();

    void sendRequestNoOperation();
    void sendRequestGetAllBooks();
    void sendRequestGetBookWordList(QString bookName);
    void sendRequestGetABook(QString bookName);
    void sendRequestBye();

    void sendRequestRegister(const ApplicationUser &user);
    void sendRequestLogin(const ApplicationUser &user);
    void sendRequestLogout(QString name);

    void sendRequestAppVersion(QString platform);
    void sendRequestUpgraderVersion(QString platform);

signals:
    void serverConnected();
    void bookListDownloaded(const QList<QString> &books);
    void bookDownloaded(const WordBook &book);
    void wordDownloaded(const Word &word);
    void fileDownloaded(QString fileName, ServerCommunicator::DownloadStatus result, const QVector<QMap<const char *, uint>> *fileContentBlocks);
    void fileDownloadProgress(QString fileName, float percentage);
    void bookWordListReceived(QString bookName, const QVector<QString> &wordList);
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
    static ServerCommunicator *m_sc;
    QString m_serverHostName;
    quint16 m_serverPort;
    QTcpSocket *m_tcpSocket;

    QMap<QString, DownloadStatus> m_wordsInDownloading;
    QMap<QString, DownloadStatus> m_filesInDownloading;

    QMap<QString, QVector<QString> *> m_mapBooksWordList;
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
    void sendRequestGetFile(QString fileName);
    void sendRequestGetAppFile(QString fileName);
    void sendRequestGetAWord(QString spelling);

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
    bool handleResponseRegister(const QByteArray &msg);
    bool handleResponseLogin(const QByteArray &msg);
    bool handleResponseInvalidTokenId(const QByteArray &msg);
    bool handleResponseLogout(const QByteArray &msg);
    bool handleResponseAppVersion(const QByteArray &msg);
    bool handleResponseUpgraderVersion(const QByteArray &msg);

    void sendMessage(const QByteArray &msg, bool needCompress = false, bool now = false);
    void sendSimpleMessage(qint32 msgCode, bool now = false);
};

#endif // SERVERCOMMUNICATOR_H
