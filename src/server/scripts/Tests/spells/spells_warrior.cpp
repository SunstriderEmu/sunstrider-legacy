#include "../ClassSpellsDamage.h"
#include "../ClassSpellsCoeff.h"
#include "PlayerbotAI.h"

class TestCaseWarrior : public TestCase
{
public:
    TestCaseWarrior(bool needMap = true) : TestCase(true) {}
    TestCaseWarrior(WorldLocation const& loc) : TestCase(loc) {}

    void TestCaseWarrior::TestRequiresStance(TestPlayer* warrior, Unit* victim, bool success, uint32 testSpellId, uint32 stanceSpellId = 0)
    {
        if (stanceSpellId > 0)
        {
            TEST_CAST(warrior, warrior, stanceSpellId);
        }
    
        SpellCastResult res = success ? SPELL_CAST_OK : SPELL_FAILED_ONLY_SHAPESHIFT;

        TEST_CAST(warrior, victim, testSpellId, res, TriggerCastFlags(TRIGGERED_IGNORE_SPELL_AND_CATEGORY_CD | TRIGGERED_IGNORE_POWER_AND_REAGENT_COST));
    }

    void TestCaseWarrior::TestRequiresMeleeWeapon(TestPlayer* warrior, Unit* victim, uint32 testSpellId, bool twoHand)
    {
        RemoveAllEquipedItems(warrior);
        warrior->RemoveAllSpellCooldown();
        TEST_CAST(warrior, victim, testSpellId, SPELL_FAILED_EQUIPPED_ITEM_CLASS);

        if (twoHand)
        {
            EQUIP_ITEM(warrior, 34247); // Apolyon, the Soul-Render
        }
        else
        {
            EQUIP_ITEM(warrior, 32837); // Warglaive of Azzinoth MH
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
        ChargeTestImpt() : TestCaseWarrior(true) { }

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

            warrior->DisableRegeneration(true);
            warrior->SetWalk(false); // run

            // Stances
            TestRequiresStance(warrior, priest2, false, ClassSpells::Warrior::CHARGE_RNK_3);
            TestRequiresStance(warrior, priest2, false, ClassSpells::Warrior::CHARGE_RNK_3, ClassSpells::Warrior::DEFENSIVE_STANCE_RNK_1);
            TestRequiresStance(warrior, priest2, false, ClassSpells::Warrior::CHARGE_RNK_3, ClassSpells::Warrior::BERSERKER_STANCE_RNK_1);
            TestRequiresStance(warrior, priest2, true, ClassSpells::Warrior::CHARGE_RNK_3, ClassSpells::Warrior::BATTLE_STANCE_RNK_1);
            // Melee weapon
            TestRequiresMeleeWeapon(warrior, priest2, ClassSpells::Warrior::CHARGE_RNK_3, true);

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
            TEST_AURA_MAX_DURATION(priest2, ClassSpells::Warrior::CHARGE_RNK_3_STUN, EFFECT_0, 1 * SECOND * IN_MILLISECONDS);

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
        HamstringTestImpt() : TestCaseWarrior(true) { }

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
            TEST_AURA_MAX_DURATION(creature, ClassSpells::Warrior::HAMSTRING_RNK_4, EFFECT_1, 15 * SECOND * IN_MILLISECONDS);

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
        HeroicStrikeTestImpt() : TestCaseWarrior(true) { }

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
        MockingBlowTestImpt() : TestCaseWarrior(true) { }

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
            uint32 const expectedMockingBlowRage = 10 * 10;
            creature->RemoveAurasDueToSpell(ClassSpells::Warrior::MOCKING_BLOW_RNK_6);
            TEST_POWER_COST(warrior, creature, ClassSpells::Warrior::MOCKING_BLOW_RNK_6, POWER_RAGE, expectedMockingBlowRage);

            // Cooldown
            TEST_ASSERT(warrior->GetSpellCooldownDelay(ClassSpells::Warrior::MOCKING_BLOW_RNK_6) == 2 * MINUTE);

            // Aura
            TEST_AURA_MAX_DURATION(creature, ClassSpells::Warrior::MOCKING_BLOW_RNK_6, EFFECT_1, 6 * SECOND * IN_MILLISECONDS);

            // Taunt
            TEST_ASSERT(creature->GetTarget() == warrior->GetGUID());
            Wait(6000);
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
        OverpowerTestImpt() : TestCaseWarrior(true) { }

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
        RendTestImpt() : TestCaseWarrior(true) { }

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
        RetaliationTestImpt() : TestCaseWarrior(true) { }

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
            Aura* aura = warrior->GetAura(ClassSpells::Warrior::RETALIATION_RNK_1, EFFECT_0);
            TEST_ASSERT(aura != nullptr);
            TEST_ASSERT(aura->GetAuraMaxDuration() == 15 * SECOND * IN_MILLISECONDS);
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
                    aura = warrior->GetAura(ClassSpells::Warrior::RETALIATION_RNK_1, EFFECT_0);
                    TEST_ASSERT(aura != nullptr);
                }
                warrior->SetFullHealth();
            }

