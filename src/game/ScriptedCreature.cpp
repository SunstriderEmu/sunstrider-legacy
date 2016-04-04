/* Copyright (C) 2008 Trinity <http://www.trinitycore.org/>
 *
 * Thanks to the original authors: ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 *
 * This program is free software licensed under GPL version 2
 * Please see the included DOCS/LICENSE.TXT for more information */

#include "ScriptedCreature.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Item.h"
#include "Spell.h"
#include "ObjectMgr.h"
#include "ScriptMgr.h"
#include "Containers.h"

 // Spell summary for ScriptedAI::SelectSpell
struct TSpellSummary
{
    uint8 Targets;                                          // set of enum SelectTarget
    uint8 Effects;                                          // set of enum SelectEffect
} extern* SpellSummary;

void SummonList::Despawn(Creature *summon)
{
    uint64 guid = summon->GetGUID();
    for(iterator i = begin(); i != end(); ++i)
    {
        if(*i == guid)
        {
            erase(i);
            return;
        }
    }
}

void SummonList::DespawnEntry(uint32 entry)
{
    for(auto itr = begin(); itr != end();)
    {
        if(Creature *summon = ObjectAccessor::GetCreature(*m_creature, *itr))
        {
            if(summon->GetEntry() == entry)
            {
                summon->RemoveFromWorld();
                summon->SetDeathState(JUST_DIED);
                summon->RemoveCorpse();
                itr = erase(itr);
                continue;
            }
        }
        itr++;
    }
}

void SummonList::DespawnAll(bool withoutWorldBoss)
{
    for(iterator i = begin(); i != end(); ++i)
    {
        if(Creature *summon = ObjectAccessor::GetCreature(*m_creature, *i))
        {
            if (withoutWorldBoss && summon->IsWorldBoss())
                continue;

            summon->AddObjectToRemoveList();
        }
    }
    clear();
}

bool SummonList::IsEmpty()
{
    return empty();
}

void BumpHelper::Update(const uint32 diff)
{
    for(auto itr = begin(); itr != end();)
    {
        if(itr->second < diff) //okay to erase
            itr = erase(itr);
        else //just decrease time left
        {
            itr->second -= diff;
            itr++;
        }
    }
}

//return true if not yet present in list
bool BumpHelper::AddCooldown(Unit* p, uint32 customValue)
{
    std::map<uint64,uint32>::iterator found = find(p->GetGUID());
    if(found != end())
        return false;

    insert(std::make_pair(p->GetGUID(),customValue?customValue:m_cooldown)); //3s before being knockable again
    return true;
}

void ScriptedAI::AttackStart(Unit* who, bool melee)
{
    if (!who)
        return;

    if (me->Attack(who, melee))
    {
        me->AddThreat(who, 0.0f);

        if(melee)
            DoStartMovement(who);
        else
            DoStartNoMovement(who);
    }
}

void ScriptedAI::AttackStart(Unit* who)
{
    if (!who)
        return;
    
    bool melee = (GetCombatDistance() > ATTACK_DISTANCE) ? me->GetDistance(who) <= ATTACK_DISTANCE : true;
    if (me->Attack(who, melee))
    {
        me->AddThreat(who, 0.0f);

        if(IsCombatMovementAllowed())
        {
            DoStartMovement(who);
        } else {
            DoStartNoMovement(who);
        }
    }
}

void ScriptedAI::UpdateAI(const uint32 diff)
{
    //Check if we have a current target
    if (me->IsAlive() && UpdateVictim())
    {
        if (me->IsAttackReady() )
        {
            //If we are within range melee the target
            if (me->IsWithinMeleeRange(me->GetVictim()))
            {
                me->AttackerStateUpdate(me->GetVictim());
                me->ResetAttackTimer();
            }
        }
    }
}

void ScriptedAI::EnterEvadeMode()
{
    //me->InterruptNonMeleeSpells(true);
    me->RemoveAllAuras();
    me->DeleteThreatList();
    me->CombatStop();
    me->InitCreatureAddon();
    me->SetLootRecipient(NULL);

    if(me->IsAlive())
    {
        if(Unit* owner = me->GetOwner())
        {
            if(owner->IsAlive())
                me->GetMotionMaster()->MoveFollow(owner,PET_FOLLOW_DIST,PET_FOLLOW_ANGLE);
        }
        else
            me->GetMotionMaster()->MoveTargetedHome();
    }

    Reset();
}

