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

class BlindTest : public TestCaseScript
{
public:
    BlindTest() : TestCaseScript("spells rogue blind") { }

    class BlindTestImpt : public TestCase
    {
    public:
        BlindTestImpt() : TestCase(STATUS_PASSING_INCOMPLETE) { }

        void Test() override
        {
            TestPlayer* rogue = SpawnPlayer(CLASS_ROGUE, RACE_HUMAN);
            Creature* dummy = SpawnCreature();

            rogue->CastSpell(dummy, ClassSpells::Rogue::BLIND_RNK_1, true);
            Wait(1000);
            TEST_HAS_AURA(dummy, ClassSpells::Rogue::BLIND_RNK_1);

            //make sure the target is empty (else visual is wrong)
            TEST_ASSERT(dummy->GetTarget() == ObjectGuid::Empty);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<BlindTestImpt>();
    }
};

void AddSC_test_spells_rogue()
{
    new SinisterStrikeTest();
    new BlindTest();
}
