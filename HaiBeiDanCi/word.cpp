#include "word.h"
#include "../golddict/gddebug.hh"
#include "worddb.h"

Word::Word(QString word) :
    m_spelling(word)
{
    m_id = 0;
    m_definition = "";
}

void Word::setDefinition(const QString &definition)
{
    m_definition = definition;
    dbsaveDefinition();
}

void Word::dbsaveDefinition()
{
    QSqlQuery query;
    if (Word::isInDatabase(m_spelling)) {
        // update
        query.prepare("UPDATE words SET definition=:definition WHERE word=:word");
    } else {
        // insert
        query.prepare("INSERT INTO words(word, definition) VALUES(:word, :definition)");
    }
    query.bindValue(":word", m_spelling);
    query.bindValue(":definition", m_definition);
    if (query.exec() == false)
    {
        WordDB::databaseError(query, "saving word \"" + m_spelling + "\"");
    }
}

void Word::dbgetDefinition()
{
    QSqlQuery query;
    query.prepare("SELECT id, definition FROM words WHERE word=:word COLLATE NOCASE");
    query.bindValue(":word", m_spelling);
    if (query.exec()) {
        if (query.first()) {
            // word exist
            m_id = query.value("id").toInt();
            m_definition = query.value("definition").toString();
        }
    } else {
        WordDB::databaseError(query, "fetching defintion of \"" + m_spelling + "\"");
    }
}

bool Word::isSaved()
{
    return Word::isInDatabase(m_spelling);
}

// return 0 if not saved
// id of the word if saved
int Word::isDefintionSaved() const
{
    QSqlQuery query;
    query.prepare("SELECT id FROM words WHERE word=:word");
    query.bindValue(":word", m_spelling);
    if (query.exec()) {
        if (query.first()) {
            return query.value("id").toInt();
        } else {
            return 0;
        }
    } else {
        WordDB::databaseError(query, "checking word \"" + m_spelling + "\"");
        return 0;
    }
}

// word is updated if the returned value is true
void Word::getFromDatabase()
{
    dbgetDefinition();
}

int Word::getId()
{
    if (m_id == 0) {
        m_id = Word::getWordId(m_spelling);
    }

    return m_id;
}

// static
int Word::getWordId(const QString &spelling)
{
    QSqlQuery query;
    query.prepare("SELECT id FROM words WHERE word=:word  COLLATE NOCASE");
    query.bindValue(":word", spelling);
    if (query.exec()) {
        if (query.first()) {
            return query.value("id").toInt();
        } else {
            return 0;
        }
    } else {
        WordDB::databaseError(query, "checking existence of \"" + spelling + "\"");
        return 0;
    }

    return 0;
}

// static
bool Word::isInDatabase(const QString &spelling)
{
    return 0 != Word::getWordId(spelling);
}

// static
bool Word::createDatabaseTables()
{
    QSqlQuery query;
    if (query.exec("SELECT * FROM words LIMIT 1") == false)
    {
        // table "words" does not exist
        if(query.exec("CREATE TABLE words (id INTEGER primary key, "
                      "word TEXT, "
                      "definition TEXT)") == false) {
            WordDB::databaseError(query, "creating table \"words\"");
            return false;
        }

        /*
        if (query.exec("CREATE TABLE words_in_study (id INTEGER primary key, "
                   "word_id INTEGER, "
                   "expire INTEGER, "
                   "study_date INTEGER)") == false) {
            WordDB::databaseError(query, "creating table \"words_in_study\"");
            return false;
        }
        */
    } else {
        // table already exist, ignore
        QString msg( "Table \"words\" already exists, doing nothing in Word::createDatabaseTables()." );
        gdDebug("%s", msg.toStdString().c_str());
    }

    return true;
}

// static
sptr<Word> Word::getWordFromDatabase(const QString &spelling)
{
    if (Word::isInDatabase(spelling) == false) {
        return sptr<Word>();
    }

    sptr<Word> word = new Word(spelling);
    word->getFromDatabase();
    return word;
}

/**
  get a list of words that is new (a new word has definition, but has no study record)
  only spelling is added to the list, Word object should be created by the caller to
  keep flexibility
  */
// static
QVector<QString> Word::getNewWords(int number)
{
    QVector<QString> wordList;

    if (number > 0) {
        QSqlQuery query;
        query.prepare("SELECT word FROM words WHERE id NOT IN (SELECT word_id FROM words_in_study) LIMIT :limit");
        query.bindValue(":limit", number);
        if (query.exec()) {
            while (query.next()) {
                QString spelling = query.value("word").toString();
                wordList.append(spelling);
            }
        } else {
            WordDB::databaseError(query, "fetching new words");
        }
    }

    return wordList;
}

// static
QVector<QString> Word::getWords(int number)
{
    QVector<QString> wordList;
    QSqlQuery query;

    if (number > 0) {
        query.prepare("SELECT word FROM words LIMIT :limit");
        query.bindValue(":limit", number);
    } else {
        query.prepare("SELECT word FROM words");
    }
    if (query.exec()) {
        while (query.next()) {
            QString spelling = query.value("word").toString();
            wordList.append(spelling);
        }
    } else {
        WordDB::databaseError(query, "fetching new words");
    }


    return wordList;
}

/**
  get a list of words that is new (a new word has definition, but has no study record)
  only spelling is added to the list, Word object should be created by the caller to
  keep flexibility
  */
// static
QVector<QString> Word::getExpiredWords(int number)
{
    QVector<QString> wordList;

    if (number > 0) {
        QSqlQuery query;
        query.prepare("SELECT word FROM words WHERE id NOT IN (SELECT word_id FROM words_in_study) LIMIT :limit");
        query.bindValue(":limit", number);
        if (query.exec()) {
            while (query.next()) {
                QString spelling = query.value("word").toString();
                wordList.append(spelling);
            }
        } else {
            WordDB::databaseError(query, "fetching new words");
        }
    }

    return wordList;
}
