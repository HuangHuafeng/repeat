#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "studywindow.h"
#include "wordbook.h"
#include "studylist.h"
#include "browserwindow.h"

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
    void onItemSelectionChanged();

    void on_pushTest_clicked();

    void on_pushBrowseExpiredWords_clicked();

    void on_pushStudyExpiredWords_clicked();

    void on_pushStudyOldWords_clicked();

    void on_pushStudyNewWords_clicked();

    void on_pushStudyAllWords_clicked();

    void on_pushBrowseOldWords_clicked();

    void on_pushBrowseNewWords_clicked();

    void on_pushBrowseAllWords_clicked();

private:
    Ui::MainWindow *ui;
    StudyWindow m_studyWindow;
    BrowserWindow m_browserWindow;

    void listBooks();
    void addBookToTheView(WordBook &book);
    void startStudy(sptr<StudyList> studyList);
    void startBrowse(sptr<StudyList> studyList);

    sptr<StudyList> expiredWordsFromCurrentBook();
    sptr<StudyList> oldWordsFromCurrentBook();
    sptr<StudyList> newWordsFromCurrentBook();
    sptr<StudyList> allWordsFromCurrentBook();
};

#endif // MAINWINDOW_H
