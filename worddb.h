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

private:
    bool connectDB();

    QSqlDatabase m_db;

};

#endif // WORDDB_H
