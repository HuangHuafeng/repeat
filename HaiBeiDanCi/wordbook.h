#ifndef WORDBOOK_H
#define WORDBOOK_H

#include "../golddict/sptr.hh"

#include <QString>
#include <QVector>
#include <QObject>

class WordBook
{
public:
    WordBook(QString name = QObject::tr("new book"), QString introduction = QObject::tr("no introduction"), int id = 0);

    int getId();

    const QString getName() const {
        return m_name;
    }

    const QString getIntroduction() const {
        return m_introduction;
    }

    bool hasWord(QString spelling);
    bool hasWord(int wordId);
    bool dbsave();
    bool dbget();
    bool dbsaveAddWords(const QVector<QString> &words);
    bool addWord(QString spelling);
    bool dbsaveDeleteAllWords();
    int totalWords();

    static bool createDatabaseTables();
    static QVector<sptr<WordBook>> getWordBooks();
    static bool isInDatabase(int bookId);
    static bool isInDatabase(const QString &name);
    static int getBookId(const QString &name);

private:
    QString m_name;
    QString m_introduction;
    int m_id;   // id in database
};

#endif // WORDBOOK_H