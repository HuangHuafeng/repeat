#include "upgrader.h"
#include "ui_upgrader.h"

#include <JlCompress.h>
#include <QtDebug>

Upgrader::Upgrader(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Upgrader)
{
    ui->setupUi(this);

    Q_ASSERT(UpgradeData::hasUpgradeData() == true);
    ApplicationVersion version(0, 0, 0);
    QString zipFile, extractDir;
    if (UpgradeData::getUpgradeData(version, zipFile, extractDir) == true)
    {
        ui->label->setText(QObject::tr("Uncompressing version %1 ...").arg(version.toString()));
        qDebug() << "extracting" << zipFile << "to" << extractDir;
        JlCompress::extractDir(zipFile, extractDir);
    }
}

Upgrader::~Upgrader()
{
    delete ui;
}
