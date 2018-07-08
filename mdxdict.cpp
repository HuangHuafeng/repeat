#include <QMessageBox>

#include "mdxdict.h"

using std::vector;
using std::string;

MdxDict::MdxDict(QString const & mdxFileFullName):
    exceptionText( "Load did not finish" ) // Will be cleared upon success
{
    this->mdxFileName = mdxFileFullName;
}

void MdxDict::loadMdxDictionary()
{
    vector< string > allFiles;
    allFiles.push_back(mdxFileName.toStdString());

    QString indexDir = QCoreApplication::applicationDirPath() + "/index";

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

void MdxDict::run()
{
    loadMdxDictionary();
}

void MdxDict::indexingDictionary( string const & dictionaryName ) throw()
{
  emit indexingDictionarySignal( QString::fromUtf8( dictionaryName.c_str() ) );
}

void loadMdx(QWidget * parent, QString const & mdxFileFullName,
             std::vector< sptr< Dictionary::Class > > & dictionaries)
{
    dictionaries.clear();

    MdxDict mdxDictionary(mdxFileFullName);

    //QObject::connect( &mdxDictionary, SIGNAL( indexingDictionarySignal( QString const & ) ),
    //                  &init, SLOT( indexing( QString const & ) ) );

    QEventLoop localLoop;

    QObject::connect( &mdxDictionary, SIGNAL( finished() ),
                      &localLoop, SLOT( quit() ) );

    mdxDictionary.start();

    localLoop.exec();

    mdxDictionary.wait();

    if ( mdxDictionary.getExceptionText().size() || mdxDictionary.getDictionaries().size() == 0)
    {
        QMessageBox::critical( parent, "Error loading dictionaries",
                               QString::fromUtf8( mdxDictionary.getExceptionText().c_str() ) );

        return;
    }

    dictionaries = mdxDictionary.getDictionaries();
}
