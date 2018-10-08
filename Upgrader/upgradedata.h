#ifndef UPGRADEDATA_H
#define UPGRADEDATA_H

#include "../HaiBeiDanCi/applicationversion.h"

class UpgradeData
{
public:
    UpgradeData(QString target = "UpgraderTarget");

    bool startUpgrader(const QStringList &arguments);
    bool startTarget();

    void setTarget(QString target);
    QString getTarget();

    bool hasUpgradeData();
    void saveUpgraderFilePath(QString ufp);
    QString upgraderFilePath();

    bool saveUpgradeData(const ApplicationVersion &version, QStringList zipFiles, QString extractDir);
    bool getUpgradeData(ApplicationVersion &version, QStringList &zipFiles, QString &extractDir);

    void saveTargetStartCommand(QString command);
    QString targetStartCommand();

    void saveTargetRunningFile(QString fileName);
    QString targetRunningFile();

    void saveDataDirectory(QString newDir);
    QString dataDirectory();

private:
    const QString dataFile();
    QString m_target;
};

#endif // UPGRADEDATA_H
