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
            m_current = WordCard::generateCardForWord(m_words.first());
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
        auto wordList = Word::getWords();
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
sptr<StudyList> StudyList::allStudiedWordsInBook(const QString &bookName)
{
    auto book = WordBook::getBook(bookName);
    if (book.get() == nullptr) {
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

        sl->setWordList(wordList);
    }

    return sl;
}
