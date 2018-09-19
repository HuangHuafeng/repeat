#include "wordcard.h"
#include "worddb.h"
#include "mysettings.h"

#include <QDateTime>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QVariant>

// m_baselineTime is my daughter's birth time
const QDateTime MyTime::m_baselineTime = QDateTime::fromString("2016-10-31T10:00:00+08:00", Qt::ISODate);
QMap<QString, sptr<WordCard>> WordCard::m_allCards;
QMutex WordCard::m_allCardsMutex;

WordCard::WordCard(const QString &spelling) : MemoryItem(MySettings::cardDefaultIntervalInMinutes(), MySettings::defaultEasiness(), 0)
{
    m_wordSpelling = spelling;
}

/**
 * @brief WordCard::WordCard
 * create a new card with study record "sr", sr is used as
 * the last study record.
 * @param spelling
 * @param sr
 */
WordCard::WordCard(const QString &spelling, const StudyRecord &sr) : MemoryItem(sr.m_interval, sr.m_easiness, sr.m_repetition)
{
    m_wordSpelling = spelling;
    m_studyHistory.append(sr);
}

WordCard::~WordCard()
{
}

void WordCard::update(ResponseQuality responseQuality)
{
    int nextInterval;
    float nextEasiness;
    int nextRepetition;

    if (responseQuality <= MemoryItem::IncorrectButCanRecall && isNew() == false)
    {
        // set it relearn
        nextInterval = estimatedInterval(responseQuality);
        nextRepetition = 0;           // repetition reset to 0
        nextEasiness = getEasiness(); // easiness don't change
    }
    else
    {
        nextInterval = estimatedInterval(responseQuality);
        nextRepetition = getRepetition() + 1;
        nextEasiness = estimatedEasiness(responseQuality);
    }

    setIntervalInMinute(nextInterval);
    setEasiness(nextEasiness);
    setRepetition(nextRepetition);
    dbsave();
}

bool WordCard::isNew()
{
    return m_studyHistory.isEmpty();
}

bool WordCard::isLearning()
{
    return getRepetition() < 3;
}

bool WordCard::isReviewing()
{
    return isLearning() == false;
}

int WordCard::estimatedInterval(ResponseQuality responseQuality)
{
    int interval;
    if (isNew())
    {
        interval = estimatedIntervalNewCard(responseQuality);
    }
    else
    {
        interval = estimatedIntervalOldCard(responseQuality);
    }

    if (interval > MySettings::cardMaximumIntervalInMinutes())
    {
        interval = MySettings::cardMaximumIntervalInMinutes();
    }

    return interval;
}

int WordCard::estimatedIntervalNewCard(ResponseQuality responseQuality)
{
    return defaultInterval(responseQuality);
}

int WordCard::defaultInterval(ResponseQuality responseQuality)
{
    float tempInterval = MySettings::cardDefaultIntervalInMinutes();
    if (responseQuality == MemoryItem::Perfect)
    {
        tempInterval = MySettings::cardDefaultIntervalInMinutes() * (1.0f + MySettings::perfectIncrease()) / (1.0f + MySettings::vagueIncrease());
    }
    else if (responseQuality == MemoryItem::CorrectAfterHesitation)
    {
        tempInterval = MySettings::cardDefaultIntervalInMinutes() * (1.0f + MySettings::correctIncrease()) / (1.0f + MySettings::vagueIncrease());
    }
    else if (responseQuality == MemoryItem::CorrectWithDifficulty)
    {
        tempInterval = MySettings::cardDefaultIntervalInMinutes();
    }
    else
    {
        // responseQuality <= MemoryItem::IncorrectButCanRecall
        tempInterval = MySettings::cardIntervalForIncorrect();
    }

    return static_cast<int>(tempInterval);
}

int WordCard::estimatedIntervalOldCard(ResponseQuality responseQuality)
{
    int estimatedInterval = 0;

    if (responseQuality == MemoryItem::Perfect || responseQuality == MemoryItem::CorrectAfterHesitation || responseQuality == MemoryItem::CorrectWithDifficulty)
    {
        int ci = getIntervalInMinute();
        float ee = estimatedEasiness(responseQuality);
        auto ia = getIntervalAdjustRatio(responseQuality);
        auto tempInterval = ci * ee * ia;
        estimatedInterval = static_cast<int>(tempInterval);
        if (estimatedInterval < MySettings::cardDefaultIntervalInMinutes())
        {
            // change it to defaultInterval()
            estimatedInterval = defaultInterval(responseQuality);
        }
    }
    else
    {
        estimatedInterval = defaultInterval(MemoryItem::IncorrectButCanRecall);
    }

    return estimatedInterval;
}

