#ifndef WORDBOOK_H
#define WORDBOOK_H

#include "../golddict/sptr.hh"
#include "wordcard.h"

#include <QString>
#include <QVector>
#include <QObject>
#include <QMutex>
#include <QDataStream>

class WordBook
{
  public:
    WordBook(QString name = QObject::tr("new book"), QString introduction = QObject::tr("no introduction"), int id = 0);

    int getId() const;
    void setId(int id);
    QString getName() const;
    void setName(QString name);
    QString getIntroduction() const;
    void setIntroduction(QString introduction);

    bool hasWord(QString spelling);
    bool hasWord(int wordId);
    bool dbsave();
    bool dbsaveAddWords(const QVector<QString> &words);
    bool addWord(QString spelling);

    QVector<QString> getAllWords(int number = 0);
    QVector<QString> getOldWords(int number = 0);
    QVector<QString> getNewWords(int number = 0);
    QVector<QString> getExpiredWords(const QDateTime expire, int number = 0);

    static bool createDatabaseTables();
    static void readAllBooksFromDatabase();

    static const QList<QString> getAllBooks();
    static sptr<WordBook> getBook(QString bookName, bool create = false);

    static bool isInDatabase(const QString &name);
    static QVector<QString> getWordsInBook(QString bookName);

    static void storeBookFromServer(sptr<WordBook> book, const QVector<QString> &wordList = QVector<QString>());

  private:
    QString m_name;
    QString m_introduction;
    int m_id; // id in database

    static QMap<QString, sptr<WordBook>> m_allBooks;
    static QMutex m_allBooksMutex;

    bool dbgetNameAndIntro();
};

QDataStream &operator<<(QDataStream &ds, const WordBook &book);
QDataStream &operator>>(QDataStream &ds, WordBook &book);

#endif // WORDBOOK_H
