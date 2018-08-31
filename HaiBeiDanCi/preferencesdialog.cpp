#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"
#include "mysettings.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QFontDialog>

PreferencesDialog::PreferencesDialog(QWidget *parent) : QDialog(parent),
                                                        ui(new Ui::PreferencesDialog)
{
    ui->setupUi(this);
    setWindowTitle(QObject::tr("Preferences"));

    loadCardSettings();
    loadDataSettings();
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
    saveCardSettings();
    bool restartNeeded = saveDataSettings();
    if (restartNeeded)
    {
        exitPreferencesDialog();
    }
}

bool PreferencesDialog::saveDataSettings()
{
    saveUpdateInterval();
    return saveDataDirectory();
}

bool PreferencesDialog::saveCardSettings()
{
    saveDefaultEasiness();
    saveCardDefaultInterval();
    saveCardMaximumInterval();
    savePerfectIncrease();
    saveCorrectIncrease();
    saveVagueDecrease();
    saveCardIntervalForIncorrect();
    return false;
}

void PreferencesDialog::exitPreferencesDialog()
{
    QMessageBox::critical(this,
                          MySettings::appName(),
                          QObject::tr("Restart is required.\nClick OK to exit the app."));

    QCoreApplication::instance()->quit();
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
    int oldInterval = MySettings::updateInterval();
    if (oldInterval == newInterval)
    {
        return false;
    }

    MySettings::saveUpdateInterval(newInterval);
    return true;
}

bool PreferencesDialog::saveDefaultEasiness()
{
    int newEasiness = ui->spinDefaultEasiness->value();
    int oldEasiness = static_cast<int>(MySettings::defaultEasiness() * 100);
    if (newEasiness == oldEasiness)
    {
        return false;
    }

    MySettings::saveDefaultEasiness(newEasiness / 100.0f);
    return true;
}

bool PreferencesDialog::saveCardDefaultInterval()
{
    int newCDI = ui->spinDefaultInterval->value();
    int oldCDI = MySettings::cardDefaultInterval();
    if (newCDI == oldCDI)
    {
        return false;
    }

    MySettings::saveCardDefaultInterval(newCDI);
    return true;
}

bool PreferencesDialog::saveCardMaximumInterval()
{
    int newCMI = ui->spinBoxMaximumInterval->value();
    int oldCMI = MySettings::cardMaximumInterval();
    if (newCMI == oldCMI)
    {
        return false;
    }

    MySettings::saveCardMaximumInterval(newCMI);
    return true;
}

bool PreferencesDialog::savePerfectIncrease()
{
    int newPI = ui->spinPerfectIncrease->value();
    int oldPI = static_cast<int>(MySettings::perfectIncrease() * 100);
    if (newPI == oldPI)
    {
        return false;
    }

    MySettings::savePerfectIncrease(newPI / 100.0f);
    return true;
}

bool PreferencesDialog::saveCorrectIncrease()
{
    int newCI = ui->spinCorrectIncrease->value();
    int oldCI = static_cast<int>(MySettings::correctIncrease() * 100);
    if (newCI == oldCI)
    {
        return false;
    }

    MySettings::saveCorrectIncrease(newCI / 100.0f);
    return true;
}

bool PreferencesDialog::saveVagueDecrease()
{
    int newVD = ui->spinVagueDecrease->value();
    int oldVD = static_cast<int>(MySettings::vagueDecrease() * 100);
    if (newVD == oldVD)
    {
        return false;
    }

    MySettings::saveVagueDecrease(newVD / 100.0f);
    return true;
}

