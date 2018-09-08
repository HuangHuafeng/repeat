#include "hbdcmanagerhandler.h"

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

    sendResponseOK(msg);

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
