#include "../ClassSpellsDamage.h"
#include "../ClassSpellsCoeff.h"
#include "PlayerbotAI.h"

class DispelMagicTest : public TestCaseScript
{
public:
    DispelMagicTest() : TestCaseScript("spells priest dispel_magic") { }

    class DispelMagicTestImpt : public TestCase
    {
    public:
        DispelMagicTestImpt() : TestCase(STATUS_KNOWN_BUG, true) { }

        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);
            TestPlayer* enemy1 = SpawnPlayer(CLASS_PRIEST, RACE_HUMAN);
            TestPlayer* enemy2 = SpawnPlayer(CLASS_DRUID, RACE_NIGHTELF);

            // Fail -- Bug here
            TEST_CAST(priest, priest, ClassSpells::Priest::DISPEL_MAGIC_RNK_2, SPELL_FAILED_NOTHING_TO_DISPEL, TRIGGERED_IGNORE_GCD);
            TEST_CAST(priest, enemy1, ClassSpells::Priest::DISPEL_MAGIC_RNK_2, SPELL_FAILED_NOTHING_TO_DISPEL, TRIGGERED_IGNORE_GCD);

            // Setup
            TEST_CAST(enemy1, enemy1, ClassSpells::Priest::DIVINE_SPIRIT_RNK_5, SPELL_CAST_OK, TRIGGERED_IGNORE_GCD);
            TEST_CAST(enemy1, enemy1, ClassSpells::Priest::FEAR_WARD_RNK_1, SPELL_CAST_OK, TRIGGERED_IGNORE_GCD);
            TEST_CAST(enemy1, enemy1, ClassSpells::Priest::INNER_FIRE_RNK_7, SPELL_CAST_OK, TRIGGERED_IGNORE_GCD);
            TEST_CAST(enemy1, priest, ClassSpells::Priest::SHADOW_WORD_PAIN_RNK_10, SPELL_CAST_OK, TRIGGERED_IGNORE_GCD);
            TEST_CAST(enemy2, priest, ClassSpells::Druid::MOONFIRE_RNK_12, SPELL_CAST_OK, TRIGGERED_IGNORE_GCD);
            TEST_CAST(enemy2, priest, ClassSpells::Druid::INSECT_SWARM_RNK_6, SPELL_CAST_OK, TRIGGERED_IGNORE_GCD);

            // Offensive dispel
            uint32 const expectedDispelMagicMana = 366;
            TEST_POWER_COST(priest, priest, ClassSpells::Priest::DISPEL_MAGIC_RNK_2, POWER_MANA, expectedDispelMagicMana);
            TEST_HAS_NOT_AURA(priest, ClassSpells::Druid::INSECT_SWARM_RNK_6);
            TEST_HAS_NOT_AURA(priest, ClassSpells::Druid::MOONFIRE_RNK_12);
            TEST_HAS_AURA(priest, ClassSpells::Priest::SHADOW_WORD_PAIN_RNK_10);
            // Defensive dispel
            TEST_CAST(priest, priest, ClassSpells::Priest::DISPEL_MAGIC_RNK_2);
            TEST_HAS_NOT_AURA(priest, ClassSpells::Priest::INNER_FIRE_RNK_7);
            TEST_HAS_NOT_AURA(priest, ClassSpells::Priest::FEAR_WARD_RNK_1);
            TEST_HAS_AURA(priest, ClassSpells::Priest::DIVINE_SPIRIT_RNK_5);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<DispelMagicTestImpt>();
    }
};

class FearWardTest : public TestCaseScript
{
public:
    FearWardTest() : TestCaseScript("spells priest fear_ward") { }

    class FearWardTestImpt : public TestCase
    {
    public:
        FearWardTestImpt() : TestCase(STATUS_PASSING, true) { }

        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_HUMAN);

            // Mana cost
            uint32 const expectedFearWardMana = 78;
            TEST_POWER_COST(priest, priest, ClassSpells::Priest::FEAR_WARD_RNK_1, POWER_MANA, expectedFearWardMana);

            // Aura
            TEST_AURA_MAX_DURATION(priest, ClassSpells::Priest::FEAR_WARD_RNK_1, EFFECT_0, 3 * MINUTE * IN_MILLISECONDS);

            // Cooldown
            TEST_HAS_COOLDOWN(priest, ClassSpells::Priest::FEAR_WARD_RNK_1, 3 * MINUTE);

            // Fear
            TEST_CAST(warlock, priest, ClassSpells::Warlock::FEAR_RNK_3, SPELL_CAST_OK, TRIGGERED_CAST_DIRECTLY);
            TEST_HAS_NOT_AURA(priest, ClassSpells::Priest::FEAR_WARD_RNK_1);
            TEST_CAST(warlock, priest, ClassSpells::Warlock::FEAR_RNK_3, SPELL_CAST_OK, TRIGGERED_CAST_DIRECTLY);
            TEST_HAS_AURA(priest, ClassSpells::Warlock::FEAR_RNK_3);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<FearWardTestImpt>();
    }
};

class InnerFireTest : public TestCaseScript
{
public:
    InnerFireTest() : TestCaseScript("spells priest inner_fire") { }

    class InnerFireTestImpt : public TestCase
    {
    public:
        InnerFireTestImpt() : TestCase(STATUS_PASSING, true) { }

        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);
            TestPlayer* rogue = SpawnPlayer(CLASS_ROGUE, RACE_HUMAN);

            priest->DisableRegeneration(true);

