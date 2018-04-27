#include "../ClassSpellsDamage.h"
#include "../ClassSpellsCoeff.h"
#include "PlayerbotAI.h"
#include "SpellHistory.h"

class TestCaseWarrior : public TestCase
{
public:
    TestCaseWarrior(TestStatus status, bool needMap = true) : TestCase(status, true) {}
    TestCaseWarrior(TestStatus status, WorldLocation const& loc) : TestCase(status, loc) {}

    void TestRequiresStance(TestPlayer* warrior, Unit* victim, bool success, uint32 testSpellId, uint32 stanceSpellId = 0, SpellCastResult result = SPELL_FAILED_ONLY_SHAPESHIFT)
    {
        if (stanceSpellId > 0)
        {
            TEST_CAST(warrior, warrior, stanceSpellId);
        }
    
        SpellCastResult res = success ? SPELL_CAST_OK : result;

        ASSERT_INFO("Stance %u", stanceSpellId);
        TEST_CAST(warrior, victim, testSpellId, res, TriggerCastFlags(TRIGGERED_IGNORE_SPELL_AND_CATEGORY_CD | TRIGGERED_IGNORE_POWER_AND_REAGENT_COST | TRIGGERED_IGNORE_CASTER_AURASTATE));
        warrior->GetSpellHistory()->ResetAllCooldowns();
    }

    void TestRequiresMeleeWeapon(TestPlayer* warrior, Unit* victim, uint32 testSpellId, bool twoHand, SpellCastResult result = SPELL_FAILED_EQUIPPED_ITEM_CLASS)
    {
        RemoveAllEquipedItems(warrior);
        warrior->GetSpellHistory()->ResetAllCooldowns();
        TEST_CAST(warrior, victim, testSpellId, result);

        if (twoHand)
        {
            EQUIP_ITEM(warrior, 34247); // Apolyon, the Soul-Render
        }
        else
        {
            EQUIP_ITEM(warrior, 32837); // Warglaive of Azzinoth MH
            Wait(1500);
            EQUIP_ITEM(warrior, 32838); // Warglaive of Azzinoth OH
        }
    }
};

class ChargeTest : public TestCaseScript
{
public:
    ChargeTest() : TestCaseScript("spells warrior charge") { }

    class ChargeTestImpt : public TestCaseWarrior
    {
    public:
        ChargeTestImpt() : TestCaseWarrior(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN);

            Position spawn(_location);
            spawn.MoveInFront(_location, 7.0f);
            TestPlayer* priest1 = SpawnPlayer(CLASS_PRIEST, RACE_HUMAN, 70, spawn);
            spawn.MoveInFront(_location, 15.0f);
            TestPlayer* priest2 = SpawnPlayer(CLASS_PRIEST, RACE_HUMAN, 70, spawn);
            spawn.MoveInFront(_location, 30.0f);
            TestPlayer* priest3 = SpawnPlayer(CLASS_PRIEST, RACE_HUMAN, 70, spawn);

            RemoveAllEquipedItems(warrior);
            warrior->DisableRegeneration(true);
            warrior->SetWalk(false); // run

            // Stances
            TestRequiresStance(warrior, priest2, false, ClassSpells::Warrior::CHARGE_RNK_3);
            TestRequiresStance(warrior, priest2, false, ClassSpells::Warrior::CHARGE_RNK_3, ClassSpells::Warrior::DEFENSIVE_STANCE_RNK_1);
            TestRequiresStance(warrior, priest2, false, ClassSpells::Warrior::CHARGE_RNK_3, ClassSpells::Warrior::BERSERKER_STANCE_RNK_1);
            TestRequiresStance(warrior, priest2, true, ClassSpells::Warrior::CHARGE_RNK_3, ClassSpells::Warrior::BATTLE_STANCE_RNK_1);
            
            warrior->TeleportTo(_location);

            // Range
            TEST_CAST(warrior, priest1, ClassSpells::Warrior::CHARGE_RNK_3, SPELL_FAILED_OUT_OF_RANGE);
            warrior->Relocate(_location);
            TEST_CAST(warrior, priest3, ClassSpells::Warrior::CHARGE_RNK_3, SPELL_FAILED_OUT_OF_RANGE);
            warrior->Relocate(_location);

            // Cast
            TEST_CAST(warrior, priest2, ClassSpells::Warrior::CHARGE_RNK_3);
            // Rage earned
            uint32 expectedRage = 15 * 10;
            TEST_ASSERT(warrior->GetPower(POWER_RAGE) == expectedRage);
            // Stun
            Wait(250);
            TEST_AURA_MAX_DURATION(priest2, ClassSpells::Warrior::CHARGE_RNK_3_STUN, Seconds(1));

            Wait(1000);
            // Warrior still running after charge
            TEST_ASSERT(!warrior->IsWalking());

            // Cannot be used in combat
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<ChargeTestImpt>();
    }
};

class HamstringTest : public TestCaseScript
{
public:
    HamstringTest() : TestCaseScript("spells warrior hamstring") { }

    class HamstringTestImpt : public TestCaseWarrior
    {
    public:
        HamstringTestImpt() : TestCaseWarrior(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* warrior = SpawnRandomPlayer(CLASS_WARRIOR);
            Creature* creature = SpawnCreature(12, true); // No immunes dummy

            warrior->DisableRegeneration(true);
            float const expectedSpeed = creature->GetSpeed(MOVE_RUN) * 0.5f;

            TestRequiresStance(warrior, creature, false, ClassSpells::Warrior::HAMSTRING_RNK_4);
            TestRequiresStance(warrior, creature, false, ClassSpells::Warrior::HAMSTRING_RNK_4, ClassSpells::Warrior::DEFENSIVE_STANCE_RNK_1);
            TestRequiresStance(warrior, creature, true, ClassSpells::Warrior::HAMSTRING_RNK_4, ClassSpells::Warrior::BATTLE_STANCE_RNK_1);
            TestRequiresStance(warrior, creature, true, ClassSpells::Warrior::HAMSTRING_RNK_4, ClassSpells::Warrior::BERSERKER_STANCE_RNK_1);

            TestRequiresMeleeWeapon(warrior, creature, ClassSpells::Warrior::HAMSTRING_RNK_4, true);

            // Rage cost
            uint32 const expectedHamstringRage = 10 * 10;
            TEST_POWER_COST(warrior, creature, ClassSpells::Warrior::HAMSTRING_RNK_4, POWER_RAGE, expectedHamstringRage);

            // Duration & CD
            TEST_AURA_MAX_DURATION(creature, ClassSpells::Warrior::HAMSTRING_RNK_4, Seconds(15));

            // Speed
            TEST_ASSERT(creature->GetSpeed(MOVE_RUN) == expectedSpeed);

            // Damage
            float const armorFactor = 1 - (creature->GetArmor() / (creature->GetArmor() + 10557.5));
            uint32 const hamstringDmg = ClassSpellsDamage::Warrior::HAMSTRING_RNK_4 * armorFactor;
            uint32 const hamstringCritDmg = ClassSpellsDamage::Warrior::HAMSTRING_RNK_4 * 2.0f * armorFactor;
            TEST_DIRECT_SPELL_DAMAGE(warrior, creature, ClassSpells::Warrior::HAMSTRING_RNK_4, hamstringDmg, hamstringDmg, false);
            TEST_DIRECT_SPELL_DAMAGE(warrior, creature, ClassSpells::Warrior::HAMSTRING_RNK_4, hamstringCritDmg, hamstringCritDmg, true);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<HamstringTestImpt>();
    }
};

class HeroicStrikeTest : public TestCaseScript
{
public:
    HeroicStrikeTest() : TestCaseScript("spells warrior heroic_strike") { }

    class HeroicStrikeTestImpt : public TestCaseWarrior
    {
    public:
        HeroicStrikeTestImpt() : TestCaseWarrior(STATUS_PARTIAL) { }

        void Test() override
        {
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN);
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_HUMAN);

            warrior->DisableRegeneration(true);
            warlock->SetMaxHealth(100000000);
            warlock->SetHealth(100000000);

            TestRequiresMeleeWeapon(warrior, warlock, ClassSpells::Warrior::HEROIC_STRIKE_RNK_10, true);

            // Rage cost
            uint32 const expectedHeroicStrikeRage = 15 * 10;
            warrior->Attack(warlock, true);
            TEST_POWER_COST(warrior, warlock, ClassSpells::Warrior::HEROIC_STRIKE_RNK_10, POWER_RAGE, expectedHeroicStrikeRage);
            warrior->AttackStop();

            // http://wowwiki.wikia.com/wiki/Heroic_Strike?oldid=1513476
            // TODO: No Rage is expended if Heroic Strike is parried or dodged. Rage is expended if Heroic Strike hits or is blocked.

            // Damage -- Apolyon, the Soul-Render - 404-607 damage
            float const apolyonSpeed = 3.4f;
            float const AP = warrior->GetTotalAttackPowerValue(BASE_ATTACK);
            float const armorFactor = 1 - (warlock->GetArmor() / (warlock->GetArmor() + 10557.5));
            uint32 const weaponMinDamage = 404 + (AP / 14 * apolyonSpeed) + ClassSpellsDamage::Warrior::HEROIC_STRIKE_RNK_10;
            uint32 const weaponMaxDamage = 607 + (AP / 14 * apolyonSpeed) + ClassSpellsDamage::Warrior::HEROIC_STRIKE_RNK_10;
            uint32 const expectedHSMin = weaponMinDamage * armorFactor;
            uint32 const expectedHSMax = weaponMaxDamage * armorFactor;
            uint32 const expectedHSCritMin = weaponMinDamage * 2.0f * armorFactor;
            uint32 const expectedHSCritMax = weaponMaxDamage * 2.0f * armorFactor;
            TEST_DIRECT_SPELL_DAMAGE(warrior, warlock, ClassSpells::Warrior::HEROIC_STRIKE_RNK_10, expectedHSMin, expectedHSMax, false);
            warlock->SetFullHealth();
            TEST_DIRECT_SPELL_DAMAGE(warrior, warlock, ClassSpells::Warrior::HEROIC_STRIKE_RNK_10, expectedHSCritMin, expectedHSCritMax, true);
            warlock->SetFullHealth();

            // Dazed
            float const hsDazedBonus = 61.6f;
            uint32 const expectedDazedHSMin = floor(weaponMinDamage + hsDazedBonus) * armorFactor;
            uint32 const expectedDazedHSMax = floor(weaponMaxDamage + hsDazedBonus) * armorFactor;
            uint32 const expectedDazedHSCritMin = floor(weaponMinDamage + hsDazedBonus) * 2.0f * armorFactor;
            uint32 const expectedDazedHSCritMax = floor(weaponMaxDamage + hsDazedBonus) * 2.0f * armorFactor;
            warlock->AddAura(1604, warlock); // Dazed
            TEST_DIRECT_SPELL_DAMAGE(warrior, warlock, ClassSpells::Warrior::HEROIC_STRIKE_RNK_10, expectedDazedHSMin, expectedDazedHSMax, false);
            warlock->SetFullHealth();
            warlock->AddAura(1604, warlock); // Dazed
            TEST_DIRECT_SPELL_DAMAGE(warrior, warlock, ClassSpells::Warrior::HEROIC_STRIKE_RNK_10, expectedDazedHSCritMin, expectedDazedHSCritMax, true);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<HeroicStrikeTestImpt>();
    }
};

class MockingBlowTest : public TestCaseScript
{
public:
    MockingBlowTest() : TestCaseScript("spells warrior mocking_blow") { }

    class MockingBlowTestImpt : public TestCaseWarrior
    {
    public:
        MockingBlowTestImpt() : TestCaseWarrior(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN);
            TestPlayer* ally    = SpawnPlayer(CLASS_WARRIOR, RACE_UNDEAD_PLAYER);
            Creature* creature  = SpawnCreature(20777, true); // Talbuk Sire
            Creature* dummy     = SpawnCreature();

            warrior->DisableRegeneration(true);

            TestRequiresStance(ally, creature, false, ClassSpells::Warrior::MOCKING_BLOW_RNK_6);
            TestRequiresStance(ally, creature, false, ClassSpells::Warrior::MOCKING_BLOW_RNK_6, ClassSpells::Warrior::DEFENSIVE_STANCE_RNK_1);
            TestRequiresStance(ally, creature, false, ClassSpells::Warrior::MOCKING_BLOW_RNK_6, ClassSpells::Warrior::BERSERKER_STANCE_RNK_1);
            TestRequiresStance(ally, creature, true,  ClassSpells::Warrior::MOCKING_BLOW_RNK_6, ClassSpells::Warrior::BATTLE_STANCE_RNK_1);
            TestRequiresMeleeWeapon(ally, creature, ClassSpells::Warrior::MOCKING_BLOW_RNK_6, true);

            // Setup
            creature->RemoveAurasDueToSpell(ClassSpells::Warrior::MOCKING_BLOW_RNK_6);
            ally->Attack(creature, true);
            Wait(3000);
            TEST_ASSERT(creature->GetTarget() == ally->GetGUID());
            TEST_CAST(warrior, warrior, ClassSpells::Warrior::BATTLE_STANCE_RNK_1);

            // Rage cost
            warrior->SetPower(POWER_RAGE, 1000); //max rage
            uint32 const expectedMockingBlowRage = 10 * 10;
            creature->RemoveAurasDueToSpell(ClassSpells::Warrior::MOCKING_BLOW_RNK_6);
            TEST_POWER_COST(warrior, creature, ClassSpells::Warrior::MOCKING_BLOW_RNK_6, POWER_RAGE, expectedMockingBlowRage);

            // Aura + Cooldown
            warrior->SetPower(POWER_RAGE, 1000); //max rage
            TEST_CAST(warrior, creature, ClassSpells::Warrior::MOCKING_BLOW_RNK_6);
            TEST_AURA_MAX_DURATION(creature, ClassSpells::Warrior::MOCKING_BLOW_RNK_6, Seconds(6));
            TEST_COOLDOWN(warrior, creature, ClassSpells::Warrior::MOCKING_BLOW_RNK_6, Minutes(2));

            // Taunt
            TEST_ASSERT(creature->GetTarget() == warrior->GetGUID());
            Wait(6000); //let taunt fade
            TEST_ASSERT(creature->GetTarget() == ally->GetGUID());

