#ifndef STUDYWINDOW_H
#define STUDYWINDOW_H

#include "wordview.h"
#include "wordcard.h"
#include "studylist.h"

#include <QDialog>
#include <QVector>

namespace Ui {
class StudyWindow;
}

class StudyWindow : public QDialog
{
    Q_OBJECT

public:
    explicit StudyWindow(QWidget *parent = 0);
    ~StudyWindow();

    bool setStudyList(sptr<StudyList> studyList);
    void reloadView();

private slots:
    void on_pushShow_clicked();

    void on_pushIncorrect_clicked();

    void on_pushCorrect3_clicked();

    void on_pushCorrect4_clicked();

    void on_pushPerfect_clicked();

private:
    typedef enum {
        ShowSpell = 1,
        ShowDefinition = 2,
        NoCard = 3
    } StudyState;

    Ui::StudyWindow *ui;
    WordView m_wordView;
    StudyState m_state;

    sptr<StudyList> m_studyList;
    sptr<WordCard> m_currentCard;

    void showCurrentCard();
    void showCard(WordCard &card);
    void showWord(sptr<Word> word);
    QString minuteToString(int minute);
    void nextWord(MemoryItem::ResponseQuality responseQulity);
    void allCardsFinished();
    void cleanTheWidgets();
    void updateButtons();
    void updateLabels(WordCard &card);

};

#endif // STUDYWINDOW_H