            uint32 innerFireBonus = 1580;
            uint32 expectedArmor = priest->GetArmor() + innerFireBonus;

            // Mana cost
            uint32 const expectedInnerFireMana = 375;
            TEST_POWER_COST(priest, priest, ClassSpells::Priest::INNER_FIRE_RNK_7, POWER_MANA, expectedInnerFireMana);

            // Aura
            TEST_AURA_MAX_DURATION(priest, ClassSpells::Priest::INNER_FIRE_RNK_7, EFFECT_0, 10 * MINUTE * IN_MILLISECONDS);

            // Charges
            TEST_AURA_CHARGE(priest, ClassSpells::Priest::INNER_FIRE_RNK_7, EFFECT_0, 20);

            // Armor
            TEST_ASSERT(priest->GetArmor() == expectedArmor);

            // Lose charge
            auto AI = rogue->GetTestingPlayerbotAI();
            int hits = 0;
            for (uint32 i = 0; i < 150; i++)
            {
                rogue->AttackerStateUpdate(priest, BASE_ATTACK);
                auto damageToTarget = AI->GetWhiteDamageDoneInfo(priest);
                TEST_ASSERT(damageToTarget->size() == i + 1);
                auto& data = damageToTarget->back();
                if (data.damageInfo.hitOutCome != MELEE_HIT_NORMAL && data.damageInfo.hitOutCome != MELEE_HIT_CRIT)
                    continue;
                hits++;
                Aura* aura = priest->GetAura(ClassSpells::Priest::INNER_FIRE_RNK_7, EFFECT_0);
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

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<InnerFireTestImpt>();
    }
};

class LevitateTest : public TestCaseScript
{
public:
    LevitateTest() : TestCaseScript("spells priest levitate") { }

    class LevitateTestImpt : public TestCase
    {
    public:
        LevitateTestImpt() : TestCase(STATUS_PASSING, true) { }

        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);
            TestPlayer* rogue = SpawnPlayer(CLASS_ROGUE, RACE_HUMAN);

            priest->DisableRegeneration(true);

            // Mana cost
            uint32 const expectedLevitateMana = 100;
            priest->AddItem(17056, 1); // Reagent Light Feather
            TEST_POWER_COST(priest, priest, ClassSpells::Priest::LEVITATE_RNK_1, POWER_MANA, expectedLevitateMana);
            TEST_ASSERT(priest->GetItemCount(17056, false) == 0);

            // Aura
            TEST_AURA_MAX_DURATION(priest, ClassSpells::Priest::LEVITATE_RNK_1, EFFECT_0, 2 * MINUTE * IN_MILLISECONDS);

            // Cancel on damage taken
            rogue->Attack(priest, true);
            Wait(500);
            rogue->AttackStop();
            TEST_ASSERT(!priest->HasAura(ClassSpells::Priest::LEVITATE_RNK_1));

            // TODO: over water, slow fall
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<LevitateTestImpt>();
    }
};

class ManaBurnTest : public TestCaseScript
{
public:
    ManaBurnTest() : TestCaseScript("spells priest mana_burn") { }

    class ManaBurnTestImpt : public TestCase
    {
    public:
        ManaBurnTestImpt() : TestCase(STATUS_PASSING, true) { }

        void TestManaBurn(TestPlayer* priest, Unit* victim, uint32 expectedMin, uint32 expectedMax)
        {
            uint32 sampleSize;
            uint32 maxPredictionError;
            _GetApproximationParams(sampleSize, maxPredictionError, expectedMin, expectedMax);

            EnableCriticals(priest, true);

            uint32 victimMaxHealth = victim->GetMaxHealth();
            uint32 victimMaxMana = victim->GetMaxPower(POWER_MANA);

            uint32 minBurn = std::numeric_limits<uint32>::max();
            uint32 maxBurn = 0;

            priest->ForceSpellHitResult(SPELL_MISS_NONE);
            for (uint32 i = 0; i < sampleSize; i++)
            {
                TEST_CAST(priest, victim, ClassSpells::Priest::MANA_BURN_RNK_7, SPELL_CAST_OK, TRIGGERED_FULL_MASK);

                uint32 manaBurnt = victimMaxMana - victim->GetPower(POWER_MANA);
                minBurn = std::min(minBurn, manaBurnt);
                maxBurn = std::max(maxBurn, manaBurnt);

                uint32 expectedHealth = victimMaxHealth - manaBurnt / 2;
                ASSERT_INFO("Health: %u, expected: %u", victim->GetHealth(), expectedHealth);
                TEST_ASSERT(victim->GetHealth() == expectedHealth);

                // Reset
                victim->SetFullHealth();
                victim->SetPower(POWER_MANA, victim->GetMaxPower(POWER_MANA));
                Wait(10);
            }
            priest->ResetForceSpellHitResult();
        }

        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);
            TestPlayer* mage = SpawnPlayer(CLASS_MAGE, RACE_HUMAN);
            TestPlayer* rogue = SpawnPlayer(CLASS_ROGUE, RACE_HUMAN);

            EQUIP_ITEM(priest, 34336); // Sunflare - 292 SP
            priest->DisableRegeneration(true);
            mage->DisableRegeneration(true);
            Wait(5000);

            // Fail
            TEST_CAST(priest, rogue, ClassSpells::Priest::MANA_BURN_RNK_7, SPELL_FAILED_BAD_TARGETS);

