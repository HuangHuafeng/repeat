#include "logindialog.h"
#include "ui_logindialog.h"
#include "mysettings.h"

#include <QSettings>
#include <QMessageBox>

LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog)
{
    ui->setupUi(this);

    connect(&m_sua, SIGNAL(loginSucceed(const ApplicationUser &, const Token &)), this, SLOT(onLoginSucceed(const ApplicationUser &, const Token &)));
    connect(&m_sua, SIGNAL(loginFailed(QString)), this, SLOT(onLoginFailed(QString)));

    loadNameAndPassword();
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

void LoginDialog::on_actionCheckInput_triggered()
{
    QString name = ui->leName->text();
    QString password = ui->lePassword->text();

    auto nameRE = MySettings::namePattern();
    auto passwordRE = MySettings::passwordPattern();
    if (nameRE.match(name).hasMatch()
            && passwordRE.match(password).hasMatch())
    {
        ui->pbLogin->setEnabled(true);
    }
    else
    {
        ui->pbLogin->setEnabled(false);
    }
}

void LoginDialog::on_pbCancel_clicked()
{
    reject();
}

void LoginDialog::on_cbName_toggled(bool checked)
{
    if (checked == false)
    {
        ui->cbPassword->setCheckState(Qt::Unchecked);
    }

    ui->cbPassword->setEnabled(checked);
}

void LoginDialog::saveNameAndPassword()
{
    QSettings settings;
    settings.beginGroup("LoginDialog");

    if (ui->cbName->checkState() == Qt::Checked)
    {
        settings.setValue("name", ui->leName->text());
    }
    else
    {
        settings.remove("name");
    }

    if (ui->cbPassword->checkState() == Qt::Checked)
    {
        settings.setValue("password", ui->lePassword->text());
    }
    else
    {
        settings.remove("password");
    }

    settings.setValue("cbName", ui->cbName->checkState());
    settings.setValue("cbPassword", ui->cbPassword->checkState());

    settings.endGroup();
}

void LoginDialog::loadNameAndPassword()
{
    QSettings settings;
    settings.beginGroup("LoginDialog");
    int nameState = settings.value("cbName", Qt::Unchecked).toInt();
    int passwordState = settings.value("cbPassword", Qt::Unchecked).toInt();
    if (nameState == Qt::Unchecked)
    {
        ui->cbName->setCheckState(Qt::Unchecked);
        ui->cbPassword->setCheckState(Qt::Unchecked);
        ui->cbPassword->setEnabled(false);
    }
    else
    {
        ui->cbName->setCheckState(Qt::Checked);
        if (passwordState == Qt::Unchecked)
        {
            ui->cbPassword->setCheckState(Qt::Unchecked);
        }
        else
        {
            ui->cbPassword->setCheckState(Qt::Checked);
        }
    }

    QString name = settings.value("name", "").toString();
    QString password = settings.value("password", "").toString();
    ui->leName->setText(name);
    ui->lePassword->setText(password);

    settings.endGroup();
}

void LoginDialog::on_pbLogin_clicked()
{
    saveNameAndPassword();
    m_sua.loginUser(ui->leName->text(), ui->lePassword->text());
}

void LoginDialog::onLoginSucceed(const ApplicationUser &user, const Token &token)
{
    QMessageBox::information(this, MySettings::appName(), "\"" + user.name() +"\"" + QObject::tr(" logged in successfully!"));
    Q_ASSERT(ApplicationUser::userExist(user.name()) != false);
    MessageHeader::setStoredTokenId(token.id());
    //qDebug() << token.id() << token.createTime() << token.lifeInSeconds() << token.peerAddress();
    accept();
}

void LoginDialog::onLoginFailed(QString why)
{
    QMessageBox::critical(this, MySettings::appName(), why);
}
