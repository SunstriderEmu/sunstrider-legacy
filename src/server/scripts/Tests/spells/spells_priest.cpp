#include "../ClassSpellsDamage.h"
#include "../ClassSpellsCoeff.h"
#include "PlayerbotAI.h"
#include "SpellHistory.h"
#include <limits>

//"spells priest consume_magic"
class ConsumeMagicTest : public TestCase
{
public:
    void ConsumeMagic(TestPlayer* caster, TriggerCastFlags triggerCastFlags = TRIGGERED_NONE)
    {
        caster->SetPower(POWER_MANA, 0);
        caster->AddAura(ClassSpells::Priest::INNER_FIRE_RNK_1, caster);
        TEST_CAST(caster, caster, ClassSpells::Priest::CONSUME_MAGIC_RNK_1, SPELL_CAST_OK, triggerCastFlags);
    }

    void Test() override
    {
        uint32 priestLevel = 70;
        TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF, priestLevel);

        priest->DisableRegeneration(true);
        WaitNextUpdate();

        // Only on Priest's spells
        priest->AddAura(ClassSpells::Druid::MARK_OF_THE_WILD_RNK_8, priest);
        TEST_CAST(priest, priest, ClassSpells::Priest::CONSUME_MAGIC_RNK_1, SPELL_FAILED_CASTER_AURASTATE);
        TEST_HAS_AURA(priest, ClassSpells::Druid::MARK_OF_THE_WILD_RNK_8);
        priest->RemoveAurasDueToSpell(ClassSpells::Druid::MARK_OF_THE_WILD_RNK_8);

        // Cooldown
        ConsumeMagic(priest);
        TEST_HAS_COOLDOWN(priest, ClassSpells::Priest::CONSUME_MAGIC_RNK_1, Minutes(2));

        // Test mana gain
        uint32 consumeMagicSpellLevel = 20;
        uint32 consumeMagicBase = 120;
        uint32 consumeMagicMax = 35;
        float consumeMagicManaPerLevel = 10.8;
        uint32 expectedMin = consumeMagicBase + consumeMagicManaPerLevel * (priestLevel - consumeMagicSpellLevel);
        uint32 expectedMax = expectedMin + consumeMagicMax;

        auto[sampleSize, maxPredictionError] = _GetApproximationParams(expectedMin, expectedMax);

        uint32 minGain = std::numeric_limits<uint32>::max();
        uint32 maxGain = 0;

        for (uint32 i = 0; i < sampleSize; i++)
        {
            ConsumeMagic(priest, TriggerCastFlags(TRIGGERED_IGNORE_SPELL_AND_CATEGORY_CD | TRIGGERED_IGNORE_GCD));
            uint32 priestMana = priest->GetPower(POWER_MANA);

            minGain = std::min(minGain, priestMana);
            maxGain = std::max(maxGain, priestMana);
            HandleThreadPause();
        }

        uint32 allowedMin = expectedMin > maxPredictionError ? expectedMin - maxPredictionError : 0; // protect against underflow
        uint32 allowedMax = expectedMax + maxPredictionError;

        TEST_ASSERT(minGain >= allowedMin);
        TEST_ASSERT(maxGain <= allowedMax);
    }
};

class DispelMagicTest : public TestCaseScript
{
public:
    DispelMagicTest() : TestCaseScript("spells priest dispel_magic") { }

    class DispelMagicTestImpt : public TestCase
    {
    public:


        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);
            TestPlayer* enemy1 = SpawnPlayer(CLASS_PRIEST, RACE_HUMAN);
            TestPlayer* enemy2 = SpawnPlayer(CLASS_DRUID, RACE_NIGHTELF);

            // Fail -- Bug here
            TEST_CAST(priest, priest, ClassSpells::Priest::DISPEL_MAGIC_RNK_2, SPELL_FAILED_NOTHING_TO_DISPEL, TRIGGERED_IGNORE_GCD);
            TEST_CAST(priest, enemy1, ClassSpells::Priest::DISPEL_MAGIC_RNK_2, SPELL_FAILED_NOTHING_TO_DISPEL, TRIGGERED_IGNORE_GCD);

            // Test mana cost
            uint32 const expectedDispelMagicMana = 366;
            TEST_POWER_COST(priest, ClassSpells::Priest::DISPEL_MAGIC_RNK_2, POWER_MANA, expectedDispelMagicMana);

            // Setup
            FORCE_CAST(enemy1, enemy1, ClassSpells::Priest::DIVINE_SPIRIT_RNK_5, SPELL_MISS_NONE, TRIGGERED_FULL_MASK);
            FORCE_CAST(enemy1, enemy1, ClassSpells::Priest::FEAR_WARD_RNK_1, SPELL_MISS_NONE, TRIGGERED_FULL_MASK);
            FORCE_CAST(enemy1, enemy1, ClassSpells::Priest::INNER_FIRE_RNK_7, SPELL_MISS_NONE, TRIGGERED_FULL_MASK);
            FORCE_CAST(enemy1, priest, ClassSpells::Priest::SHADOW_WORD_PAIN_RNK_10, SPELL_MISS_NONE, TRIGGERED_FULL_MASK);
            FORCE_CAST(enemy2, priest, ClassSpells::Druid::MOONFIRE_RNK_12, SPELL_MISS_NONE, TRIGGERED_FULL_MASK);
            FORCE_CAST(enemy2, priest, ClassSpells::Druid::INSECT_SWARM_RNK_6, SPELL_MISS_NONE, TRIGGERED_FULL_MASK);

            TEST_HAS_AURA(enemy1, ClassSpells::Priest::DIVINE_SPIRIT_RNK_5);
            TEST_HAS_AURA(enemy1, ClassSpells::Priest::FEAR_WARD_RNK_1);
            TEST_HAS_AURA(enemy1, ClassSpells::Priest::INNER_FIRE_RNK_7);
            TEST_HAS_AURA(priest, ClassSpells::Priest::SHADOW_WORD_PAIN_RNK_10);
            TEST_HAS_AURA(priest, ClassSpells::Druid::MOONFIRE_RNK_12);
            TEST_HAS_AURA(priest, ClassSpells::Druid::INSECT_SWARM_RNK_6);

            // Note: Auras are dispelled in random order (No 100% sure for defensive, it may be last in first out)
            // Defensive dispel
            priest->ResetAllPowers();
            TEST_CAST(priest, priest, ClassSpells::Priest::DISPEL_MAGIC_RNK_2, SPELL_CAST_OK, TRIGGERED_IGNORE_GCD);
            uint32 remainingBuffs =   uint32(priest->HasAura(ClassSpells::Druid::INSECT_SWARM_RNK_6))
                                    + uint32(priest->HasAura(ClassSpells::Druid::MOONFIRE_RNK_12))
                                    + uint32(priest->HasAura(ClassSpells::Priest::SHADOW_WORD_PAIN_RNK_10));
            ASSERT_INFO("Testing defensive dispel, check if priest has correctly lost 2 aura");
            TEST_ASSERT(remainingBuffs == 1);

            // Offensive dispel (random order confirmed in BC arena videos)
            FORCE_CAST(priest, enemy1, ClassSpells::Priest::DISPEL_MAGIC_RNK_2, SPELL_MISS_NONE, TRIGGERED_IGNORE_GCD);
            remainingBuffs =  uint32(enemy1->HasAura(ClassSpells::Priest::INNER_FIRE_RNK_7))
                            + uint32(enemy1->HasAura(ClassSpells::Priest::FEAR_WARD_RNK_1))
                            + uint32(enemy1->HasAura(ClassSpells::Priest::DIVINE_SPIRIT_RNK_5));
            ASSERT_INFO("Testing offensive dispel, check if target has correctly lost 2 aura");
            TEST_ASSERT(remainingBuffs == 1);
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<DispelMagicTestImpt>();
    }
};

class FearWardTest : public TestCaseScript
{
public:
    FearWardTest() : TestCaseScript("spells priest fear_ward") { }

    class FearWardTestImpt : public TestCase
    {
    public:


        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_HUMAN);
            warlock->ApplyRatingMod(CR_HIT_SPELL, 500, true); //Make sure the warlock is hit capped

            uint32 const expectedFearWardMana = 78;
            TEST_POWER_COST(priest, ClassSpells::Priest::FEAR_WARD_RNK_1, POWER_MANA, expectedFearWardMana);

            TEST_CAST(priest, priest, ClassSpells::Priest::FEAR_WARD_RNK_1);
            TEST_COOLDOWN(priest, priest, ClassSpells::Priest::FEAR_WARD_RNK_1, Minutes(3));

            TEST_CAST(priest, priest, ClassSpells::Priest::FEAR_WARD_RNK_1);
            TEST_AURA_MAX_DURATION(priest, ClassSpells::Priest::FEAR_WARD_RNK_1, Minutes(3));

            // First fear, should be resisted by ward
            // Cast fear and test immunity. We can't use ForceSpellHitResult because that would bypass immune. Return false if we received the incompressible %. 
            auto TestFear = [&](bool shouldImmune) {
                warlock->m_modSpellHitChance = 1.0f; //100% hit chance
                auto AI = _GetCasterAI(warlock);
                AI->ResetSpellCounters();
                TEST_CAST(warlock, priest, ClassSpells::Warlock::FEAR_RNK_3, SPELL_CAST_OK, TriggerCastFlags(TRIGGERED_FULL_MASK | TRIGGERED_TREAT_AS_NON_TRIGGERED)); //TRIGGERED_TREAT_AS_NON_TRIGGERED else aura does not get removed
                auto damageDoneInfo = AI->GetSpellDamageDoneInfo(priest);
                TEST_ASSERT(damageDoneInfo != nullptr);
                TEST_ASSERT(damageDoneInfo->size() == 1);
                auto itr = *(damageDoneInfo->begin());
                //We got the 1% chance, no luck, try again
                if (itr.missInfo == SPELL_MISS_MISS)
                    return false;

                //should not happen if warlock is properly capped
                TEST_ASSERT(itr.missInfo != SPELL_MISS_RESIST);

                //else, we should have received IMMUNE
                if (shouldImmune)
                {
                    ASSERT_INFO("Wrong missInfo %u", uint32(itr.missInfo));
                    TEST_ASSERT(itr.missInfo == SPELL_MISS_IMMUNE);
                }
                else
                {
                    ASSERT_INFO("Wrong missInfo %u", uint32(itr.missInfo));
                    TEST_ASSERT(itr.missInfo == SPELL_MISS_NONE);
                }

                return true;
            };
            uint32 tryCount = 0; //just to make sure we don't do an infinite loop
            while (!TestFear(true))
            {
                TEST_ASSERT(tryCount++ < 100); 
            }
            // Fear should be resisted
            TEST_HAS_NOT_AURA(priest, ClassSpells::Warlock::FEAR_RNK_3);
            // Ward shoud be consumed
            TEST_HAS_NOT_AURA(priest, ClassSpells::Priest::FEAR_WARD_RNK_1);
            // Second fear, priest should be affected
            tryCount = false;
            while (!TestFear(false))
            {
                TEST_ASSERT(tryCount++ < 100);
            }
            TEST_CAST(warlock, priest, ClassSpells::Warlock::FEAR_RNK_3, SPELL_CAST_OK, TRIGGERED_CAST_DIRECTLY);
            TEST_HAS_AURA(priest, ClassSpells::Warlock::FEAR_RNK_3);
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<FearWardTestImpt>();
    }
};

class FeedbackTest : public TestCaseScript
{
public:
    FeedbackTest() : TestCaseScript("spells priest feedback") { }

    //"The priest becomes surrounded with anti-magic energy. Any successful spell cast against the priest will burn 165 of the attacker's Mana, causing 1 Shadow damage for each point of Mana burned. Lasts 15s."
    class FeedbackTestImpt : public TestCase
    {
    public:


        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);
            TestPlayer* mage = SpawnPlayer(CLASS_MAGE, RACE_HUMAN);
            TestPlayer* rogue = SpawnPlayer(CLASS_ROGUE, RACE_HUMAN);

            EQUIP_NEW_ITEM(priest, 34336); // Sunflare - 292 SP
            priest->DisableRegeneration(true);
            mage->DisableRegeneration(true);
            mage->SetPower(POWER_MANA, 200);
            WaitNextUpdate();

            // Mana cost, aura & cd
            uint32 const expectedFeedbackMana = 705;
            TEST_POWER_COST(priest, ClassSpells::Priest::FEEDBACK_RNK_6, POWER_MANA, expectedFeedbackMana);
            TEST_CAST(priest, priest, ClassSpells::Priest::FEEDBACK_RNK_6);
            TEST_HAS_COOLDOWN(priest, ClassSpells::Priest::FEEDBACK_RNK_6, Minutes(3));
            TEST_HAS_AURA(priest, ClassSpells::Priest::FEEDBACK_RNK_6);

            // Melee attack shouldn't proc the spell
            priest->ForceSpellHitResultOverride(SPELL_MISS_NONE);
            uint32 const rogueHealth = rogue->GetHealth();
            FORCE_CAST(rogue, priest, ClassSpells::Rogue::SINISTER_STRIKE_RNK_10, SPELL_MISS_NONE);
            TEST_ASSERT(rogue->GetHealth() == rogueHealth);
            //IMPROVE ME, check if the spell has actually procced, because it could have but with 0 burn

            // Burn mana + damage
            {
                uint32 const expectedMageHealth = mage->GetHealth() - ClassSpellsDamage::Priest::FEEDBACK_BURN_RNK_6;
                uint32 const expectedMageMana = mage->GetPower(POWER_MANA) - ClassSpellsDamage::Priest::FEEDBACK_BURN_RNK_6;

                //mage cast ice lance on priest
                SECTION("Mana burn", [&] {
                    TEST_CAST(priest, priest, ClassSpells::Priest::FEEDBACK_RNK_6, SPELL_CAST_OK, TRIGGERED_FULL_MASK);
                    priest->ForceSpellHitResultOverride(SPELL_MISS_NONE); //force spell proc to allow proc to always hit mage. (Until proved otherwise we'll assume it can be resisted)
                    FORCE_CAST(mage, priest, ClassSpells::Mage::ICE_LANCE_RNK_1, SPELL_MISS_NONE, TriggerCastFlags(TRIGGERED_FULL_MASK | TRIGGERED_IGNORE_SPEED));
                    priest->ResetForceSpellHitResultOverride();
                    ASSERT_INFO("initial mana: 200, current mana: %u, expected: %u", mage->GetPower(POWER_MANA), expectedMageMana);
                    TEST_ASSERT(mage->GetPower(POWER_MANA) == expectedMageMana);
                    TEST_ASSERT(mage->GetHealth() == expectedMageHealth);
                });

                //mage cast a second ice lance, but has only 35 (expectedMageMana) mana left. Remaining mana should be burned and damage should only amount to this burned mana
                SECTION("Mana burn 2", [&] {
                    priest->ForceSpellHitResultOverride(SPELL_MISS_NONE); //force spell proc to allow proc to always hit mage. (Until proved otherwise we'll assume it can be resisted)
                    FORCE_CAST(mage, priest, ClassSpells::Mage::ICE_LANCE_RNK_1, SPELL_MISS_NONE, TriggerCastFlags(TRIGGERED_FULL_MASK | TRIGGERED_IGNORE_SPEED));
                    priest->ResetForceSpellHitResultOverride();
                    ASSERT_INFO("initial mana: %u, current mana: %u, expected: 0", expectedMageMana, mage->GetPower(POWER_MANA));
                    TEST_ASSERT(mage->GetPower(POWER_MANA) == 0);
                    TEST_ASSERT(mage->GetHealth() == expectedMageHealth - expectedMageMana);
                });
            }
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<FeedbackTestImpt>();
    }
};

class InnerFireTest : public TestCaseScript
{
public:
    InnerFireTest() : TestCaseScript("spells priest inner_fire") { }

    class InnerFireTestImpt : public TestCase
    {
    public:


        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);
            TestPlayer* rogue = SpawnPlayer(CLASS_ROGUE, RACE_HUMAN);

            priest->DisableRegeneration(true);

            uint32 innerFireBonus = 1580;
            uint32 expectedArmor = priest->GetArmor() + innerFireBonus;

            // Mana cost
            uint32 const expectedInnerFireMana = 375;
            TEST_POWER_COST(priest, ClassSpells::Priest::INNER_FIRE_RNK_7, POWER_MANA, expectedInnerFireMana);

            TEST_CAST(priest, priest, ClassSpells::Priest::INNER_FIRE_RNK_7);

            // Aura
            TEST_AURA_MAX_DURATION(priest, ClassSpells::Priest::INNER_FIRE_RNK_7, 10 * MINUTE * IN_MILLISECONDS);

