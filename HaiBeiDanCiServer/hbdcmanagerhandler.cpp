#include "hbdcmanagerhandler.h"
#include "../HaiBeiDanCi/mediafilemanager.h"
#include "../HaiBeiDanCi/mysettings.h"
#include "appreleaser.h"
#include "upgraderreleaser.h"

#include <QDir>
#include <QFile>

HBDCManagerHandler::HBDCManagerHandler(ClientWaiter &clientWaiter) : HBDCAppHandler(clientWaiter)
{
}

int HBDCManagerHandler::handleMessage(const QByteArray &msg)
{
    MessageHeader receivedMsgHeader(msg);
    bool handleResult = false;
    switch (receivedMsgHeader.code()) {
    case ServerClientProtocol::RequestGetAllWordsWithoutDefinition:
        handleResult = handleRequestGetAllWordsWithoutDefinition(msg);
        break;

    case ServerClientProtocol::RequestGetServerDataFinished:
        handleResult = handleRequestGetServerDataFinished(msg);
        break;

    case ServerClientProtocol::ResponseGetABook:
        handleResult = handleResponseGetABook(msg);
        break;

    case ServerClientProtocol::ResponseGetBookWordList:
        handleResult = handleResponseGetBookWordList(msg);
        break;

    case ServerClientProtocol::ResponseGetAWord:
        handleResult = handleResponseGetAWord(msg);
        break;

    case ServerClientProtocol::ResponseGetWordsOfBookFinished:
        handleResult = handleResponseGetWordsOfBookFinished(msg);
        break;

    case ServerClientProtocol::RequestDeleteABook:
        handleResult = handleRequestDeleteABook(msg);
        break;

    case ServerClientProtocol::RequestMissingMediaFiles:
        handleResult = handleRequestMissingMediaFiles(msg);
        break;

    case ServerClientProtocol::RequestUploadAFile:
        handleResult = handleRequestUploadAFile(msg);
        break;

    case ServerClientProtocol::RequestUploadAFileFinished:
        handleResult = handleRequestUploadAFileFinished(msg);
        break;

    case ServerClientProtocol::RequestReleaseApp:
        handleResult = handleRequestReleaseApp(msg);
        break;

    case ServerClientProtocol::RequestReleaseUpgrader:
        handleResult = handleRequestReleaseUpgrader(msg);
        break;

    default:
        return HBDCAppHandler::handleMessage(msg);

    }

    if (handleResult == true)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}


bool HBDCManagerHandler::handleRequestGetAllWordsWithoutDefinition(const QByteArray &msg)
{
    sendAllWordsWithoutDefinition(msg);
    return true;
}

bool HBDCManagerHandler::handleRequestGetServerDataFinished(const QByteArray &msg)
{
    sendResponseGetServerDataFinished(msg);

    return true;
}

void HBDCManagerHandler::sendResponseGetServerDataFinished(const QByteArray &msg)
{
    sendSimpleMessage(msg, ServerClientProtocol::ResponseGetServerDataFinished);
}

void HBDCManagerHandler::sendResponseUploadAWord(const QByteArray &msg, QString spelling)
{
    MessageHeader receivedMsgHeader(msg);
    MessageHeader responseHeader(ServerClientProtocol::ResponseUploadAWord, receivedMsgHeader.sequenceNumber());

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << responseHeader << spelling;
    sendMessage(block);
}

void HBDCManagerHandler::sendAllWordsWithoutDefinition(const QByteArray &msg)
{
    auto wordList = Word::getAllWords();
    int total = wordList.size();
    int pos = 0;
    while (pos < total)
    {
        auto subList = wordList.mid(pos, ServerClientProtocol::MaximumWordsInAMessage);
        pos += subList.size();
        sendAListOfWordsWithoutDefinition(msg, subList, pos>=total);
    }
}

void HBDCManagerHandler::sendAListOfWordsWithoutDefinition(const QByteArray &msg, const QList<QString> &wordList, bool listComplete)
{
    QVector<QString> spellingList;
    QVector<int> idList;
    QVector<int> definitionLengthList;

    for (int i = 0;i < wordList.size();i ++)
    {
        auto word = Word::getWordToRead(wordList.at(i));
        Q_ASSERT(word != nullptr);
        spellingList.append(word->getSpelling());
        idList.append(word->getId());
        definitionLengthList.append(word->getDefinition().size());
    }

    sendResponseGetAllWordsWithoutDefinition(msg, spellingList, idList, definitionLengthList, listComplete);
}

