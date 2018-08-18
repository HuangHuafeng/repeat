#ifndef SERVERDATADIALOG_H
#define SERVERDATADIALOG_H

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

private:
    Ui::ServerDataDialog *ui;
};

#endif // SERVERDATADIALOG_H
