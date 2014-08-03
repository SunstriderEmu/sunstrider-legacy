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

#include "Common.h"
#include "Database/DatabaseEnv.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include "ObjectMgr.h"
#include "SpellMgr.h"
#include "Creature.h"
#include "QuestDef.h"
#include "GossipDef.h"
#include "Player.h"
#include "Opcodes.h"
#include "Log.h"
#include "LootMgr.h"
#include "MapManager.h"
#include "CreatureAI.h"
#include "CreatureAINew.h"
#include "CreatureAISelector.h"
#include "Formulas.h"
#include "SpellAuras.h"
#include "WaypointMovementGenerator.h"
#include "InstanceData.h"
#include "Battleground.h"
#include "Util.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "CellImpl.h"
#include "OutdoorPvPMgr.h"
#include "GameEvent.h"
#include "CreatureGroups.h"
#include "ScriptMgr.h"
// apply implementation of the singletons
#include "Policies/SingletonImp.h"
#include "TemporarySummon.h"
#include "MoveSpline.h"
#include "Spell.h"

void TrainerSpellData::Clear()
{
    for (TrainerSpellList::iterator itr = spellList.begin(); itr != spellList.end(); ++itr)
        delete (*itr);
    spellList.empty();
}

TrainerSpell const* TrainerSpellData::Find(uint32 spell_id) const
{
    for(TrainerSpellList::const_iterator itr = spellList.begin(); itr != spellList.end(); ++itr)
        if((*itr)->spell == spell_id)
            return *itr;

    return NULL;
}

bool VendorItemData::RemoveItem( uint32 item_id )
{
    for(VendorItemList::iterator i = m_items.begin(); i != m_items.end(); ++i )
    {
        if((*i)->proto->ItemId==item_id)
        {
            m_items.erase(i);
            return true;
        }
    }
    return false;
}

size_t VendorItemData::FindItemSlot(uint32 item_id) const
{
    for(size_t i = 0; i < m_items.size(); ++i )
        if(m_items[i]->proto->ItemId==item_id)
            return i;
    return m_items.size();
}

VendorItem const* VendorItemData::FindItem(uint32 item_id) const
{
    for(VendorItemList::const_iterator i = m_items.begin(); i != m_items.end(); ++i )
        if((*i)->proto->ItemId==item_id)
            return *i;
    return NULL;
}

uint32 CreatureTemplate::GetRandomValidModelId() const
{
    uint32 c = 0;
    uint32 modelIDs[4];

    if (Modelid_A1) modelIDs[c++] = Modelid_A1;
    if (Modelid_A2) modelIDs[c++] = Modelid_A2;
    if (Modelid_H1) modelIDs[c++] = Modelid_H1;
    if (Modelid_H2) modelIDs[c++] = Modelid_H2;

    return ((c>0) ? modelIDs[urand(0,c-1)] : 0);
}

uint32 CreatureTemplate::GetFirstValidModelId() const
{
    if(Modelid_A1) return Modelid_A1;
    if(Modelid_A2) return Modelid_A2;
    if(Modelid_H1) return Modelid_H1;
    if(Modelid_H2) return Modelid_H2;
    return 0;
}

bool AssistDelayEvent::Execute(uint64 /*e_time*/, uint32 /*p_time*/)
{
    Unit* victim = Unit::GetUnit(m_owner, m_victim);
    if (victim)
    {
        while (!m_assistants.empty())
        {
            Creature* assistant = Unit::GetCreature(m_owner, *m_assistants.begin());
            m_assistants.pop_front();

            if (assistant && assistant->CanAssistTo(&m_owner, victim))
            {
                assistant->SetNoCallAssistance(true);
                assistant->CombatStart(victim);
                if (assistant->IsAIEnabled) {
                    assistant->AI()->EnterCombat(victim);
                    assistant->AI()->AttackStart(victim);
                }

                if (ScriptedInstance* instance = ((ScriptedInstance*)assistant->GetInstanceData()))
                    instance->MonsterPulled(assistant, victim);
            }
        }
    }
    return true;
}

bool ForcedDespawnDelayEvent::Execute(uint64 /*e_time*/, uint32 /*p_time*/)
{
    m_owner.ForcedDespawn();
    return true;
}

Creature::Creature() :
Unit(),
lootForPickPocketed(false), lootForBody(false), m_lootMoney(0), m_lootRecipient(0),
m_corpseRemoveTime(0), m_respawnTime(0), m_respawnDelay(25), m_corpseDelay(60), m_respawnradius(0.0f),
m_gossipOptionLoaded(false), m_emoteState(0), m_IsPet(false), m_isTotem(false), m_reactState(REACT_AGGRESSIVE),
m_regenTimer(2000), m_defaultMovementType(IDLE_MOTION_TYPE), m_equipmentId(0), m_areaCombatTimer(0),m_relocateTimer(60000),
m_AlreadyCallAssistance(false), m_regenHealth(true), m_AI_locked(false), m_isDeadByDefault(false),
m_meleeDamageSchoolMask(SPELL_SCHOOL_MASK_NORMAL),m_creatureInfo(NULL), m_creatureInfoAddon(NULL),m_DBTableGuid(0), m_formation(NULL),
m_PlayerDamageReq(0), m_timeSinceSpawn(0), m_creaturePoolId(0), m_AI(NULL),
m_isBeingEscorted(false), m_summoned(false), m_path_id(0), m_unreachableTargetTime(0)
{
    m_valuesCount = UNIT_END;

    for(int i =0; i < CREATURE_MAX_SPELLS; ++i)
        m_spells[i] = 0;

    m_CreatureSpellCooldowns.clear();
    m_CreatureCategoryCooldowns.clear();
    m_GlobalCooldown = 0;
    DisableReputationGain = false;
    TriggerJustRespawned = false;
}

Creature::~Creature()
{
    m_vendorItemCounts.clear();

    if(i_AI)
    {
        delete i_AI;
        i_AI = NULL;
    }

    if(m_uint32Values)
        sLog.outDetail("Deconstruct Creature Entry = %u", GetEntry());
}

void Creature::AddToWorld()
{
    ///- Register the creature for guid lookup
    if(!IsInWorld())
    {
        ObjectAccessor::Instance().AddObject(this);
        Unit::AddToWorld();
        SearchFormation();
        if(uint32 guid = GetDBTableGUIDLow())
        {
            if (CreatureData const* data = objmgr.GetCreatureData(guid))
                m_creaturePoolId = data->poolId;
            if (m_creaturePoolId)
                FindMap()->AddCreatureToPool(this, m_creaturePoolId);
        }
    }
}

void Creature::RemoveFromWorld()
{
    ///- Remove the creature from the accessor
    if(IsInWorld())
    {
        uint64 guid = GetGUID();
        if(Map *map = FindMap())
            if(map->IsDungeon() && ((InstanceMap*)map)->GetInstanceData())
                ((InstanceMap*)map)->GetInstanceData()->OnCreatureRemove(this);
        if(m_formation)
            sCreatureGroupMgr.RemoveCreatureFromGroup(m_formation, this);
        if (m_creaturePoolId)
            FindMap()->RemoveCreatureFromPool(this, m_creaturePoolId);
        Unit::RemoveFromWorld();
        ObjectAccessor::Instance().RemoveObject(this, guid);
    }
}

void Creature::DisappearAndDie()
{
    DestroyForNearbyPlayers();
    if(IsAlive())
        SetDeathState(JUST_DIED);
    RemoveCorpse(false);
}

void Creature::SearchFormation()
{
    if(IsPet())
        return;

    uint32 lowguid = GetDBTableGUIDLow();
    if(!lowguid)
    return;

    CreatureGroupInfoType::iterator frmdata = CreatureGroupMap.find(lowguid);
    if(frmdata != CreatureGroupMap.end())
        sCreatureGroupMgr.AddCreatureToGroup(frmdata->second->leaderGUID, this);

}

void Creature::RemoveCorpse(bool setSpawnTime)
{
    if( GetDeathState()!=CORPSE && !m_isDeadByDefault || GetDeathState()!=ALIVE && m_isDeadByDefault )
        return;

    m_corpseRemoveTime = time(NULL);
    SetDeathState(DEAD);
    ObjectAccessor::UpdateObjectVisibility(this);
    loot.clear();

    if (IsAIEnabled)
        AI()->CorpseRemoved(m_respawnDelay);

    // Should get removed later, just keep "compatibility" with scripts
    if(setSpawnTime)
        m_respawnTime = time(NULL) + m_respawnDelay;

    float x,y,z,o;
    GetRespawnPosition(x, y, z, &o);
    SetHomePosition(x,y,z,o);
    GetMap()->CreatureRelocation(this,x,y,z,o);
}

/**
 * change the entry of creature until respawn
 */
bool Creature::InitEntry(uint32 Entry, uint32 team, const CreatureData *data )
{
    CreatureTemplate const *normalInfo = objmgr.GetCreatureTemplate(Entry);
    if(!normalInfo)
    {
        sLog.outErrorDb("Creature::UpdateEntry creature entry %u does not exist.", Entry);
        return false;
    }

    // get heroic mode entry
    uint32 actualEntry = Entry;
    CreatureTemplate const *cinfo = normalInfo;
    if(normalInfo->HeroicEntry)
    {
        Map *map = MapManager::Instance().FindMap(GetMapId(), GetInstanceId());
        if(map && map->IsHeroic())
        {
            cinfo = objmgr.GetCreatureTemplate(normalInfo->HeroicEntry);
            if(!cinfo)
            {
                sLog.outErrorDb("Creature::UpdateEntry creature heroic entry %u does not exist.", actualEntry);
                return false;
            }
        }
    }

    SetEntry(Entry);                                        // normal entry always
    m_creatureInfo = cinfo;                                 // map mode related always

    // Cancel load if no model defined
    if (!(cinfo->GetFirstValidModelId()))
    {
        sLog.outErrorDb("Creature (Entry: %u) has no model defined in table `creature_template`, can't load. ",Entry);
        return false;
    }

    uint32 display_id = objmgr.ChooseDisplayId(team, GetCreatureTemplate(), data);
    CreatureModelInfo const *minfo = objmgr.GetCreatureModelRandomGender(&display_id);
    if (!minfo)
    {
        sLog.outErrorDb("Creature (Entry: %u) has model %u not found in table `creature_model_info`, can't load. ", Entry, display_id);
        return false;
    }

    SetDisplayId(display_id);
    SetNativeDisplayId(display_id);
    SetByteValue(UNIT_FIELD_BYTES_0, 2, minfo->gender);

    // Load creature equipment
    if(!data || data->equipmentId == 0)
    {                                                       // use default from the template
        LoadEquipment(cinfo->equipmentId);
    }
    else if(data && data->equipmentId != -1)
    {                                                       // override, -1 means no equipment
        LoadEquipment(data->equipmentId);
    }

    SetName(normalInfo->Name);                              // at normal entry always

    SetFloatValue(UNIT_FIELD_BOUNDINGRADIUS,minfo->bounding_radius);
    SetFloatValue(UNIT_FIELD_COMBATREACH,minfo->combat_reach );

    SetFloatValue(UNIT_MOD_CAST_SPEED, 1.0f);

    SetSpeed(MOVE_WALK,     cinfo->speed );
    SetSpeed(MOVE_RUN,      cinfo->speed );
    SetSpeed(MOVE_SWIM,     cinfo->speed );

    SetFloatValue(OBJECT_FIELD_SCALE_X, cinfo->scale);

    // checked at loading
    m_defaultMovementType = MovementGeneratorType(cinfo->MovementType);
    if(!m_respawnradius && m_defaultMovementType==RANDOM_MOTION_TYPE)
        m_defaultMovementType = IDLE_MOTION_TYPE;

    m_spells[0] = GetCreatureTemplate()->spell1;
    m_spells[1] = GetCreatureTemplate()->spell2;
    m_spells[2] = GetCreatureTemplate()->spell3;
    m_spells[3] = GetCreatureTemplate()->spell4;
    m_spells[4] = GetCreatureTemplate()->spell5;
    m_spells[5] = GetCreatureTemplate()->spell6;
    m_spells[6] = GetCreatureTemplate()->spell7;
    m_spells[7] = GetCreatureTemplate()->spell8;

    SetQuestPoolId(normalInfo->QuestPoolId);

    return true;
}

