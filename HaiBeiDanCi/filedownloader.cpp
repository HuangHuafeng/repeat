#include "filedownloader.h"

FileDownloader::FileDownloader(ServerCommunicator *sc, QObject *parent) :
    QObject(parent),
    m_sc(sc),
    m_showProgress(false),
    m_pd(nullptr)
{
    connect(m_sc, SIGNAL(fileDownloadProgress(QString, float)), this, SLOT(onFileDownloadProgress(QString, float)));
    connect(m_sc,
            SIGNAL(fileDownloaded(QString, ServerCommunicator::DownloadStatus, const QVector<QMap<const char *, uint>> *)),
            this,
            SLOT(onFileDownloaded(QString, ServerCommunicator::DownloadStatus, const QVector<QMap<const char *, uint>> *)));
}

FileDownloader::~FileDownloader()
{
    setShowProgress(false);
    qDebug() << "FileDownloader::~FileDownloader()";
}

void FileDownloader::setShowProgress(bool showProgress, QString labelText, QString cancelButtonText, QWidget *parent)
{
    // delete the current one, we always create a new one every time!!!
    if (m_pd != nullptr)
    {
        m_pd->deleteLater();
        m_pd = nullptr;
    }

    m_showProgress = showProgress;
    if (m_showProgress == true)
    {
        m_pd = new QProgressDialog(parent);
        m_pd->setLabelText("    " + labelText + "    ");
        m_pd->setCancelButtonText(cancelButtonText);
        m_pd->setModal(true);

        connect(m_pd, SIGNAL(canceled()), this, SLOT(onCanceled()));
    }
}

bool FileDownloader::downloadOngoing()
{
    return m_filesToDownload.isEmpty() == false;
}

bool FileDownloader::downloadFile(QString fileName, bool appFile)
{
    QStringList files;
    files << fileName;
    return downloadFiles(files, appFile);
}

bool FileDownloader::downloadFiles(const QStringList &files, bool appFile)
{
    if (files.isEmpty() == true)
    {
        return false;
    }

    if (downloadOngoing() == true)
    {
        return false;
    }

    m_downloaded = 0;
    m_toDownload = files.size();
    int requestsForARound = MySettings::numberOfRequestInEveryDownloadRound();
    for (int i = 0;i < files.size();i ++)
    {
        QString fileName = files.at(i);
        if (m_filesToDownload.contains(fileName) == false)
        {
            m_filesToDownload.insert(fileName, ServerCommunicator::WaitingDataFromServer);
            Q_ASSERT(m_sc != nullptr);
            m_sc->downloadFile(fileName, appFile);
        }
        else
        {
            m_downloaded ++;
        }

        if (i % requestsForARound == 0)
        {
            // process events so we don't make the app unresponsive
            QCoreApplication::processEvents(QEventLoop::ExcludeSocketNotifiers);
        }
    }

    return true;
}

void FileDownloader::onFileDownloadProgress(QString fileName, float percentage)
{
    qDebug() << fileName << "progress:" << percentage;
    qDebug() << "m_downloaded/m_toDownload" << m_downloaded << "/" << m_toDownload;
    if (m_pd != nullptr)
    {
        int newPercent = static_cast<int>(100 * (m_downloaded + percentage) / m_toDownload);
        m_pd->setValue(newPercent);
        qDebug() << "newPercent:" << newPercent;
    }
}

void FileDownloader::onFileDownloaded(QString fileName, ServerCommunicator::DownloadStatus result, const QVector<QMap<const char *, uint>> *fileContentBlocks)
{
    if (result == ServerCommunicator::DownloadSucceeded)
    {
        saveFileFromServer(fileName, fileContentBlocks);
    }
    else
    {
        qCritical() << "downloading of file" << fileName << "failed!";
    }

    // update the downloading result of the file
    m_filesToDownload.insert(fileName, result);

    // signal that a file is downloaded
    emit(fileDownloaded(fileName, result));

    // update the progress, this maybe redundant
    //onFileDownloadProgress(fileName, 1.0f);

    m_downloaded ++;
    if (m_downloaded == m_toDownload)
    {
        finishDownloading();
    }
}

void FileDownloader::onCanceled()
{
    qDebug() << "downloading is cancelled";
    Q_ASSERT(m_sc != nullptr);
    m_sc->cancelDownloading();
    finishDownloading();
}

void FileDownloader::finishDownloading()
{
    emit(downloadFinished(m_filesToDownload));
    m_filesToDownload.clear();
}

void FileDownloader::saveFileFromServer(QString fileName, const QVector<QMap<const char *, uint>> *fileContentBlocks)
{
    QString localFile = MySettings::dataDirectory() + "/" + fileName;
    QString folder = localFile.section('/', 0, -2);
    QDir::current().mkpath(folder);
    QFile toSave(localFile);

    if (toSave.open(QIODevice::WriteOnly) == false)
    {
        qCritical() << "Could not open" << localFile << "for writing:" << toSave.errorString();
        return;
    }

    qDebug() << "saving" << fileName;
    if (fileContentBlocks != nullptr)
    {
        for (int i = 0;i < fileContentBlocks->size();i ++)
        {
            auto currentBlock = fileContentBlocks->at(i);
            const char *data = currentBlock.firstKey();
            const uint len = currentBlock.first();
            toSave.write(data, len);
        }
    }
    else
    {
        // it's possible that there's no content for the file, like the file has size 0
    }

    toSave.close();
}
