#include "serverclientprotocol.h"

qint32 MessageHeader::m_currentSequenceNumber = 1;

MessageHeader::MessageHeader(qint32 code, qint32 respondsTo, qint32 sequenceNumber)
{
    m_code = code;
    m_respondsTo = respondsTo;
    m_sequenceNumber = sequenceNumber;

    if (m_sequenceNumber == 0)
    {
        m_sequenceNumber = m_currentSequenceNumber ++;
    }
}

MessageHeader::MessageHeader(const QByteArray &msg)
{
    QDataStream in(msg);
    in.startTransaction();
    in >> *this;
    if (in.commitTransaction() == false)
    {
        qCritical("failed to construct a MessageHeader from %s", msg.constData());
        m_code = -1;
        m_respondsTo = -1;
        m_sequenceNumber = -1;
    }
}

qint32 MessageHeader::code() const
{
    return m_code;
}

qint32 MessageHeader::sequenceNumber() const
{
    return m_sequenceNumber;
}

qint32 MessageHeader::respondsTo() const
{
    return m_respondsTo;
}


void MessageHeader::setCode(qint32 code)
{
    m_code = code;
}

void MessageHeader::setSequenceNumber(qint32 sequenceNumber)
{
    m_sequenceNumber = sequenceNumber;
}

void MessageHeader::setRespondsTo(qint32 respondsTo)
{
    m_respondsTo = respondsTo;
}

QString MessageHeader::toString() const
{
    return "{ \"code\": \"" + QString::number(m_code) + "\", \"sequenceNumber\": \"" + QString::number(m_sequenceNumber) + "\", \"respondsTo\": \"" + QString::number(m_respondsTo) + "\" }";
}

QDataStream &operator<<(QDataStream &ds, const MessageHeader &msgHead)
{
    ds << msgHead.code() << msgHead.sequenceNumber() << msgHead.respondsTo();
    return ds;
}

QDataStream &operator>>(QDataStream &ds, MessageHeader &msgHead)
{
    qint32 code;
    qint32 sequenceNumber;
    qint32 respondsTo;
    ds >> code >> sequenceNumber >> respondsTo;
    msgHead.setCode(code);
    msgHead.setSequenceNumber(sequenceNumber);
    msgHead.setRespondsTo(respondsTo);
    return ds;
}


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

ApplicationVersion ApplicationVersion::fromInt(qint32 version)
{
    quint8 major = static_cast<quint8>(version & 0x00FF0000 >> 16);
    quint8 minor = static_cast<quint8>(version & 0x0000FF00 >> 8);
    quint8 patch = static_cast<quint8>(version & 0x000000FF);
    return ApplicationVersion(major, minor, patch);
}

ApplicationVersion ApplicationVersion::fromString(QString versionInString)
{
    auto versionInStringList = versionInString.split(".");
    ApplicationVersion version(0, 0, 0);
    if (versionInStringList.size() == 3)
    {
        quint8 major = static_cast<quint8>(versionInStringList.at(0).toInt());
        quint8 minor = static_cast<quint8>(versionInStringList.at(1).toInt());
        quint8 patch = static_cast<quint8>(versionInStringList.at(2).toInt());

        version = ApplicationVersion(major, minor, patch);
    }

    return version;
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
