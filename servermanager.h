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
    void uploadBook(QString bookName);
    void downloadBook(QString bookName);
    void deleteBook(QString bookName);
    QVector<QString> getWordListOfBook(QString bookName);

signals:
    void serverDataReloaded();

public slots:
    void OnBookListReady(const QList<QString> &books);
    void OnBookDownloaded(sptr<WordBook> book);
    void OnBookWordListReceived(QString bookName, const QVector<QString> &wordList);

    void onServerConnected();
    void onGetServerDataFinished();
    void onGetAllWordsWithoutDefinitionFinished(const QVector<QString> &spellings, const QVector<int> &ids, const QVector<int> &definitionLengths);
    void onBookDeleted(QString bookName);
    void onBookUploaded(QString bookName);

private:
    ServerManager(QObject *parent = nullptr);

    static ServerManager *m_sm;

    ManagerAgent m_mgrAgt;
    bool m_serverDataLoaded;

    QMap<QString, sptr<WordBook>> m_mapBooks;
    QMap<QString, QVector<QString>> m_mapBooksWordList;
    QMap<QString, sptr<Word>> m_mapWords;

    void downloadAllBooks();
    void downloadWordsOfBook(QString bookName);
    void saveFileFromServer(QString fileName, const QByteArray &fileContent);

    void clearData();
    void printData();

    bool okToSyncBooks(QString *errorString = nullptr);
    bool okToSyncWords(QString *errorString = nullptr);

    void sendBookWordList(QString bookName);
    void sendWordsOfBook(QString bookName);
};

#endif // SERVERMANAGER_H
