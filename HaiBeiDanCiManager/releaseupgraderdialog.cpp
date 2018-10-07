#include "releaseupgraderdialog.h"
#include "ui_releaseupgraderdialog.h"
#include "../HaiBeiDanCi/mysettings.h"
#include "../HaiBeiDanCi/applicationversion.h"

#include <QFileDialog>
#include <QMessageBox>

ReleaseUpgraderDialog::ReleaseUpgraderDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ReleaseUpgraderDialog)
{
    ui->setupUi(this);
    initializeProgressDialog();

    connect(&m_sm, SIGNAL(uploadProgress(float)), this, SLOT(onUploadProgress(float)));
    connect(&m_sm, SIGNAL(fileUploaded(QString)), this, SLOT(onFileUploaded(QString)));
    connect(&m_sm, SIGNAL(upgraderReleased(bool)), this, SLOT(onUpgraderReleased(bool)));
}

ReleaseUpgraderDialog::~ReleaseUpgraderDialog()
{
    delete ui;
}

void ReleaseUpgraderDialog::initializeProgressDialog()
{
    m_progressDialog.setModal(true);
    m_progressDialog.cancel();
}

void ReleaseUpgraderDialog::createProgressDialog(const QString &labelText, const QString &cancelButtonText)
{
    m_progressDialog.reset();
    m_progressDialog.setLabelText("    " + labelText + "    ");
    m_progressDialog.setCancelButtonText(cancelButtonText);
    m_progressDialog.setValue(0);
}

void ReleaseUpgraderDialog::on_pbBrowse_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "Open File",
                                                    MySettings::dataDirectory() + "/releases",
                                                    "Compressed files (*.zip *.7z)");
    if (fileName.isNull() == false)
    {
        ui->leFile->setText(fileName);
        validateReleaseParameters();
    }
}

void ReleaseUpgraderDialog::validateReleaseParameters()
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

void ReleaseUpgraderDialog::on_leVersion_textChanged(const QString &arg1)
{
    Q_ASSERT(arg1.size() >= 0);
    validateReleaseParameters();
}

void ReleaseUpgraderDialog::on_pbCancel_clicked()
{
    reject();
}

void ReleaseUpgraderDialog::on_pbRelease_clicked()
{
    // RELEASE Upgrader
    // step 1: upload the upgrader zip file
    QString dd = MySettings::dataDirectory() + "/";
    QString fileName = ui->leFile->text();
    fileName = fileName.replace(dd, "");
    createProgressDialog("uploading \"" + fileName + "\" ...", QString());
    m_sm.uploadfile(fileName);
}

void ReleaseUpgraderDialog::onFileUploaded(QString fileName)
{
    // RELEASE APP
    // step 2: update the released app info in server
    ApplicationVersion appVer = ApplicationVersion::fromString(ui->leVersion->text());
    QString platform = ui->cbPlatform->currentText();
    m_sm.releaseUpgrader(appVer, platform, fileName);
}

void ReleaseUpgraderDialog::onUpgraderReleased(bool succeed)
{
    // RELEASE APP
    // step 3: inform the result with a message box

    if (succeed)
    {
        QMessageBox::information(this,
                                 MySettings::appName(),
                                 QObject::tr("Upgrader relese succeeded!"));
        accept();
    }
    else
    {
        QMessageBox::critical(this,
                                 MySettings::appName(),
                                 QObject::tr("Upgrader relese failed!"));
    }
}

void ReleaseUpgraderDialog::onUploadProgress(float percentage)
{
    m_progressDialog.setValue(static_cast<int>(100 * percentage));
}
