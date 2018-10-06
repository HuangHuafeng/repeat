#include "upgradedata.h"
#include "upgrader.h"
#include <QApplication>
#include <QtDebug>
#include <QTextStream>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QTextStream out(stdout);

    if (argc != 2)
    {
        QString appName = QCoreApplication::applicationName();
        out << "usage: " << appName << " target" << endl;
        out << "target is the name of the app to be upgraded." << endl;
        return 0;
    }

    // argv[1] is considered to be the target
    UpgradeData ud(argv[1]);
    if (ud.hasUpgradeData() == false)
    {
        qDebug() << "No upgrade data. This should NOT happen!";
        return 0;
    }

    Upgrader w;
    w.setTarget(argv[1]);
    w.show();

    return a.exec();
}
