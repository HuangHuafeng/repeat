#include "wordbook.h"
#include "worddb.h"
#include "../golddict/gddebug.hh"
#include "word.h"

WordBook::WordBook(QString name, QString introduction, int id) :
    m_name(name),
    m_introduction(introduction),
    m_id(id)
{

}

int WordBook::getId()
{
    if (m_id == 0) {
        m_id = WordBook::getBookId(m_name);
    }

    return m_id;
}

bool WordBook::dbsave()
{
    QSqlQuery query;
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
    QSqlQuery query;
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

    QSqlQuery query;
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

// static
QVector<sptr<WordBook>> WordBook::getWordBooks()
{
    QVector<sptr<WordBook>> books;
    QSqlQuery query;
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
    QSqlQuery query;
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

    QSqlQuery query;
    query.prepare("SELECT * FROM wordbooks WHERE book_id=:book_id");
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
    QSqlQuery query;
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
    QSqlQuery query;
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
