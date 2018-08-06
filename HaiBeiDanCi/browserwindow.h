#ifndef BROWSERWINDOW_H
#define BROWSERWINDOW_H

#include "wordview.h"
#include "wordcard.h"
#include "studylist.h"

#include <QDialog>
#include <QVector>
#include <QThread>
#include <QTreeWidget>

class TreeWidgetUpdater : public QThread {
    Q_OBJECT
    void run() override;
    void updateTreeWidget();

    QTreeWidget *m_treeWidget;
    bool m_itemsWillBeRemoved;

public:
    TreeWidgetUpdater(QTreeWidget *treeWidget, QObject *parent = nullptr);
    void setItemsWillBeRemoved() {
        m_itemsWillBeRemoved = true;
    }

signals:
    void updateFinished();

};

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

private:
    Ui::BrowserWindow *ui;
    WordView m_wordView;
    TreeWidgetUpdater *m_updaterThread;

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

#endif // BROWSERWINDOW_H
