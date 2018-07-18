#include "../ClassSpellsDamage.h"
#include "../ClassSpellsCoeff.h"

// "spells rogue sinister_strike"
// An instant strike that causes 98 damage in addition to your normal weapon damage.Awards 1 combo point.
class SinisterStrikeTest : public TestCase
{
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

//"spells rogue blind"
class BlindTest : public TestCase
{
    void Test() override
    {
        TestPlayer* rogue = SpawnPlayer(CLASS_ROGUE, RACE_HUMAN);
        Creature* dummy = SpawnCreature();
            
        FORCE_CAST(rogue, dummy, ClassSpells::Rogue::BLIND_RNK_1, SPELL_MISS_NONE, TRIGGERED_FULL_MASK);
        Wait(1000); //wait for fly time if any. Our core does have a little fake fly time.
        TEST_HAS_AURA(dummy, ClassSpells::Rogue::BLIND_RNK_1);

        SECTION("Target selection", [&] {
            //make sure the target selection is empty (else visual is wrong)
            TEST_ASSERT(dummy->GetTarget() == ObjectGuid::Empty);
        });

        SECTION("Unit state", [&] {
            dummy->HasUnitState(UNIT_STATE_CONFUSED_MOVE);
        });
    }
};

void AddSC_test_spells_rogue()
{
    RegisterTestCase("spells rogue sinister_strike", SinisterStrikeTest);
    RegisterTestCase("spells rogue blind", BlindTest);
}
