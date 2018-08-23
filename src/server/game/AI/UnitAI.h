#ifndef TRINITY_UNITAI_H
#define TRINITY_UNITAI_H

#include "Define.h"
#include "Unit.h"
#include "QuestDef.h"
#include "ThreatManager.h"
#include "SpellDefines.h"
class Unit;
class Quest;
class Player;

//Selection method used by SelectSpellTarget
enum SelectAggroTarget : int
{
    SELECT_TARGET_RANDOM = 0,                             //Just selects a random target
    SELECT_TARGET_MAXTHREAT,                              //Selects targes from top aggro to bottom
    SELECT_TARGET_MINTHREAT,                              //Selects targets from bottom aggro to top
    SELECT_TARGET_MINDISTANCE,
    SELECT_TARGET_MAXDISTANCE,
};

// default predicate function to select target based on distance, player and/or aura criteria
struct TC_GAME_API DefaultTargetSelector
{
    Unit const* me;
    float m_dist;
    bool m_playerOnly;
    Unit const* except;
    int32 m_aura;

    // unit: the reference unit
    // dist: if 0: ignored, if > 0: maximum distance to the reference unit, if < 0: minimum distance to the reference unit
    // playerOnly: self explaining
    // withMainTank: allow current tank to be selected
    // aura: if 0: ignored, if > 0: the target shall have the aura, if < 0, the target shall NOT have the aura
    DefaultTargetSelector(Unit const* unit, float dist, bool playerOnly, bool withMainTank, int32 aura);
    bool operator()(Unit const* target) const;
};

// Target selector for spell casts checking range, auras and attributes
/// @todo Add more checks from Spell::CheckCast
struct TC_GAME_API SpellTargetSelector
{
    public:
        SpellTargetSelector(Unit* caster, uint32 spellId);
        bool operator()(Unit const* target) const;

    private:
        Unit const* _caster;
        SpellInfo const* _spellInfo;
};

// Very simple target selector, will just skip main target
// NOTE: When passing to UnitAI::SelectTarget remember to use 0 as position for random selection
//       because tank will not be in the temporary list
struct TC_GAME_API NonTankTargetSelector
{
    public:
        NonTankTargetSelector(Unit* source, bool playerOnly = true) : _source(source), _playerOnly(playerOnly) { }
        bool operator()(Unit const* target) const;

    private:
        Unit* _source;
        bool _playerOnly;
};

// Simple selector for units using mana
struct TC_GAME_API PowerUsersSelector
{
public:
    PowerUsersSelector(Unit const* unit, Powers power, float dist, bool playerOnly) : _me(unit), _power(power), _dist(dist), _playerOnly(playerOnly) { }
    bool operator()(Unit const* target) const;

private:
    Unit const* _me;
    Powers const _power;
    float const _dist;
    bool const _playerOnly;
};

struct TC_GAME_API FarthestTargetSelector
{
public:
    FarthestTargetSelector(Unit const* unit, float dist, bool playerOnly, bool inLos) : _me(unit), _dist(dist), _playerOnly(playerOnly), _inLos(inLos) {}
    bool operator()(Unit const* target) const;

private:
    const Unit* _me;
    float _dist;
    bool _playerOnly;
    bool _inLos;
};


class TC_GAME_API UnitAI
{
    protected:
        Unit *me;
        //combat movement part not yet implemented. Creatures with m_combatDistance and target distance > 5.0f wont show melee weapons.
        bool m_allowCombatMovement;
        bool m_restoreCombatMovementOnOOM;
    public:
        UnitAI(Unit *u) : me(u), m_allowCombatMovement(true), m_restoreCombatMovementOnOOM(false) {}
        virtual ~UnitAI() = default;

        virtual bool CanAIAttack(Unit const* /*target*/) const { return true; }
        virtual void AttackStart(Unit *);
        void AttackStartCaster(Unit* victim, float dist);
        virtual void UpdateAI(const uint32 diff) { }

        bool IsCombatMovementAllowed() { return m_allowCombatMovement; };
        void SetCombatMovementAllowed(bool allow);
        void SetRestoreCombatMovementOnOOM(bool set);
        bool GetRestoreCombatMovementOnOOM();

        virtual void InitializeAI() { Reset(); }

        virtual void Reset() {}

        // Called when unit's charm state changes with isNew = false
        // Implementation should call me->ScheduleAIChange() if AI replacement is desired
        // If this call is made, AI will be replaced on the next tick
        // When replacement is made, OnCharmed is called with isNew = true
        virtual void OnCharmed(bool isNew);

