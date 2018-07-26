#include "studywindow.h"
#include "ui_studywindow.h"
#include "wordcard.h"

#include <QLabel>

StudyWindow::StudyWindow(GDHelper &gdhelper, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::StudyWindow),
    m_gdhelper(gdhelper),
    m_wordView(parent),
    m_definitionView(parent),
    m_wordCard()
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

    m_wordCard = WordCard::generateCardForWord("impeach");
    if (m_wordCard.getRepitition() == 0) {
        m_wordCard.setEasiness(2.0);
    }
    m_wordCard.update(MemoryItem::Perfect);
    QString html = "<html>Repitition: " + QString::number(m_wordCard.getRepitition())
            + "</br>Interval: " + QString::number(m_wordCard.getInterval() / 24.0, 'f', 2)
            + "</br>Easiness: " + QString::number(m_wordCard.getEasiness())
            + "</html>";
    m_definitionView.setHtml(html);
}
