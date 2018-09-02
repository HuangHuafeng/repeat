#ifndef SERVERMANAGER_H
#define SERVERMANAGER_H

#include "HaiBeiDanCi/serveragent.h"

class ServerManager : public ServerAgent
{
public:
    ServerManager(const QString &hostName, quint16 port = 61027, QObject *parent = nullptr);
};

#endif // SERVERMANAGER_H