            // Charges
            TEST_AURA_CHARGE(priest, ClassSpells::Priest::INNER_FIRE_RNK_7, 20);

            // Armor
            TEST_ASSERT(priest->GetArmor() == expectedArmor);

            // Lose charge
            auto AI = rogue->GetTestingPlayerbotAI();
            int hits = 0;
            for (uint32 i = 0; i < 150; i++)
            {
                rogue->AttackerStateUpdate(priest, BASE_ATTACK);
                auto damageToTarget = AI->GetMeleeDamageDoneInfo(priest);
                TEST_ASSERT(damageToTarget->size() == i + 1);
                auto& data = damageToTarget->back();
                if (data.damageInfo.HitOutCome != MELEE_HIT_NORMAL && data.damageInfo.HitOutCome != MELEE_HIT_CRIT)
                    continue;
                hits++;
                Aura* aura = priest->GetAura(ClassSpells::Priest::INNER_FIRE_RNK_7);
                if (hits < 20)
                {
                    TEST_ASSERT(aura != nullptr);
                    int currentCharges = aura->GetCharges();
                    TEST_ASSERT(currentCharges == 20 - hits);
                }
                else
                {
                    TEST_ASSERT(aura == nullptr);
                    TEST_ASSERT(!priest->HasAura(ClassSpells::Priest::INNER_FIRE_RNK_7));
                    break;
                }
            }
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<InnerFireTestImpt>();
    }
};

class LevitateTest : public TestCaseScript
{
public:
    LevitateTest() : TestCaseScript("spells priest levitate") { }

    class LevitateTestImpt : public TestCase
    {
        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);
            TestPlayer* rogue = SpawnPlayer(CLASS_ROGUE, RACE_HUMAN);

            priest->DisableRegeneration(true);

            // Mana cost & reagant
            uint32 const expectedLevitateMana = 100;
            TEST_POWER_COST(priest, ClassSpells::Priest::LEVITATE_RNK_1, POWER_MANA, expectedLevitateMana);
            priest->AddItem(17056, 1); // Reagent Light Feather
            TEST_CAST(priest, priest, ClassSpells::Priest::LEVITATE_RNK_1);
            TEST_ASSERT(priest->GetItemCount(17056, false) == 0);

            // Aura
            TEST_AURA_MAX_DURATION(priest, ClassSpells::Priest::LEVITATE_RNK_1, 2 * MINUTE * IN_MILLISECONDS);

            // Cancel on damage taken
            rogue->Attack(priest, true);
            Wait(500);
            rogue->AttackStop();
            TEST_ASSERT(!priest->HasAura(ClassSpells::Priest::LEVITATE_RNK_1));

            SECTION("WIP", STATUS_WIP, [&] {
                // TODO: over water, slow fall
            });
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<LevitateTestImpt>();
    }
};

class ManaBurnTest : public TestCaseScript
{
public:
    ManaBurnTest() : TestCaseScript("spells priest mana_burn") { }

    class ManaBurnTestImpt : public TestCase
    {
    public:


        void TestManaBurn(TestPlayer* priest, Unit* victim, uint32 expectedMin, uint32 expectedMax)
        {
            auto[sampleSize, maxPredictionError] = _GetApproximationParams(expectedMin, expectedMax);

            EnableCriticals(priest, true);

            uint32 victimMaxHealth = victim->GetMaxHealth();
            uint32 victimMaxMana = victim->GetMaxPower(POWER_MANA);

            uint32 minBurn = std::numeric_limits<uint32>::max();
            uint32 maxBurn = 0;

            // Calculate min-max mana burnt + test if missing health is matching the burnt mana
            for (uint32 i = 0; i < sampleSize; i++)
            {
                // Reset
                victim->SetFullHealth();
                victim->SetPower(POWER_MANA, victim->GetMaxPower(POWER_MANA));

                FORCE_CAST(priest, victim, ClassSpells::Priest::MANA_BURN_RNK_7, SPELL_MISS_NONE, TRIGGERED_FULL_MASK);

                uint32 manaBurnt = victimMaxMana - victim->GetPower(POWER_MANA);
                minBurn = std::min(minBurn, manaBurnt);
                maxBurn = std::max(maxBurn, manaBurnt);

                uint32 expectedHealth = victimMaxHealth - manaBurnt / 2;
                maxPredictionError += 1; //add 1 to allow for rounding error here
                ASSERT_INFO("Health: %u, expected: %u", victim->GetHealth(), expectedHealth);
                TEST_ASSERT(Between(victim->GetHealth(), expectedHealth - maxPredictionError, expectedHealth + maxPredictionError));
                HandleThreadPause();
            }

            // Check min-max values
            ASSERT_INFO("minBurn: %u, expected: %u", minBurn, expectedMin);
            TEST_ASSERT(Between(minBurn, expectedMin - maxPredictionError, expectedMin + maxPredictionError));
            ASSERT_INFO("maxBurn: %u, expected: %u", maxBurn, expectedMax);
            TEST_ASSERT(Between(maxBurn, expectedMax - maxPredictionError, expectedMax + maxPredictionError));
        }

        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);
            TestPlayer* mage = SpawnPlayer(CLASS_MAGE, RACE_HUMAN);
            TestPlayer* rogue = SpawnPlayer(CLASS_ROGUE, RACE_HUMAN);

            EQUIP_NEW_ITEM(priest, 34336); // Sunflare - 292 SP
            priest->DisableRegeneration(true);
            mage->DisableRegeneration(true);
            Wait(1500);

            // Fail
            TEST_CAST(priest, rogue, ClassSpells::Priest::MANA_BURN_RNK_7, SPELL_FAILED_BAD_TARGETS);

            // Mana cost
            uint32 const expectedManaBurnMana = 355;
            TEST_POWER_COST(priest, ClassSpells::Priest::MANA_BURN_RNK_7, POWER_MANA, expectedManaBurnMana);

            //there is no spell power coef on mana burn
            TestManaBurn(priest, mage, ClassSpellsDamage::Priest::MANA_BURN_RNK_7_MIN, ClassSpellsDamage::Priest::MANA_BURN_RNK_7_MAX);
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<ManaBurnTestImpt>();
    }
};

// "spells priest mass_dispel"
/*"Dispels magic in a 15 yard radius, removing 1 harmful spell from each friendly target and 1 beneficial spell from each enemy target.
Affects a maximum of 10 friendly targets and 10 enemy targets. This dispel is potent enough to remove Magic effects 
that are normally undispellable."*/
class MassDispelTest : public TestCase
{
    //summon shaman with some totems
    TestPlayer* SpawnShaman(Races race)
    {
        TestPlayer* shaman = SpawnPlayer(CLASS_SHAMAN, race);
        EQUIP_NEW_ITEM(shaman, 28523); // Totem
        TEST_CAST(shaman, shaman, ClassSpells::Shaman::MANA_SPRING_TOTEM_RNK_5, SPELL_CAST_OK, TRIGGERED_FULL_MASK);
        TEST_CAST(shaman, shaman, ClassSpells::Shaman::GRACE_OF_AIR_TOTEM_RNK_3, SPELL_CAST_OK, TRIGGERED_FULL_MASK);
        TEST_CAST(shaman, shaman, ClassSpells::Shaman::STRENGTH_OF_EARTH_TOTEM_RNK_6, SPELL_CAST_OK, TRIGGERED_FULL_MASK);
        TEST_CAST(shaman, shaman, ClassSpells::Shaman::FROST_RESISTANCE_TOTEM_RNK_4, SPELL_CAST_OK, TRIGGERED_FULL_MASK);
        return shaman;
    }

    //summon hunter with a pet
    TestPlayer* SpawnHunter(Races race)
    {
        TestPlayer* hunter = SpawnPlayer(CLASS_HUNTER, race);
        hunter->SummonPet(20673, hunter->GetPositionX(), hunter->GetPositionY(), hunter->GetPositionZ(), 0.0f, HUNTER_PET, 0); // Wind Serpent
        Pet* pet = hunter->GetPet();
        TEST_ASSERT(pet != nullptr);
        return hunter;
    }

    void Test() override
    {
        uint32 const MAX_DISPEL_TARGETS = 10;
        float const dist = 35.0f;
        uint32 const GAS_NOVA = 45855; //100 yard range

        // Dispel buffs from enemies and debugg from allies
        // Also check counts (+ that totems are excluded)
        SECTION("Base dispel", [&] {
            // Setup: spawn 22 players, for each side 11 + 1 hunter pet + some totems
            // Priest
            TestPlayer* priestA = SpawnPlayer(CLASS_PRIEST, RACE_HUMAN);
            TestPlayer* priestH = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);

            //10 others alliance
            SpawnRandomPlayer(RACE_HUMAN);
            SpawnRandomPlayer(RACE_HUMAN);
            SpawnRandomPlayer(RACE_HUMAN);
            SpawnShaman(RACE_DRAENEI);
            SpawnHunter(RACE_DWARF);
            SpawnRandomPlayer(RACE_HUMAN);
            SpawnRandomPlayer(RACE_NIGHTELF);
            SpawnRandomPlayer(RACE_NIGHTELF);
            SpawnRandomPlayer(RACE_NIGHTELF);
            SpawnRandomPlayer(RACE_NIGHTELF);

            //10 others horde
            SpawnRandomPlayer(RACE_BLOODELF);
            SpawnRandomPlayer(RACE_TAUREN);
            SpawnRandomPlayer(RACE_TAUREN);
            SpawnRandomPlayer(RACE_BLOODELF);
            SpawnRandomPlayer(RACE_BLOODELF);
            SpawnRandomPlayer(RACE_BLOODELF);
            SpawnShaman(RACE_ORC);
            SpawnHunter(RACE_ORC);
            SpawnRandomPlayer(RACE_BLOODELF);
            SpawnRandomPlayer(RACE_BLOODELF);

            uint32 const TOTAL_HORDE_UNITS = 12;
            uint32 const TOTAL_ALLIANCE_UNITS = 12;

            // Get all the units
            std::vector<WorldObject*> targets;
            Trinity::AllWorldObjectsInRange u_check(priestA, dist);
            Trinity::WorldObjectListSearcher<Trinity::AllWorldObjectsInRange> searcher(priestA, targets, u_check);
            Cell::VisitAllObjects(priestA, searcher, dist);

            // Group raid the players
            for (WorldObject* object : targets)
            {
                if (Player* player = object->ToPlayer())
                {
                    if (player == priestA || player == priestH)
                        continue;

                    if (player->GetTeam() == Team::ALLIANCE)
                        GroupPlayer(priestA, player);
                    else
                        GroupPlayer(priestH, player);
                }
            }

            WaitNextUpdate();

            // Buff and debuff everyone + Check if everybody has the gas nova (except Totems)
            for (WorldObject* object : targets)
            {
                if (Unit* unit = object->ToUnit())
                {
                    if (unit->IsTotem())
                        continue;

                    unit->AddAura(GAS_NOVA, unit);
                    TEST_HAS_AURA(unit, GAS_NOVA)
                        // Buff priest on horde players & pets
                        priestH->AddAura(ClassSpells::Priest::PRAYER_OF_FORTITUDE_RNK_3, unit);
                    TEST_HAS_AURA(unit, ClassSpells::Priest::PRAYER_OF_FORTITUDE_RNK_3);
                    unit->SetFullHealth();
                }
            }

            // Mass dispell
            // Alliance: no more gas nova except for 2 targets
            // Horde: gas nova but no more priest buff except for 2 targets
            uint32 targetsWithoutPrayerOfFortitude = 0;
            uint32 targetsWithPrayerOfFortitude = 0;
            uint32 targetsWithoutGasNova = 0;
            uint32 targetsWithGasNova = 0;
            //centered on self
            FORCE_CAST(priestA, priestA, ClassSpells::Priest::MASS_DISPEL_RNK_1, SPELL_MISS_NONE, TRIGGERED_CAST_DIRECTLY);

            auto countNova = [&](Unit* unit) {
                if (unit->HasAura(GAS_NOVA))
                    targetsWithGasNova++;
                else
                    targetsWithoutGasNova++;
            };

            auto countPrayer = [&](Unit* unit) {
                if (unit->HasAura(ClassSpells::Priest::PRAYER_OF_FORTITUDE_RNK_3))
                    targetsWithPrayerOfFortitude++;
                else
                    targetsWithoutPrayerOfFortitude++;
            };

            for (WorldObject* object : targets)
            {
                if (Player* player = object->ToPlayer())
                {
                    if (player->GetTeam() == Team::ALLIANCE)
                    {
                        countNova(player);
                        if (Pet* pet = player->GetPet())
                            countNova(pet);
                    }
                    else
                    {
                        ASSERT_INFO("At least 1 horde player/pet did not have gas nova after mass dispel");
                        TEST_HAS_AURA(player, GAS_NOVA);
                        countPrayer(player);
                        if (Pet* pet = player->GetPet())
                            countPrayer(pet);
                    }
                    player->RemoveAurasDueToSpell(GAS_NOVA);
                    if (Pet* pet = player->GetPet())
                        pet->RemoveAurasDueToSpell(GAS_NOVA);
                }
            }

            //check results
            TEST_ASSERT((targetsWithGasNova + targetsWithoutGasNova) == TOTAL_ALLIANCE_UNITS);
            TEST_ASSERT((targetsWithPrayerOfFortitude + targetsWithoutPrayerOfFortitude) == TOTAL_HORDE_UNITS);
            uint32 const expectedWithNova = TOTAL_ALLIANCE_UNITS - MAX_DISPEL_TARGETS;
            ASSERT_INFO("%u alliance units with nova instead of %u", targetsWithGasNova, expectedWithNova);
            TEST_ASSERT(targetsWithGasNova == expectedWithNova);
            uint32 const expectedWithPrayer = TOTAL_HORDE_UNITS - MAX_DISPEL_TARGETS;
            ASSERT_INFO("%u horde units with prayer instead of %u", targetsWithPrayerOfFortitude, expectedWithPrayer);
            TEST_ASSERT(targetsWithPrayerOfFortitude == expectedWithPrayer);
        });

        // Remove Ice Block, Divine Shield, Banish
        SECTION("Special dispel", [&]() {
            _location.MoveInFront(_location, 50.0f);  //advance a bit so we're not bothered by previously spawned units

            TestPlayer* mageH = SpawnPlayer(CLASS_MAGE, RACE_BLOODELF);
            TestPlayer* paladinH = SpawnPlayer(CLASS_PALADIN, RACE_BLOODELF);
            TestPlayer* mageA = SpawnPlayer(CLASS_MAGE, RACE_HUMAN);
            TestPlayer* warlockH = SpawnPlayer(CLASS_WARLOCK, RACE_BLOODELF);
            TestPlayer* priestA = SpawnPlayer(CLASS_PRIEST, RACE_HUMAN);

            TEST_CAST(mageH, mageH, ClassSpells::Mage::ICE_BLOCK_RNK_1);
            TEST_CAST(paladinH, paladinH, ClassSpells::Paladin::DIVINE_SHIELD_RNK_2);
            TEST_CAST(mageA, mageA, ClassSpells::Mage::SUMMON_WATER_ELEMENTAL_RNK_1);
            Guardian* elem = mageA->GetGuardianPet();
            TEST_ASSERT(elem != nullptr);
            TEST_CAST(warlockH, elem, ClassSpells::Warlock::BANISH_RNK_2, SPELL_CAST_OK, TRIGGERED_CAST_DIRECTLY);

            TEST_HAS_AURA(mageH, ClassSpells::Mage::ICE_BLOCK_RNK_1);
            TEST_HAS_AURA(paladinH, ClassSpells::Paladin::DIVINE_SHIELD_RNK_2);
            TEST_HAS_AURA(elem, ClassSpells::Warlock::BANISH_RNK_2);

            FORCE_CAST(priestA, mageH, ClassSpells::Priest::MASS_DISPEL_RNK_1, SPELL_MISS_NONE, TRIGGERED_CAST_DIRECTLY);

            TEST_HAS_NOT_AURA(mageH, ClassSpells::Mage::ICE_BLOCK_RNK_1);
            TEST_HAS_NOT_AURA(paladinH, ClassSpells::Paladin::DIVINE_SHIELD_RNK_2);
            TEST_HAS_NOT_AURA(elem, ClassSpells::Warlock::BANISH_RNK_2);

        });

