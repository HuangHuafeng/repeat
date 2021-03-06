#ifndef CLIENTHANDLER_H
#define CLIENTHANDLER_H

#include "clientwaiter.h"
#include "../HaiBeiDanCi/applicationuser.h"
#include "../HaiBeiDanCi/token.h"

#include <QByteArray>
#include <QHostAddress>

class ClientHandler
{
private:
    ClientWaiter &m_clientWaiter;
    QString m_tokenId;
    QHostAddress m_peerAddress;

    bool handleRequestNoOperation(const QByteArray &msg);
    bool handleRequestRegister(const QByteArray &msg);
    bool handleRequestLogin(const QByteArray &msg);
    bool handleRequestLogout(const QByteArray &msg);
    bool handleRequestAppVersion(const QByteArray &msg);
    bool handleRequestUpgraderVersion(const QByteArray &msg);
    void sendResponseUnknownRequest(const QByteArray &msg);
    void sendResponseNoOperation(const QByteArray &msg);
    void sendResponseRegister(const QByteArray &msg, qint32 result, const ApplicationUser &user);
    void sendResponseLogin(const QByteArray &msg, qint32 result, const ApplicationUser &user, const Token &token);
    void sendResponseInvalidTokenId(const QByteArray &msg);
    void sendResponseLogout(const QByteArray &msg, qint32 result, QString name);
    void sendResponseAppVersion(const QByteArray &msg, QString platform);
    void sendResponseUpgraderVersion(const QByteArray &msg, QString platform);

    bool registerUser(const QByteArray &msg, ApplicationUser &user);
    bool loginUser(const QByteArray &msg, ApplicationUser &user);
    bool logoutUser(const QByteArray &msg, QString name);
    bool validateUser(const ApplicationUser &user);

    bool validateMessage(const QByteArray &msg);

protected:
    virtual int handleMessage(const QByteArray &msg);
    void sendMessage(QByteArray msg, bool needCompress = false);
    void sendSimpleMessage(const QByteArray &msgToReply, qint32 msgCode);
    void sendResponseOK(const QByteArray &msg);

public:
    ClientHandler(ClientWaiter &clientWaiter);
    virtual ~ClientHandler();

    void setPeerAddress(const QHostAddress &peerAddress);
    const QHostAddress & peerAddress() const;

    int processMessage(const QByteArray &msg);
    void handleUnknownMessage(const QByteArray &msg);

    static const QString App;
    static const QString AppLib;
    static const QString Upgrader;
    static const QString UpgraderLib;
};

#endif // CLIENTHANDLER_H
