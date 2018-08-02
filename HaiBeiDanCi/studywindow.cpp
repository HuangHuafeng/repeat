#include "studywindow.h"
#include "ui_studywindow.h"
#include "wordcard.h"
#include "../golddict/gddebug.hh"

#include <QLabel>
#include <QMessageBox>

StudyWindow::StudyWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::StudyWindow),
    m_wordView(parent),
    m_definitionView(parent)
{
    m_state = NoCard;
    ui->setupUi(this);
    ui->vlDefinition->addWidget(&m_wordView);
    ui->vlDefinition->addWidget(&m_definitionView);
}

StudyWindow::~StudyWindow()
{
    delete ui;
}

/**
 * @brief StudyWindow::setStudyList
 * @param studyList
 * this function assumes studyList is NOT empty
 * so it does not take care of the UI when the list is empty!
 * Do NOT show the window if false is returned
 */
bool StudyWindow::setStudyList(sptr<StudyList> studyList)
{
    if (studyList.get() == 0
            || studyList->size() == 0) {
        return false;
    }

    cleanTheWidgets();

    m_studyList = studyList;
    m_currentCard = m_studyList->nextCard();
    if (m_currentCard) {
        m_state = StudyWindow::ShowSpell;
    }
    showCurrentCard();

    return true;
}

void StudyWindow::showCurrentCard()
{
    if (m_currentCard.get()) {
        showCard(*m_currentCard);
    } else {
        allCardsFinished();
    }
}

void StudyWindow::allCardsFinished()
{
    m_state = StudyWindow::NoCard;
    QMessageBox::information(this,
                             StudyWindow::tr(""),
                             StudyWindow::tr("Congratulations! All cards finished!"));
    close();
}

void StudyWindow::cleanTheWidgets()
{
    m_wordView.setWord("");
    m_definitionView.setHtml("");
}

void StudyWindow::showCard(WordCard &card)
{
    updateLabels(card);
    updateButtons();

    auto word = card.getWord();
    if (word.get()) {
        showWord(*word);
    }
}

QString StudyWindow::minuteToString(int minute)
{
    if (minute < 60) {
        return QString::number(minute) + QObject::tr("minute");
    }

    if (minute < 60 * 24) {
        auto hour = minute / 60.0;
        return QString::number(hour, 'f', 1) + QObject::tr("hour");
    }

    if (minute <= 60 * 24 * 30) {
        auto day = minute / (60.0 * 24);
        return QString::number(day, 'f', 1) + QObject::tr("day");
    }

    if (minute <= 60 * 24 * 365) {
        auto month = minute / (60.0 * 24 * 30);
        return QString::number(month, 'f', 1) + QObject::tr("month");
    }

    return QObject::tr("long time later");
}

void StudyWindow::showWord(Word &word)
{
    //setWindowTitle(word.getSpelling());
    m_wordView.setWord(word.getSpelling());
    if (m_state == ShowDefinition) {
        QUrl baseUrl("file:///Users/huafeng/Documents/GitHub/TextFinder/build-Repeat-Desktop_Qt_5_11_1_clang_64bit-Debug/Repeat.app/Contents/MacOS/");
        //QUrl baseUrl("file://" + QCoreApplication::applicationDirPath() + "/");
        m_definitionView.setHtml(word.getDefinition(), baseUrl);
    } else {
        m_definitionView.setHtml("<html><html>");
    }
}

void StudyWindow::nextWord(MemoryItem::ResponseQuality responseQulity)
{
    if (m_studyList.get()) {
        m_studyList->responseToCurrent(m_currentCard, responseQulity);
        m_currentCard = m_studyList->nextCard();
        m_state = ShowSpell;
    }

    showCurrentCard();
}

void StudyWindow::on_pushIncorrect_clicked()
{
    nextWord(MemoryItem::IncorrectButCanRecall);
}

void StudyWindow::on_pushCorrect3_clicked()
{
    nextWord(MemoryItem::CorrectWithDifficulty);
}

