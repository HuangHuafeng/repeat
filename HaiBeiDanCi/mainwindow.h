#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "studywindow.h"
#include "wordbook.h"
#include "bookbrowser.h"

#include <QMainWindow>

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
    void on_pushTest_clicked();

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::MainWindow *ui;
    StudyWindow m_studyWindow;
    BookBrowser m_bookBrowser;

    void listBooks();
    void addBookToTheView(WordBook &book);
};

#endif // MAINWINDOW_H
