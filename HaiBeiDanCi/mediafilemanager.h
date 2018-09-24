#ifndef MEDIAFILEMANAGER_H
#define MEDIAFILEMANAGER_H

#include "../golddict/sptr.hh"

#include <QMap>
#include <QSet>
#include <QMutex>

class MediaFileManager
{
public:
    static MediaFileManager * instance();
    bool isDataReady();
    sptr<QSet<QString>> bookMissingPronounceAudioFiles(QString bookName);
    sptr<QSet<QString>> bookMissingExampleAudioFiles(QString bookName);

    void fileDownloaded(QString fileName);
    void bookDownloaded(QString bookName);
    void bookDeleted(QString bookName);

private:
    MediaFileManager();

    static MediaFileManager *m_mfm;

    QMap<QString, sptr<QSet<QString>>> m_mapBookMissingPronounceAudioFiles;
    QMap<QString, sptr<QSet<QString>>> m_mapBookMissingExampleAudioFiles;
    QMutex m_efMutex;
    bool m_dataReady;
    QMutex m_dataRedayMutex;

    void initialize();
    void initializeBookMissingFileList(QString bookName, const QSet<QString> &existingFiles);
    QStringList bookPronounceMediaFileList(QString bookName);
    QStringList bookExampleMediaFileList(QString bookName);
};

#endif // MEDIAFILEMANAGER_H