            // Damage
            float const armorFactor = 1 - (dummy->GetArmor() / (dummy->GetArmor() + 10557.5));
            const uint32 expectedMBDmg = ClassSpellsDamage::Warrior::MOCKING_BLOW_RNK_6 * armorFactor;
            const uint32 expectedMBCrit = ClassSpellsDamage::Warrior::MOCKING_BLOW_RNK_6 * 2.0f * armorFactor;
            TEST_DIRECT_SPELL_DAMAGE(warrior, dummy, ClassSpells::Warrior::MOCKING_BLOW_RNK_6, expectedMBDmg, expectedMBDmg, false);
            TEST_DIRECT_SPELL_DAMAGE(warrior, dummy, ClassSpells::Warrior::HEROIC_STRIKE_RNK_10, expectedMBCrit, expectedMBCrit, true);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<MockingBlowTestImpt>();
    }
};

class OverpowerTest : public TestCaseScript
{
public:
    OverpowerTest() : TestCaseScript("spells warrior overpower") { }

    class OverpowerTestImpt : public TestCaseWarrior
    {
    public:
        OverpowerTestImpt() : TestCaseWarrior(STATUS_PARTIAL) { }

        void Test() override
        {
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN);
            Creature* creature = SpawnCreature();

            warrior->DisableRegeneration(true);

            TestRequiresStance(warrior, creature, false, ClassSpells::Warrior::OVERPOWER_RNK_4);
            TestRequiresStance(warrior, creature, false, ClassSpells::Warrior::OVERPOWER_RNK_4, ClassSpells::Warrior::DEFENSIVE_STANCE_RNK_1);
            TestRequiresStance(warrior, creature, false, ClassSpells::Warrior::OVERPOWER_RNK_4, ClassSpells::Warrior::BERSERKER_STANCE_RNK_1);
            TestRequiresStance(warrior, creature, true,  ClassSpells::Warrior::OVERPOWER_RNK_4, ClassSpells::Warrior::BATTLE_STANCE_RNK_1);
            TestRequiresMeleeWeapon(warrior, creature, ClassSpells::Warrior::OVERPOWER_RNK_4, true);

            // Rage cost
            uint32 const expectedOverpowerRage = 5 * 10;
            TEST_POWER_COST(warrior, creature, ClassSpells::Warrior::OVERPOWER_RNK_4, POWER_RAGE, expectedOverpowerRage);

            // TODO: Overpower only usable after target dodged.
            // TODO: Overpower cannot be blocked, dodged or parried.

            // Damage -- Apolyon, the Soul-Render - 404-607 damage
            float const normalizedSpeed = 3.3f;
            float const AP = warrior->GetTotalAttackPowerValue(BASE_ATTACK);
            float const armorFactor = 1 - (creature->GetArmor() / (creature->GetArmor() + 10557.5));
            uint32 const apScale = AP * 0.236f;
            uint32 const weaponMinDamage = 404 + (AP / 14 * normalizedSpeed) + ClassSpellsDamage::Warrior::OVERPOWER_RNK_4;
            uint32 const weaponMaxDamage = 607 + (AP / 14 * normalizedSpeed) + ClassSpellsDamage::Warrior::OVERPOWER_RNK_4;
            uint32 const expectedOverpowerMin = weaponMinDamage * armorFactor;
            uint32 const expectedOverpowerMax = weaponMaxDamage * armorFactor;
            uint32 const expectedOverpowerCritMin = weaponMinDamage * 2.0f * armorFactor;
            uint32 const expectedOverpowerCritMax = weaponMaxDamage * 2.0f * armorFactor;
            TEST_DIRECT_SPELL_DAMAGE(warrior, creature, ClassSpells::Warrior::OVERPOWER_RNK_4, expectedOverpowerMin, expectedOverpowerMax, false);
            TEST_DIRECT_SPELL_DAMAGE(warrior, creature, ClassSpells::Warrior::OVERPOWER_RNK_4, expectedOverpowerCritMin, expectedOverpowerCritMax, true);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<OverpowerTestImpt>();
    }
};

class RendTest : public TestCaseScript
{
public:
    RendTest() : TestCaseScript("spells warrior rend") { }

    class RendTestImpt : public TestCaseWarrior
    {
    public:
        RendTestImpt() : TestCaseWarrior(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN);
            Creature* creature = SpawnCreature();

            warrior->DisableRegeneration(true);

            TestRequiresStance(warrior, creature, false, ClassSpells::Warrior::REND_RNK_8);
            TestRequiresStance(warrior, creature, false, ClassSpells::Warrior::REND_RNK_8, ClassSpells::Warrior::BERSERKER_STANCE_RNK_1);
            TestRequiresStance(warrior, creature, true, ClassSpells::Warrior::REND_RNK_8, ClassSpells::Warrior::DEFENSIVE_STANCE_RNK_1);
            TestRequiresStance(warrior, creature, true, ClassSpells::Warrior::REND_RNK_8, ClassSpells::Warrior::BATTLE_STANCE_RNK_1);
            TestRequiresMeleeWeapon(warrior, creature, ClassSpells::Warrior::REND_RNK_8, true);

            // Rage cost
            uint32 const expectedRendRage = 10 * 10;
            TEST_POWER_COST(warrior, creature, ClassSpells::Warrior::REND_RNK_8, POWER_RAGE, expectedRendRage);

            // Damage -- Apolyon, the Soul-Render - 404-607 damage
            float const apolyonSpeed = 3.4f;
            float const AP = warrior->GetTotalAttackPowerValue(BASE_ATTACK);
            float const armorFactor = 1 - (creature->GetArmor() / (creature->GetArmor() + 10557.5));
            float const rendTime = 21.0f;
            float const apolyonAvg = (404 + 607) / 2.0f;
            uint32 const rendBonus = (apolyonAvg + (apolyonSpeed * AP / 14)) * rendTime * 743 / 300000;
            uint32 const rendTotal = 7 * floor((ClassSpellsDamage::Warrior::REND_RNK_8 + rendBonus) / 7.0f);
            TEST_DOT_DAMAGE(warrior, creature, ClassSpells::Warrior::REND_RNK_8, rendTotal, false);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<RendTestImpt>();
    }
};

class RetaliationTest : public TestCaseScript
{
public:
    RetaliationTest() : TestCaseScript("spells warrior retaliation") { }

    class RetaliationTestImpt : public TestCaseWarrior
    {
    public:
        RetaliationTestImpt() : TestCaseWarrior(STATUS_PARTIAL) { }

        void Test() override
        {
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN);

            Position spawn(_location);
            spawn.MoveInFront(_location, 3.0f);
            TestPlayer* enemy = SpawnPlayer(CLASS_WARRIOR, RACE_HUMAN, 70, spawn);

            spawn.MoveInFront(_location, -3.0f);
            TestPlayer* behind = SpawnPlayer(CLASS_WARRIOR, RACE_HUMAN, 70, spawn);

            warrior->DisableRegeneration(true);

            TestRequiresStance(warrior, warrior, false, ClassSpells::Warrior::RETALIATION_RNK_1);
            TestRequiresStance(warrior, warrior, false, ClassSpells::Warrior::RETALIATION_RNK_1, ClassSpells::Warrior::BERSERKER_STANCE_RNK_1);
            TestRequiresStance(warrior, warrior, false, ClassSpells::Warrior::RETALIATION_RNK_1, ClassSpells::Warrior::DEFENSIVE_STANCE_RNK_1);
            TestRequiresStance(warrior, warrior, true, ClassSpells::Warrior::RETALIATION_RNK_1, ClassSpells::Warrior::BATTLE_STANCE_RNK_1);
            EQUIP_ITEM(warrior, 34247); // Apolyon, the Soul-Render

            // Aura
            Aura* aura = warrior->GetAura(ClassSpells::Warrior::RETALIATION_RNK_1);
            TEST_ASSERT(aura != nullptr);
            TEST_ASSERT(aura->GetMaxDuration() == 15 * SECOND * IN_MILLISECONDS);
            TEST_ASSERT(aura->GetCharges() == 30);

            // Cannot counterattack from behind
            for (uint32 i = 0; i < 500; i++)
            {
                behind->AttackerStateUpdate(warrior, BASE_ATTACK);
                warrior->SetFullHealth();
            }
            TEST_ASSERT(aura->GetCharges() == 30);

            // Damage -- Apolyon, the Soul-Retaliationer - 404-607 damage
            float const apolyonSpeed = 3.3f;
            float const AP = warrior->GetTotalAttackPowerValue(BASE_ATTACK);
            float const armorFactor = 1 - (enemy->GetArmor() / (enemy->GetArmor() + 10557.5));
            uint32 const weaponMinDamage = 404 + (AP / 14 * apolyonSpeed);
            uint32 const weaponMaxDamage = 607 + (AP / 14 * apolyonSpeed);
            uint32 const expectedMin = weaponMinDamage * armorFactor;
            uint32 const expectedMax = weaponMaxDamage * armorFactor;

            // TODO: lose charge on hit

            // Counterattack from front
            EnableCriticals(warrior, false);
            for (uint32 i = 0; i < 500; i++)
            {
                enemy->AttackerStateUpdate(warrior, BASE_ATTACK);
                if (aura->GetCharges() == 0)
                {
                    warrior->AddAura(ClassSpells::Warrior::RETALIATION_RNK_1, warrior);
                    aura = warrior->GetAura(ClassSpells::Warrior::RETALIATION_RNK_1);
                    TEST_ASSERT(aura != nullptr);
                }
                warrior->SetFullHealth();
            }

            uint32 givenMin;
            uint32 givenMax;
            GetDamagePerSpellsTo(warrior, enemy, ClassSpells::Warrior::RETALIATION_RNK_1_PROC, givenMin, givenMax, false);
            TC_LOG_TRACE("test.unit_test", "givenMin: %u, givenMax: %u, expectedMin: %u, expectedMax: %u", givenMin, givenMax, expectedMin, expectedMax);
            TEST_ASSERT(givenMin < expectedMax);
            TEST_ASSERT(givenMax > expectedMin);
            TEST_ASSERT(givenMin >= expectedMin);
            TEST_ASSERT(givenMax <= expectedMax);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<RetaliationTestImpt>();
    }
};

class ThunderClapTest : public TestCaseScript
{
public:
    ThunderClapTest() : TestCaseScript("spells warrior thunder_clap") { }

    class ThunderClapTestImpt : public TestCaseWarrior
    {
    public:
        ThunderClapTestImpt() : TestCaseWarrior(STATUS_PASSING) { }

        //test duration + victim attack time
        void TestThunderClapAura(Unit* victim, float expectedAS, int count)
        {
            if (victim->HasAura(ClassSpells::Warrior::THUNDER_CLAP_RNK_7))
            {
                TEST_AURA_MAX_DURATION(victim, ClassSpells::Warrior::THUNDER_CLAP_RNK_7, Seconds(30));
                TEST_ASSERT(victim->GetAttackTimer(BASE_ATTACK) == expectedAS);
                count++;
            }
        }

        void Test() override
        {
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN);
            TestPlayer* enemy1 = SpawnPlayer(CLASS_WARRIOR, RACE_HUMAN);
            TestPlayer* enemy2 = SpawnPlayer(CLASS_WARRIOR, RACE_HUMAN);
            TestPlayer* enemy3 = SpawnPlayer(CLASS_WARRIOR, RACE_HUMAN);
            Creature* creature1 = SpawnCreature();
            Creature* creature2 = SpawnCreature();

            warrior->DisableRegeneration(true);

            TestRequiresStance(warrior, warrior, false, ClassSpells::Warrior::THUNDER_CLAP_RNK_7);
            TestRequiresStance(warrior, warrior, false, ClassSpells::Warrior::THUNDER_CLAP_RNK_7, ClassSpells::Warrior::BERSERKER_STANCE_RNK_1);
            TestRequiresStance(warrior, warrior, true, ClassSpells::Warrior::THUNDER_CLAP_RNK_7, ClassSpells::Warrior::DEFENSIVE_STANCE_RNK_1);
            TestRequiresStance(warrior, warrior, true, ClassSpells::Warrior::THUNDER_CLAP_RNK_7, ClassSpells::Warrior::BATTLE_STANCE_RNK_1);

            // Rage cost
            Wait(4500); // Cooldown
            warrior->SetPower(POWER_RAGE, warrior->GetMaxPower(POWER_RAGE));
            uint32 const expectedThunderClapRage = 20 * 10;
            TEST_POWER_COST(warrior, creature1, ClassSpells::Warrior::THUNDER_CLAP_RNK_7, POWER_RAGE, expectedThunderClapRage);

            // Cooldown
            TEST_COOLDOWN(warrior, creature1, ClassSpells::Warrior::THUNDER_CLAP_RNK_7, Seconds(4));

            // 4 affected max, aura duration, slower attacks
            warrior->SetPower(POWER_RAGE, warrior->GetMaxPower(POWER_RAGE));
            TEST_CAST(warrior, creature1, ClassSpells::Warrior::THUNDER_CLAP_RNK_7);
            float const ThunderClapFactor = 0.9f;
            float const enemy1ExpectedAS = enemy1->GetAttackTimer(BASE_ATTACK) * ThunderClapFactor;
            float const enemy2ExpectedAS = enemy1->GetAttackTimer(BASE_ATTACK) * ThunderClapFactor;
            float const enemy3ExpectedAS = enemy1->GetAttackTimer(BASE_ATTACK) * ThunderClapFactor;
            float const creature1ExpectedAS = creature1->GetAttackTimer(BASE_ATTACK) * ThunderClapFactor;
            float const creature2ExpectedAS = creature1->GetAttackTimer(BASE_ATTACK) * ThunderClapFactor;

            int count = 0;
            TestThunderClapAura(enemy1, enemy1ExpectedAS, count);
            TestThunderClapAura(enemy2, enemy2ExpectedAS, count);
            TestThunderClapAura(enemy3, enemy3ExpectedAS, count);
            TestThunderClapAura(creature1, creature1ExpectedAS, count);
            TestThunderClapAura(creature2, creature2ExpectedAS, count);
            TEST_ASSERT(count <= 4);

            // Damage
            float const armorFactor = 1 - (creature1->GetArmor() / (creature1->GetArmor() + 10557.5));
            uint32 thunderClap = ClassSpellsDamage::Warrior::THUNDER_CLAP_RNK_7 * armorFactor;
            uint32 thunderClapCrit = ClassSpellsDamage::Warrior::THUNDER_CLAP_RNK_7 * 1.5f * armorFactor;
            TEST_DIRECT_SPELL_DAMAGE(warrior, creature1, ClassSpells::Warrior::THUNDER_CLAP_RNK_7, thunderClap, thunderClap, false);
            TEST_DIRECT_SPELL_DAMAGE(warrior, creature1, ClassSpells::Warrior::THUNDER_CLAP_RNK_7, thunderClapCrit, thunderClapCrit, true);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<ThunderClapTestImpt>();
    }
};

class BattleShoutTest : public TestCaseScript
{
public:
    BattleShoutTest() : TestCaseScript("spells warrior battle_shout") { }

