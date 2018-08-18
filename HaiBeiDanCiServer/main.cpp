#include "hbdcserver.h"

#include <QCoreApplication>
#include <QtNetwork>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    HBDCServer server;
    if (server.listen(QHostAddress::Any, 65315) == false)
    {
        QString msg = QObject::tr("Unable to start the server: %1.").arg(server.errorString());
        qDebug() << msg;
        return 1;
    }


    QString ipAddress;
    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    // use the first non-localhost IPv4 address
    for (int i = 0; i < ipAddressesList.size(); ++i) {
        if (ipAddressesList.at(i) != QHostAddress::LocalHost &&
            ipAddressesList.at(i).toIPv4Address()) {
            ipAddress = ipAddressesList.at(i).toString();
            break;
        }
    }
    // if we did not find one, use IPv4 localhost
    if (ipAddress.isEmpty())
        ipAddress = QHostAddress(QHostAddress::LocalHost).toString();

    qDebug() << "\n";
    qDebug() << "The server is running on" << "\n";
    qDebug() << "IP: " << ipAddress;
    qDebug() << "port: " << server.serverPort();

    return a.exec();
}
