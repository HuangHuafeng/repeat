#ifndef GDVIEW_H
#define GDVIEW_H

#include "mdxdict.h"

#include <QWebEngineView>

class GDView : public QWebEngineView
{
    Q_OBJECT

    MdxDict & m_dict;

public:
    GDView(MdxDict & dict);
};

#endif // GDVIEW_H
