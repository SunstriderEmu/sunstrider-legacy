
#include "SpellMgr.h"
#include "ObjectMgr.h"
#include "SpellAuraDefines.h"
#include "DBCStores.h"
#include "World.h"
#include "Chat.h"
#include "Spell.h"
#include "BattleGroundMgr.h"
#include "SpellInfo.h"

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

bool IsAuraAddedBySpell(uint32 auraType, uint32 spellId)
{
    SpellInfo const *spellproto = sSpellMgr->GetSpellInfo(spellId);
    if (!spellproto) return false;

    for (const auto & Effect : spellproto->Effects)
        if (Effect.ApplyAuraName == auraType)
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
        for(const auto & Effect : spellInfo->Effects)
        {
            if( Effect.TargetA.GetTarget()==TARGET_DEST_DB || Effect.TargetB.GetTarget()==TARGET_DEST_DB )
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

    TC_LOG_INFO("server.loading", ">> Loaded %u spell teleport coordinates", count );
}

void SpellMgr::LoadSpellGroups()
{
    uint32 oldMSTime = GetMSTime();

    mSpellSpellGroup.clear();                                  // need for reload case
    mSpellGroupSpell.clear();

    //                                                0     1
    QueryResult result = WorldDatabase.Query("SELECT id, spell_id FROM spell_group");
    if (!result)
    {
        TC_LOG_INFO("server.loading", ">> Loaded 0 spell group definitions. DB table `spell_group` is empty.");
        return;
    }

    std::set<uint32> groups;
    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        uint32 group_id = fields[0].GetUInt32();
        if (group_id <= SPELL_GROUP_DB_RANGE_MIN && group_id >= SPELL_GROUP_CORE_RANGE_MAX)
        {
            TC_LOG_ERROR("sql.sql", "SpellGroup id %u listed in `spell_group` is in core range, but is not defined in core!", group_id);
            continue;
        }
        int32 spell_id = fields[1].GetInt32();

        groups.insert(group_id);
        mSpellGroupSpell.emplace(SpellGroup(group_id), spell_id);

    } while (result->NextRow());

    for (auto itr = mSpellGroupSpell.begin(); itr != mSpellGroupSpell.end();)
    {
        if (itr->second < 0)
        {
            if (groups.find(abs(itr->second)) == groups.end())
            {
                TC_LOG_ERROR("sql.sql", "SpellGroup id %u listed in `spell_group` does not exist", abs(itr->second));
                itr = mSpellGroupSpell.erase(itr);
            }
            else
                ++itr;
        }
        else
        {
            SpellInfo const* spellInfo = GetSpellInfo(itr->second);
            if (!spellInfo)
            {
                TC_LOG_ERROR("sql.sql", "The spell %u listed in `spell_group` does not exist", itr->second);
                itr = mSpellGroupSpell.erase(itr);
            }
            else if (spellInfo->GetRank() > 1)
            {
                TC_LOG_ERROR("sql.sql", "The spell %u listed in `spell_group` is not the first rank of the spell.", itr->second);
                itr = mSpellGroupSpell.erase(itr);
            }
            else
                ++itr;
        }
    }

    for (auto groupItr = groups.begin(); groupItr != groups.end(); ++groupItr)
    {
        std::set<uint32> spells;
        GetSetOfSpellsInSpellGroup(SpellGroup(*groupItr), spells);

        for (auto spellItr = spells.begin(); spellItr != spells.end(); ++spellItr)
        {
            ++count;
            mSpellSpellGroup.emplace(*spellItr, SpellGroup(*groupItr));
        }
    }

    TC_LOG_INFO("server.loading", ">> Loaded %u spell group definitions in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void SpellMgr::LoadSpellGroupStackRules()
{
    uint32 oldMSTime = GetMSTime();

    mSpellGroupStack.clear();                                  // need for reload case
    mSpellSameEffectStack.clear();

    std::vector<uint32> sameEffectGroups;

    //                                                       0         1
    QueryResult result = WorldDatabase.Query("SELECT group_id, stack_rule FROM spell_group_stack_rules");
    if (!result)
    {
        TC_LOG_INFO("server.loading", ">> Loaded 0 spell group stack rules. DB table `spell_group_stack_rules` is empty.");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        uint32 group_id = fields[0].GetUInt32();
        uint8 stack_rule = fields[1].GetInt8();
        if (stack_rule >= SPELL_GROUP_STACK_RULE_MAX)
        {
            TC_LOG_ERROR("sql.sql", "SpellGroupStackRule %u listed in `spell_group_stack_rules` does not exist.", stack_rule);
            continue;
        }

        auto bounds = GetSpellGroupSpellMapBounds((SpellGroup)group_id);
        if (bounds.first == bounds.second)
        {
            TC_LOG_ERROR("sql.sql", "SpellGroup id %u listed in `spell_group_stack_rules` does not exist.", group_id);
            continue;
        }

        mSpellGroupStack.emplace(SpellGroup(group_id), SpellGroupStackRule(stack_rule));

        // different container for same effect stack rules, need to check effect types
        if (stack_rule == SPELL_GROUP_STACK_RULE_EXCLUSIVE_SAME_EFFECT)
            sameEffectGroups.push_back(group_id);

        ++count;
    } while (result->NextRow());

    TC_LOG_INFO("server.loading", ">> Loaded %u spell group stack rules in %u ms", count, GetMSTimeDiffToNow(oldMSTime));

    count = 0;
    oldMSTime = GetMSTime();
    TC_LOG_INFO("server.loading", ">> Parsing SPELL_GROUP_STACK_RULE_EXCLUSIVE_SAME_EFFECT stack rules...");

    for (uint32 group_id : sameEffectGroups)
    {
        std::set<uint32> spellIds;
        GetSetOfSpellsInSpellGroup(SpellGroup(group_id), spellIds);

        std::unordered_set<uint32> auraTypes;

        // we have to 'guess' what effect this group corresponds to
        {
            std::unordered_multiset<uint32 /*auraName*/> frequencyContainer;

            // only waylay for the moment (shared group)
            std::vector<std::vector<uint32 /*auraName*/>> const SubGroups =
            {
                { SPELL_AURA_MOD_MELEE_HASTE, SPELL_AURA_MOD_MELEE_RANGED_HASTE, SPELL_AURA_MOD_RANGED_HASTE }
            };

            for (uint32 spellId : spellIds)
            {
                SpellInfo const* spellInfo = AssertSpellInfo(spellId);
                for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
                {
                    if (!spellInfo->Effects[i].IsAura())
                        continue;

                    int32 auraName = static_cast<int32>(spellInfo->Effects[i].ApplyAuraName);
                    for (std::vector<uint32> const& subGroup : SubGroups)
                    {
                        if (std::find(subGroup.begin(), subGroup.end(), auraName) != subGroup.end())
                        {
                            // count as first aura
                            auraName = subGroup.front();
                            break;
                        }
                    }

                    frequencyContainer.insert(auraName);
                }
            }

            uint32 auraType = 0;
            size_t auraTypeCount = 0;
            for (uint32 auraName : frequencyContainer)
            {
                size_t currentCount = frequencyContainer.count(auraName);
                if (currentCount > auraTypeCount)
                {
                    auraType = auraName;
                    auraTypeCount = currentCount;
                }
            }

            for (std::vector<uint32> const& subGroup : SubGroups)
            {
                if (auraType == subGroup.front())
                {
                    auraTypes.insert(subGroup.begin(), subGroup.end());
                    break;
                }
            }

            if (auraTypes.empty())
                auraTypes.insert(auraType);
        }

        // re-check spells against guessed group
        for (uint32 spellId : spellIds)
        {
            SpellInfo const* spellInfo = AssertSpellInfo(spellId);

            bool found = false;
            while (spellInfo)
            {
                for (uint32 auraType : auraTypes)
                {
                    if (spellInfo->HasAura(AuraType(auraType)))
                    {
                        found = true;
                        break;
                    }
                }

                if (found)
                    break;

                spellInfo = spellInfo->GetNextRankSpell();
            }

            // not found either, log error
            if (!found)
                TC_LOG_ERROR("sql.sql", "SpellId %u listed in `spell_group` with stack rule 3 does not share aura assigned for group %u", spellId, group_id);
        }

        mSpellSameEffectStack[SpellGroup(group_id)] = auraTypes;
        ++count;
    }

    TC_LOG_INFO("server.loading", ">> Parsed %u SPELL_GROUP_STACK_RULE_EXCLUSIVE_SAME_EFFECT stack rules in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

//Fill SpellEffectInfo.SpellClassMask in SpellInfo store
void SpellMgr::LoadSpellAffects()
{
    // Spell affects related declarations (accessed using SpellMgr functions)
    typedef std::multimap<uint32 /*spellId*/, std::pair<uint8 /*effect*/, uint64 /*mask*/>> SpellAffectMap;
    SpellAffectMap mSpellAffectMap;

    uint32 count = 0;

    //                                                0      1         2
    QueryResult result = WorldDatabase.Query("SELECT entry, effectId, SpellFamilyMask FROM spell_affect");
    if(!result)
    {
        TC_LOG_INFO("server.loading", ">> Loaded %u spell affect definitions", count);
        return;
    }

    do
    {
        Field* fields = result->Fetch();

        uint16 entry   = fields[0].GetUInt32();
        uint8 effectId = fields[1].GetUInt8();

        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry);

        if (!spellInfo)
        {
            TC_LOG_ERROR("server.loading","Spell %u listed in `spell_affect` does not exist", entry);
            continue;
        }

        if (effectId >= 3)
        {
            TC_LOG_ERROR("server.loading","Spell %u listed in `spell_affect` have invalid effect index (%u)", entry, effectId);
            continue;
        }

        if( spellInfo->Effects[effectId].Effect != SPELL_EFFECT_APPLY_AURA ||
            (spellInfo->Effects[effectId].ApplyAuraName != SPELL_AURA_ADD_FLAT_MODIFIER &&
             spellInfo->Effects[effectId].ApplyAuraName != SPELL_AURA_ADD_PCT_MODIFIER  &&
             spellInfo->Effects[effectId].ApplyAuraName != SPELL_AURA_ADD_TARGET_TRIGGER)  )
        {
            TC_LOG_ERROR("server.loading","Spell %u listed in `spell_affect` have not SPELL_AURA_ADD_FLAT_MODIFIER (%u) or SPELL_AURA_ADD_PCT_MODIFIER (%u) or SPELL_AURA_ADD_TARGET_TRIGGER (%u) for effect index (%u)", entry, SPELL_AURA_ADD_FLAT_MODIFIER, SPELL_AURA_ADD_PCT_MODIFIER, SPELL_AURA_ADD_TARGET_TRIGGER, effectId);
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

        mSpellAffectMap.emplace(entry, std::make_pair(effectId, spellAffectMask));

        ++count;
    } while(result->NextRow());
    
    //missing spell affect check
    for (auto itr = sObjectMgr->GetSpellStore().begin(); itr != sObjectMgr->GetSpellStore().end(); itr++)
    {
        uint32 id = itr->first;
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(id);
        if (!spellInfo)
            continue;

        auto range = mSpellAffectMap.equal_range(id);
        for (int effectId = 0; effectId < MAX_SPELL_EFFECTS; ++effectId)
        {
            if( spellInfo->Effects[effectId].Effect != SPELL_EFFECT_APPLY_AURA ||
                (spellInfo->Effects[effectId].ApplyAuraName != SPELL_AURA_ADD_FLAT_MODIFIER &&
                spellInfo->Effects[effectId].ApplyAuraName != SPELL_AURA_ADD_PCT_MODIFIER  &&
                spellInfo->Effects[effectId].ApplyAuraName != SPELL_AURA_ADD_TARGET_TRIGGER) )
                continue;

            if(spellInfo->Effects[effectId].ItemType != 0)
                continue;

            bool found = true;
            for (auto itr2 = range.first; itr2 != range.second; ++itr2)
            {
                uint8 effectIndex = itr2->second.first;
                if (effectIndex == effectId)
                {
                    found = true;
                    break;
                }
            }
            
            if(!found)
                TC_LOG_ERROR("server.loading","Spell %u (%s) misses spell_affect for effect %u", id, spellInfo->SpellName[sWorld->GetDefaultDbcLocale()], effectId);
        }
    }

    //fill SpellInfo
    for (auto itr : mSpellAffectMap)
    {
        uint32 spellId = itr.first;
        uint8 effect = itr.second.first;
        uint64 familyMask = itr.second.second;
        SpellInfo* spellInfo = sSpellMgr->_GetSpellInfo(spellId);
        if (!spellInfo)
        {
            TC_LOG_ERROR("server.loading", "Spell affect table has data for non existing spell %u, skipping", spellId);
            continue;
        }
        if (spellInfo->Effects[effect].Effect == 0)
        {
            TC_LOG_ERROR("server.loading", "Spell affect table has data for non existing effect %u for spell %u, skipping", effect, spellId);
            continue;
        }
        spellInfo->Effects[effect].SpellClassMask = familyMask;
    }
    TC_LOG_INFO("server.loading", ">> Loaded %u spell affect definitions", count);
}


void SpellMgr::LoadSpellProcs()
{
    uint32 oldMSTime = GetMSTime();

    mSpellProcMap.clear();                             // need for reload case

    //                                                     0           1                2                3 
    QueryResult result = WorldDatabase.Query("SELECT SpellId, SchoolMask, SpellFamilyName, SpellFamilyMask, "
    //           4              5               6        7               8               9      10        11      12
        "ProcFlags, SpellTypeMask, SpellPhaseMask, HitMask, AttributesMask, ProcsPerMinute, Chance, Cooldown, Charges FROM spell_proc");

    uint32 count = 0;
    if (result)
    {
        do
        {
            Field* fields = result->Fetch();

            uint8 field = 0;
            int32 spellId = fields[field++].GetInt32();

            bool allRanks = false;
            if (spellId < 0)
            {
                allRanks = true;
                spellId = -spellId;
            }

            SpellInfo const* spellInfo = GetSpellInfo(spellId);
            if (!spellInfo)
            {
                TC_LOG_ERROR("sql.sql", "The spell %u listed in `spell_proc` does not exist", spellId);
                continue;
            }

            if (allRanks)
            {
                if (!spellInfo->IsRanked())
                    TC_LOG_ERROR("sql.sql", "The spell %u listed in `spell_proc` with all ranks, but spell has no ranks.", spellId);

                if (spellInfo->GetFirstRankSpell()->Id != uint32(spellId))
                {
                    TC_LOG_ERROR("sql.sql", "The spell %u listed in `spell_proc` is not the first rank of the spell.", spellId);
                    continue;
                }
            }

            SpellProcEntry baseProcEntry;

            baseProcEntry.SchoolMask = fields[field++].GetInt8();
            baseProcEntry.SpellFamilyName = fields[field++].GetUInt16();
            baseProcEntry.SpellFamilyMask = fields[field++].GetUInt64();
            baseProcEntry.ProcFlags = fields[field++].GetUInt32();
            baseProcEntry.SpellTypeMask = fields[field++].GetUInt32();
            baseProcEntry.SpellPhaseMask = fields[field++].GetUInt32();
            baseProcEntry.HitMask = fields[field++].GetUInt32();
            baseProcEntry.AttributesMask = fields[field++].GetUInt32();
            baseProcEntry.ProcsPerMinute = fields[field++].GetFloat();
            baseProcEntry.Chance = fields[field++].GetFloat();
            baseProcEntry.Cooldown = Milliseconds(fields[field++].GetUInt32());
            baseProcEntry.Charges = fields[field++].GetUInt8();

            while (spellInfo)
            {
                if (mSpellProcMap.find(spellInfo->Id) != mSpellProcMap.end())
                {
                    TC_LOG_ERROR("sql.sql", "The spell %u listed in `spell_proc` already has its first rank in the table.", spellInfo->Id);
                    break;
                }

                SpellProcEntry procEntry = SpellProcEntry(baseProcEntry);

                // take defaults from dbcs
                if (!procEntry.ProcFlags)
                    procEntry.ProcFlags = spellInfo->ProcFlags;
                if (!procEntry.Charges)
                    procEntry.Charges = spellInfo->ProcCharges;
                if (!procEntry.Chance && !procEntry.ProcsPerMinute)
                    procEntry.Chance = float(spellInfo->ProcChance);

                // validate data
                if (procEntry.SchoolMask & ~SPELL_SCHOOL_MASK_ALL)
                    TC_LOG_ERROR("sql.sql", "`spell_proc` table entry for spellId %u has wrong `SchoolMask` set: %u", spellInfo->Id, procEntry.SchoolMask);
                if (procEntry.SpellFamilyName && (procEntry.SpellFamilyName < SPELLFAMILY_MAGE || procEntry.SpellFamilyName == 14 || procEntry.SpellFamilyName == 16 
#ifdef LICH_KING
                    || procEntry.SpellFamilyName > SPELLFAMILY_PET
#endif
                    ))
                    TC_LOG_ERROR("sql.sql", "`spell_proc` table entry for spellId %u has wrong `SpellFamilyName` set: %u", spellInfo->Id, procEntry.SpellFamilyName);
                if (procEntry.Chance < 0)
                {
                    TC_LOG_ERROR("sql.sql", "`spell_proc` table entry for spellId %u has negative value in the `Chance` field", spellInfo->Id);
                    procEntry.Chance = 0;
                }
                if (procEntry.ProcsPerMinute < 0)
                {
                    TC_LOG_ERROR("sql.sql", "`spell_proc` table entry for spellId %u has negative value in the `ProcsPerMinute` field", spellInfo->Id);
                    procEntry.ProcsPerMinute = 0;
                }
                if (!procEntry.ProcFlags)
                    TC_LOG_ERROR("sql.sql", "The `spell_proc` table entry for spellId %u doesn't have any `ProcFlags` value defined, proc will not be triggered.", spellInfo->Id);
                if (procEntry.SpellTypeMask & ~PROC_SPELL_TYPE_MASK_ALL)
                    TC_LOG_ERROR("sql.sql", "`spell_proc` table entry for spellId %u has wrong `SpellTypeMask` set: %u", spellInfo->Id, procEntry.SpellTypeMask);
                if (procEntry.SpellTypeMask && !(procEntry.ProcFlags & SPELL_PROC_FLAG_MASK))
                    TC_LOG_ERROR("sql.sql", "The `spell_proc` table entry for spellId %u has `SpellTypeMask` value defined, but it will not be used for the defined `ProcFlags` value.", spellInfo->Id);
                if (!procEntry.SpellPhaseMask && procEntry.ProcFlags & REQ_SPELL_PHASE_PROC_FLAG_MASK)
                    TC_LOG_ERROR("sql.sql", "The `spell_proc` table entry for spellId %u doesn't have any `SpellPhaseMask` value defined, but it is required for the defined `ProcFlags` value. Proc will not be triggered.", spellInfo->Id);
                if (procEntry.SpellPhaseMask & ~PROC_SPELL_PHASE_MASK_ALL)
                    TC_LOG_ERROR("sql.sql", "The `spell_proc` table entry for spellId %u has wrong `SpellPhaseMask` set: %u", spellInfo->Id, procEntry.SpellPhaseMask);
                if (procEntry.SpellPhaseMask && !(procEntry.ProcFlags & REQ_SPELL_PHASE_PROC_FLAG_MASK))
                    TC_LOG_ERROR("sql.sql", "The `spell_proc` table entry for spellId %u has a `SpellPhaseMask` value defined, but it will not be used for the defined `ProcFlags` value.", spellInfo->Id);
                if (procEntry.HitMask & ~PROC_HIT_MASK_ALL)
                    TC_LOG_ERROR("sql.sql", "The `spell_proc` table entry for spellId %u has wrong `HitMask` set: %u", spellInfo->Id, procEntry.HitMask);
                if (procEntry.HitMask && !(procEntry.ProcFlags & TAKEN_HIT_PROC_FLAG_MASK || (procEntry.ProcFlags & DONE_HIT_PROC_FLAG_MASK && (!procEntry.SpellPhaseMask || procEntry.SpellPhaseMask & (PROC_SPELL_PHASE_HIT | PROC_SPELL_PHASE_FINISH)))))
                    TC_LOG_ERROR("sql.sql", "The `spell_proc` table entry for spellId %u has `HitMask` value defined, but it will not be used for defined `ProcFlags` and `SpellPhaseMask` values.", spellInfo->Id);
                for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
                    if ((procEntry.AttributesMask & (PROC_ATTR_DISABLE_EFF_0 << i)) && !spellInfo->Effects[i].IsAura())
                        TC_LOG_ERROR("sql.sql", "The `spell_proc` table entry for spellId %u has Attribute PROC_ATTR_DISABLE_EFF_%u, but effect %u is not an aura effect", spellInfo->Id, static_cast<uint32>(i), static_cast<uint32>(i));
                if (procEntry.AttributesMask & PROC_ATTR_REQ_SPELLMOD)
                {
                    bool found = false;
                    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
                    {
                        if (!spellInfo->Effects[i].IsAura())
                            continue;

                        if (spellInfo->Effects[i].ApplyAuraName == SPELL_AURA_ADD_PCT_MODIFIER || spellInfo->Effects[i].ApplyAuraName == SPELL_AURA_ADD_FLAT_MODIFIER)
                        {
                            found = true;
                            break;
                        }
                    }

                    if (!found)
                        TC_LOG_ERROR("sql.sql", "The `spell_proc` table entry for spellId %u has Attribute PROC_ATTR_REQ_SPELLMOD, but spell has no spell mods. Proc will not be triggered", spellInfo->Id);
                }

                mSpellProcMap[spellInfo->Id] = procEntry;

                if (allRanks)
                    spellInfo = spellInfo->GetNextRankSpell();
                else
                    break;
            }
            ++count;
        } while (result->NextRow());
    }
    else
        TC_LOG_INFO("server.loading", ">> Loaded 0 spell proc conditions and data. DB table `spell_proc` is empty.");

    TC_LOG_INFO("server.loading", ">> Loaded %u spell proc conditions and data in %u ms", count, GetMSTimeDiffToNow(oldMSTime));

    // Define can trigger auras
    bool isTriggerAura[TOTAL_AURAS];
    // Triggered always, even from triggered spells
    bool isAlwaysTriggeredAura[TOTAL_AURAS];
    // SpellTypeMask to add to the proc
    uint32 spellTypeMask[TOTAL_AURAS];

    // List of auras that CAN trigger but may not exist in spell_proc
    // in most cases needed to drop charges

    // some aura types need additional checks (eg SPELL_AURA_MECHANIC_IMMUNITY needs mechanic check)
    // see AuraEffect::CheckEffectProc
    for (uint16 i = 0; i < TOTAL_AURAS; ++i)
    {
        isTriggerAura[i] = false;
        isAlwaysTriggeredAura[i] = false;
        spellTypeMask[i] = PROC_SPELL_TYPE_MASK_ALL;
    }

    isTriggerAura[SPELL_AURA_DUMMY] = true;                                 // Most dummy auras should require scripting, but there are some exceptions (ie 12311)
    isTriggerAura[SPELL_AURA_MOD_CONFUSE] = true;                           // "Any direct damaging attack will revive targets"
    isTriggerAura[SPELL_AURA_MOD_THREAT] = true;                            // Only one spell: 28762 part of Mage T3 8p bonus
    isTriggerAura[SPELL_AURA_MOD_STUN] = true;                              // Aura does not have charges but needs to be removed on trigger
    isTriggerAura[SPELL_AURA_MOD_DAMAGE_DONE] = true;
    isTriggerAura[SPELL_AURA_MOD_DAMAGE_TAKEN] = true;
    isTriggerAura[SPELL_AURA_MOD_RESISTANCE] = true;
    isTriggerAura[SPELL_AURA_MOD_STEALTH] = true;
    isTriggerAura[SPELL_AURA_MOD_FEAR] = true;                              // Aura does not have charges but needs to be removed on trigger
    isTriggerAura[SPELL_AURA_MOD_ROOT] = true;
    isTriggerAura[SPELL_AURA_TRANSFORM] = true;
    isTriggerAura[SPELL_AURA_REFLECT_SPELLS] = true;
    isTriggerAura[SPELL_AURA_DAMAGE_IMMUNITY] = true;
    isTriggerAura[SPELL_AURA_PROC_TRIGGER_SPELL] = true;
    isTriggerAura[SPELL_AURA_PROC_TRIGGER_DAMAGE] = true;
    isTriggerAura[SPELL_AURA_MOD_POWER_COST_SCHOOL_PCT] = true;
    isTriggerAura[SPELL_AURA_MOD_POWER_COST_SCHOOL] = true;
    isTriggerAura[SPELL_AURA_REFLECT_SPELLS_SCHOOL] = true;
    isTriggerAura[SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN] = true;
    isTriggerAura[SPELL_AURA_MOD_ATTACK_POWER] = true;
    isTriggerAura[SPELL_AURA_ADD_CASTER_HIT_TRIGGER] = true;
    isTriggerAura[SPELL_AURA_OVERRIDE_CLASS_SCRIPTS] = true;
    isTriggerAura[SPELL_AURA_MOD_MELEE_HASTE] = true;
    isTriggerAura[SPELL_AURA_MOD_ATTACKER_MELEE_HIT_CHANCE] = true;
    isTriggerAura[SPELL_AURA_MOD_SPELL_CRIT_CHANCE] = true;
    isTriggerAura[SPELL_AURA_ADD_FLAT_MODIFIER] = true;
    isTriggerAura[SPELL_AURA_ADD_PCT_MODIFIER] = true;
    isTriggerAura[SPELL_AURA_MOD_INVISIBILITY] = true;
    isTriggerAura[SPELL_AURA_FORCE_REACTION] = true;
    isTriggerAura[SPELL_AURA_MOD_TAUNT] = true;
    isTriggerAura[SPELL_AURA_MOD_DETAUNT] = true;
    isTriggerAura[SPELL_AURA_MOD_DAMAGE_PERCENT_DONE] = true;
    isTriggerAura[SPELL_AURA_MOD_ATTACK_POWER_PCT] = true;
    isTriggerAura[SPELL_AURA_MOD_HIT_CHANCE] = true;
    isTriggerAura[SPELL_AURA_MOD_WEAPON_CRIT_PERCENT] = true;
    isTriggerAura[SPELL_AURA_MOD_BLOCK_PERCENT] = true;
#ifdef LICH_KING
    isTriggerAura[SPELL_AURA_MOD_CASTING_SPEED_NOT_STACK] = true;
    isTriggerAura[SPELL_AURA_RAID_PROC_FROM_CHARGE] = true;
    isTriggerAura[SPELL_AURA_RAID_PROC_FROM_CHARGE_WITH_VALUE] = true;
    isTriggerAura[SPELL_AURA_PROC_TRIGGER_SPELL_WITH_VALUE] = true;
    isTriggerAura[SPELL_AURA_ABILITY_IGNORE_AURASTATE] = true;
#endif

    isAlwaysTriggeredAura[SPELL_AURA_OVERRIDE_CLASS_SCRIPTS] = true;
    isAlwaysTriggeredAura[SPELL_AURA_MOD_STEALTH] = true;
    isAlwaysTriggeredAura[SPELL_AURA_MOD_CONFUSE] = true;
    isAlwaysTriggeredAura[SPELL_AURA_MOD_FEAR] = true;
    isAlwaysTriggeredAura[SPELL_AURA_MOD_ROOT] = true;
    isAlwaysTriggeredAura[SPELL_AURA_MOD_STUN] = true;
    isAlwaysTriggeredAura[SPELL_AURA_TRANSFORM] = true;
    isAlwaysTriggeredAura[SPELL_AURA_MOD_INVISIBILITY] = true;

    spellTypeMask[SPELL_AURA_MOD_STEALTH] = PROC_SPELL_TYPE_DAMAGE | PROC_SPELL_TYPE_NO_DMG_HEAL;
    spellTypeMask[SPELL_AURA_MOD_CONFUSE] = PROC_SPELL_TYPE_DAMAGE;
    spellTypeMask[SPELL_AURA_MOD_FEAR] = PROC_SPELL_TYPE_DAMAGE;
    spellTypeMask[SPELL_AURA_MOD_ROOT] = PROC_SPELL_TYPE_DAMAGE;
    spellTypeMask[SPELL_AURA_MOD_STUN] = PROC_SPELL_TYPE_DAMAGE;
    spellTypeMask[SPELL_AURA_TRANSFORM] = PROC_SPELL_TYPE_DAMAGE;
    spellTypeMask[SPELL_AURA_MOD_INVISIBILITY] = PROC_SPELL_TYPE_DAMAGE;

    // This generates default procs to retain compatibility with previous proc system
    TC_LOG_INFO("server.loading", "Generating spell proc data from SpellMap...");
    count = 0;
    oldMSTime = GetMSTime();

    for (SpellInfo const* spellInfo : mSpellInfoMap)
    {
        if (!spellInfo)
            continue;

        // Data already present in DB, overwrites default proc
        if (mSpellProcMap.find(spellInfo->Id) != mSpellProcMap.end())
            continue;

        // Nothing to do if no flags set
        if (!spellInfo->ProcFlags)
            continue;

        bool addTriggerFlag = false;
        uint32 procSpellTypeMask = PROC_SPELL_TYPE_NONE;
        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (!spellInfo->Effects[i].IsEffect())
                continue;

            uint32 auraName = spellInfo->Effects[i].ApplyAuraName;
            if (!auraName)
                continue;

            if (!isTriggerAura[auraName])
                continue;

            procSpellTypeMask |= spellTypeMask[auraName];
            if (isAlwaysTriggeredAura[auraName])
                addTriggerFlag = true;

            // many proc auras with taken procFlag mask don't have attribute "can proc with triggered"
            // they should proc nevertheless (example mage armor spells with judgement)
            if (!addTriggerFlag && (spellInfo->ProcFlags & TAKEN_HIT_PROC_FLAG_MASK) != 0)
            {
                switch (auraName)
                {
                case SPELL_AURA_PROC_TRIGGER_SPELL:
                case SPELL_AURA_PROC_TRIGGER_DAMAGE:
                    addTriggerFlag = true;
                    break;
                default:
                    break;
                }
            }
            break;
        }

        if (!procSpellTypeMask)
        {
            for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
            {
                if (spellInfo->Effects[i].IsAura())
                {
                    TC_LOG_ERROR("sql.sql", "Spell Id %u has DBC ProcFlags %u, but it's of non-proc aura type, it probably needs an entry in `spell_proc` table to be handled correctly.", spellInfo->Id, spellInfo->ProcFlags);
                    break;
                }
            }

            continue;
        }

        SpellProcEntry procEntry;
        procEntry.SchoolMask = 0;
        procEntry.ProcFlags = spellInfo->ProcFlags;
        procEntry.SpellFamilyName = 0;
        procEntry.SpellFamilyMask = 0; //sun: added this, else we use a non init variable and get a random value (not true on TC since they use a flag96 that auto init to 0)
        for (uint32 i = 0; i < MAX_SPELL_EFFECTS; ++i)
            if (spellInfo->Effects[i].IsEffect() && isTriggerAura[spellInfo->Effects[i].ApplyAuraName])
                procEntry.SpellFamilyMask |= spellInfo->Effects[i].SpellClassMask;

        if (procEntry.SpellFamilyMask)
            procEntry.SpellFamilyName = spellInfo->SpellFamilyName;

        procEntry.SpellTypeMask = procSpellTypeMask;
        procEntry.SpellPhaseMask = PROC_SPELL_PHASE_HIT;
        procEntry.HitMask = PROC_HIT_NONE; // uses default proc @see SpellMgr::CanSpellTriggerProcOnEvent

        for (uint32 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (!spellInfo->Effects[i].IsAura())
                continue;

            switch (spellInfo->Effects[i].ApplyAuraName)
            {
                // Reflect auras should only proc off reflects
            case SPELL_AURA_REFLECT_SPELLS:
            case SPELL_AURA_REFLECT_SPELLS_SCHOOL:
                procEntry.HitMask = PROC_HIT_REFLECT;
                break;
                // Only drop charge on crit
            case SPELL_AURA_MOD_WEAPON_CRIT_PERCENT:
                procEntry.HitMask = PROC_HIT_CRITICAL;
                break;
                // Only drop charge on block
            case SPELL_AURA_MOD_BLOCK_PERCENT:
                procEntry.HitMask = PROC_HIT_BLOCK;
                break;
            default:
                continue;
            }
            break;
        }

        procEntry.AttributesMask = 0;
        if (spellInfo->ProcFlags & PROC_FLAG_KILL)
            procEntry.AttributesMask |= PROC_ATTR_REQ_EXP_OR_HONOR;
        if (addTriggerFlag)
            procEntry.AttributesMask |= PROC_ATTR_TRIGGERED_CAN_PROC;

        procEntry.ProcsPerMinute = 0;
        procEntry.Chance = spellInfo->ProcChance;
        procEntry.Cooldown = Milliseconds::zero();
        procEntry.Charges = spellInfo->ProcCharges;

        mSpellProcMap[spellInfo->Id] = procEntry;
        ++count;
    }

    TC_LOG_INFO("server.loading", ">> Generated spell proc data for %u spells in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void SpellMgr::LoadSpellElixirs()
{
    mSpellElixirs.clear();                                  // need for reload case

    uint32 count = 0;

    //                                                0      1
    QueryResult result = WorldDatabase.Query("SELECT entry, mask FROM spell_elixir");
    if( !result )
    {
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

    QueryResult result = WorldDatabase.Query("SELECT EnchantID, Chance, ProcsPerMinute, HitMask, AttributesMask FROM spell_enchant_proc_data");
    if( !result )
    {
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
        spe.Chance = fields[1].GetFloat();
        spe.ProcsPerMinute = fields[2].GetFloat();
        spe.HitMask = fields[3].GetUInt32();
        spe.AttributesMask = fields[4].GetUInt32();

        mSpellEnchantProcEventMap[enchantId] = spe;

        ++count;
    } while( result->NextRow() );

    TC_LOG_INFO("server.loading", ">> Loaded %u enchant proc data definitions", count);
}

bool SpellMgr::IsRankSpellDueToSpell(SpellInfo const *spellInfo_1,uint32 spellId_2) const
{
    SpellInfo const *spellInfo_2 = sSpellMgr->GetSpellInfo(spellId_2);
    if(!spellInfo_1 || !spellInfo_2) 
        return false;
    if(spellInfo_1->Id == spellId_2) 
        return false;

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
    for (const auto & Effect : spellInfo->Effects)
    {
        // Paladin aura Spell
        if(spellInfo->SpellFamilyName == SPELLFAMILY_PALADIN
            && Effect.Effect==SPELL_EFFECT_APPLY_AREA_AURA_PARTY)
            return false;
        // Druid form Spell
        if(spellInfo->SpellFamilyName == SPELLFAMILY_DRUID
            && Effect.Effect==SPELL_EFFECT_APPLY_AURA
            && Effect.ApplyAuraName == SPELL_AURA_MOD_SHAPESHIFT)
            return false;
        // Rogue Stealth
        if(spellInfo->SpellFamilyName == SPELLFAMILY_ROGUE
            && Effect.Effect==SPELL_EFFECT_APPLY_AURA
            && Effect.ApplyAuraName == SPELL_AURA_MOD_SHAPESHIFT)
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

SpellLearnSkillNode const* SpellMgr::GetSpellLearnSkill(uint32 spell_id) const
{
    auto itr = mSpellLearnSkills.find(spell_id);
    if (itr != mSpellLearnSkills.end())
        return &itr->second;
    else
        return nullptr;
}

SpellLearnSpellMapBounds SpellMgr::GetSpellLearnSpellMapBounds(uint32 spell_id) const
{
    return mSpellLearnSpells.equal_range(spell_id);
}

bool SpellMgr::IsSpellLearnSpell(uint32 spell_id) const
{
    return mSpellLearnSpells.find(spell_id) != mSpellLearnSpells.end();
}

bool SpellMgr::IsSpellLearnToSpell(uint32 spell_id1, uint32 spell_id2) const
{
    SpellLearnSpellMapBounds bounds = GetSpellLearnSpellMapBounds(spell_id1);
    for (auto i = bounds.first; i != bounds.second; ++i)
        if (i->second.spell == spell_id2)
            return true;
    return false;
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
    return nullptr;
}

void SpellMgr::LoadSpellRequired()
{
    mSpellsReqSpell.clear();                                   // need for reload case
    mSpellReq.clear();                                         // need for reload case

    QueryResult result = WorldDatabase.Query("SELECT spell_id, req_spell from spell_required");

    if(result == nullptr)
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

void SpellMgr::UnloadSpellInfoChains()
{
    for (auto & mSpellChain : mSpellChains)
        mSpellInfoMap[mSpellChain.first]->ChainEntry = nullptr;

    mSpellChains.clear();
}

void SpellMgr::LoadSpellTalentRanks()
{
    // cleanup core data before reload - remove reference to ChainNode from SpellInfo
    UnloadSpellInfoChains();

    for (uint32 i = 0; i < sTalentStore.GetNumRows(); ++i)
    {
        TalentEntry const* talentInfo = sTalentStore.LookupEntry(i);
        if (!talentInfo)
            continue;

        SpellInfo const* lastSpell = nullptr;
        for (uint8 rank = MAX_TALENT_RANK - 1; rank > 0; --rank)
        {
            if (talentInfo->RankID[rank])
            {
                lastSpell = GetSpellInfo(talentInfo->RankID[rank]);
                break;
            }
        }

        if (!lastSpell)
            continue;

        SpellInfo const* firstSpell = GetSpellInfo(talentInfo->RankID[0]);
        if (!firstSpell)
        {
            TC_LOG_ERROR("spells", "SpellMgr::LoadSpellTalentRanks: First Rank Spell %u for TalentEntry %u does not exist.", talentInfo->RankID[0], i);
            continue;
        }

        SpellInfo const* prevSpell = nullptr;
        for (uint8 rank = 0; rank < MAX_TALENT_RANK; ++rank)
        {
            uint32 spellId = talentInfo->RankID[rank];
            if (!spellId)
                break;

            SpellInfo const* currentSpell = GetSpellInfo(spellId);
            if (!currentSpell)
            {
                TC_LOG_ERROR("spells", "SpellMgr::LoadSpellTalentRanks: Spell %u (Rank: %u) for TalentEntry %u does not exist.", spellId, rank + 1, i);
                break;
            }

            SpellChainNode node;
            node.first = firstSpell;
            node.last = lastSpell;
            node.rank = rank + 1;

            node.prev = prevSpell;
            node.next = node.rank < MAX_TALENT_RANK ? GetSpellInfo(talentInfo->RankID[node.rank]) : nullptr;

            mSpellChains[spellId] = node;
            mSpellInfoMap[spellId]->ChainEntry = &mSpellChains[spellId];

            prevSpell = currentSpell;
        }
    }
}

void SpellMgr::LoadSpellRanks()
{
    // cleanup data and load spell ranks for talents from dbc
    LoadSpellTalentRanks();

    uint32 oldMSTime = GetMSTime();

    //                                                     0             1      2
    QueryResult result = WorldDatabase.Query("SELECT first_spell_id, spell_id, rank from spell_ranks ORDER BY first_spell_id, rank");

    if (!result)
    {
        TC_LOG_INFO("server.loading", ">> Loaded 0 spell rank records. DB table `spell_ranks` is empty.");
        return;
    }

    uint32 count = 0;
    bool finished = false;

    do
    {
        // spellid, rank
        std::list < std::pair < int32, int32 > > rankChain;
        int32 currentSpell = -1;
        int32 lastSpell = -1;

        // fill one chain
        while (currentSpell == lastSpell && !finished)
        {
            Field* fields = result->Fetch();

            currentSpell = fields[0].GetUInt32();
            if (lastSpell == -1)
                lastSpell = currentSpell;
            uint32 spell_id = fields[1].GetUInt32();
            uint32 rank = fields[2].GetUInt8();

            // don't drop the row if we're moving to the next rank
            if (currentSpell == lastSpell)
            {
                rankChain.push_back(std::make_pair(spell_id, rank));
                if (!result->NextRow())
                    finished = true;
            }
            else
                break;
        }
        // check if chain is made with valid first spell
        SpellInfo const* first = GetSpellInfo(lastSpell);
        if (!first)
        {
            TC_LOG_ERROR("sql.sql", "The spell rank identifier(first_spell_id) %u listed in `spell_ranks` does not exist!", lastSpell);
            continue;
        }
        // check if chain is long enough
        /*sun: allow chain with size 1... some spells 60-70 spells have only one ranks and it's be useful to allow handling them as the others
        if (rankChain.size() < 2)
        {
            TC_LOG_ERROR("sql.sql", "There is only 1 spell rank for identifier(first_spell_id) %u in `spell_ranks`, entry is not needed!", lastSpell);
            continue;
        }*/
        int32 curRank = 0;
        bool valid = true;
        // check spells in chain
        for (std::list<std::pair<int32, int32> >::iterator itr = rankChain.begin(); itr != rankChain.end(); ++itr)
        {
            SpellInfo const* spell = GetSpellInfo(itr->first);
            if (!spell)
            {
                TC_LOG_ERROR("sql.sql", "The spell %u (rank %u) listed in `spell_ranks` for chain %u does not exist!", itr->first, itr->second, lastSpell);
                valid = false;
                break;
            }
            ++curRank;
            if (itr->second != curRank)
            {
                TC_LOG_ERROR("sql.sql", "The spell %u (rank %u) listed in `spell_ranks` for chain %u does not have a proper rank value (should be %u)!", itr->first, itr->second, lastSpell, curRank);
                valid = false;
                break;
            }
        }
        if (!valid)
            continue;
        int32 prevRank = 0;
        // insert the chain
        std::list<std::pair<int32, int32> >::iterator itr = rankChain.begin();
        do
        {
            ++count;
            int32 addedSpell = itr->first;

            if (mSpellInfoMap[addedSpell]->ChainEntry)
                TC_LOG_ERROR("sql.sql", "The spell %u (rank: %u, first: %u) listed in `spell_ranks` already has ChainEntry from dbc.", addedSpell, itr->second, lastSpell);

            mSpellChains[addedSpell].first = GetSpellInfo(lastSpell);
            mSpellChains[addedSpell].last = GetSpellInfo(rankChain.back().first);
            mSpellChains[addedSpell].rank = itr->second;
            mSpellChains[addedSpell].prev = GetSpellInfo(prevRank);
            mSpellInfoMap[addedSpell]->ChainEntry = &mSpellChains[addedSpell];
            prevRank = addedSpell;
            ++itr;

            if (itr == rankChain.end())
            {
                mSpellChains[addedSpell].next = nullptr;
                break;
            }
            else
                mSpellChains[addedSpell].next = GetSpellInfo(itr->first);
        } while (true);
    } while (!finished);

    TC_LOG_INFO("server.loading", ">> Loaded %u spell rank records in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
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

        for(const auto & Effect : entry->Effects)
        {
            SpellLearnSkillNode dbc_node;
            switch(Effect.Effect)
            {
            case SPELL_EFFECT_SKILL:

                dbc_node.skill    = Effect.MiscValue;
                dbc_node.step     = Effect.BasePoints;
                if (dbc_node.skill != SKILL_RIDING)
                    dbc_node.value    = 1;
                else
                    dbc_node.value    = (Effect.BasePoints+1)*75;

                dbc_node.maxvalue = (Effect.BasePoints+1)*75;
                break;
            case SPELL_EFFECT_DUAL_WIELD:
                dbc_node.skill = SKILL_DUAL_WIELD;
                dbc_node.step = 1;
                dbc_node.value = 1;
                dbc_node.maxvalue = 1;
                break;
            default:
                continue;
            }

            mSpellLearnSkills[spellId] = dbc_node;
            ++dbc_count;
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

        for(const auto & Effect : spellInfo->Effects)
        {
            if(Effect.Effect ==SPELL_EFFECT_LEARN_SPELL)
            {
                SpellLearnSpellNode dbc_node;
                dbc_node.spell       = Effect.TriggerSpell;
                dbc_node.autoLearned = true;

                SpellLearnSpellMapBounds spell_bounds = sSpellMgr->GetSpellLearnSpellMapBounds(spellId);

                bool found = false;
                for(auto itr_ = spell_bounds.first; itr_ != spell_bounds.second; ++itr_)
                {
                    if(itr_->second.spell == dbc_node.spell)
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

        auto itr = mSpellPetAuraMap.find(spell);
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
    //TODO, migrate most of this to spell_override in DB

    SpellEntry* spellInfo;
    for (auto itr = sObjectMgr->GetSpellStore().begin(); itr != sObjectMgr->GetSpellStore().end(); itr++)
    {
        uint32 i = itr->first;
        spellInfo = itr->second;
        if(!spellInfo)
            continue;

        uint32 effectIndex = 0;
        for(uint32 j : spellInfo->Effect)
        {
            switch(j)
            {
                case SPELL_EFFECT_CHARGE:
                    if(!spellInfo->speed && !spellInfo->SpellFamilyName)
                        spellInfo->speed = SPEED_CHARGE;
                    break; 
                case SPELL_EFFECT_APPLY_AURA:
                    switch (spellInfo->EffectApplyAuraName[effectIndex])
                    {
                        case SPELL_AURA_PERIODIC_TRIGGER_SPELL:
                            /* Sunstrider: I failed to find the right generic logic for SPELL_AURA_PERIODIC_TRIGGER_SPELL with TARGET_UNIT_TARGET_ENEMY, so here is a hack instead
                             This fixes around 20 spells */
                            if((spellInfo->EffectImplicitTargetA[effectIndex] == TARGET_UNIT_TARGET_ENEMY)
                                && (spellInfo->SpellIconID == 225)) //arcane missiles
                                spellInfo->EffectImplicitTargetA[effectIndex] = TARGET_UNIT_CASTER;
                            break;
                    }
                    break;
            }
            effectIndex++;
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
#ifdef LICH_KING
        // Arcane Overload
        case 56430:
            spellInfo->Effect[0] = SPELL_EFFECT_TRIGGER_MISSILE;
            spellInfo->EffectTriggerSpell[0] = 56429;
            // no break intended
        case 56429:
            spellInfo->EffectImplicitTargetA[0] = TARGET_DEST_DEST;
            spellInfo->EffectImplicitTargetB[0] = 0;
            spellInfo->EffectImplicitTargetA[1] = 0;
            spellInfo->EffectImplicitTargetB[1] = 0;
            spellInfo->EffectImplicitTargetA[2] = 0;
            spellInfo->EffectImplicitTargetB[2] = 0;
            break;
#endif
        // Dragonmaw Race: All parts
        case 40890: // Oldie's Rotten Pumpkin
            spellInfo->Targets |= TARGET_FLAG_DEST_LOCATION;
            spellInfo->EffectTriggerSpell[0] = 40905;
            spellInfo->Effect[0] = SPELL_EFFECT_TRIGGER_MISSILE;
            spellInfo->EffectImplicitTargetA[0] = TARGET_DEST_DEST;
            break;
        case 40909: // Trope's Slime Cannon
            spellInfo->Targets |= TARGET_FLAG_DEST_LOCATION;
            spellInfo->EffectTriggerSpell[0] = 40905;
            spellInfo->Effect[0] = SPELL_EFFECT_TRIGGER_MISSILE;
            spellInfo->EffectImplicitTargetA[0] = TARGET_DEST_DEST;
            break;
        case 40894: // Corlok's Skull Barrage
            spellInfo->Targets |= TARGET_FLAG_DEST_LOCATION;
            spellInfo->EffectTriggerSpell[0] = 40900;
            spellInfo->Effect[0] = SPELL_EFFECT_TRIGGER_MISSILE;
            spellInfo->EffectImplicitTargetA[0] = TARGET_DEST_DEST;
            break;
        case 40928: // Ichman's Blazing Fireball
            spellInfo->Targets |= TARGET_FLAG_DEST_LOCATION;
            spellInfo->EffectTriggerSpell[0] = 40929;
            spellInfo->Effect[0] = SPELL_EFFECT_TRIGGER_MISSILE;
            spellInfo->EffectImplicitTargetA[0] = TARGET_DEST_DEST;
            break;
        case 40930: // Mulverick's Great Balls of Lightning
            spellInfo->Targets |= TARGET_FLAG_DEST_LOCATION;
            spellInfo->EffectTriggerSpell[0] = 40931;
            spellInfo->Effect[0] = SPELL_EFFECT_TRIGGER_MISSILE;
            spellInfo->EffectImplicitTargetA[0] = TARGET_DEST_DEST;
            break;
        case 40945: // Sky Shatter
            spellInfo->Targets |= TARGET_FLAG_DEST_LOCATION;
            spellInfo->EffectTriggerSpell[0] = 41064;
            spellInfo->Effect[0] = SPELL_EFFECT_TRIGGER_MISSILE;
            spellInfo->EffectImplicitTargetA[0] = TARGET_DEST_DEST;
            break;
        case 41913: //Illidan SPELL_SHADOWFIEND_PASSIVE
            spellInfo->EffectApplyAuraName[0] = 4; // proc debuff, and summon infinite fiends
            break;
        case 37770:
            spellInfo->Effect[0] = 2;
            break;
        case 31344:  //SPELL_HOWL_OF_AZGALOR
            spellInfo->EffectRadiusIndex[0] = EFFECT_RADIUS_100_YARDS;//100yards instead of 50000?!
            break;
        case 31298: //anatheron SPELL_SLEEP
            spellInfo->EffectImplicitTargetA[0] = 1;
            spellInfo->EffectImplicitTargetB[0] = 0;
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
            spellInfo->AttributesEx3 |= SPELL_ATTR3_ONLY_TARGET_PLAYERS;
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
        case 40611: // Blaze Effect
            spellInfo->EffectRadiusIndex[0] = EFFECT_RADIUS_2_YARDS;
            break;
        case 40030: // Demon Fire
            spellInfo->EffectRadiusIndex[0] = EFFECT_RADIUS_2_YARDS;
            break;
        case 40327: // Atrophy
            spellInfo->Attributes |= SPELL_ATTR0_UNAFFECTED_BY_INVULNERABILITY;
            break;
        case 40322: // Spirit Shield
            spellInfo->AttributesEx &= ~SPELL_ATTR1_CANT_BE_REFLECTED;
            break;
        case 33666:     // Sonic Boom (Murmur)
        case 38795:
            spellInfo->EffectRadiusIndex[0] = EFFECT_RADIUS_10_YARDS;
            spellInfo->EffectRadiusIndex[1] = EFFECT_RADIUS_10_YARDS;
            spellInfo->EffectRadiusIndex[2] = EFFECT_RADIUS_10_YARDS;
            break;
        case 29200:     // Purify Helboar Meat
            spellInfo->EffectImplicitTargetA[0] = TARGET_UNIT_CASTER;
            spellInfo->EffectImplicitTargetB[0] = 0;
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
        case 38157:
            spellInfo->AreaId = 3522;
            break;
        case 46411:
            spellInfo->DurationIndex = 21;
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
            spellInfo->AttributesEx3 |= SPELL_ATTR3_ONLY_TARGET_PLAYERS;
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
        case 45855: //felmyst gas nova
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
        //Totems should pulse at spawn
        case 8145: //"Tremor Totem Passive"
        case 8515:  //"Windfury Totem Passive"
        case 10609: //"Windfury Totem Passive"
        case 10612: //"Windfury Totem Passive"
        case 25581: //"Windfury Totem Passive"
        case 25582: //"Windfury Totem Passive"
        case 6474: //"Earthbind Totem Passive"
        case 8172: // Disease Cleansing Totem Passive
        case 8167: //Poison Cleansing Totem Passive 
        case 8229: //Flametongue totem Passive
        case 8251: //Flametongue totem Passive
        case 10524: //Flametongue totem Passive
        case 16388: //Flametongue totem Passive
        case 25556: //Flametongue totem Passive
            spellInfo->AttributesEx5 |= SPELL_ATTR5_START_PERIODIC_AT_APPLY;
            break;
        default:
            break;
        }
    }

    //hack for some totems/statue/standard. Some examples: SELECT * FROM spell_template WHERE effect1 = 28 AND effectMiscValueB1 = 121
    if (SummonPropertiesEntry* properties = const_cast<SummonPropertiesEntry*>(sSummonPropertiesStore.LookupEntry(121)))
        properties->Type = SUMMON_TYPE_TOTEM;
#ifdef LICH_KING
    if (SummonPropertiesEntry* properties = const_cast<SummonPropertiesEntry*>(sSummonPropertiesStore.LookupEntry(647))) // 52893
        properties->Type = SUMMON_TYPE_TOTEM;
    if (SummonPropertiesEntry* properties = const_cast<SummonPropertiesEntry*>(sSummonPropertiesStore.LookupEntry(628))) // Hungry Plaguehound
        properties->Category = SUMMON_CATEGORY_PET;
#endif

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
                    case 0: triggerSpellInfo->AttributesCu |= SPELL_ATTR0_CU_LINK_CAST; break;
                    case 1: triggerSpellInfo->AttributesCu |= SPELL_ATTR0_CU_LINK_HIT;  break;
                    case 2: triggerSpellInfo->AttributesCu |= SPELL_ATTR0_CU_LINK_AURA; break;
                }
        }
        else
        {
            if(triggerSpellInfo)
                triggerSpellInfo->AttributesCu |= SPELL_ATTR0_CU_LINK_REMOVE;
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

SkillLineAbilityMapBounds SpellMgr::GetSkillLineAbilityMapBounds(uint32 spell_id) const
{
    return mSkillLineAbilityMap.equal_range(spell_id);
}

/// Some checks for spells, to prevent adding depricated/broken spells for trainers, spell book, etc
bool SpellMgr::IsSpellValid(SpellInfo const* spellInfo, Player* pl, bool msg)
{
    // not exist
    if(!spellInfo)
        return false;

    bool need_check_reagents = false;

    // check effects
    for(const auto & Effect : spellInfo->Effects)
    {
        switch(Effect.Effect)
        {
            case 0:
                continue;

                // craft spell for crafting non-existed item (break client recipes list show)
            case SPELL_EFFECT_CREATE_ITEM:
            {
                if(!sObjectMgr->GetItemTemplate( Effect.ItemType ))
                {
                    if(msg)
                    {
                        if(pl)
                            ChatHandler(pl).PSendSysMessage("Craft spell %u create not-exist in DB item (Entry: %u) and then...",spellInfo->Id,Effect.ItemType);
                        else
                            TC_LOG_ERROR("sql.sql","Craft spell %u create not-exist in DB item (Entry: %u) and then...",spellInfo->Id,Effect.ItemType);
                    }
                    return false;
                }

                need_check_reagents = true;
                break;
            }
            case SPELL_EFFECT_LEARN_SPELL:
            {
                SpellInfo const* spellInfo2 = sSpellMgr->GetSpellInfo(Effect.TriggerSpell);
                if( !IsSpellValid(spellInfo2,pl,msg) )
                {
                    if(msg)
                    {
                        if(pl)
                            ChatHandler(pl).PSendSysMessage("Spell %u learn to broken spell %u, and then...",spellInfo->Id,Effect.TriggerSpell);
                        else
                            TC_LOG_ERROR("sql.sql","Spell %u learn to invalid spell %u, and then...",spellInfo->Id,Effect.TriggerSpell);
                    }
                    return false;
                }
                break;
            }
        }
    }

    if(need_check_reagents)
    {
        for(int j : spellInfo->Reagent)
        {
            if(j > 0 && !sObjectMgr->GetItemTemplate( j ))
            {
                if(msg)
                {
                    if(pl)
                        ChatHandler(pl).PSendSysMessage("Craft spell %u have not-exist reagent in DB item (Entry: %u) and then...",spellInfo->Id,j);
                    else
                        TC_LOG_ERROR("sql.sql","Craft spell %u have not-exist reagent in DB item (Entry: %u) and then...",spellInfo->Id,j);
                }
                return false;
            }
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

    TC_LOG_INFO("server.loading",">> Loaded %u SkillLineAbility MultiMap", count);
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

This function is wrong for LK... "Every spell with a damage component can now be partially resisted."
*/
bool SpellMgr::IsBinaryMagicResistanceSpell(SpellInfo const* spell)
{
    if(!spell)
        return false;

#ifdef LICH_KING
    // Sindragosa Frost Breath
    if (spell->Id == 69649 || spell->Id == 71056 || spell->Id == 71057 || spell->Id == 71058 || spell->Id == 73061 || spell->Id == 73062 || spell->Id == 73063 || spell->Id == 73064)
        return false;

    // Frost Fever
    if (spell->Id == 55095)
        return false;

    // Haunt
    if (spell->SpellFamilyName == SPELLFAMILY_WARLOCK && (spell->SpellFamilyFlags[1] & 0x40000))
        return false;

    // Frostbolt (changed from LK)
    if (spell->SpellFamilyName == SPELLFAMILY_MAGE && (spell->SpellFamilyFlags & 0x20))
        return false;
#endif

    if (!(spell->SchoolMask & SPELL_SCHOOL_MASK_SPELL))
        return false;

    bool doDamage = false;
    for(const auto& Effect : spell->Effects)
    {
        if( Effect.Effect == 0 )
            continue;

        //always binary if at least a control effect
        if(    Effect.ApplyAuraName == SPELL_AURA_MOD_CONFUSE
            || Effect.ApplyAuraName == SPELL_AURA_MOD_CHARM
            || Effect.ApplyAuraName == SPELL_AURA_MOD_FEAR
            || Effect.ApplyAuraName == SPELL_AURA_MOD_STUN
            || Effect.ApplyAuraName == SPELL_AURA_MOD_ROOT
            || Effect.ApplyAuraName == SPELL_AURA_MOD_SILENCE
            || Effect.ApplyAuraName == SPELL_AURA_MOD_DECREASE_SPEED)
        {
            return true;
        }

        // else not binary if spell does damage
        if(    Effect.Effect == SPELL_EFFECT_SCHOOL_DAMAGE
            || Effect.Effect == SPELL_EFFECT_ENVIRONMENTAL_DAMAGE
            || Effect.ApplyAuraName == SPELL_AURA_PERIODIC_DAMAGE
            || Effect.ApplyAuraName == SPELL_AURA_PERIODIC_DAMAGE_PERCENT
            || Effect.ApplyAuraName == SPELL_AURA_PROC_TRIGGER_DAMAGE
            || Effect.ApplyAuraName == SPELL_AURA_PERIODIC_LEECH ) // Also be partial resistable
        {
            doDamage = true;
        } 
    }

    bool binary = !doDamage;
    return binary;
}

bool SpellMgr::CanSpellTriggerProcOnEvent(SpellProcEntry const& procEntry, ProcEventInfo& eventInfo)
{
    // proc type doesn't match
    if (!(eventInfo.GetTypeMask() & procEntry.ProcFlags))
        return false;

    // check XP or honor target requirement
    if (procEntry.AttributesMask & PROC_ATTR_REQ_EXP_OR_HONOR)
        if (Player* actor = eventInfo.GetActor()->ToPlayer())
            if (eventInfo.GetActionTarget() && !actor->IsHonorOrXPTarget(eventInfo.GetActionTarget()))
                return false;

    // check mana requirement
    if (procEntry.AttributesMask & PROC_ATTR_REQ_MANA_COST)
        if (SpellInfo const* eventSpellInfo = eventInfo.GetSpellInfo())
            if (!eventSpellInfo->ManaCost && !eventSpellInfo->ManaCostPercentage)
                return false;

    // always trigger for these types
    if (eventInfo.GetTypeMask() & (PROC_FLAG_KILLED | PROC_FLAG_KILL | PROC_FLAG_DEATH))
        return true;

    // do triggered cast checks
    // Do not consider autoattacks as triggered spells
    if (!(procEntry.AttributesMask & PROC_ATTR_TRIGGERED_CAN_PROC) && !(eventInfo.GetTypeMask() & AUTO_ATTACK_PROC_FLAG_MASK))
    {
        if (Spell const* spell = eventInfo.GetProcSpell())
        {
            if (spell->IsTriggered())
            {
                SpellInfo const* spellInfo = spell->GetSpellInfo();
                if (!spellInfo->HasAttribute(SPELL_ATTR3_TRIGGERED_CAN_TRIGGER_PROC_2) &&
                    !spellInfo->HasAttribute(SPELL_ATTR2_TRIGGERED_CAN_TRIGGER_PROC))
                    return false;
            }
        }
    }

    // check school mask (if set) for other trigger types
    if (procEntry.SchoolMask && !(eventInfo.GetSchoolMask() & procEntry.SchoolMask))
        return false;

    // check spell family name/flags (if set) for spells
    if (eventInfo.GetTypeMask() & SPELL_PROC_FLAG_MASK)
    {
        if (SpellInfo const* eventSpellInfo = eventInfo.GetSpellInfo())
            if (!eventSpellInfo->IsAffected(procEntry.SpellFamilyName, procEntry.SpellFamilyMask))
                return false;

        // check spell type mask (if set)
        if (procEntry.SpellTypeMask && !(eventInfo.GetSpellTypeMask() & procEntry.SpellTypeMask))
            return false;
    }

    // check spell phase mask
    if (eventInfo.GetTypeMask() & REQ_SPELL_PHASE_PROC_FLAG_MASK)
    {
        if (!(eventInfo.GetSpellPhaseMask() & procEntry.SpellPhaseMask))
            return false;
    }

    // check hit mask (on taken hit or on done hit, but not on spell cast phase)
    if ((eventInfo.GetTypeMask() & TAKEN_HIT_PROC_FLAG_MASK) || ((eventInfo.GetTypeMask() & DONE_HIT_PROC_FLAG_MASK) && !(eventInfo.GetSpellPhaseMask() & PROC_SPELL_PHASE_CAST)))
    {
        uint32 hitMask = procEntry.HitMask;
        // get default values if hit mask not set
        if (!hitMask)
        {
            // for taken procs allow normal + critical hits by default
            if (eventInfo.GetTypeMask() & TAKEN_HIT_PROC_FLAG_MASK)
                hitMask |= PROC_HIT_NORMAL | PROC_HIT_CRITICAL;
            // for done procs allow normal + critical + absorbs by default
            else
                hitMask |= PROC_HIT_NORMAL | PROC_HIT_CRITICAL | PROC_HIT_ABSORB;
        }
        if (!(eventInfo.GetHitMask() & hitMask))
            return false;
    }

    return true;
}

/*static*/ bool SpellMgr::IsPartOfSkillLine(uint32 skillId, uint32 spellId)
{
    SkillLineAbilityMapBounds skillBounds = sSpellMgr->GetSkillLineAbilityMapBounds(spellId);
    for (SkillLineAbilityMap::const_iterator itr = skillBounds.first; itr != skillBounds.second; ++itr)
        if (itr->second->skillId == skillId)
            return true;

    return false;
}


SpellBonusEntry const* SpellMgr::GetSpellBonusData(uint32 spellId) const
{
    // Lookup data
    auto itr = mSpellBonusMap.find(spellId);
    if (itr != mSpellBonusMap.end())
        return &itr->second;
    // Not found, try lookup for 1 spell rank if exist
    if (uint32 rank_1 = GetFirstSpellInChain(spellId))
    {
        auto itr2 = mSpellBonusMap.find(rank_1);
        if (itr2 != mSpellBonusMap.end())
            return &itr2->second;
    }
    return nullptr;
}

SpellThreatEntry const* SpellMgr::GetSpellThreatEntry(uint32 spellID) const
{
    auto itr = mSpellThreatMap.find(spellID);
    if (itr != mSpellThreatMap.end())
        return &itr->second;
    else
    {
        uint32 firstSpell = GetFirstSpellInChain(spellID);
        itr = mSpellThreatMap.find(firstSpell);
        if (itr != mSpellThreatMap.end())
            return &itr->second;
    }
    return nullptr;
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
    else { //reload case
        // A lot of the core uses pointers to spellInfoMap so we can't just clear and refill it.
        // What we do here is create new SpellInfo's and replace old ones at their addresses (not sure we're allowed to do that but it seems to work)
        // This has one consequence I can see:
        // - We can't remove spells by reloading

        auto spellStore = sObjectMgr->GetSpellStore();
        for (uint32 i = 0; i < mSpellInfoMap.size(); i++)
        {
            //get SpellEntry to create the new SpellInfo
            auto itr = spellStore.find(i);
            if (itr == spellStore.end())
                continue;

            //replace old object by a new one, but keep it's adress, so that code having already pointers to some SpellInfos can continue.
            if (mSpellInfoMap[i])
                *mSpellInfoMap[i] = std::move(SpellInfo(itr->second));
            else
                mSpellInfoMap[i] = new SpellInfo(itr->second);
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

void SpellMgr::UnloadSpellInfoImplicitTargetConditionLists()
{
    for (uint32 i = 0; i < GetSpellInfoStoreSize(); ++i)
        if (mSpellInfoMap[i])
            mSpellInfoMap[i]->_UnloadImplicitTargetConditionLists();
}

void SpellMgr::LoadSpellInfoCustomAttributes()
{
    uint32 oldMSTime = GetMSTime();
    SpellInfo* spellInfo = nullptr;

    for (uint32 i = 0; i < GetSpellInfoStoreSize(); ++i)
    {
        spellInfo = mSpellInfoMap[i];
        if (!spellInfo)
            continue;

        for (uint8 j = 0; j < MAX_SPELL_EFFECTS; ++j)
        {
            switch (spellInfo->Effects[j].ApplyAuraName)
            {
            case SPELL_AURA_PERIODIC_DAMAGE:
            case SPELL_AURA_PERIODIC_DAMAGE_PERCENT:
            case SPELL_AURA_PERIODIC_LEECH:
                spellInfo->AttributesCu |= SPELL_ATTR0_CU_AURA_DOT;
                break;
            case SPELL_AURA_PERIODIC_HEAL:
            case SPELL_AURA_OBS_MOD_HEALTH:
                spellInfo->AttributesCu |= SPELL_ATTR0_CU_AURA_HOT;
                break;
            case SPELL_AURA_MOD_POSSESS:
            case SPELL_AURA_MOD_CONFUSE:
            case SPELL_AURA_MOD_CHARM:
            case SPELL_AURA_AOE_CHARM:
            case SPELL_AURA_MOD_FEAR:
            case SPELL_AURA_MOD_STUN:
                spellInfo->AttributesCu |= SPELL_ATTR0_CU_AURA_CC;
                break;
            default:
                break;
            }

            switch (spellInfo->Effects[j].Effect)
            {
                case SPELL_EFFECT_SCHOOL_DAMAGE:
                case SPELL_EFFECT_WEAPON_DAMAGE:
                case SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL:
                case SPELL_EFFECT_NORMALIZED_WEAPON_DMG:
                case SPELL_EFFECT_WEAPON_PERCENT_DAMAGE:
                case SPELL_EFFECT_HEAL:
                    spellInfo->AttributesCu |= SPELL_ATTR0_CU_DIRECT_DAMAGE;
                    break;
                case SPELL_EFFECT_CHARGE:
                    if (!spellInfo->Speed && !spellInfo->SpellFamilyName)
                        spellInfo->Speed = SPEED_CHARGE;
                    break;
                case SPELL_EFFECT_PICKPOCKET:
                    spellInfo->AttributesCu |= SPELL_ATTR0_CU_PICKPOCKET;
                    break;
                case SPELL_EFFECT_ENCHANT_ITEM:
                case SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY:
                case SPELL_EFFECT_ENCHANT_HELD_ITEM:
        #ifdef LICH_KING
                case SPELL_EFFECT_ENCHANT_ITEM_PRISMATIC:
        #endif
                {
                    // only enchanting profession enchantments procs can stack
                    if (SpellMgr::IsPartOfSkillLine(SKILL_ENCHANTING, spellInfo->Id))
                    {
                        uint32 enchantId = spellInfo->Effects[j].MiscValue;
                        SpellItemEnchantmentEntry const* enchant = sSpellItemEnchantmentStore.LookupEntry(enchantId);
                        if (!enchant)
                            break;

                        for (uint8 s = 0; s < MAX_ITEM_ENCHANTMENT_EFFECTS; ++s)
                        {
                            if (enchant->type[s] != ITEM_ENCHANTMENT_TYPE_COMBAT_SPELL)
                                continue;

                            SpellInfo* procInfo = sSpellMgr->_GetSpellInfo(enchant->spellid[s]);
                            if (!procInfo)
                                continue;

                            // if proced directly from enchantment, not via proc aura
                            // NOTE: Enchant Weapon - Blade Ward also has proc aura spell and is proced directly
                            // however its not expected to stack so this check is good
                            if (procInfo->HasAura(SPELL_AURA_PROC_TRIGGER_SPELL))
                                continue;

                            procInfo->AttributesCu |= SPELL_ATTR0_CU_ENCHANT_PROC;
                        }
                    }
                    break;
                }
            }
        }

        if (!spellInfo->_IsPositiveEffect(EFFECT_0, true))
            spellInfo->AttributesCu |= SPELL_ATTR0_CU_NEGATIVE_EFF0;

        if (!spellInfo->_IsPositiveEffect(EFFECT_1, true))
            spellInfo->AttributesCu |= SPELL_ATTR0_CU_NEGATIVE_EFF1;

        if (!spellInfo->_IsPositiveEffect(EFFECT_2, true))
            spellInfo->AttributesCu |= SPELL_ATTR0_CU_NEGATIVE_EFF2;

        if (spellInfo->SpellVisual == 3879)
            spellInfo->AttributesCu |= SPELL_ATTR0_CU_CONE_BACK;

        switch (spellInfo->Id)
        {
        case 27003:
            spellInfo->AttributesCu |= SPELL_ATTR0_CU_IGNORE_ARMOR;
            break;
        case 31041: // Roar's Mangle
            spellInfo->AttributesCu |= SPELL_ATTR0_CU_IGNORE_ARMOR;
            break;
        case 33745: // Rank 1 Lacerate (Druid)
            spellInfo->AttributesCu |= SPELL_ATTR0_CU_IGNORE_ARMOR;
            break;
        case 379: //earth shield heal effect
        case 33076: //Prayer of mending
        case 33110: //Prayer of mending
            spellInfo->AttributesCu |= SPELL_ATTR0_CU_THREAT_GOES_TO_CURRENT_CASTER;
            break;
        case 26029: // dark glare
        case 37433: // spout
        case 43140: case 43215: // flame breath
            spellInfo->AttributesCu |= SPELL_ATTR0_CU_CONE_LINE;
            break;
        case 24340: case 26558: case 28884:     // Meteor
        case 36837: case 38903: case 41276:     // Meteor
        case 26789:                             // Shard of the Fallen Star
        case 31436:                             // Malevolent Cleave
        case 35181:                             // Dive Bomb
        case 40810: case 43267: case 43268:     // Saber Lash
        case 42384:                             // Brutal Swipe
            spellInfo->AttributesCu |= SPELL_ATTR0_CU_SHARE_DAMAGE;
            break;
        case 45150:                             // Meteor Slash
            spellInfo->AttributesCu |= SPELL_ATTR0_CU_SHARE_DAMAGE;
            break;
        case 12723: // Sweeping Strikes proc
            spellInfo->AttributesCu |= SPELL_ATTR0_CU_IGNORE_ARMOR;
            break;
        case 45236:
            spellInfo->AttributesCu |= SPELL_ATTR0_CU_IGNORE_ARMOR;
            break;
        case 45232:
            spellInfo->AttributesCu |= SPELL_ATTR0_CU_IGNORE_ARMOR;
            break;
        case 45256:
            spellInfo->AttributesCu |= SPELL_ATTR0_CU_IGNORE_ARMOR;
            break;
        case 45329: // Sacrolash Show nova
            spellInfo->AttributesCu |= SPELL_ATTR0_CU_IGNORE_ARMOR;
            break;
        case 45347: // Sacrolash SPELL_DARK_TOUCHED
        case 45271:
            spellInfo->AttributesCu |= SPELL_ATTR0_CU_IGNORE_ARMOR;
            break;
        case 46771: // SPELL_FLAME_SEAR
            spellInfo->AttributesCu |= SPELL_ATTR0_CU_IGNORE_ARMOR;
            break;
        case 41173:
        case 41093:
        case 41084:
        case 34580: //Impale
            spellInfo->AttributesCu |= SPELL_ATTR0_CU_IGNORE_ARMOR;
            break;
        case 45248:
            spellInfo->AttributesCu |= SPELL_ATTR0_CU_IGNORE_ARMOR;
            break;
        case 39968: //najentus spine explosion
            spellInfo->AttributesCu |= SPELL_ATTR0_CU_AOE_CANT_TARGET_SELF;
            break;
        case 23735:
        case 23736:
        case 23737:
        case 23738:
        case 23766:
        case 23767:
        case 23768:
        case 23769:
            spellInfo->AttributesCu |= SPELL_ATTR0_CU_REMOVE_ON_INSTANCE_ENTER;
            break;
        case 26102: // Sandblast (Ouro)
        case 19272: // Lava Breath (Molten Core - Ancient Core Hound)
        case 21333: // Lava Breath, from ?
        case 36654: // Fel Breath (The Arcatraz - Soul Devourer)
        case 38813: // Fel Breath (The Arcatraz - Soul Devourer) heroic
        case 38814: // Lava Breath, from ?
            spellInfo->AttributesCu |= SPELL_ATTR0_CU_CONE_180;
            break;
        case 45770:
        case 19516:
            spellInfo->AttributesCu |= SPELL_ATTR0_CU_SAME_STACK_DIFF_CASTERS;
            break;
        case 37800:  //GM Transparency 25%
        case 37801:  //GM Transparency 50%
            spellInfo->AttributesCu |= SPELL_ATTR0_CU_NEGATIVE_EFF0;
            break;
        }

        if (SpellMgr::IsBinaryMagicResistanceSpell(spellInfo))
            spellInfo->AttributesCu |= SPELL_ATTR0_CU_BINARY_SPELL;

        // addition for binary spells, ommit spells triggering other spells
        if (spellInfo->HasAttribute(SPELL_ATTR0_CU_BINARY_SPELL)) //sunstrider: this is the opposite condition than the trinity code, I believe they got it backwards
        {
            bool allNonBinary = true;
            bool overrideAttr = false;
            for (uint8 k = 0; k < MAX_SPELL_EFFECTS; ++k)
            {
                if (spellInfo->Effects[k].IsAura() && spellInfo->Effects[k].TriggerSpell)
                {
                    switch (spellInfo->Effects[k].ApplyAuraName)
                    {
                    case SPELL_AURA_PERIODIC_TRIGGER_SPELL:
                    case SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE:
                        if (SpellInfo const* triggerSpell = sSpellMgr->GetSpellInfo(spellInfo->Effects[k].TriggerSpell))
                        {
                            overrideAttr = true;
                            if (triggerSpell->HasAttribute(SPELL_ATTR0_CU_BINARY_SPELL))
                                allNonBinary = false;
                        }
                        break;
                    default:
                        break;
                    }
                }
            }

            if (overrideAttr && allNonBinary)
                spellInfo->AttributesCu &= ~SPELL_ATTR0_CU_BINARY_SPELL;
        }
    }

    TC_LOG_INFO("server.loading", ">> Loaded SpellInfo custom attributes in %u ms", GetMSTimeDiffToNow(oldMSTime));
}

void SpellMgr::LoadSpellAreas()
{
    uint32 oldMSTime = GetMSTime();

    mSpellAreaMap.clear();                                  // need for reload case
    mSpellAreaForQuestMap.clear();
    mSpellAreaForQuestEndMap.clear();
    mSpellAreaForAuraMap.clear();

    //                                                  0     1         2              3               4                 5          6          7       8         9
    QueryResult result = WorldDatabase.Query("SELECT spell, area, quest_start, quest_start_status, quest_end_status, quest_end, aura_spell, racemask, gender, autocast FROM spell_area");
    if (!result)
    {
        TC_LOG_INFO("server.loading", ">> Loaded 0 spell area requirements. DB table `spell_area` is empty.");

        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        uint32 spell = fields[0].GetUInt32();
        SpellArea spellArea;
        spellArea.spellId = spell;
        spellArea.areaId = fields[1].GetUInt32();
        spellArea.questStart = fields[2].GetUInt32();
        spellArea.questStartStatus = fields[3].GetUInt32();
        spellArea.questEndStatus = fields[4].GetUInt32();
        spellArea.questEnd = fields[5].GetUInt32();
        spellArea.auraSpell = fields[6].GetInt32();
        spellArea.raceMask = fields[7].GetUInt32();
        spellArea.gender = Gender(fields[8].GetUInt8());
        spellArea.autocast = fields[9].GetBool();

        if (SpellInfo const* spellInfo = GetSpellInfo(spell))
        {
            if (spellArea.autocast)
                const_cast<SpellInfo*>(spellInfo)->Attributes |= SPELL_ATTR0_CANT_CANCEL;
        }
        else
        {
            TC_LOG_ERROR("sql.sql", "The spell %u listed in `spell_area` does not exist", spell);
            continue;
        }

        {
            bool ok = true;
            SpellAreaMapBounds sa_bounds = GetSpellAreaMapBounds(spellArea.spellId);
            for (auto itr = sa_bounds.first; itr != sa_bounds.second; ++itr)
            {
                if (spellArea.spellId != itr->second.spellId)
                    continue;
                if (spellArea.areaId != itr->second.areaId)
                    continue;
                if (spellArea.questStart != itr->second.questStart)
                    continue;
                if (spellArea.auraSpell != itr->second.auraSpell)
                    continue;
                if ((spellArea.raceMask & itr->second.raceMask) == 0)
                    continue;
                if (spellArea.gender != itr->second.gender)
                    continue;

                // duplicate by requirements
                ok = false;
                break;
            }

            if (!ok)
            {
                TC_LOG_ERROR("sql.sql", "The spell %u listed in `spell_area` is already listed with similar requirements.", spell);
                continue;
            }
        }

        if (spellArea.areaId && !sAreaTableStore.LookupEntry(spellArea.areaId))
        {
            TC_LOG_ERROR("sql.sql", "The spell %u listed in `spell_area` has a wrong area (%u) requirement.", spell, spellArea.areaId);
            continue;
        }

        if (spellArea.questStart && !sObjectMgr->GetQuestTemplate(spellArea.questStart))
        {
            TC_LOG_ERROR("sql.sql", "The spell %u listed in `spell_area` has a wrong start quest (%u) requirement.", spell, spellArea.questStart);
            continue;
        }

        if (spellArea.questEnd)
        {
            if (!sObjectMgr->GetQuestTemplate(spellArea.questEnd))
            {
                TC_LOG_ERROR("sql.sql", "The spell %u listed in `spell_area` has a wrong ending quest (%u) requirement.", spell, spellArea.questEnd);
                continue;
            }
        }

        if (spellArea.auraSpell)
        {
            SpellInfo const* spellInfo = GetSpellInfo(abs(spellArea.auraSpell));
            if (!spellInfo)
            {
                TC_LOG_ERROR("sql.sql", "The spell %u listed in `spell_area` has wrong aura spell (%u) requirement", spell, abs(spellArea.auraSpell));
                continue;
            }

            if (uint32(abs(spellArea.auraSpell)) == spellArea.spellId)
            {
                TC_LOG_ERROR("sql.sql", "The spell %u listed in `spell_area` has aura spell (%u) requirement for itself", spell, abs(spellArea.auraSpell));
                continue;
            }

            // not allow autocast chains by auraSpell field (but allow use as alternative if not present)
            if (spellArea.autocast && spellArea.auraSpell > 0)
            {
                bool chain = false;
                SpellAreaForAuraMapBounds saBound = GetSpellAreaForAuraMapBounds(spellArea.spellId);
                for (auto itr = saBound.first; itr != saBound.second; ++itr)
                {
                    if (itr->second->autocast && itr->second->auraSpell > 0)
                    {
                        chain = true;
                        break;
                    }
                }

                if (chain)
                {
                    TC_LOG_ERROR("sql.sql", "The spell %u listed in `spell_area` has the aura spell (%u) requirement that it autocasts itself from the aura.", spell, spellArea.auraSpell);
                    continue;
                }

                SpellAreaMapBounds saBound2 = GetSpellAreaMapBounds(spellArea.auraSpell);
                for (auto itr2 = saBound2.first; itr2 != saBound2.second; ++itr2)
                {
                    if (itr2->second.autocast && itr2->second.auraSpell > 0)
                    {
                        chain = true;
                        break;
                    }
                }

                if (chain)
                {
                    TC_LOG_ERROR("sql.sql", "The spell %u listed in `spell_area` has the aura spell (%u) requirement that the spell itself autocasts from the aura.", spell, spellArea.auraSpell);
                    continue;
                }
            }
        }

        if (spellArea.raceMask && (spellArea.raceMask & RACEMASK_ALL_PLAYABLE) == 0)
        {
            TC_LOG_ERROR("sql.sql", "The spell %u listed in `spell_area` has wrong race mask (%u) requirement.", spell, spellArea.raceMask);
            continue;
        }

        if (spellArea.gender != GENDER_NONE && spellArea.gender != GENDER_FEMALE && spellArea.gender != GENDER_MALE)
        {
            TC_LOG_ERROR("sql.sql", "The spell %u listed in `spell_area` has wrong gender (%u) requirement.", spell, spellArea.gender);
            continue;
        }

        SpellArea const* sa = &mSpellAreaMap.insert(SpellAreaMap::value_type(spell, spellArea))->second;

        // for search by current zone/subzone at zone/subzone change
        if (spellArea.areaId)
            mSpellAreaForAreaMap.insert(SpellAreaForAreaMap::value_type(spellArea.areaId, sa));

        // for search at quest start/reward
        if (spellArea.questStart)
            mSpellAreaForQuestMap.insert(SpellAreaForQuestMap::value_type(spellArea.questStart, sa));

        // for search at quest start/reward
        if (spellArea.questEnd)
            mSpellAreaForQuestEndMap.insert(SpellAreaForQuestMap::value_type(spellArea.questEnd, sa));

        // for search at aura apply
        if (spellArea.auraSpell)
            mSpellAreaForAuraMap.insert(SpellAreaForAuraMap::value_type(abs(spellArea.auraSpell), sa));

        ++count;
    } while (result->NextRow());

    TC_LOG_INFO("server.loading", ">> Loaded %u spell area requirements in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}


SpellAreaMapBounds SpellMgr::GetSpellAreaMapBounds(uint32 spell_id) const
{
    return mSpellAreaMap.equal_range(spell_id);
}

SpellAreaForQuestMapBounds SpellMgr::GetSpellAreaForQuestMapBounds(uint32 quest_id) const
{
    return mSpellAreaForQuestMap.equal_range(quest_id);
}

SpellAreaForQuestMapBounds SpellMgr::GetSpellAreaForQuestEndMapBounds(uint32 quest_id) const
{
    return mSpellAreaForQuestEndMap.equal_range(quest_id);
}

SpellAreaForAuraMapBounds SpellMgr::GetSpellAreaForAuraMapBounds(uint32 spell_id) const
{
    return mSpellAreaForAuraMap.equal_range(spell_id);
}

SpellAreaForAreaMapBounds SpellMgr::GetSpellAreaForAreaMapBounds(uint32 area_id) const
{
    return mSpellAreaForAreaMap.equal_range(area_id);
}

bool SpellArea::IsFitToRequirements(Player const* player, uint32 newZone, uint32 newArea) const
{
    if (gender != GENDER_NONE)                   // is not expected gender
        if (!player || gender != player->GetGender())
            return false;

    if (raceMask)                                // is not expected race
        if (!player || !(raceMask & player->GetRaceMask()))
            return false;

    if (areaId)                                  // is not in expected zone
        if (newZone != areaId && newArea != areaId)
            return false;

    if (questStart)                              // is not in expected required quest state
        if (!player || (((1 << player->GetQuestStatus(questStart)) & questStartStatus) == 0))
            return false;

    if (questEnd)                                // is not in expected forbidden quest state
        if (!player || (((1 << player->GetQuestStatus(questEnd)) & questEndStatus) == 0))
            return false;

    if (auraSpell)                               // does not have expected aura
        if (!player || (auraSpell > 0 && !player->HasAura(auraSpell)) || (auraSpell < 0 && player->HasAura(-auraSpell)))
            return false;

    if (player)
    {
        if (Battleground* bg = player->GetBattleground())
            return bg->IsSpellAllowed(spellId, player);
    }

#ifdef LICH_KING
    // Extra conditions
    switch (spellId)
    {
    case 58600: // No fly Zone - Dalaran
    {
        if (!player)
            return false;

        AreaTableEntry const* pArea = sAreaTableStore.LookupEntry(player->GetAreaId());
        if (!(pArea && pArea->flags & AREA_FLAG_NO_FLY_ZONE))
            return false;
        if (!player->HasAuraType(SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED) && !player->HasAuraType(SPELL_AURA_FLY))
            return false;
        break;
    }
    case 58730: // No fly Zone - Wintergrasp
    {
        if (!player)
            return false;

        Battlefield* Bf = sBattlefieldMgr->GetBattlefieldToZoneId(player->GetZoneId());
        if (!Bf || Bf->CanFlyIn() || (!player->HasAuraType(SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED) && !player->HasAuraType(SPELL_AURA_FLY)))
            return false;
        break;
    }
    case 56618: // Horde Controls Factory Phase Shift
    case 56617: // Alliance Controls Factory Phase Shift
    {
        if (!player)
            return false;

        Battlefield* bf = sBattlefieldMgr->GetBattlefieldToZoneId(player->GetZoneId());

        if (!bf || bf->GetTypeId() != BATTLEFIELD_WG)
            return false;

        // team that controls the workshop in the specified area
        uint32 team = bf->GetData(newArea);

        if (team == HORDE)
            return spellId == 56618;
        else if (team == ALLIANCE)
            return spellId == 56617;
        break;
    }
    case 57940: // Essence of Wintergrasp - Northrend
    case 58045: // Essence of Wintergrasp - Wintergrasp
    {
        if (!player)
            return false;

        if (Battlefield* battlefieldWG = sBattlefieldMgr->GetBattlefieldByBattleId(BATTLEFIELD_BATTLEID_WG))
            return battlefieldWG->IsEnabled() && (player->GetTeamId() == battlefieldWG->GetDefenderTeam()) && !battlefieldWG->IsWarTime();
        break;
    }
    case 74411: // Battleground - Dampening
    {
        if (!player)
            return false;

        if (Battlefield* bf = sBattlefieldMgr->GetBattlefieldToZoneId(player->GetZoneId()))
            return bf->IsWarTime();
        break;
    }
    }
#endif

    return true;
}

uint32 SpellMgr::GetFirstSpellInChain(uint32 spell_id) const
{
    if (SpellChainNode const* node = GetSpellChainNode(spell_id))
        if(node->first)
            return node->first->Id;

    return spell_id;
}

uint32 SpellMgr::GetPrevSpellInChain(uint32 spell_id) const
{
    if (SpellChainNode const* node = GetSpellChainNode(spell_id))
        if(node->prev)
            return node->prev->Id;

    return 0;
}

uint32 SpellMgr::GetNextSpellInChain(uint32 spell_id) const
{
    if (SpellChainNode const* node = GetSpellChainNode(spell_id))
        if (node->next)
            return node->next->Id;

    return 0;
}

// Note: not use rank for compare to spell ranks: spell chains isn't linear order
// Use IsHighRankOfSpell instead
uint8 SpellMgr::GetSpellRank(uint32 spell_id) const
{
    if (SpellChainNode const* node = GetSpellChainNode(spell_id))
        return node->rank;

    return 0;
}

uint32 SpellMgr::GetSpellWithRank(uint32 spell_id, uint32 rank, bool strict) const
{
    if (SpellChainNode const* node = GetSpellChainNode(spell_id))
    {
        if (rank != node->rank)
            return GetSpellWithRank(node->rank < rank ? node->next->Id : node->prev->Id, rank, strict);
    }
    else if (strict && rank > 1)
        return 0;
    return spell_id;
}

uint32 SpellMgr::GetLastSpellInChain(uint32 spell_id) const
{
    if (SpellChainNode const* node = GetSpellChainNode(spell_id))
        if(node->last)
            return node->last->Id;

    return spell_id;
}

uint8 SpellMgr::IsHighRankOfSpell(uint32 spell1, uint32 spell2) const
{
    auto itr = mSpellChains.find(spell1);

    uint32 rank2 = GetSpellRank(spell2);

    // not ordered correctly by rank value
    if (itr == mSpellChains.end() || !rank2 || itr->second.rank <= rank2)
        return false;

    // check present in same rank chain
    for (; itr != mSpellChains.end(); itr = mSpellChains.find(itr->second.prev->Id))
        if (itr->second.prev && itr->second.prev->Id == spell2)
            return true;

    return false;
}

void SpellMgr::LoadSpellInfoDiminishing()
{
    uint32 oldMSTime = GetMSTime();

    for (SpellInfo* spellInfo : mSpellInfoMap)
    {
        if (!spellInfo)
            continue;

        spellInfo->_LoadSpellDiminishInfo();
    }

    TC_LOG_INFO("server.loading", ">> Loaded SpellInfo diminishing infos in %u ms", GetMSTimeDiffToNow(oldMSTime));
}

SpellInfo const* SpellMgr::EnsureSpellInfo(uint32 spellId) const
{
    ASSERT(spellId < GetSpellInfoStoreSize());
    SpellInfo const* spellInfo = mSpellInfoMap[spellId];
    ASSERT(spellInfo);
    return spellInfo;
}

void SpellMgr::LoadSpellInfoImmunities()
{
    uint32 oldMSTime = GetMSTime();

    for (SpellInfo* spellInfo : mSpellInfoMap)
    {
        if (!spellInfo)
            continue;

        spellInfo->_LoadImmunityInfo();
    }

    TC_LOG_INFO("server.loading", ">> Loaded SpellInfo immunity infos in %u ms", GetMSTimeDiffToNow(oldMSTime));
}


SpellSpellGroupMapBounds SpellMgr::GetSpellSpellGroupMapBounds(uint32 spell_id) const
{
    spell_id = GetFirstSpellInChain(spell_id);
    return mSpellSpellGroup.equal_range(spell_id);
}

bool SpellMgr::IsSpellMemberOfSpellGroup(uint32 spellid, SpellGroup groupid) const
{
    SpellSpellGroupMapBounds spellGroup = GetSpellSpellGroupMapBounds(spellid);
    for (SpellSpellGroupMap::const_iterator itr = spellGroup.first; itr != spellGroup.second; ++itr)
    {
        if (itr->second == groupid)
            return true;
    }
    return false;
}

SpellGroupSpellMapBounds SpellMgr::GetSpellGroupSpellMapBounds(SpellGroup group_id) const
{
    return mSpellGroupSpell.equal_range(group_id);
}

void SpellMgr::GetSetOfSpellsInSpellGroup(SpellGroup group_id, std::set<uint32>& foundSpells) const
{
    std::set<SpellGroup> usedGroups;
    GetSetOfSpellsInSpellGroup(group_id, foundSpells, usedGroups);
}

void SpellMgr::GetSetOfSpellsInSpellGroup(SpellGroup group_id, std::set<uint32>& foundSpells, std::set<SpellGroup>& usedGroups) const
{
    if (usedGroups.find(group_id) != usedGroups.end())
        return;
    usedGroups.insert(group_id);

    SpellGroupSpellMapBounds groupSpell = GetSpellGroupSpellMapBounds(group_id);
    for (SpellGroupSpellMap::const_iterator itr = groupSpell.first; itr != groupSpell.second; ++itr)
    {
        if (itr->second < 0)
        {
            SpellGroup currGroup = (SpellGroup)abs(itr->second);
            GetSetOfSpellsInSpellGroup(currGroup, foundSpells, usedGroups);
        }
        else
        {
            foundSpells.insert(itr->second);
        }
    }
}

bool SpellMgr::AddSameEffectStackRuleSpellGroups(SpellInfo const* spellInfo, uint32 auraType, int32 amount, std::map<SpellGroup, int32>& groups) const
{
    uint32 spellId = spellInfo->GetFirstRankSpell()->Id;
    auto spellGroupBounds = GetSpellSpellGroupMapBounds(spellId);
    // Find group with SPELL_GROUP_STACK_RULE_EXCLUSIVE_SAME_EFFECT if it belongs to one
    for (auto itr = spellGroupBounds.first; itr != spellGroupBounds.second; ++itr)
    {
        SpellGroup group = itr->second;
        auto found = mSpellSameEffectStack.find(group);
        if (found != mSpellSameEffectStack.end())
        {
            // check auraTypes
            if (!found->second.count(auraType))
                continue;

            // Put the highest amount in the map
            auto groupItr = groups.find(group);
            if (groupItr == groups.end())
                groups.emplace(group, amount);
            else
            {
                int32 curr_amount = groups[group];
                // Take absolute value because this also counts for the highest negative aura
                if (std::abs(curr_amount) < std::abs(amount))
                    groupItr->second = amount;
            }
            // return because a spell should be in only one SPELL_GROUP_STACK_RULE_EXCLUSIVE_SAME_EFFECT group per auraType
            return true;
        }
    }
    // Not in a SPELL_GROUP_STACK_RULE_EXCLUSIVE_SAME_EFFECT group, so return false
    return false;
}

SpellGroupStackRule SpellMgr::CheckSpellGroupStackRules(SpellInfo const* spellInfo1, SpellInfo const* spellInfo2) const
{
    ASSERT(spellInfo1);
    ASSERT(spellInfo2);

    uint32 spellid_1 = spellInfo1->GetFirstRankSpell()->Id;
    uint32 spellid_2 = spellInfo2->GetFirstRankSpell()->Id;

    // find SpellGroups which are common for both spells
    SpellSpellGroupMapBounds spellGroup1 = GetSpellSpellGroupMapBounds(spellid_1);
    std::set<SpellGroup> groups;
    for (SpellSpellGroupMap::const_iterator itr = spellGroup1.first; itr != spellGroup1.second; ++itr)
    {
        if (IsSpellMemberOfSpellGroup(spellid_2, itr->second))
        {
            bool add = true;
            SpellGroupSpellMapBounds groupSpell = GetSpellGroupSpellMapBounds(itr->second);
            for (SpellGroupSpellMap::const_iterator itr2 = groupSpell.first; itr2 != groupSpell.second; ++itr2)
            {
                if (itr2->second < 0)
                {
                    SpellGroup currGroup = (SpellGroup)abs(itr2->second);
                    if (IsSpellMemberOfSpellGroup(spellid_1, currGroup) && IsSpellMemberOfSpellGroup(spellid_2, currGroup))
                    {
                        add = false;
                        break;
                    }
                }
            }
            if (add)
                groups.insert(itr->second);
        }
    }

    SpellGroupStackRule rule = SPELL_GROUP_STACK_RULE_DEFAULT;

    for (std::set<SpellGroup>::iterator itr = groups.begin(); itr != groups.end(); ++itr)
    {
        SpellGroupStackMap::const_iterator found = mSpellGroupStack.find(*itr);
        if (found != mSpellGroupStack.end())
            rule = found->second;
        if (rule)
            break;
    }
    return rule;
}

SpellGroupStackRule SpellMgr::GetSpellGroupStackRule(SpellGroup group) const
{
    SpellGroupStackMap::const_iterator itr = mSpellGroupStack.find(group);
    if (itr != mSpellGroupStack.end())
        return itr->second;

    return SPELL_GROUP_STACK_RULE_DEFAULT;
}