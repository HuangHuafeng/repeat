#include "servermanager.h"

ServerManager::ServerManager(const QString &hostName, quint16 port, QObject *parent) :
    ServerAgent(hostName, port, parent)
{

}
