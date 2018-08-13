#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "gdhelper.h"
#include "HaiBeiDanCi/studywindow.h"
#include "HaiBeiDanCi/worddb.h"
#include "newbook.h"

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

    void on_pushButton_2_clicked();

    void on_pushNewBook_clicked();

    void on_pushTest_clicked();

private:
    Ui::MainWindow *ui;
    GDHelper m_gdhelper;
    QWebEngineView m_definitionView;
    StudyWindow m_studyWindow;
    NewBook m_newbookWindow;
    //WordView m_wordView;

    void QueryWord();

    void searchLink(QTextFrame * parent);
    void searchLink(QTextBlock & parent);
};

#endif // MAINWINDOW_H
