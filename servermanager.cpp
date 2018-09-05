#include "servermanager.h"
#include "HaiBeiDanCi/mysettings.h"

ServerManager * ServerManager::m_sm = nullptr;

ServerManager::ServerManager(QObject *parent) : QObject(parent),
    m_mgrAgt(MySettings::serverHostName(), MySettings::serverPort(), this)
{
    connect(&m_mgrAgt, SIGNAL(bookListReady(const QList<QString>)), this, SLOT(OnBookListReady(const QList<QString>)));
    connect(&m_mgrAgt, SIGNAL(bookDownloaded(sptr<WordBook>)), this, SLOT(OnBookDownloaded(sptr<WordBook>)));
    connect(&m_mgrAgt, SIGNAL(wordDownloaded(sptr<Word>)), this, SLOT(OnWordDownloaded(sptr<Word>)));
    connect(&m_mgrAgt, SIGNAL(downloadProgress(float)), this, SLOT(OnDownloadProgress(float)));
    connect(&m_mgrAgt, SIGNAL(fileDownloaded(QString, SvrAgt::DownloadStatus, const QByteArray &)), this, SLOT(OnFileDownloaded(QString, SvrAgt::DownloadStatus, QByteArray)));
    connect(&m_mgrAgt, SIGNAL(getWordsOfBookFinished(QString)), this, SLOT(OnGetWordsOfBookFinished(QString)));
    connect(&m_mgrAgt, SIGNAL(bookWordListReceived(QString, const QVector<QString> &)), this, SLOT(OnBookWordListReceived(QString, const QVector<QString> &)));
    connect(&m_mgrAgt, SIGNAL(serverConnected()), this, SLOT(onServerConnected()));

    connect(&m_mgrAgt, SIGNAL(getServerDataFinished()), this, SLOT(onGetServerDataFinished()));
    connect(&m_mgrAgt, SIGNAL(getAllWordsWithoutDefinitionFinished(const QVector<QString> &, const QVector<int> &, const QVector<int> &)), this, SLOT(onGetAllWordsWithoutDefinitionFinished(const QVector<QString> &, const QVector<int> &, const QVector<int> &)));
}

ServerManager * ServerManager::instance()
{
    if (m_sm == nullptr)
    {
        m_sm = new ServerManager();
    }

    return m_sm;
}

void ServerManager::OnBookListReady(const QList<QString> &books)
{
    m_mapBooks.clear();
    for (int i = 0;i < books.size();i ++)
    {
        m_mapBooks.insert(books.at(i), sptr<WordBook>());
    }

    //emit(bookListReady(books));

    // RELOAD SERVER DATA
    // STEP 2: we now know the names of the books, we can get
    // * id, intro of each book
    // * word list of each book
    for (int i = 0;i < books.size();i ++)
    {
        auto bookName = books.at(i);
        m_mgrAgt.sendRequestGetABook(bookName);
        m_mgrAgt.sendRequestGetBookWordList(bookName);
    }

    // RELOAD SERVER DATA
    // STEP 3: it seems we have requested all we need
    // send the sendRequestGetServerDataFinished message
    // so we know that the data is ready when we got echo from the server
    m_mgrAgt.sendRequestGetServerDataFinished();
}

void ServerManager::OnBookDownloaded(sptr<WordBook> book)
{
    Q_ASSERT(book.get() != nullptr);

    m_mapBooks.insert(book->getName(), book);
}

void ServerManager::OnWordDownloaded(sptr<Word> word)
{
    Q_ASSERT(word.get() != nullptr);

    // words will be store later in one transaction!!!
    m_mapWords.insert(word->getSpelling(), word);
}

void ServerManager::OnDownloadProgress(float percentage)
{
    emit(downloadProgress(percentage));
}

void ServerManager::OnFileDownloaded(QString fileName, SvrAgt::DownloadStatus result, const QByteArray &fileContent)
{
    if (result == SvrAgt::DownloadSucceeded)
    {
        saveFileFromServer(fileName, fileContent);
    }
    emit(fileDownloaded(fileName, result == SvrAgt::DownloadSucceeded));
}

