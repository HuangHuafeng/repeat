#include "databaseobject.h"

DatabaseObject::DatabaseObject() :
    m_updated(false)
{

}

DatabaseObject::~DatabaseObject()
{

}

void DatabaseObject::updateFromDatabase()
{
    m_updated = true;
}
