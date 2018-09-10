#include "hbdcmanagerhandler.h"
#include "../HaiBeiDanCi/mediafilemanager.h"
#include "../HaiBeiDanCi/mysettings.h"

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

    case ServerClientProtocol::ResponseGetFile:
        handleResult = handleResponseGetFile(msg);
        break;

    case ServerClientProtocol::ResponseGetFileFinished:
        handleResult = handleResponseGetFileFinished(msg);
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
        auto word = Word::getWord(wordList.at(i));
        Q_ASSERT(word.get() != nullptr);
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

void HBDCManagerHandler::sendResponseUploadAFile(const QByteArray &msg, QString fileName)
{
    MessageHeader receivedMsgHeader(msg);
    MessageHeader responseHeader(ServerClientProtocol::ResponseUploadAFile, receivedMsgHeader.sequenceNumber());

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << responseHeader << fileName;
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
    sendMessage(block, true);
}


bool HBDCManagerHandler::handleResponseGetFile(const QByteArray &msg)
{
    QDataStream in(msg);
    MessageHeader receivedMsgHeader(-1, -1, -1);
    QString fileName;
    char *data;
    uint len;
    in.startTransaction();
    in >> receivedMsgHeader >> fileName;
    in.readBytes(data, len);
    if (in.commitTransaction() == false)
    {
        qCritical() << "failed to read words of the book in handleResponseGetFile()";
        return false;
    }

    auto currentContent = m_mapFileContent.value(fileName);
    auto newContent = currentContent + QByteArray(data, static_cast<int>(len));
    m_mapFileContent.insert(fileName, newContent);

    sendResponseOK(msg);

    return true;
}

bool HBDCManagerHandler::handleResponseGetFileFinished(const QByteArray &msg)
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
        saveFileFromServer(fileName, m_mapFileContent.value(fileName));
        auto mfm = MediaFileManager::instance();
        mfm->fileDownloaded(fileName);
    }
    else
    {
        qCritical() << fileName << "failed to upload in handleResponseAllDataSentForRequestGetFile()";
    }

    // remove the file content to release the memory, helpful?
    m_mapFileContent.remove(fileName);

    sendResponseUploadAFile(msg, fileName);

    return true;
}

void HBDCManagerHandler::saveFileFromServer(QString fileName, const QByteArray &fileContent)
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
