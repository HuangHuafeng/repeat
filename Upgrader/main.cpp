#include "upgradedata.h"
#include "upgrader.h"
#include <QApplication>
#include <QtDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    qDebug() << "argc:" << argc;

    if (argc < 2)
    {
        qCritical() << "usage:" << argv[0] << "target";
        qCritical() << "target is the name of the app to be upgraded.";
        return 0;
    }

    UpgradeData ud(argv[1]);
    if (ud.upgraderFilePath().isEmpty() == true)
    {
        // save the upgrader info
        ud.saveUpgraderFilePath();
    }

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
