#include "mysettings.h"
#include "../golddict/gddebug.hh"

#include <QSettings>
#include <QCoreApplication>
#include <QUrl>
#include <QFile>

MySettings *MySettings::m_settings = nullptr;

MySettings::MySettings()
{
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
        if (m_settings != nullptr)
        {
            connect(&m_settings->m_downloadManager, SIGNAL(fileDownloaded(QString)), m_settings, SLOT(onInfoFileDownloadedFromGithub(QString)));
            m_settings->loadSettingsFromInfoFile();
        }
    }

    return m_settings;
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

int MySettings::updateInterval()
{
    QSettings settings;
    settings.beginGroup(MySettings::group());
    int updInt = settings.value("updateInterval", -1).toInt();
    settings.endGroup();

    if (updInt == -1)
    {
        // there's no local setting, try to get the setting from info.txt
        QString updIntString = MySettings::getSettingString("updateInterval");
        if (updIntString.isEmpty() == false)
        {
            updInt = updIntString.toInt();
        }
        else
        {
            // default to one week if we cannot find any setting
            updInt = 7;
        }
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
    settings.endGroup();
}

void MySettings::restoreCardSettings()
{
    QSettings settings;
    settings.beginGroup(MySettings::group());
    settings.remove("defaultEasiness");
    settings.remove("cardMaximumInterval");
    settings.remove("cardDefaultInterval");
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

float MySettings::incorrectIncrease()
{
    return MySettings::incorrectDecrease() * -1.0f;
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
    return MySettings::getSettingString("infohttp");
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
    const QString defaultLastUpdateTime = "2016-10-31T10:00:00+08:00";
    QSettings settings;
    settings.beginGroup(MySettings::group());
    QString lutSting = settings.value("lastUpdateTime", defaultLastUpdateTime).toString();
    settings.endGroup();

    return QDateTime::fromString(lutSting, Qt::ISODate);
}

void MySettings::downloadInfoFileFromGitHub(QString saveToFileName)
{
    QString urlString = MySettings::infoFileHttpUrl();
    if (urlString.isEmpty() == true)
    {
        urlString = "https://raw.githubusercontent.com/HuangHuafeng/repeat/master/HaiBeiDanCi/info.json";
    }

    m_downloadManager.download(urlString, saveToFileName);
}

void MySettings::readInfoFile(QString infoFileName)
{
    QFile infoFile(infoFileName);
    if (!infoFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        gdDebug("failed to load info file");
        return;
    }

    QByteArray json = infoFile.readAll();
    m_infoFromGithub = QJsonDocument::fromJson(json);
}

void MySettings::onInfoFileDownloadedFromGithub(QString fileName)
{
    readInfoFile(fileName);
    MySettings::saveLastUpdateTime();
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
        QDateTime lut = MySettings::lastUpdateTime();
        QDateTime curTime = QDateTime::currentDateTime();
        auto pastDays = lut.daysTo(curTime);
        if (pastDays < MySettings::updateInterval())
        {
            // no need to update the info file
            readInfoFile(infoFileName);
            return;
        }
    }

    updateInfoFileNow();
}

void MySettings::updateInfoFileNow()
{
    QString infoFileName = MySettings::dataDirectory() + "/info.txt";

    if (QFile::exists(infoFileName) == true)
    {
        // delete the info file
        if (QFile::remove(infoFileName) == false)
        {
            gdDebug("failed to remove old info file");
        }
    }

    downloadInfoFileFromGitHub(infoFileName);
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
