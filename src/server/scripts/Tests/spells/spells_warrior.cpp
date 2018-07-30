#include "../ClassSpellsDamage.h"
#include "../ClassSpellsCoeff.h"
#include "PlayerbotAI.h"
#include "SpellHistory.h"

/*
Resources:
    - Omen (addon)
    - Diamond Threat Meter (addon)
    - Cinderhelm: (up to 2.3) https://web.archive.org/web/20080223152408/http://forums.worldofwarcraft.com:80/thread.html?topicId=75381044&sid=1&pageNo=1#2
    - Lavina: (up to 2.3) https://web.archive.org/web/20080331142436/http://forums.wow-europe.com:80/thread.html?topicId=94319875&sid=1
    - Kenco: (up to 2.1) http://web.archive.org/web/20071125211255/http://evilempireguild.org:80/guides/kenco2.php
*/

#define BATTLE_STANCE ClassSpells::Warrior::BATTLE_STANCE_RNK_1
#define DEFENSIVE_STANCE ClassSpells::Warrior::DEFENSIVE_STANCE_RNK_1
#define BERSERKER_STANCE ClassSpells::Warrior::BERSERKER_STANCE_RNK_1

//battle stance + berserker both have 0.8
#define WARRIOR_STANCE_THREAT_MOD 0.8f
#define DEFENSIVE_STANCE_THREAT_MOD 1.3f

#define APOLYON 34247
#define APOLYON_MIN 404
#define APOLYON_MAX 607
#define APOLYON_SPEED 3.4f

#define WG_MH 32837
#define WG_OH 32838

class TestCaseWarrior : public TestCase
{
public:
    TestCaseWarrior() {}
    TestCaseWarrior(WorldLocation const& loc) : TestCase(loc) {}

    uint32 GetStance(TestPlayer* warrior)
    {
        if (warrior->HasAura(ClassSpells::Warrior::BATTLE_STANCE_RNK_1))
            return ClassSpells::Warrior::BATTLE_STANCE_RNK_1;
        else if (warrior->HasAura(ClassSpells::Warrior::DEFENSIVE_STANCE_RNK_1))
            return ClassSpells::Warrior::DEFENSIVE_STANCE_RNK_1;
        else if (warrior->HasAura(ClassSpells::Warrior::BERSERKER_STANCE_RNK_1))
            return ClassSpells::Warrior::BERSERKER_STANCE_RNK_1;
        else
            return 0;
    }

    void TeleportToSpawn(TestPlayer* voyager)
    {
        voyager->TeleportTo(_location);
        auto ai = _GetCasterAI(voyager);
        ai->HandleTeleportAck(); //immediately handle teleport packet
    }

    void SwitchTo(TestPlayer* warrior, uint32 stanceSpellId)
    {
        FORCE_CAST(warrior, warrior, stanceSpellId, SPELL_MISS_NONE, TRIGGERED_FULL_MASK);
    }

    void TestStancesAllowed(TestPlayer* warrior, Unit* victim, uint32 spellId, bool battleStance, bool defensiveStance, bool berserkerStance, Optional<TestCallback>(callback) = {})
    {
        uint32 const startStance = GetStance(warrior);
        uint32 const stancesCount = 3;
        uint32 stances[stancesCount] = { BATTLE_STANCE, DEFENSIVE_STANCE, BERSERKER_STANCE };
        bool allowedStances[stancesCount] = { battleStance, defensiveStance, berserkerStance };
        for (uint32 i = 0; i < stancesCount; i++)
        {
            if (callback)
                callback.get()(warrior, victim);

            TestStance(warrior, victim, spellId, allowedStances[i], stances[i]);
            TeleportToSpawn(warrior);
            warrior->RemoveAurasDueToSpell(spellId);
            victim->RemoveAurasDueToSpell(spellId);
            victim->ClearDiminishings();
            victim->SetFullHealth();
            victim->SetFullPower(victim->GetPowerType());
        }
        SwitchTo(warrior, startStance);
        WaitNextUpdate();
    }

    void TestStance(TestPlayer* warrior, Unit* victim, uint32 spellId, bool stanceAllowed, uint32 stanceSpellId)
    {
        SpellCastResult res = stanceAllowed ? SPELL_CAST_OK : SPELL_FAILED_ONLY_SHAPESHIFT;
        TriggerCastFlags flags = TriggerCastFlags(TRIGGERED_IGNORE_SPELL_AND_CATEGORY_CD | TRIGGERED_IGNORE_POWER_AND_REAGENT_COST | TRIGGERED_IGNORE_CASTER_AURASTATE | TRIGGERED_TREAT_AS_NON_TRIGGERED);
        FORCE_CAST(warrior, warrior, stanceSpellId, SPELL_MISS_NONE, TRIGGERED_FULL_MASK);
        ASSERT_INFO("In stance %s", _SpellString(stanceSpellId).c_str());
        TEST_CAST(warrior, victim, spellId, res, flags);
        warrior->GetSpellHistory()->ResetAllCooldowns();
    }

    void TestRequiresMeleeWeapon(TestPlayer* warrior, Unit* victim, uint32 testSpellId, SpellCastResult result = SPELL_FAILED_EQUIPPED_ITEM_CLASS)
    {
        RemoveAllEquipedItems(warrior);
        warrior->GetSpellHistory()->ResetAllCooldowns();
        TEST_CAST(warrior, victim, testSpellId, result);
    }
};

class ChargeTest : public TestCaseScript
{
public:
    ChargeTest() : TestCaseScript("spells warrior charge") { }

    class ChargeTestImpt : public TestCaseWarrior
    {
    public:


        void Test() override
        {
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN);

            Position spawn(_location);
            spawn.MoveInFront(_location, 7.0f);
            TestPlayer* priest7m = SpawnPlayer(CLASS_PRIEST, RACE_HUMAN, 70, spawn);

            spawn.MoveInFront(_location, 15.0f);
            TestPlayer* priest15m = SpawnPlayer(CLASS_PRIEST, RACE_HUMAN, 70, spawn);

            spawn.MoveInFront(_location, 30.0f);
            TestPlayer* priest30m = SpawnPlayer(CLASS_PRIEST, RACE_HUMAN, 70, spawn);

            warrior->SetWalk(false); // run

            // Stances
            TestStancesAllowed(warrior, priest15m, ClassSpells::Warrior::CHARGE_RNK_3, true, false, false);

            // Range
            TEST_CAST(warrior, priest7m, ClassSpells::Warrior::CHARGE_RNK_3, SPELL_FAILED_OUT_OF_RANGE);
            TEST_CAST(warrior, priest30m, ClassSpells::Warrior::CHARGE_RNK_3, SPELL_FAILED_OUT_OF_RANGE);

            // Cast
            TEST_CAST(warrior, priest15m, ClassSpells::Warrior::CHARGE_RNK_3);
            // Rage earned
            uint32 expectedRage = 15 * 10;
            TEST_ASSERT(warrior->GetPower(POWER_RAGE) == expectedRage);
            // Stun
            Wait(250);
            TEST_AURA_MAX_DURATION(priest15m, ClassSpells::Warrior::CHARGE_RNK_3_STUN, Seconds(1));

            // Warrior still running after charge
            TEST_ASSERT(!warrior->IsWalking());

            // Cannot be used in combat
            warrior->GetSpellHistory()->ResetAllCooldowns();
            TEST_CAST(warrior, priest30m, ClassSpells::Warrior::CHARGE_RNK_3, SPELL_FAILED_AFFECTING_COMBAT);
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<ChargeTestImpt>();
    }
};

class HamstringTest : public TestCaseScript
{
public:
    HamstringTest() : TestCaseScript("spells warrior hamstring") { }

    class HamstringTestImpt : public TestCaseWarrior
    {
    public:
        /*
        Threat:
            - Omen: 180
            - DTM: 181
            - Lavina: 181
            - Ciderhelm: 181
        Decided: 181
        */

        void Test() override
        {
            TestPlayer* warrior = SpawnRandomPlayer(CLASS_WARRIOR);
            Creature* dummy = SpawnCreature(12, true); // No immunes dummy

            TestRequiresMeleeWeapon(warrior, dummy, ClassSpells::Warrior::HAMSTRING_RNK_4);
            EQUIP_NEW_ITEM(warrior, APOLYON);
            TestStancesAllowed(warrior, dummy, ClassSpells::Warrior::HAMSTRING_RNK_4, true, false, true);
            dummy->DespawnOrUnsummon();

            Creature* creature = SpawnCreature(12, true); // No immunes dummy
            float const harmstringSpeedFactor = 0.5f;
            float const expectedSpeed = creature->GetSpeed(MOVE_RUN) * harmstringSpeedFactor;

            // Rage cost
            uint32 const expectedHamstringRage = 10 * 10;
            TEST_POWER_COST(warrior, ClassSpells::Warrior::HAMSTRING_RNK_4, POWER_RAGE, expectedHamstringRage);

            warrior->SetFullPower(POWER_RAGE);
            FORCE_CAST(warrior, creature, ClassSpells::Warrior::HAMSTRING_RNK_4);

            // Duration & CD
            TEST_AURA_MAX_DURATION(creature, ClassSpells::Warrior::HAMSTRING_RNK_4, Seconds(15));

            // Speed
            TEST_ASSERT(creature->GetSpeed(MOVE_RUN) == expectedSpeed);

            // Threat
            auto[minDmg, maxDmg] = GetDamagePerSpellsTo(warrior, creature, ClassSpells::Warrior::HAMSTRING_RNK_4, {}, 1);
            float const expectedThreat = (181 + minDmg) * WARRIOR_STANCE_THREAT_MOD;
            ASSERT_INFO("Warrior has %f on Creature but %f was expected.", creature->GetThreatManager().GetThreat(warrior), expectedThreat);
            TEST_ASSERT(Between<float>(creature->GetThreatManager().GetThreat(warrior), expectedThreat - 0.1f, expectedThreat + 0.1f));

            // Damage
            float const armorFactor = GetArmorFactor(warrior, creature);
            uint32 const hamstringDmg = ClassSpellsDamage::Warrior::HAMSTRING_RNK_4 * armorFactor;
            TEST_DIRECT_SPELL_DAMAGE(warrior, creature, ClassSpells::Warrior::HAMSTRING_RNK_4, hamstringDmg, hamstringDmg, false);
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<HamstringTestImpt>();
    }
};

class HeroicStrikeTest : public TestCaseScript
{
public:
    HeroicStrikeTest() : TestCaseScript("spells warrior heroic_strike") { }

    class HeroicStrikeTestImpt : public TestCaseWarrior
    {
        /*
        Bugs:
            - Should deal +61.6 dmg when victim is Dazed
        Threat:
            - Omen: 196
            - DTM: 195
            - Lavina: 196
            - Ciderhelm: 196
        Decided: 196
        */
        void Test() override
        {
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN);
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_HUMAN);

            TestRequiresMeleeWeapon(warrior, warlock, ClassSpells::Warrior::HEROIC_STRIKE_RNK_10);
            EQUIP_NEW_ITEM(warrior, APOLYON);

            // Rage cost
            uint32 const expectedHeroicStrikeRage = 15 * 10;
            TEST_POWER_COST(warrior, ClassSpells::Warrior::HEROIC_STRIKE_RNK_10, POWER_RAGE, expectedHeroicStrikeRage);

            SECTION("Damage", [&] {
                auto[minHeroicStrike, maxHeroicStrike] = CalcMeleeDamage(warrior, warlock, BASE_ATTACK, ClassSpellsDamage::Warrior::HEROIC_STRIKE_RNK_10);
                TEST_DIRECT_SPELL_DAMAGE(warrior, warlock, ClassSpells::Warrior::HEROIC_STRIKE_RNK_10, minHeroicStrike, maxHeroicStrike, false);
            });

            SECTION("Threat", [&] {
                Creature* creature = SpawnCreature();
                FORCE_CAST(warrior, creature, ClassSpells::Warrior::HEROIC_STRIKE_RNK_10, SPELL_MISS_NONE, TRIGGERED_IGNORE_POWER_AND_REAGENT_COST);
                warrior->AttackerStateUpdate(creature, BASE_ATTACK);
                auto[minDmg, maxDmg] = GetDamagePerSpellsTo(warrior, creature, ClassSpells::Warrior::HEROIC_STRIKE_RNK_10, {}, 1);
                float const expectedThreat = (196 + minDmg) * WARRIOR_STANCE_THREAT_MOD;
                ASSERT_INFO("Warrior has %f on Creature but %f was expected.", creature->GetThreatManager().GetThreat(warrior), expectedThreat);
                TEST_ASSERT(Between<float>(creature->GetThreatManager().GetThreat(warrior), expectedThreat - 0.1f, expectedThreat + 0.1f));
            });