    class BattleShoutTestImpt : public TestCaseWarrior
    {
    public:
        BattleShoutTestImpt() : TestCaseWarrior(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN);

            Position spawn(_location);
            spawn.MoveInFront(spawn, 10.0f);
            TestPlayer* hunter = SpawnPlayer(CLASS_HUNTER, RACE_TAUREN, 70, spawn);
            spawn.MoveInFront(spawn, 30.0f);
            TestPlayer* ally2 = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN, 70, spawn);

            warrior->DisableRegeneration(true);

            TestRequiresStance(warrior, warrior, false, ClassSpells::Warrior::BATTLE_SHOUT_RNK_8);

            // Setup
            GroupPlayer(warrior, hunter);
            GroupPlayer(warrior, ally2);
            uint32 battleShoutBonus = 305;
            uint32 warriorExpectedAP = warrior->GetTotalAttackPowerValue(BASE_ATTACK) + battleShoutBonus;
            uint32 hunterExpectedAP  = hunter->GetTotalAttackPowerValue(RANGED_ATTACK) + battleShoutBonus;

            // Rage cost
            uint32 const expectedBattleShoutRage = 10 * 10;
            TEST_POWER_COST(warrior, warrior, ClassSpells::Warrior::BATTLE_SHOUT_RNK_8, POWER_RAGE, expectedBattleShoutRage);

            // Aura duration
            TEST_AURA_MAX_DURATION(warrior, ClassSpells::Warrior::BATTLE_SHOUT_RNK_8, 2 * MINUTE * IN_MILLISECONDS);
            TEST_AURA_MAX_DURATION(hunter, ClassSpells::Warrior::BATTLE_SHOUT_RNK_8, 2 * MINUTE * IN_MILLISECONDS);

            // Range
            TEST_ASSERT(!ally2->HasAura(ClassSpells::Warrior::BATTLE_SHOUT_RNK_8));

            // AP
            TEST_ASSERT(warrior->GetTotalAttackPowerValue(BASE_ATTACK) == warriorExpectedAP);
            TEST_ASSERT(hunter->GetTotalAttackPowerValue(RANGED_ATTACK) == hunterExpectedAP);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<BattleShoutTestImpt>();
    }
};

class BerserkerRageTest : public TestCaseScript
{
public:
    BerserkerRageTest() : TestCaseScript("spells warrior berserker_rage") { }

    class BerserkerRageTestImpt : public TestCaseWarrior
    {
    public:
        BerserkerRageTestImpt() : TestCaseWarrior(STATUS_PASSING) { }

        void TestImmunity(TestPlayer* warrior, TestPlayer* caster, uint32 spellId)
        {
            warrior->RemoveAurasDueToSpell(ClassSpells::Warrior::BERSERKER_RAGE_RNK_1);
            // Cast CC
            int count = 0;
            while (!warrior->HasAura(spellId))
            {
                TEST_CAST(caster, warrior, spellId, SPELL_CAST_OK, TRIGGERED_FULL_MASK);
                if (count > 10)
                {
                    ASSERT_INFO("Couldnt hit spell %u after 10 try", spellId);
                    TEST_ASSERT(false);
                }
                count++;
            }
            // Break CC
            TEST_CAST(warrior, warrior, ClassSpells::Warrior::BERSERKER_RAGE_RNK_1, SPELL_CAST_OK, TRIGGERED_IGNORE_SPELL_AND_CATEGORY_CD);
            TEST_ASSERT(!warrior->HasAura(spellId));
            // Immune CC
            TEST_CAST(caster, warrior, spellId, SPELL_CAST_OK, TRIGGERED_FULL_MASK);
            TEST_ASSERT(!warrior->HasAura(spellId));

        }

        void Test() override
        {
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN);
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_HUMAN);
            TestPlayer* rogue   = SpawnPlayer(CLASS_ROGUE, RACE_HUMAN);

            TestRequiresStance(warrior, warrior, false, ClassSpells::Warrior::BERSERKER_RAGE_RNK_1);
            TestRequiresStance(warrior, warrior, false, ClassSpells::Warrior::BERSERKER_RAGE_RNK_1, ClassSpells::Warrior::BATTLE_STANCE_RNK_1);
            TestRequiresStance(warrior, warrior, false, ClassSpells::Warrior::BERSERKER_RAGE_RNK_1, ClassSpells::Warrior::DEFENSIVE_STANCE_RNK_1);
            TestRequiresStance(warrior, warrior, true, ClassSpells::Warrior::BERSERKER_RAGE_RNK_1, ClassSpells::Warrior::BERSERKER_STANCE_RNK_1);

            TestImmunity(warrior, rogue, ClassSpells::Rogue::SAP_RNK_3);
            TestImmunity(warrior, rogue, ClassSpells::Rogue::GOUGE_RNK_6);
            TestImmunity(warrior, warlock, ClassSpells::Warlock::FEAR_RNK_3);

            warrior->SetPower(POWER_RAGE, warrior->GetMaxPower(POWER_RAGE));
            TEST_CAST(warrior, warrior, ClassSpells::Warrior::BERSERKER_RAGE_RNK_1);

            // Aura duration
            TEST_AURA_MAX_DURATION(warrior, ClassSpells::Warrior::BERSERKER_RAGE_RNK_1, Seconds(10));

            // Cooldown duration
            TEST_HAS_COOLDOWN(warrior, ClassSpells::Warrior::BERSERKER_RAGE_RNK_1, Seconds(30));

            // Extra rage
            warrior->SetPower(POWER_RAGE, 0);
            auto AI = rogue->GetTestingPlayerbotAI();
            uint32 level = 70;
            float const rageConversion = 0.0091107836f * level * level + 3.225598133f * level + 4.2652911f;
            uint32 expectedRage = 0;
            for (uint32 i = 0; i < 75; i++)
            {
                rogue->AttackerStateUpdate(warrior, BASE_ATTACK);
                auto damageToTarget = AI->GetMeleeDamageDoneInfo(warrior);
                TEST_ASSERT(damageToTarget->size() == i + 1);
                auto& data = damageToTarget->back();
                uint32 damage = 0;
                for (uint8 j = 0; j < MAX_ITEM_PROTO_DAMAGES; j++)
                {
                    damage += data.damageInfo.Damages[j].Damage;
                    damage += data.damageInfo.Damages[j].Resist;
                    damage += data.damageInfo.Damages[j].Absorb;
                }
                damage += data.damageInfo.Blocked;
                uint32 rage = 2.5f * damage / rageConversion * 2.0f * 10.0f;
                expectedRage += rage;
                uint32 warRage = warrior->GetPower(POWER_RAGE);
                ASSERT_INFO("Count: %u, Health: %u, Damage: %u, Rage: %u, Expected: %u", i, warrior->GetHealth(), damage, warRage, expectedRage);
                TEST_ASSERT(warRage == expectedRage);
            }
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<BerserkerRageTestImpt>();
    }
};

class ChallengingShoutTest : public TestCaseScript
{
public:

    ChallengingShoutTest() : TestCaseScript("spells warrior challenging_shout") { }

    class ChallengingShoutTestImpt : public TestCase
    {
    public:
        ChallengingShoutTestImpt() : TestCase(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN);
            warrior->SetPower(POWER_RAGE, warrior->GetMaxPower(POWER_RAGE));

            Position spawn3m(_location);
            spawn3m.MoveInFront(_location, 3.0f);
            TestPlayer* player3m = SpawnPlayer(CLASS_ROGUE, RACE_HUMAN, 1, spawn3m);
            Creature* creature3m = SpawnCreatureWithPosition(spawn3m, 6);

            Position spawn6m(_location);
            spawn6m.MoveInFront(_location, 6.0f);
            TestPlayer* player6m = SpawnPlayer(CLASS_ROGUE, RACE_BLOODELF, 1, spawn6m);
            Creature* creature6m = SpawnCreatureWithPosition(spawn6m, 6);

            Position spawn11m(_location);
            spawn11m.MoveInFront(_location, 15.0f);
            TestPlayer* player11m = SpawnPlayer(CLASS_ROGUE, RACE_BLOODELF, 1, spawn11m);
            Creature* creature11m = SpawnCreatureWithPosition(spawn11m, 6);

            // Setup
            player3m->Attack(creature3m, true);
            player6m->Attack(creature6m, true);
            player11m->Attack(creature11m, true);

            Wait(3000);

            TEST_CAST(warrior, warrior, ClassSpells::Warrior::BERSERKER_STANCE_RNK_1);
            TEST_CAST(warrior, warrior, ClassSpells::Warrior::CHALLENGING_SHOUT_RNK_1);

            // Aura, affected targets
            Aura* aura3m = creature3m->GetAura(ClassSpells::Warrior::CHALLENGING_SHOUT_RNK_1);
            TEST_ASSERT(aura3m != nullptr);
            Aura* aura6m = creature6m->GetAura(ClassSpells::Warrior::CHALLENGING_SHOUT_RNK_1);
            TEST_ASSERT(aura6m != nullptr);
            // Aura, too far 
            Aura* aura11m = creature11m->GetAura(ClassSpells::Warrior::CHALLENGING_SHOUT_RNK_1);
            TEST_ASSERT(aura11m == nullptr);

            // Aura duration
            TEST_ASSERT(aura3m->GetDuration() == 6 * SECOND * IN_MILLISECONDS);
            TEST_ASSERT(aura6m->GetDuration() == 6 * SECOND * IN_MILLISECONDS);

            // Target changed
            TEST_ASSERT(creature3m->GetTarget() == warrior->GetGUID());
            TEST_ASSERT(creature6m->GetTarget() == warrior->GetGUID());

            // Back to original target
            Wait(6500);
            TEST_ASSERT(creature3m->GetTarget() == player3m->GetGUID());
            TEST_ASSERT(creature6m->GetTarget() == player6m->GetGUID());


            // Rage cost
            warrior->SetPower(POWER_RAGE, warrior->GetMaxPower(POWER_RAGE));
            uint32 const expectedChallengingShoutRage = 5 * 10;
            TEST_POWER_COST(warrior, warrior, ClassSpells::Warrior::CHALLENGING_SHOUT_RNK_1, POWER_RAGE, expectedChallengingShoutRage);

            // Cooldown
            TEST_COOLDOWN(warrior, warrior, ClassSpells::Warrior::CHALLENGING_SHOUT_RNK_1, Minutes(10));
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<ChallengingShoutTestImpt>();
    }
};

class CleaveTest : public TestCaseScript
{
public:
    CleaveTest() : TestCaseScript("spells warrior cleave") { }

    class CleaveTestImpt : public TestCaseWarrior
    {
    public:
        CleaveTestImpt() : TestCaseWarrior(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN);
            Creature* creature1 = SpawnCreature(8, true);
            Creature* creature2 = SpawnCreature(8, true);
            Creature* creature3 = SpawnCreature(8, true);

            warrior->DisableRegeneration(true);

            TestRequiresMeleeWeapon(warrior, creature1, ClassSpells::Warrior::CLEAVE_RNK_6, true);

            // Setup
            uint32 startHealth = creature1->GetHealth();
            TEST_ASSERT(creature2->GetHealth() == startHealth);
            TEST_ASSERT(creature3->GetHealth() == startHealth);

            // Rage cost
            uint32 const expectedCleaveRage = 20 * 10;
            warrior->Attack(creature1, true);
            TEST_POWER_COST(warrior, creature1, ClassSpells::Warrior::CLEAVE_RNK_6, POWER_RAGE, expectedCleaveRage);
            warrior->AttackStop();


            // Damage -- Apolyon, the Soul-Render - 404-607 damage
            float const apolyonSpeed = 3.4f;
            float const AP = warrior->GetTotalAttackPowerValue(BASE_ATTACK);
            float const armorFactor = 1 - (creature1->GetArmor() / (creature1->GetArmor() + 10557.5));
            uint32 const weaponMinDamage = 404 + (AP / 14 * apolyonSpeed) + ClassSpellsDamage::Warrior::CLEAVE_RNK_6;
            uint32 const weaponMaxDamage = 607 + (AP / 14 * apolyonSpeed) + ClassSpellsDamage::Warrior::CLEAVE_RNK_6;
            uint32 const expectedCleaveMin = weaponMinDamage * armorFactor;
            uint32 const expectedCleaveMax = weaponMaxDamage * armorFactor;
            uint32 const expectedCleaveCritMin = weaponMinDamage * 2.0f * armorFactor;
            uint32 const expectedCleaveCritMax = weaponMaxDamage * 2.0f * armorFactor;
            TEST_DIRECT_SPELL_DAMAGE(warrior, creature1, ClassSpells::Warrior::CLEAVE_RNK_6, expectedCleaveMin, expectedCleaveMax, false);
            TEST_DIRECT_SPELL_DAMAGE(warrior, creature1, ClassSpells::Warrior::CLEAVE_RNK_6, expectedCleaveCritMin, expectedCleaveCritMax, true);

            // Cleave
            TEST_ASSERT(creature1->GetHealth() < startHealth);
            TEST_ASSERT(creature2->GetHealth() < startHealth);
            TEST_ASSERT(creature3->GetHealth() < startHealth);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<CleaveTestImpt>();
    }
};

class CommandingShoutTest : public TestCaseScript
{
public:
    CommandingShoutTest() : TestCaseScript("spells warrior commanding_shout") { }

    class CommandingShoutTestImpt : public TestCaseWarrior
    {
    public:
        CommandingShoutTestImpt() : TestCaseWarrior(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN);

            Position spawn(_location);
            spawn.MoveInFront(spawn, 10.0f);
            TestPlayer* hunter = SpawnPlayer(CLASS_HUNTER, RACE_TAUREN, 70, spawn);
            spawn.MoveInFront(spawn, 30.0f);
            TestPlayer* ally2 = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN, 70, spawn);

            warrior->DisableRegeneration(true);

            TestRequiresStance(warrior, warrior, false, ClassSpells::Warrior::COMMANDING_SHOUT_RNK_1);

            // Setup
            GroupPlayer(warrior, hunter);
            GroupPlayer(warrior, ally2);
            uint32 commandingShoutBonus = 1080;
            uint32 warriorExpectedHealth = warrior->GetHealth() + commandingShoutBonus;
            uint32 hunterExpectedHealth = hunter->GetHealth() + commandingShoutBonus;

            // Rage cost
            uint32 const expectedCommandingShoutRage = 10 * 10;
            TEST_POWER_COST(warrior, warrior, ClassSpells::Warrior::COMMANDING_SHOUT_RNK_1, POWER_RAGE, expectedCommandingShoutRage);

