/*
* Copyright (C) 2008-2014 TrinityCore <http://www.trinitycore.org/>
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the
* Free Software Foundation; either version 2 of the License, or (at your
* option) any later version.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
* more details.
*
* You should have received a copy of the GNU General Public License along
* with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "SpellInfo.h"
#include "DBCStores.h"
#include "SpellAuraDefines.h"
#include "Spell.h"


uint32 GetTargetFlagMask(SpellTargetObjectTypes objType)
{
    switch (objType)
    {
    case TARGET_OBJECT_TYPE_DEST:
        return TARGET_FLAG_DEST_LOCATION;
    case TARGET_OBJECT_TYPE_UNIT_AND_DEST:
        return TARGET_FLAG_DEST_LOCATION | TARGET_FLAG_UNIT;
    case TARGET_OBJECT_TYPE_CORPSE_ALLY:
        return TARGET_FLAG_CORPSE_ALLY;
    case TARGET_OBJECT_TYPE_CORPSE_ENEMY:
        return TARGET_FLAG_CORPSE_ENEMY;
    case TARGET_OBJECT_TYPE_CORPSE:
        return TARGET_FLAG_CORPSE_ALLY | TARGET_FLAG_CORPSE_ENEMY;
    case TARGET_OBJECT_TYPE_UNIT:
        return TARGET_FLAG_UNIT;
    case TARGET_OBJECT_TYPE_GOBJ:
        return TARGET_FLAG_GAMEOBJECT;
    case TARGET_OBJECT_TYPE_GOBJ_ITEM:
        return TARGET_FLAG_GAMEOBJECT_ITEM;
    case TARGET_OBJECT_TYPE_ITEM:
        return TARGET_FLAG_ITEM;
    case TARGET_OBJECT_TYPE_SRC:
        return TARGET_FLAG_SOURCE_LOCATION;
    default:
        return TARGET_FLAG_NONE;
    }
}

SpellImplicitTargetInfo::SpellImplicitTargetInfo(uint32 target)
{
    _target = Targets(target);
}

bool SpellImplicitTargetInfo::IsArea() const
{
    return GetSelectionCategory() == TARGET_SELECT_CATEGORY_AREA || GetSelectionCategory() == TARGET_SELECT_CATEGORY_CONE;
}

SpellTargetSelectionCategories SpellImplicitTargetInfo::GetSelectionCategory() const
{
    return _data[_target].SelectionCategory;
}

SpellTargetReferenceTypes SpellImplicitTargetInfo::GetReferenceType() const
{
    return _data[_target].ReferenceType;
}

SpellTargetObjectTypes SpellImplicitTargetInfo::GetObjectType() const
{
    return _data[_target].ObjectType;
}

SpellTargetCheckTypes SpellImplicitTargetInfo::GetCheckType() const
{
    return _data[_target].SelectionCheckType;
}

SpellTargetDirectionTypes SpellImplicitTargetInfo::GetDirectionType() const
{
    return _data[_target].DirectionType;
}

float SpellImplicitTargetInfo::CalcDirectionAngle() const
{
    switch (GetDirectionType())
    {
        case TARGET_DIR_FRONT:
            return 0.0f;
        case TARGET_DIR_BACK:
            return static_cast<float>(M_PI);
        case TARGET_DIR_RIGHT:
            return static_cast<float>(-M_PI/2);
        case TARGET_DIR_LEFT:
            return static_cast<float>(M_PI/2);
        case TARGET_DIR_FRONT_RIGHT:
            return static_cast<float>(-M_PI/4);
        case TARGET_DIR_BACK_RIGHT:
            return static_cast<float>(-3*M_PI/4);
        case TARGET_DIR_BACK_LEFT:
            return static_cast<float>(3*M_PI/4);
        case TARGET_DIR_FRONT_LEFT:
            return static_cast<float>(M_PI/4);
        case TARGET_DIR_RANDOM:
            return float(rand_norm())*static_cast<float>(2*M_PI);
        default:
            return 0.0f;
    }
}

Targets SpellImplicitTargetInfo::GetTarget() const
{
    return _target;
}

uint32 SpellImplicitTargetInfo::GetExplicitTargetMask(bool& srcSet, bool& dstSet) const
{
    uint32 targetMask = 0;
    if (GetTarget() == TARGET_DEST_TRAJ)
    {
        if (!srcSet)
            targetMask = TARGET_FLAG_SOURCE_LOCATION;
        if (!dstSet)
            targetMask |= TARGET_FLAG_DEST_LOCATION;
    }
    else
    {
        switch (GetReferenceType())
        {
            case TARGET_REFERENCE_TYPE_SRC:
                if (srcSet)
                    break;
                targetMask = TARGET_FLAG_SOURCE_LOCATION;
                break;
            case TARGET_REFERENCE_TYPE_DEST:
                if (dstSet)
                    break;
                targetMask = TARGET_FLAG_DEST_LOCATION;
                break;
            case TARGET_REFERENCE_TYPE_TARGET:
                switch (GetObjectType())
                {
                    case TARGET_OBJECT_TYPE_GOBJ:
                        targetMask = TARGET_FLAG_GAMEOBJECT;
                        break;
                    case TARGET_OBJECT_TYPE_GOBJ_ITEM:
                        targetMask = TARGET_FLAG_GAMEOBJECT_ITEM;
                        break;
                    case TARGET_OBJECT_TYPE_UNIT_AND_DEST:
                    case TARGET_OBJECT_TYPE_UNIT:
                    case TARGET_OBJECT_TYPE_DEST:
                        switch (GetCheckType())
                        {
                            case TARGET_CHECK_ENEMY:
                                targetMask = TARGET_FLAG_UNIT_ENEMY;
                                break;
                            case TARGET_CHECK_ALLY:
                                targetMask = TARGET_FLAG_UNIT_ALLY;
                                break;
                            case TARGET_CHECK_PARTY:
                                targetMask = TARGET_FLAG_UNIT_PARTY;
                                break;
                            case TARGET_CHECK_RAID:
                                targetMask = TARGET_FLAG_UNIT_RAID;
                                break;
#ifdef LICH_KING
                            case TARGET_CHECK_PASSENGER:
                                targetMask = TARGET_FLAG_UNIT_PASSENGER;
                                break;
#endif
                            case TARGET_CHECK_RAID_CLASS:
                                // nobreak;
                            default:
                                targetMask = TARGET_FLAG_UNIT;
                                break;
                        }
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
    }

    switch (GetObjectType())
    {
        case TARGET_OBJECT_TYPE_SRC:
            srcSet = true;
            break;
        case TARGET_OBJECT_TYPE_DEST:
        case TARGET_OBJECT_TYPE_UNIT_AND_DEST:
            dstSet = true;
            break;
        default:
            break;
    }
    return targetMask;
}

SpellImplicitTargetInfo::StaticData  SpellImplicitTargetInfo::_data[TOTAL_SPELL_TARGETS] =
{
    {TARGET_OBJECT_TYPE_NONE, TARGET_REFERENCE_TYPE_NONE,   TARGET_SELECT_CATEGORY_NYI,     TARGET_CHECK_DEFAULT,  TARGET_DIR_NONE},        //
    {TARGET_OBJECT_TYPE_UNIT, TARGET_REFERENCE_TYPE_CASTER, TARGET_SELECT_CATEGORY_DEFAULT, TARGET_CHECK_DEFAULT,  TARGET_DIR_NONE},        // 1 TARGET_UNIT_CASTER
    {TARGET_OBJECT_TYPE_UNIT, TARGET_REFERENCE_TYPE_CASTER, TARGET_SELECT_CATEGORY_NEARBY,  TARGET_CHECK_ENEMY,    TARGET_DIR_NONE},        // 2 TARGET_UNIT_NEARBY_ENEMY
    {TARGET_OBJECT_TYPE_UNIT, TARGET_REFERENCE_TYPE_CASTER, TARGET_SELECT_CATEGORY_NEARBY,  TARGET_CHECK_PARTY,    TARGET_DIR_NONE},        // 3 TARGET_UNIT_NEARBY_PARTY
    {TARGET_OBJECT_TYPE_UNIT, TARGET_REFERENCE_TYPE_CASTER, TARGET_SELECT_CATEGORY_NEARBY,  TARGET_CHECK_ALLY,     TARGET_DIR_NONE},        // 4 TARGET_UNIT_NEARBY_ALLY
    {TARGET_OBJECT_TYPE_UNIT, TARGET_REFERENCE_TYPE_CASTER, TARGET_SELECT_CATEGORY_DEFAULT, TARGET_CHECK_DEFAULT,  TARGET_DIR_NONE},        // 5 TARGET_UNIT_PET
    {TARGET_OBJECT_TYPE_UNIT, TARGET_REFERENCE_TYPE_TARGET, TARGET_SELECT_CATEGORY_DEFAULT, TARGET_CHECK_ENEMY,    TARGET_DIR_NONE},        // 6 TARGET_UNIT_TARGET_ENEMY
    {TARGET_OBJECT_TYPE_UNIT, TARGET_REFERENCE_TYPE_SRC,    TARGET_SELECT_CATEGORY_AREA,    TARGET_CHECK_ENTRY,    TARGET_DIR_NONE},        // 7 TARGET_UNIT_SRC_AREA_ENTRY
    {TARGET_OBJECT_TYPE_UNIT, TARGET_REFERENCE_TYPE_DEST,   TARGET_SELECT_CATEGORY_AREA,    TARGET_CHECK_ENTRY,    TARGET_DIR_NONE},        // 8 TARGET_UNIT_DEST_AREA_ENTRY
    {TARGET_OBJECT_TYPE_DEST, TARGET_REFERENCE_TYPE_CASTER, TARGET_SELECT_CATEGORY_DEFAULT, TARGET_CHECK_DEFAULT,  TARGET_DIR_NONE},        // 9 TARGET_DEST_HOME
    {TARGET_OBJECT_TYPE_NONE, TARGET_REFERENCE_TYPE_NONE,   TARGET_SELECT_CATEGORY_NYI,     TARGET_CHECK_DEFAULT,  TARGET_DIR_NONE},        // 10
    {TARGET_OBJECT_TYPE_UNIT, TARGET_REFERENCE_TYPE_SRC,    TARGET_SELECT_CATEGORY_NYI,     TARGET_CHECK_DEFAULT,  TARGET_DIR_NONE},        // 11 TARGET_UNIT_SRC_AREA_UNK_11
    {TARGET_OBJECT_TYPE_NONE, TARGET_REFERENCE_TYPE_NONE,   TARGET_SELECT_CATEGORY_NYI,     TARGET_CHECK_DEFAULT,  TARGET_DIR_NONE},        // 12
    {TARGET_OBJECT_TYPE_NONE, TARGET_REFERENCE_TYPE_NONE,   TARGET_SELECT_CATEGORY_NYI,     TARGET_CHECK_DEFAULT,  TARGET_DIR_NONE},        // 13
    {TARGET_OBJECT_TYPE_NONE, TARGET_REFERENCE_TYPE_NONE,   TARGET_SELECT_CATEGORY_NYI,     TARGET_CHECK_DEFAULT,  TARGET_DIR_NONE},        // 14
    {TARGET_OBJECT_TYPE_UNIT, TARGET_REFERENCE_TYPE_SRC,    TARGET_SELECT_CATEGORY_AREA,    TARGET_CHECK_ENEMY,    TARGET_DIR_NONE},        // 15 TARGET_UNIT_SRC_AREA_ENEMY
    {TARGET_OBJECT_TYPE_UNIT, TARGET_REFERENCE_TYPE_DEST,   TARGET_SELECT_CATEGORY_AREA,    TARGET_CHECK_ENEMY,    TARGET_DIR_NONE},        // 16 TARGET_UNIT_DEST_AREA_ENEMY
    {TARGET_OBJECT_TYPE_DEST, TARGET_REFERENCE_TYPE_CASTER, TARGET_SELECT_CATEGORY_DEFAULT, TARGET_CHECK_DEFAULT,  TARGET_DIR_NONE},        // 17 TARGET_DEST_DB
    {TARGET_OBJECT_TYPE_DEST, TARGET_REFERENCE_TYPE_CASTER, TARGET_SELECT_CATEGORY_DEFAULT, TARGET_CHECK_DEFAULT,  TARGET_DIR_NONE},        // 18 TARGET_DEST_CASTER
    {TARGET_OBJECT_TYPE_NONE, TARGET_REFERENCE_TYPE_NONE,   TARGET_SELECT_CATEGORY_NYI,     TARGET_CHECK_DEFAULT,  TARGET_DIR_NONE},        // 19
    {TARGET_OBJECT_TYPE_UNIT, TARGET_REFERENCE_TYPE_CASTER, TARGET_SELECT_CATEGORY_AREA,    TARGET_CHECK_PARTY,    TARGET_DIR_NONE},        // 20 TARGET_UNIT_CASTER_AREA_PARTY
    {TARGET_OBJECT_TYPE_UNIT, TARGET_REFERENCE_TYPE_TARGET, TARGET_SELECT_CATEGORY_DEFAULT, TARGET_CHECK_ALLY,     TARGET_DIR_NONE},        // 21 TARGET_UNIT_TARGET_ALLY
    {TARGET_OBJECT_TYPE_SRC,  TARGET_REFERENCE_TYPE_CASTER, TARGET_SELECT_CATEGORY_DEFAULT, TARGET_CHECK_DEFAULT,  TARGET_DIR_NONE},        // 22 TARGET_SRC_CASTER
    {TARGET_OBJECT_TYPE_GOBJ, TARGET_REFERENCE_TYPE_TARGET, TARGET_SELECT_CATEGORY_DEFAULT, TARGET_CHECK_DEFAULT,  TARGET_DIR_NONE},        // 23 TARGET_GAMEOBJECT_TARGET
    {TARGET_OBJECT_TYPE_UNIT, TARGET_REFERENCE_TYPE_CASTER, TARGET_SELECT_CATEGORY_CONE,    TARGET_CHECK_ENEMY,    TARGET_DIR_FRONT},       // 24 TARGET_UNIT_CONE_ENEMY_24
    {TARGET_OBJECT_TYPE_UNIT, TARGET_REFERENCE_TYPE_TARGET, TARGET_SELECT_CATEGORY_DEFAULT, TARGET_CHECK_DEFAULT,  TARGET_DIR_NONE},        // 25 TARGET_UNIT_TARGET_ANY
    {TARGET_OBJECT_TYPE_GOBJ_ITEM, TARGET_REFERENCE_TYPE_TARGET, TARGET_SELECT_CATEGORY_DEFAULT, TARGET_CHECK_DEFAULT, TARGET_DIR_NONE},    // 26 TARGET_GAMEOBJECT_ITEM_TARGET
    {TARGET_OBJECT_TYPE_UNIT, TARGET_REFERENCE_TYPE_CASTER, TARGET_SELECT_CATEGORY_DEFAULT, TARGET_CHECK_DEFAULT,  TARGET_DIR_NONE},        // 27 TARGET_UNIT_MASTER
    {TARGET_OBJECT_TYPE_DEST, TARGET_REFERENCE_TYPE_DEST,   TARGET_SELECT_CATEGORY_DEFAULT, TARGET_CHECK_ENEMY,    TARGET_DIR_NONE},        // 28 TARGET_DEST_DYNOBJ_ENEMY
    {TARGET_OBJECT_TYPE_DEST, TARGET_REFERENCE_TYPE_DEST,   TARGET_SELECT_CATEGORY_DEFAULT, TARGET_CHECK_ALLY,     TARGET_DIR_NONE},        // 29 TARGET_DEST_DYNOBJ_ALLY
    {TARGET_OBJECT_TYPE_UNIT, TARGET_REFERENCE_TYPE_SRC,    TARGET_SELECT_CATEGORY_AREA,    TARGET_CHECK_ALLY,     TARGET_DIR_NONE},        // 30 TARGET_UNIT_SRC_AREA_ALLY
    {TARGET_OBJECT_TYPE_UNIT, TARGET_REFERENCE_TYPE_DEST,   TARGET_SELECT_CATEGORY_AREA,    TARGET_CHECK_ALLY,     TARGET_DIR_NONE},        // 31 TARGET_UNIT_DEST_AREA_ALLY
    {TARGET_OBJECT_TYPE_DEST, TARGET_REFERENCE_TYPE_CASTER, TARGET_SELECT_CATEGORY_DEFAULT, TARGET_CHECK_DEFAULT,  TARGET_DIR_FRONT_LEFT},  // 32 TARGET_DEST_CASTER_SUMMON
    {TARGET_OBJECT_TYPE_UNIT, TARGET_REFERENCE_TYPE_SRC,    TARGET_SELECT_CATEGORY_AREA,    TARGET_CHECK_PARTY,    TARGET_DIR_NONE},        // 33 TARGET_UNIT_SRC_AREA_PARTY
    {TARGET_OBJECT_TYPE_UNIT, TARGET_REFERENCE_TYPE_DEST,   TARGET_SELECT_CATEGORY_AREA,    TARGET_CHECK_PARTY,    TARGET_DIR_NONE},        // 34 TARGET_UNIT_DEST_AREA_PARTY
    {TARGET_OBJECT_TYPE_UNIT, TARGET_REFERENCE_TYPE_TARGET, TARGET_SELECT_CATEGORY_DEFAULT, TARGET_CHECK_PARTY,    TARGET_DIR_NONE},        // 35 TARGET_UNIT_TARGET_PARTY
    {TARGET_OBJECT_TYPE_DEST, TARGET_REFERENCE_TYPE_CASTER, TARGET_SELECT_CATEGORY_NYI,     TARGET_CHECK_DEFAULT,  TARGET_DIR_NONE},        // 36 TARGET_DEST_CASTER_UNK_36
    {TARGET_OBJECT_TYPE_UNIT, TARGET_REFERENCE_TYPE_LAST,   TARGET_SELECT_CATEGORY_AREA,    TARGET_CHECK_PARTY,    TARGET_DIR_NONE},        // 37 TARGET_UNIT_LASTTARGET_AREA_PARTY
    {TARGET_OBJECT_TYPE_UNIT, TARGET_REFERENCE_TYPE_CASTER, TARGET_SELECT_CATEGORY_NEARBY,  TARGET_CHECK_ENTRY,    TARGET_DIR_NONE},        // 38 TARGET_UNIT_NEARBY_ENTRY
    {TARGET_OBJECT_TYPE_DEST, TARGET_REFERENCE_TYPE_CASTER, TARGET_SELECT_CATEGORY_DEFAULT, TARGET_CHECK_DEFAULT,  TARGET_DIR_NONE},        // 39 TARGET_DEST_CASTER_FISHING
    {TARGET_OBJECT_TYPE_GOBJ, TARGET_REFERENCE_TYPE_CASTER, TARGET_SELECT_CATEGORY_NEARBY,  TARGET_CHECK_ENTRY,    TARGET_DIR_NONE},        // 40 TARGET_GAMEOBJECT_NEARBY_ENTRY
    {TARGET_OBJECT_TYPE_DEST, TARGET_REFERENCE_TYPE_CASTER, TARGET_SELECT_CATEGORY_DEFAULT, TARGET_CHECK_DEFAULT,  TARGET_DIR_FRONT_RIGHT}, // 41 TARGET_DEST_CASTER_FRONT_RIGHT
    {TARGET_OBJECT_TYPE_DEST, TARGET_REFERENCE_TYPE_CASTER, TARGET_SELECT_CATEGORY_DEFAULT, TARGET_CHECK_DEFAULT,  TARGET_DIR_BACK_RIGHT},  // 42 TARGET_DEST_CASTER_BACK_RIGHT
    {TARGET_OBJECT_TYPE_DEST, TARGET_REFERENCE_TYPE_CASTER, TARGET_SELECT_CATEGORY_DEFAULT, TARGET_CHECK_DEFAULT,  TARGET_DIR_BACK_LEFT},   // 43 TARGET_DEST_CASTER_BACK_LEFT
    {TARGET_OBJECT_TYPE_DEST, TARGET_REFERENCE_TYPE_CASTER, TARGET_SELECT_CATEGORY_DEFAULT, TARGET_CHECK_DEFAULT,  TARGET_DIR_FRONT_LEFT},  // 44 TARGET_DEST_CASTER_FRONT_LEFT
    {TARGET_OBJECT_TYPE_UNIT, TARGET_REFERENCE_TYPE_TARGET, TARGET_SELECT_CATEGORY_DEFAULT, TARGET_CHECK_ALLY,     TARGET_DIR_NONE},        // 45 TARGET_UNIT_TARGET_CHAINHEAL_ALLY
    {TARGET_OBJECT_TYPE_DEST, TARGET_REFERENCE_TYPE_CASTER, TARGET_SELECT_CATEGORY_NEARBY,  TARGET_CHECK_ENTRY,    TARGET_DIR_NONE},        // 46 TARGET_DEST_NEARBY_ENTRY
    {TARGET_OBJECT_TYPE_DEST, TARGET_REFERENCE_TYPE_CASTER, TARGET_SELECT_CATEGORY_DEFAULT, TARGET_CHECK_DEFAULT,  TARGET_DIR_FRONT},       // 47 TARGET_DEST_CASTER_FRONT
    {TARGET_OBJECT_TYPE_DEST, TARGET_REFERENCE_TYPE_CASTER, TARGET_SELECT_CATEGORY_DEFAULT, TARGET_CHECK_DEFAULT,  TARGET_DIR_BACK},        // 48 TARGET_DEST_CASTER_BACK
    {TARGET_OBJECT_TYPE_DEST, TARGET_REFERENCE_TYPE_CASTER, TARGET_SELECT_CATEGORY_DEFAULT, TARGET_CHECK_DEFAULT,  TARGET_DIR_RIGHT},       // 49 TARGET_DEST_CASTER_RIGHT
    {TARGET_OBJECT_TYPE_DEST, TARGET_REFERENCE_TYPE_CASTER, TARGET_SELECT_CATEGORY_DEFAULT, TARGET_CHECK_DEFAULT,  TARGET_DIR_LEFT},        // 50 TARGET_DEST_CASTER_LEFT
    {TARGET_OBJECT_TYPE_GOBJ, TARGET_REFERENCE_TYPE_SRC,    TARGET_SELECT_CATEGORY_AREA,    TARGET_CHECK_DEFAULT,  TARGET_DIR_NONE},        // 51 TARGET_GAMEOBJECT_SRC_AREA
    {TARGET_OBJECT_TYPE_GOBJ, TARGET_REFERENCE_TYPE_DEST,   TARGET_SELECT_CATEGORY_AREA,    TARGET_CHECK_DEFAULT,  TARGET_DIR_NONE},        // 52 TARGET_GAMEOBJECT_DEST_AREA
    {TARGET_OBJECT_TYPE_DEST, TARGET_REFERENCE_TYPE_TARGET, TARGET_SELECT_CATEGORY_DEFAULT, TARGET_CHECK_ENEMY,    TARGET_DIR_NONE},        // 53 TARGET_DEST_TARGET_ENEMY
    {TARGET_OBJECT_TYPE_UNIT, TARGET_REFERENCE_TYPE_CASTER, TARGET_SELECT_CATEGORY_CONE,    TARGET_CHECK_ENEMY,    TARGET_DIR_FRONT},       // 54 TARGET_UNIT_CONE_ENEMY_54
    {TARGET_OBJECT_TYPE_DEST, TARGET_REFERENCE_TYPE_CASTER, TARGET_SELECT_CATEGORY_DEFAULT, TARGET_CHECK_DEFAULT,  TARGET_DIR_NONE},        // 55 TARGET_DEST_CASTER_FRONT_LEAP
    {TARGET_OBJECT_TYPE_UNIT, TARGET_REFERENCE_TYPE_CASTER, TARGET_SELECT_CATEGORY_AREA,    TARGET_CHECK_RAID,     TARGET_DIR_NONE},        // 56 TARGET_UNIT_CASTER_AREA_RAID
    {TARGET_OBJECT_TYPE_UNIT, TARGET_REFERENCE_TYPE_TARGET, TARGET_SELECT_CATEGORY_DEFAULT, TARGET_CHECK_RAID,     TARGET_DIR_NONE},        // 57 TARGET_UNIT_TARGET_RAID
    {TARGET_OBJECT_TYPE_UNIT, TARGET_REFERENCE_TYPE_CASTER, TARGET_SELECT_CATEGORY_NEARBY,  TARGET_CHECK_RAID,     TARGET_DIR_NONE},        // 58 TARGET_UNIT_NEARBY_RAID
    {TARGET_OBJECT_TYPE_UNIT, TARGET_REFERENCE_TYPE_CASTER, TARGET_SELECT_CATEGORY_CONE,    TARGET_CHECK_ALLY,     TARGET_DIR_FRONT},       // 59 TARGET_UNIT_CONE_ALLY
    {TARGET_OBJECT_TYPE_UNIT, TARGET_REFERENCE_TYPE_CASTER, TARGET_SELECT_CATEGORY_CONE,    TARGET_CHECK_ENTRY,    TARGET_DIR_FRONT},       // 60 TARGET_UNIT_CONE_ENTRY
    {TARGET_OBJECT_TYPE_UNIT, TARGET_REFERENCE_TYPE_TARGET, TARGET_SELECT_CATEGORY_AREA,    TARGET_CHECK_RAID_CLASS, TARGET_DIR_NONE},      // 61 TARGET_UNIT_TARGET_AREA_RAID_CLASS
    {TARGET_OBJECT_TYPE_NONE, TARGET_REFERENCE_TYPE_NONE,   TARGET_SELECT_CATEGORY_NYI,     TARGET_CHECK_DEFAULT,  TARGET_DIR_NONE},        // 62 TARGET_UNK_62
    {TARGET_OBJECT_TYPE_DEST, TARGET_REFERENCE_TYPE_TARGET, TARGET_SELECT_CATEGORY_DEFAULT, TARGET_CHECK_DEFAULT,  TARGET_DIR_NONE},        // 63 TARGET_DEST_TARGET_ANY
    {TARGET_OBJECT_TYPE_DEST, TARGET_REFERENCE_TYPE_TARGET, TARGET_SELECT_CATEGORY_DEFAULT, TARGET_CHECK_DEFAULT,  TARGET_DIR_FRONT},       // 64 TARGET_DEST_TARGET_FRONT
    {TARGET_OBJECT_TYPE_DEST, TARGET_REFERENCE_TYPE_TARGET, TARGET_SELECT_CATEGORY_DEFAULT, TARGET_CHECK_DEFAULT,  TARGET_DIR_BACK},        // 65 TARGET_DEST_TARGET_BACK
    {TARGET_OBJECT_TYPE_DEST, TARGET_REFERENCE_TYPE_TARGET, TARGET_SELECT_CATEGORY_DEFAULT, TARGET_CHECK_DEFAULT,  TARGET_DIR_RIGHT},       // 66 TARGET_DEST_TARGET_RIGHT
    {TARGET_OBJECT_TYPE_DEST, TARGET_REFERENCE_TYPE_TARGET, TARGET_SELECT_CATEGORY_DEFAULT, TARGET_CHECK_DEFAULT,  TARGET_DIR_LEFT},        // 67 TARGET_DEST_TARGET_LEFT
    {TARGET_OBJECT_TYPE_DEST, TARGET_REFERENCE_TYPE_TARGET, TARGET_SELECT_CATEGORY_DEFAULT, TARGET_CHECK_DEFAULT,  TARGET_DIR_FRONT_RIGHT}, // 68 TARGET_DEST_TARGET_FRONT_RIGHT
    {TARGET_OBJECT_TYPE_DEST, TARGET_REFERENCE_TYPE_TARGET, TARGET_SELECT_CATEGORY_DEFAULT, TARGET_CHECK_DEFAULT,  TARGET_DIR_BACK_RIGHT},  // 69 TARGET_DEST_TARGET_BACK_RIGHT
    {TARGET_OBJECT_TYPE_DEST, TARGET_REFERENCE_TYPE_TARGET, TARGET_SELECT_CATEGORY_DEFAULT, TARGET_CHECK_DEFAULT,  TARGET_DIR_BACK_LEFT},   // 70 TARGET_DEST_TARGET_BACK_LEFT
    {TARGET_OBJECT_TYPE_DEST, TARGET_REFERENCE_TYPE_TARGET, TARGET_SELECT_CATEGORY_DEFAULT, TARGET_CHECK_DEFAULT,  TARGET_DIR_FRONT_LEFT},  // 71 TARGET_DEST_TARGET_FRONT_LEFT
    {TARGET_OBJECT_TYPE_DEST, TARGET_REFERENCE_TYPE_CASTER, TARGET_SELECT_CATEGORY_DEFAULT, TARGET_CHECK_DEFAULT,  TARGET_DIR_RANDOM},      // 72 TARGET_DEST_CASTER_RANDOM
    {TARGET_OBJECT_TYPE_DEST, TARGET_REFERENCE_TYPE_CASTER, TARGET_SELECT_CATEGORY_DEFAULT, TARGET_CHECK_DEFAULT,  TARGET_DIR_RANDOM},      // 73 TARGET_DEST_CASTER_RADIUS
    {TARGET_OBJECT_TYPE_DEST, TARGET_REFERENCE_TYPE_TARGET, TARGET_SELECT_CATEGORY_DEFAULT, TARGET_CHECK_DEFAULT,  TARGET_DIR_RANDOM},      // 74 TARGET_DEST_TARGET_RANDOM
    {TARGET_OBJECT_TYPE_DEST, TARGET_REFERENCE_TYPE_TARGET, TARGET_SELECT_CATEGORY_DEFAULT, TARGET_CHECK_DEFAULT,  TARGET_DIR_RANDOM},      // 75 TARGET_DEST_TARGET_RADIUS
    {TARGET_OBJECT_TYPE_DEST, TARGET_REFERENCE_TYPE_CASTER, TARGET_SELECT_CATEGORY_CHANNEL, TARGET_CHECK_DEFAULT,  TARGET_DIR_NONE},        // 76 TARGET_DEST_CHANNEL_TARGET
    {TARGET_OBJECT_TYPE_UNIT, TARGET_REFERENCE_TYPE_CASTER, TARGET_SELECT_CATEGORY_CHANNEL, TARGET_CHECK_DEFAULT,  TARGET_DIR_NONE},        // 77 TARGET_UNIT_CHANNEL_TARGET
    {TARGET_OBJECT_TYPE_DEST, TARGET_REFERENCE_TYPE_DEST,   TARGET_SELECT_CATEGORY_DEFAULT, TARGET_CHECK_DEFAULT,  TARGET_DIR_FRONT},       // 78 TARGET_DEST_DEST_FRONT
    {TARGET_OBJECT_TYPE_DEST, TARGET_REFERENCE_TYPE_DEST,   TARGET_SELECT_CATEGORY_DEFAULT, TARGET_CHECK_DEFAULT,  TARGET_DIR_BACK},        // 79 TARGET_DEST_DEST_BACK
    {TARGET_OBJECT_TYPE_DEST, TARGET_REFERENCE_TYPE_DEST,   TARGET_SELECT_CATEGORY_DEFAULT, TARGET_CHECK_DEFAULT,  TARGET_DIR_RIGHT},       // 80 TARGET_DEST_DEST_RIGHT
    {TARGET_OBJECT_TYPE_DEST, TARGET_REFERENCE_TYPE_DEST,   TARGET_SELECT_CATEGORY_DEFAULT, TARGET_CHECK_DEFAULT,  TARGET_DIR_LEFT},        // 81 TARGET_DEST_DEST_LEFT
    {TARGET_OBJECT_TYPE_DEST, TARGET_REFERENCE_TYPE_DEST,   TARGET_SELECT_CATEGORY_DEFAULT, TARGET_CHECK_DEFAULT,  TARGET_DIR_FRONT_RIGHT}, // 82 TARGET_DEST_DEST_FRONT_RIGHT
    {TARGET_OBJECT_TYPE_DEST, TARGET_REFERENCE_TYPE_DEST,   TARGET_SELECT_CATEGORY_DEFAULT, TARGET_CHECK_DEFAULT,  TARGET_DIR_BACK_RIGHT},  // 83 TARGET_DEST_DEST_BACK_RIGHT
    {TARGET_OBJECT_TYPE_DEST, TARGET_REFERENCE_TYPE_DEST,   TARGET_SELECT_CATEGORY_DEFAULT, TARGET_CHECK_DEFAULT,  TARGET_DIR_BACK_LEFT},   // 84 TARGET_DEST_DEST_BACK_LEFT
    {TARGET_OBJECT_TYPE_DEST, TARGET_REFERENCE_TYPE_DEST,   TARGET_SELECT_CATEGORY_DEFAULT, TARGET_CHECK_DEFAULT,  TARGET_DIR_FRONT_LEFT},  // 85 TARGET_DEST_DEST_FRONT_LEFT
    {TARGET_OBJECT_TYPE_DEST, TARGET_REFERENCE_TYPE_DEST,   TARGET_SELECT_CATEGORY_DEFAULT, TARGET_CHECK_DEFAULT,  TARGET_DIR_RANDOM},      // 86 TARGET_DEST_DEST_RANDOM
    {TARGET_OBJECT_TYPE_DEST, TARGET_REFERENCE_TYPE_DEST,   TARGET_SELECT_CATEGORY_DEFAULT, TARGET_CHECK_DEFAULT,  TARGET_DIR_NONE},        // 87 TARGET_DEST_DEST
    {TARGET_OBJECT_TYPE_DEST, TARGET_REFERENCE_TYPE_DEST,   TARGET_SELECT_CATEGORY_DEFAULT, TARGET_CHECK_DEFAULT,  TARGET_DIR_NONE},        // 88 TARGET_DEST_DYNOBJ_NONE
    {TARGET_OBJECT_TYPE_DEST, TARGET_REFERENCE_TYPE_DEST,   TARGET_SELECT_CATEGORY_DEFAULT, TARGET_CHECK_DEFAULT,  TARGET_DIR_NONE},        // 89 TARGET_DEST_TRAJ
    {TARGET_OBJECT_TYPE_UNIT, TARGET_REFERENCE_TYPE_TARGET, TARGET_SELECT_CATEGORY_DEFAULT, TARGET_CHECK_DEFAULT,  TARGET_DIR_NONE},        // 90 TARGET_UNIT_TARGET_MINIPET
    {TARGET_OBJECT_TYPE_DEST, TARGET_REFERENCE_TYPE_DEST,   TARGET_SELECT_CATEGORY_DEFAULT, TARGET_CHECK_DEFAULT,  TARGET_DIR_RANDOM},      // 91 TARGET_DEST_DEST_RADIUS
    {TARGET_OBJECT_TYPE_UNIT, TARGET_REFERENCE_TYPE_CASTER, TARGET_SELECT_CATEGORY_DEFAULT, TARGET_CHECK_DEFAULT,  TARGET_DIR_NONE},        // 92 TARGET_UNIT_SUMMONER
    {TARGET_OBJECT_TYPE_CORPSE, TARGET_REFERENCE_TYPE_SRC,   TARGET_SELECT_CATEGORY_NYI,     TARGET_CHECK_ENEMY,    TARGET_DIR_NONE},       // 93 TARGET_CORPSE_SRC_AREA_ENEMY
#ifdef LICH_KING
    {TARGET_OBJECT_TYPE_UNIT, TARGET_REFERENCE_TYPE_CASTER, TARGET_SELECT_CATEGORY_DEFAULT, TARGET_CHECK_DEFAULT,  TARGET_DIR_NONE},        // 94 TARGET_UNIT_VEHICLE
    {TARGET_OBJECT_TYPE_UNIT, TARGET_REFERENCE_TYPE_TARGET, TARGET_SELECT_CATEGORY_DEFAULT, TARGET_CHECK_PASSENGER, TARGET_DIR_NONE},       // 95 TARGET_UNIT_TARGET_PASSENGER
    {TARGET_OBJECT_TYPE_UNIT, TARGET_REFERENCE_TYPE_CASTER, TARGET_SELECT_CATEGORY_DEFAULT, TARGET_CHECK_DEFAULT,  TARGET_DIR_NONE},        // 96 TARGET_UNIT_PASSENGER_0
    {TARGET_OBJECT_TYPE_UNIT, TARGET_REFERENCE_TYPE_CASTER, TARGET_SELECT_CATEGORY_DEFAULT, TARGET_CHECK_DEFAULT,  TARGET_DIR_NONE},        // 97 TARGET_UNIT_PASSENGER_1
    {TARGET_OBJECT_TYPE_UNIT, TARGET_REFERENCE_TYPE_CASTER, TARGET_SELECT_CATEGORY_DEFAULT, TARGET_CHECK_DEFAULT,  TARGET_DIR_NONE},        // 98 TARGET_UNIT_PASSENGER_2
    {TARGET_OBJECT_TYPE_UNIT, TARGET_REFERENCE_TYPE_CASTER, TARGET_SELECT_CATEGORY_DEFAULT, TARGET_CHECK_DEFAULT,  TARGET_DIR_NONE},        // 99 TARGET_UNIT_PASSENGER_3
    {TARGET_OBJECT_TYPE_UNIT, TARGET_REFERENCE_TYPE_CASTER, TARGET_SELECT_CATEGORY_DEFAULT, TARGET_CHECK_DEFAULT,  TARGET_DIR_NONE},        // 100 TARGET_UNIT_PASSENGER_4
    {TARGET_OBJECT_TYPE_UNIT, TARGET_REFERENCE_TYPE_CASTER, TARGET_SELECT_CATEGORY_DEFAULT, TARGET_CHECK_DEFAULT,  TARGET_DIR_NONE},        // 101 TARGET_UNIT_PASSENGER_5
    {TARGET_OBJECT_TYPE_UNIT, TARGET_REFERENCE_TYPE_CASTER, TARGET_SELECT_CATEGORY_DEFAULT, TARGET_CHECK_DEFAULT,  TARGET_DIR_NONE},        // 102 TARGET_UNIT_PASSENGER_6
    {TARGET_OBJECT_TYPE_UNIT, TARGET_REFERENCE_TYPE_CASTER, TARGET_SELECT_CATEGORY_DEFAULT, TARGET_CHECK_DEFAULT,  TARGET_DIR_NONE},        // 103 TARGET_UNIT_PASSENGER_7
    {TARGET_OBJECT_TYPE_UNIT, TARGET_REFERENCE_TYPE_CASTER, TARGET_SELECT_CATEGORY_CONE,    TARGET_CHECK_ENEMY,    TARGET_DIR_FRONT},       // 104 TARGET_UNIT_CONE_ENEMY_104
    {TARGET_OBJECT_TYPE_UNIT, TARGET_REFERENCE_TYPE_NONE,   TARGET_SELECT_CATEGORY_NYI,     TARGET_CHECK_DEFAULT,  TARGET_DIR_NONE},        // 105 TARGET_UNIT_UNK_105
    {TARGET_OBJECT_TYPE_DEST, TARGET_REFERENCE_TYPE_CASTER, TARGET_SELECT_CATEGORY_CHANNEL, TARGET_CHECK_DEFAULT,  TARGET_DIR_NONE},        // 106 TARGET_DEST_CHANNEL_CASTER
    {TARGET_OBJECT_TYPE_NONE, TARGET_REFERENCE_TYPE_DEST,   TARGET_SELECT_CATEGORY_NYI,     TARGET_CHECK_DEFAULT,  TARGET_DIR_NONE},        // 107 TARGET_UNK_DEST_AREA_UNK_107
    {TARGET_OBJECT_TYPE_GOBJ, TARGET_REFERENCE_TYPE_CASTER, TARGET_SELECT_CATEGORY_CONE,    TARGET_CHECK_DEFAULT,  TARGET_DIR_FRONT},       // 108 TARGET_GAMEOBJECT_CONE
    {TARGET_OBJECT_TYPE_NONE, TARGET_REFERENCE_TYPE_NONE,   TARGET_SELECT_CATEGORY_NYI,     TARGET_CHECK_DEFAULT,  TARGET_DIR_NONE},        // 109
    {TARGET_OBJECT_TYPE_DEST, TARGET_REFERENCE_TYPE_NONE,   TARGET_SELECT_CATEGORY_NYI,     TARGET_CHECK_DEFAULT,  TARGET_DIR_NONE},        // 110 TARGET_DEST_UNK_110
#endif
};

SpellEffectInfo::SpellEffectInfo(SpellEntry const* spellEntry, SpellInfo const* spellInfo, uint8 effIndex)
{
    _spellInfo = spellInfo;
    _effIndex = effIndex;
    Effect = spellEntry->Effect[effIndex];
    ApplyAuraName = spellEntry->EffectApplyAuraName[effIndex];
    Amplitude = spellEntry->EffectAmplitude[effIndex];
    DieSides = spellEntry->EffectDieSides[effIndex];
#ifndef LICH_KING
    BaseDice = spellEntry->EffectBaseDice[effIndex];
    DicePerLevel = spellEntry->EffectDicePerLevel[effIndex];
#endif
    RealPointsPerLevel = spellEntry->EffectRealPointsPerLevel[effIndex];
    BasePoints = spellEntry->EffectBasePoints[effIndex];
    PointsPerComboPoint = spellEntry->EffectPointsPerComboPoint[effIndex];
    ValueMultiplier = spellEntry->EffectValueMultiplier[effIndex];
    DamageMultiplier = spellEntry->EffectDamageMultiplier[effIndex];
#ifdef LICH_KING
    BonusMultiplier = spellEntry->Effects[effIndex].BonusMultiplier;
#endif
    MiscValue = spellEntry->EffectMiscValue[effIndex];
    MiscValueB = spellEntry->EffectMiscValueB[effIndex];
    Mechanic = Mechanics(spellEntry->EffectMechanic[effIndex]);
    TargetA = SpellImplicitTargetInfo(spellEntry->EffectImplicitTargetA[effIndex]);
    TargetB = SpellImplicitTargetInfo(spellEntry->EffectImplicitTargetB[effIndex]);
    RadiusEntry = spellEntry->EffectRadiusIndex[effIndex] ? sSpellRadiusStore.LookupEntry(spellEntry->EffectRadiusIndex[effIndex]) : NULL;
    ChainTarget = spellEntry->EffectChainTarget[effIndex];
    ItemType = spellEntry->EffectItemType[effIndex];
    TriggerSpell = spellEntry->EffectTriggerSpell[effIndex];
#ifdef LICH_KING
    SpellClassMask = spellEntry->EffectSpellClassMask[effIndex];
#endif
    ImplicitTargetConditions = NULL;
}

bool SpellEffectInfo::IsEffect() const
{
    return Effect != 0;
}

bool SpellEffectInfo::IsEffect(SpellEffects effectName) const
{
    return Effect == uint32(effectName);
}

bool SpellEffectInfo::IsAura() const
{
    return (IsUnitOwnedAuraEffect() || Effect == SPELL_EFFECT_PERSISTENT_AREA_AURA) && ApplyAuraName != 0;
}

bool SpellEffectInfo::IsAura(AuraType aura) const
{
    return IsAura() && ApplyAuraName == uint32(aura);
}

bool SpellEffectInfo::IsTargetingArea() const
{
    return TargetA.IsArea() || TargetB.IsArea();
}

bool SpellEffectInfo::IsAreaAuraEffect() const
{
    if (Effect == SPELL_EFFECT_APPLY_AREA_AURA_PARTY ||
#ifdef LICH_KING
        Effect == SPELL_EFFECT_APPLY_AREA_AURA_RAID ||
#endif
        Effect == SPELL_EFFECT_APPLY_AREA_AURA_FRIEND ||
        Effect == SPELL_EFFECT_APPLY_AREA_AURA_ENEMY ||
        Effect == SPELL_EFFECT_APPLY_AREA_AURA_PET ||
        Effect == SPELL_EFFECT_APPLY_AREA_AURA_OWNER)
        return true;

    return false;
}

bool SpellEffectInfo::IsFarUnitTargetEffect() const
{
    return (Effect == SPELL_EFFECT_SUMMON_PLAYER)
#ifdef LICH_KING
        || (Effect == SPELL_EFFECT_SUMMON_RAF_FRIEND)
#endif
        || (Effect == SPELL_EFFECT_RESURRECT)
        || (Effect == SPELL_EFFECT_RESURRECT_NEW)
        || (Effect == SPELL_EFFECT_SKIN_PLAYER_CORPSE);
}

bool SpellEffectInfo::IsFarDestTargetEffect() const
{
    return Effect == SPELL_EFFECT_TELEPORT_UNITS;
}

bool SpellEffectInfo::IsUnitOwnedAuraEffect() const
{
    return IsAreaAuraEffect() || Effect == SPELL_EFFECT_APPLY_AURA;
}

SpellSpecificType SpellInfo::GetSpellSpecific() const
{
    switch(SpellFamilyName)
    {
        case SPELLFAMILY_GENERIC:
        {
            //food/drink
            if (AuraInterruptFlags & AURA_INTERRUPT_FLAG_NOT_SEATED)
            {
                for(int i = 0; i < MAX_SPELL_EFFECTS; i++)
                    if( Effects[i].ApplyAuraName==SPELL_AURA_MOD_POWER_REGEN)
                        return SPELL_DRINK;
                    else if ( Effects[i].ApplyAuraName==SPELL_AURA_MOD_REGEN)
                        return SPELL_FOOD;
            }
            // this may be a hack
            else if((HasAttribute(SPELL_ATTR2_FOOD_BUFF))
                && !Category)
                return SPELL_WELL_FED;

            switch(Id)
            {
                case 12880: // warrior's Enrage rank 1
                case 14201: //           Enrage rank 2
                case 14202: //           Enrage rank 3
                case 14203: //           Enrage rank 4
                case 14204: //           Enrage rank 5
                case 12292: //             Death Wish
                    return SPELL_WARRIOR_ENRAGE;
                break;
                default: break;
            }
            break;
        }
        case SPELLFAMILY_MAGE:
        {
            // family flags 18(Molten), 25(Frost/Ice), 28(Mage)
            if (SpellFamilyFlags & 0x12040000)
                return SPELL_MAGE_ARMOR;

            if ((SpellFamilyFlags & 0x1000000) && Effects[0].ApplyAuraName==SPELL_AURA_MOD_CONFUSE)
                return SPELL_MAGE_POLYMORPH;

            break;
        }
        case SPELLFAMILY_WARRIOR:
        {
            if (SpellFamilyFlags & 0x00008000010000LL)
                return SPELL_POSITIVE_SHOUT;
            // Sunder Armor (vs Expose Armor)
            if (SpellFamilyFlags & 0x00000000004000LL)
                return SPELL_ARMOR_REDUCE;
            break;
        }
        case SPELLFAMILY_WARLOCK:
        {
            // only warlock curses have this
            if (Dispel == DISPEL_CURSE)
                return SPELL_CURSE;

            // family flag 37 (only part spells have family name)
            if (SpellFamilyFlags & 0x2000000000LL)
                return SPELL_WARLOCK_ARMOR;

            //seed of corruption and corruption
            if (SpellFamilyFlags & 0x1000000002LL)
                return SPELL_WARLOCK_CORRUPTION;
            break;
        }
        case SPELLFAMILY_HUNTER:
        {
            // only hunter stings have this
            if (Dispel == DISPEL_POISON)
                return SPELL_STING;

            break;
        }
        case SPELLFAMILY_PALADIN:
        {
            //Collection of all the seal family flags. No other paladin spell has any of those.
            if( SpellFamilyFlags & 0x4000A000200LL )
                return SPELL_SEAL;

            if (SpellFamilyFlags & 0x10000100LL)
                return SPELL_BLESSING;

            if ((SpellFamilyFlags & 0x00000820180400LL) && (HasAttribute(SPELL_ATTR3_UNK9)))
                return SPELL_JUDGEMENT;

            for (int i = 0; i < MAX_SPELL_EFFECTS; i++)
            {
                // only paladin auras have this
                if (Effects[i].Effect == SPELL_EFFECT_APPLY_AREA_AURA_PARTY)
                    return SPELL_AURA;
            }
            break;
        }
        case SPELLFAMILY_SHAMAN:
        {
            if (SpellFamilyFlags & 0x42000000400LL 
                || Id == 23552)
                return SPELL_ELEMENTAL_SHIELD;

            break;
        }
        case SPELLFAMILY_DRUID:
        {
            // Mangle
            if (SpellFamilyFlags & 0x0000044000000000LL)
                return SPELL_DRUID_MANGLE;
        
            break;
        }
        case SPELLFAMILY_ROGUE:
        {
            // Expose Armor (vs Sunder Armor)
            if (SpellFamilyFlags & 0x00000000080000LL)
                return SPELL_ARMOR_REDUCE;
            break;
        }
        case SPELLFAMILY_POTION:
            return GetSpellElixirSpecific();
    }

    // only warlock armor/skin have this (in additional to family cases)
    if (HasVisual(130) && SpellIconID == 89)
    {
        return SPELL_WARLOCK_ARMOR;
    }

    // only hunter aspects have this (but not all aspects in hunter family)
    if( ActiveIconID == 122 && (GetSchoolMask() & SPELL_SCHOOL_MASK_NATURE) &&
        (Attributes & 0x50000) != 0 && (Attributes & 0x9000010) == 0)
    {
        return SPELL_ASPECT;
    }

    for(int i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if(Effects[i].Effect == SPELL_EFFECT_APPLY_AURA)
        {
            switch(Effects[i].ApplyAuraName)
            {
                case SPELL_AURA_TRACK_CREATURES:
                case SPELL_AURA_TRACK_RESOURCES:
                case SPELL_AURA_TRACK_STEALTHED:
                    return SPELL_TRACKER;
                case SPELL_AURA_MOD_CHARM:
                case SPELL_AURA_MOD_POSSESS_PET:
                case SPELL_AURA_MOD_POSSESS:
                    return SPELL_CHARM;
            }
        }
    }

    // elixirs can have different families, but potion most ofc.
    if(SpellSpecificType sp = GetSpellElixirSpecific())
        return sp;

    return SPELL_NORMAL;
}

SpellSpecificType SpellInfo::GetSpellElixirSpecific() const
{
    uint32 mask = sSpellMgr->GetSpellElixirMask(Id);
    if (mask & ELIXIR_SHATTRATH_MASK)
        return SPELL_FLASK_ELIXIR;
    else if((mask & ELIXIR_FLASK_MASK)==ELIXIR_FLASK_MASK)
        return SPELL_FLASK_ELIXIR;
    else if(mask & ELIXIR_BATTLE_MASK)
        return SPELL_BATTLE_ELIXIR;
    else if(mask & ELIXIR_GUARDIAN_MASK)
        return SPELL_GUARDIAN_ELIXIR;
    else
        return SPELL_NORMAL;
}

SpellInfo::SpellInfo(SpellEntry const* spellEntry)
{
    Id = spellEntry->Id;
    Category = spellEntry->Category ? sSpellCategoryStore.LookupEntry(spellEntry->Category) : NULL;
    Dispel = spellEntry->Dispel;
    Mechanic = spellEntry->Mechanic;
    Attributes = spellEntry->Attributes;
    AttributesEx = spellEntry->AttributesEx;
    AttributesEx2 = spellEntry->AttributesEx2;
    AttributesEx3 = spellEntry->AttributesEx3;
    AttributesEx4 = spellEntry->AttributesEx4;
    AttributesEx5 = spellEntry->AttributesEx5;
    AttributesEx6 = spellEntry->AttributesEx6;
#ifdef LICH_KING
    AttributesEx7 = spellEntry->AttributesEx7;
#else
    AttributesEx7 = 0;
#endif
    AttributesCu = 0;
    Stances = spellEntry->Stances;
    StancesNot = spellEntry->StancesNot;
    Targets = spellEntry->Targets;
    TargetCreatureType = spellEntry->TargetCreatureType;
    RequiresSpellFocus = spellEntry->RequiresSpellFocus;
    FacingCasterFlags = spellEntry->FacingCasterFlags;
    CasterAuraState = spellEntry->CasterAuraState;
    TargetAuraState = spellEntry->TargetAuraState;
    CasterAuraStateNot = spellEntry->CasterAuraStateNot;
    TargetAuraStateNot = spellEntry->TargetAuraStateNot;
#ifdef LICH_KING
    CasterAuraSpell = spellEntry->casterAuraSpell;
    TargetAuraSpell = spellEntry->targetAuraSpell;
    ExcludeCasterAuraSpell = spellEntry->excludeCasterAuraSpell;
    ExcludeTargetAuraSpell = spellEntry->excludeTargetAuraSpell;
#else
#endif
    CastTimeEntry = spellEntry->CastingTimeIndex ? sSpellCastTimesStore.LookupEntry(spellEntry->CastingTimeIndex) : NULL;
    RecoveryTime = spellEntry->RecoveryTime;
    CategoryRecoveryTime = spellEntry->CategoryRecoveryTime;
    StartRecoveryCategory = spellEntry->StartRecoveryCategory;
    StartRecoveryTime = spellEntry->StartRecoveryTime;
    InterruptFlags = spellEntry->InterruptFlags;
    AuraInterruptFlags = spellEntry->AuraInterruptFlags;
    ChannelInterruptFlags = spellEntry->ChannelInterruptFlags;
    ProcFlags = spellEntry->ProcFlags;
    ProcChance = spellEntry->procChance;
    ProcCharges = spellEntry->procCharges;
    MaxLevel = spellEntry->MaxLevel;
    BaseLevel = spellEntry->BaseLevel;
    SpellLevel = spellEntry->SpellLevel;
    DurationEntry = spellEntry->DurationIndex ? sSpellDurationStore.LookupEntry(spellEntry->DurationIndex) : NULL;
    PowerType = spellEntry->PowerType;
    ManaCost = spellEntry->ManaCost;
    ManaCostPerlevel = spellEntry->ManaCostPerlevel;
    ManaPerSecond = spellEntry->manaPerSecond;
    ManaPerSecondPerLevel = spellEntry->ManaPerSecondPerLevel;
    ManaCostPercentage = spellEntry->ManaCostPercentage;
#ifdef LICH_KING
    RuneCostID = spellEntry->runeCostID;
#endif
    RangeEntry = spellEntry->rangeIndex ? sSpellRangeStore.LookupEntry(spellEntry->rangeIndex) : NULL;
    Speed = spellEntry->speed;
    StackAmount = spellEntry->StackAmount;
    for (uint8 i = 0; i < 2; ++i)
        Totem[i] = spellEntry->Totem[i];

    for (uint8 i = 0; i < MAX_SPELL_REAGENTS; ++i)
    {
        Reagent[i] = spellEntry->Reagent[i];
        ReagentCount[i] = spellEntry->ReagentCount[i];
    }

    EquippedItemClass = spellEntry->EquippedItemClass;
    EquippedItemSubClassMask = spellEntry->EquippedItemSubClassMask;
    EquippedItemInventoryTypeMask = spellEntry->EquippedItemInventoryTypeMask;
    for (uint8 i = 0; i < 2; ++i)
        TotemCategory[i] = spellEntry->TotemCategory[i];

    for (uint8 i = 0; i < 2; ++i)
#ifdef LICH_KING
        SpellVisual[i] = spellEntry->SpellVisual[i];
#else
        SpellVisual = spellEntry->SpellVisual;
#endif

    SpellIconID = spellEntry->SpellIconID;
    ActiveIconID = spellEntry->activeIconID;
    for (uint8 i = 0; i < 16; ++i)
        SpellName[i] = spellEntry->SpellName[i];

    for (uint8 i = 0; i < 16; ++i)
        Rank[i] = spellEntry->Rank[i];

    MaxTargetLevel = spellEntry->MaxTargetLevel;
    MaxAffectedTargets = spellEntry->MaxAffectedTargets;
    SpellFamilyName = spellEntry->SpellFamilyName;
    SpellFamilyFlags = spellEntry->SpellFamilyFlags;
    DmgClass = spellEntry->DmgClass;
    PreventionType = spellEntry->PreventionType;
#ifdef LICH_KING
    AreaGroupId = spellEntry->AreaGroupId;
#else
    AreaId = spellEntry->AreaId;
#endif
    SchoolMask = spellEntry->SchoolMask;
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        Effects[i] = SpellEffectInfo(spellEntry, this, i);

    //custom attributes from database. More custom attributes to be loaded 
    AttributesCu = spellEntry->CustomAttributesFlags;
    ChainEntry = NULL;

    ExplicitTargetMask = _GetExplicitTargetMask();

    LoadCustomAttributes();
}


bool SpellInfo::IsRanked() const
{
    return ChainEntry != NULL;
}

uint8 SpellInfo::GetRank() const
{
    if (!ChainEntry)
        return 1;
    return ChainEntry->rank;
}

SpellInfo const* SpellInfo::GetFirstRankSpell() const
{
    if (!ChainEntry)
        return this;
    return ChainEntry->first;
}

SpellInfo const* SpellInfo::GetLastRankSpell() const
{
    if (!ChainEntry)
        return NULL;
    return ChainEntry->last;
}

SpellInfo const* SpellInfo::GetNextRankSpell() const
{
    if (!ChainEntry)
        return NULL;
    return ChainEntry->next;
}

SpellInfo const* SpellInfo::GetPrevRankSpell() const
{
    if (!ChainEntry)
        return NULL;
    return ChainEntry->prev;
}

SpellInfo const* SpellInfo::GetAuraRankForLevel(uint8 level) const
{
    // ignore passive spells
    if (IsPassive())
        return this;

    bool needRankSelection = false;
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (IsPositiveEffect(i) &&
            (   Effects[i].Effect == SPELL_EFFECT_APPLY_AURA
             || Effects[i].Effect == SPELL_EFFECT_APPLY_AREA_AURA_PARTY 
#ifdef LICH_KING
             || Effects[i].Effect == SPELL_EFFECT_APPLY_AREA_AURA_RAID))
#endif
            )
           )
        {
            needRankSelection = true;
            break;
        }
    }

    // not required
    if (!needRankSelection)
        return this;

    for (SpellInfo const* nextSpellInfo = this; nextSpellInfo != NULL; nextSpellInfo = nextSpellInfo->GetPrevRankSpell())
    {
        // if found appropriate level
        if (uint32(level + 10) >= nextSpellInfo->SpellLevel)
            return nextSpellInfo;

        // one rank less then
    }

    // not found
    return NULL;
}

bool SpellInfo::IsRankOf(SpellInfo const* spellInfo) const
{
    return GetFirstRankSpell() == spellInfo->GetFirstRankSpell();
}

bool SpellInfo::IsDifferentRankOf(SpellInfo const* spellInfo) const
{
    if (Id == spellInfo->Id)
        return false;
    return IsRankOf(spellInfo);
}

bool SpellInfo::IsHighRankOf(SpellInfo const* spellInfo) const
{
    if (ChainEntry && spellInfo->ChainEntry)
    {
        if (ChainEntry->first == spellInfo->ChainEntry->first)
            if (ChainEntry->rank > spellInfo->ChainEntry->rank)
                return true;
    }
    return false;
}

int32 SpellInfo::GetDuration() const
{
    if (!DurationEntry)
        return 0;
    return (DurationEntry->Duration[0] == -1) ? -1 : abs(DurationEntry->Duration[0]);
}

int32 SpellInfo::GetMaxDuration() const
{
    if (!DurationEntry)
        return 0;
    return (DurationEntry->Duration[2] == -1) ? -1 : abs(DurationEntry->Duration[2]);
}


uint32 SpellInfo::GetMaxTicks() const
{
    int32 DotDuration = GetDuration();
    if (DotDuration == 0)
        return 1;

    // 200% limit
    if (DotDuration > 30000)
        DotDuration = 30000;

    for (uint8 x = 0; x < MAX_SPELL_EFFECTS; x++)
    {
        if (Effects[x].Effect == SPELL_EFFECT_APPLY_AURA)
            switch (Effects[x].ApplyAuraName)
            {
            case SPELL_AURA_PERIODIC_DAMAGE:
            case SPELL_AURA_PERIODIC_HEAL:
            case SPELL_AURA_PERIODIC_LEECH:
                if (Effects[x].Amplitude != 0)
                    return DotDuration / Effects[x].Amplitude;
                break;
            }
    }

    return 6;
}

uint32 SpellInfo::GetRecoveryTime() const
{
    return RecoveryTime > CategoryRecoveryTime ? RecoveryTime : CategoryRecoveryTime;
}

uint32 SpellInfo::CalcCastTime(Spell* spell) const
{
    // not all spells have cast time index and this is all is pasiive abilities
    if (!CastTimeEntry)
        return 0;

    int32 castTime = CastTimeEntry->CastTime;

    //todo, remove hack !
    if (spell && spell->m_spellInfo->Id == 8690) //heartstone
        return castTime;

    if (spell)
        spell->GetCaster()->ModSpellCastTime(this, castTime, spell);
    
    if (HasAttribute(SPELL_ATTR0_RANGED) && (!IsAutoRepeatRangedSpell()))
        castTime += 500;

    return (castTime > 0) ? uint32(castTime) : 0;
}


bool SpellInfo::HasInitialAggro() const
{
    return !(HasAttribute(SPELL_ATTR1_NO_THREAT) || HasAttribute(SPELL_ATTR3_NO_INITIAL_AGGRO));
}

bool SpellInfo::IsRangedWeaponSpell() const
{
    return ((EquippedItemSubClassMask & ITEM_SUBCLASS_MASK_WEAPON_RANGED)
        || (Attributes & SPELL_ATTR0_RANGED)); // Xinef: added
}

bool SpellInfo::IsAutoRepeatRangedSpell() const
{
    return HasAttribute(SPELL_ATTR2_AUTOREPEAT_FLAG);
}

SpellInfo::~SpellInfo()
{
    _UnloadImplicitTargetConditionLists();
}

// checks if spell targets are selected from area, doesn't include spell effects in check (like area wide auras for example)
bool SpellInfo::IsTargetingArea() const
{
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        if (Effects[i].IsEffect() && Effects[i].IsTargetingArea())
            return true;
    return false;
}

bool SpellInfo::IsAffectingArea() const
{
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        if (Effects[i].IsEffect() && (Effects[i].IsTargetingArea() || Effects[i].IsEffect(SPELL_EFFECT_PERSISTENT_AREA_AURA) || Effects[i].IsAreaAuraEffect()))
            return true;
    return false;
}

bool SpellInfo::NeedsExplicitUnitTarget() const
{
    return GetExplicitTargetMask() & TARGET_FLAG_UNIT_MASK;
}

bool SpellInfo::NeedsToBeTriggeredByCaster(SpellInfo const* triggeringSpell, uint8 effIndex) const
{
    if (NeedsExplicitUnitTarget())
        return true;

    // pussywizard:
    if (effIndex < MAX_SPELL_EFFECTS && (triggeringSpell->Effects[effIndex].TargetA.GetCheckType() == TARGET_CHECK_ENTRY || triggeringSpell->Effects[effIndex].TargetB.GetCheckType() == TARGET_CHECK_ENTRY))
    {
#ifdef LICH_KING
        if (Id == 60563)
            return true;
#endif
        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
            if (Effects[i].IsEffect() && (Effects[i].TargetA.GetCheckType() == TARGET_CHECK_ENTRY || Effects[i].TargetB.GetCheckType() == TARGET_CHECK_ENTRY))
                return true;
    }

    if (triggeringSpell->IsChanneled())
    {
        uint32 mask = 0;
        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (Effects[i].TargetA.GetTarget() != TARGET_UNIT_CASTER && Effects[i].TargetA.GetTarget() != TARGET_DEST_CASTER)
                mask |= Effects[i].GetProvidedTargetMask();
        }

        if (mask & TARGET_FLAG_UNIT_MASK)
            return true;
    }

    return false;
}

bool SpellInfo::CanBeUsedInCombat() const
{
    return !HasAttribute(SPELL_ATTR0_CANT_USED_IN_COMBAT);
}

bool SpellInfo::HasAreaAuraEffect() const
{
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        if (Effects[i].IsAreaAuraEffect())
            return true;
    return false;
}

uint32 SpellInfo::GetCategory() const
{
    return Category ? Category->Id : 0;
}

bool SpellInfo::HasEffectByEffectMask(SpellEffects effect, SpellEffectMask effectMask) const
{
    assert(effectMask <= SPELL_EFFECT_MASK_ALL);

    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if( (effectMask & (1 << i)) && Effects[i].IsEffect(effect))
            return true;
    }

    return false;
}

bool SpellInfo::HasEffect(SpellEffects effect, uint8 effectIndex) const
{
    return HasEffectByEffectMask(effect, SpellEffectMask(1 << effectIndex));
}

bool SpellInfo::HasAura(AuraType aura) const
{
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        if (Effects[i].IsAura(aura))
            return true;
    return false;
}

bool SpellInfo::HasAuraEffect(AuraType aura) const
{
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        if (Effects[i].IsAura(aura))
            return true;
    return false;
}

bool SpellInfo::IsBreakingStealth() const
{
    return !(AttributesEx & SPELL_ATTR1_NOT_BREAK_STEALTH);
}

bool SpellInfo::IsValidDeadOrAliveTarget(Unit const* target) const
{
    if (target->IsAlive())
        return !IsRequiringDeadTarget();

    return IsAllowingDeadTarget();
}

bool SpellInfo::IsRequiringDeadTarget() const
{
    return AttributesEx3 & SPELL_ATTR3_ONLY_TARGET_GHOSTS;
}

bool SpellInfo::IsAllowingDeadTarget() const
{
    return AttributesEx2 & (SPELL_ATTR2_CAN_TARGET_DEAD) || Attributes & (SPELL_ATTR0_CASTABLE_WHILE_DEAD) || Targets & (TARGET_FLAG_CORPSE_ALLY | TARGET_FLAG_CORPSE_ENEMY | TARGET_FLAG_UNIT_DEAD);
}

bool SpellInfo::IsChanneled() const
{
    return (AttributesEx & (SPELL_ATTR1_CHANNELED_1 | SPELL_ATTR1_CHANNELED_2)) != 0;
}

bool SpellInfo::NeedsComboPoints() const
{
    return (AttributesEx & (SPELL_ATTR1_REQ_COMBO_POINTS1 | SPELL_ATTR1_REQ_COMBO_POINTS2)) != 0;
}

bool SpellInfo::IsDeathPersistent() const
{
    switch(Id)
    {
        case 40214:                                     // Dragonmaw Illusion
        case 35480: case 35481: case 35482:             // Human Illusion
        case 35483: case 39824:                         // Human Illusion
        case 17619:
        case 37128:                                     // Doomwalker's Mark of Death
            return true;
    }

    return (AttributesEx3 & SPELL_ATTR3_DEATH_PERSISTENT) != 0;
}

bool SpellInfo::HasVisual(uint32 visual) const
{
#ifdef LICH_KING
    for (uint8 i = 0; i < 2; i++)
        if (SpellVisual[i] == visual)
            return true;

    return false;
#else
    return SpellVisual == visual;
#endif
}

SpellCastResult SpellInfo::CheckTarget(Unit const* caster, WorldObject const* target, bool implicit) const
{
    if (AttributesEx & SPELL_ATTR1_CANT_TARGET_SELF && caster == target)
        return SPELL_FAILED_BAD_TARGETS;

    // check visibility - ignore stealth for implicit (area) targets
#ifdef LICH_KING
    if (!(AttributesEx6 & SPELL_ATTR6_CAN_TARGET_INVISIBLE) && !caster->CanSeeOrDetect(target, implicit))
        return SPELL_FAILED_BAD_TARGETS;
#endif
    Unit const* unitTarget = target->ToUnit();

    // creature/player specific target checks
    if (unitTarget)
    {
        // xinef: spells cannot be cast if player is in fake combat also
        if (AttributesEx & SPELL_ATTR1_CANT_TARGET_IN_COMBAT && (unitTarget->IsInCombat() || unitTarget->IsPetInCombat()))
            return SPELL_FAILED_TARGET_AFFECTING_COMBAT;

        // only spells with SPELL_ATTR3_ONLY_TARGET_GHOSTS can target ghosts
        if (((IsRequiringDeadTarget() != 0) != unitTarget->HasAuraType(SPELL_AURA_GHOST)) && !(IsDeathPersistent() && IsAllowingDeadTarget()))
        {
            if (AttributesEx3 & SPELL_ATTR3_ONLY_TARGET_GHOSTS)
                return SPELL_FAILED_TARGET_NOT_GHOST;
            else
                return SPELL_FAILED_BAD_TARGETS;
        }

        if (caster != unitTarget)
        {
            if (caster->GetTypeId() == TYPEID_PLAYER)
            {
                // Do not allow these spells to target creatures not tapped by us (Banish, Polymorph, many quest spells)
                if (AttributesEx2 & SPELL_ATTR2_CANT_TARGET_TAPPED)
                    if (Creature const* targetCreature = unitTarget->ToCreature())
                        if (targetCreature->hasLootRecipient() && !targetCreature->isTappedBy(caster->ToPlayer()))
                            return SPELL_FAILED_CANT_CAST_ON_TAPPED;

                if (AttributesCu & SPELL_ATTR_CU_PICKPOCKET)
                {
                    if (unitTarget->GetTypeId() == TYPEID_PLAYER)
                        return SPELL_FAILED_BAD_TARGETS;
                    else if ((unitTarget->GetCreatureTypeMask() & CREATURE_TYPEMASK_HUMANOID_OR_UNDEAD) == 0)
                        return SPELL_FAILED_TARGET_NO_POCKETS;
                }

                // Not allow disarm unarmed player
                if (Mechanic == MECHANIC_DISARM)
                {
                    if (!unitTarget->HasMainWeapon())
                        return SPELL_FAILED_TARGET_NO_WEAPONS;
                }
            }
        }
    }
    // corpse specific target checks
    else if (Corpse const* corpseTarget = target->ToCorpse())
    {
        // cannot target bare bones
        if (corpseTarget->GetType() == CORPSE_BONES)
            return SPELL_FAILED_BAD_TARGETS;
        // we have to use owner for some checks (aura preventing resurrection for example)
        if (Player* owner = ObjectAccessor::FindPlayer(corpseTarget->GetOwnerGUID()))
            unitTarget = owner;
        // we're not interested in corpses without owner
        else
            return SPELL_FAILED_BAD_TARGETS;
    }
    // other types of objects - always valid
    else return SPELL_CAST_OK;

    // corpseOwner and unit specific target checks
    if (AttributesEx3 & SPELL_ATTR3_ONLY_TARGET_PLAYERS && !unitTarget->ToPlayer())
        return SPELL_FAILED_TARGET_NOT_PLAYER;

    if (!IsAllowingDeadTarget() && !unitTarget->IsAlive())
        return SPELL_FAILED_TARGETS_DEAD;

    // check this flag only for implicit targets (chain and area), allow to explicitly target units for spells like Shield of Righteousness
    if (implicit && AttributesEx6 & SPELL_ATTR6_CANT_TARGET_CROWD_CONTROLLED && !unitTarget->CanFreeMove())
        return SPELL_FAILED_BAD_TARGETS;

    // checked in Unit::IsValidAttack/AssistTarget, shouldn't be checked for ENTRY targets
    //if (!(AttributesEx6 & SPELL_ATTR6_CAN_TARGET_UNTARGETABLE) && target->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE))
    //    return SPELL_FAILED_BAD_TARGETS;

    //if (!(AttributesEx6 & SPELL_ATTR6_CAN_TARGET_POSSESSED_FRIENDS)

    if (!CheckTargetCreatureType(unitTarget))
    {
        if (target->GetTypeId() == TYPEID_PLAYER)
            return SPELL_FAILED_TARGET_IS_PLAYER;
        else
            return SPELL_FAILED_BAD_TARGETS;
    }

    // check GM mode and GM invisibility - only for player casts (npc casts are controlled by AI) and negative spells
    if (unitTarget != caster && (caster->IsControlledByPlayer() || !IsPositive()) && unitTarget->GetTypeId() == TYPEID_PLAYER)
    {
        if (!unitTarget->ToPlayer()->IsVisible())
            return SPELL_FAILED_BM_OR_INVISGOD;

        if (unitTarget->ToPlayer()->IsGameMaster())
            return SPELL_FAILED_BM_OR_INVISGOD;
    }

    // not allow casting on flying player
    if (unitTarget->IsInFlight() /* sunwell && !HasAttribute(SPELL_ATTR0_CU_ALLOW_INFLIGHT_TARGET) */)
        return SPELL_FAILED_BAD_TARGETS;

    /* TARGET_UNIT_MASTER gets blocked here for passengers, because the whole idea of this check is to
    not allow passengers to be implicitly hit by spells, however this target type should be an exception,
    if this is left it kills spells that award kill credit from vehicle to master (few spells),
    the use of these 2 covers passenger target check, logically, if vehicle cast this to master it should always hit
    him, because it would be it's passenger, there's no such case where this gets to fail legitimacy, this problem
    cannot be solved from within the check in other way since target type cannot be called for the spell currently
    Spell examples: [ID - 52864 Devour Water, ID - 52862 Devour Wind, ID - 49370 Wyrmrest Defender: Destabilize Azure Dragonshrine Effect] */
    if (
#ifdef LICH_KING
        !caster->IsVehicle() &&
#endif
        !(caster->GetCharmerOrOwner() == target))
    {
        if (TargetAuraState && !unitTarget->HasAuraState(AuraStateType(TargetAuraState), this, caster))
            return SPELL_FAILED_TARGET_AURASTATE;

        if (TargetAuraStateNot && unitTarget->HasAuraState(AuraStateType(TargetAuraStateNot), this, caster))
            return SPELL_FAILED_TARGET_AURASTATE;
    }

