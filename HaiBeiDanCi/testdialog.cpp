#include "testdialog.h"
#include "ui_testdialog.h"

TestDialog::TestDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TestDialog),
    m_tcpSocket(this)
{
    ui->setupUi(this);

    connect(&m_tcpSocket, SIGNAL(connected()), this, SLOT(onConnected()));
    connect(&m_tcpSocket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    connect(&m_tcpSocket, SIGNAL(disconnected()), this, SLOT(onDisconnected()));

    m_tcpSocket.connectToHost("huafengsmac", 61027);
}

TestDialog::~TestDialog()
{
    delete ui;
}

void TestDialog::on_pushButton_clicked()
{
    int opcode = ui->leAction->text().toInt();
    QString parameter = ui->leParameter->text();

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << opcode << parameter;
    m_tcpSocket.write(block);
}

void TestDialog::onReadyRead()
{
    qDebug() << "onReadyRead()";
    int opcode;
    int respRes;
    QString response;
    QDataStream in(&m_tcpSocket);
    in.startTransaction();
    in >> opcode >> respRes >> response;
    in.commitTransaction();
    qDebug() << opcode << respRes << response;
}

void TestDialog::onConnected()
{
    qDebug() << "onConnected()";
}

void TestDialog::onDisconnected()
{
    qDebug() << "onDisconnected()";
}

void TestDialog::onError(QAbstractSocket::SocketError socketError)
{
    qDebug() << "onError()";
}
