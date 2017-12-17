#include "../ClassSpellsDamage.h"
#include "../ClassSpellsCoeff.h"

class ShadowBoltTest : public TestCaseScript
{
public:
    ShadowBoltTest() : TestCaseScript("spells warlock shadow_bolt") { }

    class ShadowBoltTestImpt : public TestCase
    {
    public:
        ShadowBoltTestImpt() : TestCase(true) { }

        void Test() override
        {
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_HUMAN);
            EQUIP_ITEM(warlock, 34336); // Sunflare - 292 SP
            Creature* dummy = SpawnCreature();

            uint32 const startSP = warlock->GetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW);
            ASSERT_INFO("SP: %u", startSP);
            TEST_ASSERT(startSP == 292);

            // Damage
            float const castTime = 3.0f;
            float const spellCoefficient = castTime / 3.5f;
            uint32 const expectedSBMin = ClassSpellsDamage::Warlock::SHADOW_BOLT_RNK_11_MIN + startSP * spellCoefficient;
            uint32 const expectedSBMax = ClassSpellsDamage::Warlock::SHADOW_BOLT_RNK_11_MAX + startSP * spellCoefficient;
            TEST_DIRECT_SPELL_DAMAGE(warlock, dummy, ClassSpells::Warlock::SHADOW_BOLT_RNK_11, expectedSBMin, expectedSBMax, false);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<ShadowBoltTestImpt>();
    }
};

void AddSC_test_spells_warlock()
{
    new ShadowBoltTest();
}
