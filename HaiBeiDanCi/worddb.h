#ifndef WORDDB_H
#define WORDDB_H

#include <QVector>
#include <QDateTime>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QVariant>

class WordDB
{
public:
    WordDB();
    ~WordDB();

    bool connectDB();
    static void databaseError(QSqlQuery &query, const QString what);

private:
    QSqlDatabase m_db;

};

#endif // WORDDB_H
