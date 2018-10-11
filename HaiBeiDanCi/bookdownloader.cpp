#include "bookdownloader.h"
#include "clienttoken.h"
#include "mediafilemanager.h"

BookDownloader::BookDownloader(ServerCommunicator *sc, QObject *parent) :
    QObject(parent),
    m_sc(sc),
    m_showProgress(false),
    m_pd(nullptr),
    m_book(WordBook::invalidBook)
{
    if (m_sc == nullptr)
    {
        m_sc = ServerCommunicator::instance();
    }

    connect(m_sc, SIGNAL(bookListDownloaded(const QList<QString> &)), this, SLOT(onBookListDownloaded(const QList<QString> &)));
    connect(m_sc, SIGNAL(bookDownloaded(const WordBook &)), this, SLOT(onBookDownloaded(const WordBook &)));
    connect(m_sc, SIGNAL(bookWordListReceived(QString, const QVector<QString> &)), this, SLOT(onBookWordListReceived(QString, const QVector<QString> &)));
    connect(m_sc, SIGNAL(wordDownloaded(const Word &)), this, SLOT(onWordDownloaded(const Word &)));
}

BookDownloader::~BookDownloader()
{
    setShowProgress(false);
    qDebug() << "BookDownloader::~BookDownloader()";
}

void BookDownloader::setShowProgress(bool showProgress, QString labelText, QString cancelButtonText, QWidget *parent)
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

bool BookDownloader::downloadBookList()
{
    if (downloadOngoing() == true)
    {
        return false;
    }

    // DOWNLOAD BOOK LIST
    // STEP 1: download the book
    Q_ASSERT(m_sc != nullptr);
    m_sc->sendRequestGetAllBooks();

    return true;
}

bool BookDownloader::downloadBook(QString bookName)
{
    auto ct = ClientToken::instance();
    if (ct->hasValidUser() == false || ct->hasAliveToken() == false)
    {
        return false;
    }

    // only download a book when it does NOT exist locally
    if (WordBook::getBook(bookName).get() != nullptr)
    {
        return false;
    }

    if (downloadOngoing() == true)
    {
        return false;
    }

    // save the bookName in m_book
    m_book.setName(bookName);

    // DOWNLOAD A BOOK
    // STEP 1: download the book
    Q_ASSERT(m_sc != nullptr);
    m_sc->sendRequestGetABook(bookName);

    // set it to progress 0, so the progress dialogs shows ASAP?
    if (m_pd != nullptr)
    {
        m_pd->setValue(0);
    }

    return true;
}

void BookDownloader::onBookListDownloaded(const QList<QString> &books)
{
    emit(bookListDownloaded(books));
}

void BookDownloader::onBookDownloaded(const WordBook &book)
{
    qDebug() << book.getName() << "downloaded";
    Q_ASSERT(book.getName() == m_book.getName());

    // save the book
    m_book = book;

    // DOWNLOAD A BOOK
    // STEP 2: get the list of words in the book
    Q_ASSERT(m_sc != nullptr);
    m_sc->sendRequestGetBookWordList(m_book.getName());
}

void BookDownloader::onBookWordListReceived(QString bookName, const QVector<QString> &wordList)
{
    Q_ASSERT(bookName == m_book.getName());
    m_bookWords = wordList;

    // DOWNLOAD A BOOK
    // STEP 3: received the words of the book, get the definition of these words
    downloadWordsOfBook();
}

void BookDownloader::downloadWordsOfBook()
{
    m_downloaded = 0;
    m_toDownload = m_bookWords.size();
    int requestsForARound = MySettings::numberOfRequestInEveryDownloadRound();
    QMap<QString, ServerCommunicator::DownloadStatus> wordsToDownload;
    for (int i = 0;i < m_bookWords.size();i ++)
    {
        auto spelling = m_bookWords.at(i);
        // get the list of words which are not available locally
        if (Word::getWordToRead(spelling) == nullptr && wordsToDownload.contains(spelling) == false)
        {
            wordsToDownload.insert(spelling, ServerCommunicator::WaitingDataFromServer);  // mark it as request has been sent
            Q_ASSERT(m_sc != nullptr);
            m_sc->downloadWord(spelling);
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

    // is it so that all words are already available locally?
    if (m_downloaded == m_toDownload)
    {
        // yes, store the book
        saveBookFromServer();
    }
}

void BookDownloader::onWordDownloaded(const Word &word)
{
    sptr<Word> newWord = new Word(word);
    m_mapWords.insert(newWord->getSpelling(), newWord);
    qDebug() << "new word stored:" << newWord->getSpelling();

    m_downloaded ++;

    // update the progress
    if (m_pd != nullptr)
    {
        int newPercent = 100 * m_downloaded / m_toDownload;
        m_pd->setValue(newPercent);
        qDebug() << "newPercent:" << newPercent;
    }

    // check if all words are downloaded
    if (m_downloaded == m_toDownload)
    {
        // yes, store the book
        saveBookFromServer();
    }
}

void BookDownloader::saveBookFromServer()
{
    // the book is downloaded, save it to database
#ifndef QT_NO_DEBUG
    QElapsedTimer t;
    t.start();
#endif
    // store the words
    Word::v2StoreMultipleWordFromServer(m_mapWords);

    // store the book
    sptr<WordBook> newBook = new WordBook(m_book);
    WordBook::storeBookFromServer(newBook, m_bookWords);

    // update the media file manager that a new book is downloaded
    auto mfm = MediaFileManager::instance();
    mfm->bookDownloaded(newBook->getName());

#ifndef QT_NO_DEBUG
    qDebug() << "Used" << t.elapsed() << "ms in OnGetWordsOfBookFinished()";
#endif

    // DOWNLOAD A BOOK
    // COMPLETED;
    finishDownloading(ServerCommunicator::DownloadSucceeded);
}

bool BookDownloader::downloadOngoing()
{
    return (m_book != WordBook::invalidBook)
            || (m_mapWords.isEmpty() == false);
}

void BookDownloader::onCanceled()
{
    qDebug() << "downloading is cancelled";
    Q_ASSERT(m_sc != nullptr);
    m_sc->cancelDownloading();
    finishDownloading(ServerCommunicator::DownloadCancelled);
}

void BookDownloader::finishDownloading(ServerCommunicator::DownloadStatus result)
{
    emit(downloadFinished(m_book.getName(), result));
    m_book = WordBook::invalidBook;
    m_mapWords.clear();
}
