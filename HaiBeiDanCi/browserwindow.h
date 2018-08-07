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

class TreeWidgetUpdater;

namespace Ui {
class BrowserWindow;
}

class BrowserWindow : public QDialog
{
    Q_OBJECT

public:
    explicit BrowserWindow(QWidget *parent = nullptr);
    ~BrowserWindow();

    bool setWordList(sptr<StudyList> studyList);
    void reloadView();

    void lockTree() {
        m_mutex.lock();
    }
    void unlockTree() {
        m_mutex.unlock();
    }

private:
    Ui::BrowserWindow *ui;
    WordView m_wordView;
    TreeWidgetUpdater *m_updaterThread;
    QMutex m_mutex;

    void addWordsToTreeView(sptr<StudyList> studyList);
    void showHideButtons(bool definitionIsShown);

    void stopUpdater();
    void startUpdater();

private slots:
    void onItemSelectionChanged();
    void on_checkHideTreeview_stateChanged(int);
    void on_pushPrevious_clicked();
    void on_pushNext_clicked();
    void on_pushShow_clicked();
    void onTreeWidgetUpdated();
};

class TreeWidgetUpdater : public QThread {
    Q_OBJECT
    void run() override;
    void updateTreeWidget();

    BrowserWindow &m_bw;
    QTreeWidget *m_treeWidget;

public:
    TreeWidgetUpdater(BrowserWindow &bw, QTreeWidget *treeWidget, QObject *parent = nullptr);

signals:
    void updateFinished();

};

#endif // BROWSERWINDOW_H
