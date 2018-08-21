#include "serverdatadialog.h"
#include "ui_serverdatadialog.h"
#include "wordbook.h"

ServerDataDialog::ServerDataDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ServerDataDialog),
    m_pd(nullptr)
{
    ui->setupUi(this);

    connect(ui->twBooks, SIGNAL(itemSelectionChanged()), this, SLOT(onItemSelectionChanged()));

    QStringList header;
    header.append(QObject::tr("Book Name"));
    header.append(QObject::tr("Status"));
    ui->twBooks->setHeaderLabels(header);

    ServerAgent *serveragent = ServerAgent::instance();
    connect(serveragent, SIGNAL(bookListReady(const QList<QString>)), this, SLOT(onBookListReady(const QList<QString>)));
    connect(serveragent, SIGNAL(bookDownloaded(QString)), this, SLOT(onBookDownloaded(QString)));
    connect(serveragent, SIGNAL(downloadProgress(float)), this, SLOT(onDownloadProgress(float)));
    //connect(serveragent, SIGNAL(wordDownloaded(QString)), this, SLOT(onWordDownloaded(QString)));

    serveragent->getBookList();

}

ServerDataDialog::~ServerDataDialog()
{
    delete ui;
}

void ServerDataDialog::onItemSelectionChanged()
{
    auto ci = ui->twBooks->currentItem();
    if (ci == nullptr)
    {
        return;
    }

    auto bookName = ci->text(0);
    bool downloaded = false;
    if (WordBook::getBook(bookName).get() != nullptr)
    {
        downloaded = true;
    }
    else
    {
        downloaded = false;
    }
    ui->pbDownloadBook->setEnabled(downloaded == false);
}

void ServerDataDialog::onBookListReady(const QList<QString> books)
{
    ui->twBooks->clear();

    for (int i = 0;i < books.size();i ++)
    {
        QString bookName = books.at(i);

        QStringList infoList;
        infoList.append(bookName);
        infoList.append("");
        QTreeWidgetItem *item = new QTreeWidgetItem(infoList);
        ui->twBooks->addTopLevelItem(item);

        updateBookStatus(bookName);
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

    if (m_pd == nullptr)
    {
        m_pd = new QProgressDialog(QObject::tr("Downloading ") + "\"" + bookName + "\"",
                                   QString(),
                                   0,
                                   1000000,
                                   this);
        m_pd->setModal(true);
        m_pd->setValue(0);
        m_pd->resize(m_pd->size() + QSize(20, 0));
        m_pd->show();
    }

    ServerAgent *serveragent = ServerAgent::instance();
    serveragent->downloadBook(bookName);
}

void ServerDataDialog::onDownloadProgress(float percentage)
{
    if (m_pd != nullptr)
    {
        int value = static_cast<int>(1000000 * percentage);
        m_pd->setValue(value);
        //qDebug() << percentage;
        //qDebug() << value;
    }
}

void ServerDataDialog::onBookDownloaded(QString bookName)
{
    updateBookStatus(bookName);
    onItemSelectionChanged();

    if (m_pd != nullptr)
    {
        m_pd->deleteLater();
        m_pd = nullptr;
    }
}

void ServerDataDialog::updateBookStatus(QString bookName)
{
    auto items = ui->twBooks->findItems(bookName, Qt::MatchFlag::MatchExactly);
    if (items.size() != 0)
    {
        QString status;
        if (WordBook::getBook(bookName).get() != nullptr)
        {
            status = QObject::tr("downloaded");
        }
        else
        {
            status = QObject::tr("not downloaded");
        }
        items[0]->setText(1, status);
    }
}

void ServerDataDialog::on_pbClose_clicked()
{
    close();
}