bool Creature::UpdateEntry(uint32 Entry, uint32 team, const CreatureData *data )
{
    if(!InitEntry(Entry,team,data))
        return false;

    m_regenHealth = GetCreatureTemplate()->RegenHealth;

    if(GetCreatureTemplate()->flags_extra & CREATURE_FLAG_EXTRA_CIVILIAN)
        SetByteValue(UNIT_FIELD_BYTES_2, 0, SHEATH_STATE_UNARMED );
    else
        SetByteValue(UNIT_FIELD_BYTES_2, 0, SHEATH_STATE_MELEE );

    SetByteValue(UNIT_FIELD_BYTES_2, 1, UNIT_BYTE2_FLAG_AURAS );

    SelectLevel();
    if (team == HORDE)
        SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE, GetCreatureTemplate()->faction_H);
    else
        SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE, GetCreatureTemplate()->faction_A);

    if(GetCreatureTemplate()->flags_extra & CREATURE_FLAG_EXTRA_WORLDEVENT)
        SetUInt32Value(UNIT_NPC_FLAGS,GetCreatureTemplate()->npcflag | gameeventmgr.GetNPCFlag(this));
    else
        SetUInt32Value(UNIT_NPC_FLAGS,GetCreatureTemplate()->npcflag);

    SetAttackTime(BASE_ATTACK,  GetCreatureTemplate()->baseattacktime);
    SetAttackTime(OFF_ATTACK,   GetCreatureTemplate()->baseattacktime);
    SetAttackTime(RANGED_ATTACK,GetCreatureTemplate()->rangeattacktime);

    SetUInt32Value(UNIT_FIELD_FLAGS,GetCreatureTemplate()->unit_flags);
    SetUInt32Value(UNIT_DYNAMIC_FLAGS,GetCreatureTemplate()->dynamicflags);

    RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IN_COMBAT);

    SetMeleeDamageSchool(SpellSchools(GetCreatureTemplate()->dmgschool));
    SetModifierValue(UNIT_MOD_ARMOR,             BASE_VALUE, float(GetCreatureTemplate()->armor));
    SetModifierValue(UNIT_MOD_RESISTANCE_HOLY,   BASE_VALUE, float(GetCreatureTemplate()->resistance1));
    SetModifierValue(UNIT_MOD_RESISTANCE_FIRE,   BASE_VALUE, float(GetCreatureTemplate()->resistance2));
    SetModifierValue(UNIT_MOD_RESISTANCE_NATURE, BASE_VALUE, float(GetCreatureTemplate()->resistance3));
    SetModifierValue(UNIT_MOD_RESISTANCE_FROST,  BASE_VALUE, float(GetCreatureTemplate()->resistance4));
    SetModifierValue(UNIT_MOD_RESISTANCE_SHADOW, BASE_VALUE, float(GetCreatureTemplate()->resistance5));
    SetModifierValue(UNIT_MOD_RESISTANCE_ARCANE, BASE_VALUE, float(GetCreatureTemplate()->resistance6));

    if(GetCreatureTemplate()->flags_extra & CREATURE_FLAG_EXTRA_DUAL_WIELD)
        SetCanDualWield(true);

    SetCanModifyStats(true);
    UpdateAllStats();

    FactionTemplateEntry const* factionTemplate = sFactionTemplateStore.LookupEntry(GetCreatureTemplate()->faction_A);
    if (factionTemplate)                                    // check and error show at loading templates
    {
        FactionEntry const* factionEntry = sFactionStore.LookupEntry(factionTemplate->faction);
        if (factionEntry)
            if( !isCivilian() &&
                (factionEntry->team == ALLIANCE || factionEntry->team == HORDE) )
                SetPvP(true);
    }

    // HACK: trigger creature is always not selectable
    if(isTrigger())
    {
        SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        SetDisableGravity(true);
    }

    if(IsTotem() || isTrigger() || GetCreatureType() == CREATURE_TYPE_CRITTER)
        SetReactState(REACT_PASSIVE);
    /*else if(isCivilian())
        SetReactState(REACT_DEFENSIVE);*/
    else
        SetReactState(REACT_AGGRESSIVE);

    if(GetCreatureTemplate()->flags_extra & CREATURE_FLAG_EXTRA_NO_TAUNT)
    {
        ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, true);
        ApplySpellImmune(0, IMMUNITY_EFFECT,SPELL_EFFECT_ATTACK_ME, true);
    }
    if(GetCreatureTemplate()->flags_extra & CREATURE_FLAG_EXTRA_NO_SPELL_SLOW)
        ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_HASTE_SPELLS, true);

    UpdateMovementFlags();
    return true;
}

void Creature::Update(uint32 diff)
{
    if(m_GlobalCooldown <= diff)
        m_GlobalCooldown = 0;
    else
        m_GlobalCooldown -= diff;
        
    m_timeSinceSpawn += diff;
        
    if (IsAIEnabled && TriggerJustRespawned)
    {
        TriggerJustRespawned = false;
        AI()->JustRespawned();
        if (getAI())
            getAI()->onRespawn();
            
        Map *map = FindMap();
        if (map && map->IsDungeon() && ((InstanceMap*)map)->GetInstanceData())
            ((InstanceMap*)map)->GetInstanceData()->OnCreatureRespawn(this, GetEntry());
    }

    UpdateProhibitedSchools(diff);

    UpdateMovementFlags();

    switch( m_deathState )
    {
        case JUST_RESPAWNED:
            // Don't must be called, see Creature::setDeathState JUST_RESPAWNED -> ALIVE promoting.
            sLog.outError("Creature (GUIDLow: %u Entry: %u ) in wrong state: JUST_RESPAWNED (4)",GetGUIDLow(),GetEntry());
            break;
        case JUST_DIED:
            // Don't must be called, see Creature::setDeathState JUST_DIED -> CORPSE promoting.
            sLog.outError("Creature (GUIDLow: %u Entry: %u ) in wrong state: JUST_DEAD (1)",GetGUIDLow(),GetEntry());
            break;
        case DEAD:
        {
            if( m_respawnTime <= time(NULL) )
            {
                Map* map = FindMap();
                uint32 eventId = getInstanceEventId();
                if (eventId != -1 && map && map->IsRaid() && ((InstanceMap*)map)->GetInstanceData()) {
                    if ((((InstanceMap*)map)->GetInstanceData())->GetData(eventId) == NOT_STARTED)
                        Respawn(); // Respawn immediately
                    else if ((((InstanceMap*)map)->GetInstanceData())->GetData(eventId) == IN_PROGRESS)
                        SetRespawnTime(5*MINUTE); // Delay next respawn check (5 minutes)
                    else // event is DONE or SPECIAL, don't respawn until tag reset
                        SetRespawnTime(7*DAY);
                }
                else if (!GetLinkedCreatureRespawnTime()) // Can respawn
                    Respawn();
                else { // the master is dead
                    if(uint32 targetGuid = objmgr.GetLinkedRespawnGuid(m_DBTableGuid)) {
                        if(targetGuid == m_DBTableGuid) // if linking self, never respawn (check delayed to next day)
                            SetRespawnTime(DAY);
                        else
                            m_respawnTime = (time(NULL)>GetLinkedCreatureRespawnTime()? time(NULL):GetLinkedCreatureRespawnTime())+urand(5,MINUTE); // else copy time from master and add a little
                        SaveRespawnTime(); // also save to DB immediately
                    }
                    else
                        Respawn();
                }
            }
            break;
        }
        case CORPSE:
        {
            if (m_isDeadByDefault)
                break;

            if (m_corpseRemoveTime <= time(NULL))
            {
                RemoveCorpse(false);
                DEBUG_LOG("Removing corpse... %u ", GetUInt32Value(OBJECT_FIELD_ENTRY));
            }
            else
            {
                if (m_groupLootTimer && lootingGroupLeaderGUID)
                {
                    if(diff <= m_groupLootTimer)
                    {
                        m_groupLootTimer -= diff;
                    }
                    else
                    {
                        Group* group = objmgr.GetGroupByLeader(lootingGroupLeaderGUID);
                        if (group)
                            group->EndRoll();
                        m_groupLootTimer = 0;
                        lootingGroupLeaderGUID = 0;
                    }
                }
            }

            break;
        }
        case ALIVE:
        {
            if (m_isDeadByDefault)
            {
                if (m_corpseRemoveTime <= time(NULL))
                {
                    RemoveCorpse(false);
                    DEBUG_LOG("Removing alive corpse... %u ", GetUInt32Value(OBJECT_FIELD_ENTRY));
                }
            }

            Unit::Update( diff );

            // creature can be dead after Unit::Update call
            // CORPSE/DEAD state will processed at next tick (in other case death timer will be updated unexpectedly)
            if(!IsAlive())
                break;
                
            if(IsInCombat() && 
                (isWorldBoss() || GetCreatureTemplate()->flags_extra & CREATURE_FLAG_EXTRA_INSTANCE_BIND) &&
                GetMap() && GetMap()->IsDungeon())
            {
                if(m_areaCombatTimer < diff)
                {
                    std::list<HostilReference *> t_list = getThreatManager().getThreatList();
                    for(std::list<HostilReference *>::iterator i = t_list.begin(); i != t_list.end(); ++i)
                        if((*i) && IS_PLAYER_GUID((*i)->getUnitGuid()))
                        {
                            AreaCombat();
                            break;
                        }

                    m_areaCombatTimer = 5000;
                }else m_areaCombatTimer -= diff;
            }

            // if creature is charmed, switch to charmed AI
            if(NeedChangeAI)
            {
                UpdateCharmAI();
                NeedChangeAI = false;
                IsAIEnabled = true;
            }

            if(IsAIEnabled)
            {
                // do not allow the AI to be changed during update
                m_AI_locked = true;
                if (!IsInEvadeMode())
                {
                    i_AI->UpdateAI(diff);
                    if (getAI())
                        m_AI->update(diff);

                    CheckForUnreachableTarget();
                }
                else
                {
                    if (getAI())
                        m_AI->updateEM(diff);
                }
                m_AI_locked = false;
            }

            // creature can be dead after UpdateAI call
            // CORPSE/DEAD state will processed at next tick (in other case death timer will be updated unexpectedly)
            if(!IsAlive())
                break;

            if(!IsInCombat() && GetCreatureTemplate()->flags_extra & CREATURE_FLAG_EXTRA_PERIODIC_RELOC)
            {
                if(m_relocateTimer < diff)
                {
                     m_relocateTimer = 60000;
                     // forced recreate creature object at clients
                     UnitVisibility currentVis = GetVisibility();
                     SetVisibility(VISIBILITY_RESPAWN);
                     ObjectAccessor::UpdateObjectVisibility(this);
                     SetVisibility(currentVis); // restore visibility state
                     ObjectAccessor::UpdateObjectVisibility(this);
                } else m_relocateTimer -= diff;
            }
            
            if (m_formation)
                GetFormation()->CheckLeaderDistance(this);
                
            if(m_regenTimer > 0)
            {
                if(diff >= m_regenTimer)
                    m_regenTimer = 0;
                else
                    m_regenTimer -= diff;
            }
            
            if (m_regenTimer == 0)
            {
                if (!IsInCombat())
                {
                    if(HasFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_OTHER_TAGGER))
                        SetUInt32Value(UNIT_DYNAMIC_FLAGS, GetCreatureTemplate()->dynamicflags);
                        
                    RegenerateHealth();
                }
                else if(IsPolymorphed())
                    RegenerateHealth();

                RegenerateMana();

                m_regenTimer = 2000;
            }
            
            break;
        }
        case DEAD_FALLING:
        {
            if (!FallGround())
                SetDeathState(JUST_DIED);
        }
        default:
            break;
    }
}

void Creature::RegenerateMana()
{
    uint32 curValue = GetPower(POWER_MANA);
    uint32 maxValue = GetMaxPower(POWER_MANA);

    if (curValue >= maxValue)
        return;

    uint32 addvalue = 0;

    // Combat and any controlled creature
    if (IsInCombat() || GetCharmerOrOwnerGUID())
    {
        if(!IsUnderLastManaUseEffect())
        {
            float ManaIncreaseRate = sWorld.GetRate(RATE_POWER_MANA);
            float Spirit = GetStat(STAT_SPIRIT);

            addvalue = uint32((Spirit/5.0f + 17.0f) * ManaIncreaseRate);
        }
    }
    else
        addvalue = maxValue/3;

    ModifyPower(POWER_MANA, addvalue);
}

void Creature::RegenerateHealth()
{
    if (!isRegeneratingHealth())
        return;

    uint32 curValue = GetHealth();
    uint32 maxValue = GetMaxHealth();

    if (curValue >= maxValue)
        return;

    uint32 addvalue = 0;

    // Not only pet, but any controlled creature
    if(GetCharmerOrOwnerGUID())
    {
        float HealthIncreaseRate = sWorld.GetRate(RATE_HEALTH);
        float Spirit = GetStat(STAT_SPIRIT);

        if( GetPower(POWER_MANA) > 0 )
            addvalue = uint32(Spirit * 0.25 * HealthIncreaseRate);
        else
            addvalue = uint32(Spirit * 0.80 * HealthIncreaseRate);
    }
    else
        addvalue = maxValue/3;

    ModifyHealth(addvalue);
}

bool Creature::AIM_Initialize(CreatureAI* ai)
{
    // make sure nothing can change the AI during AI update
    if(m_AI_locked)
    {
        sLog.outError("AIM_Initialize: failed to init for creature entry %u (DB GUID: %u), locked.", GetEntry(), GetDBTableGUIDLow());
        return false;
    }

    Motion_Initialize();

    if(i_AI) delete i_AI;
    i_AI = ai ? ai : FactorySelector::selectAI(this);
    IsAIEnabled = true;     // Keep this when getting rid of old system
    i_AI->InitializeAI();
    
    // New system
    if (m_AI)
        delete m_AI;
    m_AI = sScriptMgr.getAINew(this);
    if (getAI())
        m_AI->initialize();
    
    return true;
}

