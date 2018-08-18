#include "worddb.h"
#include "word.h"
#include "wordcard.h"
#include "wordbook.h"
#include "mysettings.h"

#include <QCoreApplication>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QVariant>
#include <QThread>
#include <QMutex>

QMutex WordDB::m_mapConnMutex;
QMap<QThread *, sptr<QSqlDatabase>> WordDB::m_mapConns;

WordDB::WordDB()
{
}

WordDB::~WordDB()
{
}

bool WordDB::initialize()
{
    if (false == WordDB::prepareDatabaseForThisThread())
    {
        return false;
    }

    Word::readAllWordsFromDatabase();
    WordCard::readAllCardsFromDatabase();
    WordBook::readAllBooksFromDatabase();

    return true;
}

void WordDB::shutdown()
{
    m_mapConnMutex.lock();
    m_mapConns.clear();
    m_mapConnMutex.unlock();
}

void WordDB::rememberDatabase(sptr<QSqlDatabase> database)
{
    if (database.get() == nullptr)
    {
        return;
    }

    auto existDatabase = WordDB::connectedDatabase();
    if (existDatabase.get() != nullptr)
    {
        return;
    }

    auto ptrThread = QThread::currentThread();
    addConn(ptrThread, database);
}

void WordDB::addConn(QThread *ptrThread, sptr<QSqlDatabase> database)
{
    m_mapConnMutex.lock();
    m_mapConns.insert(ptrThread, database);
    m_mapConnMutex.unlock();
}

sptr<QSqlDatabase> WordDB::getConn(QThread *ptrThread)
{
    m_mapConnMutex.lock();
    auto database = m_mapConns.value(ptrThread);
    m_mapConnMutex.unlock();

    return database;
}

// static
bool WordDB::prepareDatabaseForThisThread()
{
    static QMutex m;
    static int nthAutoCreated = 0;
    auto database = WordDB::connectedDatabase();

    if (database.get() == nullptr)
    {
        auto pid = QCoreApplication::applicationPid();
        // this thread does NOT have a database connection yet, create one for it
        m.lock();
        nthAutoCreated++;
        QString dbConnName = "AutoCreateInProcess_" + QString::number(pid) + "_ForThread_" + QString::number(nthAutoCreated);
        m.unlock();

        return WordDB::connectDB(dbConnName);
    }

    return true;
}

// static
bool WordDB::connectDB(const QString &connectionName)
{
    auto database = WordDB::connectedDatabase();
    if (database.get() && database->isOpen())
    {
        return true;
    }

    QString dbFileName = MySettings::dataDirectory() + "/words.db";
    QSqlDatabase tempDatabase = QSqlDatabase::addDatabase("QSQLITE", connectionName);
    sptr<QSqlDatabase> newDatabase = new QSqlDatabase(tempDatabase);
    WordDB::rememberDatabase(newDatabase);

    newDatabase->setDatabaseName(dbFileName);
    if (!newDatabase->open())
    {
        qCritical("Unable to open database file!");
        return false;
    }

    if (Word::createDatabaseTables() == false)
    {
        return false;
    }

    if (WordCard::createDatabaseTables() == false)
    {
        return false;
    }

    if (WordBook::createDatabaseTables() == false)
    {
        return false;
    }

    return true;
}

// static
void WordDB::databaseError(QSqlQuery &query, const QString what)
{
    QSqlError error = query.lastError();
    QString errorText = QObject::tr("Database error when ") + what + ": " + error.text();
    qCritical("%s", errorText.toUtf8().constData());
}

// static
sptr<QSqlQuery> WordDB::createSqlQuery()
{
    sptr<QSqlQuery> query = sptr<QSqlQuery>();
    auto database = WordDB::connectedDatabase();
    if (database.get() != nullptr)
    {
        query = new QSqlQuery(*database);
    }

    return query;
}

// static
sptr<QSqlDatabase> WordDB::connectedDatabase()
{
    auto ptrThread = QThread::currentThread();
    auto database = getConn(ptrThread);

    return database;
}
