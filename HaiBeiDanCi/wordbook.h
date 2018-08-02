#ifndef WORDBOOK_H
#define WORDBOOK_H

#include "databaseobject.h"
#include "../golddict/sptr.hh"
#include "wordcard.h"

#include <QString>
#include <QVector>
#include <QObject>

class WordBook : public DatabaseObject
{
public:
    WordBook(QString name = QObject::tr("new book"), QString introduction = QObject::tr("no introduction"), int id = 0);

    int getId();

    const QString getName();
    const QString getIntroduction();

    bool hasWord(QString spelling);
    bool hasWord(int wordId);
    bool dbsave();
    bool dbsaveAddWords(const QVector<QString> &words);
    bool addWord(QString spelling);
    bool dbsaveDeleteAllWords();
    int totalWords();
    QVector<QString> getAllWords();
    QVector<QString> getStudiedWords();
    QVector<QString> getNewWords();
    QVector<QString> getExpiredWords(const QDateTime expire);

    static bool createDatabaseTables();
    static QVector<sptr<WordBook>> getWordBooks();
    static sptr<WordBook> getBook(const QString &bookName);
    static bool isInDatabase(int bookId);
    static bool isInDatabase(const QString &name);
    static int getBookId(const QString &name);
    static QVector<QString> getWordsInBook(const QString &bookName);

protected:
    virtual void updateFromDatabase() override;

private:
    QString m_name;
    QString m_introduction;
    int m_id;   // id in database

    bool dbgetNameAndIntro();
};

#endif // WORDBOOK_H
