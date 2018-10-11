#include "registerdialog.h"
#include "ui_registerdialog.h"
#include "mysettings.h"

#include <QMessageBox>

RegisterDialog::RegisterDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RegisterDialog)
{
    ui->setupUi(this);
}

RegisterDialog::~RegisterDialog()
{
    delete ui;
}

void RegisterDialog::on_pbCancel_clicked()
{
    reject();
}

void RegisterDialog::on_pbRegister_clicked()
{
    QString name = ui->leName->text();
    QString password = ui->lePassword->text();
    QString email = ui->leEmail->text();

    ServerUserAgent *sua = new ServerUserAgent();
    connect(sua, &ServerUserAgent::registerResult, [sua, this] (bool succeeded, const ApplicationUser &user, QString errorText) {
        if (succeeded == true)
        {
            this->onRegisterSucceeded(user);
        }
        else
        {
            this->onRegisterFailed(errorText);
        }
        sua->deleteLater();
    });
    sua->registerUser(name, password, email);
}

void RegisterDialog::onRegisterSucceeded(const ApplicationUser &user)
{
    QMessageBox::information(this, MySettings::appName(), "\"" + user.name() +"\"" + QObject::tr(" registered successfully!"));
    Q_ASSERT(ApplicationUser::userExist(user.name()) == false);
    user.saveRegisteredUser(user);
    accept();
}

void RegisterDialog::onRegisterFailed(QString why)
{
    QMessageBox::critical(this, MySettings::appName(), why);
}

void RegisterDialog::on_actionCheckInput_triggered()
{
    QString name = ui->leName->text();
    QString password = ui->lePassword->text();
    QString email = ui->leEmail->text();

    auto nameRE = MySettings::namePattern();
    auto passwordRE = MySettings::passwordPattern();
    auto emailRE = MySettings::emailPattern();
    if (nameRE.match(name).hasMatch()
            && passwordRE.match(password).hasMatch()
            && emailRE.match(email).hasMatch())
    {
        ui->pbRegister->setEnabled(true);
    }
    else
    {
        ui->pbRegister->setEnabled(false);
    }
}
