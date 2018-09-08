#include "mediafilemanager.h"
#include "helpfunc.h"
#include "mysettings.h"
#include "wordbook.h"
#include "word.h"

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

const QSet<QString> MediaFileManager::missingPronounceAudioFiles(QString bookName)
{
    Q_ASSERT(m_efListReady == true);

    sptr<WordBook> book = WordBook::getBook(bookName);
    Q_ASSERT(book.get() != nullptr);

    // build the list of media files
    QStringList interestedFiles;
    QVector<QString> wordList = book->getAllWords();
    for (int i = 0;i < wordList.size();i ++)
    {
        QString spelling = wordList.at(i);
        sptr<Word> word = Word::getWord(spelling);
        Q_ASSERT(word.get() != nullptr);
        interestedFiles += word->pronounceFiles() + word->otherFiles();
    }

    QSet<QString> filesMissing = QSet<QString>::fromList(interestedFiles);
    filesMissing.subtract(m_existingFiles);

    return filesMissing;
}

const QSet<QString> MediaFileManager::missingExampleAudioFiles(QString bookName)
{
    Q_ASSERT(m_efListReady == true);

    sptr<WordBook> book = WordBook::getBook(bookName);
    Q_ASSERT(book.get() != nullptr);

    // build the list of media files
    QStringList interestedFiles;
    QVector<QString> wordList = book->getAllWords();
    for (int i = 0;i < wordList.size();i ++)
    {
        QString spelling = wordList.at(i);
        sptr<Word> word = Word::getWord(spelling);
        Q_ASSERT(word.get() != nullptr);
        interestedFiles += word->exampleAudioFiles() + word->otherFiles();
    }

    QSet<QString> filesMissing = QSet<QString>::fromList(interestedFiles);
    filesMissing.subtract(m_existingFiles);

    return filesMissing;
}
