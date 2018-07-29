#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "gdhelper.h"
#include "studywindow.h"
#include "worddb.h"

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

private:
    Ui::MainWindow *ui;
    GDHelper m_gdhelper;
    QWebEngineView m_definitionView;
    StudyWindow m_studyWindow;
    //WordView m_wordView;

    void QueryWord();
    void TestHtmlParse();

    void searchLink(QTextFrame * parent);
    void searchLink(QTextBlock & parent);
    void saveWord(const QString &spelling);
};

#endif // MAINWINDOW_H
