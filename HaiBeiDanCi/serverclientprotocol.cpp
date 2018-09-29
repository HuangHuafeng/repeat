#include "serverclientprotocol.h"

#ifndef HAIBEIDANCI_SERVER
#include "clienttoken.h"
#endif

qint32 MessageHeader::m_currentSequenceNumber = 1;
const MessageHeader MessageHeader::invalidMessageHeader(-1, -1, -1, "__INVALID__");

MessageHeader::MessageHeader(qint32 code, qint32 respondsTo, qint32 sequenceNumber, QString tokenId)
{
    m_code = code;
    m_respondsTo = respondsTo;
    m_sequenceNumber = sequenceNumber;
    m_tokenId = tokenId;

    if (m_sequenceNumber == 0)
    {
        m_sequenceNumber = m_currentSequenceNumber ++;
    }
#ifndef HAIBEIDANCI_SERVER
    if (m_tokenId.isEmpty() == true)
    {
        m_tokenId = ClientToken::instance()->token().id();
    }
#endif
}

MessageHeader::MessageHeader(const QByteArray &msg)
{
    QDataStream in(msg);
    in.startTransaction();
    in >> *this;
    if (in.commitTransaction() == false)
    {
        qCritical("failed to construct a MessageHeader from %s", msg.constData());
        *this = invalidMessageHeader;
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

QString MessageHeader::tokenId() const
{
    return m_tokenId;
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

void MessageHeader::setTokenId(QString tokenId)
{
    m_tokenId = tokenId;
}

QString MessageHeader::toString() const
{
    return "{ \"code\": \"" + QString::number(m_code)
            + "\", \"sequenceNumber\": \"" + QString::number(m_sequenceNumber)
            + "\", \"respondsTo\": \"" + QString::number(m_respondsTo)
            + "\", \"tokenId\": \"" + m_tokenId + "\"}";
}

QDataStream &operator<<(QDataStream &ds, const MessageHeader &msgHead)
{
    ds << msgHead.code() << msgHead.sequenceNumber() << msgHead.respondsTo() << msgHead.tokenId();
    return ds;
}

QDataStream &operator>>(QDataStream &ds, MessageHeader &msgHead)
{
    qint32 code;
    qint32 sequenceNumber;
    qint32 respondsTo;
    QString tokenId;
    ds >> code >> sequenceNumber >> respondsTo >> tokenId;
    msgHead.setCode(code);
    msgHead.setSequenceNumber(sequenceNumber);
    msgHead.setRespondsTo(respondsTo);
    msgHead.setTokenId(tokenId);
    return ds;
}
