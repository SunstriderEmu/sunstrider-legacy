
#ifndef TRINITYCORE_TOTEM_H
#define TRINITYCORE_TOTEM_H

#include "Creature.h"

enum TotemType
{
    TOTEM_PASSIVE    = 0,
    TOTEM_ACTIVE     = 1,
    TOTEM_STATUE     = 2
};

#define SENTRY_TOTEM_ENTRY  3968

class TC_GAME_API Totem : public Minion
{
    public:
        explicit Totem(SummonPropertiesEntry const* properties, Unit* owner);
        ~Totem() override;
        void Update( uint32 time ) override;
		void InitStats(uint32 duration) override;
		void InitSummon() override;
        //void Summon(Unit* owner);
        void UnSummon(uint32 msTime = 0) override;
		uint32 GetSpell(uint8 slot = 0) const { return m_spells[slot]; }
        uint32 GetTotemDuration() const { return m_duration; }
        TotemType GetTotemType() const { return m_type; }
        void SetDuration(uint32 dur) { m_duration = dur; }
        void SetOwner(uint64 guid);

        bool UpdateStats(Stats /*stat*/) override { return true; }
        bool UpdateAllStats() override { return true; }
        void UpdateResistances(uint32 /*school*/) override {}
        void UpdateArmor() override {}
        void UpdateMaxHealth() override {}
        void UpdateMaxPower(Powers /*power*/) override {}
        void UpdateAttackPowerAndDamage(bool /*ranged*/ ) override {}
        void UpdateDamagePhysical(WeaponAttackType /*attType*/) override {}

        bool IsImmunedToSpellEffect(SpellInfo const* spellInfo, uint32 index, Unit* caster) const override;
    protected:
        TotemType m_type;
        uint32 m_duration;
};


#endif

