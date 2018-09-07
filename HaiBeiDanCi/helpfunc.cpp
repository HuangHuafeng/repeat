#include "helpfunc.h"

#include <QtDebug>

HelpFunc::HelpFunc()
{

}

QStringList HelpFunc::filesInDir(QDir dir, const QStringList &nameFilters)
{
    QStringList files;
    QDir::Filters fileFilter = QDir::Files | QDir::NoDotAndDotDot;
    files += dir.entryList(nameFilters, fileFilter);

    QDir::Filters dirFilter = QDir::Dirs | QDir::NoDotAndDotDot;
    QStringList el = dir.entryList(nameFilters, dirFilter);
    for (int i = 0;i < el.size();i ++)
    {
        QDir subDir = dir;
        subDir.cd(el.at(i));
        files += filesInDir(subDir, nameFilters);
    }

    QString dirname = dir.dirName();
    for (int k = 0;k < files.size();k ++)
    {
        files[k] = dirname + "/" + files.at(k);
    }

    return files;
}
