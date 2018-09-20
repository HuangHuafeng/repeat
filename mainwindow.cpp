#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "golddict/gddebug.hh"
#include "HaiBeiDanCi/word.h"
#include "HaiBeiDanCi/wordbook.h"
#include "HaiBeiDanCi/mysettings.h"
#include "HaiBeiDanCi/svragt.h"
#include "HaiBeiDanCi/mediafilemanager.h"
#include "newbook.h"
#include "servermanager.h"
#include "HaiBeiDanCi/serverdatadownloader.h"
#include "preferencesdialog.h"
#include "HaiBeiDanCi/logindialog.h"
#include "HaiBeiDanCi/clienttoken.h"

#include <QString>
#include <QFileDialog>
#include <QMessageBox>
#include <QDateTime>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_gdhelper(nullptr),
    m_definitionView(this),
    m_refreshTimer(this)
{
    ui->setupUi(this);

    initializeProgressDialog();

    QStringList header;
    header.append(QObject::tr("Name"));
    header.append(QObject::tr("Words"));
    header.append(QObject::tr("Missing Media Files"));
    ui->twLocalData->setHeaderLabels(header);
    ui->twServerData->setHeaderLabels(header);

    if (WordDB::initialize() == false) {
        QMessageBox::critical(this, "MySettings::appName()", MainWindow::tr("database error"));
    }

    // load LDOCE6 by default for covenience
    m_gdhelper.loadDict("/Users/huafeng/Documents/Nexus7/Dictionary/LDOCE6/LDOCE6.mdx");

    //auto definitionView = m_gdhelper.getDefinitionView();
    //ui->horizontalLayout_2->addWidget(&m_wordView);
    ui->horizontalLayout_2->addWidget(&m_definitionView);
    m_gdhelper.loadBlankPage(m_definitionView);
    this->resize(800, 600);

    ServerManager *serverManager = ServerManager::instance();
    connect(serverManager, SIGNAL(serverDataReloaded()), this, SLOT(onServerDataReloaded()));
    connect(serverManager, SIGNAL(uploadProgress(float)), this, SLOT(onUploadProgress(float)));
    connect(serverManager, SIGNAL(bookDownloaded(QString)), this, SLOT(onBookDownloaded(QString)));

    // call MediaFileManager::instance() to get the existing file list ready
    MediaFileManager::instance();

    // start a timer to refresh the missing files.
    connect(&m_refreshTimer, SIGNAL(timeout()), this, SLOT(onRefreshTimerTimeout()));
    m_refreshTimer.start(1000);

    reloadLocalData();
}

MainWindow::~MainWindow()
{
    WordDB::shutdown();
    delete ui;
}

void MainWindow::on_actionOpen_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open Dictionary");

    if (fileName.isEmpty() == false)
    {
        m_gdhelper.loadDict(fileName);
    }
}


void MainWindow::on_pushButton_clicked()
{
    QueryWord();
}

void MainWindow::on_lineEdit_returnPressed()
{
    QueryWord();
}

void MainWindow::QueryWord()
{
    m_gdhelper.loadBlankPage(m_definitionView);

    QString spelling = ui->lineEdit->text();
    if (spelling.isEmpty()) {
        return;
    }

    if (m_gdhelper.saveWord(spelling) == false) {
        QMessageBox::information(this, QObject::tr(""),
            QObject::tr("Cannot find the word ") + "\"" + spelling + "\"");
    }

    auto word = Word::getWordToRead(spelling);
    if (word != nullptr) {
        //QString html = word->getDefinition();
        QString html = word->getDefinitionDIV();
        QUrl baseUrl("file:///" + MySettings::dataDirectory() + "/");
        m_definitionView.setHtml(html, baseUrl);
    }
}

void MainWindow::on_pushNewBook_clicked()
{
    ui->actionNewBook->trigger();
}

void MainWindow::on_actionNewBook_triggered()
{
    NewBook newBookWindow(m_gdhelper, this);
    if (newBookWindow.exec() == QDialog::Accepted)
    {
        reloadLocalBooks();
    }
}

