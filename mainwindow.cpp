#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "golddict/gddebug.hh"
#include "word.h"

#include <QString>
#include <QFileDialog>
//#include <QWebEngineSettings>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_gdhelper(nullptr),
    m_definitionView(this),
    m_studyWindow(nullptr)
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
    QString spelling = ui->lineEdit->text();
    if (spelling.isEmpty()) {
        return;
    }

    auto word = Word::getWordFromDatabase(spelling);
    if (!word.get()) {
        // get the word from dictionary and save to database
        saveWord(spelling);
        // get it again
        word = Word::getWordFromDatabase(spelling);
    }

    QString html = word->getDefinition();
    QUrl baseUrl("file://" + QCoreApplication::applicationDirPath() + "/");
    m_definitionView.setHtml(html, baseUrl);
}

void MainWindow::saveWord(const QString &spelling)
{
    QString html = m_gdhelper.getWordDefinitionPage(spelling);
    Word word(spelling);
    word.setDefinition(html);
}

void MainWindow::TestHtmlParse()
{
    QString word = ui->lineEdit->text();
    QString html = m_gdhelper.getWordDefinitionPage(word);

    QUrl baseUrl("file://" + QCoreApplication::applicationDirPath() + "/");
    m_definitionView.setHtml(html, baseUrl);
}

void MainWindow::on_pushButton_2_clicked()
{
    m_studyWindow.setStudyList(StudyList::generateStudyListForAllWord());
    m_studyWindow.show();
}
