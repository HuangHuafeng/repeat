#include "token.h"

Token Token::invalidToken("__INVALID__", 0);

Token::Token(QString id, int lifeInSeconds, QDateTime createTime) :
    m_id(id),
    m_createTime(createTime),
    m_lifeInSeconds(lifeInSeconds)
{
}

QString Token::id() const
{
    return m_id;
}

const QDateTime & Token::createTime() const
{
    return m_createTime;
}

int Token::lifeInSeconds() const
{
    return m_lifeInSeconds;
}

const QHostAddress & Token::peerAddress() const
{
    return m_peerAddress;
}

void Token::setPeerAddress(const QHostAddress &peerAddress)
{
    m_peerAddress = peerAddress;
}


QDataStream &operator<<(QDataStream &ds, const Token &t)
{
    ds << t.id() << t.lifeInSeconds() << t.createTime() << t.peerAddress();
    return ds;
}

QDataStream &operator>>(QDataStream &ds, Token &t)
{
    QString id;
    QDateTime createTime;
    int lifeInSeconds;
    QHostAddress peerAddress;
    ds >> id >> lifeInSeconds >> createTime >> peerAddress;
    t = Token(id, lifeInSeconds, createTime);
    t.setPeerAddress(peerAddress);
    return ds;
}
