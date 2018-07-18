#include "../../ClassSpellsDamage.h"
#include "../../ClassSpellsCoeff.h"
#include "PlayerbotAI.h"
#include "SpellHistory.h"

// Helpers to get shapeshift base damage
// http://wowwiki.wikia.com/wiki/Attack_power?oldid=1715193

uint32 GetMoonkinBaseAP(TestPlayer* player, uint32 level = 70)
{
	uint32 const startStr = player->GetStat(STAT_STRENGTH);

	uint32 const moonkinLevelBonus = level * 1.5f;
	uint32 const moonkinMeleeBase = startStr * 2 + moonkinLevelBonus - 20;

	return moonkinMeleeBase;
}

uint32 GetCatBaseAP(TestPlayer* player, uint32 level = 70)
{
	uint32 const startAgi = player->GetStat(STAT_AGILITY);
	uint32 const startStr = player->GetStat(STAT_STRENGTH);

	uint32 const catLevelBonus = level * 2.0f;
	uint32 const catMeleeBase = startStr * 2 + startAgi + catLevelBonus - 20;

	return catMeleeBase;
}

uint32 GetBearBaseAP(TestPlayer* player, uint32 level = 70)
{
	uint32 const startStr = player->GetStat(STAT_STRENGTH);

	uint32 const bearLevelBonus = level * 3.0f;
	uint32 const bearMeleeBase = startStr * 2 + bearLevelBonus - 20;

	return bearMeleeBase;
}

uint32 GetDireBearBaseAP(TestPlayer* player, uint32 level = 70)
{
	uint32 const startStr = player->GetStat(STAT_STRENGTH);

	uint32 const bearLevelBonus = level * 3.0f;
	uint32 const bearMeleeBase = startStr * 2 + bearLevelBonus - 20;

	return bearMeleeBase;
}

class StarlightWrathTest : public TestCaseScript
{
public:

	StarlightWrathTest() : TestCaseScript("talents druid starlight_wrath") { }

	class StarlightWrathTestImpt : public TestCase
	{
	public:
		StarlightWrathTestImpt() : TestCase(STATUS_PASSING) { }

		void Test() override
		{
			TestPlayer* druid = SpawnRandomPlayer(CLASS_DRUID);
			LearnTalent(druid, Talents::Druid::STARLIGHT_WRATH_RNK_5);
            TEST_CAST_TIME(druid, ClassSpells::Druid::WRATH_RNK_10, 1500);
		}
	};

	std::unique_ptr<TestCase> GetTest() const override
	{
		return std::make_unique<StarlightWrathTestImpt>();
	}
};

class NaturesGraspTest : public TestCaseScript
{
public:

    NaturesGraspTest() : TestCaseScript("talents druid natures_grasp") { }

    class NaturesGraspTestImpt : public TestCase
    {
    public:
        NaturesGraspTestImpt() : TestCase(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_HUMAN);

            LearnTalent(druid, Talents::Druid::NATURES_GRASP_RNK_1);
            float const talentFactor = 35.f;

            TEST_MELEE_PROC_CHANCE(warrior, druid, ClassSpells::Druid::NATURES_GRASP_RNK_7_PROC, true, talentFactor, MELEE_HIT_NORMAL, BASE_ATTACK, [](Unit* attacker, Unit* victim) {
                victim->AddAura(ClassSpells::Druid::NATURES_GRASP_RNK_7, victim);
            });
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<NaturesGraspTestImpt>();
    }
};

class ImprovedNaturesGraspTest : public TestCaseScript
{
public:

	ImprovedNaturesGraspTest() : TestCaseScript("talents druid improved_natures_grasp") { }

	class ImprovedNaturesGraspTestImpt : public TestCase
	{
	public:
		ImprovedNaturesGraspTestImpt() : TestCase(STATUS_PASSING) { }

		void Test() override
		{
			TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);
			TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_HUMAN);

			LearnTalent(druid, Talents::Druid::IMPROVED_NATURES_GRASP_RNK_4);
            float const talentFactor = 100.f;

            TEST_MELEE_PROC_CHANCE(warrior, druid, ClassSpells::Druid::NATURES_GRASP_RNK_7_PROC, true, talentFactor, MELEE_HIT_NORMAL, BASE_ATTACK, [](Unit* attacker, Unit* victim) {
                victim->AddAura(ClassSpells::Druid::NATURES_GRASP_RNK_7, victim);
            });
		}
	};

	std::unique_ptr<TestCase> GetTest() const override
	{
		return std::make_unique<ImprovedNaturesGraspTestImpt>();
	}
};

class ControlOfNatureTest : public TestCaseScript
{
public:

    ControlOfNatureTest() : TestCaseScript("talents druid control_of_nature") { }

    class ControlOfNatureTestImpt : public TestCase
    {
    public:
        ControlOfNatureTestImpt() : TestCase(STATUS_WIP) { }

        void Test() override
        {
            TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_HUMAN);

            LearnTalent(druid, Talents::Druid::CONTROL_OF_NATURE_RNK_3);
            float const talentResistPushbackFactor = 100.f;

            TEST_PUSHBACK_RESIST_CHANCE(druid, warrior, ClassSpells::Druid::NATURES_GRASP_RNK_7, talentResistPushbackFactor);
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<ControlOfNatureTestImpt>();
    }
};

class FocusedStarlightTest : public TestCaseScript
{
public:

    FocusedStarlightTest() : TestCaseScript("talents druid focused_starlight") { }

    class FocusedStarlightTestImpt : public TestCase
    {
    public:
        FocusedStarlightTestImpt() : TestCase(STATUS_WIP) { }

        void Test() override
        {
            TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);
            Creature* dummy = SpawnCreature();

            LearnTalent(druid, Talents::Druid::FOCUSED_STARLIGHT_RNK_2);
            float const talentCritFactor = 4.f;
            float const expectedCritChance = druid->GetFloatValue(PLAYER_SPELL_CRIT_PERCENTAGE1 + SPELL_SCHOOL_SHADOW) + talentCritFactor;

            TEST_SPELL_CRIT_CHANCE(druid, dummy, ClassSpells::Druid::WRATH_RNK_10, expectedCritChance);
            TEST_SPELL_CRIT_CHANCE(druid, dummy, ClassSpells::Druid::STARFIRE_RNK_8, expectedCritChance);
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<FocusedStarlightTestImpt>();
    }
};

class ImprovedMoonfireTest : public TestCaseScript
{
public:

    ImprovedMoonfireTest() : TestCaseScript("talents druid improved_moonfire") { }

    class ImprovedMoonfireTestImpt : public TestCase
    {
    public:
        ImprovedMoonfireTestImpt() : TestCase(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);
            Creature* dummy = SpawnCreature();

            LearnTalent(druid, Talents::Druid::IMPROVED_MOONFIRE_RNK_2);
            float const talentDamageFactor = 1.1f;
            float const talentCritFactor = 10.f;

            // Crit
            float const expectedCritChance = druid->GetFloatValue(PLAYER_SPELL_CRIT_PERCENTAGE1 + SPELL_SCHOOL_ARCANE) + talentCritFactor;
            TEST_SPELL_CRIT_CHANCE(druid, dummy, ClassSpells::Druid::MOONFIRE_RNK_12, expectedCritChance);

			// Direct Damage
            uint32 const expectedMoonfireMinDmg = ClassSpellsDamage::Druid::MOONFIRE_RNK_12_MIN * talentDamageFactor;
            uint32 const expectedMoonfireMaxDmg = ClassSpellsDamage::Druid::MOONFIRE_RNK_12_MAX * talentDamageFactor;
            TEST_DIRECT_SPELL_DAMAGE(druid, dummy, ClassSpells::Druid::MOONFIRE_RNK_12, expectedMoonfireMinDmg, expectedMoonfireMaxDmg, false);

            // DoT
            uint32 const moonfireTickAmount = 4;
            uint32 const moonfireTick = ClassSpellsDamage::Druid::MOONFIRE_RNK_12_TICK * talentDamageFactor;
            uint32 const expectedMoonfireTotalDmg = moonfireTickAmount * moonfireTick;
            TEST_DOT_DAMAGE(druid, dummy, ClassSpells::Druid::MOONFIRE_RNK_12, expectedMoonfireTotalDmg, false);
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<ImprovedMoonfireTestImpt>();
    }
};

/*
"Increases damage caused by your Thorns and Entangling Roots spells by 75%."
Bugs:
- Entangling roots does 1/3 of the damage it should do (base spell is bugged?)
*/
class BramblesTest : public TestCase
{
public:
    void Test() override
    {
        TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);
        TestPlayer* rogue = SpawnPlayer(CLASS_ROGUE, RACE_HUMAN);

        rogue->ForceMeleeHitResult(MELEE_HIT_NORMAL);
        druid->ForceSpellHitResultOverride(SPELL_MISS_NONE); //Thorns can miss
        LearnTalent(druid, Talents::Druid::BRAMBLES_RNK_3);
        float const talentDamageFactor = 1.75f;

        SECTION("Thorns damage", [&]{
            TEST_CAST(druid, druid, ClassSpells::Druid::THORNS_RNK_7);
            uint32 const thornsDmg = ClassSpellsDamage::Druid::THORNS_RNK_7_TICK * talentDamageFactor;
            uint32 const rogueExpectedHealth = rogue->GetHealth() - thornsDmg;
            rogue->AttackerStateUpdate(druid, BASE_ATTACK);
            rogue->AttackStop();
            ASSERT_INFO("Rogue has %u HP but should have %u HP.", rogue->GetHealth(), rogueExpectedHealth);
            TEST_ASSERT(rogue->GetHealth() == rogueExpectedHealth);
        });

        SECTION("Entangling Roots damage", STATUS_KNOWN_BUG, [&] {
            uint32 const tickAmount = 9;
            uint32 const entanglingRootsTick = ClassSpellsDamage::Druid::ENTANGLING_ROOTS_RNK_7_TICK * talentDamageFactor;
            uint32 const expectedEntanglingRootsDmg = tickAmount * entanglingRootsTick;
            TEST_DOT_DAMAGE(druid, rogue, ClassSpells::Druid::ENTANGLING_ROOTS_RNK_7, expectedEntanglingRootsDmg, false);
        });
    }
};

class InsectSwarmTest : public TestCaseScript
{
public:
	InsectSwarmTest() : TestCaseScript("talents druid insect_swarm") { }

	class InsectSwarmTestImpt : public TestCase
	{
	public:
		InsectSwarmTestImpt() : TestCase(STATUS_PASSING) { }

		void Test() override
		{
			TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);
			Creature* creature = SpawnCreature();

			EQUIP_NEW_ITEM(druid, 34182); // Grand Magister's Staff of Torrents - 266 SP
			druid->DisableRegeneration(true);

			int32 staffSP = 266;
			TEST_ASSERT(druid->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_ALL) == staffSP);

			// Mana cost
			uint32 const expectedInsectSwarmMana = 175;
			TEST_POWER_COST(druid, ClassSpells::Druid::INSECT_SWARM_RNK_6, POWER_MANA, expectedInsectSwarmMana);

