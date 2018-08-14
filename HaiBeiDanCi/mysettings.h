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
};

#endif // MYSETTINGS_H
