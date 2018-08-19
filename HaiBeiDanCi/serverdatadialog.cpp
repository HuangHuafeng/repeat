#include "serverdatadialog.h"
#include "ui_serverdatadialog.h"
#include "wordbook.h"

ServerDataDialog::ServerDataDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ServerDataDialog),
    m_serverAgent(nullptr)
{
    ui->setupUi(this);

    QStringList header;
    header.append(QObject::tr("Book Name"));
    header.append(QObject::tr("Status"));
    ui->twBooks->setHeaderLabels(header);

    requestAllBooks();
}

ServerDataDialog::~ServerDataDialog()
{
    delete ui;
}

void ServerDataDialog::requestAllBooks()
{
    connectToServer();
    if (m_serverAgent != nullptr)
    {
        m_serverAgent->sendRequestGetAllBooks();
    }
}

void ServerDataDialog::requestWordsOfBook(QString bookName)
{
    connectToServer();
    if (m_serverAgent != nullptr)
    {
        m_serverAgent->sendRequestGetWordsOfBook(bookName);
    }
}

void ServerDataDialog::connectToServer()
{
    if (m_serverAgent != nullptr)
    {
        return;
    }

    m_serverAgent = new ServerAgent(this);
    if (m_serverAgent == nullptr)
    {
        return;
    }

    connect(m_serverAgent, SIGNAL(responseGetAllBooks(QList<QString>)), this, SLOT(onResponseGetAllBooks(QList<QString>)));
    connect(m_serverAgent, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
    connect(m_serverAgent, SIGNAL(responseGetWordsOfBook(QString, QVector<QString>)), this, SLOT(onResponseGetWordsOfBook(QString, QVector<QString>)));

    m_serverAgent->connectToServer("huafengsmac", 61027);
}

void ServerDataDialog::onResponseGetWordsOfBook(QString bookName, QVector<QString> wordList)
{
    qDebug() << bookName;
    qDebug() << wordList;
}

void ServerDataDialog::onDisconnected()
{
    m_serverAgent->deleteLater();
    m_serverAgent = nullptr;
}

void ServerDataDialog::onResponseGetAllBooks(QList<QString> books)
{
    //qDebug() << books;
    for (int i = 0;i < books.size();i ++)
    {
        QString bookName = books.at(i);
        QString status;;
        if (WordBook::getBook(bookName).get() == nullptr)
        {
            status = QObject::tr("not downloaded");
        }
        else
        {
            status = QObject::tr("downloaded");
        }

        QStringList infoList;
        infoList.append(bookName);
        infoList.append(status);
        QTreeWidgetItem *item = new QTreeWidgetItem(infoList);
        ui->twBooks->addTopLevelItem(item);
    }

    // select the first book
    QTreeWidgetItemIterator it(ui->twBooks);
    if (*it)
    {
        ui->twBooks->setCurrentItem(*it);
    }

    ui->twBooks->resizeColumnToContents(0);
}

void ServerDataDialog::on_pbDownloadBook_clicked()
{
    auto ci = ui->twBooks->currentItem();
    if (ci == nullptr)
    {
        return;
    }

    auto bookName = ci->text(0);
    requestWordsOfBook(bookName);
}
