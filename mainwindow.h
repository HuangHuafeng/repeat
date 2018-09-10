#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "gdhelper.h"
#include "HaiBeiDanCi/worddb.h"
#include "HaiBeiDanCi/wordbook.h"

#include <QMainWindow>
#include <QWebEngineView>
#include <QTreeWidget>
#include <QProgressDialog>

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

    void on_actionNewBook_triggered();

    void onServerDataReloaded();
    void onBookDownloaded(QString bookName);
    void onUploadProgress(float percentage);

    void on_actionPreferences_triggered();

    void on_pbReloadServerData_clicked();

    void on_actionSync_To_Local_triggered();

    void on_actionReload_data_triggered();

    void on_actionDeleteLocalBook_triggered();

    void on_pbDeleteBook_clicked();

    void on_actionUpload_Book_triggered();

    void on_pbUploadBook_clicked();

    void on_actionDownload_Book_triggered();

    void on_pbDownloadServerBook_clicked();

    void on_actionDeleteServerBook_triggered();

    void on_pbDeleteServerBook_clicked();

    void onRefreshTimerTimeout();

    void on_actionFetch_Missing_Media_Files_triggered();

    void on_actionUpload_Book_Missing_Media_Files_triggered();

private:
    Ui::MainWindow *ui;
    GDHelper m_gdhelper;
    QWebEngineView m_definitionView;
    QTimer m_refreshTimer;

    QProgressDialog m_progressDialog;
    void initializeProgressDialog();
    void createProgressDialog(const QString &labelText, const QString &cancelButtonText);

    void QueryWord();

    void reloadLocalData();
    void reloadLocalBooks();
    void addBookToTheView(QTreeWidget *tw, WordBook &book);
    void selectFirstItem(QTreeWidget *tw);
    void listServerBooks(const QList<QString> books);
    bool localServerDataConflicts();
};

#endif // MAINWINDOW_H
