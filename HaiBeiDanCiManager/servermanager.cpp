#include "servermanager.h"
#include "../HaiBeiDanCi/mysettings.h"
#include "../HaiBeiDanCi/serverdatadownloader.h"
#include "../HaiBeiDanCi/clienttoken.h"

ServerManager::ServerManager(QObject *parent) : QObject(parent),
    m_mgrAgt(MySettings::serverHostName(), MySettings::serverPort(), this),
    m_serverDataLoaded(false)
{
    connect(&m_mgrAgt, SIGNAL(bookListReady(const QList<QString>)), this, SLOT(OnBookListReady(const QList<QString>)));
    connect(&m_mgrAgt, SIGNAL(bookDownloaded(sptr<WordBook>)), this, SLOT(OnBookDownloaded(sptr<WordBook>)));
    //connect(&m_mgrAgt, SIGNAL(wordDownloaded(sptr<Word>)), this, SLOT(OnWordDownloaded(sptr<Word>)));
    //connect(&m_mgrAgt, SIGNAL(downloadProgress(float)), this, SLOT(OnDownloadProgress(float)));
    //connect(&m_mgrAgt, SIGNAL(fileDownloaded(QString, SvrAgt::DownloadStatus, const QByteArray &)), this, SLOT(OnFileDownloaded(QString, SvrAgt::DownloadStatus, QByteArray)));
    //connect(&m_mgrAgt, SIGNAL(getWordsOfBookFinished(QString)), this, SLOT(OnGetWordsOfBookFinished(QString)));
    connect(&m_mgrAgt, SIGNAL(bookWordListReceived(QString, const QVector<QString> &)), this, SLOT(OnBookWordListReceived(QString, const QVector<QString> &)));
    connect(&m_mgrAgt, SIGNAL(serverConnected()), this, SLOT(onServerConnected()));

    connect(&m_mgrAgt, SIGNAL(getServerDataFinished()), this, SLOT(onGetServerDataFinished()));
    connect(&m_mgrAgt, SIGNAL(getAllWordsWithoutDefinitionFinished(const QVector<QString> &, const QVector<int> &, const QVector<int> &)), this, SLOT(onGetAllWordsWithoutDefinitionFinished(const QVector<QString> &, const QVector<int> &, const QVector<int> &)));
    connect(&m_mgrAgt, SIGNAL(bookDeleted(QString)), this, SLOT(onBookDeleted(QString)));
    connect(&m_mgrAgt, SIGNAL(bookUploaded(QString)), this, SLOT(onBookUploaded(QString)));
    connect(&m_mgrAgt, SIGNAL(gotMissingMediaFilesOfBook(QString, const QList<QString> &)), this, SLOT(onGotMissingMediaFilesOfBook(QString, const QList<QString> &)));
    connect(&m_mgrAgt, SIGNAL(fileUploaded(QString)), this, SLOT(onFileUploaded(QString)));
    connect(&m_mgrAgt, SIGNAL(fileUploadingProgress(QString, uint, uint)), this, SLOT(onFileUploadingProgress(QString, uint, uint)));
    connect(&m_mgrAgt, SIGNAL(wordUploaded(QString)), this, SLOT(onWordUploaded(QString)));
    connect(&m_mgrAgt, SIGNAL(appReleased(bool)), this, SLOT(onAppReleased(bool)));
    connect(&m_mgrAgt, SIGNAL(upgraderReleased(bool)), this, SLOT(onUpgraderReleased(bool)));


    //m_mgrAgt.connectToServer();
}

