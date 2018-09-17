#ifndef HBDCAPPHANDLER_H
#define HBDCAPPHANDLER_H

#include "clienthandler.h"
#include "../HaiBeiDanCi/applicationuser.h"

class HBDCAppHandler : public ClientHandler
{
public:
    HBDCAppHandler(ClientWaiter &clientWaiter);

    virtual int handleMessage(const QByteArray &msg) override;

private:
    bool handleRequestGetAllBooks(const QByteArray &msg);
    bool handleRequestGetBookWordList(const QByteArray &msg);
    bool handleRequestGetWordsOfBookFinished(const QByteArray &msg);
    bool handleRequestGetAWord(const QByteArray &msg);
    bool handleRequestGetABook(const QByteArray &msg);
    bool handleRequestGetFile(const QByteArray &msg);
    bool handleRequestRegister(const QByteArray &msg);
    bool handleRequestLogin(const QByteArray &msg);

    bool registerUser(const QByteArray &msg, ApplicationUser &user);
    bool loginUser(const QByteArray &msg, ApplicationUser &user);
    bool validateUser(const ApplicationUser &user);

    void sendBookWordList(const QByteArray &msg, QString bookName);
    bool sendFile(const QByteArray &msg, QString fileName);
    bool okToSendFile(QString fileName);

    void sendResponseGetAllBooks(const QByteArray &msg, const QList<QString> &books);
    void sendResponseGetAWord(const QByteArray &msg, const Word &word);
    void sendResponseGetABook(const QByteArray &msg, const WordBook &book);
    void sendResponseGetBookWordList(const QByteArray &msg, QString bookName, const QVector<QString> &wordList, bool listComplete);
    void sendResponseGetFileFinished(const QByteArray &msg, QString fileName, bool succeeded);
    void sendResponseGetFile(const QByteArray &msg, QString fileName, const char *s, uint len);
    void sendResponseGetWordsOfBookFinished(const QByteArray &msg, QString bookName);

    void sendResponseRegister(const QByteArray &msg, qint32 result, const ApplicationUser &user);
    void sendResponseLogin(const QByteArray &msg, qint32 result, const ApplicationUser &user);
};

#endif // HBDCAPPHANDLER_H
