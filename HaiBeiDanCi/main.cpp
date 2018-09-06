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

    QTranslator qtTranslator;
    qtTranslator.load("qt_" + QLocale::system().name(),
                      QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    app.installTranslator(&qtTranslator);

    QTranslator myappTranslator;
    //myappTranslator.load("myapp_" + QLocale::system().name());
    myappTranslator.load("myapp_zh_CN");
    app.installTranslator(&myappTranslator);

    // this should be after loading the translator!!!
    MySettings::setAppName(QObject::tr("HaiBeiDanCi"));
    QCoreApplication::setOrganizationName(MySettings::orgName());
    QCoreApplication::setOrganizationDomain(MySettings::orgDomain());
    // don't call setApplicationName() so it continues to be the executable name
    QCoreApplication::setApplicationName(MySettings::appName());

    DictSchemeHandler dsh;

    MainWindow w;
    w.show();

    return app.exec();
}
