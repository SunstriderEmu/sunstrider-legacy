#include "TestCase.h"
#include "TestPlayer.h"
#include "World.h"

class EnvironmentalTrapTest : public TestCaseScript
{
public:

    EnvironmentalTrapTest() : TestCaseScript("environmental traps") { }

    class EnvironmentalTrapImpl : public TestCase
    {
    public:
        EnvironmentalTrapImpl() : TestCase(STATUS_PASSING, true) {  }

        void Test() override
        {
            TestPlayer* p = SpawnRandomPlayer(POWER_ENERGY);
            Wait(1 * SECOND * IN_MILLISECONDS);
            GameObject* obj = p->SummonGameObject(2061, p->GetPosition(), G3D::Quat(), 0); //campire
            TEST_ASSERT(obj != nullptr);
            obj->SetOwnerGUID(0); //remove owner, environmental traps don't have any

            //just test if player has taken any damage
            uint32 initialHealth = p->GetHealth();
            Wait(10 * SECOND * IN_MILLISECONDS);
            uint32 newHealth = p->GetHealth();
            TEST_ASSERT(newHealth < initialHealth);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<EnvironmentalTrapImpl>();
    }
};

void AddSC_test_spells_misc()
{
    new EnvironmentalTrapTest();
}
