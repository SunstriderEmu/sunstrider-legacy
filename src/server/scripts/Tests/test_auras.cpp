#include "TestCase.h"

/*

#include "catch.hpp"
#include <vector>

#include "Player.h"
#include "Creature.h"
#include "test_utils.h"
#include "test_case.h"

using namespace Testing;

void Touch_test_auras() {}

TEST_CASE( "Aura stacking limit", "[aura,stacking]" ) 
{
    // Setting up
    TestCase testCase;
    Player* playerBot = testCase.SpawnRandomPlayer();
    Creature* targetCreature = testCase.SpawnCreature();

    SECTION( "Improved Scorch talent ('Fire Vulnerability' aura)" ) 
    {    
        constexpr uint32 TALENT_IMPROVED_SCORCH = 12873;
        constexpr uint32 SPELL_SCORCH = 10207;
        constexpr uint32 SPELL_FIRE_VULNERABILITY = 22959;

        testCase.TestStacksCount(playerBot, targetCreature, TALENT_IMPROVED_SCORCH, SPELL_SCORCH, SPELL_FIRE_VULNERABILITY, 5);
    }

    SECTION( "Shadow weaving talent" ) 
    {
        constexpr uint32 TALENT_SHADOW_WEAVE = 15334;
        constexpr uint32 SPELL_MIND_BLAST = 25375;
        constexpr uint32 SPELL_SHADOW_WEAVE = 15258;

        testCase.TestStacksCount(playerBot, targetCreature, TALENT_SHADOW_WEAVE, SPELL_MIND_BLAST, SPELL_SHADOW_WEAVE, 5);
    }
};

TEST_CASE("Aura duration", "[aura]") 
{
    TestCase testCase;
    Player* caster = testCase.SpawnRandomPlayer();;
    Player* targetPlayer = testCase.SpawnRandomPlayer();
    Creature* targetCreature = testCase.SpawnCreature();

    constexpr uint32 SPELL_POLYMORPH = 118;

    SECTION("Sheep duration against players") 
    {
        caster->CastSpell(targetPlayer, 118, true);
        AuraApplication* app = targetPlayer->GetAuraApplication(118);
        CHECK(app->GetAuraDuration() == 10 * SECOND * IN_MILLISECONDS);
    }

    SECTION("Sheep duration against creatures")
    {
        caster->CastSpell(targetCreature, 118, true);
        AuraApplication* app = targetPlayer->GetAuraApplication(118);
        CHECK(app->GetAuraDuration() == 20 * SECOND * IN_MILLISECONDS);
    }
};
*/

/*
class AuraStackLimitTest : public TestCaseScript
{
public:

    AuraStackLimitTest() : TestCaseScript("Aura stacking") { }

    class AuraStackLimitTestImpl : public TestCase
    {
    public: 
        AuraStackLimitTestImpl() : TestCase(false) {}
        
        void Test() override
        {
            TEST_ASSERT(1 == 1);
            Wait(5 * SECOND * IN_MILLISECONDS);
            //TEST_ASSERT(0 == 1);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<AuraStackLimitTestImpl>();
    }
};
*/

void AddSC_test_auras_stacking()
{
  //  new AuraStackLimitTest();
}