void MainWindow::reloadLocalData()
{
    auto wordList = Word::getAllWords();
    ui->labelLocalWords->setText(QString::number(wordList.size()));

    auto bookList = WordBook::getAllBooks();
    ui->labelLocalBooks->setText(QString::number(bookList.size()));

    reloadLocalBooks();
}

void MainWindow::reloadLocalBooks()
{
    ui->twLocalData->clear();

    auto bookList = WordBook::getAllBooks();

    for (int i = 0; i < bookList.size(); i++)
    {
        auto book = WordBook::getBook(bookList.at(i));
        if (book.get())
        {
            addBookToTheView(ui->twLocalData, *book);
        }
    }

    selectFirstItem(ui->twLocalData);
}

void MainWindow::selectFirstItem(QTreeWidget *tw)
{
    if (tw == nullptr)
    {
        return;
    }

    tw->sortItems(0, Qt::SortOrder::AscendingOrder);
    tw->resizeColumnToContents(0);

    // select the first book
    QTreeWidgetItemIterator it(tw);
    if (*it)
    {
        tw->setCurrentItem(*it);
    }
}

void MainWindow::addBookToTheView(QTreeWidget * tw, WordBook &book)
{
    auto wordList = book.getAllWords();
    QString numberOfWords = QString::number(wordList.size());
    QString bookName = book.getName();

    QStringList infoList;
    infoList.append(bookName);
    infoList.append(numberOfWords);
    auto mfm = MediaFileManager::instance();
    if (mfm->isDataReady() == true)
    {
        QSet<QString> missingMediaFiles;
        auto bookMissingPronounceAudioFiles = mfm->bookMissingPronounceAudioFiles(bookName);
        if (bookMissingPronounceAudioFiles.get() != nullptr)
        {
            missingMediaFiles += *bookMissingPronounceAudioFiles;
        }

        auto bookMissingExampleAudioFiles = mfm->bookMissingExampleAudioFiles(bookName);
        if (bookMissingExampleAudioFiles.get() != nullptr)
        {
            missingMediaFiles += *bookMissingExampleAudioFiles;
        }

        QString numberOfMissingMediaFiles = QString::number(missingMediaFiles.size());
        infoList.append(numberOfMissingMediaFiles);
    }
    QTreeWidgetItem *item = new QTreeWidgetItem(infoList);
    tw->addTopLevelItem(item);
}

void MainWindow::onBookDownloaded(QString bookName)
{
    auto mfm = MediaFileManager::instance();
    mfm->bookDownloaded(bookName);
    reloadLocalData();
    m_refreshTimer.start();
}

void MainWindow::onServerDataReloaded()
{
    ServerManager *serverManager = ServerManager::instance();
    auto books = serverManager->getBookList();
    listServerBooks(books);

    // number of books
    ui->labelServerBooks->setText(QString::number(books.size()));

    // number of words
    auto allWords = serverManager->getAllWords();
    QString serverWords = QString::number(allWords.size());
    ui->labelServerWords->setText(serverWords);
}

void MainWindow::onUploadProgress(float percentage)
{
    m_progressDialog.setValue(static_cast<int>(100 * percentage));
}

void MainWindow::listServerBooks(const QList<QString> books)
{
    ui->twServerData->clear();

    ServerManager *serverManager = ServerManager::instance();
    for (int i = 0;i < books.size();i ++)
    {
        QString bookName = books.at(i);

        auto wordList = serverManager->getWordListOfBook(bookName);
        QString numberOfWords = QString::number(wordList.size());

        auto missingMediaFiles = serverManager->getMissingMediaFilesOfBook(bookName);
        QString numberOfMissingMediaFiles = QString::number(missingMediaFiles.size());

        QStringList infoList;
        infoList.append(bookName);
        infoList.append(numberOfWords);
        infoList.append(numberOfMissingMediaFiles);
        QTreeWidgetItem *item = new QTreeWidgetItem(infoList);
        ui->twServerData->addTopLevelItem(item);
    }

    selectFirstItem(ui->twServerData);
}

void MainWindow::on_actionPreferences_triggered()
{
    PreferencesDialog pd(this);
    pd.exec();
}

void MainWindow::on_pbReloadServerData_clicked()
{
    ui->actionReload_data->trigger();
}

