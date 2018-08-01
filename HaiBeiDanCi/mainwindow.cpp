#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "../golddict/gddebug.hh"

#include <QTreeWidgetItem>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_studyWindow(nullptr)
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
    m_studyWindow.setStudyList(StudyList::generateStudyListForAllWord());
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
