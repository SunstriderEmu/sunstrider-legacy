#include "TestCase.h"
#include "TestPlayer.h"
#include "World.h"

class DummyTest : public TestCaseScript
{
public:

    DummyTest() : TestCaseScript("testing dummy") { }

    class DummyTestImpl : public TestCase
    {
    public:
        DummyTestImpl() : TestCase(WorldLocation(209)) { }

        void Test() override
        {
            Map* m = GetMap();
            TEST_ASSERT(m != nullptr);
            //std::string str(m->GetMapName());
            //sWorld->SendGlobalText("DUMMY BEFORE WAIT");
            Creature* summon = SpawnCreature();
            Wait(10 * SECOND * IN_MILLISECONDS);
            TestPlayer* p = SpawnRandomPlayer();
            Wait(5 * SECOND * IN_MILLISECONDS);
            p->CastSpell(summon, 133);
            p->Say("YOLO!", LANG_UNIVERSAL);
            Wait(3 * SECOND * IN_MILLISECONDS);
            summon->Say("lol");
            Wait(30 * SECOND * IN_MILLISECONDS);
            //sWorld->SendGlobalText("DUMMY AFTER WAIT");
            //sWorld->SendGlobalText(str.c_str());
            //TEST_ASSERT(urand(0, 1) == 1);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<DummyTestImpl>();
    }
};

class SpawnPlayersTest : public TestCaseScript
{
public:

    SpawnPlayersTest() : TestCaseScript("testing spawnplayers") { }

    class SpawnPlayersTestImplt : public TestCase
    {
    public:
        SpawnPlayersTestImplt() : TestCase(WorldLocation(209)) { EnableMapObjects(); }

        void Test() override
        {
            TestPlayer* p = SpawnRandomPlayer(POWER_ENERGY);
            Wait(5 * SECOND * IN_MILLISECONDS);
            p->Say("ENERGY!", LANG_UNIVERSAL);

            _location.m_positionX += 5.0f;
            p = SpawnRandomPlayer(POWER_MANA);
            Wait(5 * SECOND * IN_MILLISECONDS);
            p->Say("MANA!", LANG_UNIVERSAL);

            _location.m_positionX += 5.0f;
            p = SpawnRandomPlayer(POWER_RAGE);
            Wait(5 * SECOND * IN_MILLISECONDS);
            p->Say("RAGE!", LANG_UNIVERSAL);

            _location.m_positionX += 5.0f;
            p = SpawnRandomPlayer(CLASS_MAGE);
            Wait(5 * SECOND * IN_MILLISECONDS);
            p->Say("Mage!", LANG_UNIVERSAL);

            _location.m_positionX += 5.0f;
            p = SpawnRandomPlayer(CLASS_PRIEST);
            Wait(5 * SECOND * IN_MILLISECONDS);
            p->Say("Priest!", LANG_UNIVERSAL);

            _location.m_positionX += 5.0f;
            p = SpawnRandomPlayer(RACE_DRAENEI);
            Wait(5 * SECOND * IN_MILLISECONDS);
            p->Say("Draenei!", LANG_UNIVERSAL);

            _location.m_positionX += 5.0f;
            p = SpawnRandomPlayer(RACE_DWARF);
            Wait(5 * SECOND * IN_MILLISECONDS);
            p->Say("Dwarf!", LANG_UNIVERSAL);
            RandomizePlayer(p);

            Wait(1 * SECOND * IN_MILLISECONDS);
            p->HandleEmoteCommand(EMOTE_STATE_DANCE);

            Wait(15 * SECOND * IN_MILLISECONDS);
            TEST_ASSERT(true);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<SpawnPlayersTestImplt>();
    }
};


void AddSC_test_dummy()
{
    new DummyTest();
    new SpawnPlayersTest();
}