            // Mana cost
            uint32 const expectedManaBurnMana = 355;
            TEST_POWER_COST(priest, mage, ClassSpells::Priest::MANA_BURN_RNK_7, POWER_MANA, expectedManaBurnMana);

            // Nothing on rogue
            TEST_ASSERT(rogue->GetHealth() == rogue->GetMaxHealth());

            TestManaBurn(priest, mage, ClassSpellsDamage::Priest::MANA_BURN_RNK_7_MIN, ClassSpellsDamage::Priest::MANA_BURN_RNK_7_MAX);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<ManaBurnTestImpt>();
    }
};

class MassDispelTest : public TestCaseScript
{
public:
    MassDispelTest() : TestCaseScript("spells priest mass_dispel") { }

    class MassDispelTestImpt : public TestCase
    {
    public:
        MassDispelTestImpt() : TestCase(STATUS_INCOMPLETE, true) { }

        TestPlayer* SpawnShaman(Races race, Position spawn)
        {
            TestPlayer* shaman = SpawnPlayer(CLASS_SHAMAN, race, 70, spawn);
            EQUIP_ITEM(shaman, 28523); // Totem
            TEST_CAST(shaman, shaman, ClassSpells::Shaman::MANA_SPRING_TOTEM_RNK_5, SPELL_CAST_OK, TRIGGERED_FULL_MASK);
            TEST_CAST(shaman, shaman, ClassSpells::Shaman::GRACE_OF_AIR_TOTEM_RNK_3, SPELL_CAST_OK, TRIGGERED_FULL_MASK);
            TEST_CAST(shaman, shaman, ClassSpells::Shaman::STRENGTH_OF_EARTH_TOTEM_RNK_6, SPELL_CAST_OK, TRIGGERED_FULL_MASK);
            TEST_CAST(shaman, shaman, ClassSpells::Shaman::FROST_RESISTANCE_TOTEM_RNK_4, SPELL_CAST_OK, TRIGGERED_FULL_MASK);
            return shaman;
        }

        TestPlayer* SpawnHunter(Races race, Position spawn)
        {
            TestPlayer* hunter = SpawnPlayer(CLASS_HUNTER, race, 70, spawn);
            hunter->SummonPet(20673, hunter->GetPositionX(), hunter->GetPositionY(), hunter->GetPositionZ(), 0.0f, HUNTER_PET, 0); // Wind Serpent
            Pet* pet = hunter->GetPet();
            TEST_ASSERT(pet != nullptr);
            return hunter;
        }

        void Test() override
        {
            // Creature
            Creature* creature = SpawnCreature(6);

            // Priest
            TestPlayer* priestA = SpawnPlayer(CLASS_PRIEST, RACE_HUMAN);
            TestPlayer* priestH = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);

            // Setup
            Position spawn(_location);
            // Paladin
            spawn.MoveInFront(spawn, 3.0f);
            TestPlayer* paladinA = SpawnPlayer(CLASS_PALADIN, RACE_HUMAN, 70, spawn);
            TestPlayer* paladinH = SpawnPlayer(CLASS_PALADIN, RACE_BLOODELF, 70, spawn);
            // Mage
            spawn.MoveInFront(spawn, 3.0f);
            TestPlayer* mageA = SpawnPlayer(CLASS_MAGE, RACE_HUMAN, 70, spawn);
            TestPlayer* mageH = SpawnPlayer(CLASS_MAGE, RACE_BLOODELF, 70, spawn);
            // Warlock
            spawn.MoveInFront(spawn, 3.0f);
            TestPlayer* warlockA = SpawnPlayer(CLASS_WARLOCK, RACE_HUMAN, 70, spawn);
            TestPlayer* warlockH = SpawnPlayer(CLASS_WARLOCK, RACE_BLOODELF, 70, spawn);
            // Shaman
            spawn.MoveInFront(spawn, 3.0f);
            TestPlayer* shamanA = SpawnShaman(RACE_DRAENEI, spawn);
            TestPlayer* shamanH = SpawnShaman(RACE_ORC, spawn);
            // Hunter
            spawn.MoveInFront(spawn, 3.0f);
            TestPlayer* hunterA = SpawnHunter(RACE_DWARF, spawn);
            TestPlayer* hunterH = SpawnHunter(RACE_ORC, spawn);
            // Warrior
            spawn.MoveInFront(spawn, 3.0f);
            TestPlayer* warriorA = SpawnPlayer(CLASS_WARRIOR, RACE_HUMAN, 70, spawn);
            TestPlayer* warriorH = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN, 70, spawn);
            // Druid
            spawn.MoveInFront(spawn, 3.0f);
            TestPlayer* druidA = SpawnPlayer(CLASS_DRUID, RACE_NIGHTELF, 70, spawn);
            TestPlayer* druidH = SpawnPlayer(CLASS_DRUID, RACE_TAUREN, 70, spawn);
            // Rogue
            spawn.MoveInFront(spawn, 3.0f);
            TestPlayer* rogueA = SpawnPlayer(CLASS_ROGUE, RACE_NIGHTELF, 70, spawn);
            TestPlayer* rogueH = SpawnPlayer(CLASS_ROGUE, RACE_BLOODELF, 70, spawn);
            // RIP
            spawn.MoveInFront(spawn, 3.0f);
            TestPlayer* ripA = SpawnPlayer(CLASS_ROGUE, RACE_HUMAN, 70, spawn);
            TestPlayer* ripH = SpawnPlayer(CLASS_ROGUE, RACE_ORC, 70, spawn);