            SECTION("Dazed Damage", STATUS_KNOWN_BUG, [&] {
                float const hsDazedBonus = 61.6f;
                auto[minHeroicStrikeDazed, maxHeroicStrikeDazed] = CalcMeleeDamage(warrior, warlock, BASE_ATTACK, ClassSpellsDamage::Warrior::HEROIC_STRIKE_RNK_10 + hsDazedBonus);
                TEST_DIRECT_SPELL_DAMAGE(warrior, warlock, ClassSpells::Warrior::HEROIC_STRIKE_RNK_10, minHeroicStrikeDazed, maxHeroicStrikeDazed, false, [](Unit* caster, Unit* victim) {
                    victim->AddAura(1604, victim); // Dazed

                });
            });
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<HeroicStrikeTestImpt>();
    }
};

class MockingBlowTest : public TestCaseScript
{
public:
    MockingBlowTest() : TestCaseScript("spells warrior mocking_blow") { }

    class MockingBlowTestImpt : public TestCaseWarrior
    {
    public:
        /*
        Threat:
            - Omen: 290
            - DTM: 290
            - Lavina: 290
            - Ciderhelm: 290
        Decided: 290
        */


        void Test() override
        {
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN);
            Creature* creature = SpawnCreature(20777, true); // Talbuk Sire

            TestRequiresMeleeWeapon(warrior, creature, ClassSpells::Warrior::MOCKING_BLOW_RNK_6);
            EQUIP_NEW_ITEM(warrior, APOLYON);
            TestStancesAllowed(warrior, creature, ClassSpells::Warrior::MOCKING_BLOW_RNK_6, true, false, false);
            creature->DespawnOrUnsummon();

            // Setup
            TestPlayer* ally = SpawnPlayer(CLASS_WARRIOR, RACE_UNDEAD_PLAYER);
            Creature* creature2 = SpawnCreature(20777, true); // Talbuk Sire
            ally->AttackerStateUpdate(creature2, BASE_ATTACK);
            creature2->AI()->UpdateVictim();
            TEST_ASSERT(creature2->GetTarget() == ally->GetGUID());

            // Rage cost
            uint32 const expectedMockingBlowRage = 10 * 10;
            TEST_POWER_COST(warrior, ClassSpells::Warrior::MOCKING_BLOW_RNK_6, POWER_RAGE, expectedMockingBlowRage);

            // Aura + Cooldown
            FORCE_CAST(warrior, creature2, ClassSpells::Warrior::MOCKING_BLOW_RNK_6, SPELL_MISS_NONE, TRIGGERED_IGNORE_POWER_AND_REAGENT_COST);
            TEST_AURA_MAX_DURATION(creature2, ClassSpells::Warrior::MOCKING_BLOW_RNK_6, Seconds(6));
            TEST_HAS_COOLDOWN(warrior, ClassSpells::Warrior::MOCKING_BLOW_RNK_6, Minutes(2));

            // Threat
            uint32 const mockingBlowThreat = 290;
            auto[minDmg, maxDmg] = GetDamagePerSpellsTo(warrior, creature2, ClassSpells::Warrior::MOCKING_BLOW_RNK_6, {}, 1);
            float const expectedThreat = (mockingBlowThreat + minDmg) * WARRIOR_STANCE_THREAT_MOD;
            ASSERT_INFO("Warrior has %f on Creature2 but %f was expected.", creature2->GetThreatManager().GetThreat(warrior), expectedThreat);
            TEST_ASSERT(Between<float>(creature2->GetThreatManager().GetThreat(warrior), expectedThreat - 0.1f, expectedThreat + 0.1f));

            // Taunt
            creature2->AI()->UpdateVictim();
            TEST_ASSERT(creature2->GetTarget() == warrior->GetGUID());
            for (uint32 i = 0; i < 30; i++)
                ally->AttackerStateUpdate(creature2, BASE_ATTACK);
            Wait(6500); //let taunt fade
            TEST_ASSERT(creature2->GetTarget() == ally->GetGUID());

            // Damage
            Creature* dummy = SpawnCreature();
            float const armorFactor = GetArmorFactor(warrior, dummy);
            const uint32 expectedMBDmg = ClassSpellsDamage::Warrior::MOCKING_BLOW_RNK_6 * armorFactor;
            TEST_DIRECT_SPELL_DAMAGE(warrior, dummy, ClassSpells::Warrior::MOCKING_BLOW_RNK_6, expectedMBDmg, expectedMBDmg, false);
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<MockingBlowTestImpt>();
    }
};

class OverpowerTest : public TestCaseScript
{
public:
    OverpowerTest() : TestCaseScript("spells warrior overpower") { }

    class OverpowerTestImpt : public TestCaseWarrior
    {
        void Test() override
        {
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN);
            Creature* creature = SpawnCreature();

            EQUIP_NEW_ITEM(warrior, APOLYON);

            // Rage cost
            uint32 const expectedOverpowerRage = 5 * 10;
            TEST_POWER_COST(warrior, ClassSpells::Warrior::OVERPOWER_RNK_4, POWER_RAGE, expectedOverpowerRage);

            SECTION("WIP", STATUS_WIP, [&] {
                // TODO: Overpower only usable after target dodged.
                // TODO: Overpower cannot be blocked, dodged or parried.
            }); 

            // Damage
            float const normalizedSpeed = 3.3f;
            auto[minOverpower, maxOverpower] = CalcMeleeDamage(warrior, creature, BASE_ATTACK, ClassSpellsDamage::Warrior::OVERPOWER_RNK_4, normalizedSpeed);
            TEST_DIRECT_SPELL_DAMAGE(warrior, creature, ClassSpells::Warrior::OVERPOWER_RNK_4, minOverpower, maxOverpower, false);
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<OverpowerTestImpt>();
    }
};

class RendTest : public TestCaseScript
{
public:
    RendTest() : TestCaseScript("spells warrior rend") { }

    class RendTestImpt : public TestCaseWarrior
    {
    public:


        void Test() override
        {
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN);
            Creature* creature = SpawnCreature();

            warrior->DisableRegeneration(true);

            TestStancesAllowed(warrior, creature, ClassSpells::Warrior::REND_RNK_8, true, true, false);
            TestRequiresMeleeWeapon(warrior, creature, ClassSpells::Warrior::REND_RNK_8);
            EQUIP_NEW_ITEM(warrior, APOLYON);

            // Rage cost
            uint32 const expectedRendRage = 10 * 10;
            TEST_POWER_COST(warrior, ClassSpells::Warrior::REND_RNK_8, POWER_RAGE, expectedRendRage);

            // Damage -- http://wowwiki.wikia.com/wiki/Rend?oldid=1513481
            uint32 const rendTickAmount = 7;
            float const AP = warrior->GetTotalAttackPowerValue(BASE_ATTACK);
            float const armorFactor = GetArmorFactor(warrior, creature);
            float const rendTime = 21.0f;
            float const apolyonAvg = (APOLYON_MIN + APOLYON_MAX) / 2.f;
            float const magicalFactor = 743.f / 300000.f;
            uint32 const rendBonus = (apolyonAvg + floor(APOLYON_SPEED * AP / 14)) * rendTime * magicalFactor / rendTickAmount;
            uint32 const rendTick = ClassSpellsDamage::Warrior::REND_RNK_8_TICK + rendBonus;
            uint32 const rendTotal = rendTickAmount * rendTick;
            TEST_DOT_DAMAGE(warrior, creature, ClassSpells::Warrior::REND_RNK_8, rendTotal, false);
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<RendTestImpt>();
    }
};

class RetaliationTest : public TestCaseScript
{
public:
    RetaliationTest() : TestCaseScript("spells warrior retaliation") { }

    class RetaliationTestImpt : public TestCaseWarrior
    {
    public:


        void Test() override
        {
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN);

            Position spawn(_location);
            spawn.MoveInFront(_location, 3.0f);
            TestPlayer* enemy = SpawnPlayer(CLASS_WARRIOR, RACE_HUMAN, 70, spawn);

            spawn.MoveInFront(_location, -3.0f);
            TestPlayer* behind = SpawnPlayer(CLASS_WARRIOR, RACE_HUMAN, 70, spawn);

            TestStancesAllowed(warrior, warrior, ClassSpells::Warrior::RETALIATION_RNK_1, true, false, false);
            EQUIP_NEW_ITEM(warrior, APOLYON);

            // Aura
            TEST_CAST(warrior, warrior, ClassSpells::Warrior::RETALIATION_RNK_1);
            TEST_AURA_MAX_DURATION(warrior, ClassSpells::Warrior::RETALIATION_RNK_1, Seconds(15));
            TEST_HAS_COOLDOWN(warrior, ClassSpells::Warrior::RETALIATION_RNK_1, Minutes(30));

            Aura* aura = warrior->GetAura(ClassSpells::Warrior::RETALIATION_RNK_1);
            TEST_ASSERT(aura != nullptr);

            uint32 const maxCharges = 30;

            // Cannot counterattack from behind
            for (uint32 i = 0; i < 500; i++)
            {
                behind->AttackerStateUpdate(warrior, BASE_ATTACK);
                warrior->SetFullHealth();
            }
            TEST_ASSERT(aura->GetCharges() == maxCharges);

            // Damage
            auto[minMelee, maxMelee] = CalcMeleeDamage(warrior, enemy, BASE_ATTACK);

            // Counterattack from front and lose charge on hit
            enemy->ForceMeleeHitResult(MELEE_HIT_NORMAL);
            for (uint32 i = 1; i <= maxCharges; i++)
            {
                warrior->SetFullHealth();
                enemy->SetFullHealth();
                enemy->AttackerStateUpdate(warrior, BASE_ATTACK);
                if (i == maxCharges)
                {
                    TEST_HAS_NOT_AURA(warrior, ClassSpells::Warrior::RETALIATION_RNK_1);
                }
                else
                {
                    ASSERT_INFO("Warrior has %u stacks but %u expected.", aura->GetCharges(), maxCharges - i);
                    TEST_ASSERT(aura->GetCharges() == int32(maxCharges - i));
                }
            }

            TEST_DIRECT_SPELL_DAMAGE(warrior, enemy, ClassSpells::Warrior::RETALIATION_RNK_1_PROC, minMelee, maxMelee, false);
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<RetaliationTestImpt>();
    }
};

class ThunderClapTest : public TestCaseScript
{
public:
    ThunderClapTest() : TestCaseScript("spells warrior thunder_clap") { }

    class ThunderClapTestImpt : public TestCaseWarrior
    {
    public:
        /*
        Threat:
            - Omen: 175%
            - DTM: 175%
            - Lavina: 175%
            - Ciderhelm: 175%
        Decided: 175%
        */


        void Test() override
        {
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN);
            Creature* creature = SpawnCreature();

            TestStancesAllowed(warrior, warrior, ClassSpells::Warrior::THUNDER_CLAP_RNK_7, true, true, false);

            // Rage cost
            uint32 const expectedThunderClapRage = 20 * 10;
            TEST_POWER_COST(warrior, ClassSpells::Warrior::THUNDER_CLAP_RNK_7, POWER_RAGE, expectedThunderClapRage);

            // Slower Attack Speed
            float const thunderClapASFactor = 0.9f;
            float const expectedAS = creature->GetAttackTimer(BASE_ATTACK) * thunderClapASFactor;
            FORCE_CAST(warrior, creature, ClassSpells::Warrior::THUNDER_CLAP_RNK_7, SPELL_MISS_NONE, TRIGGERED_IGNORE_POWER_AND_REAGENT_COST);
            TEST_HAS_COOLDOWN(warrior, ClassSpells::Warrior::THUNDER_CLAP_RNK_7, Seconds(4));
            TEST_AURA_MAX_DURATION(creature, ClassSpells::Warrior::THUNDER_CLAP_RNK_7, Seconds(30));
            TEST_ASSERT(creature->GetAttackTimer(BASE_ATTACK) == expectedAS);

            // Damage
            float const armorFactor = GetArmorFactor(warrior, creature);
            uint32 const thunderClap = ClassSpellsDamage::Warrior::THUNDER_CLAP_RNK_7 * armorFactor;
            TEST_DIRECT_SPELL_DAMAGE(warrior, creature, ClassSpells::Warrior::THUNDER_CLAP_RNK_7, thunderClap, thunderClap, false);
            creature->DespawnOrUnsummon();

            // 4 affected max, slower attacks, spawn 4 more
            TestPlayer* enemy1 = SpawnPlayer(CLASS_WARRIOR, RACE_HUMAN);
            TestPlayer* enemy2 = SpawnPlayer(CLASS_WARRIOR, RACE_HUMAN);
            TestPlayer* enemy3 = SpawnPlayer(CLASS_WARRIOR, RACE_HUMAN);
            Creature* creature1 = SpawnCreature();
            Creature* creature2 = SpawnCreature();

            FORCE_CAST(warrior, creature1, ClassSpells::Warrior::THUNDER_CLAP_RNK_7, SPELL_MISS_NONE, TRIGGERED_FULL_MASK);
            uint32 count = 0;
            count += uint32(enemy1->HasAura(ClassSpells::Warrior::THUNDER_CLAP_RNK_7));
            count += uint32(enemy2->HasAura(ClassSpells::Warrior::THUNDER_CLAP_RNK_7));
            count += uint32(enemy3->HasAura(ClassSpells::Warrior::THUNDER_CLAP_RNK_7));
            count += uint32(creature1->HasAura(ClassSpells::Warrior::THUNDER_CLAP_RNK_7));
            count += uint32(creature2->HasAura(ClassSpells::Warrior::THUNDER_CLAP_RNK_7));
            TEST_ASSERT(count == 4);

