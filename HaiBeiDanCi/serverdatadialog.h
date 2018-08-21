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
    //void onWordDownloaded(QString spelling);

    void on_pbDownloadBook_clicked();

    void on_pbClose_clicked();

private:
    Ui::ServerDataDialog *ui;

    QProgressDialog *m_pd;

    void updateBookStatus(QString bookName);

};

#endif // SERVERDATADIALOG_H
