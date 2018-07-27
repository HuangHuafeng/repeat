#ifndef WORDCARD_H
#define WORDCARD_H

#include "memoryitem.h"
#include "word.h"

class WordCard : public MemoryItem
{
public:
    WordCard(sptr<Word> word = sptr<Word>(), int interval = 24, float easiness = 2.5, int repition = 0);
    virtual ~WordCard();
    virtual void update(ResponseQuality responseQuality) override;
    virtual int estimatedInterval(ResponseQuality responseQuality = Perfect) const override;
    void getFromDatabase();

    sptr<Word> getWord() const
    {
        return m_word;
    }

public:
    static void createDatabaseTables();
    static sptr<WordCard> generateCardForWord(const QString &spelling);
    static void setRatio(float ratio)
    {
        m_ratio = ratio;
        if (m_ratio <= 0) {
            m_ratio = 1.0;
        }
    }

private:
    static float m_ratio;
    sptr<Word> m_word;

    void dbsave();
};

#endif // WORDCARD_H
