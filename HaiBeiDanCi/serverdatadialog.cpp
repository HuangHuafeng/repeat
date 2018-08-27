#include "serverdatadialog.h"
#include "ui_serverdatadialog.h"
#include "wordbook.h"
#include "mysettings.h"

#include <QMessageBox>

ServerDataDialog::ServerDataDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ServerDataDialog),
    m_progressDialog(this),
    m_pdMaximum(100000)
{
    ui->setupUi(this);

    connect(ui->twBooks, SIGNAL(itemSelectionChanged()), this, SLOT(onItemSelectionChanged()));

    initializeProgressDialog();

    QStringList header;
    header.append(QObject::tr("Book Name"));
    header.append(QObject::tr("Status"));
    ui->twBooks->setHeaderLabels(header);

    ServerAgent *serveragent = ServerAgent::instance();
    connect(serveragent, SIGNAL(bookListReady(const QList<QString>)), this, SLOT(onBookListReady(const QList<QString>)));
    connect(serveragent, SIGNAL(bookDownloaded(QString)), this, SLOT(onBookDownloaded(QString)));
    connect(serveragent, SIGNAL(downloadProgress(float)), this, SLOT(onDownloadProgress(float)));
    //connect(serveragent, SIGNAL(fileDownloaded(QString, bool)), this, SLOT(onFileDownloaded(QString, bool)));
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
    ui->pbDownloadMediaFiles->setEnabled(downloaded == true);
    ui->pbDownloadPronounceFiles->setEnabled(downloaded == true);
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

    createProgressDialog(QObject::tr("Downloading ") + "\"" + bookName + "\"", QObject::tr("Cancel"));

    ServerAgent *serveragent = ServerAgent::instance();
    serveragent->downloadBook(bookName);
}

void ServerDataDialog::onDownloadProgress(float percentage)
{
    if (m_progressDialog.wasCanceled() == true)
    {
        destroyProgressDialog();
        ServerAgent *serveragent = ServerAgent::instance();
        serveragent->cancelDownloading();
    }
    else
    {
        int value = static_cast<int>(m_pdMaximum * percentage);
        // Warning: If the progress dialog is modal (see QProgressDialog::QProgressDialog()), setValue() calls QApplication::processEvents(), so take care that this does not cause undesirable re-entrancy in your code. For example, don't use a QProgressDialog inside a paintEvent()!
        m_progressDialog.setValue(value);
    }
}

void ServerDataDialog::onBookDownloaded(QString bookName)
{
    updateBookStatus(bookName);
    onItemSelectionChanged();

    on_pbDownloadPronounceFiles_clicked();
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
    ServerAgent *serveragent = ServerAgent::instance();
    serveragent->disconnectServer();

    close();
}

void ServerDataDialog::on_pbDownloadMediaFiles_clicked()
{
    auto ci = ui->twBooks->currentItem();
    if (ci == nullptr)
    {
        return;
    }

    auto bookName = ci->text(0);
    sptr<WordBook> book = WordBook::getBook(bookName);
    if (book.get() == nullptr)
    {
        return;
    }

    // build the list of media files
    QList<QString> mediaFiles;
    QVector<QString> wordList = book->getAllWords();
    const QString dd = MySettings::dataDirectory() + "/";
    for (int i = 0;i < wordList.size();i ++)
    {
        QString spelling = wordList.at(i);
        sptr<Word> word = Word::getWord(spelling);
        if (word.get() == nullptr)
        {
            continue;
        }
        mediaFiles += word->mediaFiles();
    }

    ServerAgent *serveragent = ServerAgent::instance();
    auto filesToDownload = serveragent->downloadMultipleFiles(mediaFiles);
    if (filesToDownload.isEmpty() == false)
    {
        // show the progress dialog
        createProgressDialog(QObject::tr("Downloading meida files ..."), QObject::tr("Cancel"));
    }
    else
    {
        QMessageBox::information(this, MySettings::appName(), QObject::tr("All media files are already available locally!"));
    }
}

void ServerDataDialog::on_pbDownloadPronounceFiles_clicked()
{
    auto ci = ui->twBooks->currentItem();
    if (ci == nullptr)
    {
        return;
    }

    auto bookName = ci->text(0);
    sptr<WordBook> book = WordBook::getBook(bookName);
    if (book.get() == nullptr)
    {
        return;
    }

    // build the list of media files
    QList<QString> mediaFiles;
    QVector<QString> wordList = book->getAllWords();
    const QString dd = MySettings::dataDirectory() + "/";
    for (int i = 0;i < wordList.size();i ++)
    {
        QString spelling = wordList.at(i);
        sptr<Word> word = Word::getWord(spelling);
        if (word.get() == nullptr)
        {
            continue;
        }
        mediaFiles += word->pronounceFiles();
    }

    ServerAgent *serveragent = ServerAgent::instance();
    auto filesToDownload = serveragent->downloadMultipleFiles(mediaFiles);
    if (filesToDownload.isEmpty() == false)
    {
        // show the progress dialog
        createProgressDialog(QObject::tr("Downloading pronounce files ..."), QObject::tr("Cancel"));
    }
    else
    {
        QMessageBox::information(this, MySettings::appName(), QObject::tr("All pronounce files are already available locally!"));
    }
}

void ServerDataDialog::initializeProgressDialog()
{
    m_progressDialog.setModal(true);
    m_progressDialog.setRange(0, m_pdMaximum);
    m_progressDialog.cancel();
}

void ServerDataDialog::createProgressDialog(const QString &labelText, const QString &cancelButtonText)
{
    m_progressDialog.reset();
    m_progressDialog.setLabelText("    " + labelText + "    ");
    m_progressDialog.setCancelButtonText(cancelButtonText);
    m_progressDialog.setValue(0);;
}

void ServerDataDialog::destroyProgressDialog()
{
}