            // Aura duration
            TEST_AURA_MAX_DURATION(warrior, ClassSpells::Warrior::COMMANDING_SHOUT_RNK_1, 2 * MINUTE * IN_MILLISECONDS);
            TEST_AURA_MAX_DURATION(hunter, ClassSpells::Warrior::COMMANDING_SHOUT_RNK_1, 2 * MINUTE * IN_MILLISECONDS);

            // Range
            TEST_ASSERT(!ally2->HasAura(ClassSpells::Warrior::COMMANDING_SHOUT_RNK_1));

            // Health
            TEST_ASSERT(warrior->GetHealth() == warriorExpectedHealth);
            TEST_ASSERT(hunter->GetHealth() == hunterExpectedHealth);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<CommandingShoutTestImpt>();
    }
};

class DemoralizingShoutTest : public TestCaseScript
{
public:

    DemoralizingShoutTest() : TestCaseScript("spells warrior demoralizing_shout") { }

    class DemoralizingShoutTestImpt : public TestCase
    {
    public:
        DemoralizingShoutTestImpt() : TestCase(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN);

            Position spawn3m(_location);
            spawn3m.MoveInFront(_location, 3.0f);
            TestPlayer* player3m = SpawnPlayer(CLASS_ROGUE, RACE_HUMAN, 1, spawn3m);

            Position spawn6m(_location);
            spawn6m.MoveInFront(_location, 6.0f);
            Creature* creature6m = SpawnCreatureWithPosition(spawn6m, 6);

            Position spawn15m(_location);
            spawn15m.MoveInFront(_location, 15.0f);
            Creature* creature15m = SpawnCreatureWithPosition(spawn15m, 6);

            int32 demoralizingShoutMalus = 300;
            int32 startAP3m = player3m->GetTotalAttackPowerValue(BASE_ATTACK);
            int32 startAP6m = creature6m->GetTotalAttackPowerValue(BASE_ATTACK);
            int32 expectedAP3m = (startAP3m - demoralizingShoutMalus > 0) ? startAP3m - demoralizingShoutMalus : 0;
            int32 expectedAP6m = (startAP6m - demoralizingShoutMalus > 0) ? startAP6m - demoralizingShoutMalus : 0;

            // Rage cost
            uint32 const expectedDemoralizingShoutRage = 10 * 10;
            TEST_POWER_COST(warrior, warrior, ClassSpells::Warrior::DEMORALIZING_SHOUT_RNK_7, POWER_RAGE, expectedDemoralizingShoutRage);

            // Aura
            TEST_AURA_MAX_DURATION(player3m, ClassSpells::Warrior::DEMORALIZING_SHOUT_RNK_7, 30 * SECOND  * IN_MILLISECONDS);
            TEST_AURA_MAX_DURATION(creature6m, ClassSpells::Warrior::DEMORALIZING_SHOUT_RNK_7, 30 * SECOND  * IN_MILLISECONDS);

            // Range
            TEST_ASSERT(!creature15m->HasAura(ClassSpells::Warrior::DEMORALIZING_SHOUT_RNK_7));

            // AP loss
            TEST_ASSERT(player3m->GetTotalAttackPowerValue(BASE_ATTACK) == expectedAP3m);
            TEST_ASSERT(creature6m->GetTotalAttackPowerValue(BASE_ATTACK) == expectedAP6m);

            // Back to original target
            Wait(31000);
            TEST_ASSERT(player3m->GetTotalAttackPowerValue(BASE_ATTACK) == startAP3m);
            TEST_ASSERT(creature6m->GetTotalAttackPowerValue(BASE_ATTACK) == startAP6m);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<DemoralizingShoutTestImpt>();
    }
};

class ExecuteTest : public TestCaseScript
{
public:
    ExecuteTest() : TestCaseScript("spells warrior execute") { }

    class ExecuteTestImpt : public TestCaseWarrior
    {
    public:
        ExecuteTestImpt() : TestCaseWarrior(STATUS_PASSING) { }

        void TestExecuteDamage(TestPlayer* warrior, Unit* victim, bool crit)
        {
            uint32 startHealth = 20000;
            victim->SetHealth(startHealth);
            warrior->SetPower(POWER_RAGE, 1000); // full rage
            EnableCriticals(warrior, crit);

            uint32 const remainingRage = (1000 - 150) * 0.1f; // full rage - execute cost
            float const armorFactor = 1 - (victim->GetArmor() / (victim->GetArmor() + 10557.5));

            uint32 expectedExecute = ClassSpellsDamage::Warrior::EXECUTE_RNK_7 + remainingRage * ClassSpellsDamage::Warrior::EXECUTE_RNK_7_RAGE;
            if (crit)
                expectedExecute *= 2.0f;
            uint32 expectedHealth = victim->GetHealth() - floor(expectedExecute * armorFactor);

            Wait(500);
            TEST_ASSERT(victim->GetHealth() == startHealth);
            int32 count = 0;
            while (victim->GetHealth() == startHealth)
            {
                TEST_CAST(warrior, victim, ClassSpells::Warrior::EXECUTE_RNK_7);
                if (count > 20)
                {
                    ASSERT_INFO("Execute couldnt hit 20 times in a row.");
                    TEST_ASSERT(false);
                }
                Wait(500);
                count++;
            }
            ASSERT_INFO("Health: %u, Expected: %u", victim->GetHealth(), expectedHealth);
            TEST_ASSERT(victim->GetHealth() == expectedHealth);
        }

        void Test() override
        {
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN);
            Creature* creature = SpawnCreature(8, true);

            warrior->DisableRegeneration(true);
            creature->DisableRegeneration(true);

            // Needs target below 20%
            TEST_CAST(warrior, creature, ClassSpells::Warrior::EXECUTE_RNK_7, SPELL_FAILED_TARGET_AURASTATE, TRIGGERED_IGNORE_POWER_AND_REAGENT_COST);
            creature->SetHealth(20000);
            Wait(500);

            TestRequiresStance(warrior, creature, false, ClassSpells::Warrior::EXECUTE_RNK_7);
            TestRequiresStance(warrior, creature, false, ClassSpells::Warrior::EXECUTE_RNK_7, ClassSpells::Warrior::DEFENSIVE_STANCE_RNK_1);
            TestRequiresStance(warrior, creature, true, ClassSpells::Warrior::EXECUTE_RNK_7, ClassSpells::Warrior::BATTLE_STANCE_RNK_1);
            TestRequiresStance(warrior, creature, true, ClassSpells::Warrior::EXECUTE_RNK_7, ClassSpells::Warrior::BERSERKER_STANCE_RNK_1);
            TestRequiresMeleeWeapon(warrior, creature, ClassSpells::Warrior::EXECUTE_RNK_7, true);

            // Rage cost
            uint32 const expectedExecuteRage = 15 * 10;
            TEST_POWER_COST(warrior, creature, ClassSpells::Warrior::EXECUTE_RNK_7, POWER_RAGE, expectedExecuteRage);

            // Damage
            TestExecuteDamage(warrior, creature, false);
            TestExecuteDamage(warrior, creature, true);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<ExecuteTestImpt>();
    }
};

class InterceptTest : public TestCaseScript
{
public:
    InterceptTest() : TestCaseScript("spells warrior intercept") { }

    class InterceptTestImpt : public TestCaseWarrior
    {
    public:
        InterceptTestImpt() : TestCaseWarrior(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN);

            Position spawn(_location);
            spawn.MoveInFront(_location, 7.0f);
            TestPlayer* priest1 = SpawnPlayer(CLASS_PRIEST, RACE_HUMAN, 70, spawn);
            spawn.MoveInFront(_location, 15.0f);
            TestPlayer* priest2 = SpawnPlayer(CLASS_PRIEST, RACE_HUMAN, 70, spawn);
            Creature* creature = SpawnCreatureWithPosition(spawn);
            spawn.MoveInFront(_location, 30.0f);
            TestPlayer* priest3 = SpawnPlayer(CLASS_PRIEST, RACE_HUMAN, 70, spawn);

            warrior->DisableRegeneration(true);
            warrior->SetWalk(false); // run

            Wait(5000);

            // Stances
            TestRequiresStance(warrior, priest2, false, ClassSpells::Warrior::INTERCEPT_RNK_5);
            TestRequiresStance(warrior, priest2, false, ClassSpells::Warrior::INTERCEPT_RNK_5, ClassSpells::Warrior::BATTLE_STANCE_RNK_1);
            TestRequiresStance(warrior, priest2, false, ClassSpells::Warrior::INTERCEPT_RNK_5, ClassSpells::Warrior::DEFENSIVE_STANCE_RNK_1);
            TestRequiresStance(warrior, priest2, true, ClassSpells::Warrior::INTERCEPT_RNK_5, ClassSpells::Warrior::BERSERKER_STANCE_RNK_1);

            // Range
            TEST_CAST(warrior, priest1, ClassSpells::Warrior::INTERCEPT_RNK_5, SPELL_FAILED_OUT_OF_RANGE, TriggerCastFlags(TRIGGERED_IGNORE_POWER_AND_REAGENT_COST | TRIGGERED_IGNORE_SPELL_AND_CATEGORY_CD));
            warrior->Relocate(_location);
            TEST_CAST(warrior, priest3, ClassSpells::Warrior::INTERCEPT_RNK_5, SPELL_FAILED_OUT_OF_RANGE, TriggerCastFlags(TRIGGERED_IGNORE_POWER_AND_REAGENT_COST | TRIGGERED_IGNORE_SPELL_AND_CATEGORY_CD));
            warrior->Relocate(_location);
            Wait(1500);

            uint32 initialRage = warrior->GetPower(POWER_RAGE);
            TEST_CAST(warrior, priest2, ClassSpells::Warrior::INTERCEPT_RNK_5);

            // Rage cost
            uint32 expectedInterceptRageCost = 10 * 10;
            //TEST_POWER_COST(warrior, priest2, ClassSpells::Warrior::INTERCEPT_RNK_5, POWER_RAGE, expectedInterceptRageCost);
            TEST_ASSERT(warrior->GetPower(POWER_RAGE) == initialRage - expectedInterceptRageCost);

            // Cooldown
            TEST_HAS_COOLDOWN(warrior, ClassSpells::Warrior::INTERCEPT_RNK_5, Seconds(30));

            // Stun
            Wait(250);
            TEST_AURA_MAX_DURATION(priest2, ClassSpells::Warrior::INTERCEPT_RNK_5_TRIGGER, Seconds(3));

            Wait(1000);
            // Warrior still running after Intercept
            TEST_ASSERT(!warrior->IsWalking());

            // Damage
            float const armorFactor = 1 - (creature->GetArmor() / (creature->GetArmor() + 10557.5));
            uint32 const expectedIntercept = ClassSpellsDamage::Warrior::INTERCEPT_RNK_5 * armorFactor;
            uint32 const expectedInterceptCrit = ClassSpellsDamage::Warrior::INTERCEPT_RNK_5 * 2.0f * armorFactor;
            TEST_DIRECT_SPELL_DAMAGE(warrior, creature, ClassSpells::Warrior::INTERCEPT_RNK_5_TRIGGER, expectedIntercept, expectedIntercept, false);
            TEST_DIRECT_SPELL_DAMAGE(warrior, creature, ClassSpells::Warrior::INTERCEPT_RNK_5_TRIGGER, expectedInterceptCrit, expectedInterceptCrit, true);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<InterceptTestImpt>();
    }
};

class IntidimidatingShoutTest : public TestCaseScript
{
public:
    IntidimidatingShoutTest() : TestCaseScript("spells warrior intimidating_shout") { }

    class IntidimidatingShoutTestImpt : public TestCaseWarrior
    {
    public:
        IntidimidatingShoutTestImpt() : TestCaseWarrior(STATUS_PASSING) { }
        
        void TestAuraCount(TestPlayer* priest, int count)
        {
            if (priest->HasAura(ClassSpells::Warrior::INTIMIDATING_SHOUT_RNK_1))
            {
                TEST_HAS_NOT_AURA(priest, ClassSpells::Warrior::INTIMIDATING_SHOUT_RNK_1_TRIGGER);
                TEST_AURA_MAX_DURATION(priest, ClassSpells::Warrior::INTIMIDATING_SHOUT_RNK_1, Seconds(8));
                count++;
            }
        }

        void Test() override
        {
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN);

            Position spawn(_location);
            spawn.MoveInFront(_location, 9.0f);
            TestPlayer* priest1 = SpawnPlayer(CLASS_PRIEST, RACE_HUMAN, 70, spawn);
            TestPlayer* priest2 = SpawnPlayer(CLASS_PRIEST, RACE_HUMAN);
            TestPlayer* priest3 = SpawnPlayer(CLASS_PRIEST, RACE_HUMAN);
            TestPlayer* priest4 = SpawnPlayer(CLASS_PRIEST, RACE_HUMAN);
            TestPlayer* priest5 = SpawnPlayer(CLASS_PRIEST, RACE_HUMAN);
            TestPlayer* priest6 = SpawnPlayer(CLASS_PRIEST, RACE_HUMAN);
            TestPlayer* priest7 = SpawnPlayer(CLASS_PRIEST, RACE_HUMAN);

            warrior->DisableRegeneration(true);

            TEST_CAST(warrior, warrior, ClassSpells::Warrior::INTIMIDATING_SHOUT_RNK_1);

            // Cooldown
            TEST_HAS_COOLDOWN(warrior, ClassSpells::Warrior::INTIMIDATING_SHOUT_RNK_1, Minutes(3));

            // Trigger on target
            TEST_AURA_MAX_DURATION(priest1, ClassSpells::Warrior::INTIMIDATING_SHOUT_RNK_1_TRIGGER, Seconds(8));
            // https://youtu.be/9DA10SgPsj4?t=1m14s
            // Target should have only one debuff
            TEST_ASSERT(!priest1->HasAura(ClassSpells::Warrior::INTIMIDATING_SHOUT_RNK_1));

            // Fear on nearby enemies
            int count = 0;
            TestAuraCount(priest2, count);
            TestAuraCount(priest3, count);
            TestAuraCount(priest4, count);
            TestAuraCount(priest5, count);
            TestAuraCount(priest6, count);
            TestAuraCount(priest7, count);
            TEST_ASSERT(count <= 5);

            // Target effect should break on damage
            Unit::DealDamage(warrior, priest1, 1);
            Wait(500);
            TEST_ASSERT(!priest1->HasAura(ClassSpells::Warrior::INTIMIDATING_SHOUT_RNK_1_TRIGGER));

            // Rage cost
            uint32 expectedIntidimidatingShoutRageCost = 25 * 10;
            TEST_POWER_COST(warrior, priest1, ClassSpells::Warrior::INTIMIDATING_SHOUT_RNK_1, POWER_RAGE, expectedIntidimidatingShoutRageCost);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<IntidimidatingShoutTestImpt>();
    }
};

