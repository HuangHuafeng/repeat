#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "../golddict/gddebug.hh"

#include <QTreeWidgetItem>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_studyWindow(nullptr),
    m_browserWindow(nullptr)
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

    ui->twBooks->sortItems(0, Qt::SortOrder::AscendingOrder);
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
                                 MainWindow::tr("failed to set the word list to study!"));
    }
}


void MainWindow::startBrowse(sptr<StudyList> studyList)
{
    if (studyList.get() == 0
            || studyList->size() == 0) {
        QMessageBox::information(this,
                                 MainWindow::tr(""),
                                 MainWindow::tr("No word to Browse!"));
        return;
    }

    auto setRestul = m_browserWindow.setWordList(studyList);
    if (setRestul) {
        m_browserWindow.reloadView();
        m_browserWindow.show();
    } else {
        QMessageBox::critical(this,
                                 MainWindow::tr(""),
                                 MainWindow::tr("failed to set the word list to browse!"));
    }
}

sptr<StudyList> MainWindow::expiredWordsFromCurrentBook()
{
    auto bookName = ui->twBooks->currentItem()->text(0);
    auto expire = QDateTime::currentDateTime();
    return StudyList::allExpiredWordsInBook(bookName, expire);
}

void MainWindow::on_pushStudyExpiredWords_clicked()
{
    startStudy(expiredWordsFromCurrentBook());
}

void MainWindow::on_pushBrowseExpiredWords_clicked()
{
    startBrowse(expiredWordsFromCurrentBook());
}

sptr<StudyList> MainWindow::oldWordsFromCurrentBook()
{
    auto bookName = ui->twBooks->currentItem()->text(0);
    return StudyList::allStudiedWordsInBook(bookName);
}

void MainWindow::on_pushStudyOldWords_clicked()
{
    startStudy(oldWordsFromCurrentBook());
}

void MainWindow::on_pushBrowseOldWords_clicked()
{
    startBrowse(oldWordsFromCurrentBook());
}

sptr<StudyList> MainWindow::newWordsFromCurrentBook()
{
    auto bookName = ui->twBooks->currentItem()->text(0);
    return StudyList::allNewWordsInBook(bookName);
}

void MainWindow::on_pushStudyNewWords_clicked()
{
    startStudy(newWordsFromCurrentBook());
}

void MainWindow::on_pushBrowseNewWords_clicked()
{
    startBrowse(newWordsFromCurrentBook());
}

sptr<StudyList> MainWindow::allWordsFromCurrentBook()
{
    auto bookName = ui->twBooks->currentItem()->text(0);
    return StudyList::allWordsInBook(bookName);
}

void MainWindow::on_pushStudyAllWords_clicked()
{
    startStudy(allWordsFromCurrentBook());
}

void MainWindow::on_pushBrowseAllWords_clicked()
{
    startBrowse(allWordsFromCurrentBook());
}
