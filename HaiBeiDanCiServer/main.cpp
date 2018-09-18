#include "hbdcserver.h"
#include "../HaiBeiDanCi/worddb.h"
#include "../HaiBeiDanCi/word.h"
#include "../HaiBeiDanCi/wordbook.h"
#include "../HaiBeiDanCi/mysettings.h"
#include "../HaiBeiDanCi/mediafilemanager.h"

#include <QCoreApplication>
#include <QtNetwork>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    //MySettings::setAppName("HaiBeiDanCiServer");

    // use the setting from HaiBeiDanCi if it's in the same computer
    QCoreApplication::setOrganizationName(MySettings::orgName());
    QCoreApplication::setOrganizationDomain(MySettings::orgDomain());
    QCoreApplication::setApplicationName(MySettings::appName());

    // this should be after the above 3 lines!!!
    //MySettings::saveDataDirectory("/Users/huafeng/Documents/GitHub/HaiBeiDanCiServerData");

    if (WordDB::initialize() == false)
    {
        qCritical("failed to load data from database!");
        return 1;
    }

    // call MediaFileManager::instance() to get the existing file list ready
    MediaFileManager::instance();

    HBDCServer server;
    if (server.listen() == false)
    {
        QString msg = QObject::tr("Unable to start the server: %1.").arg(server.errorString());
        qCritical("%s", msg.toUtf8().constData());
        return 2;
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

    qCritical() << endl << "Data directory:" << MySettings::dataDirectory();
    qCritical() << "Words:" << Word::getAllWords().size();
    qCritical() << "Books:" << WordBook::getAllBooks().size();

    return a.exec();
}
