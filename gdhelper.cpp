#include "gdhelper.h"

GDHelper::GDHelper(QObject *parent):
    QObject(parent),
    m_dict(parent),
    m_dictSchemeHandler(m_dict, parent)
{
    m_dictSchemeHandler.installToWebEngingView(m_webEngineView);
}

bool GDHelper::lookupWord(QString word)
{
    if (m_dict.getDictionaries().size()) {
        QString wordDefinition = m_dict.getWordDefinitionPage(word);
        m_webEngineView.setHtml(wordDefinition);
        return true;
    } else {
        return false;
    }
}

void GDHelper::loadDict(const QString &dictFileFullName)
{
    m_dict.loadMdx(dictFileFullName);
}