void ScriptedAI::JustRespawned()
{
    Reset();
}

void ScriptedAI::DoStartMovement(Unit* victim, float distance, float angle)
{
    if (!victim)
        return;

    me->GetMotionMaster()->MoveChase(victim, distance, angle);
}

void ScriptedAI::DoStartNoMovement(Unit* victim)
{
    if (!victim)
        return;

    me->GetMotionMaster()->MoveIdle();
}

void ScriptedAI::DoStopAttack()
{
    if (me->GetVictim() != NULL)
    {
        me->AttackStop();
    }
}

uint32 ScriptedAI::DoCast(Unit* victim, uint32 spellId, bool triggered)
{
    //remove this?
    if (me->HasUnitState(UNIT_STATE_CASTING) && !triggered)
        return SPELL_FAILED_SPELL_IN_PROGRESS;

    uint32 reason = me->CastSpell(victim, spellId, triggered);

    //restore combat movement on out of mana
    if(reason == SPELL_FAILED_NO_POWER && GetRestoreCombatMovementOnOOM() && !IsCombatMovementAllowed())
        SetCombatMovementAllowed(true);

    return reason;
}

uint32 ScriptedAI::DoCastAOE(uint32 spellId, bool triggered)
{
    return DoCast((Unit*)NULL, spellId, triggered);
}

uint32 ScriptedAI::DoCastSpell(Unit* who,SpellInfo const *spellInfo, bool triggered)
{
    //remove this?
    if (!who || me->IsNonMeleeSpellCast(false))
        return SPELL_FAILED_SPELL_IN_PROGRESS;

    me->StopMoving();
    return me->CastSpell(who, spellInfo, triggered);
}

void ScriptedAI::DoPlaySoundToSet(Unit* unit, uint32 sound)
{
    if (!unit)
        return;

    if (!GetSoundEntriesStore()->LookupEntry(sound))
    {
        error_log("TSCR: Invalid soundId %u used in DoPlaySoundToSet (by unit TypeId %u, guid %u)", sound, unit->GetTypeId(), unit->GetGUID());
        return;
    }

    WorldPacket data(4);
    data.SetOpcode(SMSG_PLAY_SOUND);
    data << uint32(sound);
    unit->SendMessageToSet(&data,false);
}

Creature* ScriptedAI::DoSpawnCreature(uint32 id, float x, float y, float z, float angle, uint32 type, uint32 despawntime)
{
    return me->SummonCreature(id,me->GetPositionX() + x,me->GetPositionY() + y,me->GetPositionZ() + z, angle, (TempSummonType)type, despawntime);
}

SpellInfo const* ScriptedAI::SelectSpell(Unit* Target, int32 School, int32 Mechanic, SelectTarget Targets, uint32 PowerCostMin, uint32 PowerCostMax, float RangeMin, float RangeMax, SelectEffect Effects)
{
    //No target so we can't cast
    if (!Target)
        return nullptr;

    //Silenced so we can't cast
    if (me->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SILENCED))
        return nullptr;;

    //Using the extended script system we first create a list of viable spells
    SpellInfo const* Spell[4];
    Spell[0] = 0;
    Spell[1] = 0;
    Spell[2] = 0;
    Spell[3] = 0;

    uint32 SpellCount = 0;

    SpellInfo const* TempSpell;

    //Check if each spell is viable(set it to null if not)
    for (uint32 i = 0; i < CREATURE_MAX_SPELLS; i++)
    {
        TempSpell = sSpellMgr->GetSpellInfo(me->m_spells[i]);

        //This spell doesn't exist
        if (!TempSpell)
            continue;

        // Targets and Effects checked first as most used restrictions
        //Check the spell targets if specified
        if (Targets && !(SpellSummary[me->m_spells[i]].Targets & (1 << (Targets - 1))))
            continue;

        //Check the type of spell if we are looking for a specific spell type
        if (Effects && !(SpellSummary[me->m_spells[i]].Effects & (1 << (Effects - 1))))
            continue;

        //Check for school if specified
        if (School >= 0 && TempSpell->SchoolMask & School)
            continue;

        //Check for spell mechanic if specified
        if (Mechanic >= 0 && TempSpell->Mechanic != Mechanic)
            continue;

        //Make sure that the spell uses the requested amount of power
        if (PowerCostMin &&  TempSpell->ManaCost < PowerCostMin)
            continue;

        if (PowerCostMax && TempSpell->ManaCost > PowerCostMax)
            continue;

        //Continue if we don't have the mana to actually cast this spell
        if (TempSpell->ManaCost > me->GetPower((Powers)TempSpell->PowerType))
            continue;

        //Check if the spell meets our range requirements
        if (RangeMin && RangeMin < TempSpell->GetMinRange())
            continue;
        if (RangeMax && RangeMax > TempSpell->GetMaxRange())
            continue;

        //Check if our target is in range
        if (me->IsWithinDistInMap(Target, TempSpell->GetMinRange()) || !me->IsWithinDistInMap(Target, TempSpell->GetMaxRange()))
            continue;

        //All good so lets add it to the spell list
        Spell[SpellCount] = TempSpell;
        SpellCount++;
    }

    //We got our usable spells so now lets randomly pick one
    if (!SpellCount)
        return nullptr;

    return Spell[rand() % SpellCount];
}


