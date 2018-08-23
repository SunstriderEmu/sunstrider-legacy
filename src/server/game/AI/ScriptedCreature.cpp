
#include "ScriptedCreature.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Item.h"
#include "Spell.h"
#include "ObjectMgr.h"
#include "ScriptMgr.h"
#include "Containers.h"
#include "InstanceScript.h"

 // Spell summary for ScriptedAI::SelectSpell
struct TSpellSummary
{
    uint8 Targets;                                          // set of enum SelectTarget
    uint8 Effects;                                          // set of enum SelectEffect
} extern* SpellSummary;

void SummonList::Summon(Creature const* summon)
{
    storage_.push_back(summon->GetGUID());
}

void SummonList::Despawn(Creature const* summon)
{
    storage_.remove(summon->GetGUID());
}

void SummonList::DespawnEntry(uint32 entry)
{
    for (StorageType::iterator i = storage_.begin(); i != storage_.end();)
    {
        Creature* summon = ObjectAccessor::GetCreature(*me, *i);
        if (!summon)
            i = storage_.erase(i);
        else if (summon->GetEntry() == entry)
        {
            i = storage_.erase(i);
            summon->DespawnOrUnsummon();
        }
        else
            ++i;
    }
}

void SummonList::DespawnAll(bool withoutWorldBoss)
{
    while (!storage_.empty())
    {
        Creature* summon = ObjectAccessor::GetCreature(*me, storage_.front());
        storage_.pop_front();
        if (summon)
            summon->DespawnOrUnsummon();
    }
}

void SummonList::RemoveNotExisting()
{
    for (StorageType::iterator i = storage_.begin(); i != storage_.end();)
    {
        if (ObjectAccessor::GetCreature(*me, *i))
            ++i;
        else
            i = storage_.erase(i);
    }
}

Creature* SummonList::GetCreatureWithEntry(uint32 entry) const
{
    for (StorageType::const_iterator i = storage_.begin(); i != storage_.end(); ++i)
    {
        Creature* summon = ObjectAccessor::GetCreature(*me, *i);
        if (summon && summon->GetEntry() == entry)
            return summon;
    }

    return nullptr;
}

void SummonList::DoZoneInCombat(uint32 entry)
{
    for (StorageType::iterator i = storage_.begin(); i != storage_.end();)
    {
        Creature* summon = ObjectAccessor::GetCreature(*me, *i);
        ++i;
        if (summon && summon->IsAIEnabled
            && (!entry || summon->GetEntry() == entry))
        {
            summon->AI()->DoZoneInCombat(nullptr);
        }
    }
}

bool SummonList::IsAlive()
{
    for (ObjectGuid const i : *this)
    {
        if (Creature* summon = ObjectAccessor::GetCreature(*me, i))
        {
            if (summon->IsAlive())
                return true;
        }
    }
    return false;
}

void SummonList::DoActionImpl(int32 action, StorageType const& summons)
{
    for (auto const& guid : summons)
    {
        Creature* summon = ObjectAccessor::GetCreature(*me, guid);
        if (summon && summon->IsAIEnabled)
            summon->AI()->DoAction(action);
    }
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
    auto found = find(p->GetGUID());
    if(found != end())
        return false;

    insert(std::make_pair(p->GetGUID(),customValue?customValue:m_cooldown)); //3s before being knockable again
    return true;
}

ScriptedAI::ScriptedAI(Creature* creature) : CreatureAI(creature),
    _evadeCheckCooldown(2500)
{
    _isHeroic = me->GetMap()->IsHeroic();
}

void ScriptedAI::AttackStart(Unit* who, bool melee)
{
    if(melee)
        CreatureAI::AttackStart(who);
    else
        AttackStartNoMove(who);
}

void ScriptedAI::AttackStartNoMove(Unit* who)
{
    if (!who)
        return;

    if (me->Attack(who, true))
        DoStartNoMovement(who);
}

void ScriptedAI::AttackStart(Unit* who)
{
    if (IsCombatMovementAllowed())
        CreatureAI::AttackStart(who);
    else
        AttackStartNoMove(who);
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

void ScriptedAI::JustAppeared()
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
    if (me->GetVictim() != nullptr)
    {
        me->AttackStop();
    }
}

void ScriptedAI::DoCastSpell(Unit* target, SpellInfo const* spellInfo, bool triggered)
{
    if (!target || me->IsNonMeleeSpellCast(false))
        return;

    me->StopMoving();
    me->CastSpell(target, spellInfo->Id, triggered ? TRIGGERED_FULL_MASK : TRIGGERED_NONE);
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
    unit->SendMessageToSet(&data, false);
}

Creature* ScriptedAI::DoSpawnCreature(uint32 id, float x, float y, float z, float angle, uint32 type, uint32 despawntime)
{
    return me->SummonCreature(id,me->GetPositionX() + x,me->GetPositionY() + y,me->GetPositionZ() + z, angle, (TempSummonType)type, despawntime);
}

