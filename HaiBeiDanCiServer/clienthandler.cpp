#include "clienthandler.h"
#include "tokenmanager.h"
#include "appreleaser.h"
#include "upgraderreleaser.h"

ClientHandler::ClientHandler(ClientWaiter &clientWaiter) :
    m_clientWaiter(clientWaiter),
    m_tokenId("")
{
}

ClientHandler::~ClientHandler()
{
}

void ClientHandler::setPeerAddress(const QHostAddress &peerAddress)
{
    m_peerAddress = peerAddress;
}

const QHostAddress & ClientHandler::peerAddress() const
{
    return m_peerAddress;
}

/**
 * @brief ClientHandler::processMessage
 * @param msg
 * @return
 *
 */
int ClientHandler::processMessage(const QByteArray &msg)
{
    if (validateMessage(msg) == false)
    {
        sendResponseInvalidTokenId(msg);
        return -3;
    }

    return handleMessage(msg);
}

int ClientHandler::handleMessage(const QByteArray &msg)
{
    MessageHeader receivedMsgHeader(msg);
    bool handleResult = false;
    bool unknowMessage = false;
    bool clientSaidBye = false;
    switch (receivedMsgHeader.code()) {
    case ServerClientProtocol::RequestNoOperation:
        handleResult = handleRequestNoOperation(msg);
        break;

    case ServerClientProtocol::RequestBye:
        clientSaidBye = true;
        break;

    case ServerClientProtocol::RequestRegister:
        handleResult = handleRequestRegister(msg);
        break;

    case ServerClientProtocol::RequestLogin:
        handleResult = handleRequestLogin(msg);
        break;

    case ServerClientProtocol::RequestLogout:
        handleResult = handleRequestLogout(msg);
        break;

    case ServerClientProtocol::RequestAppVersion:
        handleResult = handleRequestAppVersion(msg);
        break;

    case ServerClientProtocol::RequestUpgraderVersion:
        handleResult = handleRequestUpgraderVersion(msg);
        break;

    default:
        // don't call handleUnknownMessage() here
        // as it's possible that ClientWaiter knows the message: RequestPromoteToManager
        //handleUnknownMessage(msg);
        unknowMessage = true;
        break;
    }

    if (clientSaidBye == true)
    {
        return -2;
    }

    if (unknowMessage == true)
    {
        return -1;
    }

    if (handleResult == true)
    {
        return 0;

    }
    else
    {
        return 1;
    }
}

bool ClientHandler::handleRequestNoOperation(const QByteArray &msg)
{
    qDebug() << "Heartbeat received from the client";
    sendResponseNoOperation(msg);
    return true;
}

void ClientHandler::handleUnknownMessage(const QByteArray &msg)
{
    sendResponseUnknownRequest(msg);
}

void ClientHandler::sendResponseUnknownRequest(const QByteArray &msg)
{
    MessageHeader receivedMsgHeader(msg);
    MessageHeader responseHeader(ServerClientProtocol::ResponseUnknownRequest, receivedMsgHeader.sequenceNumber());
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << responseHeader << receivedMsgHeader.code();
    sendMessage(block);
}

void ClientHandler::sendResponseNoOperation(const QByteArray &msg)
{
    sendSimpleMessage(msg, ServerClientProtocol::ResponseNoOperation);
}

void ClientHandler::sendResponseOK(const QByteArray &msg)
{
    sendSimpleMessage(msg, ServerClientProtocol::ResponseOK);
}

void ClientHandler::sendResponseInvalidTokenId(const QByteArray &msg)
{
    sendSimpleMessage(msg, ServerClientProtocol::ResponseInvalidTokenId);
}

void ClientHandler::sendMessage(QByteArray msg, bool needCompress)
{
    m_clientWaiter.sendMessage(msg, needCompress);
}

void ClientHandler::sendSimpleMessage(const QByteArray &msgToReply, qint32 msgCode)
{
    m_clientWaiter.sendSimpleMessage(msgToReply, msgCode);
}

bool ClientHandler::validateUser(const ApplicationUser &user)
{
    // we are not able to validate password as it's already md5 (by default)
    auto nameRE = MySettings::namePattern();
    auto emailRE = MySettings::emailPattern();

    return nameRE.match(user.name()).hasMatch()
            && emailRE.match(user.email()).hasMatch();
}

