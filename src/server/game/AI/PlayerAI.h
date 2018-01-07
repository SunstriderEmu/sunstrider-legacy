
#ifndef TRINITY_PLAYERAI_H
#define TRINITY_PLAYERAI_H

#include "UnitAI.h"
#include "Player.h"
#include "Spell.h"
#include "Creature.h"

class TC_GAME_API PlayerAI : public UnitAI
{
    public:
        explicit PlayerAI(Player* player) : UnitAI(static_cast<Unit*>(player)), me(player), _selfSpec(PlayerAI::GetPlayerSpec(player)), _isSelfHealer(PlayerAI::IsPlayerHealer(player)), _isSelfRangedAttacker(PlayerAI::IsPlayerRangedAttacker(player)) { }

        void OnCharmed(Unit* /*charmer*/, bool /*apply*/) override { } // charm AI application for players is handled by Unit::SetCharmedBy / Unit::RemoveCharmedBy

        // helper functions to determine player info
        // Return values range from 0 (left-most spec) to 2 (right-most spec). If two specs have the same number of talent points, the left-most of those specs is returned.
        static uint8 GetPlayerSpec(Player const* who);
        // Return values range from 0 (left-most spec) to 2 (right-most spec). If two specs have the same number of talent points, the left-most of those specs is returned.
        uint8 GetSpec(Player const* who = nullptr) const { return (!who || who == me) ? _selfSpec : GetPlayerSpec(who); }
        static bool IsPlayerHealer(Player const* who);
        bool IsHealer(Player const* who = nullptr) const { return (!who || who == me) ? _isSelfHealer : IsPlayerHealer(who); }
        static bool IsPlayerRangedAttacker(Player const* who);
        bool IsRangedAttacker(Player const* who = nullptr) const { return (!who || who == me) ? _isSelfRangedAttacker : IsPlayerRangedAttacker(who); }

    protected:
        struct TargetedSpell : public std::pair<Spell*, Unit*>
        {
            using std::pair<Spell*, Unit*>::pair;
            explicit operator bool() { return bool(first); }
        };
        typedef std::pair<TargetedSpell, uint32> PossibleSpell;
        typedef std::vector<PossibleSpell> PossibleSpellVector;

        Player* const me;
        void SetIsRangedAttacker(bool state) { _isSelfRangedAttacker = state; } // this allows overriding of the default ranged attacker detection

        enum SpellTarget
        {
            TARGET_NONE,
            TARGET_VICTIM,
            TARGET_CHARMER,
            TARGET_SELF
        };
        /* Check if the specified spell can be cast on that target.
           Caller is responsible for cleaning up created Spell object from pointer. */
        TargetedSpell VerifySpellCast(uint32 spellId, Unit* target);
        /* Check if the specified spell can be cast on that target.
        Caller is responsible for cleaning up created Spell object from pointer. */
        TargetedSpell VerifySpellCast(uint32 spellId, SpellTarget target);

        /* Helper method - checks spell cast, then pushes it onto provided vector if valid. */
        template<typename T> inline void VerifyAndPushSpellCast(PossibleSpellVector& spells, uint32 spellId, T target, uint32 weight)
        {
            if (TargetedSpell spell = VerifySpellCast(spellId, target))
                spells.push_back({ spell,weight });
        }

        /* Helper method - selects one spell from the vector and returns it, while deleting everything else.
           This invalidates the vector, and empties it to prevent accidental misuse. */
        TargetedSpell SelectSpellCast(PossibleSpellVector& spells);
        /* Helper method - casts the included spell at the included target */
        void DoCastAtTarget(TargetedSpell spell);

        virtual Unit* SelectAttackTarget() const { return me->GetCharmer() ? me->GetCharmer()->GetVictim() : nullptr; }
        void DoRangedAttackIfReady();
        void DoAutoAttackIfReady();

        // Cancels all shapeshifts that the player could voluntarily cancel
        void CancelAllShapeshifts();

    private:
        uint8 const _selfSpec;
        bool const _isSelfHealer;
        bool _isSelfRangedAttacker;
};

class SimpleCharmedPlayerAI : public PlayerAI
{
    public:
        SimpleCharmedPlayerAI(Player* player) : PlayerAI(player), _castCheckTimer(2500), _chaseCloser(false), _forceFacing(true), _isFollowing(false) { }
        void UpdateAI(const uint32 diff) override;
        void OnCharmed(Unit* /* charmer */, bool apply) override;

    protected:
        bool CanAIAttack(Unit const* who) const override;
        Unit* SelectAttackTarget() const override;

    private:
        TargetedSpell SelectAppropriateCastForSpec();
        uint32 _castCheckTimer;
        bool _chaseCloser;
        bool _forceFacing;
        bool _isFollowing;
};

#endif
