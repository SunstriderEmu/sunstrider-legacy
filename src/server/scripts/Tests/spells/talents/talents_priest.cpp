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

class SilentResolve : public TestCaseScript
{
public:
    SilentResolve() : TestCaseScript("talents priest silent_resolve") { }

    class WandSpecializationTestImpt : public TestCase
    {
    public:
        WandSpecializationTestImpt() : TestCase(STATUS_WIP) { }

        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_HUMAN);
            TestPlayer* enemy = SpawnPlayer(CLASS_PRIEST, RACE_TROLL);
            Creature* dummy = SpawnCreature(18); // Humanoid

            LearnTalent(priest, Talents::Priest::SILENT_RESOLVE_RNK_5);
            float const threatReductionFactor = 1.0f - 0.2f;
            float const healThreatFactor = 0.5f * threatReductionFactor;

            // Threat of Discipline and Holy
            // TODO: Power Word: Shield
            //TEST_DIRECT_HEAL_THREAT(priest, dummy, ClassSpells::Priest::BINDING_HEAL_RNK_1, 0.5f * healThreatFactor);
            TEST_DOT_THREAT(priest, dummy, ClassSpells::Priest::STARSHARDS_RNK_8, threatReductionFactor, false);
            //TEST_DIRECT_SPELL_THREAT(priest, dummy, ClassSpells::Priest::CHASTISE_RNK_6, threatFactor);
            TEST_DIRECT_HEAL_THREAT(priest, dummy, ClassSpells::Priest::CIRCLE_OF_HEALING_RNK_5, healThreatFactor);
            TEST_DIRECT_HEAL_THREAT(priest, dummy, ClassSpells::Priest::DESPERATE_PRAYER_RNK_8, healThreatFactor);
            TEST_DIRECT_HEAL_THREAT(priest, dummy, ClassSpells::Priest::FLASH_HEAL_RNK_9, healThreatFactor);
            TEST_DIRECT_HEAL_THREAT(priest, dummy, ClassSpells::Priest::GREATER_HEAL_RNK_7, healThreatFactor);
            TEST_DIRECT_HEAL_THREAT(priest, dummy, ClassSpells::Priest::HEAL_RNK_4, healThreatFactor);
            TEST_DOT_THREAT(priest, dummy, ClassSpells::Priest::HOLY_FIRE_RNK_9, threatReductionFactor, true);
            TEST_DIRECT_HEAL_THREAT(priest, dummy, ClassSpells::Priest::LESSER_HEAL_RNK_3, healThreatFactor);
            TEST_DIRECT_HEAL_THREAT(priest, dummy, ClassSpells::Priest::PRAYER_OF_HEALING_RNK_6, healThreatFactor);
            //TEST_DIRECT_HEAL_THREAT(priest, dummy, ClassSpells::Priest::PRAYER_OF_MENDING_RNK_1, healThreatFactor);
            TEST_HOT_THREAT(priest, dummy, ClassSpells::Priest::RENEW_RNK_12, healThreatFactor, false);
            TEST_DIRECT_SPELL_THREAT(priest, dummy, ClassSpells::Priest::SMITE_RNK_10, threatReductionFactor);

            // TODO: Resist dispell of all spells
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<WandSpecializationTestImpt>();
    }
};

class ImprovedPowerWordFortitudeTest : public TestCaseScript
{
public:

    ImprovedPowerWordFortitudeTest() : TestCaseScript("talents priest improved_power_word_fortitude") { }

    class ImprovedPowerWordFortitudeTestImpt : public TestCase
    {
    public:
        ImprovedPowerWordFortitudeTestImpt() : TestCase(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_ORC);

            LearnTalent(priest, Talents::Priest::IMPROVED_POWER_WORD_FORTITUDE_RNK_2);
            float const talentFactor = 1.3f;

            uint32 powerWordFortitudeBonus = 79 * talentFactor;
            uint32 expectedMaxHealth = warrior->GetMaxHealth() + powerWordFortitudeBonus * 10;

