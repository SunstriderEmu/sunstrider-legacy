/* Copyright (C) 2008 Trinity <http://www.trinitycore.org/>
 *
 * Thanks to the original authors: ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 *
 * This program is free software licensed under GPL version 2
 * Please see the included DOCS/LICENSE.TXT for more information */

#ifndef SC_CREATURE_H
#define SC_CREATURE_H

#include "CreatureAI.h"
#include "Creature.h"
#include "CreatureAIImpl.h"
#include "InstanceScript.h"
#include "EventMap.h"
#include "TaskScheduler.h"

#define CAST_AI(a,b)    (dynamic_cast<a*>(b))
#define ENSURE_AI(a,b)  (EnsureAI<a>(b))

template<class T, class U>
T* EnsureAI(U* ai)
{
    T* cast_ai = dynamic_cast<T*>(ai);
    ASSERT(cast_ai);
    return cast_ai;
};

class TC_GAME_API SummonList
{
public:
    typedef GuidList StorageType;
    typedef StorageType::iterator iterator;
    typedef StorageType::const_iterator const_iterator;
    typedef StorageType::size_type size_type;
    typedef StorageType::value_type value_type;

    SummonList(Creature* creature) : me(creature)
    {}

    // And here we see a problem of original inheritance approach. People started
    // to exploit presence of std::list members, so I have to provide wrappers

    iterator begin()
    {
        return storage_.begin();
    }

    const_iterator begin() const
    {
        return storage_.begin();
    }

    iterator end()
    {
        return storage_.end();
    }

    const_iterator end() const
    {
        return storage_.end();
    }

    iterator erase(iterator i)
    {
        return storage_.erase(i);
    }

    bool empty() const
    {
        return storage_.empty();
    }

    size_type size() const
    {
        return storage_.size();
    }

    // Clear the underlying storage. This does NOT despawn the creatures - use DespawnAll for that!
    void clear()
    {
        storage_.clear();
    }

    void Summon(Creature const* summon);
    void Despawn(Creature const* summon);
    void DespawnEntry(uint32 entry);
    void DespawnAll(bool withoutWorldBoss = false);
    bool IsAlive(uint32 creatureId = 0); //true if any alive

    template <typename T>
    void DespawnIf(T const& predicate)
    {
        storage_.remove_if(predicate);
    }

    template <class Predicate>
    void DoAction(int32 info, Predicate&& predicate, uint16 max = 0)
    {
        // We need to use a copy of SummonList here, otherwise original SummonList would be modified
        StorageType listCopy = storage_;
        Trinity::Containers::RandomResize<StorageType, Predicate>(listCopy, std::forward<Predicate>(predicate), max);
        DoActionImpl(info, listCopy);
    }

    void DoZoneInCombat(uint32 entry = 0);
    void RemoveNotExisting();
    Creature* GetCreatureWithEntry(uint32 entry) const; //replaces HasEntry from TC

private:
    void DoActionImpl(int32 action, StorageType const& summons);

    Creature* me;
    StorageType storage_;
};

class TC_GAME_API BumpHelper : std::map<ObjectGuid,uint32>
{
public:
    BumpHelper(uint32 cooldown) : m_cooldown(cooldown) {}
    void Update(const uint32 diff);
    bool AddCooldown(Unit* p, uint32 customValue = 0);
private:
    const uint32 m_cooldown;
};

/*
//Get a single creature of given entry
TC_GAME_API Creature* FindCreature(uint32 entry, float range, Unit* Finder);
//Get every creatures of given entry within given range
TC_GAME_API void FindCreatures(std::list<Creature*>& list, uint32 entry, float range, Unit* Finder);
//Get a single gameobject of given entry
TC_GAME_API GameObject* FindGameObject(uint32 entry, float range, Unit* Finder);
*/

struct TC_GAME_API ScriptedAI : public CreatureAI
{
    ScriptedAI(Creature* creature);
    ~ScriptedAI() override {}

    //*************
    //CreatureAI Functions
    //*************

    //Called at each attack of me by any victim
    void AttackStart(Unit *) override;
    //Attack start with forcing melee / forcing staying on place
    void AttackStart(Unit *, bool melee);
    void AttackStartNoMove(Unit *pTarget);

    // Called at any Damage from any attacker (before damage apply)
    void DamageTaken(Unit *done_by, uint32 &damage) override {}
    
    void HealReceived(Unit* /*done_by*/, uint32& /*addhealth*/) override {}

    //Called at World update tick
    void UpdateAI(const uint32) override;

    //Called at creature death
    void JustDied(Unit*) override{}

    //Called at creature killing another unit
    void KilledUnit(Unit*) override{}

    // Called when the creature summon successfully other creature
    void JustSummoned(Creature*) override {}

    // Called when a summoned creature is despawned
    void SummonedCreatureDespawn(Creature* /*unit*/) override {}

    // Called when hit by a spell
    void SpellHit(Unit* caster, const SpellInfo*) override {}

    // Called when spell hits a target
    void SpellHitTarget(Unit* target, const SpellInfo*) override {}

    // Called when creature is spawned or respawned (for reseting variables)
    void JustAppeared() override;

    //Called at waypoint reached or PointMovement end
    void MovementInform(uint32, uint32) override {}

    // Called when creature finishes a spell cast
    void OnSpellFinish(Unit *caster, uint32 spellId, Unit *target, bool ok) override {}
    
    //*************
    //Pure virtual functions
    //*************

    //Called at creature reset either by death or evade
    void Reset() override {}

    //Called at creature aggro either by MoveInLOS or Attack Start
    void JustEngagedWith(Unit*) override {}

    //*************
    //AI Helper Functions
    //*************

    //Start movement toward victim
    void DoStartMovement(Unit* victim, float distance = 0, float angle = 0);

