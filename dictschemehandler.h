#ifndef DICTSCHEMEHANDLER_H
#define DICTSCHEMEHANDLER_H

#include "mdxdict.h"

#include <QWebEngineUrlSchemeHandler>
#include <QWebEngineView>

class DictSchemeHandler : public QWebEngineUrlSchemeHandler
{
    Q_OBJECT

    MdxDict & m_dict;

private:
    sptr< Dictionary::DataRequest >  handleSchemeGdlookup(QWebEngineUrlRequestJob *request, QString & contentType);
    sptr< Dictionary::DataRequest >  handleSchemeBres(QWebEngineUrlRequestJob *request, QString & contentType);

public:
    DictSchemeHandler(MdxDict & dict, QObject *parent = Q_NULLPTR);

    virtual void requestStarted(QWebEngineUrlRequestJob *request) override;
    void installToWebEngingView(QWebEngineView & webEngineView);
};

#endif // DICTSCHEMEHANDLER_H
