#include "TestCase.h"
#include "TestPlayer.h"
#include "World.h"
#include "ClassSpells.h"

class WandSpecializationTest : public TestCaseScript
{
public:
    WandSpecializationTest() : TestCaseScript("talents priest wand_specialization") { }

    class WandSpecializationTestImpt : public TestCase
    {
    public:
        WandSpecializationTestImpt() : TestCase(true) { }

        void Test() override
        {
            TestPlayer* player = SpawnRandomPlayer(CLASS_PRIEST);
            EquipItem(player, 28783); //Eredar Wand of Obliteration, 177 - 330 Shadow Damage
            uint32 const wandMinDamage = 177;
            uint32 const wandMaxDamage = 330;

            Creature* dummyTarget = SpawnCreature();
            //Test regular damage
            TestDirectSpellDamage(player, dummyTarget, ClassSpells::Priest::WAND, wandMinDamage, wandMaxDamage);

            //Test improved damage 5%
            LearnTalent(player, Talents::Priest::WAND_SPECIALIZATION_RNK_1);
            TestDirectSpellDamage(player, dummyTarget, ClassSpells::Priest::WAND, wandMinDamage * 1.05f, wandMaxDamage * 1.05f);

            //Test improved damage 25%
            LearnTalent(player, Talents::Priest::WAND_SPECIALIZATION_RNK_5);
            TestDirectSpellDamage(player, dummyTarget, ClassSpells::Priest::WAND, wandMinDamage * 1.25f, wandMaxDamage * 1.25f);
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
		SearingLightTestImpt() : TestCase(true) { }

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_PRIEST);
            Wait(10000);
			// Smite rank 10
			uint32 const smiteMinDamage = 549;
			uint32 const smiteMaxDamage = 616;

			// Holy fire rank 9
			uint32 const holyFireMinDamage = 426;
			uint32 const holyFireMaxDamage = 537;

			Creature* dummyTarget = SpawnCreature();
			//Test regular damage
			TestDirectSpellDamage(player, dummyTarget, ClassSpells::Priest::SMITE_RNK_10, smiteMinDamage, smiteMaxDamage);
			TestDirectSpellDamage(player, dummyTarget, ClassSpells::Priest::HOLY_FIRE_RNK_9, smiteMinDamage, smiteMaxDamage);

			//Test improved damage 5%
			LearnTalent(player, Talents::Priest::SEARING_LIGHT_RNK_1);
			TestDirectSpellDamage(player, dummyTarget, ClassSpells::Priest::SMITE_RNK_10, smiteMaxDamage * 1.05f, smiteMaxDamage * 1.05f);
			TestDirectSpellDamage(player, dummyTarget, ClassSpells::Priest::HOLY_FIRE_RNK_9, holyFireMinDamage * 1.05f, holyFireMaxDamage * 1.05f);

			//Test improved damage 10%
			LearnTalent(player, Talents::Priest::SEARING_LIGHT_RNK_2);
			TestDirectSpellDamage(player, dummyTarget, ClassSpells::Priest::SMITE_RNK_10, smiteMaxDamage * 1.1f, smiteMaxDamage * 1.1f);
			TestDirectSpellDamage(player, dummyTarget, ClassSpells::Priest::HOLY_FIRE_RNK_9, holyFireMinDamage * 1.1f, holyFireMaxDamage * 1.1f);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<SearingLightTestImpt>();
	}
};

void AddSC_test_talents_priest()
{
	new WandSpecializationTest();
	new SearingLightTest();
}
