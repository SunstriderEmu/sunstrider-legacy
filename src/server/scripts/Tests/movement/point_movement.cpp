#include "TestCase.h"
#include "TestPlayer.h"
#include "ObjectMgr.h"

//"movement point inform"
// Test PointMovementGenerator MovementInform capability
class PointMovementInformTest : public TestCase
{
    const uint32 POINT_ID = 1234;

    class TestAI : public CreatureAI
    {
    public:
        TestAI(Creature* c) : CreatureAI(c) {}

        // Called at waypoint reached or point movement finished
        virtual void MovementInform(uint32 MovementType, uint32 id) override 
        {
            receivedMovementType = MovementType;
            receivedId = id;
        }
        uint32 receivedMovementType = 0;
        uint32 receivedId = 0;
    };
public:
    Creature* c;
    void Test() override
    {
        c = SpawnCreature();
        TestAI* testAI = new TestAI(c);
        c->SetAI(testAI);
        Position targetPosition = c->GetPosition();
        targetPosition.MoveInFront(targetPosition, 10.0f);
        c->GetMotionMaster()->MovePoint(POINT_ID, targetPosition);
        //wait a bit for creature to get there
        Wait(3000);
        Wait(3000);
        TEST_ASSERT(testAI->receivedMovementType == POINT_MOTION_TYPE);
        TEST_ASSERT(testAI->receivedId == POINT_ID);
    }
    void Cleanup() override
    {
        //testAI will be destroyed automatically with creature in AIM_Destroy
        //if(c)
        //    c->AIM_Destroy();
    }
};

void AddSC_test_movement_point()
{
    RegisterTestCase("movement point inform", PointMovementInformTest);
}
