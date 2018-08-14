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

    if (m_current.get() == nullptr) {
        return false;
    }

    // remove the card from the list
    m_words.pop_front();
    // update the card's data
    m_current->update(responseQulity);

    if (responseQulity < MemoryItem::CorrectWithDifficulty) {
        // the card need to be reviewed again today
        auto word = m_current->getWord();
        if (word.get()) {
            learnWordAgain(word->getSpelling());
        }
    }

    // current card finishes for the current review
    m_current = sptr<WordCard>();

    return true;
}

void StudyList::learnWordAgain(const QString spelling)
{
    m_words.append(spelling);
}

sptr<WordCard> StudyList::nextCard()
{
    if (m_current.get() == nullptr)
    {
        if (m_words.isEmpty() == false) {
            m_current = WordCard::getCard(m_words.first(), true);
        }
    }

    return m_current;
}

void StudyList::setWordList(const QVector<QString> &wordList)
{
    m_words = wordList;
}

// static
sptr<StudyList> StudyList::allWords()
{
    sptr<StudyList> sl = new StudyList();
    if (sl.get()) {
        auto wordList = WordCard::getAllWords();
        sl->setWordList(wordList);
    }

    return sl;
}

// static
sptr<StudyList> StudyList::allNewWords()
{
    sptr<StudyList> sl = new StudyList();
    if (sl.get()) {
        auto wordList = WordCard::getNewWords();
        sl->setWordList(wordList);
    }

    return sl;
}

// static
sptr<StudyList> StudyList::allOldWords()
{
    sptr<StudyList> sl = new StudyList();
    if (sl.get()) {
        auto wordList = WordCard::getOldWords();
        sl->setWordList(wordList);
    }

    return sl;
}

// static
sptr<StudyList> StudyList::allExpiredWords(QDateTime expire)
{
    sptr<StudyList> sl = new StudyList();
    if (sl.get()) {
        auto wordList = WordCard::getExpiredWords(expire);
        sl->setWordList(wordList);
    }

    return sl;
}

// static
sptr<StudyList> StudyList::allNewWordsInBook(const QString &bookName)
{
    auto book = WordBook::getBook(bookName);
    if (book.get() == nullptr) {
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
        sl->setWordList(wordList);
    }

    return sl;
}

// static
sptr<StudyList> StudyList::allOldWordsInBook(const QString &bookName)
{
    auto book = WordBook::getBook(bookName);
    if (book.get() == nullptr) {
        // the book does not exist
        return sptr<StudyList>();
    }

    auto wordList = book->getOldWords();
    if (wordList.isEmpty()) {
        // no word
        return sptr<StudyList>();
    }

    sptr<StudyList> sl = new StudyList();
    if (sl.get()) {
        sl->setWordList(wordList);
    }

    return sl;
}

// static
sptr<StudyList> StudyList::allExpiredWordsInBook(const QString &bookName, const QDateTime expire)
{
    auto book = WordBook::getBook(bookName);
    if (book.get() == nullptr) {
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
        sl->setWordList(wordList);
    }

    return sl;
}


// static
sptr<StudyList> StudyList::allWordsInBook(const QString &bookName)
{
    auto book = WordBook::getBook(bookName);
    if (book.get() == nullptr) {
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
        sl->setWordList(wordList);
    }

    return sl;
}
