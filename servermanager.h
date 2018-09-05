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
    void reloadServerData();

signals:
    void bookListReady(const QList<QString> &books);
    void downloadProgress(float percentage);
    void bookStored(QString bookName);
    void fileDownloaded(QString fileName, bool succeeded);

    void serverDataReloaded();

public slots:
    void OnBookListReady(const QList<QString> &books);
    void OnBookDownloaded(sptr<WordBook> book);
    void OnWordDownloaded(sptr<Word> word);
    void OnDownloadProgress(float percentage);
    void OnFileDownloaded(QString fileName, SvrAgt::DownloadStatus result, const QByteArray &fileContent);
    void OnGetWordsOfBookFinished(QString bookName);
    void OnBookWordListReceived(QString bookName, const QVector<QString> &wordList);

    void onServerConnected();
    void onGetServerDataFinished();
    void onGetAllWordsWithoutDefinitionFinished(const QVector<QString> &spellings, const QVector<int> &ids, const QVector<int> &definitionLengths);

private:
    ServerManager(QObject *parent = nullptr);

    static ServerManager *m_sm;

    ManagerAgent m_mgrAgt;
    bool m_bookListDownloaded;

    QMap<QString, sptr<WordBook>> m_mapBooks;
    QMap<QString, QVector<QString>> m_mapBooksWordList;
    QMap<QString, sptr<Word>> m_mapWords;

    void downloadAllBooks();
    void downloadWordsOfBook(QString bookName);
    void saveFileFromServer(QString fileName, const QByteArray &fileContent);

    void clearData();
    void printData();
};

#endif // SERVERMANAGER_H
