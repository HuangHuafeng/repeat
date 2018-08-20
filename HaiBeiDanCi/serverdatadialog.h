#ifndef SERVERDATADIALOG_H
#define SERVERDATADIALOG_H

#include "serveragent.h"

#include <QDialog>

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
    void onBookListReady(const QList<QString> books);
    void onBookDownloaded(QString bookName);

    void on_pbDownloadBook_clicked();

    void on_pbTest_clicked();

private:
    Ui::ServerDataDialog *ui;

};

#endif // SERVERDATADIALOG_H
