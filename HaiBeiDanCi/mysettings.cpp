#include "mysettings.h"

#include <QtDebug>
#include <QDateTime>
#include <QSettings>
#include <QCoreApplication>
#include <QUrl>
#include <QFile>

#ifndef HAIBEIDANCI_SERVER
#include <QFont>

QString MySettings::applicationFont()
{
    QSettings settings;
    settings.beginGroup(MySettings::group());
    QString valueInSetting = settings.value("applicationFont", "").toString();
    settings.endGroup();

    if (valueInSetting.isEmpty() == true)
    {
        // there's no local setting, try to get the setting from info.txt
        QString stringValue = MySettings::getSettingString("applicationFont");
    }

    return valueInSetting;
}

void MySettings::saveApplicationFont(QFont font)
{
    QSettings settings;
    settings.beginGroup(MySettings::group());
    settings.setValue("applicationFont", font);
    settings.endGroup();
}

#endif  // HAIBEIDANCI_SERVER

MySettings *MySettings::m_settings = nullptr;
QString MySettings::m_appName;

MySettings::MySettings()
{
    connect(&m_downloadManager, SIGNAL(fileDownloaded(QString)), this, SLOT(onInfoFileDownloaded(QString)));
    connect(&m_downloadManager, SIGNAL(fileDownloadFailed(QString)), this, SLOT(onInfoFileDownloadFailed(QString)));
}

MySettings::~MySettings()
{
    if (m_settings != nullptr)
    {
        delete m_settings;
    }
}

MySettings *MySettings::instance()
{
    if (m_settings == nullptr)
    {
        m_settings = new MySettings();
        m_settings->loadSettingsFromInfoFile();
    }

    return m_settings;
}

void MySettings::saveServerHostName(QString hostName)
{
    QSettings settings;
    settings.beginGroup(MySettings::group());
    settings.setValue("serverHostName", hostName);
    settings.endGroup();
}

QString MySettings::serverHostName()
{
    QSettings settings;
    settings.beginGroup(MySettings::group());
    QString hostName = settings.value("serverHostName", "").toString();
    settings.endGroup();

    if (hostName.isEmpty())
    {
        // there's no local setting, try to get the setting from info.txt
        QString stringValue = MySettings::getSettingString("serverHostName");
        if (stringValue.isEmpty() == false)
        {
            hostName = stringValue;
        }
        else
        {
            hostName = "localhost";
        }
    }

    return hostName;
}

void MySettings::saveServerPort(quint16 port)
{
    QSettings settings;
    settings.beginGroup(MySettings::group());
    settings.setValue("serverPort", port);
    settings.endGroup();
}

quint16 MySettings::serverPort()
{
    QSettings settings;
    settings.beginGroup(MySettings::group());
    int valueInSetting = settings.value("serverPort", -100).toInt();
    settings.endGroup();

    if (valueInSetting == -100)
    {
        // there's no local setting, try to get the setting from info.txt
        QString stringValue = MySettings::getSettingString("serverPort");
        if (stringValue.isEmpty() == false)
        {
            valueInSetting = stringValue.toInt();
        }
        else
        {
            valueInSetting = 61027;
        }
    }

    return static_cast<quint16>(valueInSetting);
}

void MySettings::saveDataDirectory(QString newDir)
{
    QSettings settings;
    settings.beginGroup(MySettings::group());
    settings.setValue("dataDirectory", newDir);
    settings.endGroup();
}

QString MySettings::dataDirectory()
{
    QSettings settings;
    settings.beginGroup(MySettings::group());
    QString dd = settings.value("dataDirectory", "").toString();
    settings.endGroup();

    if (dd.isEmpty())
    {
        dd = QCoreApplication::applicationDirPath();
    }

    return dd;
}

void MySettings::saveUpdateInterval(int days)
{
    QSettings settings;
    settings.beginGroup(MySettings::group());
    settings.setValue("updateInterval", days);
    settings.endGroup();
}

/**
 * @brief MySettings::updateInterval
 * @return
 * this is going to be called in loadSettingsFromInfoFile()
 * So it should be a pure static method
 */
int MySettings::updateInterval()
{
    QSettings settings;
    settings.beginGroup(MySettings::group());
    int updInt = settings.value("updateInterval", -1).toInt();
    settings.endGroup();

    if (updInt == -1)
    {
        // default to 1 so it's updated every day
        updInt = 1;
    }

    return updInt;
}

