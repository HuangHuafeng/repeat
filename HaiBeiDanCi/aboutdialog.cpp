#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include "mysettings.h"

#include <QDateTime>

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

    auto icon = QIcon(":/HaiBeiDanCi.ico");
    ui->lableIcon->setPixmap(icon.pixmap(128, 128));

    ui->labelText->setText(aboutText());

    setFixedSize(400, 200);
    setMyTitle();
}

AboutDialog::~AboutDialog()
{
    delete ui;
}


QString AboutDialog::aboutText()
{
    const QString compileTime = QString(__DATE__) + " " + __TIME__;
    QString text = "<b>" + MySettings::appName() + " " + APP_VERSION + "</b>";
    text += "<br />";
    text += "<br />";
    text += QObject::tr("Based on ") + "Qt 5.11.1";
    text += "<br />";
    text += "<br />";
    text += QObject::tr("Built on ") + compileTime;


    return text;
}

void AboutDialog::on_pbOK_clicked()
{
    close();
}

void AboutDialog::setMyTitle()
{
    QString title = QObject::tr("About");
    setWindowTitle(title);
}
