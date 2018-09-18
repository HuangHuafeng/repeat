#ifndef TOKEN_H
#define TOKEN_H

#include "mysettings.h"

#include <QString>
#include <QDateTime>
#include <QDataStream>
#include <QHostAddress>

class Token
{
    QString m_id;
    QDateTime m_createTime;
    qint32 m_lifeInSeconds;
    QHostAddress m_peerAddress;

public:
    static const Token invalidToken;
    Token(QString id, int lifeInSeconds = MySettings::tokenLifeInSeconds(), QDateTime createTime = QDateTime::currentDateTime());

    bool isValid() const;
    bool isAlive() const;
    QString id() const;
    const QDateTime & createTime() const;
    int lifeInSeconds() const;
    const QHostAddress & peerAddress() const;

    void setPeerAddress(const QHostAddress &peerAddress);
};

QDataStream &operator<<(QDataStream &ds, const Token &t);
QDataStream &operator>>(QDataStream &ds, Token &t);

#endif // TOKEN_H
