#include "worddb.h"
#include "../golddict/gddebug.hh"
#include "word.h"
#include "wordcard.h"
#include "wordbook.h"

#include <QCoreApplication>
#include <QMessageBox>
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
    // only clear my database
    clearDatabase();
}

void WordDB::rememberDatabase(sptr<QSqlDatabase> database)
{
    if (database.get() == nullptr) {
        return;
    }

    auto existDatabase = WordDB::connectedDatabase();
    if (existDatabase.get() != nullptr) {
        return;
    }

    //QObject obj;
    //auto ptrThread = obj.thread();
    auto ptrThread = QThread::currentThread();
    addConn(ptrThread, database);
}


void WordDB::addConn(QThread *ptrThread, sptr<QSqlDatabase> database)
{
    m_mapConnMutex.lock();
    m_mapConns.insert(ptrThread, database);
    gdDebug("m_mapConns has %d elements in addConn()", m_mapConns.size());
    m_mapConnMutex.unlock();
}

bool WordDB::removeConn(QThread *ptrThread)
{
    bool retVal = false;
    m_mapConnMutex.lock();
    QMap<QThread *, sptr<QSqlDatabase>>::iterator  it = m_mapConns.find(ptrThread);
    int count = 0;
    while (it != m_mapConns.end() && it.key() == ptrThread) {
        it ++;
        count ++;
    }
    gdDebug("Found %d connections.", count);
    if (count != 1) {
        // NOT expected! We should have one and ONLY ONE connection for this thread
        gdDebug("something wrong! We should have one and ONLY ONE connection for this thread");
    } else {
        QMap<QThread *, sptr<QSqlDatabase>>::iterator  itAgain = m_mapConns.find(ptrThread);
        m_mapConns.erase(itAgain);
        gdDebug("Good! we deleted the only one connection for this thread!");
        retVal = true;
    }
    m_mapConnMutex.unlock();

    return retVal;
}

sptr<QSqlDatabase> WordDB::getConn(QThread *ptrThread)
{
    m_mapConnMutex.lock();
    auto database = m_mapConns.value(ptrThread);
    m_mapConnMutex.unlock();

    return database;
}

void WordDB::clearDatabase()
{
    auto ptrThread = QThread::currentThread();
    removeConn(ptrThread);
}

// static
bool WordDB::removeDatabaseForThisThread()
{
    auto ptrThread = QThread::currentThread();
    return removeConn(ptrThread);
}

// static
bool WordDB::prepareDatabaseForThisThread()
{
    static QMutex m;
    static int nthAutoCreated = 0;
    auto database = WordDB::connectedDatabase();

    if (database.get() == nullptr) {
        auto pid = QCoreApplication::applicationPid();
        // this thread does NOT have a database connection yet, create one for it
        m.lock();
        nthAutoCreated ++;
        QString dbConnName = "AutoCreateInProcess_" + QString::number(pid)
                + "_ForThread_" + QString::number(nthAutoCreated);
        m.unlock();
        gdDebug("%s", dbConnName.toStdString().c_str());

        if (WordDB::connectDB(dbConnName)) {
        }
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

    QSqlDatabase tempDatabase;
    QString dbFileName = "/Users/huafeng/Documents/GitHub/TextFinder/build-Repeat-Desktop_Qt_5_11_1_clang_64bit-Debug/Repeat.app/Contents/MacOS/words.db";
    //QString dbFileName = QCoreApplication::applicationDirPath() + "/words.db";
    if (connectionName == "") {
        tempDatabase = QSqlDatabase::addDatabase("QSQLITE");
    } else {
        tempDatabase = QSqlDatabase::addDatabase("QSQLITE", connectionName);
    }
    sptr<QSqlDatabase> newDatabase = new QSqlDatabase(tempDatabase);
    WordDB::rememberDatabase(newDatabase);

    newDatabase->setDatabaseName(dbFileName);
    if (!newDatabase->open()) {
        QMessageBox::critical(nullptr, QObject::tr("Database Error"),
                              QObject::tr("Unable to open database file!\n"
                                          "Click OK to exit."));
        return false;
    }

    if (Word::createDatabaseTables() == false) {
        return false;
    }

    if (WordCard::createDatabaseTables() == false) {
        return false;
    }

    if (WordBook::createDatabaseTables() == false) {
        return false;
    }

    return true;
}


// static
void WordDB::databaseError(QSqlQuery &query, const QString what)
{
    QSqlError error = query.lastError();
//    QMessageBox::critical(nullptr, QObject::tr("Database Error"),
//        QObject::tr("Database error when ") + what + ": " + error.text(), QMessageBox::Ok);
    QString errorText = QObject::tr("Database error when ") + what + ": " + error.text();
    gdDebug("%s", errorText.toStdString().c_str());
}

// static
sptr<QSqlQuery> WordDB::createSqlQuery()
{
    sptr<QSqlQuery> query = sptr<QSqlQuery>();
    auto database = WordDB::connectedDatabase();
    if (database.get() != nullptr) {
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


