#include "upgradedata.h"

#include <QCoreApplication>
#include <QFile>
#include <QtDebug>
#include <QSettings>
#include <QProcess>

#define SETTING(x)  QSettings x("AniujSoft", "Upgrader")

UpgradeData::UpgradeData(QString target)
{
    setTarget(target);
}

bool UpgradeData::startTarget()
{
    QString tsc = targetStartCommand();
    QString workingDirectory = tsc.section('/', 0, -2);
    bool result = QProcess::startDetached(tsc,
                                          QStringList(),
                                          workingDirectory);
    if (result == false)
    {
        qCritical() << "failed to start the target:" << tsc;
    }
    return result;
}

void UpgradeData::setTarget(QString target)
{
    if (target.isEmpty() == true)
    {
        // don't allow m_target to be empty!
        target = "UpgraderTarget";
    }

    if (target != m_target)
    {
        m_target = target;
    }
}

QString UpgradeData::getTarget()
{
    return m_target;
}

bool UpgradeData::hasUpgradeData()
{
    QString fileName = dataFile();
    return QFile::exists(fileName);
}

void UpgradeData::saveUpgraderFilePath(QString ufp)
{
    SETTING(settings);
    settings.beginGroup(m_target);
    settings.setValue("upgraderFilePath", ufp);
    settings.endGroup();
}

QString UpgradeData::upgraderFilePath()
{
    SETTING(settings);
    settings.beginGroup(m_target);
    QString ufp = settings.value("upgraderFilePath", "").toString();
    settings.endGroup();

    return ufp;
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
    SETTING(settings);
    settings.beginGroup(m_target);
    settings.setValue("dataDirectory", newDir);
    settings.endGroup();
}

QString UpgradeData::dataDirectory()
{
    SETTING(settings);
    settings.beginGroup(m_target);
    QString dd = settings.value("dataDirectory", "").toString();
    settings.endGroup();

    return dd;
}

const QString UpgradeData::dataFile()
{
    return UpgradeData::dataDirectory() + "/" + "upgrader.data";
}

void UpgradeData::saveTargetStartCommand(QString command)
{
    SETTING(settings);
    settings.beginGroup(m_target);
    settings.setValue("targetStartCommand", command);
    settings.endGroup();
}

QString UpgradeData::targetStartCommand()
{
    SETTING(settings);
    settings.beginGroup(m_target);
    QString tsc = settings.value("targetStartCommand", "").toString();
    settings.endGroup();

    return tsc;
}

/**
 * @brief saveTargetRunningFile
 * @param fileName
 * if file "fileName" exists, the target is running
 */
void UpgradeData::saveTargetRunningFile(QString fileName)
{
    SETTING(settings);
    settings.beginGroup(m_target);
    settings.setValue("targetRunningFile", fileName);
    settings.endGroup();
}

QString UpgradeData::targetRunningFile()
{
    SETTING(settings);
    settings.beginGroup(m_target);
    QString tsc = settings.value("targetRunningFile", "").toString();
    settings.endGroup();

    return tsc;
}

