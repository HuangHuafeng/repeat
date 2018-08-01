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

WordDB::WordDB()
{
}

WordDB::~WordDB()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
}

bool WordDB::connectDB()
{
    if (m_db.isOpen()) {
        return true;
    }

    QString dbFileName = "/Users/huafeng/Documents/GitHub/TextFinder/build-Repeat-Desktop_Qt_5_11_1_clang_64bit-Debug/Repeat.app/Contents/MacOS/words.db";
    //QString dbFileName = QCoreApplication::applicationDirPath() + "/words.db";
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(dbFileName);
    if (!m_db.open()) {
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
    QMessageBox::critical(nullptr, QObject::tr("Database Error"),
        QObject::tr("Database error when ") + what + ": " + error.text(), QMessageBox::Ok);
}
