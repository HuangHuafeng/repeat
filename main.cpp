#include "mainwindow.h"
#include <QApplication>
//#include <QWebEngineView>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    /*
    QWebEngineView view;
    view.setUrl(QUrl(QStringLiteral("https://www.qt.io")));
    view.resize(1024, 750);
    view.show();
    */

    return a.exec();
}