void MySettings::saveDefaultEasiness(float easiness)
{
    QSettings settings;
    settings.beginGroup(MySettings::group());
    settings.setValue("defaultEasiness", easiness);
    settings.endGroup();
}

float MySettings::defaultEasiness()
{
    QSettings settings;
    settings.beginGroup(MySettings::group());
    float defaultEasiness = settings.value("defaultEasiness", 0.0f).toFloat();
    settings.endGroup();

    if (defaultEasiness ==  0.0f)
    {
        // there's no local setting, try to get the setting from info.txt
        QString deString = MySettings::getSettingString("defaultEasiness");
        if (deString.isEmpty() == false)
        {
            defaultEasiness = deString.toFloat();
        }
        else
        {
            // default to 2.5f if we cannot find any setting
            defaultEasiness = 2.5f;
        }
    }

    return defaultEasiness;
}

void MySettings::saveCardMaximumInterval(int days)
{
    QSettings settings;
    settings.beginGroup(MySettings::group());
    settings.setValue("cardMaximumInterval", days);
    settings.endGroup();
}

int MySettings::cardMaximumInterval()
{
    QSettings settings;
    settings.beginGroup(MySettings::group());
    int updInt = settings.value("cardMaximumInterval", -1).toInt();
    settings.endGroup();

    if (updInt == -1)
    {
        // there's no local setting, try to get the setting from info.txt
        QString updIntString = MySettings::getSettingString("cardMaximumInterval");
        if (updIntString.isEmpty() == false)
        {
            updInt = updIntString.toInt();
        }
        else
        {
            // default to 10 years if we cannot find any setting
            updInt = 365 * 10;
        }
    }

    return updInt;
}

int MySettings::cardMaximumIntervalInMinutes()
{
    return 60 * 24 * MySettings::cardMaximumInterval();
}

void MySettings::saveCardDefaultInterval(int days)
{
    QSettings settings;
    settings.beginGroup(MySettings::group());
    settings.setValue("cardDefaultInterval", days);
    settings.endGroup();
}

int MySettings::cardDefaultInterval()
{
    QSettings settings;
    settings.beginGroup(MySettings::group());
    int updInt = settings.value("cardDefaultInterval", 0).toInt();
    settings.endGroup();

    if (updInt == 0)
    {
        // there's no local setting, try to get the setting from info.txt
        QString updIntString = MySettings::getSettingString("cardDefaultInterval");
        if (updIntString.isEmpty() == false)
        {
            updInt = updIntString.toInt();
        }
        else
        {
            // default to 1 day if we cannot find any setting
            updInt = 1;
        }
    }

    return updInt;
}

int MySettings::cardDefaultIntervalInMinutes()
{
    return 60 * 24 * MySettings::cardDefaultInterval();
}

void MySettings::saveCardIntervalForIncorrect(int minutes)
{
    QSettings settings;
    settings.beginGroup(MySettings::group());
    settings.setValue("cardIntervalForIncorrect", minutes);
    settings.endGroup();
}

int MySettings::cardIntervalForIncorrect()
{
    QSettings settings;
    settings.beginGroup(MySettings::group());
    int updInt = settings.value("cardIntervalForIncorrect", 0).toInt();
    settings.endGroup();

    if (updInt == 0)
    {
        // there's no local setting, try to get the setting from info.txt
        QString updIntString = MySettings::getSettingString("cardDefaultIntervalForIncorrect");
        if (updIntString.isEmpty() == false)
        {
            updInt = updIntString.toInt();
        }
        else
        {
            // default to 10 minutes if we cannot find any setting
            updInt = 10;
        }
    }

    return updInt;
}

void MySettings::restoreDataSettings()
{
    QSettings settings;
    settings.beginGroup(MySettings::group());
    settings.remove("dataDirectory");
    settings.remove("updateInterval");
    settings.remove("applicationFont");
    settings.endGroup();
}

void MySettings::restoreCardSettings()
{
    QSettings settings;
    settings.beginGroup(MySettings::group());
    settings.remove("defaultEasiness");
    settings.remove("cardDefaultInterval");
    settings.remove("cardMaximumInterval");
    settings.remove("perfectIncrease");
    settings.remove("correctIncrease");
    settings.remove("vagueDecrease");
    settings.remove("cardIntervalForIncorrect");
    settings.endGroup();
}

