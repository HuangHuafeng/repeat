#ifndef SERVERDATADIALOG_H
#define SERVERDATADIALOG_H

#include "serveragent.h"

#include <QDialog>

namespace Ui {
class ServerDataDialog;
}

class ServerDataDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ServerDataDialog(QWidget *parent = nullptr);
    ~ServerDataDialog();

private slots:
    void onResponseGetAllBooks(QList<QString> books);
    //void onResponseGetABook(WordBook book);
    void onDisconnected();
    void onResponseGetWordsOfBook(QString /*bookName*/, QVector<QString> wordList);

    void on_pbDownloadBook_clicked();

    void on_pbTest_clicked();

private:
    Ui::ServerDataDialog *ui;

    ServerAgent *m_serverAgent;

    void downloadBook(const QString bookName);

    void connectToServer();
    void requestAllBooks();
    void requestWordsOfBook(QString bookName);
    void requestGetAWord(QString spelling);
    void requestGetABook(QString bookName);
};

#endif // SERVERDATADIALOG_H
