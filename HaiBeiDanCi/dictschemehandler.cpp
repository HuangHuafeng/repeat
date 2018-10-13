#include "dictschemehandler.h"
#include "mysettings.h"
#include "filedownloader.h"
#include "clienttoken.h"

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
        downloadFile(path.mid(1));
    }
}

void DictSchemeHandler::downloadFile(QString fileName)
{
    if (ClientToken::instance()->userAlreadyLogin() == false)
    {
        return;
    }

    FileDownloader *fd = new FileDownloader();
    // create a timer, we consider the downloading failed if it times out
    QTimer *t = new QTimer(this);

    connect(fd, &FileDownloader::downloadFinished, [fd, t, this] (const QMap<QString, ServerCommunicator::DownloadStatus> &downloadResult) {
        // we should have and ONLY have one file
        QString fileName = downloadResult.keys().at(0);
        bool succeeded = downloadResult.value(fileName, ServerCommunicator::DownloadFailed) == ServerCommunicator::DownloadSucceeded;
        this->onFileDownloaded(fileName, succeeded);
        fd->deleteLater();
        t->deleteLater();
        qDebug() << "fd->deleteLater() called because file downloaded!";
    });

    connect(t, &QTimer::timeout, [fd, t] () {
        fd->deleteLater();
        t->deleteLater();
        qDebug() << "fd->deleteLater() called because timer timed out!";
    });

    t->start(MySettings::audioDownloadTimeoutInSeconds() * 1000);
    fd->downloadFile(fileName);
}

void DictSchemeHandler::onFileDownloaded(QString fileName, bool succeeded)
{
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
