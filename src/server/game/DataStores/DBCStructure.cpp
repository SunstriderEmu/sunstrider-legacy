#include "DBCStructure.h"
#include "World.h"

bool AreaTableEntry::IsSanctuary() const
{
	if (mapid == 609)
		return true;
	if (sWorld->IsZoneSanctuary(ID))
		return true;

	return (flags & AREA_FLAG_SANCTUARY);
}