            // Power Word: Fortitude
            TEST_CAST(priest, warrior, ClassSpells::Priest::POWER_WORD_FORTITUDE_RNK_7);
            TEST_ASSERT(warrior->GetMaxHealth() == expectedMaxHealth);

            // Prayer of Fortitude
            GroupPlayer(priest, warrior);
            TEST_CAST(priest, warrior, ClassSpells::Priest::PRAYER_OF_FORTITUDE_RNK_3, SPELL_CAST_OK, TRIGGERED_FULL_MASK);
            TEST_ASSERT(warrior->GetMaxHealth() == expectedMaxHealth);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<ImprovedPowerWordFortitudeTestImpt>();
    }
};

class ImprovedPowerWordShieldTest : public TestCaseScript
{
public:

    ImprovedPowerWordShieldTest() : TestCaseScript("talents priest improved_power_word_shield") { }

    class ImprovedPowerWordShieldTestImpt : public TestCase
    {
    public:
        ImprovedPowerWordShieldTestImpt() : TestCase(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);
            TestPlayer* rogue = SpawnPlayer(CLASS_ROGUE, RACE_ORC);

            LearnTalent(priest, Talents::Priest::IMPROVED_POWER_WORD_SHIELD_RNK_3);
            float const talentFactor = 1.15f;

            int32 expectedAbsorb = ClassSpellsDamage::Priest::POWER_WORD_SHIELD_RNK_12 * talentFactor;
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
            rogue->ForceMeleeHitResult(MELEE_HIT_NORMAL);
            int32 totalDamage = 0;
            for (uint32 i = 0; i < 150; i++)
            {
                rogue->AttackerStateUpdate(priest, BASE_ATTACK);
                auto damageToTarget = AI->GetMeleeDamageDoneInfo(priest);

                TEST_ASSERT(damageToTarget->size() == i + 1);
                auto& data = damageToTarget->back();

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
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<ImprovedPowerWordShieldTestImpt>();
    }
};

class MartyrdomTest : public TestCaseScript
{
public:

    MartyrdomTest() : TestCaseScript("talents priest martyrdom") { }

    class MartyrdomTestImpt : public TestCase
    {
    public:
        MartyrdomTestImpt() : TestCase(STATUS_WIP) { }

        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);
            TestPlayer* shaman = SpawnPlayer(CLASS_SHAMAN, RACE_ORC);

            LearnTalent(priest, Talents::Priest::MARTYRDOM_RNK_2);
            uint32 const focusedCasting = 27828;
            float const talentFactor = 1.15f;

            shaman->ForceMeleeHitResult(MELEE_HIT_CRIT);
            shaman->AttackerStateUpdate(priest, BASE_ATTACK);
            shaman->AttackStop();
            TEST_AURA_MAX_DURATION(priest, focusedCasting, Seconds(6));
            shaman->ResetForceMeleeHitResult();

            // No pushback
            TEST_PUSHBACK_RESIST_CHANCE(priest, shaman, ClassSpells::Priest::GREATER_HEAL_RNK_7, 100.f);

            // 20% resist to interrupt
            TEST_SPELL_HIT_CHANCE_CALLBACK(shaman, priest, ClassSpells::Shaman::EARTH_SHOCK_RNK_8, 23.f, SPELL_MISS_RESIST, [focusedCasting](Unit* caster, Unit* target) {
                target->AddAura(focusedCasting, target);
                target->CastSpell(target, ClassSpells::Priest::GREATER_HEAL_RNK_7, TRIGGERED_IGNORE_POWER_AND_REAGENT_COST);
            });
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<MartyrdomTestImpt>();
    }
};

class AbsolutionTest : public TestCaseScript
{
public:

    AbsolutionTest() : TestCaseScript("talents priest absolution") { }

    class AbsolutionTestImpt : public TestCase
    {
    public:
        AbsolutionTestImpt() : TestCase(STATUS_WIP) { }

        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);

