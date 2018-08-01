#ifndef WORDCARD_H
#define WORDCARD_H

#include "databaseobject.h"
#include "memoryitem.h"
#include "word.h"

struct StudyRecord;

class MyTime
{
private:
    static const QDateTime m_baselineTime;

private:
    QDateTime m_datetime;

public:
    MyTime(qint64 minutes) :
         m_datetime(m_baselineTime.addSecs(minutes * 60))
    {
    }

    MyTime(QDateTime aTime) :
         m_datetime(aTime)
    {
    }

    qint64 toMinutes() const
    {
        return m_baselineTime.secsTo(m_datetime) / 60;
    }

    const QDateTime toDateTime() const
    {
        return m_datetime;
    }
};

class WordCard : public MemoryItem, public DatabaseObject
{
public:
    WordCard(sptr<Word> word = sptr<Word>(), int interval = 24, float easiness = 2.5, int repition = 0);
    virtual ~WordCard();
    virtual void update(ResponseQuality responseQuality) override;
    virtual int estimatedInterval(ResponseQuality responseQuality = Perfect) override;
    void dbsaveStudyRecord(const StudyRecord &sr);
    void setExpireTime(const QDateTime &expireTime);
    const QDateTime getExpireTime();

    /*
    int getIntervalInMinute();
    float getEasiness();
    int getRepitition();
    */

    sptr<Word> getWord() const
    {
        return m_word;
    }

public:
    static QDateTime defaultExpireTime();
    static bool createDatabaseTables();
    static sptr<WordCard> generateCardForWord(const QString &spelling);

protected:
    virtual void updateFromDatabase() override;

private:
    static const float m_ratio[MemoryItem::Perfect + 1];

    sptr<Word> m_word;
    QVector<StudyRecord> m_studyHistory;

    void dbsave();
    void dbgetStudyRecords();
};

struct StudyRecord
{
    int m_repition; //
    int m_interval; // in minutes
    float m_easiness;
    MyTime m_expire;
    MyTime m_studyDate;

    StudyRecord(QDateTime expire, QDateTime studyDate) :
        m_expire(expire),
        m_studyDate(studyDate)
    {
    }

    StudyRecord(int expireInMinutes, int studyDateInMinutes) :
        m_expire(expireInMinutes),
        m_studyDate(studyDateInMinutes)
    {
    }

    StudyRecord() :
        m_expire(WordCard::defaultExpireTime()),
        m_studyDate(QDateTime::currentDateTime())
    {
    }
};

#endif // WORDCARD_H
