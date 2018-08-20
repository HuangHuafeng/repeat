#include "serveragent.h"
#include "serverclientprotocol.h"

ServerAgent * ServerAgent::m_serveragent = nullptr;

ServerAgent::ServerAgent(const QString &hostName, quint16 port, QObject *parent) : QObject(parent),
    m_serverHostName(hostName),
    m_serverPort(port),
    m_tcpSocket(nullptr)
{
}

ServerAgent::~ServerAgent()
{
}

ServerAgent * ServerAgent::instance()
{
    if (m_serveragent == nullptr)
    {
        m_serveragent = new ServerAgent("huafengsmac");
    }

    return m_serveragent;
}


/**
 * @brief ServerAgent::onReadyRead
 * CORRECTION:
 * it seems size of message is NOT an issue. Although it should be taken care of.
 * We should be careful/aware that message is NOT always available as a whole, so we
 * need to wait for more data in case we failed to read the message contents.
 */
void ServerAgent::onReadyRead()
{
    static int currentMessage = 0;

    do {
        if (currentMessage == 0)
        {
            // last message processed completed, it's time for a new message
            currentMessage = readMessageCode();
        }

        if (currentMessage != 0)
        {
            // we have a message, try to process it
            int handleResult = handleMessage(currentMessage);
            if (handleResult == 0)
            {
                // successfully processed the message
                qDebug() << "successfully handled message with code" << currentMessage;
                currentMessage = 0;
            }
            else if (handleResult == 1)
            {
                qDebug() << "failed to handle message with code" << currentMessage;

                // failed to process the message, probably means the content of the message is NOT fully available
                // so quit the loop and contine in next call of onReadyRead()
                break;
            }
            else
            {
                // handleResult == -1, unknown message!
                // discard the message and continue trying to get the next message
                currentMessage = 0;
            }
        }
        else
        {
            // no messages available, quit the loop
            break;
        }
    } while (1);
}

void ServerAgent::onConnected()
{
    qDebug() << "onConnected()";
}

void ServerAgent::onDisconnected()
{
    m_tcpSocket->deleteLater();
    m_tcpSocket = nullptr;
}

void ServerAgent::onError(QAbstractSocket::SocketError socketError)
{
    qDebug() << "onError()" << socketError;
}

void ServerAgent::onResponseGetABook(const WordBook &book)
{
    funcTracker ft("onResponseGetABook()");
    sptr<WordBook> newBook = new WordBook(book);
    m_mapBooks.insert(book.getName(), newBook);
}

void ServerAgent::onResponseGetWordsOfBook(QString bookName, QVector<QString> wordList)
{
    funcTracker ft("onResponseGetWordsOfBook()");
    qDebug() << "got words for book" << bookName;

    QVector<QString> wordsToGet;
    // in this case, we download the definition of the words
    for (int i = 0;i < wordList.size();i ++)
    {
        auto spelling = wordList.at(i);
        auto word = m_mapWords.value(spelling);
        if (word.get() == nullptr)
        {
            wordsToGet.append(spelling);
        }
    }

    if (wordsToGet.size() > 0)
    {
        sendRequestGetWordsWithSmallMessages(bookName, wordsToGet);
    }
    else
    {
        // no need to download any words
        m_mapBooksStatus.insert(bookName, true);
        emit(bookDownloaded(bookName));
        qDebug() << "downloaded" << bookName;
    }
}

void ServerAgent::onResponseGetAWord(const Word &word)
{
    sptr<Word> newWord = new Word(word);
    m_mapWords.insert(word.getSpelling(), newWord);
    qDebug() << m_mapWords.size();
}

void ServerAgent::onStateChanged(QAbstractSocket::SocketState socketState)
{
    qDebug() << "onStateChanged()" << socketState;
}

int ServerAgent::readMessageCode()
{
    int messageCode;
    QDataStream in(m_tcpSocket);
    in.startTransaction();
    in >> messageCode;
    if (in.commitTransaction() == true)
    {
        return messageCode;
    }
    else
    {
        // in this case, the transaction is restored by commitTransaction()
        qInfo("failed to read message code in readMessageCode(), probably no data from peer");
        return 0;
    }
}

int ServerAgent::handleMessage(int messageCode)
{
    if (m_tcpSocket == nullptr)
    {
        return -1;
    }

    bool handleResult = false;
    bool unknowMessage = false;
    switch (messageCode) {
    case ServerClientProtocol::ResponseUnknownRequest:
        handleResult = handleResponseUnknownRequest();
        break;

    case ServerClientProtocol::ResponseGetAllBooks:
        handleResult = handleResponseGetAllBooks();
        break;

    case ServerClientProtocol::ResponseGetWordsOfBook:
        handleResult = handleResponseGetWordsOfBook();
        break;

    case ServerClientProtocol::ResponseGetAWord:
        handleResult = handleResponseGetAWord();
        break;

    case ServerClientProtocol::ResponseGetABook:
        handleResult = handleResponseGetABook();
        break;

    case ServerClientProtocol::ResponseAllDataSent:
        handleResult = handleResponseAllDataSent();
        break;

    default:
        handleUnknownMessage(messageCode);
        unknowMessage = true;
        break;

    }

    int retVal;
    if (unknowMessage == true)
    {
        retVal = -1;
    }
    else
    {
        if (handleResult == true)
        {
            retVal = 0;
        }
        else
        {
            retVal = 1;
        }
    }

    return retVal;
}

