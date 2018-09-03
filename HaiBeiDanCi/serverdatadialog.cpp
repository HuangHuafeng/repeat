#include "serverdatadialog.h"
#include "ui_serverdatadialog.h"
#include "wordbook.h"
#include "mysettings.h"
#include "serverdatadownloader.h"

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
        connect(sdd, SIGNAL(bookListReady(const QList<QString>)), this, SLOT(onBookListReady(const QList<QString>)));
        sdd->getBookList();
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

QSet<QString> & ServerDataDialog::removeExistingFiles(QSet<QString> &files)
{
    QSet<QString>::iterator it = files.begin();
    while (it != files.end())
    {
        QString fileName = *it;
        if (fileExistsLocally(fileName) == true)
        {
            it = files.erase(it);
        }
        else
        {
            it ++;
        }
    }

    return files;
}

void ServerDataDialog::downloadBookPronounceFiles(QString bookName)
{
    sptr<WordBook> book = WordBook::getBook(bookName);
    if (book.get() == nullptr)
    {
        return;
    }

    // build the list of media files
    QSet<QString> interestedFiles;
    QVector<QString> wordList = book->getAllWords();
    QProgressDialog pd("    " + QObject::tr("Preparing the list of files to be downloaded ...") + "    ", QObject::tr("Cancel"), 0, wordList.size(), this);
    pd.setModal(true);
    for (int i = 0;i < wordList.size();i ++)
    {
        pd.setValue(i + 1);
        if (pd.wasCanceled() == true)
        {
            return;
        }

        QString spelling = wordList.at(i);
        sptr<Word> word = Word::getWord(spelling);
        if (word.get() == nullptr)
        {
            continue;
        }
        QSet<QString> interestedWordFiles = word->pronounceFiles() + word->otherFiles();
        interestedFiles += removeExistingFiles(interestedWordFiles);
    }

    ServerDataDownloader *sdd = ServerDataDownloader::instance();
    auto filesToDownload = sdd->downloadMultipleFiles(interestedFiles);
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

void ServerDataDialog::downloadBookExampleAudioFiles(QString bookName)
{
    sptr<WordBook> book = WordBook::getBook(bookName);
    if (book.get() == nullptr)
    {
        return;
    }

    // build the list of media files
    QSet<QString> interestedFiles;
    QVector<QString> wordList = book->getAllWords();
    QProgressDialog pd("    " + QObject::tr("Preparing the list of files to be downloaded ...") + "    ", QObject::tr("Cancel"), 0, wordList.size(), this);
    pd.setModal(true);
    for (int i = 0;i < wordList.size();i ++)
    {
        pd.setValue(i + 1);
        if (pd.wasCanceled() == true)
        {
            return;
        }

        QString spelling = wordList.at(i);
        sptr<Word> word = Word::getWord(spelling);
        if (word.get() == nullptr)
        {
            continue;
        }
        QSet<QString> interestedWordFiles = word->exampleAudioFiles() + word->otherFiles();
        interestedFiles += removeExistingFiles(interestedWordFiles);
    }

    ServerDataDownloader *sdd = ServerDataDownloader::instance();
    auto filesToDownload = sdd->downloadMultipleFiles(interestedFiles);
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

bool ServerDataDialog::fileExistsLocally(QString fileName)
{
    const QString dd = MySettings::dataDirectory() + "/";
    return QFile::exists(dd + fileName);
}
