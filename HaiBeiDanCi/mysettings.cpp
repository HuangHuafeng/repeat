#include "mysettings.h"
#include "../golddict/gddebug.hh"

#include <QSettings>
#include <QCoreApplication>
#include <QUrl>
#include <QFile>

MySettings * MySettings::m_settings = nullptr;

MySettings::MySettings()
{
    m_group = "Preferences";

    connect(&m_downloadManager, SIGNAL(fileDownloaded(QString)), this, SLOT(onFileDownloaded(QString)));
    loadInfoFromGitHub();
}

MySettings::~MySettings()
{
    if (m_settings != nullptr)
    {
        delete m_settings;
    }
}

MySettings * MySettings::getSettings()
{
    if (m_settings == nullptr)
    {
        m_settings = new MySettings();
    }

    return m_settings;
}


void MySettings::onFileDownloaded(QString fileName)
{
    QFile infoFile(fileName);
    if (!infoFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        gdDebug("failed to load info file");
        return;
    }

    QByteArray json = infoFile.readAll();
    m_infoFromGithub = QJsonDocument::fromJson(json);
    gdDebug("%s", json.toStdString().c_str());
    if (infoFile.remove() == false)
    {
        gdDebug("failed to remove info file");
    }
}

QString MySettings::group()
{
    auto s = MySettings::getSettings();
    if (s != nullptr)
    {
        return s->m_group;
    }

    return "Preferences";
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

QString MySettings::mediaHttpUrl()
{
    QSettings settings;
    settings.beginGroup(MySettings::group());
    QString mhu = settings.value("mediaHttpUrl", "").toString();
    settings.endGroup();

    if (mhu.isEmpty())
    {
        // if there's no local setting, used the one from GitHub
        auto s = MySettings::getSettings();
        if (s != nullptr)
        {
            QJsonValue obj = s->m_infoFromGithub["mediahttp"];
            if (obj.isString() == true)
            {
                mhu = obj.toString();
            }
        }
    }

    return mhu;
}

void MySettings::loadInfoFromGitHub()
{
    QUrl infoFileUrl("https://raw.githubusercontent.com/HuangHuafeng/repeat/master/HaiBeiDanCi/info.json");
    QString infoFileLocalPath = QCoreApplication::applicationDirPath() + "/info.txt";
    m_downloadManager.download(infoFileUrl, infoFileLocalPath);
}
