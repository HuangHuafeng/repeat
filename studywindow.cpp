#include "studywindow.h"
#include "ui_studywindow.h"
#include "wordcard.h"

#include <QLabel>
#include <QMessageBox>

StudyWindow::StudyWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::StudyWindow),
    m_wordView(parent),
    m_definitionView(parent)
{
    ui->setupUi(this);
    ui->vlDefinition->addWidget(&m_wordView);
    ui->vlDefinition->addWidget(&m_definitionView);
}

StudyWindow::~StudyWindow()
{
    delete ui;
}

void StudyWindow::setStudyList(sptr<StudyList> studyList)
{
    m_studyList = studyList;
    if (m_studyList.get()) {
        m_currentCard = m_studyList->nextCard();
    }
    showCurrentWord();
}

void StudyWindow::on_pushPerfect_clicked()
{
    nextWord(MemoryItem::Perfect);
}

void StudyWindow::showCurrentWord()
{
    if (m_currentCard.get()) {
        showCard(*m_currentCard);
    } else {
        allCardsFinished();
    }
}

void StudyWindow::allCardsFinished()
{
    QMessageBox::information(this,
                             QObject::tr(""),
                             "Congratulations! All cards finished!");
    close();
}

void StudyWindow::showCard(const WordCard &card)
{
    auto word = card.getWord();
    if (word.get()) {
        showWord(*word);
    }

    int blackout = card.estimatedInterval(MemoryItem::Blackout);
    int incorrect = card.estimatedInterval(MemoryItem::Incorrect);
    int ibcr = card.estimatedInterval(MemoryItem::IncorrectButCanRecall);
    int cwd = card.estimatedInterval(MemoryItem::CorrectWithDifficulty);
    int cah = card.estimatedInterval(MemoryItem::CorrectAfterHesitation);
    int perfect = card.estimatedInterval(MemoryItem::Perfect);

    ui->labelIncorrect->setText(minuteToString(ibcr));
    ui->labelCorrect3->setText(minuteToString(cwd));
    ui->labelCorrect4->setText(minuteToString(cah));
    ui->labelPerfect->setText(minuteToString(perfect));
}

QString StudyWindow::minuteToString(int minute)
{
    if (minute < 60) {
        return QString::number(minute) + " minute";
    }

    if (minute < 60 * 24) {
        auto hour = minute / 60;
        auto min = minute % 60;
        return QString::number(hour) + " hour " + QString::number(min) + " minute";
    }

    auto day = minute / 60 / 24;
    auto hour = (minute / 60) % 24;
    auto min = minute % 60;
    return QString::number(day) + " day " + QString::number(hour) + " hour " + QString::number(min) + " minute";
}

void StudyWindow::showWord(const Word &word)
{
    QUrl baseUrl("file://" + QCoreApplication::applicationDirPath() + "/");
    m_wordView.setWord(word.getSpelling());
    m_definitionView.setHtml(word.getDefinition(), baseUrl);
}

void StudyWindow::nextWord(MemoryItem::ResponseQuality responseQulity)
{
    if (m_studyList.get()) {
        m_studyList->responseToCurrent(m_currentCard, responseQulity);
        m_currentCard = m_studyList->nextCard();
        showCurrentWord();
    }
}

void StudyWindow::on_pushCorrect_clicked()
{
    nextWord(MemoryItem::CorrectAfterHesitation);
}

void StudyWindow::on_pushIncorrect_clicked()
{
    nextWord(MemoryItem::IncorrectButCanRecall);
}

void StudyWindow::on_pushCorrect3_clicked()
{
    nextWord(MemoryItem::CorrectWithDifficulty);
}