class PummelTest : public TestCaseScript
{
public:
    PummelTest() : TestCaseScript("spells warrior pummel") { }

    class PummelTestImpt : public TestCaseWarrior
    {
    public:
        PummelTestImpt() : TestCaseWarrior(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN);
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_HUMAN);
            Creature* dummy = SpawnCreature();

            warrior->DisableRegeneration(true);

            // Stances
            TestRequiresStance(warrior, priest, false, ClassSpells::Warrior::PUMMEL_RNK_2);
            TestRequiresStance(warrior, priest, false, ClassSpells::Warrior::PUMMEL_RNK_2, ClassSpells::Warrior::BATTLE_STANCE_RNK_1);
            TestRequiresStance(warrior, priest, false, ClassSpells::Warrior::PUMMEL_RNK_2, ClassSpells::Warrior::DEFENSIVE_STANCE_RNK_1);
            TestRequiresStance(warrior, priest, true, ClassSpells::Warrior::PUMMEL_RNK_2, ClassSpells::Warrior::BERSERKER_STANCE_RNK_1);

            TEST_CAST(priest, priest, ClassSpells::Priest::GREATER_HEAL_RNK_7);
            Wait(500);

            TEST_CAST(warrior, warrior, ClassSpells::Warrior::PUMMEL_RNK_2);

            // Cooldown
            TEST_HAS_COOLDOWN(warrior, ClassSpells::Warrior::PUMMEL_RNK_2, Seconds(10));

            Wait(100);
            // Make sure it hits
            uint32 startHealth = priest->GetHealth();
            for (int i = 0; i < 30; i++)
            {
                TC_LOG_TRACE("test.unit_test", "Count: %i", i);
                TEST_CAST(warrior, priest, ClassSpells::Warrior::PUMMEL_RNK_2, SPELL_CAST_OK, TriggerCastFlags(TRIGGERED_IGNORE_POWER_AND_REAGENT_COST | TRIGGERED_IGNORE_SPELL_AND_CATEGORY_CD));
            }
            TEST_ASSERT(priest->GetHealth() < startHealth);

            // Prevent holy cast
            Wait(500);
            TEST_CAST(priest, priest, ClassSpells::Priest::FLASH_HEAL_RNK_9, SPELL_FAILED_SILENCED);
            TEST_CAST(priest, priest, ClassSpells::Priest::RENEW_RNK_12, SPELL_FAILED_SILENCED);
            TEST_CAST(priest, warrior, ClassSpells::Priest::SHADOW_WORD_PAIN_RNK_10);
            Wait(4000);
            TEST_CAST(priest, priest, ClassSpells::Priest::FLASH_HEAL_RNK_9);

            // Damage
            float const armorFactor = 1 - (dummy->GetArmor() / (dummy->GetArmor() + 10557.5));
            uint32 const expectedPummel = ClassSpellsDamage::Warrior::PUMMEL_RNK_2 * armorFactor;
            uint32 const expectedPummelCrit = ClassSpellsDamage::Warrior::PUMMEL_RNK_2 * 2.0f * armorFactor;
            TEST_DIRECT_SPELL_DAMAGE(warrior, dummy, ClassSpells::Warrior::PUMMEL_RNK_2, expectedPummel, expectedPummel, false);
            TEST_DIRECT_SPELL_DAMAGE(warrior, dummy, ClassSpells::Warrior::PUMMEL_RNK_2, expectedPummelCrit, expectedPummelCrit, true);


            // Rage cost
            uint32 expectedPummelRageCost = 10 * 10;
            TEST_POWER_COST(warrior, priest, ClassSpells::Warrior::PUMMEL_RNK_2, POWER_RAGE, expectedPummelRageCost);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<PummelTestImpt>();
    }
};

class RecklessnessTest : public TestCaseScript
{
public:
    RecklessnessTest() : TestCaseScript("spells warrior recklessness") { }

    class RecklessnessTestImpt : public TestCaseWarrior
    {
    public:
        RecklessnessTestImpt() : TestCaseWarrior(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN);
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_HUMAN);
            Creature* creature = SpawnCreature(8, true);

            warrior->DisableRegeneration(true);
            creature->DisableRegeneration(true);

            TestRequiresStance(warrior, creature, false, ClassSpells::Warrior::RECKLESSNESS_RNK_1);
            TestRequiresStance(warrior, creature, false, ClassSpells::Warrior::RECKLESSNESS_RNK_1, ClassSpells::Warrior::DEFENSIVE_STANCE_RNK_1);
            TestRequiresStance(warrior, creature, false, ClassSpells::Warrior::RECKLESSNESS_RNK_1, ClassSpells::Warrior::BATTLE_STANCE_RNK_1);
            TestRequiresStance(warrior, creature, true, ClassSpells::Warrior::RECKLESSNESS_RNK_1, ClassSpells::Warrior::BERSERKER_STANCE_RNK_1);

            TEST_CAST(warrior, warrior, ClassSpells::Warrior::RECKLESSNESS_RNK_1);

            // Aura duration
            TEST_AURA_MAX_DURATION(warrior, ClassSpells::Warrior::RECKLESSNESS_RNK_1, Seconds(15));

            // Cooldown
            TEST_HAS_COOLDOWN(warrior, ClassSpells::Warrior::RECKLESSNESS_RNK_1, Minutes(30));

            // Damage taken increase
            warrior->SetMaxHealth(5000000);
            warrior->SetFullHealth();
            Wait(200);
            float const reckelessnessFactor = 1.2f;
            uint32 const expectedShadowBoltMin = ClassSpellsDamage::Warlock::SHADOW_BOLT_RNK_11_MIN * reckelessnessFactor;
            uint32 const expectedShadowBoltMax = ClassSpellsDamage::Warlock::SHADOW_BOLT_RNK_11_MAX * reckelessnessFactor;
            TEST_DIRECT_SPELL_DAMAGE(warlock, warrior, ClassSpells::Warlock::SHADOW_BOLT_RNK_11, expectedShadowBoltMin, expectedShadowBoltMax, false);

            // Fear immune
            TEST_CAST(warlock, warrior, ClassSpells::Warlock::FEAR_RNK_3, SPELL_CAST_OK, TRIGGERED_CAST_DIRECTLY);
            TEST_ASSERT(!warrior->HasAura(ClassSpells::Warlock::FEAR_RNK_3));

            // Damage
            auto AI = warrior->GetTestingPlayerbotAI();
            for (uint32 i = 0; i < 500; i++)
            {
                TEST_ASSERT(warrior->HasAura(ClassSpells::Warrior::RECKLESSNESS_RNK_1));
                TEST_CAST(warrior, creature, ClassSpells::Warrior::HEROIC_STRIKE_RNK_10, SPELL_CAST_OK, TRIGGERED_FULL_MASK);
                auto damageToTarget = AI->GetSpellDamageDoneInfo(creature);
                auto& data = damageToTarget->back();
                if (data.spellID != ClassSpells::Warrior::HEROIC_STRIKE_RNK_10)
                    continue;
                //use only spells that hit target
                if (data.missInfo != SPELL_MISS_NONE)
                    continue;
                ASSERT_INFO("Heroic Strike %i/499: didn't crit", i);
                TEST_ASSERT(data.crit);
            }
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<RecklessnessTestImpt>();
    }
};

class SlamTest : public TestCaseScript
{
public:
    SlamTest() : TestCaseScript("spells warrior slam") { }

    class SlamTestImpt : public TestCaseWarrior
    {
    public:
        SlamTestImpt() : TestCaseWarrior(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN);
            Creature* creature = SpawnCreature();

            warrior->DisableRegeneration(true);

            TestRequiresMeleeWeapon(warrior, creature, ClassSpells::Warrior::SLAM_RNK_6, true);

            // Rage cost & reset swing
            warrior->Attack(creature, true);
            Wait(250);
            uint32 currentAttackTimer = warrior->GetAttackTimer(BASE_ATTACK);

            uint32 const expectedSlamRage = 15 * 10;
            TEST_POWER_COST(warrior, creature, ClassSpells::Warrior::SLAM_RNK_6, POWER_RAGE, expectedSlamRage);
            Wait(1750); // Cast
            ASSERT_INFO("AttackTime: %u, AttackTimer: %u, Current: %u", warrior->GetAttackTime(BASE_ATTACK), warrior->GetAttackTimer(BASE_ATTACK), currentAttackTimer);
            TEST_ASSERT(warrior->GetAttackTimer(BASE_ATTACK) > currentAttackTimer);

            // Damage -- Apolyon, the Soul-Render - 404-607 damage
            float const apolyonSpeed = 3.4f;
            float const AP = warrior->GetTotalAttackPowerValue(BASE_ATTACK);
            float const armorFactor = 1 - (creature->GetArmor() / (creature->GetArmor() + 10557.5));
            uint32 const weaponMinDamage = 404 + (AP / 14 * apolyonSpeed) + ClassSpellsDamage::Warrior::SLAM_RNK_6;
            uint32 const weaponMaxDamage = 607 + (AP / 14 * apolyonSpeed) + ClassSpellsDamage::Warrior::SLAM_RNK_6;
            uint32 const expectedSlamMin = weaponMinDamage * armorFactor;
            uint32 const expectedSlamMax = weaponMaxDamage * armorFactor;
            uint32 const expectedSlamCritMin = weaponMinDamage * 2.0f * armorFactor;
            uint32 const expectedSlamCritMax = weaponMaxDamage * 2.0f * armorFactor;
            TEST_DIRECT_SPELL_DAMAGE(warrior, creature, ClassSpells::Warrior::SLAM_RNK_6, expectedSlamMin, expectedSlamMax, false);
            TEST_DIRECT_SPELL_DAMAGE(warrior, creature, ClassSpells::Warrior::SLAM_RNK_6, expectedSlamCritMin, expectedSlamCritMax, true);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<SlamTestImpt>();
    }
};

class VictoryRushTest : public TestCaseScript
{
public:
    VictoryRushTest() : TestCaseScript("spells warrior victory_rush") { }

    class VictoryRushTestImpt : public TestCaseWarrior
    {
    public:
        VictoryRushTestImpt() : TestCaseWarrior(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN);
            TestPlayer* enemy1 = SpawnPlayer(CLASS_WARRIOR, RACE_HUMAN, 1);
            TestPlayer* enemy70 = SpawnPlayer(CLASS_WARRIOR, RACE_HUMAN);
            Creature* creature = SpawnCreature();

            enemy1->SetHealth(1);
            enemy70->SetHealth(1);
            Wait(100); // Needed

            TestRequiresStance(warrior, creature, false, ClassSpells::Warrior::VICTORY_RUSH_RNK_1);
            TestRequiresStance(warrior, creature, false, ClassSpells::Warrior::VICTORY_RUSH_RNK_1, ClassSpells::Warrior::DEFENSIVE_STANCE_RNK_1);
            TestRequiresStance(warrior, creature, true, ClassSpells::Warrior::VICTORY_RUSH_RNK_1, ClassSpells::Warrior::BERSERKER_STANCE_RNK_1);
            TestRequiresStance(warrior, creature, true, ClassSpells::Warrior::VICTORY_RUSH_RNK_1, ClassSpells::Warrior::BATTLE_STANCE_RNK_1);

            // Kill level 1
            for (uint32 i = 0; i < 50; i++)
                warrior->AttackerStateUpdate(enemy1, BASE_ATTACK);
            Wait(100); // Needed
            TEST_ASSERT(enemy1->IsDead());
            TEST_ASSERT(!warrior->HasAura(ClassSpells::Warrior::VICTORY_RUSH_RNK_1));

            // Kill level 70
            for (uint32 i = 0; i < 50; i++)
                warrior->AttackerStateUpdate(enemy70, BASE_ATTACK);
            Wait(100); // Needed
            TEST_ASSERT(enemy70->IsDead());

            // Aura
            TEST_AURA_MAX_DURATION(warrior, ClassSpells::Warrior::VICTORY_RUSH_RNK_1, Seconds(20));
            TEST_CAST(warrior, creature, ClassSpells::Warrior::VICTORY_RUSH_RNK_1);
            TEST_ASSERT(!warrior->HasAura(ClassSpells::Warrior::VICTORY_RUSH_RNK_1));

            // Damage
            float const victoryRushAPFactor = 0.45f;
            float const AP = warrior->GetTotalAttackPowerValue(BASE_ATTACK);
            float const armorFactor = 1 - (creature->GetArmor() / (creature->GetArmor() + 10557.5));
            uint32 const expectedVictoryRush = AP * victoryRushAPFactor * armorFactor;
            uint32 const expectedVictoryRushCrit = AP * victoryRushAPFactor * 2.0f * armorFactor;
            TEST_DIRECT_SPELL_DAMAGE(warrior, creature, ClassSpells::Warrior::VICTORY_RUSH_RNK_1, expectedVictoryRush, expectedVictoryRush, false);
            TEST_DIRECT_SPELL_DAMAGE(warrior, creature, ClassSpells::Warrior::VICTORY_RUSH_RNK_1, expectedVictoryRushCrit, expectedVictoryRushCrit, true);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<VictoryRushTestImpt>();
    }
};

class WhirlwindTest : public TestCaseScript
{
public:
    WhirlwindTest() : TestCaseScript("spells warrior whirlwind") { }

    class WhirlwindTestImpt : public TestCaseWarrior
    {
    public:
        WhirlwindTestImpt() : TestCaseWarrior(STATUS_INCOMPLETE) { }

        bool HasLostHealth(Unit* victim, uint32 startHealth)
        {
            return victim->GetHealth() < startHealth;
        }

        void Test() override
        {
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN);
            Creature* creature1 = SpawnCreature();

            // Stances & weapon
            TestRequiresStance(warrior, creature1, false, ClassSpells::Warrior::WHIRLWIND_RNK_1);
            TestRequiresStance(warrior, creature1, false, ClassSpells::Warrior::WHIRLWIND_RNK_1, ClassSpells::Warrior::BATTLE_STANCE_RNK_1);
            TestRequiresStance(warrior, creature1, false, ClassSpells::Warrior::WHIRLWIND_RNK_1, ClassSpells::Warrior::DEFENSIVE_STANCE_RNK_1);
            TestRequiresStance(warrior, creature1, true, ClassSpells::Warrior::WHIRLWIND_RNK_1, ClassSpells::Warrior::BERSERKER_STANCE_RNK_1);
            TestRequiresMeleeWeapon(warrior, creature1, ClassSpells::Warrior::WHIRLWIND_RNK_1, false);

            // Range
            creature1->SetFullHealth();
            Creature* creature2 = SpawnCreature();
            Creature* creature3 = SpawnCreature();
            Position spawn(_location);
            spawn.MoveInFront(spawn, 12.0f);
            Creature* furtherCreature = SpawnCreatureWithPosition(spawn);