            float const dist = 35.0f;
            uint32 const GAS_NOVA = 45855;

            // Get all the units
            std::vector<WorldObject*> targets;
            Trinity::AllWorldObjectsInRange u_check(creature, dist);
            Trinity::WorldObjectListSearcher<Trinity::AllWorldObjectsInRange> searcher(creature, targets, u_check);
            Cell::VisitAllObjects(creature, searcher, dist);

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

            // Buff horde raid
            uint32 const SACRED_CANDLE = 17029;
            priestH->AddItem(17029, 2);
            TEST_CAST(priestH, priestH, ClassSpells::Priest::PRAYER_OF_FORTITUDE_RNK_3, SPELL_CAST_OK, TriggerCastFlags(TRIGGERED_CAST_DIRECTLY | TRIGGERED_IGNORE_POWER_AND_REAGENT_COST));
            TEST_CAST(priestH, hunterH, ClassSpells::Priest::PRAYER_OF_FORTITUDE_RNK_3, SPELL_CAST_OK, TriggerCastFlags(TRIGGERED_CAST_DIRECTLY | TRIGGERED_IGNORE_POWER_AND_REAGENT_COST));
            TEST_CAST(priestH, ripH, ClassSpells::Priest::PRAYER_OF_FORTITUDE_RNK_3, SPELL_CAST_OK, TriggerCastFlags(TRIGGERED_CAST_DIRECTLY | TRIGGERED_IGNORE_POWER_AND_REAGENT_COST));

            // Gas Nova for all
            TEST_CAST(creature, creature, GAS_NOVA, SPELL_CAST_OK, TRIGGERED_FULL_MASK);
            // Everybody has the aura except Totems and creature
            for (WorldObject* object : targets)
            {
                if (Unit* unit = object->ToUnit())
                {
                    if(unit->IsTotem() || unit == creature)
                        TEST_HAS_NOT_AURA(unit, GAS_NOVA)
                    else
                        TEST_HAS_AURA(unit, GAS_NOVA)

                    // Buff priest on horde players & pets
                    if (Player* player = object->ToPlayer())
                    {
                        if (player->GetTeam() == Team::HORDE)
                        {   
                            TEST_HAS_AURA(player, ClassSpells::Priest::PRAYER_OF_FORTITUDE_RNK_3);
                            if (Pet* pet = player->GetPet())
                                TEST_HAS_AURA(pet, ClassSpells::Priest::PRAYER_OF_FORTITUDE_RNK_3);
                        }
                    }
                }
            }

            // Mass dispell
            // Alliance: no more gas nova except ripA
            // Horde: gas nova but no more priest buff expect ripH
            TEST_CAST(priestA, shamanA, ClassSpells::Priest::MASS_DISPEL_RNK_1);
            for (WorldObject* object : targets)
            {
                if (Player* player = object->ToPlayer())
                {
                    if (player->GetTeam() == Team::ALLIANCE)
                    {
                        // 10 allies were already dispelled
                        if (player == ripA)
                        {
                            TEST_HAS_AURA(player, GAS_NOVA);
                            continue;
                        }
                        TEST_HAS_NOT_AURA(player, GAS_NOVA)
                        if (Pet* pet = player->ToPet())
                            TEST_HAS_NOT_AURA(pet, GAS_NOVA)
                    }
                    else
                    {
                        // 10 enemies were already dispelled
                        if (player == ripH)
                        {
                            TEST_HAS_AURA(player, GAS_NOVA);
                            TEST_HAS_AURA(player, ClassSpells::Priest::PRAYER_OF_FORTITUDE_RNK_3);
                            continue;
                        }
                        TEST_HAS_AURA(player, GAS_NOVA)
                        TEST_HAS_NOT_AURA(player, ClassSpells::Priest::PRAYER_OF_FORTITUDE_RNK_3);
                        if (Pet* pet = player->ToPet())
                        {
                            TEST_HAS_AURA(pet, GAS_NOVA)
                            TEST_HAS_NOT_AURA(pet, ClassSpells::Priest::PRAYER_OF_FORTITUDE_RNK_3);

                        }
                    }
                }
            }

            // Remove Ice Block, Divine Shield, some banish?
            TEST_CAST(mageH, mageH, ClassSpells::Mage::ICE_BLOCK_RNK_1);
            TEST_CAST(paladinH, paladinH, ClassSpells::Paladin::DIVINE_SHIELD_RNK_2);
            TEST_CAST(mageA, mageA, ClassSpells::Mage::SUMMON_WATER_ELEMENTAL_RNK_1);
            Pet* elem = mageA->GetPet();
            TEST_ASSERT(elem != nullptr);
            TEST_CAST(warlockH, elem, ClassSpells::Warlock::BANISH_RNK_2, SPELL_CAST_OK, TRIGGERED_CAST_DIRECTLY);

            TEST_HAS_AURA(mageH, ClassSpells::Mage::ICE_BLOCK_RNK_1);
            TEST_HAS_AURA(paladinH, ClassSpells::Paladin::DIVINE_SHIELD_RNK_2);
            TEST_HAS_AURA(elem, ClassSpells::Warlock::BANISH_RNK_2);

            TEST_CAST(priestA, mageH, ClassSpells::Priest::MASS_DISPEL_RNK_1);

            TEST_HAS_NOT_AURA(mageH, ClassSpells::Mage::ICE_BLOCK_RNK_1);
            TEST_HAS_NOT_AURA(paladinH, ClassSpells::Paladin::DIVINE_SHIELD_RNK_2);
            TEST_HAS_NOT_AURA(elem, ClassSpells::Warlock::BANISH_RNK_2);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<MassDispelTestImpt>();
    }
};