#ifdef LICH_KING
    if (TargetAuraSpell && !unitTarget->HasAura(sSpellMgr->GetSpellIdForDifficulty(TargetAuraSpell, caster)))
        return SPELL_FAILED_TARGET_AURASTATE;

    if (ExcludeTargetAuraSpell && unitTarget->HasAura(sSpellMgr->GetSpellIdForDifficulty(ExcludeTargetAuraSpell, caster)))
        return SPELL_FAILED_TARGET_AURASTATE;

    if (unitTarget->HasAuraType(SPELL_AURA_PREVENT_RESURRECTION))
        if (HasEffect(SPELL_EFFECT_SELF_RESURRECT) || HasEffect(SPELL_EFFECT_RESURRECT) || HasEffect(SPELL_EFFECT_RESURRECT_NEW))
            return SPELL_FAILED_TARGET_CANNOT_BE_RESURRECTED;
#endif

    // xinef: check if stronger aura is active
    /* Todo spel strongerauras
    if (IsStrongerAuraActive(caster, unitTarget))
        return SPELL_FAILED_AURA_BOUNCED;
        */

    return SPELL_CAST_OK;
}

SpellCastResult SpellInfo::CheckExplicitTarget(Unit const* caster, WorldObject const* target, Item const* itemTarget) const
{
    uint32 neededTargets = GetExplicitTargetMask();
    if (!target)
    {
        if (neededTargets & (TARGET_FLAG_UNIT_MASK | TARGET_FLAG_GAMEOBJECT_MASK | TARGET_FLAG_CORPSE_MASK))
            if (!(neededTargets & TARGET_FLAG_GAMEOBJECT_ITEM) || !itemTarget)
                return SPELL_FAILED_BAD_TARGETS;
        return SPELL_CAST_OK;
    }

    if (Unit const* unitTarget = target->ToUnit())
    {
        if (neededTargets & (TARGET_FLAG_UNIT_ENEMY | TARGET_FLAG_UNIT_ALLY | TARGET_FLAG_UNIT_RAID | TARGET_FLAG_UNIT_PARTY | TARGET_FLAG_UNIT_MINIPET 
#ifdef LICH_KING
            | TARGET_FLAG_UNIT_PASSENGER
#endif
            ))
        {
            if (neededTargets & TARGET_FLAG_UNIT_ENEMY)
                if (caster->_IsValidAttackTarget(unitTarget, this))
                    return SPELL_CAST_OK;
            if (neededTargets & TARGET_FLAG_UNIT_ALLY
                || (neededTargets & TARGET_FLAG_UNIT_PARTY && caster->IsInPartyWith(unitTarget))
                || (neededTargets & TARGET_FLAG_UNIT_RAID && caster->IsInRaidWith(unitTarget)))
                if (caster->_IsValidAssistTarget(unitTarget, this))
                    return SPELL_CAST_OK;
            if (neededTargets & TARGET_FLAG_UNIT_MINIPET)
#ifdef LICH_KING
                if (unitTarget->GetGUID() == caster->GetCritterGUID())
                    return SPELL_CAST_OK;
#else
                if (Player const* p = caster->ToPlayer())
                {
                    Creature* miniPet = p->GetMiniPet();
                    if (miniPet && unitTarget->GetGUID() == miniPet->GetGUID())
                        return SPELL_CAST_OK;
                }
#endif
#ifdef LICH_KING
            if (neededTargets & TARGET_FLAG_UNIT_PASSENGER)
                if (unitTarget->IsOnVehicle(caster))
                    return SPELL_CAST_OK;
#endif
            return SPELL_FAILED_BAD_TARGETS;
        }
    }
    return SPELL_CAST_OK;
}

