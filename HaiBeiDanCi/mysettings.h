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

    static void restoreDataSettings();
    static void restoreCardSettings();

    static void saveCardMaximumInterval(int days);
    static int cardMaximumInterval();
    static int cardMaximumIntervalInMinutes();

    static void saveCardDefaultInterval(int days);
    static int cardDefaultInterval();
    static int cardDefaultIntervalInMinutes();

    static void saveCardIntervalForIncorrect(int minutes);
    static int cardIntervalForIncorrect();

    static void saveDefaultEasiness(float easiness);
    static float defaultEasiness();

    static void savePerfectIncrease(float increase);
    static float perfectIncrease();

    static void saveCorrectIncrease(float increase);
    static float correctIncrease();

    static void saveVagueDecrease(float deccrease);
    static float vagueDecrease();
    static float vagueIncrease();

    static void saveIncorrectDecrease(float deccrease);
    static float incorrectDecrease();
    static float incorrectIncrease();

    static MySettings *instance();

    static QString mediaHttpUrl();
    static QString infoFileHttpUrl();
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
    static QString getSettingString(QString key);
};

#endif // MYSETTINGS_H
