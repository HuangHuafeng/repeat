#include "upgradedata.h"
#include "upgrader.h"
#include <QApplication>
#include <QtDebug>
#include <QTextStream>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QTextStream out(stdout);

    if (argc < 2)
    {
        out << "usage: " << argv[0] << " target" << endl;
        out << "target is the name of the app to be upgraded." << endl;
        out << "usage: " << argv[0] << " --version to print version" << endl;
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
        qDebug() << "No upgrade data. Start the target and exit.";
        ud.startTarget();
        return 0;
    }

    Upgrader w;
    w.setTarget(argv[1]);
    w.show();

    return a.exec();
}