            uint32 const startHealth1 = creature1->GetHealth();
            uint32 const startHealth2 = creature2->GetHealth();
            uint32 const startHealth3 = creature3->GetHealth();
            uint32 const startHealthFurther = furtherCreature->GetHealth();

            for (uint32 i = 0; i < 50; i++)
                TEST_CAST(warrior, creature1, ClassSpells::Warrior::WHIRLWIND_RNK_1, SPELL_CAST_OK, TriggerCastFlags(TRIGGERED_CAST_DIRECTLY | TRIGGERED_IGNORE_POWER_AND_REAGENT_COST | TRIGGERED_IGNORE_SPELL_AND_CATEGORY_CD));
            TEST_ASSERT(creature1->GetHealth() < startHealth1);
            TEST_ASSERT(creature2->GetHealth() < startHealth2);
            TEST_ASSERT(creature3->GetHealth() < startHealth3);
            TEST_ASSERT(furtherCreature->GetHealth() == startHealthFurther);

            // Max 4, rage cost & cooldown
            warrior->GetSpellHistory()->ResetAllCooldowns();
            Creature* creature4 = SpawnCreature();
            Creature* creature5 = SpawnCreature();
            uint32 const startHealth4 = creature4->GetHealth();
            uint32 const startHealth5 = creature5->GetHealth();

            creature1->SetFullHealth();
            creature2->SetFullHealth();
            creature3->SetFullHealth();

            //Test power cost + cooldown + target count
            uint32 lostHealthCount = 0; 
            TEST_CAST(warrior, creature1, ClassSpells::Warrior::WHIRLWIND_RNK_1);
            TEST_HAS_COOLDOWN(warrior, ClassSpells::Warrior::WHIRLWIND_RNK_1, Seconds(10));
            lostHealthCount += uint32(HasLostHealth(creature1, startHealth1));
            lostHealthCount += uint32(HasLostHealth(creature2, startHealth1));
            lostHealthCount += uint32(HasLostHealth(creature3, startHealth1));
            //creature 4 is spawned further away
            lostHealthCount += uint32(HasLostHealth(creature4, startHealth1));
            lostHealthCount += uint32(HasLostHealth(creature5, startHealth1));
            ASSERT_INFO("lostHealthCount = %u", lostHealthCount);
            TEST_ASSERT(lostHealthCount <= 4);  // FAILS HERE!

            //Kill all creatures to clear space for damage test
            creature2->KillSelf();
            creature3->KillSelf();
            furtherCreature->KillSelf();
            creature4->KillSelf();
            creature5->KillSelf();

            // Damage
            float const normalizedSwordSpeed = 2.4f;
            float const AP = warrior->GetTotalAttackPowerValue(BASE_ATTACK);
            float const armorFactor = 1 - (creature1->GetArmor() / (creature1->GetArmor() + 10557.5));
            // MH -- 214 - 398
            uint32 const expectedWhirlwindMHMin = (214 + AP / 14 * normalizedSwordSpeed) * armorFactor;
            uint32 const expectedWhirlwindMHMax = (398 + AP / 14 * normalizedSwordSpeed) * armorFactor;
            TEST_DIRECT_SPELL_DAMAGE(warrior, creature1, ClassSpells::Warrior::WHIRLWIND_RNK_1, expectedWhirlwindMHMin, expectedWhirlwindMHMax, false);
            TEST_DIRECT_SPELL_DAMAGE(warrior, creature1, ClassSpells::Warrior::WHIRLWIND_RNK_1, expectedWhirlwindMHMin * 2.0f, expectedWhirlwindMHMax * 2.0f, true);
            // OH -- 107 - 199
            uint32 const expectedWhirlwindOHMin = (107 + AP / 14 * normalizedSwordSpeed) / 2 * armorFactor;
            uint32 const expectedWhirlwindOHMax = (199 + AP / 14 * normalizedSwordSpeed) / 2 * armorFactor;
            TEST_DIRECT_SPELL_DAMAGE(warrior, creature1, ClassSpells::Warrior::WHIRLWIND_RNK_1_TRIGGER, expectedWhirlwindOHMin, expectedWhirlwindOHMax, false);
            TEST_DIRECT_SPELL_DAMAGE(warrior, creature1, ClassSpells::Warrior::WHIRLWIND_RNK_1_TRIGGER, expectedWhirlwindOHMin * 2.0f, expectedWhirlwindOHMax * 2.0f, true);

            // Power cost
            uint32 const expectedWhirldwindRage = 25 * 10;
            TEST_POWER_COST(warrior, creature1, ClassSpells::Warrior::WHIRLWIND_RNK_1, POWER_RAGE, expectedWhirldwindRage);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<WhirlwindTestImpt>();
    }
};

class BloodrageTest : public TestCaseScript
{
public:
    BloodrageTest() : TestCaseScript("spells warrior bloodrage") { }

    class BloodrageTestImpt : public TestCaseWarrior
    {
    public:
        BloodrageTestImpt() : TestCaseWarrior(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_HUMAN);

            warrior->DisableRegeneration(true);

            // Health cost
            float const bloodrageFactor = 0.16f;
            uint32 const baseHealth = warrior->GetMaxHealth() - (20 + (warrior->GetStat(STAT_STAMINA) - 20) * 10.0f);
            uint32 const healthCost = baseHealth * bloodrageFactor;
            uint32 const expectedHealth = warrior->GetHealth() - healthCost;

            // Health
            TEST_CAST(warrior, warrior, ClassSpells::Warrior::BLOODRAGE_RNK_1);
            ASSERT_INFO("Max: %u, Health: %u, Expected: %u", warrior->GetMaxHealth(), warrior->GetHealth(), expectedHealth);
            TEST_ASSERT(warrior->GetHealth() == expectedHealth);

            // Cooldown
            TEST_HAS_COOLDOWN(warrior, ClassSpells::Warrior::BLOODRAGE_RNK_1, Minutes(1));

            // Aura
            TEST_AURA_MAX_DURATION(warrior, ClassSpells::Warrior::BLOODRAGE_RNK_1_TRIGGER, Seconds(10));

            // Rage gain
            TEST_ASSERT(warrior->GetPower(POWER_RAGE) == 10 * 10);
            Wait(10000);
            TEST_ASSERT(Between<uint32>(warrior->GetPower(POWER_RAGE), 20 * 10 - 3, 20 * 10)); // might have lower because out of combat
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<BloodrageTestImpt>();
    }
};

class DisarmTest : public TestCaseScript
{
public:
    DisarmTest() : TestCaseScript("spells warrior disarm") { }

    class DisarmTestImpt : public TestCaseWarrior
    {
    public:
        DisarmTestImpt() : TestCaseWarrior(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN);

            Position spawn(_location);
            spawn.MoveInFront(spawn, 3.0f);
            TestPlayer* rogue = SpawnPlayer(CLASS_ROGUE, RACE_HUMAN, 70, spawn);
            TestPlayer* rogue2 = SpawnPlayer(CLASS_ROGUE, RACE_HUMAN, 70, spawn);
            EQUIP_ITEM(rogue2, 32837); // Warglaive of Azzinoth MH
            Creature* dummy = SpawnCreature();

            // Rogue setup
            RemoveAllEquipedItems(rogue);
            float const armorFactor = 1 - (dummy->GetArmor() / (dummy->GetArmor() + 10557.5));
            float const startAP = rogue->GetTotalAttackPowerValue(BASE_ATTACK);
            float const startDmgMin = rogue->GetFloatValue(UNIT_FIELD_MINDAMAGE);
            float const startDmgMax = rogue->GetFloatValue(UNIT_FIELD_MAXDAMAGE);
            uint32 const expectedStartMinDmg = startDmgMin * armorFactor;
            uint32 const expectedStartMaxDmg = startDmgMax * armorFactor;
            TEST_MELEE_DAMAGE(rogue, dummy, BASE_ATTACK, expectedStartMinDmg, expectedStartMaxDmg, false);

            EQUIP_ITEM(rogue, 32837); // Warglaive of Azzinoth MH
            Wait(1500);
            EQUIP_ITEM(rogue, 32838); // Warglaive of Azzinoth OH

            float const rogueMHSpeed = 2.8f;
            float const rogueOHSpeed = 1.4f;
            float const rogueAP = rogue->GetTotalAttackPowerValue(BASE_ATTACK);
            uint32 const expectedMHMinDmg = (214 + (rogueAP / 14 * rogueMHSpeed)) * armorFactor;
            uint32 const expectedMHMaxDmg = (398 + (rogueAP / 14 * rogueMHSpeed)) * armorFactor;
            uint32 const expectedOHMinDmg = (107 + (rogueAP / 14 * rogueOHSpeed)) / 2.0f * armorFactor;
            uint32 const expectedOHMaxDmg = (199 + (rogueAP / 14 * rogueOHSpeed)) / 2.0f * armorFactor;
            TEST_MELEE_DAMAGE(rogue, dummy, BASE_ATTACK, expectedMHMinDmg, expectedMHMaxDmg, false);
            TEST_MELEE_DAMAGE(rogue, dummy, OFF_ATTACK, expectedOHMinDmg, expectedOHMaxDmg, false);

            // Stances & weapon
            TestRequiresStance(warrior, rogue2, false, ClassSpells::Warrior::DISARM_RNK_1);
            TestRequiresStance(warrior, rogue2, false, ClassSpells::Warrior::DISARM_RNK_1, ClassSpells::Warrior::BATTLE_STANCE_RNK_1);
            TestRequiresStance(warrior, rogue2, false, ClassSpells::Warrior::DISARM_RNK_1, ClassSpells::Warrior::BERSERKER_STANCE_RNK_1);
            TestRequiresStance(warrior, rogue2, true, ClassSpells::Warrior::DISARM_RNK_1, ClassSpells::Warrior::DEFENSIVE_STANCE_RNK_1);

            TEST_CAST(warrior, rogue, ClassSpells::Warrior::DISARM_RNK_1);

            // Aura
            TEST_AURA_MAX_DURATION(rogue, ClassSpells::Warrior::DISARM_RNK_1, Seconds(10));

            // Cooldown
            TEST_HAS_COOLDOWN(warrior, ClassSpells::Warrior::DISARM_RNK_1, Minutes(1));

            // Flag
            TEST_ASSERT(rogue->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISARMED));

            // Damage under disarmed effect
            uint32 gainedAP = rogueAP - startAP;
            uint32 const disarmedMin = (startDmgMin + gainedAP / 14 * 2.0f) * armorFactor;
            uint32 const disarmedMax = (startDmgMax + gainedAP / 14 * 2.0f) * armorFactor;
            TEST_MELEE_DAMAGE(rogue, dummy, BASE_ATTACK, disarmedMin, disarmedMax, false);
            rogue->AddAura(ClassSpells::Warrior::DISARM_RNK_1, rogue);
            TEST_MELEE_DAMAGE(rogue, dummy, OFF_ATTACK, expectedOHMinDmg, expectedOHMaxDmg, false);

            // Power cost
            uint32 expectedDisarmRage = 20 * 10;
            TEST_POWER_COST(warrior, rogue, ClassSpells::Warrior::DISARM_RNK_1, POWER_RAGE, expectedDisarmRage);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<DisarmTestImpt>();
    }
};

class InterveneTest : public TestCaseScript
{
public:
    InterveneTest() : TestCaseScript("spells warrior intervene") { }

    class InterveneTestImpt : public TestCaseWarrior
    {
    public:
        InterveneTestImpt() : TestCaseWarrior(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN);
            TestPlayer* mage = SpawnPlayer(CLASS_MAGE, RACE_GNOME);

            Position spawn(_location);
            spawn.MoveInFront(spawn, 15.0f);
            TestPlayer* rogue = SpawnPlayer(CLASS_ROGUE, RACE_NIGHTELF, 70, spawn);
            TestPlayer* shaman = SpawnPlayer(CLASS_SHAMAN, RACE_TAUREN, 70, spawn);

            spawn.MoveInFront(spawn, 5.0f);
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF, 70, spawn);
            spawn.MoveInFront(spawn, 30.0f);
            TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN, 70, spawn);

            // Group
            GroupPlayer(warrior, shaman);

            // Stances
            TestRequiresStance(warrior, shaman, false, ClassSpells::Warrior::INTERVENE_RNK_1);
            TestRequiresStance(warrior, shaman, false, ClassSpells::Warrior::INTERVENE_RNK_1, ClassSpells::Warrior::BATTLE_STANCE_RNK_1);
            TestRequiresStance(warrior, shaman, false, ClassSpells::Warrior::INTERVENE_RNK_1, ClassSpells::Warrior::BERSERKER_STANCE_RNK_1);
            TestRequiresStance(warrior, shaman, true, ClassSpells::Warrior::INTERVENE_RNK_1, ClassSpells::Warrior::DEFENSIVE_STANCE_RNK_1);
            warrior->Relocate(_location);

            // Range
            TEST_CAST(warrior, priest, ClassSpells::Warrior::INTERVENE_RNK_1, SPELL_FAILED_TOO_CLOSE, TRIGGERED_IGNORE_POWER_AND_REAGENT_COST);
            TEST_CAST(warrior, druid, ClassSpells::Warrior::INTERVENE_RNK_1, SPELL_FAILED_OUT_OF_RANGE, TRIGGERED_IGNORE_POWER_AND_REAGENT_COST);

            // Setup
            warrior->DisableRegeneration(true);
            warrior->Attack(mage, true);
            Wait(2000);

            uint32 startingRage = warrior->GetPower(POWER_RAGE);
            TEST_CAST(warrior, shaman, ClassSpells::Warrior::INTERVENE_RNK_1);

            // Rage cost
            uint32 expectedInterveneRage = 10 * 10;
            TEST_ASSERT(warrior->GetPower(POWER_RAGE) == startingRage - expectedInterveneRage);

            // Cooldown
            TEST_HAS_COOLDOWN(warrior, ClassSpells::Warrior::INTERVENE_RNK_1, Seconds(30));

            // Aura
            TEST_AURA_MAX_DURATION(shaman, ClassSpells::Warrior::INTERVENE_RNK_1, Seconds(10));

            // Take an attack
            Wait(3000);
            TEST_CAST(rogue, shaman, ClassSpells::Rogue::SAP_RNK_3);
            Wait(100);
            TEST_ASSERT(!shaman->HasAura(ClassSpells::Rogue::SAP_RNK_3));
            TEST_ASSERT(warrior->HasAura(ClassSpells::Rogue::SAP_RNK_3));
            TEST_ASSERT(!shaman->HasAura(ClassSpells::Warrior::INTERVENE_RNK_1));
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<InterveneTestImpt>();
    }
};

