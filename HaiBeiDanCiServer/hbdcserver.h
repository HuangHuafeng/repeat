#ifndef HBDCSERVER_H
#define HBDCSERVER_H

#include <QStringList>
#include <QTcpServer>

//! [0]
class HBDCServer : public QTcpServer
{
    Q_OBJECT

public:
    HBDCServer(QObject *parent = nullptr);

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private:
};

#endif // HBDCSERVER_H