class PowerWordFortitudeTest : public TestCaseScript
{
public:
    PowerWordFortitudeTest() : TestCaseScript("spells priest power_word_fortitude") { }

    class PowerWordFortitudeTestImpt : public TestCase
    {
    public:
        PowerWordFortitudeTestImpt() : TestCase(STATUS_PASSING, true) { }

        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_ORC);

            uint32 powerWordFortitudeBonus = 79;
            uint32 expectedHealth = warrior->GetMaxHealth() + powerWordFortitudeBonus * 10;

            // Mana cost
            uint32 const expectedPowerWordFortitudeMana = 700;
            TEST_POWER_COST(priest, warrior, ClassSpells::Priest::POWER_WORD_FORTITUDE_RNK_7, POWER_MANA, expectedPowerWordFortitudeMana);

            // Health
            ASSERT_INFO("Health: %u, expected: %u", warrior->GetHealth(), expectedHealth);
            TEST_ASSERT(warrior->GetMaxHealth() == expectedHealth);

            // Aura
            TEST_AURA_MAX_DURATION(warrior, ClassSpells::Priest::POWER_WORD_FORTITUDE_RNK_7, EFFECT_0, 30 * MINUTE * IN_MILLISECONDS);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<PowerWordFortitudeTestImpt>();
    }
};

class PowerWordShieldTest : public TestCaseScript
{
public:
    PowerWordShieldTest() : TestCaseScript("spells priest power_word_shield") { }

    class PowerWordShieldTestImpt : public TestCase
    {
    public:
        PowerWordShieldTestImpt() : TestCase(STATUS_PASSING, true) { }

        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);
            TestPlayer* rogue = SpawnPlayer(CLASS_ROGUE, RACE_HUMAN);

            EQUIP_ITEM(priest, 34335); // Hammer of Sanctification -- 550 BH

            uint32 expectedAbsorb = ClassSpellsDamage::Priest::POWER_WORD_SHIELD_RNK_12 + ClassSpellsCoeff::Priest::POWER_WORD_SHIELD * 550;

            // Mana cost
            uint32 const expectedPowerWordShieldMana = 600;
            TEST_POWER_COST(priest, priest, ClassSpells::Priest::POWER_WORD_SHIELD_RNK_12, POWER_MANA, expectedPowerWordShieldMana);

            // Aura
            TEST_AURA_MAX_DURATION(priest, ClassSpells::Priest::POWER_WORD_SHIELD_RNK_12, EFFECT_0, 30 * SECOND * IN_MILLISECONDS);
            TEST_AURA_MAX_DURATION(priest, 6788, EFFECT_0, 15 * SECOND * IN_MILLISECONDS); // Weakened Aura

            // Cooldown
            TEST_HAS_COOLDOWN(priest, ClassSpells::Priest::POWER_WORD_SHIELD_RNK_12, 4 * SECOND);

            // Absorb
            priest->SetFullHealth();
            auto AI = rogue->GetTestingPlayerbotAI();
            int totalDamage = 0;
            for (uint32 i = 0; i < 150; i++)
            {
                rogue->AttackerStateUpdate(priest, BASE_ATTACK);
                auto damageToTarget = AI->GetWhiteDamageDoneInfo(priest);

                TEST_ASSERT(damageToTarget->size() == i + 1);
                auto& data = damageToTarget->back();

                if (data.damageInfo.hitOutCome != MELEE_HIT_NORMAL && data.damageInfo.hitOutCome != MELEE_HIT_CRIT)
                    continue;

                totalDamage += data.damageInfo.damage;
                totalDamage += data.damageInfo.resist;
                totalDamage += data.damageInfo.blocked_amount;
                totalDamage += data.damageInfo.absorb;

                if (totalDamage < expectedAbsorb)
                {
                    TEST_HAS_AURA(priest, ClassSpells::Priest::POWER_WORD_SHIELD_RNK_12, EFFECT_0);
                    continue;
                }

                TEST_HAS_NOT_AURA(priest, ClassSpells::Priest::POWER_WORD_SHIELD_RNK_12, EFFECT_0);
                uint32 expectedHealth = priest->GetMaxHealth() - (totalDamage - expectedAbsorb);
                TEST_ASSERT(priest->GetHealth() == expectedHealth);
                break;
            }
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<PowerWordShieldTestImpt>();
    }
};

class PrayerOfFortitudeTest : public TestCaseScript
{
public:
    PrayerOfFortitudeTest() : TestCaseScript("spells priest prayer_of_fortitude") { }

    class PrayerOfFortitudeTestImpt : public TestCase
    {
    public:
        PrayerOfFortitudeTestImpt() : TestCase(STATUS_PASSING, true) { }

        void TestPrayerOfFortitude(TestPlayer* priest, Unit* warrior, uint32 spellId, uint32 reagentId, uint32 manaCost, uint32 staminaBonus, uint32 priestStartHealth, uint32 warriorStartHealth)
        {
            uint32 expectedPriestHealth = priestStartHealth + staminaBonus * 10;
            uint32 expectedWarriorHealth = warriorStartHealth + staminaBonus * 10;

            // Mana cost
            priest->AddItem(reagentId, 1); // Reagent
            TEST_POWER_COST(priest, warrior, spellId, POWER_MANA, manaCost);
            TEST_ASSERT(priest->GetItemCount(reagentId, false) == 0);

            // Aura
            TEST_AURA_MAX_DURATION(warrior, spellId, EFFECT_0, 1 * HOUR * IN_MILLISECONDS);
            TEST_AURA_MAX_DURATION(priest, spellId, EFFECT_0, 1 * HOUR * IN_MILLISECONDS);

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

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<PrayerOfFortitudeTestImpt>();
    }
};

class ShackleUndeadTest : public TestCaseScript
{
public:
    ShackleUndeadTest() : TestCaseScript("spells priest shackle_undead") { }

