#include "studywindow.h"
#include "ui_studywindow.h"

#include <QLabel>

StudyWindow::StudyWindow(GDHelper &gdhelper, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::StudyWindow),
    m_gdhelper(gdhelper),
    m_wordView(parent),
    m_definitionView(parent)
{
    ui->setupUi(this);

    ui->vlDefinition->addWidget(&m_wordView);
    ui->vlDefinition->addWidget(&m_definitionView);
    m_gdhelper.loadBlankPage(m_definitionView);
}

StudyWindow::~StudyWindow()
{
    delete ui;
}

void StudyWindow::on_pushButton_clicked()
{
    m_wordView.setWord("impeachment");
    m_gdhelper.lookupWord("impeachment", m_definitionView);
    //ui->pushButton->hide();
}
