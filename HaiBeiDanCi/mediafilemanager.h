#ifndef MEDIAFILEMANAGER_H
#define MEDIAFILEMANAGER_H

#include <QSet>
#include <QMutex>

class MediaFileManager
{
public:
    static MediaFileManager * instance();
    const QSet<QString> & existingMediaFiles();
    void fileDownloaded(QString fileName);
    bool isExistingFileListReady();

private:
    MediaFileManager();

    static MediaFileManager *m_mfm;

    QSet<QString> m_existingFiles;
    QMutex m_efMutex;
    bool m_efListReady;

    void refreshExistingFiles();
};

#endif // MEDIAFILEMANAGER_H