bool SpellInfo::CheckTargetCreatureType(Unit const* target) const
{
    // Curse of Doom & Exorcism: not find another way to fix spell target check :/
    if (SpellFamilyName == SPELLFAMILY_WARLOCK && GetCategory() == 1179)
    {
        // not allow cast at player
        if (target->GetTypeId() == TYPEID_PLAYER)
            return false;
        else
            return true;
    }
    uint32 creatureType = target->GetCreatureTypeMask();
    return !TargetCreatureType || !creatureType || (creatureType & TargetCreatureType);
}

SpellSchoolMask SpellInfo::GetSchoolMask() const
{
    return SpellSchoolMask(SchoolMask);
}

uint32 SpellInfo::GetAllEffectsMechanicMask() const
{
    uint32 mask = 0;
    if (Mechanic)
        mask |= 1 << Mechanic;
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        if (Effects[i].IsEffect() && Effects[i].Mechanic)
            mask |= 1 << Effects[i].Mechanic;
    return mask;
}

uint32 SpellInfo::GetEffectMechanicMask(uint8 effIndex) const
{
    uint32 mask = 0;
    if (Mechanic)
        mask |= 1 << Mechanic;
    if (Effects[effIndex].IsEffect() && Effects[effIndex].Mechanic)
        mask |= 1 << Effects[effIndex].Mechanic;
    return mask;
}

