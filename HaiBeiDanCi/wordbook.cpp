#include "wordbook.h"
#include "worddb.h"
#include "../golddict/gddebug.hh"
#include "word.h"

#include <QMessageBox>

WordBook::WordBook(QString name, QString introduction, int id) :
    m_name(name),
    m_introduction(introduction),
    m_id(id)
{

}

int WordBook::getId()
{
    updateFromDatabase();
    return m_id;
}

const QString WordBook::getName()
{
    updateFromDatabase();
    return m_name;
}

const QString WordBook::getIntroduction()
{
    updateFromDatabase();
    return m_introduction;
}


bool WordBook::dbsave()
{
    auto ptrQuery = WordDB::createSqlQuery();auto query = *ptrQuery;
    if (WordBook::isInDatabase(getId())) {
        // update
        query.prepare("UPDATE wordbooks SET name=:name, introduction=:introduction WHERE id=:id");
        query.bindValue(":id", getId());
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

    return true;
}

int WordBook::totalWords()
{
    auto ptrQuery = WordDB::createSqlQuery();auto query = *ptrQuery;
    query.prepare("SELECT count(*) FROM words_in_books WHERE book_id=:book_id");
    query.bindValue(":book_id", getId());
    if (query.exec()) {
        if (query.first()) {
            return query.value(0).toInt();
        } else {
            return 0;
        }
    } else {
        WordDB::databaseError(query, "fetching word books");
    }

    return 0;
}

QVector<QString> WordBook::getAllWords()
{
    return getStudiedWords() + getNewWords();
}

/**
 * @brief WordBook::getStudiedWords
 * @return a list of words ASC
 * A "studied word" is a word that has a record in table wordcards
 */
QVector<QString> WordBook::getStudiedWords()
{
    QVector<QString> wordList;
    auto ptrQuery = WordDB::createSqlQuery();auto query = *ptrQuery;
    query.prepare(" SELECT word"
                  " FROM words AS w INNER JOIN wordcards as c ON w.id=c.word_id"
                  " WHERE"
                            " w.id in (SELECT word_id FROM words_in_books WHERE book_id=:book_id)"
                            " AND c.id IN (SELECT MAX(id) FROM wordcards GROUP BY word_id)"
                  " ORDER BY c.expire ASC");
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
QVector<QString> WordBook::getNewWords()
{
    QVector<QString> wordList;
    auto ptrQuery = WordDB::createSqlQuery();auto query = *ptrQuery;
    query.prepare(" SELECT word"
                  " FROM words AS w"
                  " WHERE"
                            " w.id in (SELECT word_id FROM words_in_books WHERE book_id=:book_id)"
                            " AND w.id NOT IN (SELECT word_id FROM wordcards GROUP BY word_id)"
                  " ORDER BY w.id ASC");
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


QVector<QString> WordBook::getExpiredWords(const QDateTime expire)
{
    auto expireInt = MyTime(expire).toMinutes();
    QVector<QString> wordList;
    auto ptrQuery = WordDB::createSqlQuery();auto query = *ptrQuery;
    query.prepare(" SELECT word"
                  " FROM words AS w INNER JOIN wordcards as c ON w.id=c.word_id"
                  " WHERE"
                            " w.id in (SELECT word_id FROM words_in_books WHERE book_id=:book_id)"
                            " AND c.id IN (SELECT MAX(id) FROM wordcards GROUP BY word_id)"
                            " AND c.expire<:expireint"
                  " ORDER BY c.expire ASC");
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
    auto ptrQuery = WordDB::createSqlQuery();auto query = *ptrQuery;
    query.prepare("SELECT * FROM words_in_books WHERE word_id=:word_id AND book_id=:book_id");
    query.bindValue(":word_id", wordId);
    query.bindValue(":book_id", getId());
    if (query.exec()) {
        if (query.first()) {
            return true;
        } else {
            return false;
        }
    } else {
        WordDB::databaseError(query, "checking if word with id \"" + QString::number(wordId) + "\" is in book \"" + m_name + "\"");
        return false;
    }

    return false;
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

    auto ptrQuery = WordDB::createSqlQuery();auto query = *ptrQuery;
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

void WordBook::updateFromDatabase()
{
    m_id = WordBook::getBookId(m_name);
    if (m_id == 0) {
        // the book does NOT exist in the database
        return;
    }

    if (hasUpdatedFromDatabase() == true) {
        return;
    }

    DatabaseObject::updateFromDatabase();

    if (dbgetNameAndIntro() == false) {
        QMessageBox::critical(nullptr, "", "something wrong when updating book information from database");
    }
}

bool WordBook::dbgetNameAndIntro()
{
    int bookId = getId();
    auto ptrQuery = WordDB::createSqlQuery();auto query = *ptrQuery;
    query.prepare("SELECT name, introduction FROM wordbooks WHERE id=:id");
    query.bindValue(":id", bookId);
    if (query.exec()) {
        if (query.first()) {
            m_name = query.value("name").toString();
            m_introduction = query.value("introduction").toString();
            return true;
        } else {
            return false;
        }
    } else {
        WordDB::databaseError(query, "fetching word books");
        return false;
    }

    return true;
}

// static
sptr<WordBook> WordBook::getBook(const QString &bookName)
{
    sptr<WordBook> book = new WordBook(bookName);
    if (book.get()) {
        if (book->getId() == 0) {
            // the book does not exist in the database
            return sptr<WordBook>();
        }
    }

    return book;
}

// static
QVector<QString> WordBook::getWordsInBook(const QString &bookName)
{
    QVector<QString> wordList;
    auto book = WordBook::getBook(bookName);
    if (book.get()) {
        wordList = book->getAllWords();
    }

    return wordList;
}

// static
QVector<sptr<WordBook>> WordBook::getWordBooks()
{
    QVector<sptr<WordBook>> books;
    auto ptrQuery = WordDB::createSqlQuery();auto query = *ptrQuery;
    query.prepare("SELECT id, name, introduction FROM wordbooks");
    if (query.exec()) {
        while (query.next()) {
            int id = query.value("id").toInt();
            QString name = query.value("name").toString();
            QString introduction = query.value("introduction").toString();
            auto abook = new WordBook(name, introduction, id);
            books.append(abook);
        }
    } else {
        WordDB::databaseError(query, "fetching word books");
    }

    return books;
}

// static
bool WordBook::createDatabaseTables()
{
    auto ptrQuery = WordDB::createSqlQuery();auto query = *ptrQuery;
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
    } else {
        // table already exist, ignore
        QString msg( "Table \"wordbooks\" already exists, doing nothing in WordBook::createDatabaseTables()." );
        gdDebug("%s", msg.toStdString().c_str());
    }

    return true;
}

// static
bool WordBook::isInDatabase(int bookId)
{
    if (bookId == 0) {
        return false;
    }

    auto ptrQuery = WordDB::createSqlQuery();auto query = *ptrQuery;
    query.prepare("SELECT * FROM wordbooks WHERE id=:book_id");
    query.bindValue(":book_id", bookId);
    if (query.exec()) {
        if (query.first()) {
            return true;
        } else {
            return false;
        }
    } else {
        WordDB::databaseError(query, "checking existence of word book with id \"" + QString::number(bookId) + "\"");
        return false;
    }

    return false;
}

// static
bool WordBook::isInDatabase(const QString &name)
{
    auto ptrQuery = WordDB::createSqlQuery();auto query = *ptrQuery;
    query.prepare("SELECT * FROM wordbooks WHERE name=:name");
    query.bindValue(":name", name);
    if (query.exec()) {
        if (query.first()) {
            return true;
        } else {
            return false;
        }
    } else {
        WordDB::databaseError(query, "checking existence of word book \"" + name + "\"");
        return false;
    }

    return false;
}

// static
int WordBook::getBookId(const QString &name)
{
    auto ptrQuery = WordDB::createSqlQuery();auto query = *ptrQuery;
    query.prepare("SELECT id FROM wordbooks WHERE name=:name  COLLATE NOCASE");
    query.bindValue(":name", name);
    if (query.exec()) {
        if (query.first()) {
            return query.value("id").toInt();
        } else {
            return 0;
        }
    } else {
        WordDB::databaseError(query, "checking id of book \"" + name + "\"");
        return 0;
    }

    return 0;
}