void HBDCManagerHandler::sendResponseGetAllWordsWithoutDefinition(const QByteArray &msg, const QVector<QString> &spellings, const QVector<int> &ids, const QVector<int> &definitionLengths, bool listComplete)
{
    MessageHeader receivedMsgHeader(msg);
    MessageHeader responseHeader(ServerClientProtocol::ResponseGetAllWordsWithoutDefinition, receivedMsgHeader.sequenceNumber());

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << responseHeader << spellings << ids << definitionLengths << listComplete;
    sendMessage(block, true);
}

bool HBDCManagerHandler::handleResponseGetABook(const QByteArray &msg)
{
    QDataStream in(msg);
    MessageHeader receivedMsgHeader(-1, -1, -1);
    WordBook book;
    in.startTransaction();
    in >> receivedMsgHeader >> book;
    if (in.commitTransaction() == false)
    {
        qCritical() << "failed to read words of the book in handleResponseGetABook()";
        return false;
    }

    // store the book temporary in m_mapBooks, it will be saved to local database in completeBookDownload()
    sptr<WordBook> newBook = new WordBook(book);
    m_mapBooks.insert(book.getName(), newBook);

    sendResponseOK(msg);

    return true;
}

bool HBDCManagerHandler::handleResponseGetBookWordList(const QByteArray &msg)
{
    QDataStream in(msg);
    MessageHeader receivedMsgHeader(-1, -1, -1);
    QString bookName;
    QVector<QString> wordList;
    bool listComplete;
    in.startTransaction();
    in >> receivedMsgHeader >> bookName >> wordList >> listComplete;
    if (in.commitTransaction() == false)
    {
        qCritical() << "failed to read words of the book in handleResponseGetBookWordList()";
        return false;
    }

    auto currentList = m_mapBooksWordList.value(bookName);
    auto newList = currentList + wordList;
    m_mapBooksWordList.insert(bookName, newList);

    if (listComplete == true)
    {
        // we've got the full list, but nothing need to be done here
    }

    sendResponseOK(msg);

    return true;
}

bool HBDCManagerHandler::handleResponseGetAWord(const QByteArray &msg)
{
    QDataStream in(msg);
    MessageHeader receivedMsgHeader(-1, -1, -1);
    Word word;
    in.startTransaction();
    in >> receivedMsgHeader >> word;
    if (in.commitTransaction() == false)
    {
        qCritical() << "failed to read words of the book in handleResponseGetAWord()";
        return false;
    }

    // store the word
    sptr<Word> newWord = new Word(word);
    m_mapWords.insert(word.getSpelling(), newWord);

    sendResponseUploadAWord(msg, word.getSpelling());

    return true;
}

bool HBDCManagerHandler::handleResponseGetWordsOfBookFinished(const QByteArray &msg)
{
    QDataStream in(msg);
    MessageHeader receivedMsgHeader(-1, -1, -1);
    QString bookName;
    in.startTransaction();
    in >> receivedMsgHeader >> bookName;
    if (in.commitTransaction() == false)
    {
        qCritical() << "failed to read the book name in handleResponseGetWordsOfBookFinished()";
        return false;
    }

    // store the new received words
    Word::v2StoreMultipleWordFromServer(m_mapWords);

    // store the book
    auto book = m_mapBooks.value(bookName);
    auto serverBook = WordBook::getBook(bookName);
    if (book.get() != nullptr && serverBook.get() == nullptr)
    {
        // only store the book if we have the book information received earlier
        // AND there's no such book in the server database
        auto wordList = m_mapBooksWordList.value(bookName);
        WordBook::storeBookFromServer(book, wordList);
    }

    // update the media file manager
    auto mfm = MediaFileManager::instance();
    mfm->bookDownloaded(bookName);

    // clear the data
    m_mapBooks.clear();
    m_mapWords.clear();
    m_mapBooksWordList.clear();

    // how to update the server?! Not needed, it's already updated!

    sendResponseUploadABook(msg, bookName);

    return true;
}

