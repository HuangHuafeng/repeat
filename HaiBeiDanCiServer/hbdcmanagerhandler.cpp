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
        sendAListOfWordsWithoutDefinition(msg, subList);
        pos += subList.size();
    }

    sendResponseGetAllWordsWithoutDefinitionFinished(msg);
}

void HBDCManagerHandler::sendAListOfWordsWithoutDefinition(const QByteArray &msg, const QList<QString> &wordList)
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

    sendResponseGetAllWordsWithoutDefinition(msg, spellingList, idList, definitionLengthList);
}

void HBDCManagerHandler::sendResponseGetAllWordsWithoutDefinition(const QByteArray &msg, const QVector<QString> &spellings, const QVector<int> &ids, const QVector<int> &definitionLengths)
{
    MessageHeader receivedMsgHeader(msg);
    MessageHeader responseHeader(ServerClientProtocol::ResponseGetAllWordsWithoutDefinition, receivedMsgHeader.sequenceNumber());

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << responseHeader << spellings << ids << definitionLengths;
    sendMessage(block);
}

void HBDCManagerHandler::sendResponseGetAllWordsWithoutDefinitionFinished(const QByteArray &msg)
{
    sendSimpleMessage(msg, ServerClientProtocol::ResponseGetAllWordsWithoutDefinitionFinished);
}
