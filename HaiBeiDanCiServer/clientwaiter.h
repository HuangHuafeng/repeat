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

    void sendMessage(QByteArray msg, bool = false);

    sptr<MessageHeader> readMessageHeader();
    QByteArray readMessage();
    int handleMessage(const QByteArray &msg);
    void handleUnknownMessage(const QByteArray &msg);

    bool handleRequestNoOperation(const QByteArray &msg);
    bool handleRequestGetAllBooks(const QByteArray &msg);
    bool handleRequestGetBookWordList(const QByteArray &msg);
    bool handleRequestGetWordsOfBookFinished(const QByteArray &msg);
    bool handleRequestGetAWord(const QByteArray &msg);
    bool handleRequestGetABook(const QByteArray &msg);
    bool handleRequestGetFile(const QByteArray &msg);

    void sendBookWordList(const QByteArray &msg, const QString bookName);
    bool sendFile(const QByteArray &msg, const QString fileName);
    bool okToSendFile(const QString fileName);

    // the following functions all send ONE message
    void sendResponseNoOperation(const QByteArray &msg);
    void sendResponseGetAllBooks(const QByteArray &msg, const QList<QString> &books);
    void sendResponseGetAWord(const QByteArray &msg, const Word &word);
    void sendResponseGetABook(const QByteArray &msg, const WordBook &book);
    void sendResponseGetBookWordList(const QByteArray &msg, const QString bookName, const QVector<QString> &wordList);
    void sendResponseBookWordListAllSent(const QByteArray &msg, const QString bookName);
    void sendResponseGetFileFinished(const QByteArray &msg, const QString fileName, bool succeeded);
    void sendResponseUnknownRequest(const QByteArray &msg);
    void sendResponseGetFile(const QByteArray &msg, const QString fileName, const char *s, uint len);
    void sendResponseGetWordsOfBookFinished(const QByteArray &msg, const QString bookName);

};

#endif // CLIENTWAITER_H
