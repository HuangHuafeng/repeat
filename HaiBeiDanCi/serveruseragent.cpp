#include "serveruseragent.h"
#include "mysettings.h"

ServerUserAgent::ServerUserAgent(QObject *parent) : QObject(parent),
    m_svrAgt(MySettings::serverHostName(), MySettings::serverPort(), this)
{
    connect(&m_svrAgt, SIGNAL(registerResult(qint32, const ApplicationUser &)), this, SLOT(onRegisterResult(qint32, const ApplicationUser &)));
    connect(&m_svrAgt, SIGNAL(loginResult(qint32, const ApplicationUser &, const Token &)), this, SLOT(onLoginResult(qint32, const ApplicationUser &, const Token &)));
    connect(&m_svrAgt, SIGNAL(logoutResult(qint32, QString)), this, SLOT(onLogoutResult(qint32, QString)));
}

void ServerUserAgent::onRegisterResult(qint32 result, const ApplicationUser &user)
{
    // emit signal here may cause problem?!
    // as there might be message box displayed in the slot connect to signal registerSucceeded/registerFailed
    // and it's possible that the user don't close the message box for 1 miutes or longer
    // in such case, it's possilbe to stop the svrAgt to send heartbeat?!
    // NO! IT DOES NOT STOP sending the heartbeat
    // But it does not process the reply unless this function returns.
    // Looks like this is OK!!!

    if (result == ApplicationUser::ResultRegisterOK)
    {
        emit(registerSucceeded(user));
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

        emit(registerFailed(why));
    }
}

void ServerUserAgent::onLoginResult(qint32 result, const ApplicationUser &user, const Token &token)
{
    if (result == ApplicationUser::ResultLoginOK)
    {
        emit(loginSucceeded(user, token));
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

        emit(registerFailed(why));
    }
}

void ServerUserAgent::registerUser(QString name, QString password, QString email)
{
    auto user = ApplicationUser::getUser(name);
    if (user.get() != nullptr)
    {
        emit(registerFailed(QObject::tr("user already exists locally")));
        return;
    }

    ApplicationUser userToRegister(name, password, email);
    m_svrAgt.sendRequestRegister(userToRegister);
}

void ServerUserAgent::loginUser(QString name, QString password)
{
    auto user = ApplicationUser::getUser(name);
    if (user.get() == nullptr)
    {
        emit(loginFailed(QObject::tr("user does not exist")));
        return;
    }

    ApplicationUser tempUser(name, password, "__INVALID__");

    if (user->password() != tempUser.password())
    {
        emit(loginFailed(QObject::tr("incorrect password")));
        return;
    }

    m_svrAgt.sendRequestLogin(*user);
}

void ServerUserAgent::logoutUser(QString name)
{
    m_svrAgt.sendRequestLogout(name);
}

void ServerUserAgent::onLogoutResult(qint32 result, QString name)
{
    qDebug() << "logout result" << result;
    emit(logoutSucceeded(name));
}