bool ScriptedAI::CanCast(Unit* Target, SpellInfo const *Spell, bool Triggered)
{
    //No target so we can't cast
    if (!Target || !Spell)
        return false;

    //Silenced so we can't cast
    if (!Triggered && me->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SILENCED))
        return false;

    //Check for power
    if (!Triggered && me->GetPower((Powers)Spell->PowerType) < Spell->ManaCost)
        return false;

    //Unit is out of range of this spell
    if (me->GetDistance(Target) > Spell->GetMaxRange() || me->GetDistance(Target) < Spell->GetMinRange())
        return false;

    return true;
}

void ScriptedAI::SetEquipmentSlots(bool bLoadDefault, int32 uiMainHand, int32 uiOffHand, int32 uiRanged)
{
    if (bLoadDefault)
    {
        if (CreatureTemplate const* pInfo = GetCreatureTemplates(me->GetEntry()))
            me->LoadEquipment(pInfo->equipmentId,true);

        return;
    }
#ifdef LICH_KING
    if (uiMainHand >= 0)
        me->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID + 0, uint32(uiMainHand));

    if (uiOffHand >= 0)
        me->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID + 1, uint32(uiOffHand));

    if (uiRanged >= 0)
        me->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID + 2, uint32(uiRanged));
#endif
}

void ScriptedAI::DoResetThreat()
{
    if (!me->CanHaveThreatList() || me->getThreatManager().isThreatListEmpty())
    {
        error_log("TSCR: DoResetThreat called for creature that either cannot have threat list or has empty threat list (me entry = %d)", me->GetEntry());

        return;
    }

    std::list<HostileReference*>& m_threatlist = me->getThreatManager().getThreatList();
    std::list<HostileReference*>::iterator itr;

    for(itr = m_threatlist.begin(); itr != m_threatlist.end(); ++itr)
    {
        Unit* pUnit = NULL;
        pUnit = ObjectAccessor::GetUnit((*me), (*itr)->getUnitGuid());
        if(pUnit && me->GetThreat(pUnit))
            DoModifyThreatPercent(pUnit, -100);
    }
}

void ScriptedAI::DoModifyThreatPercent(Unit *pUnit, int32 pct)
{
    if(!pUnit) return;
    me->getThreatManager().modifyThreatPercent(pUnit, pct);
}

void ScriptedAI::DoTeleportTo(float x, float y, float z, uint32 time)
{
    me->Relocate(x, y, z);
    float speed = me->GetDistance(x, y, z) / ((float)time * 0.001f);
    me->MonsterMoveWithSpeed(x, y, z, speed);
}

void ScriptedAI::DoTeleportPlayer(Unit* pUnit, float x, float y, float z, float o)
{
    if(!pUnit || pUnit->GetTypeId() != TYPEID_PLAYER)
    {
        if(pUnit)
            error_log("TSCR: Creature %u (Entry: %u) Tried to teleport non-player unit (Type: %u GUID: %u) to x: %f y:%f z: %f o: %f. Aborted.", me->GetGUID(), me->GetEntry(), pUnit->GetTypeId(), pUnit->GetGUID(), x, y, z, o);
        return;
    }

    (pUnit->ToPlayer())->TeleportTo(pUnit->GetMapId(), x, y, z, o, TELE_TO_NOT_LEAVE_COMBAT);
}

