
#include "catch.hpp"
#include <vector>

#include "Player.h"
#include "Creature.h"
#include "test_utils.h"

void Touch_test_auras() {}

void TestStacksCount(Player* caster, Unit* target, uint32 talent, uint32 castSpell, uint32 testSpell, uint32 requireCount)
{
    caster->LearnSpell(castSpell, false);
    caster->LearnSpell(talent, false);

    uint32 castCount = 0;
    while(castCount < 50) {
        caster->CastSpell(target, castSpell, true);
        castCount++;
    }
    uint32 auraCount = target->GetAuraCount(testSpell);
    CHECK(auraCount == requireCount);
}

TEST_CASE( "Aura stacking limit", "[aura,stacking]" ) {
	
    // Setting up
    Player* playerBot = nullptr;
    Creature* targetCreature = nullptr;
    Testing::PrepareCasterAndTarget(playerBot, targetCreature);

    SECTION( "Improved Scorch talent ('Fire Vulnerability' aura)" ) {	
        uint32 TALENT_IMPROVED_SCORCH = 12873;
        uint32 SPELL_SCORCH = 10207;
        uint32 SPELL_FIRE_VULNERABILITY = 22959;

        TestStacksCount(playerBot, targetCreature, TALENT_IMPROVED_SCORCH, SPELL_SCORCH, SPELL_FIRE_VULNERABILITY, 5);
    }

    SECTION( "Shadow weaving talent" ) {
        uint32 TALENT_SHADOW_WEAVE = 15334;
        uint32 SPELL_MIND_BLAST = 25375;
        uint32 SPELL_SHADOW_WEAVE = 15258;

        TestStacksCount(playerBot, targetCreature, TALENT_SHADOW_WEAVE, SPELL_MIND_BLAST, SPELL_SHADOW_WEAVE, 5);
    }

    Testing::PrepareCasterAndTarget_cleanup(playerBot, targetCreature);
};

