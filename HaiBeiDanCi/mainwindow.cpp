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
    connect(ui->twBooks, SIGNAL(itemSelectionChanged()), this, SLOT(onItemSelectionChanged()));

    listBooks();

    updateAllBooksData();
    connect(&m_studyWindow, SIGNAL(wordStudied(QString)), this, SLOT(updateAllBooksData()));
    connect(&m_studyWindow, SIGNAL(wordStudied(QString)), this, SLOT(updateCurrentBookData()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::listBooks()
{
    QStringList header;
    header.append(MainWindow::tr("Book Name"));
    header.append(MainWindow::tr("Book Introduction"));
    ui->twBooks->setHeaderLabels(header);

    auto bookList = WordBook::getWordBooks();

    for (int i = 0;i < bookList.size();i ++) {
        auto book = bookList.at(i);
        if (book.get()) {
            addBookToTheView(*book);
        }
    }

    ui->twBooks->sortItems(0, Qt::SortOrder::AscendingOrder);

    // select the first book
    QTreeWidgetItemIterator it(ui->twBooks);
    if (*it) {
        ui->twBooks->setCurrentItem(*it);
    }
}

void MainWindow::onItemSelectionChanged()
{
    updateCurrentBookData();
}

void MainWindow::updateCurrentBookData()
{
    // expired words
    auto expired = expiredWordsFromCurrentBook();
    int numOfWords = 0;
    if (expired.get()) {
        numOfWords = expired->size();
    }
    ui->labelExpired->setText(QString::number(numOfWords) + MainWindow::tr(" words"));
    ui->pushBrowseExpiredWords->setEnabled(numOfWords > 0);
    ui->pushStudyExpiredWords->setEnabled(numOfWords > 0);

    // old words
    auto old = oldWordsFromCurrentBook();
    numOfWords = 0;
    if (old.get()) {
        numOfWords = old->size();
    }
    ui->labelOld->setText(QString::number(numOfWords) + MainWindow::tr(" words"));
    ui->pushBrowseOldWords->setEnabled(numOfWords > 0);
    ui->pushStudyOldWords->setEnabled(numOfWords > 0);

    // new words
    auto newWords = newWordsFromCurrentBook();
    numOfWords = 0;
    if (newWords.get()) {
        numOfWords = newWords->size();
    }
    ui->labelNew->setText(QString::number(numOfWords) + MainWindow::tr(" words"));
    ui->pushBrowseNewWords->setEnabled(numOfWords > 0);
    ui->pushStudyNewWords->setEnabled(numOfWords > 0);

    // all words
    auto all = allWordsFromCurrentBook();
    numOfWords = 0;
    if (all.get()) {
        numOfWords = all->size();
    }
    ui->labelAll->setText(QString::number(numOfWords) + MainWindow::tr(" words"));
    ui->pushBrowseAllWords->setEnabled(numOfWords > 0);
    ui->pushStudyAllWords->setEnabled(numOfWords > 0);
}

void MainWindow::updateAllBooksData()
{
    // expired words
    auto expired = StudyList::allExpiredWords();
    auto numOfWords = 0;
    if (expired.get()) {
        numOfWords = expired->size();
    }
    ui->labelGlobalExpired->setText(QString::number(numOfWords) + MainWindow::tr(" words"));
    ui->pushGlobalBrowseExpiredWords->setEnabled(numOfWords > 0);
    ui->pushGlobalStudyExpiredWords->setEnabled(numOfWords > 0);

    // old words
    auto old = StudyList::allOldWords();
    numOfWords = 0;
    if (old.get()) {
        numOfWords = old->size();
    }
    ui->labelGlobalOld->setText(QString::number(numOfWords) + MainWindow::tr(" words"));
    ui->pushGlobalBrowseOldWords->setEnabled(numOfWords > 0);
    ui->pushGlobalStudyOldWords->setEnabled(numOfWords > 0);

    // new words
    auto newWords = StudyList::allNewWords();
    numOfWords = 0;
    if (newWords.get()) {
        numOfWords = newWords->size();
    }
    ui->labelGlobalNew->setText(QString::number(numOfWords) + MainWindow::tr(" words"));
    ui->pushGlobalBrowseNewWords->setEnabled(numOfWords > 0);
    ui->pushGlobalStudyNewWords->setEnabled(numOfWords > 0);

    // all words
    auto all = StudyList::allWords();
    numOfWords = 0;
    if (all.get()) {
        numOfWords = all->size();
    }
    ui->labelGlobalAll->setText(QString::number(numOfWords) + MainWindow::tr(" words"));
    ui->pushGlobalBrowseAllWords->setEnabled(numOfWords > 0);
    ui->pushGlobalStudyAllWords->setEnabled(numOfWords > 0);
}

void MainWindow::addBookToTheView(WordBook &book)
{
    QStringList infoList;
    infoList.append(book.getName());
    infoList.append(book.getIntroduction());
    QTreeWidgetItem *item = new QTreeWidgetItem(infoList);
    ui->twBooks->addTopLevelItem(item);
}

void MainWindow::startStudy(sptr<StudyList> studyList)
{
    if (studyList.get() == nullptr
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
    if (studyList.get() == nullptr
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
    return StudyList::allOldWordsInBook(bookName);
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

void MainWindow::on_pushGlobalStudyAllWords_clicked()
{
    auto studyList = StudyList::allWords();
    startStudy(studyList);
}

void MainWindow::on_pushGlobalBrowseAllWords_clicked()
{
    auto studyList = StudyList::allWords();
    startBrowse(studyList);
}

void MainWindow::on_pushGlobalStudyNewWords_clicked()
{
    auto studyList = StudyList::allNewWords();
    startStudy(studyList);
}

void MainWindow::on_pushGlobalBrowseNewWords_clicked()
{
    auto studyList = StudyList::allNewWords();
    startBrowse(studyList);
}

void MainWindow::on_pushGlobalStudyOldWords_clicked()
{
    auto studyList = StudyList::allOldWords();
    startStudy(studyList);
}

void MainWindow::on_pushGlobalBrowseOldWords_clicked()
{
    auto studyList = StudyList::allOldWords();
    startBrowse(studyList);
}

void MainWindow::on_pushGlobalStudyExpiredWords_clicked()
{
    auto studyList = StudyList::allExpiredWords();
    startStudy(studyList);
}

void MainWindow::on_pushGlobalBrowseExpiredWords_clicked()
{
    auto studyList = StudyList::allExpiredWords();
    startBrowse(studyList);
}

