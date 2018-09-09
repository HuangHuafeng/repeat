#include "mediafilemanager.h"
#include "helpfunc.h"
#include "mysettings.h"
#include "wordbook.h"
#include "word.h"
#include "worddb.h"

#include <QtConcurrent>

MediaFileManager * MediaFileManager::m_mfm;

MediaFileManager::MediaFileManager()
{
}

MediaFileManager * MediaFileManager::instance()
{
    if (m_mfm == nullptr)
    {
        m_mfm = new MediaFileManager();
        QtConcurrent::run(std::bind(&MediaFileManager::initialize, m_mfm));
    }

    return m_mfm;
}

void MediaFileManager::fileDownloaded(QString fileName)
{
    auto allBooks = WordBook::getAllBooks();
    for (int i = 0;i < allBooks.size();i ++)
    {
        auto bookName = allBooks.at(i);
        m_efMutex.lock();

        auto missingProunceAudioFiles = m_mapBookMissingPronounceAudioFiles.value(bookName);
        Q_ASSERT(missingProunceAudioFiles.get() != nullptr);
        missingProunceAudioFiles->remove(fileName);

        auto missingExampleAudioFiles = m_mapBookMissingExampleAudioFiles.value(bookName);
        Q_ASSERT(missingExampleAudioFiles.get() != nullptr);
        missingExampleAudioFiles->remove(fileName);

        m_efMutex.unlock();
    }
}

void MediaFileManager::bookDownloaded(QString bookName)
{
    m_dataRedayMutex.lock();
    m_dataReady = false;
    m_dataRedayMutex.unlock();

    qDebug() << bookName << "downloaded." << "Run initialize() again to build missing file list.";
    QtConcurrent::run(std::bind(&MediaFileManager::initialize, this));
}

bool MediaFileManager::isDataReady()
{
    m_dataRedayMutex.lock();
    bool dataReady = m_dataReady;
    m_dataRedayMutex.unlock();

    return dataReady;
}

sptr<QSet<QString>> MediaFileManager::bookMissingPronounceAudioFiles(QString bookName)
{
    m_efMutex.lock();
    auto missingProunceAudioFiles = m_mapBookMissingPronounceAudioFiles.value(bookName);
    m_efMutex.unlock();

    return missingProunceAudioFiles;
}

sptr<QSet<QString>> MediaFileManager::bookMissingExampleAudioFiles(QString bookName)
{
    m_efMutex.lock();
    auto missingExampleAudioFiles = m_mapBookMissingExampleAudioFiles.value(bookName);
    m_efMutex.unlock();

    return missingExampleAudioFiles;
}

QStringList MediaFileManager::bookPronounceMediaFileList(QString bookName)
{
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

    return interestedFiles;
}

QStringList MediaFileManager::bookExampleMediaFileList(QString bookName)
{
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

    return interestedFiles;
}

void MediaFileManager::initialize()
{
    m_dataRedayMutex.lock();
    m_dataReady = false;
    m_dataRedayMutex.unlock();
    // it's required to create the database connection
    // as we need to query database to like WordBook::getAllWords()
    // And this is in a different thread!
    WordDB::prepareDatabaseForThisThread();

    QStringList existingFileList = HelpFunc::filesInDir(MySettings::dataDirectory() + "/media");
    auto existingFiles = QSet<QString>::fromList(existingFileList);

    auto allBooks = WordBook::getAllBooks();
    for (int i = 0;i < allBooks.size();i ++)
    {
        initializeBookMissingFileList(allBooks.at(i), existingFiles);
    }

    m_dataRedayMutex.lock();
    m_dataReady = true;
    m_dataRedayMutex.unlock();
}

void MediaFileManager::initializeBookMissingFileList(QString bookName, const QSet<QString> &existingFiles)
{
    m_efMutex.lock();
    auto bookMissingFiles = m_mapBookMissingPronounceAudioFiles.value(bookName);
    m_efMutex.unlock();

    if (bookMissingFiles.get() != nullptr)
    {
        // the book is already processed
        return;
    }

    // pronounce media files
    auto pronounceAudioFiles = bookPronounceMediaFileList(bookName);
    sptr<QSet<QString>> missingPronounceAudioFiles = new QSet<QString>();
    *missingPronounceAudioFiles = QSet<QString>::fromList(pronounceAudioFiles);
    missingPronounceAudioFiles->subtract(existingFiles);
    m_efMutex.lock();
    m_mapBookMissingPronounceAudioFiles.insert(bookName, missingPronounceAudioFiles);
    m_efMutex.unlock();

    // example media files
    auto exampleAudioFiles = bookExampleMediaFileList(bookName);
    sptr<QSet<QString>> missingExampleAudioFiles = new QSet<QString>();
    *missingExampleAudioFiles = QSet<QString>::fromList(exampleAudioFiles);
    missingExampleAudioFiles->subtract(existingFiles);
    m_efMutex.lock();
    m_mapBookMissingExampleAudioFiles.insert(bookName, missingExampleAudioFiles);
    m_efMutex.unlock();
}