            // Threat -- does 175% of damage
            float const expectedThreat = thunderClap * 1.75f * WARRIOR_STANCE_THREAT_MOD;
            Creature* hasThunderClap = creature1->HasAura(ClassSpells::Warrior::THUNDER_CLAP_RNK_7) ? creature1 : creature2;
            ASSERT_INFO("Warrior has %f threat with a creature but %f was expected.", hasThunderClap->GetThreatManager().GetThreat(warrior), expectedThreat);
            TEST_ASSERT(Between<float>(hasThunderClap->GetThreatManager().GetThreat(warrior), expectedThreat - 0.1f, expectedThreat + 0.1f));
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<ThunderClapTestImpt>();
    }
};

class BattleShoutTest : public TestCaseScript
{
public:
    BattleShoutTest() : TestCaseScript("spells warrior battle_shout") { }

    class BattleShoutTestImpt : public TestCaseWarrior
    {
    public:
        /*
        Threat:
            - Omen: 69
            - DTM: /
            - Lavina: 69
            - Ciderhelm: 69
        Decided: 69
        */


        void Test() override
        {
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN);
            Creature* creature = SpawnCreature();

            Position spawn(_location);
            spawn.MoveInFront(spawn, 10.0f);
            TestPlayer* hunter = SpawnPlayer(CLASS_HUNTER, RACE_TAUREN, 70, spawn);
            TestPlayer* hunter2 = SpawnPlayer(CLASS_HUNTER, RACE_TAUREN, 70, spawn);
            spawn.MoveInFront(spawn, 30.0f);
            TestPlayer* ally2 = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN, 70, spawn);

            // Battle Shout AP Boost
            int32 const spellLevel = 69;
            int32 const pointPerLevel = 1;
            uint32 const pointPerLevelGain = std::max(int32(warrior->GetLevel()) - spellLevel, int32(0)) * pointPerLevel;
            uint32 const battleShoutBonus = 305 + pointPerLevelGain;

            // Setup
            GroupPlayer(warrior, hunter);
            GroupPlayer(warrior, hunter2);
            GroupPlayer(warrior, ally2);
            //SetInCombatWith is not sufficient here in current threat implementation
            creature->GetThreatManager().AddThreat(warrior, 0.0f);
            creature->GetThreatManager().AddThreat(hunter, 0.0f);

            /* Threat should act as follow:
            The flat 69 threat is divided between each target => 23
            For each player this 23 threat gets forwarded to creatures in combat with the player. 
            In this case, 23 get forwarded to creature two times, one for warrior and one for hunter.
            So the final calculation is the following:
            */
            float const expectedThreat = 69 / 3.0f * 2 * WARRIOR_STANCE_THREAT_MOD;
            float const warriorExpectedAP = warrior->GetTotalAttackPowerValue(BASE_ATTACK) + battleShoutBonus;
            float const hunterExpectedAP = hunter->GetTotalAttackPowerValue(RANGED_ATTACK); // the shout only increases "melee attack power"

            // Rage cost
            uint32 const expectedBattleShoutRage = 10 * 10;
            TEST_POWER_COST(warrior, ClassSpells::Warrior::BATTLE_SHOUT_RNK_8, POWER_RAGE, expectedBattleShoutRage);

            TEST_CAST(warrior, warrior, ClassSpells::Warrior::BATTLE_SHOUT_RNK_8, SPELL_CAST_OK, TRIGGERED_FULL_MASK);

            // Aura duration
            TEST_AURA_MAX_DURATION(warrior, ClassSpells::Warrior::BATTLE_SHOUT_RNK_8, Minutes(2));
            TEST_AURA_MAX_DURATION(hunter, ClassSpells::Warrior::BATTLE_SHOUT_RNK_8, Minutes(2));
            TEST_AURA_MAX_DURATION(hunter2, ClassSpells::Warrior::BATTLE_SHOUT_RNK_8, Minutes(2));

            // Range
            TEST_HAS_NOT_AURA(ally2, ClassSpells::Warrior::BATTLE_SHOUT_RNK_8);

            // AP
            ASSERT_INFO("Warrior has %f AP but %f was expected.", warrior->GetTotalAttackPowerValue(BASE_ATTACK), warriorExpectedAP);
            TEST_ASSERT(Between<float>(warrior->GetTotalAttackPowerValue(BASE_ATTACK), warriorExpectedAP - 0.1f, warriorExpectedAP + 0.1f));
            ASSERT_INFO("Hunter has %f AP but %f was expected.", hunter->GetTotalAttackPowerValue(BASE_ATTACK), hunterExpectedAP);
            TEST_ASSERT(Between<float>(hunter->GetTotalAttackPowerValue(RANGED_ATTACK), hunterExpectedAP - 0.1f, hunterExpectedAP + 0.1f));
            ASSERT_INFO("Hunter2 has %f AP but %f was expected.", hunter->GetTotalAttackPowerValue(BASE_ATTACK), hunterExpectedAP);
            TEST_ASSERT(Between<float>(hunter2->GetTotalAttackPowerValue(RANGED_ATTACK), hunterExpectedAP - 0.1f, hunterExpectedAP + 0.1f));

            // Threat
            ASSERT_INFO("Warrior has %f threat with Creature but %f was expected.", creature->GetThreatManager().GetThreat(warrior), expectedThreat);
            TEST_ASSERT(Between<float>(creature->GetThreatManager().GetThreat(warrior), expectedThreat - 0.1f, expectedThreat + 0.1f));
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<BattleShoutTestImpt>();
    }
};

class BerserkerRageTest : public TestCaseScript
{
public:
    BerserkerRageTest() : TestCaseScript("spells warrior berserker_rage") { }

    /*
    "The warrior enters a berserker rage, becoming immune to Fear, Sap and Incapacitate effects and generating extra rage when taking damage."
    Bugs:
    - Should double the generated rage by being hit
    Infos:
    - Berserker Rage generates doubles the rages generated by being hit
    Source: https://youtu.be/MLktLWxholI?t=2m20s
    */
    class BerserkerRageTestImpt : public TestCaseWarrior
    {
    public:


        void TestImmunity(TestPlayer* warrior, TestPlayer* caster, uint32 spellId)
        {
            // Cast CC
            FORCE_CAST(caster, warrior, spellId, SPELL_MISS_NONE, TRIGGERED_FULL_MASK);
            TEST_HAS_AURA(warrior, spellId);
            // Break CC
            ASSERT_INFO("Warrior couldnt cast Berserker Rage under the effect of %u", spellId);
            TEST_CAST(warrior, warrior, ClassSpells::Warrior::BERSERKER_RAGE_RNK_1, SPELL_CAST_OK, TriggerCastFlags(TRIGGERED_IGNORE_SPELL_AND_CATEGORY_CD | TRIGGERED_IGNORE_GCD));
            TEST_HAS_NOT_AURA(warrior, spellId);
            // Immune CC
            TEST_CAST(caster, warrior, spellId, SPELL_CAST_OK, TRIGGERED_FULL_MASK);
            TEST_HAS_NOT_AURA(warrior, spellId);
            warrior->RemoveAurasDueToSpell(ClassSpells::Warrior::BERSERKER_RAGE_RNK_1);
        }

        void Test() override
        {
            TestPlayer* warrior;

            BEFORE_EACH([&] {
                warrior = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN);
                SwitchTo(warrior, ClassSpells::Warrior::BERSERKER_STANCE_RNK_1);
            });

            SECTION("Stances", [&] {
                TestStancesAllowed(warrior, warrior, ClassSpells::Warrior::BERSERKER_RAGE_RNK_1, false, false, true);
            });

            SECTION("Immunities", [&] {
                TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_HUMAN);
                TestPlayer* rogue = SpawnPlayer(CLASS_ROGUE, RACE_HUMAN);
                TestImmunity(warrior, rogue, ClassSpells::Rogue::SAP_RNK_3);
                TestImmunity(warrior, rogue, ClassSpells::Rogue::GOUGE_RNK_6);
                TestImmunity(warrior, warlock, ClassSpells::Warlock::FEAR_RNK_3);
            });

            SECTION("Cooldown & duration", [&] {
                TEST_CAST(warrior, warrior, ClassSpells::Warrior::BERSERKER_RAGE_RNK_1);
                TEST_AURA_MAX_DURATION(warrior, ClassSpells::Warrior::BERSERKER_RAGE_RNK_1, Seconds(10));
                TEST_HAS_COOLDOWN(warrior, ClassSpells::Warrior::BERSERKER_RAGE_RNK_1, Seconds(30));
            });

            // Berserker Rage generates doubles the rages generated by being hit
            SECTION("Extra rage", STATUS_KNOWN_BUG, [&] {
                TestPlayer* rogue = SpawnPlayer(CLASS_ROGUE, RACE_HUMAN);

                float const berserkerRageFactor = 2.f;
                auto AI = rogue->GetTestingPlayerbotAI();
                uint32 const level = 70;
                float const rageConversion = 0.0091107836f * level * level + 3.225598133f * level + 4.2652911f; //formula from?

                for (uint32 i = 0; i < 75; i++)
                {
                    AI->ResetSpellCounters();
                    warrior->SetPower(POWER_RAGE, 0);
                    rogue->AttackerStateUpdate(warrior, BASE_ATTACK);
                    auto damageToTarget = AI->GetMeleeDamageDoneInfo(warrior);
                    TEST_ASSERT(damageToTarget != nullptr);
                    auto& data = damageToTarget->front();

                    uint32 damage = 0;
                    for (uint8 j = 0; j < MAX_ITEM_PROTO_DAMAGES; j++)
                    {
                        damage += data.damageInfo.Damages[j].Damage;
                        damage += data.damageInfo.Damages[j].Resist;
                        damage += data.damageInfo.Damages[j].Absorb;
                    }
                    damage += data.damageInfo.Blocked;

                    uint32 const rage = 2.5f * damage / rageConversion * 2.0f * 10.0f;
                    uint32 const expectedRage = rage * berserkerRageFactor;
                    uint32 const warRage = warrior->GetPower(POWER_RAGE);
                    ASSERT_INFO("Count: %u, Health: %u, Damage: %u, Rage: %u, Expected: %u", i, warrior->GetHealth(), damage, warRage, expectedRage);
                    TEST_ASSERT(warRage == expectedRage);
                }
            });
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<BerserkerRageTestImpt>();
    }
};

class ChallengingShoutTest : public TestCaseScript
{
public:

    ChallengingShoutTest() : TestCaseScript("spells warrior challenging_shout") { }

    class ChallengingShoutTestImpt : public TestCase
    {
    public:


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
            player3m->ForceMeleeHitResult(MELEE_HIT_NORMAL);
            player3m->AttackerStateUpdate(creature3m, BASE_ATTACK);
            player6m->ForceMeleeHitResult(MELEE_HIT_NORMAL);
            player6m->AttackerStateUpdate(creature6m, BASE_ATTACK);
            player11m->ForceMeleeHitResult(MELEE_HIT_NORMAL);
            player11m->AttackerStateUpdate(creature11m, BASE_ATTACK);

            FORCE_CAST(warrior, warrior, ClassSpells::Warrior::CHALLENGING_SHOUT_RNK_1);
            TEST_HAS_COOLDOWN(warrior, ClassSpells::Warrior::CHALLENGING_SHOUT_RNK_1, Minutes(10));

            // Aura, affected targets
            TEST_AURA_MAX_DURATION(creature3m, ClassSpells::Warrior::CHALLENGING_SHOUT_RNK_1, Seconds(6));
            TEST_AURA_MAX_DURATION(creature6m, ClassSpells::Warrior::CHALLENGING_SHOUT_RNK_1, Seconds(6));
            TEST_HAS_NOT_AURA(creature11m, ClassSpells::Warrior::CHALLENGING_SHOUT_RNK_1);

            // Target changed
            creature3m->AI()->UpdateVictim();
            TEST_ASSERT(creature3m->GetTarget() == warrior->GetGUID());
            creature6m->AI()->UpdateVictim();
            TEST_ASSERT(creature6m->GetTarget() == warrior->GetGUID());

            // Back to original target
            Wait(6500);
            TEST_ASSERT(creature3m->GetTarget() == player3m->GetGUID());
            TEST_ASSERT(creature6m->GetTarget() == player6m->GetGUID());

            // Rage cost
            uint32 const expectedChallengingShoutRage = 5 * 10;
            TEST_POWER_COST(warrior, ClassSpells::Warrior::CHALLENGING_SHOUT_RNK_1, POWER_RAGE, expectedChallengingShoutRage);
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<ChallengingShoutTestImpt>();
    }
};

class CleaveTest : public TestCaseScript
{
public:
    CleaveTest() : TestCaseScript("spells warrior cleave") { }

    class CleaveTestImpt : public TestCaseWarrior
    {
    public:
        /*
        Threat:
            - Omen: 130
            - DTM: 130
            - Lavina: 130
            - Ciderhelm: 130
        Decided: 130
        */


