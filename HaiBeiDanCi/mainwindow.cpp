#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "../golddict/gddebug.hh"

#include <QTreeWidgetItem>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_studyWindow(nullptr),
    m_bookBrowser(nullptr)
{
    ui->setupUi(this);
    listBooks();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::listBooks()
{
    QStringList header;
    header.append(QObject::tr("Book Name"));
    header.append(QObject::tr("Total Words"));
    header.append(QObject::tr("Book Introduction"));
    ui->twBooks->setHeaderLabels(header);

    auto bookList = WordBook::getWordBooks();

    for (int i = 0;i < bookList.size();i ++) {
        auto book = bookList.at(i);
        if (book.get()) {
            addBookToTheView(*book);
        }
    }
}

void MainWindow::addBookToTheView(WordBook &book)
{
    int totalWords = book.totalWords();
    QStringList infoList;
    infoList.append(book.getName());
    infoList.append(QString::number(totalWords));
    infoList.append(book.getIntroduction());
    QTreeWidgetItem *item = new QTreeWidgetItem(infoList);
    ui->twBooks->addTopLevelItem(item);
}

void MainWindow::on_pushTest_clicked()
{
    auto studyList = StudyList::allWords();
    startStudy(studyList);

    /* if the window is created on heap, when to delete it?!
    auto sw = new StudyWindow(this);
    sw->setStudyList(StudyList::allWords());
    sw->show();
    */
}

void MainWindow::on_pushBrowse_clicked()
{
    auto bookName = ui->twBooks->currentItem()->text(0);
    if (m_bookBrowser.setBook(bookName) == true) {
        m_bookBrowser.show();
    }
}

void MainWindow::on_pushStudy_clicked()
{
    auto bookName = ui->twBooks->currentItem()->text(0);
    auto studyList = StudyList::allNewWordsInBook(bookName);
    startStudy(studyList);
}

void MainWindow::on_pushReview_clicked()
{
    auto bookName = ui->twBooks->currentItem()->text(0);
    auto studyList = StudyList::allStudiedWordsInBook(bookName);
    startStudy(studyList);
}

void MainWindow::startStudy(sptr<StudyList> studyList)
{
    if (studyList.get() == 0
            || studyList->size() == 0) {
        QMessageBox::information(this,
                                 MainWindow::tr(""),
                                 MainWindow::tr("No word to study!"));
        return;
    }

    auto setRestul = m_studyWindow.setStudyList(studyList);
    if (setRestul) {
        m_studyWindow.reloadView();
        m_studyWindow.show();
    } else {
        QMessageBox::critical(this,
                                 MainWindow::tr(""),
                                 MainWindow::tr("failed to set the study list!"));
    }
}

void MainWindow::on_pushRevSd_clicked()
{
    auto bookName = ui->twBooks->currentItem()->text(0);
    auto studyList = StudyList::allWordsInBook(bookName);
    startStudy(studyList);
}

void MainWindow::on_pushExpired_clicked()
{
    auto bookName = ui->twBooks->currentItem()->text(0);

    auto expire = QDateTime::currentDateTime().addDays(10); // add 10 days for test purpose!!!
    gdDebug("book: %s", bookName.toStdString().c_str());
    gdDebug("going to get words expired on %lld, the date is %s", MyTime(expire).toMinutes(), expire.toString().toStdString().c_str());
    //auto studyList = StudyList::allExpiredWords(expire);

    auto studyList = StudyList::allExpiredWordsInBook(bookName, expire);

    startStudy(studyList);
}
