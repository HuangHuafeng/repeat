#ifndef BOOKDOWNLOADER_H
#define BOOKDOWNLOADER_H

#include "servercommunicator.h"

#include <QObject>
#include <QProgressDialog>

class BookDownloader : public QObject
{
    Q_OBJECT

public:
    explicit BookDownloader(ServerCommunicator *sc, QObject *parent = nullptr);
    ~BookDownloader();

    void setShowProgress(bool showProgress = true, QString labelText = QObject::tr("Downloading ..."), QString cancelButtonText = QString(), QWidget *parent = nullptr);
    bool downloadBook(QString bookName);
    bool downloadBookList();

signals:
    void downloadFinished(QString bookName, ServerCommunicator::DownloadStatus result);
    void bookListDownloaded(const QList<QString> &books);

private slots:
    void onBookListDownloaded(const QList<QString> &books);
    void onBookDownloaded(const WordBook &book);
    void onBookWordListReceived(QString bookName, const QVector<QString> &wordList);
    void onWordDownloaded(const Word &word);
    void onCanceled();

private:
    ServerCommunicator *m_sc;
    bool m_showProgress;
    QProgressDialog *m_pd;

    WordBook m_book;
    QVector<QString> m_bookWords;

    QMap<QString, sptr<Word>> m_mapWords;
    int m_toDownload;
    int m_downloaded;

    void downloadWordsOfBook();
    bool downloadOngoing();
    void finishDownloading(ServerCommunicator::DownloadStatus result);
    void saveBookFromServer();
};

#endif // BOOKDOWNLOADER_H
