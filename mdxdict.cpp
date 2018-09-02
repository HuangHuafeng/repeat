#include "mdxdict.h"
#include "golddict/gddebug.hh"
#include "HaiBeiDanCi/mysettings.h"

#include <QMessageBox>


using std::vector;
using std::string;

MdxLoader::MdxLoader(QString const & mdxFileFullName):
    exceptionText( "Load did not finish" ) // Will be cleared upon success
{
    this->mdxFileName = mdxFileFullName;
}

void MdxLoader::loadMdxDictionary()
{
    vector< string > allFiles;
    allFiles.push_back(mdxFileName.toStdString());

    QString indexDir = MySettings::dataDirectory() + "/";
    gdDebug("index direcory: %s\n", indexDir.toStdString().c_str());

    try
    {
        dictionaries = Mdx::makeDictionaries(allFiles, indexDir.toStdString(), *this);

        if (dictionaries.size() == 0) {
            exceptionText = (mdxFileName + " is not a valid mdx file!").toStdString();
        } else {
            exceptionText.clear();
        }
    } catch( std::exception & e ) {
        exceptionText = e.what();
    }
}

void MdxLoader::run()
{
    loadMdxDictionary();
}

void MdxLoader::indexingDictionary( string const & dictionaryName ) noexcept
{
  emit indexingDictionarySignal( QString::fromUtf8( dictionaryName.c_str() ) );
}

void MdxDict::loadMdx(QString const & mdxFileFullName)
{
    m_dictionaries.clear();

    MdxLoader loader(mdxFileFullName);

    //QObject::connect( &mdxDictionary, SIGNAL( indexingDictionarySignal( QString const & ) ),
    //                  &init, SLOT( indexing( QString const & ) ) );

    QEventLoop localLoop;

    QObject::connect( &loader, SIGNAL( finished() ),
                      &localLoop, SLOT( quit() ) );

    loader.start();

    localLoop.exec();

    loader.wait();

    if ( loader.getExceptionText().size() || loader.getDictionaries().size() == 0)
    {
        QMessageBox::critical( nullptr, "Error loading dictionaries",
                               QString::fromUtf8( loader.getExceptionText().c_str() ) );

        return;
    }

    m_dictionaries = loader.getDictionaries();
}

MdxDict::MdxDict(QObject *parent): m_articleMaker( m_dictionaries, m_groupInstances, "",""),
    m_articleNetMgr( parent, m_dictionaries, m_articleMaker, m_disallowContentFromOtherSites, m_hideGoldenDictHeader),
    m_disallowContentFromOtherSites(true),
    m_hideGoldenDictHeader(true)
{
}

QString MdxDict::getWordDefinitionPage(QString word)
{
    if (m_dictionaries.size() == 0)
    {
        return "";
    }

    QString contentType;
    QString req("gdlookup://localhost?word=" + word + "&dictionaries=" + m_dictionaries[0]->getId().c_str());
    sptr< Dictionary::DataRequest > r = m_articleNetMgr.getResource( req,
                                                                     contentType );
    MdxDict::waitRequest(r);
    if (r.get()) {
        return QString::fromUtf8( &( r->getFullData().front() ),
                                                 r->getFullData().size() );
    } else {
        return "";
    }
}
