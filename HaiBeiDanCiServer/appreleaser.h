#ifndef APPRELEASER_H
#define APPRELEASER_H

#include "../HaiBeiDanCi/serverclientprotocol.h"

#include <QString>
#include <QDateTime>
#include <QMap>

class AppReleaser
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

    static AppReleaser * instance();

    ReleaseInfo currentVersion(QString platform) const
    {
        return m_currentVersion.value(platform);
    }

    bool releaseNewVersion(ApplicationVersion version, QString platform, QString fileName, QString info);

private:
    AppReleaser();

    static AppReleaser *m_ar;
    QMap<QString, ReleaseInfo> m_currentVersion;

    static bool createDatabaseTables();
    void initialize();
    bool loadLatestVersionFromDatabase();
};

#endif // APPRELEASER_H
