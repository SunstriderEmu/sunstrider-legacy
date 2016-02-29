/*
 * Copyright (C) 2005-2008 MaNGOS <http://www.mangosproject.org/>
 *
 * Copyright (C) 2008 Trinity <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "SpellMgr.h"
#include "ObjectMgr.h"
#include "SpellAuraDefines.h"
#include "DBCStores.h"
#include "World.h"
#include "Chat.h"
#include "Spell.h"
#include "BattleGroundMgr.h"
#include "SpellInfo.h"

bool IsAreaEffectTarget[TOTAL_SPELL_TARGETS];

bool SpellMgr::IsPrimaryProfessionSkill(uint32 skill)
{
    SkillLineEntry const *pSkill = sSkillLineStore.LookupEntry(skill);
    if (!pSkill)
        return false;

    if (pSkill->categoryId != SKILL_CATEGORY_PROFESSION)
        return false;

    return true;
};

bool SpellMgr::IsProfessionSkill(uint32 skill)
{
    return IsPrimaryProfessionSkill(skill) || skill == SKILL_FISHING || skill == SKILL_COOKING || skill == SKILL_FIRST_AID;
};

bool SpellMgr::IsProfessionOrRidingSkill(uint32 skill)
{
    return IsProfessionSkill(skill) || skill == SKILL_RIDING;
};

SpellMgr::SpellMgr()
{
    for(int i = 0; i < TOTAL_SPELL_EFFECTS; ++i)
    {
        switch(i)
        {
            case SPELL_EFFECT_PERSISTENT_AREA_AURA: //27
            case SPELL_EFFECT_SUMMON:               //28
            case SPELL_EFFECT_TRIGGER_MISSILE:      //32
            case SPELL_EFFECT_SUMMON_WILD:          //41
            case SPELL_EFFECT_SUMMON_GUARDIAN:      //42
            case SPELL_EFFECT_TRANS_DOOR:           //50 summon object
            case SPELL_EFFECT_SUMMON_PET:           //56
            case SPELL_EFFECT_ADD_FARSIGHT:         //72
            case SPELL_EFFECT_SUMMON_POSSESSED:     //73
            case SPELL_EFFECT_SUMMON_TOTEM:         //74
            case SPELL_EFFECT_SUMMON_OBJECT_WILD:   //76
            case SPELL_EFFECT_SUMMON_TOTEM_SLOT1:   //87
            case SPELL_EFFECT_SUMMON_TOTEM_SLOT2:   //88
            case SPELL_EFFECT_SUMMON_TOTEM_SLOT3:   //89
            case SPELL_EFFECT_SUMMON_TOTEM_SLOT4:   //90
            case SPELL_EFFECT_SUMMON_CRITTER:       //97
            case SPELL_EFFECT_SUMMON_OBJECT_SLOT1:  //104
            case SPELL_EFFECT_SUMMON_OBJECT_SLOT2:  //105
            case SPELL_EFFECT_SUMMON_OBJECT_SLOT3:  //106
            case SPELL_EFFECT_SUMMON_OBJECT_SLOT4:  //107
            case SPELL_EFFECT_SUMMON_DEAD_PET:      //109
            case SPELL_EFFECT_SUMMON_DEMON:         //112
            case SPELL_EFFECT_TRIGGER_SPELL_2:      //151 ritual of summon
                EffectTargetType[i] = SPELL_REQUIRE_DEST;
                break;
            case SPELL_EFFECT_PARRY: // 0
            case SPELL_EFFECT_BLOCK: // 0
            case SPELL_EFFECT_SKILL: // always with dummy 3 as A
            //case SPELL_EFFECT_LEARN_SPELL: // 0 may be 5 pet
            case SPELL_EFFECT_TRADE_SKILL: // 0 or 1
            case SPELL_EFFECT_PROFICIENCY: // 0
                EffectTargetType[i] = SPELL_REQUIRE_NONE;
                break;
            case SPELL_EFFECT_ENCHANT_ITEM:
            case SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY:
            case SPELL_EFFECT_DISENCHANT:
            case SPELL_EFFECT_FEED_PET:
            case SPELL_EFFECT_PROSPECTING:
                EffectTargetType[i] = SPELL_REQUIRE_ITEM;
                break;
            //caster must be pushed otherwise no sound
            case SPELL_EFFECT_APPLY_AREA_AURA_PARTY:
            case SPELL_EFFECT_APPLY_AREA_AURA_FRIEND:
            case SPELL_EFFECT_APPLY_AREA_AURA_ENEMY:
            case SPELL_EFFECT_APPLY_AREA_AURA_PET:
            case SPELL_EFFECT_APPLY_AREA_AURA_OWNER:
                EffectTargetType[i] = SPELL_REQUIRE_CASTER;
                break;
            default:
                EffectTargetType[i] = SPELL_REQUIRE_UNIT;
                break;
        }
    }

    for(int i = 0; i < TOTAL_SPELL_TARGETS; ++i)
    {
        switch(i)
        {
            case TARGET_UNIT_CASTER:
            case TARGET_UNIT_CASTER_FISHING:
            case TARGET_UNIT_MASTER:
            case TARGET_UNIT_PET:
            case TARGET_UNIT_CASTER_AREA_PARTY:
            case TARGET_UNIT_CASTER_AREA_RAID:
                SpellTargetType[i] = TARGET_TYPE_UNIT_CASTER;
                break;
            case TARGET_UNIT_TARGET_MINIPET:
            case TARGET_UNIT_TARGET_ALLY:
            case TARGET_UNIT_TARGET_RAID:
            case TARGET_UNIT_TARGET_ANY:
            case TARGET_UNIT_TARGET_ENEMY:
            case TARGET_UNIT_TARGET_PARTY:
            case TARGET_UNIT_LASTTARGET_AREA_PARTY:
            case TARGET_UNIT_TARGET_AREA_RAID_CLASS:
            case TARGET_UNIT_TARGET_CHAINHEAL_ALLY:
                SpellTargetType[i] = TARGET_TYPE_UNIT_TARGET;
                break;
            case TARGET_UNIT_NEARBY_ENEMY:
            case TARGET_UNIT_NEARBY_ALLY:
            case TARGET_UNIT_NEARBY_ALLY_UNK:
            case TARGET_UNIT_NEARBY_ENTRY:
            case TARGET_UNIT_NEARBY_RAID:
                SpellTargetType[i] = TARGET_TYPE_UNIT_NEARBY;
                break;
            case TARGET_UNIT_SRC_AREA_ENEMY:
            case TARGET_UNIT_SRC_AREA_ALLY:
            case TARGET_UNIT_SRC_AREA_ENTRY:
            case TARGET_UNIT_SRC_AREA_PARTY:
            case TARGET_GAMEOBJECT_SRC_AREA:
                SpellTargetType[i] = TARGET_TYPE_AREA_SRC;
                break;
            case TARGET_UNIT_DEST_AREA_ENEMY:
            case TARGET_UNIT_DEST_AREA_ALLY:
            case TARGET_UNIT_DEST_AREA_ENTRY:
            case TARGET_UNIT_DEST_AREA_PARTY:
            case TARGET_GAMEOBJECT_DEST_AREA:
                SpellTargetType[i] = TARGET_TYPE_AREA_DST;
                break;
            case TARGET_UNIT_CONE_ENEMY:
            case TARGET_UNIT_CONE_ALLY:
            case TARGET_UNIT_CONE_ENTRY:
            case TARGET_UNIT_CONE_ENEMY_UNKNOWN:
                SpellTargetType[i] = TARGET_TYPE_AREA_CONE;
                break;
            case TARGET_DEST_CASTER:
            case TARGET_SRC_CASTER:
            case TARGET_DEST_CASTER_SUMMON:
            case TARGET_DEST_CASTER_FRONT_LEAP:
            case TARGET_DEST_CASTER_FRONT:
            case TARGET_DEST_CASTER_BACK:
            case TARGET_DEST_CASTER_RIGHT:
            case TARGET_DEST_CASTER_LEFT:
            case TARGET_DEST_CASTER_FRONT_LEFT:
            case TARGET_DEST_CASTER_BACK_LEFT:
            case TARGET_DEST_CASTER_BACK_RIGHT:
            case TARGET_DEST_CASTER_FRONT_RIGHT:
            case TARGET_DEST_CASTER_RANDOM:
            case TARGET_DEST_CASTER_RADIUS:
                SpellTargetType[i] = TARGET_TYPE_DEST_CASTER;
                break;
            case TARGET_DEST_TARGET_ANY:
            case TARGET_DEST_TARGET_FRONT:
            case TARGET_DEST_TARGET_BACK:
            case TARGET_DEST_TARGET_RIGHT:
            case TARGET_DEST_TARGET_LEFT:
            case TARGET_DEST_TARGET_FRONT_LEFT:
            case TARGET_DEST_TARGET_BACK_LEFT:
            case TARGET_DEST_TARGET_BACK_RIGHT:
            case TARGET_DEST_TARGET_FRONT_RIGHT:
            case TARGET_DEST_TARGET_RANDOM:
            case TARGET_DEST_TARGET_RADIUS:
                SpellTargetType[i] = TARGET_TYPE_DEST_TARGET;
                break;
            case TARGET_DEST_DYNOBJ_ENEMY:
            case TARGET_DEST_DYNOBJ_ALLY:
            case TARGET_DEST_DYNOBJ_NONE:
            case TARGET_DEST_DEST:
            case TARGET_DEST_TRAJ:
            case TARGET_DEST_DEST_FRONT_LEFT:
            case TARGET_DEST_DEST_BACK_LEFT:
            case TARGET_DEST_DEST_BACK_RIGHT:
            case TARGET_DEST_DEST_FRONT_RIGHT:
            case TARGET_DEST_DEST_FRONT:
            case TARGET_DEST_DEST_BACK:
            case TARGET_DEST_DEST_RIGHT:
            case TARGET_DEST_DEST_LEFT:
            case TARGET_DEST_DEST_RANDOM:
                SpellTargetType[i] = TARGET_TYPE_DEST_DEST;
                break;
            case TARGET_DEST_DB:
            case TARGET_DEST_HOME:
            case TARGET_DEST_NEARBY_ENTRY:
                SpellTargetType[i] = TARGET_TYPE_DEST_SPECIAL;
                break;
            case TARGET_UNIT_CHANNEL:
            case TARGET_DEST_CHANNEL:
                SpellTargetType[i] = TARGET_TYPE_CHANNEL;
                break;
            case TARGET_DEST_TARGET_ENEMY:
                SpellTargetType[i] = TARGET_TYPE_DEST_TARGET_ENEMY;
                break;
            default:
                SpellTargetType[i] = TARGET_TYPE_DEFAULT;
        }
    }

    for(int i = 0; i < TOTAL_SPELL_TARGETS; ++i)
    {
        switch(i)
        {
            case TARGET_UNIT_DEST_AREA_ENEMY:
            case TARGET_UNIT_SRC_AREA_ENEMY:
            case TARGET_UNIT_DEST_AREA_ALLY:
            case TARGET_UNIT_SRC_AREA_ALLY:
            case TARGET_UNIT_DEST_AREA_ENTRY:
            case TARGET_UNIT_SRC_AREA_ENTRY:
            case TARGET_UNIT_DEST_AREA_PARTY:
            case TARGET_UNIT_SRC_AREA_PARTY:
            case TARGET_UNIT_LASTTARGET_AREA_PARTY:
            case TARGET_UNIT_CASTER_AREA_PARTY:
            case TARGET_UNIT_CONE_ENEMY:
            case TARGET_UNIT_CONE_ALLY:
            case TARGET_UNIT_CONE_ENEMY_UNKNOWN:
            case TARGET_UNIT_CASTER_AREA_RAID:
                IsAreaEffectTarget[i] = true;
                break;
            default:
                IsAreaEffectTarget[i] = false;
                break;
        }
    }
}

SpellMgr::~SpellMgr()
{
    UnloadSpellInfoStore();
}

/* If you already got spellInfo, use spellInfo->IsPassive, this is just a helper if you don't */
bool IsPassiveSpell(uint32 spellId)
{
    SpellInfo const *spellInfo = sSpellMgr->GetSpellInfo(spellId);
    if (!spellInfo)
        return false;
    return spellInfo->IsPassive();
}

/*bool IsNoStackAuraDueToAura(uint32 spellId_1, uint32 effIndex_1, uint32 spellId_2, uint32 effIndex_2)
{
    SpellInfo const *spellInfo_1 = sSpellMgr->GetSpellInfo(spellId_1);
    SpellInfo const *spellInfo_2 = sSpellMgr->GetSpellInfo(spellId_2);
    if(!spellInfo_1 || !spellInfo_2) return false;
    if(spellInfo_1->Id == spellId_2) return false;

    if (spellInfo_1->Effects[effIndex_1].Effect != spellInfo_2->Effects[effIndex_2].Effect ||
        spellInfo_1->Effects[effIndex_1].ItemType != spellInfo_2->Effects[effIndex_2].ItemType ||
        spellInfo_1->Effects[effIndex_1].MiscValue != spellInfo_2->Effects[effIndex_2].MiscValue ||
        spellInfo_1->Effects[effIndex_1].ApplyAuraName != spellInfo_2->Effects[effIndex_2].ApplyAuraName)
        return false;

    return true;
}*/

int32 CompareAuraRanks(uint32 spellId_1, uint32 effIndex_1, uint32 spellId_2, uint32 effIndex_2)
{
    SpellInfo const*spellInfo_1 = sSpellMgr->GetSpellInfo(spellId_1);
    SpellInfo const*spellInfo_2 = sSpellMgr->GetSpellInfo(spellId_2);
    if(!spellInfo_1 || !spellInfo_2) return 0;
    if (spellId_1 == spellId_2) return 0;

    int32 diff = spellInfo_1->Effects[effIndex_1].BasePoints - spellInfo_2->Effects[effIndex_2].BasePoints;
    if (spellInfo_1->Effects[effIndex_1].BasePoints+1 < 0 && spellInfo_2->Effects[effIndex_2].BasePoints+1 < 0) return -diff;
    else return diff;
}

bool IsSingleFromSpellSpecificPerCaster(uint32 spellSpec1,uint32 spellSpec2)
{
    switch(spellSpec1)
    {
        case SPELL_SEAL:
        case SPELL_BLESSING:
        case SPELL_AURA:
        case SPELL_STING:
        case SPELL_CURSE:
        case SPELL_ASPECT:
        case SPELL_POSITIVE_SHOUT:
        case SPELL_JUDGEMENT:
        case SPELL_WARLOCK_CORRUPTION:
            return spellSpec1==spellSpec2;
        default:
            return false;
    }
}

bool IsSingleFromSpellSpecificPerTarget(uint32 spellSpec1,uint32 spellSpec2)
{
    switch(spellSpec1)
    {
        case SPELL_TRACKER:
        case SPELL_WARLOCK_ARMOR:
        case SPELL_MAGE_ARMOR:
        case SPELL_ELEMENTAL_SHIELD:
        case SPELL_MAGE_POLYMORPH:
        case SPELL_WELL_FED:
        case SPELL_DRINK:
        case SPELL_FOOD:
        case SPELL_CHARM:
        case SPELL_WARRIOR_ENRAGE:
        case SPELL_DRUID_MANGLE:
        case SPELL_ARMOR_REDUCE:
            return spellSpec1==spellSpec2;
        case SPELL_BATTLE_ELIXIR:
            return spellSpec2==SPELL_BATTLE_ELIXIR
                || spellSpec2==SPELL_FLASK_ELIXIR;
        case SPELL_GUARDIAN_ELIXIR:
            return spellSpec2==SPELL_GUARDIAN_ELIXIR
                || spellSpec2==SPELL_FLASK_ELIXIR;
        case SPELL_FLASK_ELIXIR:
            return spellSpec2==SPELL_BATTLE_ELIXIR
                || spellSpec2==SPELL_GUARDIAN_ELIXIR
                || spellSpec2==SPELL_FLASK_ELIXIR;
        default:
            return false;
    }
}