        void Test() override
        {
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN);
            Creature* creature = SpawnCreature();

            TestRequiresMeleeWeapon(warrior, creature, ClassSpells::Warrior::CLEAVE_RNK_6);
            EQUIP_NEW_ITEM(warrior, APOLYON);
            creature->DespawnOrUnsummon();

            Creature* creature1 = SpawnCreature();
            Creature* creature2 = SpawnCreature();
            Position spawn;
            spawn.MoveInFront(_location, 3.5f);
            Creature* creature3 = SpawnCreatureWithPosition(spawn);

            // Setup
            uint32 startHealth = creature1->GetHealth();
            TEST_ASSERT(creature2->GetHealth() == startHealth);
            TEST_ASSERT(creature3->GetHealth() == startHealth);

            // Rage cost
            uint32 const expectedCleaveRage = 20 * 10;
            TEST_POWER_COST(warrior, ClassSpells::Warrior::CLEAVE_RNK_6, POWER_RAGE, expectedCleaveRage);

            // Cleave only hits the nearest ally
            FORCE_CAST(warrior, creature1, ClassSpells::Warrior::CLEAVE_RNK_6, SPELL_MISS_NONE, TRIGGERED_IGNORE_POWER_AND_REAGENT_COST);
            warrior->AttackerStateUpdate(creature1, BASE_ATTACK);
            TEST_ASSERT(creature1->GetHealth() < startHealth);
            TEST_ASSERT(creature2->GetHealth() < startHealth);
            TEST_ASSERT(creature3->GetHealth() == startHealth);

            auto testThreat = [&](Unit* target) {
                float const flatThreat = 130.0f; //split among creatures
                uint32 const targetCount = 2;
                auto[dmgMin, dmgMax] = GetDamagePerSpellsTo(warrior, target, ClassSpells::Warrior::CLEAVE_RNK_6, {}, 1);
                float const expectedCreatureThreat = (flatThreat / targetCount + dmgMin) * WARRIOR_STANCE_THREAT_MOD;
                ASSERT_INFO("Warrior has %f threat with Creature but %f was expected.", target->GetThreatManager().GetThreat(warrior), expectedCreatureThreat);
                TEST_ASSERT(Between<float>(target->GetThreatManager().GetThreat(warrior), expectedCreatureThreat - 0.1f, expectedCreatureThreat + 0.1f));
            };

            testThreat(creature1);
            testThreat(creature2);
          
            // Damage
            auto[minCleave, maxCleave] = CalcMeleeDamage(warrior, creature1, BASE_ATTACK, ClassSpellsDamage::Warrior::CLEAVE_RNK_6);
            TEST_DIRECT_SPELL_DAMAGE(warrior, creature1, ClassSpells::Warrior::CLEAVE_RNK_6, minCleave, maxCleave, false);
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<CleaveTestImpt>();
    }
};

class CommandingShoutTest : public TestCaseScript
{
public:
    /*
    Threat:
        - Omen: 58
        - DTM: /
        - Lavina: 68
        - Ciderhelm: 68
    Decided: 68
    */
    CommandingShoutTest() : TestCaseScript("spells warrior commanding_shout") { }

    class CommandingShoutTestImpt : public TestCaseWarrior
    {
    public:


        void Test() override
        {
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_ORC);

            Position spawn(_location);
            spawn.MoveInFront(spawn, 10.0f);
            TestPlayer* hunter = SpawnPlayer(CLASS_HUNTER, RACE_ORC, 70, spawn);
            TestPlayer* hunter2 = SpawnPlayer(CLASS_HUNTER, RACE_ORC, 70, spawn);
            spawn.MoveInFront(spawn, 30.0f);
            TestPlayer* ally2 = SpawnPlayer(CLASS_WARRIOR, RACE_ORC, 70, spawn);
            Creature* creature = SpawnCreature();

            // Setup
            GroupPlayer(warrior, hunter);
            GroupPlayer(warrior, hunter2);
            GroupPlayer(warrior, ally2);
            creature->GetThreatManager().AddThreat(warrior, 0.0f);
            creature->GetThreatManager().AddThreat(hunter, 0.0f);

            uint32 commandingShoutBonus = 1080;
            uint32 warriorExpectedHealth = warrior->GetHealth() + commandingShoutBonus;
            uint32 hunterExpectedHealth = hunter->GetHealth() + commandingShoutBonus;
            /* Threat should act as follow:
            The flat 69 threat is divided between each target => 23
            For each player this 23 threat gets forwarded to creatures in combat with the player.
            In this case, 23 get forwarded to creature two times, one for warrior and one for hunter.
            So the final calculation is the following:
            */
            float const expectedThreat = 68 / 3.0f * 2 * WARRIOR_STANCE_THREAT_MOD;

            // Rage cost
            uint32 const expectedCommandingShoutRage = 10 * 10;
            TEST_POWER_COST(warrior, ClassSpells::Warrior::COMMANDING_SHOUT_RNK_1, POWER_RAGE, expectedCommandingShoutRage);

            warrior->SetFullPower(POWER_RAGE);
            TEST_CAST(warrior, warrior, ClassSpells::Warrior::COMMANDING_SHOUT_RNK_1);

            // Aura duration
            TEST_AURA_MAX_DURATION(warrior, ClassSpells::Warrior::COMMANDING_SHOUT_RNK_1, Minutes(2));
            TEST_AURA_MAX_DURATION(hunter, ClassSpells::Warrior::COMMANDING_SHOUT_RNK_1, Minutes(2));
            TEST_AURA_MAX_DURATION(hunter2, ClassSpells::Warrior::COMMANDING_SHOUT_RNK_1, Minutes(2));

            // Range
            TEST_HAS_NOT_AURA(ally2, ClassSpells::Warrior::COMMANDING_SHOUT_RNK_1);

            // Health
            ASSERT_INFO("Warrior has %u HP but %u HP was expected.", warrior->GetHealth(), warriorExpectedHealth);
            TEST_ASSERT(warrior->GetHealth() == warriorExpectedHealth);
            ASSERT_INFO("Hunter has %u HP but %u HP was expected.", hunter->GetHealth(), hunterExpectedHealth);
            TEST_ASSERT(hunter->GetHealth() == hunterExpectedHealth);
            ASSERT_INFO("Hunter2 has %u HP but %u HP was expected.", hunter->GetHealth(), hunterExpectedHealth);
            TEST_ASSERT(hunter2->GetHealth() == hunterExpectedHealth);

            // Threat
            ASSERT_INFO("Warrior has %f threat with Creature but %f was expected.", creature->GetThreatManager().GetThreat(warrior), expectedThreat);
            TEST_ASSERT(Between<float>(creature->GetThreatManager().GetThreat(warrior), expectedThreat - 0.1f, expectedThreat + 0.1f));
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<CommandingShoutTestImpt>();
    }
};

class DemoralizingShoutTest : public TestCaseScript
{
public:
    DemoralizingShoutTest() : TestCaseScript("spells warrior demoralizing_shout") { }

    class DemoralizingShoutTestImpt : public TestCase
    {
    public:
        /*
        "Reduces the melee attack power of all enemies within 10 yards by 300 for 30sec."
        Threat:
            - Omen: 56
            - DTM: 55
            - Lavina: 56
        Decided: 56
        */


        void Test() override
        {
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN);

            Position spawn3m(_location);
            spawn3m.MoveInFront(_location, 3.0f);
            TestPlayer* player3m = SpawnPlayer(CLASS_ROGUE, RACE_HUMAN, 1, spawn3m);
            Creature* creature3m = SpawnCreatureWithPosition(spawn3m, 6);

            Position spawn6m(_location);
            spawn6m.MoveInFront(_location, 6.0f);
            Creature* creature6m = SpawnCreatureWithPosition(spawn6m, 6);

            Position spawn15m(_location);
            spawn15m.MoveInFront(_location, 15.0f);
            Creature* creature15m = SpawnCreatureWithPosition(spawn15m, 6);

            uint32 const demoralizingShoutMalus = 300;
            float const startAP3m = player3m->GetTotalAttackPowerValue(BASE_ATTACK);
            float const startAP6m = creature6m->GetTotalAttackPowerValue(BASE_ATTACK);
            float const expectedAP3m = (startAP3m - demoralizingShoutMalus > 0) ? startAP3m - demoralizingShoutMalus : 0;
            float const expectedAP6m = (startAP6m - demoralizingShoutMalus > 0) ? startAP6m - demoralizingShoutMalus : 0;

            // Rage cost
            uint32 const expectedDemoralizingShoutRage = 10 * 10;
            TEST_POWER_COST(warrior, ClassSpells::Warrior::DEMORALIZING_SHOUT_RNK_7, POWER_RAGE, expectedDemoralizingShoutRage);

            warrior->SetFullPower(POWER_RAGE);
            FORCE_CAST(warrior, warrior, ClassSpells::Warrior::DEMORALIZING_SHOUT_RNK_7);

            // Aura
            TEST_AURA_MAX_DURATION(player3m, ClassSpells::Warrior::DEMORALIZING_SHOUT_RNK_7, Seconds(30));
            TEST_AURA_MAX_DURATION(creature3m, ClassSpells::Warrior::DEMORALIZING_SHOUT_RNK_7, Seconds(30));
            TEST_AURA_MAX_DURATION(creature6m, ClassSpells::Warrior::DEMORALIZING_SHOUT_RNK_7, Seconds(30));
            TEST_HAS_NOT_AURA(creature15m, ClassSpells::Warrior::DEMORALIZING_SHOUT_RNK_7);
            uint32 const affectedTargets = 3;

            // AP loss
            TEST_ASSERT(Between<float>(player3m->GetTotalAttackPowerValue(BASE_ATTACK), expectedAP3m - 0.1f, expectedAP3m + 0.1f));
            TEST_ASSERT(Between<float>(creature6m->GetTotalAttackPowerValue(BASE_ATTACK), expectedAP6m - 0.1f, expectedAP6m + 0.1f));

            // Threat -- 56 split per target hit
            float const expectedThreat = 56 / float(affectedTargets) * WARRIOR_STANCE_THREAT_MOD;
            ASSERT_INFO("Warrior has %f threat on Creature3m but %f was expected.", creature3m->GetThreatManager().GetThreat(warrior), expectedThreat);
            TEST_ASSERT(Between<float>(creature3m->GetThreatManager().GetThreat(warrior), expectedThreat - 0.1f, expectedThreat + 0.1f));
            ASSERT_INFO("Warrior has %f threat on Creature6m but %f was expected.", creature6m->GetThreatManager().GetThreat(warrior), expectedThreat);
            TEST_ASSERT(Between<float>(creature6m->GetThreatManager().GetThreat(warrior), expectedThreat - 0.1f, expectedThreat + 0.1f));
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<DemoralizingShoutTestImpt>();
    }
};

class ExecuteTest : public TestCaseScript
{
public:
    ExecuteTest() : TestCaseScript("spells warrior execute") { }

    //"Attempt to finish off a wounded foe, causing 925 damage and converting each extra point of rage into 21 additional damage. 
    // Only usable on enemies that have less than 20% health."
    class ExecuteTestImpt : public TestCaseWarrior
    {
    public:
        /*
        Threat:
            - Omen: 125% (Probably to remove the 0.8 threat factor from battle stance)
            - DTM: 125% (Probably to remove the 0.8 threat factor from battle stance)
            - Lavina: 100% (Unaffected by Stance Multiplier)
            - Ciderhelm: 100% (Unaffected by Stance Multiplier)
        Decided: 100%
        */


        void Test() override
        {
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN);
            Creature* creature = SpawnCreature();

            creature->DisableRegeneration(true);
            creature->SetHealth(creature->GetHealth() * 0.19f);

            // Needs target below 20%
            Creature* dummy = SpawnCreature();
            TEST_CAST(warrior, dummy, ClassSpells::Warrior::EXECUTE_RNK_7, SPELL_FAILED_TARGET_AURASTATE, TRIGGERED_IGNORE_POWER_AND_REAGENT_COST);
            dummy->DespawnOrUnsummon();

            EQUIP_NEW_ITEM(warrior, WG_MH);
            EQUIP_NEW_ITEM(warrior, WG_OH);

            // Rage cost
            uint32 const expectedExecuteRageCost = 15 * 10;
            TEST_POWER_COST(warrior, ClassSpells::Warrior::EXECUTE_RNK_7, POWER_RAGE, expectedExecuteRageCost);

            // Threat
            dummy->GetThreatManager().ResetThreat(warrior);
            warrior->SetFullPower(POWER_RAGE);
            dummy->SetHealth(dummy->GetMaxHealth() * 0.1f);
            WaitNextUpdate(); //Wait for AURA_STATE_HEALTHLESS_20_PERCENT to update on unit
            FORCE_CAST(warrior, creature, ClassSpells::Warrior::EXECUTE_RNK_7);
            auto[minDmg, maxDmg] = GetDamagePerSpellsTo(warrior, creature, ClassSpells::Warrior::EXECUTE_RNK_7_TRIGGER, {}, 1);
            float const expectedThreat = minDmg;
            ASSERT_INFO("Warrior has %f threat on Creature but %f was expected.", creature->GetThreatManager().GetThreat(warrior), expectedThreat);
            TEST_ASSERT(Between<float>(creature->GetThreatManager().GetThreat(warrior), expectedThreat - 0.1f, expectedThreat + 0.1f));

