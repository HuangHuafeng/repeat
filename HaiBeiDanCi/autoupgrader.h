#ifndef AUTOUPGRADER_H
#define AUTOUPGRADER_H

#include "applicationversion.h"

#include <QString>

class AutoUpgrader
{
public:
    AutoUpgrader();
    ~AutoUpgrader();

    bool startUpgrader();
    void newVersionAvailable(ApplicationVersion version, QString fileName);

private:
    QString runningFile();
    void createRunningFile();
    void removeRunningFile();
};

#endif // AUTOUPGRADER_H
