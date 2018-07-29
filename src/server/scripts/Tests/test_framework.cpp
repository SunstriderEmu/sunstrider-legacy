#include "TestCase.h"
#include "TestPlayer.h"
#include "TestMgr.h"

class DummyTest : public TestCaseScript
{
public:
    DummyTest() : TestCaseScript("testing framework dummy") { }

    class DummyTestImpl : public TestCase
    {
    public:
        DummyTestImpl() : TestCase(STATUS_PASSING, WorldLocation(209)) { }

        void Test() override
        {
            Map* m = GetMap();
            TEST_ASSERT(m != nullptr);

            //std::string str(m->GetMapName());
            //sWorld->SendGlobalText("DUMMY BEFORE WAIT");
            Creature* summon = SpawnCreature();
            Wait(Seconds(10));
            TestPlayer* p = SpawnRandomPlayer();
            Wait(Seconds(5));
            p->CastSpell(summon, 133, false);
            p->Say("YOLO!", LANG_UNIVERSAL);
            Wait(Seconds(3));
            summon->Say("lol");
            Wait(Seconds(2));

            //sWorld->SendGlobalText("DUMMY AFTER WAIT");
            //sWorld->SendGlobalText(str.c_str());
            //TEST_ASSERT(urand(0, 1) == 1);
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<DummyTestImpl>();
    }
};

class SpawnPlayersTest : public TestCaseScript
{
public:
    SpawnPlayersTest() : TestCaseScript("testing framework spawnplayers") { }

    class SpawnPlayersTestImplt : public TestCase
    {
    public:
        SpawnPlayersTestImplt() : TestCase(STATUS_PASSING, WorldLocation(209)) { /* EnableMapObjects();*/ }

        void Test() override
        {
            TestPlayer* p = SpawnRandomPlayer(POWER_ENERGY);
            Wait(Seconds(5));
            p->Say("ENERGY!", LANG_UNIVERSAL);

            _location.m_positionX += 5.0f;
            p = SpawnRandomPlayer(POWER_MANA);
            Wait(Seconds(5));
            p->Say("MANA!", LANG_UNIVERSAL);

            _location.m_positionX += 5.0f;
            p = SpawnRandomPlayer(POWER_RAGE);
            Wait(Seconds(5));
            p->Say("RAGE!", LANG_UNIVERSAL);

            _location.m_positionX += 5.0f;
            p = SpawnRandomPlayer(CLASS_MAGE);
            Wait(Seconds(5));
            p->Say("Mage!", LANG_UNIVERSAL);

            _location.m_positionX += 5.0f;
            p = SpawnRandomPlayer(CLASS_PRIEST);
            Wait(Seconds(5));
            p->Say("Priest!", LANG_UNIVERSAL);

            _location.m_positionX += 5.0f;
            p = SpawnRandomPlayer(RACE_DRAENEI);
            Wait(Seconds(5));
            p->Say("Draenei!", LANG_UNIVERSAL);

            _location.m_positionX += 5.0f;
            p = SpawnRandomPlayer(RACE_DWARF);
            Wait(Seconds(5));
            p->Say("Dwarf!", LANG_UNIVERSAL);
            RandomizePlayer(p);

            Wait(Seconds(1));
            p->HandleEmoteCommand(EMOTE_STATE_DANCE);

            Wait(Seconds(15));
            TEST_ASSERT(true);
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<SpawnPlayersTestImplt>();
    }
};

//use this one to slow world updates and simulate delay
class FakeSlowdownsTest : public TestCase
{
    void Test() override
    {
        for (uint32 i = 0; i < 1000; i++)
        {
           /* std::this_thread::sleep_for(10s);
            HandleThreadPause();*/
            /*if (sTestMgr->GetRemainingTestsCount() == 1)
                break;*/
        }
    }
};

void AddSC_test_dummy()
{
    new DummyTest();
    new SpawnPlayersTest();
    RegisterTestCase("testing framework slowdowns", FakeSlowdownsTest);
}
