#include "memoryitem.h"

MemoryItem::MemoryItem(int intervalInMinutes, float easiness, int repetition)
{
    m_interval = intervalInMinutes;
    m_easiness = easiness;
    m_repetition = repetition;
}

MemoryItem::~MemoryItem()
{
}

/*
void MemoryItem::update(ResponseQuality responseQuality)
{
    // m_easiness and m_interval should NOT change before calculating estimated value!!!
    float nextEasiness = estimatedEasiness(responseQuality);
    int nextInterval = estimatedInterval(responseQuality);

    // update the easiness and the interval
    m_easiness = nextEasiness;
    m_interval = nextInterval;

    if (responseQuality <= IncorrectButCanRecall) {
        m_repition = 0;
    } else {
        m_repition ++;
    }
}
*/

/**
 * @brief MemoryItem::estimatedInterval
 * @param responseQuality
 * @return expire in MINUTE
 */
/*
int MemoryItem::estimatedInterval(ResponseQuality responseQuality)
{
    if (m_repition == 0) {//I(1)
        return 24 * 60;
    }

    if (m_repition == 1) {//I(2)
        return 24 * 60 * 6;
    }

    if (responseQuality <= IncorrectButCanRecall) {
        return 24 * 60;
    }

    float nextEasiness = estimatedEasiness(responseQuality);
    int nextInterval = static_cast<int>(m_interval * nextEasiness);
    if (nextInterval > 60 * 24 * 365 * 1000) {
        nextInterval = 60 * 24 * 365 * 1000;   // let's set maximum to 1000 years, to avoid overflow
    }
    return nextInterval;
}

float MemoryItem::estimatedEasiness(ResponseQuality responseQuality) const
{
    if (m_repition == 0 || m_repition == 1 || responseQuality <= IncorrectButCanRecall) {
        return m_easiness;
    }

    float nextEasiness = m_easiness + (0.1f - (5 - responseQuality) * (0.08f + (5 - responseQuality) * 0.02f));
    if (nextEasiness < 1.3f) {
        nextEasiness = 1.3f;
    }

    return nextEasiness;
}
*/
