#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "golddict/gddebug.hh"

#include <QString>
#include <QFileDialog>
//#include <QWebEngineSettings>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_dict(this),
    m_dictSchemeHandler(m_dict, this)
{
    ui->setupUi(this);

    // load LDOCE6 by default for covenience
    m_dict.loadMdx("/Users/huafeng/Documents/Nexus7/Dictionary/LDOCE6/LDOCE6.mdx");

    m_webEngineView = new QWebEngineView;
    ui->horizontalLayout_2->addWidget(m_webEngineView);
    m_dictSchemeHandler.installToWebEngingView(*m_webEngineView);
    //m_webEngineView->page()->
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionOpen_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open Dictionary");


    m_dict.loadMdx(fileName);
}


void MainWindow::on_pushButton_clicked()
{
    //QueryWord();
    m_webEngineView->load(QUrl("file:///Users/huafeng/Downloads/tickets/temp/test.html"));
    //m_webEngineView->setHtml(QString("<html><img src=\"/var/folders/l3/wbxd0lcj7j93qchzjvhnvblh0000gn/T/yGXqZC-spkr_b.png\"></html>"));
}

void MainWindow::on_lineEdit_returnPressed()
{
    QueryWord();
}

void MainWindow::QueryWord()
{
    if (m_dict.getDictionaries().size()) {
        QString word = ui->lineEdit->text();
        QString wordDefinition = m_dict.getWordDefinitionPage(word);
        m_webEngineView->setHtml(wordDefinition);
    }
}
