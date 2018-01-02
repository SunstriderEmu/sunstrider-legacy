#ifndef TRINITY_UNITAI_H
#define TRINITY_UNITAI_H

#include "Define.h"
#include "Unit.h"
#include "QuestDef.h"
class Unit;
class Quest;
class Player;

//Selection method used by SelectSpellTarget
enum SelectAggroTarget : int
{
    SELECT_TARGET_RANDOM = 0,                               //Just selects a random target
    SELECT_TARGET_TOPAGGRO,                                 //Selects targes from top aggro to bottom
    SELECT_TARGET_BOTTOMAGGRO,                              //Selects targets from bottom aggro to top
    SELECT_TARGET_NEAREST,
    SELECT_TARGET_FARTHEST,
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

        // Called when unit is charmed
        virtual void OnCharmed(Unit* charmer, bool apply) = 0;
        virtual void OnPossess(Unit* charmer, bool apply) { }

        // Pass parameters between AI
        virtual void DoAction(const int32 param) {}
        virtual uint32 GetData(uint32 /*id = 0*/) const { return 0; }
        virtual void SetData(uint32 /*id*/, uint32 /*value*/, Unit* setter = nullptr) {}
        virtual void SetGUID(ObjectGuid /*guid*/, int32 /*id*/ = 0) { }
        virtual ObjectGuid GetGUID(int32 /*id*/ = 0) const { return ObjectGuid::Empty; }

        // Called at any Damage from any attacker (before damage apply)
        virtual void DamageTaken(Unit *done_by, uint32 & /*damage*/) {}
        
        // Called at any Damage to any victim (before damage apply)
        virtual void DamageDealt(Unit* /*victim*/, uint32& /*damage*/, DamageEffectType /*damageType*/) { }

        // Called when the creature receives heal
        virtual void HealReceived(Unit* /*done_by*/, uint32& /*addhealth*/) {}

        // Called when the unit heals
        virtual void HealDone(Unit* /*done_to*/, uint32& /*addhealth*/) { }

        //Cast spell by Id, return SpellCastResult
        uint32 DoCast(Unit* victim, uint32 spellId, bool triggered = false);
        uint32 DoCastSelf(uint32 spellId, bool triggered = false) { return DoCast(me, spellId, triggered); }
        uint32 DoCastAOE(uint32 spellId, bool triggered = false);
        uint32 DoCastVictim(uint32 spellId, bool triggered = false);
        //Cast spell by spell info
        uint32 DoCastSpell(Unit* who, SpellInfo const *spellInfo, bool triggered = false);

        //Do melee swing of current victim if in rnage and ready and not casting
        virtual void DoMeleeAttackIfReady();
        bool DoSpellAttackIfReady(uint32 spell);

        //Selects a unit from the creature's current aggro list
        bool checkTarget(Unit* target, bool playersOnly, float radius, bool noTank = false);

        //SelectTargetFromPlayerList -> me->AI()->SelectTarget(SELECT_TARGET_RANDOM, 0.0f, 500.0f, true)
        // Select the targets satisfying the predicate.
        // predicate shall extend std::unary_function<Unit*, bool>
        template<class PREDICATE> Unit* SelectTarget(SelectAggroTarget targetType, uint32 position, PREDICATE const& predicate)
        {
            ThreatContainer::StorageType const& threatlist = me->GetThreatManager().getThreatList();
            if (position >= threatlist.size())
                return nullptr;

            std::list<Unit*> targetList;
            for (auto itr : threatlist)
                if (predicate(itr->getTarget()))
                    targetList.push_back(itr->getTarget());

            if (position >= targetList.size())
                return nullptr;

            if (targetType == SELECT_TARGET_NEAREST || targetType == SELECT_TARGET_FARTHEST)
                targetList.sort(Trinity::ObjectDistanceOrderPred(me));

            switch (targetType)
            {
            case SELECT_TARGET_NEAREST:
            case SELECT_TARGET_TOPAGGRO:
            {
                auto itr = targetList.begin();
                std::advance(itr, position);
                return *itr;
            }
            case SELECT_TARGET_FARTHEST:
            case SELECT_TARGET_BOTTOMAGGRO:
            {
                auto ritr = targetList.rbegin();
                std::advance(ritr, position);
                return *ritr;
            }
            case SELECT_TARGET_RANDOM:
            {
                auto itr = targetList.begin();
                std::advance(itr, urand(position, targetList.size() - 1));
                return *itr;
            }
            default:
                break;
            }

            return nullptr;
        }

        Unit* SelectTarget(SelectAggroTarget target, uint32 position);
        Unit* SelectTarget(SelectAggroTarget target, uint32 position, float dist, bool playerOnly, bool noTank = false);
        Unit* SelectTarget(SelectAggroTarget target, uint32 position, float distNear, float distFar, bool playerOnly);
        Unit* SelectTarget(uint32 position, float distMin, float distMax, bool playerOnly, bool auraCheck, bool exceptPossesed, uint32 spellId, uint32 effIndex);
        void SelectUnitList(std::list<Unit*> &targetList, uint32 num, SelectAggroTarget target, float dist, bool playerOnly, uint32 notHavingAuraId = 0, uint8 effIndex = 0);
};

#endif //TRINITY_UNITAI_H