uint32 SpellInfo::GetSpellMechanicMaskByEffectMask(SpellEffectMask effectMask) const
{
    uint32 mask = 0;
    if (Mechanic)
        mask |= 1 << Mechanic;
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        if ((effectMask & (1 << i)) && Effects[i].Mechanic)
            mask |= 1 << Effects[i].Mechanic;
    return mask;
}

Mechanics SpellInfo::GetEffectMechanic(uint8 effIndex) const
{
    if (Effects[effIndex].IsEffect() && Effects[effIndex].Mechanic)
        return Mechanics(Effects[effIndex].Mechanic);
    if (Mechanic)
        return Mechanics(Mechanic);
    return MECHANIC_NONE;
}

bool SpellInfo::HasAnyEffectMechanic() const
{
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        if (Effects[i].Mechanic)
            return true;
    return false;
}

float SpellInfo::GetMinRange(bool /* positive */) const
{
    if (!RangeEntry)
        return 0.0f;
    /* LK
    if (positive)
        return RangeEntry->minRangeFriend;
    else
        return RangeEntry->minRangeHostile; */
    return RangeEntry->minRange;
}

float SpellInfo::GetMaxRange(bool /* positive */, Unit* caster, Spell* spell) const
{
    if (!RangeEntry)
        return 0.0f;
    float range;
    /* LK
    if (positive)
        range = RangeEntry->maxRangeFriend;
    else
        range = RangeEntry->maxRangeHostile; */
    range = RangeEntry->maxRange;
    if (caster)
        if (Player* modOwner = caster->GetSpellModOwner())
            modOwner->ApplySpellMod(Id, SPELLMOD_RANGE, range, spell);
    return range;
}