        // What we're testing:
        // BC WoWWiki: "The spell targets those closest to the casting priest first and then moves outward."
        // but was changed to "The spell targets those closest to the center of the green targeting circle first and then moves outward." on
        // the wiki during LK beta. I'm assuming this second text is the right one and the spell did not actually change.
        SECTION("Outward target selection", [&] {
            _location.MoveInFront(_location, 50.0f); //advance a bit so we're not bothered by previously spawned units

            uint32 const MAX_AFFECTED = 10;
            uint32 const SPAWN_PLAYER_COUNT = 20;
            uint32 const dotSpellId = ClassSpells::Priest::SHADOW_WORD_PAIN_RNK_1;
            std::vector<TestPlayer*> players(SPAWN_PLAYER_COUNT);
            Position spawnPos(_location);

            //spawn SPAWN_PLAYER_COUNT players a bit further each time
            for (uint32 i = 0; i < SPAWN_PLAYER_COUNT; i++)
            {
                players[i] = SpawnPlayer(CLASS_PRIEST, RACE_HUMAN, 70, spawnPos);
                //free dot for everyone
                players[i]->AddAura(dotSpellId, players[i]);
                spawnPos.MoveInFront(spawnPos, 0.5f);
            }
            //so further player will be at SPAWN_PLAYER_COUNT * 0.5f = 10.0f
            //(spell max range is 15 so we're okay)

            //cast mass dispel centered on first player
            FORCE_CAST(players[0], players[0], ClassSpells::Priest::MASS_DISPEL_RNK_1, SPELL_MISS_NONE, TRIGGERED_CAST_DIRECTLY);
            //10 closest player should have lost the spell
            for (uint32 i = 0; i < MAX_AFFECTED; i++)
            {
                ASSERT_INFO("Player with pos %u", i);
                TEST_HAS_NOT_AURA(players[i], dotSpellId);
            }
            //10 furthest should still have it
            for (uint32 i = MAX_AFFECTED; i < SPAWN_PLAYER_COUNT; i++)
            {
                ASSERT_INFO("Player with pos %u", i);
                TEST_HAS_AURA(players[i], dotSpellId);
            }
        });
    }
};

class PowerWordFortitudeTest : public TestCaseScript
{
public:
    PowerWordFortitudeTest() : TestCaseScript("spells priest power_word_fortitude") { }

    class PowerWordFortitudeTestImpt : public TestCase
    {
    public:


        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_ORC);

            uint32 powerWordFortitudeBonus = 79;
            uint32 expectedHealth = warrior->GetMaxHealth() + powerWordFortitudeBonus * 10;

            // Mana cost
            uint32 const expectedPowerWordFortitudeMana = 700;
            TEST_POWER_COST(priest, ClassSpells::Priest::POWER_WORD_FORTITUDE_RNK_7, POWER_MANA, expectedPowerWordFortitudeMana);

            TEST_CAST(priest, warrior, ClassSpells::Priest::POWER_WORD_FORTITUDE_RNK_7);

            // Health
            ASSERT_INFO("Health: %u, expected: %u", warrior->GetHealth(), expectedHealth);
            TEST_ASSERT(warrior->GetMaxHealth() == expectedHealth);

            // Aura
            TEST_AURA_MAX_DURATION(warrior, ClassSpells::Priest::POWER_WORD_FORTITUDE_RNK_7, 30 * MINUTE * IN_MILLISECONDS);
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<PowerWordFortitudeTestImpt>();
    }
};

class PowerWordShieldTest : public TestCaseScript
{
public:
    PowerWordShieldTest() : TestCaseScript("spells priest power_word_shield") { }

    class PowerWordShieldTestImpt : public TestCase
    {
    public:


        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);
            TestPlayer* rogue = SpawnPlayer(CLASS_ROGUE, RACE_HUMAN);
            uint32 const WEAKENED_SOUL = 6788;

            EQUIP_NEW_ITEM(priest, 34335); // Hammer of Sanctification -- 550 BH

            int32 expectedAbsorb = ClassSpellsDamage::Priest::POWER_WORD_SHIELD_RNK_12 + ClassSpellsCoeff::Priest::POWER_WORD_SHIELD * 550;

            // Mana cost
            uint32 const expectedPowerWordShieldMana = 600;
            TEST_POWER_COST(priest, ClassSpells::Priest::POWER_WORD_SHIELD_RNK_12, POWER_MANA, expectedPowerWordShieldMana);

            // Aura
            TEST_CAST(priest, priest, ClassSpells::Priest::POWER_WORD_SHIELD_RNK_12);
            TEST_AURA_MAX_DURATION(priest, ClassSpells::Priest::POWER_WORD_SHIELD_RNK_12, Seconds(30));
            TEST_AURA_MAX_DURATION(priest, 6788, Seconds(15)); // Weakened Aura
            priest->RemoveAura(WEAKENED_SOUL);

            // Cooldown
            TEST_COOLDOWN(priest, priest, ClassSpells::Priest::POWER_WORD_SHIELD_RNK_12, Seconds(4));
            priest->RemoveAura(WEAKENED_SOUL);

            // Absorb
            {
                TEST_CAST(priest, priest, ClassSpells::Priest::POWER_WORD_SHIELD_RNK_12);

                // Step 1 - test theoritical amount
                Aura const* shield = priest->GetAura(ClassSpells::Priest::POWER_WORD_SHIELD_RNK_12);
                TEST_ASSERT(shield != nullptr);
                AuraEffect const* absorbEffect = shield->GetEffect(EFFECT_0);
                int32 const absorbAmount = absorbEffect->GetAmount();
                ASSERT_INFO("absorbAmount %u != expectedAbsorb %u", absorbAmount, expectedAbsorb);
                TEST_ASSERT(absorbAmount == expectedAbsorb);

                // Step 2 - Test with real damage
                priest->SetFullHealth();
                auto AI = rogue->GetTestingPlayerbotAI();
                int32 totalDamage = 0;
                for (uint32 i = 0; i < 150; i++)
                {
                    rogue->AttackerStateUpdate(priest, BASE_ATTACK);
                    auto damageToTarget = AI->GetMeleeDamageDoneInfo(priest);

                    TEST_ASSERT(damageToTarget->size() == i + 1);
                    auto& data = damageToTarget->back();

                    if (data.damageInfo.HitOutCome != MELEE_HIT_NORMAL && data.damageInfo.HitOutCome != MELEE_HIT_CRIT)
                        continue;

                    for (uint8 j = 0; j < MAX_ITEM_PROTO_DAMAGES; j++)
                    {
                        totalDamage += data.damageInfo.Damages[j].Damage;
                        totalDamage += data.damageInfo.Damages[j].Resist;
                        totalDamage += data.damageInfo.Damages[j].Absorb;
                    }
                    totalDamage += data.damageInfo.Blocked;

                    if (totalDamage < expectedAbsorb)
                    {
                        TEST_HAS_AURA(priest, ClassSpells::Priest::POWER_WORD_SHIELD_RNK_12); //if this fails, shield did not absorb enough 
                        continue;
                    }

                    TEST_HAS_NOT_AURA(priest, ClassSpells::Priest::POWER_WORD_SHIELD_RNK_12);
                    uint32 expectedHealth = priest->GetMaxHealth() - (totalDamage - expectedAbsorb);
                    TEST_ASSERT(priest->GetHealth() == expectedHealth);
                    break;
                }
                priest->RemoveAura(WEAKENED_SOUL);
            }
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<PowerWordShieldTestImpt>();
    }
};

class PrayerOfFortitudeTest : public TestCaseScript
{
public:
    PrayerOfFortitudeTest() : TestCaseScript("spells priest prayer_of_fortitude") { }

    class PrayerOfFortitudeTestImpt : public TestCase
    {
    public:


        void TestPrayerOfFortitude(TestPlayer* priest, Unit* warrior, uint32 spellId, uint32 reagentId, uint32 manaCost, uint32 staminaBonus, uint32 priestStartHealth, uint32 warriorStartHealth)
        {
            uint32 expectedPriestHealth = priestStartHealth + staminaBonus * 10;
            uint32 expectedWarriorHealth = warriorStartHealth + staminaBonus * 10;

            // Mana cost & reagents
            TEST_POWER_COST(priest, spellId, POWER_MANA, manaCost);
            priest->AddItem(reagentId, 1); // Reagent
            TEST_CAST(priest, warrior, spellId, SPELL_CAST_OK, TRIGGERED_IGNORE_GCD);
            TEST_ASSERT(priest->GetItemCount(reagentId, false) == 0);

            // Aura
            TEST_AURA_MAX_DURATION(warrior, spellId, 1 * HOUR * IN_MILLISECONDS);
            TEST_AURA_MAX_DURATION(priest, spellId, 1 * HOUR * IN_MILLISECONDS);

            // Health
            TEST_ASSERT(priest->GetMaxHealth() == expectedPriestHealth);
            TEST_ASSERT(warrior->GetMaxHealth() == expectedWarriorHealth);
        }

        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_ORC);

            GroupPlayer(priest, warrior);

            uint32 const startPriestHealth = priest->GetMaxHealth();
            uint32 const startWarriorHealth = warrior->GetMaxHealth();

            uint32 const HOLY_CANDLE = 17028;
            uint32 const SACRED_CANDLE = 17029;

            TestPrayerOfFortitude(priest, warrior, ClassSpells::Priest::PRAYER_OF_FORTITUDE_RNK_1, HOLY_CANDLE, 1200, 43, startPriestHealth, startWarriorHealth);
            TestPrayerOfFortitude(priest, warrior, ClassSpells::Priest::PRAYER_OF_FORTITUDE_RNK_2, SACRED_CANDLE, 1500, 54, startPriestHealth, startWarriorHealth);
            TestPrayerOfFortitude(priest, warrior, ClassSpells::Priest::PRAYER_OF_FORTITUDE_RNK_3, SACRED_CANDLE, 1800, 79, startPriestHealth, startWarriorHealth);
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<PrayerOfFortitudeTestImpt>();
    }
};

class ShackleUndeadTest : public TestCaseScript
{
public:
    ShackleUndeadTest() : TestCaseScript("spells priest shackle_undead") { }

    class ShackleUndeadTestImpt : public TestCase
    {
    public:


        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);
            Creature* creature0 = SpawnCreature(6, true); // "Kobold Vermin", humanoid

            // Should fail
            TEST_CAST(priest, creature0, ClassSpells::Priest::SHACKLE_UNDEAD_RNK_3, SPELL_FAILED_BAD_TARGETS);

            Position spawn(_location);
            spawn.MoveInFront(spawn, 20.0f);
            Creature* creature1 = SpawnCreatureWithPosition(spawn, 16525); // Undead
            Creature* creature2 = SpawnCreatureWithPosition(spawn, 16525); // Undead

            WaitNextUpdate();

            uint32 const expectedShackleUndeadMana = 150;
            TEST_POWER_COST(priest, ClassSpells::Priest::SHACKLE_UNDEAD_RNK_3, POWER_MANA, expectedShackleUndeadMana);

            FORCE_CAST(priest, creature1, ClassSpells::Priest::SHACKLE_UNDEAD_RNK_3, SPELL_MISS_NONE, TRIGGERED_CAST_DIRECTLY);
            TEST_AURA_MAX_DURATION(creature1, ClassSpells::Priest::SHACKLE_UNDEAD_RNK_3, Seconds(50));
            //creature 1 should have shackle on him at this point

            FORCE_CAST(priest, creature2, ClassSpells::Priest::SHACKLE_UNDEAD_RNK_3, SPELL_MISS_NONE, TriggerCastFlags(TRIGGERED_CAST_DIRECTLY | TRIGGERED_IGNORE_POWER_AND_REAGENT_COST));
            TEST_HAS_AURA(creature2, ClassSpells::Priest::SHACKLE_UNDEAD_RNK_3);
            //shackle can only be used on one target at a time, creature 1 should be freed
            TEST_HAS_NOT_AURA(creature1, ClassSpells::Priest::SHACKLE_UNDEAD_RNK_3);

            //creature should be rooted (actually, stunned with this spell)
            TEST_ASSERT(creature2->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_STUNNED));
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<ShackleUndeadTestImpt>();
    }
};

class StarshardsTest : public TestCaseScript
{
public:
    StarshardsTest() : TestCaseScript("spells priest starshards") { }

    class StarshardsTestImpt : public TestCase
    {
    public:


        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_NIGHTELF);
            Creature* dummy = SpawnCreature();

            EQUIP_NEW_ITEM(priest, 34336); // Sunflare -- 292 SP

            // Mana cost
            uint32 const expectedStarshardsMana = 0;
            TEST_POWER_COST(priest, ClassSpells::Priest::STARSHARDS_RNK_8, POWER_MANA, expectedStarshardsMana);

            FORCE_CAST(priest, dummy, ClassSpells::Priest::STARSHARDS_RNK_8);

            // Cooldown 
            TEST_COOLDOWN(priest, dummy, ClassSpells::Priest::STARSHARDS_RNK_8, Seconds(30));

            /*
            Damage (test fails here)
            About spell coeff:
                - Base calculation says 100% (see below)
                - DrDamage says 100%
                - WoW Wiki lists 83.5% - http://wowwiki.wikia.com/wiki/Spell_power_coefficient?oldid=1549180
                - Dwarf priest says 100% - https://dwarfpriest.wordpress.com/2007/10/16/priest-racial-spell-changes-in-23/
                -> We choose base calculation (100%)
            */
            float const starshardsDuration = 15.0f;
            float const starshardsCoeff = starshardsDuration / 15.0f;
            uint32 const spellBonus = 292 * starshardsCoeff;
            uint32 const starshardsTotal = ClassSpellsDamage::Priest::STARSHARDS_RNK_8_TOTAL + spellBonus;
            TEST_DOT_DAMAGE(priest, dummy, ClassSpells::Priest::STARSHARDS_RNK_8, starshardsTotal, false);
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<StarshardsTestImpt>();
    }
};

class SymbolOfHopeTest : public TestCaseScript
{
public:
    SymbolOfHopeTest() : TestCaseScript("spells priest symbol_of_hope") { }

    class SymbolOfHopeTestImpt : public TestCase
    {
    public:


        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_DRAENEI);
            TestPlayer* mage = SpawnPlayer(CLASS_MAGE, RACE_HUMAN);

            GroupPlayer(priest, mage);

            float const symbolOfHopeBonus = (5.0f * (priest->GetLevel() - 10) + 33) / 5.0f;

            float const expectedPriestStartMPS = priest->GetFloatValue(PLAYER_FIELD_MOD_MANA_REGEN) + symbolOfHopeBonus;
            float const expectedPriestStartMPSFSR = priest->GetFloatValue(PLAYER_FIELD_MOD_MANA_REGEN_INTERRUPT) + symbolOfHopeBonus;

            float const expectedMageStartMPS = mage->GetFloatValue(PLAYER_FIELD_MOD_MANA_REGEN) + symbolOfHopeBonus;
            float const expectedMageStartMPSFSR = mage->GetFloatValue(PLAYER_FIELD_MOD_MANA_REGEN_INTERRUPT) + symbolOfHopeBonus;

            uint32 const expectedSymbolOfHopeManaCost = 15;
            priest->SetPower(POWER_MANA, expectedSymbolOfHopeManaCost);

            TEST_CAST(priest, priest, ClassSpells::Priest::SYMBOL_OF_HOPE_RNK_1);
            // Mana cost
            TEST_ASSERT(priest->GetPower(POWER_MANA) == 0);
            // Aura duration
            TEST_AURA_MAX_DURATION(priest, ClassSpells::Priest::SYMBOL_OF_HOPE_RNK_1, Seconds(15));
            TEST_AURA_MAX_DURATION(mage, ClassSpells::Priest::SYMBOL_OF_HOPE_RNK_1, Seconds(15));
            // Cooldown
            TEST_HAS_COOLDOWN(priest, ClassSpells::Priest::SYMBOL_OF_HOPE_RNK_1, Minutes(5));
            Wait(1000);
            // MPS
            TEST_ASSERT(priest->GetFloatValue(PLAYER_FIELD_MOD_MANA_REGEN) == expectedPriestStartMPS);
            TEST_ASSERT(priest->GetFloatValue(PLAYER_FIELD_MOD_MANA_REGEN_INTERRUPT) == expectedPriestStartMPSFSR);
            TEST_ASSERT(mage->GetFloatValue(PLAYER_FIELD_MOD_MANA_REGEN) == expectedMageStartMPS);
            TEST_ASSERT(mage->GetFloatValue(PLAYER_FIELD_MOD_MANA_REGEN_INTERRUPT) == expectedMageStartMPSFSR);

            
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<SymbolOfHopeTestImpt>();
    }
};

class AbolishDiseaseTest : public TestCaseScript
{
public:
    AbolishDiseaseTest() : TestCaseScript("spells priest abolish_disease") { }

