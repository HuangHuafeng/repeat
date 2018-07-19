#include "studywindow.h"
#include "ui_studywindow.h"

#include <QLabel>

StudyWindow::StudyWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::StudyWindow),
    m_gdhelper(parent),
    m_wordView(parent)
{
    ui->setupUi(this);

    // load LDOCE6 by default for covenience
    m_gdhelper.loadDict("/Users/huafeng/Documents/Nexus7/Dictionary/LDOCE6/LDOCE6.mdx");

    m_wordView.setWord("Morning");
    ui->vlDefinition->addWidget(&m_wordView);

    m_gdhelper.lookupWord("impeachment");
    auto definitionView = m_gdhelper.getDefinitionView();
    ui->vlDefinition->addWidget(definitionView);
}

StudyWindow::~StudyWindow()
{
    delete ui;
}

void StudyWindow::on_pushButton_clicked()
{
    m_wordView.setWord("impeachment");
}