bool SpellInfo::IsChannelCategorySpell() const
{
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        if (Effects[i].TargetA.GetSelectionCategory() == TARGET_SELECT_CATEGORY_CHANNEL || Effects[i].TargetB.GetSelectionCategory() == TARGET_SELECT_CATEGORY_CHANNEL)
            return true;
    return false;
}

bool SpellInfo::IsPassive() const
{
    return HasAttribute(SPELL_ATTR0_PASSIVE);
}

bool SpellInfo::IsPositive(bool hostileTarget /* = false */) const
{
    if(HasEffect(SPELL_EFFECT_DISPEL) || HasEffect(SPELL_EFFECT_DISPEL_MECHANIC))
        return !hostileTarget;  // positive on friendly, negative on hostile

    return !HasAttribute(SPELL_ATTR_CU_NEGATIVE);
}

bool SpellInfo::IsPositiveEffect(uint8 effIndex, bool hostileTarget /* = false */) const
{
    if(HasEffect(SPELL_EFFECT_DISPEL, effIndex) || HasEffect(SPELL_EFFECT_DISPEL_MECHANIC, effIndex))
        return !hostileTarget;  // positive on friendly, negative on hostile

    switch (effIndex)
    {
        default:
        case 0:
            return !HasAttribute(SPELL_ATTR_CU_NEGATIVE_EFF0);
        case 1:
            return !HasAttribute(SPELL_ATTR_CU_NEGATIVE_EFF1);
        case 2:
            return !HasAttribute(SPELL_ATTR_CU_NEGATIVE_EFF2);
    }
}


uint32 SpellInfo::GetExplicitTargetMask() const
{
    return ExplicitTargetMask;
}

