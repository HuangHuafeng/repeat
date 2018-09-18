#ifndef SERVERDATADIALOG_H
#define SERVERDATADIALOG_H

#include "serverdatadownloader.h"

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
    void onBookListReady(const QList<QString> books);
    void onBookDownloaded(QString bookName);
    void onDownloadProgress(float percentage);

    void on_pbDownloadBook_clicked();

    void on_pbClose_clicked();

    void on_pbDownloadMediaFiles_clicked();

    void on_pbDownloadPronounceFiles_clicked();

signals:
    void bookDownloaded(QString bookName);

private:
    Ui::ServerDataDialog *ui;

    QProgressDialog m_progressDialog;
    int m_pdMaximum;

    QDateTime m_downloadStartTime;
    ServerDataDownloader *m_sdd;

    void updateBookStatus(QString bookName);

    void initializeProgressDialog();
    void createProgressDialog(const QString &labelText, const QString &cancelButtonText);
    void destroyProgressDialog();

    void downloadBookPronounceFiles(QString bookName);
    void downloadBookExampleAudioFiles(QString bookName);
    bool fileExistsLocally(QString fileName);

    bool userAlreadyLogin();
};

#endif // SERVERDATADIALOG_H
