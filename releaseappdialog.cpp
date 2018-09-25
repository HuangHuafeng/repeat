#include "releaseappdialog.h"
#include "ui_releaseappdialog.h"
#include "HaiBeiDanCi/mysettings.h"
#include "HaiBeiDanCi/serverclientprotocol.h"

#include <QFileDialog>
#include <QProgressDialog>

ReleaseAppDialog::ReleaseAppDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ReleaseAppDialog),
    m_sm(this),
    m_progressDialog(this)
{
    ui->setupUi(this);
    initializeProgressDialog();

    connect(&m_sm, SIGNAL(uploadProgress(float)), this, SLOT(onUploadProgress(float)));
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
    QString dd = MySettings::dataDirectory() + "/";
    QString fileName = ui->leFile->text();
    fileName = fileName.replace(dd, "");
    createProgressDialog("uploading \"" + fileName + "\" ...", QString());
    m_sm.uploadfile(fileName);
}

void ReleaseAppDialog::on_pbBrowse_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "Open File",
                                                    MySettings::dataDirectory() + "/releases",
                                                    "Compressed files (*.zip *.7z)");
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
    bool validVersion = ApplicationVersion::isValidVersion(ui->leVersion->text());
    bool validFileName = false;

    QString fileName = ui->leFile->text();
    QString dd = MySettings::dataDirectory();
    if (fileName.startsWith(dd) == true)
    {
        validFileName = true;
    }

    ui->pbRelease->setEnabled(validVersion && validFileName);
}

void ReleaseAppDialog::onUploadProgress(float percentage)
{
    m_progressDialog.setValue(static_cast<int>(100 * percentage));
}

void ReleaseAppDialog::initializeProgressDialog()
{
    m_progressDialog.setModal(true);
    m_progressDialog.cancel();
}

void ReleaseAppDialog::createProgressDialog(const QString &labelText, const QString &cancelButtonText)
{
    m_progressDialog.reset();
    m_progressDialog.setLabelText("    " + labelText + "    ");
    m_progressDialog.setCancelButtonText(cancelButtonText);
    m_progressDialog.setValue(0);
}
