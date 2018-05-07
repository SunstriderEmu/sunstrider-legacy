#include "../ClassSpellsDamage.h"
#include "../ClassSpellsCoeff.h"

class SinisterStrikeTest : public TestCaseScript
{
public:
    SinisterStrikeTest() : TestCaseScript("spells rogue sinister_strike") { }

    class SinisterStrikeTestImpt : public TestCase
    {
    public:
        SinisterStrikeTestImpt() : TestCase(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* rogue = SpawnPlayer(CLASS_ROGUE, RACE_HUMAN);
            Creature* dummy = SpawnCreature();

            // MH, OH, spells
            EQUIP_NEW_ITEM(rogue, 32837); // Warglaive of Azzinoth MH
            Wait(1500);
            EQUIP_NEW_ITEM(rogue, 32838); // Warglaive of Azzinoth OH
            WaitNextUpdate();
            // Damage
            int const sinisterStrikeBonus = 98;
            float const normalizedSwordSpeed = 2.4f;
            float const AP = rogue->GetTotalAttackPowerValue(BASE_ATTACK);
            float const armorFactor = 1 - (dummy->GetArmor() / (dummy->GetArmor() + 10557.5f));
            // Sinister strike
            uint32 const weaponMinDamage = 214 + (AP / 14 * normalizedSwordSpeed) + sinisterStrikeBonus;
            uint32 const weaponMaxDamage = 398 + (AP / 14 * normalizedSwordSpeed) + sinisterStrikeBonus;
            uint32 const expectedSinisterStrikeMin = weaponMinDamage * armorFactor;
            uint32 const expectedSinisterStrikeMax = weaponMaxDamage * armorFactor;
            uint32 const expectedSinisterStrikeCritMin = weaponMinDamage * 2.0f * armorFactor;
            uint32 const expectedSinisterStrikeCritMax = weaponMaxDamage * 2.0f * armorFactor;
            TC_LOG_DEBUG("test.unit_test", "dummy: %u, armor: %f, min: %u, max: %u", dummy->GetArmor(), armorFactor, expectedSinisterStrikeMin, expectedSinisterStrikeMax);
            TEST_DIRECT_SPELL_DAMAGE(rogue, dummy, ClassSpells::Rogue::SINISTER_STRIKE_RNK_10, expectedSinisterStrikeMin, expectedSinisterStrikeMax, false);
            TEST_DIRECT_SPELL_DAMAGE(rogue, dummy, ClassSpells::Rogue::SINISTER_STRIKE_RNK_10, expectedSinisterStrikeCritMin, expectedSinisterStrikeCritMax, true);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<SinisterStrikeTestImpt>();
    }
};

void AddSC_test_spells_rogue()
{
    new SinisterStrikeTest();
}
