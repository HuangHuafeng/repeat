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
    virtual ~WordDB();

    static bool initialize();
    static bool prepareDatabaseForThisThread();
    static sptr<QSqlQuery> createSqlQuery();
    static void databaseError(QSqlQuery &query, QString what);
    static sptr<QSqlDatabase> connectedDatabase();
    static void removeMyConnection();
    static QString status();

  private:
    WordDB();

    static bool connectDB(const QString &connectionName);
    static void addConn(QThread *ptrThread, sptr<QSqlDatabase> database);
    static sptr<QSqlDatabase> getConn(QThread *ptrThread);
    static void rememberDatabase(sptr<QSqlDatabase> database);
    static void forgetMyDatabae();

  private:
    static QMutex m_mapConnMutex;
    static QMap<QThread *, sptr<QSqlDatabase>> m_mapConns;
};

#endif // WORDDB_H