void MySettings::savePerfectIncrease(float increase)
{
    int valueInt = static_cast<int>(increase * 100);
    QSettings settings;
    settings.beginGroup(MySettings::group());
    settings.setValue("perfectIncrease", valueInt);
    settings.endGroup();
}

float MySettings::perfectIncrease()
{
    QSettings settings;
    settings.beginGroup(MySettings::group());
    int valueInSetting = settings.value("perfectIncrease", -100).toInt();
    settings.endGroup();

    if (valueInSetting == -100)
    {
        // there's no local setting, try to get the setting from info.txt
        QString stringValue = MySettings::getSettingString("perfectIncrease");
        if (stringValue.isEmpty() == false)
        {
            valueInSetting = stringValue.toInt();
        }
        else
        {
            // default to 15% if we cannot find any setting
            valueInSetting = 15;
        }
    }

    return valueInSetting / 100.f;
}

void MySettings::saveCorrectIncrease(float increase)
{
    int valueInt = static_cast<int>(increase * 100);
    QSettings settings;
    settings.beginGroup(MySettings::group());
    settings.setValue("correctIncrease", valueInt);
    settings.endGroup();
}

float MySettings::correctIncrease()
{
    QSettings settings;
    settings.beginGroup(MySettings::group());
    int valueInSetting = settings.value("correctIncrease", -100).toInt();
    settings.endGroup();

    if (valueInSetting == -100)
    {
        // there's no local setting, try to get the setting from info.txt
        QString stringValue = MySettings::getSettingString("correctIncrease");
        if (stringValue.isEmpty() == false)
        {
            valueInSetting = stringValue.toInt();
        }
        else
        {
            // default to 0% if we cannot find any setting
            valueInSetting = 0;
        }
    }

    return valueInSetting / 100.f;
}

/*
void MySettings::saveIncorrectDecrease(float deccrease)
{
    int valueInt = static_cast<int>(deccrease * 100);
    QSettings settings;
    settings.beginGroup(MySettings::group());
    settings.setValue("incorrectDecrease", valueInt);
    settings.endGroup();
}

float MySettings::incorrectDecrease()
{
    QSettings settings;
    settings.beginGroup(MySettings::group());
    int valueInSetting = settings.value("incorrectDecrease", -100).toInt();
    settings.endGroup();

    if (valueInSetting == -100)
    {
        // there's no local setting, try to get the setting from info.txt
        QString stringValue = MySettings::getSettingString("incorrectDecrease");
        if (stringValue.isEmpty() == false)
        {
            valueInSetting = stringValue.toInt();
        }
        else
        {
            // default to 20% if we cannot find any setting
            valueInSetting = 20;
        }
    }

    return valueInSetting / 100.f;
}
*/

/**
 * @brief MySettings::incorrectIncrease
 * @return
 * incorrectDecrease not exposed to user, so we just use vagueDecrease() * 110%
 */
float MySettings::incorrectIncrease()
{
    return MySettings::vagueDecrease() * -1.10f;
}

void MySettings::saveVagueDecrease(float deccrease)
{
    int valueInt = static_cast<int>(deccrease * 100);
    QSettings settings;
    settings.beginGroup(MySettings::group());
    settings.setValue("vagueDecrease", valueInt);
    settings.endGroup();
}

float MySettings::vagueDecrease()
{
    QSettings settings;
    settings.beginGroup(MySettings::group());
    int valueInSetting = settings.value("vagueDecrease", -100).toInt();
    settings.endGroup();

    if (valueInSetting == -100)
    {
        // there's no local setting, try to get the setting from info.txt
        QString stringValue = MySettings::getSettingString("vagueDecrease");
        if (stringValue.isEmpty() == false)
        {
            valueInSetting = stringValue.toInt();
        }
        else
        {
            // default to 15% if we cannot find any setting
            valueInSetting = 15;
        }
    }

    return valueInSetting / 100.f;
}

float MySettings::vagueIncrease()
{
    return MySettings::vagueDecrease() * -1.0f;
}

QString MySettings::mediaHttpUrl()
{
    return MySettings::getSettingString("mediahttp");
}

