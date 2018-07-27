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
    static QVector<QString> getNewWords(int number = 0);
    static QVector<QString> getWords(int number = 0);
    static QVector<QString> getExpiredWords(int number);
    static QDateTime defaultExpireTime();
    static void databaseError(QSqlQuery &query, const QString what);

private:
    static int getWordId(const QString &spelling);

    void dbsaveDefinition();
    void dbgetDefinition();
    void dbgetStudyRecords();
    void dbsaveStudyRecord(const StudyRecord &sr);

private:
    bool m_new;
    int m_id;   // id in database
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

    StudyRecord(int expireInMinutes, int studyDateInMinutes) :
        m_expire(expireInMinutes),
        m_studyDate(studyDateInMinutes)
    {
    }

    StudyRecord() :
        m_expire(Word::defaultExpireTime()),
        m_studyDate(QDateTime::currentDateTime())
    {
    }
};

#endif // WORD_H