bool PreferencesDialog::saveCardIntervalForIncorrect()
{
    int newIFI = ui->spinIntervalForIncorrect->value();
    int oldIFI = MySettings::cardIntervalForIncorrect();
    if (newIFI == oldIFI)
    {
        return false;
    }

    MySettings::saveCardIntervalForIncorrect(newIFI);
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

void PreferencesDialog::on_buttonBox_clicked(QAbstractButton *button)
{
    auto which = ui->buttonBox->standardButton(button);
    if (which == QDialogButtonBox::RestoreDefaults)
    {
        restoreDefaults();
    }
}

void PreferencesDialog::restoreDefaults()
{
    int result = 0;
    auto currentWidget = ui->tabWidget->currentWidget();
    if (currentWidget == ui->tabCard)
    {
        result = restoreCardSettings();
    }
    else if (currentWidget == ui->tabData)
    {
        result = restoreDataSettings();
    }

    if (result == 2)
    {
        close();
        exitPreferencesDialog();
    }
}

/**
 * @brief PreferencesDialog::restoreCardSettings
 * @return
 * 0    cancelled
 * 1    restored, no need to restart
 * 2    restored, a restart is required
 */
int PreferencesDialog::restoreCardSettings()
{
    if (QMessageBox::No == QMessageBox::warning(this,
                                                MySettings::appName(),
                                                QObject::tr("Restoring these settings takes effective immediately and cannot be undone!\n Do you want to continue?"),
                                                QMessageBox::Yes,
                                                QMessageBox::No))
    {
        return 0;
    }

    MySettings::restoreCardSettings();
    loadCardSettings();
    return 1;
}

/**
 * @brief PreferencesDialog::restoreDataSettings
 * @return
 * 0    cancelled
 * 1    restored, no need to restart
 * 2    restored, a restart is required
 */
int PreferencesDialog::restoreDataSettings()
{
    if (QMessageBox::No == QMessageBox::warning(this,
                                                MySettings::appName(),
                                                QObject::tr("Restoring these settings takes effective immediately and cannot be undone!\n Do you want to continue?"),
                                                QMessageBox::Yes,
                                                QMessageBox::No))
    {
        return 0;
    }

    int result = 2;
    QString dirBeforeRestore = MySettings::dataDirectory();
    MySettings::restoreDataSettings();
    QString dirAfterRestore = MySettings::dataDirectory();
    loadDataSettings();

    if (dirAfterRestore == dirBeforeRestore)
    {
        result = 1;
    }
    else
    {
        // data directory changed, need a restart
        result = 2;
    }

    return result;
}

void PreferencesDialog::loadCardSettings()
{
    ui->spinDefaultEasiness->setRange(130, 800);
    int de = static_cast<int>(MySettings::defaultEasiness() * 100);
    ui->spinDefaultEasiness->setValue(de);

    ui->spinDefaultInterval->setRange(1, 365 * 100);
    int di = MySettings::cardDefaultInterval();
    ui->spinDefaultInterval->setValue(di);

    ui->spinBoxMaximumInterval->setRange(1, 365 * 100);
    int mi = MySettings::cardMaximumInterval();
    ui->spinBoxMaximumInterval->setValue(mi);

    int pi = static_cast<int>(MySettings::perfectIncrease() * 100);
    ui->spinPerfectIncrease->setValue(pi);

    int ci = static_cast<int>(MySettings::correctIncrease() * 100);
    ui->spinCorrectIncrease->setValue(ci);

    int vd = static_cast<int>(MySettings::vagueDecrease() * 100);
    ui->spinVagueDecrease->setValue(vd);

    ui->spinIntervalForIncorrect->setRange(1, 60 * 24 * di / 2);
    int ii = MySettings::cardIntervalForIncorrect();
    ui->spinIntervalForIncorrect->setValue(ii);
}

void PreferencesDialog::loadDataSettings()
{
    ui->leDataPath->setText(MySettings::dataDirectory());
    ui->spinInfoUpdate->setValue(MySettings::updateInterval());

    updateCurrentFontInfo();
}

void PreferencesDialog::updateCurrentFontInfo()
{
    QFont currentFont = QApplication::font();
    QString fontInfo = currentFont.family();
    ui->leCurrentFont->setText(fontInfo);
}

void PreferencesDialog::on_pushChangeFont_clicked()
{
    bool ok;
    QFont currentFont = QApplication::font();
    ui->leCurrentFont->setText(currentFont.family());
    QFont font = QFontDialog::getFont(
                    &ok, currentFont, this);
    if (ok) {
        QApplication::setFont(font);
        updateCurrentFontInfo();
        MySettings::saveApplicationFont(font);
    }
}