class RevengeTest : public TestCaseScript
{
public:
    RevengeTest() : TestCaseScript("spells warrior revenge") { }

    class RevengeTestImpt : public TestCaseWarrior
    {
    public:
        RevengeTestImpt() : TestCaseWarrior(STATUS_PASSING) { }

        void TestRevengeTrigger(TestPlayer* warrior, TestPlayer* rogue)
        {
            bool dodge = false;
            bool parry = false;
            bool block = false;

            auto AI = rogue->GetTestingPlayerbotAI();
            for (uint32 i = 0; i < 500; i++)
            {
                rogue->AttackerStateUpdate(warrior, BASE_ATTACK);
                auto damageToTarget = AI->GetMeleeDamageDoneInfo(warrior);
                ASSERT_INFO("After 500 hits: dodge: %i, parry: %i, block: %i", int(dodge), int(parry), int(block));
                TEST_ASSERT(i != 499);
                TEST_ASSERT(damageToTarget->size() == i + 1);
                auto& data = damageToTarget->back();

                switch (data.damageInfo.HitOutCome)
                {
                    case MELEE_HIT_DODGE: 
                        TEST_CAST(warrior, rogue, ClassSpells::Warrior::REVENGE_RNK_8, SPELL_CAST_OK, TriggerCastFlags(TRIGGERED_IGNORE_POWER_AND_REAGENT_COST | TRIGGERED_IGNORE_SPELL_AND_CATEGORY_CD));
                        dodge = true;
                        break;
                    case MELEE_HIT_PARRY:
                        TEST_CAST(warrior, rogue, ClassSpells::Warrior::REVENGE_RNK_8, SPELL_CAST_OK, TriggerCastFlags(TRIGGERED_IGNORE_POWER_AND_REAGENT_COST | TRIGGERED_IGNORE_SPELL_AND_CATEGORY_CD));
                        parry = true;
                        break;
                    case MELEE_HIT_BLOCK:
                        TEST_CAST(warrior, rogue, ClassSpells::Warrior::REVENGE_RNK_8, SPELL_CAST_OK, TriggerCastFlags(TRIGGERED_IGNORE_POWER_AND_REAGENT_COST | TRIGGERED_IGNORE_SPELL_AND_CATEGORY_CD));
                        block = true;
                        break;
                    default: break;
                }

                warrior->SetFullHealth();

                if (dodge && parry && block)
                    break;
            }
        }

        void Test() override
        {
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN);

            Position spawn(_location);
            spawn.MoveInFront(spawn, 3.0f);
            TestPlayer* rogue = SpawnPlayer(CLASS_ROGUE, RACE_HUMAN, 70, spawn);
            Creature* dummy = SpawnCreature(8, true);

            // Stances & weapon
            TestRequiresStance(warrior, dummy, false, ClassSpells::Warrior::REVENGE_RNK_8);
            TestRequiresStance(warrior, dummy, false, ClassSpells::Warrior::REVENGE_RNK_8, ClassSpells::Warrior::BATTLE_STANCE_RNK_1);
            TestRequiresStance(warrior, dummy, false, ClassSpells::Warrior::REVENGE_RNK_8, ClassSpells::Warrior::BERSERKER_STANCE_RNK_1);
            TestRequiresStance(warrior, dummy, true, ClassSpells::Warrior::REVENGE_RNK_8, ClassSpells::Warrior::DEFENSIVE_STANCE_RNK_1);
            TestRequiresMeleeWeapon(warrior, dummy, ClassSpells::Warrior::REVENGE_RNK_8, false, SPELL_FAILED_CASTER_AURASTATE);
            EQUIP_ITEM(warrior, 34185); // Shield

            // Triggers
            Wait(5200);
            TestRevengeTrigger(warrior, rogue);

            // Cooldown
            TEST_COOLDOWN(warrior, dummy, ClassSpells::Warrior::REVENGE_RNK_8, Seconds(5));

            // Damage
            float const armorFactor = 1 - (dummy->GetArmor() / (dummy->GetArmor() + 10557.5));
            uint32 const expectedRevengeMin = ClassSpellsDamage::Warrior::REVENGE_RNK_8_MIN * armorFactor;
            uint32 const expectedRevengeMax = ClassSpellsDamage::Warrior::REVENGE_RNK_8_MAX * armorFactor;
            TEST_DIRECT_SPELL_DAMAGE(warrior, dummy, ClassSpells::Warrior::REVENGE_RNK_8, expectedRevengeMin, expectedRevengeMax, false);
            TEST_DIRECT_SPELL_DAMAGE(warrior, dummy, ClassSpells::Warrior::REVENGE_RNK_8, expectedRevengeMin * 2.0f, expectedRevengeMax * 2.0f, true);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<RevengeTestImpt>();
    }
};

class ShieldBashTest : public TestCaseScript
{
public:
    ShieldBashTest() : TestCaseScript("spells warrior shield_bash") { }

    class ShieldBashTestImpt : public TestCaseWarrior
    {
    public:
        ShieldBashTestImpt() : TestCaseWarrior(STATUS_PARTIAL) { }

        void Test() override
        {
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN);
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_HUMAN);
            Creature* dummy = SpawnCreature();

            warrior->DisableRegeneration(true);
            EQUIP_ITEM(warrior, 34185); // Shield

            // Stances
            TestRequiresStance(warrior, priest, false, ClassSpells::Warrior::SHIELD_BASH_RNK_4);
            TestRequiresStance(warrior, priest, false, ClassSpells::Warrior::SHIELD_BASH_RNK_4, ClassSpells::Warrior::BERSERKER_STANCE_RNK_1);
            TestRequiresStance(warrior, priest, true, ClassSpells::Warrior::SHIELD_BASH_RNK_4, ClassSpells::Warrior::DEFENSIVE_STANCE_RNK_1);
            TestRequiresStance(warrior, priest, true, ClassSpells::Warrior::SHIELD_BASH_RNK_4, ClassSpells::Warrior::BATTLE_STANCE_RNK_1);

            TEST_CAST(priest, priest, ClassSpells::Priest::GREATER_HEAL_RNK_7);
            Wait(500);

            // Rage cost
            uint32 expectedShieldBashRageCost = 10 * 10;
            TEST_POWER_COST(warrior, priest, ClassSpells::Warrior::SHIELD_BASH_RNK_4, POWER_RAGE, expectedShieldBashRageCost);

            // TODO: dazes target

            // Cooldown
            TEST_COOLDOWN(warrior, priest, ClassSpells::Warrior::SHIELD_BASH_RNK_4, Seconds(12));

            Wait(100);
            // Make sure it hits
            uint32 startHealth = priest->GetHealth();
            for (int i = 0; i < 30; i++)
            {
                TC_LOG_TRACE("test.unit_test", "Count: %i", i);
                TEST_CAST(warrior, priest, ClassSpells::Warrior::SHIELD_BASH_RNK_4, SPELL_CAST_OK, TriggerCastFlags(TRIGGERED_IGNORE_POWER_AND_REAGENT_COST | TRIGGERED_IGNORE_SPELL_AND_CATEGORY_CD));
            }
            TEST_ASSERT(priest->GetHealth() < startHealth);

            // Prevent holy cast
            Wait(500);
            TEST_CAST(priest, priest, ClassSpells::Priest::FLASH_HEAL_RNK_9, SPELL_FAILED_SILENCED);
            TEST_CAST(priest, priest, ClassSpells::Priest::RENEW_RNK_12, SPELL_FAILED_SILENCED);
            TEST_CAST(priest, warrior, ClassSpells::Priest::SHADOW_WORD_PAIN_RNK_10);
            Wait(6000);
            TEST_CAST(priest, priest, ClassSpells::Priest::FLASH_HEAL_RNK_9);

            // Damage
            float const armorFactor = 1 - (dummy->GetArmor() / (dummy->GetArmor() + 10557.5));
            uint32 const expectedShieldBash = ClassSpellsDamage::Warrior::SHIELD_BASH_RNK_4 * armorFactor;
            uint32 const expectedShieldBashCrit = ClassSpellsDamage::Warrior::SHIELD_BASH_RNK_4 * 2.0f * armorFactor;
            TEST_DIRECT_SPELL_DAMAGE(warrior, dummy, ClassSpells::Warrior::SHIELD_BASH_RNK_4, expectedShieldBash, expectedShieldBash, false);
            TEST_DIRECT_SPELL_DAMAGE(warrior, dummy, ClassSpells::Warrior::SHIELD_BASH_RNK_4, expectedShieldBashCrit, expectedShieldBashCrit, true);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<ShieldBashTestImpt>();
    }
};

class ShieldBlockTest : public TestCaseScript
{
public:
    ShieldBlockTest() : TestCaseScript("spells warrior shield_block") { }

    class ShieldBlockTestImpt : public TestCaseWarrior
    {
    public:
        ShieldBlockTestImpt() : TestCaseWarrior(STATUS_PASSING) { }

        void TestShieldBlock(TestPlayer* warrior, float expectedResult, bool castShieldBlock = false)
        {
            uint32 sampleSize;
            float resultingAbsoluteTolerance;

            Position spawn(_location);
            spawn.MoveInFront(spawn, 3.0f);

            TestPlayer* rogue = SpawnPlayer(CLASS_ROGUE, RACE_HUMAN, 70, spawn);
            rogue->SetFacingToObject(warrior);

            _GetPercentApproximationParams(sampleSize, resultingAbsoluteTolerance, expectedResult / 100.0f);
            //auto AI = rogue->GetTestingPlayerbotAI();
            for (uint32 i = 0; i < sampleSize; i++)
            {
                if (castShieldBlock && !warrior->HasAura(ClassSpells::Warrior::SHIELD_BLOCK_RNK_1))
                    warrior->AddAura(ClassSpells::Warrior::SHIELD_BLOCK_RNK_1, warrior);
                rogue->AttackerStateUpdate(warrior, BASE_ATTACK);
                warrior->SetFullHealth();
            }
            TEST_MELEE_OUTCOME_PERCENTAGE(rogue, warrior, BASE_ATTACK, MELEE_HIT_BLOCK, expectedResult, resultingAbsoluteTolerance * 100);
            rogue->KillSelf();
        }

        void Test() override
        {
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN);

            // Stances & weapon
            TestRequiresStance(warrior, warrior, false, ClassSpells::Warrior::SHIELD_BLOCK_RNK_1);
            TestRequiresStance(warrior, warrior, false, ClassSpells::Warrior::SHIELD_BLOCK_RNK_1, ClassSpells::Warrior::BATTLE_STANCE_RNK_1);
            TestRequiresStance(warrior, warrior, false, ClassSpells::Warrior::SHIELD_BLOCK_RNK_1, ClassSpells::Warrior::BERSERKER_STANCE_RNK_1);
            TestRequiresStance(warrior, warrior, false, ClassSpells::Warrior::SHIELD_BLOCK_RNK_1, ClassSpells::Warrior::DEFENSIVE_STANCE_RNK_1, SPELL_FAILED_EQUIPPED_ITEM_CLASS);
            TestRequiresMeleeWeapon(warrior, warrior, ClassSpells::Warrior::SHIELD_BLOCK_RNK_1, false, SPELL_FAILED_EQUIPPED_ITEM_CLASS);
            EQUIP_ITEM(warrior, 8320); //random useless shield

            // Triggers
            float const startBlock = warrior->GetUnitBlockChance(BASE_ATTACK, warrior);
            float const expectedBlock = startBlock + 75.0f;
            TestShieldBlock(warrior, startBlock);
            //@Nasty> can't work, block can be pushed out of attack table
            TestShieldBlock(warrior, expectedBlock, true);

            // Aura
            warrior->ModifyPower(POWER_RAGE, 100); //shield block costs 10 rage
            warrior->CastSpell(warrior, ClassSpells::Warrior::SHIELD_BLOCK_RNK_1);
            TEST_AURA_MAX_DURATION(warrior, ClassSpells::Warrior::SHIELD_BLOCK_RNK_1, Seconds(5));

            // Cooldown
            TEST_HAS_COOLDOWN(warrior, ClassSpells::Warrior::SHIELD_BLOCK_RNK_1, Seconds(5));
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<ShieldBlockTestImpt>();
    }
};

class ShieldWallTest : public TestCaseScript
{
public:
    ShieldWallTest() : TestCaseScript("spells warrior shield_wall") { }

    class ShieldWallTestImpt : public TestCaseWarrior
    {
    public:
        ShieldWallTestImpt() : TestCaseWarrior(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN);

            TestPlayer* rogue = SpawnPlayer(CLASS_ROGUE, RACE_HUMAN);
            EQUIP_ITEM(rogue, 32837); // Warglaive of Azzinoth MH
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_HUMAN);
            TestPlayer* hunter = SpawnPlayer(CLASS_HUNTER, RACE_DWARF);
            EQUIP_ITEM(hunter, 32336); // Black Bow of the Betrayer

            // Stances & weapon
            TestRequiresStance(warrior, warrior, false, ClassSpells::Warrior::SHIELD_WALL_RNK_1);
            TestRequiresStance(warrior, warrior, false, ClassSpells::Warrior::SHIELD_WALL_RNK_1, ClassSpells::Warrior::BATTLE_STANCE_RNK_1);
            TestRequiresStance(warrior, warrior, false, ClassSpells::Warrior::SHIELD_WALL_RNK_1, ClassSpells::Warrior::BERSERKER_STANCE_RNK_1);
            TestRequiresStance(warrior, warrior, false, ClassSpells::Warrior::SHIELD_WALL_RNK_1, ClassSpells::Warrior::DEFENSIVE_STANCE_RNK_1, SPELL_FAILED_EQUIPPED_ITEM_CLASS);
            TestRequiresMeleeWeapon(warrior, warrior, ClassSpells::Warrior::SHIELD_WALL_RNK_1, false, SPELL_FAILED_EQUIPPED_ITEM_CLASS);
            EQUIP_ITEM(warrior, 34185); // Shield

            // Aura
            TEST_CAST(warrior, warrior, ClassSpells::Warrior::SHIELD_WALL_RNK_1);
            TEST_AURA_MAX_DURATION(warrior, ClassSpells::Warrior::SHIELD_WALL_RNK_1, Seconds(10));

            // Cooldown
            TEST_HAS_COOLDOWN(warrior, ClassSpells::Warrior::SHIELD_WALL_RNK_1, Minutes(30));

