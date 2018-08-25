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
