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

    case ServerClientProtocol::ResponseGetAllWordsWithoutDefinitionFinished:
        handleResult = handleResponseGetAllWordsWithoutDefinitionFinished(msg);
        break;

    case ServerClientProtocol::ResponseGetServerDataFinished:
        handleResult = handleResponseGetServerDataFinished(msg);
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
    in.startTransaction();
    in >> receivedMsgHeader >> spellingList >> idList >> definitionLengthList;
    if (in.commitTransaction() == false)
    {
        qCritical() << "failed to read words of the book in handleResponseGetBookWordList()";
        return false;
    }

    m_spellingList += spellingList;
    m_idList += idList;
    m_definitionLengthList += definitionLengthList;

    return true;
}

bool ManagerAgent::handleResponseGetAllWordsWithoutDefinitionFinished(const QByteArray &msg)
{
    qDebug() << msg;

    emit(getAllWordsWithoutDefinitionFinished(m_spellingList, m_idList, m_definitionLengthList));
    m_spellingList.clear();
    m_idList.clear();
    m_definitionLengthList.clear();

    return true;
}

bool ManagerAgent::handleResponseGetServerDataFinished(const QByteArray &msg)
{
    qDebug() << msg;

    emit(getServerDataFinished());

    return true;
}
