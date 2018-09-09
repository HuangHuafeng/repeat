#ifndef DICTSCHEMEHANDLER_H
#define DICTSCHEMEHANDLER_H

#include "mdxdict.h"
#include "HaiBeiDanCi/mediaplayer.h"
#include "HaiBeiDanCi/temporaryfilemanager.h"

#include <QWebEngineUrlSchemeHandler>
#include <QWebEngineView>

class DictSchemeHandler : public QWebEngineUrlSchemeHandler
{
    Q_OBJECT

    MdxDict & m_dict;
    MediaPlayer m_mediaPlayer;
    TemporaryFileManager m_tfm;

private:
    void  handleSchemeHhfaudio(QWebEngineUrlRequestJob *request);
    sptr< Dictionary::DataRequest >  handleSchemeBres(QUrl url);

    void fetchHrefFiles(const QString html);
    void saveQcrx(QUrl url);
    void saveOtherSchemes(QUrl url);
    QString createTemporaryFile(sptr< Dictionary::DataRequest > dr, QString fileName);
    void saveMediaFile(QUrl url);
    QString getMediaDir() const {
        return "media";
    }

public:
    DictSchemeHandler(MdxDict & dict, QObject *parent = Q_NULLPTR);
    ~DictSchemeHandler();

    void installSchemeHandler();
    void modifyHtml(QString &html);
    void fetchAndSaveFile(QUrl url);

    virtual void requestStarted(QWebEngineUrlRequestJob *request) override;
};

#endif // DICTSCHEMEHANDLER_H
