#include "releasemanager.h"
#include "../HaiBeiDanCi/worddb.h"

ReleaseManager * ReleaseManager::m_rm;

ReleaseManager::ReleaseManager()
{
}

ReleaseManager * ReleaseManager::instance()
{
    if (m_rm == nullptr)
    {
        m_rm = new ReleaseManager;
        m_rm->initialize();
    }

    return m_rm;
}

bool ReleaseManager::releaseObject(QString object, ApplicationVersion version, QString platform, QString fileName, QString info)
{
    auto ptrQuery = WordDB::createSqlQuery();
    if (ptrQuery.get() == nullptr)
    {
        return false;
    }
    auto query = *ptrQuery;
    QDateTime now = QDateTime::currentDateTime();
    query.prepare("INSERT INTO releaseobjects(object, version, platform, file, info, release_date)"
                  " VALUES(:object, :version, :platform, :file, :info, :release_date)");
    query.bindValue(":object", object);
    query.bindValue(":version", version.toString());
    query.bindValue(":platform", platform);
    query.bindValue(":file", fileName);
    query.bindValue(":info", info);
    query.bindValue(":release_date", now.toString());
    if (query.exec() == false)
    {
        WordDB::databaseError(query, "saving information to table appreleases");
        return false;
    }

    ReleaseInfo nri;
    nri.object = object;
    nri.version = version;
    nri.platform = platform;
    nri.fileName = fileName;
    nri.info = info;
    nri.releaseTime = now;
    updateCurrentVersion(nri);

    return true;
}

void ReleaseManager::initialize()
{
    if (ReleaseManager::createDatabaseTables() == false)
    {
        return;
    }

    if (loadDataFromDatabase() == false)
    {
        return;
    }
}

bool ReleaseManager::loadDataFromDatabase()
{
    auto ptrQuery = WordDB::createSqlQuery();
    if (ptrQuery.get() == nullptr)
    {
        return false;
    }
    auto query = *ptrQuery;
    bool retVal = true;
    query.prepare("SELECT * FROM releaseobjects GROUP BY object, platform;");
    if (query.exec())
    {
        while (query.next())
        {
            ReleaseInfo nri;
            nri.object = query.value("object").toString();
            nri.version = ApplicationVersion::fromString(query.value("version").toString());
            nri.platform = query.value("platform").toString();
            nri.fileName = query.value("file").toString();
            nri.info = query.value("info").toString();
            nri.releaseTime = QDateTime::fromString(query.value("release_date").toString());
            updateCurrentVersion(nri);
        }
    }
    else
    {
        WordDB::databaseError(query, "loading information from table releaseobjects");
        retVal = false;
    }

    return retVal;
}

// static
bool ReleaseManager::createDatabaseTables()
{
    auto ptrQuery = WordDB::createSqlQuery();
    if (ptrQuery.get() == nullptr)
    {
        return false;
    }
    auto query = *ptrQuery;
    if (query.exec("SELECT * FROM releaseobjects LIMIT 1") == false)
    {
        // table "releaseobjects" does not exist
        if (query.exec("CREATE TABLE releaseobjects (id INTEGER primary key, "
                       "object TEXT,"
                       "version TEXT, "
                       "platform TEXT, "
                       "file TEXT, "
                       "info TEXT, "
                       "release_date TEXT)") == false)
        {
            WordDB::databaseError(query, "creating table \"releaseobjects\"");
            return false;
        }
    }

    return true;
}
