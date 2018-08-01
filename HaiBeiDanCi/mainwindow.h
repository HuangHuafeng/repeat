#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "studywindow.h"
#include "wordbook.h"

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

private:
    Ui::MainWindow *ui;
    StudyWindow m_studyWindow;

    void listBooks();
    void addBookToTheView(WordBook &book);
};

#endif // MAINWINDOW_H
