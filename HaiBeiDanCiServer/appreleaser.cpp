#include "appreleaser.h"
#include "../HaiBeiDanCi/worddb.h"

AppReleaser * AppReleaser::m_ar;

AppReleaser::AppReleaser()
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

bool AppReleaser::releaseNewVersion(ApplicationVersion version, QString platform, QString fileName, QString info)
{
    if (version.toInt() <= m_currentVersion.value(platform).version.toInt())
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
    query.prepare("INSERT INTO apps(version, platform, file, info, release_date)"
                  " VALUES(:version, :platform, :file, :info, :release_date)");
    query.bindValue(":version", version.toString());
    query.bindValue(":platform", platform);
    query.bindValue(":file", fileName);
    query.bindValue(":info", info);
    query.bindValue(":release_date", now.toString());
    if (query.exec() == false)
    {
        WordDB::databaseError(query, "saving new released version");
        return false;
    }

    ReleaseInfo nri;
    nri.version = version;
    nri.platform = platform;
    nri.fileName = fileName;
    nri.info = info;
    nri.releaseTime = now;
    m_currentVersion.insert(nri.platform, nri);

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
    query.prepare("SELECT * FROM apps GROUP BY platform;");
    if (query.exec())
    {
        while (query.next())
        {

            ReleaseInfo nri;
            nri.version = ApplicationVersion::fromString(query.value("version").toString());
            nri.platform = query.value("platform").toString();
            nri.fileName = query.value("file").toString();
            nri.info = query.value("info").toString();
            nri.releaseTime = QDateTime::fromString(query.value("release_date").toString());
            m_currentVersion.insert(nri.platform, nri);
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
                       "platform TEXT, "
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
