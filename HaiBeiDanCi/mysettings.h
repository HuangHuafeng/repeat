#ifndef MYSETTINGS_H
#define MYSETTINGS_H

#include "downloadmanager.h"
#include "../golddict/sptr.hh"

#include <QObject>
#include <QString>
#include <QJsonDocument>

class MySettings : public QObject
{
    Q_OBJECT

    QJsonDocument m_infoFromGithub;
    DownloadManager m_downloadManager;

    static MySettings *m_settings;

  public:
    virtual ~MySettings();

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
    static void saveUpdateInterval(int days);
    static int updateInterval();

    static MySettings *instance();

    QString mediaHttpUrl();
    void updateInfoFileNow();

  private slots:
    void onInfoFileDownloadedFromGithub(QString fileName);

  private:
    MySettings();

    void loadSettingsFromInfoFile();
    void downloadInfoFileFromGitHub(QString saveToFileName);
    void readInfoFile(QString infoFileName);

    static QString group()
    {
        return "Preferences";
    }

    static void saveLastUpdateTime();
    static QDateTime lastUpdateTime();
};

#endif // MYSETTINGS_H
