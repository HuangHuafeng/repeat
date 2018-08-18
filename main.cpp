#include "mainwindow.h"
#include "HaiBeiDanCi/worddb.h"
#include "HaiBeiDanCi/mysettings.h"

#include <QApplication>
#include <QTranslator>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // use the setting from HaiBeiDanCi if it's in the same computer
    QCoreApplication::setOrganizationName(MySettings::orgName());
    QCoreApplication::setOrganizationDomain(MySettings::orgDomain());
    QCoreApplication::setApplicationName(MySettings::appName());

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
