#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"
#include "HaiBeiDanCi/mysettings.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QAction>

PreferencesDialog::PreferencesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PreferencesDialog)
{
    ui->setupUi(this);

    ui->leDataPath->setText(MySettings::dataDirectory());
}

PreferencesDialog::~PreferencesDialog()
{
    delete ui;
}

void PreferencesDialog::on_pbBrowse_clicked()
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
    QString oldDir = MySettings::dataDirectory();
    QString newDir = ui->leDataPath->text();
    if (newDir != oldDir)
    {
        MySettings::saveDataDirectory(newDir);

        QMessageBox::critical(this,
                              MySettings::appName(),
                              QObject::tr("Restart is required.\nClick OK to exit the app."));

        QAction *quitAct = new QAction(this);
        connect(quitAct, SIGNAL(triggered()), QCoreApplication::instance(), SLOT(quit()), Qt::QueuedConnection);
        quitAct->trigger();
    }
}
