#ifndef FILEDOWNLOADER_H
#define FILEDOWNLOADER_H

#include "servercommunicator.h"

#include <QObject>
#include <QProgressDialog>

/**
 * @brief The FileDownloader class
 * FileDownloader is used to download files from the server.
 * It intends to be short time object as the ServerCommunicator object
 * it uses may also be used by other objects. FileDownloader should be
 * deleted after the download finishes so no unexpected signals will be
 * received.
 * This is also true for BookDownloader, ServerUserAgent, VersionChecker
 */
class FileDownloader : public QObject
{
    Q_OBJECT

public:
    explicit FileDownloader(ServerCommunicator *sc = nullptr, QObject *parent = nullptr);
    ~FileDownloader();

    void setShowProgress(bool showProgress = true, QString labelText = QObject::tr("Downloading ..."), QString cancelButtonText = QString(), QWidget *parent = nullptr);
    bool downloadFiles(const QStringList &files, bool appFile = false);
    bool downloadFile(QString fileName, bool appFile = false);

signals:
    void fileDownloaded(QString fileName, ServerCommunicator::DownloadStatus result);
    void downloadFinished(const QMap<QString, ServerCommunicator::DownloadStatus> &downloadResult);

private slots:
    void onFileDownloadProgress(QString fileName, float percentage);
    void onFileDownloaded(QString fileName, ServerCommunicator::DownloadStatus result, const QVector<QMap<const char *, uint>> *fileContentBlocks);
    void onCanceled();

private:
    ServerCommunicator *m_sc;
    bool m_showProgress;
    QProgressDialog *m_pd;

    QMap<QString, ServerCommunicator::DownloadStatus> m_filesToDownload;
    int m_toDownload;
    int m_downloaded;

    void saveFileFromServer(QString fileName, const QVector<QMap<const char *, uint>> *fileContentBlocks);
    bool downloadOngoing();
    void finishDownloading();
};

#endif // FILEDOWNLOADER_H