SpellInfo const* ScriptedAI::SelectSpell(Unit* target, SpellSchoolMask School, Mechanics Mechanic, SelectSpellTarget Targets, uint32 PowerCostMin, uint32 PowerCostMax, float RangeMin, float RangeMax, SelectEffect Effects)
{
    //No target so we can't cast
    if (!target)
        return nullptr;

    //Silenced so we can't cast
    if (me->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SILENCED))
        return nullptr;;

    //Using the extended script system we first create a list of viable spells
    SpellInfo const* Spell[4];
    Spell[0] = nullptr;
    Spell[1] = nullptr;
    Spell[2] = nullptr;
    Spell[3] = nullptr;

    uint32 SpellCount = 0;

    SpellInfo const* TempSpell;

    //Check if each spell is viable(set it to null if not)
    for (uint32 m_spell : me->m_spells)
    {
        TempSpell = sSpellMgr->GetSpellInfo(m_spell);

        //This spell doesn't exist
        if (!TempSpell)
            continue;

        // Targets and Effects checked first as most used restrictions
        //Check the spell targets if specified
        if (Targets && !(SpellSummary[m_spell].Targets & (1 << (Targets - 1))))
            continue;

        //Check the type of spell if we are looking for a specific spell type
        if (Effects && !(SpellSummary[m_spell].Effects & (1 << (Effects - 1))))
            continue;

        //Check for school if specified
        if (School != SPELL_SCHOOL_MASK_NONE && !(TempSpell->SchoolMask & School))
            continue;

        //Check for spell mechanic if specified
        if (Mechanic != MECHANIC_NONE && TempSpell->Mechanic != Mechanic)
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
        if (RangeMin && RangeMin < me->GetSpellMinRangeForTarget(target, TempSpell))
            continue;
        if (RangeMax && RangeMax > me->GetSpellMaxRangeForTarget(target, TempSpell))
            continue;

        //Check if our target is in range
        if (me->IsWithinDistInMap(target, me->GetSpellMinRangeForTarget(target, TempSpell)) || !me->IsWithinDistInMap(target, me->GetSpellMaxRangeForTarget(target, TempSpell)))
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

void ScriptedAI::SetEquipmentSlots(bool loadDefault, int32 uiMainHand, int32 uiOffHand, int32 uiRanged)
{
    if (loadDefault)
    {
        if (CreatureTemplate const* pInfo = sObjectMgr->GetCreatureTemplate(me->GetEntry()))
            me->LoadEquipment(me->GetOriginalEquipmentId(), true);

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

bool ScriptedAI::EnterEvadeIfOutOfCombatArea()
{
    if (me->IsInEvadeMode() || !me->IsInCombat())
        return false;

    if (_evadeCheckCooldown == time(nullptr))
        return false;
    _evadeCheckCooldown = time(nullptr);

    if (!CheckEvadeIfOutOfCombatArea())
        return false;

    EnterEvadeMode(EVADE_REASON_BOUNDARY);
    return true;
}

void ScriptedAI::AddThreat(Unit* victim, float amount, Unit* who)
{
    if (!victim)
        return;
    if (!who)
        who = me;
    who->GetThreatManager().AddThreat(victim, amount, nullptr, true, true);
}

void ScriptedAI::ModifyThreatByPercent(Unit* victim, int32 pct, Unit* who)
{
    if (!victim)
        return;
    if (!who)
        who = me;
    who->GetThreatManager().ModifyThreatByPercent(victim, pct);
}

void ScriptedAI::ResetThreat(Unit* victim, Unit* who)
{
    if (!victim)
        return;
    if (!who)
        who = me;
    who->GetThreatManager().ResetThreat(victim);
}

void ScriptedAI::ResetThreatList(Unit* who)
{
    if (!who)
        who = me;
    who->GetThreatManager().ResetAllThreat();
}

float ScriptedAI::GetThreat(Unit const* victim, Unit const* who)
{
    if (!victim)
        return 0.0f;
    if (!who)
        who = me;
    return who->GetThreatManager().GetThreat(victim);
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
    for(const auto & i : PlayerList)
        if (Player* i_pl = i.GetSource())
            if (i_pl->IsAlive())
                i_pl->TeleportTo(me->GetMapId(), x, y, z, o, TELE_TO_NOT_LEAVE_COMBAT);
}


// BossAI - for instanced bosses

BossAI::BossAI(Creature* creature, uint32 bossId) : ScriptedAI(creature),
instance(creature->GetInstanceScript()),
summons(creature),
_bossId(bossId)
{
    if (instance)
        SetBoundary(instance->GetBossBoundary(bossId));
    scheduler.SetValidator([this]
    {
        return !me->HasUnitState(UNIT_STATE_CASTING);
    });
}

void BossAI::_Reset()
{
    if (!me->IsAlive())
        return;

    me->SetCombatPulseDelay(0);
    //TC me->ResetLootMode();
    events.Reset();
    summons.DespawnAll();
    scheduler.CancelAll();
    if (instance && instance->GetBossState(_bossId) != DONE)
        instance->SetBossState(_bossId, NOT_STARTED);
}

void BossAI::_JustDied()
{
    events.Reset();
    summons.DespawnAll();
    if (instance)
    {
        instance->SetBossState(_bossId, DONE);
        instance->SaveToDB();
    }
}

void BossAI::_JustReachedHome()
{
    me->SetKeepActive(false);
}

void BossAI::_JustEngagedWith()
{
    if (instance)
    {
        // bosses do not respawn, check only on enter combat
        if (!instance->CheckRequiredBosses(_bossId))
        {
            EnterEvadeMode(EVADE_REASON_SEQUENCE_BREAK);
            return;
        }
        instance->SetBossState(_bossId, IN_PROGRESS);
    }
    me->SetCombatPulseDelay(5);
    me->SetKeepActive(true);
    DoZoneInCombat();
    ScheduleTasks();
}

void BossAI::TeleportCheaters()
{
    float x, y, z;
    me->GetPosition(x, y, z);

    for (auto const& pair : me->GetCombatManager().GetPvECombatRefs())
    {
        Unit* target = pair.second->GetOther(me);
        if (target->IsControlledByPlayer() && !IsInBoundary(target))
            target->NearTeleportTo(x, y, z, 0);
    }
}

void BossAI::JustSummoned(Creature* summon)
{
    summons.Summon(summon);
    if (me->IsInCombat())
        DoZoneInCombat(summon);
}

void BossAI::SummonedCreatureDespawn(Creature* summon)
{
    summons.Despawn(summon);
}

void BossAI::UpdateAI(uint32 diff)
{
    if (!UpdateVictim())
        return;

    events.Update(diff);

    if (me->HasUnitState(UNIT_STATE_CASTING))
        return;

    while (uint32 eventId = events.ExecuteEvent())
    {
        ExecuteEvent(eventId);
        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;
    }

    DoMeleeAttackIfReady();
}

bool BossAI::CanAIAttack(Unit const* target) const
{
    return IsInBoundary(target);
}

void BossAI::_DespawnAtEvade(Seconds delayToRespawn, Creature* who)
{
    if (delayToRespawn < Seconds(2))
    {
        TC_LOG_ERROR("scripts", "_DespawnAtEvade called with delay of %ld seconds, defaulting to 2.", delayToRespawn.count());
        delayToRespawn = Seconds(2);
    }

    if (!who)
        who = me;

    if (TempSummon* whoSummon = who->ToTempSummon())
    {
        TC_LOG_WARN("scripts", "_DespawnAtEvade called on a temporary summon.");
        whoSummon->UnSummon();
        return;
    }

    who->DespawnOrUnsummon(0, Seconds(delayToRespawn));

    if (instance && who == me)
        instance->SetBossState(_bossId, FAIL);
}

Unit* ScriptedAI::DoSelectLowestHpFriendly(float range, uint32 MinHPDiff)
{
    Unit* pUnit = nullptr;
    Trinity::MostHPMissingInRange u_check(me, range, MinHPDiff);
    Trinity::UnitLastSearcher<Trinity::MostHPMissingInRange> searcher(me, pUnit, u_check);
    Cell::VisitGridObjects(me, searcher, range);
    return pUnit;
}

std::list<Creature*> ScriptedAI::DoFindFriendlyCC(float range)
{
    std::list<Creature*> pList;
    Trinity::FriendlyCCedInRange u_check(me, range);
    Trinity::CreatureListSearcher<Trinity::FriendlyCCedInRange> searcher(me, pList, u_check);
    Cell::VisitGridObjects(me, searcher, range);
    return pList;
}

std::list<Creature*> ScriptedAI::DoFindFriendlyMissingBuff(float range, uint32 spellid)
{
    std::list<Creature*> pList;
    Trinity::FriendlyMissingBuffInRange u_check(me, range, spellid);
    Trinity::CreatureListSearcher<Trinity::FriendlyMissingBuffInRange> searcher(me, pList, u_check);
    Cell::VisitGridObjects(me, searcher, range);
    return pList;
}


void LoadOverridenSQLData()
{
    GameObjectTemplate *goInfo;

    // Sunwell Plateau : Kalecgos : Spectral Rift
    goInfo = const_cast<GameObjectTemplate*>(sObjectMgr->GetGameObjectTemplate(187055));
    if(goInfo && goInfo->type == GAMEOBJECT_TYPE_GOOBER)
        goInfo->goober.lockId = 57; // need LOCKTYPE_QUICK_OPEN
}
