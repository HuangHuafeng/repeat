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

    QString m_group;
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
    static MySettings * getSettings();

    QString mediaHttpUrl();

private slots:
    void onFileDownloaded(QString fileName);

private:
    MySettings();

    void loadInfoFromGitHub();
    static QString group();
};

#endif // MYSETTINGS_H