void MainWindow::on_actionReload_data_triggered()
{
    ServerManager *serverManager = ServerManager::instance();
    serverManager->reloadServerData();
}

void MainWindow::on_actionDeleteLocalBook_triggered()
{
    auto ci = ui->twLocalData->currentItem();
    if (ci == nullptr)
    {
        return;
    }

    auto bookName = ci->text(0);
    WordBook::deleteBook(bookName);
    reloadLocalData();
}

void MainWindow::on_pbDeleteBook_clicked()
{
    ui->actionDeleteLocalBook->trigger();
}

void MainWindow::on_actionUpload_Book_triggered()
{
    auto ci = ui->twLocalData->currentItem();
    if (ci == nullptr)
    {
        return;
    }

    if (okToPerformServerRelatedOperation() == false)
    {
        return;
    }

    auto bookName = ci->text(0);
    ServerManager *serverManager = ServerManager::instance();
    if (serverManager->bookExistsInServer(bookName) == true)
    {
        QMessageBox::information(this, MySettings::appName(), "Book \"" + bookName + "\" already exists in server.");
        return;
    }

    createProgressDialog("uploading book \"" + bookName + "\" ...", QString());
    serverManager->uploadBook(bookName);
}

void MainWindow::on_pbUploadBook_clicked()
{
    ui->actionUpload_Book->trigger();
}

void MainWindow::on_actionDownload_Book_triggered()
{
    auto ci = ui->twServerData->currentItem();
    if (ci == nullptr)
    {
        return;
    }

    auto bookName = ci->text(0);
    auto localBook = WordBook::getBook(bookName);
    if (localBook.get() != nullptr)
    {
        QMessageBox::information(this, MySettings::appName(), "Book \"" + bookName + "\" already exists locally.");
        return;
    }

    if (okToPerformServerRelatedOperation() == false)
    {
        return;
    }

    ServerManager *serverManager = ServerManager::instance();
    serverManager->downloadBook(bookName);
}

void MainWindow::on_pbDownloadServerBook_clicked()
{
    ui->actionDownload_Book->trigger();
}

void MainWindow::on_actionDeleteServerBook_triggered()
{
    auto ci = ui->twServerData->currentItem();
    if (ci == nullptr)
    {
        return;
    }

    if (okToPerformServerRelatedOperation() == false)
    {
        return;
    }

    auto bookName = ci->text(0);
    ServerManager *serverManager = ServerManager::instance();
    serverManager->deleteBook(bookName);
}

void MainWindow::on_pbDeleteServerBook_clicked()
{
    ui->actionDeleteServerBook->trigger();
}

void MainWindow::onRefreshTimerTimeout()
{
    auto mfm = MediaFileManager::instance();
    if (mfm->isDataReady() == true)
    {
        // data of all the books is ready
        qDebug() << "MediaFileManager data is ready!";
        reloadLocalData();
        m_refreshTimer.stop();
    }
    else
    {
        qDebug() << "Waiting for MediaFileManager!";
    }
}

void MainWindow::on_actionFetch_Missing_Media_Files_triggered()
{
    auto ci = ui->twLocalData->currentItem();
    if (ci == nullptr)
    {
        return;
    }
    auto bookName = ci->text(0);

    auto mfm = MediaFileManager::instance();

    QSet<QString> missingMediaFiles;
    auto missingPronounceAudioFiles = mfm->bookMissingPronounceAudioFiles(bookName);
    if (missingPronounceAudioFiles.get() != nullptr)
    {
        missingMediaFiles += *missingPronounceAudioFiles;
    }
    auto missingExampleAudioFiles = mfm->bookMissingExampleAudioFiles(bookName);
    if (missingExampleAudioFiles.get() != nullptr)
    {
        missingMediaFiles += *missingExampleAudioFiles;
    }

    QProgressDialog pd("fetching media files from dictionary ...", "Cancel", 0, missingMediaFiles.size() - 1, this);
    int counter = 0;
    QSet<QString>::const_iterator it = missingMediaFiles.constBegin();
    while (it != missingMediaFiles.constEnd()) {
        m_gdhelper.fecthAndSaveFile((*it).mid(5));
        mfm->fileDownloaded(*it);
        it ++;

        pd.setValue(counter);
        counter ++;
        if (pd.wasCanceled() == true)
        {
            break;
        }
    }

    reloadLocalData();
}

