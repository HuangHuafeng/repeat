#ifndef CLIENTTOKEN_H
#define CLIENTTOKEN_H

#include "token.h"
#include "applicationuser.h"

#include <QObject>
#include <QAction>

class ClientToken : public QObject
{
    Q_OBJECT

public:
    static ClientToken * instance();
    bool isUserLoggedIn();
    bool promptUserToLogin(QWidget *parent = nullptr, QString text = QObject::tr("This requires a user to login first!"), QString informativeText= QObject::tr("Would you like to login now?"));

    void setLoginAction(QAction *loginAction);

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
    QAction *m_loginAction;
};

#endif // CLIENTTOKEN_H
