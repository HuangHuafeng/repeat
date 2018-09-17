#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include "serveruseragent.h"

#include <QDialog>

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();

private slots:
    void onLoginSucceed(const ApplicationUser &user);
    void onLoginFailed(QString why);

    void on_actionCheckInput_triggered();

    void on_pbCancel_clicked();

    void on_cbName_toggled(bool checked);

    void on_pbLogin_clicked();

private:
    Ui::LoginDialog *ui;

    ServerUserAgent m_sua;

    void saveNameAndPassword();
    void loadNameAndPassword();
};

#endif // LOGINDIALOG_H