            LearnTalent(priest, Talents::Priest::ABSOLUTION_RNK_3);
            float const talentFactor = 1 - 0.15f;

            TEST_POWER_COST(priest, ClassSpells::Priest::DISPEL_MAGIC_RNK_2, POWER_MANA, 366 * talentFactor);
            TEST_POWER_COST(priest, ClassSpells::Priest::CURE_DISEASE_RNK_1, POWER_MANA, 314 * talentFactor);
            TEST_POWER_COST(priest, ClassSpells::Priest::ABOLISH_DISEASE_RNK_1, POWER_MANA, 314 * talentFactor);
            uint32 const baseMana = priest->GetMaxPower(POWER_MANA) - priest->GetManaBonusFromIntellect();
            float const massDispelMana = baseMana * 0.33f;
            TEST_POWER_COST(priest, ClassSpells::Priest::MASS_DISPEL_RNK_1, POWER_MANA, massDispelMana * talentFactor);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<AbsolutionTestImpt>();
    }
};

class InnerFocusTest : public TestCaseScript
{
public:

    InnerFocusTest() : TestCaseScript("talents priest inner_focus") { }

    class InnerFocusTestImpt : public TestCase
    {
    public:
        InnerFocusTestImpt() : TestCase(STATUS_KNOWN_BUG) { } // Crit chance fails

        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);
            Creature* dummy = SpawnCreature();
            float const talentFactor = 25.f;
            float const expectedSpellCritChance = priest->GetFloatValue(PLAYER_CRIT_PERCENTAGE) + talentFactor;

            // CD
            TEST_CAST(priest, priest, ClassSpells::Priest::INNER_FOCUS_RNK_1);
            TEST_HAS_AURA(priest, ClassSpells::Priest::INNER_FOCUS_RNK_1);
            TEST_POWER_COST(priest, ClassSpells::Priest::SHADOW_WORD_PAIN_RNK_10, POWER_MANA, uint32(0));
            TEST_CAST(priest, dummy, ClassSpells::Priest::SHADOW_WORD_PAIN_RNK_10);
            TEST_HAS_COOLDOWN(priest, ClassSpells::Priest::INNER_FOCUS_RNK_1, Minutes(3));

            // Crit chance
            TEST_SPELL_CRIT_CHANCE_CALLBACK(priest, dummy, ClassSpells::Priest::SMITE_RNK_10, expectedSpellCritChance, [](Unit* caster, Unit* target) {
                caster->AddAura(ClassSpells::Priest::INNER_FOCUS_RNK_1, caster);
            });
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<InnerFocusTestImpt>();
    }
};

class MeditationTest : public TestCaseScript
{
public:

    MeditationTest() : TestCaseScript("talents priest meditation") { }

    class MeditationTestImpt : public TestCase
    {
    public:
        MeditationTestImpt() : TestCase(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);

            float const talentFactor = 0.3f;
            float const expectedManaRegen = priest->GetFloatValue(PLAYER_FIELD_MOD_MANA_REGEN) * talentFactor;

            LearnTalent(priest, Talents::Priest::MEDITATION_RNK_3);
            ASSERT_INFO("Priest should have %f mana regen but has %f.", expectedManaRegen, priest->GetFloatValue(PLAYER_FIELD_MOD_MANA_REGEN_INTERRUPT));
            TEST_ASSERT(Between<float>(priest->GetFloatValue(PLAYER_FIELD_MOD_MANA_REGEN_INTERRUPT), expectedManaRegen - 0.1f, expectedManaRegen + 0.1));
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<MeditationTestImpt>();
    }
};

class ImprovedInnerFireTest : public TestCaseScript
{
public:

    ImprovedInnerFireTest() : TestCaseScript("talents priest improved_inner_fire") { }

    class ImprovedInnerFireTestImpt : public TestCase
    {
    public:
        ImprovedInnerFireTestImpt() : TestCase(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);

            float const talentFactor = 1.3f;
            uint32 const innerFireBonus = 1580 * talentFactor;
            uint32 const expectedArmor = priest->GetArmor() + innerFireBonus;

