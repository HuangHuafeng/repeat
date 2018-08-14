#include "word.h"
#include "../golddict/gddebug.hh"
#include "worddb.h"

#include <QRegularExpression>
#include <QRegularExpressionMatch>

QMap<QString, sptr<Word>> Word::m_words;
QMutex Word::m_wordsMutex;

Word::Word(QString word, QString definition, int id) :
    m_spelling(word),
    m_definition(definition),
    m_id(id)
{
}

bool Word::dbsaveDefinition()
{
    auto ptrQuery = WordDB::createSqlQuery();if (ptrQuery.get() == nullptr) {return false;}auto query = *ptrQuery;
    if (m_id != 0 || Word::isInDatabase(m_spelling)) {
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
        return false;
    }

    if (m_id == 0) {
        // update the id
        m_id = query.lastInsertId().toInt();
    }

    return true;
}

const QString & Word::getDefinition()
{
    return m_definition;
}

QString Word::getDefinitionDIV()
{
    QString div = getDefinition();

    const QRegularExpression body("<body>(?<wanted_div>(.|\\n)*)</body>");
    QRegularExpressionMatch match = body.match(div, 0);
    if (match.hasMatch()) {
        div = match.captured("wanted_div");
    }

    return div;
}

void Word::setDefinition(const QString &definition)
{
    m_definition = definition;
    dbsaveDefinition();
}

int Word::getId()
{
    return m_id;
}

// static
QList<QString> Word::getAllWords()
{
    return m_words.keys();
}

// static
int Word::getWordId(const QString &spelling)
{
    auto word = Word::getWord(spelling);
    if (word.get() != nullptr) {
        return word->m_id;
    }

    return 0;

    /*
    int id = 0;
    auto ptrQuery = WordDB::createSqlQuery();if (ptrQuery.get() == nullptr) {return false;}auto query = *ptrQuery;
    query.prepare("SELECT id FROM words WHERE word=:word COLLATE NOCASE");
    query.bindValue(":word", spelling);
    if (query.exec()) {
        if (query.first()) {
            id = query.value("id").toInt();
        }
    } else {
        WordDB::databaseError(query, "checking existence of \"" + spelling + "\"");
    }

    return id;
    */
}

// static
bool Word::isInDatabase(const QString &spelling)
{
    return Word::getWordId(spelling) != 0;
}

// static
bool Word::createDatabaseTables()
{
    auto ptrQuery = WordDB::createSqlQuery();if (ptrQuery.get() == nullptr) {return false;}auto query = *ptrQuery;
    if (query.exec("SELECT * FROM words LIMIT 1") == false)
    {
        // table "words" does not exist
        if(query.exec("CREATE TABLE words (id INTEGER primary key, "
                      "word TEXT, "
                      "definition TEXT)") == false) {
            WordDB::databaseError(query, "creating table \"words\"");
            return false;
        }
    }

    return true;
}

void Word::readAllWordsFromDatabase()
{
    auto ptrQuery = WordDB::createSqlQuery();
    if (ptrQuery.get() == nullptr) {
        return;
    }
    auto query = *ptrQuery;
    query.prepare(" SELECT *"
                  " FROM words"
                  " ORDER BY id ASC");
    if (query.exec()) {
        m_wordsMutex.lock();
        while (query.next()) {
            int id = query.value("id").toInt();
            QString spelling = query.value("word").toString();
            QString definition = query.value("definition").toString();
            sptr<Word> word = new Word(spelling, definition, id);
            m_words.insert(spelling, word);
        }
        m_wordsMutex.unlock();
    } else {
        WordDB::databaseError(query, "fetching all cards from database");
    }
}

// static
sptr<Word> Word::getWord(const QString &spelling, bool create)
{
    m_wordsMutex.lock();
    sptr<Word> word = m_words.value(spelling);
    if (word.get() == nullptr && create == true) {
        word = new Word(spelling);
        if (word.get()) {
            m_words.insert(spelling, word);
        }
    }
    m_wordsMutex.unlock();

    return word;
}