			// Spell coefficient
			float const insectSwarmSpellCoeff = 12.0f / 15.0f;
			uint32 const insectSwarmBonusSP = insectSwarmSpellCoeff * staffSP;

			// Damage
            uint32 const tickAmount = 6;
            uint32 const bonusSPPerTick = insectSwarmBonusSP / tickAmount;
			uint32 const expectedInsectSwarmTotalDmg = tickAmount * (ClassSpellsDamage::Druid::INSECT_SWARM_RNK_6_TICK + bonusSPPerTick);
			TEST_DOT_DAMAGE(druid, creature, ClassSpells::Druid::INSECT_SWARM_RNK_6, expectedInsectSwarmTotalDmg, false);

			//-2% hit chance
            TestPlayer* rogue = SpawnPlayer(CLASS_ROGUE, RACE_HUMAN);
            float const baseMissChance70v70 = 5.f;
            float const insectSwarmMissFactor = 2.f;
            float const expectedMissChance = baseMissChance70v70 + insectSwarmMissFactor;
            TEST_MELEE_HIT_CHANCE(rogue, druid, BASE_ATTACK, expectedMissChance, MELEE_HIT_MISS, [](Unit* attacker, Unit* victim) {
                attacker->AddAura(ClassSpells::Druid::INSECT_SWARM_RNK_6, attacker);
            });
		}
	};

	std::unique_ptr<TestCase> GetTest() const override
	{
		return std::make_unique<InsectSwarmTestImpt>();
	}
};

class NaturesReachTest : public TestCaseScript
{
public:

	NaturesReachTest() : TestCaseScript("talents druid natures_reach") { }

	class NaturesReachTestImpt : public TestCase
	{
	public:
		NaturesReachTestImpt() : TestCase(STATUS_PASSING) { }

		void Test() override
		{
			TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);

			Position range48m(_location);
            range48m.MoveInFront(_location, 48.0f);
			Creature* creature48m = SpawnCreatureWithPosition(range48m, 26); // Beast for Soothe Animal - 48m

			Position range36m(_location);
            range36m.MoveInFront(_location, 36.0f);
			Creature* creature36m = SpawnCreatureWithPosition(range36m, 20); // Draconid for Hibernate - 36m
			TestPlayer* ally36m = SpawnPlayer(CLASS_DRUID, RACE_TAUREN, 70, range36m); // Player for Innervate - 36m

			Position range24m(_location);
            range24m.MoveInFront(_location, 24.0f);
			TestPlayer* enemy24m = SpawnPlayer(CLASS_DRUID, RACE_NIGHTELF, 70, range24m); // 24m

			LearnTalent(druid, ClassSpells::Druid::BEAR_FORM_RNK_1);
			LearnTalent(druid, Talents::Druid::FAERIE_FIRE_FERAL_RNK_1);
			LearnTalent(druid, Talents::Druid::NATURES_REACH_RNK_2);

            TEST_CAST(druid, enemy24m, ClassSpells::Druid::CYCLONE_RNK_1, SPELL_CAST_OK, TriggerCastFlags(TRIGGERED_FULL_MASK &~TRIGGERED_IGNORE_RANGE));
            TEST_CAST(druid, creature36m, ClassSpells::Druid::ENTANGLING_ROOTS_RNK_7, SPELL_CAST_OK, TriggerCastFlags(TRIGGERED_FULL_MASK &~TRIGGERED_IGNORE_RANGE));
            TEST_CAST(druid, creature36m, ClassSpells::Druid::FAERIE_FIRE_RNK_5, SPELL_CAST_OK, TriggerCastFlags(TRIGGERED_FULL_MASK &~TRIGGERED_IGNORE_RANGE));
            TEST_CAST(druid, creature36m, ClassSpells::Druid::HURRICANE_RNK_4, SPELL_CAST_OK, TriggerCastFlags(TRIGGERED_FULL_MASK &~TRIGGERED_IGNORE_RANGE));
            TEST_CAST(druid, ally36m, ClassSpells::Druid::INNERVATE_RNK_1, SPELL_CAST_OK, TriggerCastFlags(TRIGGERED_FULL_MASK &~TRIGGERED_IGNORE_RANGE));
            TEST_CAST(druid, creature36m, ClassSpells::Druid::MOONFIRE_RNK_12, SPELL_CAST_OK, TriggerCastFlags(TRIGGERED_FULL_MASK &~TRIGGERED_IGNORE_RANGE));
            TEST_CAST(druid, creature48m, ClassSpells::Druid::SOOTHE_ANIMAL_RNK_4, SPELL_CAST_OK, TriggerCastFlags(TRIGGERED_FULL_MASK &~TRIGGERED_IGNORE_RANGE));
            TEST_CAST(druid, creature36m, ClassSpells::Druid::STARFIRE_RNK_8, SPELL_CAST_OK, TriggerCastFlags(TRIGGERED_FULL_MASK &~TRIGGERED_IGNORE_RANGE));
            TEST_CAST(druid, creature36m, ClassSpells::Druid::WRATH_RNK_10, SPELL_CAST_OK, TriggerCastFlags(TRIGGERED_FULL_MASK &~TRIGGERED_IGNORE_RANGE));
            TEST_CAST(druid, druid, ClassSpells::Druid::BEAR_FORM_RNK_1, SPELL_CAST_OK, TRIGGERED_FULL_MASK);
            TEST_CAST(druid, creature36m, ClassSpells::Druid::FAERIE_FIRE_FERAL_RNK_5, SPELL_CAST_OK, TriggerCastFlags(TRIGGERED_FULL_MASK &~TRIGGERED_IGNORE_RANGE));
		}
	};

	std::unique_ptr<TestCase> GetTest() const override
	{
		return std::make_unique<NaturesReachTestImpt>();
	}
};

class VengeanceTest : public TestCaseScript
{
public:

    VengeanceTest() : TestCaseScript("talents druid vengeance") { }

    class VengeanceTestImpt : public TestCase
    {
    public:
        VengeanceTestImpt() : TestCase(STATUS_WIP) { }

        void Test() override
        {
            TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);
            Creature* dummy = SpawnCreature();

            LearnTalent(druid, Talents::Druid::VENGEANCE_RNK_5);

            // Starfire
            uint32 const expectedStarfireMinCrit = ClassSpellsDamage::Druid::STARFIRE_RNK_8_MIN * 2;
            uint32 const expectedStarfireMaxCrit = ClassSpellsDamage::Druid::STARFIRE_RNK_8_MAX * 2;
            TEST_DIRECT_SPELL_DAMAGE(druid, dummy, ClassSpells::Druid::STARFIRE_RNK_8, expectedStarfireMinCrit, expectedStarfireMaxCrit, true);

            // Moonfire
            uint32 const expectedMoonfireMinCrit = ClassSpellsDamage::Druid::MOONFIRE_RNK_12_MIN * 2;
            uint32 const expectedMoonfireMaxCrit = ClassSpellsDamage::Druid::MOONFIRE_RNK_12_MAX * 2;
            TEST_DIRECT_SPELL_DAMAGE(druid, dummy, ClassSpells::Druid::MOONFIRE_RNK_12, expectedMoonfireMinCrit, expectedMoonfireMaxCrit, true);

            // Wrath
            uint32 const expectedWrathMinCrit = ClassSpellsDamage::Druid::WRATH_RNK_10_MIN * 2;
            uint32 const expectedWrathMaxCrit = ClassSpellsDamage::Druid::WRATH_RNK_10_MAX * 2;
            TEST_DIRECT_SPELL_DAMAGE(druid, dummy, ClassSpells::Druid::WRATH_RNK_10, expectedWrathMinCrit, expectedWrathMaxCrit, true);
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<VengeanceTestImpt>();
    }
};

class CelestialFocusTest : public TestCaseScript
{
public:

    CelestialFocusTest() : TestCaseScript("talents druid celestial_focus") { }

    class CelestialFocusTestImpt : public TestCase
    {
    public:
        CelestialFocusTestImpt() : TestCase(STATUS_WIP) { }

        void Test() override
        {
            TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_HUMAN);

            LearnTalent(druid, Talents::Druid::CELESTIAL_FOCUS_RNK_3);
            float const talentResistPushbackFactor = 70.f;
            float const talentStunChance = 35.f;

            // Proc
            druid->AddAura(Talents::Druid::CELESTIAL_FOCUS_PROC, druid);
            TEST_AURA_MAX_DURATION(druid, Talents::Druid::CELESTIAL_FOCUS_PROC, Seconds(3));
            TEST_ASSERT(druid->HasAuraType(SPELL_AURA_MOD_STUN));
            TEST_SPELL_PROC_CHANCE(druid, warrior, ClassSpells::Druid::STARFIRE_RNK_8, Talents::Druid::CELESTIAL_FOCUS_PROC, false, talentStunChance, SPELL_MISS_NONE, false);

            // Resist pushback
            TEST_PUSHBACK_RESIST_CHANCE(druid, warrior, ClassSpells::Druid::WRATH_RNK_10, talentResistPushbackFactor);
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<CelestialFocusTestImpt>();
    }
};

class LunarGuidanceTest : public TestCaseScript
{
public:
	LunarGuidanceTest() : TestCaseScript("talents druid lunar_guidance") { }

	class LunarGuidanceTestImpt : public TestCase
	{
	public:
		LunarGuidanceTestImpt() : TestCase(STATUS_PASSING) { } //Need whole file review

		void Test() override
		{
			TestPlayer* druid = SpawnRandomPlayer(CLASS_DRUID);

            float const talentFactor = 0.25f;

			float const intellect = druid->GetStat(STAT_INTELLECT);
			int32 const expectedBH = druid->GetInt32Value(PLAYER_FIELD_MOD_HEALING_DONE_POS) + floor(intellect * talentFactor);
			int32 const expectedSP = druid->GetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_ARCANE) + floor(intellect * talentFactor);

            LearnTalent(druid, Talents::Druid::LUNAR_GUIDANCE_RNK_3);
			TEST_ASSERT(druid->GetInt32Value(PLAYER_FIELD_MOD_HEALING_DONE_POS) == expectedBH);
			TEST_ASSERT(druid->GetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_ARCANE) == expectedSP);
		}
	};

	std::unique_ptr<TestCase> GetTest() const override
	{
		return std::make_unique<LunarGuidanceTestImpt>();
	}
};

class NaturesGraceTest : public TestCaseScript
{
public:

    NaturesGraceTest() : TestCaseScript("talents druid natures_grace") { }

    class NaturesGraceTestImpt : public TestCase
    {
    public:
        /*
        Infos:
            - Patch 2.1: This talent is now triggered by Swiftmend and Lifebloom, and is triggered by and affects Cyclone.
        */
        NaturesGraceTestImpt() : TestCase(STATUS_WIP) { }

