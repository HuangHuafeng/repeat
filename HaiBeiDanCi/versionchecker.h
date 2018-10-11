#ifndef VERSIONCHECKER_H
#define VERSIONCHECKER_H

#include "servercommunicator.h"

#include <QObject>

class VersionChecker : public QObject
{
    Q_OBJECT

public:
    explicit VersionChecker(ServerCommunicator *sc = nullptr, QObject *parent = nullptr);

    void checkUpgrader(QString platform);
    void checkApp(QString platform);

signals:
    void appVersion(ReleaseInfo appReleaseInfo, ReleaseInfo appLibReleaseInfo);
    void upgraderVersion(ReleaseInfo upgraderReleaseInfo, ReleaseInfo upgraderLibReleaseInfo);

private slots:
    void onAppVersion(ReleaseInfo appReleaseInfo, ReleaseInfo appLibReleaseInfo);
    void onUpgraderVersion(ReleaseInfo upgraderReleaseInfo, ReleaseInfo upgraderLibReleaseInfo);

private:
    ServerCommunicator *m_sc;
};

#endif // VERSIONCHECKER_H
