#include "DBCStructure.h"

bool AreaTableEntry::IsSanctuary() const
{
    if (mapid == 609)
        return true;

    return (flags & AREA_FLAG_SANCTUARY);
}

bool AreaTableEntry::IsFlyable() const
{
    if (flags & AREA_FLAG_OUTLAND)
    {
#ifdef LICH_KING
        if (!(flags & AREA_FLAG_NO_FLY_ZONE))
#endif
            return true;
    }

    return false;
}