#include "mainwindow.h"
#include "golddict/gddebug.hh"
#include "worddb.h"

#include <QApplication>
#include <QTranslator>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    WordDB appDb;
    if (appDb.connectDB() == false) {
        return 1;
    }

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
