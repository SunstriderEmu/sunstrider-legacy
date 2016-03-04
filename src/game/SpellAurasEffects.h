
#ifndef TRINITY_SPELLAURAEFFECTS_H
#define TRINITY_SPELLAURAEFFECTS_H

class AuraEffect
{
public:
    int32 GetPeriodicTimer() { return m_periodicTimer; }
    void SetPeriodicTimer(int32 newTimer) { m_periodicTimer = newTimer; }
    bool IsPeriodic() const { return m_isPeriodic; }
    uint32 GetTickNumber() const { return m_tickNumber; }
    int32 GetMiscValue() const;
    int32 GetMiscBValue() const;

private:
    uint32 m_effIndex;
    int32 m_periodicTimer;
    int32 m_amplitude;
    uint32 m_tickNumber;
    
    bool m_isPeriodic : 1;

}

#include "SpellAuraDefines.h"

#endif