        void AssertSpellProcsTalent(TestPlayer* druid, Unit* victim, uint32 spellId, uint32 wait = 0)
        {
            TriggerCastFlags flags = TriggerCastFlags(TRIGGERED_CAST_DIRECTLY | TRIGGERED_IGNORE_POWER_AND_REAGENT_COST | TRIGGERED_IGNORE_GCD);
            TEST_CAST(druid, victim, spellId, SPELL_CAST_OK, flags);
            if (wait > 0)
                Wait(wait);
            TEST_AURA_MAX_DURATION(druid, Talents::Druid::NATURES_GRACE_PROC, Seconds(15));
            druid->RemoveAurasDueToSpell(Talents::Druid::NATURES_GRACE_PROC);
        }

        void Test() override
        {
            TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);
            Creature* dummy = SpawnCreature();

            EnableCriticals(druid, true);

            LearnTalent(druid, Talents::Druid::NATURES_GRACE_RNK_1);

            // Procs on...
            AssertSpellProcsTalent(druid, druid, ClassSpells::Druid::HEALING_TOUCH_RNK_13);
            AssertSpellProcsTalent(druid, druid, ClassSpells::Druid::REGROWTH_RNK_10);
            AssertSpellProcsTalent(druid, dummy, ClassSpells::Druid::MOONFIRE_RNK_12);
            AssertSpellProcsTalent(druid, dummy, ClassSpells::Druid::WRATH_RNK_10);
            AssertSpellProcsTalent(druid, dummy, ClassSpells::Druid::STARFIRE_RNK_8);
            druid->AddAura(ClassSpells::Druid::REJUVENATION_RNK_13, druid);
            AssertSpellProcsTalent(druid, druid, ClassSpells::Druid::SWIFTMEND_RNK_1);
            AssertSpellProcsTalent(druid, druid, ClassSpells::Druid::LIFEBLOOM_RNK_1, 7500);

            // Reduces cast time of...
            druid->AddAura(Talents::Druid::NATURES_GRACE_PROC, druid);
            TEST_CAST_TIME(druid, ClassSpells::Druid::CYCLONE_RNK_1, 1000);
            TEST_CAST_TIME(druid, ClassSpells::Druid::ENTANGLING_ROOTS_RNK_7, 1000);
            TEST_CAST_TIME(druid, ClassSpells::Druid::HEALING_TOUCH_RNK_13, 3000);
            TEST_CAST_TIME(druid, ClassSpells::Druid::HIBERNATE_RNK_3, 1000);
            TEST_CAST_TIME(druid, ClassSpells::Druid::HURRICANE_RNK_4, 9500);
            TEST_CAST_TIME(druid, ClassSpells::Druid::REGROWTH_RNK_10, 1500);
            TEST_CAST_TIME(druid, ClassSpells::Druid::SOOTHE_ANIMAL_RNK_4, 1000);
            TEST_CAST_TIME(druid, ClassSpells::Druid::STARFIRE_RNK_8, 3000);
            TEST_CAST_TIME(druid, ClassSpells::Druid::TRANQUILITY_RNK_5, 7500);
            TEST_CAST_TIME(druid, ClassSpells::Druid::WRATH_RNK_10, 1500);
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<NaturesGraceTestImpt>();
    }
};

class MoonglowTest : public TestCaseScript
{
public:
	MoonglowTest() : TestCaseScript("talents druid moonglow") { }

	class MoonglowTestImpt : public TestCase
	{
	public:
		MoonglowTestImpt() : TestCase(STATUS_PASSING) { }

		void Test() override
		{
			TestPlayer* druid = SpawnRandomPlayer(CLASS_DRUID);

			LearnTalent(druid, Talents::Druid::MOONGLOW_RNK_3);
			float const moonglowFactor = 1 - 3 * 0.03f;

			uint32 const expectedMoonfireMana = 495 * moonglowFactor;
			uint32 const expectedStarfireMana = 370 * moonglowFactor;
			uint32 const expectedWrathMana = 255 * moonglowFactor;
			uint32 const expectedHealingTouchMana = 935 * moonglowFactor;
			uint32 const expectedRegrowthMana = 675 * moonglowFactor;
			uint32 const expectedRejuvenationMana = 415 * moonglowFactor;

            TEST_POWER_COST(druid, ClassSpells::Druid::MOONFIRE_RNK_12, POWER_MANA, expectedMoonfireMana);
            TEST_POWER_COST(druid, ClassSpells::Druid::STARFIRE_RNK_8, POWER_MANA, expectedStarfireMana);
            TEST_POWER_COST(druid, ClassSpells::Druid::WRATH_RNK_10, POWER_MANA, expectedWrathMana);
            TEST_POWER_COST(druid, ClassSpells::Druid::HEALING_TOUCH_RNK_13, POWER_MANA, expectedHealingTouchMana);
            TEST_POWER_COST(druid, ClassSpells::Druid::REGROWTH_RNK_10, POWER_MANA, expectedRegrowthMana);
            TEST_POWER_COST(druid, ClassSpells::Druid::REJUVENATION_RNK_13, POWER_MANA, expectedRejuvenationMana);
		}
	};

	std::unique_ptr<TestCase> GetTest() const override
	{
		return std::make_unique<MoonglowTestImpt>();
	}
};

class MoonfuryTest : public TestCaseScript
{
public:

	MoonfuryTest() : TestCaseScript("talents druid moonfury") { }

	class MoonfuryTestImpt : public TestCase
	{
	public:
		MoonfuryTestImpt() : TestCase(STATUS_PASSING) { }

		void Test() override
		{
			TestPlayer* druid = SpawnRandomPlayer(CLASS_DRUID);
			Creature* dummy = SpawnCreature();

			LearnTalent(druid, Talents::Druid::MOONFURY_RNK_5);
            float const talentDamageFactor = 1.1f;

			// Starfire rank 8
            uint32 const expectedStarfireMinDmg = ClassSpellsDamage::Druid::STARFIRE_RNK_8_MIN * talentDamageFactor;
            uint32 const expectedStarfireMaxDmg = ClassSpellsDamage::Druid::STARFIRE_RNK_8_MAX * talentDamageFactor;
			TEST_DIRECT_SPELL_DAMAGE(druid, dummy, ClassSpells::Druid::STARFIRE_RNK_8, expectedStarfireMinDmg, expectedStarfireMaxDmg, false);

            // Moonfire rank 12
            uint32 const expectedMoonfireMinDmg = ClassSpellsDamage::Druid::MOONFIRE_RNK_12_MIN * talentDamageFactor;
            uint32 const expectedMoonfireMaxDmg = ClassSpellsDamage::Druid::MOONFIRE_RNK_12_MAX * talentDamageFactor;
            TEST_DIRECT_SPELL_DAMAGE(druid, dummy, ClassSpells::Druid::MOONFIRE_RNK_12, expectedMoonfireMinDmg, expectedMoonfireMaxDmg, false);
            uint32 const moonfireTickAmount = 4;
            uint32 const expectedMoonfireDotDmg = moonfireTickAmount * floor(ClassSpellsDamage::Druid::MOONFIRE_RNK_12_TICK * talentDamageFactor);
            TEST_DOT_DAMAGE(druid, dummy, ClassSpells::Druid::MOONFIRE_RNK_12, expectedMoonfireDotDmg, false);

            // Wrath rank 10
            uint32 const expectedWrathMinDmg = ClassSpellsDamage::Druid::WRATH_RNK_10_MIN * talentDamageFactor;
            uint32 const expectedWrathMaxDmg = ClassSpellsDamage::Druid::WRATH_RNK_10_MAX * talentDamageFactor;
			TEST_DIRECT_SPELL_DAMAGE(druid, dummy, ClassSpells::Druid::WRATH_RNK_10, expectedWrathMinDmg, expectedWrathMaxDmg, false);
		}
	};

	std::unique_ptr<TestCase> GetTest() const override
	{
		return std::make_unique<MoonfuryTestImpt>();
	}
};

class DreamstateTest : public TestCaseScript
{
public:
	DreamstateTest() : TestCaseScript("talents druid dreamstate") { }

	class DreamstateTestImpt : public TestCase
	{
	public:
		DreamstateTestImpt() : TestCase(STATUS_PASSING) { }

		void Test() override
		{
			TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);

            float const talentFactor = 0.1f;

            float const intellect = druid->GetStat(STAT_INTELLECT);
            float const regenGain = intellect * talentFactor / 5.f;

            float const expectedFsr = druid->GetFloatValue(PLAYER_FIELD_MOD_MANA_REGEN) + regenGain;
            float const expectedOofsr = druid->GetFloatValue(PLAYER_FIELD_MOD_MANA_REGEN_INTERRUPT) + regenGain;

            LearnTalent(druid, Talents::Druid::DREAMSTATE_RNK_3);
            ASSERT_INFO("Expected: %f, actual: %f", expectedFsr, druid->GetFloatValue(PLAYER_FIELD_MOD_MANA_REGEN));
            TEST_ASSERT(Between<float>(druid->GetFloatValue(PLAYER_FIELD_MOD_MANA_REGEN), expectedFsr - 0.1f, expectedFsr + 0.1f));
            TEST_ASSERT(Between<float>(druid->GetFloatValue(PLAYER_FIELD_MOD_MANA_REGEN_INTERRUPT), expectedOofsr - 0.1f, expectedOofsr + 0.1f));
		}
	};

	std::unique_ptr<TestCase> GetTest() const override
	{
		return std::make_unique<DreamstateTestImpt>();
	}
};

class WrathOfCenariusTest : public TestCaseScript
{
public:

	WrathOfCenariusTest() : TestCaseScript("talents druid wrath_of_cenarius") { }

	class WrathOfCenariusTestImpt : public TestCase
	{
	public:
        WrathOfCenariusTestImpt() : TestCase(STATUS_PASSING) { }

		void Test() override
		{
			TestPlayer* druid = SpawnRandomPlayer(CLASS_DRUID);
			Creature* dummy = SpawnCreature();

			LearnTalent(druid, Talents::Druid::WRATH_OF_CENARIUS_RNK_5);
            float const starfireFactor = 5 * 0.04f;
            float const wrathFactor = 5 * 0.02f;

			EQUIP_NEW_ITEM(druid, 34182); // Grand Magister's Staff of Torrents - 266 SP

			uint32 const arcaneSP = druid->GetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_ARCANE);
			uint32 const natureSP = druid->GetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_NATURE);

			uint32 const expectedStarfireMinDamage = ClassSpellsDamage::Druid::STARFIRE_RNK_8_MIN + arcaneSP * (ClassSpellsCoeff::Druid::STARFIRE + starfireFactor);
			uint32 const expectedStarfireMaxDamage = ClassSpellsDamage::Druid::STARFIRE_RNK_8_MAX + arcaneSP * (ClassSpellsCoeff::Druid::STARFIRE + starfireFactor);
			TEST_DIRECT_SPELL_DAMAGE(druid, dummy, ClassSpells::Druid::STARFIRE_RNK_8, expectedStarfireMinDamage, expectedStarfireMaxDamage, false);

