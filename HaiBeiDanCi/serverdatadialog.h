#ifndef SERVERDATADIALOG_H
#define SERVERDATADIALOG_H

#include "serveragent.h"

#include <QDialog>
#include <QProgressDialog>

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

private:
    Ui::ServerDataDialog *ui;

    QProgressDialog m_progressDialog;
    int m_pdMaximum;

    QDateTime m_downloadStartTime;

    void updateBookStatus(QString bookName);

    void initializeProgressDialog();
    void createProgressDialog(const QString &labelText, const QString &cancelButtonText);
    void destroyProgressDialog();

};

#endif // SERVERDATADIALOG_H