bool ClientHandler::registerUser(const QByteArray &msg, ApplicationUser &user)
{
    bool retVal = false;
    qint32 result = ApplicationUser::ResultRegisterFailedUnknown;
    if (user.id() != 0 || validateUser(user) == false)
    {
        // we expect the id is 0 when the client tries to register a user!
        // and name/email must meet the required rules!
        result = ApplicationUser::ResultRegisterFailedUnknown;
        retVal = false;
    }
    else if (ApplicationUser::userExist(user.name()) == true)
    {
        result = ApplicationUser::ResultRegisterFailedNameAlreadyUsed;
        retVal = false;
    }
    else
    {
        // add more check here later, now we just create the user
        retVal = ApplicationUser::createUser(user);
        if (retVal == true)
        {
            result = ApplicationUser::ResultRegisterOK;
        }
        else
        {
            result = ApplicationUser::ResultRegisterFailedServerError;
        }
    }

    sendResponseRegister(msg, result, user);

    return retVal;
}

bool ClientHandler::handleRequestRegister(const QByteArray &msg)
{
    funcTracker ft("handleRequestRegister()");

    QDataStream in(msg);
    MessageHeader receivedMsgHeader(-1, -1, -1);
    ApplicationUser user = ApplicationUser::invalidUser;
    in.startTransaction();
    in >> receivedMsgHeader >> user;
    if (in.commitTransaction() == false)
    {
        qCritical() << "failed to get file name in handleRequestRegister()";
        return false;
    }

    return registerUser(msg, user);
}

bool ClientHandler::handleRequestLogin(const QByteArray &msg)
{
    funcTracker ft("handleRequestLogin()");

    QDataStream in(msg);
    MessageHeader receivedMsgHeader(-1, -1, -1);
    ApplicationUser user = ApplicationUser::invalidUser;
    in.startTransaction();
    in >> receivedMsgHeader >> user;
    if (in.commitTransaction() == false)
    {
        qCritical() << "failed to get file name in handleRequestLogin()";
        return false;
    }

    return loginUser(msg, user);
}

bool ClientHandler::handleRequestLogout(const QByteArray &msg)
{
    funcTracker ft("handleRequestLogout()");

    QDataStream in(msg);
    MessageHeader receivedMsgHeader(-1, -1, -1);
    QString name;
    in.startTransaction();
    in >> receivedMsgHeader >> name;
    if (in.commitTransaction() == false)
    {
        qCritical() << "failed to get name in handleRequestLogout()";
        return false;
    }

    return logoutUser(msg, name);
}

bool ClientHandler::handleRequestAppVersion(const QByteArray &msg)
{
    funcTracker ft("handleRequestAppVersion()");

    QDataStream in(msg);
    MessageHeader receivedMsgHeader(-1, -1, -1);
    QString platform;
    in.startTransaction();
    in >> receivedMsgHeader >> platform;
    if (in.commitTransaction() == false)
    {
        qCritical() << "failed to get platform in handleRequestAppVersion()";
        return false;
    }

    sendResponseAppVersion(msg, platform);

    return true;
}

bool ClientHandler::handleRequestUpgraderVersion(const QByteArray &msg)
{
    funcTracker ft("handleRequestAppVersion()");

    QDataStream in(msg);
    MessageHeader receivedMsgHeader(-1, -1, -1);
    QString platform;
    in.startTransaction();
    in >> receivedMsgHeader >> platform;
    if (in.commitTransaction() == false)
    {
        qCritical() << "failed to get platform in handleRequestUpgraderVersion()";
        return false;
    }

    sendResponseUpgraderVersion(msg, platform);

    return true;
}

bool ClientHandler::logoutUser(const QByteArray &msg, QString name)
{
    MessageHeader receivedMsgHeader(msg);
    TokenManager::instance()->destroyToken(receivedMsgHeader.tokenId());
    sendResponseLogout(msg, ApplicationUser::ResultLogoutOK, name);

    return true;
}

bool ClientHandler::loginUser(const QByteArray &msg, ApplicationUser &user)
{
    bool retVal = false;
    qint32 result = ApplicationUser::ResultLoginFailedUnknown;
    Token token = Token::invalidToken;
    if (validateUser(user) == false)
    {        
        // we expect the id is 0 when the client tries to register a user!
        // [THIS IS NOT ALWAYS TRUE, as it's possilbe the client don't have full info of the user!!]
        // and name/email must meet the required rules!
        result = ApplicationUser::ResultLoginFailedUnknown;
        retVal = false;
    }
    else
    {
        auto existingUser = ApplicationUser::getUser(user.name());
        if (existingUser.get() == nullptr)
        {
            result = ApplicationUser::ResultLoginFailedNameDoesNotExist;
            retVal = false;
        }
        else
        {
            if (existingUser->password() != user.password())
            {
                result = ApplicationUser::ResultLoginFailedIncorrectPassword;
                retVal = false;
            }
            else
            {
                // we cannot expect the client to send us the email
                // [THIS IS NOT ALWAYS TRUE, as it's possilbe the client don't have full info of the user!!]
                //if (existingUser->email() != user.email())
                //{
                //    result = ApplicationUser::ResultLoginFailedUnknown;
                //    retVal = false;
                //}
                //else
                {
                    // create token for this user
                    auto nt = TokenManager::instance()->createToken();
                    nt->setPeerAddress(peerAddress());
                    token = *nt;
                    m_tokenId = token.id();

                    // send back all the user information as the client may need this
                    user = *existingUser;

                    result = ApplicationUser::ResultLoginOK;
                    retVal = true;
                }
            }
        }
    }

    sendResponseLogin(msg, result, user, token);

    return retVal;
}

