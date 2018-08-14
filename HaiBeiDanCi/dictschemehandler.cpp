#include "dictschemehandler.h"
#include "../golddict/gddebug.hh"

#include <QWebEngineUrlRequestJob>
#include <QCoreApplication>
#include <QBuffer>
#include <QWebEngineProfile>
#include <QMessageBox>

DictSchemeHandler::DictSchemeHandler(QObject *parent) : QWebEngineUrlSchemeHandler(parent),
                                                        m_mediaPlayer()
{
    installSchemeHandler();
}

DictSchemeHandler::~DictSchemeHandler()
{
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
    QString audioFile = "/Users/huafeng/Documents/GitHub/TextFinder/build-Repeat-Desktop_Qt_5_11_1_clang_64bit-Debug/Repeat.app/Contents/MacOS" + request->requestUrl().path();
    //QString audioFile = QCoreApplication::applicationDirPath() + request->requestUrl().path();
    m_mediaPlayer.play(audioFile);
}
