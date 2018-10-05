#include "upgraderreleaser.h"
#include "../HaiBeiDanCi/worddb.h"

UpgraderReleaser * UpgraderReleaser::m_ur;

UpgraderReleaser::UpgraderReleaser()
{
}

UpgraderReleaser * UpgraderReleaser::instance()
{
    if (m_ur == nullptr)
    {
        m_ur = new UpgraderReleaser;
        m_ur->initialize();
    }

    return m_ur;
}

bool UpgraderReleaser::releaseNewVersion(ApplicationVersion version, QString platform, QString fileName, QString info)
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
    query.prepare("INSERT INTO upgraders(version, platform, file, info, release_date)"
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

void UpgraderReleaser::initialize()
{
    if (UpgraderReleaser::createDatabaseTables() == false)
    {
        return;
    }

    if (loadLatestVersionFromDatabase() == false)
    {
        return;
    }
}

bool UpgraderReleaser::loadLatestVersionFromDatabase()
{
    auto ptrQuery = WordDB::createSqlQuery();
    if (ptrQuery.get() == nullptr)
    {
        return false;
    }
    auto query = *ptrQuery;
    bool retVal = true;
    query.prepare("SELECT * FROM upgraders GROUP BY platform;");
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
bool UpgraderReleaser::createDatabaseTables()
{
    auto ptrQuery = WordDB::createSqlQuery();
    if (ptrQuery.get() == nullptr)
    {
        return false;
    }
    auto query = *ptrQuery;
    if (query.exec("SELECT * FROM upgraders LIMIT 1") == false)
    {
        // table "words" does not exist
        if (query.exec("CREATE TABLE upgraders (id INTEGER primary key, "
                       "version TEXT, "
                       "platform TEXT, "
                       "file TEXT, "
                       "info TEXT, "
                       "release_date TEXT)") == false)
        {
            WordDB::databaseError(query, "creating table \"upgraders\"");
            return false;
        }
    }

    return true;
}
