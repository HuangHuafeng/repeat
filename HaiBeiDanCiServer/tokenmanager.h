#ifndef TOKENMANAGER_H
#define TOKENMANAGER_H

#include "../HaiBeiDanCi/token.h"
#include "../golddict/sptr.hh"

#include <QObject>
#include <QString>
#include <QMap>
#include <QMutex>

class TokenManager : public QObject
{
    Q_OBJECT

public:
    static TokenManager * instance();

    sptr<Token> createToken(int lifeInSeconds = MySettings::tokenLifeInSeconds());
    void destroyToken(QString id);
    sptr<Token> getToken(QString id);
    QStringList popUnscheduledTokens();

private:
    TokenManager();

    static TokenManager *m_tm;

    QMap<QString, sptr<Token>> m_mapTokens;
    QMutex m_mapMutex;
    QStringList m_tokensToSecheuleDelete;

    QString generateTokenID();

private slots:
    void testOnTimerOut();
};

#endif // TOKENMANAGER_H
