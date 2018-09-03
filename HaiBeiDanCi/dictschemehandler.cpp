#include "dictschemehandler.h"
#include "mysettings.h"
#include "serverdatadownloader.h"

#include <QWebEngineUrlRequestJob>
#include <QCoreApplication>
#include <QBuffer>
#include <QWebEngineProfile>
#include <QMessageBox>
#include <QFile>

DictSchemeHandler::DictSchemeHandler(QObject *parent) : QWebEngineUrlSchemeHandler(parent),
                                                        m_mediaPlayer()
{
    installSchemeHandler();

    ServerDataDownloader *sdd = ServerDataDownloader::instance();
    connect(sdd, SIGNAL(fileDownloaded(QString, bool)), this, SLOT(onFileDownloaded(QString, bool)));
}

DictSchemeHandler::~DictSchemeHandler()
{
}

void DictSchemeHandler::onFileDownloaded(QString fileName, bool succeeded)
{
    if (m_filesInDownloading.contains(fileName) == true)
    {
        // this is a file that DictSchemeHandler asked to download
        m_filesInDownloading.removeOne(fileName);   // remove from the list
        if (succeeded == true)
        {
            QString audioFile = MySettings::dataDirectory() + "/" + fileName;
            m_mediaPlayer.play(audioFile);
        }
        else
        {
            qDebug() << "cannot play" << fileName << "because downloading from the server failed";
        }
    }
}

void DictSchemeHandler::installSchemeHandler()
{
    // just use the default profile, we are not going to be complicated
    // to the situation that pages use different profiles
    QWebEngineProfile *defaultProfile = QWebEngineProfile::defaultProfile();
    if (defaultProfile->urlSchemeHandler(QByteArray("hhfaudio")))
    {
        // already installed
        qInfo("We should not run to this line in DictSchemeHandler::installSchemeHandler()!");
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
        qInfo("scheme %s is unknown in DictSchemeHandler::requestStarted()", requestUrl.scheme().toStdString().c_str());
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
        const QString fileName = path.mid(1);
        m_filesInDownloading.append(fileName);
        ServerDataDownloader *sdd = ServerDataDownloader::instance();
        sdd->downloadFile(fileName);
    }
}
