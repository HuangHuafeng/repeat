#ifndef HBDCSERVER_H
#define HBDCSERVER_H

#include <QStringList>
#include <QTcpServer>
#include <QTimer>

//! [0]
class HBDCServer : public QTcpServer
{
    Q_OBJECT

public:
    HBDCServer(QObject *parent = nullptr);
    bool listen(const QHostAddress &address = QHostAddress::Any, quint16 port = 0);

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private slots:
    void onScheduleDelete();

private:
    QTimer m_deleteScheduler;
};

#endif // HBDCSERVER_H
