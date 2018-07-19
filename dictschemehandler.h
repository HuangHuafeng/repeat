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
    void  handleSchemeQrcx(QWebEngineUrlRequestJob *request);
    void  handleSchemeGdau(QWebEngineUrlRequestJob *request);

public:
    DictSchemeHandler(MdxDict & dict, QObject *parent = Q_NULLPTR);
    ~DictSchemeHandler();

    void installToWebEngingView(QWebEngineView & webEngineView);

    virtual void requestStarted(QWebEngineUrlRequestJob *request) override;
};

#endif // DICTSCHEMEHANDLER_H