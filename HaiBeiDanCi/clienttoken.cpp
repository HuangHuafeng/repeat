#include "clienttoken.h"

ClientToken * ClientToken::m_ct = nullptr;

ClientToken::ClientToken(QObject *parent) :
    QObject (parent),
    m_token(Token::invalidToken),
    m_user(ApplicationUser::invalidUser)
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
