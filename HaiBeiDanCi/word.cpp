#include "word.h"
#include "worddb.h"

#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QElapsedTimer>
#include <QtDebug>

QMap<QString, sptr<Word>> Word::m_allWords;
QMutex Word::m_allWordsMutex;

Word::Word(QString word, QString definition, int id) : m_spelling(word),
                                                       m_definition(definition),
                                                       m_id(id)
{
}

bool Word::dbsave()
{
    auto ptrQuery = WordDB::createSqlQuery();
    if (ptrQuery.get() == nullptr)
    {
        return false;
    }
    auto query = *ptrQuery;
    if (Word::isInDatabase(m_spelling) == true)
    {
        // the word is already in database, update
        query.prepare("UPDATE words SET definition=:definition WHERE word=:word");
    }
    else
    {
        // insert
        if (m_id == 0)
        {
            // no id yet, let the database decide the id
            query.prepare("INSERT INTO words(word, definition) VALUES(:word, :definition)");
        }
        else
        {
            // already have an id, keep it as the word may downloaded from the server
            query.prepare("INSERT INTO words(id, word, definition) VALUES(:id, :word, :definition)");
            query.bindValue(":id", m_id);
        }
    }
    query.bindValue(":word", m_spelling);
    query.bindValue(":definition", m_definition);
    if (query.exec() == false)
    {
        WordDB::databaseError(query, "saving word \"" + m_spelling + "\"");
        return false;
    }

    if (m_id == 0)
    {
        // update the id
        m_id = query.lastInsertId().toInt();
    }

    return true;
}

/*
bool Word::dbsaveDefinition()
{
    auto ptrQuery = WordDB::createSqlQuery();
    if (ptrQuery.get() == nullptr)
    {
        return false;
    }
    auto query = *ptrQuery;
    if (m_id != 0 || Word::isInDatabase(m_spelling))
    {
        // update
        query.prepare("UPDATE words SET definition=:definition WHERE word=:word");
    }
    else
    {
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

    if (m_id == 0)
    {
        // update the id
        m_id = query.lastInsertId().toInt();
    }

    return true;
}
*/

void Word::setId(int id)
{
    m_id = id;
}

void Word::setSpelling(QString spelling)
{
    m_spelling = spelling;
}

void Word::setDefinition(const QString &definition)
{
    m_definition = definition;

    // we should not call dbsaveDefinition() here as we serialize Word
    //dbsaveDefinition();
}

int Word::getId() const
{
    return m_id;
}

const QString &Word::getDefinition() const
{
    return m_definition;
}

QString Word::getDefinitionDIV() const
{
    QString div = getDefinition();

    const QRegularExpression body("<body>(?<wanted_div>(.|\\n)*)</body>");
    QRegularExpressionMatch match = body.match(div, 0);
    if (match.hasMatch())
    {
        div = match.captured("wanted_div");
    }

    return div;
}

QStringList Word::exampleAudioFiles() const
{
    QStringList mf;

    const QRegularExpression mfre("media/exa/[^\"<>']*\\.mp3");
    QRegularExpressionMatchIterator it = mfre.globalMatch(m_definition);
    while (it.hasNext())
    {
        QRegularExpressionMatch match = it.next();
        QString onemf = match.captured(0);
        mf.append(onemf);
    }

    return mf;
}

QStringList Word::pronounceFiles() const
{
    QStringList mf;

    const QRegularExpression mfre("media/hwd/[^\"<>']*\\.mp3");
    QRegularExpressionMatchIterator it = mfre.globalMatch(m_definition);
    while (it.hasNext())
    {
        QRegularExpressionMatch match = it.next();
        QString onemf = match.captured(0);
        mf.append(onemf);
    }

    return mf;
}

QStringList Word::otherFiles() const
{
    QStringList mf;

    const QRegularExpression mfre("media/[^\"<>']*\\.(js|css|png|jpg)");
    QRegularExpressionMatchIterator it = mfre.globalMatch(m_definition);
    while (it.hasNext())
    {
        QRegularExpressionMatch match = it.next();
        QString onemf = match.captured(0);
        mf.append(onemf);
    }

    return mf;
}

// static
QList<QString> Word::getAllWords()
{
    return m_allWords.keys();
}

