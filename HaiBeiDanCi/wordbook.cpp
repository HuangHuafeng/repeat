#include "wordbook.h"
#include "worddb.h"
#include "../golddict/gddebug.hh"
#include "word.h"

#include <QMessageBox>

QMap<QString, sptr<WordBook>> WordBook::m_books;
QMutex WordBook::m_booksMutex;

WordBook::WordBook(QString name, QString introduction, int id) :
    m_name(name),
    m_introduction(introduction),
    m_id(id)
{

}

int WordBook::getId()
{
    return m_id;
}

QString WordBook::getName()
{
    return m_name;
}

QString WordBook::getIntroduction()
{
    return m_introduction;
}


bool WordBook::dbsave()
{
    auto ptrQuery = WordDB::createSqlQuery();if (ptrQuery.get() == nullptr) {return false;}auto query = *ptrQuery;
    if (m_id != 0) {
        // update
        query.prepare("UPDATE wordbooks SET name=:name, introduction=:introduction WHERE id=:id");
        query.bindValue(":id", m_id);
    } else {
        // dont save this book if there's already a book with name "m_name"
        if (WordBook::isInDatabase(m_name)) {
            return false;
        }
        // insert
        query.prepare("INSERT INTO wordbooks(name, introduction) VALUES(:name, :introduction)");
    }
    query.bindValue(":name", m_name);
    query.bindValue(":introduction", m_introduction);
    if (query.exec() == false)
    {
        WordDB::databaseError(query, "saving book \"" + m_name + "\"");
        return false;
    }

    if (m_id == 0) {
        // update the id
        m_id = query.lastInsertId().toInt();
    }

    return true;
}

QVector<QString> WordBook::getAllWords(int number)
{
    auto allWords = getOldWords(number) + getNewWords(number);
    if (number > 0 && allWords.size() > number) {
        allWords.resize(number);
    }

    return allWords;
}

/**
 * @brief WordBook::getStudiedWords
 * @return a list of words ASC
 * A "studied word" is a word that has a record in table wordcards
 */
QVector<QString> WordBook::getOldWords(int number)
{
    QVector<QString> wordList;
    auto ptrQuery = WordDB::createSqlQuery();if (ptrQuery.get() == nullptr) {return wordList;}auto query = *ptrQuery;
    QString sql = " SELECT word"
                  " FROM words AS w INNER JOIN wordcards AS c ON w.id=c.word_id"
                  " WHERE"
                            " w.id in (SELECT word_id FROM words_in_books WHERE book_id=:book_id)"
                            " AND c.id IN (SELECT MAX(id) FROM wordcards GROUP BY word_id)"
                  " ORDER BY c.expire ASC";
    if (number > 0) {
        query.prepare(sql + " LIMIT :limit");
        query.bindValue(":limit", number);
    } else {
        query.prepare(sql);
    }
    query.bindValue(":book_id", getId());
    if (query.exec()) {
        while (query.next()) {
            QString spelling = query.value("word").toString();
            wordList.append(spelling);
        }
    } else {
        WordDB::databaseError(query, "fetching words in book \"" + getName() + "\"");
    }

    return wordList;
}

/**
 * @brief WordBook::getNewWords
 * @return a list of words ordered by word id ASC
 * A "new word" is a word that has NO record in table wordcards
 */
QVector<QString> WordBook::getNewWords(int number)
{
    QVector<QString> wordList;
    auto ptrQuery = WordDB::createSqlQuery();if (ptrQuery.get() == nullptr) {return wordList;}auto query = *ptrQuery;
    QString sql = " SELECT word"
                  " FROM words AS w"
                  " WHERE"
                            " w.id in (SELECT word_id FROM words_in_books WHERE book_id=:book_id)"
                            " AND w.id NOT IN (SELECT DISTINCT word_id FROM wordcards)"
                  " ORDER BY w.id ASC";
    if (number > 0) {
        query.prepare(sql + " LIMIT :limit");
        query.bindValue(":limit", number);
    } else {
        query.prepare(sql);
    }
    query.bindValue(":book_id", getId());
    if (query.exec()) {
        while (query.next()) {
            QString spelling = query.value("word").toString();
            wordList.append(spelling);
        }
    } else {
        WordDB::databaseError(query, "fetching words in book \"" + getName() + "\"");
    }

    return wordList;
}


QVector<QString> WordBook::getExpiredWords(const QDateTime expire, int number)
{
    QVector<QString> wordList;
    auto ptrQuery = WordDB::createSqlQuery();if (ptrQuery.get() == nullptr) {return wordList;}auto query = *ptrQuery;
    auto expireInt = MyTime(expire).toMinutes();
    QString sql = " SELECT word"
                  " FROM words AS w INNER JOIN wordcards as c ON w.id=c.word_id"
                  " WHERE"
                            " w.id in (SELECT word_id FROM words_in_books WHERE book_id=:book_id)"
                            " AND c.id IN (SELECT MAX(id) FROM wordcards GROUP BY word_id)"
                            " AND c.expire<:expireint"
                  " ORDER BY c.expire ASC";
    if (number > 0) {
        query.prepare(sql + " LIMIT :limit");
        query.bindValue(":limit", number);
    } else {
        query.prepare(sql);
    }
    query.bindValue(":book_id", getId());
    query.bindValue(":expireint", expireInt);
    if (query.exec()) {
        while (query.next()) {
            QString spelling = query.value("word").toString();
            wordList.append(spelling);
        }
    } else {
        WordDB::databaseError(query, "fetching expired words in book \"" + getName() + "\"");
    }

    return wordList;
}

