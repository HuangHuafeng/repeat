#include "serveruseragent.h"
#include "mysettings.h"

ServerUserAgent::ServerUserAgent(QObject *parent) : QObject(parent),
    m_svrAgt(MySettings::serverHostName(), MySettings::serverPort(), this)
{
    connect(&m_svrAgt, SIGNAL(registerResult(qint32, const ApplicationUser &)), this, SLOT(onRegisterResult(qint32, const ApplicationUser &)));
}

void ServerUserAgent::onRegisterResult(qint32 result, const ApplicationUser &user)
{
    if (result == ApplicationUser::ResultOK)
    {
        emit(registerSucceed(user));
    }
    else
    {
        QString why;
        switch (result) {
        case ApplicationUser::ResultFailedNotAllowed:
            why = QObject::tr("Server does not allow to register");
            break;

        case ApplicationUser::ResultFailedNameAlreadyUsed:
            why = QObject::tr("name is already in use");
            break;

        case ApplicationUser::ResultFailedUnknown:
        default:
            why = QObject::tr("Unknown error!");
            break;
        }

        emit(registerFailed(why));
    }
}

void ServerUserAgent::registerUser(QString name, QString password, QString email)
{
    ApplicationUser user(name, password, email);
    m_svrAgt.sendRequestRegister(user);
}
