#ifndef APPRELEASER_H
#define APPRELEASER_H

#include "../HaiBeiDanCi/serverclientprotocol.h"

#include <QString>
#include <QDateTime>

class AppReleaser
{
public:
    static AppReleaser * instance();

    const ApplicationVersion & currentVersion() const
    {
        return m_currentVersion;
    }

    const QString & fileName() const
    {
        return m_fileName;
    }

    const QString & info() const
    {
        return m_info;
    }

    bool releaseNewVersion(ApplicationVersion version, QString fileName, QString info);

private:
    AppReleaser();

    static AppReleaser *m_ar;

    ApplicationVersion m_currentVersion;
    QString m_fileName;
    QString m_info;
    QDateTime m_releaseTime;

    static bool createDatabaseTables();
    void initialize();
    bool loadLatestVersionFromDatabase();
};

#endif // APPRELEASER_H
