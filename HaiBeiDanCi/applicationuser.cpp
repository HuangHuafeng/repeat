#include "applicationuser.h"
#include "worddb.h"

#include <QCryptographicHash>

const ApplicationUser ApplicationUser::invalidUser = ApplicationUser("__INVALID__", QByteArray("__INVALID__"), "__INVALID__", -1);

ApplicationUser::ApplicationUser(QString name, QString password, QString email, qint32 id) :
    m_name(name),
    m_email(email),
    m_id(id)
{
    m_password = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Md5);
}

ApplicationUser::ApplicationUser(QString name, const QByteArray &password, QString email, qint32 id) :
    m_name(name),
    m_password(password),
    m_email(email),
    m_id(id)
{
}

bool ApplicationUser::createDatabaseTables()
{
    auto ptrQuery = WordDB::createSqlQuery();
    if (ptrQuery.get() == nullptr)
    {
        return false;
    }
    auto query = *ptrQuery;
    if (query.exec("SELECT * FROM users LIMIT 1") == false)
    {
        // table "users" does not exist
        if (query.exec("CREATE TABLE users (id INTEGER primary key, "
                       "name TEXT, "
                       "password BLOB, "
                       "email TEXT)") == false)
        {
            WordDB::databaseError(query, "creating table \"users\"");
            return false;
        }
    }

    return true;
}

bool ApplicationUser::userExist(QString name)
{
    return ApplicationUser::getUser(name).get() != nullptr;
}

sptr<ApplicationUser> ApplicationUser::getUser(QString name)
{
    auto ptrQuery = WordDB::createSqlQuery();
    if (ptrQuery.get() == nullptr)
    {
        return sptr<ApplicationUser>();
    }

    auto query = *ptrQuery;
    query.prepare(" SELECT *"
                  " FROM users"
                  " WHERE name=:name");
    query.bindValue(":name", name);
    sptr<ApplicationUser> user = sptr<ApplicationUser>();
    if (query.exec())
    {
        if (query.first())
        {
            qint32 id = query.value("id").toInt();
            QByteArray password = query.value("password").toByteArray();
            QString email = query.value("email").toString();
            user = new ApplicationUser(name, password, email, id);
        }
    }
    else
    {
        WordDB::databaseError(query, "fetching user with name \"" + name + "\"");
    }

    return user;
}

/**
 * @brief ApplicationUser::createUser
 * @param user
 * @return
 * createUser should ONLY be called in the server code
 */
bool ApplicationUser::createUser(ApplicationUser &user)
{
    Q_ASSERT(ApplicationUser::userExist(user.name()) == false);
    Q_ASSERT(user.id() == 0);

    auto ptrQuery = WordDB::createSqlQuery();
    if (ptrQuery.get() == nullptr)
    {
        return false;
    }

    auto query = *ptrQuery;
    query.prepare("INSERT INTO users(name, password, email) VALUES(:name, :password, :email)");
    query.bindValue(":name", user.name());
    query.bindValue(":password", user.password());
    query.bindValue(":email", user.email());
    if (query.exec() == false)
    {
        WordDB::databaseError(query, "insert user \"" + user.name() + "\"");
        return false;
    }

    user.setId(query.lastInsertId().toInt());

    return true;
}

/**
 * @brief ApplicationUser::saveRegisteredUser
 * @param user
 * @return
 * used the the app to save a user locally when the user successfully registered
 */
bool ApplicationUser::saveRegisteredUser(const ApplicationUser &user)
{
    Q_ASSERT(ApplicationUser::userExist(user.name()) == false);
    Q_ASSERT(user.id() != 0);

    auto ptrQuery = WordDB::createSqlQuery();
    if (ptrQuery.get() == nullptr)
    {
        return false;
    }

    auto query = *ptrQuery;
    query.prepare("INSERT INTO users(id, name, password, email) VALUES(:id, :name, :password, :email)");
    query.bindValue(":id", user.id());
    query.bindValue(":name", user.name());
    query.bindValue(":password", user.password());
    query.bindValue(":email", user.email());
    if (query.exec() == false)
    {
        WordDB::databaseError(query, "insert user \"" + user.name() + "\"");
        return false;
    }

    return true;
}

QDataStream &operator<<(QDataStream &ds, const ApplicationUser &appUser)
{
    ds << appUser.id() << appUser.name() << appUser.password() << appUser.email();
    return ds;
}

QDataStream &operator>>(QDataStream &ds, ApplicationUser &appUser)
{
    qint32 id;
    QString name;
    QByteArray password;
    QString email;
    ds >> id >> name >> password >> email;
    appUser = ApplicationUser(name, password, email, id);
    return ds;
}
