#include "serverdatadialog.h"
#include "ui_serverdatadialog.h"
#include "wordbook.h"
#include "mysettings.h"
#include "serverdatadownloader.h"
#include "mediafilemanager.h"

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

    ServerDataDownloader *sdd = ServerDataDownloader::instance();
    connect(sdd, SIGNAL(bookStored(QString)), this, SLOT(onBookDownloaded(QString)));
    connect(sdd, SIGNAL(downloadProgress(float)), this, SLOT(onDownloadProgress(float)));

    if (sdd->isBookListReady() == true)
    {
        onBookListReady(sdd->getBookList());
    }
    else
    {
        // in this case, sdd must failed to connect to the server when it's constructed
        // so let it try ot connect to the server again
        connect(sdd, SIGNAL(bookListReady(const QList<QString>)), this, SLOT(onBookListReady(const QList<QString>)));
        sdd->connectServer();
    }
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

    auto mfm = MediaFileManager::instance();
    bool eflReady = mfm->isExistingFileListReady();

    ui->pbDownloadBook->setEnabled(downloaded == false);
    ui->pbDownloadMediaFiles->setEnabled(downloaded == true && eflReady == true);
    ui->pbDownloadPronounceFiles->setEnabled(downloaded == true && eflReady == true);
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

    ServerDataDownloader *sdd = ServerDataDownloader::instance();
    sdd->downloadBook(bookName);
}

void ServerDataDialog::onDownloadProgress(float percentage)
{
    if (m_progressDialog.wasCanceled() == true)
    {
        destroyProgressDialog();
        ServerDataDownloader *sdd = ServerDataDownloader::instance();
        sdd->cancelDownloading();
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

void ServerDataDialog::on_pbDownloadMediaFiles_clicked()
{
    auto ci = ui->twBooks->currentItem();
    if (ci == nullptr)
    {
        return;
    }

    auto bookName = ci->text(0);
    downloadBookExampleAudioFiles(bookName);
}

void ServerDataDialog::on_pbDownloadPronounceFiles_clicked()
{
    auto ci = ui->twBooks->currentItem();
    if (ci == nullptr)
    {
        return;
    }

    auto bookName = ci->text(0);
    downloadBookPronounceFiles(bookName);
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

void ServerDataDialog::downloadBookPronounceFiles(QString bookName)
{
    // removing the progress dialog, we should NOT have very big word book (10000+ words), so this should be fast
    auto mfm = MediaFileManager::instance();
    QSet<QString> filesToDownload = mfm->missingPronounceAudioFiles(bookName);
    if (filesToDownload.isEmpty() == false)
    {
        // show the progress dialog
        createProgressDialog(QObject::tr("Downloading pronounce files ..."), QObject::tr("Cancel"));
        ServerDataDownloader *sdd = ServerDataDownloader::instance();
        sdd->downloadMultipleFiles(filesToDownload);
    }
    else
    {
        QMessageBox::information(this, MySettings::appName(), QObject::tr("All pronounce files are already available locally!"));
    }
}

void ServerDataDialog::downloadBookExampleAudioFiles(QString bookName)
{
    // removing the progress dialog, we should NOT have very big word book (10000+ words), so this should be fast
    auto mfm = MediaFileManager::instance();
    QSet<QString> filesToDownload = mfm->missingExampleAudioFiles(bookName);
    if (filesToDownload.isEmpty() == false)
    {
        // show the progress dialog
        createProgressDialog(QObject::tr("Downloading meida files ..."), QObject::tr("Cancel"));
        ServerDataDownloader *sdd = ServerDataDownloader::instance();
        sdd->downloadMultipleFiles(filesToDownload);
    }
    else
    {
        QMessageBox::information(this, MySettings::appName(), QObject::tr("All media files are already available locally!"));
    }
}

bool ServerDataDialog::fileExistsLocally(QString fileName)
{
    const QString dd = MySettings::dataDirectory() + "/";
    return QFile::exists(dd + fileName);
}