    //"Attempts to cure 1 disease effect on the target, and 1 more disease effect every 5 seconds thereafter for 20 sec."
    class AbolishDiseaseTestImpt : public TestCase
    {
        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);
            _location.MoveInFront(_location, 10.f);
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN);
            warrior->SetMaxHealth(1000000);
            warrior->SetFullHealth();

            uint32 const WEAKENING_DISEASE = 18633;
            uint32 const DISEASED_SLIME    = 6907;
            uint32 const FEVERED_DISEASE   = 32903;
            uint32 const DISEASE_SHOT      = 11397;
            uint32 const DEVOURING_PLAGUE  = ClassSpells::Priest::DEVOURING_PLAGUE_RNK_7;

            // Apply
            uint32 const baseDiseaseCount = 5;
            warrior->AddAura(WEAKENING_DISEASE, warrior);
            warrior->AddAura(DISEASED_SLIME, warrior);
            warrior->AddAura(FEVERED_DISEASE, warrior);
            warrior->AddAura(DISEASE_SHOT, warrior);
            warrior->AddAura(DEVOURING_PLAGUE, warrior);

            // Mana cost
            uint32 const expectedAbolishDiseaseMana = 314;
            TEST_POWER_COST(priest, ClassSpells::Priest::ABOLISH_DISEASE_RNK_1, POWER_MANA, expectedAbolishDiseaseMana);

            TEST_CAST(priest, warrior, ClassSpells::Priest::ABOLISH_DISEASE_RNK_1);

            // Aura duration
            TEST_AURA_MAX_DURATION(warrior, ClassSpells::Priest::ABOLISH_DISEASE_RNK_1, Seconds(20));

            SECTION("Dispel", [&] {
                auto CountDisease = [&]() {
                    TEST_ASSERT(warrior->IsAlive());
                    return uint32(warrior->HasAura(WEAKENING_DISEASE))
                         + uint32(warrior->HasAura(DISEASED_SLIME))
                         + uint32(warrior->HasAura(FEVERED_DISEASE))
                         + uint32(warrior->HasAura(DISEASE_SHOT))
                         + uint32(warrior->HasAura(DEVOURING_PLAGUE));
                };
                ASSERT_INFO("One disease should already be dispelled, but found %u disease", CountDisease());
                TEST_ASSERT(CountDisease() == baseDiseaseCount - 1);

                Wait(500);
                //check each dispel tick. No dispel resist chance are present here.
                auto tickTime = 5s;
                for (uint32 i = 0; i < baseDiseaseCount - 1; i++)
                {
                    Wait(tickTime); 
                    uint32 expectedDiseasesCount = baseDiseaseCount - i - 2; //from 3 to 0
                    uint32 actualDiseasesCount = CountDisease();
                    ASSERT_INFO("Target has %u diseases instead of %u", actualDiseasesCount, expectedDiseasesCount);
                    TEST_ASSERT(actualDiseasesCount == expectedDiseasesCount);
                }
            });
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<AbolishDiseaseTestImpt>();
    }
};

class BindingHealTest : public TestCaseScript
{
public:
    BindingHealTest() : TestCaseScript("spells priest binding_heal") { }

    class BindingHealTestImpt : public TestCase
    {
    public:


        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);
            TestPlayer* rogue  = SpawnPlayer(CLASS_ROGUE,  RACE_BLOODELF);

            EQUIP_NEW_ITEM(priest, 34335); // Hammer of Sanctification -- 550 BH

            // Cast & Mana cost
            uint32 const expectedBindingHealMana = 705;
            TEST_POWER_COST(priest, ClassSpells::Priest::BINDING_HEAL_RNK_1, POWER_MANA, expectedBindingHealMana);
            
            // Test Healing value
            float const bindingHealCastTime = 1.5f;
            float const bindingHealCoeff = bindingHealCastTime / 3.5f;
            uint32 const bonusHeal = 550 * bindingHealCoeff;
            uint32 const bindingHealMin = ClassSpellsDamage::Priest::BINDING_HEAL_RNK_1_MIN + bonusHeal;
            uint32 const bindingHealMax = ClassSpellsDamage::Priest::BINDING_HEAL_RNK_1_MAX + bonusHeal;
            TEST_DIRECT_HEAL(priest, rogue, ClassSpells::Priest::BINDING_HEAL_RNK_1, bindingHealMin, bindingHealMax, false);
            TEST_DIRECT_HEAL(priest, rogue, ClassSpells::Priest::BINDING_HEAL_RNK_1, bindingHealMin * 1.5f, bindingHealMax * 1.5f, true);

            // Commented out, no proof this should be the case!
            // Test if priest received the same amount of heal
            // Use data from the last TEST_DIRECT_HEAL, min and max heal should match for priest and rogue
            /*
            uint32 minHealToRogue;
            uint32 maxHealToRogue;
            uint32 minHealToPriest;
            uint32 maxhealToPriest;
            auto [minHealToRogue, maxHealToRogue] = GetHealingPerSpellsTo(priest, rogue, ClassSpells::Priest::BINDING_HEAL_RNK_1, {});
            auto [minHealToPriest, maxhealToPriest] = GetHealingPerSpellsTo(priest, priest, ClassSpells::Priest::BINDING_HEAL_RNK_1, {});

            ASSERT_INFO("min heal %u - %u", minHealToRogue, minHealToPriest);
            TEST_ASSERT(minHealToRogue == minHealToPriest);
            ASSERT_INFO("max heal %u - %u", maxHealToRogue, maxhealToPriest);
            TEST_ASSERT(maxHealToRogue == maxhealToPriest);
            */
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<BindingHealTestImpt>();
    }
};

class ChastiseTest : public TestCaseScript
{
public:
    ChastiseTest() : TestCaseScript("spells priest chastise") { }

    class ChastiseTestImpt : public TestCase
    {
    public:


        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_DWARF);

            Creature* humanoid = SpawnCreature(724, true);  // Burly Rockjaw Trogg
            humanoid->SetMaxHealth(std::numeric_limits<uint32>::max()); //enough health to avoid dying
            humanoid->SetFullHealth();
            Creature* beast = SpawnCreature(705, true);     // Ragged Young Wolf

            // Only cast on humanoid
            TEST_CAST(priest, beast, ClassSpells::Priest::CHASTISE_RNK_1, SPELL_FAILED_BAD_TARGETS);
            beast->DespawnOrUnsummon();
            beast = nullptr;

            // Mana cost
            uint32 const expectedChastiseMana = 50;
            TEST_POWER_COST(priest, ClassSpells::Priest::CHASTISE_RNK_1, POWER_MANA, expectedChastiseMana);

            humanoid->ClearDiminishings();
            TEST_CAST(priest, humanoid, ClassSpells::Priest::CHASTISE_RNK_1, SPELL_CAST_OK, TRIGGERED_IGNORE_POWER_AND_REAGENT_COST);

            // Aura
            TEST_HAS_COOLDOWN(priest, ClassSpells::Priest::CHASTISE_RNK_1, Seconds(30));
            TEST_AURA_MAX_DURATION(humanoid, ClassSpells::Priest::CHASTISE_RNK_1, Seconds(2));
            TEST_ASSERT(!humanoid->CanFreeMove());
            
            // Damage -- Bug Here, spell coeff too big
            // Note: got a feeling that it crits way too much
            EQUIP_NEW_ITEM(priest, 34336); // Sunflare -- 292 SP
            // DrDamage says 0.142 which is the calculation with castTime = 0.5
            float const chastiseCastTime = 0.5f;
            float const chastiseCoeff = chastiseCastTime / 3.5f;
            uint32 const bonusSpell = 292 * chastiseCoeff;
            uint32 const chastiseMin = ClassSpellsDamage::Priest::CHASTISE_RNK_6_MIN + bonusSpell;
            uint32 const chastiseMax = ClassSpellsDamage::Priest::CHASTISE_RNK_6_MAX + bonusSpell;
            TEST_DIRECT_SPELL_DAMAGE(priest, humanoid, ClassSpells::Priest::CHASTISE_RNK_6, chastiseMin, chastiseMax, false);
            TEST_DIRECT_SPELL_DAMAGE(priest, humanoid, ClassSpells::Priest::CHASTISE_RNK_6, chastiseMin * 1.5f, chastiseMax * 1.5f, true);
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<ChastiseTestImpt>();
    }
};

class CureDiseaseTest : public TestCaseScript
{
public:
    CureDiseaseTest() : TestCaseScript("spells priest cure_disease") { }

    class CureDiseaseTestImpt : public TestCase
    {
    public:


        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN);

            // setup
            priest->DisableRegeneration(true);

            uint32 const WEAKENING_DISEASE = 18633;
            uint32 const SPORE_DISEASE = 31423;
            /*uint32 const FEVERED_DISEASE = 34363;
            uint32 const DISEASE_BUFFET = 46481;
            uint32 const VOLATILE_DISEASE = 46483;*/

            // Apply 2 diseases
            warrior->AddAura(WEAKENING_DISEASE, warrior);
            warrior->AddAura(SPORE_DISEASE, warrior);
            WaitNextUpdate();

            // mana cost
            uint32 const expectedCureDiseaseMana = 314;
            TEST_POWER_COST(priest, ClassSpells::Priest::CURE_DISEASE_RNK_1, POWER_MANA, expectedCureDiseaseMana);

            TEST_CAST(priest, warrior, ClassSpells::Priest::CURE_DISEASE_RNK_1);

            // should cure 1 disease only
            uint8 diseasedCured = uint8(!warrior->HasAura(WEAKENING_DISEASE)) + uint8(!warrior->HasAura(SPORE_DISEASE));
            ASSERT_INFO("diseasedCured = %u instead of %u", diseasedCured, 1);
            TEST_ASSERT(diseasedCured == 1);
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<CureDiseaseTestImpt>();
    }
};

class DesperatePrayerTest : public TestCaseScript
{
public:
    DesperatePrayerTest() : TestCaseScript("spells priest desperate_prayer") { }

    class DesperatePrayerTestImpt : public TestCase
    {
    public:


        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_DWARF);
            TestPlayer* ally = SpawnPlayer(CLASS_WARRIOR, RACE_HUMAN);


            EQUIP_NEW_ITEM(priest, 34335); // Hammer of Sanctification -- 550 BH

            // Mana cost
            uint32 const expectedDesperatePrayerMana = 0;
            TEST_POWER_COST(priest, ClassSpells::Priest::DESPERATE_PRAYER_RNK_8, POWER_MANA, expectedDesperatePrayerMana);

            // Only heals caster + Cooldown
            ally->DisableRegeneration(true);
            const int allyHealth = 1;
            ally->SetHealth(allyHealth);
            TEST_CAST(priest, ally, ClassSpells::Priest::DESPERATE_PRAYER_RNK_8);
            TEST_ASSERT(ally->GetHealth() == allyHealth);
            TEST_HAS_COOLDOWN(priest, ClassSpells::Priest::DESPERATE_PRAYER_RNK_8, Minutes(10));

            // Heal
            float const desperatePrayerCastTime = 1.5f;
            float const desperatePrayerCoeff = desperatePrayerCastTime / 3.5f;
            uint32 const bonusHeal = 550 * desperatePrayerCoeff;
            uint32 const desperatePrayerMin = ClassSpellsDamage::Priest::DESPERATE_PRAYER_RNK_8_MIN + bonusHeal;
            uint32 const desperatePrayerMax = ClassSpellsDamage::Priest::DESPERATE_PRAYER_RNK_8_MAX + bonusHeal;
            TEST_DIRECT_HEAL(priest, priest, ClassSpells::Priest::DESPERATE_PRAYER_RNK_8, desperatePrayerMin, desperatePrayerMax, false);
            TEST_DIRECT_HEAL(priest, priest, ClassSpells::Priest::DESPERATE_PRAYER_RNK_8, desperatePrayerMin * 1.5f, desperatePrayerMax * 1.5f, true);
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<DesperatePrayerTestImpt>();
    }
};

class ElunesGraceTest : public TestCaseScript
{
public:
    ElunesGraceTest() : TestCaseScript("spells priest elunes_grace") { }

    class ElunesGraceTestImpt : public TestCase
    {
    public:


        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_NIGHTELF);
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_ORC);
            TestPlayer* hunter = SpawnPlayer(CLASS_HUNTER, RACE_ORC);

            WaitNextUpdate();

            uint32 expectedElunesGraceMana = 78;
            TEST_POWER_COST(priest, ClassSpells::Priest::ELUNES_GRACE_RNK_1, POWER_MANA, expectedElunesGraceMana);

            TEST_CAST(priest, priest, ClassSpells::Priest::ELUNES_GRACE_RNK_1);
            TEST_AURA_MAX_DURATION(priest, ClassSpells::Priest::ELUNES_GRACE_RNK_1, Seconds(15));
            TEST_HAS_COOLDOWN(priest, ClassSpells::Priest::ELUNES_GRACE_RNK_1, Minutes(3));
            TEST_HAS_AURA(priest, ClassSpells::Priest::ELUNES_GRACE_RNK_1);

            //test both melee and ranged
            {
                float const expectedResult = 25.f; // PvP Hit 5% + Elune's Grace 20%
                TEST_SPELL_HIT_CHANCE(hunter, priest, ClassSpells::Hunter::AUTO_SHOT_RNK_1, expectedResult, SPELL_MISS_MISS);
                //re cast aura, it may have expired while previous TEST_SPELL_HIT_CHANCE finished
                priest->CastSpell(priest, ClassSpells::Priest::ELUNES_GRACE_RNK_1, true);
                TEST_HAS_AURA(priest, ClassSpells::Priest::ELUNES_GRACE_RNK_1);
                TEST_MELEE_HIT_CHANCE(warrior, priest, BASE_ATTACK, expectedResult, MELEE_HIT_MISS);
            }
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<ElunesGraceTestImpt>();
    }
};

class FlashHealTest : public TestCaseScript
{
public:
    FlashHealTest() : TestCaseScript("spells priest flash_heal") { }

    class FlashHealTestImpt : public TestCase
    {
    public:


        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);

            EQUIP_NEW_ITEM(priest, 34335); // Hammer of Sanctification -- 550 BH

            // Mana cost
            uint32 const expectedFlashHealMana = 470;
            TEST_POWER_COST(priest, ClassSpells::Priest::FLASH_HEAL_RNK_9, POWER_MANA, expectedFlashHealMana);

            // Heal
            float const flashHealCastTime = 1.5f;
            float const flashHealCoeff = flashHealCastTime / 3.5f;
            uint32 const bonusHeal = 550 * flashHealCoeff;
            uint32 const flashHealMin = ClassSpellsDamage::Priest::FLASH_HEAL_RNK_9_MIN + bonusHeal;
            uint32 const flashHealMax = ClassSpellsDamage::Priest::FLASH_HEAL_RNK_9_MAX + bonusHeal;
            TEST_DIRECT_HEAL(priest, priest, ClassSpells::Priest::FLASH_HEAL_RNK_9, flashHealMin, flashHealMax, false);
            TEST_DIRECT_HEAL(priest, priest, ClassSpells::Priest::FLASH_HEAL_RNK_9, flashHealMin * 1.5f, flashHealMax * 1.5f, true);
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<FlashHealTestImpt>();
    }
};

class GreaterHealTest : public TestCaseScript
{
public:
    GreaterHealTest() : TestCaseScript("spells priest greater_heal") { }

    class GreaterHealTestImpt : public TestCase
    {
    public:


        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);

            EQUIP_NEW_ITEM(priest, 34335); // Hammer of Sanctification -- 550 BH

            // Mana cost
            uint32 const expectedGreaterHealMana = 825;
            TEST_POWER_COST(priest, ClassSpells::Priest::GREATER_HEAL_RNK_7, POWER_MANA, expectedGreaterHealMana);

            // Heal
            float const greaterHealCastTime = 3.0f;
            float const greaterHealCoeff = greaterHealCastTime / 3.5f;
            uint32 const bonusHeal = 550 * greaterHealCoeff;
            uint32 const greaterHealMin = ClassSpellsDamage::Priest::GREATER_HEAL_RNK_7_MIN + bonusHeal;
            uint32 const greaterHealMax = ClassSpellsDamage::Priest::GREATER_HEAL_RNK_7_MAX + bonusHeal;
            TEST_DIRECT_HEAL(priest, priest, ClassSpells::Priest::GREATER_HEAL_RNK_7, greaterHealMin, greaterHealMax, false);
            TEST_DIRECT_HEAL(priest, priest, ClassSpells::Priest::GREATER_HEAL_RNK_7, greaterHealMin * 1.5f, greaterHealMax * 1.5f, true);
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<GreaterHealTestImpt>();
    }
};

class HealTest : public TestCaseScript
{
public:
    HealTest() : TestCaseScript("spells priest heal") { }

    class HealTestImpt : public TestCase
    {
    public:


        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);

            EQUIP_NEW_ITEM(priest, 34335); // Hammer of Sanctification -- 550 BH

            // Mana cost
            uint32 const expectedHealMana = 305;
            TEST_POWER_COST(priest, ClassSpells::Priest::HEAL_RNK_4, POWER_MANA, expectedHealMana);

