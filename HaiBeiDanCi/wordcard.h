#ifndef WORDCARD_H
#define WORDCARD_H

#include "memoryitem.h"
#include "word.h"

#include <QFuture>
#include <QMutex>

struct StudyRecord;

class MyTime
{
  private:
    static const QDateTime m_baselineTime;

  private:
    QDateTime m_datetime;

  public:
    MyTime(qint64 minutes) : m_datetime(m_baselineTime.addSecs(minutes * 60))
    {
    }

    MyTime(QDateTime aTime) : m_datetime(aTime)
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

class WordCard : public MemoryItem
{
  public:
    virtual ~WordCard() override;

    bool dbsaveStudyRecord(const StudyRecord &sr);
    void setExpireTime(const QDateTime &expireTime);
    const QDateTime getExpireTime();
    const QDateTime getLastStudyTime();
    QVector<StudyRecord> getStudyHistory();

    virtual void update(ResponseQuality responseQuality) override;
    int estimatedInterval(ResponseQuality responseQuality = Perfect);

    // card status: new, learning, reviewing
    // a new card is a learning card
    bool isNew();
    bool isLearning();
    bool isReviewing();

    sptr<Word> getWord() const
    {
        return Word::getWord(m_wordSpelling);
    }

    static bool createDatabaseTables();
    static void readAllCardsFromDatabase();

    static bool doesWordHaveCard(const QString &spelling);
    static sptr<WordCard> getCard(const QString &spelling, bool create = false);

    static QDateTime defaultExpireTime();

    static QVector<QString> getAllWords(int number = 0);
    static QVector<QString> getNewWords(int number = 0);
    static QVector<QString> getOldWords(int number = 0);
    static QVector<QString> getExpiredWords(const QDateTime expire, int number = 0);

  private:
    WordCard(const QString &spelling);
    WordCard(const QString &spelling, const StudyRecord &sr);

    int defaultInterval()
    {
        return m_defaultInterval;
    }

    int defaultIntervalForKnownNewWord()
    {
        return m_defaultIntervalForKnownNewWord;
    }

    int defaultIntervalForUnknownNewWord()
    {
        return m_defaultIntervalForUnknownNewWord;
    }

    int defaultIntervalForRelearning()
    {
        return m_defaultIntervalForUnknownNewWord;
    }

    void dbsave();

    int estimatedIntervalNewCard(ResponseQuality responseQuality = Perfect);
    int estimatedIntervalOldCard(ResponseQuality responseQuality = Perfect);
    float getAdjustProportion();
    float getEasinessAdjustRatio(ResponseQuality responseQuality);
    float getIntervalAdjustRatio(ResponseQuality responseQuality);

    float estimatedEasiness(ResponseQuality responseQuality);

  private:
    QString m_wordSpelling;
    QVector<StudyRecord> m_studyHistory;

    static QMap<QString, sptr<WordCard>> m_allCards;
    static QMutex m_allCardsMutex;

    static int m_defaultInterval;
    static int m_defaultIntervalForUnknownNewWord;
    static int m_defaultIntervalForKnownNewWord;
    static float m_defaultEasiness;

    static float m_defaultPerfectIncrease;
    static float m_defaultCorrectIncrease;
    static float m_defaultKindRememberIncrease;
    static float m_defaultIncorrectIncrease;
};

struct StudyRecord
{
    int m_repetition; //
    int m_interval;   // in minutes
    float m_easiness;
    MyTime m_expire;
    MyTime m_studyDate;

    StudyRecord(QDateTime expire, QDateTime studyDate) : m_expire(expire),
                                                         m_studyDate(studyDate)
    {
    }

    StudyRecord(qint64 expireInMinutes, qint64 studyDateInMinutes) : m_expire(expireInMinutes),
                                                                     m_studyDate(studyDateInMinutes)
    {
    }

    StudyRecord() : m_expire(WordCard::defaultExpireTime()),
                    m_studyDate(QDateTime::currentDateTime())
    {
    }
};

#endif // WORDCARD_H
