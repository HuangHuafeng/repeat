#ifndef APPLICATIONVERSION_H
#define APPLICATIONVERSION_H

#include <QDataStream>

class ApplicationVersion
{
    quint8 m_major;
    quint8 m_minor;
    quint8 m_patch;

public:
    typedef enum {
        VersionPreferred = 0,       // the version is preferred
        VersionOldButOK = 1,        // version is not the preferred one, but OK to continue
        VersionNotSupported = 2,    // not supported, need to upgrade/downgrade
        VersionUnknown = 3,
    } Status;

    ApplicationVersion(quint8 major, quint8 minor, quint8 patch);

    qint32 toInt() const;
    QString toString() const;
    static ApplicationVersion fromInt(qint32 version);
    static ApplicationVersion fromString(QString versionInString);
    static bool isValidVersion(QString versionInString);
};

QDataStream &operator<<(QDataStream &ds, const ApplicationVersion &appVer);
QDataStream &operator>>(QDataStream &ds, ApplicationVersion &appVer);

#endif // APPLICATIONVERSION_H