            // Heal
            int const spellMaxLevel = 39;
            int const spellBaseLevel = 34;
            float const spellRealPointsPerLevel = 4.5f;
            float const downrankingCoeff = (spellMaxLevel + 6) / 70.0f;
            uint32 const bonusPoints = (spellMaxLevel - spellBaseLevel) * spellRealPointsPerLevel;
            uint32 const bonusMaxPoints = 93; // effectDieSides1
            float const healCastTime = 3.0f;
            float const healCoeff = healCastTime / 3.5f;
            uint32 const bonusHeal = 550 * healCoeff * downrankingCoeff;
            uint32 const healMin = ClassSpellsDamage::Priest::HEAL_RNK_4_MIN + bonusPoints + bonusHeal;
            uint32 const healMax = ClassSpellsDamage::Priest::HEAL_RNK_4_MIN + bonusPoints + bonusMaxPoints + bonusHeal;
            TEST_DIRECT_HEAL(priest, priest, ClassSpells::Priest::HEAL_RNK_4, healMin, healMax, false);
            TEST_DIRECT_HEAL(priest, priest, ClassSpells::Priest::HEAL_RNK_4, healMin * 1.5f, healMax * 1.5f, true);
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<HealTestImpt>();
    }
};

class HolyFireTest : public TestCaseScript
{
public:
    HolyFireTest() : TestCaseScript("spells priest holy_fire") { }

    class HolyFireTestImpt : public TestCase
    {
    public:


        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);
            Creature* creature = SpawnCreature();

            EQUIP_NEW_ITEM(priest, 34336); // Sunflare -- 292 BH

            // Mana cost
            uint32 const expectedHolyFireMana = 290;
            TEST_POWER_COST(priest, ClassSpells::Priest::HOLY_FIRE_RNK_9, POWER_MANA, expectedHolyFireMana);

            // Direct
            float const holyFireCoeff = ClassSpellsCoeff::Priest::HOLY_FIRE;
            uint32 const bonusSP = 292 * holyFireCoeff;
            uint32 const holyFireMin = ClassSpellsDamage::Priest::HOLY_FIRE_RNK_9_MIN + bonusSP;
            uint32 const holyFireMax = ClassSpellsDamage::Priest::HOLY_FIRE_RNK_9_MAX + bonusSP;
            TEST_DIRECT_SPELL_DAMAGE(priest, creature, ClassSpells::Priest::HOLY_FIRE_RNK_9, holyFireMin, holyFireMax, false);
            TEST_DIRECT_SPELL_DAMAGE(priest, creature, ClassSpells::Priest::HOLY_FIRE_RNK_9, holyFireMin * 1.5f, holyFireMax * 1.5f, true);
            
            // DoT 
            float const holyFireDoTCoeff = ClassSpellsCoeff::Priest::HOLY_FIRE_DOT;
            uint32 const bonusDoTSP = 292 * holyFireDoTCoeff;
            uint32 const holyFireDoT = (ClassSpellsDamage::Priest::HOLY_FIRE_RNK_9_TOTAL + bonusDoTSP);
            TEST_DOT_DAMAGE(priest, creature, ClassSpells::Priest::HOLY_FIRE_RNK_9, holyFireDoT, true);
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<HolyFireTestImpt>();
    }
};

class LesserHealTest : public TestCaseScript
{
public:
    LesserHealTest() : TestCaseScript("spells priest lesser_heal") { }

    //Heal your target for 135 to 158.
    class LesserHealTestImpt : public TestCase
    {
    public:


        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);

            EQUIP_NEW_ITEM(priest, 34335); // Hammer of Sanctification -- 550 BH

            // Mana cost
            uint32 const expectedLesserHealMana = 75;
            TEST_POWER_COST(priest, ClassSpells::Priest::LESSER_HEAL_RNK_3, POWER_MANA, expectedLesserHealMana);

            // Lesser Heal
            int const spellMaxLevel = 15;
            int const spellBaseLevel = 10;
            float const spellRealPointsPerLevel = 1.6f;
            float const downrankingCoeff = (spellMaxLevel + 6) / 70.0f; //  (spellMaxLevel + 5) / playerLevel <- downranking coeff formula
            float const below20penaltyCoeff = 1.0f - ((20.0f - spellBaseLevel) * 0.0375f);
            uint32 const bonusPoints = (spellMaxLevel - spellBaseLevel) * spellRealPointsPerLevel;
            uint32 const bonusMaxPoints = 23; // effectDieSides1
            float const healCastTime = 2.5f;
            float const healCoeff = healCastTime / 3.5f;
            uint32 const bonusLesserHeal = 550 * healCoeff * downrankingCoeff * below20penaltyCoeff;
            uint32 const healMin = ClassSpellsDamage::Priest::LESSER_HEAL_RNK_3_MIN + bonusPoints + bonusLesserHeal;
            uint32 const healMax = ClassSpellsDamage::Priest::LESSER_HEAL_RNK_3_MAX + bonusPoints + bonusLesserHeal;
            TEST_DIRECT_HEAL(priest, priest, ClassSpells::Priest::LESSER_HEAL_RNK_3, healMin, healMax, false);
            TEST_DIRECT_HEAL(priest, priest, ClassSpells::Priest::LESSER_HEAL_RNK_3, healMin * 1.5f, healMax * 1.5f, true);
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<LesserHealTestImpt>();
    }
};

class PrayerOfHealingTest : public TestCaseScript
{
public:
    PrayerOfHealingTest() : TestCaseScript("spells priest prayer_of_healing") { }

    class PrayerOfHealingTestImpt : public TestCase
    {
    public:


        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);
            //spawn some priests to target
            //priest 1 to 4 should be in range
            //5 should be too far
            //6 should be in range but in another group
            Position spawn(_location);
            spawn.MoveInFront(spawn, 3.0f);
            TestPlayer* priest2 = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF, 70, spawn);
            spawn.MoveInFront(spawn, 3.0f); //6y
            TestPlayer* priest3 = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF, 70, spawn);
            spawn.MoveInFront(spawn, 19.0f); //25y
            TestPlayer* priest4 = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF, 70, spawn);
            spawn.MoveInFront(spawn, 10.0f); //35y
            TestPlayer* priest5 = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF, 70, spawn);

            TestPlayer* priest6 = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF, 70, priest->GetPosition());

            // Group raid the players
            std::list<TestPlayer*> toGroup { priest2, priest3, priest4, priest5, priest6 };
            priest->DisableRegeneration(true);
            priest->SetHealth(1);
            for (auto player : toGroup)
            {
                player->DisableRegeneration(true);
                player->SetHealth(1);

                GroupPlayer(priest, player);
            }

            // Priest 6 should end up in group 2 (group 1 is full)
            TEST_ASSERT(!priest->GetGroup()->SameSubGroup(priest, priest6));

            EQUIP_NEW_ITEM(priest, 34335); // Hammer of Sanctification -- 550 BH

            // Mana cost
            uint32 const expectedPrayerOfHealingMana = 1255;
            TEST_POWER_COST(priest, ClassSpells::Priest::PRAYER_OF_HEALING_RNK_6, POWER_MANA, expectedPrayerOfHealingMana);

            TEST_CAST(priest, priest, ClassSpells::Priest::PRAYER_OF_HEALING_RNK_6, SPELL_CAST_OK, TRIGGERED_CAST_DIRECTLY);

            // Spell has range 30y and should only target self group
            TEST_ASSERT(priest->GetHealth() > 1);
            TEST_ASSERT(priest2->GetHealth() > 1);
            TEST_ASSERT(priest3->GetHealth() > 1);
            TEST_ASSERT(priest4->GetHealth() > 1);
            TEST_ASSERT(priest5->GetHealth() == 1); // 35y
            TEST_ASSERT(priest6->GetHealth() == 1); // close but not in the same group

            // Heal value
            float const prayerOfHealingCastTime = 3.0f;
            float const prayerOfHealingCoeff = prayerOfHealingCastTime / 3.5f / 2.0f;
            uint32 const bonusHeal = 550 * prayerOfHealingCoeff;
            uint32 const prayerOfHealingMin = ClassSpellsDamage::Priest::PRAYER_OF_HEALING_RNK_6_MIN + bonusHeal;
            uint32 const prayerOfHealingMax = ClassSpellsDamage::Priest::PRAYER_OF_HEALING_RNK_6_MAX + bonusHeal;
            TEST_DIRECT_HEAL(priest, priest, ClassSpells::Priest::PRAYER_OF_HEALING_RNK_6, prayerOfHealingMin, prayerOfHealingMax, false);
            TEST_DIRECT_HEAL(priest, priest, ClassSpells::Priest::PRAYER_OF_HEALING_RNK_6, prayerOfHealingMin * 1.5f, prayerOfHealingMax * 1.5f, true);
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<PrayerOfHealingTestImpt>();
    }
};

class PrayerOfMendingTest : public TestCaseScript
{
public:
    PrayerOfMendingTest() : TestCaseScript("spells priest prayer_of_mending") { }

    class PrayerOfMendingTestImpt : public TestCase
    {
    public:


        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);

            Position spawn(_location);
            spawn.MoveInFront(spawn, -2.0f);
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_BLOODELF, 70, spawn);
            spawn.MoveInFront(spawn, -2.0f);
            TestPlayer* priest2 = SpawnPlayer(CLASS_PRIEST, RACE_HUMAN, 70, spawn);
            EQUIP_NEW_ITEM(priest, 34335); // Hammer of Sanctification -- 550 BH

            // Failed
            TEST_CAST(priest, warlock, ClassSpells::Priest::PRAYER_OF_MENDING_RNK_1, SPELL_FAILED_BAD_TARGETS);
            GroupPlayer(priest, warlock);

            uint32 const expectedPrayerOfMendingMana = 390;
            TEST_POWER_COST(priest, ClassSpells::Priest::PRAYER_OF_MENDING_RNK_1, POWER_MANA, expectedPrayerOfMendingMana);

            TEST_CAST(priest, warlock, ClassSpells::Priest::PRAYER_OF_MENDING_RNK_1);
            TEST_COOLDOWN(priest, warlock, ClassSpells::Priest::PRAYER_OF_MENDING_RNK_1, Seconds(10));
            TEST_AURA_MAX_DURATION(warlock, ClassSpells::Priest::PRAYER_OF_MENDING_RNK_1_BUFF, Seconds(30));

            TEST_AURA_CHARGE(warlock, ClassSpells::Priest::PRAYER_OF_MENDING_RNK_1_BUFF, 5);

            // Changed target and 1 charge less
            priest2->ForceMeleeHitResult(MELEE_HIT_NORMAL);
            priest2->AttackerStateUpdate(warlock);
            WaitNextUpdate();
            TEST_AURA_CHARGE(priest, ClassSpells::Priest::PRAYER_OF_MENDING_RNK_1_BUFF, 4);

            // Heal
            auto AI = priest->GetTestingPlayerbotAI(); //on retail, healing appears done by target on combat journal
            auto healingToTarget = AI->GetHealingDoneInfo(warlock);
            TEST_ASSERT(healingToTarget != nullptr);
            //SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(ClassSpells::Priest::PRAYER_OF_MENDING_RNK_1);
            TEST_ASSERT(healingToTarget->size() == 1);

            uint32 heal = healingToTarget->begin()->healing;

            float const prayerOfMendingCoeff = ClassSpellsCoeff::Priest::PRAYER_OF_MENDING;
            uint32 const bonusHeal = 550 * prayerOfMendingCoeff;
            uint32 const prayerOfMending = ClassSpellsDamage::Priest::PRAYER_OF_MENDING_RNK_1 + bonusHeal;

            ASSERT_INFO("Heal: %u, expected: %u", heal, prayerOfMending);
            TEST_ASSERT(Between(heal, prayerOfMending - 1, prayerOfMending + 1));
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<PrayerOfMendingTestImpt>();
    }
};

class RenewTest : public TestCaseScript
{
public:
    RenewTest() : TestCaseScript("spells priest renew") { }

    class RenewTestImpt : public TestCase
    {
    public:


        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);

            EQUIP_NEW_ITEM(priest, 34335); // Hammer of Sanctification -- 550 BH

            // Mana cost
            uint32 const expectedRenewMana = 450;
            TEST_POWER_COST(priest, ClassSpells::Priest::RENEW_RNK_12, POWER_MANA, expectedRenewMana);

            // Heal
            float const renewDuration = 15.0f;
            float const renewCoeff = renewDuration / 15.0f;
            uint32 const bonusHeal = 550 * renewCoeff;
            uint32 const renewTicks = 5.0f * floor((ClassSpellsDamage::Priest::RENEW_RNK_12_TOTAL + bonusHeal) / 5.0f);
            TEST_DOT_DAMAGE(priest, priest, ClassSpells::Priest::RENEW_RNK_12, renewTicks, true);
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<RenewTestImpt>();
    }
};

class ResurrectionTest : public TestCaseScript
{
public:
    ResurrectionTest() : TestCaseScript("spells priest resurrection") { }

    //"Brings a dead player back to life with 1100 health and 1150 mana. Cannot be cast when in combat."
    class ResurrectionTestImpt : public TestCase
    {
    public:


        void TestResurrection(TestPlayer* caster, TestPlayer* victim, uint32 spellId, uint32 manaCost, uint32 expectedHealth, uint32 expectedMana, bool fail = false)
        {
            victim->KillSelf(true);
            TEST_POWER_COST(caster, spellId, POWER_MANA, manaCost);

            TEST_CAST(caster, victim, spellId, SPELL_CAST_OK, TRIGGERED_FULL_MASK);
            WaitNextUpdate();
            victim->RessurectUsingRequestData();
            WaitNextUpdate(); //resurrect needs 1 update to be done
            if (!fail)
            {
                TEST_ASSERT(victim->GetHealth() == expectedHealth);
                TEST_ASSERT(victim->GetPower(POWER_MANA) == expectedMana);
            } 
            else
            {
                ASSERT_INFO("Victim was resurrected but spell should have failed");
                TEST_ASSERT(victim->IsDead());
            }
        }

        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);
            TestPlayer* ally = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);
            TestPlayer* enemy = SpawnPlayer(CLASS_DRUID, RACE_NIGHTELF);

            uint32 manaCost = 1572;

            TestResurrection(priest, ally, ClassSpells::Priest::RESURRECTION_RNK_1, manaCost, 70, 135);
            TestResurrection(priest, ally, ClassSpells::Priest::RESURRECTION_RNK_2, manaCost, 160, 300);
            TestResurrection(priest, ally, ClassSpells::Priest::RESURRECTION_RNK_3, manaCost, 300, 520);
            TestResurrection(priest, ally, ClassSpells::Priest::RESURRECTION_RNK_4, manaCost, 500, 750);
            TestResurrection(priest, ally, ClassSpells::Priest::RESURRECTION_RNK_5, manaCost, 750, 1000);
            TestResurrection(priest, ally, ClassSpells::Priest::RESURRECTION_RNK_6, manaCost, 1100, 1150);

            /*WoWWiki: Note that there is no way to tell whether or not a corpse belongs to the player's faction except by inspecting 
            it visually. If the corpse does belong to the player's faction, it is possible to tell whether the player is still online 
            or not by typing "/who playername." A response of "0 players found" indicates that the player has logged off (or is of the
            wrong faction). A response giving the target's race, level, and class indicates that the player is still online and can be 
            successfully resurrected. Note that landing a resurrection on an offline or cross-faction player's corpses is 
            indistinguishable from landing one on an online player's corpse who simply declines the resurrection. In both cases, 
            the mana is wasted, and resurrection will not take place.*/
            //-> Should not work crossfaction
            TestResurrection(priest, enemy, ClassSpells::Priest::RESURRECTION_RNK_6, manaCost, 1100, 1150, true);
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<ResurrectionTestImpt>();
    }
};

class SmiteTest : public TestCaseScript
{
public:
    SmiteTest() : TestCaseScript("spells priest smite") { }

    class SmiteTestImpt : public TestCase
    {
    public:


        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);
            Creature* dummy = SpawnCreature();

            EQUIP_NEW_ITEM(priest, 34336); // Sunflare -- 292 SP

            // Mana cost
            uint32 const expectedSmiteMana = 385;
            TEST_POWER_COST(priest, ClassSpells::Priest::SMITE_RNK_10, POWER_MANA, expectedSmiteMana);

