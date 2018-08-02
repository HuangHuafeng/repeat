#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "studywindow.h"
#include "wordbook.h"
#include "bookbrowser.h"
#include "studylist.h"

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

    void on_pushBrowse_clicked();

    void on_pushStudy_clicked();

    void on_pushReview_clicked();

    void on_pushRevSd_clicked();

    void on_pushExpired_clicked();

private:
    Ui::MainWindow *ui;
    StudyWindow m_studyWindow;
    BookBrowser m_bookBrowser;

    void listBooks();
    void addBookToTheView(WordBook &book);
    void startStudy(sptr<StudyList> studyList);
};

#endif // MAINWINDOW_H
