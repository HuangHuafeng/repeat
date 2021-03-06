#ifndef STUDYWINDOW_H
#define STUDYWINDOW_H

#include "wordview.h"
#include "wordcard.h"
#include "studylist.h"

#include <QDialog>
#include <QVector>

namespace Ui
{
class StudyWindow;
}

class StudyWindow : public QDialog
{
    Q_OBJECT

  public:
    explicit StudyWindow(QWidget *parent = nullptr);
    ~StudyWindow() override;

    bool setStudyList(sptr<StudyList> studyList);
    void reloadView();

    virtual void closeEvent(QCloseEvent *event) override;

  signals:
    void wordStudied(QString spelling);

  private slots:
    void on_pushShow_clicked();

    void on_pushIncorrect_clicked();

    void on_pushCorrect3_clicked();

    void on_pushCorrect4_clicked();

    void on_pushPerfect_clicked();

  private:
    typedef enum
    {
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
    void showWord(const Word *word);
    QString minuteToString(int minute);
    void nextWord(MemoryItem::ResponseQuality responseQulity = MemoryItem::Perfect);
    void allCardsFinished();
    void cleanTheWidgets();
    void updateButtons();
    void updateLabels(WordCard &card);
    void setMyTitle();

    void saveSettings();
    void loadSetting();
};

#endif // STUDYWINDOW_H
