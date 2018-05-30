#include "../ClassSpellsDamage.h"
#include "../ClassSpellsCoeff.h"

class SinisterStrikeTest : public TestCaseScript
{
public:
    SinisterStrikeTest() : TestCaseScript("spells rogue sinister_strike") { }

    //An instant strike that causes 98 damage in addition to your normal weapon damage.Awards 1 combo point.
    class SinisterStrikeTestImpt : public TestCase
    {
    public:
        SinisterStrikeTestImpt() : TestCase(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* rogue = SpawnPlayer(CLASS_ROGUE, RACE_HUMAN);
            Creature* dummy = SpawnCreature();

            // MH, OH, spells
            EQUIP_NEW_ITEM(rogue, 32837); // Warglaive of Azzinoth MH - 214-398 dmg
            Wait(1500);
            WaitNextUpdate();
            // Damage
            uint32 const sinisterStrikeBonus = 98;
            float const normalizedSwordSpeed = 2.4f;
            auto[expectedSinisterStrikeMin, expectedSinisterStrikeMax] = CalcMeleeDamage(rogue, dummy, BASE_ATTACK, sinisterStrikeBonus, normalizedSwordSpeed);
            TEST_DIRECT_SPELL_DAMAGE(rogue, dummy, ClassSpells::Rogue::SINISTER_STRIKE_RNK_10, expectedSinisterStrikeMin, expectedSinisterStrikeMax, false);
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