            // Damage (tested at 100 rage)
            uint32 const remainingRage = (1000 - expectedExecuteRageCost) * 0.1f; // full rage - execute cost
            float const armorFactor = GetArmorFactor(warrior, creature);
            uint32 expectedExecuteDmg = floor(ClassSpellsDamage::Warrior::EXECUTE_RNK_7 + remainingRage * ClassSpellsDamage::Warrior::EXECUTE_RNK_7_RAGE) * armorFactor;

            auto callback = [&](Unit* caster, Unit* victim) {
                victim->SetHealth(victim->GetHealth() * 0.19f);
                warrior->SetFullPower(POWER_RAGE);
                //little hack here, remove rage cost before casting the spell, because TEST_DIRECT_SPELL_DAMAGE will cast the spell with no power cost
                caster->ModifyPower(POWER_RAGE, -int32(expectedExecuteRageCost));
            };
            TEST_DIRECT_SPELL_DAMAGE(warrior, creature, ClassSpells::Warrior::EXECUTE_RNK_7, expectedExecuteDmg, expectedExecuteDmg, false, callback, ClassSpells::Warrior::EXECUTE_RNK_7_TRIGGER);
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<ExecuteTestImpt>();
    }
};

class InterceptTest : public TestCaseScript
{
public:
    InterceptTest() : TestCaseScript("spells warrior intercept") { }

    //"Charge an enemy, causing 105 damage and stunning it for 3sec."
    class InterceptTestImpt : public TestCaseWarrior
    {
    public:


        void Test() override
        {
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN);

            Position spawn(_location);
            spawn.MoveInFront(_location, 15.0f);
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_HUMAN, 70, spawn);
            Creature* creature = SpawnCreatureWithPosition(spawn);

            warrior->DisableRegeneration(true);
            warrior->SetWalk(false); // run

            // Stances
            TestStancesAllowed(warrior, creature, ClassSpells::Warrior::INTERCEPT_RNK_5, false, false, true);

            // Range
            TEST_RANGE(warrior, creature, ClassSpells::Warrior::INTERCEPT_RNK_5, 25.f, 8.f);
            TeleportToSpawn(warrior);

            // Rage cost
            uint32 expectedInterceptRageCost = 10 * 10;
            TEST_POWER_COST(warrior, ClassSpells::Warrior::INTERCEPT_RNK_5, POWER_RAGE, expectedInterceptRageCost);

            SwitchTo(warrior, BERSERKER_STANCE);
            warrior->SetPower(POWER_RAGE, expectedInterceptRageCost);
            TEST_CAST(warrior, priest, ClassSpells::Warrior::INTERCEPT_RNK_5);

            // Cooldown
            TEST_HAS_COOLDOWN(warrior, ClassSpells::Warrior::INTERCEPT_RNK_5, Seconds(30));

            // Stun
            Wait(250);
            TEST_AURA_MAX_DURATION(priest, ClassSpells::Warrior::INTERCEPT_RNK_5_TRIGGER, Seconds(3));

            Wait(1000);
            // Warrior still running after Intercept
            TEST_ASSERT(!warrior->IsWalking());

            // Damage
            float const armorFactor = GetArmorFactor(warrior, creature);
            uint32 const expectedIntercept = ClassSpellsDamage::Warrior::INTERCEPT_RNK_5 * armorFactor;
            TEST_DIRECT_SPELL_DAMAGE(warrior, creature, ClassSpells::Warrior::INTERCEPT_RNK_5_TRIGGER, expectedIntercept, expectedIntercept, false);
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<InterceptTestImpt>();
    }
};

class IntidimidatingShoutTest : public TestCaseScript
{
public:
    IntidimidatingShoutTest() : TestCaseScript("spells warrior intimidating_shout") { }

    //The warrior shouts, causing enemies within 10 yards to cower in fear. Up to 5 total nearby enemies will flee in fear. Lasts 8sec.
    class IntidimidatingShoutTestImpt : public TestCaseWarrior
    {
    public:


        void Test() override
        {
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN);

            Position spawn(_location);
            spawn.MoveInFront(_location, 9.0f);
            TestPlayer* priest1 = SpawnPlayer(CLASS_PRIEST, RACE_HUMAN);
            TestPlayer* priest2 = SpawnPlayer(CLASS_PRIEST, RACE_HUMAN);
            TestPlayer* priest3 = SpawnPlayer(CLASS_PRIEST, RACE_HUMAN);
            TestPlayer* priest4 = SpawnPlayer(CLASS_PRIEST, RACE_HUMAN);
            TestPlayer* priest5 = SpawnPlayer(CLASS_PRIEST, RACE_HUMAN);
            TestPlayer* priest6 = SpawnPlayer(CLASS_PRIEST, RACE_HUMAN);

            // Rage cost
            uint32 expectedIntidimidatingShoutRageCost = 25 * 10;
            TEST_POWER_COST(warrior, ClassSpells::Warrior::INTIMIDATING_SHOUT_RNK_1, POWER_RAGE, expectedIntidimidatingShoutRageCost);
            Wait(3000);
            warrior->SetPower(POWER_RAGE, expectedIntidimidatingShoutRageCost);
            FORCE_CAST(warrior, priest1, ClassSpells::Warrior::INTIMIDATING_SHOUT_RNK_1);
            TEST_HAS_COOLDOWN(warrior, ClassSpells::Warrior::INTIMIDATING_SHOUT_RNK_1, Minutes(3));

            // Trigger on target
            TEST_AURA_MAX_DURATION(priest1, ClassSpells::Warrior::INTIMIDATING_SHOUT_RNK_1_TRIGGER, Seconds(8));
            // https://youtu.be/9DA10SgPsj4?t=1m14s
            // Target should have only one debuff
            TEST_HAS_NOT_AURA(priest1, ClassSpells::Warrior::INTIMIDATING_SHOUT_RNK_1);

            // Fear on nearby enemies 
            // WoWWiki: "At the same time, it makes 5 other enemies within 10 yards to run away randomly" -> main target is seemingly not counted
            // Other source: https://youtu.be/CFlVAV9Tm-Y?t=36s - I see 5 enemies running + immune on main target
            uint32 count = 0;
            for (auto priest : { priest2, priest3, priest4, priest5, priest6 })
                count += uint32(priest->HasAura(ClassSpells::Warrior::INTIMIDATING_SHOUT_RNK_1));

            ASSERT_INFO("Count %u", count);
            TEST_ASSERT(count == 5);

            // Target effect should break on damage
            Unit::DealDamage(warrior, priest1, 1);
            WaitNextUpdate();
            TEST_HAS_NOT_AURA(priest1, ClassSpells::Warrior::INTIMIDATING_SHOUT_RNK_1_TRIGGER);

        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<IntidimidatingShoutTestImpt>();
    }
};

class PummelTest : public TestCaseScript
{
public:
    PummelTest() : TestCaseScript("spells warrior pummel") { }

    class PummelTestImpt : public TestCaseWarrior
    {
    public:


        void Test() override
        {
            TestPlayer* warrior;
						
            BEFORE_EACH([&] {
                warrior = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN);
                SwitchTo(warrior, BERSERKER_STANCE);
            });
            
            // Stances
            SECTION("Stances", [&]{
                TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_HUMAN);
                TestStancesAllowed(warrior, priest, ClassSpells::Warrior::PUMMEL_RNK_2, false, false, true);
            });

            uint32 const expectedPummelRageCost = 10 * 10;
            SECTION("Cost", [&]{
                TEST_POWER_COST(warrior, ClassSpells::Warrior::PUMMEL_RNK_2, POWER_RAGE, expectedPummelRageCost);
            });

			SECTION("Effects", [&]{
                TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_HUMAN);
                TEST_CAST(priest, priest, ClassSpells::Priest::GREATER_HEAL_RNK_7);
                Wait(500);
                warrior->SetPower(POWER_RAGE, expectedPummelRageCost);
                FORCE_CAST(warrior, priest, ClassSpells::Warrior::PUMMEL_RNK_2);
                TEST_HAS_COOLDOWN(warrior, ClassSpells::Warrior::PUMMEL_RNK_2, Seconds(10));
                TEST_CAST(priest, priest, ClassSpells::Priest::FLASH_HEAL_RNK_9, SPELL_FAILED_NOT_READY);
                TEST_CAST(priest, priest, ClassSpells::Priest::RENEW_RNK_12, SPELL_FAILED_NOT_READY);
                TEST_CAST(priest, warrior, ClassSpells::Priest::SHADOW_WORD_PAIN_RNK_10);
                Wait(4100);
                TEST_CAST(priest, priest, ClassSpells::Priest::FLASH_HEAL_RNK_9);
            });

            SECTION("Damage", [&]{
                TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_HUMAN);
                float const armorFactor = GetArmorFactor(warrior, priest);
                uint32 const expectedPummel = ClassSpellsDamage::Warrior::PUMMEL_RNK_2 * armorFactor;
                TEST_DIRECT_SPELL_DAMAGE(warrior, priest, ClassSpells::Warrior::PUMMEL_RNK_2, expectedPummel, expectedPummel, false);
            });
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<PummelTestImpt>();
    }
};

class RecklessnessTest : public TestCaseScript
{
public:
    RecklessnessTest() : TestCaseScript("spells warrior recklessness") { }

    class RecklessnessTestImpt : public TestCaseWarrior
    {
    public:


        void Test() override
        {
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN);
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_HUMAN);
            Creature* creature = SpawnCreature();

            TestStancesAllowed(warrior, warrior, ClassSpells::Warrior::RECKLESSNESS_RNK_1, false, false, true);
            SwitchTo(warrior, BERSERKER_STANCE);

            TEST_CAST(warrior, warrior, ClassSpells::Warrior::RECKLESSNESS_RNK_1);
            TEST_AURA_MAX_DURATION(warrior, ClassSpells::Warrior::RECKLESSNESS_RNK_1, Seconds(15));
            TEST_HAS_COOLDOWN(warrior, ClassSpells::Warrior::RECKLESSNESS_RNK_1, Minutes(30));

            // Damage taken increase
            float const berserkerStanceFactor = 1.1f;
            float const reckelessnessFactor = 1.2f;
            uint32 const expectedShadowBoltMin = ClassSpellsDamage::Warlock::SHADOW_BOLT_RNK_11_MIN * berserkerStanceFactor * reckelessnessFactor;
            uint32 const expectedShadowBoltMax = ClassSpellsDamage::Warlock::SHADOW_BOLT_RNK_11_MAX * berserkerStanceFactor * reckelessnessFactor;
            TEST_DIRECT_SPELL_DAMAGE(warlock, warrior, ClassSpells::Warlock::SHADOW_BOLT_RNK_11, expectedShadowBoltMin, expectedShadowBoltMax, false);

            // Fear immune
            TEST_CAST(warlock, warrior, ClassSpells::Warlock::FEAR_RNK_3, SPELL_CAST_OK, TRIGGERED_CAST_DIRECTLY);
            TEST_HAS_NOT_AURA(warrior, ClassSpells::Warlock::FEAR_RNK_3);

            // Damage
            TEST_SPELL_CRIT_CHANCE(warrior, creature, ClassSpells::Warrior::HEROIC_STRIKE_RNK_10, 100.f, [](Unit* caster, Unit* victim) {
                caster->AddAura(ClassSpells::Warrior::RECKLESSNESS_RNK_1, caster);
            });
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<RecklessnessTestImpt>();
    }
};

class SlamTest : public TestCaseScript
{
public:
    SlamTest() : TestCaseScript("spells warrior slam") { }

    class SlamTestImpt : public TestCaseWarrior
    {
    public:


        void Test() override
        {
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN);
            Creature* creature = SpawnCreature();

            warrior->DisableRegeneration(true);

            TestRequiresMeleeWeapon(warrior, creature, ClassSpells::Warrior::SLAM_RNK_6);
            EQUIP_NEW_ITEM(warrior, 2489); //two handed sword
            uint32 const weaponAttackTimer = 2900;
            uint32 const slamCastTime = 1500;

            uint32 const expectedSlamRage = 15 * 10;
            TEST_POWER_COST(warrior, ClassSpells::Warrior::SLAM_RNK_6, POWER_RAGE, expectedSlamRage);

