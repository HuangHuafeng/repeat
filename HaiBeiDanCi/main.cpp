#include "mainwindow.h"
#include "worddb.h"
#include "dictschemehandler.h"
#include "mysettings.h"

#include <QApplication>
#include <QLibraryInfo>
#include <QTranslator>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MySettings::setAppName(QObject::tr("HaiBeiDanCi"));

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

    DictSchemeHandler dsh;

    MainWindow w;
    w.show();

    return app.exec();
}
