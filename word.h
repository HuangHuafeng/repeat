#ifndef WORD_H
#define WORD_H

#include "golddict/sptr.hh"

#include <QString>
#include <QDateTime>
#include <QSqlQuery>
#include <QVector>

struct StudyRecord;

class MyTime
{
private:
    static const QDateTime m_baselineTime;

private:
    QDateTime m_datetime;

public:
    MyTime(int seconds) :
         m_datetime(m_baselineTime.addSecs(seconds))
    {
    }

    MyTime(QDateTime aTime) :
         m_datetime(aTime)
    {
    }

    int toSeconds() const
    {
        return m_baselineTime.secsTo(m_datetime);
    }

    const QDateTime toDateTime() const
    {
        return m_datetime;
    }
};

class Word
{
public:
    Word(QString word);

    bool isNew() const
    {
        return m_new;
    }

    void setDefinition(const QString &definition);
    void setExpireTime(const QDateTime &expireTime);
    const QDateTime & getExpireTime() const
    {
        return m_expireTime;
    }

    const QString & getSpelling() const
    {
        return m_spelling;
    }

    const QString & getDefinition() const
    {
        return m_definition;
    }

    void addExpireTimeRecord() const;
    bool isSaved();
    int isDefintionSaved() const;
    int hasExpireTimeRecord() const;

    void getFromDatabase();
    int getId();

    static void createDatabaseTables();
    static bool isInDatabase(const QString &spelling);
    static sptr<Word> getWordFromDatabase(const QString &spelling);
    static QVector<sptr<Word>> getNewWords(int number);
    static QDateTime defaultExpireTime();

private:
    static void databaseError(QSqlQuery &query, const QString what);
    static int getWordId(const QString &spelling);

    void dbsaveDefinition();
    void dbgetDefinition();
    void dbgetStudyRecords();
    void dbsaveStudyRecord(const StudyRecord &sr);

private:
    bool m_new;
    QString m_spelling;
    QString m_definition;
    QDateTime m_expireTime;
    QVector<StudyRecord> m_studyHistory;
};

struct StudyRecord
{
    MyTime m_expire;
    MyTime m_studyDate;

    StudyRecord(QDateTime expire, QDateTime studyDate) :
        m_expire(expire),
        m_studyDate(studyDate)
    {
    }

    StudyRecord(int expireInSeconds, int studyDateInSeconds) :
        m_expire(expireInSeconds),
        m_studyDate(studyDateInSeconds)
    {
    }

    StudyRecord() :
        m_expire(Word::defaultExpireTime()),
        m_studyDate(QDateTime::currentDateTime())
    {
    }
};

#endif // WORD_H
