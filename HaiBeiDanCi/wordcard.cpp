#include "wordcard.h"
#include "../golddict/gddebug.hh"
#include "worddb.h"

#include <QDateTime>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QVariant>

// m_baselineTime is my daughter's birth time
const QDateTime MyTime::m_baselineTime = QDateTime::fromString("2016-10-31T10:00:00+08:00", Qt::ISODate);
const float WordCard::m_ratio[MemoryItem::Perfect + 1] = {0.1, 0.1, 0.1, 0.64, 0.8, 1.0};

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
    int interval = static_cast<int>(getIntervalInMinute() * m_ratio[responseQuality]);
    setInterval(interval);

    dbsave();
}

int WordCard::estimatedInterval(ResponseQuality responseQuality)
{
    // we need to update the values from database before estimating
    updateFromDatabase();

    /*
    如何计算下一次复习的时间？
    至少应和下面这些相关：
    1、这个单词的难度，最小值1.3，最大值呢？
    2、这次复习对这个单词的熟悉程度（“不认识”、“有点印象”、“想起来了”，“记住了”）
    3、上次复习到这次复习的时间间隔（通常来说，这个也就是上次复习时定下来的间隔【这个是不对的，因为这次复习的时候是间隔时长已经过去了，单词已经expire了】。但有些时候，我们可以提前复习，那么这两个值就不相等了。）
    */

    return MemoryItem::estimatedInterval(responseQuality) * m_ratio[responseQuality];
}

void WordCard::dbgetStudyRecords()
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
    query.prepare("SELECT interval, easiness, repitition, expire, study_date"
                  " FROM wordcards WHERE word_id=:word_id");
    query.bindValue(":word_id", wordId);
    if (query.exec()) {
        while (query.next()) {
            int interval = query.value("interval").toInt();
            float easiness = query.value("easiness").toInt() / 100.0;
            int repitition = query.value("repitition").toInt();
            qint64 expire = query.value("expire").toLongLong();
            qint64 studyDate = query.value("study_date").toLongLong();
            StudyRecord sr(expire, studyDate);
            sr.m_easiness = easiness;
            sr.m_interval = interval;
            sr.m_repition = repitition;
            m_studyHistory.append(sr);
            setInterval(interval);
            setEasiness(easiness);
            setRepitition(repitition);
        }
    } else {
        WordDB::databaseError(query, "fetching card of \"" + m_word->getSpelling()  + "\"");
    }
}

/*
void WordCard::dbgetStudyRecords()
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
            qint64 expire = query.value("expire").toLongLong();
            qint64 studyDate = query.value("study_date").toLongLong();
            StudyRecord sr(expire, studyDate);
            m_studyHistory.append(sr);
        }
    } else {
        WordDB::databaseError(query, "fetching expire time of \"" + m_spelling + "\"");
    }
}
*/

const QDateTime WordCard::getExpireTime()
{
    updateFromDatabase();

    if (m_studyHistory.isEmpty())
    {
        return defaultExpireTime();
    }

    return m_studyHistory.last().m_expire.toDateTime();
}

void WordCard::setExpireTime(const QDateTime &expireTime)
{
    // we need to make sure easiness, reptition, interval are ALREADY updated from the database
    // m_studyHistory also need to be updated before setting
    updateFromDatabase();

    StudyRecord newSR(expireTime, QDateTime::currentDateTime());
    newSR.m_easiness = getEasiness();
    newSR.m_interval = getIntervalInMinute();
    newSR.m_repition = getRepitition();
    m_studyHistory.append(newSR);
    dbsaveStudyRecord(newSR);
}

void WordCard::dbsave()
{
    int days = qRound(getIntervalInMinute() / 60.0 / 24.0);
    QDateTime expire = QDateTime::currentDateTime().addDays(days);
    setExpireTime(expire);
}

QVector<StudyRecord> WordCard::getStudyHistory()
{
    updateFromDatabase();
    return m_studyHistory;
}

void WordCard::dbsaveStudyRecord(const StudyRecord &sr)
{
    if (!m_word.get()) {
        return;
    }

    int wordId = m_word->getId();
    if (wordId == 0) {
        assert(false);
        return;
    }

    int easiness = static_cast<int>(sr.m_easiness * 100);

    QSqlQuery query;
    query.prepare("INSERT INTO wordcards(word_id, interval, easiness, repitition, expire, study_date)"
                  " VALUES(:word_id, :interval, :easiness, :repitition, :expire, :study_date)");
    query.bindValue(":word_id", wordId);
    query.bindValue(":interval", sr.m_interval);
    query.bindValue(":easiness", easiness);
    query.bindValue(":repitition", sr.m_repition);
    query.bindValue(":expire", sr.m_expire.toMinutes());
    query.bindValue(":study_date", sr.m_studyDate.toMinutes());
    if (query.exec() == false)
    {
        WordDB::databaseError(query, "saving card of \"" + m_word->getSpelling()  + "\"");
    }
}

void WordCard::updateFromDatabase()
{
    if (hasUpdatedFromDatabase() == true) {
        return;
    }

    DatabaseObject::updateFromDatabase();
    dbgetStudyRecords();
}

int WordCard::getIntervalInMinute()
{
    updateFromDatabase();

    return MemoryItem::getIntervalInMinute();
}

float WordCard::getEasiness()
{
    updateFromDatabase();

    return MemoryItem::getEasiness();
}

int WordCard::getRepitition()
{
    updateFromDatabase();

    return MemoryItem::getRepitition();
}

// static
QDateTime WordCard::defaultExpireTime()
{
    return QDateTime::currentDateTime().addYears(100);
}

// static
sptr<WordCard> WordCard::generateCardForWord(const QString &spelling)
{
    //sptr<Word> word = Word::getWordFromDatabase(spelling);
    sptr<Word> word = new Word(spelling);
    sptr<WordCard> card = new WordCard(word);

    return card;
}

// static
bool WordCard::createDatabaseTables()
{
    QSqlQuery query;
    if (query.exec("SELECT * FROM wordcards LIMIT 1") == false)
    {
        // table "wordcards" does not exist
        if(query.exec("CREATE TABLE wordcards (id INTEGER primary key, "
                      "word_id INTEGER, "
                      "interval INTEGER, "
                      "easiness INTEGER, "
                      "repitition INTEGER, "
                      "expire INTEGER, "
                      "study_date INTEGER)") == false) {
            WordDB::databaseError(query, "creating table \"wordcards\"");
            return false;
        }
    } else {
        // table already exist
        QString msg( "Table \"wordcards\" already exists, doing nothing in WordCard::createDatabaseTables()." );
        gdDebug("%s", msg.toStdString().c_str());
    }

    return true;
}
