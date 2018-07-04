#include "golddict/mdx.hh"

#include <vector>
#include <string>

using std::vector;
using std::string;

sptr< Dictionary::Class > loadMdx(QString mdxFileName)
{
    vector< string > fileNames;
    fileNames.push_back(mdxFileName.toStdString());

//    vector< sptr< Dictionary::Class > > mdxDictionaries = Mdx::makeDictionaries(fileNames, "", );

    return 0;
}
