
#include "SpellInfo.h"
#include "DBCStores.h"

bool SpellInfo::hasEffect(SpellEntry const* entry, uint32 effect)
{
    return entry->Effect[0] == effect || entry->Effect[1] == effect || entry->Effect[2] == effect;
}

bool SpellInfo::hasAuraName(SpellEntry const* entry, uint32 auraname)
{
    return entry->EffectApplyAuraName[0] == auraname || entry->EffectApplyAuraName[1] == auraname || entry->EffectApplyAuraName[2] == auraname;
}
