#ifndef SERVERDATADOWNLOADER_H
#define SERVERDATADOWNLOADER_H

#include "svragt.h"

#include <QObject>

class ServerDataDownloader : public QObject
{
    Q_OBJECT

public:
    static ServerDataDownloader * instance();
    static void destroy();

    QList<QString> getBookList();
    void downloadBook(QString bookName);
    void downloadFile(QString fileName);
    void downloadMultipleFiles(QSet<QString> files);
    void cancelDownloading();

signals:
    void bookListReady(const QList<QString> &books);
    void downloadProgress(float percentage);
    void bookStored(QString bookName);
    void fileDownloaded(QString fileName, bool succeeded);

public slots:
    void onServerConnected();
    void OnBookListReady(const QList<QString> &books);
    void OnBookDownloaded(sptr<WordBook> book);
    void OnWordDownloaded(sptr<Word> word);
    void OnDownloadProgress(float percentage);
    void OnFileDownloaded(QString fileName, SvrAgt::DownloadStatus result, const QByteArray &fileContent);
    void OnGetWordsOfBookFinished(QString bookName);
    void OnBookWordListReceived(QString bookName, const QVector<QString> &wordList);

private:
    explicit ServerDataDownloader(QObject *parent = nullptr);

    static ServerDataDownloader *m_sdd;

    SvrAgt m_svrAgt;

    QMap<QString, sptr<WordBook>> m_mapBooks;
    QMap<QString, QVector<QString>> m_mapBooksWordList;
    QMap<QString, sptr<Word>> m_mapWords;

    void downloadAllBooks();
    void downloadWordsOfBook(QString bookName);
    void saveFileFromServer(QString fileName, const QByteArray &fileContent);
};

#endif // SERVERDATADOWNLOADER_H