            // Damage
            float const smiteCastTime = 2.5f;
            float const smiteCoeff = smiteCastTime / 3.5f;
            uint32 const bonusHeal = 292 * smiteCoeff;
            uint32 const smiteMin = ClassSpellsDamage::Priest::SMITE_RNK_10_MIN + bonusHeal;
            uint32 const smiteMax = ClassSpellsDamage::Priest::SMITE_RNK_10_MAX + bonusHeal;
            TEST_DIRECT_SPELL_DAMAGE(priest, dummy, ClassSpells::Priest::SMITE_RNK_10, smiteMin, smiteMax, false);
            TEST_DIRECT_SPELL_DAMAGE(priest, dummy, ClassSpells::Priest::SMITE_RNK_10, smiteMin * 1.5f, smiteMax * 1.5f, true);
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<SmiteTestImpt>();
    }
};

class DevouringPlagueTest : public TestCaseScript
{
public:
    DevouringPlagueTest() : TestCaseScript("spells priest devouring_plague") { }

    class DevouringPlagueTestImpt : public TestCase
    {
    public:


        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_UNDEAD_PLAYER);
            Creature* dummy = SpawnCreature();

            priest->DisableRegeneration(true);
            priest->SetHealth(1);

            EQUIP_NEW_ITEM(priest, 34335); // Hammer of Sanctification -- 183 SP & 550 BH

            // Mana cost
            uint32 const expectedDevouringPlagueMana = 1145;
            TEST_POWER_COST(priest, ClassSpells::Priest::DEVOURING_PLAGUE_RNK_7, POWER_MANA, expectedDevouringPlagueMana);

            FORCE_CAST(priest, dummy, ClassSpells::Priest::DEVOURING_PLAGUE_RNK_7);
            TEST_AURA_MAX_DURATION(dummy, ClassSpells::Priest::DEVOURING_PLAGUE_RNK_7, Seconds(24));
            TEST_HAS_COOLDOWN(priest, ClassSpells::Priest::DEVOURING_PLAGUE_RNK_7, Minutes(3));

            // Damage
            float const dvouringPlagueDotTime = 24.0f;
            float const devouringPlagueCoeff = dvouringPlagueDotTime / 15 / 2;
            uint32 const spellBonus = 183 * devouringPlagueCoeff;

            int const devouringPlagueTickCount = 8;
            uint32 const devouringPlagueTick = ClassSpellsDamage::Priest::DEVOURING_PLAGUE_RNK_7_TICK + spellBonus / devouringPlagueTickCount;
            uint32 const devouringPlagueTotal = devouringPlagueTick * devouringPlagueTickCount;
            TEST_DOT_DAMAGE(priest, dummy, ClassSpells::Priest::DEVOURING_PLAGUE_RNK_7, devouringPlagueTotal, true);

            // Heal
            TEST_ASSERT(priest->GetHealth() == 1 + devouringPlagueTotal);
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<DevouringPlagueTestImpt>();
    }
};

class FadeTest : public TestCaseScript
{
public:
    FadeTest() : TestCaseScript("spells priest fade") { }

    class FadeTestImpt : public TestCase
    {
    public:


        void Test() override
        {

            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);
            TestPlayer* tank = SpawnPlayer(CLASS_WARRIOR, RACE_HUMAN);
            Creature* creature = SpawnCreature(16878, true); // Shattered Hand Berserker -- Lvl 61

            float const fadeValue = 1512.0f; //this value are from Spell.dbc, with a caster of level 70 (1500 base value + some calc with BaseLevel and such)

            // Setup
            tank->SetMaxHealth(30000);
            tank->SetFullHealth();
            FORCE_CAST(tank, tank, ClassSpells::Warrior::DEFENSIVE_STANCE_RNK_1);
            EQUIP_NEW_ITEM(tank, 34185); // Shield
            tank->SetPower(POWER_RAGE, tank->GetMaxPower(POWER_RAGE));
            tank->Attack(creature, true);
            Wait(500);
            TEST_ASSERT(creature->GetTarget() == tank->GetGUID());

            // Threat
            //generate some threat
            for(uint32 i = 0; i < 5; i++)
                FORCE_CAST(priest, creature, ClassSpells::Priest::MIND_BLAST_RNK_11, SPELL_MISS_NONE, TRIGGERED_FULL_MASK);
            WaitNextUpdate();
            float startThreat = creature->GetThreatManager().GetThreat(priest);
            //some consistency checks
            TEST_ASSERT(startThreat > creature->GetThreatManager().GetThreat(tank));
            TEST_ASSERT(startThreat > fadeValue); 
            ASSERT_INFO("Wrong target. tank threat: %f, priest threat: %f", creature->GetThreatManager().GetThreat(tank), creature->GetThreatManager().GetThreat(priest));
            TEST_ASSERT(creature->GetTarget() == priest->GetGUID());

            // Aura duration
            TEST_CAST(priest, priest, ClassSpells::Priest::FADE_RNK_7);
            TEST_AURA_MAX_DURATION(priest, ClassSpells::Priest::FADE_RNK_7, Seconds(10));

            // Cooldown
            TEST_HAS_COOLDOWN(priest, ClassSpells::Priest::FADE_RNK_7, Seconds(30));

            // Effect
            float const expectedThreat = startThreat - fadeValue;
            float const currentThreat = creature->GetThreatManager().GetThreat(priest);
            ASSERT_INFO("currentThreat %f - expectedThreat %f", currentThreat, expectedThreat);
            TEST_ASSERT(Between(currentThreat, expectedThreat - 1.0f, expectedThreat + 1.0f));

            // Check if threat is given back
            // http://wowwiki.wikia.com/wiki/Fade?oldid=1431850
            // When Fade's duration expires, the **next threat-generating action** will generate normal threat plus all the threat that was originally Faded from the target.
            // Note: On our server threat is immediately given back, we're not reproducing this specific behavior
            Wait(11000); // fade last 10 seconds
            TEST_ASSERT(priest->IsInCombatWith(creature));
            TEST_HAS_NOT_AURA(priest, ClassSpells::Priest::FADE_RNK_7);
            float const currentThreat2 = creature->GetThreatManager().GetThreat(priest);
            ASSERT_INFO("currentThreat %f - startThreat %f", currentThreat2, startThreat);
            TEST_ASSERT(Between(currentThreat2, startThreat - 1.0f, startThreat + 1.0f));

            // Power cost
            uint32 const expectedFadeMana = 330;
            TEST_POWER_COST(priest, ClassSpells::Priest::FADE_RNK_7, POWER_MANA, expectedFadeMana);
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<FadeTestImpt>();
    }
};

class HexOfWeaknessTest : public TestCaseScript
{
public:
    HexOfWeaknessTest() : TestCaseScript("spells priest hex_of_weakness") { }

    class HexOfWeaknessTestImpt : public TestCase
    {
    public:


        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);
            TestPlayer* rogue = SpawnPlayer(CLASS_ROGUE, RACE_HUMAN);
            TestPlayer* healRogue = SpawnPlayer(CLASS_PRIEST, RACE_HUMAN);

            EQUIP_NEW_ITEM(priest, 34336); // Sunflare - 292 SP

            // Mana cost, aura & cd
            // priest applies hex on rogue
            uint32 const expectedHexOfWeaknessMana = 295;
            TEST_POWER_COST(priest, ClassSpells::Priest::HEX_OF_WEAKNESS_RNK_7, POWER_MANA, expectedHexOfWeaknessMana);

            FORCE_CAST(priest, rogue, ClassSpells::Priest::HEX_OF_WEAKNESS_RNK_7);
            TEST_HAS_AURA(rogue, ClassSpells::Priest::HEX_OF_WEAKNESS_RNK_7);
            TEST_AURA_MAX_DURATION(rogue, ClassSpells::Priest::HEX_OF_WEAKNESS_RNK_7, Minutes(2));

            // Healing reduced by 20%
            float const hexOfWeaknessHealMalus = 0.80f;
            float const greaterHealCastTime = 3.0f;
            float const greaterHealCoeff = greaterHealCastTime / 3.5f;
            uint32 const greaterHealMin = ClassSpellsDamage::Priest::GREATER_HEAL_RNK_7_MIN * hexOfWeaknessHealMalus;
            uint32 const greaterHealMax = ClassSpellsDamage::Priest::GREATER_HEAL_RNK_7_MAX * hexOfWeaknessHealMalus;
            TEST_DIRECT_HEAL(healRogue, rogue, ClassSpells::Priest::GREATER_HEAL_RNK_7, greaterHealMin, greaterHealMax, false);
            TEST_DIRECT_HEAL(healRogue, rogue, ClassSpells::Priest::GREATER_HEAL_RNK_7, greaterHealMin * 1.5f, greaterHealMax * 1.5f, true);

            // Damage reduction
            {
                Creature* dummy = SpawnCreature();
                EQUIP_NEW_ITEM(rogue, 32837); // Warglaive of Azzinoth MH
                Wait(1500);
                EQUIP_NEW_ITEM(rogue, 32838); // Warglaive of Azzinoth OH
                WaitNextUpdate();

                // Sinister strike
                int const hexOfWeaknessDamageMalus = 35;
                int const sinisterStrikeBonus = 98;
                float const normalizedSwordSpeed = 2.4f;
                auto[minSinisterStrike, maxSinisterStrike] = CalcMeleeDamage(rogue, dummy, BASE_ATTACK, sinisterStrikeBonus - hexOfWeaknessDamageMalus, normalizedSwordSpeed);
                TEST_DIRECT_SPELL_DAMAGE(rogue, dummy, ClassSpells::Rogue::SINISTER_STRIKE_RNK_10, minSinisterStrike, maxSinisterStrike, false);

                // Melee
                auto[minMHMelee, maxMHMelee] = CalcMeleeDamage(rogue, dummy, BASE_ATTACK, -hexOfWeaknessDamageMalus);
                TEST_MELEE_DAMAGE(rogue, dummy, BASE_ATTACK, minMHMelee, maxMHMelee, false);
                auto[minOHMelee, maxOHMelee] = CalcMeleeDamage(rogue, dummy, OFF_ATTACK, -hexOfWeaknessDamageMalus);
                TEST_MELEE_DAMAGE(rogue, dummy, OFF_ATTACK, minOHMelee, maxOHMelee, false);
            }
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<HexOfWeaknessTestImpt>();
    }
};

class MindBlastTest : public TestCaseScript
{
public:
    MindBlastTest() : TestCaseScript("spells priest mind_blast") { }

    class MindBlastTestImpt : public TestCase
    {
    public:


        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);
            Creature* dummy = SpawnCreature();

            EQUIP_NEW_ITEM(priest, 34336); // Sunflare -- 292 SP

            // Mana cost
            uint32 const expectedMindBlastMana = 450;
            TEST_POWER_COST(priest, ClassSpells::Priest::MIND_BLAST_RNK_11, POWER_MANA, expectedMindBlastMana);

            TEST_CAST(priest, dummy, ClassSpells::Priest::MIND_BLAST_RNK_11, SPELL_CAST_OK, TRIGGERED_CAST_DIRECTLY);
            // Cooldown
            TEST_COOLDOWN(priest, dummy, ClassSpells::Priest::MIND_BLAST_RNK_11, Seconds(8));

            // Damage
            float const mindBlastCastTime = 1.5f;
            float const mindBlastCoeff = mindBlastCastTime / 3.5f;
            uint32 const bonusSpell = 292 * mindBlastCoeff;
            uint32 const mindBlastMin = ClassSpellsDamage::Priest::MIND_BLAST_RNK_11_MIN + bonusSpell;
            uint32 const mindBlastMax = ClassSpellsDamage::Priest::MIND_BLAST_RNK_11_MAX + bonusSpell;
            TEST_DIRECT_SPELL_DAMAGE(priest, dummy, ClassSpells::Priest::MIND_BLAST_RNK_11, mindBlastMin, mindBlastMax, false);
            TEST_DIRECT_SPELL_DAMAGE(priest, dummy, ClassSpells::Priest::MIND_BLAST_RNK_11, mindBlastMin * 1.5f, mindBlastMax * 1.5f, true);
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<MindBlastTestImpt>();
    }
};

class MindControlTest : public TestCaseScript
{
public:
    MindControlTest() : TestCaseScript("spells priest mind_control") { }

    class MindControlTestImpt : public TestCase
    {
    public:


        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);
            TestPlayer* enemy = SpawnPlayer(CLASS_WARRIOR, RACE_HUMAN);
            uint32 const baseAttackSpeed = enemy->GetAttackTime(BASE_ATTACK);

            // Mana cost
            uint32 const expectedMindControlMana = 750;
            TEST_POWER_COST(priest, ClassSpells::Priest::MIND_CONTROL_RNK_3, POWER_MANA, expectedMindControlMana);

            FORCE_CAST(priest, enemy, ClassSpells::Priest::MIND_CONTROL_RNK_3, SPELL_MISS_NONE, TRIGGERED_CAST_DIRECTLY);
            Wait(5000);
            _StartUnitChannels(priest);

            // PvP Duration
            TEST_AURA_MAX_DURATION(enemy, ClassSpells::Priest::MIND_CONTROL_RNK_3, Seconds(10));

            SECTION("Attack Speed +25%", [&] {
                uint32 const expectedAttackSpeed = baseAttackSpeed * 1.25f;
                enemy->ResetAttackTimer(BASE_ATTACK);
                TEST_ASSERT(Between(enemy->GetAttackTimer(BASE_ATTACK), expectedAttackSpeed - 1, expectedAttackSpeed + 1));
            });

            SECTION("Statuses and flag", [&] {
                TEST_ASSERT(enemy->IsPossessedByPlayer());
                TEST_ASSERT(priest->GetSession()->GetActiveMover() == enemy);
                TEST_ASSERT(priest->GetViewpoint() == enemy);
                TEST_ASSERT(enemy->HasUnitState(UNIT_STATE_POSSESSED));
                TEST_ASSERT(enemy->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_POSSESSED));
                TEST_ASSERT(priest->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL));
            });

            priest->InterruptNonMeleeSpells(true);
            WaitNextUpdate();
             
            SECTION("Unapply", [&] {
                TEST_ASSERT(!enemy->IsPossessed());
                TEST_ASSERT(priest->GetSession()->GetActiveMover() == priest);
                TEST_ASSERT(priest->GetViewpoint() == nullptr);
                TEST_ASSERT(!enemy->HasUnitState(UNIT_STATE_POSSESSED));
                TEST_ASSERT(!enemy->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_POSSESSED));
                TEST_ASSERT(!priest->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL));
            });

            SECTION("Resist bug", [&] {
                // Previously, resisting a MC was leaving the dummy aura as well as the channeling on the priest
                FORCE_CAST(priest, enemy, ClassSpells::Priest::MIND_CONTROL_RNK_3, SPELL_MISS_IMMUNE, TRIGGERED_CAST_DIRECTLY);
                Wait(5000);
                _StartUnitChannels(priest);
                TEST_ASSERT(priest->m_currentSpells[CURRENT_CHANNELED_SPELL] == nullptr);
                TEST_HAS_NOT_AURA(priest, ClassSpells::Priest::MIND_CONTROL_RNK_3);
            });
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<MindControlTestImpt>();
    }
};

class MindSootheTest : public TestCaseScript
{
public:
    MindSootheTest() : TestCaseScript("spells priest mind_soothe") { }

    class MindSootheTestImpt : public TestCase
    {
        //get approximative aggro range (may be around 1y wrong max)
        float GetAggroRange(TestPlayer* priest, Creature* target, float maxDistance)
        {
            TEST_ASSERT(target->GetVictim() == nullptr); //target must not have target when we start
            //teleport priest gradually closer to target and stop when target has aggroed priest
            uint32 dist = 0.0f;
            float baseX = priest->GetPositionX();
            while (!target->GetVictim())
            {
                dist += 1.0f;
                priest->TeleportTo(priest->GetMapId(), baseX + dist, priest->GetPositionY(), priest->GetPositionZ(), priest->GetOrientation());
                WaitNextUpdate(); //try with a second WaitNextUpdate if humanoid still won't aggro priest
                Wait(1000);
                TEST_ASSERT(dist < maxDistance);
            }
            return target->GetDistance(priest);
        }