            // 75% less damage
            float const shieldWallFactor = 1 - 0.75f;
            float const defensiveStanceFactor = 1 - 0.1f;
            float const armorFactor = 1 - (warrior->GetArmor() / (warrior->GetArmor() + 10557.5));
            // Spell
            uint32 const expectedSBMin = ClassSpellsDamage::Warlock::SHADOW_BOLT_RNK_11_MIN * defensiveStanceFactor * shieldWallFactor;
            uint32 const expectedSBMax = ClassSpellsDamage::Warlock::SHADOW_BOLT_RNK_11_MAX * defensiveStanceFactor * shieldWallFactor;
            warrior->SetMaxHealth(999999999);
            warrior->SetFullHealth();
            TEST_DIRECT_SPELL_DAMAGE(warlock, warrior, ClassSpells::Warlock::SHADOW_BOLT_RNK_11, expectedSBMin, expectedSBMax, false);
            // Melee
            float const rogueWeaponSpeed = 2.8f;
            float const rogueAP = rogue->GetTotalAttackPowerValue(BASE_ATTACK);
            uint32 const weaponMinDamage = 214 + (rogueAP / 14 * rogueWeaponSpeed);
            uint32 const weaponMaxDamage = 398 + (rogueAP / 14 * rogueWeaponSpeed);
            uint32 const expectedMHMin = weaponMinDamage * armorFactor * defensiveStanceFactor * shieldWallFactor;
            uint32 const expectedMHMax = weaponMaxDamage * armorFactor * defensiveStanceFactor * shieldWallFactor;
            TEST_CAST(warrior, warrior, ClassSpells::Warrior::SHIELD_WALL_RNK_1, SPELL_CAST_OK, TRIGGERED_IGNORE_SPELL_AND_CATEGORY_CD);
            warrior->SetMaxHealth(999999999);
            warrior->SetFullHealth();
            TEST_MELEE_DAMAGE(rogue, warrior, BASE_ATTACK, expectedMHMin, expectedMHMax, false);
            // Ranged
            float const hunterWeaponSpeed = 3.0f;
            float const hunterAP = hunter->GetTotalAttackPowerValue(RANGED_ATTACK);
            uint32 const bowMinDamage = 201 + (hunterAP / 14 * hunterWeaponSpeed);
            uint32 const bowMaxDamage = 374 + (hunterAP / 14 * hunterWeaponSpeed);
            uint32 const expectedRangedMin = bowMinDamage * armorFactor * defensiveStanceFactor * shieldWallFactor;
            uint32 const expectedRangedMax = bowMaxDamage * armorFactor * defensiveStanceFactor * shieldWallFactor;
            TEST_CAST(warrior, warrior, ClassSpells::Warrior::SHIELD_WALL_RNK_1, SPELL_CAST_OK, TRIGGERED_IGNORE_SPELL_AND_CATEGORY_CD);
            warrior->SetMaxHealth(999999999);
            warrior->SetFullHealth();
            TEST_MELEE_DAMAGE(hunter, warrior, RANGED_ATTACK, expectedRangedMin, expectedRangedMax, false);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<ShieldWallTestImpt>();
    }
};

class SpellReflectionTest : public TestCaseScript
{
public:
    SpellReflectionTest() : TestCaseScript("spells warrior spell_reflection") { }

    class SpellReflectionTestImpt : public TestCaseWarrior
    {
    public:
        SpellReflectionTestImpt() : TestCaseWarrior(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN);

            Position spawn(_location);
            spawn.MoveInFront(spawn, 15.0f);
            TestPlayer* fire = SpawnPlayer(CLASS_MAGE, RACE_HUMAN, 70, spawn);
            TestPlayer* frost = SpawnPlayer(CLASS_MAGE, RACE_HUMAN, 70, spawn);

            // Stances & weapon
            TestRequiresStance(warrior, warrior, false, ClassSpells::Warrior::SPELL_REFLECTION_RNK_1);
            TestRequiresStance(warrior, warrior, false, ClassSpells::Warrior::SPELL_REFLECTION_RNK_1, ClassSpells::Warrior::BERSERKER_STANCE_RNK_1);
            TestRequiresStance(warrior, warrior, false, ClassSpells::Warrior::SPELL_REFLECTION_RNK_1, ClassSpells::Warrior::BATTLE_STANCE_RNK_1, SPELL_FAILED_EQUIPPED_ITEM_CLASS);
            TestRequiresStance(warrior, warrior, false, ClassSpells::Warrior::SPELL_REFLECTION_RNK_1, ClassSpells::Warrior::DEFENSIVE_STANCE_RNK_1, SPELL_FAILED_EQUIPPED_ITEM_CLASS);
            TestRequiresMeleeWeapon(warrior, warrior, ClassSpells::Warrior::SPELL_REFLECTION_RNK_1, false, SPELL_FAILED_EQUIPPED_ITEM_CLASS);
            EQUIP_ITEM(warrior, 34185); // Shield

            TEST_CAST(warrior, warrior, ClassSpells::Warrior::SPELL_REFLECTION_RNK_1);

            // Aura
            TEST_AURA_MAX_DURATION(warrior, ClassSpells::Warrior::SPELL_REFLECTION_RNK_1, Seconds(5));

            // Cooldown
            TEST_HAS_COOLDOWN(warrior, ClassSpells::Warrior::SPELL_REFLECTION_RNK_1, Seconds(10));

            // Reflect
            warrior->SetFullHealth();
            fire->SetFullHealth();
            frost->SetFullHealth();
            Wait(100);
            for (int i = 0; i < 3; i++)
            {
                TEST_CAST(fire, warrior, ClassSpells::Mage::FIREBALL_RNK_13, SPELL_CAST_OK, TRIGGERED_FULL_MASK);
                TEST_CAST(frost, warrior, ClassSpells::Mage::FROSTBOLT_RNK_13, SPELL_CAST_OK, TRIGGERED_FULL_MASK);
            }
            Wait(1000);
            TEST_ASSERT(warrior->GetHealth() == warrior->GetMaxHealth());
            TEST_ASSERT(fire->GetHealth() < fire->GetMaxHealth());
            TEST_ASSERT(frost->GetHealth() < frost->GetMaxHealth());
            // https://youtu.be/ohnzL-deZDM?t=1m24s
            // Spell reflection should be removed as soon as the spell is reflected not when the reflected spells hits its caster
            TEST_ASSERT(!warrior->HasAura(ClassSpells::Warrior::SPELL_REFLECTION_RNK_1));

            // Aura removed when no shield
            TEST_CAST(warrior, warrior, ClassSpells::Warrior::SPELL_REFLECTION_RNK_1, SPELL_CAST_OK, TRIGGERED_FULL_MASK);
            EQUIP_ITEM(warrior, 34247); // Apolyon -- 2 Hands
            TEST_ASSERT(!warrior->HasAura(ClassSpells::Warrior::SPELL_REFLECTION_RNK_1));

            // Rage cost
            uint32 expectedSpellReflectionRage = 25 * 10;
            TEST_POWER_COST(warrior, warrior, ClassSpells::Warrior::SPELL_REFLECTION_RNK_1, POWER_RAGE, expectedSpellReflectionRage);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<SpellReflectionTestImpt>();
    }
};

class StanceMasteryTest : public TestCaseScript
{
public:
    StanceMasteryTest() : TestCaseScript("spells warrior stance_mastery") { }

    class StanceMasteryTestImpt : public TestCaseWarrior
    {
    public:
        StanceMasteryTestImpt() : TestCaseWarrior(STATUS_PASSING) { }

        void TestStanceRage(TestPlayer* warrior, uint32 stanceSpellId)
        {
            warrior->SetPower(POWER_RAGE, 1000); // 100 rage
            ASSERT_INFO("Initialize with 1000 rage failed. Rage: %u", warrior->GetPower(POWER_RAGE));
            TEST_ASSERT(warrior->GetPower(POWER_RAGE) == 1000); // 10 rage
            TEST_CAST(warrior, warrior, stanceSpellId);
            ASSERT_INFO("Stance %u, should have 100 rage. Rage: %u", stanceSpellId, warrior->GetPower(POWER_RAGE));
            TEST_ASSERT(warrior->GetPower(POWER_RAGE) == 100); // 10 rage
        }

        void Test() override
        {
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN);
            LearnTalent(warrior, ClassSpells::Warrior::STANCE_MASTERY_RNK_1);
            TestStanceRage(warrior, ClassSpells::Warrior::BATTLE_STANCE_RNK_1);
            TestStanceRage(warrior, ClassSpells::Warrior::BERSERKER_STANCE_RNK_1);
            TestStanceRage(warrior, ClassSpells::Warrior::DEFENSIVE_STANCE_RNK_1);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<StanceMasteryTestImpt>();
    }
};

class SunderArmorTest : public TestCaseScript
{
public:
    SunderArmorTest() : TestCaseScript("spells warrior sunder_armor") { }

    class SunderArmorTestImpt : public TestCaseWarrior
    {
    public:
        SunderArmorTestImpt() : TestCaseWarrior(STATUS_PASSING) { }

        void TestSunderArmor(TestPlayer* warrior, Unit* victim, uint32 startArmor, int sunderArmorStack, uint32 armorReduced)
        {
            uint32 const expectedSunderArmorRage = 15 * 10;
            uint32 const expectedArmor = startArmor - sunderArmorStack * armorReduced;
            TEST_POWER_COST(warrior, victim, ClassSpells::Warrior::SUNDER_ARMOR_RNK_6, POWER_RAGE, expectedSunderArmorRage);
            ASSERT_INFO("Sunder Armor %i stacks -> start armor: %u, current armor: %u, expected: %u", sunderArmorStack, startArmor, victim->GetArmor(), expectedArmor);
            TEST_ASSERT(victim->GetArmor() == expectedArmor);
            Aura* aura = victim->GetAura(ClassSpells::Warrior::SUNDER_ARMOR_RNK_6);
            TEST_ASSERT(aura != nullptr);
            TEST_ASSERT(aura->GetStackAmount() == sunderArmorStack);
            TEST_ASSERT(aura->GetMaxDuration() == 30 * SECOND * IN_MILLISECONDS);
        }

        void Test() override
        {
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN);
            Creature* dummy = SpawnCreature(8, true);
            uint32 startArmor = dummy->GetArmor();

            // Weapon required
            TestRequiresMeleeWeapon(warrior, dummy, ClassSpells::Warrior::SUNDER_ARMOR_RNK_6, false);
            dummy->RemoveAurasDueToSpell(ClassSpells::Warrior::SUNDER_ARMOR_RNK_6);

            // Stack, aura duration, armor reduction
            TestSunderArmor(warrior, dummy, startArmor, 1, 520);
            TestSunderArmor(warrior, dummy, startArmor, 2, 520);
            TestSunderArmor(warrior, dummy, startArmor, 3, 520);
            TestSunderArmor(warrior, dummy, startArmor, 4, 520);
            TestSunderArmor(warrior, dummy, startArmor, 5, 520);
            TestSunderArmor(warrior, dummy, startArmor, 5, 520);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<SunderArmorTestImpt>();
    }
};

class TauntTest : public TestCaseScript
{
public:

    TauntTest() : TestCaseScript("spells warrior taunt") { }

    class TauntTestImpt : public TestCaseWarrior
    {
    public:
        TauntTestImpt() : TestCaseWarrior(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN);

            Position spawn3m(_location);
            spawn3m.MoveInFront(_location, 3.0f);
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_HUMAN, 70, spawn3m);
            Creature* creature = SpawnCreatureWithPosition(spawn3m, 6);

            // Setup
            creature->SetMaxHealth(10000);
            creature->SetHealth(10000);
            TEST_CAST(warlock, creature, ClassSpells::Warlock::SHADOW_BOLT_RNK_11, SPELL_CAST_OK, TRIGGERED_CAST_DIRECTLY);
            Wait(500);
            uint32 warlockThreat = creature->GetThreatManager().GetThreat(warlock);
            TEST_ASSERT(warlockThreat > 0);
            TEST_ASSERT(creature->GetTarget() == warlock->GetGUID());

            // Stances & creature
            TestRequiresStance(warrior, creature, false, ClassSpells::Warrior::TAUNT_RNK_1);
            TestRequiresStance(warrior, creature, false, ClassSpells::Warrior::TAUNT_RNK_1, ClassSpells::Warrior::BERSERKER_STANCE_RNK_1);
            TestRequiresStance(warrior, creature, false, ClassSpells::Warrior::TAUNT_RNK_1, ClassSpells::Warrior::BATTLE_STANCE_RNK_1);
            TestRequiresStance(warrior, creature, true, ClassSpells::Warrior::TAUNT_RNK_1, ClassSpells::Warrior::DEFENSIVE_STANCE_RNK_1);

            // Acquire threat, aura duration, cooldown
            TEST_CAST(warrior, creature, ClassSpells::Warrior::TAUNT_RNK_1);
            TEST_AURA_MAX_DURATION(creature, ClassSpells::Warrior::TAUNT_RNK_1, Seconds(3));
            TEST_HAS_COOLDOWN(warrior, ClassSpells::Warrior::TAUNT_RNK_1, Seconds(10));
            TEST_ASSERT(creature->GetThreatManager().GetThreat(warrior) == warlockThreat);

            // Keep aggro
            Wait(1000);
            TEST_CAST(warlock, creature, ClassSpells::Warlock::SHADOW_BOLT_RNK_11, SPELL_CAST_OK, TRIGGERED_CAST_DIRECTLY);
            Wait(500);
            warlockThreat = creature->GetThreatManager().GetThreat(warlock);
            TEST_ASSERT(warlockThreat > creature->GetThreatManager().GetThreat(warrior));
            TEST_ASSERT(creature->GetTarget() == warrior->GetGUID());

            // Lose aggro
            Wait(2000);
            TEST_ASSERT(creature->GetTarget() == warlock->GetGUID());
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<TauntTestImpt>();
    }
};

void AddSC_test_spells_warrior()
{
    // Arms: 8/8
    new ChargeTest();
    new HamstringTest();
    new HeroicStrikeTest();
    new MockingBlowTest();
    new OverpowerTest();
    new RendTest();
    new RetaliationTest();
    new ThunderClapTest();
    // Fury: 14/14
    new BattleShoutTest();
    new BerserkerRageTest();
    new ChallengingShoutTest();
    new CleaveTest();
    new CommandingShoutTest();
    new DemoralizingShoutTest();
    new ExecuteTest();
    new InterceptTest();
    new IntidimidatingShoutTest();
    new PummelTest();
    new RecklessnessTest();
    new SlamTest();
    new VictoryRushTest();
    new WhirlwindTest();
    // Protection: 4/11
    new BloodrageTest();
    new DisarmTest();
    new InterveneTest();
    new RevengeTest();
    new ShieldBashTest();
    new ShieldBlockTest();
    new ShieldWallTest();
    new SpellReflectionTest();
    new StanceMasteryTest();
    new SunderArmorTest();
    new TauntTest();
}
