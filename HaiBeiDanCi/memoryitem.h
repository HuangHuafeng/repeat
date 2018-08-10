#ifndef MEMORYITEM_H
#define MEMORYITEM_H

// SuperMemo, refer to https://www.supermemo.com/english/ol/sm2.htm

#include <QDateTime>

class MemoryItem
{
public:
    typedef enum {
        Blackout = 0,
        Incorrect = 1,
        IncorrectButCanRecall = 2,
        CorrectWithDifficulty = 3,
        CorrectAfterHesitation = 4,
        Perfect = 5
    } ResponseQuality;

    MemoryItem(int intervalInMinutes = 24 * 60, float easiness = 2.5, int repition = 0);
    virtual ~MemoryItem();
    virtual void update(ResponseQuality responseQuality) = 0;

    int getIntervalInMinute()
    {
        return m_interval;
    }

    void setIntervalInMinute(int interval)
    {
        m_interval = interval;
    }

    float getEasiness()
    {
        return m_easiness;
    }

    void setEasiness(float easiness)
    {
        m_easiness = easiness;
    }

    int getRepetition()
    {
        return m_repetition;
    }

    void setRepetition( int repetition)
    {
        m_repetition = repetition;
    }

private:
    int m_repetition; //
    int m_interval; // in minutes
    float m_easiness;

};

#endif // MEMORYITEM_H