ServerManager::~ServerManager()
{
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
    // * missing media files of each book
    for (int i = 0;i < books.size();i ++)
    {
        downloadBookDetails(books.at(i));
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

void ServerManager::OnBookWordListReceived(QString bookName, const QVector<QString> &wordList)
{
    m_mapBooksWordList.insert(bookName, wordList);
}

void ServerManager::downloadBookDetails(QString bookName)
{
    m_mgrAgt.sendRequestGetABook(bookName);
    m_mgrAgt.sendRequestGetBookWordList(bookName);
    m_mgrAgt.sendRequestMissingMediaFiles(bookName);
}

QList<QString> ServerManager::getBookList()
{
    return m_mapBooks.keys();
}

void ServerManager::onServerConnected()
{
    qDebug() << "server connected";

    m_mgrAgt.sendRequestPromoteToManager();
    reloadServerData();
}

void ServerManager::onGetServerDataFinished()
{
    // RELOAD SERVER DATA
    // we got all the data when we get this message
    m_serverDataLoaded = true;
    emit(serverDataReloaded());
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
        auto definition = QString::number(defLen);
        sptr<Word> newWord = new Word(spelling, definition, id);
        m_mapWords.insert(spelling, newWord);
    }
}

void ServerManager::onBookDeleted(QString bookName)
{
    // don't assert here as it's possible when delete requests are sent to server multiple times
    //Q_ASSERT(m_mapBooks.contains(bookName) == true);
    if (bookExistsInServer(bookName) == true)
    {
        m_mapBooks.remove(bookName);
        Q_ASSERT(m_mapBooksWordList.contains(bookName) == true);
        m_mapBooksWordList.remove(bookName);
    }
    else
    {
        // do nothing if the book is already deleted
    }

    emit(serverDataReloaded());
}

void ServerManager::onBookUploaded(QString bookName)
{
    qDebug() << "successfully uploaded book:" << bookName;

    downloadBookDetails(bookName);
}

void ServerManager::onFileUploadingProgress(QString fileName, uint uploadedBytes, uint totalBytes)
{
    qDebug() << "onFileUploadingProgress():" << fileName;
    if (totalBytes != 0)
    {
        emit(uploadProgress((m_uploaded + uploadedBytes * 1.0f / totalBytes) / m_toUpload));
    }
}

void ServerManager::onFileUploaded(QString fileName)
{
    qDebug() << "successfully uploaded file:" << fileName;
    m_uploaded ++;
    if (m_toUpload != 0)
    {
        emit(uploadProgress(m_uploaded * 1.0f / m_toUpload));
    }

    emit(fileUploaded(fileName));

    /*
     * this way is to time consuming!!!
    auto books = m_mapBooks.keys();
    for (int i = 0;i < books.size();i ++)
    {
        auto bookName = books.at(i);
        auto missingFiles = m_mapBooksMissingFiles.value(bookName);
        missingFiles.removeOne(fileName);
        m_mapBooksMissingFiles.insert(bookName, missingFiles);
    }
    */

    if (m_uploaded == m_toUpload)
    {
        // get the updated missing file information of all books
        auto books = m_mapBooks.keys();
        for (int i = 0;i < books.size();i ++)
        {
            m_mgrAgt.sendRequestMissingMediaFiles(books.at(i));
        }
    }
}

void ServerManager::onWordUploaded(QString spelling)
{
    qDebug() << "successfully uploaded word:" << spelling;
    m_uploaded ++;
    if (m_toUpload != 0)
    {
        emit(uploadProgress(m_uploaded * 1.0f / m_toUpload));
    }

    // no need to reload, as it will be reloaded later when the book finish uploading onBookUploaded()
    //if (m_uploaded == m_toUpload)
    //{
    //    reloadServerData();
    //}
}

void ServerManager::onAppReleased(bool succeed)
{
    emit(appReleased(succeed));
}

void ServerManager::onUpgraderReleased(bool succeed)
{
    emit(upgraderReleased(succeed));
}

void ServerManager::onGotMissingMediaFilesOfBook(QString bookName, const QList<QString> &missingFiles)
{
    m_mapBooksMissingFiles.insert(bookName, missingFiles);
    emit(serverDataReloaded());
}

/**
 * @brief ServerManager::reloadServerData
 * send messages to the server to get the data:
 *  1. words (only spelling and id)
 *  2. books (id, name, intro, wordlist)
 */
void ServerManager::reloadServerData()
{
    auto ct = ClientToken::instance();
    if (ct->hasAliveToken() == true
            && ct->hasValidUser() == true)
    {
        clearData();

        // RELOAD SERVER DATA
        // STEP 1: at this moment, we can get
        // * all the words without definition (as definition is big)
        // * the list of books
        m_mgrAgt.sendRequestGetAllWordsWithoutDefinition();
        m_mgrAgt.sendRequestGetAllBooks();
    }
}


void ServerManager::clearData()
{
    m_mapBooks.clear();
    m_mapBooksWordList.clear();
    m_mapWords.clear();
}

bool ServerManager::okToSync(QString *errorString)
{
    auto ct = ClientToken::instance();
    if (ct->hasAliveToken() == false
            || ct->hasValidUser() == false)
    {
        if (errorString != nullptr)
        {
            *errorString = QString("no user has logged in.");
        }
        return false;
    }

    if (okToSyncBooks(errorString) == false)
    {
        return false;
    }

    if (okToSyncWords(errorString) == false)
    {
        return false;
    }

    return true;
}

bool ServerManager::okToSyncBooks(QString *errorString)
{
    if (m_serverDataLoaded == false)
    {
        return false;
    }

    bool ok = true;
    auto allServerBooks = m_mapBooks.keys();
    auto allLocalBooks = WordBook::getAllBooks();

    // get the IDs used locally
    QSet<int> idsUsedLoally;
    for (int i = 0;i < allLocalBooks.size();i ++)
    {
        auto bookName = allLocalBooks.at(i);
        auto localBook = WordBook::getBook(bookName);
        Q_ASSERT(localBook.get() != nullptr);
        Q_ASSERT(idsUsedLoally.contains(localBook->getId()) == false);
        idsUsedLoally.insert(localBook->getId());
    }

    // get the IDs used in server
    QSet<int> idsUsedInServer;
    for (int i = 0;i < allServerBooks.size();i ++)
    {
        auto bookName = allServerBooks.at(i);
        auto serverBook = m_mapBooks.value(bookName);
        Q_ASSERT(serverBook.get() != nullptr);
        Q_ASSERT(idsUsedInServer.contains(serverBook->getId()) == false);
        idsUsedInServer.insert(serverBook->getId());
    }

    for (int i = 0;i < allLocalBooks.size();i ++)
    {
        auto bookName = allLocalBooks.at(i);

        auto localBook = WordBook::getBook(bookName);
        Q_ASSERT(localBook.get() != nullptr);
        if (m_mapBooks.contains(bookName) == true)
        {
            auto serverBook = m_mapBooks.value(bookName);
            Q_ASSERT(serverBook.get() != nullptr);
            if (localBook->getId() != serverBook->getId())
            {
                ok = false;
                if (errorString != nullptr)
                {
                    *errorString = QString("book \"%1\" has id %2 locally and id %3 in server.")
                            .arg(bookName)
                            .arg(localBook->getId())
                            .arg(serverBook->getId());
                }
                break;
            }
            else
            {
                allServerBooks.removeOne(bookName);
            }
        }
        else
        {
            if (idsUsedInServer.contains(localBook->getId()) == true)
            {
                ok = false;
                if (errorString != nullptr)
                {
                    *errorString = QString("local book \"%1\" has id %2, the book does not exist in server but the id is used by another server book.")
                            .arg(bookName)
                            .arg(localBook->getId());
                }
                break;
            }
        }
    }

    // check if there're words from server that are not checked yet
    if (ok == true && allServerBooks.size() > 0)
    {
        for (int i = 0;i < allServerBooks.size();i ++)
        {
            auto bookName = allServerBooks.at(i);
            auto serverBook = m_mapBooks.value(bookName);
            Q_ASSERT(serverBook.get() != nullptr);
            if (idsUsedLoally.contains(serverBook->getId()) == true)
            {
                ok = false;
                if (errorString != nullptr)
                {
                    *errorString = QString("server book \"%1\" has id %2, the book does not exist locally but the id is used by another local book.")
                            .arg(bookName)
                            .arg(serverBook->getId());
                }
                break;
            }
        }
    }

    return ok;
}

bool ServerManager::okToSyncWords(QString *errorString)
{
    bool ok = true;
    auto allServerWords = m_mapWords.keys();
    auto allLocalWords = Word::getAllWords();

    // get the IDs used locally
    QSet<int> idsUsedLoally;
    for (int i = 0;i < allLocalWords.size();i ++)
    {
        auto spelling = allLocalWords.at(i);
        auto localWord = Word::getWordToRead(spelling);
        Q_ASSERT(localWord != nullptr);
        Q_ASSERT(idsUsedLoally.contains(localWord->getId()) == false);
        idsUsedLoally.insert(localWord->getId());
    }

    // get the IDs used in server
    QSet<int> idsUsedInServer;
    for (int i = 0;i < allServerWords.size();i ++)
    {
        auto spelling = allServerWords.at(i);
        auto serverWord = m_mapWords.value(spelling);
        Q_ASSERT(serverWord.get() != nullptr);
        Q_ASSERT(idsUsedInServer.contains(serverWord->getId()) == false);
        idsUsedInServer.insert(serverWord->getId());
    }

    for (int i = 0;i < allLocalWords.size();i ++)
    {
        auto spelling = allLocalWords.at(i);
        auto localWord = Word::getWordToRead(spelling);
        Q_ASSERT(localWord != nullptr);
        if (m_mapWords.contains(spelling) == true)
        {
            auto serverWord = m_mapWords.value(spelling);
            Q_ASSERT(serverWord.get() != nullptr);
            if (localWord->getId() != serverWord->getId()
                    || localWord->getDefinition().size() != serverWord->getDefinition().toInt())
            {
                ok = false;
                if (errorString != nullptr)
                {
                    *errorString = QString("word \"%1\" has id %2 locally and id %3 in server.")
                            .arg(spelling)
                            .arg(localWord->getId())
                            .arg(serverWord->getId());
                }
                break;
            }
            else
            {
                allServerWords.removeOne(spelling);
            }
        }
        else
        {
            if (idsUsedInServer.contains(localWord->getId()) == true)
            {
                ok = false;
                if (errorString != nullptr)
                {
                    *errorString = QString("local word \"%1\" has id %2, the word does not exist in server but the id is used by another server word.")
                            .arg(spelling)
                            .arg(localWord->getId());
                }
                break;
            }
        }
    }

    // check if there're words from server that are not checked yet
    if (ok == true && allServerWords.size() > 0)
    {
        for (int i = 0;i < allServerWords.size();i ++)
        {
            auto spelling = allServerWords.at(i);
            auto serverWord = m_mapWords.value(spelling);
            Q_ASSERT(serverWord.get() != nullptr);
            if (idsUsedLoally.contains(serverWord->getId()) == true)
            {
                ok = false;
                if (errorString != nullptr)
                {
                    *errorString = QString("server word \"%1\" has id %2, the word does not exist locally but the id is used by another local word.")
                            .arg(spelling)
                            .arg(serverWord->getId());
                }
                break;
            }
        }
    }

    return ok;
}

void ServerManager::downloadBook(QString bookName)
{
    Q_ASSERT(okToSync() == true);

    auto localBook = WordBook::getBook(bookName);
    if (localBook.get() != nullptr)
    {
        // the book already exists locally
        return;
    }

    auto sdd = new ServerDataDownloader(this);
    connect(sdd, &ServerDataDownloader::bookStored, [sdd, this] (QString bookName) {
        emit(bookDownloaded(bookName));
        sdd->deleteLater();
    });
    sdd->downloadBook(bookName);
}

void ServerManager::deleteBook(QString bookName)
{
    if (bookExistsInServer(bookName) == true)
    {
        // only delete the book if it exists in the server
        m_mgrAgt.sendRequestDeleteABook(bookName);
    }
}

QVector<QString> ServerManager::getWordListOfBook(QString bookName)
{
    return m_mapBooksWordList.value(bookName);
}

QList<QString> ServerManager::getMissingMediaFilesOfBook(QString bookName)
{
    return m_mapBooksMissingFiles.value(bookName);
}

void ServerManager::uploadBook(QString bookName)
{
    Q_ASSERT(okToSync() == true);

    auto serverBook = m_mapBooks.value(bookName);
    if (serverBook.get() != nullptr)
    {
        // the book already exists in the server
        return;
    }

    // send the book
    // send the list of words of the book
    // send the words which do not exist in the server
    // send upload complete
    auto localBook = WordBook::getBook(bookName);
    Q_ASSERT(localBook.get() != nullptr);
    m_mgrAgt.sendResponseGetABook(*localBook);
    sendBookWordList(bookName);
    sendWordsOfBook(bookName);
}

void ServerManager::sendBookWordList(QString bookName)
{
    auto book = WordBook::getBook(bookName);
    Q_ASSERT(book.get() != nullptr);
    auto wordList = book->getAllWords();
    m_mgrAgt.sendBookWordList(bookName, wordList);
}

void ServerManager::sendWordsOfBook(QString bookName)
{
    auto book = WordBook::getBook(bookName);
    Q_ASSERT(book.get() != nullptr);
    auto wordList = book->getAllWords();
    m_toUpload = wordList.size();
    m_uploaded = 0;
    for (int i = 0;i < wordList.size();i ++)
    {
        auto spelling = wordList.at(i);
        if (m_mapWords.contains(spelling) == false)
        {
            // send the word only if it does not exit in the server
            auto word = Word::getWordToRead(spelling);
            Q_ASSERT(word != nullptr);
            m_mgrAgt.sendResponseGetAWord(*word);
            QCoreApplication::processEvents();
        }
        else
        {
            // no need to upload this word, so mark it as uploaded for progress calculation
            onWordUploaded(spelling);
        }
    }
    m_mgrAgt.sendResponseGetWordsOfBookFinished(bookName);
}

bool ServerManager::sendFile(QString fileName)
{
    // check if the file is OK to send, we cannot expose everything on the sever!!!
    if (okToSendFile(fileName) != true)
    {
        qCritical() << "cannot send file" << fileName << "because it violates the security policy!";
        return false;
    }

    QString localFile = MySettings::dataDirectory() + "/" + fileName;
    qDebug() << "send file" << localFile;

    QFile toSend(localFile);
    if (toSend.open(QIODevice::ReadOnly | QIODevice::ExistingOnly) == false)
    {
        qCritical() << "cannot open file" << fileName << "because" << toSend.errorString();
        return false;
    }

    const int totalBytes = static_cast<int>(toSend.size());
    int sentBytes = 0;
    bool succeeded = true;
    QDataStream fileDS(&toSend);
    char buf[ServerClientProtocol::MaximumBytesForFileTransfer + 1];
    while (sentBytes < totalBytes)
    {
        auto readBytes = fileDS.readRawData(buf, ServerClientProtocol::MaximumBytesForFileTransfer);
        if (readBytes == -1)
        {
            succeeded = false;
            break;
        }

        sentBytes += readBytes;
        m_mgrAgt.sendRequestUploadAFile(fileName, buf, static_cast<uint>(readBytes), sentBytes, totalBytes);
        qDebug() << "send" << readBytes << "bytes of total" << totalBytes;
    }

    return succeeded;
}

bool ServerManager::okToSendFile(QString fileName)
{
    // only allow files in folder media and releases
    if (fileName.startsWith("media", Qt::CaseInsensitive) == false
            && fileName.startsWith("releases", Qt::CaseInsensitive) == false)
    {
        return false;
    }

    // only allow mp3/png/jpg/css/js
    QString ext = fileName.section('.', -1);
    if (ext.compare("mp3", Qt::CaseInsensitive) != 0
            && ext.compare("png", Qt::CaseInsensitive) != 0
            && ext.compare("jpg", Qt::CaseInsensitive) != 0
            && ext.compare("css", Qt::CaseInsensitive) != 0
            && ext.compare("js", Qt::CaseInsensitive) != 0
            && ext.compare("zip", Qt::CaseInsensitive) != 0
            && ext.compare("7z", Qt::CaseInsensitive) != 0)
    {
        return false;
    }

    // don't allow to move upper level folder
    if (fileName.contains("..") == true)
    {
        return false;
    }

    return true;
}

void ServerManager::uploadfile(QString fileName)
{
    m_toUpload = 1;
    m_uploaded = 0;
    bool succeeded = sendFile(fileName);
    m_mgrAgt.sendRequestUploadAFileFinished(fileName, succeeded);
}

void ServerManager::uploadBookMissingMediaFiles(QString bookName)
{
    auto bookMissingMediaFiles = m_mapBooksMissingFiles.value(bookName);
    m_toUpload = bookMissingMediaFiles.size();
    m_uploaded = 0;
    for (int i = 0;i < bookMissingMediaFiles.size();i ++)
    {
        auto fileName = bookMissingMediaFiles.at(i);
        bool succeeded = sendFile(fileName);
        m_mgrAgt.sendRequestUploadAFileFinished(fileName, succeeded);
        QCoreApplication::processEvents();
    }
}

bool ServerManager::bookExistsInServer(QString bookName)
{
    Q_ASSERT(m_serverDataLoaded == true);
    return m_mapBooks.value(bookName).get() != nullptr;
}

void ServerManager::releaseApp(ApplicationVersion version, QString platform, QString fileName, QString info)
{
    m_mgrAgt.sendRequestReleaseApp(version, platform, fileName, info);
}

void ServerManager::releaseUpgrader(ApplicationVersion version, QString platform, QString fileName)
{
    m_mgrAgt.sendRequestReleaseUpgrader(version, platform, fileName);
}