            // Reset swing timer
            warrior->Attack(creature, true);
            Wait(weaponAttackTimer - 1000);
            warrior->SetPower(POWER_RAGE, expectedSlamRage);
            FORCE_CAST(warrior, creature, ClassSpells::Warrior::SLAM_RNK_6);
            uint32 oneUpdateDiff = 50;
            Wait(slamCastTime - oneUpdateDiff);
            //... We need to have two seperate wait because in unit updates, spell reset swings before the attack timer is decreased. If we did Wait(1600) we would have a reset THEN a 1600 decrease.
            Wait(oneUpdateDiff);
            // Make sure slam was launched
            auto AI = _GetCasterAI(warrior);
            auto damageDone = AI->GetSpellDamageDoneInfo(creature);
            TEST_ASSERT(damageDone != nullptr && !damageDone->empty())
            // Should have reset swing so AttackTime back to decreasing from weaponAttackTimer and should be approx weaponAttackTimer, minus time since last update
            uint32 currentAttackTimer = warrior->GetAttackTimer(BASE_ATTACK);
            uint32 expectedAttackTimer = weaponAttackTimer - oneUpdateDiff;
            ASSERT_INFO("Current Attack Time: %u, expected: %u", currentAttackTimer, expectedAttackTimer);
            //at time of writing, currentAttackTimer is exactly expectedAttackTimer but this may vary a little if implementation changes
            TEST_ASSERT(Between(currentAttackTimer, expectedAttackTimer - oneUpdateDiff, expectedAttackTimer + oneUpdateDiff));

            // Damage
            auto[minSlam, maxSlam] = CalcMeleeDamage(warrior, creature, BASE_ATTACK, ClassSpellsDamage::Warrior::SLAM_RNK_6);
            TEST_DIRECT_SPELL_DAMAGE(warrior, creature, ClassSpells::Warrior::SLAM_RNK_6, minSlam, maxSlam, false);
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<SlamTestImpt>();
    }
};

class VictoryRushTest : public TestCaseScript
{
public:
    VictoryRushTest() : TestCaseScript("spells warrior victory_rush") { }

    class VictoryRushTestImpt : public TestCaseWarrior
    {
    public:


        void Test() override
        {
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN);
            TestPlayer* enemy1 = SpawnPlayer(CLASS_WARRIOR, RACE_HUMAN, 1);
            TestPlayer* enemy70 = SpawnPlayer(CLASS_WARRIOR, RACE_HUMAN);
            Creature* creature = SpawnCreature();

            warrior->ForceMeleeHitResult(MELEE_HIT_NORMAL);
            enemy1->SetHealth(1);
            enemy70->SetHealth(1);

            TestStancesAllowed(warrior, creature, ClassSpells::Warrior::VICTORY_RUSH_RNK_1, true, false, true);

            // Kill level 1
            warrior->AttackerStateUpdate(enemy1, BASE_ATTACK);
            WaitNextUpdate();
            TEST_ASSERT(enemy1->IsDead());
            TEST_HAS_NOT_AURA(warrior, ClassSpells::Warrior::VICTORY_RUSH_RNK_1_TRIGGER);

            // Kill level 70
            warrior->AttackerStateUpdate(enemy70, BASE_ATTACK);
            WaitNextUpdate();
            TEST_ASSERT(enemy70->IsDead());
            TEST_AURA_MAX_DURATION(warrior, ClassSpells::Warrior::VICTORY_RUSH_RNK_1_TRIGGER, Seconds(20));
            FORCE_CAST(warrior, creature, ClassSpells::Warrior::VICTORY_RUSH_RNK_1);
            TEST_HAS_NOT_AURA(warrior, ClassSpells::Warrior::VICTORY_RUSH_RNK_1_TRIGGER);

            // Damage
            float const victoryRushAPFactor = 0.45f;
            float const AP = warrior->GetTotalAttackPowerValue(BASE_ATTACK);
            float const armorFactor = GetArmorFactor(warrior, creature);
            uint32 const expectedVictoryRush = AP * victoryRushAPFactor * armorFactor;
            TEST_DIRECT_SPELL_DAMAGE(warrior, creature, ClassSpells::Warrior::VICTORY_RUSH_RNK_1, expectedVictoryRush, expectedVictoryRush, false);
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<VictoryRushTestImpt>();
    }
};

class WhirlwindTest : public TestCaseScript
{
public:
    WhirlwindTest() : TestCaseScript("spells warrior whirlwind") { }

    /*
    "In a whirlwind of steel you attack up to 4 enemies within 8 yards, causing weapon damage from both melee weapons to each enemy."
    */
    class WhirlwindTestImpt : public TestCaseWarrior
    {
    public:


        void Test() override
        {
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN);
            SwitchTo(warrior, BERSERKER_STANCE);
            EQUIP_NEW_ITEM(warrior, WG_MH);
            EQUIP_NEW_ITEM(warrior, WG_OH);

            // Power cost
            uint32 const expectedWhirldwindRage = 25 * 10;
            TEST_POWER_COST(warrior, ClassSpells::Warrior::WHIRLWIND_RNK_1, POWER_RAGE, expectedWhirldwindRage);

            SECTION("Damages", [&] {
                // Damage
                Creature* creature = SpawnCreature();
                float const normalizedSwordSpeed = 2.4f;
                // MH
                auto[minMH, maxMH] = CalcMeleeDamage(warrior, creature, BASE_ATTACK, 0, normalizedSwordSpeed);
                TEST_DIRECT_SPELL_DAMAGE(warrior, creature, ClassSpells::Warrior::WHIRLWIND_RNK_1, minMH, maxMH, false);
                // OH
                auto[minOH, maxOH] = CalcMeleeDamage(warrior, creature, OFF_ATTACK, 0, normalizedSwordSpeed);
                TEST_DIRECT_SPELL_DAMAGE(warrior, creature, ClassSpells::Warrior::WHIRLWIND_RNK_1, minOH, maxOH, false, DefaultCallback, ClassSpells::Warrior::WHIRLWIND_RNK_1_TRIGGER);

                creature->DisappearAndDie();
            });

            SECTION("Target counts", [&] {
                // Hit max 4 targets
                // WoWWiki: It does Weapon Damage to 4 adjacent enemies, in any direction and has a 10 second cooldown.
                // BUT: Since the spell triggers a second aoe spell for the off hand, and it may not hit the same targets as the main spell. 
                // Is this supposed to happen? This is coherent with the spell effects and targets, but Blizzard may have hacked something
                // to make sure off hand spell hit the same targets.
                // Unless proven though, we're gonna suppose that is NOT the case.
                Creature* creature1 = SpawnCreature();
                Creature* creature2 = SpawnCreature();
                Creature* creature3 = SpawnCreature();
                Creature* creature4 = SpawnCreature();
                Creature* creature5 = SpawnCreature();
                Creature* creature6 = SpawnCreature();

                warrior->SetFullPower(POWER_RAGE);
                auto AI = _GetCasterAI(warrior);
                AI->ResetSpellCounters();

                FORCE_CAST(warrior, creature1, ClassSpells::Warrior::WHIRLWIND_RNK_1);

                auto HasSpellHit = [&](Unit const* target, uint32 spellId) {
                    auto damageDone = AI->GetSpellDamageDoneInfo(target);
                    if (!damageDone)
                        return false;
                    for (auto info : *damageDone)
                    {
                        if (info.spellID == spellId)
                            return true;
                    }
                    return false;
                };

                auto CountHits = [&](uint32 spellId) {
                    uint32 hits = 0;
                    for (auto creature : { creature1, creature2, creature3, creature4, creature5, creature6 })
                        hits += uint32(HasSpellHit(creature, spellId));

                    return hits;
                };

                uint32 MHandHits = CountHits(ClassSpells::Warrior::WHIRLWIND_RNK_1);
                uint32 OHandHits = CountHits(ClassSpells::Warrior::WHIRLWIND_RNK_1_TRIGGER);


                ASSERT_INFO("Whirlwind MH should hit 4 max, but actually hit %u.", MHandHits);
                TEST_ASSERT(MHandHits == 4);
                ASSERT_INFO("Whirlwind OH should hit 4 max, but actually hit %u.", OHandHits);
                TEST_ASSERT(OHandHits == 4);
            });
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<WhirlwindTestImpt>();
    }
};

class BloodrageTest : public TestCaseScript
{
public:
    BloodrageTest() : TestCaseScript("spells warrior bloodrage") { }

    class BloodrageTestImpt : public TestCaseWarrior
    {
    public:


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
            ASSERT_INFO("Warrior has %u HP but %u was expected.", warrior->GetHealth(), expectedHealth);
            TEST_ASSERT(warrior->GetHealth() == expectedHealth);
            TEST_AURA_MAX_DURATION(warrior, ClassSpells::Warrior::BLOODRAGE_RNK_1_TRIGGER, Seconds(10));
            TEST_HAS_COOLDOWN(warrior, ClassSpells::Warrior::BLOODRAGE_RNK_1, Minutes(1));

            // Rage gain
            TEST_ASSERT(warrior->GetPower(POWER_RAGE) == 10 * 10); // Instant gain
            Wait(10000);
            TEST_ASSERT(Between<uint32>(warrior->GetPower(POWER_RAGE), 20 * 10 - 3, 20 * 10)); // might have lower because out of combat
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<BloodrageTestImpt>();
    }
};

class DisarmTest : public TestCaseScript
{
public:
    /*
    Threat:
        Omen: 104
        DTM: 105
        Lavina: 104
        Ciderhelm: 104
    Decided: 104
    */
    DisarmTest() : TestCaseScript("spells warrior disarm") { }

    class DisarmTestImpt : public TestCaseWarrior
    {
    public:


        void Test() override
        {
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN);

            // Stances & weapon
            TestPlayer* rogue2 = SpawnPlayer(CLASS_ROGUE, RACE_HUMAN);
            EQUIP_NEW_ITEM(rogue2, WG_MH);
            Wait(500);
            TestStancesAllowed(warrior, rogue2, ClassSpells::Warrior::DISARM_RNK_1, false, true, false);
            SwitchTo(warrior, DEFENSIVE_STANCE);
            rogue2->KillSelf();

            // Power cost
            uint32 expectedDisarmRage = 20 * 10;
            TEST_POWER_COST(warrior, ClassSpells::Warrior::DISARM_RNK_1, POWER_RAGE, expectedDisarmRage);

            // Victim setup
            Creature* dummy = SpawnCreature();;
            TestPlayer* rogue = SpawnPlayer(CLASS_ROGUE, RACE_HUMAN);
            RemoveAllEquipedItems(rogue);

            // Test unarmed damage without Disarm
            float const armorFactor = GetArmorFactor(rogue, dummy);
            float const rogueAPUnarmed = rogue->GetTotalAttackPowerValue(BASE_ATTACK);
            float const unarmedMin = rogue->GetFloatValue(UNIT_FIELD_MINDAMAGE);
            float const unarmedMax = rogue->GetFloatValue(UNIT_FIELD_MAXDAMAGE);
            uint32 const expectedUnarmedMinDmg = unarmedMin * armorFactor;
            uint32 const expectedUnarmedMaxDmg = unarmedMax * armorFactor;
            TEST_MELEE_DAMAGE(rogue, dummy, BASE_ATTACK, expectedUnarmedMinDmg, expectedUnarmedMaxDmg, false);

            // Test dmg with WGs
            EQUIP_NEW_ITEM(rogue, WG_MH);
            Wait(1500);
            EQUIP_NEW_ITEM(rogue, WG_OH);
            WaitNextUpdate();
            auto[expectedMHMinDmg, expectedMHMaxDmg] = CalcMeleeDamage(rogue, dummy, BASE_ATTACK);
            auto[expectedOHMinDmg, expectedOHMaxDmg] = CalcMeleeDamage(rogue, dummy, OFF_ATTACK);
            float const rogueAPWithWeapons = rogue->GetTotalAttackPowerValue(BASE_ATTACK);
            TEST_MELEE_DAMAGE(rogue, dummy, BASE_ATTACK, expectedMHMinDmg, expectedMHMaxDmg, false);
            TEST_MELEE_DAMAGE(rogue, dummy, OFF_ATTACK, expectedOHMinDmg, expectedOHMaxDmg, false);

            // Disarm
            warrior->SetPower(POWER_RAGE, expectedDisarmRage);
            FORCE_CAST(warrior, rogue, ClassSpells::Warrior::DISARM_RNK_1);
            TEST_AURA_MAX_DURATION(rogue, ClassSpells::Warrior::DISARM_RNK_1, Seconds(10));
            TEST_HAS_COOLDOWN(warrior, ClassSpells::Warrior::DISARM_RNK_1, Minutes(1));
            TEST_ASSERT(rogue->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISARMED));

            // Damage under disarmed effect
            uint32 const gainedAPWithWeapons = rogueAPWithWeapons - rogueAPUnarmed;
            uint32 const disarmedMin = (unarmedMin + gainedAPWithWeapons / 14 * 2.0f) * armorFactor;
            uint32 const disarmedMax = (unarmedMax + gainedAPWithWeapons / 14 * 2.0f) * armorFactor;
            TEST_MELEE_DAMAGE(rogue, dummy, BASE_ATTACK, disarmedMin, disarmedMax, false, [](Unit* attacker, Unit* victim) {
                attacker->AddAura(ClassSpells::Warrior::DISARM_RNK_1, attacker);
            });
            TEST_MELEE_DAMAGE(rogue, dummy, OFF_ATTACK, expectedOHMinDmg, expectedOHMaxDmg, false, [](Unit* attacker, Unit* victim) {
                attacker->AddAura(ClassSpells::Warrior::DISARM_RNK_1, attacker);
            });

