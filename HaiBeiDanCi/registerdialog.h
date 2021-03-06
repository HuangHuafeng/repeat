#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include "serveruseragent.h"
#include "applicationuser.h"
#include "serveruseragent.h"

#include <QDialog>

namespace Ui {
class RegisterDialog;
}

class RegisterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RegisterDialog(QWidget *parent = nullptr);
    ~RegisterDialog();

private slots:
    void on_pbCancel_clicked();

    void on_pbRegister_clicked();

    void on_actionCheckInput_triggered();

private:
    Ui::RegisterDialog *ui;

    void onRegisterSucceeded(const ApplicationUser &user);
    void onRegisterFailed(QString why);
};

#endif // REGISTERDIALOG_H
