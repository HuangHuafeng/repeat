#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "worddb.h"
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
    m_worddb(),
    m_studyWindow(m_gdhelper, nullptr)
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
    word.setExpireTime(QDateTime::currentDateTime().addDays(10));
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
    auto wordList = Word::getNewWords(10);
    for (int i = 0;i < wordList.size();i ++) {
        auto word = wordList.at(i);
        gdDebug("%s: %s", word->getSpelling().toStdString().c_str(), word->getExpireTime().toString().toStdString().c_str());
    }
    m_studyWindow.show();
}


void MainWindow::searchLink(QTextFrame * parent)
{
    for( QTextFrame::iterator it = parent->begin(); !it.atEnd(); ++it )
    {
        QTextFrame *textFrame = it.currentFrame();
        QTextBlock textBlock = it.currentBlock();

        if( textFrame )
        {
            this->searchLink(textFrame);
        }
        else if( textBlock.isValid() )
        {
            this->searchLink(textBlock);
        }
    }
}

void MainWindow::searchLink(QTextBlock & parent)
{
    for(QTextBlock::iterator it = parent.begin(); !it.atEnd(); ++it)
    {
        QTextFragment textFragment = it.fragment();
        if( textFragment.isValid() )
        {
            QTextCharFormat textCharFormat = textFragment.charFormat();
            if( textCharFormat.isAnchor() )
            {
                 textCharFormat.anchorHref();  // <-- URL
                 gdDebug("%s", textCharFormat.anchorHref().toStdString().c_str());
            }
        }
    }
}
