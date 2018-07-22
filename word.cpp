#include "word.h"
#include "golddict/gddebug.hh"

#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QVariant>

// m_baselineTime is my daughter's birth time
const QDateTime Word::m_baselineTime = QDateTime::fromString("2016-10-31T10:00:00+08:00", Qt::ISODate);

Word::Word(QString word, QString definition) :
    m_new(true),
    m_spelling(word),
    m_definition(definition)
{
    m_expireTime = defaultExpireTime();
}

void Word::setExpireTime(const QDateTime &expireTime)
{
    m_new = false;
    m_expireTime = expireTime;
}

void Word::setDefinition(const QString &definition)
{
    m_definition = definition;
}

bool Word::isInDatabase(const QString &spelling)
{
    QSqlQuery query;
    query.prepare("SELECT * FROM words WHERE word=:word  COLLATE NOCASE");
    query.bindValue(":word", spelling);
    if (query.exec()) {
        return query.first();
    } else {
        QSqlError error = query.lastError();
        QMessageBox::critical(nullptr, QObject::tr(""),
            "failed to query " + spelling + " in Word::isInDatabase()" + error.text(), QMessageBox::Ok);
        return false;
    }

    return false;
}

void Word::saveDefinition() const
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
        QSqlError error = query.lastError();
        QMessageBox::critical(nullptr, QObject::tr(""),
            "failed to insert " + m_spelling + " in Word::saveToDatabase()" + error.text(), QMessageBox::Ok);
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
            return query.value(0).toInt();
        } else {
            return 0;
        }
    } else {
        QSqlError error = query.lastError();
        QMessageBox::critical(nullptr, QObject::tr(""),
            "failed to query " + m_spelling + " in Word::isInDatabase()" + error.text(), QMessageBox::Ok);
        return 0;
    }
}

// return 0 if not saved
// id of the word if saved
int Word::isExpireTimeSaved() const
{
    int id = isDefintionSaved();
    if (id == 0) {
        return 0;
    }

    QSqlQuery query;
    query.prepare("SELECT * FROM words_in_study WHERE id=:id LIMIT 1");
    query.bindValue(":id", id);
    if (query.exec()) {
        if (query.first()) {
            return id;
        } else {
            return 0;
        }
    } else {
        QSqlError error = query.lastError();
        QMessageBox::critical(nullptr, QObject::tr(""),
            "failed to query " + QString::number(id) + " in Word::isExpireTimeSaved()" + error.text(), QMessageBox::Ok);
        return 0;
    }
}

void Word::saveExpireTime() const
{
    if (isNew()) {
        return;
    }

    int id = isDefintionSaved();
    if (id == 0) {
        return;
    }
    gdDebug("id is %d", id);

    int expireSeconds = getIntExpireTime();
    QSqlQuery query;
    if (isExpireTimeSaved()) {
        // update
        query.prepare("UPDATE words_in_study SET expire=:expire WHERE id=:id");
    } else {
        // insert
        query.prepare("INSERT INTO words_in_study(id, expire) VALUES(:id, :expire)");
    }
    query.bindValue(":id", id);
    query.bindValue(":expire", expireSeconds);
    if (query.exec() == false)
    {
        QSqlError error = query.lastError();
        QMessageBox::critical(nullptr, QObject::tr(""),
            "failed in Word::saveExpireTime()" + error.text(), QMessageBox::Ok);
    }
}

void Word::saveToDatabase() const
{
    saveDefinition();
    saveExpireTime();
}

// word is updated if the returned value is true
bool Word::getFromDatabase()
{
    int id = 0;
    QSqlQuery query;
    query.prepare("SELECT id, word, definition FROM words WHERE word=:word COLLATE NOCASE");
    query.bindValue(":word", m_spelling);
    if (query.exec()) {
        if (query.first()) {
            // word exist
            setDefinition(query.value(2).toString());
            id = query.value(0).toInt();
        } else {
            return false;
        }
    } else {
        QSqlError error = query.lastError();
        QMessageBox::critical(nullptr, QObject::tr(""),
            "failed in Word::getFromDatabase()" + error.text(), QMessageBox::Ok);
        return false;
    }

    query.prepare("SELECT id, expire FROM words_in_study WHERE id=:id LIMIT 1");
    query.bindValue(":id", id);
    if (query.exec()) {
        if (query.first()) {
            int seconds = query.value(1).toInt();
            setExpireTime(getDatetimeExpireTime(seconds));
        }
        return true;
    } else {
        QSqlError error = query.lastError();
        QMessageBox::critical(nullptr, QObject::tr(""),
            "failed to query " + QString::number(id) + " in Word::getFromDatabase()" + error.text(), QMessageBox::Ok);
        return false;   // something wrong, let's return false
    }
}

sptr<Word> Word::getWordFromDatabase(const QString &spelling) {
    sptr<Word> word = new Word(spelling);
    if (word->getFromDatabase()) {
        return word;
    } else {
        return sptr<Word>();
    }
}


QVector<sptr<Word>> Word::getNewWords(int number)
{
    QVector<sptr<Word>> wordList;

    if (number > 0) {
        QSqlQuery query;//("SELECT word FROM words WHERE id NOT IN (SELECT id FROM words_in_study) LIMIT '" + QString::number(number) + "'");
        query.prepare("SELECT word FROM words WHERE id NOT IN (SELECT id FROM words_in_study) LIMIT :number");
        query.bindValue(":number", number);

        if (query.exec()) {
            while (query.next()) {
                auto word = getWordFromDatabase(query.value(0).toString());
                wordList.append(word);
            }
        } else {
            QSqlError error = query.lastError();
            QMessageBox::critical(nullptr, QObject::tr(""),
                "failed to query in Word::getNewWords()" + error.text(), QMessageBox::Ok);
        }

    }

    return wordList;
}

void Word::createDatabaseTables()
{
    QSqlQuery query;
    if (query.exec("SELECT * FROM words LIMIT 1") == false)
    {
        // table "words" does not exist
        query.exec("CREATE TABLE words (id INTEGER primary key, "
                   "word TEXT, "
                   "definition TEXT)");
        query.exec("CREATE TABLE words_in_study (id INTEGER, "
                   "expire INTEGER)");
    } else {
        // table already exist
        QString msg( "Table \"words\" already exists, doing nothing in Word::createDatabaseTables()." );
        gdDebug("%s", msg.toStdString().c_str());
        QMessageBox::information(nullptr, QObject::tr(""),
            msg, QMessageBox::Ok);
    }
}
