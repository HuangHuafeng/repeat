#include "testdialog.h"
#include "ui_testdialog.h"
#include "serverclientprotocol.h"

TestDialog::TestDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TestDialog),
    m_tcpSocket(this)
{
    ui->setupUi(this);

    connect(&m_tcpSocket, SIGNAL(connected()), this, SLOT(onConnected()));
    connect(&m_tcpSocket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    connect(&m_tcpSocket, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
    connect(&m_tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)));
    connect(&m_tcpSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(onStateChanged(QAbstractSocket::SocketState)));

    m_tcpSocket.connectToHost("huafengsmac", 61027);

    // it seems waitForConnected() helps to get/trigger the error ConnectionRefusedError
    if (m_tcpSocket.waitForConnected() == false)
    {
        qDebug() << "waitForConnected() failed" << m_tcpSocket.error();
    }
}

TestDialog::~TestDialog()
{
    delete ui;
}

void TestDialog::on_pushButton_clicked()
{
    //qDebug() << m_tcpSocket.error();

    int opcode = ui->leAction->text().toInt();
    if (opcode == 0)
    {
        return;
    }

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << opcode;

    QString parameter = ui->leParameter->text();
    if (parameter.isEmpty() == false)
    {
        out << parameter;
    }

    m_tcpSocket.write(block);
}

void TestDialog::onReadyRead()
{
    int messageCode = readMessageCode();
    if (messageCode != 0)
    {
        if (handleMessage(messageCode) == false)
        {
            qDebug() << "failed to handle message with code" << messageCode;
        }
    }
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
    qDebug() << "onError()" << socketError;
}


void TestDialog::onStateChanged(QAbstractSocket::SocketState socketState)
{
    qDebug() << "onStateChanged()" << socketState;
}

int TestDialog::readMessageCode()
{
    int messageCode;
    QDataStream in(&m_tcpSocket);
    in.startTransaction();
    in >> messageCode;
    if (in.commitTransaction() == true)
    {
        return messageCode;
    }
    else
    {
        // in this case, the transaction is restored by commitTransaction()
        qDebug() << "failed to read message code in readMessageCode()";
        return 0;
    }
}

bool TestDialog::handleMessage(int messageCode)
{
    bool handleResult = false;
    switch (messageCode) {
    case ServerClientProtocol::ResponseFailedToRequest:
        handleResult = handleResponseUnknownRequest();
        break;

    case ServerClientProtocol::ResponseGetAllBooks:
        handleResult = handleResponseGetAllBooks();
        break;

    case ServerClientProtocol::ResponseGetWordsOfBook:
        handleResult = handleResponseGetWordsOfBook();
        break;

    default:
        qDebug() << "got unknown message with code" << messageCode << "in handleMessage()";
        handleResult = false;
        break;

    }

    return handleResult;
}

bool TestDialog::handleResponseGetAllBooks()
{
    QDataStream in(&m_tcpSocket);
    QList<QString> books;
    in.startTransaction();
    in >> books;
    if (in.commitTransaction() == false)
    {
        // in this case, the transaction is restored by commitTransaction()
        qDebug() << "failed to read books in handleResponseGetAllBooks()";
        return false;
    }

    qDebug() << books;

    return true;
}

bool TestDialog::handleResponseGetWordsOfBook()
{
    QString bookName;
    QVector<QString> wordList;
    QDataStream in(&m_tcpSocket);
    in.startTransaction();
    in >> bookName >> wordList;
    if (in.commitTransaction() == false)
    {
        // in this case, the transaction is restored by commitTransaction()
        qDebug() << "failed to read words of the book in handleResponseGetWordsOfBook()";
        return false;
    }

    qDebug() << "got words of book" << bookName;
    for (int i = 0;i < wordList.size();i ++)
    {
        qDebug() << wordList.at(i);
    }

    return true;
}

bool TestDialog::handleResponseUnknownRequest()
{
    QDataStream in(&m_tcpSocket);
    int requestCode;
    in.startTransaction();
    in >> requestCode;
    if (in.commitTransaction() == false)
    {
        // in this case, the transaction is restored by commitTransaction()
        qDebug() << "failed to read books in handleResponseGetAllBooks()";
        return false;
    }

    qDebug() << "the server responed that failed to handle request with code" << requestCode;

    return true;
}
