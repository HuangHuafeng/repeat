#include "serverdatadownloader.h"
#include "mysettings.h"

ServerDataDownloader * ServerDataDownloader::m_sdd = nullptr;

ServerDataDownloader::ServerDataDownloader(QObject *parent) : QObject(parent),
    m_svrAgt(MySettings::serverHostName(), MySettings::serverPort(), this),
    m_bookListDownloaded(false)
{
    connect(&m_svrAgt, SIGNAL(serverConnected()), this, SLOT(onServerConnected()));
    connect(&m_svrAgt, SIGNAL(bookListReady(const QList<QString>)), this, SLOT(OnBookListReady(const QList<QString>)));
    connect(&m_svrAgt, SIGNAL(bookDownloaded(sptr<WordBook>)), this, SLOT(OnBookDownloaded(sptr<WordBook>)));
    connect(&m_svrAgt, SIGNAL(wordDownloaded(sptr<Word>)), this, SLOT(OnWordDownloaded(sptr<Word>)));
    connect(&m_svrAgt, SIGNAL(downloadProgress(float)), this, SLOT(OnDownloadProgress(float)));
    connect(&m_svrAgt, SIGNAL(fileDownloaded(QString, SvrAgt::DownloadStatus, const QByteArray &)), this, SLOT(OnFileDownloaded(QString, SvrAgt::DownloadStatus, QByteArray)));
    connect(&m_svrAgt, SIGNAL(getWordsOfBookFinished(QString)), this, SLOT(OnGetWordsOfBookFinished(QString)));
    connect(&m_svrAgt, SIGNAL(bookWordListReceived(QString, const QVector<QString> &)), this, SLOT(OnBookWordListReceived(QString, const QVector<QString> &)));

    m_svrAgt.connectToServer();
}

ServerDataDownloader * ServerDataDownloader::instance()
{
    if (m_sdd == nullptr)
    {
        m_sdd = new ServerDataDownloader();
    }

    return m_sdd;
}

void ServerDataDownloader::destroy()
{
    if (m_sdd != nullptr)
    {
        m_sdd->disconnectServer();
        m_sdd->deleteLater();
        m_sdd = nullptr;
    }
}

void ServerDataDownloader::onServerConnected()
{
    qDebug() << "server connected";
    m_svrAgt.sendRequestGetAllBooks();
}

void ServerDataDownloader::OnBookListReady(const QList<QString> &books)
{
    m_mapBooks.clear();
    for (int i = 0;i < books.size();i ++)
    {
        m_mapBooks.insert(books.at(i), sptr<WordBook>());
    }
    m_bookListDownloaded = true;
    emit(bookListReady(books));
    downloadAllBooks();
}

void ServerDataDownloader::OnBookDownloaded(sptr<WordBook> book)
{
    Q_ASSERT(book.get() != nullptr);

    m_mapBooks.insert(book->getName(), book);
}

void ServerDataDownloader::OnWordDownloaded(sptr<Word> word)
{
    Q_ASSERT(word.get() != nullptr);

    // words will be store later in one transaction!!!
    m_mapWords.insert(word->getSpelling(), word);
}

void ServerDataDownloader::OnDownloadProgress(float percentage)
{
    emit(downloadProgress(percentage));
}

void ServerDataDownloader::OnFileDownloaded(QString fileName, SvrAgt::DownloadStatus result, const QByteArray &fileContent)
{
    if (result == SvrAgt::DownloadSucceeded)
    {
        saveFileFromServer(fileName, fileContent);
    }
    else
    {
        qDebug() << "downloading of file" << fileName << "failed!";
    }
    emit(fileDownloaded(fileName, result == SvrAgt::DownloadSucceeded));
}

void ServerDataDownloader::OnGetWordsOfBookFinished(QString bookName)
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

    // DOWNLOAD BOOK
    // STEP 3: words of the book saved, book saved, signal the completion

    emit(bookStored(bookName));
    emit(downloadProgress(1.0f));
}

void ServerDataDownloader::OnBookWordListReceived(QString bookName, const QVector<QString> &wordList)
{
    m_mapBooksWordList.insert(bookName, wordList);

    // DOWNLOAD BOOK
    // STEP 2: received the words of the book, get the definition of these words
    downloadWordsOfBook(bookName);
}


QList<QString> ServerDataDownloader::getBookList()
{
    return m_mapBooks.keys();
}

void ServerDataDownloader::downloadBook(QString bookName)
{
    // only download a book when it does NOT exist locally
    if (WordBook::getBook(bookName).get() == nullptr)
    {
        // DOWNLOAD BOOK
        // STEP 1: get the list of words in the book
        m_svrAgt.sendRequestGetBookWordList(bookName);
    }
}

void ServerDataDownloader::downloadFile(QString fileName)
{
    m_svrAgt.downloadFile(fileName);
}

const QMap<QString, SvrAgt::DownloadStatus> & ServerDataDownloader::downloadMultipleFiles(QSet<QString> files)
{
    return m_svrAgt.downloadMultipleFiles(files);
}

void ServerDataDownloader::cancelDownloading()
{
    m_svrAgt.cancelDownloading();
    m_mapWords.clear();
}

void ServerDataDownloader::disconnectServer()
{
    m_svrAgt.disconnectServer();
}

/**
 * @brief ServerDataDownloader::downloadAllBooks
 * downloadAllBooks()
 * * does not download the list of words in the book
 * * let alone the words
 */
void ServerDataDownloader::downloadAllBooks()
{
    auto books = m_mapBooks.keys();
    for (int i = 0;i < books.size();i ++)
    {
        m_svrAgt.sendRequestGetABook(books.at(i));
    }
}

void ServerDataDownloader::downloadWordsOfBook(QString bookName)
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

    m_svrAgt.downloadWords(wordsToDownload);
    m_svrAgt.sendRequestGetWordsOfBookFinished(bookName);
}

void ServerDataDownloader::saveFileFromServer(QString fileName, const QByteArray &fileContent)
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
