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

    connect(&m_sua, SIGNAL(registerSucceeded(const ApplicationUser &)), this, SLOT(onRegisterSucceeded(const ApplicationUser &)));
    connect(&m_sua, SIGNAL(registerFailed(QString)), this, SLOT(onRegisterFailed(QString)));
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

    m_sua.registerUser(name, password, email);
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
