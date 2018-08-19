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
    void onResponseGetAllBooks(QList<QString> books);

private:
    Ui::ServerDataDialog *ui;

    ServerAgent m_serverAgent;
};

#endif // SERVERDATADIALOG_H
