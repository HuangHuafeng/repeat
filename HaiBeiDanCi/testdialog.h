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

private:
    Ui::TestDialog *ui;

    QTcpSocket m_tcpSocket;
};

#endif // TESTDIALOG_H
