#include "studywindow.h"
#include "ui_studywindow.h"
#include "wordcard.h"

#include <QLabel>

StudyWindow::StudyWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::StudyWindow),
    m_wordView(parent),
    m_definitionView(parent),
    m_wordCard()
{
    ui->setupUi(this);

    ui->vlDefinition->addWidget(&m_wordView);
    ui->vlDefinition->addWidget(&m_definitionView);
}

StudyWindow::~StudyWindow()
{
    delete ui;
}

void StudyWindow::setWordList(const QVector<QString> &wordList)
{
    // remove the old cards
    if (m_cardList.isEmpty() == false) {
        m_cardList.clear();
    }

    // create the new cards according to the list
    for (int i = 0;i < wordList.size();i ++) {
        WordCard card = WordCard::generateCardForWord(wordList.at(i));
        m_cardList.append(card);
    }

    // update the window
    showCurrentWord();
}

void StudyWindow::on_pushPerfect_clicked()
{
    nextWord(MemoryItem::Perfect);
}

void StudyWindow::showCurrentWord()
{
    if (m_cardList.isEmpty()) {
        const QString html = "<html><body><h1>Congratulations! You've finished all words today!</h1></body></html>";
        m_wordView.setWord("");
        m_definitionView.setHtml(html);
        return;
    }

    WordCard current = m_cardList.first();
    sptr<Word> word = current.getWord();
    if (word.get()) {
        QString html = word->getDefinition();
        QUrl baseUrl("file://" + QCoreApplication::applicationDirPath() + "/");
        m_wordView.setWord(word->getSpelling());
        m_definitionView.setHtml(word->getDefinition(), baseUrl);
    }
}

void StudyWindow::nextWord(MemoryItem::ResponseQuality responseQulity)
{
    if (m_cardList.isEmpty()) {
        return;
    }

    WordCard current = m_cardList.first();
    current.update(responseQulity);

    if (responseQulity < MemoryItem::CorrectAfterHesitation) {
        m_cardList.append(current);
        m_cardList.pop_front();
    } else {
        // the word is OK today, remove it
        m_cardList.pop_front();
    }

    showCurrentWord();
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