			uint32 const expectedWrathMinDamage = ClassSpellsDamage::Druid::WRATH_RNK_10_MIN + natureSP * (ClassSpellsCoeff::Druid::WRATH + wrathFactor);
			uint32 const expectedWrathMaxDamage = ClassSpellsDamage::Druid::WRATH_RNK_10_MAX + natureSP * (ClassSpellsCoeff::Druid::WRATH + wrathFactor);
			TEST_DIRECT_SPELL_DAMAGE(druid, dummy, ClassSpells::Druid::WRATH_RNK_10, expectedWrathMinDamage, expectedWrathMaxDamage, false);
		}
	};

	std::unique_ptr<TestCase> GetTest() const override
	{
		return std::make_unique<WrathOfCenariusTestImpt>();
	}
};

class FerocityTest : public TestCaseScript
{
public:

	FerocityTest() : TestCaseScript("talents druid ferocity") { }

	class FerocityTestImpt : public TestCase
	{
	public:
		FerocityTestImpt() : TestCase(STATUS_WIP) { }

		void Test() override
		{
			TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);

			LearnTalent(druid, Talents::Druid::FEROCITY_RNK_5);
			uint32 const talentPowerFactor = 5;

			uint32 const expectedMaulRage = (15 - talentPowerFactor) * 10;
			uint32 const expectedSwipeRage = (20 - talentPowerFactor) * 10;
			uint32 const expectedMangleRage = (20 - talentPowerFactor) * 10;

			uint32 const expectedClawEnergy	= 45 - talentPowerFactor;
			uint32 const expectedRakeEnergy	= 40 - talentPowerFactor;
			uint32 const expectedMangleEnergy = 45 - talentPowerFactor;

			druid->AddAura(ClassSpells::Druid::DIRE_BEAR_FORM_RNK_2, druid);
            TEST_POWER_COST(druid, ClassSpells::Druid::MAUL_RNK_8, POWER_RAGE, expectedMaulRage);
            TEST_POWER_COST(druid, ClassSpells::Druid::SWIPE_RNK_6, POWER_RAGE, expectedSwipeRage);
            TEST_POWER_COST(druid, ClassSpells::Druid::MANGLE_BEAR_RNK_3, POWER_RAGE, expectedMangleRage);
			druid->RemoveAurasDueToSpell(ClassSpells::Druid::BEAR_FORM_RNK_1);

            druid->AddAura(ClassSpells::Druid::CAT_FORM_RNK_1, druid);
            TEST_POWER_COST(druid, ClassSpells::Druid::CLAW_RNK_6, POWER_ENERGY, expectedClawEnergy);
            TEST_POWER_COST(druid, ClassSpells::Druid::RAKE_RNK_5, POWER_ENERGY, expectedRakeEnergy);
            TEST_POWER_COST(druid, ClassSpells::Druid::MANGLE_CAT_RNK_3, POWER_ENERGY, expectedMangleEnergy);
		}
	};

	std::unique_ptr<TestCase> GetTest() const override
	{
		return std::make_unique<FerocityTestImpt>();
	}
};

class BrutalImpactTest : public TestCaseScript
{
public:

	BrutalImpactTest() : TestCaseScript("talents druid brutal_impact") { }

	class BrutalImpactTestImpt : public TestCase
	{
	public:
		BrutalImpactTestImpt() : TestCase(STATUS_PASSING) { }

		void Test() override
		{
			TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);
			TestPlayer* enemy = SpawnPlayer(CLASS_DRUID, RACE_NIGHTELF);

			LearnTalent(druid, Talents::Druid::BRUTAL_IMPACT_RNK_2);

			// Bash
            FORCE_CAST(druid, enemy, ClassSpells::Druid::BASH_RNK_3, SPELL_MISS_NONE, TRIGGERED_FULL_MASK);
            TEST_AURA_MAX_DURATION(enemy, ClassSpells::Druid::BASH_RNK_3, Seconds(5));
            enemy->RemoveAurasDueToSpell(ClassSpells::Druid::BASH_RNK_3);

            // Pounce
            enemy->ClearDiminishings();
            FORCE_CAST(druid, enemy, ClassSpells::Druid::POUNCE_RNK_4, SPELL_MISS_NONE, TRIGGERED_FULL_MASK);
            TEST_AURA_MAX_DURATION(enemy, ClassSpells::Druid::POUNCE_RNK_4, Seconds(4));
		}
	};

	std::unique_ptr<TestCase> GetTest() const override
	{
		return std::make_unique<BrutalImpactTestImpt>();
	}
};

class ThickHideTest : public TestCaseScript
{
public:
	ThickHideTest() : TestCaseScript("talents druid thick_hide") { }

	class ThickHideTestImpt : public TestCase
	{
	public:
        /*
        Bugs:
            - Should take the 140 hidden armor into account (ArmorDamageModifier)
        */
		ThickHideTestImpt() : TestCase(STATUS_KNOWN_BUG) { }

		void Test() override
		{
			TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);

			RemoveAllEquipedItems(druid);

			uint32 const startingArmorViaAgi = druid->GetStat(STAT_AGILITY) * 2;
			uint32 const startingArmor = druid->GetArmor() - startingArmorViaAgi;

			EQUIP_NEW_ITEM(druid, 34392); // Demontooth Shoulderpads - 514 Armor (140 hidden) - 38 Agi

			uint32 const itemArmor = 514 + 140;
			uint32 const itemAgi = 38 * 2;
			uint32 currentArmor = startingArmor + itemArmor +startingArmorViaAgi + itemAgi;
			ASSERT_INFO("Druid armor: %u armor, expected: %u", druid->GetArmor(), currentArmor);
			TEST_ASSERT(druid->GetArmor() == currentArmor);

			LearnTalent(druid, Talents::Druid::THICK_HIDE_RNK_3);
            float const talentFactor = 1.1f;

			uint32 const expectedArmor = (startingArmor + itemArmor) * talentFactor + startingArmorViaAgi + itemAgi;
			ASSERT_INFO("Druid armor: %u armor, expected: %u", druid->GetArmor(), expectedArmor);
			TEST_ASSERT(druid->GetArmor() == expectedArmor);

            druid->AddAura(33079, druid); // Scroll of Protection V - 300 armor
			ASSERT_INFO("Druid armor: %u armor, expected: %u", druid->GetArmor(), expectedArmor);
			TEST_ASSERT(druid->GetArmor() == expectedArmor);
		}
	};

	std::unique_ptr<TestCase> GetTest() const override
	{
		return std::make_unique<ThickHideTestImpt>();
	}
};

class ShreddingAttacksTest : public TestCaseScript
{
public:

	ShreddingAttacksTest() : TestCaseScript("talents druid shredding_attacks") { }

	class ShreddingAttacksTestImpt : public TestCase
	{
	public:
		ShreddingAttacksTestImpt() : TestCase(STATUS_PASSING) { }

		void Test() override
		{
			TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);

			LearnTalent(druid, Talents::Druid::SHREDDING_ATTACKS_RNK_2);

			uint32 const expectedLacerateRage = (15 - 2) * 10;
			uint32 const expectedShredEnergy = 60 - 18;

            TEST_POWER_COST(druid, ClassSpells::Druid::LACERATE_RNK_1, POWER_RAGE, expectedLacerateRage);
            TEST_POWER_COST(druid, ClassSpells::Druid::SHRED_RNK_7, POWER_ENERGY, expectedShredEnergy);
		}
	};

	std::unique_ptr<TestCase> GetTest() const override
	{
		return std::make_unique<ShreddingAttacksTestImpt>();
	}
};

class PredatoryStrikesTest : public TestCaseScript
{
public:

	PredatoryStrikesTest() : TestCaseScript("talents druid predatory_strikes") { }

	class PredatoryStrikesTestImpt : public TestCase
	{
	public:
		PredatoryStrikesTestImpt() : TestCase(STATUS_WIP) { } //Need whole file review

		void TestAP(TestPlayer* druid, uint32 spellId, uint32 expectedAP)
		{
            druid->AddAura(spellId, druid);
            ASSERT_INFO("In shapeshift %u, Druid has %f AP but %u was expected.", spellId, druid->GetTotalAttackPowerValue(BASE_ATTACK), expectedAP);
			TEST_ASSERT(druid->GetTotalAttackPowerValue(BASE_ATTACK) == expectedAP);
            druid->RemoveAurasDueToSpell(spellId);
		}

		void Test() override
		{
			uint32 const level = 70;
			TestPlayer* player = SpawnPlayer(CLASS_DRUID, RACE_TAUREN, level);

			LearnTalent(player, Talents::Druid::PREDATORY_STRIKES_RNK_3);
			uint32 const predatoryStrikesBonus = level * 1.5f;

			uint32 const expectedMoonkinAP	= GetMoonkinBaseAP(player, level)  + predatoryStrikesBonus;
			uint32 const expectedCatAP		= GetCatBaseAP(player, level)	   + predatoryStrikesBonus;
			uint32 const expectedBearAP		= GetBearBaseAP(player, level)	   + predatoryStrikesBonus;
			uint32 const expectedDireBearAP = GetDireBearBaseAP(player, level) + predatoryStrikesBonus;

            TestAP(player, ClassSpells::Druid::MOONKIN_FORM_RNK_1, expectedMoonkinAP);
			TestAP(player, ClassSpells::Druid::CAT_FORM_RNK_1, expectedCatAP);
			TestAP(player, ClassSpells::Druid::BEAR_FORM_RNK_1, expectedBearAP);
			TestAP(player, ClassSpells::Druid::DIRE_BEAR_FORM_RNK_2, expectedDireBearAP);
		}
	};

	std::unique_ptr<TestCase> GetTest() const override
	{
		return std::make_unique<PredatoryStrikesTestImpt>();
	}
};

class SavageFuryTest : public TestCaseScript
{
public:

	SavageFuryTest() : TestCaseScript("talents druid savage_fury") { }

	class SavageFuryTestImpt : public TestCase
	{
	public:
		SavageFuryTestImpt() : TestCase(STATUS_WIP) { }

		void Test() override
		{
			TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);
			Creature* dummy = SpawnCreature();

            druid->AddAura(ClassSpells::Druid::CAT_FORM_RNK_1, druid);
			LearnTalent(druid, Talents::Druid::SAVAGE_FURY_RNK_2);

			float const AP = druid->GetTotalAttackPowerValue(BASE_ATTACK);
			float const armorFactor = GetArmorFactor(druid, dummy);
			float const minDamage = ClassSpellsDamage::Druid::GetCatMinDmg() + AP / 14;
			float const maxDamage = ClassSpellsDamage::Druid::GetCatMaxDmg() + AP / 14;
			float const savageFuryFactor = 1.2f;

			// Mangle (Cat)
            uint32 const mangleDmgBonus = 264;
            float  const mangleDmgCoeff = 1.6f;
			uint32 const expectedMangleMinDmg = floor((minDamage * mangleDmgCoeff + mangleDmgBonus) * savageFuryFactor) * armorFactor;
            uint32 const expectedMangleMaxDmg = floor((maxDamage * mangleDmgCoeff + mangleDmgBonus) * savageFuryFactor) * armorFactor;
			TEST_DIRECT_SPELL_DAMAGE(druid, dummy, ClassSpells::Druid::MANGLE_CAT_RNK_3, expectedMangleMinDmg, expectedMangleMaxDmg, false);
            dummy->RemoveAurasDueToSpell(ClassSpells::Druid::MANGLE_CAT_RNK_3);