void ScriptedAI::DoTeleportAll(float x, float y, float z, float o)
{
    Map *map = me->GetMap();
    if (!map->IsDungeon())
        return;

    Map::PlayerList const &PlayerList = map->GetPlayers();
    for(Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
        if (Player* i_pl = i->GetSource())
            if (i_pl->IsAlive())
                i_pl->TeleportTo(me->GetMapId(), x, y, z, o, TELE_TO_NOT_LEAVE_COMBAT);
}

Creature* FindCreature(uint32 entry, float range, Unit* Finder)
{
    if(!Finder)
        return NULL;
    Creature* target = NULL;


    Trinity::AllCreaturesOfEntryInRange check(Finder, entry, range);
    Trinity::CreatureSearcher<Trinity::AllCreaturesOfEntryInRange> searcher(target, check);
    Finder->VisitNearbyGridObject(range, searcher);
    
    return target;
}

void FindCreatures(std::list<Creature*>& list, uint32 entry, float range, Unit* Finder)
{
    Trinity::AllCreaturesOfEntryInRange check(Finder, entry, range);
    Trinity::CreatureListSearcher<Trinity::AllCreaturesOfEntryInRange> searcher(Finder, list, check);
    Finder->VisitNearbyGridObject(range, searcher);
}

GameObject* FindGameObject(uint32 entry, float range, Unit* Finder)
{
    if(!Finder)
        return NULL;
    GameObject* target = NULL;

    Trinity::AllGameObjectsWithEntryInGrid go_check(entry);
    Trinity::GameObjectSearcher<Trinity::AllGameObjectsWithEntryInGrid> searcher(target, go_check);
    Finder->VisitNearbyGridObject(range, searcher);
    return target;
}

Unit* ScriptedAI::DoSelectLowestHpFriendly(float range, uint32 MinHPDiff)
{
    Unit* pUnit = NULL;
    Trinity::MostHPMissingInRange u_check(me, range, MinHPDiff);
    Trinity::UnitLastSearcher<Trinity::MostHPMissingInRange> searcher(me, pUnit, u_check);
    me->VisitNearbyObject(range, searcher);
    return pUnit;
}

std::list<Creature*> ScriptedAI::DoFindFriendlyCC(float range)
{
    std::list<Creature*> pList;
    Trinity::FriendlyCCedInRange u_check(me, range);
    Trinity::CreatureListSearcher<Trinity::FriendlyCCedInRange> searcher(me, pList, u_check);
    me->VisitNearbyObject(range, searcher);
    return pList;
}

std::list<Creature*> ScriptedAI::DoFindFriendlyMissingBuff(float range, uint32 spellid)
{
    std::list<Creature*> pList;
    Trinity::FriendlyMissingBuffInRange u_check(me, range, spellid);
    Trinity::CreatureListSearcher<Trinity::FriendlyMissingBuffInRange> searcher(me, pList, u_check);
    me->VisitNearbyObject(range, searcher);
    return pList;
}


// SD2 grid searchers.
Creature* GetClosestCreatureWithEntry(WorldObject* source, uint32 entry, float maxSearchRange, bool alive /*= true*/)
{
    return source->FindNearestCreature(entry, maxSearchRange, alive);
}

GameObject* GetClosestGameObjectWithEntry(WorldObject* source, uint32 entry, float maxSearchRange)
{
    return source->FindNearestGameObject(entry, maxSearchRange);
}

void LoadOverridenSQLData()
{
    GameObjectTemplate *goInfo;

    // Sunwell Plateau : Kalecgos : Spectral Rift
    goInfo = const_cast<GameObjectTemplate*>(sObjectMgr->GetGameObjectTemplate(187055));
    if(goInfo && goInfo->type == GAMEOBJECT_TYPE_GOOBER)
        goInfo->goober.lockId = 57; // need LOCKTYPE_QUICK_OPEN
}

void LoadOverridenDBCData()
{

}