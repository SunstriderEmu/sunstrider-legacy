#include "../../ClassSpellsDamage.h"
#include "../../ClassSpellsCoeff.h"

class PriestWandSpecializationTest : public TestCaseScript
{
public:
    PriestWandSpecializationTest() : TestCaseScript("talents priest wand_specialization") { }

    class WandSpecializationTestImpt : public TestCase
    {
    public:
        WandSpecializationTestImpt() : TestCase(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* player = SpawnRandomPlayer(CLASS_PRIEST);
            EQUIP_NEW_ITEM(player, 28783); //Eredar Wand of Obliteration, 177 - 330 Shadow Damage
            uint32 const wandMinDamage = 177;
            uint32 const wandMaxDamage = 330;

            Creature* dummyTarget = SpawnCreature();
            //Test regular damage
            TEST_DIRECT_SPELL_DAMAGE(player, dummyTarget, ClassSpells::Priest::WAND, wandMinDamage, wandMaxDamage, false);

            //Test improved damage 5%
            LearnTalent(player, Talents::Priest::WAND_SPECIALIZATION_RNK_1);
            TEST_DIRECT_SPELL_DAMAGE(player, dummyTarget, ClassSpells::Priest::WAND, wandMinDamage * 1.05f, wandMaxDamage * 1.05f, false);

            //Test improved damage 25%
            LearnTalent(player, Talents::Priest::WAND_SPECIALIZATION_RNK_5);
            TEST_DIRECT_SPELL_DAMAGE(player, dummyTarget, ClassSpells::Priest::WAND, wandMinDamage * 1.25f, wandMaxDamage * 1.25f, false);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<WandSpecializationTestImpt>();
    }
};

class SearingLightTest : public TestCaseScript
{
public:

	SearingLightTest() : TestCaseScript("talents priest searing_light") { }

	class SearingLightTestImpt : public TestCase
	{
	public:
		SearingLightTestImpt() : TestCase(STATUS_PASSING) { }

		void Test() override
		{
			TestPlayer* priest = SpawnRandomPlayer(CLASS_PRIEST);
			// Smite
			uint32 const smiteMinDamage = ClassSpellsDamage::Priest::SMITE_RNK_10_MIN;
			uint32 const smiteMaxDamage = ClassSpellsDamage::Priest::SMITE_RNK_10_MAX;

			// Holy Fire
			uint32 const holyFireMinDamage = ClassSpellsDamage::Priest::HOLY_FIRE_RNK_9_MIN;
			uint32 const holyFireMaxDamage = ClassSpellsDamage::Priest::HOLY_FIRE_RNK_9_MAX;
            uint32 const holyFireDotDamage = 5.0f * floor(ClassSpellsDamage::Priest::HOLY_FIRE_RNK_9_TOTAL / 5.0f);

			Creature* dummyTarget = SpawnCreature();

			//Test improved damage 10%
			LearnTalent(priest, Talents::Priest::SEARING_LIGHT_RNK_2);
            TEST_DIRECT_SPELL_DAMAGE(priest, dummyTarget, ClassSpells::Priest::SMITE_RNK_10, smiteMinDamage * 1.1f, smiteMaxDamage * 1.1f, false);
            TEST_DIRECT_SPELL_DAMAGE(priest, dummyTarget, ClassSpells::Priest::HOLY_FIRE_RNK_9, holyFireMinDamage * 1.1f, holyFireMaxDamage * 1.1f, false);
            TEST_DOT_DAMAGE(priest, dummyTarget, ClassSpells::Priest::HOLY_FIRE_RNK_9, holyFireDotDamage * 1.1f, false);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<SearingLightTestImpt>();
	}
};

void AddSC_test_talents_priest()
{
	new PriestWandSpecializationTest();
	new SearingLightTest();
}
