#ifndef TESTDIALOG_H
#define TESTDIALOG_H

#include <QDialog>
#include <QtNetwork>

namespace Ui {
class TestDialog;
}

class TestDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TestDialog(QWidget *parent = nullptr);
    ~TestDialog();

private slots:
    void on_pushButton_clicked();

    void onConnected();
    void onDisconnected();
    void onError(QAbstractSocket::SocketError socketError);
    void onReadyRead();
    void onStateChanged(QAbstractSocket::SocketState socketState);

private:
    Ui::TestDialog *ui;

    QTcpSocket m_tcpSocket;

    int readMessageCode();
    bool handleMessage(int messageCode);
    bool handleResponseGetAllBooks();
    bool handleResponseGetWordsOfBook();
    bool handleResponseUnknownRequest();
};

#endif // TESTDIALOG_H
