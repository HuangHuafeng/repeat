#ifndef HBDCMANAGERHANDLER_H
#define HBDCMANAGERHANDLER_H

#include "hbdcapphandler.h"

class HBDCManagerHandler : public HBDCAppHandler
{
public:
    HBDCManagerHandler(ClientWaiter &clientWaiter);

    virtual int handleMessage(const QByteArray &msg) override;

private:
    bool handleRequestGetAllWordsWithoutDefinition(const QByteArray &msg);
    bool handleRequestGetServerDataFinished(const QByteArray &msg);

    void sendAllWordsWithoutDefinition(const QByteArray &msg);
    void sendResponseGetServerDataFinished(const QByteArray &msg);
    void sendAListOfWordsWithoutDefinition(const QByteArray &msg, const QList<QString> &wordList);
    void sendResponseGetAllWordsWithoutDefinition(const QByteArray &msg, const QVector<QString> &spellings, const QVector<int> &ids, const QVector<int> &definitionLengths);
    void sendResponseGetAllWordsWithoutDefinitionFinished(const QByteArray &msg);
};

#endif // HBDCMANAGERHANDLER_H
