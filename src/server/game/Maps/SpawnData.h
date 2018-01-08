
#ifndef TRINITY_SPAWNDATA_H
#define TRINITY_SPAWNDATA_H

#include "Position.h"

enum SpawnObjectType
{
    SPAWN_TYPE_CREATURE = 0,
    SPAWN_TYPE_GAMEOBJECT = 1,

    SPAWN_TYPE_MAX
};

enum SpawnObjectTypeMask
{
    SPAWN_TYPEMASK_CREATURE = (1 << SPAWN_TYPE_CREATURE),
    SPAWN_TYPEMASK_GAMEOBJECT = (1 << SPAWN_TYPE_GAMEOBJECT),

    SPAWN_TYPEMASK_ALL = (1 << SPAWN_TYPE_MAX)-1
};

enum SpawnGroupFlags
{
    SPAWNGROUP_FLAG_NONE                = 0x00,
    SPAWNGROUP_FLAG_SYSTEM              = 0x01,
    SPAWNGROUP_FLAG_COMPATIBILITY_MODE  = 0x02,
    SPAWNGROUP_FLAG_MANUAL_SPAWN        = 0x04,
    SPAWNGROUP_FLAG_DYNAMIC_SPAWN_RATE  = 0x08,
    SPAWNGROUP_FLAG_ESCORTQUESTNPC      = 0x10,

    SPAWNGROUP_FLAGS_ALL = (SPAWNGROUP_FLAG_SYSTEM | SPAWNGROUP_FLAG_COMPATIBILITY_MODE | SPAWNGROUP_FLAG_MANUAL_SPAWN | SPAWNGROUP_FLAG_DYNAMIC_SPAWN_RATE | SPAWNGROUP_FLAG_ESCORTQUESTNPC)
};

struct SpawnGroupTemplateData
{
    uint32 groupId;
    std::string name;
    uint32 mapId;
    SpawnGroupFlags flags;
};

struct SpawnData
{
    SpawnObjectType const type;
    uint32 spawnId = 0;
    uint32 id = 0; // entry in respective _template table
    WorldLocation spawnPoint;
    uint32 phaseMask = 0;
    int32 spawntimesecs = 0;
    uint8 spawnMask = 0;
    SpawnGroupTemplateData const* spawnGroupData = nullptr;
    uint32 scriptId = 0;
    bool dbData = true;

    protected:
    SpawnData(SpawnObjectType t) : type(t) {}
};

enum LinkedRespawnType
{
    LINKED_RESPAWN_CREATURE_TO_CREATURE = 0,
    LINKED_RESPAWN_CREATURE_TO_GO = 1, // Creature is dependant on GameObject
    LINKED_RESPAWN_GO_TO_GO = 2,
    LINKED_RESPAWN_GO_TO_CREATURE = 3, // GameObject is dependant on Creature
};

#endif