            // Threat
            Creature* coilfangWarrior = SpawnCreature(17802); // Has a weapon
            warrior->GetSpellHistory()->ResetAllCooldowns();
            warrior->SetPower(POWER_RAGE, expectedDisarmRage);
            FORCE_CAST(warrior, coilfangWarrior, ClassSpells::Warrior::DISARM_RNK_1);
            auto[minDmg, maxDmg] = GetDamagePerSpellsTo(warrior, coilfangWarrior, ClassSpells::Warrior::DISARM_RNK_1, {}, 1);
            float const expectedDisarmThreat = (104 + minDmg) * DEFENSIVE_STANCE_THREAT_MOD;
            TEST_ASSERT(Between<float>(coilfangWarrior->GetThreatManager().GetThreat(warrior), expectedDisarmThreat - 0.1f, expectedDisarmThreat + 0.1f));
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<DisarmTestImpt>();
    }
};

class InterveneTest : public TestCaseScript
{
public:
    InterveneTest() : TestCaseScript("spells warrior intervene") { }

    //"Run at high speed towards a party member, intercepting the next melee or ranged attack made against them."
    class InterveneTestImpt : public TestCaseWarrior
    {
    public:


        void Test() override
        {
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN);

            Position spawn(_location);
            spawn.MoveInFront(spawn, 15.0f);
            TestPlayer* rogue = SpawnPlayer(CLASS_ROGUE, RACE_NIGHTELF, 70, spawn);
            TestPlayer* shaman = SpawnPlayer(CLASS_SHAMAN, RACE_TAUREN, 70, spawn);

            // Group 
            GroupPlayer(warrior, shaman);

            // Stances
            TestStancesAllowed(warrior, shaman, ClassSpells::Warrior::INTERVENE_RNK_1, false, true, false);

            // Range
            TEST_RANGE(warrior, shaman, ClassSpells::Warrior::INTERVENE_RNK_1, 25.f, 8.f);
            TeleportToSpawn(warrior);

            // Rage cost
            uint32 const expectedInterveneRage = 10 * 10;
            TEST_POWER_COST(warrior, ClassSpells::Warrior::INTERVENE_RNK_1, POWER_RAGE, expectedInterveneRage);

            SwitchTo(warrior, DEFENSIVE_STANCE);
            warrior->SetPower(POWER_RAGE, expectedInterveneRage);
            TEST_CAST(warrior, shaman, ClassSpells::Warrior::INTERVENE_RNK_1);
            TEST_HAS_COOLDOWN(warrior, ClassSpells::Warrior::INTERVENE_RNK_1, Seconds(30));
            Wait(500); //wait for intervene to hit
            TEST_AURA_MAX_DURATION(shaman, ClassSpells::Warrior::INTERVENE_RNK_1, Seconds(10));

            // Take an attack
            rogue->CastSpell(rogue, ClassSpells::Rogue::STEALTH_RNK_4, true);
            FORCE_CAST(rogue, shaman, ClassSpells::Rogue::SAP_RNK_3);
            WaitNextUpdate();
            TEST_HAS_NOT_AURA(shaman, ClassSpells::Rogue::SAP_RNK_3);
            TEST_HAS_AURA(warrior, ClassSpells::Rogue::SAP_RNK_3);
            TEST_HAS_NOT_AURA(shaman, ClassSpells::Warrior::INTERVENE_RNK_1);
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<InterveneTestImpt>();
    }
};

class RevengeTest : public TestCaseScript
{
public:
    /*
    "Instantly counterattack an enemy for 414 to 506 damage and a high amount of threat. Revenge must follow a block, dodge or parry."
    Threat:
        Omen: 201
        DTM: 200
        Lavina: 201
        Ciderhelm: 200
    Decided: 201
    */
    RevengeTest() : TestCaseScript("spells warrior revenge") { }

    class RevengeTestImpt : public TestCaseWarrior
    {
    public:


        void TestRevengeTrigger(TestPlayer* warrior, TestPlayer* rogue, MeleeHitOutcome meleeHitOutcome)
        {
            rogue->ForceMeleeHitResult(meleeHitOutcome);
            rogue->AttackerStateUpdate(warrior, BASE_ATTACK);
            rogue->ResetForceMeleeHitResult();

            warrior->SetFullPower(POWER_RAGE);
            TEST_CAST(warrior, rogue, ClassSpells::Warrior::REVENGE_RNK_8);
            warrior->GetSpellHistory()->ResetAllCooldowns();
        }

        void Test() override
        {
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN);
            TestPlayer* rogue = SpawnPlayer(CLASS_ROGUE, RACE_HUMAN);
            float const defensiveStanceFactor = 0.9f; //stance decreases damage caused

            // Stances & weapon
            //TestRequiresMeleeWeapon(warrior, rogue, ClassSpells::Warrior::REVENGE_RNK_8);
            EQUIP_NEW_ITEM(warrior, WG_MH);
            TestStancesAllowed(warrior, rogue, ClassSpells::Warrior::REVENGE_RNK_8, false, true, false);
            SwitchTo(warrior, DEFENSIVE_STANCE);

             // Triggers
            EQUIP_NEW_ITEM(warrior, 34185); // Shield to block
            TestRevengeTrigger(warrior, rogue, MELEE_HIT_BLOCK);
            TestRevengeTrigger(warrior, rogue, MELEE_HIT_DODGE);
            TestRevengeTrigger(warrior, rogue, MELEE_HIT_PARRY);

            // Cooldown
            TEST_COOLDOWN(warrior, rogue, ClassSpells::Warrior::REVENGE_RNK_8, Seconds(5));

            // Damage
            float const armorFactor = GetArmorFactor(warrior, rogue);
            uint32 const expectedRevengeMin = ClassSpellsDamage::Warrior::REVENGE_RNK_8_MIN * armorFactor * defensiveStanceFactor;
            uint32 const expectedRevengeMax = ClassSpellsDamage::Warrior::REVENGE_RNK_8_MAX * armorFactor * defensiveStanceFactor;
            TEST_DIRECT_SPELL_DAMAGE(warrior, rogue, ClassSpells::Warrior::REVENGE_RNK_8, expectedRevengeMin, expectedRevengeMax, false);

            // Threat
            Creature* dummy = SpawnCreature();
            FORCE_CAST(warrior, dummy, ClassSpells::Warrior::REVENGE_RNK_8);
            auto[minDmg, maxDmg] = GetDamagePerSpellsTo(warrior, dummy, ClassSpells::Warrior::REVENGE_RNK_8, {}, 1);
            float const expectedRevengeThreat = (201 + minDmg) * DEFENSIVE_STANCE_THREAT_MOD;
            TEST_ASSERT(Between<float>(dummy->GetThreatManager().GetThreat(warrior), expectedRevengeThreat - 0.1f, expectedRevengeThreat + 0.1f));
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<RevengeTestImpt>();
    }
};

class ShieldBashTest : public TestCaseScript
{
public:
    /*
    Threat:
        Omen: 230
        DTM: 230
        Lavina: 230
        Ciderhelm: ??
    Decided: 230
    */
    ShieldBashTest() : TestCaseScript("spells warrior shield_bash") { }

    class ShieldBashTestImpt : public TestCaseWarrior
    {
    public:


        void Test() override
        {
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN);
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_HUMAN);

            RemoveAllEquipedItems(warrior);
            EQUIP_NEW_ITEM(warrior, 34185); // Shield

            // Stances
            TestStancesAllowed(warrior, priest, ClassSpells::Warrior::SHIELD_BASH_RNK_4, true, true, false);

            // Rage cost
            uint32 expectedShieldBashRageCost = 10 * 10;
            TEST_POWER_COST(warrior, ClassSpells::Warrior::SHIELD_BASH_RNK_4, POWER_RAGE, expectedShieldBashRageCost);

            TEST_CAST(priest, priest, ClassSpells::Priest::GREATER_HEAL_RNK_7);
            Wait(500);
            warrior->SetPower(POWER_RAGE, expectedShieldBashRageCost);
            FORCE_CAST(warrior, priest, ClassSpells::Warrior::SHIELD_BASH_RNK_4);
            TEST_COOLDOWN(warrior, priest, ClassSpells::Warrior::SHIELD_BASH_RNK_4, Seconds(12));
            TEST_AURA_MAX_DURATION(priest, ClassSpells::Warrior::SHIELD_BASH_RNK_4_DAZED, Seconds(6));
            TEST_CAST(priest, priest, ClassSpells::Priest::FLASH_HEAL_RNK_9, SPELL_FAILED_NOT_READY);
            TEST_CAST(priest, priest, ClassSpells::Priest::RENEW_RNK_12, SPELL_FAILED_NOT_READY);
            TEST_CAST(priest, warrior, ClassSpells::Priest::SHADOW_WORD_PAIN_RNK_10);
            Wait(6100);
            //TEST_CAST(priest, priest, ClassSpells::Priest::FLASH_HEAL_RNK_9);

            // Damage
            float const armorFactor = GetArmorFactor(warrior, priest);
            uint32 const expectedShieldBash = ClassSpellsDamage::Warrior::SHIELD_BASH_RNK_4 * armorFactor;
            TEST_DIRECT_SPELL_DAMAGE(warrior, priest, ClassSpells::Warrior::SHIELD_BASH_RNK_4, expectedShieldBash, expectedShieldBash, false);

            // Threat
            Creature* dummy = SpawnCreature();
            warrior->SetPower(POWER_RAGE, expectedShieldBashRageCost);
            FORCE_CAST(warrior, dummy, ClassSpells::Warrior::SHIELD_BASH_RNK_4);
            auto[minDmg, maxDmg] = GetDamagePerSpellsTo(warrior, dummy, ClassSpells::Warrior::SHIELD_BASH_RNK_4, {}, 1);
            float const expectedShieldBashThreat = (230 + minDmg) * WARRIOR_STANCE_THREAT_MOD;
            TEST_ASSERT(Between<float>(dummy->GetThreatManager().GetThreat(warrior), expectedShieldBashThreat - 0.1f, expectedShieldBashThreat + 0.1f));
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<ShieldBashTestImpt>();
    }
};

class ShieldBlockTest : public TestCaseScript
{
public:
    ShieldBlockTest() : TestCaseScript("spells warrior shield_block") { }

    class ShieldBlockTestImpt : public TestCaseWarrior
    {
    public:


        void Test() override
        {
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN);

            Position spawn(_location);
            spawn.MoveInFront(spawn, 3.0f);
            TestPlayer* rogue = SpawnPlayer(CLASS_ROGUE, RACE_HUMAN, 70, spawn);
            rogue->SetFacingToObject(warrior);

            RemoveAllEquipedItems(warrior);
            EQUIP_NEW_ITEM(warrior, 8320); //random useless shield
            SwitchTo(warrior, DEFENSIVE_STANCE);

            // Triggers
            float const shieldBlockFactor = 75.f;
            float const expectedBlockChance = warrior->GetUnitBlockChance(BASE_ATTACK, warrior) + shieldBlockFactor;

            EnableCriticals(rogue, false);
            TEST_MELEE_HIT_CHANCE(rogue, warrior, BASE_ATTACK, expectedBlockChance, MELEE_HIT_BLOCK, [](Unit* Attacker, Unit* victim) {
                victim->AddAura(ClassSpells::Warrior::SHIELD_BLOCK_RNK_1, victim);
            });

            // Aura
            warrior->SetPower(POWER_RAGE, 100); //shield block costs 10 rage
            FORCE_CAST(warrior, warrior, ClassSpells::Warrior::SHIELD_BLOCK_RNK_1);
            TEST_AURA_MAX_DURATION(warrior, ClassSpells::Warrior::SHIELD_BLOCK_RNK_1, Seconds(5));
            TEST_HAS_COOLDOWN(warrior, ClassSpells::Warrior::SHIELD_BLOCK_RNK_1, Seconds(5));
            rogue->ForceMeleeHitResult(MELEE_HIT_BLOCK);
            rogue->AttackerStateUpdate(warrior, BASE_ATTACK);
            TEST_HAS_NOT_AURA(warrior, ClassSpells::Warrior::SHIELD_BLOCK_RNK_1);
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<ShieldBlockTestImpt>();
    }
};

class ShieldWallTest : public TestCaseScript
{
public:
    ShieldWallTest() : TestCaseScript("spells warrior shield_wall") { }

    class ShieldWallTestImpt : public TestCaseWarrior
    {
    public:


        void Test() override
        {
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN);
            SwitchTo(warrior, DEFENSIVE_STANCE);

            TestPlayer* rogue = SpawnPlayer(CLASS_ROGUE, RACE_HUMAN);
            EQUIP_NEW_ITEM(rogue, WG_MH);
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_HUMAN);
            TestPlayer* hunter = SpawnPlayer(CLASS_HUNTER, RACE_DWARF);
            EQUIP_NEW_ITEM(hunter, 32336); // Black Bow of the Betrayer
            
