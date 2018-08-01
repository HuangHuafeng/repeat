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

void MainWindow::on_pushTest_clicked()
{
    m_studyWindow.setStudyList(StudyList::generateStudyListForAllWords());
    m_studyWindow.show();
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

void MainWindow::on_pushButton_clicked()
{
    auto bookName = ui->twBooks->currentItem()->text(0);
    //m_studyWindow.setStudyList(StudyList::generateStudyListForAllWordsInBook(bookName));

    auto expire = QDateTime::currentDateTime().addDays(10);
    gdDebug("going to get words expired on %lld", MyTime(expire).toMinutes());
    auto studyList = StudyList::generateStudyListForAllExpiredWords(expire);
    if (studyList.get() == 0
            || studyList->getList().isEmpty()) {
        QMessageBox::information(this,
                                 MainWindow::tr(""),
                                 MainWindow::tr("No word to study!"));
        return;
    }

    auto setRestul = m_studyWindow.setStudyList(studyList);
    if (setRestul) {
        m_studyWindow.show();
    } else {
        QMessageBox::critical(this,
                                 MainWindow::tr(""),
                                 MainWindow::tr("failed to set the study list!"));
    }
}

void MainWindow::on_pushButton_2_clicked()
{
    auto bookName = ui->twBooks->currentItem()->text(0);
    if (m_bookBrowser.setBook(bookName) == true) {
        m_bookBrowser.show();
    }
}
