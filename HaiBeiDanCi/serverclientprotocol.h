#ifndef SERVERCLIENTPROTOCOL_H
#define SERVERCLIENTPROTOCOL_H

#include "applicationversion.h"

#include <QString>
#include <QDataStream>
#include <QDateTime>

/**
 * a message between the server and the client should:
 * 1. be small, so it can be read in easilier, no risk to overflow the socket buffer
 *
 * CORRECTION:
 * it seems size of message is NOT an issue. Although it should be taken care of.
 * We should be careful/aware that message is NOT always available as a whole, so we
 * need to wait for more data in case we failed to read the message contents.
 */

class ServerClientProtocol
{
public:
    typedef enum {
        MaximumWordsInAMessage = 10000,
        MaximumBytesForFileTransfer = 1024 * 32,    // 32k, most of the files are less than 20k in this app
    } MessageParameters;

    typedef enum {
        RequestNoOperation = 10000, // Yes
        RequestGetAllBooks = RequestNoOperation + 1,    // Yes, 1:1
        RequestGetAWord = RequestNoOperation + 2,   // Yes, 1:1
        RequestGetABook = RequestNoOperation + 3,   // Yes, 1:1
        RequestGetBookWordList = RequestNoOperation + 4, // Yes, n:1

        RequestGetFile = RequestNoOperation + 5,    // Yes, n:1
        RequestGetAppFile = RequestGetFile + 2000,    // functions same as RequestGetFile, but server will not check the token
        RequestGetUpgrader = RequestGetFile + 2001,    // functions same as RequestGetFile, but server will not check the token

        RequestGetWordsOfBookFinished = RequestNoOperation + 6, // Yes, 1:1 // used to echo to the client to let it aware that downloading words finished
        RequestBye = RequestNoOperation + 7,    // Yes, 1:0

        RequestAppVersion = RequestNoOperation + 8,

        RequestLogin = RequestNoOperation + 9,
        RequestRegister = RequestNoOperation + 10,
        RequestLogout = RequestNoOperation + 11,
        RequestUpgraderVersion = RequestNoOperation + 12,
    } ClientToServerMessageCode;

    typedef enum {
        ResponseNoOperation = 20000,    // Yes
        ResponseGetAllBooks = ResponseNoOperation + 1,  // Yes
        ResponseGetAWord = ResponseNoOperation + 2, // Yes
        ResponseGetABook = ResponseNoOperation + 3, // Yes
        ResponseGetBookWordList = ResponseNoOperation + 4,   // Yes

        ResponseGetFile = ResponseNoOperation + 5,  // Yes
        ResponseGetFileFinished = ResponseGetFile + 1000,

        ResponseGetWordsOfBookFinished = ResponseNoOperation + 6,   // Yes

        ResponseAppVersion = ResponseNoOperation + 8,

        ResponseLogin = ResponseNoOperation + 9,
        ResponseRegister = ResponseNoOperation + 10,
        ResponseLogout = ResponseNoOperation + 11,
        ResponseUpgraderVersion = ResponseNoOperation + 12,

        // ResponseOK is useful when there's no data need to be sent, just to let the client know that the server got your message
        ResponseOK = ResponseNoOperation + 8888,
        ResponseInvalidTokenId = ResponseNoOperation + 8889,

        ResponseFailedToRequest = ResponseNoOperation + 9000,   // Yes
        ResponseUnknownRequest = ResponseNoOperation + 9001,    // Yes
    } ServerToClientMessageCode;

    typedef enum {
        ManagerToServerMessageCodeBase = 30000,
        RequestPromoteToManager = ManagerToServerMessageCodeBase + 1,
        RequestGetAllWordsWithoutDefinition = ManagerToServerMessageCodeBase + 2,
        RequestGetServerDataFinished = ManagerToServerMessageCodeBase + 3,
        RequestDeleteABook = ManagerToServerMessageCodeBase + 4,
        RequestUploadABook = ManagerToServerMessageCodeBase + 5,    // not used, uploading a book uses Response* messages
        RequestMissingMediaFiles = ManagerToServerMessageCodeBase + 6,    // 1:n
        RequestUploadAFile = ManagerToServerMessageCodeBase + 7,
        RequestUploadAFileFinished = RequestUploadAFile + 1000,
        RequestUploadAWord = ManagerToServerMessageCodeBase + 8,    // not used, uploading a word uses Response* messages
        RequestReleaseApp = ManagerToServerMessageCodeBase + 9,
        RequestReleaseUpgrader = ManagerToServerMessageCodeBase + 10,
    } ManagerToServerMessageCode;

    typedef enum {
        ServerToManagerMessageCodeBase = 40000,
        ResponsePromoteToManager = ServerToManagerMessageCodeBase + 1,
        ResponseGetAllWordsWithoutDefinition = ServerToManagerMessageCodeBase + 2,
        ResponseGetServerDataFinished = ServerToManagerMessageCodeBase + 3,
        ResponseDeleteABook = ServerToManagerMessageCodeBase + 4,
        ResponseUploadABook = ServerToManagerMessageCodeBase + 5,
        ResponseMissingMediaFiles = ServerToManagerMessageCodeBase + 6,   // n:1
        ResponseUploadAFile = ServerToManagerMessageCodeBase + 7,
        ResponseUploadAFileFinished = ResponseUploadAFile + 1000,
        ResponseUploadAWord = ServerToManagerMessageCodeBase + 8,
        ResponseReleaseApp = ServerToManagerMessageCodeBase + 9,
        ResponseReleaseUpgrader = ServerToManagerMessageCodeBase + 10,
    } ServerToManagerMessageCode;
};

class funcTracker
{
public:
    funcTracker(QString funcName) : m_funcName(funcName)
    {
        qDebug("funcTracker: entering %s", m_funcName.toLatin1().constData());
    }

    ~funcTracker()
    {
        qDebug("funcTracker: leaving %s", m_funcName.toLatin1().constData());
    }

private:
    QString m_funcName;
};

class MessageHeader
{
    qint32 m_code;
    qint32 m_sequenceNumber;  // sequence number of this message
    qint32 m_respondsTo;  // this message responds to the message with sequence number "respondsTo" from the peer
    //qint32 m_version;     // version of the message, not needed yet
    QString m_tokenId;

    static qint32 m_currentSequenceNumber;

public:
    static const MessageHeader invalidMessageHeader;
    MessageHeader(qint32 code = 0, qint32 respondsTo = 0, qint32 sequenceNumber = 0, QString tokenId = QString());
    MessageHeader(const QByteArray &msg);

    qint32 code() const;
    qint32 sequenceNumber() const;
    qint32 respondsTo() const;
    QString tokenId() const;

    void setCode(qint32 code);
    void setSequenceNumber(qint32 sequenceNumber);
    void setRespondsTo(qint32 respondsTo);
    void setTokenId(QString tokenId);

    QString toString() const;
};

QDataStream &operator<<(QDataStream &ds, const MessageHeader &msgHead);
QDataStream &operator>>(QDataStream &ds, MessageHeader &msgHead);

class ReleaseInfo {
public:
    QString object;
    ApplicationVersion version;
    QString platform;
    QString fileName;
    QString info;
    QDateTime releaseTime;

    ReleaseInfo() : version(0, 0, 0)
    {
    }
};

QDataStream &operator<<(QDataStream &ds, const ReleaseInfo &ri);
QDataStream &operator>>(QDataStream &ds, ReleaseInfo &ri);

#endif // SERVERCLIENTPROTOCOL_H
