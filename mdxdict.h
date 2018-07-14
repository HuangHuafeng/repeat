#ifndef MDXDICT_H
#define MDXDICT_H

#include "golddict/mdx.hh"
#include "golddict/instances.hh"
#include "golddict/article_maker.hh"
#include "golddict/article_netmgr.hh"

class MdxLoader : public QThread, public Dictionary::Initializing
{
    Q_OBJECT

    QString mdxFileName;
    std::vector< sptr< Dictionary::Class > > dictionaries;
    std::string exceptionText;

    void loadMdxDictionary();

public:
    MdxLoader(QString const & mdxFileFullName);

    virtual void run();

    std::vector< sptr< Dictionary::Class > > const & getDictionaries() const
    { return dictionaries; }

    /// Empty string means to exception occured
    std::string const & getExceptionText() const
    { return exceptionText; }

signals:

  void indexingDictionarySignal( QString const & dictionaryName );

public:

  virtual void indexingDictionary( std::string const & dictionaryName ) throw();

};


class MdxDict {

    std::vector< sptr< Dictionary::Class > > m_dictionaries;
    Instances::Groups m_groupInstances;
    ArticleMaker m_articleMaker;
    ArticleNetworkAccessManager m_articleNetMgr;

    QWidget *m_parent;


    bool m_disallowContentFromOtherSites;
    bool m_hideGoldenDictHeader;

public:
    MdxDict(QWidget * parent);

    void loadMdx(QString const & mdxFileFullName);
    ArticleNetworkAccessManager & getArticleNetworkAccessManager() {
        return m_articleNetMgr;
    }

    std::vector< sptr< Dictionary::Class > > const & getDictionaries() const
    { return m_dictionaries; }

    QString getWordDefinitionPage(QString word);

    static void waitRequest(sptr< Dictionary::DataRequest > dr) {
        if (dr.get()) {
            QEventLoop localLoop;
            QObject::connect( dr.get(), SIGNAL( finished() ),
                              &localLoop, SLOT( quit() ) );
            localLoop.exec();
        }
    }
};

#endif // MDXDICT_H