AuraStateType SpellInfo::GetAuraState() const
{
    if (GetSpellSpecific() == SPELL_SEAL)
        return AURA_STATE_JUDGEMENT;

    // Conflagrate aura state
    if (SpellFamilyName == SPELLFAMILY_WARLOCK && (SpellFamilyFlags & SPELLFAMILYFLAG_WARLOCK_IMMOLATE))
        return AURA_STATE_CONFLAGRATE;

    // Swiftmend state on Regrowth & Rejuvenation
    if (SpellFamilyName == SPELLFAMILY_DRUID
        && (SpellFamilyFlags & (SPELLFAMILYFLAG_DRUID_REJUVENATION | SPELLFAMILYFLAG_DRUID_REGROWTH)))
        return AURA_STATE_SWIFTMEND;

    // Deadly poison
    if (SpellFamilyName == SPELLFAMILY_ROGUE && (SpellFamilyFlags & SPELLFAMILYFLAG_ROGUE_DEADLYPOISON))
        return AURA_STATE_DEADLY_POISON;

    // Faerie Fire (druid versions)
    if ((SpellFamilyName == SPELLFAMILY_DRUID &&
        SpellFamilyFlags & SPELLFAMILYFLAG_DRUID_FAERIEFIRE)
        || Id == 35325) //glowing blood (why ?)
        return AURA_STATE_FAERIE_FIRE;

    // Forbearance and Weakened Soul
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; i++)
    {
        if (Effects[i].ApplyAuraName == SPELL_AURA_MECHANIC_IMMUNITY)
        {
            if (Effects[i].MiscValue == MECHANIC_INVULNERABILITY)
                return AURA_STATE_FORBEARANCE;
            if (Effects[i].MiscValue == MECHANIC_SHIELD)
                return AURA_STATE_WEAKENED_SOUL;
        }
    }

    if (SpellFamilyName == SPELLFAMILY_WARRIOR
        && (SpellFamilyFlags == SPELLFAMILYFLAG_WARRIOR_VICTORYRUSH))
        return AURA_STATE_WARRIOR_VICTORY_RUSH;

    switch (Id)
    {
    case 41425: // Hypothermia
        return AURA_STATE_HYPOTHERMIA;
    case 32216: // Victorious
        return AURA_STATE_WARRIOR_VICTORY_RUSH;
    case 1661: // Berserking (troll racial traits)
        return AURA_STATE_BERSERKING;
    default:
        break;
    }

    return AURA_STATE_NONE;
}

bool SpellInfo::IsStackableWithRanks() const
{
    if (IsPassive())
        return false;
    if (PowerType != POWER_MANA && PowerType != POWER_HEALTH)
        return false;
    if (IsProfessionOrRiding())
        return false;

    if (IsAbilityLearnedWithProfession())
        return false;

    // All stance spells. if any better way, change it.
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        switch (SpellFamilyName)
        {
        case SPELLFAMILY_PALADIN:
            // Paladin aura Spell
#ifdef LICH_KING
            if (Effects[i].Effect == SPELL_EFFECT_APPLY_AREA_AURA_RAID)
#else
            if (Effects[i].Effect == SPELL_EFFECT_APPLY_AREA_AURA_PARTY)
#endif
                return false;
            break;
        case SPELLFAMILY_DRUID:
            // Druid form Spell
            if (Effects[i].Effect == SPELL_EFFECT_APPLY_AURA &&
                Effects[i].ApplyAuraName == SPELL_AURA_MOD_SHAPESHIFT)
                return false;
            break;
        }
    }
    return true;
}

bool SpellInfo::IsProfessionOrRiding() const
{
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (Effects[i].Effect == SPELL_EFFECT_SKILL)
        {
            uint32 skill = Effects[i].MiscValue;

            if (sSpellMgr->IsProfessionOrRidingSkill(skill))
                return true;
        }
    }
    return false;
}

bool SpellInfo::IsAbilityLearnedWithProfession() const
{
    SkillLineAbilityMapBounds bounds = sSpellMgr->GetSkillLineAbilityMapBounds(Id);

    for (SkillLineAbilityMap::const_iterator _spell_idx = bounds.first; _spell_idx != bounds.second; ++_spell_idx)
    {
        SkillLineAbilityEntry const* pAbility = _spell_idx->second;
        if (!pAbility || pAbility->AutolearnType != SKILL_LINE_ABILITY_LEARNED_ON_SKILL_VALUE)
            continue;

        if (pAbility->req_skill_value > 0)
            return true;
    }

    return false;
}

float SpellEffectInfo::CalcValueMultiplier(Unit* caster, Spell* spell) const
{
    float multiplier = ValueMultiplier;
    if (Player* modOwner = (caster ? caster->GetSpellModOwner() : NULL))
        modOwner->ApplySpellMod(_spellInfo->Id, SPELLMOD_VALUE_MULTIPLIER, multiplier, spell);
    return multiplier;
}

float SpellEffectInfo::CalcDamageMultiplier(Unit* caster, Spell* spell) const
{
    float multiplier = DamageMultiplier;
    if (Player* modOwner = (caster ? caster->GetSpellModOwner() : NULL))
        modOwner->ApplySpellMod(_spellInfo->Id, SPELLMOD_DAMAGE_MULTIPLIER, multiplier, spell);
    return multiplier; 
}

bool SpellEffectInfo::HasRadius() const
{
    return RadiusEntry != NULL;
}

float SpellEffectInfo::CalcRadius(Unit* caster, Spell* spell) const
{
    if (!HasRadius())
        return 0.0f;

    float radius = RadiusEntry->RadiusMin;
    if (caster)
    {
        //LK radius += RadiusEntry->RadiusPerLevel * caster->getLevel();
        radius = std::min(radius, RadiusEntry->RadiusMax);
        if (Player* modOwner = caster->GetSpellModOwner())
            modOwner->ApplySpellMod(_spellInfo->Id, SPELLMOD_RADIUS, radius, spell);
    }

    return radius;
}


uint32 SpellEffectInfo::GetProvidedTargetMask() const
{
    return GetTargetFlagMask(TargetA.GetObjectType()) | GetTargetFlagMask(TargetB.GetObjectType());
}

uint32 SpellEffectInfo::GetMissingTargetMask(bool srcSet /*= false*/, bool dstSet /*= false*/, uint32 mask /*=0*/) const
{
    uint32 effImplicitTargetMask = GetTargetFlagMask(GetUsedTargetObjectType());
    uint32 providedTargetMask = GetTargetFlagMask(TargetA.GetObjectType()) | GetTargetFlagMask(TargetB.GetObjectType()) | mask;

    // remove all flags covered by effect target mask
    if (providedTargetMask & TARGET_FLAG_UNIT_MASK)
        effImplicitTargetMask &= ~(TARGET_FLAG_UNIT_MASK);
    if (providedTargetMask & TARGET_FLAG_CORPSE_MASK)
        effImplicitTargetMask &= ~(TARGET_FLAG_UNIT_MASK | TARGET_FLAG_CORPSE_MASK);
    if (providedTargetMask & TARGET_FLAG_GAMEOBJECT_ITEM)
        effImplicitTargetMask &= ~(TARGET_FLAG_GAMEOBJECT_ITEM | TARGET_FLAG_GAMEOBJECT | TARGET_FLAG_ITEM);
    if (providedTargetMask & TARGET_FLAG_GAMEOBJECT)
        effImplicitTargetMask &= ~(TARGET_FLAG_GAMEOBJECT | TARGET_FLAG_GAMEOBJECT_ITEM);
    if (providedTargetMask & TARGET_FLAG_ITEM)
        effImplicitTargetMask &= ~(TARGET_FLAG_ITEM | TARGET_FLAG_GAMEOBJECT_ITEM);
    if (dstSet || providedTargetMask & TARGET_FLAG_DEST_LOCATION)
        effImplicitTargetMask &= ~(TARGET_FLAG_DEST_LOCATION);
    if (srcSet || providedTargetMask & TARGET_FLAG_SOURCE_LOCATION)
        effImplicitTargetMask &= ~(TARGET_FLAG_SOURCE_LOCATION);

    return effImplicitTargetMask;
}

SpellEffectImplicitTargetTypes SpellEffectInfo::GetImplicitTargetType() const
{
    return _data[Effect].ImplicitTargetType;
}

SpellTargetObjectTypes SpellEffectInfo::GetUsedTargetObjectType() const
{
    return _data[Effect].UsedTargetObjectType;
}

SpellEffectInfo::StaticData  SpellEffectInfo::_data[TOTAL_SPELL_EFFECTS] =
{
    // implicit target type           used target object type
    { EFFECT_IMPLICIT_TARGET_NONE,     TARGET_OBJECT_TYPE_NONE }, // 0
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 1 SPELL_EFFECT_INSTAKILL
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 2 SPELL_EFFECT_SCHOOL_DAMAGE
    { EFFECT_IMPLICIT_TARGET_NONE,     TARGET_OBJECT_TYPE_NONE }, // 3 SPELL_EFFECT_DUMMY
    { EFFECT_IMPLICIT_TARGET_NONE,     TARGET_OBJECT_TYPE_NONE }, // 4 SPELL_EFFECT_PORTAL_TELEPORT
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT_AND_DEST }, // 5 SPELL_EFFECT_TELEPORT_UNITS
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 6 SPELL_EFFECT_APPLY_AURA
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 7 SPELL_EFFECT_ENVIRONMENTAL_DAMAGE
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 8 SPELL_EFFECT_POWER_DRAIN
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 9 SPELL_EFFECT_HEALTH_LEECH
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 10 SPELL_EFFECT_HEAL
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 11 SPELL_EFFECT_BIND
    { EFFECT_IMPLICIT_TARGET_NONE,     TARGET_OBJECT_TYPE_NONE }, // 12 SPELL_EFFECT_PORTAL
    { EFFECT_IMPLICIT_TARGET_NONE,     TARGET_OBJECT_TYPE_NONE }, // 13 SPELL_EFFECT_RITUAL_BASE
    { EFFECT_IMPLICIT_TARGET_NONE,     TARGET_OBJECT_TYPE_NONE }, // 14 SPELL_EFFECT_RITUAL_SPECIALIZE
    { EFFECT_IMPLICIT_TARGET_NONE,     TARGET_OBJECT_TYPE_NONE }, // 15 SPELL_EFFECT_RITUAL_ACTIVATE_PORTAL
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 16 SPELL_EFFECT_QUEST_COMPLETE
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 17 SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_CORPSE_ALLY }, // 18 SPELL_EFFECT_RESURRECT
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 19 SPELL_EFFECT_ADD_EXTRA_ATTACKS
    { EFFECT_IMPLICIT_TARGET_NONE,     TARGET_OBJECT_TYPE_UNIT }, // 20 SPELL_EFFECT_DODGE
    { EFFECT_IMPLICIT_TARGET_NONE,     TARGET_OBJECT_TYPE_UNIT }, // 21 SPELL_EFFECT_EVADE
    { EFFECT_IMPLICIT_TARGET_NONE,     TARGET_OBJECT_TYPE_UNIT }, // 22 SPELL_EFFECT_PARRY
    { EFFECT_IMPLICIT_TARGET_NONE,     TARGET_OBJECT_TYPE_UNIT }, // 23 SPELL_EFFECT_BLOCK
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 24 SPELL_EFFECT_CREATE_ITEM
    { EFFECT_IMPLICIT_TARGET_NONE,     TARGET_OBJECT_TYPE_UNIT }, // 25 SPELL_EFFECT_WEAPON
    { EFFECT_IMPLICIT_TARGET_NONE,     TARGET_OBJECT_TYPE_UNIT }, // 26 SPELL_EFFECT_DEFENSE
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_DEST }, // 27 SPELL_EFFECT_PERSISTENT_AREA_AURA
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_DEST }, // 28 SPELL_EFFECT_SUMMON
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT_AND_DEST }, // 29 SPELL_EFFECT_LEAP
    { EFFECT_IMPLICIT_TARGET_NONE,     TARGET_OBJECT_TYPE_UNIT }, // 30 SPELL_EFFECT_ENERGIZE
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 31 SPELL_EFFECT_WEAPON_PERCENT_DAMAGE
    { EFFECT_IMPLICIT_TARGET_NONE,     TARGET_OBJECT_TYPE_NONE }, // 32 SPELL_EFFECT_TRIGGER_MISSILE
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_GOBJ_ITEM }, // 33 SPELL_EFFECT_OPEN_LOCK
    { EFFECT_IMPLICIT_TARGET_NONE,     TARGET_OBJECT_TYPE_UNIT }, // 34 SPELL_EFFECT_SUMMON_CHANGE_ITEM
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 35 SPELL_EFFECT_APPLY_AREA_AURA_PARTY
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 36 SPELL_EFFECT_LEARN_SPELL
    { EFFECT_IMPLICIT_TARGET_NONE,     TARGET_OBJECT_TYPE_UNIT }, // 37 SPELL_EFFECT_SPELL_DEFENSE
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 38 SPELL_EFFECT_DISPEL
    { EFFECT_IMPLICIT_TARGET_NONE,     TARGET_OBJECT_TYPE_UNIT }, // 39 SPELL_EFFECT_LANGUAGE
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 40 SPELL_EFFECT_DUAL_WIELD
#ifdef LICH_KING
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 41 SPELL_EFFECT_JUMP
    { EFFECT_IMPLICIT_TARGET_NONE,     TARGET_OBJECT_TYPE_DEST }, // 42 SPELL_EFFECT_JUMP_DEST
#else
    { EFFECT_IMPLICIT_TARGET_NONE,     TARGET_OBJECT_TYPE_NONE }, // 41 SPELL_EFFECT_SUMMON_WILD
    { EFFECT_IMPLICIT_TARGET_NONE,     TARGET_OBJECT_TYPE_NONE }, // 42 SPELL_EFFECT_SUMMON_GUARDIAN
#endif
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT_AND_DEST }, // 43 SPELL_EFFECT_TELEPORT_UNITS_FACE_CASTER
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 44 SPELL_EFFECT_SKILL_STEP
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 45 SPELL_EFFECT_ADD_HONOR
    { EFFECT_IMPLICIT_TARGET_NONE,     TARGET_OBJECT_TYPE_UNIT }, // 46 SPELL_EFFECT_SPAWN
    { EFFECT_IMPLICIT_TARGET_NONE,     TARGET_OBJECT_TYPE_UNIT }, // 47 SPELL_EFFECT_TRADE_SKILL
    { EFFECT_IMPLICIT_TARGET_NONE,     TARGET_OBJECT_TYPE_UNIT }, // 48 SPELL_EFFECT_STEALTH
    { EFFECT_IMPLICIT_TARGET_NONE,     TARGET_OBJECT_TYPE_UNIT }, // 49 SPELL_EFFECT_DETECT
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_DEST }, // 50 SPELL_EFFECT_TRANS_DOOR
    { EFFECT_IMPLICIT_TARGET_NONE,     TARGET_OBJECT_TYPE_UNIT }, // 51 SPELL_EFFECT_FORCE_CRITICAL_HIT
    { EFFECT_IMPLICIT_TARGET_NONE,     TARGET_OBJECT_TYPE_UNIT }, // 52 SPELL_EFFECT_GUARANTEE_HIT
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_ITEM }, // 53 SPELL_EFFECT_ENCHANT_ITEM
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_ITEM }, // 54 SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 55 SPELL_EFFECT_TAMECREATURE
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_DEST }, // 56 SPELL_EFFECT_SUMMON_PET
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 57 SPELL_EFFECT_LEARN_PET_SPELL
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 58 SPELL_EFFECT_WEAPON_DAMAGE
#ifdef LICH_KING
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 59 SPELL_EFFECT_CREATE_RANDOM_ITEM
#else
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_ITEM }, // 59 SPELL_EFFECT_OPEN_LOCK_ITEM
#endif
    { EFFECT_IMPLICIT_TARGET_NONE,     TARGET_OBJECT_TYPE_UNIT }, // 60 SPELL_EFFECT_PROFICIENCY
    { EFFECT_IMPLICIT_TARGET_NONE,     TARGET_OBJECT_TYPE_NONE }, // 61 SPELL_EFFECT_SEND_EVENT
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 62 SPELL_EFFECT_POWER_BURN
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 63 SPELL_EFFECT_THREAT
    { EFFECT_IMPLICIT_TARGET_NONE,     TARGET_OBJECT_TYPE_NONE }, // 64 SPELL_EFFECT_TRIGGER_SPELL
#ifdef LICH_KING
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 65 SPELL_EFFECT_APPLY_AREA_AURA_RAID
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 66 SPELL_EFFECT_CREATE_MANA_GEM
#else
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 66 SPELL_EFFECT_HEALTH_FUNNEL
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 66 SPELL_EFFECT_POWER_FUNNEL
#endif
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 67 SPELL_EFFECT_HEAL_MAX_HEALTH
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 68 SPELL_EFFECT_INTERRUPT_CAST
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT_AND_DEST }, // 69 SPELL_EFFECT_DISTRACT
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 70 SPELL_EFFECT_PULL
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 71 SPELL_EFFECT_PICKPOCKET
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_DEST }, // 72 SPELL_EFFECT_ADD_FARSIGHT
#ifdef LICH_KING
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 73 SPELL_EFFECT_UNTRAIN_TALENTS
    { EFFECT_IMPLICIT_TARGET_NONE,     TARGET_OBJECT_TYPE_UNIT }, // 74 SPELL_EFFECT_APPLY_GLYPH