void StudyWindow::on_pushCorrect4_clicked()
{
    nextWord(MemoryItem::CorrectAfterHesitation);
}

void StudyWindow::on_pushPerfect_clicked()
{
    nextWord(MemoryItem::Perfect);
}

void StudyWindow::on_pushShow_clicked()
{
    m_state = ShowDefinition;
    showCurrentCard();
}

void StudyWindow::updateLabels(WordCard &card)
{
    //return;
    //int blackout = card.estimatedInterval(MemoryItem::Blackout);
    //int incorrect = card.estimatedInterval(MemoryItem::Incorrect);
    //int ibcr = card.estimatedInterval(MemoryItem::IncorrectButCanRecall);
    int ibcr = card.estimatedInterval(MemoryItem::IncorrectButCanRecall);
    int cwd = card.estimatedInterval(MemoryItem::CorrectWithDifficulty);
    int cah = card.estimatedInterval(MemoryItem::CorrectAfterHesitation);
    int perfect = card.estimatedInterval(MemoryItem::Perfect);

    ui->labelIncorrect->setText(minuteToString(ibcr));
    ui->labelCorrect3->setText(minuteToString(cwd));
    ui->labelCorrect4->setText(minuteToString(cah));
    ui->labelPerfect->setText(minuteToString(perfect));

    if (m_studyList.get()) {
        auto numberOfCards = m_studyList->getList().size();
        ui->labelShow->setText("<html><span style=\"color:blue\">" + QString::number(numberOfCards) + "</span><html>");
    }
}


void StudyWindow::updateButtons()
{
    /* uncomment the following code to make the layout better
     * But layout should really be taken care in ui file
     * Not by code here!
     * */
    /*
    // remove and add the buttons
    if (m_state == ShowDefinition) {
        ui->verticalLayout_6->removeWidget(ui->pushShow);

        ui->verticalLayout->addWidget(ui->pushIncorrect);
        ui->verticalLayout->addWidget(ui->labelIncorrect);
        ui->verticalLayout_2->addWidget(ui->pushCorrect3);
        ui->verticalLayout_2->addWidget(ui->labelCorrect3);
        ui->verticalLayout_4->addWidget(ui->pushCorrect4);
        ui->verticalLayout_4->addWidget(ui->labelCorrect4);
        ui->verticalLayout_5->addWidget(ui->pushPerfect);
        ui->verticalLayout_5->addWidget(ui->labelPerfect);
    } else {
        ui->verticalLayout->removeWidget(ui->labelIncorrect);
        ui->verticalLayout->removeWidget(ui->pushIncorrect);
        ui->verticalLayout_2->removeWidget(ui->labelCorrect3);
        ui->verticalLayout_2->removeWidget(ui->pushCorrect3);
        ui->verticalLayout_4->removeWidget(ui->labelCorrect4);
        ui->verticalLayout_4->removeWidget(ui->pushCorrect4);
        ui->verticalLayout_5->removeWidget(ui->labelPerfect);
        ui->verticalLayout_5->removeWidget(ui->pushPerfect);

        ui->verticalLayout_6->addWidget(ui->pushShow);
    }
    */

    // show and hide the buttons
    ui->pushShow->setVisible(m_state == ShowSpell);
    ui->labelShow->setVisible(m_state == ShowSpell);
    ui->pushIncorrect->setVisible(m_state == ShowDefinition);
    ui->pushCorrect4->setVisible(m_state == ShowDefinition);
    ui->pushCorrect3->setVisible(m_state == ShowDefinition);
    ui->pushPerfect->setVisible(m_state == ShowDefinition);
    ui->labelIncorrect->setVisible(m_state == ShowDefinition);
    ui->labelCorrect3->setVisible(m_state == ShowDefinition);
    ui->labelCorrect4->setVisible(m_state == ShowDefinition);
    ui->labelPerfect->setVisible(m_state == ShowDefinition);
}
