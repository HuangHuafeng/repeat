#ifndef TEMPORARYFILEMANAGER_H
#define TEMPORARYFILEMANAGER_H

#include <QObject>
#include <QTemporaryFile>
#include <QVector>

class TemporaryFileManager : public QObject
{
    Q_OBJECT

    QVector<QString> m_files;

public:
    explicit TemporaryFileManager(QObject *parent = nullptr);
    ~TemporaryFileManager();

    void addTemporaryFile(const QTemporaryFile & temporaryFile);

signals:

public slots:
};

#endif // TEMPORARYFILEMANAGER_H
