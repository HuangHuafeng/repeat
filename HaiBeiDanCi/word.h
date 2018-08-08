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
    QString getDefinitionDIV();

    static bool createDatabaseTables();
    static bool isInDatabase(const QString &spelling);
    static sptr<Word> getWordFromDatabase(const QString &spelling);
    static int getWordId(const QString &spelling);

protected:
    virtual void updateFromDatabase() override;

private:
    void dbsaveDefinition();
    void dbgetDefinition();

private:
    static QMap<QString, sptr<Word>> m_words;
    int m_id;   // id in database
    QString m_spelling;
    QString m_definition;
};

#endif // WORD_H
