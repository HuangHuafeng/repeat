#include "appreleaser.h"
#include "../HaiBeiDanCi/worddb.h"

AppReleaser * AppReleaser::m_ar;

AppReleaser::AppReleaser() :
    m_currentVersion(0, 0, 0),
    m_fileName(),
    m_info()
{

}

AppReleaser * AppReleaser::instance()
{
    if (m_ar == nullptr)
    {
        m_ar = new AppReleaser;
        m_ar->initialize();
    }

    return m_ar;
}

bool AppReleaser::releaseNewVersion(ApplicationVersion version, QString fileName, QString info)
{
    if (version.toInt() <= m_currentVersion.toInt())
    {
        return false;
    }

    auto ptrQuery = WordDB::createSqlQuery();
    if (ptrQuery.get() == nullptr)
    {
        return false;
    }
    auto query = *ptrQuery;
    QDateTime now = QDateTime::currentDateTime();
    query.prepare("INSERT INTO apps(version, file, info, release_date)"
                  " VALUES(:version, :file, :info, :release_date)");
    query.bindValue(":version", version.toString());
    query.bindValue(":file", fileName);
    query.bindValue(":info", info);
    query.bindValue(":release_date", now.toString());
    if (query.exec() == false)
    {
        WordDB::databaseError(query, "saving new released version");
        return false;
    }

    m_currentVersion = version;
    m_fileName = fileName;
    m_info = info;
    m_releaseTime = now;

    return true;
}

void AppReleaser::initialize()
{
    if (AppReleaser::createDatabaseTables() == false)
    {
        return;
    }

    if (loadLatestVersionFromDatabase() == false)
    {
        return;
    }
}

bool AppReleaser::loadLatestVersionFromDatabase()
{
    auto ptrQuery = WordDB::createSqlQuery();
    if (ptrQuery.get() == nullptr)
    {
        return false;
    }
    auto query = *ptrQuery;
    bool retVal = true;
    query.prepare("SELECT * FROM apps ORDER BY id DESC LIMIT 1;");
    if (query.exec())
    {
        if (query.first())
        {
            m_currentVersion = ApplicationVersion::fromString(query.value("version").toString());
            m_fileName = query.value("file").toString();
            m_info = query.value("info").toString();
            m_releaseTime = QDateTime::fromString(query.value("release_date").toString());
        }
    }
    else
    {
        WordDB::databaseError(query, "loading latest released app information");
        retVal = false;
    }

    return retVal;
}

// static
bool AppReleaser::createDatabaseTables()
{
    auto ptrQuery = WordDB::createSqlQuery();
    if (ptrQuery.get() == nullptr)
    {
        return false;
    }
    auto query = *ptrQuery;
    if (query.exec("SELECT * FROM apps LIMIT 1") == false)
    {
        // table "words" does not exist
        if (query.exec("CREATE TABLE apps (id INTEGER primary key, "
                       "version TEXT, "
                       "file TEXT, "
                       "info TEXT, "
                       "release_date TEXT)") == false)
        {
            WordDB::databaseError(query, "creating table \"apps\"");
            return false;
        }
    }

    return true;
}
