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
    void loginUser(QString name, QString password);
    void logoutUser(QString name);

signals:
    void registerSucceeded(const ApplicationUser &user);
    void registerFailed(QString why);

    void loginSucceeded(const ApplicationUser &user, const Token &token);
    void loginFailed(QString why);

    void logoutSucceeded(QString name);

private slots:
    void onRegisterResult(qint32 result, const ApplicationUser &user);
    void onLoginResult(qint32 result, const ApplicationUser &user, const Token &token);
    void onLogoutResult(qint32 result, QString name);

public slots:
};

#endif // SERVERUSERAGENT_H