/**
 * @brief WordCard::getAdjustProportion
 * @return -1.0 < proportion < 1.0
 */
float WordCard::getAdjustProportion()
{
    if (isNew())
    {
        return 0.0f;
    }

    int lastInterval = static_cast<int>(m_studyHistory.last().m_interval);
    int minutesPast = static_cast<int>((MyTime(QDateTime::currentDateTime()).toMinutes() - m_studyHistory.last().m_studyDate.toMinutes()));
    int diff = minutesPast - lastInterval;
    float proportion = diff * 1.0f / lastInterval;
    if (proportion > 1.0f)
    {
        proportion = 1.0f;
    }

    return proportion;
}

/**
 * @brief WordCard::getEasinessAdjustRatio
 * @param responseQuality
 * @return
 * assumes:
 * 1.0f > perfectIncrease >= 0
 * 1.0f > correctIncrease >= 0
 * -1.0f < vagueIncrease <= 0
 */
float WordCard::getEasinessAdjustRatio(ResponseQuality responseQuality)
{
    // set a factor so we don't punish too much
    // don't expose it to the user, avoid making this too complicated
    const auto punishmentFactor = 0.4f;

    auto proportion = getAdjustProportion();
    float ratio = 0.0f;
    if (responseQuality == MemoryItem::Perfect)
    {
        ratio = 1.0f + MySettings::perfectIncrease() * (1.0f + proportion);
    }
    else if (responseQuality == MemoryItem::CorrectAfterHesitation)
    {
        ratio = 1.0f + MySettings::correctIncrease() * (1.0f + proportion);
    }
    else if (responseQuality == MemoryItem::CorrectWithDifficulty)
    {
        if (isLearning())
        {
            ratio = 1.0f;
        }
        else
        {
            ratio = 1.0f + MySettings::vagueIncrease() * (1.0f - punishmentFactor * proportion);
        }
    }
    else
    {
        if (isLearning())
        {
            ratio = 1.0f;
        }
        else
        {
            ratio = 1.0f + MySettings::incorrectIncrease() * (1.0f - punishmentFactor * proportion);
        }
    }

    return ratio;
}

float WordCard::getIntervalAdjustRatio(ResponseQuality responseQuality)
{
    auto proportion = getAdjustProportion();
    float ratio = 0.0f;

    if (proportion > -0.5f)
    {
        ratio = 1.0f + proportion;
    }
    else
    {
        auto currentEasiness = getEasiness();
        ratio = 1.0f / currentEasiness;
    }

    // make it related to responseQuality
    float r1 = 0.0f;
    if (responseQuality == MemoryItem::Perfect)
    {
        r1 = 1.0f + MySettings::perfectIncrease();
    }
    else if (responseQuality == MemoryItem::CorrectAfterHesitation)
    {
        r1 = 1.0f + MySettings::correctIncrease();
    }
    else if (responseQuality == MemoryItem::CorrectWithDifficulty)
    {
        r1 = 1.0f + MySettings::vagueIncrease();
    }
    else
    {
        // the code should NOT reach here!
        r1 = 1.0f + MySettings::incorrectIncrease();
    }
    ratio *= r1;

    return ratio;
}

float WordCard::estimatedEasiness(ResponseQuality responseQuality)
{
    auto currentEasiness = getEasiness();
    auto ratio = getEasinessAdjustRatio(responseQuality);
    float estimated = currentEasiness * ratio;

    if (estimated < 1.3f)
    {
        estimated = 1.3f;
    }

    return estimated;
}

const QDateTime WordCard::getExpireTime()
{
    if (m_studyHistory.isEmpty())
    {
        return defaultExpireTime();
    }

    return m_studyHistory.last().m_expire.toDateTime();
}

const QDateTime WordCard::getLastStudyTime()
{
    if (m_studyHistory.isEmpty())
    {
        return defaultExpireTime();
    }

    return m_studyHistory.last().m_studyDate.toDateTime();
}

