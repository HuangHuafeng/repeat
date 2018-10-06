#include "autoupgrader.h"
#include "mysettings.h"
#include "../Upgrader/upgradedata.h"

#include <QCoreApplication>
#include <QFile>
#include <QtDebug>
#include <QProcess>
#include <JlCompress.h>

AutoUpgrader::AutoUpgrader()
{
    createRunningFile();

    QString fileName = runningFile();
    UpgradeData ud(QCoreApplication::applicationName());
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

bool AutoUpgrader::startUpgrader(QStringList arguments)
{
    if (arguments.isEmpty() == true)
    {
        arguments << QCoreApplication::applicationName();
    }

    UpgradeData ud(QCoreApplication::applicationName());
    return ud.startUpgrader(arguments);
}

ApplicationVersion AutoUpgrader::upgraderVersion()
{
    UpgradeData ud(QCoreApplication::applicationName());
    QString ufp = ud.upgraderFilePath();
    if (ufp.isEmpty() == true)
    {
        // no upgrader yet!
        return ApplicationVersion(0, 0, 0);
    }

    QStringList arguments;
    arguments << "--version";
    QString workingDirectory = ufp.section('/', 0, -2);
    QProcess upgrader;
    upgrader.setWorkingDirectory(workingDirectory);
    upgrader.start(ufp, arguments);
    if (upgrader.waitForReadyRead() == false)
    {
        qCritical() << "failed to run upgrader to get version information.";
        upgrader.waitForFinished();
        return ApplicationVersion(0, 0, 0);
    }
    upgrader.waitForFinished();

    char version[20];
    if (upgrader.readLine(version, 20) <= 0)
    {
        qCritical() << "failed to read the output from the upgrader process.";
        return ApplicationVersion(0, 0, 0);
    }

    QString versionInString(version);
    versionInString.remove(QChar('\r'));
    versionInString.remove(QChar('\n'));

    qDebug() << "upgrader version:" << ApplicationVersion::fromString(versionInString).toString();

    return ApplicationVersion::fromString(versionInString);
}

/**
 * @brief AutoUpgrader::newVersionAvailable
 * @param version
 * @param fileName  full path to the zip file that contains the new version
 */
void AutoUpgrader::newVersionAvailable(ApplicationVersion version, QString fileName)
{
    UpgradeData ud(QCoreApplication::applicationName());

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

void AutoUpgrader::newUpgraderDownloaded(ApplicationVersion version, QString fileName)
{
    qDebug() << version.toString();

    QString dd = MySettings::dataDirectory();
    QString extractDir;
    UpgradeData ud(QCoreApplication::applicationName());
    // ALWAYS store the upgrader to the data directory since we have a new version
    ud.saveUpgraderFilePath(hardcodedUpgraderFilePath());
    extractDir = MySettings::dataDirectory();

    /*
    QString ufp = ud.upgraderFilePath();
    if (ufp.startsWith(dd) == true)
    {
        // overwrite the current upgrader!
#ifdef Q_OS_WIN
        extractDir = ufp.section('/', 0, -2);
#elif defined(Q_OS_MACOS)
        extractDir = ufp.section('/', 0, -5);
#elif defined(Q_OS_LINUX)
    ;
#else
#error "We don't support the platform yet..."
#endif
    }
    else
    {
        extractDir = MySettings::dataDirectory();
        // in this case, we want the upgrader to be in the data directory
        ud.saveUpgraderFilePath(hardcodedUpgraderFilePath());
    }
    */

    qDebug() << "extracting upgrader to" << extractDir;
    auto extractedFiles = JlCompress::extractDir(fileName, extractDir);
    if (extractedFiles.isEmpty() == true)
    {
        qCritical() << "failed to extract" << fileName;
    }
    else
    {
        qDebug() << extractedFiles;
    }
}

QString AutoUpgrader::hardcodedUpgraderFilePath()
{
    QString ufp;
#ifdef Q_OS_WIN
        ufp = MySettings::dataDirectory() + "/Upgrader/ConsoleUpgrader.exe";
#elif defined(Q_OS_MACOS)
        ufp = MySettings::dataDirectory() + "/ConsoleUpgrader.app/Contents/MacOS/ConsoleUpgrader";
#elif defined(Q_OS_LINUX)
        ;
#else
#error "We don't support the platform yet..."
#endif

    return ufp;
}

