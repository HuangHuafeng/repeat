#ifndef SERVERCLIENTPROTOCOL_H
#define SERVERCLIENTPROTOCOL_H

#include <QString>
#include <QRegExp>
#include <QDataStream>

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
    static QString partPrefix(qint32 partNumber = -1)
    {
        if (partNumber == -1)
        {
            return "__PART__";
        }
        else
        {
            return "__PART__" + QString::number(partNumber) + "__";
        }
    }

    static QRegExp partPrefixReplaceRegExp()
    {
        return QRegExp("__PART__\\d+__");
    }

    typedef enum {
        MaximumWordsInAMessage = 2000,
        MaximumBytesForFileTransfer = 1024 * 32,    // 32k, most of the files are less than 20k in this app
    } MessageParameters;

    typedef enum {
        RequestNoOperation = 10000, // Yes
        RequestGetAllBooks = RequestNoOperation + 1,    // Yes, 1:1
        RequestGetAWord = RequestNoOperation + 2,   // Yes, 1:1
        RequestGetABook = RequestNoOperation + 3,   // Yes, 1:1
        RequestGetBookWordList = RequestNoOperation + 4, // Yes, n:1
        RequestGetFile = RequestNoOperation + 5,    // Yes, n:1
        RequestGetWordsOfBookFinished = RequestNoOperation + 6, // Yes, 1:1 // used to echo to the client to let it aware that downloading words finished
        RequestBye = RequestNoOperation + 7,    // Yes, 1:0
    } ClientToServerMessageCode;

    typedef enum {
        ResponseNoOperation = 20000,    // Yes
        ResponseGetAllBooks = ResponseNoOperation + 1,  // Yes
        ResponseGetAWord = ResponseNoOperation + 2, // Yes
        ResponseGetABook = ResponseNoOperation + 3, // Yes

        ResponseGetBookWordList = ResponseNoOperation + 4,   // Yes
        ResponseBookWordListAllSent = ResponseGetBookWordList + 1000,   // Yes

        ResponseGetFile = ResponseNoOperation + 5,  // Yes
        ResponseGetFileFinished = ResponseGetFile + 1000,

        ResponseGetWordsOfBookFinished = ResponseNoOperation + 6,   // Yes

        ResponseFailedToRequest = ResponseNoOperation + 9000,   // Yes
        ResponseUnknownRequest = ResponseNoOperation + 9001,    // Yes
    } ServerToClientMessageCode;

    typedef enum {
        ManagerToServerMessageCodeBase = 30000,
        RequestPromoteToManager = ManagerToServerMessageCodeBase + 1,
        RequestGetAllWordsWithoutDefinition = ManagerToServerMessageCodeBase + 2,
        RequestGetServerDataFinished = ManagerToServerMessageCodeBase + 3,
    } ManagerToServerMessageCode;

    typedef enum {
        ServerToManagerMessageCodeBase = 40000,
        ResponsePromoteToManager = ServerToManagerMessageCodeBase + 1,

        ResponseGetAllWordsWithoutDefinition = ServerToManagerMessageCodeBase + 2,
        ResponseGetAllWordsWithoutDefinitionFinished = ResponseGetAllWordsWithoutDefinition + 1000,

        ResponseGetServerDataFinished = ServerToManagerMessageCodeBase + 3,
    } ServerToManagerMessageCode;
};

class funcTracker
{
public:
    funcTracker(QString funcName) : m_funcName(funcName)
    {
        qInfo("funcTracker: entering %s", m_funcName.toLatin1().constData());
    }

    ~funcTracker()
    {
        qInfo("funcTracker: leaving %s", m_funcName.toLatin1().constData());
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

    static qint32 m_currentSequenceNumber;

public:
    MessageHeader(qint32 code = 0, qint32 respondsTo = 0, qint32 sequenceNumber = 0);
    MessageHeader(const QByteArray &msg);

    qint32 code() const;
    qint32 sequenceNumber() const;
    qint32 respondsTo() const;

    void setCode(qint32 code);
    void setSequenceNumber(qint32 sequenceNumber);
    void setRespondsTo(qint32 respondsTo);

    QString toString() const;
};

QDataStream &operator<<(QDataStream &ds, const MessageHeader &word);
QDataStream &operator>>(QDataStream &ds, MessageHeader &word);

#endif // SERVERCLIENTPROTOCOL_H
