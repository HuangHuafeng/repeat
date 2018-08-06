#ifndef DATABASEOBJECT_H
#define DATABASEOBJECT_H

/**
 * @brief The DatabaseObject class
 * The main purpose is to provide a way of lazy update from database
 */

#include "../golddict/sptr.hh"
#include "worddb.h"

#include <QSqlQuery>

class DatabaseObject
{
public:
    DatabaseObject();
    virtual ~DatabaseObject();

    bool hasUpdatedFromDatabase() const {
        return m_updated;
    }

protected:
    virtual void updateFromDatabase();

private:
    bool m_updated;
};

#endif // DATABASEOBJECT_H