            LearnTalent(priest, Talents::Priest::IMPROVED_INNER_FIRE_RNK_3);
            TEST_CAST(priest, priest, ClassSpells::Priest::INNER_FIRE_RNK_7);
            ASSERT_INFO("Priest should have %f armor but has %f.", expectedArmor, priest->GetArmor());
            TEST_ASSERT(priest->GetArmor() == expectedArmor);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<ImprovedInnerFireTestImpt>();
    }
};

class MentalAgilityTest : public TestCaseScript
{
public:

    MentalAgilityTest() : TestCaseScript("talents priest mental_agility") { }

    class MentalAgilityTestImpt : public TestCase
    {
    public:
        MentalAgilityTestImpt() : TestCase(STATUS_WIP) { }

        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);

            float const talentFactor = 1 - 0.1f;
            uint32 const baseMana = priest->GetMaxPower(POWER_MANA) - priest->GetManaBonusFromIntellect();

            LearnTalent(priest, Talents::Priest::MENTAL_STRENGTH_RNK_5);
            WaitNextUpdate();
            // Discipline
            TEST_POWER_COST(priest, ClassSpells::Priest::DISPEL_MAGIC_RNK_2, POWER_MANA, baseMana * 0.14f * talentFactor);
            TEST_POWER_COST(priest, ClassSpells::Priest::DIVINE_SPIRIT_RNK_5, POWER_MANA, 680 * talentFactor);
            TEST_POWER_COST(priest, ClassSpells::Priest::ELUNES_GRACE_RNK_1, POWER_MANA, baseMana * 0.03f * talentFactor);
            TEST_POWER_COST(priest, ClassSpells::Priest::FEAR_WARD_RNK_1, POWER_MANA, baseMana * 0.03f * talentFactor);
            TEST_POWER_COST(priest, ClassSpells::Priest::FEEDBACK_RNK_6, POWER_MANA, 705 * talentFactor);
            TEST_POWER_COST(priest, ClassSpells::Priest::INNER_FIRE_RNK_7, POWER_MANA, 375 * talentFactor);
            TEST_POWER_COST(priest, ClassSpells::Priest::LEVITATE_RNK_1, POWER_MANA, 100 * talentFactor);
            TEST_POWER_COST(priest, ClassSpells::Priest::MANA_BURN_RNK_7, POWER_MANA, 355 * talentFactor);
            TEST_POWER_COST(priest, ClassSpells::Priest::MASS_DISPEL_RNK_1, POWER_MANA, baseMana * 0.33f * talentFactor);
            TEST_POWER_COST(priest, ClassSpells::Priest::POWER_INFUSION_RNK_1, POWER_MANA, baseMana * 0.16f * talentFactor);
            TEST_POWER_COST(priest, ClassSpells::Priest::POWER_WORD_FORTITUDE_RNK_7, POWER_MANA, 700 * talentFactor);
            TEST_POWER_COST(priest, ClassSpells::Priest::POWER_WORD_SHIELD_RNK_12, POWER_MANA, 600 * talentFactor);
            TEST_POWER_COST(priest, ClassSpells::Priest::PRAYER_OF_FORTITUDE_RNK_3, POWER_MANA, 1800 * talentFactor);
            TEST_POWER_COST(priest, ClassSpells::Priest::PRAYER_OF_SPIRIT_RNK_2, POWER_MANA, 1800 * talentFactor);
            TEST_POWER_COST(priest, ClassSpells::Priest::SYMBOL_OF_HOPE_RNK_1, POWER_MANA, 15 * talentFactor);
            // Holy
            TEST_POWER_COST(priest, ClassSpells::Priest::ABOLISH_DISEASE_RNK_1, POWER_MANA, baseMana * 0.12f * talentFactor);
            TEST_POWER_COST(priest, ClassSpells::Priest::CHASTISE_RNK_6, POWER_MANA, 300 * talentFactor);
            TEST_POWER_COST(priest, ClassSpells::Priest::CIRCLE_OF_HEALING_RNK_5, POWER_MANA, 450 * talentFactor);
            TEST_POWER_COST(priest, ClassSpells::Priest::CURE_DISEASE_RNK_1, POWER_MANA, baseMana * 0.12f * talentFactor);
            TEST_POWER_COST(priest, ClassSpells::Priest::HOLY_NOVA_RNK_7, POWER_MANA, 875 * talentFactor);
            TEST_POWER_COST(priest, ClassSpells::Priest::PRAYER_OF_MENDING_RNK_1, POWER_MANA, 390 * talentFactor);
            TEST_POWER_COST(priest, ClassSpells::Priest::RENEW_RNK_12, POWER_MANA, 450 * talentFactor);
            // Shadow
            TEST_POWER_COST(priest, ClassSpells::Priest::DEVOURING_PLAGUE_RNK_7, POWER_MANA, 1145 * talentFactor);
            TEST_POWER_COST(priest, ClassSpells::Priest::FADE_RNK_7, POWER_MANA, 330 * talentFactor);
            TEST_POWER_COST(priest, ClassSpells::Priest::HEX_OF_WEAKNESS_RNK_7, POWER_MANA, 295 * talentFactor);
            TEST_POWER_COST(priest, ClassSpells::Priest::MIND_SOOTHE_RNK_4, POWER_MANA, 120 * talentFactor);
            TEST_POWER_COST(priest, ClassSpells::Priest::PRAYER_OF_SHADOW_PROTECTION_RNK_2, POWER_MANA, 1620 * talentFactor);
            TEST_POWER_COST(priest, ClassSpells::Priest::PSYCHIC_SCREAM_RNK_4, POWER_MANA, 210 * talentFactor);
            TEST_POWER_COST(priest, ClassSpells::Priest::SHADOW_PROTECTION_RNK_4, POWER_MANA, 810 * talentFactor);
            TEST_POWER_COST(priest, ClassSpells::Priest::SHADOW_WORD_DEATH_RNK_2, POWER_MANA, 309 * talentFactor);
            TEST_POWER_COST(priest, ClassSpells::Priest::SHADOW_WORD_PAIN_RNK_10, POWER_MANA, 575 * talentFactor);
            TEST_POWER_COST(priest, ClassSpells::Priest::SHADOWFIEND_RNK_1, POWER_MANA, baseMana * 0.06f * talentFactor);
            TEST_POWER_COST(priest, ClassSpells::Priest::SHADOW_GUARD_RNK_7, POWER_MANA, 270 * talentFactor);
            TEST_POWER_COST(priest, ClassSpells::Priest::TOUCH_OF_WEAKNESS_RNK_7, POWER_MANA, 235 * talentFactor);
            TEST_POWER_COST(priest, ClassSpells::Priest::VAMPIRIC_EMBRACE_RNK_1, POWER_MANA, baseMana * 0.02f * talentFactor);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<MentalAgilityTestImpt>();
    }
};

