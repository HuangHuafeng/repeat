#ifndef DICTSCHEMEHANDLER_H
#define DICTSCHEMEHANDLER_H

#include "mdxdict.h"
#include "mediaplayer.h"
#include "temporaryfilemanager.h"

#include <QWebEngineUrlSchemeHandler>
#include <QWebEngineView>

class DictSchemeHandler : public QWebEngineUrlSchemeHandler
{
    Q_OBJECT

    MdxDict & m_dict;
    MediaPlayer m_mediaPlayer;
    TemporaryFileManager m_tfm;

private:
    sptr< Dictionary::DataRequest >  handleSchemeGdlookup(QWebEngineUrlRequestJob *request, QString & contentType);
    sptr< Dictionary::DataRequest >  handleSchemeBres(QWebEngineUrlRequestJob *request, QString &);
    sptr< Dictionary::DataRequest >  handleSchemeBres(QUrl url);
    void  handleSchemeQrcx(QWebEngineUrlRequestJob *request);
    void  handleSchemeGdau(QWebEngineUrlRequestJob *request);

    void saveQcrx(QUrl url);
    void saveOtherSchemes(QUrl url);
    QString createTemporaryFile(sptr< Dictionary::DataRequest > dr, QString fileName);

public:
    DictSchemeHandler(MdxDict & dict, QObject *parent = Q_NULLPTR);
    ~DictSchemeHandler();

    void installSchemeHandler();
    void modifyHtml(QString &html);
    void saveMediaFile(QUrl url);
    QString getMediaDir() const {
        return "media";
    }

    virtual void requestStarted(QWebEngineUrlRequestJob *request) override;
};

#endif // DICTSCHEMEHANDLER_H
