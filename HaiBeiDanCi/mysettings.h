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
    static QString m_appName;

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

    static void setAppName(QString appName)
    {
        m_appName = appName;
    }

    static QString appName()
    {
        return m_appName;
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
    static float incorrectIncrease();

    static int heartbeatIntervalInSeconds();
    static int maximumConsecutiveHeartbeat();
    static int maximumReadTimeout();
    static int downloadIntervalInMilliseconds();
    static int numberOfRequestInEveryDownloadRound();

#ifndef HAIBEIDANCI_SERVER

    static QString applicationFont();
    static void saveApplicationFont(QFont font);

#endif  // HAIBEIDANCI_SERVER

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