QString MySettings::infoFileHttpUrl()
{
    QString infoHttp = MySettings::getSettingString("infoFileHttpUrl");
    if (infoHttp.isEmpty() == true)
    {
        //infoHttp = "http://www.huafeng.ga/wp-content/uploads/info.json";
        infoHttp = "https://raw.githubusercontent.com/HuangHuafeng/repeat/master/HaiBeiDanCi/info.json";
    }

    return infoHttp;
}

void MySettings::saveLastUpdateTime()
{
    QString lutSting = QDateTime::currentDateTime().toString(Qt::ISODate);
    QSettings settings;
    settings.beginGroup(MySettings::group());
    settings.setValue("lastUpdateTime", lutSting);
    settings.endGroup();
}

QDateTime MySettings::lastUpdateTime()
{
    QString defaultLastUpdateTime = "2016-10-31T10:00:00+08:00";
    QSettings settings;
    settings.beginGroup(MySettings::group());
    QString lutSting = settings.value("lastUpdateTime", defaultLastUpdateTime).toString();
    settings.endGroup();

    return QDateTime::fromString(lutSting, Qt::ISODate);
}

int MySettings::heartbeatIntervalInSeconds()
{
    QString stringValue = MySettings::getSettingString("heartbeatIntervalInSeconds");
    int valueInSetting;
    if (stringValue.isEmpty() == false)
    {
        valueInSetting = stringValue.toInt();
    }
    else
    {
        // default to 10 seconds
        valueInSetting = 10;
    }

    return valueInSetting;
}

int MySettings::maximumConsecutiveHeartbeat()
{
    QString stringValue = MySettings::getSettingString("maximumConsecutiveHeartbeat");
    int valueInSetting;
    if (stringValue.isEmpty() == false)
    {
        valueInSetting = stringValue.toInt();
    }
    else
    {
        // default to 60 (10 minutes with heartbeat every 10 seconds)
        valueInSetting = 60;
    }

    return valueInSetting;
}

int MySettings::downloadIntervalInMilliseconds()
{
    QString stringValue = MySettings::getSettingString("downloadIntervalInMilliseconds");
    int valueInSetting;
    if (stringValue.isEmpty() == false)
    {
        valueInSetting = stringValue.toInt();
    }
    else
    {
        // default to 100 ms, we would like to send message as fast as possilbe as we have a flow control
        valueInSetting = 100;
    }

    return valueInSetting;
}

int MySettings::numberOfRequestInEveryDownloadRound()
{
    QString stringValue = MySettings::getSettingString("numberOfRequestInEveryDownloadRound");
    int valueInSetting;
    if (stringValue.isEmpty() == false)
    {
        valueInSetting = stringValue.toInt();
    }
    else
    {
        // default to 40 messages every round, should not be big because this can affect cancelling the downloading
        valueInSetting = 40;
    }

    return valueInSetting;
}

void MySettings::downloadInfoFileFromGitHub(QString saveToFileName)
{
    QString urlString = MySettings::infoFileHttpUrl();
    qDebug() << "try to download" << urlString << "and save as" << saveToFileName;
    m_downloadManager.download(urlString, saveToFileName);
}

void MySettings::readInfoFile(QString infoFileName)
{
    QFile infoFile(infoFileName);
    if (!infoFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug("failed to load info file");
        return;
    }

    QByteArray json = infoFile.readAll();
    QJsonParseError jpe;
    m_infoFromGithub = QJsonDocument::fromJson(json, &jpe);
    if (m_infoFromGithub.isNull() == true)
    {
        qDebug() << "failed to load setting from" << infoFileName << "because of" << jpe.errorString();
    }
}

/**
 * @brief MySettings::loadSettingsFromInfoFile
 * We need a stragety to decide when to download info.json from Github!
 * We update the file every "settable" days. Also allow user to update manually in preferences
 */
void MySettings::loadSettingsFromInfoFile()
{
    QString infoFileName = MySettings::dataDirectory() + "/info.txt";
    if (QFile::exists(infoFileName) == true)
    {
        // we should read the settings in the current file, downloading might fail!
        // then if the download succeed, we refresh the settings
        readInfoFile(infoFileName);

        QDateTime lut = MySettings::lastUpdateTime();
        QDateTime curTime = QDateTime::currentDateTime();
        auto pastDays = lut.daysTo(curTime);
        if (pastDays >= MySettings::updateInterval())
        {
            // we haven't update the file for a period longer then the settings
            // so update it
            updateInfoFileNow();
        }
    }
    else
    {
        updateInfoFileNow();
    }
}