bool WordBook::hasWord(QString spelling)
{
    int wordId = Word::getWordId(spelling);
    if (wordId == 0) {
        // the word does not exist in the database
        return false;
    }

    return hasWord(wordId);
}

bool WordBook::hasWord(int wordId)
{
    bool retVal = false;
    auto ptrQuery = WordDB::createSqlQuery();if (ptrQuery.get() == nullptr) {return false;}auto query = *ptrQuery;
    query.prepare("SELECT * FROM words_in_books WHERE word_id=:word_id AND book_id=:book_id");
    query.bindValue(":word_id", wordId);
    query.bindValue(":book_id", getId());
    if (query.exec()) {
        if (query.first()) {
            retVal = true;
        } else {
            retVal = false;
        }
    } else {
        WordDB::databaseError(query, "checking if word with id \"" + QString::number(wordId) + "\" is in book \"" + m_name + "\"");
        retVal = false;
    }

    return retVal;
}

bool WordBook::addWord(QString spelling)
{
    int wordId = Word::getWordId(spelling);
    if (wordId == 0) {
        // the word does not exist in the database
        return false;
    }

    if (hasWord(wordId)) {
        // word is already in this book
        return true;
    }

    auto ptrQuery = WordDB::createSqlQuery();if (ptrQuery.get() == nullptr) {return false;}auto query = *ptrQuery;
    query.prepare("INSERT INTO words_in_books(word_id, book_id) VALUES(:word_id, :book_id)");
    query.bindValue(":word_id", wordId);
    query.bindValue(":book_id", getId());

    if (query.exec() == false)
    {
        WordDB::databaseError(query, "adding word \"" + spelling + "\" to book \"" + m_name + "\"");
        return false;
    }

    return true;
}

bool WordBook::dbsaveAddWords(const QVector<QString> &words)
{
    bool retValue = true;
    for (int i = 0;i < words.size();i ++) {
        if (addWord(words.at(i)) == false) {
            retValue = false;
        }
    }

    return retValue;
}

bool WordBook::dbgetNameAndIntro()
{
    bool retVal = false;
    int bookId = getId();
    auto ptrQuery = WordDB::createSqlQuery();if (ptrQuery.get() == nullptr) {return false;}auto query = *ptrQuery;
    query.prepare("SELECT name, introduction FROM wordbooks WHERE id=:id");
    query.bindValue(":id", bookId);
    if (query.exec()) {
        if (query.first()) {
            m_name = query.value("name").toString();
            m_introduction = query.value("introduction").toString();
            retVal = true;
        } else {
            retVal = false;
        }
    } else {
        WordDB::databaseError(query, "fetching word books");
        retVal = false;
    }

    return retVal;
}

// static
sptr<WordBook> WordBook::getBook(QString bookName, bool create)
{
    m_booksMutex.lock();
    auto book = m_books.value(bookName);
    if (book.get() == nullptr && create == true) {
        book = new WordBook(bookName);
        if (book.get()) {
            m_books.insert(bookName, book);
        }
    }
    m_booksMutex.unlock();

    return book;
}

// static
QVector<QString> WordBook::getWordsInBook(QString bookName)
{
    QVector<QString> wordList;
    auto book = WordBook::getBook(bookName);
    if (book.get()) {
        wordList = book->getAllWords();
    }

    return wordList;
}

// static
void WordBook::readAllBooksFromDatabase()
{
    auto ptrQuery = WordDB::createSqlQuery();if (ptrQuery.get() == nullptr) {return;}auto query = *ptrQuery;
    query.prepare("SELECT id, name, introduction FROM wordbooks");
    if (query.exec()) {
        m_booksMutex.lock();
        while (query.next()) {
            int id = query.value("id").toInt();
            QString name = query.value("name").toString();
            QString introduction = query.value("introduction").toString();
            auto abook = new WordBook(name, introduction, id);
            m_books.insert(name, abook);
        }
        m_booksMutex.unlock();
    } else {
        WordDB::databaseError(query, "fetching word books");
    }
}

// static
const QList<QString> WordBook::getAllBooks()
{
    return m_books.keys();
}

// static
bool WordBook::createDatabaseTables()
{
    auto ptrQuery = WordDB::createSqlQuery();if (ptrQuery.get() == nullptr) {return false;}auto query = *ptrQuery;
    if (query.exec("SELECT * FROM wordbooks LIMIT 1") == false)
    {
        // table "wordbooks" does not exist
        if(query.exec("CREATE TABLE wordbooks (id INTEGER primary key, "
                      "name TEXT, "
                      "introduction TEXT)") == false) {
            WordDB::databaseError(query, "creating table \"wordbooks\"");
            return false;
        }

        if (query.exec("CREATE TABLE words_in_books (word_id INTEGER, "
                       "book_id INTEGER)") == false) {
            WordDB::databaseError(query, "creating table \"words_in_books\"");
            return false;
        }
    }

    return true;
}

// static
bool WordBook::isInDatabase(const QString &name)
{
    bool retVal = false;
    auto ptrQuery = WordDB::createSqlQuery();if (ptrQuery.get() == nullptr) {return false;}auto query = *ptrQuery;
    query.prepare("SELECT name FROM wordbooks WHERE name=:name");
    query.bindValue(":name", name);
    if (query.exec()) {
        if (query.first()) {
            retVal = true;
        } else {
            retVal = false;
        }
    } else {
        WordDB::databaseError(query, "checking existence of word book \"" + name + "\"");
        retVal = false;
    }

    return retVal;
}
