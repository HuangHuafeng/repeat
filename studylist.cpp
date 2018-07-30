#include "studylist.h"
#include "golddict/gddebug.hh"

StudyList::StudyList()
{

}

StudyList::~StudyList()
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

    // remove the card from the list
    m_cards.pop_front();
    // update the card's data
    m_current->update(responseQulity);

    if (responseQulity < MemoryItem::CorrectWithDifficulty) {
        // the card need to be reviewed again today
        addCard(m_current);
    }

    // current card finishes for the current review
    m_current = sptr<WordCard>();

    return true;
}

/**
 * @brief StudyList::addAgainCard
 * @param againCard: the card that should be reviewed again today
 * We search the list from the end as the card's expire would
 * porbably be later than most of the cards in the list
 */
void StudyList::addCard(sptr<WordCard> card)
{
    if (card.get() == 0
            || card->getWord().get() == 0)
    {
        return;
    }

    auto expire = card->getExpireTime();
    QLinkedList<sptr<WordCard>>::iterator it = m_cards.end();
    while (it != m_cards.begin()) {
        it --;
        if ((*it).get()) {
            if ((*it)->getExpireTime() < expire) {
                it ++;
                break;
            }
        }
    }
    m_cards.insert(it, card);
}

sptr<WordCard> StudyList::nextCard()
{
    if (m_current.get() == 0)
    {
        if (m_cards.isEmpty() == false) {
            m_current = m_cards.first();
            //m_cards.pop_front();
        }
    }

    return m_current;
}

bool StudyList::initiCards(const QVector<QString> &wordList)
{
    // if there's already cards, fail
    if (m_cards.isEmpty() == false
            || m_current.get()) {
        return false;
    }

    // create the new cards according to the list
    for (int i = 0;i < wordList.size();i ++) {
        auto card = WordCard::generateCardForWord(wordList.at(i));
        //m_cards.append(card);
        addCard(card);
    }

    return true;
}

const QLinkedList<sptr<WordCard>> & StudyList::getList() const
{
    return m_cards;
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
