#include "hbdcapphandler.h"
#include "../HaiBeiDanCi/mysettings.h"

#include <QFile>

HBDCAppHandler::HBDCAppHandler(ClientWaiter &clientWaiter) : ClientHandler(clientWaiter)
{

}

int HBDCAppHandler::handleMessage(const QByteArray &msg)
{
    MessageHeader receivedMsgHeader(msg);
    bool handleResult = false;
    switch (receivedMsgHeader.code()) {
    case ServerClientProtocol::RequestGetAllBooks:
        handleResult = handleRequestGetAllBooks(msg);
        break;

    case ServerClientProtocol::RequestGetAWord:
        handleResult = handleRequestGetAWord(msg);
        break;

    case ServerClientProtocol::RequestGetABook:
        handleResult = handleRequestGetABook(msg);
        break;

    case ServerClientProtocol::RequestGetBookWordList:
        handleResult = handleRequestGetBookWordList(msg);
        break;

    case ServerClientProtocol::RequestGetFile:
        handleResult = handleRequestGetFile(msg);
        break;

    case ServerClientProtocol::RequestGetWordsOfBookFinished:
        handleResult = handleRequestGetWordsOfBookFinished(msg);
        break;

    default:
        return ClientHandler::handleMessage(msg);

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

bool HBDCAppHandler::handleRequestGetAllBooks(const QByteArray &msg)
{
    funcTracker ft("handleRequestGetAllBooks()");

    auto books = WordBook::getAllBooks();
    sendResponseGetAllBooks(msg, books);
    return true;
}

void HBDCAppHandler::sendResponseGetAllBooks(const QByteArray &msg, const QList<QString> &books)
{
    MessageHeader receivedMsgHeader(msg);
    MessageHeader responseHeader(ServerClientProtocol::ResponseGetAllBooks, receivedMsgHeader.sequenceNumber());
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << responseHeader << books;
    sendMessage(block, true);
}

bool HBDCAppHandler::handleRequestGetAWord(const QByteArray &msg)
{
    funcTracker ft("handleRequestGetAWord()");

    // read the spelling of the word
    QDataStream in(msg);
    MessageHeader receivedMsgHeader(-1, -1, -1);
    QString spelling;
    in.startTransaction();
    in >> receivedMsgHeader >> spelling;
    if (in.commitTransaction() == false)
    {
        qCritical() << "failed to get book name in handleRequestGetAWord()";
        return false;
    }

    auto word = Word::getWord(spelling);
    if (word.get() == nullptr)
    {
        return false;
    }

    sendResponseGetAWord(msg, *word);

    return true;
}

void HBDCAppHandler::sendResponseGetAWord(const QByteArray &msg, const Word &word)
{
    MessageHeader receivedMsgHeader(msg);
    MessageHeader responseHeader(ServerClientProtocol::ResponseGetAWord, receivedMsgHeader.sequenceNumber());

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << responseHeader << word;
    sendMessage(block, true);
}

void HBDCAppHandler::sendResponseGetWordsOfBookFinished(const QByteArray &msg, QString bookName)
{
    MessageHeader receivedMsgHeader(msg);
    MessageHeader responseHeader(ServerClientProtocol::ResponseGetWordsOfBookFinished, receivedMsgHeader.sequenceNumber());

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << responseHeader << bookName;
    sendMessage(block);
}

void HBDCAppHandler::sendResponseGetFileFinished(const QByteArray &msg, QString fileName, bool succeeded)
{
    MessageHeader receivedMsgHeader(msg);
    MessageHeader responseHeader(ServerClientProtocol::ResponseGetFileFinished, receivedMsgHeader.sequenceNumber());

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << responseHeader << fileName << succeeded;
    sendMessage(block);
}

bool HBDCAppHandler::handleRequestGetABook(const QByteArray &msg)
{
    funcTracker ft("handleRequestGetABook()");

    // read the name of the book
    MessageHeader receivedMsgHeader(-1, -1, -1);
    QDataStream in(msg);
    QString bookName;
    in.startTransaction();
    in >> receivedMsgHeader >> bookName;
    if (in.commitTransaction() == false)
    {
        qCritical() << "failed to get book name in handleRequestGetABook()";
        return false;
    }

    auto book = WordBook::getBook(bookName);
    if (book.get() == nullptr)
    {
        return false;
    }

    sendResponseGetABook(msg, *book);

    return true;
}

bool HBDCAppHandler::handleRequestGetFile(const QByteArray &msg)
{
    funcTracker ft("handleRequestGetFile()");

    // read the name of the book
    QDataStream in(msg);
    MessageHeader receivedMsgHeader(-1, -1, -1);
    QString fileName;
    in.startTransaction();
    in >> receivedMsgHeader >> fileName;
    if (in.commitTransaction() == false)
    {
        qCritical() << "failed to get file name in handleRequestGetFile()";
        return false;
    }

    bool succeeded = sendFile(msg, fileName);
    sendResponseGetFileFinished(msg, fileName, succeeded);

    // the message has been processed, so return true regardless if succeeded or not
    return true;
}

void HBDCAppHandler::sendResponseGetABook(const QByteArray &msg, const WordBook &book)
{
    MessageHeader receivedMsgHeader(msg);
    MessageHeader responseHeader(ServerClientProtocol::ResponseGetABook, receivedMsgHeader.sequenceNumber());

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << responseHeader << book;
    sendMessage(block, true);
}

bool HBDCAppHandler::handleRequestGetBookWordList(const QByteArray &msg)
{
    funcTracker ft("handleRequestGetWordsOfBook()");

    // read the name of the book
    QDataStream in(msg);
    MessageHeader receivedMsgHeader(-1, -1, -1);
    QString bookName;
    in.startTransaction();
    in >> receivedMsgHeader >> bookName;
    if (in.commitTransaction() == false)
    {
        qCritical() << "failed to get book name in handleRequestGetBookWordList()";
        return false;
    }

    sendBookWordList(msg, bookName);

    return true;
}

bool HBDCAppHandler::handleRequestGetWordsOfBookFinished(const QByteArray &msg)
{
    funcTracker ft("handleRequestGetWordsOfBook()");

    // read the name of the book
    QDataStream in(msg);
    MessageHeader receivedMsgHeader(-1, -1, -1);
    QString bookName;
    in.startTransaction();
    in >> receivedMsgHeader >> bookName;
    if (in.commitTransaction() == false)
    {
        qCritical() << "failed to get book name in handleRequestGetWordsOfBookFinished()";
        return false;
    }

    sendResponseGetWordsOfBookFinished(msg, bookName);

    return true;
}

void HBDCAppHandler::sendBookWordList(const QByteArray &msg, QString bookName)
{
    auto book = WordBook::getBook(bookName);
    if (book == nullptr)
    {
        return;
    }

    QVector<QString> wordList = book->getAllWords();
    int total = wordList.size();
    int pos = 0;
    int counter = 0;
    while (pos < total)
    {
        counter ++;
        QVector<QString> subList = wordList.mid(pos, ServerClientProtocol::MaximumWordsInAMessage);
        pos += subList.size();
        sendResponseGetBookWordList(msg, bookName, subList, pos>=total);
    }
}

bool HBDCAppHandler::okToSendFile(QString fileName)
{
    // only allow files in folder media
    if (fileName.startsWith("media", Qt::CaseInsensitive) == false)
    {
        return false;
    }

    // only allow mp3/png/jpg/css/js
    QString ext = fileName.section('.', -1);
    if (ext.compare("mp3", Qt::CaseInsensitive) != 0
            && ext.compare("png", Qt::CaseInsensitive) != 0
            && ext.compare("jpg", Qt::CaseInsensitive) != 0
            && ext.compare("css", Qt::CaseInsensitive) != 0
            && ext.compare("js", Qt::CaseInsensitive) != 0)
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

bool HBDCAppHandler::sendFile(const QByteArray &msg, QString fileName)
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

    const int fileSize = static_cast<int>(toSend.size());
    int sentBytes = 0;
    int counter = 0;
    bool succeeded = true;
    QDataStream fileDS(&toSend);
    char buf[ServerClientProtocol::MaximumBytesForFileTransfer + 1];
    while (sentBytes < fileSize)
    {
        auto readBytes = fileDS.readRawData(buf, ServerClientProtocol::MaximumBytesForFileTransfer);
        if (readBytes == -1)
        {
            succeeded = false;
            break;
        }

        counter ++;
        sendResponseGetFile(msg, fileName, buf, static_cast<uint>(readBytes));
        sentBytes += readBytes;
        qDebug() << "send" << readBytes << "bytes of total" << fileSize;
    }

    return succeeded;
}

void HBDCAppHandler::sendResponseGetFile(const QByteArray &msg, QString fileName, const char *s, uint len)
{
    MessageHeader receivedMsgHeader(msg);
    MessageHeader responseHeader(ServerClientProtocol::ResponseGetFile, receivedMsgHeader.sequenceNumber());

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << responseHeader << fileName;
    out.writeBytes(s, len);
    sendMessage(block);
}

void HBDCAppHandler::sendResponseGetBookWordList(const QByteArray &msg, QString bookName, const QVector<QString> &wordList, bool listComplete)
{
    Q_ASSERT(wordList.size() <= ServerClientProtocol::MaximumWordsInAMessage);

    MessageHeader receivedMsgHeader(msg);
    MessageHeader responseHeader(ServerClientProtocol::ResponseGetBookWordList, receivedMsgHeader.sequenceNumber());

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << responseHeader << bookName << wordList << listComplete;
    sendMessage(block, true);
}
