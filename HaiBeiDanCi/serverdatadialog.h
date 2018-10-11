#ifndef SERVERDATADIALOG_H
#define SERVERDATADIALOG_H

#include "servercommunicator.h"

#include <QDialog>
#include <QProgressDialog>
#include <QDateTime>

namespace Ui {
class ServerDataDialog;
}

class ServerDataDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ServerDataDialog(QWidget *parent = nullptr);
    ~ServerDataDialog();

private slots:
    void onItemSelectionChanged();
    void onBookListReady(const QList<QString> &books);
    void onBookDownloaded(QString bookName);

    void onFilesDownloadFinished(const QMap<QString, ServerCommunicator::DownloadStatus> &downloadResult);
    void onBookDownloadFinished(QString bookName, ServerCommunicator::DownloadStatus result);

    void on_pbDownloadBook_clicked();

    void on_pbClose_clicked();

    void on_pbDownloadMediaFiles_clicked();

    void on_pbDownloadPronounceFiles_clicked();

signals:
    void bookDownloaded(QString bookName);

private:
    Ui::ServerDataDialog *ui;

    QDateTime m_downloadStartTime;

    void updateBookStatus(QString bookName);

    void downloadBookPronounceFiles(QString bookName);
    void downloadBookExampleAudioFiles(QString bookName);
    bool fileExistsLocally(QString fileName);

    void downloadFiles(const QSet<QString> &setFiles, bool showProgress = true, QString labelText = QObject::tr("Downloading ..."), QString cancelButtonText = QString());
    void downloadBook(QString bookName, bool showProgress = true, QString labelText = QObject::tr("Downloading ..."), QString cancelButtonText = QString());
    void downloadBookList();
};

#endif // SERVERDATADIALOG_H
