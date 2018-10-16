#include "serverdatadialog.h"
#include "ui_serverdatadialog.h"
#include "wordbook.h"
#include "mysettings.h"
#include "mediafilemanager.h"
#include "clienttoken.h"
#include "filedownloader.h"
#include "bookdownloader.h"

#include <QMessageBox>

ServerDataDialog::ServerDataDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ServerDataDialog)
{
    ui->setupUi(this);

    connect(ui->twBooks, SIGNAL(itemSelectionChanged()), this, SLOT(onItemSelectionChanged()));

    QStringList header;
    header.append(QObject::tr("Book Name"));
    header.append(QObject::tr("Status"));
    ui->twBooks->setHeaderLabels(header);

    downloadBookList();
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

    ui->pbDownloadBook->setEnabled(downloaded == false);
    ui->pbDownloadMediaFiles->setEnabled(downloaded == true
                                         && mfm->isDataReady() == true
                                         && mfm->bookMissingExampleAudioFiles(bookName)->isEmpty() == false);
    ui->pbDownloadPronounceFiles->setEnabled(downloaded == true
                                             && mfm->isDataReady() == true
                                             && mfm->bookMissingPronounceAudioFiles(bookName)->isEmpty() == false);
}

void ServerDataDialog::onBookListReady(const QList<QString> &books)
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
    if (ClientToken::instance()->promptUserToLogin(this,
                                                  QObject::tr("Downloading book requires a user to login first!")) == false)
    {
        return;
    }

    auto ci = ui->twBooks->currentItem();
    if (ci == nullptr)
    {
        return;
    }

    auto bookName = ci->text(0);
    downloadBook(bookName, true, QObject::tr("Downloading \"%1\" ...").arg(bookName), QObject::tr("Cancel"));
}

void ServerDataDialog::onFilesDownloadFinished(const QMap<QString, ServerCommunicator::DownloadStatus> &downloadResult)
{
    auto succeededFiles = downloadResult.keys(ServerCommunicator::DownloadSucceeded);
    auto mfm = MediaFileManager::instance();
    mfm->fileDownloaded(succeededFiles);

    qDebug() << "total files:" << downloadResult.size();
    qDebug() << "DownloadSucceeded:" << downloadResult.keys(ServerCommunicator::DownloadSucceeded).size();
    qDebug() << "DownloadFailed:" << downloadResult.keys(ServerCommunicator::DownloadFailed).size();
    qDebug() << "WaitingDataFromServer:" << downloadResult.keys(ServerCommunicator::WaitingDataFromServer).size();
    qDebug() << "DownloadCancelled:" << downloadResult.keys(ServerCommunicator::DownloadCancelled).size();
}

void ServerDataDialog::onBookDownloadFinished(QString bookName, ServerCommunicator::DownloadStatus result)
{
    if (result == ServerCommunicator::DownloadSucceeded)
    {
        onBookDownloaded(bookName);
    }
    else if (result == ServerCommunicator::DownloadFailed)
    {
        QMessageBox::critical(this, MySettings::appName(), QObject::tr("Downloading %1 failed!").arg(bookName));
    }
    else
    {
        // cancelled, do nothing
    }
}

void ServerDataDialog::onBookDownloaded(QString bookName)
{
    updateBookStatus(bookName);
    onItemSelectionChanged();
    emit(bookDownloaded(bookName));
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
    if (ClientToken::instance()->promptUserToLogin(this,
                                                  QObject::tr("Downloading media files requires a user to login first!")) == false)
    {
        return;
    }

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
    if (ClientToken::instance()->promptUserToLogin(this,
                                                  QObject::tr("Downloading pronounce files requires a user to login first!")) == false)
    {
        return;
    }

    auto ci = ui->twBooks->currentItem();
    if (ci == nullptr)
    {
        return;
    }

    auto bookName = ci->text(0);
    downloadBookPronounceFiles(bookName);
}

void ServerDataDialog::downloadBookPronounceFiles(QString bookName)
{
    // removing the progress dialog, we should NOT have very big word book (10000+ words), so this should be fast
    auto mfm = MediaFileManager::instance();
    auto filesToDownload = mfm->bookMissingPronounceAudioFiles(bookName);
    Q_ASSERT(filesToDownload.get() != nullptr);
    if (filesToDownload->isEmpty() == false)
    {
        downloadFiles(*filesToDownload, true, QObject::tr("Downloading pronounce files ..."), QObject::tr("Cancel"));

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
    auto filesToDownload = mfm->bookMissingExampleAudioFiles(bookName);
    Q_ASSERT(filesToDownload.get() != nullptr);
    if (filesToDownload->isEmpty() == false)
    {
        downloadFiles(*filesToDownload, true, QObject::tr("Downloading meida files ..."), QObject::tr("Cancel"));
    }
    else
    {
        QMessageBox::information(this, MySettings::appName(), QObject::tr("All media files are already available locally!"));
    }
}

bool ServerDataDialog::fileExistsLocally(QString fileName)
{
    QString dd = MySettings::dataDirectory() + "/";
    return QFile::exists(dd + fileName);
}


void ServerDataDialog::downloadFiles(const QSet<QString> &setFiles, bool showProgress, QString labelText, QString cancelButtonText)
{
    FileDownloader *fd = new FileDownloader();
    connect(fd, SIGNAL(downloadFinished(const QMap<QString, ServerCommunicator::DownloadStatus> &)), this, SLOT(onFilesDownloadFinished(const QMap<QString, ServerCommunicator::DownloadStatus> &)));
    connect(fd, &FileDownloader::downloadFinished, [fd] () {
        fd->deleteLater();
        qDebug() << "fd->deleteLater() called";
    });
    fd->setShowProgress(showProgress, labelText, cancelButtonText, this);
    QStringList ftdList = setFiles.toList();
    fd->downloadFiles(ftdList);
}

void ServerDataDialog::downloadBook(QString bookName, bool showProgress, QString labelText, QString cancelButtonText)
{
    BookDownloader *bd = new BookDownloader();
    connect(bd, SIGNAL(downloadFinished(QString, ServerCommunicator::DownloadStatus)), this, SLOT(onBookDownloadFinished(QString, ServerCommunicator::DownloadStatus)));
    connect(bd, &BookDownloader::downloadFinished, [bd] () {
        bd->deleteLater();
        qDebug() << "bd->deleteLater() called";
    });
    bd->setShowProgress(showProgress, labelText, cancelButtonText, this);
    bd->downloadBook(bookName);
}

void ServerDataDialog::downloadBookList()
{
    BookDownloader *bd = new BookDownloader();
    connect(bd, SIGNAL(bookListDownloaded(const QList<QString> &)), this, SLOT(onBookListReady(const QList<QString> &)));
    connect(bd, &BookDownloader::bookListDownloaded, [bd] () {
        bd->deleteLater();
        qDebug() << "bd->deleteLater() called";
    });
    bd->downloadBookList();
}
