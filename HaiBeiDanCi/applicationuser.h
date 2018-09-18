#ifndef APPLICATIONUSER_H
#define APPLICATIONUSER_H

#include "../golddict/sptr.hh"

#include <QString>
#include <QByteArray>
#include <QDataStream>

class ApplicationUser
{
    QString m_name;
    QByteArray m_password;
    QString m_email;
    qint32 m_id;

public:
    typedef enum
    {
        ResultRegisterOK = 0,
        ResultRegisterFailedNameAlreadyUsed = 1,
        ResultRegisterFailedNotAllowed = 2,   // server does not allow this client to register, probably that this client regists too often
        ResultRegisterFailedUnknown = 3,
        ResultRegisterFailedServerError = 4,
    } RegisterResult;

    typedef enum
    {
        ResultLoginOK = 0,
        ResultLoginFailedNameDoesNotExist = 1,
        ResultLoginFailedNotAllowed = 2,   // server does not allow this client to register, probably that this client regists too often
        ResultLoginFailedUnknown = 3,
        ResultLoginFailedServerError = 4,
        ResultLoginFailedIncorrectPassword = 5,
    } LoginResult;

    typedef enum
    {
        ResultLogoutOK = 0,
    } LogoutResult;

    static const ApplicationUser invalidUser;

    ApplicationUser(QString name, QString password, QString email, qint32 id = 0);
    ApplicationUser(QString name, const QByteArray &password, QString email, qint32 id);

    bool isValid() const
    {
        return m_name != invalidUser.name() && m_id != invalidUser.id();
    }

    qint32 id() const
    {
        return m_id;
    }

    void setId(qint32 id)
    {
        m_id = id;
    }

    QString name() const
    {
        return m_name;
    }

    const QByteArray & password() const
    {
        return m_password;
    }

    QString email() const
    {
        return m_email;
    }

    static bool createDatabaseTables();
    static bool userExist(QString name);
    static sptr<ApplicationUser> getUser(QString name);
    static bool createUser(ApplicationUser &user);
    static bool saveRegisteredUser(const ApplicationUser &user);
};


QDataStream &operator<<(QDataStream &ds, const ApplicationUser &appVer);
QDataStream &operator>>(QDataStream &ds, ApplicationUser &appVer);

#endif // APPLICATIONUSER_H