bool Creature::Create (uint32 guidlow, Map *map, uint32 Entry, uint32 team, const CreatureData *data)
{
    SetMapId(map->GetId());
    SetInstanceId(map->GetInstanceId());
    //m_DBTableGuid = guidlow;

    //oX = x;     oY = y;    dX = x;    dY = y;    m_moveTime = 0;    m_startMove = 0;
    const bool bResult = CreateFromProto(guidlow, Entry, team, data);

    if (bResult)
    {
        switch (GetCreatureTemplate()->rank)
        {
            case CREATURE_ELITE_RARE:
                m_corpseDelay = sWorld.getConfig(CONFIG_CORPSE_DECAY_RARE);
                break;
            case CREATURE_ELITE_ELITE:
                m_corpseDelay = sWorld.getConfig(CONFIG_CORPSE_DECAY_ELITE);
                break;
            case CREATURE_ELITE_RAREELITE:
                m_corpseDelay = sWorld.getConfig(CONFIG_CORPSE_DECAY_RAREELITE);
                break;
            case CREATURE_ELITE_WORLDBOSS:
                m_corpseDelay = sWorld.getConfig(CONFIG_CORPSE_DECAY_WORLDBOSS);
                break;
            default:
                m_corpseDelay = sWorld.getConfig(CONFIG_CORPSE_DECAY_NORMAL);
                break;
        }
        LoadCreatureAddon();
        InitCreatureAddon();
    }
    return bResult;
}

bool Creature::isTrainerFor(Player* pPlayer, bool msg) const
{
    if(!isTrainer())
        return false;

    TrainerSpellData const* trainer_spells = GetTrainerSpells();

    if(!trainer_spells || trainer_spells->spellList.empty())
    {
        sLog.outErrorDb("Creature %u (Entry: %u) have UNIT_NPC_FLAG_TRAINER but have empty trainer spell list.",
            GetGUIDLow(),GetEntry());
        return false;
    }

    switch(GetCreatureTemplate()->trainer_type)
    {
        case TRAINER_TYPE_CLASS:
            if(pPlayer->GetClass()!=GetCreatureTemplate()->classNum)
            {
                if(msg)
                {
                    pPlayer->PlayerTalkClass->ClearMenus();
                    switch(GetCreatureTemplate()->classNum)
                    {
                        case CLASS_DRUID:  pPlayer->PlayerTalkClass->SendGossipMenu( 4913,GetGUID()); break;
                        case CLASS_HUNTER: pPlayer->PlayerTalkClass->SendGossipMenu(10090,GetGUID()); break;
                        case CLASS_MAGE:   pPlayer->PlayerTalkClass->SendGossipMenu(  328,GetGUID()); break;
                        case CLASS_PALADIN:pPlayer->PlayerTalkClass->SendGossipMenu( 1635,GetGUID()); break;
                        case CLASS_PRIEST: pPlayer->PlayerTalkClass->SendGossipMenu( 4436,GetGUID()); break;
                        case CLASS_ROGUE:  pPlayer->PlayerTalkClass->SendGossipMenu( 4797,GetGUID()); break;
                        case CLASS_SHAMAN: pPlayer->PlayerTalkClass->SendGossipMenu( 5003,GetGUID()); break;
                        case CLASS_WARLOCK:pPlayer->PlayerTalkClass->SendGossipMenu( 5836,GetGUID()); break;
                        case CLASS_WARRIOR:pPlayer->PlayerTalkClass->SendGossipMenu( 4985,GetGUID()); break;
                    }
                }
                return false;
            }
            break;
        case TRAINER_TYPE_PETS:
            if(pPlayer->GetClass()!=CLASS_HUNTER)
            {
                pPlayer->PlayerTalkClass->ClearMenus();
                pPlayer->PlayerTalkClass->SendGossipMenu(3620,GetGUID());
                return false;
            }
            break;
        case TRAINER_TYPE_MOUNTS:
            if(GetCreatureTemplate()->race && pPlayer->GetRace() != GetCreatureTemplate()->race)
            {
                if(msg)
                {
                    pPlayer->PlayerTalkClass->ClearMenus();
                    switch(GetCreatureTemplate()->classNum)
                    {
                        case RACE_DWARF:        pPlayer->PlayerTalkClass->SendGossipMenu(5865,GetGUID()); break;
                        case RACE_GNOME:        pPlayer->PlayerTalkClass->SendGossipMenu(4881,GetGUID()); break;
                        case RACE_HUMAN:        pPlayer->PlayerTalkClass->SendGossipMenu(5861,GetGUID()); break;
                        case RACE_NIGHTELF:     pPlayer->PlayerTalkClass->SendGossipMenu(5862,GetGUID()); break;
                        case RACE_ORC:          pPlayer->PlayerTalkClass->SendGossipMenu(5863,GetGUID()); break;
                        case RACE_TAUREN:       pPlayer->PlayerTalkClass->SendGossipMenu(5864,GetGUID()); break;
                        case RACE_TROLL:        pPlayer->PlayerTalkClass->SendGossipMenu(5816,GetGUID()); break;
                        case RACE_UNDEAD_PLAYER:pPlayer->PlayerTalkClass->SendGossipMenu( 624,GetGUID()); break;
                        case RACE_BLOODELF:     pPlayer->PlayerTalkClass->SendGossipMenu(5862,GetGUID()); break;
                        case RACE_DRAENEI:      pPlayer->PlayerTalkClass->SendGossipMenu(5864,GetGUID()); break;
                    }
                }
                return false;
            }
            break;
        case TRAINER_TYPE_TRADESKILLS:
            if(GetCreatureTemplate()->trainer_spell && !pPlayer->HasSpell(GetCreatureTemplate()->trainer_spell))
            {
                if(msg)
                {
                    pPlayer->PlayerTalkClass->ClearMenus();
                    pPlayer->PlayerTalkClass->SendGossipMenu(11031,GetGUID());
                }
                return false;
            }
            break;
        default:
            return false;                                   // checked and error output at creature_template loading
    }
    return true;
}

bool Creature::canBeBattleMasterFor(Player* pPlayer, bool msg) const
{
    if(!isBattleMaster())
        return false;

    uint32 bgTypeId = objmgr.GetBattleMasterBG(GetEntry());
    if(!msg)
        return pPlayer->GetBGAccessByLevel(bgTypeId);

    if(!pPlayer->GetBGAccessByLevel(bgTypeId))
    {
        pPlayer->PlayerTalkClass->ClearMenus();
        switch(bgTypeId)
        {
            case BATTLEGROUND_AV:  pPlayer->PlayerTalkClass->SendGossipMenu(7616,GetGUID()); break;
            case BATTLEGROUND_WS:  pPlayer->PlayerTalkClass->SendGossipMenu(7599,GetGUID()); break;
            case BATTLEGROUND_AB:  pPlayer->PlayerTalkClass->SendGossipMenu(7642,GetGUID()); break;
            case BATTLEGROUND_EY:
            case BATTLEGROUND_NA:
            case BATTLEGROUND_BE:
            case BATTLEGROUND_AA:
            case BATTLEGROUND_RL:  pPlayer->PlayerTalkClass->SendGossipMenu(10024,GetGUID()); break;
            break;
        }
        return false;
    }
    return true;
}

bool Creature::canResetTalentsOf(Player* pPlayer) const
{
    return pPlayer->GetLevel() >= 10
        && GetCreatureTemplate()->trainer_type == TRAINER_TYPE_CLASS
        && pPlayer->GetClass() == GetCreatureTemplate()->classNum;
}

void Creature::prepareGossipMenu( Player *pPlayer,uint32 gossipid )
{
    //Prevent gossip from NPCs that are possessed.
    Unit* Charmed = Unit::GetCharmer();
    if (Charmed)
        return;

    PlayerMenu* pm=pPlayer->PlayerTalkClass;
    pm->ClearMenus();

    // lazy loading single time at use
    LoadGossipOptions();

    for( GossipOptionList::iterator i = m_goptions.begin( ); i != m_goptions.end( ); ++i )
    {
        GossipOption* gso=&*i;
        if(gso->GossipId == gossipid)
        {
            bool cantalking=true;
            if(gso->Id==1)
            {
                uint32 textid=GetNpcTextId();
                GossipText * gossiptext=objmgr.GetGossipText(textid);
                if(!gossiptext)
                    cantalking=false;
            }
            else
            {
                switch (gso->Action)
                {
                    case GOSSIP_OPTION_QUESTGIVER:
                        pPlayer->PrepareQuestMenu(GetGUID());
                        //if (pm->GetQuestMenu()->MenuItemCount() == 0)
                        cantalking=false;
                        //pm->GetQuestMenu()->ClearMenu();
                        break;
                    case GOSSIP_OPTION_ARMORER:
                        cantalking=false;                   // added in special mode
                        break;
                    case GOSSIP_OPTION_SPIRITHEALER:
                        if( !pPlayer->IsDead() )
                            cantalking=false;
                        break;
                    case GOSSIP_OPTION_VENDOR:
                    {
                        VendorItemData const* vItems = GetVendorItems();
                        if(!vItems || vItems->Empty())
                        {
                            sLog.outErrorDb("Creature %u (Entry: %u) have UNIT_NPC_FLAG_VENDOR but have empty trading item list.",
                                GetGUIDLow(),GetEntry());
                            cantalking=false;
                        }
                        break;
                    }
                    case GOSSIP_OPTION_TRAINER:
                        if(!isTrainerFor(pPlayer,false))
                            cantalking=false;
                        break;
                    case GOSSIP_OPTION_UNLEARNTALENTS:
                        if(!canResetTalentsOf(pPlayer))
                            cantalking=false;
                        break;
                    case GOSSIP_OPTION_UNLEARNPETSKILLS:
                        if(!pPlayer->GetPet() || pPlayer->GetPet()->getPetType() != HUNTER_PET || pPlayer->GetPet()->m_spells.size() <= 1 || GetCreatureTemplate()->trainer_type != TRAINER_TYPE_PETS || GetCreatureTemplate()->classNum != CLASS_HUNTER)
                            cantalking=false;
                        break;
                    case GOSSIP_OPTION_TAXIVENDOR:
                        if ( pPlayer->GetSession()->SendLearnNewTaxiNode(this) )
                            return;
                        break;
                    case GOSSIP_OPTION_BATTLEFIELD:
                        if(!canBeBattleMasterFor(pPlayer,false))
                            cantalking=false;
                        break;
                    case GOSSIP_OPTION_SPIRITGUIDE:
                    case GOSSIP_OPTION_INNKEEPER:
                    case GOSSIP_OPTION_BANKER:
                    case GOSSIP_OPTION_PETITIONER:
                    case GOSSIP_OPTION_STABLEPET:
                    case GOSSIP_OPTION_TABARDDESIGNER:
                    case GOSSIP_OPTION_AUCTIONEER:
                        break;                              // no checks
                    case GOSSIP_OPTION_OUTDOORPVP:
                        if ( !sOutdoorPvPMgr.CanTalkTo(pPlayer,this,(*gso)) )
                            cantalking = false;
                        break;
                    case GOSSIP_OPTION_HALLOWS_END:
                        if (!gameeventmgr.IsActiveEvent(12))
                            cantalking = false;
                        break;
                    default:
                        sLog.outErrorDb("Creature %u (entry: %u) have unknown gossip option %u",GetDBTableGUIDLow(),GetEntry(),gso->Action);
                        break;
                }
            }

            //note for future dev: should have database fields for BoxMessage & BoxMoney
            if(!gso->OptionText.empty() && cantalking)
            {
                std::string OptionText = gso->OptionText;
                std::string BoxText = gso->BoxText;
                int loc_idx = pPlayer->GetSession()->GetSessionDbLocaleIndex();
                if (loc_idx >= 0)
                {
                    NpcOptionLocale const *no = objmgr.GetNpcOptionLocale(gso->Id);
                    if (no)
                    {
                        if (no->OptionText.size() > loc_idx && !no->OptionText[loc_idx].empty())
                            OptionText=no->OptionText[loc_idx];
                        if (no->BoxText.size() > loc_idx && !no->BoxText[loc_idx].empty())
                            BoxText=no->BoxText[loc_idx];
                    }
                }
                pm->GetGossipMenu().AddMenuItem((uint8)gso->Icon,OptionText, gossipid,gso->Action,BoxText,gso->BoxMoney,gso->Coded);
            }
        }
    }

    ///some gossips aren't handled in normal way ... so we need to do it this way .. TODO: handle it in normal way ;-)
    if(pm->Empty())
    {
        if(HasFlag(UNIT_NPC_FLAGS,UNIT_NPC_FLAG_TRAINER))
        {
            isTrainerFor(pPlayer,true);                  // output error message if need
        }
        if(HasFlag(UNIT_NPC_FLAGS,UNIT_NPC_FLAG_BATTLEMASTER))
        {
            canBeBattleMasterFor(pPlayer,true);     // output error message if need
        }
    }
}

void Creature::sendPreparedGossip(Player* player)
{
    if(!player)
        return;

    GossipMenu& gossipmenu = player->PlayerTalkClass->GetGossipMenu();

    if(GetCreatureTemplate()->flags_extra & CREATURE_FLAG_EXTRA_WORLDEVENT) // if world event npc then
        gameeventmgr.HandleWorldEventGossip(player, this);      // update world state with progress

    // in case empty gossip menu open quest menu if any
    if (gossipmenu.Empty() && GetNpcTextId() == 0)
    {
        player->SendPreparedQuest(GetGUID());
        return;
    }

    // in case non empty gossip menu (that not included quests list size) show it
    // (quest entries from quest menu will be included in list)
    player->PlayerTalkClass->SendGossipMenu(GetNpcTextId(), GetGUID());
}

