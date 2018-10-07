#include "upgradedata.h"
#include "upgrader.h"
#include <QApplication>
#include <QtDebug>
#include <QTextStream>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    if (argc == 3)
    {
        if (QString::compare(argv[1], "--version") == 0)
        {
            //"Upgrader --version file", writing the version to file

            QFile tf(argv[2]);
            if (tf.open(QIODevice::Text | QIODevice::WriteOnly) == true)
            {
                QTextStream s(&tf);
                s << APP_VERSION << "\n";
                tf.close();
            }
            else
            {
                qCritical() << "failed to open file" << tf.fileName();
            }
        }
        else if (QString::compare(argv[1], "--target") == 0)
        {
            //"Upgrader --target name", upgrading the target app
            // argv[2] is considered to be the target

            QString target = argv[2];
            UpgradeData ud(target);
            if (ud.hasUpgradeData() == false)
            {
                ud.startTarget();
            }
            else
            {
                Upgrader w;
                w.setTarget(target);
                w.show();
                a.exec();
            }
        }
        else
        {
            qCritical() << "Wrong parameter!";
        }

        return 0;
    }
    else
    {
        QString appName = QCoreApplication::applicationName();
        qCritical() << "usage:" << appName << "[--target name] [--version file]";
        return 0;
    }
}
