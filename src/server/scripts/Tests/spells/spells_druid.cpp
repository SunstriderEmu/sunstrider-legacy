#include "../ClassSpellsDamage.h"
#include "../ClassSpellsCoeff.h"

class WrathTest : public TestCaseScript
{
public:
	WrathTest() : TestCaseScript("spells druid wrath") { }

	class WrathTestImpt : public TestCase
	{
	public:
		WrathTestImpt() : TestCase(true) { }

		void Test() override
		{
			TestPlayer* druid = SpawnRandomPlayer(CLASS_DRUID);
			Creature* creature = SpawnCreature();

			EQUIP_ITEM(druid, 34182); // Grand Magister's Staff of Torrents - 266 SP
			druid->DisableRegeneration(true);

			uint32 staffSP = 266;
			TEST_ASSERT(druid->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_ALL) == staffSP);

			// Mana cost
			uint32 const expectedWrathMana = 255;
			TEST_POWER_COST(druid, creature, ClassSpells::Druid::WRATH_RNK_10, 2000, POWER_MANA, expectedWrathMana);

			// Spell coefficient
			float const wrathSpellCoeff = ClassSpellsCoeff::Druid::WRATH;
			uint32 const wrathBonusSP = wrathSpellCoeff * staffSP;

			// Damage
			uint32 const expectedWrathMinDmg = ClassSpellsDamage::Druid::WRATH_RNK_10_MIN + wrathBonusSP;
			uint32 const expectedWrathMaxDmg = ClassSpellsDamage::Druid::WRATH_RNK_10_MAX + wrathBonusSP;
			TEST_DIRECT_SPELL_DAMAGE(druid, creature, ClassSpells::Druid::WRATH_RNK_10, expectedWrathMinDmg, expectedWrathMaxDmg);

			// Crit damage
			uint32 const expectedWrathCritMinDmg = expectedWrathMinDmg * 1.5f;
			uint32 const expectedWrathCritMaxDmg = expectedWrathMaxDmg * 1.5f;
			//TEST_DIRECT_SPELL_DAMAGE(druid, creature, ClassSpells::Druid::WRATH_RNK_10, expectedWrathMinDmg, expectedWrathMaxDmg);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<WrathTestImpt>();
	}
};

void AddSC_test_spells_druid()
{
	// Total:
	// Balance:
	new WrathTest();
}