void ClientHandler::sendResponseRegister(const QByteArray &msg, qint32 result, const ApplicationUser &user)
{
    MessageHeader receivedMsgHeader(msg);
    MessageHeader responseHeader(ServerClientProtocol::ResponseRegister, receivedMsgHeader.sequenceNumber());

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << responseHeader << result << user;
    sendMessage(block);
}

void ClientHandler::sendResponseLogin(const QByteArray &msg, qint32 result, const ApplicationUser &user, const Token &token)
{
    MessageHeader receivedMsgHeader(msg);
    MessageHeader responseHeader(ServerClientProtocol::ResponseLogin, receivedMsgHeader.sequenceNumber());

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << responseHeader << result << user << token;
    sendMessage(block);
}

void ClientHandler::sendResponseLogout(const QByteArray &msg, qint32 result, QString name)
{
    MessageHeader receivedMsgHeader(msg);
    MessageHeader responseHeader(ServerClientProtocol::ResponseLogout, receivedMsgHeader.sequenceNumber());

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << responseHeader << result << name;
    sendMessage(block);
}

void ClientHandler::sendResponseAppVersion(const QByteArray &msg, QString platform)
{
    MessageHeader receivedMsgHeader(msg);
    MessageHeader responseHeader(ServerClientProtocol::ResponseAppVersion, receivedMsgHeader.sequenceNumber());

    AppReleaser *ar = AppReleaser::instance();
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    auto cv = ar->currentVersion(platform);
    out << responseHeader << cv.version << cv.fileName << cv.info << cv.releaseTime;
    sendMessage(block);
}

void ClientHandler::sendResponseUpgraderVersion(const QByteArray &msg, QString platform)
{
    MessageHeader receivedMsgHeader(msg);
    MessageHeader responseHeader(ServerClientProtocol::ResponseUpgraderVersion, receivedMsgHeader.sequenceNumber());

    UpgraderReleaser *ur = UpgraderReleaser::instance();
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    auto cv = ur->currentVersion(platform);
    out << responseHeader << cv.version << cv.fileName << cv.info << cv.releaseTime;
    sendMessage(block);
}

bool ClientHandler::validateMessage(const QByteArray &msg)
{
    MessageHeader receivedMsgHeader(msg);
    if (receivedMsgHeader.code() != ServerClientProtocol::RequestLogin
            && receivedMsgHeader.code() != ServerClientProtocol::RequestRegister
            && receivedMsgHeader.code() != ServerClientProtocol::RequestNoOperation
            && receivedMsgHeader.code() != ServerClientProtocol::RequestGetAllBooks
            && receivedMsgHeader.code() != ServerClientProtocol::RequestGetABook
            && receivedMsgHeader.code() != ServerClientProtocol::RequestAppVersion
            && receivedMsgHeader.code() != ServerClientProtocol::RequestGetApp
            && receivedMsgHeader.code() != ServerClientProtocol::RequestUpgraderVersion
            && receivedMsgHeader.code() != ServerClientProtocol::RequestGetUpgrader)
    {
        QString msgTokenId = receivedMsgHeader.tokenId();
        if (msgTokenId.isEmpty() == true)
        {
            return false;
        }
        else
        {
            if (m_tokenId.isEmpty() == false)
            {
                if (m_tokenId != msgTokenId)
                {
                    qCritical() << "expected tokenID:" << m_tokenId << ", received tokenId:" << msgTokenId;
                    return false;
                }
                else
                {
                    qDebug() << "token id matches";
                    return true;
                }
            }
            else
            {// msgTokenId.isEmpty() == false and m_tokenId.isEmpty() == true
                auto token = TokenManager::instance()->getToken(msgTokenId);
                if (token.get() != nullptr)
                {
                    if (token->peerAddress() == peerAddress())
                    {
                        // the tokenId in the message is valid, use it for this client handler
                        m_tokenId = msgTokenId;
                        qDebug() << "token fetched for the client";
                        return true;
                    }
                    else
                    {
                        qCritical() << token->peerAddress() << " does not match" << peerAddress();
                        return false;
                    }
                }
                else
                {
                    qCritical() << "received tokenId" << msgTokenId << "not found!";
                    return false;
                }
            }
        }
    }

    return true;
}
