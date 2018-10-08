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
        arguments << "--target" << QCoreApplication::applicationName();
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

    QString versionFile = QCoreApplication::applicationDirPath() + "/UpgraderVersion.txt";
    QStringList arguments;
    arguments << "--version" << versionFile;
    QString workingDirectory = ufp.section('/', 0, -2);
    QProcess upgrader;
    upgrader.setWorkingDirectory(workingDirectory);
    upgrader.start(ufp, arguments);
    if (upgrader.waitForFinished() == false)
    {
        qCritical() << "failed to run upgrader to get version information.";
        return ApplicationVersion(0, 0, 0);
    }

    char version[20];
    QFile vf(versionFile);
    if (vf.open(QIODevice::Text | QIODevice::ReadWrite) == false
            || vf.readLine(version, 20) <= 0)
    {
        qCritical() << "failed to read version from" << versionFile;
        return ApplicationVersion(0, 0, 0);
    }
    if (vf.remove() == false)
    {
        qCritical() << "failed to remove file" << versionFile;
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
void AutoUpgrader::newAppDownloaded(ApplicationVersion version, QStringList zipFiles)
{
    UpgradeData ud(QCoreApplication::applicationName());

    if (ud.dataDirectory().isEmpty() == true)
    {
        ud.saveDataDirectory(MySettings::dataDirectory());
    }

    QString extractDir = QCoreApplication::applicationDirPath();

#ifdef Q_OS_WIN
    // we should extract the files to QCoreApplication::applicationDirPath() directly
    // becase we don't know if the user has changed the name of the directory or not
    // this requires compressing the conents in "release" when releasing Windows version
    // not the "release' folder
#elif defined(Q_OS_MACOS)
    // it's possible that user rename HaiBeiDanCi.app to a different name
    // so we should exact the files to QCoreApplication::applicationDirPath()/../../
    // which contains the folder "Contents"
    extractDir = extractDir.section('/', 0, -3);
#elif defined(Q_OS_LINUX)
    ;
#else
#error "We don't support the platform yet..."
#endif

    // tell the upgrader where is the zip file and where to extract to
    ud.saveUpgradeData(version, zipFiles, extractDir);
    qDebug() << version.toString() << zipFiles << extractDir;
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
        ufp = MySettings::dataDirectory() + "/Upgrader/Upgrader.exe";
#elif defined(Q_OS_MACOS)
        ufp = MySettings::dataDirectory() + "/Upgrader.app/Contents/MacOS/Upgrader";
#elif defined(Q_OS_LINUX)
        ;
#else
#error "We don't support the platform yet..."
#endif

    return ufp;
}

