#ifndef SERVERUSERAGENT_H
#define SERVERUSERAGENT_H

#include "servercommunicator.h"

#include <QObject>

class ServerUserAgent : public QObject
{
    Q_OBJECT

public:
    explicit ServerUserAgent(ServerCommunicator *sc = nullptr, QObject *parent = nullptr);

    void registerUser(QString name, QString password, QString email);
    void loginUser(QString name, QString password);
    void logoutUser(QString name);

signals:
    void registerResult(bool succeeded, const ApplicationUser &user, QString errorText);
    void loginResult(bool succeeded, const ApplicationUser &user, const Token &token, QString errorText);
    void logoutSucceeded(QString name);

private slots:
    void onRegisterResult(qint32 result, const ApplicationUser &user);
    void onLoginResult(qint32 result, const ApplicationUser &user, const Token &token);
    void onLogoutResult(qint32 result, QString name);

private:
    ServerCommunicator *m_sc;

};

#endif // SERVERUSERAGENT_H
