#ifndef STUDYLIST_H
#define STUDYLIST_H

#include "wordcard.h"
#include "../golddict/sptr.hh"

#include <QVector>
#include <QLinkedList>
#include <QDateTime>

/**
 * @brief The StudyList class
 * A list of words that is going to be learned
 */
class StudyList
{
  public:
    StudyList();
    ~StudyList();

    static sptr<StudyList> allWords();
    static sptr<StudyList> allNewWords();
    static sptr<StudyList> allOldWords();
    static sptr<StudyList> allExpiredWords(QDateTime expire = QDateTime::currentDateTime());

    static sptr<StudyList> allWordsInBook(const QString &bookName);
    static sptr<StudyList> allNewWordsInBook(const QString &bookName);
    static sptr<StudyList> allOldWordsInBook(const QString &bookName);
    static sptr<StudyList> allExpiredWordsInBook(const QString &bookName, const QDateTime expire);

    sptr<WordCard> nextCard();
    bool responseToCurrent(sptr<WordCard> current, MemoryItem::ResponseQuality responseQulity);

    const QVector<QString> &getWordList() const
    {
        return m_words;
    }

    int size() const
    {
        return m_words.size();
    }

  private:
    QVector<QString> m_words;
    sptr<WordCard> m_current;

    void setWordList(const QVector<QString> &wordList);
    void learnWordAgain(const QString spelling);
};

#endif // STUDYLIST_H