// static
int Word::getWordId(const QString &spelling)
{
    auto word = Word::getWord(spelling);
    if (word.get() != nullptr)
    {
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
void Word::storeWordFromServer(sptr<Word> word)
{
    if (word.get() == nullptr)
    {
        return;
    }

    if (Word::getWord(word->getSpelling()).get() != nullptr)
    {
        return;
    }

    if (Word::isInDatabase(word->getSpelling()) == true)
    {
        // WON'T reach here as "Word::getWord(word->getSpelling()).get() != nullptr" already resturns
        return;
    }

    word->dbsave();
    m_allWordsMutex.lock();
    m_allWords.insert(word->getSpelling(), word);
    m_allWordsMutex.unlock();
}

/**
 * @brief Word::storeMultipleWordFromServer
 * @param mapWords
 * should use batch execute to store these words
 */
void Word::storeMultipleWordFromServer(const QMap<QString, sptr<Word>> mapWords)
{
    auto it = mapWords.begin();
    while (it != mapWords.end())
    {
        auto word = (*it);
        if (word.get() != nullptr)
        {
            word->dbsave();
            m_allWordsMutex.lock();
            m_allWords.insert(word->getSpelling(), word);
            m_allWordsMutex.unlock();
        }
        it ++;
    }
}

void Word::v2StoreMultipleWordFromServer(const QMap<QString, sptr<Word>> mapWords)
{
    auto it = mapWords.begin();
    QVariantList ids;
    QVariantList spellings;
    QVariantList definitions;
    m_allWordsMutex.lock();
    while (it != mapWords.end())
    {
        auto word = (*it);
        if (word.get() != nullptr)
        {
            m_allWords.insert(word->getSpelling(), word);
            ids << word->getId();
            spellings << word->getSpelling();
            definitions << word->getDefinition();
        }
        it ++;
    }
    m_allWordsMutex.unlock();

    auto database = WordDB::connectedDatabase();
    auto ptrQuery = WordDB::createSqlQuery();
    if (ptrQuery.get() == nullptr || database.get() == nullptr)
    {
        return;
    }
    database->transaction();
    auto query = *ptrQuery;
    query.prepare("INSERT INTO words VALUES(?, ?, ?)");
    query.addBindValue(ids);
    query.addBindValue(spellings);
    query.addBindValue(definitions);
    if (query.execBatch() == false)
    {
        WordDB::databaseError(query, "storing words from server");
        return;
    }
    database->commit();
}

void Word::batchStoreMultipleWordFromServer(const QMap<QString, sptr<Word>> mapWords)
{
    auto it = mapWords.begin();
    QVariantList ids;
    QVariantList spellings;
    QVariantList definitions;
    m_allWordsMutex.lock();
    while (it != mapWords.end())
    {
        auto word = (*it);
        if (word.get() != nullptr)
        {
            m_allWords.insert(word->getSpelling(), word);
            ids << word->getId();
            spellings << word->getSpelling();
            definitions << word->getDefinition();
        }
        it ++;
    }
    m_allWordsMutex.unlock();

    auto ptrQuery = WordDB::createSqlQuery();
    if (ptrQuery.get() == nullptr)
    {
        return;
    }
    auto query = *ptrQuery;
    query.prepare("INSERT INTO words VALUES(?, ?, ?)");
    query.addBindValue(ids);
    query.addBindValue(spellings);
    query.addBindValue(definitions);
    if (query.execBatch() == false)
    {
        WordDB::databaseError(query, "storing words from server");
        return;
    }
}

// static
bool Word::createDatabaseTables()
{
    auto ptrQuery = WordDB::createSqlQuery();
    if (ptrQuery.get() == nullptr)
    {
        return false;
    }
    auto query = *ptrQuery;
    if (query.exec("SELECT * FROM words LIMIT 1") == false)
    {
        // table "words" does not exist
        if (query.exec("CREATE TABLE words (id INTEGER primary key, "
                       "word TEXT, "
                       "definition TEXT)") == false)
        {
            WordDB::databaseError(query, "creating table \"words\"");
            return false;
        }
    }

    return true;
}

void Word::readAllWordsFromDatabase()
{
    m_allWordsMutex.lock();
    bool alreadyRead = m_allWords.isEmpty() == false;
    m_allWordsMutex.unlock();
    if (alreadyRead == true)
    {
        return;
    }

    auto ptrQuery = WordDB::createSqlQuery();
    if (ptrQuery.get() == nullptr)
    {
        return;
    }
    auto query = *ptrQuery;
    query.prepare(" SELECT *"
                  " FROM words"
                  " ORDER BY id ASC");
    if (query.exec())
    {
        m_allWordsMutex.lock();
        while (query.next())
        {
            int id = query.value("id").toInt();
            QString spelling = query.value("word").toString();
            QString definition = query.value("definition").toString();
            sptr<Word> word = new Word(spelling, definition, id);
            m_allWords.insert(spelling, word);
        }
        m_allWordsMutex.unlock();
    }
    else
    {
        WordDB::databaseError(query, "fetching all cards from database");
    }
}

// static
sptr<Word> Word::getWord(const QString &spelling, bool create)
{
    m_allWordsMutex.lock();
    sptr<Word> word = m_allWords.value(spelling);
    if (word.get() == nullptr && create == true)
    {
        word = new Word(spelling);
        if (word.get())
        {
            m_allWords.insert(spelling, word);
        }
    }
    m_allWordsMutex.unlock();

    return word;
}

// static
const Word * Word::getWordToRead(QString spelling)
{
    m_allWordsMutex.lock();
    Word *ptrWord = m_allWords.value(spelling).get();
    m_allWordsMutex.unlock();

    return ptrWord;
}

QDataStream &operator<<(QDataStream &ds, const Word &word)
{
    ds << word.getId() << word.getSpelling() << word.getDefinition();
    return ds;
}

QDataStream &operator>>(QDataStream &ds, Word &word)
{
    int id;
    QString spelling;
    QString definition;
    ds >> id >> spelling >> definition;
    word.setId(id);
    word.setSpelling(spelling);
    word.setDefinition(definition);
    return ds;
}