        void Test() override
        { 
            //INCOMPLETE: Logic seems to be ok and distance is affected by spell, but creatures aggro distance is wonky.
            // More precisely, it appears creatures aggro on a tick basis. FIXME
            SECTION("WIP", STATUS_WIP, [&] {
                TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);
                Position spawn(_location);
                float const spawnDistance = 40.0f;
                spawn.MoveInFront(_location, spawnDistance);
                Creature* humanoid = SpawnCreatureWithPosition(spawn, 25363); //sunblade cabalist
                Creature* beast = SpawnCreatureWithPosition(spawn, 22885);

                // Only cast on humanoid
                TEST_CAST(priest, beast, ClassSpells::Priest::MIND_SOOTHE_RNK_4, SPELL_FAILED_BAD_TARGETS);
                beast->DespawnOrUnsummon();

                // Get initial aggro range
                float const aggroRange = GetAggroRange(priest, humanoid, spawnDistance);

                // Reset to find aggro range with Mind Soothe
                priest->TeleportTo(_location);
                humanoid->AI()->EnterEvadeMode();
                Wait(Seconds(5));

                // Mana cost
                uint32 const expectedMindSootheMana = 120;
                TEST_POWER_COST(priest, ClassSpells::Priest::MIND_SOOTHE_RNK_4, POWER_MANA, expectedMindSootheMana);

                FORCE_CAST(priest, humanoid, ClassSpells::Priest::MIND_SOOTHE_RNK_4);

                // Aura
                TEST_AURA_MAX_DURATION(humanoid, ClassSpells::Priest::MIND_SOOTHE_RNK_4, Seconds(15));

                WaitNextUpdate();
                float const reducedAggroRange = GetAggroRange(priest, humanoid, spawnDistance);

                float const mindSootheRangeEffect = aggroRange - reducedAggroRange;
                //TC_LOG_DEBUG("test.unit_test", "aggroRange: %f, reduced: %f, diff: %f", aggroRange, reducedAggroRange, mindSootheRangeEffect);
                TEST_ASSERT(mindSootheRangeEffect > 0);
                TEST_ASSERT(Between<float>(mindSootheRangeEffect, aggroRange - 11.0f, aggroRange - 9.0f));
            });
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<MindSootheTestImpt>();
    }
};

class MindVisionTest : public TestCaseScript
{
public:
    MindVisionTest() : TestCaseScript("spells priest mind_vision") { }

    //Allows the caster to see through the target's eyes for 1min. Will not work if the target is in another instance or on another continent.
    class MindVisionTestImpt : public TestCase
    {
    public:
        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);
            priest->SetCommandStatusOn(CHEAT_COOLDOWN);

            Position spawn;
            float mapVisibilityRange = GetMap()->GetVisibilityRange();
            spawn.MoveInFront(_location, mapVisibilityRange / 2.0f); // in priest's sight
            TestPlayer* warriorClose = SpawnPlayer(CLASS_WARRIOR, RACE_ORC, 70, spawn); 
            spawn.MoveInFront(_location, mapVisibilityRange * 1.2f); // out of priest's sight but in warrior sight
            TestPlayer* rogueFar = SpawnPlayer(CLASS_ROGUE, RACE_HUMAN, 70, spawn); 

            Wait(4000); //wait for players to appear to each others

            // Assert visibility
            TEST_ASSERT(priest->HaveAtClient(warriorClose));
            TEST_ASSERT(!priest->HaveAtClient(rogueFar));
            TEST_ASSERT(warriorClose->HaveAtClient(priest));
            TEST_ASSERT(warriorClose->HaveAtClient(rogueFar));

            FORCE_CAST(priest, warriorClose, ClassSpells::Priest::MIND_VISION_RNK_2);
            WaitNextUpdate();
            WaitNextUpdate();
            WorldPacket fakeClientResponse;
            fakeClientResponse << bool(true);
            priest->GetSession()->HandleFarSightOpcode(fakeClientResponse);

            // Aura
            TEST_AURA_MAX_DURATION(priest, ClassSpells::Priest::MIND_VISION_RNK_2, 1 * MINUTE * IN_MILLISECONDS);
            TEST_AURA_MAX_DURATION(warriorClose, ClassSpells::Priest::MIND_VISION_RNK_2, 1 * MINUTE * IN_MILLISECONDS);

            Wait(4000); //wait a bit to update view for priest
            //priest should now have both players in view
            TEST_ASSERT(priest->HaveAtClient(warriorClose));
            TEST_ASSERT(priest->HaveAtClient(rogueFar));

            // Priest should be able to cast vision on rogue, even though he was out of vision the first time
            priest->SetFullPower(POWER_MANA);
            FORCE_CAST(priest, rogueFar, ClassSpells::Priest::MIND_VISION_RNK_2, SPELL_MISS_NONE);
            WaitNextUpdate();
            WaitNextUpdate();
            TEST_HAS_AURA(rogueFar, ClassSpells::Priest::MIND_VISION_RNK_2);
            TEST_HAS_AURA(priest, ClassSpells::Priest::MIND_VISION_RNK_2);

            // Aura isnt removed by stealth
            WaitNextUpdate();
            TEST_CAST(rogueFar, rogueFar, ClassSpells::Rogue::STEALTH_RNK_4);
            TEST_HAS_AURA(rogueFar, ClassSpells::Priest::MIND_VISION_RNK_2);

            // Break if in another instance
            rogueFar->TeleportTo(37, 128.205002, 135.136002, 236.025055, 0); // Teleport to Azshara Crater
            Wait(1000);
            TEST_HAS_NOT_AURA(rogueFar, ClassSpells::Priest::MIND_VISION_RNK_2);
            TEST_HAS_NOT_AURA(priest, ClassSpells::Priest::MIND_VISION_RNK_2);

            // Mana cost
            priest->InterruptNonMeleeSpells(true);
            uint32 const expectedMindVisionMana = 150;
            TEST_POWER_COST(priest, ClassSpells::Priest::MIND_VISION_RNK_2, POWER_MANA, expectedMindVisionMana);

            //TODO: should mind vision be resistable? Because it currently is
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<MindVisionTestImpt>();
    }
};

class PrayerOfShadowProtectionTest : public TestCaseScript
{
public:
    PrayerOfShadowProtectionTest() : TestCaseScript("spells priest prayer_of_shadow_protection") { }

    //Power infuses the target's party, increasing their Shadow resistance by 70 for 20min.
    class PrayerOfShadowProtectionTestImpt : public TestCase
    {
    public:
        void TestPrayerOfShadowProtection(TestPlayer* priest, Unit* warrior, uint32 spellId, uint32 reagentId, uint32 manaCost, uint32 shadowResistanceBonus, uint32 priestStartShadowResistance, uint32 warriorStartShadowResistance)
        {
            uint32 expectedPriestSR = priestStartShadowResistance + shadowResistanceBonus;
            uint32 expectedWarriorSR = warriorStartShadowResistance + shadowResistanceBonus;

            // Mana cost & Reagent
            TEST_POWER_COST(priest, spellId, POWER_MANA, manaCost);
            priest->AddItem(reagentId, 1);
            TEST_CAST(priest, warrior, spellId, SPELL_CAST_OK, TRIGGERED_IGNORE_GCD);
            TEST_ASSERT(priest->GetItemCount(reagentId, false) == 0);

            // Aura
            TEST_AURA_MAX_DURATION(warrior, spellId, 20 * MINUTE * IN_MILLISECONDS);
            TEST_AURA_MAX_DURATION(priest, spellId, 20 * MINUTE* IN_MILLISECONDS);

            // Shadow Resistance
            TEST_ASSERT(priest->GetResistance(SPELL_SCHOOL_SHADOW) == expectedPriestSR);
            TEST_ASSERT(warrior->GetResistance(SPELL_SCHOOL_SHADOW) == expectedWarriorSR);
        }

        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_ORC);

            GroupPlayer(priest, warrior);

            uint32 const startPriestSR = priest->GetResistance(SPELL_SCHOOL_SHADOW);
            uint32 const startWarriorSR = warrior->GetResistance(SPELL_SCHOOL_SHADOW);

            uint32 const SACRED_CANDLE = 17029;

            TestPrayerOfShadowProtection(priest, warrior, ClassSpells::Priest::PRAYER_OF_SHADOW_PROTECTION_RNK_1, SACRED_CANDLE, 1300, 60, startPriestSR, startWarriorSR);
            TestPrayerOfShadowProtection(priest, warrior, ClassSpells::Priest::PRAYER_OF_SHADOW_PROTECTION_RNK_2, SACRED_CANDLE, 1620, 70, startPriestSR, startWarriorSR);
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<PrayerOfShadowProtectionTestImpt>();
    }
};

class PsychicScreamTest : public TestCaseScript
{
public:
    PsychicScreamTest() : TestCaseScript("spells priest psychic_scream") { }

    //The caster lets out a psychic scream, causing 5 enemies within 8 yards to flee for 8sec. Damage caused may interrupt the effect.
    class PsychicScreamTestImpt : public TestCase
    {
    public:


        bool isFeared(Unit* victim)
        {
            return victim->HasAura(ClassSpells::Priest::PSYCHIC_SCREAM_RNK_4);
        }

        void Test() override
        {
            uint32 const expectedPsychicScreamMana = 210;

            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);
            TestPlayer* enemy1 = SpawnPlayer(CLASS_WARRIOR, RACE_HUMAN);
            TestPlayer* enemy2 = SpawnPlayer(CLASS_WARRIOR, RACE_HUMAN);
            TestPlayer* enemy3 = SpawnPlayer(CLASS_WARRIOR, RACE_HUMAN);
            TestPlayer* enemy4 = SpawnPlayer(CLASS_WARRIOR, RACE_HUMAN);
            Position spawn(_location);
            spawn.MoveInFront(spawn, 12.0f); //spell has 8 yard range
            TestPlayer* enemyFurther = SpawnPlayer(CLASS_WARRIOR, RACE_HUMAN, 70, spawn);

            // Auras & range
            FORCE_CAST(priest, priest, ClassSpells::Priest::PSYCHIC_SCREAM_RNK_4);
            TEST_AURA_MAX_DURATION(enemy1, ClassSpells::Priest::PSYCHIC_SCREAM_RNK_4, Seconds(8));
            TEST_AURA_MAX_DURATION(enemy2, ClassSpells::Priest::PSYCHIC_SCREAM_RNK_4, Seconds(8));
            TEST_AURA_MAX_DURATION(enemy3, ClassSpells::Priest::PSYCHIC_SCREAM_RNK_4, Seconds(8));
            TEST_AURA_MAX_DURATION(enemy4, ClassSpells::Priest::PSYCHIC_SCREAM_RNK_4, Seconds(8));
            TEST_HAS_NOT_AURA(enemyFurther, ClassSpells::Priest::PSYCHIC_SCREAM_RNK_4);

            // Reset auras & cd
            priest->GetSpellHistory()->ResetAllCooldowns();
            enemy1->RemoveAurasDueToSpell(ClassSpells::Priest::PSYCHIC_SCREAM_RNK_4);
            enemy2->RemoveAurasDueToSpell(ClassSpells::Priest::PSYCHIC_SCREAM_RNK_4);
            enemy3->RemoveAurasDueToSpell(ClassSpells::Priest::PSYCHIC_SCREAM_RNK_4);
            enemy4->RemoveAurasDueToSpell(ClassSpells::Priest::PSYCHIC_SCREAM_RNK_4);
            enemyFurther->KillSelf(); // not needed anymore

            // Max 5 enemies touched
            {
                TestPlayer* enemy5 = SpawnPlayer(CLASS_WARRIOR, RACE_HUMAN);
                TestPlayer* enemy6 = SpawnPlayer(CLASS_WARRIOR, RACE_HUMAN);
                priest->SetPower(POWER_MANA, expectedPsychicScreamMana);
                FORCE_CAST(priest, priest, ClassSpells::Priest::PSYCHIC_SCREAM_RNK_4);

                uint32 count = 0;
                count += uint32(isFeared(enemy1));
                count += uint32(isFeared(enemy2));
                count += uint32(isFeared(enemy3));
                count += uint32(isFeared(enemy4));
                count += uint32(isFeared(enemy5));
                count += uint32(isFeared(enemy6));
                TEST_ASSERT(count == 5);
            }

            // Mana
            TEST_POWER_COST(priest, ClassSpells::Priest::PSYCHIC_SCREAM_RNK_4, POWER_MANA, expectedPsychicScreamMana);

            priest->GetSpellHistory()->ResetAllCooldowns();
            TEST_CAST(priest, priest, ClassSpells::Priest::PSYCHIC_SCREAM_RNK_4, SPELL_CAST_OK, TRIGGERED_IGNORE_POWER_AND_REAGENT_COST);

            // Cooldown
            TEST_COOLDOWN(priest, priest, ClassSpells::Priest::PSYCHIC_SCREAM_RNK_4, Seconds(30));
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<PsychicScreamTestImpt>();
    }
};

class ShadowProtectionTest : public TestCaseScript
{
public:
    ShadowProtectionTest() : TestCaseScript("spells priest shadow_protection") { }

    class ShadowProtectionTestImpt : public TestCase
    {
    public:


        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_ORC);

            uint32 shadowResistanceBonus = 70;
            uint32 expectedShadowResistance = warrior->GetResistance(SPELL_SCHOOL_SHADOW) + shadowResistanceBonus;

            // Mana cost
            uint32 const expectedShadowProtectionMana = 810;
            TEST_POWER_COST(priest, ClassSpells::Priest::SHADOW_PROTECTION_RNK_4, POWER_MANA, expectedShadowProtectionMana);

            TEST_CAST(priest, warrior, ClassSpells::Priest::SHADOW_PROTECTION_RNK_4);

            // Health
            ASSERT_INFO("Shadow resistance: %u, expected: %u", warrior->GetResistance(SPELL_SCHOOL_SHADOW), expectedShadowResistance);
            TEST_ASSERT(warrior->GetResistance(SPELL_SCHOOL_SHADOW) == expectedShadowResistance);

            // Aura
            TEST_AURA_MAX_DURATION(warrior, ClassSpells::Priest::SHADOW_PROTECTION_RNK_4, 10 * MINUTE * IN_MILLISECONDS);
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<ShadowProtectionTestImpt>();
    }
};

class ShadowWordDeathTest : public TestCaseScript
{
public:
    ShadowWordDeathTest() : TestCaseScript("spells priest shadow_word_death") { }

    //A word of dark binding that inflicts 572 to 665 Shadow damage to the target.If the target is not killed by Shadow Word : Death, the caster takes damage equal to the damage inflicted upon the target.
    class ShadowWordDeathTestImpt : public TestCase
    {
    public:


        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_TROLL);
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_HUMAN);
            Creature* dummy = SpawnCreature();

            EQUIP_NEW_ITEM(priest, 34336); // Sunflare -- 292 SP

            float const shadowWordDeathCastTime = 1.5f;
            float const shadowWordDeathCoeff = shadowWordDeathCastTime / 3.5f;
            uint32 const bonusSpell = 292 * shadowWordDeathCoeff;
            uint32 const shadowWordDeathMin = ClassSpellsDamage::Priest::SHADOW_WORD_DEATH_RNK_2_MIN + bonusSpell;
            uint32 const shadowWordDeathMax = ClassSpellsDamage::Priest::SHADOW_WORD_DEATH_RNK_2_MAX + bonusSpell;

            warrior->DisableRegeneration(true);
            priest->DisableRegeneration(true);
            EnableCriticals(priest, false);

            uint32 const priestStartHealth = priest->GetHealth();
            uint32 const warriorStartHealth = warrior->GetHealth();

            // Damage = backlash (regen is disabled)
            priest->GetSpellHistory()->ResetAllCooldowns();
            priest->SetPower(POWER_MANA, 2000);
            FORCE_CAST(priest, warrior, ClassSpells::Priest::SHADOW_WORD_DEATH_RNK_2);
            Wait(1000);
            Wait(1000);
            TEST_ASSERT(warrior->IsAlive());
            uint32 damage = warriorStartHealth - warrior->GetHealth();
            uint32 backlash = priestStartHealth - priest->GetHealth();
            ASSERT_INFO("damage %u, backlash %u", damage, backlash);
            TEST_ASSERT(damage == backlash);

            // Backlash absorbed by shield
            priest->GetSpellHistory()->ResetAllCooldowns();
            uint32 priestHealth = priest->GetHealth();
            FORCE_CAST(priest, priest, ClassSpells::Priest::POWER_WORD_SHIELD_RNK_12, SPELL_MISS_NONE, TRIGGERED_CAST_DIRECTLY);
            FORCE_CAST(priest, warrior, ClassSpells::Priest::SHADOW_WORD_DEATH_RNK_2);
            TEST_ASSERT(priest->GetHealth() == priestHealth);
            priest->RemoveAurasDueToSpell(ClassSpells::Priest::POWER_WORD_SHIELD_RNK_12);

            //On spell reflect: What happens?
            //We couldn't find any sources on this... 
            /*
            priest->GetSpellHistory()->ResetAllCooldowns();
            uint32 warriorHealth = warrior->GetHealth();
            FORCE_CAST(warrior, warrior, ClassSpells::Warrior::DEFENSIVE_STANCE_RNK_1);
            EQUIP_NEW_ITEM(warrior, 34185); // Shield
            warrior->SetPower(POWER_RAGE, 100 * 10);
            FORCE_CAST(warrior, warrior, ClassSpells::Warrior::SPELL_REFLECTION_RNK_1);
            Wait(1000);
            FORCE_CAST(priest, warrior, ClassSpells::Priest::SHADOW_WORD_DEATH_RNK_2);
            TEST_ASSERT(warrior->GetHealth() == warriorHealth);
            */

            // No backlash on kill
            priest->GetSpellHistory()->ResetAllCooldowns();
            priestHealth = priest->GetHealth();
            warrior->SetHealth(1);
            FORCE_CAST(priest, warrior, ClassSpells::Priest::SHADOW_WORD_DEATH_RNK_2);
            Wait(1000); //backlash has a travel time of 1000
            Wait(1000);
            TEST_ASSERT(warrior->IsDead());
            TEST_ASSERT(priest->GetHealth() == priestHealth);

            // No durability damage on suicide
            priest->GetSpellHistory()->ResetAllCooldowns();
            priest->SetHealth(1);
            FORCE_CAST(priest, dummy, ClassSpells::Priest::SHADOW_WORD_DEATH_RNK_2);
            Wait(1000); //backlash has a travel time of 1000
            Wait(1000);
            TEST_ASSERT(priest->IsDead());
            Item* sunflare = priest->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
            TEST_ASSERT(sunflare != nullptr);
            uint32 sunflareDurability = sunflare->GetUInt32Value(ITEM_FIELD_DURABILITY);
            uint32 sunflareMaxDurability = sunflare->GetUInt32Value(ITEM_FIELD_MAXDURABILITY);
            TEST_ASSERT(sunflareDurability == sunflareMaxDurability);

            // Test damages
            priest->ResurrectPlayer(1.0f);
            TEST_DIRECT_SPELL_DAMAGE(priest, dummy, ClassSpells::Priest::SHADOW_WORD_DEATH_RNK_2, shadowWordDeathMin, shadowWordDeathMax, false, [](Unit* caster, Unit* target) {
                caster->SetFullHealth();
            });
            dummy->SetFullHealth();
            TEST_DIRECT_SPELL_DAMAGE(priest, dummy, ClassSpells::Priest::SHADOW_WORD_DEATH_RNK_2, shadowWordDeathMin * 1.5f, shadowWordDeathMax * 1.5f, true, [](Unit* caster, Unit* target) {
                caster->SetFullHealth();
            });
            dummy->SetFullHealth();

            // Cooldown
            priest->ResurrectPlayer(1.0f);
            TEST_COOLDOWN(priest, dummy, ClassSpells::Priest::SHADOW_WORD_DEATH_RNK_2, Seconds(12));

            // Power cost
            uint32 const expectedShadowWordDeathMana = 309;
            TEST_POWER_COST(priest, ClassSpells::Priest::SHADOW_WORD_DEATH_RNK_2, POWER_MANA, expectedShadowWordDeathMana);
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<ShadowWordDeathTestImpt>();
    }
};

