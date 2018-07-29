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
    static bool createDatabaseTables();
    static sptr<WordCard> generateCardForWord(const QString &spelling);

private:
    static const float m_ratio[MemoryItem::Perfect + 1];
    sptr<Word> m_word;

    void dbsave();
};

#endif // WORDCARD_H