bool IsSingleTargetSpells(SpellInfo const *spellInfo1, SpellInfo const *spellInfo2)
{
    // TODO - need better check
    // Equal icon and spellfamily
    if( spellInfo1->SpellFamilyName == spellInfo2->SpellFamilyName &&
        spellInfo1->SpellIconID == spellInfo2->SpellIconID )
        return true;

    // TODO - need found Judgements rule
    SpellSpecificType spec1 = spellInfo1->GetSpellSpecific();
    // spell with single target specific types
    switch(spec1)
    {
        case SPELL_JUDGEMENT:
        case SPELL_MAGE_POLYMORPH:
            if(spellInfo2->GetSpellSpecific() == spec1)
                return true;
            break;
        default:
            break;
    }

    return false;
}

bool IsAuraAddedBySpell(uint32 auraType, uint32 spellId)
{
    SpellInfo const *spellproto = sSpellMgr->GetSpellInfo(spellId);
    if (!spellproto) return false;

    for (int i = 0; i < MAX_SPELL_EFFECTS; i++)
        if (spellproto->Effects[i].ApplyAuraName == auraType)
            return true;
    return false;
}

SpellCastResult GetErrorAtShapeshiftedCast(SpellInfo const *spellInfo, uint32 form)
{
    // talents that learn spells can have stance requirements that need ignore
    // (this requirement only for client-side stance show in talent description)
    if( GetTalentSpellCost(spellInfo->Id) > 0 &&
        (spellInfo->Effects[0].Effect==SPELL_EFFECT_LEARN_SPELL || spellInfo->Effects[1].Effect==SPELL_EFFECT_LEARN_SPELL || spellInfo->Effects[2].Effect==SPELL_EFFECT_LEARN_SPELL) )
        return SPELL_CAST_OK;

    uint32 stanceMask = (form ? 1 << (form - 1) : 0);
    if (stanceMask & spellInfo->StancesNot)                 // can explicitly not be casted in this stance
        return SPELL_FAILED_NOT_SHAPESHIFT;

    if (stanceMask & spellInfo->Stances)                    // can explicitly be casted in this stance
        return SPELL_CAST_OK;
        
    // Spirit of Redemption
    if (form == 0x20 && spellInfo->IsPositive() && spellInfo->SpellFamilyName == SPELLFAMILY_PRIEST)
        return SPELL_CAST_OK;

    bool actAsShifted = false;
    if (form > 0)
    {
        SpellShapeshiftEntry const *shapeInfo = sSpellShapeshiftStore.LookupEntry(form);
        if (!shapeInfo)
        {
            TC_LOG_ERROR("spell","GetErrorAtShapeshiftedCast: unknown shapeshift %u", form);
            return SPELL_CAST_OK;
        }
        actAsShifted = !(shapeInfo->flags1 & 1);            // shapeshift acts as normal form for spells
    }

    if(actAsShifted)
    {
        if (spellInfo->Attributes & SPELL_ATTR0_NOT_SHAPESHIFT) // not while shapeshifted
            return SPELL_FAILED_NOT_SHAPESHIFT;
        else if (spellInfo->Stances != 0)                   // needs other shapeshift
            return SPELL_FAILED_ONLY_SHAPESHIFT;
    }
    else
    {
        // needs shapeshift
        if(!(spellInfo->HasAttribute(SPELL_ATTR2_NOT_NEED_SHAPESHIFT)) && spellInfo->Stances != 0)
            return SPELL_FAILED_ONLY_SHAPESHIFT;
    }

    return SPELL_CAST_OK;
}

void SpellMgr::LoadSpellTargetPositions()
{
    mSpellTargetPositions.clear();                                // need for reload case

    uint32 count = 0;

    //                                                0   1           2                  3                  4                  5
    QueryResult result = WorldDatabase.Query("SELECT id, target_map, target_position_x, target_position_y, target_position_z, target_orientation FROM spell_target_position");
    if( !result )
    {
        TC_LOG_INFO("server.loading", ">> Loaded %u spell target coordinates", count );
        return;
    }

    do
    {
        Field *fields = result->Fetch();

        uint32 Spell_ID = fields[0].GetUInt32();

        SpellTargetPosition st;

        st.target_mapId       = fields[1].GetUInt16();
        st.target_X           = fields[2].GetFloat();
        st.target_Y           = fields[3].GetFloat();
        st.target_Z           = fields[4].GetFloat();
        st.target_Orientation = fields[5].GetFloat();
        
        MapEntry const* mapEntry = sMapStore.LookupEntry(st.target_mapId);
        if(!mapEntry)
        {
            TC_LOG_ERROR("spell","Spell (ID:%u) target map (ID: %u) does not exist in `Map.dbc`.",Spell_ID,st.target_mapId);
            continue;
        }

        if(st.target_X==0 && st.target_Y==0 && st.target_Z==0)
        {
            TC_LOG_ERROR("spell","Spell (ID:%u) target coordinates not provided.",Spell_ID);
            continue;
        }

        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(Spell_ID);
        if(!spellInfo)
        {
            TC_LOG_ERROR("spell","Spell (ID:%u) listed in `spell_target_position` does not exist.",Spell_ID);
            continue;
        }

        bool found = false;
        for(int i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if( spellInfo->Effects[i].TargetA.GetTarget()==TARGET_DEST_DB || spellInfo->Effects[i].TargetB.GetTarget()==TARGET_DEST_DB )
            {
                found = true;
                break;
            }
        }
        if(!found)
        {
            TC_LOG_ERROR("spell","Spell (Id: %u) listed in `spell_target_position` does not have target TARGET_DEST_DB (17).",Spell_ID);
            continue;
        }

        mSpellTargetPositions[Spell_ID] = st;
        ++count;

    } while( result->NextRow() );

    TC_LOG_INFO("server.loading"," ");
    TC_LOG_INFO("server.loading", ">> Loaded %u spell teleport coordinates", count );
}

void SpellMgr::LoadSpellAffects()
{
    mSpellAffectMap.clear();                                // need for reload case

    uint32 count = 0;

    //                                                0      1         2
    QueryResult result = WorldDatabase.Query("SELECT entry, effectId, SpellFamilyMask FROM spell_affect");
    if( !result )
    {
        TC_LOG_INFO("server.loading", ">> Loaded %u spell affect definitions", count );
        return;
    }

    do
    {
        Field *fields = result->Fetch();

        uint16 entry = fields[0].GetUInt32();
        uint8 effectId = fields[1].GetUInt8();

        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry);

        if (!spellInfo)
        {
            TC_LOG_ERROR("server.loading","Spell %u listed in `spell_affect` does not exist", entry);
            continue;
        }

        if (effectId >= 3)
        {
            TC_LOG_ERROR("server.loading","Spell %u listed in `spell_affect` have invalid effect index (%u)", entry,effectId);
            continue;
        }

        if( spellInfo->Effects[effectId].Effect != SPELL_EFFECT_APPLY_AURA ||
            (spellInfo->Effects[effectId].ApplyAuraName != SPELL_AURA_ADD_FLAT_MODIFIER &&
            spellInfo->Effects[effectId].ApplyAuraName != SPELL_AURA_ADD_PCT_MODIFIER  &&
            spellInfo->Effects[effectId].ApplyAuraName != SPELL_AURA_ADD_TARGET_TRIGGER) )
        {
            TC_LOG_ERROR("server.loading","Spell %u listed in `spell_affect` have not SPELL_AURA_ADD_FLAT_MODIFIER (%u) or SPELL_AURA_ADD_PCT_MODIFIER (%u) or SPELL_AURA_ADD_TARGET_TRIGGER (%u) for effect index (%u)", entry,SPELL_AURA_ADD_FLAT_MODIFIER,SPELL_AURA_ADD_PCT_MODIFIER,SPELL_AURA_ADD_TARGET_TRIGGER,effectId);
            continue;
        }

        uint64 spellAffectMask = fields[2].GetUInt64();

        // Spell.dbc have own data for low part of SpellFamilyMask
        if( spellInfo->Effects[effectId].ItemType)
        {
            if(spellInfo->Effects[effectId].ItemType == spellAffectMask)
            {
                TC_LOG_ERROR("server.loading","Spell %u listed in `spell_affect` have redundant (same with EffectItemType%d) data for effect index (%u) and not needed, skipped.", entry,effectId+1,effectId);
                continue;
            }

            // 24429 have wrong data in EffectItemType and overwrites by DB, possible bug in client
            if(spellInfo->Id != 24429 && spellInfo->Id != 11189 && spellInfo->Id != 28332 && spellInfo->Effects[effectId].ItemType != spellAffectMask)
            {
                TC_LOG_ERROR("server.loading","Spell %u listed in `spell_affect` have different low part from EffectItemType%d for effect index (%u) and not needed, skipped.", entry,effectId+1,effectId);
                continue;
            }
        }

        mSpellAffectMap.insert(SpellAffectMap::value_type((entry<<8) + effectId,spellAffectMask));

        ++count;
    } while( result->NextRow() );

    TC_LOG_INFO("server.loading", ">> Loaded %u spell affect definitions", count );

    //for (uint32 id = 0; id < sSpellStore.GetNumRows(); ++id)
    for (auto itr = sObjectMgr->GetSpellStore().begin(); itr != sObjectMgr->GetSpellStore().end(); itr++)
    {
        uint32 id = itr->first;
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(id);
        if (!spellInfo)
            continue;

        for (int effectId = 0; effectId < 3; ++effectId)
        {
            if( spellInfo->Effects[effectId].Effect != SPELL_EFFECT_APPLY_AURA ||
                (spellInfo->Effects[effectId].ApplyAuraName != SPELL_AURA_ADD_FLAT_MODIFIER &&
                spellInfo->Effects[effectId].ApplyAuraName != SPELL_AURA_ADD_PCT_MODIFIER  &&
                spellInfo->Effects[effectId].ApplyAuraName != SPELL_AURA_ADD_TARGET_TRIGGER) )
                continue;

            if(spellInfo->Effects[effectId].ItemType != 0)
                continue;

            if(mSpellAffectMap.find((id<<8) + effectId) !=  mSpellAffectMap.end())
                continue;

            TC_LOG_ERROR("server.loading","Spell %u (%s) misses spell_affect for effect %u",id,spellInfo->SpellName[sWorld->GetDefaultDbcLocale()], effectId);
        }
    }
}

bool SpellMgr::IsAffectedBySpell(SpellInfo const *spellInfo, uint32 spellId, uint8 effectId, uint64 familyFlags) const
{
    // false for spellInfo == NULL
    if (!spellInfo)
        return false;

    SpellInfo const *affect_spell = sSpellMgr->GetSpellInfo(spellId);
    // false for affect_spell == NULL
    if (!affect_spell)
        return false;

    // False if spellFamily not equal
    if (affect_spell->SpellFamilyName != spellInfo->SpellFamilyName)
        return false;

    // If familyFlags == 0
    if (!familyFlags)
    {
        // Get it from spellAffect table
        familyFlags = GetSpellAffectMask(spellId,effectId);
        // false if familyFlags == 0
        if (!familyFlags)
            return false;
    }
    // true
    if (familyFlags & spellInfo->SpellFamilyFlags)
        return true;
        
    return false;
}

void SpellMgr::LoadSpellProcEvents()
{
    mSpellProcEventMap.clear();                             // need for reload case

    uint32 count = 0;

    //                                                0      1           2                3                4          5       6        7             8
    QueryResult result = WorldDatabase.Query("SELECT entry, SchoolMask, SpellFamilyName, SpellFamilyMask, ProcFlags, procEx, ppmRate, CustomChance, Cooldown FROM spell_proc_event");
    if( !result )
    {
        TC_LOG_INFO("server.loading"," ");
        TC_LOG_INFO("server.loading", ">> Loaded %u spell proc event conditions", count  );
        return;
    }

    uint32 customProc = 0;
    do
    {
        Field *fields = result->Fetch();

        uint16 entry = fields[0].GetUInt32();

        const SpellInfo *spell = sSpellMgr->GetSpellInfo(entry);
        if (!spell)
        {
            TC_LOG_ERROR("server.loading","Spell %u listed in `spell_proc_event` does not exist", entry);
            continue;
        }

        SpellProcEventEntry spe;

        spe.schoolMask      = fields[1].GetUInt8();
        spe.spellFamilyName = fields[2].GetUInt16();
        spe.spellFamilyMask = fields[3].GetUInt64();
        spe.ProcFlags       = fields[4].GetUInt32();
        spe.procEx          = fields[5].GetUInt32();
        spe.ppmRate         = fields[6].GetFloat();
        spe.customChance    = fields[7].GetFloat();
        spe.cooldown        = fields[8].GetUInt32();

        mSpellProcEventMap[entry] = spe;

        if (spell->ProcFlags==0)
        {
            if (spe.ProcFlags == 0)
            {
                TC_LOG_ERROR("sql.sql","Spell %u listed in `spell_proc_event` probably not triggered spell", entry);
                continue;
            }
            customProc++;
        }
        ++count;
    } while( result->NextRow() );

    TC_LOG_INFO("server.loading"," ");
    if (customProc)
        TC_LOG_INFO("server.loading", ">> Loaded %u custom spell proc event conditions +%u custom",  count, customProc );
    else
        TC_LOG_INFO("server.loading", ">> Loaded %u spell proc event conditions", count );

    /*
    // Commented for now, as it still produces many errors (still quite many spells miss spell_proc_event)
    for (uint32 id = 0; id < sSpellStore.GetNumRows(); ++id)
    {
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(id);
        if (!spellInfo)
            continue;

        bool found = false;
        for (int effectId = 0; effectId < 3; ++effectId)
        {
            // at this moment check only SPELL_AURA_PROC_TRIGGER_SPELL
            if( spellInfo->Effects[effectId].ApplyAuraName == SPELL_AURA_PROC_TRIGGER_SPELL )
            {
                found = true;
                break;
            }
        }

        if(!found)
            continue;

        if(GetSpellProcEvent(id))
            continue;

        TC_LOG_ERROR("Spell %u (%s) misses spell_proc_event",id,spellInfo->SpellName[sWorld->GetDBClang()]);
    }
    */
}

