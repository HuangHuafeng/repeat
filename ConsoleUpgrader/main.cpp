#include "../Upgrader/upgradedata.h"

#include <QCoreApplication>
#include <QTextStream>
#include <QtDebug>
#include <QProcess>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QTextStream out(stdout);
    if (argc < 2)
    {
        QString appName = QCoreApplication::applicationName();
        out << "usage: " << appName << " target" << endl;
        out << "target is the name of the app to be upgraded." << endl;
        out << "usage: " << appName << " --version to print version" << endl;
        return 0;
    }

    if (QString::compare(argv[1], "--version") == 0)
    {
        out << APP_VERSION << endl;
        return 0;
    }

    // argv[1] is considered to be the target
    UpgradeData ud(argv[1]);
    if (ud.hasUpgradeData() == false)
    {
        out << "No upgrade data. Start the target and exit." << endl;
        ud.startTarget();
        return 0;
    }
    else
    {
        out << "New version available. Start the upgrader." << endl;

        // Upgrader is in the same folder where ConsoleUpgrader locates
        QStringList arguments;
        arguments << argv[1];

        QString ufp;
#ifdef Q_OS_WIN
        ufp = QCoreApplication::applicationDirPath() + "/Upgrader.exe";
#elif defined(Q_OS_MACOS)
    if (ufp.contains("Contents/MacOS") == true)
    {
        // should be the folder where HaiBeiDanCi.app locates
        ufp = QCoreApplication::applicationDirPath().section('/', 0, -4) + "/Upgrader.app/Contents/MacOS/Upgrader";
    }
#elif defined(Q_OS_LINUX)
    ;
#else
#error "We don't support the platform yet..."
#endif
        QString workingDirectory = ufp.section('/', 0, -2);
        bool result = QProcess::startDetached(ufp,
                                              arguments,
                                              workingDirectory);
        if (result == false)
        {
            qCritical() << "failed to start the upgrader:" << ufp;
            return 1;
        }

        return 0;
    }
}
