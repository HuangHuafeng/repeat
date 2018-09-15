#include "registerdialog.h"
#include "ui_registerdialog.h"
#include "serverdatadownloader.h"
#include "mysettings.h"

#include <QMessageBox>

RegisterDialog::RegisterDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RegisterDialog),
    m_sua(this)
{
    ui->setupUi(this);

    connect(&m_sua, SIGNAL(registerSucceed(const ApplicationUser &)), this, SLOT(onRegisterSucceed(const ApplicationUser &)));
    connect(&m_sua, SIGNAL(registerFailed(QString)), this, SLOT(onRegisterFailed(QString)));
}

RegisterDialog::~RegisterDialog()
{
    delete ui;
}

void RegisterDialog::on_pbCancel_clicked()
{
    close();
}

void RegisterDialog::on_pbRegister_clicked()
{
    QString name = ui->leName->text();
    QString password = ui->lePassword->text();
    QString email = ui->leEmail->text();

    m_sua.registerUser(name, password, email);
}

void RegisterDialog::onRegisterSucceed(const ApplicationUser &user)
{
    QMessageBox::information(this, MySettings::appName(), QObject::tr("register succeeded!") + " " + user.name());
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
