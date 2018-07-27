#include "studylist.h"
#include "golddict/gddebug.hh"

StudyList::StudyList()
{

}

bool StudyList::responseToCurrent(sptr<WordCard> current, MemoryItem::ResponseQuality responseQulity)
{
    if (current != m_current) {
        return false;
    }

    if (m_current.get() == 0) {
        return false;
    }

    m_current->update(responseQulity);

    if (responseQulity < MemoryItem::CorrectWithDifficulty) {
        // the card need to be reviewed again today
        addAgainCard(m_current);
    }

    // current card finishes for the current review
    m_current = sptr<WordCard>();

    return true;
}

void StudyList::addAgainCard(sptr<WordCard> againCard)
{
    if (againCard.get() == 0
            || againCard->getWord().get() == 0)
    {
        return;
    }

    auto expire = againCard->getWord()->getExpireTime();
    QLinkedList<sptr<WordCard>>::iterator it = m_againCards.begin();
    while (it != m_againCards.end()) {
        if ((*it).get()) {
            auto word = (*it)->getWord();
            if (word.get()) {
                if (word->getExpireTime() > expire) {
                    break;
                }
            }
        }
    }
    m_againCards.insert(it, againCard);
}

sptr<WordCard> StudyList::nextCard()
{
    if (!m_current.get())
    {
        sptr<WordCard> firstCard, firstAgainCard;
        if (m_cards.isEmpty() == false) {
            firstCard = m_cards.first();
        }
        if (m_againCards.isEmpty() == false) {
            firstAgainCard = m_againCards.first();
        }

        if (firstCard.get()) {
            if (firstAgainCard.get()) {
                // compare
                auto firstWord = firstCard->getWord();
                auto firstAgainWord = firstAgainCard->getWord();
                if (firstWord.get()) {
                    if (firstAgainWord.get()) {
                        // compare expire
                        auto firstWordExpire = firstWord->getExpireTime();
                        auto firstAgainWordExpire = firstAgainWord->getExpireTime();
                        if (firstWordExpire < firstAgainWordExpire) {
                            m_current = firstCard;
                            m_cards.pop_front();
                        } else {
                            m_current = firstAgainCard;
                            m_againCards.pop_front();
                        }
                    } else {
                        m_current = firstCard;
                        m_cards.pop_front();;
                    }
                } else {
                    if (firstAgainWord.get()) {
                        m_current = firstAgainCard;
                        m_againCards.pop_front();
                    } else {
                        // no word, should be impossible
                        assert(false);
                    }
                }
            } else {
                m_current = firstCard;
                m_cards.pop_front();
            }
        } else {
            if (firstAgainCard.get()) {
                m_current = firstAgainCard;
                m_againCards.pop_front();
            } else {
                // no card
            }
        }
    }

    return m_current;
}

bool StudyList::initiCards(const QVector<QString> &wordList)
{
    // if there's already cards, fail
    if (m_cards.isEmpty() == false
            || m_againCards.isEmpty() == false
            || m_current.get()) {
        return false;
    }

    // create the new cards according to the list
    // IMPORTANT: this assumes "wordList" already sorted by "expire"
    for (int i = 0;i < wordList.size();i ++) {
        auto card = WordCard::generateCardForWord(wordList.at(i));
        m_cards.append(card);
    }

    return true;
}

// static
sptr<StudyList> StudyList::generateStudyList()
{
    return sptr<StudyList>();
}

// static
sptr<StudyList> StudyList::generateStudyListForAllWord()
{
    sptr<StudyList> sl = new StudyList();
    if (sl.get()) {
        auto wordList = Word::getWords();
        for (int i = 0;i < wordList.size();i ++) {
            auto word = wordList.at(i);
            gdDebug("%s", word.toStdString().c_str());
        }
        sl->initiCards(wordList);
    }

    return sl;
}
