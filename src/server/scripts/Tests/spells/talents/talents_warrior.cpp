#include "../../ClassSpellsDamage.h"
#include "../../ClassSpellsCoeff.h"

#define WG_MH 32837
#define WG_OH 32838

#define MELEE_CRIT_FACTOR 2.0f

class ImpaleTest : public TestCaseScript
{
public:
    // "Increases the critical strike damage bonus of your abilities in Battle, Defensive, and Berserker stance by 20%."
    ImpaleTest() : TestCaseScript("talents warrior impale") { }

    class ImpaleTestImpl : public TestCase
    {
    public:
        void Test() override
        {
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN);
            FORCE_CAST(warrior, warrior, ClassSpells::Warrior::BATTLE_STANCE_RNK_1, SPELL_MISS_NONE, TRIGGERED_FULL_MASK);
            Creature* dummy = SpawnCreature();
            EQUIP_NEW_ITEM(warrior, WG_MH);
            EQUIP_NEW_ITEM(warrior, WG_OH);
            float talentCritFactorBonus = 0.2f;
            float const armorFactor = GetArmorFactor(warrior, dummy);
            float const critMultiplicator = MELEE_CRIT_FACTOR + talentCritFactorBonus;

            SECTION("Bloodthirst", [&] {
                uint32 const expectedDmg = float(ClassSpellsDamage::Warrior::BLOODTHIRST_RNK_6) / 100.0f * warrior->GetTotalAttackPowerValue(BASE_ATTACK) * armorFactor * MELEE_CRIT_FACTOR;

                //without talent
                TEST_DIRECT_SPELL_DAMAGE(warrior, dummy, ClassSpells::Warrior::BLOODTHIRST_RNK_6, expectedDmg, expectedDmg, true);

                //with talent
                warrior->AddAura(Talents::Warrior::IMPALE_RNK_2, warrior);
                uint32 const expectedDmgWithTalent = float(ClassSpellsDamage::Warrior::BLOODTHIRST_RNK_6) / 100.0f * warrior->GetTotalAttackPowerValue(BASE_ATTACK) * armorFactor * critMultiplicator;
                TEST_DIRECT_SPELL_DAMAGE(warrior, dummy, ClassSpells::Warrior::BLOODTHIRST_RNK_6, expectedDmgWithTalent, expectedDmgWithTalent, true);
            });

            warrior->RemoveAurasDueToSpell(Talents::Warrior::IMPALE_RNK_2);

            SECTION("Heroic strike", [&] {
                //without talent
                auto[minHeroicStrike, maxHeroicStrike] = CalcMeleeDamage(warrior, dummy, BASE_ATTACK, ClassSpellsDamage::Warrior::HEROIC_STRIKE_RNK_10);
                TEST_DIRECT_SPELL_DAMAGE(warrior, dummy, ClassSpells::Warrior::HEROIC_STRIKE_RNK_10, minHeroicStrike * MELEE_CRIT_FACTOR, maxHeroicStrike * MELEE_CRIT_FACTOR, true);

                //with talent
                warrior->AddAura(Talents::Warrior::IMPALE_RNK_2, warrior);
                TEST_DIRECT_SPELL_DAMAGE(warrior, dummy, ClassSpells::Warrior::HEROIC_STRIKE_RNK_10, minHeroicStrike * critMultiplicator, maxHeroicStrike * critMultiplicator, true);
            });

            SECTION("WIP", STATUS_WIP, [&] {
                //TODO: ALL WARRIOR SPELLS
            });
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<ImpaleTestImpl>();
    }
};


void AddSC_test_talents_warrior()
{
    //Fury
    //Prot
    //Arms
    new ImpaleTest();
}
