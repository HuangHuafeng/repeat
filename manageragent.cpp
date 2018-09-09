#include "manageragent.h"

ManagerAgent::ManagerAgent(const QString &hostName, quint16 port, QObject *parent) :
    SvrAgt (hostName, port, parent)
{
}

ManagerAgent::~ManagerAgent()
{
}

int ManagerAgent::handleMessage(const QByteArray &msg)
{
    MessageHeader receivedMsgHeader(msg);
    bool handleResult = false;
    switch (receivedMsgHeader.code()) {
    case ServerClientProtocol::ResponsePromoteToManager:
        handleResult = handleResponsePromoteToManager(msg);
        break;

    case ServerClientProtocol::ResponseGetAllWordsWithoutDefinition:
        handleResult = handleResponseGetAllWordsWithoutDefinition(msg);
        break;

    case ServerClientProtocol::ResponseGetServerDataFinished:
        handleResult = handleResponseGetServerDataFinished(msg);
        break;

    case ServerClientProtocol::ResponseDeleteABook:
        handleResult = handleResponseDeleteABook(msg);
        break;

    case ServerClientProtocol::ResponseUploadABook:
        handleResult = handleResponseUploadABook(msg);
        break;

    case ServerClientProtocol::ResponseMissingMediaFiles:
        handleResult = handleResponseMissingMediaFiles(msg);
        break;

    case ServerClientProtocol::ResponseUploadAFile:
        handleResult = handleResponseUploadAFile(msg);
        break;

    default:
        return SvrAgt::handleMessage(msg);

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

void ManagerAgent::sendRequestPromoteToManager()
{
    sendSimpleMessage(ServerClientProtocol::RequestPromoteToManager);
}


void ManagerAgent::sendRequestGetAllWordsWithoutDefinition()
{
    sendSimpleMessage(ServerClientProtocol::RequestGetAllWordsWithoutDefinition);
}

void ManagerAgent::sendRequestGetServerDataFinished()
{
    sendSimpleMessage(ServerClientProtocol::RequestGetServerDataFinished);
}


bool ManagerAgent::handleResponsePromoteToManager(const QByteArray &msg)
{
    qDebug() << msg;

    return true;
}

bool ManagerAgent::handleResponseGetAllWordsWithoutDefinition(const QByteArray &msg)
{
    QDataStream in(msg);
    MessageHeader receivedMsgHeader(-1, -1, -1);
    QVector<QString> spellingList;
    QVector<int> idList;
    QVector<int> definitionLengthList;
    bool listComplete;
    in.startTransaction();
    in >> receivedMsgHeader >> spellingList >> idList >> definitionLengthList >> listComplete;
    if (in.commitTransaction() == false)
    {
        qCritical() << "failed to read words of the book in handleResponseGetBookWordList()";
        return false;
    }

    m_spellingList += spellingList;
    m_idList += idList;
    m_definitionLengthList += definitionLengthList;

    if (listComplete == true)
    {
        emit(getAllWordsWithoutDefinitionFinished(m_spellingList, m_idList, m_definitionLengthList));
        m_spellingList.clear();
        m_idList.clear();
        m_definitionLengthList.clear();
    }

    return true;
}

bool ManagerAgent::handleResponseGetServerDataFinished(const QByteArray &msg)
{
    qDebug() << msg;

    emit(getServerDataFinished());

    return true;
}

bool ManagerAgent::handleResponseDeleteABook(const QByteArray &msg)
{
    QDataStream in(msg);
    MessageHeader receivedMsgHeader(-1, -1, -1);
    QString bookName;
    in.startTransaction();
    in >> receivedMsgHeader >> bookName;
    if (in.commitTransaction() == false)
    {
        qCritical() << "failed to read the book name in handleResponseDeleteABook()";
        return false;
    }

    emit(bookDeleted(bookName));

    return true;
}

bool ManagerAgent::handleResponseUploadABook(const QByteArray &msg)
{
    QDataStream in(msg);
    MessageHeader receivedMsgHeader(-1, -1, -1);
    QString bookName;
    in.startTransaction();
    in >> receivedMsgHeader >> bookName;
    if (in.commitTransaction() == false)
    {
        qCritical() << "failed to read the book name in handleResponseUploadABook()";
        return false;
    }

    emit(bookUploaded(bookName));

    return true;
}

bool ManagerAgent::handleResponseUploadAFile(const QByteArray &msg)
{
    QDataStream in(msg);
    MessageHeader receivedMsgHeader(-1, -1, -1);
    QString fileName;
    in.startTransaction();
    in >> receivedMsgHeader >> fileName;
    if (in.commitTransaction() == false)
    {
        qCritical() << "failed to read the book name in handleResponseUploadABook()";
        return false;
    }

    emit(fileUploaded(fileName));

    return true;
}

bool ManagerAgent::handleResponseMissingMediaFiles(const QByteArray &msg)
{
    QDataStream in(msg);
    MessageHeader receivedMsgHeader(-1, -1, -1);
    QString bookName;
    QList<QString> missingMediaFiles;
    in.startTransaction();
    in >> receivedMsgHeader >> bookName >> missingMediaFiles;
    if (in.commitTransaction() == false)
    {
        qCritical() << "failed to read the book name in handleResponseUploadABook()";
        return false;
    }

    emit(gotMissingMediaFilesOfBook(bookName, missingMediaFiles));

    return true;
}

void ManagerAgent::sendBookWordList(const QString bookName, const QVector<QString> &wordList)
{
    int total = wordList.size();
    int pos = 0;
    int counter = 0;
    while (pos < total)
    {
        counter ++;
        QVector<QString> subList = wordList.mid(pos, ServerClientProtocol::MaximumWordsInAMessage);
        pos += subList.size();
        sendResponseGetBookWordList(bookName, subList, pos>=total);
    }
}

// the following messages are used to upload a book
void ManagerAgent::sendResponseGetABook(const WordBook &book)
{
    MessageHeader responseHeader(ServerClientProtocol::ResponseGetABook);
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << responseHeader << book;
    sendMessage(block);
}

void ManagerAgent::sendResponseGetBookWordList(QString bookName, const QVector<QString> &wordList, bool listComplete)
{
    Q_ASSERT(wordList.size() <= ServerClientProtocol::MaximumWordsInAMessage);
    MessageHeader responseHeader(ServerClientProtocol::ResponseGetBookWordList);
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << responseHeader << bookName << wordList << listComplete;
    sendMessage(block);
}

void ManagerAgent::sendResponseGetAWord(const Word &word)
{
    MessageHeader responseHeader(ServerClientProtocol::ResponseGetAWord);
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << responseHeader << word;
    sendMessage(block);
}

void ManagerAgent::sendResponseGetWordsOfBookFinished(QString bookName)
{
    MessageHeader responseHeader(ServerClientProtocol::ResponseGetWordsOfBookFinished);
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << responseHeader << bookName;
    sendMessage(block);
}

void ManagerAgent::sendRequestDeleteABook(QString bookName)
{
    MessageHeader responseHeader(ServerClientProtocol::RequestDeleteABook);
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << responseHeader << bookName;
    sendMessage(block);
}

void ManagerAgent::sendRequestMissingMediaFiles(QString bookName)
{
    MessageHeader responseHeader(ServerClientProtocol::RequestMissingMediaFiles);
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << responseHeader << bookName;
    sendMessage(block);
}


void ManagerAgent::sendResponseGetFile(QString fileName, const char *s, uint len)
{
    MessageHeader responseHeader(ServerClientProtocol::ResponseGetFile);

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << responseHeader << fileName;
    out.writeBytes(s, len);
    sendMessage(block);
}

void ManagerAgent::sendResponseGetFileFinished(QString fileName, bool succeeded)
{
    MessageHeader responseHeader(ServerClientProtocol::ResponseGetFileFinished);

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << responseHeader << fileName << succeeded;
    sendMessage(block);
}
