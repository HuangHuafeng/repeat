#include "serverdatadialog.h"
#include "ui_serverdatadialog.h"
#include "wordbook.h"

ServerDataDialog::ServerDataDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ServerDataDialog),
    m_serverAgent(this)
{
    ui->setupUi(this);

    QStringList header;
    header.append(QObject::tr("Book Name"));
    header.append(QObject::tr("Status"));
    ui->twBooks->setHeaderLabels(header);

    connect(&m_serverAgent, SIGNAL(responseGetAllBooks(QList<QString>)), this, SLOT(onResponseGetAllBooks(QList<QString>)));
    m_serverAgent.sendRequestGetAllBooks();
}

ServerDataDialog::~ServerDataDialog()
{
    delete ui;
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
