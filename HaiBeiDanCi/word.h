#ifndef WORD_H
#define WORD_H

#include "../golddict/sptr.hh"

#include <QString>
#include <QDateTime>
#include <QSqlQuery>
#include <QVector>
#include <QMutex>

class Word
{
public:
    const QString & getSpelling() const
    {
        return m_spelling;
    }

    int getId();
    const QString & getDefinition();
    QString getDefinitionDIV();

    static bool createDatabaseTables();
    static void readAllWordsFromDatabase();

    static QList<QString> getAllWords();
    static sptr<Word> getWord(const QString &spelling, bool create = false);
    static int getWordId(const QString &spelling);
    static bool isInDatabase(const QString &spelling);

private:
    Word(QString word, QString definition = "", int id = 0);

    bool dbsaveDefinition();

private:
    QString m_spelling;
    QString m_definition;
    int m_id;   // id in database

    static QMap<QString, sptr<Word>> m_words;
    static QMutex m_wordsMutex;
};

#endif // WORD_H
