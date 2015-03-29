

#ifndef _SPELLINFO_H
#define _SPELLINFO_H

#include "Common.h"

struct SpellEntry;

class SpellInfo
{
public:
    static bool hasEffect(SpellEntry const* entry, uint32 effect);
    static bool hasAuraName(SpellEntry const* entry, uint32 auraname);
    
private:
    // Prevent instanciation
    SpellInfo() {}
    ~SpellInfo() {}
};

#endif
