#include "versionchecker.h"
#include "mysettings.h"

VersionChecker::VersionChecker(ServerCommunicator *sc, QObject *parent) :
    QObject(parent),
    m_sc(sc)
{
    if (m_sc == nullptr)
    {
        m_sc = ServerCommunicator::instance();
    }

    connect(m_sc, SIGNAL(appVersion(ReleaseInfo, ReleaseInfo)), this, SLOT(onAppVersion(ReleaseInfo, ReleaseInfo)));
    connect(m_sc, SIGNAL(upgraderVersion(ReleaseInfo, ReleaseInfo)), this, SLOT(onUpgraderVersion(ReleaseInfo, ReleaseInfo)));
}

void VersionChecker::checkUpgrader(QString platform)
{
    m_sc->sendRequestUpgraderVersion(platform);
}

void VersionChecker::checkApp(QString platform)
{
    m_sc->sendRequestAppVersion(platform);
}

void VersionChecker::onAppVersion(ReleaseInfo appReleaseInfo, ReleaseInfo appLibReleaseInfo)
{
    emit(appVersion(appReleaseInfo, appLibReleaseInfo));
}

void VersionChecker::onUpgraderVersion(ReleaseInfo upgraderReleaseInfo, ReleaseInfo upgraderLibReleaseInfo)
{
    emit(upgraderVersion(upgraderReleaseInfo, upgraderLibReleaseInfo));
}
