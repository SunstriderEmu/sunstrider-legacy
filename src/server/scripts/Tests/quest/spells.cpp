#include "TestCase.h"
#include "TestPlayer.h"
#include "World.h"

/* "quest spells sendeventchannel"
Testing spell 40129.
That spell used to fail because of no target was found for first effect. This only happened if spell was a channel.
*/
class SpellSendEventChannelTest : public TestCase
{
public:
    SpellSendEventChannelTest() : TestCase(WorldLocation(530, -2482.84, 4661.56, 161.6)) //summon position for event 14856 
    { }

    void Test() override
    {
        uint32 const SPELL_SUMMON_AIR_ELEMENTAL = 40129;
        Creature* c = SpawnCreature(23093); //Sar'this, npc from the quest
        c->CastSpell(c, SPELL_SUMMON_AIR_ELEMENTAL);
        Wait(5000);
        c->HasAura(SPELL_SUMMON_AIR_ELEMENTAL);
        //also test if event is triggered?
        //spell trigger event 14856 which should do SCRIPT_COMMAND_TEMP_SUMMON_CREATURE
        Creature* elemental = c->FindNearestCreature(23096, 100.0f);
        TEST_ASSERT(elemental != nullptr);
    }
};

void AddSC_test_quest_spells()
{
    RegisterTestCase("quest spells sendeventchannel", SpellSendEventChannelTest);
}