bool HBDCManagerHandler::handleRequestDeleteABook(const QByteArray &msg)
{
    QDataStream in(msg);
    MessageHeader receivedMsgHeader(-1, -1, -1);
    QString bookName;
    in.startTransaction();
    in >> receivedMsgHeader >> bookName;
    if (in.commitTransaction() == false)
    {
        qCritical() << "failed to read the book name in handleRequestDeleteABook()";
        return false;
    }

    WordBook::deleteBook(bookName);
    MediaFileManager::instance()->bookDeleted(bookName);

    // how to update the server?! Not needed, it's already updated!

    sendResponseDeleteABook(msg, bookName);

    return true;
}

bool HBDCManagerHandler::handleRequestMissingMediaFiles(const QByteArray &msg)
{
    QDataStream in(msg);
    MessageHeader receivedMsgHeader(-1, -1, -1);
    QString bookName;
    in.startTransaction();
    in >> receivedMsgHeader >> bookName;
    if (in.commitTransaction() == false)
    {
        qCritical() << "failed to read the book name in handleRequestDeleteABook()";
        return false;
    }

    sendBookMissingMediaFiles(msg, bookName);

    return true;
}

void HBDCManagerHandler::sendResponseUploadABook(const QByteArray &msg, QString bookName)
{
    MessageHeader receivedMsgHeader(msg);
    MessageHeader responseHeader(ServerClientProtocol::ResponseUploadABook, receivedMsgHeader.sequenceNumber());

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << responseHeader << bookName;
    sendMessage(block);
}


void HBDCManagerHandler::sendResponseDeleteABook(const QByteArray &msg, QString bookName)
{
    MessageHeader receivedMsgHeader(msg);
    MessageHeader responseHeader(ServerClientProtocol::ResponseDeleteABook, receivedMsgHeader.sequenceNumber());

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << responseHeader << bookName;
    sendMessage(block);
}

void HBDCManagerHandler::sendBookMissingMediaFiles(const QByteArray &msg, QString bookName)
{
    auto mfm = MediaFileManager::instance();
    QSet<QString> missingMediaFiles;
    auto missingPronounceAudioFiles = mfm->bookMissingPronounceAudioFiles(bookName);
    if (missingPronounceAudioFiles.get() != nullptr)
    {
        missingMediaFiles += *missingPronounceAudioFiles;
    }
    auto missingExampleAudioFiles = mfm->bookMissingExampleAudioFiles(bookName);
    if (missingExampleAudioFiles.get() != nullptr)
    {
        missingMediaFiles += *missingExampleAudioFiles;
    }
    auto fileList = missingMediaFiles.toList();
    sendResponseMissingMediaFiles(msg, bookName, fileList);
}

void HBDCManagerHandler::sendResponseMissingMediaFiles(const QByteArray &msg, QString bookName, const QList<QString> &fileList)
{
    MessageHeader receivedMsgHeader(msg);
    MessageHeader responseHeader(ServerClientProtocol::ResponseMissingMediaFiles, receivedMsgHeader.sequenceNumber());

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << responseHeader << bookName << fileList;
    sendMessage(block);
}


bool HBDCManagerHandler::handleRequestUploadAFile(const QByteArray &msg)
{
    QDataStream in(msg);
    MessageHeader receivedMsgHeader(-1, -1, -1);
    QString fileName;
    char *data;
    uint len, sentBytes, totalBytes;
    in.startTransaction();
    in >> receivedMsgHeader >> fileName;
    in.readBytes(data, len);
    in >> sentBytes >> totalBytes;
    if (in.commitTransaction() == false)
    {
        qCritical() << "failed to read words of the book in handleRequestUploadAFile()";
        return false;
    }

    QMap<const char *, uint> newBlock;
    newBlock.insert(data, len);
    auto contentBlocks = m_mapFileContentBlocks.value(fileName);
    if (contentBlocks == nullptr)
    {
        contentBlocks = new QVector<QMap<const char *, uint>>;
        m_mapFileContentBlocks.insert(fileName, contentBlocks);
    }
    contentBlocks->append(newBlock);

    sendResponseUploadAFile(msg, fileName, sentBytes, totalBytes);

    return true;
}

void HBDCManagerHandler::sendResponseUploadAFile(const QByteArray &msg, QString fileName, uint receivedBytes, uint totalBytes)
{
    MessageHeader receivedMsgHeader(msg);
    MessageHeader responseHeader(ServerClientProtocol::ResponseUploadAFile, receivedMsgHeader.sequenceNumber());

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << responseHeader << fileName << receivedBytes << totalBytes;
    sendMessage(block);
}

