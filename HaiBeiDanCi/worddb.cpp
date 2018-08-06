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

    auto existDatabase = WordDB::getDatabase();
    if (existDatabase.get() != nullptr) {
        return;
    }

    QObject obj;
    auto ptrThread = obj.thread();
    m_mapConns.insert(ptrThread, database);

    gdDebug("%s", database->connectionName().toStdString().c_str());
}

void WordDB::clearDatabase()
{
    QObject obj;
    auto ptrThread = obj.thread();
    QMap<QThread *, sptr<QSqlDatabase>>::iterator  it = m_mapConns.find(ptrThread);
    m_mapConns.erase(it);
}

bool WordDB::connectDB(const QString &connectionName)
{
    auto database = WordDB::getDatabase();
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
    rememberDatabase(newDatabase);

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
    auto database = WordDB::getDatabase();
    sptr<QSqlQuery> query = new QSqlQuery(*database);

    return query;
}

// static
sptr<QSqlDatabase> WordDB::getDatabase()
{
    QObject obj;
    auto ptrThread = obj.thread();
    auto database = m_mapConns.value(ptrThread);

    return database;
}

