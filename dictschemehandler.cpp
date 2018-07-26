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
    if (defaultProfile->urlSchemeHandler(QByteArray("hhfaudio"))) {
        // already installed
        gdDebug("We should not run to this line in DictSchemeHandler::installSchemeHandler()!");
    } else {
        defaultProfile->installUrlSchemeHandler(QByteArray("hhfaudio"), this);
    }
}

void DictSchemeHandler::requestStarted(QWebEngineUrlRequestJob *request)
{
    QUrl requestUrl = request->requestUrl();

    if (requestUrl.scheme().compare("hhfaudio") == 0) {
        return handleSchemeHhfaudio(request);
    } else {
        // impossible as we only register hhfaudio
        gdDebug("scheme %s is unknown in DictSchemeHandler::requestStarted()", requestUrl.scheme().toStdString().c_str());
    }
}

void DictSchemeHandler::handleSchemeHhfaudio(QWebEngineUrlRequestJob *request)
{
    QString audioFile = QCoreApplication::applicationDirPath() + request->requestUrl().path();
    m_mediaPlayer.play(audioFile);
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

    gdDebug("unprocessed scheme: %s", scheme.toStdString().c_str());
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

void DictSchemeHandler::fetchHrefFiles(const QString html)
{
    const QRegularExpression gdlink("(bres|gdau|gico|qrcx|gdlookup|gdpicture|gdvideo|bword)://[^\"<>']*");

    // save the media files
    int offset = 0;
    QRegularExpressionMatch match = gdlink.match(html, offset);
    while (match.hasMatch()) {
        saveMediaFile(match.captured());
        offset = match.capturedEnd();
        match = gdlink.match(html, offset);
    }
}

void DictSchemeHandler::modifyHtml(QString &html)
{
    // save the media files
    fetchHrefFiles(html);

    // modify the html
    QString after;
    QString tomatch;
    QRegExp rx;

    // step 1: replace the buttons like "Word origin", "Examples", ...
    tomatch = "<a class=\"popup-button\" href=\"gdlookup://[^\"<>']*\">[\\w|\\s]+</a>";
    rx = QRegExp(tomatch.toStdString().c_str());
    after = "";
    html = html.replace(rx, after);

    // step 2: replace the schemes like "bres", "gdau", ...
    std::string dictId = m_dict.getDictionaries()[0].get()->getId();
    tomatch = QString("=\"(bres|gico|qrcx|gdpicture|gdvideo|bword)://") + dictId.c_str();
    after = "=\"" + getMediaDir();
    rx = QRegExp(tomatch.toStdString().c_str());
    html = html.replace(rx, after);

    // step 3: replace the scheme "gdau" with "hhfaudio"
    tomatch = QString("=\"gdau://") + dictId.c_str();
    after = "=\"hhfaudio:///" + getMediaDir();
    rx = QRegExp(tomatch.toStdString().c_str());
    html = html.replace(rx, after);

    /*
    // step 2: replace the schemes like "bres", "gdau", ...
    int offset = 0;
    const QRegularExpression gdlink("(bres|gdau|gico|qrcx|gdpicture|gdvideo|bword)://[^\"<>']*");
    QRegularExpressionMatch match = gdlink.match(html, offset);
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
            html = html.replace(before, after);
            offset = 0;
        } else if (url.scheme().compare("gdlookup") == 0) {
            before = url.toString();
            after = "#";    // invalidate the link
            html = html.replace(before, after);
            offset = 0;
        } else {
            offset = match.capturedEnd();
        }
        match = gdlink.match(html, offset);
    }
    */
}

