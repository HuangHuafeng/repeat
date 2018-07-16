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
    this->resize(800, 600);
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
    QueryWord();
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
