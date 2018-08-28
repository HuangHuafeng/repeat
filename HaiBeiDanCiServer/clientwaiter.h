#ifndef CLIENTWAITER_H
#define CLIENTWAITER_H

#include "../HaiBeiDanCi/word.h"
#include "../HaiBeiDanCi/worddb.h"
#include "../HaiBeiDanCi/serverclientprotocol.h"
#include "../HaiBeiDanCi/wordbook.h"

#include <QThread>
#include <QTcpSocket>

//! [0]
class ClientWaiter : public QThread
{
    Q_OBJECT

public:
    ClientWaiter(qintptr socketDescriptor, QObject *parent);

    void run() override;

signals:
    void error(QTcpSocket::SocketError socketError);

private:
    qintptr m_socketDescriptor;
    QTcpSocket *m_tcpSocket;

    void disconnectPeer();
    bool waitForMoreData();

    sptr<MessageHeader> readMessageHeader();
    int handleMessage(const MessageHeader &msgHeader);
    void handleUnknownMessage(const MessageHeader &msgHeader);

    bool handleRequestNoOperation(const MessageHeader &msgHeader);
    bool handleRequestGetAllBooks(const MessageHeader &msgHeader);
    bool handleRequestGetBookWordList(const MessageHeader &msgHeader);
    bool handleRequestGetWordsOfBookFinished(const MessageHeader &msgHeader);
    bool handleRequestGetAWord(const MessageHeader &msgHeader);
    bool handleRequestGetABook(const MessageHeader &msgHeader);
    bool handleRequestGetFile(const MessageHeader &msgHeader);

    void sendBookWordList(const MessageHeader &msgHeader, const QString bookName);
    bool sendFile(const MessageHeader &msgHeader, const QString fileName);
    bool okToSendFile(const QString fileName);

    // the following functions all send ONE message
    void sendResponseNoOperation(const MessageHeader &msgHeader);
    void sendResponseGetAllBooks(const MessageHeader &msgHeader, const QList<QString> &books);
    void sendResponseGetAWord(const MessageHeader &msgHeader, const Word &word);
    void sendResponseGetABook(const MessageHeader &msgHeader, const WordBook &book);
    void sendResponseGetBookWordList(const MessageHeader &msgHeader, const QString bookName, const QVector<QString> &wordList);
    void sendResponseBookWordListAllSent(const MessageHeader &msgHeader, const QString bookName);
    void sendResponseGetFileFinished(const MessageHeader &msgHeader, const QString fileName, bool succeeded);
    void sendResponseUnknownRequest(const MessageHeader &msgHeader);
    void sendResponseGetFile(const MessageHeader &msgHeader, const QString fileName, const char *s, uint len);
    void sendResponseGetWordsOfBookFinished(const MessageHeader &msgHeader, const QString bookName);

};

#endif // CLIENTWAITER_H
