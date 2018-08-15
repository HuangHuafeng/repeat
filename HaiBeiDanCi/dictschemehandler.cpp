#include "dictschemehandler.h"
#include "../golddict/gddebug.hh"
#include "mysettings.h"

#include <QWebEngineUrlRequestJob>
#include <QCoreApplication>
#include <QBuffer>
#include <QWebEngineProfile>
#include <QMessageBox>

DictSchemeHandler::DictSchemeHandler(QObject *parent) : QWebEngineUrlSchemeHandler(parent),
                                                        m_mediaPlayer()
{
    installSchemeHandler();

    connect(&m_downloadManager, SIGNAL(fileDownloaded(QString)), this, SLOT(onFileDownloaded(QString)));
}

DictSchemeHandler::~DictSchemeHandler()
{
}

void DictSchemeHandler::onFileDownloaded(QString fileName)
{
    //gdDebug("%s downloaded!", fileName.toStdString().c_str());
    m_mediaPlayer.play(fileName);
}

void DictSchemeHandler::installSchemeHandler()
{
    // just use the default profile, we are not going to be complicated
    // to the situation that pages use different profiles
    QWebEngineProfile *defaultProfile = QWebEngineProfile::defaultProfile();
    if (defaultProfile->urlSchemeHandler(QByteArray("hhfaudio")))
    {
        // already installed
        gdDebug("We should not run to this line in DictSchemeHandler::installSchemeHandler()!");
    }
    else
    {
        defaultProfile->installUrlSchemeHandler(QByteArray("hhfaudio"), this);
    }
}

void DictSchemeHandler::requestStarted(QWebEngineUrlRequestJob *request)
{
    QUrl requestUrl = request->requestUrl();

    if (requestUrl.scheme().compare("hhfaudio") == 0)
    {
        return handleSchemeHhfaudio(request);
    }
    else
    {
        // impossible as we only register hhfaudio
        gdDebug("scheme %s is unknown in DictSchemeHandler::requestStarted()", requestUrl.scheme().toStdString().c_str());
    }
}

void DictSchemeHandler::handleSchemeHhfaudio(QWebEngineUrlRequestJob *request)
{
    QString path = request->requestUrl().path();
    QString audioFile = MySettings::dataDirectory() + path;
    if (QFile::exists(audioFile))
    {
        m_mediaPlayer.play(audioFile);
    } else
    {
        auto s = MySettings::getSettings();
        if (s != nullptr)
        {
            const QString mhu = s->mediaHttpUrl();
            QUrl url(mhu + path);
            m_downloadManager.download(url, audioFile);
        }
        //gdDebug("try to download %s", url.toString().toStdString().c_str());
        //gdDebug("and save to %s", audioFile.toStdString().c_str());
    }
}
