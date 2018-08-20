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

    typedef enum {
        MaximumWordsInAMessage = 25000,
    } MessageParameters;

signals:
    void error(QTcpSocket::SocketError socketError);

private:
    qintptr m_socketDescriptor;
    QTcpSocket *m_tcpSocket;

    void disconnectPeer();
    void failedToHandleMessage(int messageCode);

    int readMessageCode();
    bool handleMessage(int messageCode);

    bool handleRequestGetAllBooks();
    bool handleRequestGetWordsOfBook();
    bool handleRequestGetAWord();
    bool handleRequestGetABook();

    void sendWordsOfBook(const QString bookName);

    // the following functions all send ONE message
    void sendResponseGetAllBooks(const QList<QString> &books);
    void sendResponseGetAWord(const Word &word);
    void sendResponseGetABook(const WordBook &book);
    void sendResponseGetWordsOfBook(const QString bookName, const QVector<QString> &wordList);
    void sendResponseAllDataSent(int messageCode);

};

#endif // CLIENTWAITER_H
