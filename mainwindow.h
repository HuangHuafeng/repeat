#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWebEngineView>

#include "mdxdict.h"
#include "dictschemehandler.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_actionOpen_triggered();

    void on_pushButton_clicked();

    void on_lineEdit_returnPressed();

private:
    Ui::MainWindow *ui;
    QWebEngineView *m_webEngineView;
    MdxDict m_dict;
    DictSchemeHandler m_dictSchemeHandler;

    void QueryWord();
};

#endif // MAINWINDOW_H