class ShadowWordPainTest : public TestCaseScript
{
public:
    ShadowWordPainTest() : TestCaseScript("spells priest shadow_word_pain") { }

    class ShadowWordPainTestImpt : public TestCase
    {
    public:


        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);
            Creature* dummy = SpawnCreature();

            EQUIP_NEW_ITEM(priest, 34336); // Sunflare -- 292 SP

            // Mana cost
            uint32 const expectedShadowWordPainMana = 575;
            TEST_POWER_COST(priest, ClassSpells::Priest::SHADOW_WORD_PAIN_RNK_10, POWER_MANA, expectedShadowWordPainMana);

            // Damage
            float const shadowWordPainCoeff = ClassSpellsCoeff::Priest::SHADOW_WORD_PAIN;
            uint32 const spellBonus = 292 * shadowWordPainCoeff;
            uint32 const shadowWordPainTotal = ClassSpellsDamage::Priest::SHADOW_WORD_PAIN_RNK_10_TOTAL + spellBonus;
            TEST_DOT_DAMAGE(priest, dummy, ClassSpells::Priest::SHADOW_WORD_PAIN_RNK_10, shadowWordPainTotal, true);
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<ShadowWordPainTestImpt>();
    }
};

//"spells priest shadowfiend"
//Creates a shadowy fiend to attack the target. Caster receives mana when the Shadowfiend deals damage. Lasts 15sec.
class ShadowfiendTest : public TestCase
{
    /*
    Data:
    - http://wowwiki.wikia.com/wiki/Shadowfiend?oldid=1581560
    - https://github.com/FeenixServerProject/Archangel_2.4.3_Bugtracker/issues/2590
    - Corecraft: 65% shadow spell coeff / Adding 90% melee and ranged attacker's miss chance auras
    */
    void Test() override
    {
        TestPlayer* priest;
        Guardian* shadowfiend;

        Position spawn(_location);
        spawn.MoveInFront(spawn, 20.0f);
        TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_HUMAN, 70, spawn);
        warrior->SetMaxHealth(std::numeric_limits<int32>::max());
        warrior->SetFullHealth();

        BEFORE_EACH([&] {
            priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);
            priest->DisableRegeneration(true);
            EQUIP_NEW_ITEM(priest, 34336); // Sunflare -- 292 SP

            TEST_CAST(priest, warrior, ClassSpells::Priest::SHADOWFIEND_RNK_1);
            shadowfiend = priest->GetGuardianPet();
            TEST_ASSERT(shadowfiend != nullptr);
        });

        AFTER_EACH([&] {
            priest->KillSelf();
        });

        SECTION("Cost & Cooldown", [&] {
            uint32 const expectedShadowFiendMana = 157;
            TEST_POWER_COST(priest, ClassSpells::Priest::SHADOWFIEND_RNK_1, POWER_MANA, expectedShadowFiendMana);
            TEST_HAS_COOLDOWN(priest, ClassSpells::Priest::SHADOWFIEND_RNK_1, Minutes(5));
        });

        // Summon in melee range of target
        SECTION("Summon in melee", [&] {
            TEST_ASSERT(warrior->GetDistance(shadowfiend) < 5.0f);
        });

        SECTION("Attacking target", [&] {
            WaitNextUpdate(); //wait for shadowfiend to acquire target
            TEST_ASSERT(shadowfiend->GetVictim() == warrior);
        });

        // Damage coeff + Mana returned is 2.5 * damage + damage is shadow
        uint32 const spellBonus = 292 * 0.65f;
        uint32 const expectedShadowfiendMin = ClassSpellsDamage::Priest::SHADOWFIEND_RNK_1_MIN + spellBonus;
        uint32 const expectedShadowfiendMax = ClassSpellsDamage::Priest::SHADOWFIEND_RNK_1_MAX + spellBonus;

        SECTION("Damage & Mana returned", [&] {
            auto[sampleSize, absoluteAllowedError] = _GetApproximationParams(expectedShadowfiendMin, expectedShadowfiendMax);
            auto AI = priest->GetTestingPlayerbotAI();

            uint32 minDamage = std::numeric_limits<uint32>::max();
            uint32 maxDamage = 0;

            shadowfiend->ForceMeleeHitResult(MELEE_HIT_NORMAL);
            AI->ResetSpellCounters();
            uint32 const startingMana = 1;
            for (uint32 i = 0; i < sampleSize; i++)
            {
                priest->SetPower(POWER_MANA, startingMana);
                shadowfiend->AttackerStateUpdate(warrior, BASE_ATTACK);

                auto damageToTarget = AI->GetMeleeDamageDoneInfo(warrior);
                TEST_ASSERT(damageToTarget->size() == i + 1);
                auto& data = damageToTarget->back();

                TEST_ASSERT(data.damageInfo.Damages[0].DamageSchoolMask == SPELL_SCHOOL_MASK_SHADOW);

                uint32 damage = data.damageInfo.Damages[0].Damage;
                uint32 expectedPriestCurrentMana = startingMana + damage * 2.5f;
                TEST_ASSERT(priest->GetPower(POWER_MANA) == expectedPriestCurrentMana);

                minDamage = std::min(minDamage, damage);
                maxDamage = std::max(maxDamage, damage);
                HandleThreadPause();
            }

            uint32 allowedMin = expectedShadowfiendMin > absoluteAllowedError ? expectedShadowfiendMin - absoluteAllowedError : 0; //protect against underflow
            uint32 allowedMax = expectedShadowfiendMax + absoluteAllowedError;

            TEST_ASSERT(expectedShadowfiendMax <= allowedMax);
            TEST_ASSERT(expectedShadowfiendMin >= allowedMin);
            shadowfiend->ResetForceMeleeHitResult();
        });

        SECTION("Shadowfiend stats", STATUS_KNOWN_BUG, [&] {
            // Has Shadow Armor, 90% dodge
            // TODO: sources on this?
            TEST_HAS_AURA(shadowfiend, 34424);
            TEST_ASSERT(warrior->GetUnitDodgeChance(BASE_ATTACK, shadowfiend) >= 90.0f);
            TEST_ASSERT(warrior->GetUnitDodgeChance(RANGED_ATTACK, shadowfiend) >= 90.0f);

            // Resistance: 150 but 250 for shadow
            TEST_ASSERT(shadowfiend->GetResistance(SPELL_SCHOOL_ARCANE) >= 150);
            TEST_ASSERT(shadowfiend->GetResistance(SPELL_SCHOOL_FIRE) >= 150);
            TEST_ASSERT(shadowfiend->GetResistance(SPELL_SCHOOL_FROST) >= 150);
            TEST_ASSERT(shadowfiend->GetResistance(SPELL_SCHOOL_NATURE) >= 150);
            TEST_ASSERT(shadowfiend->GetResistance(SPELL_SCHOOL_SHADOW) >= 250);

            // Attack speed 1.5
            TEST_ASSERT(shadowfiend->GetAttackTime(BASE_ATTACK) == 1500);
        });
    }
};

class TouchOfWeaknessTest : public TestCaseScript
{
public:
    TouchOfWeaknessTest() : TestCaseScript("spells priest touch_of_weakness") { }

    //The next melee attack on the caster will cause 80 Shadow damage and reduce the damage caused by the attacker by 35 for 2min.
    class TouchOfWeaknessTestImpt : public TestCase
    {
    public:


        void Test() override        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);
            TestPlayer* mage = SpawnPlayer(CLASS_MAGE, RACE_HUMAN);
            TestPlayer* rogue = SpawnPlayer(CLASS_ROGUE, RACE_HUMAN);
            priest->ForceSpellHitResultOverride(SPELL_MISS_NONE);

            EQUIP_NEW_ITEM(priest, 34336); // Sunflare - 292 SP

            // Priest cast hex on rogue
            // Mana cost, aura
            uint32 const expectedTouchOfWeaknessMana = 235;
            TEST_POWER_COST(priest, ClassSpells::Priest::TOUCH_OF_WEAKNESS_RNK_7, POWER_MANA, expectedTouchOfWeaknessMana);
            FORCE_CAST(priest, priest, ClassSpells::Priest::TOUCH_OF_WEAKNESS_RNK_7);
            TEST_AURA_MAX_DURATION(priest, ClassSpells::Priest::TOUCH_OF_WEAKNESS_RNK_7, Minutes(10));

            // Shouldn't proc on spells
            FORCE_CAST(mage, priest, ClassSpells::Mage::ICE_LANCE_RNK_1, SPELL_MISS_NONE, TriggerCastFlags(TRIGGERED_FULL_MASK | TRIGGERED_TREAT_AS_NON_TRIGGERED));
            TEST_HAS_AURA(priest, ClassSpells::Priest::TOUCH_OF_WEAKNESS_RNK_7);
            TEST_HAS_NOT_AURA(mage, ClassSpells::Priest::TOUCH_OF_WEAKNESS_RNK_7_TRIGGER_AURA);

            // Should proc on melee
            rogue->ForceMeleeHitResult(MELEE_HIT_NORMAL);
            rogue->AttackerStateUpdate(priest);
            Wait(1000);
            rogue->AttackStop();
            rogue->ResetForceMeleeHitResult();
            TEST_HAS_NOT_AURA(priest, ClassSpells::Priest::TOUCH_OF_WEAKNESS_RNK_7);
            TEST_HAS_AURA(rogue, ClassSpells::Priest::TOUCH_OF_WEAKNESS_RNK_7_TRIGGER_AURA);
            TEST_AURA_MAX_DURATION(rogue, ClassSpells::Priest::TOUCH_OF_WEAKNESS_RNK_7_TRIGGER_AURA, Minutes(2));

            // Test damage reduc ON MH, OH, spells
            {
                Creature* dummy = SpawnCreature();
                EQUIP_NEW_ITEM(rogue, 32837); // Warglaive of Azzinoth MH
                Wait(1500);
                EQUIP_NEW_ITEM(rogue, 32838); // Warglaive of Azzinoth OH
                WaitNextUpdate();

                // Sinister strike
                int const touchOfWeaknessMalus = 35;
                int const sinisterStrikeBonus = 98;
                float const normalizedSwordSpeed = 2.4f;
                auto[minSinisterStrike, maxSinisterStrike] = CalcMeleeDamage(rogue, dummy, BASE_ATTACK, sinisterStrikeBonus - touchOfWeaknessMalus, normalizedSwordSpeed);
                TEST_DIRECT_SPELL_DAMAGE(rogue, dummy, ClassSpells::Rogue::SINISTER_STRIKE_RNK_10, minSinisterStrike, maxSinisterStrike, false);

                // Melee
                auto[minMHMelee, maxMHMelee] = CalcMeleeDamage(rogue, dummy, BASE_ATTACK, -touchOfWeaknessMalus);
                TEST_MELEE_DAMAGE(rogue, dummy, BASE_ATTACK, minMHMelee, maxMHMelee, false);
                auto[minOHMelee, maxOHMelee] = CalcMeleeDamage(rogue, dummy, OFF_ATTACK, -touchOfWeaknessMalus);
                TEST_MELEE_DAMAGE(rogue, dummy, OFF_ATTACK, minOHMelee, maxOHMelee, false);
            }

            // Proc damage
            Creature* dummy = SpawnCreature();
            float const touchOfWeaknessCoeff = ClassSpellsCoeff::Priest::TOUCH_OF_WEAKNESS;
            uint32 const spellBonus = 292 * touchOfWeaknessCoeff;
            uint32 const touchOfWeaknessDmg = ClassSpellsDamage::Priest::TOUCH_OF_WEAKNESS_RNK_7 + spellBonus;
            TEST_DIRECT_SPELL_DAMAGE(priest, dummy, ClassSpells::Priest::TOUCH_OF_WEAKNESS_RNK_7_TRIGGER_AURA, touchOfWeaknessDmg, touchOfWeaknessDmg, false);
            TEST_DIRECT_SPELL_DAMAGE(priest, dummy, ClassSpells::Priest::TOUCH_OF_WEAKNESS_RNK_7_TRIGGER_AURA, touchOfWeaknessDmg * 1.5f, touchOfWeaknessDmg * 1.5f, true);
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<TouchOfWeaknessTestImpt>();
    }
};

class ShadowguardTest : public TestCaseScript
{
public:
    ShadowguardTest() : TestCaseScript("spells priest shadowguard") { }

    class ShadowguardTestImpt : public TestCase
    {
        void Test() override
        {
            SECTION("WIP", STATUS_WIP, [&] {
                //TODO
            });
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<ShadowguardTestImpt>();
    }
};


void AddSC_test_spells_priest()
{
    // Discipline: 14/14
    RegisterTestCase("spells priest consume_magic", ConsumeMagicTest);  
    new DispelMagicTest();
    new FearWardTest();
    new FeedbackTest();
    new InnerFireTest();
    new LevitateTest();
    new ManaBurnTest();
    RegisterTestCase("spells priest mass_dispel", MassDispelTest);
    new PowerWordFortitudeTest();
    new PowerWordShieldTest();
    new PrayerOfFortitudeTest();
    new ShackleUndeadTest();
    new StarshardsTest();
    new SymbolOfHopeTest();
    // Holy: 16/16
    new AbolishDiseaseTest();
    new BindingHealTest();
    new ChastiseTest();
    new CureDiseaseTest();
    new DesperatePrayerTest();
    new ElunesGraceTest();
    new FlashHealTest();
    new GreaterHealTest();
    new HealTest();
    new HolyFireTest();
    new LesserHealTest();
    new PrayerOfHealingTest();
    new PrayerOfMendingTest();
    new RenewTest();
    new ResurrectionTest();
    new SmiteTest();
    // Shadow: 15/16
    new DevouringPlagueTest();
    new FadeTest();
    new HexOfWeaknessTest();
    new MindBlastTest();
    new MindControlTest();
    new MindSootheTest();
    new MindVisionTest();
    new PrayerOfShadowProtectionTest();
    new PsychicScreamTest();
    new ShadowProtectionTest();
    new ShadowWordDeathTest();
    new ShadowWordPainTest();
    RegisterTestCase("spells priest shadowfiend", ShadowfiendTest);
    new TouchOfWeaknessTest();
    new ShadowguardTest();
}