#include "applicationversion.h"

#include <QRegularExpression>

ApplicationVersion::ApplicationVersion(quint8 major, quint8 minor, quint8 patch)
{
    m_major = major;
    m_minor = minor;
    m_patch = patch;
}


qint32 ApplicationVersion::toInt() const
{
    return m_major << 16 | m_minor << 8 | m_patch;
}

QString ApplicationVersion::toString() const
{
    return QString("%1.%2.%3").arg(m_major).arg(m_minor).arg(m_patch);
}

ApplicationVersion ApplicationVersion::fromInt(qint32 version)
{
    quint8 major = static_cast<quint8>((version & 0x00FF0000) >> 16);
    quint8 minor = static_cast<quint8>((version & 0x0000FF00) >> 8);
    quint8 patch = static_cast<quint8>(version & 0x000000FF);
    return ApplicationVersion(major, minor, patch);
}

ApplicationVersion ApplicationVersion::fromString(QString versionInString)
{
    if (ApplicationVersion::isValidVersion(versionInString) == false)
    {
        return ApplicationVersion(0, 0, 0);
    }

    auto versionInStringList = versionInString.split(".");
    quint8 major = static_cast<quint8>(versionInStringList.at(0).toInt());
    quint8 minor = static_cast<quint8>(versionInStringList.at(1).toInt());
    quint8 patch = static_cast<quint8>(versionInStringList.at(2).toInt());

    return ApplicationVersion(major, minor, patch);
}

// static
bool ApplicationVersion::isValidVersion(QString versionInString)
{
    QRegularExpression re("^\\d+\\.\\d+\\.\\d+$");
    return re.match(versionInString).hasMatch();
}

QDataStream &operator<<(QDataStream &ds, const ApplicationVersion &appVer)
{
    ds << appVer.toInt();
    return ds;
}

QDataStream &operator>>(QDataStream &ds, ApplicationVersion &appVer)
{
    qint32 version;
    ds >> version;
    appVer = ApplicationVersion::fromInt(version);
    return ds;
}