			// Rake -- Coeffs are from http://wowwiki.wikia.com/wiki/Rake?oldid=1524037
            // Initial
            uint32 const expectedRakeInitialBleed = floor(AP / 100 + ClassSpellsDamage::Druid::RAKE_RNK_5) * savageFuryFactor;
            //TEST_DIRECT_SPELL_DAMAGE(druid, dummy, ClassSpells::Druid::RAKE_RNK_5, expectedRakeInitialBleed, expectedRakeInitialBleed, false);
            // Bleed
            float const rakeAPCoeff = 0.06f;
            float const rakeTickCount = 3.0f;
            uint32 const rakeBleedTotal = (AP * rakeAPCoeff + ClassSpellsDamage::Druid::RAKE_RNK_5_BLEED) * savageFuryFactor;
            uint32 const rakeBleedTick = floor(rakeBleedTotal / rakeTickCount);
            uint32 const expectedRakeBleedDamage = 3 * rakeBleedTick;
            //TEST_DOT_DAMAGE(druid, dummy, ClassSpells::Druid::RAKE_RNK_5, expectedRakeBleedDamage, false);

			// Claw
			uint32 const expectedClawMin = floor((ClassSpellsDamage::Druid::CLAW_RNK_6 + minDamage) * savageFuryFactor) * armorFactor;
            uint32 const expectedClawMax = floor((ClassSpellsDamage::Druid::CLAW_RNK_6 + maxDamage) * savageFuryFactor) * armorFactor;
			TEST_DIRECT_SPELL_DAMAGE(druid, dummy, ClassSpells::Druid::CLAW_RNK_6, expectedClawMin, expectedClawMax, false);
		}
	};

	std::unique_ptr<TestCase> GetTest() const override
	{
		return std::make_unique<SavageFuryTestImpt>();
	}
};

class FaerieFireFeralTest : public TestCaseScript
{
public:

	FaerieFireFeralTest() : TestCaseScript("talents druid faerie_fire_feral") { }

	class FaerieFireFeralTestImpt : public TestCase
	{
	public:
		FaerieFireFeralTestImpt() : TestCase(STATUS_PASSING) { }

		void Test() override
		{
			TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);

			Position spawnPosition(_location);
			spawnPosition.MoveInFront(_location, 3.0f);
			TestPlayer* rogue = SpawnPlayer(CLASS_ROGUE, RACE_HUMAN, 70, spawnPosition);
			TestPlayer* mage = SpawnPlayer(CLASS_MAGE, RACE_HUMAN, 70, spawnPosition);

			EQUIP_NEW_ITEM(rogue, 34211); // S4 Chest
			int32 const expectedRogueArmor = rogue->GetArmor() - 610;

			// Only cast in Bear or Cat form
            TEST_CAST(druid, druid, ClassSpells::Druid::FAERIE_FIRE_FERAL_RNK_5, SPELL_FAILED_ONLY_SHAPESHIFT);

			// Bear Form
            TEST_CAST(druid, druid, ClassSpells::Druid::BEAR_FORM_RNK_1, SPELL_CAST_OK, TRIGGERED_FULL_MASK);

			// Faerie Fire Feral
            TEST_CAST(druid, rogue, ClassSpells::Druid::FAERIE_FIRE_FERAL_RNK_5);
            TEST_AURA_MAX_DURATION(rogue, ClassSpells::Druid::FAERIE_FIRE_FERAL_RNK_5, Seconds(40));
			TEST_ASSERT(int32(rogue->GetArmor()) == expectedRogueArmor);
            TEST_HAS_COOLDOWN(druid, ClassSpells::Druid::FAERIE_FIRE_FERAL_RNK_5, Seconds(6));

			// Rogue can't stealth
            TEST_CAST(rogue, rogue, ClassSpells::Rogue::STEALTH_RNK_4, SPELL_FAILED_CASTER_AURASTATE);

			// Cat form
            druid->RemoveAurasDueToSpell(ClassSpells::Druid::BEAR_FORM_RNK_1);
            TEST_CAST(druid, druid, ClassSpells::Druid::CAT_FORM_RNK_1, SPELL_CAST_OK, TRIGGERED_FULL_MASK);

			// Mage can't invisible
            druid->GetSpellHistory()->ResetAllCooldowns();
            TEST_CAST(druid, mage, ClassSpells::Druid::FAERIE_FIRE_FERAL_RNK_5);
            TEST_CAST(mage, mage, ClassSpells::Mage::INVISIBILITY_RNK_1, SPELL_FAILED_CASTER_AURASTATE);
		}
	};

	std::unique_ptr<TestCase> GetTest() const override
	{
		return std::make_unique<FaerieFireFeralTestImpt>();
	}
};

class HeartOfTheWildTest : public TestCase
{
    /*
    "Increases your Intellect by 20%. In addition, while in Bear or Dire Bear Form your Stamina is increased by 20% and while in Cat Form your attack power is increased by 10%."
    */
	void Test() override
	{
		TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);
        float const talentIntellectFactor = 1.2f;
        float const talentStaminaFactor = 1.2f;
        float const talentAPFactor = 1.1f;

        SECTION("Improved intellect", [&] {
            float const expectedInt = druid->GetStat(STAT_INTELLECT) * talentIntellectFactor;
            LearnTalent(druid, Talents::Druid::HEART_OF_THE_WILD_RNK_5);
            TEST_ASSERT(Between<float>(druid->GetStat(STAT_INTELLECT), expectedInt - 0.1f, expectedInt + 0.1f));
        });
        druid->RemoveSpell(Talents::Druid::HEART_OF_THE_WILD_RNK_5);

        //Prepare base values for next sections
        TEST_CAST(druid, druid, ClassSpells::Druid::DIRE_BEAR_FORM_RNK_2, SPELL_CAST_OK, TRIGGERED_FULL_MASK);
        float const expectedSta = druid->GetStat(STAT_STAMINA) * talentStaminaFactor;
        TEST_CAST(druid, druid, ClassSpells::Druid::CAT_FORM_RNK_1, SPELL_CAST_OK, TRIGGERED_FULL_MASK);
        float const expectedAP = druid->GetTotalAttackPowerValue(BASE_ATTACK) * talentAPFactor; //recast after learn
        druid->RemoveAurasDueToSpell(ClassSpells::Druid::CAT_FORM_RNK_1); 

        SECTION("Bear stamina", [&] {
            LearnTalent(druid, Talents::Druid::HEART_OF_THE_WILD_RNK_5);
            TEST_CAST(druid, druid, ClassSpells::Druid::DIRE_BEAR_FORM_RNK_2, SPELL_CAST_OK, TRIGGERED_FULL_MASK); 
            ASSERT_INFO("Druid has %f Stamina but %f was expected.", druid->GetStat(STAT_STAMINA), expectedSta);
            TEST_ASSERT(Between<float>(druid->GetStat(STAT_STAMINA), expectedSta - 1.f, expectedSta + 1.f));
        });
        druid->RemoveSpell(Talents::Druid::HEART_OF_THE_WILD_RNK_5);
            
        SECTION("Cat AP", [&] {
            LearnTalent(druid, Talents::Druid::HEART_OF_THE_WILD_RNK_5);
            TEST_CAST(druid, druid, ClassSpells::Druid::CAT_FORM_RNK_1, SPELL_CAST_OK, TRIGGERED_FULL_MASK);
            ASSERT_INFO("Druid has %f AP, %f expected.", druid->GetTotalAttackPowerValue(BASE_ATTACK), expectedAP);
            TEST_ASSERT(Between<float>(druid->GetTotalAttackPowerValue(BASE_ATTACK), expectedAP - 1.f, expectedAP + 1.f));
        });
        druid->RemoveSpell(Talents::Druid::HEART_OF_THE_WILD_RNK_5);
	}
};

class SurvivalOfTheFittestTest : public TestCaseScript
{
public:
	SurvivalOfTheFittestTest() : TestCaseScript("talents druid survival_of_the_fittest") { }

	class SurvivalOfTheFittestTestImpt : public TestCase
	{
	public:
		SurvivalOfTheFittestTestImpt() : TestCase(STATUS_PASSING) { }

		void TestStat(TestPlayer* player, Stats stat, uint32 expectedStat)
		{
			ASSERT_INFO("Druid stat %u is: %f, expected: %u", stat, player->GetStat(stat), expectedStat);
			TEST_ASSERT(player->GetStat(stat) == expectedStat);
		}

		void Test() override
		{
			TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);
            
            float const talentStatFactor = 1.03f;

			uint32 const expectedAgi = druid->GetStat(STAT_AGILITY) * talentStatFactor;
			uint32 const expectedInt = druid->GetStat(STAT_INTELLECT) * talentStatFactor;
			uint32 const expectedSpi = druid->GetStat(STAT_SPIRIT) * talentStatFactor;
			uint32 const expectedSta = druid->GetStat(STAT_STAMINA) * talentStatFactor;
			uint32 const expectedStr = druid->GetStat(STAT_STRENGTH) * talentStatFactor;

			LearnTalent(druid, Talents::Druid::SURVIVAL_OF_THE_FITTEST_RNK_3);
			TestStat(druid, STAT_STRENGTH, expectedStr);
			TestStat(druid, STAT_AGILITY, expectedAgi);
			TestStat(druid, STAT_STAMINA, expectedSta);
			TestStat(druid, STAT_INTELLECT, expectedInt);
			TestStat(druid, STAT_SPIRIT, expectedSpi);

			// -3% chance to be critically hit by melee
            TestPlayer* rogue = SpawnPlayer(CLASS_ROGUE, RACE_HUMAN);
            rogue->SetFloatValue(PLAYER_CRIT_PERCENTAGE, 50.f); // BASE_ATTACK
            TEST_MELEE_HIT_CHANCE(rogue, druid, BASE_ATTACK, 47.f, MELEE_HIT_CRIT);
		}
	};

	std::unique_ptr<TestCase> GetTest() const override
	{
		return std::make_unique<SurvivalOfTheFittestTestImpt>();
	}
};

class MangleTest : public TestCaseScript
{
public:
	MangleTest() : TestCaseScript("talents druid mangle") { }

	class MangleTestImpt : public TestCase
	{
	public:
		MangleTestImpt() : TestCase(STATUS_WIP) { }

