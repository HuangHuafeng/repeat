#ifndef STUDYWINDOW_H
#define STUDYWINDOW_H

#include "wordview.h"
#include "wordcard.h"

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

    void setWordList(const QVector<QString> &wordList);

private slots:
    void on_pushPerfect_clicked();

    void on_pushCorrect_clicked();

    void on_pushIncorrect_clicked();

    void on_pushCorrect3_clicked();

private:
    Ui::StudyWindow *ui;
    WordView m_wordView;
    QWebEngineView m_definitionView;
    WordCard m_wordCard;

    // the list of words to learn
    QVector<WordCard> m_cardList;
    //QVector<WordCard> m_againList;

    void showCurrentWord();
    void nextWord(MemoryItem::ResponseQuality responseQulity);

};

#endif // STUDYWINDOW_H
