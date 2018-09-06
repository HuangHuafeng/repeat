#ifndef HBDCMANAGERHANDLER_H
#define HBDCMANAGERHANDLER_H

#include "hbdcapphandler.h"

class HBDCManagerHandler : public HBDCAppHandler
{
public:
    HBDCManagerHandler(ClientWaiter &clientWaiter);

    virtual int handleMessage(const QByteArray &msg) override;

private:
    QMap<QString, sptr<WordBook>> m_mapBooks;
    QMap<QString, QVector<QString>> m_mapBooksWordList;
    QMap<QString, sptr<Word>> m_mapWords;

    bool handleRequestGetAllWordsWithoutDefinition(const QByteArray &msg);
    bool handleRequestGetServerDataFinished(const QByteArray &msg);
    bool handleResponseGetABook(const QByteArray &msg);
    bool handleResponseGetBookWordList(const QByteArray &msg);
    bool handleResponseBookWordListAllSent(const QByteArray &msg);
    bool handleResponseGetAWord(const QByteArray &msg);
    bool handleResponseGetWordsOfBookFinished(const QByteArray &msg);
    bool handleRequestDeleteABook(const QByteArray &msg);

    void sendAllWordsWithoutDefinition(const QByteArray &msg);
    void sendResponseGetServerDataFinished(const QByteArray &msg);
    void sendAListOfWordsWithoutDefinition(const QByteArray &msg, const QList<QString> &wordList);
    void sendResponseGetAllWordsWithoutDefinition(const QByteArray &msg, const QVector<QString> &spellings, const QVector<int> &ids, const QVector<int> &definitionLengths);
    void sendResponseGetAllWordsWithoutDefinitionFinished(const QByteArray &msg);
    void sendResponseUploadABook(const QByteArray &msg, QString bookName);
    void sendResponseDeleteABook(const QByteArray &msg, QString bookName);
};

#endif // HBDCMANAGERHANDLER_H