    //Start no movement on victim
    void DoStartNoMovement(Unit* victim);

    //Stop attack of current victim
    void DoStopAttack();

    //Cast spell by spell info
    void DoCastSpell(Unit* target, SpellInfo const* spellInfo, bool triggered = false);

    //Plays a sound to all nearby players
    void DoPlaySoundToSet(Unit* unit, uint32 sound);

    // Add specified amount of threat directly to victim (ignores redirection effects) - also puts victim in combat and engages them if necessary
    void AddThreat(Unit* victim, float amount, Unit* who = nullptr);
    // Adds/removes the specified percentage from the specified victim's threat (to who, or me if not specified)
    void ModifyThreatByPercent(Unit* victim, int32 pct, Unit* who = nullptr);
    // Resets the victim's threat level to who (or me if not specified) to zero
    void ResetThreat(Unit* victim, Unit* who = nullptr);
    // Resets the specified unit's threat list (me if not specified) - does not delete entries, just sets their threat to zero
    void ResetThreatList(Unit* who = nullptr);
    // Returns the threat level of victim towards who (or me if not specified)
    float GetThreat(Unit const* victim, Unit const* who = nullptr);

    void DoTeleportTo(float x, float y, float z, uint32 time = 0);

    void DoAction(const int32 param) override {}

    //Teleports a player without dropping threat (only teleports to same map)
    void DoTeleportPlayer(Unit* pUnit, float x, float y, float z, float o);
    void DoTeleportAll(float x, float y, float z, float o);

    //Returns friendly unit with the most amount of hp missing from max hp
    Unit* DoSelectLowestHpFriendly(float range, uint32 MinHPDiff = 1);

    //Returns a list of friendly CC'd units within range
    std::list<Creature*> DoFindFriendlyCC(float range);

    //Returns a list of all friendly units missing a specific buff within range
    std::list<Creature*> DoFindFriendlyMissingBuff(float range, uint32 spellid);

    //Spawns a creature relative to m_creature
    Creature* DoSpawnCreature(uint32 id, float x, float y, float z, float angle, uint32 type, uint32 despawntime);

    // return true for heroic mode. i.e.
    bool IsHeroic() const { return _isHeroic; }

    //Returns spells that meet the specified criteria from the creatures spell list
    SpellInfo const* SelectSpell(Unit* Target, SpellSchoolMask School, Mechanics Mechanic, SelectSpellTarget Targets, uint32 PowerCostMin, uint32 PowerCostMax, float RangeMin, float RangeMax, SelectEffect Effect);

    //Checks if you can cast the specified spell
    bool CanCast(Unit* Target, SpellInfo const *Spell, bool Triggered = false);
    
    void SetEquipmentSlots(bool bLoadDefault, int32 uiMainHand = EQUIP_NO_CHANGE, int32 uiOffHand = EQUIP_NO_CHANGE, int32 uiRanged = EQUIP_NO_CHANGE);

    bool EnterEvadeIfOutOfCombatArea();
    virtual bool CheckEvadeIfOutOfCombatArea() const { return false; }

private:
    uint32 _evadeCheckCooldown;
    bool _isHeroic;

};

class TC_GAME_API BossAI : public ScriptedAI
{
public:
    BossAI(Creature* creature, uint32 bossId);
    ~BossAI() override {}

    InstanceScript* const instance;

    void JustSummoned(Creature* summon) override;
    void SummonedCreatureDespawn(Creature* summon) override;

    void UpdateAI(const uint32 diff) override;

    // Hook used to execute events scheduled into EventMap without the need
    // to override UpdateAI
    // note: You must re-schedule the event within this method if the event
    // is supposed to run more than once
    virtual void ExecuteEvent(uint32 /*eventId*/) { }

    virtual void ScheduleTasks() { }

    void Reset() override { _Reset(); }
    void JustEngagedWith(Unit* /*who*/) override { _JustEngagedWith(); }
    void JustDied(Unit* /*killer*/) override { _JustDied(); }
    void JustReachedHome() override { _JustReachedHome(); }

    bool CanAIAttack(Unit const* target) const override;

protected:
    void _Reset();
    void _JustEngagedWith();
    void _JustDied();
    void _JustReachedHome();
    void _DespawnAtEvade(Seconds delayToRespawn, Creature* who = nullptr);
    void _DespawnAtEvade(uint32 delayToRespawn = 30, Creature* who = nullptr) { _DespawnAtEvade(Seconds(delayToRespawn), who); }

    //teleport players out of boundaries on boss
    void TeleportCheaters();

    EventMap events;
    SummonList summons;
    TaskScheduler scheduler;

private:
    uint32 const _bossId;
};

// SD2 grid searchers.
inline Creature* GetClosestCreatureWithEntry(WorldObject const* source, uint32 entry, float maxSearchRange, bool alive = true)
{
    return source->FindNearestCreature(entry, maxSearchRange, alive);
}

inline GameObject* GetClosestGameObjectWithEntry(WorldObject const* source, uint32 entry, float maxSearchRange)
{
    return source->FindNearestGameObject(entry, maxSearchRange);
}

template <typename Container>
inline void GetCreatureListWithEntryInGrid(Container& container, WorldObject* source, uint32 entry, float maxSearchRange)
{
    source->GetCreatureListWithEntryInGrid(container, entry, maxSearchRange);
}

template <typename Container>
inline void GetGameObjectListWithEntryInGrid(Container& container, WorldObject* source, uint32 entry, float maxSearchRange)
{
    source->GetGameObjectListWithEntryInGrid(container, entry, maxSearchRange);
}

template <typename Container>
inline void GetPlayerListInGrid(Container& container, WorldObject* source, float maxSearchRange)
{
    source->GetPlayerListInGrid(container, maxSearchRange);
}

#endif

