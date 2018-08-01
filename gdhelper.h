#ifndef GDHELPER_H
#define GDHELPER_H

#include "mdxdict.h"
#include "dictschemehandler.h"

#include <QObject>
#include <QWebEngineView>

class GDHelper : public QObject
{
    Q_OBJECT

    MdxDict m_dict;
    DictSchemeHandler m_dictSchemeHandler;

public:
    GDHelper(QObject *parent = nullptr);

    void lookupWord(QString word, QWebEngineView &viewToUpdate);
    void loadBlankPage(QWebEngineView &viewToUpdate);
    void loadDict(QString const & dictFileFullName);
    QString getWordDefinitionPage(QString word);
    bool saveWord(const QString &spelling);

private:
    void modifyHtml(QString &html);
};

#endif // GDHELPER_H
