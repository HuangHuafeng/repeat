#include "wordcard.h"
#include "golddict/gddebug.hh"

#include <QDateTime>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QVariant>

float WordCard::m_ratio = 1.0f;

WordCard::WordCard(sptr<Word> word, int interval, float easiness, int repition) :
    MemoryItem(interval, easiness, repition)
{
    m_word = word;
}

WordCard::~WordCard()
{

}

void WordCard::update(ResponseQuality responseQuality)
{
    MemoryItem::update(responseQuality);

    // changes the interval with the ratio
    int interval = static_cast<int>(getIntervalInMinute() * m_ratio);
    setInterval(interval);

    // save to database
    dbsave();

    // update the word's expire
    if (m_word.get()) {
        int days = qRound(getIntervalInMinute() / 60.0 / 24.0);
        QDateTime expire = QDateTime::currentDateTime().addDays(days);
        m_word->setExpireTime(expire);
    }
}

int WordCard::estimatedInterval(ResponseQuality responseQuality) const
{
    /*
    int repitition = getRepitition();
    if (repitition == 0) {//I(1)
        return 24 * 60;
    }

    if (repitition == 1) {//I(2)
        return 24 * 60;
    }

    if (responseQuality <= IncorrectButCanRecall) {
        return 24 * 60;
    }
    */

    return MemoryItem::estimatedInterval(responseQuality);
}

void WordCard::getFromDatabase()
{
    if (!m_word.get()) {
        return;
    }

    int wordId = m_word->getId();
    if (wordId == 0) {
        assert(false);
        return;
    }

    QSqlQuery query;
    query.prepare("SELECT interval, easiness, repitition FROM wordcard WHERE word_id=:word_id");
    query.bindValue(":word_id", wordId);
    if (query.exec()) {
        if (query.last()) {
            int interval = query.value("interval").toInt();
            float easiness = query.value("easiness").toInt() / 100.0;
            int repitition = query.value("repitition").toInt();
            setInterval(interval);
            setEasiness(easiness);
            setRepitition(repitition);
        }
    } else {
        Word::databaseError(query, "fetching card of \"" + m_word->getSpelling()  + "\"");
    }
}

void WordCard::dbsave()
{
    if (!m_word.get()) {
        return;
    }

    int wordId = m_word->getId();
    if (wordId == 0) {
        assert(false);
        return;
    }

    int easiness = static_cast<int>(getEasiness() * 100);
    int interval = getIntervalInMinute();
    QSqlQuery query;
    query.prepare("INSERT INTO wordcard(word_id, interval, easiness, repitition) VALUES(:word_id, :interval, :easiness, :repitition)");
    query.bindValue(":word_id", wordId);
    query.bindValue(":interval", interval);
    query.bindValue(":easiness", easiness);
    query.bindValue(":repitition", getRepitition());
    if (query.exec() == false)
    {
        Word::databaseError(query, "saving card of \"" + m_word->getSpelling()  + "\"");
    }
}

// static
sptr<WordCard> WordCard::generateCardForWord(const QString &spelling)
{
    sptr<Word> word = Word::getWordFromDatabase(spelling);
    sptr<WordCard> card = new WordCard(word);
    if (card.get()) {
        card->getFromDatabase();
    }

    return card;
}

// static
void WordCard::createDatabaseTables()
{
    QSqlQuery query;
    if (query.exec("SELECT * FROM wordcard LIMIT 1") == false)
    {
        // table "wordcard" does not exist
        if(query.exec("CREATE TABLE wordcard (id INTEGER primary key, "
                      "word_id INTEGER, "
                      "interval INTEGER, "
                      "easiness INTEGER, "
                      "repitition INTEGER)") == false) {
            Word::databaseError(query, "creating table \"wordcard\"");
        }
    } else {
        // table already exist
        QString msg( "Table \"wordcard\" already exists, doing nothing in WordCard::createDatabaseTables()." );
        gdDebug("%s", msg.toStdString().c_str());
    }
}
