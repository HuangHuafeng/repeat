#ifndef WORD_H
#define WORD_H

#include "databaseobject.h"
#include "../golddict/sptr.hh"

#include <QString>
#include <QDateTime>
#include <QSqlQuery>
#include <QVector>

class Word : public DatabaseObject
{
public:
    Word(QString word);

    void setDefinition(const QString &definition);

    const QString & getSpelling() const
    {
        return m_spelling;
    }

    int isDefintionSaved() const;

    int getId();
    const QString & getDefinition();

    static bool createDatabaseTables();
    static bool isInDatabase(const QString &spelling);
    static sptr<Word> getWordFromDatabase(const QString &spelling);
    static QVector<QString> getNewWords(int number = 0);
    static QVector<QString> getWords(int number = 0);
    static QVector<QString> getExpiredWords(int number);
    static int getWordId(const QString &spelling);

protected:
    virtual void updateFromDatabase() override;

private:
    void dbsaveDefinition();
    void dbgetDefinition();

private:
    int m_id;   // id in database
    QString m_spelling;
    QString m_definition;
};

#endif // WORD_H
