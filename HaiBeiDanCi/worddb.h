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
#include <QMutex>

class WordDB
{
public:
    WordDB();
    virtual ~WordDB();

    static bool prepareDatabaseForThisThread();
    static bool removeDatabaseForThisThread();
    static bool connectDB(const QString &connectionName = "");

    static void databaseError(QSqlQuery &query, const QString what);
    static sptr<QSqlDatabase> connectedDatabase();
    static sptr<QSqlQuery> createSqlQuery();

private:
    static QMutex m_mapConnMutex;
    static QMap<QThread *, sptr<QSqlDatabase>> m_mapConns;

    static void addConn(QThread *ptrThread, sptr<QSqlDatabase> database);
    static bool removeConn(QThread *ptrThread);
    static sptr<QSqlDatabase> getConn(QThread *ptrThread);

    static void rememberDatabase(sptr<QSqlDatabase> database);
    void clearDatabase();

};

#endif // WORDDB_H
