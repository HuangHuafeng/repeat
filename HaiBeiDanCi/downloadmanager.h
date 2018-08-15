#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>

class DownloadManager: public QObject
{
    Q_OBJECT
    QNetworkAccessManager manager;
    QVector<QNetworkReply *> currentDownloads;
    QMap<QNetworkReply *, QString> m_fileNames;

public:
    DownloadManager();

    void download(const QUrl &url, QString saveTo);

signals:
    void fileDownloadFailed(QString fileName);
    void fileDownloaded(QString fileName);

public slots:
    void downloadFinished(QNetworkReply *reply);
    void sslErrors(const QList<QSslError> &errors);

private:
    void doDownload(const QUrl &url, QString saveTo);
    static QString saveFileName(const QUrl &url);
    bool saveToDisk(const QString &filename, QIODevice *data);
    static bool isHttpRedirect(QNetworkReply *reply);
};

#endif // DOWNLOADMANAGER_H
