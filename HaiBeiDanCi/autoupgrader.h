#ifndef AUTOUPGRADER_H
#define AUTOUPGRADER_H

#include "applicationversion.h"

#include <QString>

class AutoUpgrader
{
public:
    AutoUpgrader();
    ~AutoUpgrader();

    bool startUpgrader(QStringList arguments = QStringList());
    void newAppDownloaded(ApplicationVersion version, QStringList zipFiles);
    void newUpgraderDownloaded(ApplicationVersion version, QString fileName);
    ApplicationVersion upgraderVersion();
    QString hardcodedUpgraderFilePath();

private:
    QString runningFile();
    void createRunningFile();
    void removeRunningFile();
};

#endif // AUTOUPGRADER_H