/*
ProcFlags = proc flags generated from current event
EventProcFlag = at which event should we proc (either from spell_proc_event or spell proto if no entry in the table)
procExtra = extra info from current event
*/
bool SpellMgr::IsSpellProcEventCanTriggeredBy(SpellProcEventEntry const * spellProcEvent, uint32 EventProcFlag, SpellInfo const * procSpell, uint32 ProcFlags, uint32 procExtra, bool active)
{
    // No extra req need
    uint32 procEvent_procEx = PROC_EX_NONE;

    // check prockFlags for condition
    if((ProcFlags & EventProcFlag) == 0)
        return false;

    /* Check Periodic Auras

    * Both hots and dots can trigger if spell has no PROC_FLAG_SUCCESSFUL_POSITIVE_SPELL
        nor PROC_FLAG_SUCCESSFUL_NEGATIVE_SPELL_HIT

    *Only Hots can trigger if spell has PROC_FLAG_SUCCESSFUL_POSITIVE_SPELL

    *Only dots can trigger if spell has both positivity flags or PROC_FLAG_SUCCESSFUL_NEGATIVE_SPELL_HIT

    */

    if (ProcFlags & PROC_FLAG_ON_DO_PERIODIC)
    {
        if (EventProcFlag & PROC_FLAG_SUCCESSFUL_NEGATIVE_SPELL_HIT)
        {
            if (!(procExtra & PROC_EX_INTERNAL_DOT))
                return false;
        }
        else if (EventProcFlag & PROC_FLAG_SUCCESSFUL_POSITIVE_SPELL
            && !(procExtra & PROC_EX_INTERNAL_HOT))
            return false;
    }

    if (ProcFlags & PROC_FLAG_TAKEN_PERIODIC)
    {
        if (EventProcFlag & PROC_FLAG_TAKEN_NEGATIVE_SPELL_HIT)
        {
            if (!(procExtra & PROC_EX_INTERNAL_DOT))
                return false;
        }
        else if (EventProcFlag & PROC_FLAG_TAKEN_POSITIVE_SPELL
            && !(procExtra & PROC_EX_INTERNAL_HOT))
            return false;
    }

    // Always trigger for this
    if (EventProcFlag & (PROC_FLAG_KILLED | PROC_FLAG_KILL_AND_GET_XP | PROC_FLAG_ON_TRAP_ACTIVATION))
        return true;
    //for traps : this is not actually true, not every trap should trigger this. Example : Entrapment talent shouldn't proc on Freezing Trap

    if (spellProcEvent)     // Exist event data
    {
        // Store extra req
        procEvent_procEx = spellProcEvent->procEx;

        // For melee triggers
        if (procSpell == NULL)
        {
            // Check (if set) for school (melee attack have Normal school)
            if(spellProcEvent->schoolMask && (spellProcEvent->schoolMask & SPELL_SCHOOL_MASK_NORMAL) == 0)
                return false;
        }
        else // For spells need check school/spell family/family mask
        {
            // Potions can trigger only if spellfamily given
            if (procSpell->SpellFamilyName == SPELLFAMILY_POTION)
            {
                if (procSpell->SpellFamilyName == spellProcEvent->spellFamilyName)
                    return true;
                return false;
            }
            
            // Check (if set) for school
            if(spellProcEvent->schoolMask && (spellProcEvent->schoolMask & procSpell->SchoolMask) == 0)
                return false;

            // Check (if set) for spellFamilyName
            if(spellProcEvent->spellFamilyName && (spellProcEvent->spellFamilyName != procSpell->SpellFamilyName))
                return false;

            // spellFamilyName is Ok need check for spellFamilyMask if present
            if(spellProcEvent->spellFamilyMask)
            {
                if ((spellProcEvent->spellFamilyMask & procSpell->SpellFamilyFlags) == 0)
                    return false;
                active = true; // Spell added manualy -> so its active spell
            }
        }
    }

    // Check for extra req (if none) and hit/crit
    if (procEvent_procEx == PROC_EX_NONE)
    {
        // No extra req, so can trigger only for active (damage/healing present) and hit/crit
        if((procExtra & (PROC_EX_NORMAL_HIT|PROC_EX_CRITICAL_HIT)) && active)
            return true;
    }
    else // Passive spells hits here only if resist/reflect/immune/evade
    {
        // Exist req for PROC_EX_EX_TRIGGER_ALWAYS
        if (procEvent_procEx & PROC_EX_EX_TRIGGER_ALWAYS)
            return true;
        // Passive spells can`t trigger if need hit
        if ((procEvent_procEx & PROC_EX_NORMAL_HIT) && !active)
            return false;
        // Check Extra Requirement like (hit/crit/miss/resist/parry/dodge/block/immune/reflect/absorb and other)
        if (procEvent_procEx & procExtra)
            return true;
    }
    
    return false;
}

void SpellMgr::LoadSpellElixirs()
{
    mSpellElixirs.clear();                                  // need for reload case

    uint32 count = 0;

    //                                                0      1
    QueryResult result = WorldDatabase.Query("SELECT entry, mask FROM spell_elixir");
    if( !result )
    {
        TC_LOG_INFO("server.loading"," ");
        TC_LOG_INFO("server.loading", ">> Loaded %u spell elixir definitions", count );
        return;
    }

    do
    {
        Field *fields = result->Fetch();

        uint16 entry = fields[0].GetUInt32();
        uint8 mask = fields[1].GetUInt8();

        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry);

        if (!spellInfo)
        {
            TC_LOG_ERROR("server.loading","Spell %u listed in `spell_elixir` does not exist", entry);
            continue;
        }

        mSpellElixirs[entry] = mask;

        ++count;
    } while( result->NextRow() );

    TC_LOG_INFO("server.loading"," ");
    TC_LOG_INFO("server.loading", ">> Loaded %u spell elixir definitions", count );
}


