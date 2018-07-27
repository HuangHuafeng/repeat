#ifndef STUDYLIST_H
#define STUDYLIST_H

/**
 * @brief The StudyList class
 * A list of words that is going to be learned
 */


#include "wordcard.h"
#include "golddict/sptr.hh"

#include <QVector>
#include <QLinkedList>

class StudyList
{
public:
    StudyList();

    static sptr<StudyList> generateStudyList();
    static sptr<StudyList> generateStudyListForAllWord();
    sptr<WordCard> nextCard();
    bool responseToCurrent(sptr<WordCard> current, MemoryItem::ResponseQuality responseQulity);

private:
    QVector<sptr<WordCard>> m_cards;
    QLinkedList<sptr<WordCard>> m_againCards;
    sptr<WordCard> m_current;

private:
    bool initiCards(const QVector<QString> &wordList);
    void addAgainCard(sptr<WordCard> againCard);
};

#endif // STUDYLIST_H
