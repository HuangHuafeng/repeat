#include "dictschemehandler.h"
#include "golddict/gddebug.hh"

#include <QWebEngineUrlRequestJob>
#include <QBuffer>
#include <QWebEngineProfile>
#include <QMessageBox>

DictSchemeHandler::DictSchemeHandler(MdxDict & dict, QObject *parent): QWebEngineUrlSchemeHandler(parent),
    m_dict(dict),
    m_mediaPlayer(),
    m_tfm(parent)
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
    if (defaultProfile->urlSchemeHandler(QByteArray("gdau"))
            || defaultProfile->urlSchemeHandler(QByteArray("gdlookup"))
            || defaultProfile->urlSchemeHandler(QByteArray("bres"))
            || defaultProfile->urlSchemeHandler(QByteArray("qrcx"))
            || defaultProfile->urlSchemeHandler(QByteArray("gdpicture"))
            || defaultProfile->urlSchemeHandler(QByteArray("gdvideo"))
            || defaultProfile->urlSchemeHandler(QByteArray("bword"))) {
        // already installed
        gdDebug("We should not run to this line in DictSchemeHandler::installSchemeHandler()!");
    } else {
        defaultProfile->installUrlSchemeHandler(QByteArray("gdau"), this);
        defaultProfile->installUrlSchemeHandler(QByteArray("gdlookup"), this);
        defaultProfile->installUrlSchemeHandler(QByteArray("bres"), this);
        defaultProfile->installUrlSchemeHandler(QByteArray("qrcx"), this);
        defaultProfile->installUrlSchemeHandler(QByteArray("gdpicture"), this);
        defaultProfile->installUrlSchemeHandler(QByteArray("gdvideo"), this);
        defaultProfile->installUrlSchemeHandler(QByteArray("bword"), this);
    }
}

void DictSchemeHandler::requestStarted(QWebEngineUrlRequestJob *request)
{
    QUrl requestUrl = request->requestUrl();
    //gdDebug("request is %s in DictSchemeHandler::requestStarted()", requestUrl.toString().toStdString().c_str());

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
        gdDebug("failed to handle the request in DictSchemeHandler::handleSchemeGdlookup(): %s", requestUrl.toString().toStdString().c_str());
        request->fail(QWebEngineUrlRequestJob::UrlInvalid);
    }
}

void DictSchemeHandler::handleSchemeGdau(QWebEngineUrlRequestJob *request)
{
    QString contentType;
    auto dr = handleSchemeBres(request, contentType);
    MdxDict::waitRequest(dr);
    QString tempFile = createTemporaryFile(dr, request->requestUrl().path().section( '/', -1 ));
    m_mediaPlayer.play(tempFile);
}

QString DictSchemeHandler::createTemporaryFile(sptr< Dictionary::DataRequest > dr, QString fileName)
{
    QTemporaryFile tmp(QDir::temp().filePath( "XXXXXX-" + fileName ), this );
    if (dr.get() && dr->getFullData().size() != 0) {
        vector< char > const & data = dr->getFullData();

        if ( !tmp.open() || (size_t) tmp.write( &data.front(), data.size() ) != data.size() )
        {
          QMessageBox::critical( 0, "GoldenDict", tr( "Failed to create temporary file." ) );
        } else {
            tmp.setAutoRemove(false);
            m_tfm.addTemporaryFile(tmp);
        }
    } else {
        // anyway, we write an empty file in this case
        gdDebug("failed to get data in DictSchemeHandler::createTemporaryFile()");

        if ( !tmp.open() || (size_t) tmp.write( "FAKEDATA", 0 ) != 0 )
        {
          QMessageBox::critical( 0, "GoldenDict", tr( "Failed to create temporary file." ) );
        } else {
            tmp.setAutoRemove(false);
            m_tfm.addTemporaryFile(tmp);
        }
    }

    return tmp.fileName();
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
    return handleSchemeBres(request->requestUrl());
}


sptr< Dictionary::DataRequest > DictSchemeHandler::handleSchemeBres(QUrl url)
{
    std::string id = url.host().toStdString();
    auto dictionaries = m_dict.getDictionaries();

    for( unsigned x = 0; x < dictionaries.size(); ++x )
    {
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


void DictSchemeHandler::saveMediaFile(QUrl url)
{
    QString scheme = url.scheme();

    if (scheme.compare("qrcx") == 0) {
        return saveQcrx(url);
    }

    if (scheme.compare("gdau") == 0
            || scheme.compare("gico") == 0
            || scheme.compare("bres") == 0) {
        return saveOtherSchemes(url);
    }


    if (scheme.compare("gdlookup") == 0) {
        return;
    }
}

void DictSchemeHandler::saveOtherSchemes(QUrl url)
{
    QString fileName = QCoreApplication::applicationDirPath() + "/" + getMediaDir() + url.path();
    if (QFile::exists(fileName)) {
        return;
    }

    QString folder = fileName.section('/', 0, -2);
    QDir::current().mkpath(folder);
    auto dr = handleSchemeBres(url);
    MdxDict::waitRequest(dr);
    QString tempFile = createTemporaryFile(dr, "doesntmatter");
    if (QFile::copy(tempFile, fileName) == false) {
        gdDebug("failed to copy %s to %s", tempFile.toStdString().c_str(),
                fileName.toStdString().c_str());
    }
}


void DictSchemeHandler::saveQcrx(QUrl url)
{
    QString fileName = QCoreApplication::applicationDirPath() + "/media" + url.path();
    if (QFile::exists(fileName)) {
        return;
    }

    QString folder = fileName.section('/', 0, -2);
    QDir::current().mkpath(folder);
    QString tempFile = ":" + url.path();
    if (QFile::copy(tempFile, fileName) == false) {
        gdDebug("failed to copy %s to %s", tempFile.toStdString().c_str(),
                fileName.toStdString().c_str());
    }
}

void DictSchemeHandler::modifyHtml(QString &html)
{
    const QRegularExpression gdlink("(bres|gdau|gico|qrcx|gdlookup)://[^\"<>']*");

    // save the media files
    int offset = 0;
    QRegularExpressionMatch match = gdlink.match(html, offset);
    while (match.hasMatch()) {
        saveMediaFile(match.captured());
        offset = match.capturedEnd();
        match = gdlink.match(html, offset);
    }

    // modify the html
    match = gdlink.match(html, 0);
    while (match.hasMatch()) {
        QString matched = match.captured();
        QUrl url(matched);
        QString before = "";
        QString after = "";
        if (url.scheme().compare("gdau") == 0
                || url.scheme().compare("gico") == 0
                || url.scheme().compare("bres") == 0
                || url.scheme().compare("qrcx") == 0) {
            before = url.scheme() + "://" + url.host();
            if (url.scheme().compare("gdau") == 0) {
                after = "hhfaudio:///" + getMediaDir();
            } else {
                after = getMediaDir();
            }
            gdDebug("replacing %s with %s", before.toStdString().c_str(), after.toStdString().c_str());
            html = html.replace(before, after);
            offset = 0;
        } else {
            offset = match.capturedEnd();
        }
        match = gdlink.match(html, offset);
    }
}
