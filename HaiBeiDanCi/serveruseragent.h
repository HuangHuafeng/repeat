#ifndef SERVERUSERAGENT_H
#define SERVERUSERAGENT_H

#include "svragt.h"

#include <QObject>

class ServerUserAgent : public QObject
{
    Q_OBJECT

    SvrAgt m_svrAgt;
public:
    explicit ServerUserAgent(QObject *parent = nullptr);

    void registerUser(QString name, QString password, QString email);

signals:
    void registerSucceed(const ApplicationUser &user);
    void registerFailed(QString why);

private slots:
    void onRegisterResult(qint32 result, const ApplicationUser &user);

public slots:
};

#endif // SERVERUSERAGENT_H