    class ShackleUndeadTestImpt : public TestCase
    {
    public:
        ShackleUndeadTestImpt() : TestCase(STATUS_PASSING, true) { }

        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);
            Creature* creature0 = SpawnCreature(6, true);

            Position spawn(_location);
            spawn.MoveInFront(spawn, 20.0f);
            Creature* creature1 = SpawnCreatureWithPosition(spawn, 16525); // Undead
            Creature* creature2 = SpawnCreatureWithPosition(spawn, 16525); // Undead

            // Fail
            TEST_CAST(priest, creature0, ClassSpells::Priest::SHACKLE_UNDEAD_RNK_3, SPELL_FAILED_BAD_TARGETS);

            Wait(3000);

            // Mana cost
            uint32 const expectedShackleUndeadMana = 150;
            TEST_POWER_COST(priest, creature1, ClassSpells::Priest::SHACKLE_UNDEAD_RNK_3, POWER_MANA, expectedShackleUndeadMana);

            // Aura
            TEST_AURA_MAX_DURATION(creature1, ClassSpells::Priest::SHACKLE_UNDEAD_RNK_3, EFFECT_0, 50 * SECOND * IN_MILLISECONDS);

            creature2->Attack(priest, false);
            for (int count = 0; count < 10; count++)
            {
                if (!creature2->IsMovementPreventedByCasting())
                {
                    Wait(1000);
                    continue;
                }
                TEST_CAST(priest, creature2, ClassSpells::Priest::SHACKLE_UNDEAD_RNK_3, SPELL_CAST_OK, TriggerCastFlags(TRIGGERED_CAST_DIRECTLY | TRIGGERED_IGNORE_POWER_AND_REAGENT_COST));
                TEST_HAS_AURA(creature2, ClassSpells::Priest::SHACKLE_UNDEAD_RNK_3, EFFECT_0);
                TEST_HAS_NOT_AURA(creature1, ClassSpells::Priest::SHACKLE_UNDEAD_RNK_3, EFFECT_0);
            }
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<ShackleUndeadTestImpt>();
    }
};

class AbolishDiseaseTest : public TestCaseScript
{
public:
    AbolishDiseaseTest() : TestCaseScript("spells priest abolish_disease") { }

    class AbolishDiseaseTestImpt : public TestCase
    {
    public:
        AbolishDiseaseTestImpt() : TestCase(STATUS_KNOWN_BUG, true) { }

        void TestDispelDisease(TestPlayer* victim, uint32 Disease1, uint32 Disease2, uint32 Disease3, uint32 Disease4, uint32 Disease5, int8 count)
        {
            victim->SetFullHealth();
            ASSERT_INFO("TestDispelDisease maximum trials reached");
            TEST_ASSERT(count < 10);
            count++;

            uint32 tickTime = 5 * SECOND * IN_MILLISECONDS;

            if (victim->HasAura(Disease5))
            {
                TEST_ASSERT(victim->HasAura(Disease4));
                TEST_ASSERT(victim->HasAura(Disease3));
                TEST_ASSERT(victim->HasAura(Disease2));
                TEST_ASSERT(victim->HasAura(Disease1));
                Wait(tickTime); // wait for next tick
                TestDispelDisease(victim, Disease1, Disease2, Disease3, Disease4, Disease5, count);
            }
            else
            {
                if (victim->HasAura(Disease4))
                {
                    TEST_ASSERT(victim->HasAura(Disease3));
                    TEST_ASSERT(victim->HasAura(Disease2));
                    TEST_ASSERT(victim->HasAura(Disease1));
                    Wait(tickTime); // wait for next tick
                    TestDispelDisease(victim, Disease1, Disease2, Disease3, Disease4, Disease5, count);
                }
                else
                {
                    if (victim->HasAura(Disease3))
                    {
                        TEST_ASSERT(victim->HasAura(Disease2));
                        TEST_ASSERT(victim->HasAura(Disease1));
                        Wait(tickTime); // wait for next tick
                        TestDispelDisease(victim, Disease1, Disease2, Disease3, Disease4, Disease5, count);
                    }
                    else
                    {
                        if (victim->HasAura(Disease2))
                        {
                            TEST_ASSERT(victim->HasAura(Disease1));
                            Wait(tickTime); // wait for next tick
                            TestDispelDisease(victim, Disease1, Disease2, Disease3, Disease4, Disease5, count);
                        }
                        else
                        {
                            if (victim->HasAura(Disease1))
                            {
                                Wait(tickTime); // wait for next tick
                                TestDispelDisease(victim, Disease1, Disease2, Disease3, Disease4, Disease5, count);
                            }
                        }
                    }
                }
            }
        }

        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN);

            // setup
            priest->DisableRegeneration(true);

            uint32 const WEAKENING_DISEASE = 18633;
            uint32 const SPORE_DISEASE     = 31423;
            uint32 const FEVERED_DISEASE   = 34363;
            uint32 const DISEASE_BUFFET    = 46481;
            uint32 const VOLATILE_DISEASE  = 46483;

