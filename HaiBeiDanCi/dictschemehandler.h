#ifndef DICTSCHEMEHANDLER_H
#define DICTSCHEMEHANDLER_H

#include "mediaplayer.h"

#include <QWebEngineUrlSchemeHandler>
#include <QWebEngineView>

class DictSchemeHandler : public QWebEngineUrlSchemeHandler
{
    Q_OBJECT

    MediaPlayer m_mediaPlayer;

private:
    void handleSchemeHhfaudio(QWebEngineUrlRequestJob *request);
    void downloadFile(QString fileName);

    QString getMediaDir() const
    {
        return "media";
    }

private slots:
    void onFileDownloaded(QString fileName, bool succeeded);

public:
    DictSchemeHandler(QObject *parent = Q_NULLPTR);
    virtual ~DictSchemeHandler() override;

    void installSchemeHandler();

    virtual void requestStarted(QWebEngineUrlRequestJob *request) override;
};

#endif // DICTSCHEMEHANDLER_H