class ImprovedManaBurnTest : public TestCaseScript
{
public:

    ImprovedManaBurnTest() : TestCaseScript("talents priest improved_mana_burn") { }

    class ImprovedManaBurnTestImpt : public TestCase
    {
    public:
        ImprovedManaBurnTestImpt() : TestCase(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);

            LearnTalent(priest, Talents::Priest::IMPROVED_MANA_BURN_RNK_2);
            TEST_SPELL_CAST_TIME(priest, ClassSpells::Priest::MANA_BURN_RNK_7, 2000);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<ImprovedManaBurnTestImpt>();
    }
};

class MentalStrengthTest : public TestCaseScript
{
public:

    MentalStrengthTest() : TestCaseScript("talents priest mental_strength") { }

    class MentalStrengthTestImpt : public TestCase
    {
    public:
        MentalStrengthTestImpt() : TestCase(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);
            float const talentFactor = 1.1f;

            uint32 expectedMaxMana = priest->GetMaxPower(POWER_MANA) * talentFactor;

            LearnTalent(priest, Talents::Priest::MENTAL_STRENGTH_RNK_5);
            ASSERT_INFO("Priest has %u max MP, should have %u.", priest->GetMaxPower(POWER_MANA), expectedMaxMana);
            TEST_ASSERT(Between<uint32>(priest->GetMaxPower(POWER_MANA), expectedMaxMana - 1, expectedMaxMana + 1));
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<MentalStrengthTestImpt>();
    }
};

