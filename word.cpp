#include "word.h"
#include "golddict/gddebug.hh"

#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QVariant>

// m_baselineTime is my daughter's birth time
const QDateTime MyTime::m_baselineTime = QDateTime::fromString("2016-10-31T10:00:00+08:00", Qt::ISODate);

Word::Word(QString word) :
    m_spelling(word)
{
    m_id = 0;
    m_new = true;
    m_definition = "";
    m_expireTime = defaultExpireTime();
}

void Word::setExpireTime(const QDateTime &expireTime)
{
    m_new = false;
    m_expireTime = expireTime;

    StudyRecord newSR(m_expireTime, QDateTime::currentDateTime());
    m_studyHistory.append(newSR);
    dbsaveStudyRecord(newSR);
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
        databaseError(query, "saving word \"" + m_spelling + "\"");
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
        databaseError(query, "fetching defintion of \"" + m_spelling + "\"");
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
        databaseError(query, "checking word \"" + m_spelling + "\"");
        return 0;
    }
}

// return 0 if not saved
// id of the word if saved
int Word::hasExpireTimeRecord() const
{
    int wordId = isDefintionSaved();
    if (wordId == 0) {
        return 0;
    }

    QSqlQuery query;
    query.prepare("SELECT * FROM words_in_study WHERE word_id=:word_id LIMIT 1");
    query.bindValue(":word_id", wordId);
    if (query.exec()) {
        if (query.first()) {
            return wordId;
        } else {
            return 0;
        }
    } else {
        databaseError(query, "checking expire time of word \"" + m_spelling + "\"");
        return 0;
    }
}

void Word::dbsaveStudyRecord(const StudyRecord &sr)
{
    if (isNew()) {
        return;
    }

    int wordId = getId();
    if (wordId == 0) {
        return;
    }

    int expire = sr.m_expire.toSeconds();
    int studyDate = sr.m_studyDate.toSeconds();
    QSqlQuery query;
    query.prepare("INSERT INTO words_in_study(word_id, expire, study_date) VALUES(:word_id, :expire, :study_date)");
    query.bindValue(":word_id", wordId);
    query.bindValue(":expire", expire);
    query.bindValue(":study_date", studyDate);
    if (query.exec() == false)
    {
        databaseError(query, "adding expire time of \"" + m_spelling + "\"");
    }
}

void Word::dbgetStudyRecords()
{
    int wordId = getId();
    if (wordId == 0) {
        return;
    }
    if (m_studyHistory.size() > 0) {
        // it's already updated!
        return;
    }

    QSqlQuery query;
    query.prepare("SELECT expire, study_date FROM words_in_study WHERE word_id=:word_id");
    query.bindValue(":word_id", wordId);
    if (query.exec()) {
        while (query.next()) {
            int expire = query.value("expire").toInt();
            int studyDate = query.value("study_date").toInt();
            StudyRecord sr(expire, studyDate);
            m_studyHistory.append(sr);
        }
    } else {
        databaseError(query, "fetching expire time of \"" + m_spelling + "\"");
    }

    m_new = (m_studyHistory.size() > 0);
}

// word is updated if the returned value is true
void Word::getFromDatabase()
{
    dbgetDefinition();
    dbgetStudyRecords();
}


int Word::getId()
{
    if (m_id == 0) {
        m_id = Word::getWordId(m_spelling);
    }

    return m_id;
}

// static
void Word::databaseError(QSqlQuery &query, const QString what)
{
    QSqlError error = query.lastError();
    QMessageBox::critical(nullptr, QObject::tr(""),
        "Database error when " + what + ": " + error.text(), QMessageBox::Ok);
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
        databaseError(query, "check existence of \"" + spelling + "\"");
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
void Word::createDatabaseTables()
{
    QSqlQuery query;
    if (query.exec("SELECT * FROM words LIMIT 1") == false)
    {
        // table "words" does not exist
        if(query.exec("CREATE TABLE words (id INTEGER primary key, "
                   "word TEXT, "
                      "definition TEXT)") == false) {
            databaseError(query, "creating table \"words\"");
        }

        if (query.exec("CREATE TABLE words_in_study (id INTEGER primary key, "
                   "word_id INTEGER, "
                   "expire INTEGER, "
                   "study_date INTEGER)") == false) {
            databaseError(query, "creating table \"words_in_study\"");
        }
    } else {
        // table already exist
        QString msg( "Table \"words\" already exists, doing nothing in Word::createDatabaseTables()." );
        gdDebug("%s", msg.toStdString().c_str());
        QMessageBox::information(nullptr, QObject::tr(""),
            msg, QMessageBox::Ok);
    }
}

// static
sptr<Word> Word::getWordFromDatabase(const QString &spelling) {
    if (Word::isInDatabase(spelling) == false) {
        return sptr<Word>();
    }

    sptr<Word> word = new Word(spelling);
    word->getFromDatabase();
    return word;
}

// static
QVector<sptr<Word>> Word::getNewWords(int number)
{
    QVector<sptr<Word>> wordList;

    if (number > 0) {

    }

    return wordList;
}

// static
QDateTime Word::defaultExpireTime()
{
    return QDateTime::currentDateTime().addYears(100);
}