void Creature::OnGossipSelect(Player* player, uint32 option)
{
    GossipMenu& gossipmenu = player->PlayerTalkClass->GetGossipMenu();

    if(option >= gossipmenu.MenuItemCount())
        return;

    uint32 action=gossipmenu.GetItem(option).m_gAction;
    uint32 zoneid=GetZoneId();
    uint64 guid=GetGUID();

    GossipOption const *gossip=GetGossipOption( action );
    if(!gossip)
    {
        zoneid=0;
        gossip=GetGossipOption( action );
        if(!gossip)
            return;
    }

    switch (gossip->Action)
    {
        case GOSSIP_OPTION_GOSSIP:
        {
            uint32 textid = GetGossipTextId(action, zoneid);
            if (textid == 0)
                textid=GetNpcTextId();

            player->PlayerTalkClass->CloseGossip();
            player->PlayerTalkClass->SendTalking(textid);
            break;
        }
        case GOSSIP_OPTION_OUTDOORPVP:
            sOutdoorPvPMgr.HandleGossipOption(player, GetGUID(), option);
            break;
        case GOSSIP_OPTION_SPIRITHEALER:
            if (player->IsDead())
                CastSpell(this,17251,true,NULL,NULL,player->GetGUID());
            break;
        case GOSSIP_OPTION_QUESTGIVER:
            player->PrepareQuestMenu( guid );
            player->SendPreparedQuest( guid );
            break;
        case GOSSIP_OPTION_VENDOR:
        case GOSSIP_OPTION_ARMORER:
            player->GetSession()->SendListInventory(guid);
            break;
        case GOSSIP_OPTION_STABLEPET:
            player->GetSession()->SendStablePet(guid);
            break;
        case GOSSIP_OPTION_TRAINER:
            player->GetSession()->SendTrainerList(guid);
            break;
        case GOSSIP_OPTION_UNLEARNTALENTS:
            player->PlayerTalkClass->CloseGossip();
            player->SendTalentWipeConfirm(guid);
            break;
        case GOSSIP_OPTION_UNLEARNPETSKILLS:
            player->PlayerTalkClass->CloseGossip();
            player->SendPetSkillWipeConfirm();
            break;
        case GOSSIP_OPTION_TAXIVENDOR:
            player->GetSession()->SendTaxiMenu(this);
            break;
        case GOSSIP_OPTION_INNKEEPER:
            player->PlayerTalkClass->CloseGossip();
            player->SetBindPoint( guid );
            break;
        case GOSSIP_OPTION_BANKER:
            player->GetSession()->SendShowBank( guid );
            break;
        case GOSSIP_OPTION_PETITIONER:
            player->PlayerTalkClass->CloseGossip();
            player->GetSession()->SendPetitionShowList( guid );
            break;
        case GOSSIP_OPTION_TABARDDESIGNER:
            player->PlayerTalkClass->CloseGossip();
            player->GetSession()->SendTabardVendorActivate( guid );
            break;
        case GOSSIP_OPTION_AUCTIONEER:
            player->GetSession()->SendAuctionHello( guid, this );
            break;
        case GOSSIP_OPTION_SPIRITGUIDE:
        case GOSSIP_GUARD_SPELLTRAINER:
        case GOSSIP_GUARD_SKILLTRAINER:
            prepareGossipMenu( player,gossip->Id );
            sendPreparedGossip( player );
            break;
        case GOSSIP_OPTION_BATTLEFIELD:
        {
            uint32 bgTypeId = objmgr.GetBattleMasterBG(GetEntry());
            player->GetSession()->SendBattlegGroundList( GetGUID(), bgTypeId );
            break;
        }
        case GOSSIP_OPTION_HALLOWS_END:
        {
            player->PlayerTalkClass->CloseGossip();
            if (!player->HasAura(24755)) {
                player->CastSpell(player, 24755, true);

                // Either trick or treat, 50% chance
                if (rand()%2) 
                    player->CastSpell(player, 24715, true);
                else
                {
                    uint32 trickspell = 0;
                    switch (rand()%9) {                             // note that female characters can get male costumes and vice versa
                    case 0:
                        trickspell = 24753;                       // cannot cast, random 30sec
                        break;
                    case 1:
                        trickspell = 24713;                       // lepper gnome costume
                        break;
                    case 2:
                        trickspell = 24735;                       // male ghost costume
                        break;
                    case 3:
                        trickspell = 24736;                       // female ghostcostume
                        break;
                    case 4:
                        trickspell = 24710;                       // male ninja costume
                        break;
                    case 5:
                        trickspell = 24711;                       // female ninja costume
                        break;
                    case 6:
                        trickspell = 24708;                       // male pirate costume
                        break;
                    case 7:
                        trickspell = 24709;                       // female pirate costume
                        break;
                    case 8:
                        trickspell = 24723;                       // skeleton costume
                        break;
                    }
                    player->CastSpell(player, trickspell, true);
                }
            }
            break;
        }
        default:
            OnPoiSelect( player, gossip );
            break;
    }

}

void Creature::OnPoiSelect(Player* player, GossipOption const *gossip)
{
    if(gossip->GossipId==GOSSIP_GUARD_SPELLTRAINER || gossip->GossipId==GOSSIP_GUARD_SKILLTRAINER)
    {
        Poi_Icon icon = ICON_POI_0;
        //need add more case.
        switch(gossip->Action)
        {
            case GOSSIP_GUARD_BANK:
                icon=ICON_POI_HOUSE;
                break;
            case GOSSIP_GUARD_RIDE:
                icon=ICON_POI_RWHORSE;
                break;
            case GOSSIP_GUARD_GUILD:
                icon=ICON_POI_BLUETOWER;
                break;
            default:
                icon=ICON_POI_TOWER;
                break;
        }
        uint32 textid = GetGossipTextId( gossip->Action, GetZoneId() );
        player->PlayerTalkClass->SendTalking(textid);
        // std::string areaname= gossip->OptionText;
        // how this could worked player->PlayerTalkClass->SendPointOfInterest( x, y, icon, 2, 15, areaname.c_str() );
    }
}

uint32 Creature::GetGossipTextId(uint32 action, uint32 zoneid)
{
    QueryResult *result= WorldDatabase.PQuery("SELECT textid FROM npc_gossip_textid WHERE action = '%u' AND zoneid ='%u'", action, zoneid );

    if(!result)
        return 0;

    Field *fields = result->Fetch();
    uint32 id = fields[0].GetUInt32();

    delete result;

    return id;
}

uint32 Creature::GetNpcTextId()
{
    // don't cache / use cache in case it's a world event announcer
    if(GetCreatureTemplate()->flags_extra & CREATURE_FLAG_EXTRA_WORLDEVENT)
        if(uint32 textid = gameeventmgr.GetNpcTextId(m_DBTableGuid))
            return textid;

    if (!m_DBTableGuid)
        return DEFAULT_GOSSIP_MESSAGE;

    if(uint32 pos = objmgr.GetNpcGossip(m_DBTableGuid))
        return pos;

    return DEFAULT_GOSSIP_MESSAGE;
}

GossipOption const* Creature::GetGossipOption( uint32 id ) const
{
    for( GossipOptionList::const_iterator i = m_goptions.begin( ); i != m_goptions.end( ); ++i )
    {
        if(i->Action==id )
            return &*i;
    }
    return NULL;
}

void Creature::ResetGossipOptions()
{
    m_gossipOptionLoaded = false;
    m_goptions.clear();
}

void Creature::LoadGossipOptions()
{
    if(m_gossipOptionLoaded)
        return;

    uint32 npcflags=GetUInt32Value(UNIT_NPC_FLAGS);

    CacheNpcOptionList const& noList = objmgr.GetNpcOptions ();
    for (CacheNpcOptionList::const_iterator i = noList.begin (); i != noList.end (); ++i)
        if(i->NpcFlag & npcflags)
            AddGossipOption(*i);

    m_gossipOptionLoaded = true;
}

Player *Creature::GetLootRecipient() const
{
    if (!m_lootRecipient) return NULL;
    else return ObjectAccessor::FindPlayer(m_lootRecipient);
}

void Creature::SetLootRecipient(Unit *unit)
{
    // set the player whose group should receive the right
    // to loot the creature after it dies
    // should be set to NULL after the loot disappears

    if (!unit)
    {
        m_lootRecipient = 0;
        RemoveFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);
        RemoveFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_OTHER_TAGGER);
        return;
    }

    Player* player = unit->GetCharmerOrOwnerPlayerOrPlayerItself();
    if(!player)                                             // normal creature, no player involved
        return;

    m_lootRecipient = player->GetGUID();
    SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_OTHER_TAGGER);
}

void Creature::SaveToDB()
{
    // this should only be used when the creature has already been loaded
    // preferably after adding to map, because mapid may not be valid otherwise
    CreatureData const *data = objmgr.GetCreatureData(m_DBTableGuid);
    if(!data)
    {
        sLog.outError("Creature::SaveToDB failed, cannot get creature data!");
        return;
    }

    SaveToDB(GetMapId(), data->spawnMask);
}

void Creature::SaveToDB(uint32 mapid, uint8 spawnMask)
{
    // update in loaded data
    if (!m_DBTableGuid)
        m_DBTableGuid = GetGUIDLow();

    CreatureData& data = objmgr.NewOrExistCreatureData(m_DBTableGuid);

    uint32 displayId = GetNativeDisplayId();

    CreatureTemplate const *cinfo = GetCreatureTemplate();
    if(cinfo)
    {
        // check if it's a custom model and if not, use 0 for displayId
        if(displayId == cinfo->Modelid_A1 || displayId == cinfo->Modelid_A2 ||
            displayId == cinfo->Modelid_H1 || displayId == cinfo->Modelid_H2) displayId = 0;

        if(objmgr.isUsingAlternateGuidGeneration() && m_DBTableGuid > objmgr.getAltCreatureGuidStartIndex())
            sLog.outError("Creature with guid %u (entry %u) in temporary range was saved to database.",m_DBTableGuid,cinfo->Entry); 
    }

    // data->guid = guid don't must be update at save
    data.id = GetEntry();
    data.mapid = mapid;
    data.displayid = displayId;
    data.equipmentId = GetEquipmentId();
    if(!GetTransport())
        GetPosition(data.posX,data.posY,data.posZ,data.orientation);
    else {
        data.posX = GetTransOffsetX();
        data.posY = GetTransOffsetY();
        data.posZ = GetTransOffsetZ();
        data.orientation = GetTransOffsetO();
    }
    data.spawntimesecs = m_respawnDelay;
    // prevent add data integrity problems
    data.spawndist = GetDefaultMovementType()==IDLE_MOTION_TYPE ? 0 : m_respawnradius;
    data.currentwaypoint = 0;
    data.curhealth = GetHealth();
    data.curmana = GetPower(POWER_MANA);
    data.is_dead = m_isDeadByDefault;
    // prevent add data integrity problems
    data.movementType = !m_respawnradius && GetDefaultMovementType()==RANDOM_MOTION_TYPE
        ? IDLE_MOTION_TYPE : GetDefaultMovementType();
    data.spawnMask = spawnMask;
    data.poolId = m_creaturePoolId;

    // updated in DB
    SQLTransaction trans = WorldDatabase.BeginTransaction();

    trans->PAppend("DELETE FROM creature WHERE guid = '%u'", m_DBTableGuid);

    std::ostringstream ss;
    ss << "INSERT INTO creature (guid,id,map,spawnMask,modelid,equipment_id,position_x,position_y,position_z,orientation,spawntimesecs,spawndist,currentwaypoint,curhealth,curmana,DeathState,MovementType, pool_id) VALUES ("
        << m_DBTableGuid << ","
        << GetEntry() << ","
        << mapid <<","
        << (uint32)spawnMask << ","
        << displayId <<","
        << GetEquipmentId() <<","
        << data.posX << ","
        << data.posY << ","
        << data.posZ << ","
        << data.orientation << ","
        << m_respawnDelay << ","                            //respawn time
        << (float) m_respawnradius << ","                   //spawn distance (float)
        << (uint32) (0) << ","                              //currentwaypoint
        << GetHealth() << ","                               //curhealth
        << GetPower(POWER_MANA) << ","                      //curmana
        << (m_isDeadByDefault ? 1 : 0) << ","               //is_dead
        << GetDefaultMovementType() << ","                  //default movement generator type
        << m_creaturePoolId << ")";                          //creature pool id

    trans->Append( ss.str( ).c_str( ) );

    WorldDatabase.CommitTransaction(trans);

    if(objmgr.IsInTemporaryGuidRange(HIGHGUID_UNIT,m_DBTableGuid))
        sLog.outError("Creature %u has been saved but was in temporary guid range ! fixmefixmefixme", m_DBTableGuid);
}

void Creature::SelectLevel()
{
    const CreatureTemplate *cinfo = GetCreatureTemplate();
    if(!cinfo)
        return;

    uint32 rank = IsPet()? 0 : cinfo->rank;

    // level
    uint32 minlevel = std::min(cinfo->maxlevel, cinfo->minlevel);
    uint32 maxlevel = std::max(cinfo->maxlevel, cinfo->minlevel);
    uint32 level = minlevel == maxlevel ? minlevel : urand(minlevel, maxlevel);
    SetLevel(level);

    float rellevel = maxlevel == minlevel ? 0 : (float(level - minlevel))/(maxlevel - minlevel);

    uint32 minhealth = std::min(cinfo->maxhealth, cinfo->minhealth);
    uint32 maxhealth = std::max(cinfo->maxhealth, cinfo->minhealth);
    uint32 health = uint32(minhealth + uint32(rellevel*(maxhealth - minhealth)));

    SetCreateHealth(health);
    SetMaxHealth(health);
    SetHealth(health);
    ResetPlayerDamageReq();

    // mana
    uint32 minmana = std::min(cinfo->maxmana, cinfo->minmana);
    uint32 maxmana = std::max(cinfo->maxmana, cinfo->minmana);
    uint32 mana = minmana + uint32(rellevel*(maxmana - minmana));

    SetCreateMana(mana);
    SetMaxPower(POWER_MANA, mana);                          //MAX Mana
    SetPower(POWER_MANA, mana);

    SetModifierValue(UNIT_MOD_HEALTH, BASE_VALUE, health);
    SetModifierValue(UNIT_MOD_MANA, BASE_VALUE, mana);

    SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, cinfo->mindmg);
    SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, cinfo->maxdmg);
    SetBaseWeaponDamage(OFF_ATTACK, MINDAMAGE, cinfo->mindmg);
    SetBaseWeaponDamage(OFF_ATTACK, MAXDAMAGE, cinfo->maxdmg);
    SetBaseWeaponDamage(RANGED_ATTACK, MINDAMAGE, cinfo->minrangedmg);
    SetBaseWeaponDamage(RANGED_ATTACK, MAXDAMAGE, cinfo->maxrangedmg);

    // this value is not accurate, but should be close to the real value
    SetModifierValue(UNIT_MOD_ATTACK_POWER, BASE_VALUE, cinfo->attackpower);
    SetModifierValue(UNIT_MOD_ATTACK_POWER_RANGED, BASE_VALUE, cinfo->rangedattackpower);
}

