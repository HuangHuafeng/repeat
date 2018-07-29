#include "wordcard.h"
#include "golddict/gddebug.hh"
#include "worddb.h"

#include <QDateTime>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QVariant>

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
    如何计算下一次复习的时间？
    至少应和下面这些相关：
    1、这个单词的难度，最小值1.3，最大值呢？
    2、这次复习对这个单词的熟悉程度（“不认识”、“有点印象”、“想起来了”，“记住了”）
    3、上次复习到这次复习的时间间隔（通常来说，这个也就是上次复习时定下来的间隔【这个是不对的，因为这次复习的时候是间隔时长已经过去了，单词已经expire了】。但有些时候，我们可以提前复习，那么这两个值就不相等了。）
    */

    return MemoryItem::estimatedInterval(responseQuality) * m_ratio[responseQuality];
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
        WordDB::databaseError(query, "fetching card of \"" + m_word->getSpelling()  + "\"");
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
        WordDB::databaseError(query, "saving card of \"" + m_word->getSpelling()  + "\"");
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
bool WordCard::createDatabaseTables()
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
            WordDB::databaseError(query, "creating table \"wordcard\"");
            return false;
        }
    } else {
        // table already exist
        QString msg( "Table \"wordcard\" already exists, doing nothing in WordCard::createDatabaseTables()." );
        gdDebug("%s", msg.toStdString().c_str());
    }

    return true;
}