bool ServerAgent::handleUnknownMessage(int messageCode)
{
    // read all following data in the socket
    //auto abondonData = m_tcpSocket->readAll();

    qDebug() << "got unknown message with code" << messageCode;

    return true;
}

bool ServerAgent::handleResponseGetAllBooks()
{
    QDataStream in(m_tcpSocket);
    QList<QString> books;
    in.startTransaction();
    in >> books;
    if (in.commitTransaction() == false)
    {
        // in this case, the transaction is restored by commitTransaction()
        qDebug() << "failed to read books in handleResponseGetAllBooks()";
        return false;
    }

    m_books = books;
    emit(bookListReady(m_books));

    return true;
}

bool ServerAgent::handleResponseGetWordsOfBook()
{
    QString bookName;
    QVector<QString> wordList;
    QDataStream in(m_tcpSocket);
    in.startTransaction();
    in >> bookName >> wordList;
    if (in.commitTransaction() == false)
    {
        // in this case, the transaction is restored by commitTransaction()
        qDebug() << "failed to read words of the book in handleResponseGetWordsOfBook()";
        return false;
    }

    emit(responseGetWordsOfBook(bookName, wordList));

    return true;
}

bool ServerAgent::handleResponseGetAWord()
{
    Word word;
    QDataStream in(m_tcpSocket);
    in.startTransaction();
    in >> word;
    if (in.commitTransaction() == false)
    {
        // in this case, the transaction is restored by commitTransaction()
        qDebug() << "failed to read words of the book in handleResponseGetAWord()";
        return false;
    }

    emit(responseGetAWord(word));

    //qDebug() << word.getId() << word.getSpelling() << word.getDefinition();

    return true;
}

bool ServerAgent::handleResponseAllDataSent()
{
    QDataStream in(m_tcpSocket);
    int messageCode;
    in.startTransaction();
    in >> messageCode;
    if (in.commitTransaction() == false)
    {
        // in this case, the transaction is restored by commitTransaction()
        qDebug() << "failed to read books in handleResponseGetAllBooks()";
        return false;
    }

    bool handleResult = false;
    switch (messageCode) {
    case ServerClientProtocol::RequestGetWordsOfBook:
        handleResult = handleResponseAllDataSentForRequestGetWordsOfBook();
        break;

    case ServerClientProtocol::RequestGetWords:
        handleResult = handleResponseAllDataSentForRequestGetWords();
        break;

    default:
        qDebug() << "unhandled message code" << messageCode << "in handleResponseAllDataSent()";
        handleResult = false;
        break;
    }

    return true;
}

bool ServerAgent::handleResponseAllDataSentForRequestGetWordsOfBook()
{
    QDataStream in(m_tcpSocket);
    QString bookName;
    in.startTransaction();
    in >> bookName;
    if (in.commitTransaction() == false)
    {
        // in this case, the transaction is restored by commitTransaction()
        qDebug() << "failed to read books in handleResponseAllDataSentForRequestGetWordsOfBook()";
        return false;
    }

    return true;
}

bool ServerAgent::handleResponseAllDataSentForRequestGetWords()
{
    QDataStream in(m_tcpSocket);
    QString bookName;
    in.startTransaction();
    in >> bookName;
    if (in.commitTransaction() == false)
    {
        // in this case, the transaction is restored by commitTransaction()
        qDebug() << "failed to read books in handleResponseAllDataSentForRequestGetWordsOfBook()";
        return false;
    }

    if (bookName.startsWith(ServerClientProtocol::partPrefix()) == false)
    {
        m_mapBooksStatus.insert(bookName, true);
        emit(bookDownloaded(bookName));
        qDebug() << "downloaded" << bookName;
    }
    else
    {
        qDebug() << "received part" << bookName;
    }

    return true;
}

bool ServerAgent::handleResponseGetABook()
{
    WordBook book;
    QDataStream in(m_tcpSocket);
    in.startTransaction();
    in >> book;
    if (in.commitTransaction() == false)
    {
        // in this case, the transaction is restored by commitTransaction()
        qDebug() << "failed to read words of the book in handleResponseGetWordsOfBook()";
        return false;
    }

    emit(responseGetABook(book));

    qDebug() << book.getId() << book.getName() << book.getIntroduction();

    return true;
}

