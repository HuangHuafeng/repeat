#include "studylist.h"
#include "../golddict/gddebug.hh"
#include "wordbook.h"
#include "worddb.h"

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
        addCardAagainToday(m_current);
    }

    // current card finishes for the current review
    m_current = sptr<WordCard>();

    return true;
}

/**
 * @brief StudyList::addCardNoSort
 * add card to the end of m_cards
 */
void StudyList::addCardNoSort(sptr<WordCard> card)
{
    if (card.get() == 0
            || card->getWord().get() == 0)
    {
        return;
    }

    m_cards.append(card);

    /* this can be very slow as getExpireTime() caused database query
     * also the QDateTime comparison is heavy.
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
    */
}

/**
 * @brief StudyList::addCardAagainToday
 * @param card
 * card need to be reviewed again, should be taken care
 * specially in order to avoid being buried by the existing cards
 */
void StudyList::addCardAagainToday(sptr<WordCard> card)
{
    if (card.get() == 0
            || card->getWord().get() == 0)
    {
        return;
    }

    m_cards.append(card);
}

sptr<WordCard> StudyList::nextCard()
{
    if (m_current.get() == 0)
    {
        if (m_cards.isEmpty() == false) {
            m_current = m_cards.first();
        }
    }

    return m_current;
}

/**
 * @brief StudyList::initiCards
 * @param wordList
 * @return
 * this function assumes that wordList is already ordered
 * so NO sorting is done here!
 */
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
        addCardNoSort(card);
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
sptr<StudyList> StudyList::allWords()
{
    sptr<StudyList> sl = new StudyList();
    if (sl.get()) {
        auto wordList = Word::getWords();
        sl->initiCards(wordList);
    }

    return sl;
}

// static
sptr<StudyList> StudyList::allNewWordsInBook(const QString &bookName)
{
    auto book = WordBook::getBook(bookName);
    if (book.get() == 0) {
        // the book does not exist
        return sptr<StudyList>();
    }

    auto wordList = book->getNewWords();
    if (wordList.isEmpty()) {
        // no word
        return sptr<StudyList>();
    }

    sptr<StudyList> sl = new StudyList();
    if (sl.get()) {
        sl->initiCards(wordList);
    }

    return sl;
}

// static
sptr<StudyList> StudyList::allStudiedWordsInBook(const QString &bookName)
{
    auto book = WordBook::getBook(bookName);
    if (book.get() == 0) {
        // the book does not exist
        return sptr<StudyList>();
    }

    auto wordList = book->getStudiedWords();
    if (wordList.isEmpty()) {
        // no word
        return sptr<StudyList>();
    }

    sptr<StudyList> sl = new StudyList();
    if (sl.get()) {
        sl->initiCards(wordList);
    }

    return sl;
}

// static
sptr<StudyList> StudyList::allExpiredWordsInBook(const QString &bookName, const QDateTime expire)
{
    auto book = WordBook::getBook(bookName);
    if (book.get() == 0) {
        // the book does not exist
        return sptr<StudyList>();
    }

    auto wordList = book->getExpiredWords(expire);
    if (wordList.isEmpty()) {
        // no word
        return sptr<StudyList>();
    }

    sptr<StudyList> sl = new StudyList();
    if (sl.get()) {
        sl->initiCards(wordList);
    }

    return sl;
}


// static
sptr<StudyList> StudyList::allWordsInBook(const QString &bookName)
{
    auto book = WordBook::getBook(bookName);
    if (book.get() == 0) {
        // the book does not exist
        return sptr<StudyList>();
    }

    auto wordList = book->getAllWords();
    if (wordList.isEmpty()) {
        // no word
        return sptr<StudyList>();
    }

    sptr<StudyList> sl = new StudyList();
    if (sl.get()) {
        sl->initiCards(wordList);
    }

    return sl;
}

sptr<StudyList> StudyList::allExpiredWords(QDateTime expire)
{
    sptr<StudyList> sl = new StudyList();
    if (sl.get()) {
        QVector<QString> wordList;
        auto expireInt = MyTime(expire).toMinutes();

        QSqlQuery query;
        query.prepare(" SELECT word"
                      " FROM wordcards AS card INNER JOIN words AS word ON card.word_id=word.id"
                      " WHERE"
                              " card.id IN (SELECT MAX(id) FROM wordcards GROUP BY word_id)"
                              " AND expire<:expire"
                      " ORDER BY expire ASC;"
                    );
        query.bindValue(":expire", expireInt);
        if (query.exec()) {
            while (query.next()) {
                QString spelling = query.value("word").toString();
                wordList.append(spelling);
            }
        } else {
            WordDB::databaseError(query, "fetching all expired words");
        }

        sl->initiCards(wordList);
    }

    return sl;
}
