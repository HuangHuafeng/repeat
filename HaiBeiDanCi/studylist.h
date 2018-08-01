#ifndef STUDYLIST_H
#define STUDYLIST_H

#include "wordcard.h"
#include "../golddict/sptr.hh"

#include <QVector>
#include <QLinkedList>
#include <QDateTime>

/**
 * @brief The StudyList class
 * A list of words that is going to be learned
 */
class StudyList
{
public:
    StudyList();
    ~StudyList();

    static sptr<StudyList> generateStudyList();
    static sptr<StudyList> generateStudyListForAllWords();
    static sptr<StudyList> generateStudyListForAllWordsInBook(const QString &bookName);
    static sptr<StudyList> generateStudyListForAllExpiredWords(QDateTime expire = QDateTime::currentDateTime());

    sptr<WordCard> nextCard();
    bool responseToCurrent(sptr<WordCard> current, MemoryItem::ResponseQuality responseQulity);

    const QLinkedList<sptr<WordCard>> & getList() const;

private:
    QLinkedList<sptr<WordCard>> m_cards;
    sptr<WordCard> m_current;

private:
    bool initiCards(const QVector<QString> &wordList);
    void addCardNoSort(sptr<WordCard> card);
    void addCardAagainToday(sptr<WordCard> card);
};

#endif // STUDYLIST_H
