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

    case ServerClientProtocol::ResponseUploadAFileFinished:
        handleResult = handleResponseUploadAFileFinished(msg);
        break;

    case ServerClientProtocol::ResponseUploadAWord:
        handleResult = handleResponseUploadAWord(msg);
        break;
        
    case ServerClientProtocol::ResponseReleaseApp:
        handleResult = handleResponseReleaseApp(msg);
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
    Q_ASSERT(msg.size() >= 0);

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
    Q_ASSERT(msg.size() >= 0);

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
    uint receivedBytes, totalBytes;
    in.startTransaction();
    in >> receivedMsgHeader >> fileName >> receivedBytes >> totalBytes;
    if (in.commitTransaction() == false)
    {
        qCritical() << "failed to read the book name in handleResponseUploadABook()";
        return false;
    }

    emit(fileUploadingProgress(fileName, receivedBytes, totalBytes));

    return true;
}

bool ManagerAgent::handleResponseUploadAFileFinished(const QByteArray &msg)
{
    QDataStream in(msg);
    MessageHeader receivedMsgHeader(-1, -1, -1);
    QString fileName;
    in.startTransaction();
    in >> receivedMsgHeader >> fileName;
    if (in.commitTransaction() == false)
    {
        qCritical() << "failed to read the book name in handleResponseUploadAFileFinished()";
        return false;
    }

    emit(fileUploaded(fileName));

    return true;
}

bool ManagerAgent::handleResponseUploadAWord(const QByteArray &msg)
{
    QDataStream in(msg);
    MessageHeader receivedMsgHeader(-1, -1, -1);
    QString spelling;
    in.startTransaction();
    in >> receivedMsgHeader >> spelling;
    if (in.commitTransaction() == false)
    {
        qCritical() << "failed to read the spelling in handleResponseUploadAWord()";
        return false;
    }

    emit(wordUploaded(spelling));

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

void ManagerAgent::sendBookWordList(QString bookName, const QVector<QString> &wordList)
{
    int total = wordList.size();
    int pos = 0;
    while (pos < total)
    {
        QVector<QString> subList = wordList.mid(pos, ServerClientProtocol::MaximumWordsInAMessage);
        pos += subList.size();
        sendResponseGetBookWordList(bookName, subList, pos>=total);
    }
}

// the following messages are used to upload a book
void ManagerAgent::sendResponseGetABook(const WordBook &book)
{
    MessageHeader msgHeader(ServerClientProtocol::ResponseGetABook);
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << msgHeader << book;
    sendMessage(block);
}

void ManagerAgent::sendResponseGetBookWordList(QString bookName, const QVector<QString> &wordList, bool listComplete)
{
    Q_ASSERT(wordList.size() <= ServerClientProtocol::MaximumWordsInAMessage);
    MessageHeader msgHeader(ServerClientProtocol::ResponseGetBookWordList);
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << msgHeader << bookName << wordList << listComplete;
    sendMessage(block);
}

void ManagerAgent::sendResponseGetAWord(const Word &word)
{
    MessageHeader msgHeader(ServerClientProtocol::ResponseGetAWord);
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << msgHeader << word;
    sendMessage(block);
}

void ManagerAgent::sendResponseGetWordsOfBookFinished(QString bookName)
{
    MessageHeader msgHeader(ServerClientProtocol::ResponseGetWordsOfBookFinished);
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << msgHeader << bookName;
    sendMessage(block);
}

void ManagerAgent::sendRequestDeleteABook(QString bookName)
{
    MessageHeader msgHeader(ServerClientProtocol::RequestDeleteABook);
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << msgHeader << bookName;
    sendMessage(block);
}

void ManagerAgent::sendRequestMissingMediaFiles(QString bookName)
{
    MessageHeader msgHeader(ServerClientProtocol::RequestMissingMediaFiles);
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << msgHeader << bookName;
    sendMessage(block);
}


void ManagerAgent::sendRequestUploadAFile(QString fileName, const char *s, uint len, int sentBytes, int totalBytes)
{
    MessageHeader msgHeader(ServerClientProtocol::RequestUploadAFile);

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << msgHeader << fileName;
    out.writeBytes(s, len);
    out << sentBytes << totalBytes;
    sendMessage(block);
}

void ManagerAgent::sendRequestUploadAFileFinished(QString fileName, bool succeeded)
{
    MessageHeader msgHeader(ServerClientProtocol::RequestUploadAFileFinished);

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << msgHeader << fileName << succeeded;
    sendMessage(block);
}

void ManagerAgent::sendRequestReleaseApp(ApplicationVersion version, QString platform, QString fileName, QString info)
{
    MessageHeader msgHeader(ServerClientProtocol::RequestReleaseApp);

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << msgHeader << version << platform << fileName << info;
    sendMessage(block);
}

bool ManagerAgent::handleResponseReleaseApp(const QByteArray &msg)
{
    QDataStream in(msg);
    MessageHeader receivedMsgHeader(-1, -1, -1);
    bool succeed;
    in.startTransaction();
    in >> receivedMsgHeader >> succeed;
    if (in.commitTransaction() == false)
    {
        qCritical() << "failed to read info in handleResponseReleaseApp()";
        return false;
    }

    emit(appReleased(succeed));

    return true;
}