bool Creature::CreateFromProto(uint32 guidlow, uint32 Entry, uint32 team, const CreatureData *data)
{
    CreatureTemplate const *cinfo = objmgr.GetCreatureTemplate(Entry);
    if(!cinfo)
    {
        sLog.outErrorDb("Error: creature entry %u does not exist.", Entry);
        return false;
    }
    m_originalEntry = Entry;

    Object::_Create(guidlow, Entry, HIGHGUID_UNIT);

    if(!UpdateEntry(Entry, team, data))
        return false;

    //Notify the map's instance data.
    //Only works if you create the object in it, not if it is moves to that map.
    //Normally non-players do not teleport to other maps.
    Map *map = FindMap();
    if(map && map->IsDungeon() && ((InstanceMap*)map)->GetInstanceData())
    {
        // Workaround, I need position_x in OnCreatureCreate for Felmyst. I'll rewrite the hook with data as third parameter later
        if (data)
            m_positionX = data->posX;
        ((InstanceMap*)map)->GetInstanceData()->OnCreatureCreate(this, Entry);
    }

    return true;
}

bool Creature::LoadFromDB(uint32 guid, Map *map)
{
    CreatureData const* data = objmgr.GetCreatureData(guid);

    if(!data)
    {
        sLog.outErrorDb("Creature (GUID: %u) not found in table `creature`, can't load. ",guid);
        return false;
    }
    
    // Rare creatures in dungeons have 15% chance to spawn
    CreatureTemplate const *cinfo = objmgr.GetCreatureTemplate(data->id);
    if (cinfo && map->GetInstanceId() != 0 && (cinfo->rank == 2 || cinfo->rank == 4)) {
        if (rand()%5 != 0)
            return false;
    }

    m_DBTableGuid = guid;
    if (map->GetInstanceId() != 0) guid = objmgr.GenerateLowGuid(HIGHGUID_UNIT,true);

    uint16 team = 0;
    if(!Create(guid,map,data->id,team,data))
        return false;

    Relocate(data->posX,data->posY,data->posZ,data->orientation);

    if(!IsPositionValid())
    {
        sLog.outError("ERROR: Creature (guidlow %d, entry %d) not loaded. Suggested coordinates isn't valid (X: %f Y: %f)",GetGUIDLow(),GetEntry(),GetPositionX(),GetPositionY());
        return false;
    }
    //We should set first home position, because then AI calls home movement
    SetHomePosition(data->posX,data->posY,data->posZ,data->orientation);

    m_respawnradius = data->spawndist;

    m_respawnDelay = data->spawntimesecs;
    m_isDeadByDefault = data->is_dead;
    m_deathState = m_isDeadByDefault ? DEAD : ALIVE;

    m_respawnTime  = objmgr.GetCreatureRespawnTime(m_DBTableGuid,GetInstanceId());
    if(m_respawnTime)                          // respawn on Update
    {
        m_deathState = DEAD;
        if(CanFly())
        {
            float tz = GetMap()->GetHeight(data->posX,data->posY,data->posZ,false);
            if(data->posZ - tz > 0.1)
                Relocate(data->posX,data->posY,tz);
        }
    }

    uint32 curhealth = data->curhealth;

    SetHealth(m_deathState == ALIVE ? curhealth : 0);
    SetPower(POWER_MANA,data->curmana);

    // checked at creature_template loading
    m_defaultMovementType = MovementGeneratorType(data->movementType);

    AIM_Initialize();
    return true;
}

void Creature::LoadEquipment(uint32 equip_entry, bool force)
{
    if(equip_entry == 0)
    {
        if (force)
        {
            for (uint8 i = 0; i < 3; i++)
            {
                SetUInt32Value( UNIT_VIRTUAL_ITEM_SLOT_DISPLAY + i, 0);
                SetUInt32Value( UNIT_VIRTUAL_ITEM_INFO + (i * 2), 0);
                SetUInt32Value( UNIT_VIRTUAL_ITEM_INFO + (i * 2) + 1, 0);
            }
            m_equipmentId = 0;
        }
        return;
    }

    EquipmentInfo const *einfo = objmgr.GetEquipmentInfo(equip_entry);
    if (!einfo)
        return;

    m_equipmentId = equip_entry;
    for (uint8 i = 0; i < 3; i++)
    {
        SetUInt32Value( UNIT_VIRTUAL_ITEM_SLOT_DISPLAY + i, einfo->equipmodel[i]);
        SetUInt32Value( UNIT_VIRTUAL_ITEM_INFO + (i * 2), einfo->equipinfo[i]);
        SetUInt32Value( UNIT_VIRTUAL_ITEM_INFO + (i * 2) + 1, einfo->equipslot[i]);
    }
}

bool Creature::hasQuest(uint32 quest_id) const
{
    QuestRelations const& qr = objmgr.mCreatureQuestRelations;
    for(QuestRelations::const_iterator itr = qr.lower_bound(GetEntry()); itr != qr.upper_bound(GetEntry()); ++itr)
    {
        if(itr->second==quest_id)
            return true;
    }
    return false;
}

bool Creature::hasInvolvedQuest(uint32 quest_id) const
{
    QuestRelations const& qr = objmgr.mCreatureQuestInvolvedRelations;
    for(QuestRelations::const_iterator itr = qr.lower_bound(GetEntry()); itr != qr.upper_bound(GetEntry()); ++itr)
    {
        if(itr->second==quest_id)
            return true;
    }
    return false;
}

void Creature::DeleteFromDB()
{
    if (!m_DBTableGuid)
        return;

    objmgr.SaveCreatureRespawnTime(m_DBTableGuid,GetInstanceId(),0);
    objmgr.DeleteCreatureData(m_DBTableGuid);

    SQLTransaction trans = WorldDatabase.BeginTransaction();
    trans->PAppend("DELETE FROM creature WHERE guid = '%u'", m_DBTableGuid);
    trans->PAppend("DELETE FROM creature_addon WHERE guid = '%u'", m_DBTableGuid);
    trans->PAppend("DELETE FROM game_event_creature WHERE guid = '%u'", m_DBTableGuid);
    trans->PAppend("DELETE FROM game_event_model_equip WHERE guid = '%u'", m_DBTableGuid);
    WorldDatabase.CommitTransaction(trans);
}

bool Creature::CanSeeOrDetect(Unit const* u, bool detect, bool inVisibleList, bool is3dDistance) const
{
    // not in world
    if(!IsInWorld() || !u->IsInWorld())
        return false;

    // all dead creatures/players not visible for any creatures
    if(!u->IsAlive() || !IsAlive())
        return false;

    // Always can see self
    if (u == this)
        return true;

    // always seen by owner
    if(GetGUID() == u->GetCharmerOrOwnerGUID())
        return true;

    if(u->GetVisibility() == VISIBILITY_OFF) //GM
        return false;

    // invisible aura
    if((m_invisibilityMask || u->m_invisibilityMask) && !CanDetectInvisibilityOf(u))
        return false;

    // unit got in stealth in this moment and must ignore old detected state
    //if (m_Visibility == VISIBILITY_GROUP_NO_DETECT)
    //    return false;

    // GM invisibility checks early, invisibility if any detectable, so if not stealth then visible
    if(u->GetVisibility() == VISIBILITY_GROUP_STEALTH)
    {
        //do not know what is the use of this detect
        if(!detect || !CanDetectStealthOf(u, GetDistance(u)))
            return false;
    }

    // Now check is target visible with LoS
    //return u->IsWithinLOS(GetPositionX(),GetPositionY(),GetPositionZ());
    return true;
}

bool Creature::IsWithinSightDist(Unit const* u) const
{
    return IsWithinDistInMap(u, sWorld.getConfig(CONFIG_SIGHT_MONSTER));
}

bool Creature::canStartAttack(Unit const* who) const
{
    if(isCivilian()
        || !who->isInAccessiblePlaceFor(this)
        || !CanFly() && GetDistanceZ(who) > CREATURE_Z_ATTACK_RANGE
        || !IsWithinDistInMap(who, GetAttackDistance(who)))
        return false;

    if(!CanAttack(who, false))
        return false;

    return IsWithinLOSInMap(who);
}

float Creature::GetAttackDistance(Unit const* pl) const
{
    float aggroRate = sWorld.GetRate(RATE_CREATURE_AGGRO);
    if(aggroRate==0)
        return 0.0f;

    int32 playerlevel   = pl->GetLevelForTarget(this);
    int32 creaturelevel = GetLevelForTarget(pl);

    int32 leveldif       = playerlevel - creaturelevel;

    // "The maximum Aggro Radius has a cap of 25 levels under. Example: A level 30 char has the same Aggro Radius of a level 5 char on a level 60 mob."
    if ( leveldif < - 25)
        leveldif = -25;

    // "The aggro radius of a mob having the same level as the player is roughly 20 yards"
    float RetDistance = 20;

    // "Aggro Radius varies with level difference at a rate of roughly 1 yard/level"
    // radius grow if playlevel < creaturelevel
    RetDistance -= (float)leveldif;

    /*if(creaturelevel+5 <= sWorld.getConfig(CONFIG_MAX_PLAYER_LEVEL))
    {*/
        // detect range auras
        RetDistance += GetTotalAuraModifier(SPELL_AURA_MOD_DETECT_RANGE);

        // detected range auras
        RetDistance += pl->GetTotalAuraModifier(SPELL_AURA_MOD_DETECTED_RANGE);
    //}

    // "Minimum Aggro Radius for a mob seems to be combat range (5 yards)"
    if(RetDistance < 5)
        RetDistance = 5;

    return (RetDistance*aggroRate);
}

void Creature::SetDeathState(DeathState s)
{
    if((s == JUST_DIED && !m_isDeadByDefault)||(s == JUST_RESPAWNED && m_isDeadByDefault))
    {
        m_corpseRemoveTime = time(NULL) + m_corpseDelay;
        m_respawnTime = time(NULL) + m_respawnDelay + m_corpseDelay;

        // always save boss respawn time at death to prevent crash cheating
        if(sWorld.getConfig(CONFIG_SAVE_RESPAWN_TIME_IMMEDIATELY) || isWorldBoss())
            SaveRespawnTime();
            
        Map *map = FindMap();
        if(map && map->IsDungeon() && ((InstanceMap*)map)->GetInstanceData())
            ((InstanceMap*)map)->GetInstanceData()->OnCreatureDeath(this);

        if (CanFly() && FallGround())
            return;
    }
    Unit::SetDeathState(s);

    if(s == JUST_DIED)
    {
        SetUInt64Value (UNIT_FIELD_TARGET,0);               // remove target selection in any cases (can be set at aura remove in Unit::setDeathState)
        SetUInt32Value(UNIT_NPC_FLAGS, 0);
        //if(!IsPet())
            setActive(false);

        if(!IsPet() && GetCreatureTemplate()->SkinLootId)
            if ( LootTemplates_Skinning.HaveLootFor(GetCreatureTemplate()->SkinLootId) )
                SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE);

        if ((CanFly() || IsFlying()))
            GetMotionMaster()->MoveFall();

        Unit::SetDeathState(CORPSE);
    }
    if(s == JUST_RESPAWNED)
    {
        SetHealth(GetMaxHealth());
        SetLootRecipient(NULL);
        ResetPlayerDamageReq();

        UpdateMovementFlags();

        RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE);
        AddUnitMovementFlag(MOVEMENTFLAG_WALKING);
        SetUInt32Value(UNIT_NPC_FLAGS, GetCreatureTemplate()->npcflag);
        ClearUnitState(uint32(UNIT_STATE_ALL_STATE & ~UNIT_STATE_IGNORE_PATHFINDING));
        Motion_Initialize();
        SetMeleeDamageSchool(SpellSchools(GetCreatureTemplate()->dmgschool));
        InitCreatureAddon(true);
        Unit::SetDeathState(ALIVE);
    }
}

bool Creature::FallGround()
{
    // Let's abort after we called this function one time
    if (GetDeathState() == DEAD_FALLING)
        return false;

    if(ToTemporarySummon() && ToTemporarySummon()->DespawnOnDeath())
        return false;

    float x, y, z;
    GetPosition(x, y, z);
    float ground_Z = GetMap()->GetHeight(x, y, z);
    UpdateAllowedPositionZ(x, y, z);
    if (fabs(ground_Z - z) < 0.1f)
        return false;

    Unit::SetDeathState(DEAD_FALLING);
    GetMotionMaster()->MovePoint(0, x, y, ground_Z);
    Relocate(x, y, ground_Z);
    return true;
}