void WordCard::setExpireTime(const QDateTime &expireTime)
{
    StudyRecord newSR(expireTime, QDateTime::currentDateTime());
    newSR.m_easiness = getEasiness();
    newSR.m_interval = getIntervalInMinute();
    newSR.m_repetition = getRepetition();
    m_studyHistory.append(newSR);
    dbsaveStudyRecord(newSR);
}

void WordCard::dbsave()
{
    qint64 seconds = getIntervalInMinute() * 60;
    QDateTime expire = QDateTime::currentDateTime().addSecs(seconds);
    setExpireTime(expire);
}

QVector<StudyRecord> WordCard::getStudyHistory()
{
    return m_studyHistory;
}

bool WordCard::dbsaveStudyRecord(const StudyRecord &sr)
{
    auto word = Word::getWordToRead(m_wordSpelling);
    if (word == nullptr)
    {
        return false;
    }

    int easiness = static_cast<int>(sr.m_easiness * 100);

    auto ptrQuery = WordDB::createSqlQuery();
    if (ptrQuery.get() == nullptr)
    {
        return false;
    }
    auto query = *ptrQuery;
    query.prepare("INSERT INTO wordcards(word_id, interval, easiness, repetition, expire, study_date)"
                  " VALUES(:word_id, :interval, :easiness, :repetition, :expire, :study_date)");
    query.bindValue(":word_id", word->getId());
    query.bindValue(":interval", sr.m_interval);
    query.bindValue(":easiness", easiness);
    query.bindValue(":repetition", sr.m_repetition);
    query.bindValue(":expire", sr.m_expire.toMinutes());
    query.bindValue(":study_date", sr.m_studyDate.toMinutes());
    if (query.exec() == false)
    {
        WordDB::databaseError(query, "saving card of \"" + m_wordSpelling + "\"");
        return false;
    }

    return true;
}

// static
QDateTime WordCard::defaultExpireTime()
{
    return QDateTime::currentDateTime().addYears(100);
}

// static
/**
 * @brief WordCard::readAllCardsFromDatabase
 * read all cards from the database with only one query
 * this improves the performance by using less query
 * @return
 */
void WordCard::readAllCardsFromDatabase()
{
    m_allCardsMutex.lock();
    bool alreadyRead = m_allCards.isEmpty() == false;
    m_allCardsMutex.unlock();
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
    query.prepare(" SELECT word, c.*"
                  " FROM wordcards AS c"
                  " INNER JOIN words AS w"
                  " ON c.word_id=w.id"
                  " ORDER BY c.id DESC");
    if (query.exec())
    {
        m_allCardsMutex.lock();
        while (query.next())
        {
            QString spelling = query.value("word").toString();
            qint64 expire = query.value("expire").toLongLong();
            qint64 studyDate = query.value("study_date").toLongLong();
            StudyRecord sr(expire, studyDate);
            sr.m_easiness = query.value("easiness").toInt() / 100.0f;
            sr.m_interval = query.value("interval").toInt();
            sr.m_repetition = query.value("repetition").toInt();

            auto card = m_allCards.value(spelling);
            if (card.get() == nullptr)
            {
                // create the card
                sptr<WordCard> card = new WordCard(spelling, sr);
                m_allCards.insert(spelling, card);
            }
            else
            {
                // add the study history
                card->m_studyHistory.prepend(sr);
            }
        }
        m_allCardsMutex.unlock();
    }
    else
    {
        WordDB::databaseError(query, "fetching all cards from database");
    }
}

// static
/**
 * @brief WordCard::getCard
 * get the card of the word or nullptr if there's no card for the word
 * @param spelling
 */
sptr<WordCard> WordCard::getCard(const QString &spelling, bool create)
{
    m_allCardsMutex.lock();
    auto card = m_allCards.value(spelling);
    if (card.get() == nullptr && create == true)
    {
        card = new WordCard(spelling);
        if (card.get())
        {
            m_allCards.insert(spelling, card);
        }
    }
    m_allCardsMutex.unlock();

    return card;
}

// static
bool WordCard::doesWordHaveCard(const QString &spelling)
{
    return WordCard::getCard(spelling).get() != nullptr;
}

