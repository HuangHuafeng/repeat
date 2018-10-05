#ifndef UPGRADERRELEASER_H
#define UPGRADERRELEASER_H

#include "../HaiBeiDanCi/serverclientprotocol.h"

#include <QString>
#include <QDateTime>
#include <QMap>

class UpgraderReleaser
{
public:
    class ReleaseInfo {
    public:
        ApplicationVersion version;
        QString platform;
        QString fileName;
        QString info;
        QDateTime releaseTime;

        ReleaseInfo() : version(0, 0, 0)
        {
        }
    };

    static UpgraderReleaser * instance();

    ReleaseInfo currentVersion(QString platform) const
    {
        return m_currentVersion.value(platform);
    }

    bool releaseNewVersion(ApplicationVersion version, QString platform, QString fileName, QString info);

private:
    UpgraderReleaser();

    static UpgraderReleaser *m_ur;
    QMap<QString, ReleaseInfo> m_currentVersion;

    static bool createDatabaseTables();
    void initialize();
    bool loadLatestVersionFromDatabase();
};

#endif // UPGRADERRELEASER_H