void Creature::Respawn()
{
    RemoveCorpse(false);

    if(!IsInWorld())
        AddToWorld();

    // forced recreate creature object at clients
    UnitVisibility currentVis = GetVisibility();
    SetVisibility(VISIBILITY_RESPAWN);
    ObjectAccessor::UpdateObjectVisibility(this);
    SetVisibility(currentVis);                              // restore visibility state
    ObjectAccessor::UpdateObjectVisibility(this);
    this->loot.ClearRemovedItemsList();

    if(GetDeathState()==DEAD)
    {
        if (m_DBTableGuid)
            objmgr.SaveCreatureRespawnTime(m_DBTableGuid,GetInstanceId(),0);

        DEBUG_LOG("Respawning...");
        m_respawnTime = 0;
        lootForPickPocketed = false;
        lootForBody         = false;

        if(m_originalEntry != GetEntry())
            UpdateEntry(m_originalEntry);

        SelectLevel();

        if (m_isDeadByDefault)
        {
            SetDeathState(JUST_DIED);
            SetHealth(0);
            ClearUnitState(UNIT_STATE_ALL_STATE);
            InitCreatureAddon(true);
        }
        else
            SetDeathState( JUST_RESPAWNED );

        GetMotionMaster()->InitDefault();

        //re rand level & model
        SelectLevel();

        uint32 displayID = GetNativeDisplayId();
        CreatureModelInfo const* minfo = objmgr.GetCreatureModelRandomGender(&displayID);
        if (minfo)                                             // Cancel load if no model defined
        {
            SetDisplayId(displayID);
            SetNativeDisplayId(displayID);
            SetByteValue(UNIT_FIELD_BYTES_0, 2, minfo->gender);
        }

        //Call AI respawn virtual function
        if (IsAIEnabled)
            TriggerJustRespawned = true;//delay event to next tick so all creatures are created on the map before processing
    }
    
    m_timeSinceSpawn = 0;
}

void Creature::ForcedDespawn(uint32 timeMSToDespawn)
{
    if (timeMSToDespawn)
    {
        ForcedDespawnDelayEvent* pEvent = new ForcedDespawnDelayEvent(*this);

        m_Events.AddEvent(pEvent, m_Events.CalculateTime(timeMSToDespawn));
        return;
    }
    
    RemoveFromWorld();
    SetDeathState(JUST_DIED);
    RemoveCorpse(false);
    SetHealth(0);                                           // just for nice GM-mode view
}

bool Creature::IsImmunedToSpell(SpellEntry const* spellInfo, bool useCharges)
{
    if (!spellInfo)
        return false;

    if (GetCreatureTemplate()->MechanicImmuneMask & (1 << (spellInfo->Mechanic - 1)))
        return true;

    return Unit::IsImmunedToSpell(spellInfo, useCharges);
}

bool Creature::IsImmunedToSpellEffect(uint32 effect, uint32 mechanic) const
{
    if (GetCreatureTemplate()->MechanicImmuneMask & (1 << (mechanic-1)))
        return true;

    return Unit::IsImmunedToSpellEffect(effect, mechanic);
}

void Creature::ProhibitSpellSchool(SpellSchoolMask idSchoolMask, uint32 unTimeMs)
{
    if (idSchoolMask & SPELL_SCHOOL_MASK_NORMAL)
        m_prohibitedSchools[SPELL_SCHOOL_NORMAL] = unTimeMs;
    if (idSchoolMask & SPELL_SCHOOL_MASK_HOLY)
        m_prohibitedSchools[SPELL_SCHOOL_HOLY] = unTimeMs;
    if (idSchoolMask & SPELL_SCHOOL_MASK_FIRE)
        m_prohibitedSchools[SPELL_SCHOOL_FIRE] = unTimeMs;
    if (idSchoolMask & SPELL_SCHOOL_MASK_NATURE)
        m_prohibitedSchools[SPELL_SCHOOL_NATURE] = unTimeMs;
    if (idSchoolMask & SPELL_SCHOOL_MASK_FROST)
        m_prohibitedSchools[SPELL_SCHOOL_FROST] = unTimeMs;
    if (idSchoolMask & SPELL_SCHOOL_MASK_SHADOW)
        m_prohibitedSchools[SPELL_SCHOOL_ARCANE] = unTimeMs;
    if (idSchoolMask & SPELL_SCHOOL_MASK_ARCANE)
        m_prohibitedSchools[SPELL_SCHOOL_ARCANE] = unTimeMs;
}

void Creature::UpdateProhibitedSchools(uint32 const diff)
{
    for (uint8 i = 0; i < MAX_SPELL_SCHOOL; i++) {
        if (m_prohibitedSchools[i] == 0)
            continue;

        if (m_prohibitedSchools[i] <= diff) {
            m_prohibitedSchools[i] = 0;
            continue;
        }

        m_prohibitedSchools[i] -= diff;
    }
}

bool Creature::IsSpellSchoolMaskProhibited(SpellSchoolMask idSchoolMask)
{
    bool prohibited = false;

    for (int i = 0; i < MAX_SPELL_SCHOOL; i++) {
        if ((idSchoolMask & (1 << i)) && m_prohibitedSchools[i] > 0) {
            prohibited = true;
            break;
        }
    }

    return prohibited;
}

SpellEntry const *Creature::reachWithSpellAttack(Unit *pVictim)
{
    if(!pVictim)
        return NULL;

    for(uint32 i=0; i < CREATURE_MAX_SPELLS; i++)
    {
        if(!m_spells[i])
            continue;
        SpellEntry const *spellInfo = spellmgr.LookupSpell(m_spells[i] );
        if(!spellInfo)
        {
            sLog.outError("WORLD: unknown spell id %i\n", m_spells[i]);
            continue;
        }

        bool bcontinue = true;
        for(uint32 j=0;j<3;j++)
        {
            if( (spellInfo->Effect[j] == SPELL_EFFECT_SCHOOL_DAMAGE )       ||
                (spellInfo->Effect[j] == SPELL_EFFECT_INSTAKILL)            ||
                (spellInfo->Effect[j] == SPELL_EFFECT_ENVIRONMENTAL_DAMAGE) ||
                (spellInfo->Effect[j] == SPELL_EFFECT_HEALTH_LEECH )
                )
            {
                bcontinue = false;
                break;
            }
        }
        if(bcontinue) continue;

        if(spellInfo->manaCost > GetPower(POWER_MANA))
            continue;
        SpellRangeEntry const* srange = sSpellRangeStore.LookupEntry(spellInfo->rangeIndex);
        float range = GetSpellMaxRange(srange);
        float minrange = GetSpellMinRange(srange);
        float dist = GetDistance(pVictim);
        //if(!isInFront( pVictim, range ) && spellInfo->AttributesEx )
        //    continue;
        if( dist > range || dist < minrange )
            continue;
        if(HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SILENCED))
            continue;
        return spellInfo;
    }
    return NULL;
}

SpellEntry const *Creature::reachWithSpellCure(Unit *pVictim)
{
    if(!pVictim)
        return NULL;

    for(uint32 i=0; i < CREATURE_MAX_SPELLS; i++)
    {
        if(!m_spells[i])
            continue;
        SpellEntry const *spellInfo = spellmgr.LookupSpell(m_spells[i] );
        if(!spellInfo)
        {
            sLog.outError("WORLD: unknown spell id %i\n", m_spells[i]);
            continue;
        }

        bool bcontinue = true;
        for(uint32 j=0;j<3;j++)
        {
            if( (spellInfo->Effect[j] == SPELL_EFFECT_HEAL ) )
            {
                bcontinue = false;
                break;
            }
        }
        if(bcontinue) continue;

        if(spellInfo->manaCost > GetPower(POWER_MANA))
            continue;
        SpellRangeEntry const* srange = sSpellRangeStore.LookupEntry(spellInfo->rangeIndex);
        float range = GetSpellMaxRange(srange);
        float minrange = GetSpellMinRange(srange);
        float dist = GetDistance(pVictim);
        //if(!isInFront( pVictim, range ) && spellInfo->AttributesEx )
        //    continue;
        if( dist > range || dist < minrange )
            continue;
        if(HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SILENCED))
            continue;
        return spellInfo;
    }
    return NULL;
}

bool Creature::IsVisibleInGridForPlayer(Player const* pl) const
{
    // gamemaster in GM mode see all, including ghosts
    if(pl->IsGameMaster() || pl->isSpectator())
        return true;

    // CREATURE_FLAG_EXTRA_ALIVE_INVISIBLE handling
    if(GetCreatureTemplate()->flags_extra & CREATURE_FLAG_EXTRA_ALIVE_INVISIBLE)
        return pl->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST);

    // Live player (or with not release body see live creatures or death creatures with corpse disappearing time > 0
    if(pl->IsAlive() || pl->GetDeathTimer() > 0)
    {
        if( GetEntry() == VISUAL_WAYPOINT && !pl->IsGameMaster() )
            return false;
        return IsAlive() || m_corpseRemoveTime > time(NULL) || m_isDeadByDefault && m_deathState==CORPSE;
    }

    // Dead player see creatures near own corpse
    Corpse *corpse = pl->GetCorpse();
    if (corpse) {
        // 20 - aggro distance for same level, 25 - max additional distance if player level less that creature level
        if (corpse->IsWithinDistInMap(this,(20+25)*sWorld.GetRate(RATE_CREATURE_AGGRO)))
            return true;
    }

    // Dead player see Spirit Healer or Spirit Guide
    if(isSpiritService())
        return true;

    // and not see any other
    return false;
}

void Creature::DoFleeToGetAssistance(float radius) // Optional parameter
{
    if (!GetVictim())
        return;
        
    if (HasUnitState(UNIT_STATE_STUNNED))
        return;

    if (HasAuraType(SPELL_AURA_PREVENTS_FLEEING))
        return;

    if (radius <= 0)
        return;
        
    if (IsNonMeleeSpellCast(false))
        InterruptNonMeleeSpells(true);

    Creature* pCreature = NULL;

    CellPair p(Trinity::ComputeCellPair(GetPositionX(), GetPositionY()));
    Cell cell(p);
    cell.data.Part.reserved = ALL_DISTRICT;
    cell.SetNoCreate();
    Trinity::NearestAssistCreatureInCreatureRangeCheck u_check(this,GetVictim(),radius);
    Trinity::CreatureLastSearcher<Trinity::NearestAssistCreatureInCreatureRangeCheck> searcher(pCreature, u_check);

    TypeContainerVisitor<Trinity::CreatureLastSearcher<Trinity::NearestAssistCreatureInCreatureRangeCheck>, GridTypeMapContainer > grid_creature_searcher(searcher);
    cell.Visit(p, grid_creature_searcher, *GetMap(), *this, radius);

    if(!pCreature)
        SetControlled(true, UNIT_STATE_FLEEING);
    else
        GetMotionMaster()->MoveSeekAssistance(pCreature->GetPositionX(), pCreature->GetPositionY(), pCreature->GetPositionZ());
}

Unit* Creature::SelectNearestTarget(float dist, bool playerOnly /* = false */, bool furthest /* = false */) const
{
    CellPair p(Trinity::ComputeCellPair(GetPositionX(), GetPositionY()));
    Cell cell(p);
    cell.data.Part.reserved = ALL_DISTRICT;
    cell.SetNoCreate();

    Unit *target = NULL;

    {
        Trinity::NearestHostileUnitInAttackDistanceCheck u_check(this, dist,playerOnly,furthest);
        Trinity::UnitLastSearcher<Trinity::NearestHostileUnitInAttackDistanceCheck> searcher(target, u_check);

        TypeContainerVisitor<Trinity::UnitLastSearcher<Trinity::NearestHostileUnitInAttackDistanceCheck>, WorldTypeMapContainer > world_unit_searcher(searcher);
        TypeContainerVisitor<Trinity::UnitLastSearcher<Trinity::NearestHostileUnitInAttackDistanceCheck>, GridTypeMapContainer >  grid_unit_searcher(searcher);

        cell.Visit(p, world_unit_searcher, *GetMap(), *this, ATTACK_DISTANCE);
        cell.Visit(p, grid_unit_searcher, *GetMap(), *this, ATTACK_DISTANCE);
    }

    return target;
}

