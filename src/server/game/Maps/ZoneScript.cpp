
#include "ZoneScript.h"
#include "Database/DatabaseEnv.h"
#include "Map.h"

ObjectGuid ZoneScript::GetGuidData(ObjectGuid::LowType /*DataId*/) const
{ 
    return ObjectGuid::Empty; 
}

uint32 ZoneScript::GetCreatureEntry(ObjectGuid::LowType /*guidlow*/, CreatureData const* data, uint32 chosenEntry)
{ 
    return chosenEntry;
}
