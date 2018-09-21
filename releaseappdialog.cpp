#include "releaseappdialog.h"
#include "ui_releaseappdialog.h"
#include "HaiBeiDanCi/mysettings.h"
#include "HaiBeiDanCi/serverclientprotocol.h"

#include <QFileDialog>
#include <QProgressDialog>

ReleaseAppDialog::ReleaseAppDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ReleaseAppDialog)
{
    ui->setupUi(this);
}

ReleaseAppDialog::~ReleaseAppDialog()
{
    delete ui;
}

void ReleaseAppDialog::on_pbCancel_clicked()
{
    reject();
}

void ReleaseAppDialog::on_pbRelease_clicked()
{
    ;
}

void ReleaseAppDialog::on_pbBrowse_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open Word File", MySettings::dataDirectory());
    ui->leFile->setText(fileName);
    validateReleaseParameters();
}

void ReleaseAppDialog::on_leVersion_textChanged(const QString &arg1)
{
    Q_ASSERT(arg1.size() >= 0);
    validateReleaseParameters();
}

void ReleaseAppDialog::validateReleaseParameters()
{
    QString version = ui->leVersion->text();
    QString file = ui->leFile->text();

    ui->pbRelease->setEnabled(ApplicationVersion::isValidVersion(version)
                              && file.isEmpty() == false);
}
