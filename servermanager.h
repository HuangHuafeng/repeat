#ifndef SERVERMANAGER_H
#define SERVERMANAGER_H

#include "manageragent.h"

#include <QString>
#include <QObject>

class ServerManager : public QObject
{
    Q_OBJECT

public:
    ServerManager(QObject *parent = nullptr);
    ~ServerManager() override;

    QList<QString> getBookList();
    const QMap<QString, sptr<Word>> & getAllWords()
    {
        return m_mapWords;
    }

    bool serverDataLoaded()
    {
        return m_serverDataLoaded;
    }

    void reloadServerData();
    bool okToSync(QString *errorString = nullptr);
    bool bookExistsInServer(QString bookName);
    void uploadBook(QString bookName);
    void uploadfile(QString fileName);
    void downloadBook(QString bookName);
    void deleteBook(QString bookName);
    QVector<QString> getWordListOfBook(QString bookName);
    QList<QString> getMissingMediaFilesOfBook(QString bookName);
    void uploadBookMissingMediaFiles(QString bookName);
    void releaseApp(ApplicationVersion version, QString platform, QString fileName, QString info);

signals:
    void serverDataReloaded();
    void uploadProgress(float percentage);
    void bookDownloaded(QString bookName);
    void fileUploaded(QString fileName);
    void appReleased(bool succeed);

public slots:
    void OnBookListReady(const QList<QString> &books);
    void OnBookDownloaded(sptr<WordBook> book);
    void OnBookWordListReceived(QString bookName, const QVector<QString> &wordList);

    void onServerConnected();
    void onGetServerDataFinished();
    void onGetAllWordsWithoutDefinitionFinished(const QVector<QString> &spellings, const QVector<int> &ids, const QVector<int> &definitionLengths);
    void onBookDeleted(QString bookName);
    void onBookUploaded(QString bookName);
    void onGotMissingMediaFilesOfBook(QString bookName, const QList<QString> &missingFiles);
    void onFileUploaded(QString fileName);
    void onFileUploadingProgress(QString fileName, uint uploadedBytes, uint totalBytes);
    void onWordUploaded(QString spelling);
    void onAppReleased(bool succeed);

private:
    ManagerAgent m_mgrAgt;
    bool m_serverDataLoaded;

    QMap<QString, sptr<WordBook>> m_mapBooks;
    QMap<QString, QVector<QString>> m_mapBooksWordList;
    QMap<QString, QList<QString>> m_mapBooksMissingFiles;
    QMap<QString, sptr<Word>> m_mapWords;

    // used to calculate upload progress
    int m_toUpload;
    int m_uploaded;

    void downloadBookDetails(QString bookName);

    void clearData();

    bool okToSyncBooks(QString *errorString = nullptr);
    bool okToSyncWords(QString *errorString = nullptr);

    void sendBookWordList(QString bookName);
    void sendWordsOfBook(QString bookName);

    bool okToSendFile(QString fileName);
    bool sendFile(QString fileName);
};

#endif // SERVERMANAGER_H
