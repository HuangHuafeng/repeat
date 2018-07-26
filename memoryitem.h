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

    MemoryItem(int interval = 24, float easiness = 2.5, int repition = 0);
    virtual void update(ResponseQuality responseQuality);

    int estimatedInterval(ResponseQuality responseQuality = Perfect) const;

    int getInterval() const
    {
        return m_interval;
    }

    void setInterval(int interval)
    {
        m_interval = interval;
    }

    float getEasiness() const
    {
        return m_easiness;
    }

    void setEasiness(float easiness)
    {
        m_easiness = easiness;
    }

    int getRepitition() const
    {
        return m_repition;
    }

    void setRepitition( int repitition)
    {
        m_repition = repitition;
    }

private:
    int m_repition; //
    int m_interval; // in hours, using seconds may result in overflow
    float m_easiness;

    float estimatedEasiness(ResponseQuality responseQuality = Perfect) const;

};

#endif // MEMORYITEM_H