#else
    { EFFECT_IMPLICIT_TARGET_EXPLICIT,     TARGET_OBJECT_TYPE_DEST }, // 73 SPELL_EFFECT_SUMMON_POSSESSED
    { EFFECT_IMPLICIT_TARGET_EXPLICIT,     TARGET_OBJECT_TYPE_DEST }, // 74 SPELL_EFFECT_SUMMON_TOTEM
#endif
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 75 SPELL_EFFECT_HEAL_MECHANICAL
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_DEST }, // 76 SPELL_EFFECT_SUMMON_OBJECT_WILD
    { EFFECT_IMPLICIT_TARGET_NONE,     TARGET_OBJECT_TYPE_NONE }, // 77 SPELL_EFFECT_SCRIPT_EFFECT
    { EFFECT_IMPLICIT_TARGET_NONE,     TARGET_OBJECT_TYPE_UNIT }, // 78 SPELL_EFFECT_ATTACK
    { EFFECT_IMPLICIT_TARGET_NONE,     TARGET_OBJECT_TYPE_UNIT }, // 79 SPELL_EFFECT_SANCTUARY
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 80 SPELL_EFFECT_ADD_COMBO_POINTS
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_DEST }, // 81 SPELL_EFFECT_CREATE_HOUSE
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 82 SPELL_EFFECT_BIND_SIGHT
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT_AND_DEST }, // 83 SPELL_EFFECT_DUEL
    { EFFECT_IMPLICIT_TARGET_NONE,     TARGET_OBJECT_TYPE_UNIT }, // 84 SPELL_EFFECT_STUCK
    { EFFECT_IMPLICIT_TARGET_NONE,     TARGET_OBJECT_TYPE_NONE }, // 85 SPELL_EFFECT_SUMMON_PLAYER
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_GOBJ }, // 86 SPELL_EFFECT_ACTIVATE_OBJECT
#ifdef LICH_KING
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_GOBJ }, // 87 SPELL_EFFECT_GAMEOBJECT_DAMAGE
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_GOBJ }, // 88 SPELL_EFFECT_GAMEOBJECT_REPAIR
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_GOBJ }, // 89 SPELL_EFFECT_GAMEOBJECT_SET_DESTRUCTION_STATE
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 90 SPELL_EFFECT_KILL_CREDIT
#else
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_DEST }, // 87 SPELL_EFFECT_SUMMON_TOTEM_SLOT1
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_DEST }, // 88 SPELL_EFFECT_SUMMON_TOTEM_SLOT2
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_DEST }, // 89 SPELL_EFFECT_SUMMON_TOTEM_SLOT3
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_DEST }, // 90 SPELL_EFFECT_SUMMON_TOTEM_SLOT4
#endif
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 91 SPELL_EFFECT_THREAT_ALL
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 92 SPELL_EFFECT_ENCHANT_HELD_ITEM
#ifdef LICH_KING
    { EFFECT_IMPLICIT_TARGET_NONE,     TARGET_OBJECT_TYPE_UNIT }, // 93 SPELL_EFFECT_FORCE_DESELECT
#else
    { EFFECT_IMPLICIT_TARGET_EXPLICIT,  TARGET_OBJECT_TYPE_DEST }, // 93 SPELL_EFFECT_SUMMON_PHANTASM
#endif
    { EFFECT_IMPLICIT_TARGET_NONE,     TARGET_OBJECT_TYPE_UNIT }, // 94 SPELL_EFFECT_SELF_RESURRECT
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 95 SPELL_EFFECT_SKINNING
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 96 SPELL_EFFECT_CHARGE
#ifdef LICH_KING
    { EFFECT_IMPLICIT_TARGET_NONE,     TARGET_OBJECT_TYPE_UNIT }, // 97 SPELL_EFFECT_CAST_BUTTON
#else
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_DEST }, // 97 SPELL_EFFECT_SUMMON_CRITTER
#endif
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 98 SPELL_EFFECT_KNOCK_BACK
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_ITEM }, // 99 SPELL_EFFECT_DISENCHANT
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 100 SPELL_EFFECT_INEBRIATE
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_ITEM }, // 101 SPELL_EFFECT_FEED_PET
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 102 SPELL_EFFECT_DISMISS_PET
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 103 SPELL_EFFECT_REPUTATION
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_DEST }, // 104 SPELL_EFFECT_SUMMON_OBJECT_SLOT1
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_DEST }, // 105 SPELL_EFFECT_SUMMON_OBJECT_SLOT2
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_DEST }, // 106 SPELL_EFFECT_SUMMON_OBJECT_SLOT3
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_DEST }, // 107 SPELL_EFFECT_SUMMON_OBJECT_SLOT4
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 108 SPELL_EFFECT_DISPEL_MECHANIC
#ifdef LICH_KING
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_DEST }, // 109 SPELL_EFFECT_RESURRECT_PET
#else
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_DEST }, // 109 SPELL_EFFECT_SUMMON_DEAD_PET
#endif
    { EFFECT_IMPLICIT_TARGET_NONE,     TARGET_OBJECT_TYPE_UNIT }, // 110 SPELL_EFFECT_DESTROY_ALL_TOTEMS
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 111 SPELL_EFFECT_DURABILITY_DAMAGE
#ifdef LICH_KING
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 112 SPELL_EFFECT_112
#else
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_DEST }, // 112 SPELL_EFFECT_SUMMON_DEMON

#endif
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_CORPSE_ALLY }, // 113 SPELL_EFFECT_RESURRECT_NEW
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 114 SPELL_EFFECT_ATTACK_ME
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 115 SPELL_EFFECT_DURABILITY_DAMAGE_PCT
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_CORPSE_ENEMY }, // 116 SPELL_EFFECT_SKIN_PLAYER_CORPSE
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 117 SPELL_EFFECT_SPIRIT_HEAL
    { EFFECT_IMPLICIT_TARGET_NONE,     TARGET_OBJECT_TYPE_UNIT }, // 118 SPELL_EFFECT_SKILL
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 119 SPELL_EFFECT_APPLY_AREA_AURA_PET
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 120 SPELL_EFFECT_TELEPORT_GRAVEYARD
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 121 SPELL_EFFECT_NORMALIZED_WEAPON_DMG
    { EFFECT_IMPLICIT_TARGET_NONE,     TARGET_OBJECT_TYPE_NONE }, // 122 SPELL_EFFECT_122
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 123 SPELL_EFFECT_SEND_TAXI
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 124 SPELL_EFFECT_PULL_TOWARDS
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 125 SPELL_EFFECT_MODIFY_THREAT_PERCENT
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 126 SPELL_EFFECT_STEAL_BENEFICIAL_BUFF
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_ITEM }, // 127 SPELL_EFFECT_PROSPECTING
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 128 SPELL_EFFECT_APPLY_AREA_AURA_FRIEND
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 129 SPELL_EFFECT_APPLY_AREA_AURA_ENEMY
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 130 SPELL_EFFECT_REDIRECT_THREAT
    { EFFECT_IMPLICIT_TARGET_NONE,     TARGET_OBJECT_TYPE_UNIT }, // 131 SPELL_EFFECT_PLAY_SOUND // unused SPELL_EFFECT_131 on BC
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 132 SPELL_EFFECT_PLAY_MUSIC
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 133 SPELL_EFFECT_UNLEARN_SPECIALIZATION
    { EFFECT_IMPLICIT_TARGET_NONE,     TARGET_OBJECT_TYPE_UNIT }, // 134 SPELL_EFFECT_KILL_CREDIT2
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_DEST }, // 135 SPELL_EFFECT_CALL_PET
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 136 SPELL_EFFECT_HEAL_PCT
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 137 SPELL_EFFECT_ENERGIZE_PCT
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 138 SPELL_EFFECT_LEAP_BACK //unused on BC
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 139 SPELL_EFFECT_CLEAR_QUEST //unused on BC
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 140 SPELL_EFFECT_FORCE_CAST
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 141 SPELL_EFFECT_FORCE_CAST_WITH_VALUE
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 142 SPELL_EFFECT_TRIGGER_SPELL_WITH_VALUE
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 143 SPELL_EFFECT_APPLY_AREA_AURA_OWNER
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT_AND_DEST }, // 144 SPELL_EFFECT_KNOCK_BACK_DEST
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT_AND_DEST }, // 145 SPELL_EFFECT_PULL_TOWARDS_DEST //unused on BC
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 146 SPELL_EFFECT_ACTIVATE_RUNE //unused on BC
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 147 SPELL_EFFECT_QUEST_FAIL
    { EFFECT_IMPLICIT_TARGET_NONE,     TARGET_OBJECT_TYPE_NONE }, // 148 SPELL_EFFECT_TRIGGER_MISSILE_SPELL_WITH_VALUE //unused on BC
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_DEST }, // 149 SPELL_EFFECT_CHARGE_DEST //unused on BC
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 150 SPELL_EFFECT_QUEST_START // unused on BC
    { EFFECT_IMPLICIT_TARGET_NONE,     TARGET_OBJECT_TYPE_NONE }, // 151 SPELL_EFFECT_TRIGGER_SPELL_2
    { EFFECT_IMPLICIT_TARGET_NONE,     TARGET_OBJECT_TYPE_NONE }, // 152 SPELL_EFFECT_SUMMON_RAF_FRIEND // unused on BC
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 153 SPELL_EFFECT_CREATE_TAMED_PET
#ifdef LICH_KING
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 154 SPELL_EFFECT_DISCOVER_TAXI
    { EFFECT_IMPLICIT_TARGET_NONE,     TARGET_OBJECT_TYPE_UNIT }, // 155 SPELL_EFFECT_TITAN_GRIP
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_ITEM }, // 156 SPELL_EFFECT_ENCHANT_ITEM_PRISMATIC
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 157 SPELL_EFFECT_CREATE_ITEM_2
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_ITEM }, // 158 SPELL_EFFECT_MILLING
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 159 SPELL_EFFECT_ALLOW_RENAME_PET
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 160 SPELL_EFFECT_160
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 161 SPELL_EFFECT_TALENT_SPEC_COUNT
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 162 SPELL_EFFECT_TALENT_SPEC_SELECT
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 163 SPELL_EFFECT_163
    { EFFECT_IMPLICIT_TARGET_EXPLICIT, TARGET_OBJECT_TYPE_UNIT }, // 164 SPELL_EFFECT_REMOVE_AURA
#endif
};

void SpellInfo::LoadCustomAttributes()
{
    for(uint32 j = 0; j < 3; ++j)
    {
        switch(Effects[j].ApplyAuraName)
        {
            case SPELL_AURA_PERIODIC_DAMAGE:
            case SPELL_AURA_PERIODIC_DAMAGE_PERCENT:
            case SPELL_AURA_PERIODIC_LEECH:
                AttributesCu |= SPELL_ATTR_CU_AURA_DOT;
                break;
            case SPELL_AURA_PERIODIC_HEAL:
            case SPELL_AURA_OBS_MOD_HEALTH:
                AttributesCu |= SPELL_ATTR_CU_AURA_HOT;
                break;
            case SPELL_AURA_MOD_ROOT:
            //    AttributesCu |= SPELL_ATTR_CU_AURA_CC;
                AttributesCu |= SPELL_ATTR_CU_MOVEMENT_IMPAIR;
                break;
            case SPELL_AURA_MOD_DECREASE_SPEED:
                AttributesCu |= SPELL_ATTR_CU_MOVEMENT_IMPAIR;
                break;
            case SPELL_AURA_MOD_POSSESS:
            case SPELL_AURA_MOD_CONFUSE:
            case SPELL_AURA_MOD_CHARM:
            case SPELL_AURA_MOD_FEAR:
            case SPELL_AURA_MOD_STUN:
                AttributesCu |= SPELL_ATTR_CU_AURA_CC;
                AttributesCu &= ~SPELL_ATTR_CU_MOVEMENT_IMPAIR;
                break;
            case SPELL_EFFECT_PICKPOCKET:
                AttributesCu |= SPELL_ATTR_CU_PICKPOCKET;
                break;
            default:
                break;
        }

        switch(Effects[j].Effect)
        {
            case SPELL_EFFECT_SCHOOL_DAMAGE:
            case SPELL_EFFECT_WEAPON_DAMAGE:
            case SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL:
            case SPELL_EFFECT_NORMALIZED_WEAPON_DMG:
            case SPELL_EFFECT_WEAPON_PERCENT_DAMAGE:
            case SPELL_EFFECT_HEAL:
                AttributesCu |= SPELL_ATTR_CU_DIRECT_DAMAGE;
                break;
            case SPELL_EFFECT_CHARGE:
                if(!Speed && !SpellFamilyName)
                    Speed = SPEED_CHARGE;
                AttributesCu |= SPELL_ATTR_CU_CHARGE;
                break;
        }
    }

    if (!_IsPositiveEffect(EFFECT_0, true))
        AttributesCu |= SPELL_ATTR_CU_NEGATIVE_EFF0;

    if (!_IsPositiveEffect(EFFECT_1, true))
        AttributesCu |= SPELL_ATTR_CU_NEGATIVE_EFF1;

    if (!_IsPositiveEffect(EFFECT_2, true))
        AttributesCu |= SPELL_ATTR_CU_NEGATIVE_EFF2;

    if(SpellVisual == 3879)
        AttributesCu |= SPELL_ATTR_CU_CONE_BACK;

    switch (Id)
    {
         case 27003:
            AttributesCu |= SPELL_ATTR_CU_IGNORE_ARMOR;
            break;
        case 31041: // Roar's Mangle
            AttributesCu |= SPELL_ATTR_CU_IGNORE_ARMOR;
            break;
        case 33745: // Rank 1 Lacerate (Druid)
            AttributesCu |= SPELL_ATTR_CU_IGNORE_ARMOR;
            break;
        case 379: //earth shield heal effect
        case 33076: //Prayer of mending
            AttributesCu |= SPELL_ATTR_CU_THREAT_GOES_TO_CURRENT_CASTER;
            break;
        case 26029: // dark glare
        case 37433: // spout
        case 43140: case 43215: // flame breath
            AttributesCu |= SPELL_ATTR_CU_CONE_LINE;
            break;
        case 24340: case 26558: case 28884:     // Meteor
        case 36837: case 38903: case 41276:     // Meteor
        case 26789:                             // Shard of the Fallen Star
        case 31436:                             // Malevolent Cleave
        case 35181:                             // Dive Bomb
        case 40810: case 43267: case 43268:     // Saber Lash
        case 42384:                             // Brutal Swipe
            AttributesCu |= SPELL_ATTR_CU_SHARE_DAMAGE;
            break;
        case 45150:                             // Meteor Slash
            AttributesCu |= SPELL_ATTR_CU_SHARE_DAMAGE;
            break;
        case 12723: // Sweeping Strikes proc
            AttributesCu |= SPELL_ATTR_CU_IGNORE_ARMOR;
            break;
        case 40327: // Atrophy
            AttributesCu |= SPELL_ATTR_CU_SAME_STACK_DIFF_CASTERS;
            break;
        case 45236:
            AttributesCu |= SPELL_ATTR_CU_IGNORE_ARMOR;
            break;
        case 45232:
            AttributesCu |= SPELL_ATTR_CU_IGNORE_ARMOR;
            break;
        case 45256:
            AttributesCu |= SPELL_ATTR_CU_IGNORE_ARMOR;
            break;
        case 45329: // Sacrolash Show nova
            AttributesCu |= SPELL_ATTR_CU_IGNORE_ARMOR;
            break;
        case 45347: // Sacrolash SPELL_DARK_TOUCHED
            AttributesCu |= SPELL_ATTR_CU_SAME_STACK_DIFF_CASTERS;
            AttributesCu |= SPELL_ATTR_CU_IGNORE_ARMOR;
            break;
        case 46771: // SPELL_FLAME_SEAR
            AttributesCu |= SPELL_ATTR_CU_IGNORE_ARMOR;
            break;
        case 15258:
        case 22959:
        case 12579:
        case 13218:
        case 13222:
        case 13223:
        case 13224:
        case 27189:
        case 33878:
        case 33986:
        case 33987:
        case 33876:
        case 33982:
        case 33983:
        case 43299:
        case 43114:
        case 7386:
        case 7405:
        case 8380:
        case 11596:
        case 11597:
        case 25225:
        case 40520: //shadow of akama slow aura from canalisations (spell 40401)
            AttributesCu |= SPELL_ATTR_CU_SAME_STACK_DIFF_CASTERS;
            break;
        case 1120:
        case 8288:
        case 8289:
        case 11675:
        case 27217:
        case 41083: //Illidan's shadow demons Paralyze
        case 39123: //purple beam
            AttributesCu |= SPELL_ATTR_CU_ONE_STACK_PER_CASTER_SPECIAL;
            break;
        case 44335:
            AttributesCu |= SPELL_ATTR_CU_SAME_STACK_DIFF_CASTERS;
            break;
        case 45271:
            AttributesCu |= SPELL_ATTR_CU_IGNORE_ARMOR;
            AttributesCu |= SPELL_ATTR_CU_SAME_STACK_DIFF_CASTERS;
            break;
         case 40845: //Fury
            AttributesCu |= SPELL_ATTR_CU_SAME_STACK_DIFF_CASTERS;
            break;
        case 41173:
        case 41093:
        case 41084:
        case 34580: //Impale
            AttributesCu |= SPELL_ATTR_CU_IGNORE_ARMOR;
            break;
        case 33619: // Reflective Shield
            AttributesCu |= SPELL_ATTR_CU_CANT_BREAK_CC;
            break;
        case 45248:
            AttributesCu |= SPELL_ATTR_CU_IGNORE_ARMOR;
            break;
        case 39968: //najentus spine explosion
            AttributesCu |= SPELL_ATTR_CU_AOE_CANT_TARGET_SELF;
            break;
        case 23735:
        case 23736:
        case 23737:
        case 23738:
        case 23766:
        case 23767:
        case 23768:
        case 23769:
            AttributesCu |= SPELL_ATTR_CU_REMOVE_ON_INSTANCE_ENTER;
            break;
        case 26102: // Sandblast (Ouro)
        case 19272: // Lava Breath (Molten Core - Ancient Core Hound)
        case 21333: // Lava Breath, from ?
        case 36654: // Fel Breath (The Arcatraz - Soul Devourer)
        case 38813: // Fel Breath (The Arcatraz - Soul Devourer) heroic
        case 38814: // Lava Breath, from ?
            AttributesCu |= SPELL_ATTR_CU_CONE_180;
            break;
        case 45770:
        case 19516:
            AttributesCu |= SPELL_ATTR_CU_SAME_STACK_DIFF_CASTERS;
            break;
    }
}

