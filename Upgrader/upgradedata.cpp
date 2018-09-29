#include "upgradedata.h"

#include <QCoreApplication>
#include <QFile>
#include <QtDebug>
#include <QSettings>

UpgradeData::UpgradeData()
{

}

bool UpgradeData::hasUpgradeData()
{
    QString fileName = dataFile();
    return QFile::exists(fileName);
}

bool UpgradeData::saveUpgradeData(const ApplicationVersion &version, QString zipFile, QString extractDir)
{
    QString fileName = dataFile();
    QFile dataFile(fileName);
    if (dataFile.open(QFile::WriteOnly) == false)
    {
        qCritical() << "failed to write file" << fileName;
        return false;
    }

    QDataStream ds(&dataFile);
    ds << version << zipFile << extractDir;

    dataFile.close();

    return true;
}

bool UpgradeData::getUpgradeData(ApplicationVersion &version, QString &zipFile, QString &extractDir)
{
    QString fileName = dataFile();
    Q_ASSERT(QFile::exists(fileName) == true);

    QFile dataFile(fileName);
    if (dataFile.open(QFile::ReadOnly) == false)
    {
        qCritical() << "failed to open file" << fileName;
        return false;
    }

    QDataStream ds(&dataFile);
    ds >> version >> zipFile >> extractDir;

    dataFile.close();

    return true;
}

void UpgradeData::saveDataDirectory(QString newDir)
{
    QSettings settings("AniujSoft", "Upgrader");
    settings.beginGroup("data");
    settings.setValue("dataDirectory", newDir);
    settings.endGroup();
}

QString UpgradeData::dataDirectory()
{
    QSettings settings("AniujSoft", "Upgrader");
    settings.beginGroup("data");
    QString dd = settings.value("dataDirectory", "").toString();
    settings.endGroup();

    return dd;
}

const QString UpgradeData::dataFile()
{
    return UpgradeData::dataDirectory() + "/" + "upgrader.data";
}
