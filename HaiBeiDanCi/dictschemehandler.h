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
    void  handleSchemeHhfaudio(QWebEngineUrlRequestJob *request);

    QString getMediaDir() const {
        return "media";
    }

public:
    DictSchemeHandler(QObject *parent = Q_NULLPTR);
    ~DictSchemeHandler();

    void installSchemeHandler();

    virtual void requestStarted(QWebEngineUrlRequestJob *request) override;
};

#endif // DICTSCHEMEHANDLER_H
