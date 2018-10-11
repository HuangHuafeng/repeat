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
    void on_actionCheckInput_triggered();

    void on_pbCancel_clicked();

    void on_cbName_toggled(bool checked);

    void on_pbLogin_clicked();

private:
    Ui::LoginDialog *ui;

    void saveNameAndPassword();
    void loadNameAndPassword();

    void onLoginSucceeded(const ApplicationUser &user, const Token &token);
    void onLoginFailed(QString why);
};

#endif // LOGINDIALOG_H