		void TestMangle(TestPlayer* druidWithMangle, TestPlayer* druidTestDamage, Unit* creature, uint32 mangleSpellId)
		{
			// Calcul base
			float const AP = druidTestDamage->GetTotalAttackPowerValue(BASE_ATTACK);
            float const armorFactor = GetArmorFactor(druidTestDamage, creature);
			uint32 const minDamage = ClassSpellsDamage::Druid::GetCatMinDmg() + AP / 14;
			uint32 const maxDamage = ClassSpellsDamage::Druid::GetCatMaxDmg() + AP / 14;
			float const mangleFactor = 1.3;

			// Initialize Mangle (Cat) on dummy
			FORCE_CAST(druidWithMangle, creature, mangleSpellId, SPELL_MISS_NONE, TRIGGERED_IGNORE_GCD);
            TEST_AURA_MAX_DURATION(creature, mangleSpellId, Seconds(12));
			TEST_ASSERT(druidWithMangle->GetComboTarget() == creature->GetGUID());
			TEST_ASSERT(druidWithMangle->GetComboPoints() == 1);

			// Shred
            float const shredFactor = 2.25f;
			uint32 const expectedShredMinDmg = floor(minDamage * shredFactor + ClassSpellsDamage::Druid::SHRED_RNK_7) * armorFactor * mangleFactor;
			uint32 const expectedShredMaxDmg = floor(maxDamage * shredFactor + ClassSpellsDamage::Druid::SHRED_RNK_7) * armorFactor * mangleFactor;
			TEST_DIRECT_SPELL_DAMAGE(druidTestDamage, creature, ClassSpells::Druid::SHRED_RNK_7, expectedShredMinDmg, expectedShredMaxDmg, false);

			// Mangle (Cat)
            FORCE_CAST(druidWithMangle, creature, mangleSpellId, SPELL_MISS_NONE, TRIGGERED_FULL_MASK);
            float const mangleDamageFactor = 1.6f;
            uint32 const mangleDmgBonus = 264;
			float const expectedMangleMinDmg = (minDamage * mangleDamageFactor + mangleDmgBonus) * armorFactor;
			float const expectedMangleMaxDmg = (maxDamage * mangleDamageFactor + mangleDmgBonus) * armorFactor;
			TEST_DIRECT_SPELL_DAMAGE(druidTestDamage, creature, mangleSpellId, expectedMangleMinDmg, expectedMangleMaxDmg, false);

			// Bleed: Rake
			//	Initial damage
            FORCE_CAST(druidWithMangle, creature, mangleSpellId, SPELL_MISS_NONE, TRIGGERED_FULL_MASK);
            TEST_CAST(druidWithMangle, creature, mangleSpellId);
			float const expectedRakeDmg = (AP / 100 + ClassSpellsDamage::Druid::RAKE_RNK_5) * mangleFactor;
			TEST_DIRECT_SPELL_DAMAGE(druidTestDamage, creature, ClassSpells::Druid::RAKE_RNK_5, expectedRakeDmg, expectedRakeDmg, false);

			//	Rake bleed
            FORCE_CAST(druidWithMangle, creature, mangleSpellId, SPELL_MISS_NONE, TRIGGERED_FULL_MASK);
            TEST_CAST(druidWithMangle, creature, mangleSpellId);
            uint32 const expectedRakeTick = floor(AP * 0.06f + ClassSpellsDamage::Druid::RAKE_RNK_5_BLEED_TICK) * mangleFactor;
            uint32 const expectedRakeBleed = 3 * expectedRakeTick;
            TEST_DOT_DAMAGE(druidTestDamage, creature, ClassSpells::Druid::RAKE_RNK_5, expectedRakeBleed, false);
		}

		void Test() override
		{
			// Initialize 2 druids, same stats
			TestPlayer* druidWithMangle = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);
			TestPlayer* druidTestDamage = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);

			Position spawnPosition(_location);
			spawnPosition.MoveInFront(_location, 3.0f);
			Creature* creature = SpawnCreatureWithPosition(spawnPosition, 17); // 0 armor

			EQUIP_NEW_ITEM(druidWithMangle, 30883); // Pillar of Ferocity
			EQUIP_NEW_ITEM(druidTestDamage, 30883); // Pillar of Ferocity

			LearnTalent(druidWithMangle, Talents::Druid::MANGLE_RNK_1);
			TEST_ASSERT(druidWithMangle->HasSpell(ClassSpells::Druid::MANGLE_BEAR_RNK_1));
			TEST_ASSERT(druidWithMangle->HasSpell(ClassSpells::Druid::MANGLE_CAT_RNK_1));
			TEST_ASSERT(druidWithMangle->GetTotalAttackPowerValue(BASE_ATTACK) == druidTestDamage->GetTotalAttackPowerValue(BASE_ATTACK));

			// Morph both druids in cat form
            TEST_CAST(druidWithMangle, druidWithMangle, ClassSpells::Druid::CAT_FORM_RNK_1, SPELL_CAST_OK, TRIGGERED_FULL_MASK);
            TEST_CAST(druidTestDamage, druidTestDamage, ClassSpells::Druid::CAT_FORM_RNK_1, SPELL_CAST_OK, TRIGGERED_FULL_MASK);
            druidWithMangle->Regenerate(POWER_ENERGY);
            druidTestDamage->Regenerate(POWER_ENERGY);
			TestMangle(druidWithMangle, druidTestDamage, creature, ClassSpells::Druid::MANGLE_CAT_RNK_3);

            // Druid with Mangle shapeshift into Bear
            druidWithMangle->RemoveAurasDueToSpell(ClassSpells::Druid::CAT_FORM_RNK_1);
            TEST_CAST(druidWithMangle, druidWithMangle, ClassSpells::Druid::DIRE_BEAR_FORM_RNK_2, SPELL_CAST_OK, TRIGGERED_FULL_MASK);
            druidWithMangle->Regenerate(POWER_RAGE);
			TestMangle(druidWithMangle, druidTestDamage, creature, ClassSpells::Druid::MANGLE_BEAR_RNK_3);
		}
	};

	std::unique_ptr<TestCase> GetTest() const override
	{
		return std::make_unique<MangleTestImpt>();
	}
};

class ImprovedMarkOfTheWildTest : public TestCaseScript
{
public:
	ImprovedMarkOfTheWildTest() : TestCaseScript("talents druid improved_mark_of_the_wild") { }

	class ImprovedMarkOfTheWildTestImpt : public TestCase
	{
	public:
		ImprovedMarkOfTheWildTestImpt() : TestCase(STATUS_PASSING) { }

		void TestSpellOfTheWild(TestPlayer* player, TestPlayer* victim, uint32 spellId)
		{
			LearnTalent(player, Talents::Druid::IMPROVED_MARK_OF_THE_WILD_RNK_5);
            float const talentFactor = 1.35f;

            uint32 const statBoost = 14 * talentFactor;
            uint32 const resBoost = 25 * talentFactor;

            uint32 const expectedArmor = victim->GetArmor() + 340 * talentFactor + statBoost * 2; //also add armor related to agility increase
			uint32 const expectedStr = victim->GetStat(STAT_STRENGTH) + statBoost;
			uint32 const expectedAgi = victim->GetStat(STAT_AGILITY) + statBoost;
			uint32 const expectedSta = victim->GetStat(STAT_STAMINA) + statBoost;
			uint32 const expectedInt = victim->GetStat(STAT_INTELLECT) + statBoost;
			uint32 const expectedSpi = victim->GetStat(STAT_SPIRIT) + statBoost;
			uint32 const expectedResArcane = victim->GetResistance(SPELL_SCHOOL_ARCANE) + resBoost;
			uint32 const expectedResFire = victim->GetResistance(SPELL_SCHOOL_FIRE) + resBoost;
			uint32 const expectedResFrost = victim->GetResistance(SPELL_SCHOOL_FROST) + resBoost;
			uint32 const expectedResNature = victim->GetResistance(SPELL_SCHOOL_NATURE) + resBoost;
			uint32 const expectedResShadow = victim->GetResistance(SPELL_SCHOOL_SHADOW) + resBoost;

            TEST_CAST(player, victim, spellId, SPELL_CAST_OK, TRIGGERED_IGNORE_GCD);
            TEST_HAS_AURA(victim, spellId);

			TEST_ASSERT(Between<uint32>(victim->GetArmor(), expectedArmor - 1, expectedArmor + 1));
			TEST_ASSERT(Between<uint32>(victim->GetStat(STAT_AGILITY), expectedAgi - 1, expectedAgi + 1));
			TEST_ASSERT(Between<uint32>(victim->GetStat(STAT_INTELLECT), expectedInt - 1, expectedInt + 1));
			TEST_ASSERT(Between<uint32>(victim->GetStat(STAT_SPIRIT), expectedSpi - 1, expectedSpi + 1));
			TEST_ASSERT(Between<uint32>(victim->GetStat(STAT_STAMINA), expectedSta - 1, expectedSta + 1));
			TEST_ASSERT(Between<uint32>(victim->GetStat(STAT_STRENGTH), expectedStr - 1, expectedStr + 1));
			TEST_ASSERT(Between<uint32>(victim->GetResistance(SPELL_SCHOOL_ARCANE), expectedResArcane - 1, expectedResArcane + 1));
			TEST_ASSERT(Between<uint32>(victim->GetResistance(SPELL_SCHOOL_FIRE), expectedResFire - 1, expectedResFire + 1));
			TEST_ASSERT(Between<uint32>(victim->GetResistance(SPELL_SCHOOL_FROST), expectedResFrost - 1, expectedResFrost + 1));
			TEST_ASSERT(Between<uint32>(victim->GetResistance(SPELL_SCHOOL_NATURE), expectedResNature - 1, expectedResNature + 1));
			TEST_ASSERT(Between<uint32>(victim->GetResistance(SPELL_SCHOOL_SHADOW), expectedResShadow - 1, expectedResShadow + 1));
		}

		void Test() override
		{
			TestPlayer* druid = SpawnRandomPlayer(CLASS_DRUID);
			TestSpellOfTheWild(druid, druid, ClassSpells::Druid::MARK_OF_THE_WILD_RNK_8);

			TestPlayer* player2 = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);
			TestPlayer* target = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN);
			player2->AddItem(22148, 1); // Wild Quillvine, regeant for Git of the Wild
			TestSpellOfTheWild(player2, target, ClassSpells::Druid::GIFT_OF_THE_WILD_RNK_3);
		}
	};

	std::unique_ptr<TestCase> GetTest() const override
	{
		return std::make_unique<ImprovedMarkOfTheWildTestImpt>();
	}
};

class FurorTest : public TestCase
{
    /*
    "Gives you 100% chance to gain 10 Rage when you shapeshift into Bear and Dire Bear Form or 40 Energy when you shapeshift into Cat Form."
    */
	void Test() override
	{
		TestPlayer* druid = SpawnRandomPlayer(CLASS_DRUID);
		LearnTalent(druid, Talents::Druid::FUROR_RNK_5);

        SECTION("Bear rage", [&] {
            TEST_CAST(druid, druid, ClassSpells::Druid::BEAR_FORM_RNK_1, SPELL_CAST_OK, TRIGGERED_IGNORE_GCD);
            ASSERT_INFO("Druid has %u Rage but %u was expected.", druid->GetPower(POWER_RAGE), uint32(100));
            TEST_ASSERT(druid->GetPower(POWER_RAGE) == uint32(100)); // = 10 rage
        });
        druid->RemoveAurasDueToSpell(ClassSpells::Druid::BEAR_FORM_RNK_1); //we need to dispell it manually to be able to switch to cat, clients send CMSG_CANCEL_AURA when switching form

        SECTION("Cat power", [&] {
            TEST_CAST(druid, druid, ClassSpells::Druid::CAT_FORM_RNK_1);
            TEST_HAS_AURA(druid, ClassSpells::Druid::CAT_FORM_RNK_1);
            TEST_ASSERT(druid->GetPower(POWER_ENERGY) == 40);
        });
	}
};

