#include "autoupgrader.h"
#include "mysettings.h"
#include "../Upgrader/upgradedata.h"

#include <QCoreApplication>
#include <QFile>
#include <QtDebug>
#include <QProcess>

AutoUpgrader::AutoUpgrader()
{
    createRunningFile();

    QString fileName = runningFile();
    UpgradeData ud(MySettings::appName());
    if (ud.targetRunningFile() != fileName)
    {
        ud.saveTargetRunningFile(fileName);
    }

    if (ud.targetStartCommand() != QCoreApplication::applicationFilePath())
    {
        // tell the upgrader how to start me
        ud.saveTargetStartCommand(QCoreApplication::applicationFilePath());
    }
}

AutoUpgrader::~AutoUpgrader()
{
    removeRunningFile();
    qDebug() << "~AutoUpgrader()";
}

QString AutoUpgrader::runningFile()
{
    return QCoreApplication::applicationFilePath() + ".running";
}

void AutoUpgrader::createRunningFile()
{
    QString fileName = runningFile();
    QFile rf(fileName);
    if (rf.open(QIODevice::NewOnly | QIODevice::Text | QIODevice::WriteOnly) == false)
    {
        qCritical() << "failed to open" << fileName;
        return;
    }

    QTextStream out(&rf);
    out << QCoreApplication::applicationFilePath() << "\n";
    out << "pid: " << QCoreApplication::applicationPid() << "\n";
    rf.close();
}

void AutoUpgrader::removeRunningFile()
{
    QString fileName = runningFile();
    if (QFile::exists(fileName) == true)
    {
        if (QFile::remove(fileName) == false)
        {
            qCritical() << "failed to remove" << fileName;
        }
    }
}

bool AutoUpgrader::startUpgrader()
{
    UpgradeData ud(MySettings::appName());
    QStringList arguments;
    arguments << MySettings::appName();
    return ud.startUpgrader(arguments);
}

/**
 * @brief AutoUpgrader::newVersionAvailable
 * @param version
 * @param fileName  full path to the zip file that contains the new version
 */
void AutoUpgrader::newVersionAvailable(ApplicationVersion version, QString fileName)
{
    UpgradeData ud(MySettings::appName());

    if (ud.dataDirectory().isEmpty() == true)
    {
        ud.saveDataDirectory(MySettings::dataDirectory());
    }

    QString extractDir = QCoreApplication::applicationDirPath();

#ifdef Q_OS_WIN
    ;
#elif defined(Q_OS_MACOS)
    if (extractDir.contains("Contents/MacOS") == true)
    {
        // should be the folder where HaiBeiDanCi.app locates
        extractDir = extractDir.section('/', 0, -4);
    }
#elif defined(Q_OS_LINUX)
    ;
#else
#error "We don't support the platform yet..."
#endif

    // tell the upgrader where is the zip file and where to extract to
    ud.saveUpgradeData(version, fileName, extractDir);
    qDebug() << version.toString() << fileName << extractDir;
}
