#include "temporaryfilemanager.h"
#include "golddict/gddebug.hh"

TemporaryFileManager::TemporaryFileManager(QObject *parent) : QObject(parent),
    m_files()
{

}

TemporaryFileManager::~TemporaryFileManager()
{
    // delete the temporary files
    for (int i = 0; i < m_files.size(); ++i) {
        QString fileName = m_files.at(i);
        if (QFile::exists(fileName))
        {
            QFile::remove(fileName);
        }
    }
}

void TemporaryFileManager::addTemporaryFile(const QTemporaryFile &temporaryFile)
{
    //gdDebug("temporary file: %s", temporaryFile.fileName().toStdString().c_str());
    // autoRemove() seems removes the file, peculiar!
    //if (temporaryFile.autoRemove() == false)
    {
        m_files.append(temporaryFile.fileName());
    }
}
