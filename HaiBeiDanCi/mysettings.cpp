#include "mysettings.h"

#include <QSettings>
#include <QCoreApplication>

const QString MySettings::m_group = "Preferences";

MySettings::MySettings()
{
}

void MySettings::saveDataDirectory(QString newDir)
{
    QSettings settings;
    settings.beginGroup(m_group);
    settings.setValue("dataDirectory", newDir);
    settings.endGroup();
}

QString MySettings::dataDirectory()
{
    QSettings settings;
    settings.beginGroup(m_group);
    QString dd = settings.value("dataDirectory", "").toString();
    settings.endGroup();

    if (dd.isEmpty())
    {
        dd = QCoreApplication::applicationDirPath();
    }

    return dd;
}
