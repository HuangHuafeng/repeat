#ifndef HELPFUNC_H
#define HELPFUNC_H

#include <QStringList>
#include <QDir>

class HelpFunc
{
public:
    HelpFunc();

    static QStringList filesInDir(QDir dir, const QStringList &nameFilters = QStringList());

};

#endif // HELPFUNC_H
