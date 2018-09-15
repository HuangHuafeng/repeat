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
        ResultOK = 0,
        ResultFailedNameAlreadyUsed = 1,
        ResultFailedNotAllowed = 2,   // server does not allow this client to register, probably that this client regists too often
        ResultFailedUnknown = 3,
    } RegisterResult;

    static const ApplicationUser invalidUser;

    ApplicationUser(QString name, QString password, QString email, qint32 id = 0);
    ApplicationUser(QString name, const QByteArray &password, QString email, qint32 id);

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
};


QDataStream &operator<<(QDataStream &ds, const ApplicationUser &appVer);
QDataStream &operator>>(QDataStream &ds, ApplicationUser &appVer);

#endif // APPLICATIONUSER_H