class DivineSpiritTest : public TestCaseScript
{
public:

    DivineSpiritTest() : TestCaseScript("talents priest divine_spirit") { }

    class DivineSpiritTestImpt : public TestCase
    {
    public:
        DivineSpiritTestImpt() : TestCase(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);

            uint32 expectedSpirit = priest->GetStat(STAT_SPIRIT) + 50;

            TEST_CAST(priest, priest, ClassSpells::Priest::DIVINE_SPIRIT_RNK_5);
            TEST_ASSERT(priest->GetStat(STAT_SPIRIT) == expectedSpirit);

            TEST_AURA_MAX_DURATION(priest, ClassSpells::Priest::DIVINE_SPIRIT_RNK_5, Minutes(30));

            TEST_POWER_COST(priest, ClassSpells::Priest::DIVINE_SPIRIT_RNK_5, POWER_MANA, 680);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<DivineSpiritTestImpt>();
    }
};

class ImprovedDivineSpiritTest : public TestCaseScript
{
public:

    ImprovedDivineSpiritTest() : TestCaseScript("talents priest improved_divine_spirit") { }

    class ImprovedDivineSpiritTestImpt : public TestCase
    {
    public:
        ImprovedDivineSpiritTestImpt() : TestCase(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);

            float const spirit = priest->GetStat(STAT_SPIRIT) + 50;

            EQUIP_NEW_ITEM(priest, 34335); // Hammer of Sanctification -- 183 SP & 550 BH
            uint32 const bonusHeal = priest->GetInt32Value(PLAYER_FIELD_MOD_HEALING_DONE_POS);
            TEST_ASSERT(bonusHeal == 550);
            uint32 const spellPower = priest->GetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW);
            TEST_ASSERT(spellPower == 183);

            float const talentFactor = 0.1f;
            uint32 expectedBH = bonusHeal + spirit * talentFactor;
            uint32 expectedSP = spellPower + spirit * talentFactor;

            LearnTalent(priest, Talents::Priest::IMPROVED_DIVINE_SPIRIT_RNK_2);
            TEST_CAST(priest, priest, ClassSpells::Priest::DIVINE_SPIRIT_RNK_5);
            TEST_ASSERT(priest->GetInt32Value(PLAYER_FIELD_MOD_HEALING_DONE_POS) == int32(expectedBH));
            TEST_ASSERT(priest->GetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW) == int32(expectedSP));

            TEST_CAST(priest, priest, ClassSpells::Priest::PRAYER_OF_SPIRIT_RNK_2, SPELL_CAST_OK, TRIGGERED_FULL_MASK);
            TEST_ASSERT(priest->GetInt32Value(PLAYER_FIELD_MOD_HEALING_DONE_POS) == int32(expectedBH));
            TEST_ASSERT(priest->GetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW) == int32(expectedSP));
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<ImprovedDivineSpiritTestImpt>();
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
    // Discipline
    // TODO: Unbreakable Will
	new PriestWandSpecializationTest();
    new SilentResolve();
    new ImprovedPowerWordFortitudeTest();
    new ImprovedPowerWordShieldTest();
    new MartyrdomTest();
    new AbsolutionTest();
    new InnerFocusTest();
    new MeditationTest();
    new ImprovedInnerFireTest();
    new MentalAgilityTest();
    new ImprovedManaBurnTest();
    new MentalStrengthTest();
    new DivineSpiritTest();
    new ImprovedDivineSpiritTest();
    // Holy
	new SearingLightTest();
    // Shadow
}
