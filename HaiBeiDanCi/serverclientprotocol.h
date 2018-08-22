#ifndef SERVERCLIENTPROTOCOL_H
#define SERVERCLIENTPROTOCOL_H

#include <QString>
#include <QRegExp>

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
    static QString partPrefix(int partNumber = -1)
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
        MaximumWordsInAMessage = 500,
        MaximumBytesForFileTransfer = 1024 * 1,    // 50k
    } MessageParameters;

    typedef enum {
        RequestNoOperation = 10000,
        RequestGetAllBooks = RequestNoOperation + 1,
        RequestGetWordsOfBook = RequestNoOperation + 2,     // should be careful, not too big
        RequestGetWords = RequestNoOperation + 3,
        RequestGetAWord = RequestNoOperation + 4,
        RequestGetABook = RequestNoOperation + 5,
        RequestGetFile = RequestNoOperation + 6,
        RequestBye = RequestNoOperation + 1000,
    } RequestCode;

    typedef enum {
        ResponseNoOperation = 20000,
        ResponseGetAllBooks = ResponseNoOperation + 1,
        ResponseGetWordsOfBook = ResponseNoOperation + 2, // Resp:Req is n:1
        ResponseGetWords = ResponseNoOperation + 3,
        ResponseGetAWord = ResponseNoOperation + 4,
        ResponseGetABook = ResponseNoOperation + 5,
        ResponseGetFile = ResponseNoOperation + 6,
        ResponseFailedToRequest = ResponseNoOperation + 1000,
        ResponseUnknownRequest = ResponseNoOperation + 1001,
        ResponseAllDataSent = ResponseNoOperation + 1002,
    } ResponseCode;
};

/****
 * Request format:
 * NoOperation: RequestCode
 * Bye: RequestCode
 * GetAllBooks: RequestCode
 * GetWordsOfBook: RequestCode + book name
 * GetWord: RequestCode + list of spellings
 * RequestGetAWord: RequestCode + spelling
 * RequestGetABook: RequestCode + book name
 ****/

/****
 * Response format:
 * GetAllBooksResponse: ResponseCode + list of names
 * GetWordsOfBookResponse: ResponseCode + book name + list of spellings
 * GetWordResponse: ResponseCode + list of Words
 * ResponseFailedToRequest: ResponseCode + RequestCode
 * ResponseGetAWord: ResponseCode + (Word serialization)
 * ResponseGetABook: ResponseCode + (WordBook serialization)
 * ResponseAllDataSent: ResponseCode + RequestCode
 ****/

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

#endif // SERVERCLIENTPROTOCOL_H
