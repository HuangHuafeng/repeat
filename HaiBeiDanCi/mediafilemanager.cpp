#include "mediafilemanager.h"
#include "helpfunc.h"
#include "mysettings.h"

#include <QtConcurrent>

MediaFileManager * MediaFileManager::m_mfm;

MediaFileManager::MediaFileManager()
{
    m_efListReady = false;
}

MediaFileManager * MediaFileManager::instance()
{
    if (m_mfm == nullptr)
    {
        m_mfm = new MediaFileManager();
        QtConcurrent::run(std::bind(&MediaFileManager::refreshExistingFiles, m_mfm));
    }

    return m_mfm;
}

void MediaFileManager::refreshExistingFiles()
{
    qDebug() << "start checking existing media files.";
    m_efMutex.lock();
    QStringList existingFilesList = HelpFunc::filesInDir(MySettings::dataDirectory() + "/media");
    m_existingFiles = QSet<QString>::fromList(existingFilesList);
    m_efListReady = true;
    m_efMutex.unlock();
    qDebug() << "finish checking existing media files.";
}

const QSet<QString> & MediaFileManager::existingMediaFiles()
{
    return m_existingFiles;
}

void MediaFileManager::fileDownloaded(QString fileName)
{
    m_efMutex.lock();
    m_existingFiles.insert(fileName);
    m_efMutex.unlock();
}

bool MediaFileManager::isExistingFileListReady()
{
    return m_efListReady;
}
