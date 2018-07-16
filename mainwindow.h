#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "gdhelper.h"

#include <QMainWindow>
#include <QWebEngineView>

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
    GDHelper m_gdhelper;

    void QueryWord();
};

#endif // MAINWINDOW_H