bool SpellInfo::IsSingleTarget() const
{
    // all other single target spells have if it has AttributesEx5
    if (HasAttribute(SPELL_ATTR5_SINGLE_TARGET_SPELL))
        return true;

    // TODO - need find Judgements rule
    switch (GetSpellSpecific())
    {
    case SPELL_JUDGEMENT:
        return true;
    default:
        break;
    }

    // single target triggered spell.
    // Not real client side single target spell, but it' not triggered until prev. aura expired.
    // This is allow store it in single target spells list for caster for spell proc checking
    if (Id == 38324)                                // Regeneration (triggered by 38299 (HoTs on Heals))
        return true;

    return false;
}


uint32 SpellInfo::_GetExplicitTargetMask() const
{
    bool srcSet = false;
    bool dstSet = false;
    uint32 targetMask = Targets;
    // prepare target mask using effect target entries
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (!Effects[i].IsEffect())
            continue;
        targetMask |= Effects[i].TargetA.GetExplicitTargetMask(srcSet, dstSet);
        targetMask |= Effects[i].TargetB.GetExplicitTargetMask(srcSet, dstSet);

        // add explicit target flags based on spell effects which have EFFECT_IMPLICIT_TARGET_EXPLICIT and no valid target provided
        if (Effects[i].GetImplicitTargetType() != EFFECT_IMPLICIT_TARGET_EXPLICIT)
            continue;

        // extend explicit target mask only if valid targets for effect could not be provided by target types
        uint32 effectTargetMask = Effects[i].GetMissingTargetMask(srcSet, dstSet, targetMask);

        // don't add explicit object/dest flags when spell has no max range
        if (GetMaxRange(true) == 0.0f && GetMaxRange(false) == 0.0f)
            effectTargetMask &= ~(TARGET_FLAG_UNIT_MASK | TARGET_FLAG_GAMEOBJECT | TARGET_FLAG_CORPSE_MASK | TARGET_FLAG_DEST_LOCATION);
        targetMask |= effectTargetMask;
    }
    return targetMask;
}

bool SpellInfo::_IsPositiveEffect(uint32 effIndex, bool deep) const
{
    // talents
    if (IsPassive() && GetTalentSpellCost(Id))
        return true;

    switch(Id)
    {
        case 23333:                                         // BG spell
        case 23335:                                         // BG spell
        case 34976:                                         // BG spell
        case 31579:                                         // Arcane Empowerment Rank1 talent aura with one positive and one negative (check not needed in wotlk)
        case 31582:                                         // Arcane Empowerment Rank2
        case 31583:                                         // Arcane Empowerment Rank3
        case 38307:                                         // The Dark of Night
        case 40477:                                         // Forceful Strike
        case 38318:                                         // Transformation - Black Whelp
        case 24732:                                         // Bat Costume
        case 24740:                                         // Wisp Costume
        case 43730:                                         // Electrified
        case 37472:
        case 45989:
        case 20553:
        case 45856:                                         // Breath: Haste
        case 45860:                                         // Breath: Revitalize
        case 45848:                                         // Shield of the Blue
        case 45839:                                         // Vengeance of the Blue Flight
        case 23505:                                         // Berserking (BG buff)
        case 1462:                                          // Beast Lore
        case 30877:                                         // Tag Murloc
            return true;
        case 1852:                                          // Silenced (GM)
        case 46392:                                         // Focused Assault
        case 46393:                                         // Brutal Assault
        case 43437:                                         // Paralyzed
        case 28441:                                         // not positive dummy spell
        case 37675:                                         // Chaos Blast
        case 41519:                                         // Mark of Stormrage
        case 34877:                                         // Custodian of Time
        case 34700:                                         // Allergic Reaction
        case 31719:                                         // Suspension
        case 43501:                                         // Siphon Soul (Malacrass)
        case 30500:                                         // Death Coil (Nethekurse - Shattered Halls)
        case 38065:                                         // Death Coil (Nexus Terror - Mana Tombs)
        case 45661:                                         // Encapsulate (Felmyst - Sunwell Plateau)
        case 45662:
        case 45665:
        case 47002:
        case 41070:                                         // Death Coil (Shadowmoon Deathshaper - Black Temple)
        case 40165:
        case 40055:
        case 40166:
        case 40167:                                         // Introspection
        case 46458:
        case 16097:                                         // Hex
        case 7103:
        case 6945:                                          // Chest Pains
        case 23182:
        case 40695:                                         // Illidan - Caged
        case 37108:                                         // Quest 10557
        case 37966:
        case 30529:
        case 37463:
        case 37461:
        case 37462:
        case 37465:
        case 37453:
        case 37498:
        case 37427:
        case 37406:
        case 27861:
        case 29214: // Wrath of the Plaguebringer
        case 30421: //netherspite spell
        case 30422: //netherspite spell
        case 30423: //netherspite spell
            return false;
    }

    switch(Effects[effIndex].Effect)
    {
        // Those are positive on friendly, negative on hostile, handled is IsPositive instead since we cannot give a static value here
        /* case SPELL_EFFECT_DISPEL:
        case SPELL_EFFECT_DISPEL_MECHANIC:
            return true; */

        // always positive effects (check before target checks that provided non-positive result in some case for positive effects)
        case SPELL_EFFECT_HEAL:
        case SPELL_EFFECT_LEARN_SPELL:
        case SPELL_EFFECT_SKILL_STEP:
        case SPELL_EFFECT_HEAL_PCT:
        case SPELL_EFFECT_ENERGIZE_PCT:
        case SPELL_EFFECT_PLAY_SOUND:
        case SPELL_EFFECT_PLAY_MUSIC:
            return true;

        case SPELL_EFFECT_APPLY_AREA_AURA_ENEMY:
            return false;
            // non-positive aura use
        case SPELL_EFFECT_APPLY_AURA:
        case SPELL_EFFECT_APPLY_AREA_AURA_FRIEND:
        {
            switch(Effects[effIndex].ApplyAuraName)
            {
                case SPELL_AURA_DUMMY:
                {
                    // dummy aura can be positive or negative dependent from casted spell
                    switch(Id)
                    {
                        case 13139:                         // net-o-matic special effect
                        case 23445:                         // evil twin
                        case 38637:                         // Nether Exhaustion (red)
                        case 38638:                         // Nether Exhaustion (green)
                        case 38639:                         // Nether Exhaustion (blue)
                            return false;
                        default:
                            break;
                    }
                }   break;
                case SPELL_AURA_MOD_DAMAGE_DONE:            // dependent from bas point sign (negative -> negative)
                case SPELL_AURA_MOD_STAT:
                case SPELL_AURA_MOD_SKILL:
                case SPELL_AURA_MOD_DODGE_PERCENT:
                case SPELL_AURA_MOD_HEALING_DONE:
                case SPELL_AURA_MOD_HEALING_PCT:
                case SPELL_AURA_MOD_DAMAGE_PERCENT_DONE:
                {
                    if(Effects[effIndex].BasePoints+int32(Effects[effIndex].BaseDice) < 0)
                        return false;
                    break;
                }
                case SPELL_AURA_MOD_DAMAGE_TAKEN:           // dependent from bas point sign (positive -> negative)
                    if(Effects[effIndex].BasePoints+int32(Effects[effIndex].BaseDice) > 0)
                        return false;
                    break;
                case SPELL_AURA_MOD_SPELL_CRIT_CHANCE:
                    if(Effects[effIndex].BasePoints+int32(Effects[effIndex].BaseDice) > 0)
                        return true;                        // some expected positive spells have SPELL_ATTR1_NEGATIVE
                    break;
                case SPELL_AURA_ADD_TARGET_TRIGGER:
                    return true;
                case SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE:
                case SPELL_AURA_PERIODIC_TRIGGER_SPELL:
                    if (deep)
                    {
                        if(Id != Effects[effIndex].TriggerSpell)
                        {
                            uint32 spellTriggeredId = Effects[effIndex].TriggerSpell;
                            SpellInfo const *spellTriggeredProto = sSpellMgr->GetSpellInfo(spellTriggeredId);

                            if(spellTriggeredProto)
                            {
                                // non-positive targets of main spell return early
                                for(int i = 0; i < MAX_SPELL_EFFECTS; ++i)
                                {
                                    // if non-positive trigger cast targeted to positive target this main cast is non-positive
                                    // this will place this spell auras as debuffs
                                    if(_IsPositiveTarget(spellTriggeredProto->Effects[effIndex].TargetA.GetTarget(),spellTriggeredProto->Effects[effIndex].TargetB.GetTarget()) && !spellTriggeredProto->IsPositiveEffect(i))
                                        return false;
                                }
                            }
                        }
                    }
                    break;
                case SPELL_AURA_PROC_TRIGGER_SPELL:
                    // many positive auras have negative triggered spells at damage for example and this not make it negative (it can be canceled for example)
                    break;
                case SPELL_AURA_MOD_STUN:                   //have positive and negative spells, we can't sort its correctly at this moment.
                    if(effIndex==0 && Effects[1].Effect==0 && Effects[2].Effect==0)
                        return false;                       // but all single stun aura spells is negative

                    // Petrification
                    if(Id == 17624)
                        return false;
                    break;
                case SPELL_AURA_MOD_PACIFY_SILENCE:
                    if (Id == 24740)             // Wisp Costume
                        return true;
                    return false;
                case SPELL_AURA_MOD_ROOT:
                case SPELL_AURA_MOD_SILENCE:
                case SPELL_AURA_GHOST:
                case SPELL_AURA_PERIODIC_LEECH:
                case SPELL_AURA_MOD_STALKED:
                case SPELL_AURA_PERIODIC_DAMAGE_PERCENT:
                    return false;
                case SPELL_AURA_PERIODIC_DAMAGE:            // used in positive spells also.
                    // part of negative spell if casted at self (prevent cancel)
                    if(Effects[effIndex].TargetA.GetTarget() == TARGET_UNIT_CASTER)
                        return false;
                    break;
                case SPELL_AURA_MOD_DECREASE_SPEED:         // used in positive spells also
                    // part of positive spell if casted at self
                    if(Effects[effIndex].TargetA.GetTarget() != TARGET_UNIT_CASTER)
                        return false;
                    // but not this if this first effect (don't found batter check)
                    if(Attributes & SPELL_ATTR0_NEGATIVE_1 && effIndex==0)
                        return false;
                    break;
                case SPELL_AURA_TRANSFORM:
                    // some spells negative
                    switch(Id)
                    {
                        case 36897:                         // Transporter Malfunction (race mutation to horde)
                        case 36899:                         // Transporter Malfunction (race mutation to alliance)
                            return false;
                    }
                    break;
                case SPELL_AURA_MOD_SCALE:
                    // some spells negative
                    switch(Id)
                    {
                        case 36900:                         // Soul Split: Evil!
                        case 36901:                         // Soul Split: Good
                        case 36893:                         // Transporter Malfunction (decrease size case)
                        case 36895:                         // Transporter Malfunction (increase size case)
                            return false;
                    }
                    break;
                case SPELL_AURA_MECHANIC_IMMUNITY:
                {
                    // non-positive immunities
                    switch(Effects[effIndex].MiscValue)
                    {
                        case MECHANIC_BANDAGE:
                        case MECHANIC_SHIELD:
                        case MECHANIC_MOUNT:
                        case MECHANIC_INVULNERABILITY:
                            return false;
                        default:
                            break;
                    }
                }   break;
                case SPELL_AURA_ADD_FLAT_MODIFIER:          // mods
                case SPELL_AURA_ADD_PCT_MODIFIER:
                {
                    // non-positive mods
                    switch(Effects[effIndex].MiscValue)
                    {
                        case SPELLMOD_COST:                 // dependent from bas point sign (negative -> positive)
                            if(Effects[effIndex].BasePoints+int32(Effects[effIndex].BaseDice) > 0)
                            {
                                if (!deep)
                                {
                                    bool negative = true;
                                    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
                                    {
                                        if (i != effIndex)
                                            if (_IsPositiveEffect(i, true))
                                            {
                                                negative = false;
                                                break;
                                            }
                                    }
                                    if (negative)
                                        return false;
                                }
                            }
                            break;
                        default:
                            break;
                    }
                }   break;
                case SPELL_AURA_FORCE_REACTION:
                    if(Id==42792)               // Recently Dropped Flag (prevent cancel)
                        return false;
                    break;
                case SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN:
                    if (Effects[effIndex].BasePoints+int32(Effects[effIndex].BaseDice) > 0)
                        return false;
                    break;
                default:
                    break;
            }
            break;
        }
        
        default:
            break;
    }

    // non-positive targets
    if(!_IsPositiveTarget(Effects[effIndex].TargetA.GetTarget(),Effects[effIndex].TargetB.GetTarget()))
        return false;

    if(HasAttribute(SPELL_ATTR1_CANT_BE_REFLECTED) //all those should be negative
       || HasAttribute(SPELL_ATTR0_NEGATIVE_1))
        return false;

    // negative spell if triggered spell is negative
    if (!deep && !Effects[effIndex].ApplyAuraName && Effects[effIndex].TriggerSpell)
    {
        if (SpellInfo const* spellTriggeredProto = sSpellMgr->GetSpellInfo(Effects[effIndex].TriggerSpell))
            if (!spellTriggeredProto->_IsPositiveSpell())
                return false;
    }

    // ok, positive
    return true;
}

bool SpellInfo::_IsPositiveTarget(uint32 targetA, uint32 targetB)
{
    // non-positive targets
    switch (targetA)
    {
        case TARGET_UNIT_NEARBY_ENEMY:
        case TARGET_UNIT_TARGET_ENEMY:
        case TARGET_UNIT_SRC_AREA_ENEMY:
        case TARGET_UNIT_DEST_AREA_ENEMY:
        case TARGET_UNIT_CONE_ENEMY:
//LK    case TARGET_UNIT_CONE_ENEMY_104:
        case TARGET_DEST_DYNOBJ_ENEMY:
        case TARGET_DEST_TARGET_ENEMY:
            return false;
        default:
            break;
    }
    if (targetB)
        return _IsPositiveTarget(targetB, 0);
    return true;
}

bool SpellInfo::_IsPositiveSpell() const
{
    // spells with at least one negative effect are considered negative
    // some self-applied spells have negative effects but in self casting case negative check ignored.
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        if (!_IsPositiveEffect(i, true))
            return false;
    return true;
}

void SpellInfo::_UnloadImplicitTargetConditionLists()
{
    // find the same instances of ConditionList and delete them.
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        ConditionList* cur = Effects[i].ImplicitTargetConditions;
        if (!cur)
            continue;
        for (uint8 j = i; j < MAX_SPELL_EFFECTS; ++j)
        {
            if (Effects[j].ImplicitTargetConditions == cur)
                Effects[j].ImplicitTargetConditions = NULL;
        }
        delete cur;
    }
}