class NaturalistTest : public TestCaseScript
{
public:
	NaturalistTest() : TestCaseScript("talents druid naturalist") { }

	class NaturalistTestImpt : public TestCase
	{
	public:
		NaturalistTestImpt() : TestCase(STATUS_PASSING_INCOMPLETE) { }

		void Test() override
		{
			TestPlayer* druid = SpawnRandomPlayer(CLASS_DRUID);

			LearnTalent(druid, Talents::Druid::NATURALIST_RNK_5);

            // Healing Touch reduced cast time by 0.5s
            TEST_CAST_TIME(druid, ClassSpells::Druid::HEALING_TOUCH_RNK_13, 3000);

			// TODO: increases physical damage dealt in all forms by 10%
		}
	};

	std::unique_ptr<TestCase> GetTest() const override
	{
		return std::make_unique<NaturalistTestImpt>();
	}
};

class NaturalShapeshifterTest : public TestCase
{
    /*
    "Reduces the mana cost of all shapeshifting by 30%."
    Bugs:
        - Should reduce mana cost of Tree of Life
    */
	void Test() override
	{
		TestPlayer* druid = SpawnRandomPlayer(CLASS_DRUID);
		LearnTalent(druid, Talents::Druid::NATURAL_SHAPESHIFTER_RNK_3);
        float const talentFactor = 0.7f;

		uint32 const expectedBearMana	= 829 * talentFactor;
		uint32 const expectedCatMana	= 829 * talentFactor;
		uint32 const expectedTravelMana = 308 * talentFactor;
		uint32 const expectedAquaMana	= 308 * talentFactor;
		uint32 const expectedMoonMana	= 521 * talentFactor;
		uint32 const expectedTreeMana	= 663 * talentFactor;

        SECTION("Bear form", [&] {
            TEST_POWER_COST(druid, ClassSpells::Druid::BEAR_FORM_RNK_1, POWER_MANA, expectedBearMana);
        });
        SECTION("Cat form", [&] {
            TEST_POWER_COST(druid, ClassSpells::Druid::CAT_FORM_RNK_1, POWER_MANA, expectedCatMana);
        });
        SECTION("Travel form", [&] {
            TEST_POWER_COST(druid, ClassSpells::Druid::TRAVEL_FORM_RNK_1, POWER_MANA, expectedTravelMana);
        });
		//TEST_POWER_COST(player, ClassSpells::Druid::AQUATIC_FORM_RNK_1, POWER_MANA, expectedAquaMana);
        SECTION("Moonkin form", [&] {
            TEST_POWER_COST(druid, ClassSpells::Druid::MOONKIN_FORM_RNK_1, POWER_MANA, expectedMoonMana);
        });
        SECTION("Tree of life form", STATUS_KNOWN_BUG, [&] {
            TEST_POWER_COST(druid, ClassSpells::Druid::TREE_OF_LIFE_RNK_1, POWER_MANA, expectedTreeMana);
        });
	}
};

class IntensityTest : public TestCaseScript
{
public:
	IntensityTest() : TestCaseScript("talents druid intensity") { }

	class IntensityTestImpt : public TestCase
	{
	public:
		IntensityTestImpt() : TestCase(STATUS_PASSING) { }

		void Test() override
		{
			TestPlayer* druid = SpawnRandomPlayer(CLASS_DRUID);

			LearnTalent(druid, Talents::Druid::INTENSITY_RNK_3);

            // Allows 30% of your Mana regen to continue while casting
            float const talentFactor = 0.3f;
            float const expectedManaRegen = druid->GetFloatValue(PLAYER_FIELD_MOD_MANA_REGEN) * talentFactor;

            ASSERT_INFO("Druid should have %f mana regen but has %f.", expectedManaRegen, druid->GetFloatValue(PLAYER_FIELD_MOD_MANA_REGEN_INTERRUPT));
            TEST_ASSERT(Between<float>(druid->GetFloatValue(PLAYER_FIELD_MOD_MANA_REGEN_INTERRUPT), expectedManaRegen - 0.1f, expectedManaRegen + 0.1));

			// 10 rage when using Enrage
            TEST_CAST(druid, druid, ClassSpells::Druid::BEAR_FORM_RNK_1, SPELL_CAST_OK, TRIGGERED_FULL_MASK);
            TEST_CAST(druid, druid, ClassSpells::Druid::ENRAGE_RNK_1);
			ASSERT_INFO("Druid doesnt have 10 rage but %u", druid->GetPower(POWER_RAGE));
			TEST_ASSERT(druid->GetPower(POWER_RAGE) == 100);
		}
	};

	std::unique_ptr<TestCase> GetTest() const override
	{
		return std::make_unique<IntensityTestImpt>();
	}
};

class TranquilSpiritTest : public TestCaseScript
{
public:
	TranquilSpiritTest() : TestCaseScript("talents druid tranquil_spirit") { }

	class TranquilSpiritTestImpt : public TestCase
	{
	public:
		TranquilSpiritTestImpt() : TestCase(STATUS_PASSING) { }

		void Test() override
		{
			TestPlayer* druid = SpawnRandomPlayer(CLASS_DRUID);

			LearnTalent(druid, Talents::Druid::TRANQUIL_SPIRIT_RNK_5);
            float const talentFactor = 0.9f;

			uint32 const expectedHealingTouchMana = 935 * talentFactor;
			uint32 const expectedTranquilityMana = 1650 * talentFactor;

			TEST_POWER_COST(druid, ClassSpells::Druid::HEALING_TOUCH_RNK_13, POWER_MANA, expectedHealingTouchMana);
            TEST_POWER_COST(druid, ClassSpells::Druid::TRANQUILITY_RNK_5, POWER_MANA, expectedTranquilityMana);
		}
	};

	std::unique_ptr<TestCase> GetTest() const override
	{
		return std::make_unique<TranquilSpiritTestImpt>();
	}
};

class ImprovedRejuvenationTest : public TestCaseScript
{
public:
	ImprovedRejuvenationTest() : TestCaseScript("talents druid improved_rejuvenation") { }

	class ImprovedRejuvenationTestImpt : public TestCase
	{
	public:
		ImprovedRejuvenationTestImpt() : TestCase(STATUS_PASSING) { }

		void Test() override
		{
			TestPlayer* druid = SpawnRandomPlayer(CLASS_DRUID);

			LearnTalent(druid, Talents::Druid::IMPROVED_REJUVENATION_RNK_3);
            float const talentFactor = 1.15f;

            uint32 const tickAmount = 4;
            uint32 const rejuvenationTick = ClassSpellsDamage::Druid::REJUVENATION_RNK_13_TICK * talentFactor;
			uint32 const expectedRejuvenationTotal = tickAmount * rejuvenationTick;
            TEST_DOT_DAMAGE(druid, druid, ClassSpells::Druid::REJUVENATION_RNK_13, expectedRejuvenationTotal, false);
		}
	};

	std::unique_ptr<TestCase> GetTest() const override
	{
		return std::make_unique<ImprovedRejuvenationTestImpt>();
	}
};

class GiftOfNatureTest : public TestCase
{
    /*
    Increases the effect of all healing spells by $s1%.
    Bugs:
        - Should affect the Lifebloom burst.
    */
	void Test() override
	{
		TestPlayer* druid = SpawnRandomPlayer(CLASS_DRUID);
            
        druid->DisableRegeneration(true);

		LearnTalent(druid, Talents::Druid::GIFT_OF_NATURE_RNK_5);
		float const giftOfNatureFactor = 1.1f;

        uint32 const regrowthTick = ClassSpellsDamage::Druid::REGROWTH_RNK_10_TICK * giftOfNatureFactor;
        uint32 const regrowthTotal = 6 * regrowthTick;

        uint32 const rejuvenationTotal = 4 * floor(ClassSpellsDamage::Druid::REJUVENATION_RNK_13_TICK * giftOfNatureFactor);
        SECTION("Switfmend", [&] {
            TEST_DIRECT_HEAL(druid, druid, ClassSpells::Druid::SWIFTMEND_RNK_1, rejuvenationTotal, rejuvenationTotal, false, [](Unit* caster, Unit* victim) {
                caster->AddAura(ClassSpells::Druid::REJUVENATION_RNK_13, caster);
            });
            TEST_DIRECT_HEAL(druid, druid, ClassSpells::Druid::SWIFTMEND_RNK_1, regrowthTotal, regrowthTotal, false, [](Unit* caster, Unit* victim) {
                caster->AddAura(ClassSpells::Druid::REGROWTH_RNK_10, caster);
            });
        });

        SECTION("Rejuvenation", [&] {
            TEST_DOT_DAMAGE(druid, druid, ClassSpells::Druid::REJUVENATION_RNK_13, rejuvenationTotal, false);
        });

        SECTION("Lifebloom hot", [&] {
            druid->SetHealth(1);
            uint32 const lifebloomTickAmount = 7;
            uint32 const lifebloomTick = ClassSpellsDamage::Druid::LIFEBLOOM_RNK_1_TICK * giftOfNatureFactor;
            uint32 const expectedLifebloom = lifebloomTickAmount * lifebloomTick;
            EnableCriticals(druid, false);
            TEST_CAST(druid, druid, ClassSpells::Druid::LIFEBLOOM_RNK_1);
            Wait(7500);
            auto AI = _GetCasterAI(druid);
            auto[dotDamageToTarget, tickCount] = AI->GetDotDamage(druid, ClassSpells::Druid::LIFEBLOOM_RNK_1);
            TEST_ASSERT(tickCount == lifebloomTickAmount);
            ASSERT_INFO("HoT did %u instead of %u", dotDamageToTarget, expectedLifebloom);
            TEST_ASSERT(dotDamageToTarget == int32(expectedLifebloom));
        });

        SECTION("Lifebloom burst", STATUS_KNOWN_BUG, [&] {
            /* Kelno: Spells has no family class flags... and Gift Of Nature class mask affect from LK client does not include it either.
            Thus it's very possible that this spell shouldn't be affected, but we have no definitive proof of this and probably should include it,
            else it will seem bugged
            */
            uint32 const expectedBurst = ClassSpellsDamage::Druid::LIFEBLOOM_RNK_1_BURST * giftOfNatureFactor;
            auto[dealtMin, dealtMax] = GetHealingPerSpellsTo(druid, druid, ClassSpells::Druid::LIFEBLOOM_RNK_1_FINAL_PROC, false, 1);
            ASSERT_INFO("Lifebloom bursted for %u instead of %u.", dealtMin, expectedBurst);
            TEST_ASSERT(dealtMin == expectedBurst);
        });

        SECTION("Regrowth", [&] {
            // Regrowth HoTs
            TEST_DOT_DAMAGE(druid, druid, ClassSpells::Druid::REGROWTH_RNK_10, 7 * regrowthTick, false);
            // Regrowth Direct
            uint32 const regrowthMin = ClassSpellsDamage::Druid::REGROWTH_RNK_10_MIN * giftOfNatureFactor;
            uint32 const regrowthMax = ClassSpellsDamage::Druid::REGROWTH_RNK_10_MAX * giftOfNatureFactor;
            TEST_DIRECT_HEAL(druid, druid, ClassSpells::Druid::REGROWTH_RNK_10, regrowthMin, regrowthMax, false);
        });

        SECTION("Healing Touch", [&] {
            uint32 const healingTouchMin = ClassSpellsDamage::Druid::HEALING_TOUCH_RNK_13_MIN * giftOfNatureFactor;
            uint32 const healingTouchMax = ClassSpellsDamage::Druid::HEALING_TOUCH_RNK_13_MAX * giftOfNatureFactor;
            TEST_DIRECT_HEAL(druid, druid, ClassSpells::Druid::HEALING_TOUCH_RNK_13, healingTouchMin, healingTouchMax, false);
        });
	}
};

