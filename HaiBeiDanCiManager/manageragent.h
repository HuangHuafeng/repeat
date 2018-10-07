#ifndef MANAGERAGENT_H
#define MANAGERAGENT_H

#include "../HaiBeiDanCi/svragt.h"

class ManagerAgent : public SvrAgt
{
    Q_OBJECT

public:
    ManagerAgent(const QString &hostName, quint16 port = 61027, QObject *parent = nullptr);
    virtual ~ManagerAgent() override;

    void sendRequestPromoteToManager();
    void sendRequestGetAllWordsWithoutDefinition();
    void sendRequestGetServerDataFinished();

    void sendRequestDeleteABook(QString bookName);
    void sendRequestMissingMediaFiles(QString bookName);

    void sendRequestUploadAFile(QString fileName, const char *s, uint len, int sentBytes, int totalBytes);
    void sendRequestUploadAFileFinished(QString fileName, bool succeeded);

    void sendRequestReleaseApp(ApplicationVersion version, QString platform, QString fileName, QString info);
    void sendRequestReleaseUpgrader(ApplicationVersion version, QString platform, QString fileName);

    void sendBookWordList(QString bookName, const QVector<QString> &wordList);
    // the following messages are used to upload a book
    void sendResponseGetABook(const WordBook &book);
    void sendResponseGetBookWordList(QString bookName, const QVector<QString> &wordList, bool listComplete);
    void sendResponseGetAWord(const Word &word);
    void sendResponseGetWordsOfBookFinished(QString bookName);

signals:
    void getAllWordsWithoutDefinitionFinished(const QVector<QString> &spellings, const QVector<int> &ids, const QVector<int> &definitionLengths);
    void getServerDataFinished();
    void bookDeleted(QString bookName);
    void bookUploaded(QString bookName);
    void fileUploaded(QString fileName);
    void fileUploadingProgress(QString fileName, uint uploadedBytes, uint totalBytes);
    void wordUploaded(QString spelling);
    void gotMissingMediaFilesOfBook(QString bookName, const QList<QString> &fileList);
    void appReleased(bool succeed);
    void upgraderReleased(bool succeed);

protected:
    virtual int handleMessage(const QByteArray &msg) override;    

private:
    QVector<QString> m_spellingList;
    QVector<int> m_idList;
    QVector<int> m_definitionLengthList;

    bool handleResponsePromoteToManager(const QByteArray &msg);
    bool handleResponseGetAllWordsWithoutDefinition(const QByteArray &msg);
    bool handleResponseGetServerDataFinished(const QByteArray &msg);
    bool handleResponseDeleteABook(const QByteArray &msg);
    bool handleResponseUploadABook(const QByteArray &msg);
    bool handleResponseMissingMediaFiles(const QByteArray &msg);
    bool handleResponseUploadAFile(const QByteArray &msg);
    bool handleResponseUploadAWord(const QByteArray &msg);
    bool handleResponseUploadAFileFinished(const QByteArray &msg);
    bool handleResponseReleaseApp(const QByteArray &msg);
    bool handleResponseReleaseUpgrader(const QByteArray &msg);
};

#endif // MANAGERAGENT_H
