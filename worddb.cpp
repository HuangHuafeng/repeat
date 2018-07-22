#include "worddb.h"
#include "golddict/gddebug.hh"
#include "word.h"

#include <QCoreApplication>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QVariant>

WordDB::WordDB()
{
    connectDB();
    Word::createDatabaseTables();
}

WordDB::~WordDB()
{
    m_db.close();
}

bool WordDB::connectDB()
{
    QString dbFileName = QCoreApplication::applicationDirPath() + "/words.db";
    gdDebug("database file is %s", dbFileName.toStdString().c_str());
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(dbFileName);
    if (!m_db.open()) {
        QMessageBox::critical(nullptr, QObject::tr("Cannot open database"),
            QObject::tr("Unable to establish a database connection.\n"
                        "This example needs SQLite support. Please read "
                        "the Qt SQL driver documentation for information how "
                        "to build it.\n\n"
                        "Click Cancel to exit."), QMessageBox::Cancel);
        return false;
    }

    return true;
}

