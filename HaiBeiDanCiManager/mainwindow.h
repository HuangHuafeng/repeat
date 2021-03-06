#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "gdhelper.h"
#include "../HaiBeiDanCi/worddb.h"
#include "../HaiBeiDanCi/wordbook.h"
#include "servermanager.h"

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
    void onUploadProgress(float percentage);

    void on_actionPreferences_triggered();

    void on_pbReloadServerData_clicked();

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

    void on_actionLogin_triggered();

    void on_actionLogout_triggered();

    void on_actionUsers_triggered();

    void on_actionRelease_App_triggered();

    void on_actionRelease_Upgrader_triggered();

    void on_pbTest_clicked();

private:
    Ui::MainWindow *ui;
    GDHelper m_gdhelper;
    QWebEngineView m_definitionView;
    QTimer m_refreshTimer;
    ServerManager m_sm;

    QProgressDialog m_progressDialog;
    void initializeProgressDialog();
    void createProgressDialog(const QString &labelText, const QString &cancelButtonText);

    void QueryWord();

    void reloadLocalData();
    void reloadLocalBooks();
    void addBookToTheView(QTreeWidget *tw, WordBook &book);
    void selectFirstItem(QTreeWidget *tw);
    void listServerBooks(const QList<QString> books);
    bool okToPerformServerRelatedOperation();

    void downloadBook(QString bookName, bool showProgress, QString labelText, QString cancelButtonText);
    void onBookDownloaded(QString bookName);

#ifndef QT_NO_DEBUG
    void test();
#endif
};

#endif // MAINWINDOW_H
