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

    int readMessageCode();
    int handleMessage(int messageCode);

    bool handleRequestNoOperation();
    bool handleRequestGetAllBooks();
    bool handleRequestGetWordsOfBook();
    bool handleRequestGetWords();
    bool handleRequestGetAWord();
    bool handleRequestGetABook();
    bool handleRequestGetFile();

    void sendWordsOfBook(const QString bookName);
    void sendFile(const QString fileName);

    // the following functions all send ONE message
    void sendResponseNoOperation();
    void sendResponseGetAllBooks(const QList<QString> &books);
    void sendResponseGetAWord(const Word &word);
    void sendResponseGetABook(const WordBook &book);
    void sendResponseGetWordsOfBook(const QString bookName, const QVector<QString> &wordList);
    void sendResponseAllDataSent(int messageCode);
    void sendResponseAllDataSentForRequestGetWordsOfBook(const QString bookName);
    void sendResponseAllDataSentForRequestGetWords(const QString bookName);
    void sendResponseAllDataSentForRequestGetFile(const QString fileName, bool errorHappened);
    void sendResponseUnknownRequest(int messageCode);
    void sendResponseGetFile(const QString fileName, const char *s, uint len);

};

#endif // CLIENTWAITER_H
