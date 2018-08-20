#include "serverdatadialog.h"
#include "ui_serverdatadialog.h"
#include "wordbook.h"

ServerDataDialog::ServerDataDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ServerDataDialog)
{
    ui->setupUi(this);

    QStringList header;
    header.append(QObject::tr("Book Name"));
    header.append(QObject::tr("Status"));
    ui->twBooks->setHeaderLabels(header);

    ServerAgent *serveragent = ServerAgent::instance();
    connect(serveragent, SIGNAL(bookListReady(const QList<QString>)), this, SLOT(onBookListReady(const QList<QString>)));

    serveragent->getBookList();

}

ServerDataDialog::~ServerDataDialog()
{
    delete ui;
}


void ServerDataDialog::onBookListReady(const QList<QString> books)
{
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

    qDebug() << "start to download" << bookName;

    ServerAgent *serveragent = ServerAgent::instance();
    connect(serveragent, SIGNAL(bookDownloaded(QString)), this, SLOT(onBookDownloaded(QString)));
    serveragent->downloadBook(bookName);
}

void ServerDataDialog::onBookDownloaded(QString bookName)
{
    ServerAgent *serveragent = ServerAgent::instance();
    disconnect(serveragent, SIGNAL(bookDownloaded(QString)), this, SLOT(onBookDownloaded(QString)));

    qDebug() << bookName << "downloaded";
}

void ServerDataDialog::on_pbTest_clicked()
{
    auto ci = ui->twBooks->currentItem();
    if (ci == nullptr)
    {
        return;
    }

    auto bookName = ci->text(0);
}