void MySettings::updateInfoFileNow()
{
    QString tempFileName = MySettings::dataDirectory() + "/info.tmp";
    if (QFile::exists(tempFileName) == true && QFile::remove(tempFileName) == false)
    {
        qCritical() << tempFileName << "exists and cannot be removed. This is unexpected!";
    }
    downloadInfoFileFromGitHub(tempFileName);
}

void MySettings::onInfoFileDownloaded(QString fileName)
{
    QString tempFileName = MySettings::dataDirectory() + "/info.tmp";
    if (tempFileName.compare(fileName) == 0)
    {
        QString infoFileName = MySettings::dataDirectory() + "/info.txt";

        if (QFile::exists(infoFileName) == true)
        {
            QString backupFile = infoFileName + ".bak";
            // delete the backup file
            if (QFile::exists(backupFile) == true && QFile::remove(backupFile) == false)
            {
                qCritical() << "failed to remove the old backup file" << backupFile;
            }

            // rename the current one as backup
            if (QFile::rename(infoFileName, backupFile) == false)
            {
                qCritical() << "failed to create a new backup" << backupFile;
            }
        }

        if (QFile::rename(fileName, infoFileName) == true)
        {
            qCritical() << infoFileName << "updated.";
            readInfoFile(infoFileName);
            MySettings::saveLastUpdateTime();
        }
        else
        {
            qCritical() << "failed to save file" << infoFileName;
        }
    }
}

void MySettings::onInfoFileDownloadFailed(QString fileName)
{
    // no need to log precisely as the error is already logged in DownloadManager::downloadFinished()
    qCritical() << "downloading" << fileName << "failed!";
}

QString MySettings::getSettingString(QString key)
{
    QString value = "";
    auto s = MySettings::instance();
    if (s != nullptr)
    {
        auto jsonValue = s->m_infoFromGithub[key];
        if (jsonValue.isString() == true)
        {
            value = jsonValue.toString();
        }
    }

    return value;
}

int MySettings::tokenLifeInSeconds()
{
    QString stringValue = MySettings::getSettingString("tokenLifeInSeconds");
    int valueInSetting;
    if (stringValue.isEmpty() == false)
    {
        valueInSetting = stringValue.toInt();
    }
    else
    {
        // default to 1800 seconds
        valueInSetting = 1800;
    }

    return valueInSetting;
}

int MySettings::audioDownloadTimeoutInSeconds()
{
    QString stringValue = MySettings::getSettingString("audioDownloadTimeoutInSeconds");
    int valueInSetting;
    if (stringValue.isEmpty() == false)
    {
        valueInSetting = stringValue.toInt();
    }
    else
    {
        // default to 5 seconds, I cannot wait for the sound longer than 5 seconds
        valueInSetting = 5;
    }

    return valueInSetting;
}

QRegularExpression MySettings::namePattern()
{
    QString pattern = MySettings::getSettingString("namePattern");
    if (pattern.isEmpty() == true)
    {
        // letters in alphabet and digital, at least 4 characters
        pattern = "^[A-Z0-9]{4,}$";
    }

    return QRegularExpression(pattern, QRegularExpression::CaseInsensitiveOption);
}

QRegularExpression MySettings::passwordPattern()
{
    QString pattern = MySettings::getSettingString("passwordPattern");
    if (pattern.isEmpty() == true)
    {
        // must include letters in alphabet and digital, at least 6 characters
        pattern = "^(?=.*\\d)(?=.*[A-Za-z]).{6,}$";
    }

    return QRegularExpression(pattern);
}

QRegularExpression MySettings::emailPattern()
{
    QString pattern = MySettings::getSettingString("emailPattern");
    if (pattern.isEmpty() == true)
    {
        pattern = "^[A-Z0-9._%+-]+@[A-Z0-9.-]+\\.[A-Z]{2,4}$";
    }

    return QRegularExpression(pattern, QRegularExpression::CaseInsensitiveOption);
}
