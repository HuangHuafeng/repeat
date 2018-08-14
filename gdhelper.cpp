#include "gdhelper.h"
#include "golddict/gddebug.hh"
#include "HaiBeiDanCi/word.h"

GDHelper::GDHelper(QObject *parent):
    QObject(parent),
    m_dict(parent),
    m_dictSchemeHandler(m_dict, parent)
{
    //m_dictSchemeHandler.installToWebEngingView(m_webEngineView);
}

void GDHelper::lookupWord(QString word, QWebEngineView &viewToUpdate)
{
    if (m_dict.getDictionaries().size()) {
        QString wordDefinition = getWordDefinitionPage(word);
        QUrl baseUrl("file://" + QCoreApplication::applicationDirPath() + "/");
        viewToUpdate.setHtml(wordDefinition, baseUrl);
    }
}

void GDHelper::loadBlankPage(QWebEngineView &viewToUpdate)
{
    QString contentType;
    QUrl blankPage( "gdlookup://localhost?blank=1" );

    sptr< Dictionary::DataRequest > r = m_dict.getArticleNetworkAccessManager().getResource( blankPage,
                                                                   contentType );

    // we should NOT call MdxDict::waitRequest(r) here!!! I don't know why.
    // MdxDict::waitRequest(r);
    viewToUpdate.setHtml( QString::fromUtf8( &( r->getFullData().front() ),
                                               r->getFullData().size() ),
                            blankPage );
}

void GDHelper::loadDict(const QString &dictFileFullName)
{
    m_dict.loadMdx(dictFileFullName);
}

QString GDHelper::getWordDefinitionPage(QString word)
{
    QString html = m_dict.getWordDefinitionPage(word);
    modifyHtml(html);
    return html;
}


void GDHelper::modifyHtml(QString &html)
{
    m_dictSchemeHandler.modifyHtml(html);
}


bool GDHelper::saveWord(const QString &spelling)
{
    if (Word::getWordId(spelling) != 0) {
        // already in database
        return true;
    }

    QString html = getWordDefinitionPage(spelling);
    if (html.contains("<p>No translation ")) {
        // cannot find the word in the dictionary
        return false;
    }

    auto word = Word::getWord(spelling, true);
    if (word.get()) {
        word->setDefinition(html);
    } else {
        return false;
    }

    return true;
}

bool GDHelper::saveWord(const QString &spelling, const QString &lemma)
{
    if (Word::getWordId(spelling) != 0) {
        // already in database
        return true;
    }

    QString html = getWordDefinitionPage(lemma);
    if (html.contains("<p>No translation ")) {
        // cannot find the word in the dictionary
        return false;
    }

    auto word = Word::getWord(spelling, true);
    if (word.get()) {
        word->setDefinition(html);
    } else {
        return false;
    }

    return true;
}
