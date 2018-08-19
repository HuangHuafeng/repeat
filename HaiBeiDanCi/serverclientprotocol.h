#ifndef SERVERCLIENTPROTOCOL_H
#define SERVERCLIENTPROTOCOL_H

class ServerClientProtocol
{
public:
    typedef enum {
        RequestNoOperation = 10000,
        RequestBye = RequestNoOperation + 1,
        RequestGetAllBooks = RequestNoOperation + 2,
        RequestGetWordsOfBook = RequestNoOperation + 3,
        RequestGetWord = RequestNoOperation + 4,
    } RequestCode;

    typedef enum {
        ResponseNoOperation = 20000,
        ResponseUnknownRequest = ResponseNoOperation + 1,
        ResponseGetAllBooks = ResponseNoOperation + 2,
        ResponseGetWordsOfBook = ResponseNoOperation + 3,
        ResponseGetWord = ResponseNoOperation + 4,
        ResponseFailedToRequest = ResponseNoOperation + 5,
    } ResponseCode;
};

/****
 * Request format:
 * NoOperation: RequestCode
 * Bye: RequestCode
 * GetAllBooks: RequestCode
 * GetWordsOfBook: RequestCode + book name
 * GetWord: RequestCode + list of spellings
 ****/

/****
 * Response format:
 * GetAllBooksResponse: ResponseCode + list of names
 * GetWordsOfBookResponse: ResponseCode + book name + list of spellings
 * GetWordResponse: ResponseCode + list of Words
 * ResponseFailedToRequest: ResponseCode + RequestCode
 ****/

#endif // SERVERCLIENTPROTOCOL_H
