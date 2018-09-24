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
    QMap<QString, QByteArray>  m_mapFileContent;

    bool handleRequestGetAllWordsWithoutDefinition(const QByteArray &msg);
    bool handleRequestGetServerDataFinished(const QByteArray &msg);
    bool handleRequestMissingMediaFiles(const QByteArray &msg);
    bool handleRequestDeleteABook(const QByteArray &msg);

    // for uploading a book from the manager
    bool handleResponseGetABook(const QByteArray &msg);
    bool handleResponseGetBookWordList(const QByteArray &msg);
    bool handleResponseGetAWord(const QByteArray &msg);
    bool handleResponseGetWordsOfBookFinished(const QByteArray &msg);

    // for uploading a file from the manager
    bool handleRequestUploadAFile(const QByteArray &msg);
    bool handleRequestUploadAFileFinished(const QByteArray &msg);
    void saveFileFromServer(QString fileName, const QByteArray &fileContent);
    void sendResponseUploadAFile(const QByteArray &msg, QString fileName, uint receivedBytes, uint totalBytes);
    void sendResponseUploadAFileFinished(const QByteArray &msg, QString fileName);

    void sendResponseGetServerDataFinished(const QByteArray &msg);

    void sendAllWordsWithoutDefinition(const QByteArray &msg);
    void sendAListOfWordsWithoutDefinition(const QByteArray &msg, const QList<QString> &wordList, bool listComplete);
    void sendResponseGetAllWordsWithoutDefinition(const QByteArray &msg, const QVector<QString> &spellings, const QVector<int> &ids, const QVector<int> &definitionLengths, bool listComplete);

    void sendResponseUploadABook(const QByteArray &msg, QString bookName);
    void sendResponseDeleteABook(const QByteArray &msg, QString bookName);
    void sendResponseUploadAWord(const QByteArray &msg, QString spelling);

    void sendBookMissingMediaFiles(const QByteArray &msg, QString bookName);
    void sendResponseMissingMediaFiles(const QByteArray &msg, QString bookName, const QList<QString> &fileList);

};

#endif // HBDCMANAGERHANDLER_H