class EmpoweredTouchTest : public TestCaseScript
{
public:
	EmpoweredTouchTest() : TestCaseScript("talents druid empowered_touch") { }

	class EmpoweredTouchTestImpt : public TestCase
	{
	public:
		EmpoweredTouchTestImpt() : TestCase(STATUS_PASSING) { }

		void Test() override
		{
			TestPlayer* druid = SpawnRandomPlayer(CLASS_DRUID);

			LearnTalent(druid, Talents::Druid::EMPOWERED_TOUCH_RNK_2);
            float const talentFactor = 0.2f;

            EQUIP_NEW_ITEM(druid, 34335); // Hammer of Sanctification -- 183 SP & 550 BH

			uint32 const bh = druid->GetInt32Value(PLAYER_FIELD_MOD_HEALING_DONE_POS);
			float const healingTouchCoeff = ClassSpellsCoeff::Druid::HEALING_TOUCH + talentFactor;

			uint32 const healingTouchMin = ClassSpellsDamage::Druid::HEALING_TOUCH_RNK_13_MIN + bh * healingTouchCoeff;
			uint32 const healingTouchMax = ClassSpellsDamage::Druid::HEALING_TOUCH_RNK_13_MAX + bh * healingTouchCoeff;
			TEST_DIRECT_HEAL(druid, druid, ClassSpells::Druid::HEALING_TOUCH_RNK_13, healingTouchMin, healingTouchMax, false);
		}
	};

	std::unique_ptr<TestCase> GetTest() const override
	{
		return std::make_unique<EmpoweredTouchTestImpt>();
	}
};

class LivingSpiritTest : public TestCaseScript
{
public:
	LivingSpiritTest() : TestCaseScript("talents druid living_spirit") { }

	class LivingSpiritTestImpt : public TestCase
	{
	public:
		LivingSpiritTestImpt() : TestCase(STATUS_PASSING) { }

		void Test() override
		{
			TestPlayer* druid = SpawnRandomPlayer(CLASS_DRUID);

            float const talentFactor = 1.15f;
			float const expectedSpi = floor(druid->GetStat(STAT_SPIRIT) * talentFactor); 

			LearnTalent(druid, Talents::Druid::LIVING_SPIRIT_RNK_3);
            ASSERT_INFO("Druid has %f Spirit but %f was expected.", druid->GetStat(STAT_SPIRIT), expectedSpi);
			TEST_ASSERT(Between<float>(druid->GetStat(STAT_SPIRIT), expectedSpi - 1, expectedSpi + 1));
		}
	};

	std::unique_ptr<TestCase> GetTest() const override
	{
		return std::make_unique<LivingSpiritTestImpt>();
	}
};

class EmpoweredRejuvenationTest : public TestCase
{
    /* "talents druid empowered_rejuvenation"
    "The bonus healing effects of your healing over time spells is increased by 20%."
    Bugs:
        - Lifebloom burst should be affected by the talent.
        - Regrowth has some issues with its spell coeff.
    */
	void Test() override
	{
		TestPlayer* druid = SpawnRandomPlayer(CLASS_DRUID);

		EQUIP_NEW_ITEM(druid, 34335); // Hammer of Sanctification - 550 SP
        uint32 const bh = druid->GetInt32Value(PLAYER_FIELD_MOD_HEALING_DONE_POS);

		LearnTalent(druid, Talents::Druid::EMPOWERED_REJUVENATION_RNK_5);
        float const talentFactor = 1.2f;

        uint32 const rejuvenationTickAmount = 4;
        uint32 const rejuvenationBhPerTick = bh * ClassSpellsCoeff::Druid::REJUVENATION * talentFactor / rejuvenationTickAmount;
        uint32 const rejuvenationTick = ClassSpellsDamage::Druid::REJUVENATION_RNK_13_TICK + rejuvenationBhPerTick;
        uint32 const rejuvenationTotal = rejuvenationTickAmount * rejuvenationTick;
        SECTION("Rejuvenation", [&] {
            TEST_DOT_DAMAGE(druid, druid, ClassSpells::Druid::REJUVENATION_RNK_13, rejuvenationTotal, false);
        });

        uint32 const regrowthTick = ClassSpellsDamage::Druid::REGROWTH_RNK_10_TICK + bh * ClassSpellsCoeff::Druid::REGROWTH_HOT * talentFactor;
        SECTION("Regrowth", [&] {
            // Regrowth HoT
            uint32 const expectedRegrowthTotal = 7 * regrowthTick;
            TEST_DOT_DAMAGE(druid, druid, ClassSpells::Druid::REGROWTH_RNK_10, expectedRegrowthTotal, false);
            // Regrowth Direct
            uint32 const regrowthBhBonus = bh * ClassSpellsCoeff::Druid::REGROWTH * talentFactor;
            uint32 const regrowthMin = ClassSpellsDamage::Druid::REGROWTH_RNK_10_MIN + regrowthBhBonus;
            uint32 const regrowthMax = ClassSpellsDamage::Druid::REGROWTH_RNK_10_MAX + regrowthBhBonus;
            TEST_DIRECT_HEAL(druid, druid, ClassSpells::Druid::REGROWTH_RNK_10, regrowthMin, regrowthMax, false);
        });

        SECTION("Swiftmend", [&] {
            TEST_DIRECT_HEAL(druid, druid, ClassSpells::Druid::SWIFTMEND_RNK_1, rejuvenationTotal, rejuvenationTotal, false, [](Unit* caster, Unit* victim) {
                caster->AddAura(ClassSpells::Druid::REJUVENATION_RNK_13, caster);
            });
            uint32 const regrowthTotal = 6 * regrowthTick;
            TEST_DIRECT_HEAL(druid, druid, ClassSpells::Druid::SWIFTMEND_RNK_1, regrowthTotal, regrowthTotal, false, [](Unit* caster, Unit* victim) {
                caster->AddAura(ClassSpells::Druid::REGROWTH_RNK_10, caster);
            });
        });

        SECTION("Tranquility", [&] {
            float const tranquilityTickAmount = 4.f;
            float const tranquilityTickCoeff = ClassSpellsCoeff::Druid::TRANQUILITY_LVL_70 * talentFactor / tranquilityTickAmount;
            uint32 const expectedTranquilityTick = ClassSpellsDamage::Druid::TRANQUILITY_RNK_5_TICK + bh * tranquilityTickCoeff;
            TEST_CHANNEL_HEALING(druid, druid, ClassSpells::Druid::TRANQUILITY_RNK_5, 4, expectedTranquilityTick, ClassSpells::Druid::TRANQUILITY_RNK_5_PROC);
        });

        SECTION("Lifebloom hot", [&] {
            druid->SetHealth(1);
            uint32 const lifebloomTickAmount = 7;
            uint32 const lifebloomBhPerTick = bh * ClassSpellsCoeff::Druid::LIFEBLOOM_HOT * talentFactor / lifebloomTickAmount;
            uint32 const lifebloomTick = ClassSpellsDamage::Druid::LIFEBLOOM_RNK_1_TICK + lifebloomBhPerTick;
            int32 const expectedLifebloom = lifebloomTickAmount * lifebloomTick;
            EnableCriticals(druid, false);
            TEST_CAST(druid, druid, ClassSpells::Druid::LIFEBLOOM_RNK_1);
            Wait(7500);
            auto AI = _GetCasterAI(druid);
            auto[dotDamageToTarget, tickCount] = AI->GetDotDamage(druid, ClassSpells::Druid::LIFEBLOOM_RNK_1);
            TEST_ASSERT(tickCount == lifebloomTickAmount);
            ASSERT_INFO("HoT did %u instead of %u", dotDamageToTarget, expectedLifebloom);
            TEST_ASSERT(dotDamageToTarget == int32(expectedLifebloom));
        });

        SECTION("Lifebloom burst", STATUS_KNOWN_BUG, [&] {
            uint32 const expectedBurst = ClassSpellsDamage::Druid::LIFEBLOOM_RNK_1_BURST + bh * ClassSpellsCoeff::Druid::LIFEBLOOM * talentFactor;
            auto[dealtMin, dealtMax] = GetHealingPerSpellsTo(druid, druid, ClassSpells::Druid::LIFEBLOOM_RNK_1_FINAL_PROC, false, 1);
            ASSERT_INFO("Lifebloom bursted for %u instead of %u.", dealtMin, expectedBurst);
            TEST_ASSERT(dealtMin == expectedBurst);
        });
	}
};

void AddSC_test_talents_druid()
{
	// Total: 37/62
	// Balance: 16/21
	new StarlightWrathTest();
    new NaturesGraspTest();
	new ImprovedNaturesGraspTest();
    new ControlOfNatureTest();
    new FocusedStarlightTest();
    new ImprovedMoonfireTest();
    RegisterTestCase("talents druid brambles", BramblesTest);
	new InsectSwarmTest();
	new NaturesReachTest();
    new VengeanceTest();
	new LunarGuidanceTest();
    new NaturesGraceTest();
	new MoonglowTest();
	new MoonfuryTest();
	new DreamstateTest();
	new WrathOfCenariusTest();
	// Feral: 10/21
	new FerocityTest();
	new BrutalImpactTest();
	new ThickHideTest();
	new ShreddingAttacksTest();
	new PredatoryStrikesTest();
	new SavageFuryTest();
	new FaerieFireFeralTest();
    RegisterTestCase("talents druid heart_of_the_wild", HeartOfTheWildTest);
	new SurvivalOfTheFittestTest();
	new MangleTest();
	// Restoration: 11/20
	new ImprovedMarkOfTheWildTest();
    RegisterTestCase("talents druid furor", FurorTest);
	new NaturalistTest();
    RegisterTestCase("talents druid natural_shapeshifter", NaturalShapeshifterTest);
	new IntensityTest();
	new TranquilSpiritTest();
	new ImprovedRejuvenationTest();
    RegisterTestCase("talents druid gift_of_nature", GiftOfNatureTest);
	new EmpoweredTouchTest();
	new LivingSpiritTest();
    RegisterTestCase("talents druid empowered_rejuvenation", EmpoweredRejuvenationTest);
}