bool MainWindow::okToPerformServerRelatedOperation()
{
    ServerManager *serverManager = ServerManager::instance();
    QString errorString;
    if (serverManager->okToSync(&errorString) == false)
    {
        QMessageBox::critical(this, MySettings::appName(), errorString);
        return false;
    }

    return true;
}

void MainWindow::on_actionUpload_Book_Missing_Media_Files_triggered()
{
    auto ci = ui->twLocalData->currentItem();
    if (ci == nullptr)
    {
        return;
    }

    if (okToPerformServerRelatedOperation() == false)
    {
        return;
    }

    auto bookName = ci->text(0);
    ServerManager *serverManager = ServerManager::instance();
    if (serverManager->bookExistsInServer(bookName) == false)
    {
        QMessageBox::information(this, MySettings::appName(), "Book \"" + bookName + "\" does not exist in server.");
        return;
    }

    auto missingMediaFiles = serverManager->getMissingMediaFilesOfBook(bookName);
    if (missingMediaFiles.isEmpty() == true)
    {
        // no file missing
        QMessageBox::information(this, MySettings::appName(), "All media files of book \"" + bookName + "\" are already available in server.");
        return;
    }

    createProgressDialog("uploading media files of book \"" + bookName + "\" ...", QString());
    serverManager->uploadBookMissingMediaFiles(bookName);
}


void MainWindow::initializeProgressDialog()
{
    m_progressDialog.setModal(true);
    m_progressDialog.cancel();
}

void MainWindow::createProgressDialog(const QString &labelText, const QString &cancelButtonText)
{
    m_progressDialog.reset();
    m_progressDialog.setLabelText("    " + labelText + "    ");
    m_progressDialog.setCancelButtonText(cancelButtonText);
    m_progressDialog.setValue(0);
}

void MainWindow::on_actionLogin_triggered()
{
    auto ct = ClientToken::instance();
    if (ct->hasAliveToken() == true
            && ct->hasValidUser() == true)
    {
        auto answer = QMessageBox::warning(this,
                                           MySettings::appName(),
                                           "\"" + ct->user().name() + "\" " + QObject::tr("already logged in. Would you like to logout?"),
                                           QMessageBox::Yes,
                                           QMessageBox::No);
        if (answer == QMessageBox::Yes)
        {
            ui->actionLogout->trigger();
        }
        else
        {
            return;
        }
    }

    LoginDialog ld(this);
    auto result = ld.exec();
    if (result == QDialog::Accepted)
    {
        // a user logged in
        ui->actionReload_data->trigger();
    }
    else
    {
        // no user is registered and the dialog is cancelled
    }
}

void MainWindow::on_actionLogout_triggered()
{
    auto ct = ClientToken::instance();
    if (ct->hasAliveToken() == true
            && ct->hasValidUser() == true)
    {
        ServerUserAgent *sua = new ServerUserAgent(this);
        connect(sua, &ServerUserAgent::logoutSucceeded, [sua] (QString name) { sua->deleteLater(); qDebug() << "sua->deleteLater() called for" << name; });
        sua->logoutUser(ct->user().name());
        ct->setToken(Token::invalidToken);
        ct->setUser(ApplicationUser::invalidUser);
    }
}

#ifndef QT_NO_DEBUG
void MainWindow::test()
{
    static int counter = 1;
    qDebug() << "==== TEST" << counter << "START ====";

    auto allLocalWords = Word::getAllWords();

    for (int i = 0;i < allLocalWords.size();i ++)
    {
        auto spelling = allLocalWords.at(i);
        auto localWord = Word::getWordToRead(spelling);
        Q_ASSERT(localWord != nullptr);
        qDebug() << localWord->getId() << localWord->getSpelling() << localWord->getDefinition().size();
    }

    qDebug() << "==== TEST" << counter << "END ====";
    counter ++;
}
#endif

void MainWindow::on_actionUsers_triggered()
{

}

void MainWindow::on_actionRelease_App_triggered()
{

}
