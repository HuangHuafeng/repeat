#ifndef MDXDICT_H
#define MDXDICT_H

#include "golddict/mdx.hh"

class MdxDict : public QThread, public Dictionary::Initializing
{
    Q_OBJECT

    QString mdxFileName;
    std::vector< sptr< Dictionary::Class > > dictionaries;
    std::string exceptionText;

    void loadMdxDictionary();

public:
    MdxDict(QString const & mdxFileFullName);

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

void loadMdx(QWidget * parent, QString const & mdxFileFullName,
             std::vector< sptr< Dictionary::Class > > & dictionaries);

#endif // MDXDICT_H
