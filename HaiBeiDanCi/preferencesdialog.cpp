#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"
#include "mysettings.h"

#include <QFileDialog>
#include <QMessageBox>

PreferencesDialog::PreferencesDialog(QWidget *parent) : QDialog(parent),
                                                        ui(new Ui::PreferencesDialog)
{
    ui->setupUi(this);
    setWindowTitle(QObject::tr("Preferences"));

    ui->leDataPath->setText(MySettings::dataDirectory());
    ui->spinInfoUpdate->setValue(MySettings::updateInterval());
}

PreferencesDialog::~PreferencesDialog()
{
    delete ui;
}

void PreferencesDialog::on_pushBrowse_clicked()
{
    QString curDir = MySettings::dataDirectory();
    QString newDir = QFileDialog::getExistingDirectory(this,
                                                       PreferencesDialog::tr("Choose Data Directory"),
                                                       curDir,
                                                       QFileDialog::ShowDirsOnly);
    if (newDir.isEmpty() == false)
    {
        ui->leDataPath->setText(newDir);
    }
}

void PreferencesDialog::on_buttonBox_accepted()
{
    bool dataDirUpdated = saveDataDirectory();
    saveUpdateInterval();

    if (dataDirUpdated)
    {
        QMessageBox::critical(this,
                              MySettings::appName(),
                              QObject::tr("Restart is required.\nClick OK to exit the app."));

        QCoreApplication::instance()->quit();
    }
}

/**
 * @brief PreferencesDialog::saveDataDirectory
 * @return
 * true if the data is different and updated, thus a restart is needed
 * false if nothing happened
 */
bool PreferencesDialog::saveDataDirectory()
{
    QString oldDir = MySettings::dataDirectory();
    QString newDir = ui->leDataPath->text();
    if (oldDir == newDir)
    {
        return false;
    }

    MySettings::saveDataDirectory(newDir);
    return true;
}

bool PreferencesDialog::saveUpdateInterval()
{
    int newInterval = ui->spinInfoUpdate->value();
    MySettings::saveUpdateInterval(newInterval);
    return true;
}

void PreferencesDialog::on_pushUpdateInfo_clicked()
{
    auto s = MySettings::instance();
    if (s != nullptr)
    {
        s->updateInfoFileNow();
        QMessageBox::information(this, MySettings::appName(), QObject::tr("Info file update!"));
    }
}
