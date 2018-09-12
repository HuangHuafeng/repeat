#ifndef SERVERMANAGER_H
#define SERVERMANAGER_H

#include "manageragent.h"

#include <QString>
#include <QObject>

class ServerManager : public QObject
{
    Q_OBJECT

public:
    static ServerManager * instance();

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
    void syncToLocal();
    bool bookExistsInServer(QString bookName);
    void uploadBook(QString bookName);
    void uploadfile(QString fileName);
    void downloadBook(QString bookName);
    void deleteBook(QString bookName);
    QVector<QString> getWordListOfBook(QString bookName);
    QList<QString> getMissingMediaFilesOfBook(QString bookName);
    void uploadBookMissingMediaFiles(QString bookName);

signals:
    void serverDataReloaded();
    void uploadProgress(float percentage);

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
    void onWordUploaded(QString spelling);

private:
    ServerManager(QObject *parent = nullptr);

    static ServerManager *m_sm;

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

    bool okToSendFile(const QString fileName);
    bool sendFile(const QString fileName);
};

#endif // SERVERMANAGER_H
