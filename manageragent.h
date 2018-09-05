#ifndef MANAGERAGENT_H
#define MANAGERAGENT_H

#include "HaiBeiDanCi/svragt.h"

class ManagerAgent : public SvrAgt
{
    Q_OBJECT

public:
    ManagerAgent(const QString &hostName, quint16 port = 61027, QObject *parent = nullptr);
    virtual ~ManagerAgent() override;

    void sendRequestPromoteToManager();
    void sendRequestGetAllWordsWithoutDefinition();
    void sendRequestGetServerDataFinished();

signals:
    void getAllWordsWithoutDefinitionFinished(const QVector<QString> &spellings, const QVector<int> &ids, const QVector<int> &definitionLengths);
    void getServerDataFinished();

protected:
    virtual int handleMessage(const QByteArray &msg) override;    

private:
    QVector<QString> m_spellingList;
    QVector<int> m_idList;
    QVector<int> m_definitionLengthList;

    bool handleResponsePromoteToManager(const QByteArray &msg);
    bool handleResponseGetAllWordsWithoutDefinition(const QByteArray &msg);
    bool handleResponseGetAllWordsWithoutDefinitionFinished(const QByteArray &msg);
    bool handleResponseGetServerDataFinished(const QByteArray &msg);
};

#endif // MANAGERAGENT_H
