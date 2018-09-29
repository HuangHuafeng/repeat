#include "upgradedata.h"
#include "upgrader.h"
#include <QApplication>
#include <QtDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    if (UpgradeData::hasUpgradeData() == false)
    {
        qDebug() << "start HaiBeiDanCi and exit.";
        return 0;
    }

    Upgrader w;
    w.show();

    return a.exec();
}
