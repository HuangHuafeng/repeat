#ifndef WORD_H
#define WORD_H

#include "golddict/sptr.hh"

#include <QString>
#include <QDateTime>

class Word
{
public:
    Word(QString word, QString definition = QString(""));

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

    void saveDefinition() const;
    void saveExpireTime() const;
    bool isSaved();
    int isDefintionSaved() const;
    int isExpireTimeSaved() const;

    void saveToDatabase() const;
    bool getFromDatabase();

    static void createDatabaseTables();
    static bool isInDatabase(const QString &spelling);
    static sptr<Word> getWordFromDatabase(const QString &spelling);
    static QVector<sptr<Word>> getNewWords(int number);

private:
    bool m_new;
    QString m_spelling;
    QString m_definition;
    QDateTime m_expireTime;

private:
    static const QDateTime m_baselineTime;

    QDateTime defaultExpireTime()
    {
        // default expire time is after 100 years
        return m_baselineTime.addYears(100);
    }

    int getIntExpireTime() const
    {
        return m_baselineTime.secsTo(m_expireTime);
    }

    QDateTime getDatetimeExpireTime(int seconds) const
    {
        return m_baselineTime.addSecs(seconds);
    }
};

#endif // WORD_H
