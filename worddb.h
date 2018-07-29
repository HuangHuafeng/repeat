#ifndef WORDDB_H
#define WORDDB_H

#include <QSqlDatabase>
#include <QVector>
#include <QDateTime>

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
