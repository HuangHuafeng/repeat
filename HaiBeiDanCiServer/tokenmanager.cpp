#include "tokenmanager.h"

#include <QRandomGenerator>
#include <QTimer>

TokenManager * TokenManager::m_tm = nullptr;

TokenManager::TokenManager()
{

}

TokenManager * TokenManager::instance()
{
    if (m_tm == nullptr)
    {
        m_tm = new TokenManager();
    }

    return m_tm;
}

sptr<Token> TokenManager::getToken(QString id)
{
    m_mapMutex.lock();
    auto r = m_mapTokens.value(id);
    m_mapMutex.unlock();

    return r;
}

void TokenManager::destroyToken(QString id)
{
    m_mapMutex.lock();
    m_mapTokens.remove(id);
    m_mapMutex.unlock();
    qDebug() << "token" << id << "destroyed";
}

/**
 * @brief TokenManager::createToken
 * @param lifeInSeconds
 * @return
 * createToken() is called typically when a user logged in
 * this means that it's called in different threads (which do not have event loop)
 * so we emit tokenCreated instead of creating a timer (to delete the token)
 * the server should connect to tokenCreated (using Qt::QueuedConnection) and create a timer as it has an event loop
 * emit signal seems also REQUIRES event loop, we the abvoe does not work
 * we then let the server thread scans the m_tokensToSecheuleDelete list
 */
sptr<Token> TokenManager::createToken(int lifeInSeconds)
{
    QString id = generateTokenID();
    sptr<Token> t = new Token(id, lifeInSeconds);
    m_mapMutex.lock();
    m_mapTokens.insert(id, t);
    m_tokensToSecheuleDelete.append(id);
    m_mapMutex.unlock();

    return t;
}

QString TokenManager::generateTokenID()
{
    // make it simple and short
    QString id = QString::number(QRandomGenerator::global()->generate(), 16);

    return id;
}

void TokenManager::testOnTimerOut()
{
    qDebug() << "testOnTimerOut() called";
}

QStringList TokenManager::popUnscheduledTokens()
{
    QStringList tokens;
    m_mapMutex.lock();
    tokens = m_tokensToSecheuleDelete;
    m_tokensToSecheuleDelete.clear();
    m_mapMutex.unlock();

    return tokens;
}
