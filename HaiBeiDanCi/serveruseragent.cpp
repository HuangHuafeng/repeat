#include "serveruseragent.h"
#include "mysettings.h"

ServerUserAgent::ServerUserAgent(ServerCommunicator *sc, QObject *parent) :
    QObject(parent),
    m_sc(sc)
{
    if (m_sc == nullptr)
    {
        m_sc = ServerCommunicator::instance();
    }

    connect(m_sc, SIGNAL(registerResult(qint32, const ApplicationUser &)), this, SLOT(onRegisterResult(qint32, const ApplicationUser &)));
    connect(m_sc, SIGNAL(loginResult(qint32, const ApplicationUser &, const Token &)), this, SLOT(onLoginResult(qint32, const ApplicationUser &, const Token &)));
    connect(m_sc, SIGNAL(logoutResult(qint32, QString)), this, SLOT(onLogoutResult(qint32, QString)));
}

void ServerUserAgent::onRegisterResult(qint32 result, const ApplicationUser &user)
{
    // emit signal here may cause problem?!
    // as there might be message box displayed in the slot connect to signal registerSucceeded/registerFailed
    // and it's possible that the user don't close the message box for 1 miutes or longer
    // in such case, it's possilbe to stop the connection to send heartbeat?!
    // NO! IT DOES NOT STOP sending the heartbeat
    // But it does not process the reply unless this function returns.
    // Looks like this is OK!!!

    if (result == ApplicationUser::ResultRegisterOK)
    {
        emit(registerResult(true, user, QString()));
    }
    else
    {
        QString why;
        switch (result) {
        case ApplicationUser::ResultRegisterFailedNotAllowed:
            why = QObject::tr("server does not allow to register");
            break;

        case ApplicationUser::ResultRegisterFailedNameAlreadyUsed:
            why = QObject::tr("name is already in use");
            break;

        case ApplicationUser::ResultRegisterFailedServerError:
            why = QObject::tr("server internal error");
            break;

        case ApplicationUser::ResultRegisterFailedUnknown:
        default:
            why = QObject::tr("unknown error");
            break;
        }

        emit(registerResult(false, user, why));
    }
}

void ServerUserAgent::onLoginResult(qint32 result, const ApplicationUser &user, const Token &token)
{
    if (result == ApplicationUser::ResultLoginOK)
    {
        emit(loginResult(true, user, token, QString()));
    }
    else
    {
        QString why;
        switch (result) {
        case ApplicationUser::ResultLoginFailedIncorrectPassword:
            why = QObject::tr("incorrect password");
            break;

        case ApplicationUser::ResultLoginFailedNameDoesNotExist:
            why = QObject::tr("user does not exist");
            break;

        case ApplicationUser::ResultLoginFailedNotAllowed:
        case ApplicationUser::ResultLoginFailedServerError:
        case ApplicationUser::ResultLoginFailedUnknown:
        default:
            why = QObject::tr("unknown error");
            break;
        }

        emit(loginResult(false, user, token, why));
    }
}

void ServerUserAgent::registerUser(QString name, QString password, QString email)
{
    ApplicationUser userToRegister(name, password, email);
    m_sc->sendRequestRegister(userToRegister);
}

void ServerUserAgent::loginUser(QString name, QString password)
{
    ApplicationUser tempUser(name, password, "a@b.com");
    auto user = ApplicationUser::getUser(name);
    if (user.get() == nullptr)
    {
        // we should allow this as this is possible!!!
        qDebug() << "user does not exist locally!";
    }
    else
    {
        if (user->password() != tempUser.password())
        {
            qDebug() << "password does not match locally!";
        }
    }

    m_sc->sendRequestLogin(tempUser);
}

void ServerUserAgent::logoutUser(QString name)
{
    m_sc->sendRequestLogout(name);
}

void ServerUserAgent::onLogoutResult(qint32 result, QString name)
{
    qDebug() << "logout result" << result;
    emit(logoutSucceeded(name));
}
