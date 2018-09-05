#include "mainwindow.h"
#include "HaiBeiDanCi/worddb.h"
#include "HaiBeiDanCi/mysettings.h"

#include <QApplication>
#include <QTranslator>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MySettings::setAppName("HaiBeiDanCiManager");
    QCoreApplication::setOrganizationName(MySettings::orgName());
    QCoreApplication::setOrganizationDomain(MySettings::orgDomain());
    // don't call setApplicationName() so it continues to be the executable name
    //QCoreApplication::setApplicationName(MySettings::appName());

    // this should be after the above 3 lines!!!
    //MySettings::saveDataDirectory("/Users/huafeng/Documents/GitHub/HaiBeiDanCiData");
    //qDebug() << QCoreApplication::applicationName() << MySettings::dataDirectory();

    QTranslator qtTranslator;
    qtTranslator.load("qt_" + QLocale::system().name(),
            QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    app.installTranslator(&qtTranslator);

    QTranslator myappTranslator;
    //myappTranslator.load("myapp_" + QLocale::system().name());
    myappTranslator.load("myapp_zh_CN");
    app.installTranslator(&myappTranslator);

    MainWindow w;
    w.show();

    return app.exec();
}
