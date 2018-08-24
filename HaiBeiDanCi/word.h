#ifndef WORD_H
#define WORD_H

#include "../golddict/sptr.hh"

#include <QString>
#include <QDateTime>
#include <QSqlQuery>
#include <QVector>
#include <QMutex>
#include <QDataStream>

class Word
{
  public:
    Word(QString word = "SOMETHING_IS_WRONG", QString definition = "", int id = 0);

    const QString &getSpelling() const
    {
        return m_spelling;
    }

    void setId(int id);
    void setSpelling(QString spelling);
    void setDefinition(const QString &definition);

    int getId() const;
    const QString &getDefinition() const;
    QString getDefinitionDIV() const;
    QList<QString> mediaFiles() const;

    bool dbsave();

    static bool createDatabaseTables();
    static void readAllWordsFromDatabase();

    static QList<QString> getAllWords();
    static sptr<Word> getWord(const QString &spelling, bool create = false);
    static int getWordId(const QString &spelling);
    static bool isInDatabase(const QString &spelling);
    static void storeWordFromServer(sptr<Word> word);
    static void storeMultipleWordFromServer(const QMap<QString, sptr<Word>> mapWords);
    static void batchStoreMultipleWordFromServer(const QMap<QString, sptr<Word>> mapWords);
    static void v2StoreMultipleWordFromServer(const QMap<QString, sptr<Word>> mapWords);

  private:
    //bool dbsaveDefinition();

  private:
    QString m_spelling;
    QString m_definition;
    int m_id; // id in database

    static QMap<QString, sptr<Word>> m_allWords;
    static QMutex m_allWordsMutex;
};

QDataStream &operator<<(QDataStream &ds, const Word &word);
QDataStream &operator>>(QDataStream &ds, Word &word);

#endif // WORD_H