bool HBDCManagerHandler::handleRequestUploadAFileFinished(const QByteArray &msg)
{
    QDataStream in(msg);
    MessageHeader receivedMsgHeader(-1, -1, -1);
    QString fileName;
    bool succeeded;
    in.startTransaction();
    in >> receivedMsgHeader >> fileName >> succeeded;
    if (in.commitTransaction() == false)
    {
        qCritical() << "failed to read books in handleResponseAllDataSentForRequestGetFile()";
        return false;
    }


    if (succeeded == true)
    {
        saveFileFromServer(fileName);
        auto mfm = MediaFileManager::instance();
        mfm->fileDownloaded(fileName);
    }
    else
    {
        qCritical() << fileName << "failed to upload in handleResponseAllDataSentForRequestGetFile()";
    }

    discardFileContent(fileName);

    sendResponseUploadAFileFinished(msg, fileName);

    return true;
}

void HBDCManagerHandler::discardFileContent(QString fileName)
{
    QVector<QMap<const char *, uint>> *fileContentBlocks = m_mapFileContentBlocks.value(fileName);
    if (fileContentBlocks != nullptr)
    {
        for (int i = 0;i < fileContentBlocks->size();i ++)
        {
            auto currentBlock = fileContentBlocks->at(i);
            // free the memory allocated in.readBytes() in handleRequestUploadAFile()
            delete [] currentBlock.firstKey();
        }

        // free the memory allocated in handleRequestUploadAFile()
        delete fileContentBlocks;
    }

    // clear the file in the map
    m_mapFileContentBlocks.remove(fileName);
}

void HBDCManagerHandler::sendResponseUploadAFileFinished(const QByteArray &msg, QString fileName)
{
    MessageHeader receivedMsgHeader(msg);
    MessageHeader responseHeader(ServerClientProtocol::ResponseUploadAFileFinished, receivedMsgHeader.sequenceNumber());

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << responseHeader << fileName;
    sendMessage(block);
}

void HBDCManagerHandler::saveFileFromServer(QString fileName)
{
    QVector<QMap<const char *, uint>> *fileContentBlocks = m_mapFileContentBlocks.value(fileName);

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

bool HBDCManagerHandler::handleRequestReleaseApp(const QByteArray &msg)
{
    QDataStream in(msg);
    MessageHeader receivedMsgHeader(-1, -1, -1);
    ApplicationVersion appVer(0, 0, 0);
    QString platform, fileName, info;
    in.startTransaction();
    in >> receivedMsgHeader >> appVer >> platform >> fileName >> info;
    if (in.commitTransaction() == false)
    {
        qCritical() << "failed to read words of the book in handleRequestReleaseApp()";
        return false;
    }

    AppReleaser *ar = AppReleaser::instance();
    bool succeed = ar->releaseNewVersion(appVer, platform, fileName, info);

    sendResponseReleaseApp(msg, succeed);

    return true;
}

void HBDCManagerHandler::sendResponseReleaseApp(const QByteArray &msg, bool succeed)
{
    MessageHeader receivedMsgHeader(msg);
    MessageHeader responseHeader(ServerClientProtocol::ResponseReleaseApp, receivedMsgHeader.sequenceNumber());

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << responseHeader << succeed;
    sendMessage(block);
}

bool HBDCManagerHandler::handleRequestReleaseUpgrader(const QByteArray &msg)
{
    QDataStream in(msg);
    MessageHeader receivedMsgHeader(-1, -1, -1);
    ApplicationVersion appVer(0, 0, 0);
    QString platform, fileName;
    in.startTransaction();
    in >> receivedMsgHeader >> appVer >> platform >> fileName;
    if (in.commitTransaction() == false)
    {
        qCritical() << "failed to read words of the book in handleRequestReleaseApp()";
        return false;
    }

    UpgraderReleaser *ur = UpgraderReleaser::instance();
    bool succeed = ur->releaseNewVersion(appVer, platform, fileName, "NOT USED");

    sendResponseReleaseUpgrader(msg, succeed);

    return true;
}

void HBDCManagerHandler::sendResponseReleaseUpgrader(const QByteArray &msg, bool succeed)
{
    MessageHeader receivedMsgHeader(msg);
    MessageHeader responseHeader(ServerClientProtocol::ResponseReleaseUpgrader, receivedMsgHeader.sequenceNumber());

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << responseHeader << succeed;
    sendMessage(block);
}
