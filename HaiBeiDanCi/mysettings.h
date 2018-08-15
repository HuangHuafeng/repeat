#ifndef MYSETTINGS_H
#define MYSETTINGS_H

#include <QString>
#include <QObject>

class MySettings
{
  public:
    MySettings();

    static QString orgName()
    {
        return "AniujSoft";
    }

    static QString orgDomain()
    {
        return "aniujsoft.com";
    }

    static QString appName()
    {
        return QObject::tr("HaiBeiDanCi");
    }

    static void saveDataDirectory(QString newDir);
    static QString dataDirectory();

private:
    static const QString m_group;
};

#endif // MYSETTINGS_H