            // Apply
            warrior->AddAura(WEAKENING_DISEASE, warrior);
            Wait(1);
            warrior->AddAura(SPORE_DISEASE, warrior);
            Wait(1);
            warrior->AddAura(FEVERED_DISEASE, warrior);
            Wait(1);
            warrior->AddAura(DISEASE_BUFFET, warrior);
            Wait(1);
            warrior->AddAura(VOLATILE_DISEASE, warrior);
            Wait(1);

            // Mana cost
            uint32 const expectedAbolishDiseaseMana = 314;
            TEST_POWER_COST(priest, warrior, ClassSpells::Priest::ABOLISH_DISEASE_RNK_1, POWER_MANA, expectedAbolishDiseaseMana);

            // Aura duration
            TEST_AURA_MAX_DURATION(warrior, ClassSpells::Priest::ABOLISH_DISEASE_RNK_1, EFFECT_0, 20 * SECOND * IN_MILLISECONDS);

            Wait(500);
            int8 count = 0;
            TestDispelDisease(warrior, WEAKENING_DISEASE, SPORE_DISEASE, FEVERED_DISEASE, DISEASE_BUFFET, VOLATILE_DISEASE, count);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<AbolishDiseaseTestImpt>();
    }
};

class BindingHealTest : public TestCaseScript
{
public:
    BindingHealTest() : TestCaseScript("spells priest binding_heal") { }

    class BindingHealTestImpt : public TestCase
    {
    public:
        BindingHealTestImpt() : TestCase(STATUS_PASSING, true) { }

        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);
            TestPlayer* rogue = SpawnPlayer(CLASS_ROGUE, RACE_BLOODELF);

            EQUIP_ITEM(priest, 34335); // Hammer of Sanctification -- 550 BH

            // Mana cost
            uint32 const expectedBindingHealMana = 705;
            TEST_POWER_COST(priest, rogue, ClassSpells::Priest::BINDING_HEAL_RNK_1, POWER_MANA, expectedBindingHealMana);
            
            // Heal
            float const bindingHealCastTime = 1.5f;
            float const bindingHealCoeff = bindingHealCastTime / 3.5f;
            uint32 const bonusHeal = 550 * bindingHealCoeff;
            uint32 const bindingHealMin = ClassSpellsDamage::Priest::BINDING_HEAL_RNK_1_MIN + bonusHeal;
            uint32 const bindingHealMax = ClassSpellsDamage::Priest::BINDING_HEAL_RNK_1_MAX + bonusHeal;
            TEST_DIRECT_HEAL(priest, priest, ClassSpells::Priest::BINDING_HEAL_RNK_1, bindingHealMin, bindingHealMax, false);
            TEST_DIRECT_HEAL(priest, priest, ClassSpells::Priest::BINDING_HEAL_RNK_1, bindingHealMin * 1.5f, bindingHealMax * 1.5f, true);
            TEST_DIRECT_HEAL(priest, rogue, ClassSpells::Priest::BINDING_HEAL_RNK_1, bindingHealMin, bindingHealMax, false);
            TEST_DIRECT_HEAL(priest, rogue, ClassSpells::Priest::BINDING_HEAL_RNK_1, bindingHealMin * 1.5f, bindingHealMax * 1.5f, true);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<BindingHealTestImpt>();
    }
};

class CureDiseaseTest : public TestCaseScript
{
public:
    CureDiseaseTest() : TestCaseScript("spells priest cure_disease") { }

    class CureDiseaseTestImpt : public TestCase
    {
    public:
        CureDiseaseTestImpt() : TestCase(STATUS_KNOWN_BUG, true) { }

        void TestDispelDisease(TestPlayer* victim, uint32 Disease1, uint32 Disease2, uint32 Disease3, uint32 Disease4, uint32 Disease5, int8 count)
        {
            victim->SetFullHealth();
            ASSERT_INFO("TestDispelDisease maximum trials reached");
            TEST_ASSERT(count < 10);
            count++;

            TEST_CAST(victim, victim, ClassSpells::Priest::CURE_DISEASE_RNK_1, SPELL_CAST_OK, TRIGGERED_FULL_MASK);

            if (victim->HasAura(Disease5))
            {
                TEST_ASSERT(victim->HasAura(Disease4));
                TEST_ASSERT(victim->HasAura(Disease3));
                TEST_ASSERT(victim->HasAura(Disease2));
                TEST_ASSERT(victim->HasAura(Disease1));
                TestDispelDisease(victim, Disease1, Disease2, Disease3, Disease4, Disease5, count);
            }
            else
            {
                if (victim->HasAura(Disease4))
                {
                    TEST_ASSERT(victim->HasAura(Disease3));
                    TEST_ASSERT(victim->HasAura(Disease2));
                    TEST_ASSERT(victim->HasAura(Disease1));
                    TestDispelDisease(victim, Disease1, Disease2, Disease3, Disease4, Disease5, count);
                }
                else
                {
                    if (victim->HasAura(Disease3))
                    {
                        TEST_ASSERT(victim->HasAura(Disease2));
                        TEST_ASSERT(victim->HasAura(Disease1));
                        TestDispelDisease(victim, Disease1, Disease2, Disease3, Disease4, Disease5, count);
                    }
                    else
                    {
                        if (victim->HasAura(Disease2))
                        {
                            TEST_ASSERT(victim->HasAura(Disease1));
                            TestDispelDisease(victim, Disease1, Disease2, Disease3, Disease4, Disease5, count);
                        }
                        else
                        {
                            if (victim->HasAura(Disease1))
                            {
                                TestDispelDisease(victim, Disease1, Disease2, Disease3, Disease4, Disease5, count);
                            }
                        }
                    }
                }
            }
        }

        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN);

