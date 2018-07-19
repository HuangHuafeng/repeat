#include "gdhelper.h"

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
        QString wordDefinition = m_dict.getWordDefinitionPage(word);
        viewToUpdate.setHtml(wordDefinition);
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
    return m_dict.getWordDefinitionPage(word);
}
