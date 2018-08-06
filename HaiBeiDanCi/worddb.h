#ifndef WORDDB_H
#define WORDDB_H

#include "../golddict/sptr.hh"

#include <QVector>
#include <QDateTime>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QVariant>
#include <QMap>

class WordDB
{
public:
    WordDB();
    virtual ~WordDB();

    bool connectDB(const QString &connectionName = "");

    static void databaseError(QSqlQuery &query, const QString what);
    static sptr<QSqlDatabase> getDatabase();
    static sptr<QSqlQuery> createSqlQuery();

private:
    static QMap<QThread *, sptr<QSqlDatabase>> m_mapConns;

    void rememberDatabase(sptr<QSqlDatabase> database);
    void clearDatabase();

};

#endif // WORDDB_H
