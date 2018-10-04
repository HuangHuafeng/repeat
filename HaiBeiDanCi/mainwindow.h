#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "studywindow.h"
#include "wordbook.h"
#include "studylist.h"
#include "browserwindow.h"
#include "applicationuser.h"
#include "serverclientprotocol.h"
#include "autoupgrader.h"

#include <QMainWindow>

namespace Ui
{
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

  public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    virtual void closeEvent(QCloseEvent *event) override;

  private slots:
    void onItemSelectionChanged();
    void onWordStudied(QString spelling);
    void onBookDownloaded(QString /*bookName*/);
    void onAppVersion(ApplicationVersion version, QString fileName, QString info, QDateTime releaseTime);

    void on_pushBrowseExpiredWords_clicked();

    void on_pushStudyExpiredWords_clicked();

    void on_pushStudyOldWords_clicked();

    void on_pushStudyNewWords_clicked();

    void on_pushStudyAllWords_clicked();

    void on_pushBrowseOldWords_clicked();

    void on_pushBrowseNewWords_clicked();

    void on_pushBrowseAllWords_clicked();

    void on_pushGlobalStudyAllWords_clicked();

    void on_pushGlobalBrowseAllWords_clicked();

    void on_pushGlobalStudyNewWords_clicked();

    void on_pushGlobalBrowseNewWords_clicked();

    void on_pushGlobalStudyOldWords_clicked();

    void on_pushGlobalBrowseOldWords_clicked();

    void on_pushGlobalStudyExpiredWords_clicked();

    void on_pushGlobalBrowseExpiredWords_clicked();

    void on_action_About_triggered();

    void on_actionPreferences_triggered();

    void on_actionBooks_triggered();

    void on_actionexit_triggered();

    void on_actionRegister_User_triggered();

    void on_actionLogin_triggered();

    void on_actionLogout_triggered();

    void on_actionCheck_for_Updates_triggered();

private:
    Ui::MainWindow *ui;
    StudyWindow m_studyWindow;
    BrowserWindow m_browserWindow;
    AutoUpgrader m_au;

    void reloadBooks();
    void addBookToTheView(WordBook &book);
    void startStudy(sptr<StudyList> studyList);
    void startBrowse(sptr<StudyList> studyList);

    void updateAllBooksData();
    void updateCurrentBookData();
    void setMyTitle();

    void saveSettings();
    void loadSetting();
    void loadFont();

    sptr<StudyList> expiredWordsFromCurrentBook();
    sptr<StudyList> oldWordsFromCurrentBook();
    sptr<StudyList> newWordsFromCurrentBook();
    sptr<StudyList> allWordsFromCurrentBook();
    void showCurrentBookIntroduction();
    void downloadLatestVersion(ApplicationVersion version, QString fileName);
    void onAppDownloaded(ApplicationVersion version, QString fileName);

};

#endif // MAINWINDOW_H