void ServerManager::OnGetWordsOfBookFinished(QString bookName)
{
    // the book is downloaded, save it to database
#ifndef QT_NO_DEBUG
    QElapsedTimer t;
    t.start();
#endif
    // store the words
    Word::v2StoreMultipleWordFromServer(m_mapWords);
    m_mapWords.clear();

    // store the book
    auto book = m_mapBooks.value(bookName);
    Q_ASSERT(book.get() != nullptr);
    auto wordList = m_mapBooksWordList.value(bookName);
    WordBook::storeBookFromServer(book, wordList);

#ifndef QT_NO_DEBUG
    qDebug() << "Used" << t.elapsed() << "ms in OnGetWordsOfBookFinished()";
#endif

    emit(bookStored(bookName));
    emit(downloadProgress(1.0f));
}

void ServerManager::OnBookWordListReceived(QString bookName, const QVector<QString> &wordList)
{
    m_mapBooksWordList.insert(bookName, wordList);
}

/**
 * @brief ServerManager::downloadAllBooks
 * downloadAllBooks()
 * * does not download the list of words in the book
 * * let alone the words
 */
void ServerManager::downloadAllBooks()
{
    auto books = m_mapBooks.keys();
    for (int i = 0;i < books.size();i ++)
    {
        m_mgrAgt.sendRequestGetABook(books.at(i));
    }
}

void ServerManager::saveFileFromServer(QString fileName, const QByteArray &fileContent)
{
    QString localFile = MySettings::dataDirectory() + "/" + fileName;
    QString folder = localFile.section('/', 0, -2);
    QDir::current().mkpath(folder);
    QFile toSave(localFile);

    if (toSave.open(QIODevice::WriteOnly) == false)
    {
        qInfo() << "Could not open" << localFile << "for writing:" << toSave.errorString();
        return;
    }

    qDebug() << "saving" << fileName << "size" << fileContent.size();
    toSave.write(fileContent.constData(), fileContent.size());
    toSave.close();
}

void ServerManager::downloadWordsOfBook(QString bookName)
{
    m_mapWords.clear();
    auto wordList = m_mapBooksWordList.value(bookName);
    QVector<QString> wordsToDownload;
    // get the list of words which are not available locally
    for (int i = 0;i < wordList.size();i ++)
    {
        auto spelling = wordList.at(i);

        if (Word::getWord(spelling).get() == nullptr && m_mapWords.contains(spelling) == false)
        {
            wordsToDownload.append(spelling);
        }
    }

    m_mgrAgt.downloadWords(wordsToDownload);
    m_mgrAgt.sendRequestGetWordsOfBookFinished(bookName);
}

QList<QString> ServerManager::getBookList()
{
    return m_mapBooks.keys();
}

void ServerManager::onServerConnected()
{
    qDebug() << "server connected";

    m_mgrAgt.sendRequestPromoteToManager();
}

void ServerManager::onGetServerDataFinished()
{
    // RELOAD SERVER DATA
    // we got all the data when we get this message
    emit(serverDataReloaded());
    printData();
}

void ServerManager::onGetAllWordsWithoutDefinitionFinished(const QVector<QString> &spellings, const QVector<int> &ids, const QVector<int> &definitionLengths)
{
    int length = spellings.size();
    Q_ASSERT(length == ids.size());
    Q_ASSERT(length == definitionLengths.size());

    for (int i = 0;i < length;i ++)
    {
        auto spelling = spellings.at(i);
        auto id = ids.at(i);
        auto defLen = definitionLengths.at(i);
        auto definition = QString::number(defLen) + "bytes";
        sptr<Word> newWord = new Word(spelling, definition, id);
        m_mapWords.insert(spelling, newWord);
    }
}

/**
 * @brief ServerManager::reloadServerData
 * send messages to the server to get the data:
 *  1. words (only spelling and id)
 *  2. books (id, name, intro, wordlist)
 */
void ServerManager::reloadServerData()
{
    clearData();

    // RELOAD SERVER DATA
    // STEP 1: at this moment, we can get
    // * all the words without definition (as definition is big)
    // * the list of books
    m_mgrAgt.sendRequestGetAllWordsWithoutDefinition();
    m_mgrAgt.sendRequestGetAllBooks();
}


void ServerManager::clearData()
{
    m_mapBooks.clear();
    m_mapBooksWordList.clear();
    m_mapWords.clear();
}

void ServerManager::printData()
{
    auto books = m_mapBooks.keys();
    for (int i = 0;i < books.size();i ++)
    {
        auto bookName = books.at(i);
        qDebug() << bookName << m_mapBooksWordList.value(bookName).size() << "words";
    }

    qDebug() << "total words:" << m_mapWords.size();
}
