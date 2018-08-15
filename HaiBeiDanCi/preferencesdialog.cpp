#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"
#include "mysettings.h"

#include <QFileDialog>
#include <QMessageBox>

PreferencesDialog::PreferencesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PreferencesDialog)
{
    ui->setupUi(this);
    setWindowTitle(QObject::tr("Preferences"));

    ui->leDataPath->setText(MySettings::dataDirectory());
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

    if (dataDirUpdated)
    {
        QMessageBox::critical(this,
                              MySettings::appName(),
                              QObject::tr("Please restart the app to make the changes effect"));
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
