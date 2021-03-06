#include "clienttoken.h"

#include <QMessageBox>

ClientToken * ClientToken::m_ct = nullptr;

ClientToken::ClientToken(QObject *parent) :
    QObject (parent),
    m_token(Token::invalidToken),
    m_user(ApplicationUser::invalidUser),
    m_loginAction(nullptr)
{
}

ClientToken * ClientToken::instance()
{
    if (m_ct == nullptr)
    {
        m_ct = new ClientToken();
    }

    return m_ct;
}

void ClientToken::setToken(const Token &token)
{
    m_token = token;
}

const Token & ClientToken::token() const
{
    return m_token;
}

bool ClientToken::hasAliveToken() const
{
    return m_token.isAlive();
}

bool ClientToken::hasValidUser() const
{
    return m_user.isValid();
}

void ClientToken::setUser(const ApplicationUser &user)
{
    m_user = user;
}

const ApplicationUser & ClientToken::user() const
{
    return m_user;
}

void ClientToken::setLoginAction(QAction *loginAction)
{
    m_loginAction = loginAction;
}

bool ClientToken::isUserLoggedIn()
{
    return hasAliveToken() == true && hasValidUser() == true;
}

bool ClientToken::promptUserToLogin(QWidget *parent, QString text, QString informativeText)
{
    if (isUserLoggedIn() == true)
    {
        return true;
    }
    else
    {
        QMessageBox msgBox(parent);
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setText(text);
        if (m_loginAction != nullptr)
        {
            msgBox.setInformativeText(informativeText);
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            msgBox.setDefaultButton(QMessageBox::Yes);
            int ret = msgBox.exec();
            if (ret == QMessageBox::Yes)
            {
                m_loginAction->trigger();
            }
        }
        else
        {
            msgBox.exec();
        }
        return false;
    }
}