        // Pass parameters between AI
        virtual void DoAction(const int32 param) {}
        virtual uint32 GetData(uint32 /*id = 0*/) const { return 0; }
        virtual void SetData(uint32 /*id*/, uint32 /*value*/, Unit* setter = nullptr) {}
        virtual void SetGUID(ObjectGuid const& /*guid*/, int32 /*id*/ = 0) { }
        virtual ObjectGuid GetGUID(int32 /*id*/ = 0) const { return ObjectGuid::Empty; }

        // Called when the unit enters combat
        // (NOTE: Creature engage logic should NOT be here, but in JustEngagedWith, which happens once threat is established!)
        virtual void JustEnteredCombat(Unit* /*who*/) { }

        // Called when the unit leaves combat
        virtual void JustExitedCombat() { }

        // Called when the unit is about to be removed from the world (despawn, grid unload, corpse disappearing, player logging out etc.)
        virtual void LeavingWorld() { }

        // Called at any Damage from any attacker (before damage apply)
        virtual void DamageTaken(Unit *done_by, uint32 & /*damage*/) {}
        
        // Called at any Damage to any victim (before damage apply)
        virtual void DamageDealt(Unit* /*victim*/, uint32& /*damage*/, DamageEffectType /*damageType*/) { }

        // Called when the creature receives heal
        virtual void HealReceived(Unit* /*done_by*/, uint32& /*addhealth*/) {}

        // Called when the unit heals
        virtual void HealDone(Unit* /*done_to*/, uint32& /*addhealth*/) { }

        //Cast spell by Id, return SpellCastResult

        //TC uint32 DoCast(uint32 spellId);
        uint32 DoCast(Unit* victim, uint32 spellId, CastSpellExtraArgs const& args = {});
        uint32 DoCastSelf(uint32 spellId, CastSpellExtraArgs const& args = {}) { return DoCast(me, spellId, args); }
        uint32 DoCastVictim(uint32 spellId, CastSpellExtraArgs const& args = {});
        uint32 DoCastAOE(uint32 spellId, CastSpellExtraArgs const& args = {}) { return DoCast(nullptr, spellId, args); }
       /* uint32 DoCast(Unit* victim, uint32 spellId, bool triggered = false);
        uint32 DoCastSelf(uint32 spellId, bool triggered = false) { return DoCast(me, spellId, triggered); }
        uint32 DoCastAOE(uint32 spellId, bool triggered = false);
        uint32 DoCastVictim(uint32 spellId, bool triggered = false);
        //Cast spell by spell info
        uint32 DoCastSpell(Unit* who, SpellInfo const *spellInfo, bool triggered = false);*/

        //Do melee swing of current victim if in rnage and ready and not casting
        virtual void DoMeleeAttackIfReady();
        bool DoSpellAttackIfReady(uint32 spell);

        // Called when a game event starts or ends
        virtual void OnGameEvent(bool /*start*/, uint16 /*eventId*/) { }

        // Select the best target (in <targetType> order) from the threat list that fulfill the following:
        // - Not among the first <offset> entries in <targetType> order (or MAXTHREAT order, if <targetType> is RANDOM).
        // - Within at most <dist> yards (if dist > 0.0f)
        // - At least -<dist> yards away (if dist < 0.0f)
        // - Is a player (if playerOnly = true)
        // - Not the current tank (if withTank = false)
        // - Has aura with ID <aura> (if aura > 0)
        // - Does not have aura with ID -<aura> (if aura < 0)
        Unit* SelectTarget(SelectAggroTarget targetType, uint32 offset = 0, float dist = 0.0f, bool playerOnly = false, bool withTank = true, int32 aura = 0);
        // Select the best target (in <targetType> order) satisfying <predicate> from the threat list.
        // If <offset> is nonzero, the first <offset> entries in <targetType> order (or MAXTHREAT order, if <targetType> is RANDOM) are skipped.
        template<class PREDICATE>
        Unit* SelectTarget(SelectAggroTarget targetType, uint32 offset, PREDICATE const& predicate)
        {
            ThreatManager& mgr = GetThreatManager();
            // shortcut: if we ignore the first <offset> elements, and there are at most <offset> elements, then we ignore ALL elements
            if (mgr.GetThreatListSize() <= offset)
                return nullptr;

            std::list<Unit*> targetList;
            SelectTargetList(targetList, mgr.GetThreatListSize(), targetType, offset, predicate);

            // maybe nothing fulfills the predicate
            if (targetList.empty())
                return nullptr;

            switch (targetType)
            {
            case SELECT_TARGET_MAXTHREAT:
            case SELECT_TARGET_MINTHREAT:
            case SELECT_TARGET_MAXDISTANCE:
            case SELECT_TARGET_MINDISTANCE:
                return targetList.front();
            case SELECT_TARGET_RANDOM:
                return Trinity::Containers::SelectRandomContainerElement(targetList);
            default:
                return nullptr;
            }
        }

