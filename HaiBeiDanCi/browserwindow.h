#ifndef BROWSERWINDOW_H
#define BROWSERWINDOW_H

#include "wordview.h"
#include "wordcard.h"
#include "studylist.h"
#include "../golddict/gddebug.hh"

#include <QDialog>
#include <QVector>
#include <QThread>
#include <QTreeWidget>

namespace Ui
{
class BrowserWindow;
}

class BrowserWindow : public QDialog
{
    Q_OBJECT

  public:
    explicit BrowserWindow(QWidget *parent = nullptr);
    ~BrowserWindow() override;

    bool setWordList(sptr<StudyList> studyList);
    void reloadView();

    virtual void closeEvent(QCloseEvent *event) override;

    void lockTree()
    {
        m_mutex.lock();
    }
    void unlockTree()
    {
        m_mutex.unlock();
    }

  private:
    Ui::BrowserWindow *ui;
    WordView m_wordView;
    QMutex m_mutex;

    void addWordsToTreeView(sptr<StudyList> studyList);
    void showHideButtons(bool definitionIsShown);

    void saveSettings();
    void loadSetting();
    void setMyTitle();

  private slots:
    void onItemSelectionChanged();
    void on_checkHideTreeview_stateChanged(int);
    void on_pushPrevious_clicked();
    void on_pushNext_clicked();
    void on_pushShow_clicked();
    void on_checkShowDefinitionDirectly_stateChanged(int arg1);
};

#endif // BROWSERWINDOW_H