            // setup
            priest->DisableRegeneration(true);

            uint32 const WEAKENING_DISEASE = 18633;
            uint32 const SPORE_DISEASE = 31423;
            uint32 const FEVERED_DISEASE = 34363;
            uint32 const DISEASE_BUFFET = 46481;
            uint32 const VOLATILE_DISEASE = 46483;

            // Apply
            warrior->AddAura(WEAKENING_DISEASE, warrior);
            Wait(1);
            warrior->AddAura(SPORE_DISEASE, warrior);
            Wait(1);
            warrior->AddAura(FEVERED_DISEASE, warrior);
            Wait(1);
            warrior->AddAura(DISEASE_BUFFET, warrior);
            Wait(1);
            warrior->AddAura(VOLATILE_DISEASE, warrior);
            Wait(1);

            // Mana cost
            uint32 const expectedCureDiseaseMana = 314;
            TEST_POWER_COST(priest, warrior, ClassSpells::Priest::CURE_DISEASE_RNK_1, POWER_MANA, expectedCureDiseaseMana);

            Wait(500);
            int8 count = 0;
            TestDispelDisease(warrior, WEAKENING_DISEASE, SPORE_DISEASE, FEVERED_DISEASE, DISEASE_BUFFET, VOLATILE_DISEASE, count);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<CureDiseaseTestImpt>();
    }
};

class FlashHealTest : public TestCaseScript
{
public:
    FlashHealTest() : TestCaseScript("spells priest greater_heal") { }

    class FlashHealTestImpt : public TestCase
    {
    public:
        FlashHealTestImpt() : TestCase(STATUS_PASSING, true) { }

        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);

            EQUIP_ITEM(priest, 34335); // Hammer of Sanctification -- 550 BH

            // Mana cost
            uint32 const expectedFlashHealMana = 470;
            TEST_POWER_COST(priest, priest, ClassSpells::Priest::FLASH_HEAL_RNK_9, POWER_MANA, expectedFlashHealMana);

            // Heal
            float const greaterHealCastTime = 1.5f;
            float const greaterHealCoeff = greaterHealCastTime / 3.5f;
            uint32 const bonusHeal = 550 * greaterHealCoeff;
            uint32 const greaterHealMin = ClassSpellsDamage::Priest::FLASH_HEAL_RNK_9_MIN + bonusHeal;
            uint32 const greaterHealMax = ClassSpellsDamage::Priest::FLASH_HEAL_RNK_9_MAX + bonusHeal;
            TEST_DIRECT_HEAL(priest, priest, ClassSpells::Priest::FLASH_HEAL_RNK_9, greaterHealMin, greaterHealMax, false);
            TEST_DIRECT_HEAL(priest, priest, ClassSpells::Priest::FLASH_HEAL_RNK_9, greaterHealMin * 1.5f, greaterHealMax * 1.5f, true);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<FlashHealTestImpt>();
    }
};

class GreaterHealTest : public TestCaseScript
{
public:
    GreaterHealTest() : TestCaseScript("spells priest greater_heal") { }

    class GreaterHealTestImpt : public TestCase
    {
    public:
        GreaterHealTestImpt() : TestCase(STATUS_PASSING, true) { }

        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);

            EQUIP_ITEM(priest, 34335); // Hammer of Sanctification -- 550 BH

            // Mana cost
            uint32 const expectedGreaterHealMana = 825;
            TEST_POWER_COST(priest, priest, ClassSpells::Priest::GREATER_HEAL_RNK_7, POWER_MANA, expectedGreaterHealMana);

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

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<GreaterHealTestImpt>();
    }
};

class HealTest : public TestCaseScript
{
public:
    HealTest() : TestCaseScript("spells priest heal") { }

    class HealTestImpt : public TestCase
    {
    public:
        HealTestImpt() : TestCase(STATUS_PASSING, true) { }

        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);

            EQUIP_ITEM(priest, 34335); // Hammer of Sanctification -- 550 BH

            // Mana cost
            uint32 const expectedHealMana = 305;
            TEST_POWER_COST(priest, priest, ClassSpells::Priest::HEAL_RNK_4, POWER_MANA, expectedHealMana);

            // Heal
            int const spellMaxLevel = 39;
            int const spellBaseLevel = 34;
            float const spellRealPointsPerLevel = 4.5;
            float const downrankingCoeff = (spellBaseLevel + 6) / 70.0f; //  (spellMaxLevel + 5) / playerLevel <- downranking coeff formula
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

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<HealTestImpt>();
    }
};

void AddSC_test_spells_priest()
{
    // Discipline: 10/10
    new DispelMagicTest();
    new FearWardTest();
    new InnerFireTest();
    new LevitateTest();
    new ManaBurnTest();
    new MassDispelTest();
    new PowerWordFortitudeTest();
    new PowerWordShieldTest();
    new PrayerOfFortitudeTest();
    new ShackleUndeadTest();
    // Holy: /
    new AbolishDiseaseTest();
    new BindingHealTest();
    new CureDiseaseTest();
    new FlashHealTest();
    new GreaterHealTest();
    new HealTest();
}