// static
bool WordCard::createDatabaseTables()
{
    auto ptrQuery = WordDB::createSqlQuery();
    if (ptrQuery.get() == nullptr)
    {
        return false;
    }
    auto query = *ptrQuery;
    if (query.exec("SELECT * FROM wordcards LIMIT 1") == false)
    {
        // table "wordcards" does not exist
        if (query.exec("CREATE TABLE wordcards (id INTEGER primary key, "
                       "word_id INTEGER, "
                       "interval INTEGER, "
                       "easiness INTEGER, "
                       "repetition INTEGER, "
                       "expire INTEGER, "
                       "study_date INTEGER)") == false)
        {
            WordDB::databaseError(query, "creating table \"wordcards\"");
            return false;
        }
    }

    return true;
}

/**
  get a list of words that is new (a new word has definition, but has no study record)
  only spelling is added to the list, Word object should be created by the caller to
  keep flexibility
  */
// static
QVector<QString> WordCard::getNewWords(int number)
{
    QVector<QString> wordList;
    auto ptrQuery = WordDB::createSqlQuery();
    if (ptrQuery.get() == nullptr)
    {
        return wordList;
    }
    auto query = *ptrQuery;
    QString sql = "SELECT word"
                  " FROM words"
                  " WHERE id"
                  " NOT IN (SELECT DISTINCT word_id FROM wordcards)"
                  " ORDER BY id ASC";
    if (number > 0)
    {
        query.prepare(sql + " LIMIT :limit");
        query.bindValue(":limit", number);
    }
    else
    {
        query.prepare(sql);
    }
    if (query.exec())
    {
        while (query.next())
        {
            QString spelling = query.value("word").toString();
            wordList.append(spelling);
        }
    }
    else
    {
        WordDB::databaseError(query, "fetching new words");
    }

    return wordList;
}

// static
QVector<QString> WordCard::getOldWords(int number)
{
    QVector<QString> wordList;
    auto ptrQuery = WordDB::createSqlQuery();
    if (ptrQuery.get() == nullptr)
    {
        return wordList;
    }
    auto query = *ptrQuery;
    QString sql = " SELECT word"
                  " FROM words AS w INNER JOIN wordcards as c ON w.id=c.word_id"
                  " WHERE"
                  " c.id IN (SELECT MAX(id) FROM wordcards GROUP BY word_id)"
                  " ORDER BY c.expire ASC";
    if (number > 0)
    {
        query.prepare(sql + " LIMIT :limit");
        query.bindValue(":limit", number);
    }
    else
    {
        query.prepare(sql);
    }
    if (query.exec())
    {
        while (query.next())
        {
            QString spelling = query.value("word").toString();
            wordList.append(spelling);
        }
    }
    else
    {
        WordDB::databaseError(query, "fetching new words");
    }

    return wordList;
}

// static
QVector<QString> WordCard::getAllWords(int number)
{
    QVector<QString> wordList;
    auto ptrQuery = WordDB::createSqlQuery();
    if (ptrQuery.get() == nullptr)
    {
        return wordList;
    }
    auto query = *ptrQuery;
    QString sql = "SELECT word FROM words ORDER BY id ASC";
    if (number > 0)
    {
        query.prepare(sql + " LIMIT :limit");
        query.bindValue(":limit", number);
    }
    else
    {
        query.prepare(sql);
    }
    if (query.exec())
    {
        while (query.next())
        {
            QString spelling = query.value("word").toString();
            wordList.append(spelling);
        }
    }
    else
    {
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
QVector<QString> WordCard::getExpiredWords(const QDateTime expire, int number)
{
    QVector<QString> wordList;
    auto ptrQuery = WordDB::createSqlQuery();
    if (ptrQuery.get() == nullptr)
    {
        return wordList;
    }
    auto query = *ptrQuery;
    auto expireInt = MyTime(expire).toMinutes();
    QString sql = " SELECT word"
                  " FROM words AS w INNER JOIN wordcards AS c ON w.id=c.word_id"
                  " WHERE"
                  " c.id IN (SELECT MAX(id) FROM wordcards GROUP BY word_id)"
                  " AND c.expire<:expireint"
                  " ORDER BY c.expire ASC";
    if (number > 0)
    {
        query.prepare(sql + " LIMIT :limit");
        query.bindValue(":limit", number);
    }
    else
    {
        query.prepare(sql);
    }
    query.bindValue(":expireint", expireInt);
    if (query.exec())
    {
        while (query.next())
        {
            QString spelling = query.value("word").toString();
            wordList.append(spelling);
        }
    }
    else
    {
        WordDB::databaseError(query, "fetching all expired words");
    }

    return wordList;
}
