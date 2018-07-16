#ifndef GDHELPER_H
#define GDHELPER_H

#include "mdxdict.h"
#include "dictschemehandler.h"

#include <QObject>
#include <QWebEngineView>

class GDHelper : public QObject
{
    Q_OBJECT

    QWebEngineView m_webEngineView;
    MdxDict m_dict;
    DictSchemeHandler m_dictSchemeHandler;

public:
    GDHelper(QObject *parent = nullptr);

    QWebEngineView * getDefinitionView()
    {
        return &m_webEngineView;
    }

    bool lookupWord(QString word);
    void loadDict(QString const & dictFileFullName);
};

#endif // GDHELPER_H
