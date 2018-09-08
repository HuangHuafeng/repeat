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

#include <QString>
#include <QFileDialog>
#include <QMessageBox>
#include <QDateTime>
#include <QProgressDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_gdhelper(nullptr),
    m_definitionView(this)
{
    ui->setupUi(this);

    QStringList header;
    header.append(QObject::tr("Name"));
    header.append(QObject::tr("Words"));
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

    Word::readAllWordsFromDatabase();
    WordCard::readAllCardsFromDatabase();
    WordBook::readAllBooksFromDatabase();


    auto sdd = ServerDataDownloader::instance();
    connect(sdd, SIGNAL(bookStored(QString)), this, SLOT(onBookDownloaded(QString)));

    ServerManager *serverManager = ServerManager::instance();
    connect(serverManager, SIGNAL(serverDataReloaded()), this, SLOT(onServerDataReloaded()));

    // call MediaFileManager::instance() to get the existing file list ready
    MediaFileManager::instance();

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

    auto word = Word::getWord(spelling);
    if (word.get()) {
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
    QStringList infoList;
    infoList.append(book.getName());
    infoList.append(numberOfWords);
    QTreeWidgetItem *item = new QTreeWidgetItem(infoList);
    tw->addTopLevelItem(item);
}

void MainWindow::onBookDownloaded(QString bookName)
{
    qDebug() << bookName;

    reloadLocalData();
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

void MainWindow::listServerBooks(const QList<QString> books)
{
    ui->twServerData->clear();

    ServerManager *serverManager = ServerManager::instance();
    for (int i = 0;i < books.size();i ++)
    {
        QString bookName = books.at(i);
        auto wordList = serverManager->getWordListOfBook(bookName);
        QString numberOfWords = QString::number(wordList.size());
        QStringList infoList;
        infoList.append(bookName);
        infoList.append(numberOfWords);
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

    auto bookName = ci->text(0);

    ServerManager *serverManager = ServerManager::instance();
    serverManager->uploadBook(bookName);
}

void MainWindow::on_pbUploadBook_clicked()
{
    ui->actionUpload_Book->trigger();
}

void MainWindow::on_actionSync_To_Local_triggered()
{
    ServerManager *serverManager = ServerManager::instance();
    QString errorString;
    if (serverManager->okToSync(&errorString) == false)
    {
        QMessageBox::critical(this, MySettings::appName(), errorString);
    }
    else
    {
        serverManager->syncToLocal();
    }
}

void MainWindow::on_actionDownload_Book_triggered()
{
    ServerManager *serverManager = ServerManager::instance();
    QString errorString;
    if (serverManager->okToSync(&errorString) == false)
    {
        QMessageBox::critical(this, MySettings::appName(), errorString);
    }
    else
    {
        auto ci = ui->twServerData->currentItem();
        if (ci == nullptr)
        {
            return;
        }

        auto bookName = ci->text(0);
        serverManager->downloadBook(bookName);
    }
}

void MainWindow::on_pbDownloadServerBook_clicked()
{
    ui->actionDownload_Book->trigger();
}

void MainWindow::on_actionDeleteServerBook_triggered()
{
    ServerManager *serverManager = ServerManager::instance();
    auto ci = ui->twServerData->currentItem();
    if (ci == nullptr)
    {
        return;
    }

    auto bookName = ci->text(0);
    serverManager->deleteBook(bookName);
}

void MainWindow::on_pbDeleteServerBook_clicked()
{
    ui->actionDeleteServerBook->trigger();
}
