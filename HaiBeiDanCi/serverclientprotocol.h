#ifndef SERVERCLIENTPROTOCOL_H
#define SERVERCLIENTPROTOCOL_H

class ServerClientProtocol
{
public:
    typedef enum {
        RequestNoOperation = 10000,
        RequestGetAllBooks = RequestNoOperation + 1,
        RequestGetWordsOfBook = RequestNoOperation + 2,
        RequestGetWords = RequestNoOperation + 3,
        RequestGetAWord = RequestNoOperation + 4,
        RequestGetABook = RequestNoOperation + 5,
        RequestBye = RequestNoOperation + 1000,
    } RequestCode;

    typedef enum {
        ResponseNoOperation = 20000,
        ResponseGetAllBooks = ResponseNoOperation + 1,
        ResponseGetWordsOfBook = ResponseNoOperation + 2,
        ResponseGetWords = ResponseNoOperation + 3,
        ResponseGetAWord = ResponseNoOperation + 4,
        ResponseGetABook = ResponseNoOperation + 5,
        ResponseFailedToRequest = ResponseNoOperation + 1000,
        ResponseUnknownRequest = ResponseNoOperation + 1001,
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
 ****/

/****
 * Response format:
 * GetAllBooksResponse: ResponseCode + list of names
 * GetWordsOfBookResponse: ResponseCode + book name + list of spellings
 * GetWordResponse: ResponseCode + list of Words
 * ResponseFailedToRequest: ResponseCode + RequestCode
 * ResponseGetAWord: ResponseCode + id + spelling + definition
 ****/

#endif // SERVERCLIENTPROTOCOL_H
