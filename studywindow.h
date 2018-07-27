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

    void setStudyList(sptr<StudyList> studyList);

private slots:
    void on_pushPerfect_clicked();

    void on_pushCorrect_clicked();

    void on_pushIncorrect_clicked();

    void on_pushCorrect3_clicked();

private:
    Ui::StudyWindow *ui;
    WordView m_wordView;
    QWebEngineView m_definitionView;

    sptr<StudyList> m_studyList;
    sptr<WordCard> m_currentCard;

    void showCurrentWord();
    void showCard(const WordCard &card);
    void showWord(const Word &word);
    QString minuteToString(int minute);
    void nextWord(MemoryItem::ResponseQuality responseQulity);
    void allCardsFinished();

};

#endif // STUDYWINDOW_H