            // Aura
            TEST_CAST(warrior, warrior, ClassSpells::Warrior::SHIELD_WALL_RNK_1);
            TEST_AURA_MAX_DURATION(warrior, ClassSpells::Warrior::SHIELD_WALL_RNK_1, Seconds(10));
            TEST_HAS_COOLDOWN(warrior, ClassSpells::Warrior::SHIELD_WALL_RNK_1, Minutes(30));

            // 75% less damage
            float const shieldWallFactor = 1 - 0.75f;
            float const defensiveStanceFactor = 1 - 0.1f;
            auto callback = [](Unit* caster, Unit* victim) {
                victim->AddAura(ClassSpells::Warrior::SHIELD_WALL_RNK_1, victim);
            };
            // Spell
            uint32 const expectedSBMin = ClassSpellsDamage::Warlock::SHADOW_BOLT_RNK_11_MIN * defensiveStanceFactor * shieldWallFactor;
            uint32 const expectedSBMax = ClassSpellsDamage::Warlock::SHADOW_BOLT_RNK_11_MAX * defensiveStanceFactor * shieldWallFactor;
            TEST_DIRECT_SPELL_DAMAGE(warlock, warrior, ClassSpells::Warlock::SHADOW_BOLT_RNK_11, expectedSBMin, expectedSBMax, false, callback);
            // Melee
            auto[expectedMHMin, expectedMHMax] = CalcMeleeDamage(rogue, warrior, BASE_ATTACK);
            expectedMHMin *= defensiveStanceFactor * shieldWallFactor;
            expectedMHMax *= defensiveStanceFactor * shieldWallFactor;
            TEST_MELEE_DAMAGE(rogue, warrior, BASE_ATTACK, expectedMHMin, expectedMHMax, false, callback);
            // Ranged
            auto[expectedRangedMin, expectedRangedMax] = CalcMeleeDamage(hunter, warrior, RANGED_ATTACK);
            expectedRangedMin *= defensiveStanceFactor * shieldWallFactor;
            expectedRangedMax *= defensiveStanceFactor * shieldWallFactor;
            TEST_MELEE_DAMAGE(hunter, warrior, RANGED_ATTACK, expectedRangedMin, expectedRangedMax, false, callback);
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<ShieldWallTestImpt>();
    }
};

class SpellReflectionTest : public TestCaseScript
{
public:
    SpellReflectionTest() : TestCaseScript("spells warrior spell_reflection") { }

    class SpellReflectionTestImpt : public TestCaseWarrior
    {
    public:
        /*
        Bugs:
            - Aura should be removed when no shield is equiped
        Threat:
            Lavina: 100%
            Ciderhelm: 100%
        Decided: 100%
        */

        void Test() override
        {
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN);

            // Rage cost
            uint32 const expectedSpellReflectionRage = 25 * 10;
            TEST_POWER_COST(warrior, ClassSpells::Warrior::SPELL_REFLECTION_RNK_1, POWER_RAGE, expectedSpellReflectionRage);

            warrior->SetPower(POWER_RAGE, expectedSpellReflectionRage);
            TEST_CAST(warrior, warrior, ClassSpells::Warrior::SPELL_REFLECTION_RNK_1);
            TEST_AURA_MAX_DURATION(warrior, ClassSpells::Warrior::SPELL_REFLECTION_RNK_1, Seconds(5));
            TEST_HAS_COOLDOWN(warrior, ClassSpells::Warrior::SPELL_REFLECTION_RNK_1, Seconds(10));

            SECTION("Reflect", [&] {
                Position spawn(_location);
                spawn.MoveInFront(spawn, 15.0f);
                TestPlayer* fire = SpawnPlayer(CLASS_MAGE, RACE_HUMAN, 70, spawn);
                LearnTalent(fire, Talents::Mage::IMPROVED_FIREBALL_RNK_5);
                TestPlayer* frost = SpawnPlayer(CLASS_MAGE, RACE_HUMAN, 70, spawn);
                fire->SetFacingToObject(warrior);
                frost->SetFacingToObject(warrior);
                //Make sure the mages are hit capped
                fire->ApplyRatingMod(CR_HIT_SPELL, 500, true); 
                frost->ApplyRatingMod(CR_HIT_SPELL, 500, true);

                // Reflect
                TriggerCastFlags flags = TriggerCastFlags(TRIGGERED_FULL_MASK | TRIGGERED_IGNORE_SPEED);
                for (int i = 0; i < 5; i++) //throw some more than 1 spell to make sure we avoid the incompressible resist
                {
                    TEST_CAST(fire, warrior, ClassSpells::Mage::FIREBALL_RNK_1, SPELL_CAST_OK, flags);
                    TEST_CAST(frost, warrior, ClassSpells::Mage::FROSTBOLT_RNK_1, SPELL_CAST_OK, flags);
                }
                WaitNextUpdate();
                // https://youtu.be/ohnzL-deZDM?t=1m24s
                // Spell reflection should be removed as soon as the spell is reflected not when the reflected spells hits its caster
                // Note: On Vanilla/BC (and possibly for some time later), spell reflection could reflect several spell if they were processed in the same batch
                // (To reproduce this we could simply have the spell remove itself a bit after the first reflect instead of immediately, don't remember how this is currently done)
                TEST_HAS_NOT_AURA(warrior, ClassSpells::Warrior::SPELL_REFLECTION_RNK_1);
                TEST_ASSERT(warrior->IsFullHealth());
                TEST_ASSERT(!fire->IsFullHealth());
                TEST_ASSERT(!frost->IsFullHealth());
            });
           
            SECTION("Threat", STATUS_KNOWN_BUG, [&] {
                //Currently 0 threat, threat actually goes to the original caster in spell system atm. (so is this case dummy tries to add threat to dummy and it has no effect)
                Creature* dummy = SpawnCreature();
                TEST_ASSERT(dummy->IsFullHealth());
                dummy->DisableRegeneration(true);
                TEST_CAST(warrior, warrior, ClassSpells::Warrior::SPELL_REFLECTION_RNK_1, SPELL_CAST_OK, TRIGGERED_FULL_MASK);
                for (int i = 0; i < 50; i++) //throw some more than 1 spell to make sure we avoid resist
                    TEST_CAST(dummy, warrior, ClassSpells::Mage::FIREBALL_RNK_1, SPELL_CAST_OK, TriggerCastFlags(TRIGGERED_FULL_MASK | TRIGGERED_IGNORE_SPEED));
                WaitNextUpdate();
                uint32 const damage = dummy->GetMaxHealth() - dummy->GetHealth();
                TEST_ASSERT(damage != 0);
                float const expectedThreat = damage * WARRIOR_STANCE_THREAT_MOD;
                float currentThreat = dummy->GetThreatManager().GetThreat(warrior);
                ASSERT_INFO("Threat %f instead of expected %f", currentThreat, expectedThreat);
                TEST_ASSERT(Between<float>(currentThreat, expectedThreat - 0.1f, expectedThreat + 0.1f));
                dummy->DespawnOrUnsummon();
            });

            SECTION("Removed on shield unequip", STATUS_KNOWN_BUG, [&]{
                TEST_CAST(warrior, warrior, ClassSpells::Warrior::SPELL_REFLECTION_RNK_1, SPELL_CAST_OK, TRIGGERED_FULL_MASK);
                RemoveAllEquipedItems(warrior);
                TEST_HAS_NOT_AURA(warrior, ClassSpells::Warrior::SPELL_REFLECTION_RNK_1);
            });
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<SpellReflectionTestImpt>();
    }
};

class StanceMasteryTest : public TestCaseScript
{
public:
    StanceMasteryTest() : TestCaseScript("spells warrior stance_mastery") { }

    //"You retain up to 10 of your rage points when you change stances."
    class StanceMasteryTestImpt : public TestCaseWarrior
    {
    public:


        void TestStanceRage(TestPlayer* warrior, uint32 stanceSpellId)
        {
            warrior->SetFullPower(POWER_RAGE);
            SwitchTo(warrior, stanceSpellId);
            ASSERT_INFO("Stance %u, should have 100 rage. Rage: %u", stanceSpellId, warrior->GetPower(POWER_RAGE));
            TEST_ASSERT(warrior->GetPower(POWER_RAGE) == 100); // 10 rage
        }

        void Test() override
        {
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN);
            LearnTalent(warrior, ClassSpells::Warrior::STANCE_MASTERY_RNK_1); // Lvl 20

            TestStanceRage(warrior, BATTLE_STANCE);
            TestStanceRage(warrior, DEFENSIVE_STANCE);
            TestStanceRage(warrior, BERSERKER_STANCE);
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<StanceMasteryTestImpt>();
    }
};

class SunderArmorTest : public TestCaseScript
{
public:
    /*
    Threat:
        Omen: 301
        DTM: 300
        Lavina: 301
        Ciderhelm: 301
    Decided: 301
    */
    SunderArmorTest() : TestCaseScript("spells warrior sunder_armor") { }

    class SunderArmorTestImpt : public TestCaseWarrior
    {
    public:


        void TestSunderArmor(TestPlayer* warrior, Unit* victim, uint32 startArmor, int sunderArmorStack, uint32 armorReduced)
        {
            warrior->SetFullPower(POWER_RAGE);
            FORCE_CAST(warrior, victim, ClassSpells::Warrior::SUNDER_ARMOR_RNK_6, SPELL_MISS_NONE, TRIGGERED_IGNORE_GCD);

            uint32 const expectedArmor = startArmor - sunderArmorStack * armorReduced;
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
            Creature* dummy = SpawnCreature();

            uint32 startArmor = dummy->GetArmor();
            
            // Weapon required
            EQUIP_NEW_ITEM(warrior, APOLYON);

            uint32 const expectedSunderArmorRage = 15 * 10;
            TEST_POWER_COST(warrior, ClassSpells::Warrior::SUNDER_ARMOR_RNK_6, POWER_RAGE, expectedSunderArmorRage);

            // Threat
            warrior->SetPower(POWER_RAGE, expectedSunderArmorRage);
            FORCE_CAST(warrior, dummy, ClassSpells::Warrior::SUNDER_ARMOR_RNK_6);
            float const expectedThreat = 301 * WARRIOR_STANCE_THREAT_MOD;
            TEST_ASSERT(Between<float>(dummy->GetThreatManager().GetThreat(warrior), expectedThreat - 0.1f, expectedThreat + 0.1f));
            dummy->RemoveAllAuras();

            // Stack, aura duration, armor reduction
            TestSunderArmor(warrior, dummy, startArmor, 1, 520);
            TestSunderArmor(warrior, dummy, startArmor, 2, 520);
            TestSunderArmor(warrior, dummy, startArmor, 3, 520);
            TestSunderArmor(warrior, dummy, startArmor, 4, 520);
            TestSunderArmor(warrior, dummy, startArmor, 5, 520);
            TestSunderArmor(warrior, dummy, startArmor, 5, 520);
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<SunderArmorTestImpt>();
    }
};

class TauntTest : public TestCaseScript
{
public:

    TauntTest() : TestCaseScript("spells warrior taunt") { }

    class TauntTestImpt : public TestCaseWarrior
    {
    public:


        void Test() override
        {
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN);
            SwitchTo(warrior, DEFENSIVE_STANCE);
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_HUMAN);
            Creature* kobold = SpawnCreature(6, true);

            // Setup
            kobold->SetMaxHealth(10000);
            kobold->SetFullHealth();
            FORCE_CAST(warlock, kobold, ClassSpells::Warlock::SHADOW_BOLT_RNK_11, SPELL_MISS_NONE, TRIGGERED_FULL_DEBUG_MASK);
            float const warlockThreat = kobold->GetThreatManager().GetThreat(warlock);
            TEST_ASSERT(warlockThreat > 0);
            kobold->AI()->UpdateVictim();
            TEST_ASSERT(kobold->GetTarget() == warlock->GetGUID());

            // Acquire threat, aura duration, cooldown
            TEST_CAST(warrior, kobold, ClassSpells::Warrior::TAUNT_RNK_1);
            TEST_AURA_MAX_DURATION(kobold, ClassSpells::Warrior::TAUNT_RNK_1, Seconds(3));
            TEST_HAS_COOLDOWN(warrior, ClassSpells::Warrior::TAUNT_RNK_1, Seconds(10));
            TEST_ASSERT(kobold->GetThreatManager().GetThreat(warrior) == warlockThreat);
            kobold->AI()->UpdateVictim();
            TEST_ASSERT(kobold->GetTarget() == warrior->GetGUID());

            // Keep aggro
            FORCE_CAST(warlock, kobold, ClassSpells::Warlock::SHADOW_BOLT_RNK_11, SPELL_MISS_NONE, TRIGGERED_FULL_DEBUG_MASK);
            TEST_ASSERT(kobold->GetThreatManager().GetThreat(warlock) > kobold->GetThreatManager().GetThreat(warrior) * 1.1f);
            kobold->AI()->UpdateVictim();
            TEST_ASSERT(kobold->GetTarget() == warrior->GetGUID());

            // Lose aggro
            Wait(3000);
            kobold->AI()->UpdateVictim();
            TEST_ASSERT(kobold->GetTarget() == warlock->GetGUID());
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<TauntTestImpt>();
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