void Creature::CallAssistance()
{
    if( !m_AlreadyCallAssistance && GetVictim() && !IsPet() && !IsCharmed())
    {
        SetNoCallAssistance(true);

        float radius = sWorld.getConfig(CONFIG_CREATURE_FAMILY_ASSISTANCE_RADIUS);
        if(radius > 0)
        {
            std::list<Creature*> assistList;

            // Check near creatures for assistance
            CellPair p(Trinity::ComputeCellPair(GetPositionX(), GetPositionY()));
            Cell cell(p);
            cell.data.Part.reserved = ALL_DISTRICT;
            cell.SetNoCreate();

            Trinity::AnyAssistCreatureInRangeCheck u_check(this, GetVictim(), radius);
            Trinity::CreatureListSearcher<Trinity::AnyAssistCreatureInRangeCheck> searcher(assistList, u_check);

            TypeContainerVisitor<Trinity::CreatureListSearcher<Trinity::AnyAssistCreatureInRangeCheck>, GridTypeMapContainer >  grid_creature_searcher(searcher);

            cell.Visit(p, grid_creature_searcher, *GetMap());

            // Add creatures from linking DB system
            if (m_creaturePoolId) {
                std::list<Creature*> allCreatures = FindMap()->GetAllCreaturesFromPool(m_creaturePoolId);
                for (auto itr : allCreatures) 
                {
                    if (itr->IsAlive() && itr->IsInWorld())
                        assistList.push_back(itr);
                }
                if(allCreatures.size() == 0)
                    sLog.outError("Broken data in table creature_pool_relations for creature pool %u.", m_creaturePoolId);
            }

            if (!assistList.empty())
            {
                uint32 count = 0;
                AssistDelayEvent *e = new AssistDelayEvent(GetVictim()->GetGUID(), *this);
                while (!assistList.empty())
                {
                    ++count;
                    // Pushing guids because in delay can happen some creature gets despawned => invalid pointer
//                    sLog.outString("ASSISTANCE: calling creature at %p", *assistList.begin());
                    Creature *cr = *assistList.begin();
                    if (!cr->IsInWorld()) {
                        sLog.outError("ASSISTANCE: ERROR: creature is not in world");
                        assistList.pop_front();
                        continue;
                    }
                    cr->setActive(true);
                    e->AddAssistant((*assistList.begin())->GetGUID());
                    assistList.pop_front();
                    if (GetInstanceId() == 0 && count >= 4)
                        break;
                }
                m_Events.AddEvent(e, m_Events.CalculateTime(sWorld.getConfig(CONFIG_CREATURE_FAMILY_ASSISTANCE_DELAY)));
            }
        }
    }
}

bool Creature::CanAssistTo(const Unit* u, const Unit* enemy) const
{
    // is it true?
    if(!HasReactState(REACT_AGGRESSIVE) || (HasJustRespawned() && !m_summoner)) //ignore justrespawned if summoned
        return false;

    // we don't need help from zombies :)
    if( !IsAlive() )
        return false;

    // skip fighting creature
    if( IsInCombat() )
        return false;

    // only from same creature faction
    if(GetFaction() != u->GetFaction() )
        return false;

    // only free creature
    if( GetCharmerOrOwnerGUID() )
        return false;

    // skip non hostile to caster enemy creatures
    if( !IsHostileTo(enemy) )
        return false;

    // don't slay innocent critters
    if( enemy->GetTypeId() == CREATURE_TYPE_CRITTER )
        return false;

    return true;
}

void Creature::SaveRespawnTime()
{
    if(IsPet() || !m_DBTableGuid)
        return;

    objmgr.SaveCreatureRespawnTime(m_DBTableGuid,GetInstanceId(),m_respawnTime);
}

bool Creature::IsOutOfThreatArea(Unit* pVictim) const
{
    if(!pVictim)
        return true;

    if(!pVictim->IsInMap(this))
        return true;

    if(!CanAttack(pVictim))
        return true;

    if(!pVictim->isInAccessiblePlaceFor(this))
        return true;

    if (((Creature*)this)->IsCombatStationary() && !CanReachWithMeleeAttack(pVictim))
        return true;

    if(sMapStore.LookupEntry(GetMapId())->IsDungeon())
        return false;

    float length;
    if (GetCreatureTemplate()->flags_extra & CREATURE_FLAG_EXTRA_HOMELESS || ((Creature*)this)->IsBeingEscorted())
        length = pVictim->GetDistance(GetPositionX(), GetPositionY(), GetPositionZ());
    else {
        float x,y,z,o;
        GetHomePosition(x,y,z,o);
        length = pVictim->GetDistance(x, y, z);
    }
    float AttackDist = GetAttackDistance(pVictim);
    uint32 ThreatRadius = sWorld.getConfig(CONFIG_THREAT_RADIUS);

    //Use AttackDistance in distance check if threat radius is lower. This prevents creature bounce in and out of combat every update tick.
    return ( length > (ThreatRadius > AttackDist ? ThreatRadius : AttackDist));
}

void Creature::LoadCreatureAddon()
{
    if (m_DBTableGuid)
    {
        if(m_creatureInfoAddon = ObjectMgr::GetCreatureAddon(m_DBTableGuid))
            return;
    }

    m_creatureInfoAddon = ObjectMgr::GetCreatureTemplateAddon(GetCreatureTemplate()->Entry);
}

//creature_addon table
bool Creature::InitCreatureAddon(bool reload)
{
    if(!m_creatureInfoAddon)
        return false;

    if (m_creatureInfoAddon->mount != 0)
        Mount(m_creatureInfoAddon->mount);

    if (m_creatureInfoAddon->bytes0 != 0)
        SetUInt32Value(UNIT_FIELD_BYTES_0, m_creatureInfoAddon->bytes0);

    if (m_creatureInfoAddon->bytes1 != 0)
        SetUInt32Value(UNIT_FIELD_BYTES_1, m_creatureInfoAddon->bytes1);

    if (m_creatureInfoAddon->bytes2 != 0)
        SetUInt32Value(UNIT_FIELD_BYTES_2, m_creatureInfoAddon->bytes2);

    if (m_creatureInfoAddon->emote != 0)
        SetUInt32Value(UNIT_NPC_EMOTESTATE, m_creatureInfoAddon->emote);

    if (m_creatureInfoAddon->move_flags != 0)
        SetUnitMovementFlags(m_creatureInfoAddon->move_flags);

    //Load Path
    if (m_creatureInfoAddon->path_id != 0)
        m_path_id = m_creatureInfoAddon->path_id;

    if(m_creatureInfoAddon->auras)
    {
        for (CreatureDataAddonAura const* cAura = m_creatureInfoAddon->auras; cAura->spell_id; ++cAura)
        {
            SpellEntry const *AdditionalSpellInfo = spellmgr.LookupSpell(cAura->spell_id);
            if (!AdditionalSpellInfo)
            {
                sLog.outErrorDb("Creature (GUIDLow: %u Entry: %u ) has wrong spell %u defined in `auras` field.",GetGUIDLow(),GetEntry(),cAura->spell_id);
                continue;
            }

            // skip already applied aura
            if(HasAura(cAura->spell_id,cAura->effect_idx))
            {
                if(!reload)
                    sLog.outErrorDb("Creature (GUIDLow: %u Entry: %u ) has duplicate aura (spell %u effect %u) in `auras` field.",GetGUIDLow(),GetEntry(),cAura->spell_id,cAura->effect_idx);

                continue;
            }

            Aura* AdditionalAura = CreateAura(AdditionalSpellInfo, cAura->effect_idx, NULL, this, this, 0);
            AddAura(AdditionalAura);
        }
    }
    return true;
}

/// Send a message to LocalDefense channel for players opposition team in the zone
void Creature::SendZoneUnderAttackMessage(Player* attacker)
{
    uint32 enemy_team = attacker->GetTeam();

    WorldPacket data(SMSG_ZONE_UNDER_ATTACK,4);
    data << (uint32)GetZoneId();
    sWorld.SendGlobalMessage(&data,NULL,(enemy_team==ALLIANCE ? HORDE : ALLIANCE));
}

void Creature::_AddCreatureSpellCooldown(uint32 spell_id, time_t end_time)
{
    m_CreatureSpellCooldowns[spell_id] = end_time;
}

void Creature::_AddCreatureCategoryCooldown(uint32 category, time_t apply_time)
{
    m_CreatureCategoryCooldowns[category] = apply_time;
}

void Creature::AddCreatureSpellCooldown(uint32 spellid)
{
    SpellEntry const *spellInfo = spellmgr.LookupSpell(spellid);
    if(!spellInfo)
        return;

    uint32 cooldown = GetSpellRecoveryTime(spellInfo);
    if(cooldown)
        _AddCreatureSpellCooldown(spellid, time(NULL) + cooldown/1000);

    if(spellInfo->Category)
        _AddCreatureCategoryCooldown(spellInfo->Category, time(NULL));

    m_GlobalCooldown = spellInfo->StartRecoveryTime;
}

bool Creature::HasCategoryCooldown(uint32 spell_id) const
{
    SpellEntry const *spellInfo = spellmgr.LookupSpell(spell_id);
    if(!spellInfo)
        return false;

    // check global cooldown if spell affected by it
    if (spellInfo->StartRecoveryCategory > 0 && m_GlobalCooldown > 0)
        return true;

    CreatureSpellCooldowns::const_iterator itr = m_CreatureCategoryCooldowns.find(spellInfo->Category);
    return(itr != m_CreatureCategoryCooldowns.end() && time_t(itr->second + (spellInfo->CategoryRecoveryTime / 1000)) > time(NULL));
}

bool Creature::HasSpellCooldown(uint32 spell_id) const
{
    CreatureSpellCooldowns::const_iterator itr = m_CreatureSpellCooldowns.find(spell_id);
    return (itr != m_CreatureSpellCooldowns.end() && itr->second > time(NULL)) || HasCategoryCooldown(spell_id);
}

bool Creature::HasSpell(uint32 spellID) const
{
    uint8 i;
    for(i = 0; i < CREATURE_MAX_SPELLS; ++i)
        if(spellID == m_spells[i])
            break;
    return i < CREATURE_MAX_SPELLS;                         //broke before end of iteration of known spells
}

time_t Creature::GetRespawnTimeEx() const
{
    time_t now = time(NULL);
    if (m_respawnTime > now)
        return m_respawnTime;
    else
        return now;
}

void Creature::GetRespawnPosition( float &x, float &y, float &z, float* ori, float* dist ) const
{
    if (m_DBTableGuid)
    {
        if (CreatureData const* data = objmgr.GetCreatureData(GetDBTableGUIDLow()))
        {
            x = data->posX;
            y = data->posY;
            z = data->posZ;
            if(ori)
                *ori = data->orientation;
            if(dist)
                *dist = data->spawndist;

            return;
        }
    }

    x = GetPositionX();
    y = GetPositionY();
    z = GetPositionZ();
    if(ori)
        *ori = GetOrientation();
    if(dist)
        *dist = 0;
}

void Creature::AllLootRemovedFromCorpse()
{
    if (!HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE))
    {
        // Do not reset corpse remove time if corpse is already removed
        time_t now = time(NULL);
        if(m_corpseRemoveTime <= now)
            return;
            
        float decayRate = sWorld.GetRate(RATE_CORPSE_DECAY_LOOTED);
        CreatureTemplate const *cinfo = GetCreatureTemplate();

        // corpse skinnable, but without skinning flag, and then skinned, corpse will despawn next update
        if (cinfo && cinfo->SkinLootId)
            m_corpseRemoveTime = now;
        else
            m_corpseRemoveTime = now + m_corpseDelay * decayRate;

        m_respawnTime = m_corpseRemoveTime + m_respawnDelay;
    }
}

uint32 Creature::GetLevelForTarget( Unit const* target ) const
{
    if(!isWorldBoss())
        return Unit::GetLevelForTarget(target);

    uint32 level = target->GetLevel()+sWorld.getConfig(CONFIG_WORLD_BOSS_LEVEL_DIFF);
    if(level < 1)
        return 1;
    if(level > 255)
        return 255;
    return level;
}

std::string Creature::GetScriptName()
{
    return objmgr.GetScriptName(GetScriptId());
}

// New
std::string Creature::getScriptName()
{
    std::string scriptName = "";
    
    if (CreatureData const* myData = objmgr.GetCreatureData(m_DBTableGuid)) {
        if (myData->scriptName != "")
            scriptName = myData->scriptName;
        else if (GetCreatureTemplate()->scriptName != "")
            scriptName = std::string(GetCreatureTemplate()->scriptName);
    }
    else if (GetCreatureTemplate()->scriptName != "")
            scriptName = std::string(GetCreatureTemplate()->scriptName);

    return scriptName;
}

uint32 Creature::getInstanceEventId()
{
    if (CreatureData const* myData = objmgr.GetCreatureData(m_DBTableGuid))
        return myData->instanceEventId;
        
    return 0;
}   

uint32 Creature::GetScriptId()
{
    return ObjectMgr::GetCreatureTemplate(GetEntry())->ScriptID;
    //return m_scriptId ? NULL : ObjectMgr::GetCreatureTemplate(GetEntry())->ScriptID;
}

std::string Creature::GetAIName() const
{
    return ObjectMgr::GetCreatureTemplate(GetEntry())->AIName;
}

VendorItemData const* Creature::GetVendorItems() const
{
    return objmgr.GetNpcVendorItemList(GetEntry());
}

uint32 Creature::GetVendorItemCurrentCount(VendorItem const* vItem)
{
    if(!vItem->maxcount)
        return vItem->maxcount;

    VendorItemCounts::iterator itr = m_vendorItemCounts.begin();
    for(; itr != m_vendorItemCounts.end(); ++itr)
        if(itr->itemId==vItem->proto->ItemId)
            break;

    if(itr == m_vendorItemCounts.end())
        return vItem->maxcount;

    VendorItemCount* vCount = &*itr;

    time_t ptime = time(NULL);

    if( vCount->lastIncrementTime + vItem->incrtime <= ptime )
    {
        ItemPrototype const* pProto = objmgr.GetItemPrototype(vItem->proto->ItemId);

        uint32 diff = uint32((ptime - vCount->lastIncrementTime)/vItem->incrtime);
        if((vCount->count + diff * pProto->BuyCount) >= vItem->maxcount )
        {
            m_vendorItemCounts.erase(itr);
            return vItem->maxcount;
        }

        vCount->count += diff * pProto->BuyCount;
        vCount->lastIncrementTime = ptime;
    }

    return vCount->count;
}

