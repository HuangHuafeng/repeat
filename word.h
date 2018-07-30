#ifndef WORD_H
#define WORD_H

#include "golddict/sptr.hh"

#include <QString>
#include <QDateTime>
#include <QSqlQuery>
#include <QVector>

class Word
{
public:
    Word(QString word);

    bool isNew() const
    {
        return m_new;
    }

    void setDefinition(const QString &definition);

    const QString & getSpelling() const
    {
        return m_spelling;
    }

    const QString & getDefinition() const
    {
        return m_definition;
    }

    bool isSaved();
    int isDefintionSaved() const;
    int hasExpireTimeRecord() const;

    void getFromDatabase();
    int getId();

    static bool createDatabaseTables();
    static bool isInDatabase(const QString &spelling);
    static sptr<Word> getWordFromDatabase(const QString &spelling);
    static QVector<QString> getNewWords(int number = 0);
    static QVector<QString> getWords(int number = 0);
    static QVector<QString> getExpiredWords(int number);

private:
    static int getWordId(const QString &spelling);

    void dbsaveDefinition();
    void dbgetDefinition();

private:
    bool m_new;
    int m_id;   // id in database
    QString m_spelling;
    QString m_definition;
};

#endif // WORD_H
