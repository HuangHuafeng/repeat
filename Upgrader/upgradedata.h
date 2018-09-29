#ifndef UPGRADEDATA_H
#define UPGRADEDATA_H

#include "../HaiBeiDanCi/applicationversion.h"

class UpgradeData
{
public:
    UpgradeData();

    static bool hasUpgradeData();
    static bool saveUpgradeData(const ApplicationVersion &version, QString zipFile, QString extractDir);
    static bool getUpgradeData(ApplicationVersion &version, QString &zipFile, QString &extractDir);

    static void saveDataDirectory(QString newDir);
    static QString dataDirectory();

private:
    static const QString dataFile();
};

#endif // UPGRADEDATA_H
