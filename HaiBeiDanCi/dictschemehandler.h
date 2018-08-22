#ifndef DICTSCHEMEHANDLER_H
#define DICTSCHEMEHANDLER_H

#include "mediaplayer.h"

#include <QWebEngineUrlSchemeHandler>
#include <QWebEngineView>

class DictSchemeHandler : public QWebEngineUrlSchemeHandler
{
    Q_OBJECT

    MediaPlayer m_mediaPlayer;
    QVector<QString> m_filesInDownloading;

  private:
    void handleSchemeHhfaudio(QWebEngineUrlRequestJob *request);

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
