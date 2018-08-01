#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "golddict/gddebug.hh"
#include "HaiBeiDanCi/word.h"
#include "HaiBeiDanCi/wordbook.h"

#include <QString>
#include <QFileDialog>
#include <QMessageBox>
#include <QDateTime>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_gdhelper(nullptr),
    m_definitionView(this),
    m_studyWindow(nullptr),
    m_newbookWindow(m_gdhelper, this)
{
    ui->setupUi(this);



    // load LDOCE6 by default for covenience
    m_gdhelper.loadDict("/Users/huafeng/Documents/Nexus7/Dictionary/LDOCE6/LDOCE6.mdx");

    //auto definitionView = m_gdhelper.getDefinitionView();
    //ui->horizontalLayout_2->addWidget(&m_wordView);
    ui->horizontalLayout_2->addWidget(&m_definitionView);
    m_gdhelper.loadBlankPage(m_definitionView);
    this->resize(800, 600);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionOpen_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open Dictionary");

    m_gdhelper.loadDict(fileName);
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

    auto word = Word::getWordFromDatabase(spelling);
    if (word.get()) {
        QString html = word->getDefinition();
        QUrl baseUrl("file://" + QCoreApplication::applicationDirPath() + "/");
        m_definitionView.setHtml(html, baseUrl);
    }
}

void MainWindow::on_pushButton_2_clicked()
{
    QDateTime start = QDateTime::currentDateTime();

    m_studyWindow.setStudyList(StudyList::generateStudyListForAllWord());

    QDateTime end = QDateTime::currentDateTime();
    gdDebug("used %lld seconds", start.secsTo(end));

    m_studyWindow.show();
}

void MainWindow::on_pushNewBook_clicked()
{
    m_newbookWindow.show();
}
