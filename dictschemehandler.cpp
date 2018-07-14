#include "dictschemehandler.h"
#include "golddict/gddebug.hh"

#include <QWebEngineUrlRequestJob>
#include <QBuffer>
#include <QWebEngineProfile>
#include <QMessageBox>

DictSchemeHandler::DictSchemeHandler(MdxDict & dict, QObject *parent): QWebEngineUrlSchemeHandler(parent),
    m_dict(dict),
    m_mediaPlayer(),
    m_tmpFiles()
{
}

DictSchemeHandler::~DictSchemeHandler()
{
    // delete the temporary files
    for (int i = 0; i < m_tmpFiles.size(); ++i) {
        QFile::remove(m_tmpFiles.at(i));
    }
}

void DictSchemeHandler::installToWebEngingView(QWebEngineView &webEngineView)
{
    webEngineView.page()->profile()->installUrlSchemeHandler(QByteArray("gdau"), this);
    webEngineView.page()->profile()->installUrlSchemeHandler(QByteArray("gdlookup"), this);
    webEngineView.page()->profile()->installUrlSchemeHandler(QByteArray("bres"), this);
    webEngineView.page()->profile()->installUrlSchemeHandler(QByteArray("qrcx"), this);
    webEngineView.page()->profile()->installUrlSchemeHandler(QByteArray("gdpicture"), this);
    webEngineView.page()->profile()->installUrlSchemeHandler(QByteArray("gdvideo"), this);
    webEngineView.page()->profile()->installUrlSchemeHandler(QByteArray("bword"), this);
}

void DictSchemeHandler::requestStarted(QWebEngineUrlRequestJob *request)
{
    QUrl requestUrl = request->requestUrl();
    gdDebug("request is %s in DictSchemeHandler::requestStarted()", requestUrl.toString().toStdString().c_str());

    sptr< Dictionary::DataRequest > dr;
    QString contentType;

    if (requestUrl.scheme().compare("gdlookup") == 0) {
        dr = handleSchemeGdlookup(request, contentType);
    } else if (requestUrl.scheme().compare("bres") == 0) {
        dr = handleSchemeBres(request, contentType);
    } else if (requestUrl.scheme().compare("gdau") == 0) {
        return handleSchemeGdau(request);
    } else if (requestUrl.scheme().compare("qrcx") == 0) {
        return handleSchemeQrcx(request);
    } else {
        gdDebug("scheme %s is unknown in DictSchemeHandler::requestStarted()", requestUrl.scheme().toStdString().c_str());
    }

    MdxDict::waitRequest(dr);
    if (dr.get()) {
        QByteArray *replyData = new QByteArray(&( dr->getFullData().front()), dr->getFullData().size());
        QBuffer *reply = new QBuffer(replyData);
        connect(reply, &QIODevice::aboutToClose, reply, &QObject::deleteLater);
        request->reply(QByteArray(contentType.toStdString().c_str()), reply);
    }
    else {
        gdDebug("failed in DictSchemeHandler::handleSchemeGdlookup()");
        request->fail(QWebEngineUrlRequestJob::UrlInvalid);
    }
}

void DictSchemeHandler::handleSchemeGdau(QWebEngineUrlRequestJob *request)
{
    QString contentType;
    auto dr = handleSchemeBres(request, contentType);
    MdxDict::waitRequest(dr);
    if (dr.get()) {
        vector< char > const & data = dr->getFullData();
        QTemporaryFile tmp(QDir::temp().filePath( "XXXXXX-" + request->requestUrl().path().section( '/', -1 ) ), this );

        if ( !tmp.open() || (size_t) tmp.write( &data.front(), data.size() ) != data.size() )
        {
          QMessageBox::critical( 0, "GoldenDict", tr( "Failed to create temporary file." ) );
          return;
        }

        tmp.setAutoRemove(false);
        QString fileName = tmp.fileName();
        m_tmpFiles.append(fileName);
        m_mediaPlayer.setMedia(QUrl::fromLocalFile(fileName));
        m_mediaPlayer.play();
    }
}

void DictSchemeHandler::handleSchemeQrcx(QWebEngineUrlRequestJob *request)
{
    QUrl newUrl( request->requestUrl() );
    newUrl.setScheme( "qrc" );
    newUrl.setHost( "" );
    request->redirect(newUrl);
}

sptr< Dictionary::DataRequest > DictSchemeHandler::handleSchemeGdlookup(QWebEngineUrlRequestJob *request, QString & contentType)
{
    ArticleNetworkAccessManager & anam = m_dict.getArticleNetworkAccessManager();

    QUrl queryUrl = request->requestUrl();
    if( Qt4x5::Url::hasQueryItem( queryUrl, "word" ) == false )
    {
        queryUrl.setHost( "localhost" );
        Qt4x5::Url::addQueryItem( queryUrl, "word", queryUrl.path().mid( 1 ) );
        Qt4x5::Url::addQueryItem( queryUrl, "group", QString::number( 0 ) );
    }

    return anam.getResource( queryUrl, contentType );
}

sptr< Dictionary::DataRequest > DictSchemeHandler::handleSchemeBres(QWebEngineUrlRequestJob *request, QString & /*contentType*/)
{
    QUrl url = request->requestUrl();

        std::string id = url.host().toStdString();

        bool search = ( id == "search" );

        auto dictionaries = m_dict.getDictionaries();

        if ( !search )
        {
          for( unsigned x = 0; x < dictionaries.size(); ++x )
            if ( dictionaries[ x ]->getId() == id )
            {
                if( url.scheme() == "gico" )
                {
                    QByteArray bytes;
                    QBuffer buffer(&bytes);
                    buffer.open(QIODevice::WriteOnly);
                    dictionaries[ x ]->getIcon().pixmap( 16 ).save(&buffer, "PNG");
                    buffer.close();
                    sptr< Dictionary::DataRequestInstant > ico = new Dictionary::DataRequestInstant( true );
                    ico->getData().resize( bytes.size() );
                    memcpy( &( ico->getData().front() ), bytes.data(), bytes.size() );
                    return ico;
                }
                try
                {
                  return  dictionaries[ x ]->getResource( Qt4x5::Url::path( url ).mid( 1 ).toUtf8().data() );
                }
                catch( std::exception & e )
                {
                  gdWarning( "getResource request error (%s) in \"%s\"\n", e.what(),
                             dictionaries[ x ]->getName().c_str() );
                  return sptr< Dictionary::DataRequest >();
                }
            }
        }

        return sptr< Dictionary::DataRequest >();
}
