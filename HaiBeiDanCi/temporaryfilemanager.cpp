#include "temporaryfilemanager.h"

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
    m_files.append(temporaryFile.fileName());
}
