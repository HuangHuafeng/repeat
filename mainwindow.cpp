#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "golddict/mdx.hh"

#include <QString>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setCentralWidget(ui->textEdit);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionOpen_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open Dictionary");
    ui->textEdit->setText(fileName);

    sptr< Dictionary::Class > mdx = Mdx::makeDictionary(fileName.toStdString());
    if (mdx) {
        ui->textEdit->append(QString::fromStdString(mdx->getName()));
    } else {
        ui->textEdit->append("failed!");
    }
}