bool ServerAgent::handleResponseUnknownRequest()
{
    QDataStream in(m_tcpSocket);
    int requestCode;
    in.startTransaction();
    in >> requestCode;
    if (in.commitTransaction() == false)
    {
        // in this case, the transaction is restored by commitTransaction()
        qDebug() << "failed to read books in handleResponseGetAllBooks()";
        return false;
    }

    emit(responseUnknownRequest());
    qDebug() << "the server responed that failed to handle request with code" << requestCode;

    return true;
}

void ServerAgent::connectToServer()
{
    if (m_tcpSocket != nullptr)
    {
        return;
    }

    m_tcpSocket = new QTcpSocket(this);
    if (m_tcpSocket == nullptr)
    {
        return;
    }

    connect(m_tcpSocket, SIGNAL(connected()), this, SLOT(onConnected()));
    connect(m_tcpSocket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    connect(m_tcpSocket, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
    connect(m_tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)));
    connect(m_tcpSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(onStateChanged(QAbstractSocket::SocketState)));


    connect(this, SIGNAL(responseGetABook(const WordBook &)), this, SLOT(onResponseGetABook(const WordBook &)));
    connect(this, SIGNAL(responseGetWordsOfBook(QString, QVector<QString>)), this, SLOT(onResponseGetWordsOfBook(QString, QVector<QString>)));
    connect(this, SIGNAL(responseGetAWord(const Word &)), this, SLOT(onResponseGetAWord(const Word &)));
    //connect(this, SIGNAL(responseGetAWord(const Word &)), this, SLOT(onResponseGetAWord(const Word &)));
    //connect(this, SIGNAL(responseGetAWord(const Word &)), this, SLOT(onResponseGetAWord(const Word &)));

    m_tcpSocket->connectToHost(m_serverHostName, m_serverPort);

    // it seems waitForConnected() helps to get/trigger the error ConnectionRefusedError
    if (m_tcpSocket->waitForConnected() == false)
    {
        qDebug() << "waitForConnected() failed" << m_tcpSocket->error();
    }
}

void ServerAgent::sendRequestGetAllBooks()
{
    connectToServer();
    int messageCode = ServerClientProtocol::RequestGetAllBooks;

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << messageCode;
    m_tcpSocket->write(block);
}

void ServerAgent::sendRequestGetWordsOfBook(QString bookName)
{
    connectToServer();
    int messageCode = ServerClientProtocol::RequestGetWordsOfBook;

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << messageCode << bookName;
    m_tcpSocket->write(block);
}

void ServerAgent::sendRequestGetAWord(QString spelling)
{
    connectToServer();
    int messageCode = ServerClientProtocol::RequestGetAWord;

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << messageCode << spelling;
    m_tcpSocket->write(block);
}

void ServerAgent::sendRequestGetABook(QString bookName)
{
    connectToServer();
    int messageCode = ServerClientProtocol::RequestGetABook;

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << messageCode << bookName;
    m_tcpSocket->write(block);
}

void ServerAgent::sendRequestGetWords(QString bookName, QVector<QString> wordList)
{
    if (wordList.size() > ServerClientProtocol::MaximumWordsInAMessage)
    {
        return;
    }

    connectToServer();
    int messageCode = ServerClientProtocol::RequestGetWords;

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << messageCode << bookName << wordList;
    m_tcpSocket->write(block);
}

void ServerAgent::sendRequestGetWordsWithSmallMessages(QString bookName, QVector<QString> wordList)
{
    int total = wordList.size();
    int pos = 0;
    int counter = 0;
    while (pos + ServerClientProtocol::MaximumWordsInAMessage < total)
    {
        counter ++;
        QVector<QString> subList = wordList.mid(pos, ServerClientProtocol::MaximumWordsInAMessage);
        const QString partName = ServerClientProtocol::partPrefix() + QString::number(counter) + "__" + bookName;
        sendRequestGetWords(partName, subList);
        pos += ServerClientProtocol::MaximumWordsInAMessage;
    };

    QVector<QString> leftWords = wordList.mid(pos, ServerClientProtocol::MaximumWordsInAMessage);
    sendRequestGetWords(bookName, leftWords);
}

void ServerAgent::downloadBook(QString bookName)
{
    bool downloaded = m_mapBooksStatus.value(bookName);
    if (downloaded == true)
    {
        sptr<WordBook> book = m_mapBooks.value(bookName);
        if (book.get() != nullptr)
        {
            emit(bookDownloaded(bookName));
            qDebug() << "downloaded" << bookName;
        }
        else
        {
            qDebug() << "The code should NOT run to this line!";
        }
    }
    else
    {
        // it's OK even if the book is already in downloading!
        sendRequestGetABook(bookName);
        sendRequestGetWordsOfBook(bookName);
    }
}

void ServerAgent::getBookList()
{
    if (m_books.size() == 0)
    {
        // no book yet, send message to server to get books
        sendRequestGetAllBooks();
    }
    else
    {
        // books are already available
        emit(bookListReady(m_books));
    }
}
