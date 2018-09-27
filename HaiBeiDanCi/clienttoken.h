#ifndef CLIENTTOKEN_H
#define CLIENTTOKEN_H

#include "token.h"
#include "applicationuser.h"

#include <QObject>

class ClientToken : public QObject
{
    Q_OBJECT

public:
    static ClientToken * instance();
    static bool userAlreadyLogin(QWidget *parent = nullptr);

    void setToken(const Token &token);
    const Token & token() const;

    void setUser(const ApplicationUser &user);
    const ApplicationUser & user() const;

    bool hasAliveToken() const;
    bool hasValidUser() const;

private:
    ClientToken(QObject *parent = nullptr);

    static ClientToken * m_ct;
    Token m_token;
    ApplicationUser m_user;
};

#endif // CLIENTTOKEN_H
