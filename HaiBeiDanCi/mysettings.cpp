#include "mysettings.h"
#include "../golddict/gddebug.hh"

#include <QSettings>
#include <QCoreApplication>
#include <QUrl>
#include <QFile>

MySettings *MySettings::m_settings = nullptr;

MySettings::MySettings()
{
    connect(&m_downloadManager, SIGNAL(fileDownloaded(QString)), this, SLOT(onInfoFileDownloadedFromGithub(QString)));
    loadSettingsFromInfoFile();
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
    int ui = settings.value("updateInterval", 7).toInt();
    settings.endGroup();

    return ui;
}

QString MySettings::mediaHttpUrl()
{
    QString mhu = "";
    auto s = MySettings::instance();
    if (s != nullptr)
    {
        QJsonValue obj = s->m_infoFromGithub["mediahttp"];
        if (obj.isString() == true)
        {
            mhu = obj.toString();
        }
    }

    return mhu;
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
    QUrl infoFileUrl("https://raw.githubusercontent.com/HuangHuafeng/repeat/master/HaiBeiDanCi/info.json");
    m_downloadManager.download(infoFileUrl, saveToFileName);
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