uint32 Creature::UpdateVendorItemCurrentCount(VendorItem const* vItem, uint32 used_count)
{
    if(!vItem->maxcount)
        return 0;

    VendorItemCounts::iterator itr = m_vendorItemCounts.begin();
    for(; itr != m_vendorItemCounts.end(); ++itr)
        if(itr->itemId==vItem->proto->ItemId)
            break;

    if(itr == m_vendorItemCounts.end())
    {
        uint32 new_count = vItem->maxcount > used_count ? vItem->maxcount-used_count : 0;
        m_vendorItemCounts.push_back(VendorItemCount(vItem->proto->ItemId,new_count));
        return new_count;
    }

    VendorItemCount* vCount = &*itr;

    time_t ptime = time(NULL);

    if( vCount->lastIncrementTime + vItem->incrtime <= ptime )
    {
        uint32 diff = uint32((ptime - vCount->lastIncrementTime)/vItem->incrtime);
        if((vCount->count + diff * vItem->proto->BuyCount) < vItem->maxcount )
            vCount->count += diff * vItem->proto->BuyCount;
        else
            vCount->count = vItem->maxcount;
    }

    vCount->count = vCount->count > used_count ? vCount->count-used_count : 0;
    vCount->lastIncrementTime = ptime;
    return vCount->count;
}

TrainerSpellData const* Creature::GetTrainerSpells() const
{
    return objmgr.GetNpcTrainerSpells(GetEntry());
}

// overwrite WorldObject function for proper name localization
const char* Creature::GetNameForLocaleIdx(int32 loc_idx) const
{
    if (loc_idx >= 0)
    {
        CreatureLocale const *cl = objmgr.GetCreatureLocale(GetEntry());
        if (cl)
        {
            if (cl->Name.size() > loc_idx && !cl->Name[loc_idx].empty())
                return cl->Name[loc_idx].c_str();
        }
    }

    return GetName();
}

const CreatureData* Creature::GetLinkedRespawnCreatureData() const
{
    if(!m_DBTableGuid) // only hard-spawned creatures from DB can have a linked master
        return NULL;

    if(uint32 targetGuid = objmgr.GetLinkedRespawnGuid(m_DBTableGuid))
        return objmgr.GetCreatureData(targetGuid);

    return NULL;
}

// returns master's remaining respawn time if any
time_t Creature::GetLinkedCreatureRespawnTime() const
{
    if(!m_DBTableGuid) // only hard-spawned creatures from DB can have a linked master
        return 0;

    if(uint32 targetGuid = objmgr.GetLinkedRespawnGuid(m_DBTableGuid))
    {
        Map* targetMap = NULL;
        if(const CreatureData* data = objmgr.GetCreatureData(targetGuid))
        {
            if(data->mapid == GetMapId())   // look up on the same map
                targetMap = GetMap();
            else                            // it shouldn't be instanceable map here
                targetMap = MapManager::Instance().FindMap(data->mapid);
        }
        if(targetMap)
            return objmgr.GetCreatureRespawnTime(targetGuid,targetMap->GetInstanceId());
    }

    return 0;
}

void Creature::AreaCombat()
{
    if(Map* map = GetMap())
    {
        float range = 0.0f;
        if(GetCreatureTemplate()->flags_extra & CREATURE_FLAG_EXTRA_INSTANCE_BIND)
            range += 100.0f;
        if(isWorldBoss())
            range += 100.0f;

        Map::PlayerList const &PlayerList = map->GetPlayers();
        for(Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
        {
            if (Player* i_pl = i->GetSource())
                if (i_pl->IsAlive() && IsWithinCombatRange(i_pl, range) && CanAttack(i_pl, false))
                {
                    SetInCombatWith(i_pl);
                    i_pl->SetInCombatWith(this);
                    AddThreat(i_pl, 0.0f);
               }
        }
    }
}

bool Creature::IsAllowedToLoot(uint64 guid)
{
    // Temporary fix for Sathrovarr (Kalecgos encounter - Sunwell Plateau)
    if (GetEntry() == 24892)
        return true;
    for (std::vector<uint64>::const_iterator itr = m_allowedToLoot.begin(); itr != m_allowedToLoot.end(); itr++) {
        if ((*itr) == guid)
            return true;
    }
    
    return false;
}

bool Creature::IsBelowHPPercent(float percent)
{
    float healthAtPercent = (GetMaxHealth() / 100.0f) * percent;
    
    return GetHealth() < healthAtPercent;
}

bool Creature::IsAboveHPPercent(float percent)
{
    float healthAtPercent = (GetMaxHealth() / 100.0f) * percent;
    
    return GetHealth() > healthAtPercent;
}

bool Creature::IsBetweenHPPercent(float minPercent, float maxPercent)
{
    float minHealthAtPercent = (GetMaxHealth() / 100.0f) * minPercent;
    float maxHealthAtPercent = (GetMaxHealth() / 100.0f) * maxPercent;

    return GetHealth() > minHealthAtPercent && GetHealth() < maxHealthAtPercent;
}

bool Creature::isMoving()
{
    float x, y ,z;
    return GetMotionMaster()->GetDestination(x,y,z);
}

TemporarySummon* Creature::ToTemporarySummon()  
{ 
    return m_summoned ? dynamic_cast<TemporarySummon*>(this) : nullptr; 
}

bool AIMessageEvent::Execute(uint64 /*e_time*/, uint32 /*p_time*/) 
{ 
    if(owner.AI())
        owner.AI()->message(id,data);
    if(owner.getAI())
        owner.getAI()->message(id,data);

    return true; 
}

void Creature::AddMessageEvent(uint64 timer, uint32 messageId, uint64 data)
{
    AIMessageEvent* messageEvent = new AIMessageEvent(*this,messageId,data);
    m_Events.AddEvent(messageEvent, m_Events.CalculateTime(timer), false);
}

float Creature::GetDistanceFromHome() const
{
    return GetDistance(GetHomePosition());
}

void Creature::Motion_Initialize()
{
    if (!m_formation)
        GetMotionMaster()->Initialize();
    else if (m_formation->getLeader() == this)
    {
        m_formation->FormationReset(false);
        GetMotionMaster()->Initialize();
    }
    else if (m_formation->isFormed())
        GetMotionMaster()->MoveIdle(); //wait the order of leader
    else
        GetMotionMaster()->Initialize();
}

void Creature::SetTarget(uint64 guid)
{
    if (!_focusSpell)
        SetUInt64Value(UNIT_FIELD_TARGET, guid);
}

void Creature::FocusTarget(Spell const* focusSpell, WorldObject const* target)
{
    // already focused
    if (_focusSpell)
        return;

    _focusSpell = focusSpell;
    SetUInt64Value(UNIT_FIELD_TARGET, target->GetGUID());
    if (focusSpell->GetSpellInfo()->AttributesEx5 & SPELL_ATTR_EX5_DONT_TURN_DURING_CAST)
        AddUnitState(UNIT_STATE_ROTATING);

    // Set serverside orientation if needed (needs to be after attribute check)
    SetInFront(target);
}

void Creature::ReleaseFocus(Spell const* focusSpell)
{
    // focused to something else
    if (focusSpell != _focusSpell)
        return;

    _focusSpell = NULL;
    if (Unit* victim = GetVictim())
        SetUInt64Value(UNIT_FIELD_TARGET, victim->GetGUID());
    else
        SetUInt64Value(UNIT_FIELD_TARGET, 0);

    if (focusSpell->GetSpellInfo()->AttributesEx5 & SPELL_ATTR_EX5_DONT_TURN_DURING_CAST)
        ClearUnitState(UNIT_STATE_ROTATING);
}

bool Creature::SetDisableGravity(bool disable, bool packetOnly/*=false*/)
{
    //! It's possible only a packet is sent but moveflags are not updated
    //! Need more research on this
    if (!packetOnly && !Unit::SetDisableGravity(disable))
        return false;

    return true;
}

bool Creature::SetWalk(bool enable)
{
    if (!Unit::SetWalk(enable))
        return false;

    WorldPacket data(enable ? SMSG_SPLINE_MOVE_SET_WALK_MODE : SMSG_SPLINE_MOVE_SET_RUN_MODE, 9);
    data.append(GetPackGUID());
    SendMessageToSet(&data, false);
    return true;
}


bool Creature::SetSwim(bool enable)
{
    if (!Unit::SetSwim(enable))
        return false;

    if (!movespline->Initialized())
        return true;

    WorldPacket data(enable ? SMSG_SPLINE_MOVE_START_SWIM : SMSG_SPLINE_MOVE_STOP_SWIM);
    data.append(GetPackGUID());
    SendMessageToSet(&data, true);
    return true;
}

bool Creature::SetCanFly(bool enable)
{
    if (!Unit::SetCanFly(enable))
        return false;

    if (!movespline->Initialized())
        return true;

    WorldPacket data(enable ? SMSG_SPLINE_MOVE_SET_FLYING : SMSG_SPLINE_MOVE_UNSET_FLYING, 9);
    data.append(GetPackGUID());
    SendMessageToSet(&data, false);
    return true;
}

bool Creature::SetWaterWalking(bool enable, bool packetOnly /* = false */)
{
    if (!packetOnly && !Unit::SetWaterWalking(enable))
        return false;

    if (!movespline->Initialized())
        return true;

    WorldPacket data(enable ? SMSG_SPLINE_MOVE_WATER_WALK : SMSG_SPLINE_MOVE_LAND_WALK);
    data.append(GetPackGUID());
    SendMessageToSet(&data, true);
    return true;
}

bool Creature::SetFeatherFall(bool enable, bool packetOnly /* = false */)
{
    if (!packetOnly && !Unit::SetFeatherFall(enable))
        return false;

    if (!movespline->Initialized())
        return true;

    WorldPacket data(enable ? SMSG_SPLINE_MOVE_FEATHER_FALL : SMSG_SPLINE_MOVE_NORMAL_FALL);
    data.append(GetPackGUID());
    SendMessageToSet(&data, true);
    return true;
}

bool Creature::SetHover(bool enable, bool packetOnly /*= false*/)
{
    if (!packetOnly && !Unit::SetHover(enable))
        return false;

    //! Unconfirmed for players:
    if (enable)
        SetByteFlag(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_HOVER);
    else
        RemoveByteFlag(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_HOVER);

    if (!movespline->Initialized())
        return true;

    //! Not always a packet is sent
    WorldPacket data(enable ? SMSG_SPLINE_MOVE_SET_HOVER : SMSG_SPLINE_MOVE_UNSET_HOVER, 9);
    data.append(GetPackGUID());
    SendMessageToSet(&data, false);
    return true;
}

void Creature::SetPosition(float x, float y, float z, float o)
{
    // prevent crash when a bad coord is sent by the client
    if (!Trinity::IsValidMapCoord(x, y, z, o))
    {
        TC_LOG_DEBUG("entities.unit", "Creature::SetPosition(%f, %f, %f) .. bad coordinates!", x, y, z);
        return;
    }

    GetMap()->CreatureRelocation(this, x, y, z, o);
}

void Creature::UpdateMovementFlags()
{
    // Do not update movement flags if creature is controlled by a player (charm/vehicle)
    if (m_movedByPlayer)
        return;

    // Set the movement flags if the creature is in that mode. (Only fly if actually in air, only swim if in water, etc)
    float ground = GetMap()->GetHeight(GetPositionX(), GetPositionY(), GetPositionZMinusOffset());

    bool isInAir = (G3D::fuzzyGt(GetPositionZMinusOffset(), ground + 0.05f) || G3D::fuzzyLt(GetPositionZMinusOffset(), ground - 0.05f)); // Can be underground too, prevent the falling

    if (GetCreatureTemplate()->InhabitType & INHABIT_AIR && isInAir && !IsFalling())
    {
        if (GetCreatureTemplate()->InhabitType & INHABIT_GROUND)
            SetCanFly(true);
        else
            SetDisableGravity(true);
    }
    else
    {
        SetCanFly(false);
        SetDisableGravity(false);
    }

    if (!isInAir)
        RemoveUnitMovementFlag(MOVEMENTFLAG_FALLING);

    SetSwim(GetCreatureTemplate()->InhabitType & INHABIT_WATER && IsInWater());
}

bool Creature::IsInEvadeMode() const 
{ 
    return HasUnitState(UNIT_STATE_EVADE); 
}

//Do not know if this works or not, moving creature to another map is very dangerous
void Creature::FarTeleportTo(Map* map, float X, float Y, float Z, float O)
{
    //TODOMOV okay tout ça ?
    CleanupsBeforeDelete(false);
    GetMap()->Remove(this,false);
    Relocate(X, Y, Z, O);
    SetMapId(map->GetId());
    SetInstanceId(map->GetInstanceId());
    map->Add(this);
}

void Creature::CheckForUnreachableTarget()
{
    if(!AI() || !IsInCombat())
        return;

    uint32 maxTime = sWorld.getConfig(CONFIG_CREATURE_MAX_UNREACHABLE_TARGET_TIME);
    if(!maxTime)
        return;

    if(GetUnreachableTargetTime() > maxTime)
        AI()->EnterEvadeMode();
}

void Creature::ResetCreatureEmote()
{
    if(CreatureDataAddon const* cAddon = GetCreatureAddon())
        SetUInt32Value(UNIT_NPC_EMOTESTATE, cAddon->emote); 
    else
        SetUInt32Value(UNIT_NPC_EMOTESTATE, 0); 

    SetStandState(UNIT_STAND_STATE_STAND);
}