        // Select the best (up to) <num> targets (in <targetType> order) from the threat list that fulfill the following:
        // - Not among the first <offset> entries in <targetType> order (or MAXTHREAT order, if <targetType> is RANDOM).
        // - Within at most <dist> yards (if dist > 0.0f)
        // - At least -<dist> yards away (if dist < 0.0f)
        // - Is a player (if playerOnly = true)
        // - Not the current tank (if withTank = false)
        // - Has aura with ID <aura> (if aura > 0)
        // - Does not have aura with ID -<aura> (if aura < 0)
        // The resulting targets are stored in <targetList> (which is cleared first).
        void SelectTargetList(std::list<Unit*>& targetList, uint32 num, SelectAggroTarget targetType, uint32 offset = 0, float dist = 0.0f, bool playerOnly = false, bool withTank = true, int32 aura = 0);

        // Select the best (up to) <num> targets (in <targetType> order) satisfying <predicate> from the threat list and stores them in <targetList> (which is cleared first).
        // If <offset> is nonzero, the first <offset> entries in <targetType> order (or MAXTHREAT order, if <targetType> is RANDOM) are skipped.
        template <class PREDICATE>
        void SelectTargetList(std::list<Unit*>& targetList, uint32 num, SelectAggroTarget targetType, uint32 offset, PREDICATE const& predicate)
        {
            targetList.clear();
            ThreatManager& mgr = GetThreatManager();
            // shortcut: we're gonna ignore the first <offset> elements, and there's at most <offset> elements, so we ignore them all - nothing to do here
            if (mgr.GetThreatListSize() <= offset)
                return;

            if (targetType == SELECT_TARGET_MAXDISTANCE || targetType == SELECT_TARGET_MINDISTANCE)
            {
                for (ThreatReference const* ref : mgr.GetUnsortedThreatList())
                {
                    if (ref->IsOffline())
                        continue;

                    targetList.push_back(ref->GetVictim());
                }
            }
            else
            {
                Unit* currentVictim = mgr.GetCurrentVictim();
                if (currentVictim)
                    targetList.push_back(currentVictim);

                for (ThreatReference const* ref : mgr.GetSortedThreatList())
                {
                    if (ref->IsOffline())
                        continue;

                    Unit* thisTarget = ref->GetVictim();
                    if (thisTarget != currentVictim)
                        targetList.push_back(thisTarget);
                }
            }

            // shortcut: the list isn't gonna get any larger
            if (targetList.size() <= offset)
            {
                targetList.clear();
                return;
            }

            // right now, list is unsorted for DISTANCE types - re-sort by MAXDISTANCE
            if (targetType == SELECT_TARGET_MAXDISTANCE || targetType == SELECT_TARGET_MINDISTANCE)
                SortByDistance(targetList, targetType == SELECT_TARGET_MINDISTANCE);

            // now the list is MAX sorted, reverse for MIN types
            if (targetType == SELECT_TARGET_MINTHREAT)
                targetList.reverse();

            // ignore the first <offset> elements
            while (offset)
            {
                targetList.pop_front();
                --offset;
            }

            // then finally filter by predicate
            targetList.remove_if([&predicate](Unit* target) { return !predicate(target); });

            if (targetList.size() <= num)
                return;

            if (targetType == SELECT_TARGET_RANDOM)
                Trinity::Containers::RandomResize(targetList, num);
            else
                targetList.resize(num);
        }

        /*
        Unit* SelectTarget(SelectAggroTarget target, uint32 position);
        Unit* SelectTarget(SelectAggroTarget target, uint32 position, float dist, bool playerOnly, bool noTank = false);
        Unit* SelectTarget(SelectAggroTarget target, uint32 position, float distNear, float distFar, bool playerOnly);
        Unit* SelectTarget(uint32 position, float distMin, float distMax, bool playerOnly, bool auraCheck, bool exceptPossesed, uint32 spellId, uint32 effIndex);
        */

    private:
        UnitAI(UnitAI const& right) = delete;
        UnitAI& operator=(UnitAI const& right) = delete;

        ThreatManager& GetThreatManager();
        void SortByDistance(std::list<Unit*> list, bool ascending = true);
};

#endif //TRINITY_UNITAI_H