void SpellMgr::LoadSpellBonusess()
{
    uint32 oldMSTime = GetMSTime();

    mSpellBonusMap.clear();                             // need for reload case

                                                        //                                                0      1             2          3         4
    QueryResult result = WorldDatabase.Query("SELECT entry, direct_bonus, dot_bonus, ap_bonus, ap_dot_bonus FROM spell_bonus_data");
    if (!result)
    {
        TC_LOG_INFO("server.loading", ">> Loaded 0 spell bonus data. DB table `spell_bonus_data` is empty.");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();
        uint32 entry = fields[0].GetUInt32();

        SpellInfo const* spell = GetSpellInfo(entry);
        if (!spell)
        {
            TC_LOG_ERROR("sql.sql", "Spell %u listed in `spell_bonus_data` does not exist", entry);
            continue;
        }

        SpellBonusEntry& sbe = mSpellBonusMap[entry];
        sbe.direct_damage = fields[1].GetFloat();
        sbe.dot_damage = fields[2].GetFloat();
        sbe.ap_bonus = fields[3].GetFloat();
        sbe.ap_dot_bonus = fields[4].GetFloat();

        ++count;
    } while (result->NextRow());

    TC_LOG_INFO("server.loading", ">> Loaded %u extra spell bonus data in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void SpellMgr::LoadSpellThreats()
{
    uint32 oldMSTime = GetMSTime();

    mSpellThreatMap.clear();                                // need for reload case

    //                                                0      1        2   
    QueryResult result = WorldDatabase.Query("SELECT entry, flatMod, pctMod FROM spell_threat");
    if (!result)
    {
        TC_LOG_INFO("server.loading", ">> Loaded 0 aggro generating spells. DB table `spell_threat` is empty.");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        uint32 entry = fields[0].GetUInt32();

        if (!GetSpellInfo(entry))
        {
            TC_LOG_ERROR("sql.sql", "Spell %u listed in `spell_threat` does not exist", entry);
            continue;
        }

        SpellThreatEntry ste;
        ste.flatMod  = fields[1].GetInt32();
        ste.pctMod   = fields[2].GetFloat();

        mSpellThreatMap[entry] = ste;
        ++count;
    } while (result->NextRow());

    TC_LOG_INFO("server.loading", ">> Loaded %u SpellThreatEntries in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void SpellMgr::LoadSpellEnchantProcData()
{
    mSpellEnchantProcEventMap.clear();                             // need for reload case

    uint32 count = 0;

    //                                                0      1             2          3
    QueryResult result = WorldDatabase.Query("SELECT entry, customChance, PPMChance, procEx FROM spell_enchant_proc_data");
    if( !result )
    {
        TC_LOG_INFO("server.loading"," ");
        TC_LOG_INFO("server.loading", ">> Loaded %u spell enchant proc event conditions", count );
        return;
    }

    do
    {
        Field *fields = result->Fetch();

        uint32 enchantId = fields[0].GetUInt32();

        SpellItemEnchantmentEntry const *ench = sSpellItemEnchantmentStore.LookupEntry(enchantId);
        if (!ench)
        {
            TC_LOG_ERROR("server.loading","Enchancment %u listed in `spell_enchant_proc_data` does not exist", enchantId);
            continue;
        }

        SpellEnchantProcEntry spe;

        spe.customChance = fields[1].GetUInt32();
        spe.PPMChance = fields[2].GetFloat();
        spe.procEx = fields[3].GetFloat();

        mSpellEnchantProcEventMap[enchantId] = spe;

        ++count;
    } while( result->NextRow() );

    TC_LOG_INFO("server.loading", ">> Loaded %u enchant proc data definitions", count);
}

bool SpellMgr::IsRankSpellDueToSpell(SpellInfo const *spellInfo_1,uint32 spellId_2) const
{
    SpellInfo const *spellInfo_2 = sSpellMgr->GetSpellInfo(spellId_2);
    if(!spellInfo_1 || !spellInfo_2) return false;
    if(spellInfo_1->Id == spellId_2) return false;

    return GetFirstSpellInChain(spellInfo_1->Id)==GetFirstSpellInChain(spellId_2);
}

bool SpellMgr::canStackSpellRanks(SpellInfo const *spellInfo)
{
    //hack for faerie fire, client has wrong data in SkillLineAbility so let's send every rank
    if( spellInfo->SpellFamilyName == SPELLFAMILY_DRUID && spellInfo->SpellFamilyFlags == 0x400)
        return true; 
    if(spellInfo->IsPassive())
        return false;
    if(spellInfo->PowerType != POWER_MANA && spellInfo->PowerType != POWER_HEALTH)
        return false;
    if(IsProfessionSpell(spellInfo->Id))
        return false;

    // All stance spells. if any better way, change it.
    for (int i = 0; i < MAX_SPELL_EFFECTS; i++)
    {
        // Paladin aura Spell
        if(spellInfo->SpellFamilyName == SPELLFAMILY_PALADIN
            && spellInfo->Effects[i].Effect==SPELL_EFFECT_APPLY_AREA_AURA_PARTY)
            return false;
        // Druid form Spell
        if(spellInfo->SpellFamilyName == SPELLFAMILY_DRUID
            && spellInfo->Effects[i].Effect==SPELL_EFFECT_APPLY_AURA
            && spellInfo->Effects[i].ApplyAuraName == SPELL_AURA_MOD_SHAPESHIFT)
            return false;
        // Rogue Stealth
        if(spellInfo->SpellFamilyName == SPELLFAMILY_ROGUE
            && spellInfo->Effects[i].Effect==SPELL_EFFECT_APPLY_AURA
            && spellInfo->Effects[i].ApplyAuraName == SPELL_AURA_MOD_SHAPESHIFT)
            return false;
    }
    
    switch (spellInfo->Id) {
    case 14326: //"Scare Beast"
    case 14327: //"Scare Beast"
        return false;
    default:
        break;
    }
    
    return true;
}

bool SpellMgr::IsNoStackSpellDueToSpell(uint32 spellId_1, uint32 spellId_2, bool sameCaster) const
{
    //if(spellId_1 == spellId_2) // auras due to the same spell
    //    return false;
    SpellInfo const *spellInfo_1 = sSpellMgr->GetSpellInfo(spellId_1);
    SpellInfo const *spellInfo_2 = sSpellMgr->GetSpellInfo(spellId_2);

    if(!spellInfo_1 || !spellInfo_2)
        return false;

    SpellSpecificType spellId_spec_1 = spellInfo_1->GetSpellSpecific();
    SpellSpecificType spellId_spec_2 = spellInfo_2->GetSpellSpecific();
    if (spellId_spec_1 && spellId_spec_2)
        if (IsSingleFromSpellSpecificPerTarget(spellId_spec_1, spellId_spec_2)
            ||(IsSingleFromSpellSpecificPerCaster(spellId_spec_1, spellId_spec_2) && sameCaster))
            return true;

    // spells with different specific always stack
    if(spellId_spec_1 != spellId_spec_2)
        return false;

    if(spellInfo_1->SpellFamilyName != spellInfo_2->SpellFamilyName)
        return false;

    // generic spells
    if(!spellInfo_1->SpellFamilyName)
    {
        if(!spellInfo_1->SpellIconID
            || spellInfo_1->SpellIconID == 1
            || spellInfo_1->SpellIconID != spellInfo_2->SpellIconID
            || (spellInfo_1->SpellIconID == 156 && spellInfo_2->SpellIconID == 156)
            || (spellInfo_1->SpellIconID == 240 && spellInfo_2->SpellIconID == 240)
            || (spellInfo_1->SpellIconID == 502 && spellInfo_2->SpellIconID == 502))
            return false;
    }

    // check for class spells
    else
    {
        if (spellInfo_1->SpellFamilyFlags != spellInfo_2->SpellFamilyFlags)
            return false;
    }
    
    if (spellInfo_1->SpellIconID == 59 && spellInfo_2->SpellIconID == 59)
        return true;

    if(!sameCaster)
    {
        if (spellInfo_1->HasAttribute(SPELL_ATTR3_STACK_FOR_DIFF_CASTERS))
            return true;
            
        for(uint32 i = 0; i < MAX_SPELL_EFFECTS; ++i) {
            if (spellInfo_1->Effects[i].Effect == SPELL_EFFECT_APPLY_AURA
                || spellInfo_1->Effects[i].Effect == SPELL_EFFECT_PERSISTENT_AREA_AURA) {
                // not area auras (shaman totem)
                switch(spellInfo_1->Effects[i].ApplyAuraName)
                {
                    case SPELL_AURA_MOD_DECREASE_SPEED:
                        // Mind Flay
                        if(spellInfo_1->SpellFamilyFlags & 0x0000000000800000LL && spellInfo_1->SpellFamilyName == SPELLFAMILY_PRIEST &&
                           spellInfo_2->SpellFamilyFlags & 0x0000000000800000LL && spellInfo_2->SpellFamilyName == SPELLFAMILY_PRIEST)
                            return false;
                        break;
                    case SPELL_AURA_DUMMY:
                        /* X don't merge to TC2 - spell removed */
                        // Blessing of Light exception - only one per caster allowed on a target
                        if (spellInfo_1->HasVisual(9180) && spellInfo_1->SpellFamilyName == SPELLFAMILY_PALADIN &&
                            spellInfo_2->HasVisual(9180) && spellInfo_2->SpellFamilyName == SPELLFAMILY_PALADIN)
                            break;
                        /* /X */
                    // DOT or HOT from different casters will stack
                    case SPELL_AURA_PERIODIC_DAMAGE:
                    case SPELL_AURA_PERIODIC_HEAL:
                    case SPELL_AURA_PERIODIC_TRIGGER_SPELL:
                    case SPELL_AURA_PERIODIC_ENERGIZE:
                    case SPELL_AURA_PERIODIC_MANA_LEECH:
                    case SPELL_AURA_PERIODIC_LEECH:
                    case SPELL_AURA_POWER_BURN_MANA:
                    case SPELL_AURA_OBS_MOD_POWER:
                    case SPELL_AURA_OBS_MOD_HEALTH:
                        return false;
                    default:
                        break;
                }
            }
        }
        
        if ((spellInfo_1->SpellIconID == 2312 && spellInfo_2->SpellIconID == 2312)
            || (spellInfo_1->SpellIconID == 44955 && spellInfo_2->SpellIconID == 44955))
            return true;
    }

//    not needed now because we compare effects last rank of spells
//    if(spellInfo_1->SpellFamilyName && IsRankSpellDueToSpell(spellInfo_1, spellId_2))
//        return true;

    //use data of highest rank spell(needed for spells which ranks have different effects)
    spellInfo_1=sSpellMgr->GetSpellInfo(GetLastSpellInChain(spellId_1));
    spellInfo_2=sSpellMgr->GetSpellInfo(GetLastSpellInChain(spellId_2));

    //if spells have exactly the same effect they cannot stack
    for(uint32 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        if(spellInfo_1->Effects[i].Effect != spellInfo_2->Effects[i].Effect
            || spellInfo_1->Effects[i].ApplyAuraName != spellInfo_2->Effects[i].ApplyAuraName
            || spellInfo_1->Effects[i].MiscValue != spellInfo_2->Effects[i].MiscValue) // paladin resist aura
            return false; // need itemtype check? need an example to add that check

    return true;
}
bool SpellMgr::IsProfessionSpell(uint32 spellId)
{
    SpellInfo const *spellInfo = sSpellMgr->GetSpellInfo(spellId);
    if(!spellInfo)
        return false;

    if(spellInfo->Effects[1].Effect != SPELL_EFFECT_SKILL)
        return false;

    uint32 skill = spellInfo->Effects[1].MiscValue;

    return IsProfessionSkill(skill);
}

bool SpellMgr::IsPrimaryProfessionSpell(uint32 spellId)
{
    SpellInfo const *spellInfo = sSpellMgr->GetSpellInfo(spellId);
    if(!spellInfo)
        return false;

    if(spellInfo->Effects[1].Effect != SPELL_EFFECT_SKILL)
        return false;

    uint32 skill = spellInfo->Effects[1].MiscValue;

    return IsPrimaryProfessionSkill(skill);
}

bool SpellMgr::IsPrimaryProfessionFirstRankSpell(uint32 spellId) const
{
    return IsPrimaryProfessionSpell(spellId) && GetSpellRank(spellId)==1;
}

bool SpellMgr::IsNearbyEntryEffect(SpellInfo const* spellInfo, uint8 eff) const
{
    return     spellInfo->Effects[eff].TargetA.GetTarget() == TARGET_UNIT_NEARBY_ENTRY
            || spellInfo->Effects[eff].TargetB.GetTarget() == TARGET_UNIT_NEARBY_ENTRY
            || spellInfo->Effects[eff].TargetA.GetTarget() == TARGET_UNIT_SRC_AREA_ENTRY
            || spellInfo->Effects[eff].TargetB.GetTarget() == TARGET_UNIT_SRC_AREA_ENTRY
            || spellInfo->Effects[eff].TargetA.GetTarget() == TARGET_UNIT_DEST_AREA_ENTRY
            || spellInfo->Effects[eff].TargetB.GetTarget() == TARGET_UNIT_DEST_AREA_ENTRY;
}

SpellInfo const* SpellMgr::SelectAuraRankForPlayerLevel(SpellInfo const* spellInfo, uint32 playerLevel, bool hostileTarget) const
{
    // ignore passive spells
    if(spellInfo->IsPassive())
        return spellInfo;

    bool needRankSelection = false;
    for(int i=0;i<3;i++)
    {
        if( spellInfo->IsPositiveEffect(i, hostileTarget) && (
            spellInfo->Effects[i].Effect == SPELL_EFFECT_APPLY_AURA ||
            spellInfo->Effects[i].Effect == SPELL_EFFECT_APPLY_AREA_AURA_PARTY
            ) )
        {
            needRankSelection = true;
            break;
        }
    }

    // not required
    if(!needRankSelection)
        return spellInfo;

    for(uint32 nextSpellId = spellInfo->Id; nextSpellId != 0; nextSpellId = GetPrevSpellInChain(nextSpellId))
    {
        SpellInfo const *nextSpellInfo = sSpellMgr->GetSpellInfo(nextSpellId);
        if(!nextSpellInfo)
            break;

        // if found appropriate level
        if(playerLevel + 10 >= nextSpellInfo->SpellLevel)
            return nextSpellInfo;

        // one rank less then
    }

    // not found
    return NULL;
}

void SpellMgr::LoadSpellRequired()
{
    mSpellsReqSpell.clear();                                   // need for reload case
    mSpellReq.clear();                                         // need for reload case

    QueryResult result = WorldDatabase.Query("SELECT spell_id, req_spell from spell_required");

    if(result == NULL)
    {
        TC_LOG_INFO("server.loading", ">> Loaded 0 spell required records" );
        TC_LOG_ERROR("server.loading","`spell_required` table is empty!");
        return;
    }
    uint32 rows = 0;

    do
    {
        Field *fields = result->Fetch();

        uint32 spell_id = fields[0].GetUInt32();
        uint32 spell_req = fields[1].GetUInt32();

        mSpellsReqSpell.insert (std::pair<uint32, uint32>(spell_req, spell_id));
        mSpellReq[spell_id] = spell_req;
        ++rows;
    } while( result->NextRow() );

    TC_LOG_INFO("server.loading", ">> Loaded %u spell required records", rows );
}

struct SpellRankEntry
{
    uint32 SkillId;
    char const *SpellName;
    uint32 DurationIndex;
    uint32 RangeIndex;
    uint32 SpellVisual;
    uint32 ProcFlags;
    uint64 SpellFamilyFlags;
    uint32 TargetAuraState;
    uint32 ManaCost;

    bool operator()(const SpellRankEntry & _Left,const SpellRankEntry & _Right)const
    {
        //behold the most ugly ternary operator ever
        return (_Left.SkillId != _Right.SkillId ? _Left.SkillId < _Right.SkillId
            : (strcmp(_Left.SpellName, _Right.SpellName) != 0) ? ((strcmp(_Left.SpellName, _Right.SpellName) < 0) ? true : false)
            : _Left.ProcFlags != _Right.ProcFlags ? _Left.ProcFlags < _Right.ProcFlags
            : _Left.SpellFamilyFlags != _Right.SpellFamilyFlags ? _Left.SpellFamilyFlags < _Right.SpellFamilyFlags
            : (_Left.SpellVisual != _Right.SpellVisual) && (!_Left.SpellVisual || !_Right.SpellVisual) ? _Left.SpellVisual < _Right.SpellVisual
            : (_Left.ManaCost != _Right.ManaCost) && (!_Left.ManaCost || !_Right.ManaCost) ? _Left.ManaCost < _Right.ManaCost
            : (_Left.DurationIndex != _Right.DurationIndex) && (!_Left.DurationIndex || !_Right.DurationIndex)? _Left.DurationIndex < _Right.DurationIndex
            : (_Left.RangeIndex != _Right.RangeIndex) && (!_Left.RangeIndex || !_Right.RangeIndex || _Left.RangeIndex == 1 || (!_Right.RangeIndex) == 1) ? _Left.RangeIndex < _Right.RangeIndex
            : _Left.TargetAuraState < _Right.TargetAuraState
            );
    }
};

struct SpellRankValue
{
    uint32 Id;
    char const *Rank;
};

void SpellMgr::LoadSpellChains()
{
    mSpellChains.clear();                                   // need for reload case

    std::vector<uint32> ChainedSpells;
    for (uint32 ability_id=0;ability_id<sSkillLineAbilityStore.GetNumRows();ability_id++)
    {
        SkillLineAbilityEntry const *AbilityInfo=sSkillLineAbilityStore.LookupEntry(ability_id);
        if (!AbilityInfo)
            continue;
        if (AbilityInfo->spellId==20154) //exception to these rules (not needed in 3.0.3)
            continue;
        if (AbilityInfo->skillId == 756)
            continue;
        if (AbilityInfo->skillId == 125)
            continue;
        if (!AbilityInfo->forward_spellid)
            continue;
        /*if (AbilityInfo->forward_spellid == 20575 || AbilityInfo->forward_spellid == 20576 || AbilityInfo->forward_spellid == 21563)
            continue;*/
        ChainedSpells.push_back(AbilityInfo->forward_spellid);
    }

    std::multimap<SpellRankEntry, SpellRankValue, SpellRankEntry> RankMap;

    for (uint32 ability_id=0;ability_id<sSkillLineAbilityStore.GetNumRows();ability_id++)
    {
        SkillLineAbilityEntry const *AbilityInfo=sSkillLineAbilityStore.LookupEntry(ability_id);
        if (!AbilityInfo)
            continue;

        //get only spell with lowest ability_id to prevent doubles
        uint32 spell_id=AbilityInfo->spellId;
        if (spell_id==20154) //exception to these rules (not needed in 3.0.3)
            continue;
        if (AbilityInfo->skillId == 125)
            continue;
        bool found=false;
        for (uint32 i=0; i<ChainedSpells.size(); i++)
        {
           if (ChainedSpells.at(i)==spell_id)
               found=true;
        }
        if (found)
            continue;

        if(mSkillLineAbilityMap.lower_bound(spell_id)->second->id!=ability_id)
            continue;
        SpellInfo const *SpellInfo=sSpellMgr->GetSpellInfo(spell_id);
        if (!SpellInfo)
            continue;
        std::string sRank = SpellInfo->Rank[sWorld->GetDefaultDbcLocale()];
        if(sRank.empty())
            continue;
        
        //exception to polymorph spells-make pig and turtle other chain than sheep
        if ((SpellInfo->SpellFamilyName==SPELLFAMILY_MAGE) && (SpellInfo->SpellFamilyFlags & 0x1000000) && (SpellInfo->SpellIconID!=82))
            continue;
            
        switch (SpellInfo->Id) 
        {
            case 13819:
            case 34769:
            case 25046:
            case 28730:
                continue;
            default:
                break;
        }

        SpellRankEntry entry;
        SpellRankValue value;
        entry.SkillId=AbilityInfo->skillId;
        entry.SpellName=SpellInfo->SpellName[sWorld->GetDefaultDbcLocale()];
        entry.DurationIndex= SpellInfo->DurationEntry ? SpellInfo->DurationEntry->ID : 0;
        entry.RangeIndex=SpellInfo->RangeEntry->ID;
        entry.ProcFlags=SpellInfo->ProcFlags;
        entry.SpellFamilyFlags=SpellInfo->SpellFamilyFlags;
        entry.TargetAuraState=SpellInfo->TargetAuraState;
        entry.SpellVisual=SpellInfo->SpellVisual;
        entry.ManaCost=SpellInfo->ManaCost;

        for (;;)
        {
            AbilityInfo = mSkillLineAbilityMap.lower_bound(spell_id)->second;
            value.Id = spell_id;
            value.Rank = SpellInfo->Rank[sWorld->GetDefaultDbcLocale()];
            RankMap.insert(std::pair<SpellRankEntry, SpellRankValue>(entry,value));
            spell_id=AbilityInfo->forward_spellid;
            SpellInfo = sSpellMgr->GetSpellInfo(spell_id);
            if (!SpellInfo)
                break;
        }
    }

    uint32 count=0;

    for (std::multimap<SpellRankEntry, SpellRankValue,SpellRankEntry>::iterator itr = RankMap.begin();itr!=RankMap.end();)
    {
        SpellRankEntry entry = itr->first;
        //trac errors in extracted data
        std::multimap<char const *, std::multimap<SpellRankEntry, SpellRankValue,SpellRankEntry>::iterator> RankErrorMap;
        for (std::multimap<SpellRankEntry, SpellRankValue,SpellRankEntry>::iterator itr2 = RankMap.lower_bound(entry);itr2!=RankMap.upper_bound(entry);itr2++)
        {
            RankErrorMap.insert(std::pair<char const *, std::multimap<SpellRankEntry, SpellRankValue,SpellRankEntry>::iterator>(itr2->second.Rank,itr2));
        }
        for (std::multimap<char const *, std::multimap<SpellRankEntry, SpellRankValue,SpellRankEntry>::iterator>::iterator itr2 = RankErrorMap.begin();itr2!=RankErrorMap.end();)
        {
            char const * err_entry=itr2->first;
            uint32 rank_count=RankErrorMap.count(itr2->first);
            if (rank_count>1)
            for (itr2 = RankErrorMap.lower_bound(err_entry);itr2!=RankErrorMap.upper_bound(err_entry);itr2++)
            {
                TC_LOG_ERROR("There is a duplicate rank entry (%s) for spell: %u",itr2->first,itr2->second->second.Id);
                TC_LOG_ERROR("sql.sql","Spell %u removed from chain data.",itr2->second->second.Id);
                RankMap.erase(itr2->second);
                itr=RankMap.lower_bound(entry);
            }
            else
                itr2++;
        }
        //do not proceed for spells with less than 2 ranks
        uint32 spell_max_rank=RankMap.count(entry);
        if (spell_max_rank<2)
        {
            itr=RankMap.upper_bound(entry);
            continue;
        }

        itr=RankMap.upper_bound(entry);

        //order spells by spells by SpellLevel
        std::list<uint32> RankedSpells;
        uint32 min_spell_lvl=0;
        std::multimap<SpellRankEntry, SpellRankValue,SpellRankEntry>::iterator min_itr;
        for (;RankMap.count(entry);)
        {
            for (std::multimap<SpellRankEntry, SpellRankValue,SpellRankEntry>::iterator itr2 = RankMap.lower_bound(entry);itr2!=RankMap.upper_bound(entry);itr2++)
            {
                SpellInfo const *SpellInfo=sSpellMgr->GetSpellInfo(itr2->second.Id);
                if (SpellInfo->SpellLevel<min_spell_lvl || itr2==RankMap.lower_bound(entry))
                {
                    min_spell_lvl=SpellInfo->SpellLevel;
                    min_itr=itr2;
                }
            }
            RankedSpells.push_back(min_itr->second.Id);
            RankMap.erase(min_itr);
        }

        //use data from talent.dbc
        uint16 talent_id=0;
        for(std::list<uint32>::iterator itr2 = RankedSpells.begin();itr2!=RankedSpells.end();)
        {
            if (TalentSpellPos const* TalentPos=GetTalentSpellPos(*itr2))
            {
                talent_id=TalentPos->talent_id;
                RankedSpells.erase(itr2);
                itr2 = RankedSpells.begin();
            }
            else
                itr2++;
        }
        if (talent_id)
        {
            TalentEntry const *TalentInfo = sTalentStore.LookupEntry(talent_id);
            for (uint8 rank=5;rank;rank--)
            {
                if (TalentInfo->RankID[rank-1])
                    RankedSpells.push_front(TalentInfo->RankID[rank-1]);
            }
        }

        count++;

        itr=RankMap.upper_bound(entry);
        uint32 spell_rank=1;
        for(std::list<uint32>::iterator itr2 = RankedSpells.begin();itr2!=RankedSpells.end();spell_rank++)
        {
            uint32 spell_id=*itr2;
            mSpellChains[spell_id].rank=spell_rank;
            mSpellChains[spell_id].first=RankedSpells.front();
            mSpellChains[spell_id].last=RankedSpells.back();

            itr2++;
            if (spell_rank<2)
                mSpellChains[spell_id].prev=0;

            if (spell_id==RankedSpells.back())
                mSpellChains[spell_id].next=0;
            else
            {
                if ((*itr2) == 14326) {
                    mSpellChains[*itr2].prev = 1513;
                    mSpellChains[1513].next=*itr2;
                }
                else {
                    mSpellChains[*itr2].prev=spell_id;
                    mSpellChains[spell_id].next=*itr2;
                }
            }
        }
    }

//uncomment these two lines to print yourself list of spell_chains on startup
//    for (std::unordered_map<uint32, SpellChainNode>::iterator itr=mSpellChains.begin();itr!=mSpellChains.end();itr++)
//       TC_LOG_INFO( "Id: %u, Rank: %d , %s",itr->first,itr->second.rank, sSpellMgr->GetSpellInfo(itr->first)->Rank[sWorld->GetDefaultDbcLocale()]);

    TC_LOG_INFO("server.loading", ">> Loaded %u spell chains",count);
}

void SpellMgr::LoadSpellLearnSkills()
{
    mSpellLearnSkills.clear();                              // need for reload case

    // search auto-learned skills and add its to map also for use in unlearn spells/talents
    uint32 dbc_count = 0;
    //for(uint32 spell = 0; spell < sSpellStore.GetNumRows(); ++spell)
    for (auto itr = sObjectMgr->GetSpellStore().begin(); itr != sObjectMgr->GetSpellStore().end(); itr++)
    {
        uint32 spellId = itr->first;
        SpellInfo const* entry = sSpellMgr->GetSpellInfo(spellId);

        if(!entry)
            continue;

        for(int i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if(entry->Effects[i].Effect==SPELL_EFFECT_SKILL)
            {
                SpellLearnSkillNode dbc_node;
                dbc_node.skill    = entry->Effects[i].MiscValue;
                if ( dbc_node.skill != SKILL_RIDING )
                    dbc_node.value    = 1;
                else
                    dbc_node.value    = (entry->Effects[i].BasePoints+1)*75;
                dbc_node.maxvalue = (entry->Effects[i].BasePoints+1)*75;
                
                mSpellLearnSkills[spellId] = dbc_node;
                ++dbc_count;
                break;
            }
        }
    }

    TC_LOG_INFO("server.loading", ">> Loaded %u Spell Learn Skills from DBC", dbc_count );
}

void SpellMgr::LoadSpellLearnSpells()
{
    mSpellLearnSpells.clear();                              // need for reload case

    QueryResult result = WorldDatabase.Query("SELECT entry, SpellID FROM spell_learn_spell");
    if(!result)
    {
        TC_LOG_INFO("server.loading", ">> Loaded 0 spell learn spells" );
        TC_LOG_ERROR("server.loading","`spell_learn_spell` table is empty!");
        return;
    }

    uint32 count = 0;

    do
    {
        Field *fields = result->Fetch();

        uint32 spell_id    = fields[0].GetUInt32();

        SpellLearnSpellNode node;
        node.spell      = fields[1].GetUInt32();
        node.autoLearned= false;

        if(!sSpellMgr->GetSpellInfo(spell_id))
        {
            TC_LOG_ERROR("sql.sql","Spell %u listed in `spell_learn_spell` does not exist",spell_id);
            continue;
        }

        if(!sSpellMgr->GetSpellInfo(node.spell))
        {
            TC_LOG_ERROR("sql.sql","Spell %u listed in `spell_learn_spell` does not exist",node.spell);
            continue;
        }

        mSpellLearnSpells.insert(SpellLearnSpellMap::value_type(spell_id,node));

        ++count;
    } while( result->NextRow() );

    // search auto-learned spells and add its to map also for use in unlearn spells/talents
    uint32 dbc_count = 0;
    //for(uint32 spell = 0; spell < sSpellStore.GetNumRows(); ++spell)
    for (auto itr = sObjectMgr->GetSpellStore().begin(); itr != sObjectMgr->GetSpellStore().end(); itr++)
    {
        uint32 spellId = itr->first;
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
        if(!spellInfo)
            continue;

        for(int i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if(spellInfo->Effects[i].Effect ==SPELL_EFFECT_LEARN_SPELL)
            {
                SpellLearnSpellNode dbc_node;
                dbc_node.spell       = spellInfo->Effects[i].TriggerSpell;
                dbc_node.autoLearned = true;

                SpellLearnSpellMap::const_iterator db_node_begin = GetBeginSpellLearnSpell(spellId);
                SpellLearnSpellMap::const_iterator db_node_end   = GetEndSpellLearnSpell(spellId);

                bool found = false;
                for(SpellLearnSpellMap::const_iterator itr = db_node_begin; itr != db_node_end; ++itr)
                {
                    if(itr->second.spell == dbc_node.spell)
                    {
                        TC_LOG_ERROR("sql.sql","Spell %u auto-learn spell %u in spell.dbc then the record in `spell_learn_spell` is redundant, please fix DB.",
                            spellId,dbc_node.spell);
                        found = true;
                        break;
                    }
                }

                if(!found)                                  // add new spell-spell pair if not found
                {
                    mSpellLearnSpells.insert(SpellLearnSpellMap::value_type(spellId,dbc_node));
                    ++dbc_count;
                }
            }
        }
    }

    TC_LOG_INFO("server.loading", ">> Loaded %u spell learn spells + %u found in DBC", count, dbc_count );
}

void SpellMgr::LoadSpellScriptTarget()
{
    mSpellScriptTarget.clear();                             // need for reload case

    uint32 count = 0;

    QueryResult result = WorldDatabase.Query("SELECT entry,type,targetEntry FROM spell_script_target");

    if(!result)
    {
        TC_LOG_INFO("server.loading", ">> Loaded 0 spell script target" );
        TC_LOG_ERROR("server.loading","`spell_script_target` table is empty!");
        return;
    }

    do
    {
        Field *fields = result->Fetch();

        uint32 spellId     = fields[0].GetUInt32();
        uint32 type        = fields[1].GetUInt8();
        uint32 targetEntry = fields[2].GetUInt32();

        SpellInfo const* spellProto = sSpellMgr->GetSpellInfo(spellId);

        if(!spellProto)
        {
            TC_LOG_ERROR("FIXME","Table `spell_script_target`: spellId %u listed for TargetEntry %u does not exist.",spellId,targetEntry);
            continue;
        }

        /*bool targetfound = false;
        for(int i = 0; i <3; ++i)
        {
            if( spellProto->Effects[i].TargetA==TARGET_UNIT_NEARBY_ENTRY ||
                spellProto->Effects[i].TargetB==TARGET_UNIT_NEARBY_ENTRY ||
                spellProto->Effects[i].TargetA==TARGET_DEST_NEARBY_ENTRY ||
                spellProto->Effects[i].TargetB==TARGET_DEST_NEARBY_ENTRY )
            {
                targetfound = true;
                break;
            }
        }
        if(!targetfound)
        {
            TC_LOG_ERROR("FIXME","Table `spell_script_target`: spellId %u listed for TargetEntry %u does not have any implicit target TARGET_UNIT_NEARBY_ENTRY(38) or TARGET_DEST_NEARBY_ENTRY (46).",spellId,targetEntry);
            continue;
        }*/

        if( type >= MAX_SPELL_TARGET_TYPE )
        {
            TC_LOG_ERROR("FIXME","Table `spell_script_target`: target type %u for TargetEntry %u is incorrect.",type,targetEntry);
            continue;
        }

        switch(type)
        {
            case SPELL_TARGET_TYPE_GAMEOBJECT:
            {
                if( targetEntry==0 )
                    break;

                if (!sObjectMgr->GetGameObjectTemplate(targetEntry))
                {
                    TC_LOG_ERROR("sql.sql","Table `spell_script_target`: gameobject template entry %u does not exist.",targetEntry);
                    continue;
                }
                break;
            }
            default:
            {
                //players
                /*if( targetEntry==0 )
                {
                    TC_LOG_ERROR("sql.sql","Table `spell_script_target`: target entry == 0 for not GO target type (%u).",type);
                    continue;
                }*/
                if(targetEntry && !sObjectMgr->GetCreatureTemplate(targetEntry))
                {
                    TC_LOG_ERROR("sql.sql","Table `spell_script_target`: creature template entry %u does not exist.",targetEntry);
                    continue;
                }
                const CreatureTemplate* cInfo = sObjectMgr->GetCreatureTemplate(targetEntry);

                if(spellId == 30427 && !cInfo->SkinLootId)
                {
                    TC_LOG_ERROR("sql.sql","Table `spell_script_target` has creature %u as a target of spellid 30427, but this creature has no skinlootid. Gas extraction will not work!", cInfo->Entry);
                    continue;
                }
                break;
            }
        }

        mSpellScriptTarget.insert(SpellScriptTarget::value_type(spellId,SpellTargetEntry(SpellScriptTargetType(type),targetEntry)));

        ++count;
    } while (result->NextRow());

    // Check all spells
    /* Disabled (lot errors at this moment)
    for(uint32 i = 1; i < sSpellStore.nCount; ++i)
    {
        SpellInfo const * spellInfo = sSpellMgr->GetSpellInfo(i);
        if(!spellInfo)
            continue;

        bool found = false;
        for(int j=0; j<3; ++j)
        {
            if( spellInfo->Effects[j].TargetA.GetTarget()== TARGET_UNIT_NEARBY_ENTRY || spellInfo->Effects[j].TargetA.GetTarget()!= TARGET_UNIT_CASTER && spellInfo->Effects[j].TargetB->GetTarget() == TARGET_UNIT_NEARBY_ENTRY )
            {
                SpellScriptTarget::const_iterator lower = sSpellMgr->GetBeginSpellScriptTarget(spellInfo->Id);
                SpellScriptTarget::const_iterator upper = sSpellMgr->GetEndSpellScriptTarget(spellInfo->Id);
                if(lower==upper)
                {
                    TC_LOG_ERROR("sql.sql","Spell (ID: %u) has effect EffectImplicitTargetA/EffectImplicitTargetB = %u (TARGET_UNIT_NEARBY_ENTRY), but does not have record in `spell_script_target`",spellInfo->Id,TARGET_UNIT_NEARBY_ENTRY);
                    break;                                  // effects of spell
                }
            }
        }
    }
    */
    
    TC_LOG_INFO("server.loading",">> Loaded %u Spell Script Targets", count);
}

void SpellMgr::LoadSpellPetAuras()
{
    mSpellPetAuraMap.clear();                                  // need for reload case

    uint32 count = 0;

    //                                                0      1    2
    QueryResult result = WorldDatabase.Query("SELECT spell, pet, aura FROM spell_pet_auras");
    if( !result )
    {
        TC_LOG_INFO("server.loading", ">> Loaded %u spell pet auras", count );
        return;
    }

    do
    {
        Field *fields = result->Fetch();

        uint16 spell = fields[0].GetUInt32();
        uint16 pet = fields[1].GetUInt32();
        uint16 aura = fields[2].GetUInt32();

        SpellPetAuraMap::iterator itr = mSpellPetAuraMap.find(spell);
        if(itr != mSpellPetAuraMap.end())
        {
            itr->second.AddAura(pet, aura);
        }
        else
        {
            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spell);
            if (!spellInfo)
            {
                TC_LOG_ERROR("sql.sql","Spell %u listed in `spell_pet_auras` does not exist", spell);
                continue;
            }
            int i = 0;
            for(; i < MAX_SPELL_EFFECTS; ++i)
                if((spellInfo->Effects[i].Effect == SPELL_EFFECT_APPLY_AURA &&
                    spellInfo->Effects[i].ApplyAuraName == SPELL_AURA_DUMMY) ||
                    spellInfo->Effects[i].Effect == SPELL_EFFECT_DUMMY)
                    break;

            if(i == 3)
            {
                TC_LOG_ERROR("sql.sql","Spell %u listed in `spell_pet_auras` does not have dummy aura or dummy effect", spell);
                continue;
            }

            SpellInfo const* spellInfo2 = sSpellMgr->GetSpellInfo(aura);
            if (!spellInfo2)
            {
                TC_LOG_ERROR("sql.sql","Aura %u listed in `spell_pet_auras` does not exist", aura);
                continue;
            }

            PetAura pa(pet, aura, spellInfo->Effects[i].TargetA.GetTarget() == TARGET_UNIT_PET, spellInfo->Effects[i].BasePoints + spellInfo->Effects[i].BaseDice);
            mSpellPetAuraMap[spell] = pa;
        }

        ++count;
    } while( result->NextRow() );

    TC_LOG_INFO("server.loading", ">> Loaded %u spell pet auras", count );
}

void SpellMgr::OverrideSpellItemEnchantment()
{
    SpellItemEnchantmentEntry *pEnchant;
    for (uint32 i = 0; i < sSpellItemEnchantmentStore.GetNumRows(); ++i) {
        pEnchant = (SpellItemEnchantmentEntry*)sSpellItemEnchantmentStore.LookupEntry(i);
        if (!pEnchant)
            continue;
            
        switch (i) {
        case 3265:
            pEnchant->type[0] = ITEM_ENCHANTMENT_TYPE_COMBAT_SPELL;
            pEnchant->spellid[0] = 45403;
            break;
        case 3266:
            pEnchant->type[0] = ITEM_ENCHANTMENT_TYPE_COMBAT_SPELL;
            pEnchant->spellid[0] = 45401;
            break;
        default:
            //TC_LOG_ERROR("FIXME","Processing enchantment %u", i);
            break;
        }
    }
}

// set data in core for now
void SpellMgr::LoadSpellCustomAttr()
{
    SpellEntry* spellInfo;
    for (auto itr = sObjectMgr->GetSpellStore().begin(); itr != sObjectMgr->GetSpellStore().end(); itr++)
    {
        uint32 i = itr->first;
        spellInfo = itr->second;
        if(!spellInfo)
            continue;

        for(uint32 j = 0; j < 3; ++j)
        {
            switch(spellInfo->Effect[j])
            {
                case SPELL_EFFECT_CHARGE:
                    if(!spellInfo->speed && !spellInfo->SpellFamilyName)
                        spellInfo->speed = SPEED_CHARGE;
                    break;
                case SPELL_EFFECT_TRIGGER_SPELL:
                    if (IsPositionTarget(spellInfo->EffectImplicitTargetA[j]) ||
                        spellInfo->Targets & (TARGET_FLAG_SOURCE_LOCATION|TARGET_FLAG_DEST_LOCATION))
                        spellInfo->Effect[j] = SPELL_EFFECT_TRIGGER_MISSILE;
                    break;
            }
        }

        if (spellInfo->Dispel == DISPEL_POISON)
            spellInfo->AttributesEx |= SPELL_ATTR1_CANT_BE_REDIRECTED;
            
        if (spellInfo->SpellIconID == 104 && spellInfo->AttributesEx == 0x4044) //First Aid
            spellInfo->InterruptFlags |= SPELL_INTERRUPT_FLAG_MOVEMENT;

        if (   (spellInfo->SpellIconID == 267 && spellInfo->SpellFamilyName == 9)  //Hunter: Mend pet
            || (spellInfo->SpellIconID == 534) // Hunter: Heal pet
            || (spellInfo->SpellIconID == 1874 && spellInfo->SpellFamilyName == 6) //holy nova
            || (spellInfo->SpellVisual == 367 && spellInfo->SpellIconID == 338)  // Mana Spring Totem
           ) 
            spellInfo->AttributesEx |= SPELL_ATTR1_NO_THREAT;

        /* X */
        /* This code explicitly sets bleed effect mechanic of the direct damage effect of certain physical spells. MECHANIC_BLEED in the overall SpellInfo.Mechanic 
           is now ignored for direct damage effects since it is more often than not ignored by the official blizz servers. Because DD bleed is so unusual, this arrangement 
           has been made required by the bleed damage increase code. Also the ignore armor flag must be explicitly set here for any DD bleed spells.
        */
        switch (i)
        {
        case 41913: //Illidan SPELL_SHADOWFIEND_PASSIVE
            spellInfo->EffectApplyAuraName[0] = 4; // proc debuff, and summon infinite fiends
            break;
        case 37770:
            spellInfo->Effect[0] = 2;
            break;
        case 31344:  //SPELL_HOWL_OF_AZGALOR
            spellInfo->EffectRadiusIndex[0] = 12;//100yards instead of 50000?!
            break;
        case 43734: //jan'alai SPELL_HATCH_EGG
            spellInfo->EffectImplicitTargetA[0] = 1;
            spellInfo->EffectImplicitTargetB[0] = 0;
            break;
        case 31298: //anatheron SPELL_SLEEP
            spellInfo->EffectImplicitTargetA[0] = 1;
            spellInfo->EffectImplicitTargetB[0] = 0;
            break;
        case 44869: //kalecgos SPELL_SPECTRAL_BLAST
            spellInfo->EffectImplicitTargetB[0] = TARGET_UNIT_TARGET_ENEMY;
            break;
        case 30541: //Maghteridon SPELL_BLAZE_TARGET
            spellInfo->EffectImplicitTargetA[0] = 6; // target 7, random target with certain entry spell, need core fix
            spellInfo->EffectImplicitTargetB[0] = 0;
            break;
        case 30657: //maghteridon SPELL_QUAKE_TRIGGER
            spellInfo->EffectTriggerSpell[0] = 30571; // SPELL_QUAKE_KNOCKBACK
            break;
        case 42821: //Headless horseman SPELL_WISP_BLUE
        case 42818: //Headless horseman SPELL_WISP_FLIGHT_PORT
            //that's hack but there are no info about range of this spells in dbc
            spellInfo->rangeIndex = 6; //100 yards
            break;
        case 42380: //Headless horseman SPELL_CONFLAGRATION
            spellInfo->EffectApplyAuraName[0] = SPELL_AURA_PERIODIC_DAMAGE_PERCENT;
            spellInfo->EffectBasePoints[0] = 10;
            spellInfo->EffectBaseDice[0] = 10;
            spellInfo->EffectDamageMultiplier[0] = 1;
            break;
        case 1822: // Rank 1 to 5 Rake (Druid)
        case 1823:
        case 1824:
        case 9904:
        case 27003:
            spellInfo->EffectMechanic[0] = MECHANIC_BLEED;
            break;
        case 31041: // Roar's Mangle
            spellInfo->EffectMechanic[0] = MECHANIC_BLEED;
            break;
        case 33745: // Rank 1 Lacerate (Druid)
            spellInfo->EffectMechanic[1] = MECHANIC_BLEED;
            break;
        default:
            break;
        }
        /* /X */

        switch(i)
        {
        case 45150:                             // Meteor Slash
            spellInfo->AttributesEx3 |= SPELL_ATTR3_PLAYERS_ONLY;
            break;
        case 44978: case 45001: case 45002:     // Wild Magic
        case 45004: case 45006: case 45010:     // Wild Magic
        case 31347: // Doom
        case 41635: // Prayer of Mending
        case 44869: // Spectral Blast
        case 45027: // Revitalize
        case 45976: // Muru Portal Channel
        case 39365: // Thundering Storm
        case 41071: // Raise Dead
        case 40834: // Flammes déchirantes
        case 39090: // Positive charge damage
        case 39093: // Negative charge damage
        case 45032: // Curse of Boundless Agony - cast by the boss
        case 45034: // Curse of Boundless Agony - bounced off players
            spellInfo->MaxAffectedTargets = 1;
            break;
        case 45026: // Heroic Strike
            spellInfo->EffectBasePoints[0] = 5200;
            break;
        case 41357: // L1 Arcane Charge
            spellInfo->MaxAffectedTargets = 3;
            break;
        case 41376: // Spite
        case 39992: // Needle Spine
        case 29576: //Multi-Shot
        case 40816: //Saber Lash
        case 37790: //Spread Shot
        //case 46771: //Flame Sear
        //case 45248: //Shadow Blades
        case 41303: // Soul Drain
            spellInfo->MaxAffectedTargets = 3;
            break;
        case 38310: //Multi-Shot
            spellInfo->MaxAffectedTargets = 4;
            break;
        case 46008: //Negative Energy
            spellInfo->AttributesEx4 |= SPELL_ATTR4_IGNORE_RESISTANCES;
            // no break
        case 42005: // Bloodboil
        case 38296: //Spitfire Totem
        case 37676: //Insidious Whisper
        case 45641: //Fire Bloom
            spellInfo->DmgClass = SPELL_DAMAGE_CLASS_MAGIC;
            spellInfo->MaxAffectedTargets = 5;
            break;
        case 40243: //Crushing Shadows
            spellInfo->MaxAffectedTargets = 5;
            break;
        case 46292: // Cataclysm Breath
        case 46480: // Fel Lightning
            spellInfo->MaxAffectedTargets = 8;
            break;
        case 40827: //Sinful Beam
        case 40859: //Sinister Beam
        case 40860: //Vile Beam
        case 40861: //Wicked Beam
            spellInfo->MaxAffectedTargets = 10;
            break;
        case 8122: case 8124: case 10888: case 10890: // Psychic Scream
        case 12494: // Frostbite
            spellInfo->Attributes |= SPELL_ATTR0_HEARTBEAT_RESIST_CHECK;
            break;
        case 38794: case 33711: //Murmur's Touch
            spellInfo->MaxAffectedTargets = 1;
            spellInfo->EffectTriggerSpell[0] = 33760;
            break;
        case 24905: // Moonkin form -> elune's touch
            spellInfo->EffectImplicitTargetA[2] = TARGET_UNIT_CASTER;
        break;
        case 32727: // Arena Preparation - remove invisibility aura
        case 44949: // Whirlwind's offhand attack - 50% weapon damage effect must be removed as offhand damage halving is done independently and automatically.
            spellInfo->Effect[1] = 0;
            break;
        case 39844: // Skyguard Blasting Charge - change target until correctly implemented in core
            spellInfo->EffectImplicitTargetA[0] = TARGET_UNIT_SRC_AREA_ENEMY;
            break;
        case 12723: // Sweeping Strikes proc
            spellInfo->Attributes |= SPELL_ATTR0_IMPOSSIBLE_DODGE_PARRY_BLOCK;
            break;
        // cleansing totem pulse when it is spawned
        case 8172:
            spellInfo->AttributesEx5 |= SPELL_ATTR5_START_PERIODIC_AT_APPLY;
            break;
        case 30421:     // Nether Portal - Perseverence
            spellInfo->EffectBasePoints[2] += 30000;
            break;
        // Mark of Death (Doomwalker)
        case 37128:
            spellInfo->Attributes |= SPELL_ATTR0_CANT_CANCEL;
            spellInfo->Attributes |= SPELL_ATTR0_CASTABLE_WHILE_DEAD;
            spellInfo->AttributesEx |= SPELL_ATTR1_CANT_BE_REFLECTED;
            spellInfo->AttributesEx3 |= SPELL_ATTR3_DEATH_PERSISTENT;
            break;
        // Improved Shadow Bolt
        case 17800:
            spellInfo->procCharges = 5;     // Core handles that in a wrong way: it looks like the bolt that 
            break;                          // triggers the talent consumes the first charge, then put 5 instead of 4.
        // Should be parried/blocked/dodged
        case 41032:
            spellInfo->Attributes &= ~SPELL_ATTR0_UNAFFECTED_BY_INVULNERABILITY;
            spellInfo->AttributesEx3 |= SPELL_ATTR3_IGNORE_HIT_RESULT;
            break;
        case 41068: // Blood Siphon deals a lot more damage - 3x
            spellInfo->EffectValueMultiplier[0] = 1;
            break;
        case 6358: // Sedduction - remove immunity ignoring attributes
            spellInfo->Attributes &= ~SPELL_ATTR1_UNAFFECTED_BY_SCHOOL_IMMUNE;
            break;
        case 27066:
            spellInfo->Dispel = 0;
            break;
        case 40611: // Blaze Effect
            spellInfo->EffectRadiusIndex[0] = 7;
            break;
        case 40030: // Demon Fire
            spellInfo->EffectRadiusIndex[0] = 7;
            break;
        case 40327: // Atrophy
            spellInfo->Attributes |= SPELL_ATTR0_UNAFFECTED_BY_INVULNERABILITY;
            break;
        case 40322: // Spirit Shield
            spellInfo->AttributesEx &= ~SPELL_ATTR1_CANT_BE_REFLECTED;
            break;
        case 33666:     // Sonic Boom (Murmur)
        case 38795:
            spellInfo->EffectRadiusIndex[0] = 13;
            spellInfo->EffectRadiusIndex[1] = 13;
            spellInfo->EffectRadiusIndex[2] = 13;
            break;
        case 603:       // Curse of Doom
        case 30910:
            spellInfo->SpellFamilyName = SPELLFAMILY_WARLOCK;
            break;
        case 48025:
            spellInfo->Attributes |= SPELL_ATTR0_CANT_USED_IN_COMBAT;
            spellInfo->EffectApplyAuraName[0] = SPELL_AURA_MOUNTED;
            break;
        case 20184:
            spellInfo->Mechanic = MECHANIC_SNARE;
            break;
        case 23575:
        case 33737:
            spellInfo->SpellFamilyFlags = 1056;
            break;
        case 38157:
            spellInfo->AreaId = 3522;
            break;
        case 46411:
            spellInfo->DurationIndex = 21;
            break;
        case 38429:
            spellInfo->SpellFamilyFlags = 2416967680;
            break;
        case 38397:
            spellInfo->SpellFamilyFlags = 1574945;
            break;
        case 37384:
            spellInfo->SpellFamilyFlags = 6;
            break;
        case 38393:
            spellInfo->SpellFamilyFlags = 429496729601LL;
            break;
        case 32747:
            spellInfo->InterruptFlags = 8;
            spellInfo->PreventionType = SPELL_PREVENTION_TYPE_SILENCE;
            break;
        case 26888:
            spellInfo->EffectBasePoints[1] = 740;
            break;
        case 45662:
            spellInfo->DmgClass = SPELL_DAMAGE_CLASS_MAGIC;
            spellInfo->AttributesEx2 |= SPELL_ATTR2_CANT_CRIT;
            spellInfo->AttributesEx3 |= SPELL_ATTR3_NO_DONE_BONUS;
            spellInfo->AttributesEx3 |= SPELL_ATTR3_IGNORE_HIT_RESULT;
            spellInfo->AttributesEx4 |= SPELL_ATTR4_IGNORE_RESISTANCES;
            break;
        case 46394:
        case 45661:
        case 45665:
            spellInfo->DmgClass = SPELL_DAMAGE_CLASS_MAGIC;
            spellInfo->AttributesEx4 |= SPELL_ATTR4_IGNORE_RESISTANCES;
            break;
        case 45401:
            spellInfo->procChance = 15;
            break;
        case 46562:
            spellInfo->MaxAffectedTargets = 5;
            break;
        // Eredar Twins spells (Sunwell)
        //case 45248:
        //case 46771:
        case 45236:
            spellInfo->EffectImplicitTargetA[0] = TARGET_DEST_TARGET_ENEMY;
            spellInfo->AttributesEx4 |= SPELL_ATTR4_IGNORE_RESISTANCES;
            break;
        case 45230:
        case 45235:
        case 45246:
            spellInfo->AttributesEx4 |= SPELL_ATTR4_IGNORE_RESISTANCES;
            // no break
        case 45342: // Alythess Conflagration
            spellInfo->AttributesEx4 |= SPELL_ATTR4_IGNORE_RESISTANCES;
            // no break
        case 45348: // Alythess SPELL_FLAME_TOUCHED
            spellInfo->AttributesEx4 |= SPELL_ATTR4_IGNORE_RESISTANCES;
            // no break
        case 45347: // Sacrolash SPELL_DARK_TOUCHED
            spellInfo->DmgClass = SPELL_DAMAGE_CLASS_MAGIC;
            spellInfo->AttributesEx3 |= SPELL_ATTR3_STACK_FOR_DIFF_CASTERS;
            break;
        case 46771: // SPELL_FLAME_SEAR
            spellInfo->MaxAffectedTargets = 5;
            spellInfo->AttributesEx4 |= SPELL_ATTR4_IGNORE_RESISTANCES;
            spellInfo->AttributesEx3 |= SPELL_ATTR3_PLAYERS_ONLY;
            break;
        case 45111:
            spellInfo->DurationIndex = 9;       // 30 sec
            break;
        case 15407:
        case 17311:
        case 17312:
        case 17313:
        case 17314:
        case 18807:
        case 25387:
            spellInfo->AttributesEx3 |= SPELL_ATTR3_STACK_FOR_DIFF_CASTERS;
            break;
        case 40238:
            spellInfo->InterruptFlags = 0;
            break;
        // Simon game auras at the beginning, limit it to 5 sec
        case 40281: // Blue
        case 40287: // Green
        case 40288: // Red
        case 40289: // Yellow
            spellInfo->DurationIndex = 28;
            break;
        case 40160:
        case 33655:
            spellInfo->EffectImplicitTargetA[0] = 0;
            break;
        case 2825:
        case 32182:
        case 19574:
        case 31944:
            spellInfo->AttributesEx2 |= SPELL_ATTR2_CAN_TARGET_NOT_IN_LOS;
            break;
        case 44335:
            spellInfo->DmgClass = SPELL_DAMAGE_CLASS_MAGIC;
            break;
        case 13261:
            spellInfo->EffectImplicitTargetA[0] = TARGET_UNIT_TARGET_ENEMY;
            break;
        case 30346:
            spellInfo->Effect[1] = 0;
            spellInfo->EffectBasePoints[1] = 0;
            break;
        case 40869:
            spellInfo->Effect[1] = 0;
            break;
        case 5720:
        case 5723:
        case 6263:
        case 6262:
        case 11732:
        case 23468:
        case 23469:
        case 23470:
        case 23471:
        case 23472:
        case 23473:
        case 23474:
        case 23475:
        case 23476:
        case 23477:
        case 27235:
        case 27236:
        case 27237:
            spellInfo->StartRecoveryCategory = 42;
            spellInfo->StartRecoveryTime = 150;
            break;
        case 45399:
            spellInfo->rangeIndex = 2;
            break;
        case 43383:
            spellInfo->ChannelInterruptFlags = 0;
            break;
        case 28730:
            spellInfo->EffectImplicitTargetA[1] = 1;
            //spellInfo->EffectImplicitTargetB[1] = 22;
            break;
        case 7870:
            spellInfo->Attributes |= SPELL_ATTR0_CANT_USED_IN_COMBAT;
            break;
        case 16979: // Feral charge
        case 45334: // Feral charge effect (root)
        case 100:   // Warrior charge BEGIN
        case 6178:
        case 11578:
        case 20508:
        case 22120:
        case 22911:
        case 24023:
        case 24193:
        case 24315:
        case 24408:
        case 25821:
        case 29320:
        case 29847:
        case 31733:
        case 32323:
        case 33709:     // No, SpellFamilyFlags is not used only for charge...
        case 35412:
        case 35570:
        case 35754:
        case 36058:
        case 36140:
        case 36509:
        case 37511:
        case 38461:
        case 39574:
        case 40602:
        case 43519:
        case 50874: // Warrior charge END
        case 20252: // Warrior intercept BEGIN
        case 20616:
        case 20617:
        case 25272:
        case 25275:
        case 27577:
        case 27826: // Warrior intercept END
            spellInfo->AttributesEx |= SPELL_ATTR1_CANT_BE_REDIRECTED;
            break;
        case 41360:
            spellInfo->AttributesEx2 |= SPELL_ATTR2_CANT_CRIT;
            break;
        case 5530:
            spellInfo->Effect[1] = 0;
            break;
        case 18694:
        case 18695:
        case 18696:
            spellInfo->EffectItemType[0] = 0;
            break;
        case 25713:
        case 25735:
        case 25736:
        case 25737:
        case 25738:
        case 25739:
        case 25740:
        case 25741:
        case 25742:
        case 27156:
            spellInfo->AttributesEx2 &= ~SPELL_ATTR2_CANT_CRIT;
            break;
        case 17116:
            spellInfo->AttributesEx4 &= ~SPELL_ATTR4_NOT_STEALABLE;
            break;
        case 20577:
        case 20578:
        case 31537:
        case 31538:
            spellInfo->InterruptFlags |= SPELL_INTERRUPT_FLAG_MOVEMENT;
            break;
        case 46458:
            spellInfo->StackAmount = 10;
            spellInfo->EffectApplyAuraName[0] = 23;
            spellInfo->EffectAmplitude[0] = 3000;
            spellInfo->EffectTriggerSpell[0] = 46576;
            break;
        case 40851: //Disgruntled
            spellInfo->MaxAffectedTargets = 1;
            spellInfo->EffectImplicitTargetB[0] = TARGET_UNIT_TARGET_ENEMY;
            spellInfo->EffectImplicitTargetB[1] = TARGET_UNIT_TARGET_ENEMY;
            spellInfo->EffectImplicitTargetB[2] = TARGET_UNIT_TARGET_ENEMY;
            break;
        case 40845: //Fury
            spellInfo->EffectImplicitTargetA[0] = TARGET_UNIT_TARGET_ALLY;
            spellInfo->EffectImplicitTargetA[1] = TARGET_UNIT_TARGET_ALLY;
            spellInfo->EffectImplicitTargetA[2] = TARGET_UNIT_TARGET_ALLY;
            break;
        case 39578:
            spellInfo->EffectImplicitTargetB[0] = TARGET_UNIT_TARGET_ALLY;
            break;
        case 30009:
            spellInfo->EffectImplicitTargetA[0] = TARGET_UNIT_TARGET_ALLY;
            break;
        case 33824:
            spellInfo->Effect[2] = 0;
            break;
        case 19583:
        case 19584:
        case 19585:
        case 19586:
        case 19587:
            spellInfo->EffectImplicitTargetB[1] = TARGET_UNIT_PET;
            break;
        case 24869:
            spellInfo->Effect[1] = SPELL_EFFECT_APPLY_AURA;
            spellInfo->EffectApplyAuraName[1] = SPELL_AURA_PERIODIC_TRIGGER_SPELL;
            spellInfo->EffectAmplitude[1] = 10000;
            spellInfo->EffectTriggerSpell[1] = 24870;
            spellInfo->EffectImplicitTargetA[1] = TARGET_UNIT_CASTER;
            break;
        case 5171: //Slice and Dice rank 1
        case 6774: //Slice and Dice rank 2
            spellInfo->AttributesEx3 |= SPELL_ATTR3_NO_INITIAL_AGGRO;
            //spellInfo->AttributesEx |= SPELL_ATTR1_NOT_BREAK_STEALTH; // Check if it wasn't changed later (in 3.x)
            break;
        case 29200: // Purification de la viande de sanglier infernal
            spellInfo->EffectImplicitTargetA[0] = TARGET_UNIT_CASTER;
            break;
        case 35460: // Fureur des anciens crapuches
            spellInfo->EffectImplicitTargetA[1] = TARGET_TYPE_UNIT_TARGET;
            break;
        case 20625:
            spellInfo->EffectImplicitTargetA[0] = TARGET_UNIT_CASTER_AREA_PARTY;
            break;
        case 1978:
        case 13549:
        case 13550:
        case 13551:
        case 13552:
        case 13553:
        case 13554:
        case 13555:
        case 24467:
        case 25295:
        case 27016:
            spellInfo->DmgClass = SPELL_DAMAGE_CLASS_MAGIC;
            break;
        case 41089:
        case 41091:
            spellInfo->Attributes |= SPELL_ATTR0_IMPOSSIBLE_DODGE_PARRY_BLOCK;
            spellInfo->AttributesEx3 |= SPELL_ATTR3_IGNORE_HIT_RESULT;
            break;
        case 32911:
            spellInfo->EffectTriggerSpell[0] = 32910;
            break;
        case 45248:
            spellInfo->Attributes |= SPELL_ATTR0_RANGED;
            spellInfo->MaxAffectedTargets = 25;
            break;
        case 11094:
            spellInfo->Effect[1] = SPELL_EFFECT_APPLY_AURA;
            spellInfo->EffectApplyAuraName[1] = SPELL_AURA_ADD_PCT_MODIFIER;
            spellInfo->EffectBasePoints[1] = 9;
            spellInfo->EffectImplicitTargetA[1] = 1;
            spellInfo->EffectMiscValue[1] = 8;
            break;
        case 13043:
            spellInfo->Effect[1] = SPELL_EFFECT_APPLY_AURA;
            spellInfo->EffectApplyAuraName[1] = SPELL_AURA_ADD_PCT_MODIFIER;
            spellInfo->EffectBasePoints[1] = 19;
            spellInfo->EffectImplicitTargetA[1] = 1;
            spellInfo->EffectMiscValue[1] = 8;
            break;
        case 42463:
            spellInfo->AttributesEx2 |= SPELL_ATTR2_CANT_CRIT;
            break;
        case 42339:
            spellInfo->EffectImplicitTargetB[0] = TARGET_UNIT_DEST_AREA_ALLY;
            break;
        case 42079:
            spellInfo->rangeIndex = 6;
            spellInfo->Effect[0] = SPELL_EFFECT_DUMMY;
            break;
        case 42667:
        case 42668:
        case 42680:
        case 42683:
            spellInfo->AttributesEx4 |= SPELL_ATTR4_NOT_USABLE_IN_ARENA;
            break;
        case 45996:
            spellInfo->AttributesEx3 |= SPELL_ATTR3_NO_DONE_BONUS;
            // no break
        case 46009: // M'uru and Entropius spells
        case 45999:
        case 46268:
        case 46282:
        case 46284:
        case 46230:
        case 46238:
        case 45944:
        case 46101:
        case 46102:
        case 46087:
        case 46289:
        case 45657: //Darkness of a Thousand Souls
        case 45782: // Fog corruption
        case 45714: // Fog corruption
        case 45717: // Fog corruption
        case 45726: // Fog corruption
        case 41467: //Gathios Judgement (proc other spells that can be resisted)
            spellInfo->AttributesEx4 |= SPELL_ATTR4_IGNORE_RESISTANCES;
            spellInfo->AttributesEx3 |= SPELL_ATTR3_IGNORE_HIT_RESULT;
            break;
        case 46579: //Deathfrost
        case 31024: //Living Ruby Pendant
        case 20911:
        case 20912:
        case 20913:
        case 20914:
        case 27168:
        case 25899:
        case 27169:
        case 40470: //Paladin Tier 6 Trinket
        case 40471: //trinket heal effect
        case 40472: //trinket damage effect
        case 13897: //Fiery Weapon
        case 43733: //Stormchops (item 33866)
        case 43731:
            spellInfo->AttributesEx3 |= SPELL_ATTR3_NO_DONE_BONUS;
            break;
        case 45391: // Vapor Select
            spellInfo->AttributesEx3 |= SPELL_ATTR3_IGNORE_HIT_RESULT;
            spellInfo->AttributesEx4 |= SPELL_ATTR4_IGNORE_RESISTANCES;
            spellInfo->MaxAffectedTargets = 1;
            break;
        case 45892: // Sinister reflection
            spellInfo->MaxAffectedTargets = 1;
            break;
        case 45866:
        case 45855:
        case 47002:
        case 46931:
        case 45402:
            spellInfo->AttributesEx3 |= SPELL_ATTR3_NO_DONE_BONUS;
            spellInfo->AttributesEx3 |= SPELL_ATTR3_IGNORE_HIT_RESULT;
            spellInfo->AttributesEx4 |= SPELL_ATTR4_IGNORE_RESISTANCES;
            break;
        case 45284:
        case 45286:
        case 45287:
        case 45288:
        case 45289:
        case 45290:
        case 45291:
        case 45292:
        case 45293:
        case 45294:
        case 45295:
        case 45296:
        case 45297:
        case 45298:
        case 45299:
        case 45300:
        case 45301:
        case 45302:
        case 33201: // Reflective Shield
        case 33202:
        case 33203:
        case 33204:
        case 33205:
        case 33219:
            spellInfo->AttributesEx |= SPELL_ATTR1_NO_THREAT;
            break;
        case 37027:
            spellInfo->AttributesEx2 |= SPELL_ATTR2_CAN_TARGET_NOT_IN_LOS;
            break;
        default:
            break;
        }
    }
}

void SpellMgr::LoadSpellLinked()
{
    mSpellLinkedMap.clear();    // need for reload case
    uint32 count = 0;

    //                                                0              1             2
    QueryResult result = WorldDatabase.Query("SELECT spell_trigger, spell_effect, type FROM spell_linked_spell");
    if( !result )
    {
        TC_LOG_INFO("server.loading", ">> Loaded %u linked spells", count );
        return;
    }

    do
    {
        Field *fields = result->Fetch();

        int32 trigger = fields[0].GetInt32();
        int32 effect = fields[1].GetInt32();
        int32 type = fields[2].GetInt8();

        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(abs(trigger));
        if (!spellInfo)
        {
            TC_LOG_ERROR("sql.sql","Spell %u listed in `spell_linked_spell` does not exist", abs(trigger));
            continue;
        }
        spellInfo = sSpellMgr->GetSpellInfo(abs(effect));
        if (!spellInfo)
        {
            TC_LOG_ERROR("sql.sql","Spell %u listed in `spell_linked_spell` does not exist", abs(effect));
            continue;
        }
        
        SpellInfo* triggerSpellInfo = sSpellMgr->_GetSpellInfo(abs(trigger));
        if(trigger > 0)
        {
            if(triggerSpellInfo)
                switch(type)
                {
                    case 0: triggerSpellInfo->AttributesCu |= SPELL_ATTR_CU_LINK_CAST; break;
                    case 1: triggerSpellInfo->AttributesCu |= SPELL_ATTR_CU_LINK_HIT;  break;
                    case 2: triggerSpellInfo->AttributesCu |= SPELL_ATTR_CU_LINK_AURA; break;
                }
        }
        else
        {
            if(triggerSpellInfo)
                triggerSpellInfo->AttributesCu |= SPELL_ATTR_CU_LINK_REMOVE;
        }

        if(type) //we will find a better way when more types are needed
        {
            if(trigger > 0)
                trigger += SPELL_LINKED_MAX_SPELLS * type;
            else
                trigger -= SPELL_LINKED_MAX_SPELLS * type;
        }
        mSpellLinkedMap[trigger].push_back(effect);

        ++count;
    } while( result->NextRow() );

    TC_LOG_INFO("server.loading", ">> Loaded %u linked spells", count );
}

/// Some checks for spells, to prevent adding depricated/broken spells for trainers, spell book, etc
bool SpellMgr::IsSpellValid(SpellInfo const* spellInfo, Player* pl, bool msg)
{
    // not exist
    if(!spellInfo)
        return false;

    bool need_check_reagents = false;

    // check effects
    for(int i=0; i<3; ++i)
    {
        switch(spellInfo->Effects[i].Effect)
        {
            case 0:
                continue;

                // craft spell for crafting non-existed item (break client recipes list show)
            case SPELL_EFFECT_CREATE_ITEM:
            {
                if(!sObjectMgr->GetItemTemplate( spellInfo->Effects[i].ItemType ))
                {
                    if(msg)
                    {
                        if(pl)
                            ChatHandler(pl).PSendSysMessage("Craft spell %u create not-exist in DB item (Entry: %u) and then...",spellInfo->Id,spellInfo->Effects[i].ItemType);
                        else
                            TC_LOG_ERROR("sql.sql","Craft spell %u create not-exist in DB item (Entry: %u) and then...",spellInfo->Id,spellInfo->Effects[i].ItemType);
                    }
                    return false;
                }

                need_check_reagents = true;
                break;
            }
            case SPELL_EFFECT_LEARN_SPELL:
            {
                SpellInfo const* spellInfo2 = sSpellMgr->GetSpellInfo(spellInfo->Effects[i].TriggerSpell);
                if( !IsSpellValid(spellInfo2,pl,msg) )
                {
                    if(msg)
                    {
                        if(pl)
                            ChatHandler(pl).PSendSysMessage("Spell %u learn to broken spell %u, and then...",spellInfo->Id,spellInfo->Effects[i].TriggerSpell);
                        else
                            TC_LOG_ERROR("sql.sql","Spell %u learn to invalid spell %u, and then...",spellInfo->Id,spellInfo->Effects[i].TriggerSpell);
                    }
                    return false;
                }
                break;
            }
        }
    }

    if(need_check_reagents)
    {
        for(int j = 0; j < 8; ++j)
        {
            if(spellInfo->Reagent[j] > 0 && !sObjectMgr->GetItemTemplate( spellInfo->Reagent[j] ))
            {
                if(msg)
                {
                    if(pl)
                        ChatHandler(pl).PSendSysMessage("Craft spell %u have not-exist reagent in DB item (Entry: %u) and then...",spellInfo->Id,spellInfo->Reagent[j]);
                    else
                        TC_LOG_ERROR("sql.sql","Craft spell %u have not-exist reagent in DB item (Entry: %u) and then...",spellInfo->Id,spellInfo->Reagent[j]);
                }
                return false;
            }
        }
    }

    return true;
}

bool IsSpellAllowedInLocation(SpellInfo const *spellInfo,uint32 map_id,uint32 zone_id,uint32 area_id)
{
    // normal case
    if( spellInfo->AreaId && spellInfo->AreaId != zone_id && spellInfo->AreaId != area_id )
        return false;

    MapEntry const* mapEntry = sMapStore.LookupEntry(map_id);
    if (mapEntry && mapEntry->IsRaid() && spellInfo->HasAttribute(SPELL_ATTR6_NOT_IN_RAID_INSTANCE))
        return false;

    // elixirs (all area dependent elixirs have family SPELLFAMILY_POTION, use this for speedup)
    if(spellInfo->SpellFamilyName==SPELLFAMILY_POTION)
    {
        if(uint32 mask = sSpellMgr->GetSpellElixirMask(spellInfo->Id))
        {
            if(mask & ELIXIR_BATTLE_MASK)
            {
                if(spellInfo->Id==45373)                    // Bloodberry Elixir
                    return zone_id==4075;
            }
            if(mask & ELIXIR_UNSTABLE_MASK)
            {
                // in the Blade's Edge Mountains Plateaus and Gruul's Lair.
                return zone_id ==3522 || map_id==565;
            }
            if(mask & ELIXIR_SHATTRATH_MASK)
            {
                // in Tempest Keep, Serpentshrine Cavern, Caverns of Time: Mount Hyjal, Black Temple, Sunwell Plateau
                if(zone_id ==3607 || map_id==534 || map_id==564 || zone_id==4075)
                    return true;

                MapEntry const* mapEntry = sMapStore.LookupEntry(map_id);
                if(!mapEntry)
                    return false;

                return mapEntry->multimap_id==206;
            }

            return true;
        }
    }

    // special cases zone check (maps checked by multimap common id)
    switch(spellInfo->Id)
    {
        case 46394:
        {
            if (map_id != 580)
                return false;
            break;
        }
        case 23333:                                         // Warsong Flag
        case 23335:                                         // Silverwing Flag
        case 46392:                                         // Focused Assault
        case 46393:                                         // Brutal Assault
        {
            MapEntry const* mapEntry = sMapStore.LookupEntry(map_id);
            if(!mapEntry)
                return false;

            if(!mapEntry->IsBattleground())
                return false;

            if(zone_id == 3277)
                return true;

            return false;
        }
        case 34976:                                         // Netherstorm Flag
        {
            MapEntry const* mapEntry = sMapStore.LookupEntry(map_id);
            if(!mapEntry)
                return false;

            if(!mapEntry->IsBattleground())
                return false;

            if(zone_id == 3820)
                return true;

            return false;
        }
        case 32724:                                         // Gold Team (Alliance)
        case 32725:                                         // Green Team (Alliance)
        case 32727:                                         // Arena Preparation
        case 35774:                                         // Gold Team (Horde)
        case 35775:                                         // Green Team (Horde)
        {
            MapEntry const* mapEntry = sMapStore.LookupEntry(map_id);
            if(!mapEntry)
                return false;
            
            //the follow code doesn't work.
            //if(!mapEntry->IsBattleArena())
            //    return false;

            //this is the working code, HACK
            if(zone_id == 3702 || zone_id == 3968 || zone_id == 3698)
                return true;

            return false;
        }
        case 41618:                                         // Bottled Nethergon Energy
        case 41620:                                         // Bottled Nethergon Vapor
        {
            MapEntry const* mapEntry = sMapStore.LookupEntry(map_id);
            if(!mapEntry)
                return false;

            return mapEntry->multimap_id==206;
        }
        case 41617:                                         // Cenarion Mana Salve
        case 41619:                                         // Cenarion Healing Salve
        {
            MapEntry const* mapEntry = sMapStore.LookupEntry(map_id);
            if(!mapEntry)
                return false;

            return mapEntry->multimap_id==207;
        }
        case 40216:                                         // Dragonmaw Illusion
        case 42016:                                         // Dragonmaw Illusion
            return area_id == 3759 || area_id == 3966 || area_id == 3939 || area_id == 3965;
        case 2584:                                          // Waiting to Resurrect
        case 22011:                                         // Spirit Heal Channel
        case 22012:                                         // Spirit Heal
        case 24171:                                         // Resurrection Impact Visual
        case 42792:                                         // Recently Dropped Flag
        case 43681:                                         // Inactive
        case 44535:                                         // Spirit Heal (mana)
        case 44521:                                         // Preparation
        {
            MapEntry const* mapEntry = sMapStore.LookupEntry(map_id);
            if(!mapEntry)
                return false;
            else if(!mapEntry->IsBattleground())
                return false;
            else
                return true;
        }
        case 27720:
        case 27721:
        case 27722:
        case 27723:
        {
            MapEntry const* mapEntry = sMapStore.LookupEntry(map_id);
            if(!mapEntry)
                return false;

            if(mapEntry->MapID == 580)
                return false;
                
            break;
        }
        case 40817:                                         // Quest 11026
        {
            if (area_id != 3784 && area_id != 3785)
                return false;
            
            break;
        }
        case 32314:
        {
            if (area_id != 3616)
                return false;
            else
                return true;
        }
        case 45401:
        {
            if (zone_id == 4080 || map_id == 585 || map_id == 580)
                return true;
            else
                return false;
        }
        case 40310:
        case 40623:
        {
            switch (area_id) {
            case 3784:
            case 3832:
            case 3786:
            case 4008:
            case 3785:
            case 3864:
            case 3865:
            case 3971:
            case 3972:
                return true;
            default:
                return false;
            }
            break;
        }
    }

    return true;
}

void SpellMgr::LoadSkillLineAbilityMap()
{
    mSkillLineAbilityMap.clear();

    uint32 count = 0;

    for (uint32 i = 0; i < sSkillLineAbilityStore.GetNumRows(); i++)
    {
        SkillLineAbilityEntry const *SkillInfo = sSkillLineAbilityStore.LookupEntry(i);
        if(!SkillInfo)
            continue;

        mSkillLineAbilityMap.insert(SkillLineAbilityMap::value_type(SkillInfo->spellId,SkillInfo));
        ++count;
    }

    TC_LOG_INFO("server.loading"," ");
    TC_LOG_INFO("server.loading",">> Loaded %u SkillLineAbility MultiMap", count);
}

DiminishingGroup GetDiminishingReturnsGroupForSpell(SpellInfo const* spellproto, bool triggered)
{
    // Explicit Diminishing Groups
    switch(spellproto->SpellFamilyName)
    {
        case SPELLFAMILY_GENERIC:
        {
            switch (spellproto->Id) {
            case 30529: // Recently In Game - Karazhan Chess Event
            case 44799: // Frost Breath (Kalecgos)
            case 46562: // Mind Flay
            case 6945: // Chest Pains
                return DIMINISHING_NONE;
            case 12494: // Frostbite
                return DIMINISHING_TRIGGER_ROOT;
            }
        }
        case SPELLFAMILY_MAGE:
        {
            // Polymorph
            if ((spellproto->SpellFamilyFlags & 0x00001000000LL) && spellproto->Effects[0].ApplyAuraName==SPELL_AURA_MOD_CONFUSE)
                return DIMINISHING_POLYMORPH;
            if (spellproto->Id == 33395) // Elemental's freeze
                return DIMINISHING_CONTROL_ROOT;
            break;
        }
        case SPELLFAMILY_ROGUE:
        {
            // Kidney Shot
            if (spellproto->SpellFamilyFlags & 0x00000200000LL)
                return DIMINISHING_KIDNEYSHOT;
            // Sap
            else if (spellproto->SpellFamilyFlags & 0x00000000080LL)
                return DIMINISHING_POLYMORPH;
            // Gouge
            else if (spellproto->SpellFamilyFlags & 0x00000000008LL)
                return DIMINISHING_POLYMORPH;
            // Blind
            else if (spellproto->SpellFamilyFlags & 0x00001000000LL)
                return DIMINISHING_BLIND_CYCLONE;
            break;
        }
        case SPELLFAMILY_HUNTER:
        {
            // Freezing trap
            if (spellproto->SpellFamilyFlags & 0x00000000008LL)
                return DIMINISHING_FREEZE;
            break;
        }
        case SPELLFAMILY_WARLOCK:
        {
            // Death Coil
            if (spellproto->SpellFamilyFlags & 0x00000080000LL)
                return DIMINISHING_DEATHCOIL;
            // Seduction
            if (spellproto->SpellFamilyFlags & 0x00040000000LL)
                return DIMINISHING_FEAR;
            // Fear
            //else if (spellproto->SpellFamilyFlags & 0x40840000000LL)
            //    return DIMINISHING_WARLOCK_FEAR;
            // Curses/etc
            if (spellproto->SpellVisual == 339 && spellproto->SpellIconID == 692) // Curse of Languages
                return DIMINISHING_LIMITONLY;
            else if (spellproto->SpellFamilyFlags & 0x00080000000LL) {
                if (spellproto->SpellVisual == 1265 && spellproto->SpellIconID == 93)   // Curse of Recklessness
                    return DIMINISHING_NONE;
                else
                    return DIMINISHING_LIMITONLY;
            }
            break;
        }
        case SPELLFAMILY_DRUID:
        {
            // Cyclone
            if (spellproto->SpellFamilyFlags & 0x02000000000LL)
                return DIMINISHING_BLIND_CYCLONE;
            // Nature's Grasp trigger
            if (spellproto->SpellFamilyFlags & 0x00000000200LL && spellproto->Attributes == 0x49010000)
                return DIMINISHING_CONTROL_ROOT;
            break;
        }
        case SPELLFAMILY_WARRIOR:
        {
            // Hamstring - limit duration to 10s in PvP
            if (spellproto->SpellFamilyFlags & 0x00000000002LL)
                return DIMINISHING_LIMITONLY;
            break;
        }
        default:
        {
            break;
        }
    }

    // Get by mechanic
    for (uint8 i=0;i<3;++i)
    {
        if (spellproto->Mechanic      == MECHANIC_STUN    || spellproto->Effects[i].Mechanic == MECHANIC_STUN)
            return triggered ? DIMINISHING_TRIGGER_STUN : DIMINISHING_CONTROL_STUN;
        else if (spellproto->Mechanic == MECHANIC_SLEEP   || spellproto->Effects[i].Mechanic == MECHANIC_SLEEP)
            return DIMINISHING_SLEEP;
        else if (spellproto->Mechanic == MECHANIC_ROOT    || spellproto->Effects[i].Mechanic == MECHANIC_ROOT)
            return triggered ? DIMINISHING_TRIGGER_ROOT : DIMINISHING_CONTROL_ROOT;
        else if (spellproto->Mechanic == MECHANIC_FEAR    || spellproto->Effects[i].Mechanic == MECHANIC_FEAR)
            return DIMINISHING_FEAR;
        else if (spellproto->Mechanic == MECHANIC_CHARM   || spellproto->Effects[i].Mechanic == MECHANIC_CHARM)
            return DIMINISHING_CHARM;
     /*   else if (spellproto->Mechanic == MECHANIC_SILENCE || spellproto->Effects[i].Mechanic == MECHANIC_SILENCE)
            return DIMINISHING_SILENCE; */
        else if (spellproto->Mechanic == MECHANIC_DISARM  || spellproto->Effects[i].Mechanic == MECHANIC_DISARM)
            return DIMINISHING_DISARM;
        else if (spellproto->Mechanic == MECHANIC_FREEZE  || spellproto->Effects[i].Mechanic == MECHANIC_FREEZE)
            return DIMINISHING_FREEZE;
        else if (spellproto->Mechanic == MECHANIC_KNOCKOUT|| spellproto->Effects[i].Mechanic == MECHANIC_KNOCKOUT ||
                 spellproto->Mechanic == MECHANIC_SAPPED  || spellproto->Effects[i].Mechanic == MECHANIC_SAPPED)
            return DIMINISHING_KNOCKOUT;
        else if (spellproto->Mechanic == MECHANIC_BANISH  || spellproto->Effects[i].Mechanic == MECHANIC_BANISH)
            return DIMINISHING_BANISH;
    }

    return DIMINISHING_NONE;
}

bool IsDiminishingReturnsGroupDurationLimited(DiminishingGroup group)
{
    switch(group)
    {
        case DIMINISHING_CONTROL_STUN:
        case DIMINISHING_TRIGGER_STUN:
        case DIMINISHING_KIDNEYSHOT:
        case DIMINISHING_SLEEP:
        case DIMINISHING_CONTROL_ROOT:
        case DIMINISHING_TRIGGER_ROOT:
        case DIMINISHING_FEAR:
        case DIMINISHING_WARLOCK_FEAR:
        case DIMINISHING_CHARM:
        case DIMINISHING_POLYMORPH:
        case DIMINISHING_FREEZE:
        case DIMINISHING_KNOCKOUT:
        case DIMINISHING_BLIND_CYCLONE:
        case DIMINISHING_BANISH:
        case DIMINISHING_LIMITONLY:
            return true;
    }
    return false;
}

DiminishingReturnsType GetDiminishingReturnsGroupType(DiminishingGroup group)
{
    switch(group)
    {
        case DIMINISHING_BLIND_CYCLONE:
        case DIMINISHING_CONTROL_STUN:
        case DIMINISHING_TRIGGER_STUN:
        case DIMINISHING_KIDNEYSHOT:
            return DRTYPE_ALL;
        case DIMINISHING_SLEEP:
        case DIMINISHING_CONTROL_ROOT:
        case DIMINISHING_TRIGGER_ROOT:
        case DIMINISHING_FEAR:
        case DIMINISHING_CHARM:
        case DIMINISHING_POLYMORPH:
        case DIMINISHING_SILENCE:
        case DIMINISHING_DISARM:
        case DIMINISHING_DEATHCOIL:
        case DIMINISHING_FREEZE:
        case DIMINISHING_BANISH:
        case DIMINISHING_WARLOCK_FEAR:
        case DIMINISHING_KNOCKOUT:
            return DRTYPE_PLAYER;
    }

    return DRTYPE_NONE;
}

float SpellMgr::GetSpellThreatModPercent(SpellInfo const* spellInfo) const
{
    if(spellInfo)
    {
        SpellThreatEntry const *threatSpell = sSpellMgr->GetSpellThreatEntry(spellInfo->Id);
        if(threatSpell)
            return threatSpell->pctMod;

        //try to get first in chain
        uint32 firstSpellId = sSpellMgr->GetFirstSpellInChain(spellInfo->Id);
        if(!firstSpellId)
            return 1.0f;

        //see if we have this one in store
        threatSpell = sSpellMgr->GetSpellThreatEntry(firstSpellId);
        if(threatSpell)
            return threatSpell->pctMod;
    }
    return 1.0f;
}

int SpellMgr::GetSpellThreatModFlat(SpellInfo const* spellInfo) const
{
    if(!spellInfo)
        return 0;

    SpellThreatEntry const* threatSpell = sSpellMgr->GetSpellThreatEntry(spellInfo->Id);
    int32 flatMod = 0;
    if(!threatSpell) 
    {
        //try to get first in chain
        uint32 firstSpellId = sSpellMgr->GetFirstSpellInChain(spellInfo->Id);
        if(!firstSpellId)
            return 0;

        //see if we have this one in store
        threatSpell = sSpellMgr->GetSpellThreatEntry(firstSpellId);
        if(!threatSpell)
            return 0;

        //get spell info and create a new adapted flatMod
        SpellInfo const* spellInfoFirstInChain = sSpellMgr->GetSpellInfo(firstSpellId);
        if(!spellInfoFirstInChain)
            return 0;

        flatMod = (threatSpell->flatMod / (float)spellInfoFirstInChain->SpellLevel) * spellInfo->SpellLevel;
    } else {
        flatMod = threatSpell->flatMod;
    }

    return flatMod;
}


/* Used to determine if a spell should take magic resist into account 
Not sure of the original rule 
wowwiki says :
"For spells that have a non-damage effec, such as slow, root, stun, you'll either take the hit or avoid the hit altogether; these are examples of binary spells."
dwarfpriest : 
"Spells that do no damage, or that have a snare effect built in to them (like Mind Flay or Frostbolt), are binary spells."
Taking this second one. And let's extend "snare effect" to "control effects".
Also I'm assuming this is not checked in the attack table but is a plain check after this.
*/
bool SpellMgr::IsBinaryMagicResistanceSpell(SpellInfo const* spell)
{
    if(!spell)
        return false;

    if (!(spell->SchoolMask & SPELL_SCHOOL_MASK_SPELL))
        return false;

    bool doDamage = false;
    for(uint8 i = 0; i < MAX_SPELL_EFFECTS; i++)
    {
        if( spell->Effects[i].Effect == 0 )
            continue;

        //always binary if at least a control effect
        if(    spell->Effects[i].ApplyAuraName == SPELL_AURA_MOD_CONFUSE
            || spell->Effects[i].ApplyAuraName == SPELL_AURA_MOD_CHARM
            || spell->Effects[i].ApplyAuraName == SPELL_AURA_MOD_FEAR
            || spell->Effects[i].ApplyAuraName == SPELL_AURA_MOD_STUN
            || spell->Effects[i].ApplyAuraName == SPELL_AURA_MOD_ROOT
            || spell->Effects[i].ApplyAuraName == SPELL_AURA_MOD_SILENCE
            || spell->Effects[i].ApplyAuraName == SPELL_AURA_MOD_DECREASE_SPEED)
        {
            return true;
        }

        // else not binary if spell does damage
        if(    spell->Effects[i].Effect == SPELL_EFFECT_SCHOOL_DAMAGE
            || spell->Effects[i].Effect == SPELL_EFFECT_ENVIRONMENTAL_DAMAGE
            || spell->Effects[i].ApplyAuraName == SPELL_AURA_PERIODIC_DAMAGE
            || spell->Effects[i].ApplyAuraName == SPELL_AURA_PERIODIC_DAMAGE_PERCENT
            || spell->Effects[i].ApplyAuraName == SPELL_AURA_PROC_TRIGGER_DAMAGE
            || spell->Effects[i].ApplyAuraName == SPELL_AURA_PERIODIC_LEECH ) // Also be partial resistable
        {
            doDamage = true;
        } 
    }

    return !doDamage;
}

SpellBonusEntry const* SpellMgr::GetSpellBonusData(uint32 spellId) const
{
    // Lookup data
    SpellBonusMap::const_iterator itr = mSpellBonusMap.find(spellId);
    if (itr != mSpellBonusMap.end())
        return &itr->second;
    // Not found, try lookup for 1 spell rank if exist
    if (uint32 rank_1 = GetFirstSpellInChain(spellId))
    {
        SpellBonusMap::const_iterator itr2 = mSpellBonusMap.find(rank_1);
        if (itr2 != mSpellBonusMap.end())
            return &itr2->second;
    }
    return NULL;
}

SpellThreatEntry const* SpellMgr::GetSpellThreatEntry(uint32 spellID) const
{
    SpellThreatMap::const_iterator itr = mSpellThreatMap.find(spellID);
    if (itr != mSpellThreatMap.end())
        return &itr->second;
    else
    {
        uint32 firstSpell = GetFirstSpellInChain(spellID);
        itr = mSpellThreatMap.find(firstSpell);
        if (itr != mSpellThreatMap.end())
            return &itr->second;
    }
    return NULL;
}

void SpellMgr::LoadSpellInfoStore(bool reload /* = false */)
{
    uint32 oldMSTime = GetMSTime();

    if (!reload)
    {
        UnloadSpellInfoStore();

        auto lastSpell = sObjectMgr->GetSpellStore().rbegin();
        mSpellInfoMap.resize(lastSpell->first + 1, nullptr); //fill with null pointers by default

        for (auto i : sObjectMgr->GetSpellStore())
            mSpellInfoMap[i.first] = new SpellInfo(i.second);
    }
    else {
        auto spellStore = sObjectMgr->GetSpellStore();
        for (uint32 i = 0; i < mSpellInfoMap.size(); i++)
        {
            auto itr = spellStore.find(i);
            if (itr == spellStore.end())
                continue;

            //replace old object by a new one, but keep it's adress
            *mSpellInfoMap[i] = std::move(SpellInfo(itr->second));
        }
    }

    TC_LOG_INFO("server.loading", ">> Loaded SpellInfo store in %u ms", GetMSTimeDiffToNow(oldMSTime));
}

void SpellMgr::UnloadSpellInfoStore()
{
    for (uint32 i = 0; i < GetSpellInfoStoreSize(); ++i)
        delete mSpellInfoMap[i];

    mSpellInfoMap.clear();
}