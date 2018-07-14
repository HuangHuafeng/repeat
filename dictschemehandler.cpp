#include "dictschemehandler.h"
#include "golddict/gddebug.hh"

#include <QWebEngineUrlRequestJob>
#include <QBuffer>
#include <QWebEngineProfile>
#include <QMessageBox>

DictSchemeHandler::DictSchemeHandler(MdxDict & dict, QObject *parent): QWebEngineUrlSchemeHandler(parent), m_dict(dict)
{
}

void DictSchemeHandler::installToWebEngingView(QWebEngineView &webEngineView)
{
    webEngineView.page()->profile()->installUrlSchemeHandler(QByteArray("gdau"), this);
    webEngineView.page()->profile()->installUrlSchemeHandler(QByteArray("gdlookup"), this);
    webEngineView.page()->profile()->installUrlSchemeHandler(QByteArray("bres"), this);
    //webEngineView.page()->profile()->installUrlSchemeHandler(QByteArray("qrcx"), this);
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
        ;
    } else {
        gdDebug("scheme %s is unknown in DictSchemeHandler::requestStarted()", requestUrl.scheme().toStdString().c_str());
    }

    if (dr.get()) {
        QEventLoop localLoop;

        QObject::connect( dr.get(), SIGNAL( finished() ),
                          &localLoop, SLOT( quit() ) );
        localLoop.exec();
/*
        QString replyContent = QString::fromUtf8( &( dr->getFullData().front() ),
                                                 dr->getFullData().size() );

        QByteArray *replyData = new QByteArray(replyContent.toStdString().c_str());
        QBuffer *reply = new QBuffer(replyData);
        connect(reply, &QIODevice::aboutToClose, reply, &QObject::deleteLater);
        request->reply(QByteArray(contentType.toStdString().c_str()), reply);
*/
        /* this way has bug!*/
        //QString replyContent = QString::fromUtf8( &( dr->getFullData().front() ),
                                                 //dr->getFullData().size() );

        QByteArray *replyData = new QByteArray(&( dr->getFullData().front()), dr->getFullData().size());
        QBuffer *reply = new QBuffer(replyData);
        connect(reply, &QIODevice::aboutToClose, reply, &QObject::deleteLater);
        request->reply(QByteArray(contentType.toStdString().c_str()), reply);

/*
        {
            if (requestUrl.scheme().compare("bres") == 0) {
                //if ( requestUrl.scheme() == "gdau" ||
                  //           Dictionary::WebMultimediaDownload::isAudioUrl( requestUrl ) )
                        {
                    //gdDebug("audio");
                          // Audio data
                          //connect( audioPlayer.data(), SIGNAL( error( QString ) ), this, SLOT( audioPlayerError( QString ) ), Qt::UniqueConnection );
                          //QString errorMessage = audioPlayer->play( data.data(), data.size() );
                          //if( !errorMessage.isEmpty() )
                          //  QMessageBox::critical( this, "GoldenDict", tr( "Failed to play sound file: %1" ).arg( errorMessage ) );
                        }
                    //    else
                        {
                          // Create a temporary file
                          // Remove the ones previously used, if any
                          //cleanupTemp();
                          QString fileName;

                          {
                        vector< char > const & data = dr->getFullData();
                            QTemporaryFile tmp(
                              QDir::temp().filePath( "XXXXXX-" + requestUrl.path().section( '/', -1 ) ), this );

                            if ( !tmp.open() || (size_t) tmp.write( &data.front(), data.size() ) != data.size() )
                            {
                              QMessageBox::critical( 0, "GoldenDict", tr( "Failed to create temporary file." ) );
                              return;
                            }

                            tmp.setAutoRemove( false );

                            //desktopOpenedTempFiles.insert( fileName = tmp.fileName() );

                            fileName = tmp.fileName();
                          }

                          //if ( !QDesktopServices::openUrl( QUrl::fromLocalFile( fileName ) ) )
                          //  QMessageBox::critical( 0, "GoldenDict",
                          //                         tr( "Failed to auto-open resource file, try opening manually: %1." ).arg( fileName ) );
                        }
            }
        }
            */
    }
    else {
        gdDebug("failed in DictSchemeHandler::handleSchemeGdlookup()");
        request->fail(QWebEngineUrlRequestJob::UrlInvalid);
    }
}

sptr< Dictionary::DataRequest > DictSchemeHandler::handleSchemeGdlookup(QWebEngineUrlRequestJob *request, QString & contentType)
{
    ArticleNetworkAccessManager & anam = m_dict.getArticleNetworkAccessManager();

    QUrl requestUrl = request->requestUrl();
    //gdDebug("url.path() is %s", requestUrl.path().toStdString().c_str());
    //gdDebug("url.path().mid( 1 ) is %s", requestUrl.path().mid( 1 ).toStdString().c_str());

    QUrl queryUrl = requestUrl;
    if( Qt4x5::Url::hasQueryItem( requestUrl, "word" ) == false )
    {
        queryUrl.setHost( "localhost" );
        Qt4x5::Url::addQueryItem( queryUrl, "word", requestUrl.path().mid( 1 ) );
        Qt4x5::Url::addQueryItem( queryUrl, "group", QString::number( 0 ) );
    }

    return anam.getResource( queryUrl, contentType );
}

sptr< Dictionary::DataRequest > DictSchemeHandler::handleSchemeBres(QWebEngineUrlRequestJob *request, QString & contentType)
{
    //ArticleNetworkAccessManager & anam = m_dict.getArticleNetworkAccessManager();

    //contentType = "text/html";
    //contentType = "image/png";
    QUrl url = request->requestUrl();

    //DPRINTF( "Get %s\n", req.url().host().toLocal8Bit().data() );
        //DPRINTF( "Get %s\n", req.url().path().toLocal8Bit().data() );

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
        else
        {
          // We don't do search requests for now
            return sptr< Dictionary::DataRequest >();
        }

        return sptr< Dictionary::DataRequest >();
}
