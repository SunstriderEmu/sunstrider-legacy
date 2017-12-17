
#include "ZoneScript.h"
#include "Database/DatabaseEnv.h"
#include "Map.h"

ObjectGuid ZoneScript::GetGuidData(uint32 /*DataId*/) const 
{ 
    return ObjectGuid::Empty; 
}

uint32 ZoneScript::GetCreatureEntry(uint32 /*guidlow*/, CreatureData const* data) 
{ 
    return data->id; 
}