            uint32 givenMin;
            uint32 givenMax;
            bool foundData = GetDamagePerSpellsTo(warrior, enemy, ClassSpells::Warrior::RETALIATION_RNK_1_PROC, givenMin, givenMax);
            TC_LOG_DEBUG("test.unit_test", "givenMin: %u, givenMax: %u, expectedMin: %u, expectedMax: %u", givenMin, givenMax, expectedMin, expectedMax);
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
        ThunderClapTestImpt() : TestCaseWarrior(true) { }

        void TestThunderClapAura(Unit* victim, float expectedAS, int count)
        {
            if (victim->HasAura(ClassSpells::Warrior::THUNDER_CLAP_RNK_7))
            {
                TEST_AURA_MAX_DURATION(victim, ClassSpells::Warrior::THUNDER_CLAP_RNK_7, EFFECT_1, 30 * SECOND * IN_MILLISECONDS);
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
            uint32 const expectedThunderClapRage = 20 * 10;
            TEST_POWER_COST(warrior, creature1, ClassSpells::Warrior::THUNDER_CLAP_RNK_7, POWER_RAGE, expectedThunderClapRage);

            // Cooldown
            TEST_ASSERT(warrior->GetSpellCooldownDelay(ClassSpells::Warrior::THUNDER_CLAP_RNK_7) == 4 * SECOND);

            // 4 affected max, aura duration, slower attacks
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
        BattleShoutTestImpt() : TestCaseWarrior(true) { }

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
            TEST_AURA_MAX_DURATION(warrior, ClassSpells::Warrior::BATTLE_SHOUT_RNK_8, EFFECT_0, 2 * MINUTE * IN_MILLISECONDS);
            TEST_AURA_MAX_DURATION(hunter, ClassSpells::Warrior::BATTLE_SHOUT_RNK_8, EFFECT_0, 2 * MINUTE * IN_MILLISECONDS);

            // Range
            TEST_ASSERT(ally2->HasAura(ClassSpells::Warrior::BATTLE_SHOUT_RNK_8));

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
        BerserkerRageTestImpt() : TestCaseWarrior(true) { }

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

            // Aura duration
            TEST_AURA_MAX_DURATION(warrior, ClassSpells::Warrior::BERSERKER_RAGE_RNK_1, EFFECT_0, 10 * SECOND * IN_MILLISECONDS);

            // Cooldown duration
            TEST_ASSERT(warrior->GetSpellCooldownDelay(ClassSpells::Warrior::BERSERKER_RAGE_RNK_1) == 30 * SECOND);

            // Reset
            warrior->SetPower(POWER_RAGE, 0);
            TEST_ASSERT(warrior->GetPower(POWER_RAGE) == 0);

            // Extra rage
            auto AI = rogue->GetTestingPlayerbotAI();
            uint32 level = 70;
            float const rageConversion = 0.0091107836f * level * level + 3.225598133f * level + 4.2652911f;
            uint32 expectedRage = 0;
            for (uint32 i = 0; i < 75; i++)
            {
                rogue->AttackerStateUpdate(warrior, BASE_ATTACK);
                auto damageToTarget = AI->GetWhiteDamageDoneInfo(warrior);
                TEST_ASSERT(damageToTarget->size() == i + 1);
                auto& data = damageToTarget->back();
                uint32 damage = data.damageInfo.damage;
                damage += data.damageInfo.resist;
                damage += data.damageInfo.blocked_amount;
                damage += data.damageInfo.absorb;
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
    // Fury:
    new BattleShoutTest();
    new BerserkerRageTest();
}
