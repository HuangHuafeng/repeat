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
QMap<QString, sptr<WordCard>> WordCard::m_cards;

float WordCard::m_defaultEasiness = 2.5f;
int WordCard::m_defaultIntervalForUnknownNewWord = 10;   // 10 minutes
int WordCard::m_defaultInterval = 60 * 24;     // one day
int WordCard::m_defaultIntervalForKnownNewWord = 60 * 24  * 3;  // three days

WordCard::WordCard(sptr<Word> word) :
    MemoryItem(m_defaultInterval, m_defaultEasiness, 0)
{
    m_word = word;
}

WordCard::~WordCard()
{

}

void WordCard::update(ResponseQuality responseQuality)
{
    // changes the interval with the ratio
    //int interval = static_cast<int>(getIntervalInMinute());
    //setInterval(interval);
    int nextInterval;
    float nextEasiness;
    int nextRepetition;
    if (responseQuality <= MemoryItem::IncorrectButCanRecall && isNew() == false) {
        // set it relearn
        nextInterval = estimatedInterval(responseQuality);
        nextRepetition = 0;             // repetition reset to 0
        nextEasiness = getEasiness();   // easiness don't change
    } else {
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
    updateFromDatabase();

    return m_studyHistory.isEmpty();
}

int WordCard::estimatedInterval(ResponseQuality responseQuality)
{
    // we need to update the values from database before estimating
    updateFromDatabase();

    int interval;
    if (isNew()) {
        interval = estimatedIntervalNewCard(responseQuality);
    } else {
        interval = estimatedIntervalOldCard(responseQuality);
    }

    return interval;

    /*
    如何计算下一次复习的时间？
    至少应和下面这些相关：
    1、这个单词的难度，最小值1.3，最大值呢？
    2、这次复习对这个单词的熟悉程度（“不认识”、“有点印象”、“想起来了”，“记住了”）
    3、上次复习到这次复习的时间间隔（通常来说，这个也就是上次复习时定下来的间隔【这个是不对的，因为这次复习的时候是间隔时长已经过去了，单词已经expire了】。但有些时候，我们可以提前复习，那么这两个值就不相等了。）
    */
}

int WordCard::estimatedIntervalNewCard(ResponseQuality responseQuality)
{
    if (responseQuality <= MemoryItem::IncorrectButCanRecall)
    {
        return defaultIntervalForUnknownNewWord();
    } else if (responseQuality < MemoryItem::CorrectAfterHesitation)
    {
        // the user does not know the new word
        return defaultInterval();
    } else {
        // the user knows the new word!
        return defaultIntervalForKnownNewWord();
    }
}

int WordCard::estimatedIntervalOldCard(ResponseQuality responseQuality)
{
    int estimatedInterval = 0;

    if (responseQuality == MemoryItem::Perfect
            || responseQuality == MemoryItem::CorrectAfterHesitation
            || responseQuality == MemoryItem::CorrectWithDifficulty) {
        int ci = getIntervalInMinute();
        float ee = estimatedEasiness(responseQuality);
        estimatedInterval = static_cast<int>(ci * ee);
        // adjust the interval
        estimatedInterval = adjustInterval(responseQuality, estimatedInterval);
    } else {
        estimatedInterval = defaultIntervalForRelearning();
    }

    return estimatedInterval;
}

int WordCard::adjustInterval(ResponseQuality responseQuality, int interval)
{
    if (isNew() == true) {
        // the code should not reach here!
        return interval;
    }

    int lastInterval = static_cast<int>(m_studyHistory.last().m_interval);
    int minutesPast = static_cast<int>((MyTime(QDateTime::currentDateTime()).toMinutes() - m_studyHistory.last().m_studyDate.toMinutes()));

    if (minutesPast < (lastInterval / 2)) {
        // the user seems to review the wordcard again too soon!
        // in this case, we use the last interval
        //auto ee = estimatedEasiness(responseQuality);
        //interval = static_cast<int>(interval / ee);
        return interval;
    }

    int diff = minutesPast - lastInterval;

    if (diff < 0) {
        // the user reviews earlier than planned, so add penalty
        if (responseQuality == MemoryItem::Perfect) {
            diff = diff / 4;
        } else if (responseQuality == MemoryItem::CorrectAfterHesitation) {
            diff = diff / 2;
        } else if (responseQuality == MemoryItem::CorrectWithDifficulty) {
            diff = diff * 7 / 8;
        } else {
            // the code should NOT reach here!
            diff = 0;
        }
    } else {
        if (responseQuality == MemoryItem::Perfect) {
            diff = diff * 7 / 8;
        } else if (responseQuality == MemoryItem::CorrectAfterHesitation) {
            diff = diff / 2;
        } else if (responseQuality == MemoryItem::CorrectWithDifficulty) {
            diff = diff / 4;
        } else {
            // the code should NOT reach here!
            diff = 0;
        }
    }

    if (diff < 60 * 24) {
        // make it bigger than one day!!!
    }

    return interval + diff;
}

float WordCard::adjustEasiness(ResponseQuality responseQuality, float easiness)
{
    float adjusted = 0.0;
    if (isNew() == true) {
        if (responseQuality == MemoryItem::Perfect) {
            adjusted = m_defaultEasiness * 1.15f;
        } else if (responseQuality == MemoryItem::CorrectAfterHesitation) {
            adjusted = m_defaultEasiness * 1.10f;
        } else {
            // do not decrease the ease if the user don't know the NEW word
            adjusted = m_defaultEasiness;
        }
    } else {
        if (responseQuality < MemoryItem::CorrectWithDifficulty) {
            // the card going to relearning, so we don't change the easiness
            adjusted = easiness;
        } else {
            int lastInterval = static_cast<int>(m_studyHistory.last().m_interval);
            int minutesPast = static_cast<int>((MyTime(QDateTime::currentDateTime()).toMinutes() - m_studyHistory.last().m_studyDate.toMinutes()));

            if (0 && minutesPast < (lastInterval / 2)) {
                // the user seems to review the wordcard again too soon!
                adjusted = getEasiness();
            } else {
                int diff = minutesPast - lastInterval;

                if (diff < 0) {
                    auto ratio = diff * 0.3f / lastInterval;    // maximum 15% adjustment
                    // the user reviews earlier than planned, so add penalty
                    if (responseQuality == MemoryItem::Perfect
                            || responseQuality == MemoryItem::CorrectAfterHesitation
                            || responseQuality == MemoryItem::CorrectWithDifficulty) {
                        adjusted = easiness * (1 + ratio);
                    } else {
                        // the code should NOT reach here!
                        adjusted = easiness;
                    }
                } else {
                    float ratio = 0.0;    // maximum 15% adjustment
                    if (responseQuality == MemoryItem::Perfect) {
                        ratio = diff * 0.3f / lastInterval;
                        if (ratio > 0.3f) {
                            ratio = 0.3f;
                        }
                        adjusted = easiness * (1 + ratio);
                    } else if (responseQuality == MemoryItem::CorrectAfterHesitation) {
                        ratio = diff * 0.15f / lastInterval;
                        if (ratio > 0.15f) {
                            ratio = 0.15f;
                        }
                        adjusted = easiness * (1 + ratio);
                    } else if (responseQuality == MemoryItem::CorrectWithDifficulty) {
                        ratio = diff * 0.05f / lastInterval;
                        if (ratio > 0.05f) {
                            ratio = 0.05f;
                        }
                        adjusted = easiness * (1 + ratio);
                    } else {
                        // the code should NOT reach here!
                        adjusted = easiness;
                    }
                }
            }
        }
    }

    return adjusted;
}

// static
float WordCard::estimatedEasinessNoAdjustment(ResponseQuality responseQuality, float currentEasiness)
{
    float estimated = currentEasiness;

    if (responseQuality == MemoryItem::Perfect) {
        estimated = currentEasiness * 1.15f;    // add 15%
    } else if (responseQuality == MemoryItem::CorrectAfterHesitation) {
        estimated = currentEasiness;    // no change
    } else if (responseQuality == MemoryItem::CorrectWithDifficulty) {
        estimated = currentEasiness * 0.85f;
    } else {
        estimated = currentEasiness * 0.80f;
    }

    return estimated;
}

float WordCard::estimatedEasiness(ResponseQuality responseQuality)
{
    auto ce = getEasiness();
    auto estimated = estimatedEasinessNoAdjustment(responseQuality, ce);
    estimated = adjustEasiness(responseQuality, estimated);

    if (estimated < 1.3f) {
        estimated = 1.3f;
    }

    return estimated;
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

    auto ptrQuery = WordDB::createSqlQuery();auto query = *ptrQuery;
    query.prepare("SELECT interval, easiness, repetition, expire, study_date"
                  " FROM wordcards WHERE word_id=:word_id");
    query.bindValue(":word_id", wordId);
    if (query.exec()) {
        while (query.next()) {
            int interval = query.value("interval").toInt();
            float easiness = query.value("easiness").toInt() / 100.0f;
            int repetition = query.value("repetition").toInt();
            qint64 expire = query.value("expire").toLongLong();
            qint64 studyDate = query.value("study_date").toLongLong();
            StudyRecord sr(expire, studyDate);
            sr.m_easiness = easiness;
            sr.m_interval = interval;
            sr.m_repetition = repetition;
            m_studyHistory.append(sr);
        }
    } else {
        WordDB::databaseError(query, "fetching card of \"" + m_word->getSpelling()  + "\"");
    }

    if (m_studyHistory.isEmpty() == false) {
        auto sr = m_studyHistory.last();
        setIntervalInMinute(sr.m_interval);
        setEasiness(sr.m_easiness);
        setRepetition(sr.m_repetition);
    }
}

const QDateTime WordCard::getExpireTime()
{
    updateFromDatabase();

    if (m_studyHistory.isEmpty())
    {
        return defaultExpireTime();
    }

    return m_studyHistory.last().m_expire.toDateTime();
}

const QDateTime WordCard::getLastStudyTime()
{
    updateFromDatabase();

    if (m_studyHistory.isEmpty())
    {
        return QDateTime::currentDateTime();
    }

    return m_studyHistory.last().m_studyDate.toDateTime();
}

void WordCard::setExpireTime(const QDateTime &expireTime)
{
    // we need to make sure easiness, reptition, interval are ALREADY updated from the database
    // m_studyHistory also need to be updated before setting
    updateFromDatabase();

    StudyRecord newSR(expireTime, QDateTime::currentDateTime());
    newSR.m_easiness = getEasiness();
    newSR.m_interval = getIntervalInMinute();
    newSR.m_repetition = getRepetition();
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

    auto ptrQuery = WordDB::createSqlQuery();auto query = *ptrQuery;
    query.prepare("INSERT INTO wordcards(word_id, interval, easiness, repetition, expire, study_date)"
                  " VALUES(:word_id, :interval, :easiness, :repetition, :expire, :study_date)");
    query.bindValue(":word_id", wordId);
    query.bindValue(":interval", sr.m_interval);
    query.bindValue(":easiness", easiness);
    query.bindValue(":repetition", sr.m_repetition);
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

int WordCard::getRepetition()
{
    updateFromDatabase();

    return MemoryItem::getRepetition();
}

// static
QDateTime WordCard::defaultExpireTime()
{
    return QDateTime::currentDateTime().addYears(100);
}

// static
void WordCard::readAllCardsFromDatabase()
{
    static bool allCardsCreated = false;
    if (allCardsCreated == true) {
        return;
    }

    allCardsCreated = true;

    auto ptrQuery = WordDB::createSqlQuery();auto query = *ptrQuery;
    query.prepare(" SELECT word"
                  " FROM wordcards AS c"
                  " INNER JOIN words AS w"
                            " ON c.word_id=w.id"
                  " WHERE c.id"
                  " IN (SELECT MAX(id) FROM wordcards GROUP BY word_id)");
    if (query.exec()) {
        while (query.next()) {
            QString spelling = query.value("word").toString();
            sptr<Word> word = Word::getWordFromDatabase(spelling);
            sptr<WordCard> card = new WordCard(word);
            m_cards.insert(spelling, card);
        }
    } else {
        WordDB::databaseError(query, "fetching all cards from database");
    }
}

// static
/**
 * @brief WordCard::generateCardForWord
 * @param spelling
 * This function returns a card.
 * nullptr ONLY IN CASE word "spelling" does not exist in the database
 */
sptr<WordCard> WordCard::generateCardForWord(const QString &spelling)
{
    WordCard::readAllCardsFromDatabase();

    sptr<WordCard> card = getCardForWord(spelling);
    if (card.get()) {
        return  card;
    }

    sptr<Word> word = Word::getWordFromDatabase(spelling);
    if (word.get()) {
        card = new WordCard(word);
        m_cards.insert(spelling, card);
    }

    return card;
}

// static
/**
 * @brief WordCard::getCardForWord
 * @param spelling
 * This function returns a card.
 * 1) nullptr if there's no card for word "spelling"
 * 2) nullptr IN CASE word "spelling" does not exist in the database
 */
sptr<WordCard> WordCard::getCardForWord(const QString &spelling)
{
    WordCard::readAllCardsFromDatabase();

    return m_cards.value(spelling);
}

// static
bool WordCard::doesWordHaveCard(const QString &spelling)
{
    WordCard::readAllCardsFromDatabase();

    return m_cards.value(spelling).get() != nullptr;
}

// static
bool WordCard::createDatabaseTables()
{
    auto ptrQuery = WordDB::createSqlQuery();auto query = *ptrQuery;
    if (query.exec("SELECT * FROM wordcards LIMIT 1") == false)
    {
        // table "wordcards" does not exist
        if(query.exec("CREATE TABLE wordcards (id INTEGER primary key, "
                      "word_id INTEGER, "
                      "interval INTEGER, "
                      "easiness INTEGER, "
                      "repetition INTEGER, "
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


/**
  get a list of words that is new (a new word has definition, but has no study record)
  only spelling is added to the list, Word object should be created by the caller to
  keep flexibility
  */
// static
QVector<QString> WordCard::getNewWords(int number)
{
    QVector<QString> wordList;
    auto ptrQuery = WordDB::createSqlQuery();auto query = *ptrQuery;
    QString sql = "SELECT word"
                  " FROM words"
                  " WHERE id"
                            " NOT IN (SELECT DISTINCT word_id FROM wordcards)"
                  " ORDER BY id ASC";
    if (number > 0) {
        query.prepare(sql + " LIMIT :limit");
        query.bindValue(":limit", number);
    } else {
        query.prepare(sql);
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

// static
QVector<QString> WordCard::getOldWords(int number)
{
    QVector<QString> wordList;
    auto ptrQuery = WordDB::createSqlQuery();auto query = *ptrQuery;
    QString sql = " SELECT word"
                  " FROM words AS w INNER JOIN wordcards as c ON w.id=c.word_id"
                  " WHERE"
                            " c.id IN (SELECT MAX(id) FROM wordcards GROUP BY word_id)"
                  " ORDER BY c.expire ASC";
    if (number > 0) {
        query.prepare(sql + " LIMIT :limit");
        query.bindValue(":limit", number);
    } else {
        query.prepare(sql);
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

// static
QVector<QString> WordCard::getAllWords(int number)
{
    QVector<QString> wordList;
    auto ptrQuery = WordDB::createSqlQuery();auto query = *ptrQuery;
    QString sql = "SELECT word FROM words ORDER BY id ASC";
    if (number > 0) {
        query.prepare(sql + " LIMIT :limit");
        query.bindValue(":limit", number);
    } else {
        query.prepare(sql);
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
QVector<QString> WordCard::getExpiredWords(const QDateTime expire, int number)
{
    auto expireInt = MyTime(expire).toMinutes();
    QVector<QString> wordList;
    auto ptrQuery = WordDB::createSqlQuery();auto query = *ptrQuery;
    QString sql = " SELECT word"
                  " FROM words AS w INNER JOIN wordcards AS c ON w.id=c.word_id"
                  " WHERE"
                            " c.id IN (SELECT MAX(id) FROM wordcards GROUP BY word_id)"
                            " AND c.expire<:expireint"
                  " ORDER BY c.expire ASC";
    if (number > 0) {
        query.prepare(sql + " LIMIT :limit");
        query.bindValue(":limit", number);
    } else {
        query.prepare(sql);
    }
    query.bindValue(":expireint", expireInt);
    if (query.exec()) {
        while (query.next()) {
            QString spelling = query.value("word").toString();
            wordList.append(spelling);
        }
    } else {
        WordDB::databaseError(query, "fetching all expired words");
    }

    return wordList;
}
