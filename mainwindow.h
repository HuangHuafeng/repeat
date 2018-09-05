#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "gdhelper.h"
#include "HaiBeiDanCi/worddb.h"
#include "HaiBeiDanCi/wordbook.h"

#include <QMainWindow>
#include <QWebEngineView>
#include <QTreeWidget>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionOpen_triggered();

    void on_pushButton_clicked();

    void on_lineEdit_returnPressed();

    void on_pushNewBook_clicked();

    void on_pushTest_clicked();

    void on_actionNewBook_triggered();

    void onBookListReady(const QList<QString> books);
    void onServerDataReloaded();

    void on_actionPreferences_triggered();

private:
    Ui::MainWindow *ui;
    GDHelper m_gdhelper;
    QWebEngineView m_definitionView;

    void QueryWord();

    void reloadLocalData();
    void reloadServerData();
    void reloadLocalBooks();
    void reloadServerBooks();
    void addBookToTheView(QTreeWidget *tw, WordBook &book);
    void selectFirstItem(QTreeWidget *tw);
};

#endif // MAINWINDOW_H
