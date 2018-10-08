#ifndef RELEASEMANAGER_H
#define RELEASEMANAGER_H

#include "../HaiBeiDanCi/serverclientprotocol.h"

#include <QString>
#include <QDateTime>
#include <QMap>

class ReleaseManager
{
public:
    static ReleaseManager * instance();

    ReleaseInfo currentVersion(QString object, QString platform) const
    {
        return m_currentVersion.value(object).value(platform);
    }

    bool releaseObject(QString object, ApplicationVersion version, QString platform, QString fileName, QString info);

private:
    ReleaseManager();

    static ReleaseManager *m_rm;
    QMap<QString, QMap<QString, ReleaseInfo>> m_currentVersion;

    static bool createDatabaseTables();
    void initialize();

    bool loadDataFromDatabase();

    void updateCurrentVersion(ReleaseInfo nri)
    {
        QMap<QString, ReleaseInfo> tm = m_currentVersion.value(nri.object);
        tm.insert(nri.platform, nri);
        m_currentVersion.insert(nri.object, tm);
    }
};

#endif // RELEASEMANAGER_H
