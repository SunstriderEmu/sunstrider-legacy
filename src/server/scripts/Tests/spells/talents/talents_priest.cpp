#include "TestCase.h"
#include "TestPlayer.h"
#include "World.h"
#include "ClassSpells.h"

class WandSpecializationTest : public TestCaseScript
{
public:

    WandSpecializationTest() : TestCaseScript("talents priest wandspecialization") { }

    class WandSpecializationTestImpt : public TestCase
    {
    public:
        WandSpecializationTestImpt() : TestCase(true) { }

        void Test() override
        {
            TestPlayer* player = SpawnRandomPlayer(CLASS_PRIEST);
            EquipItem(player, 28783); //Eredar Wand of Obliteration, 177 - 330 Shadow Damage
            uint32 const wandMinDamage = 177;
            uint32 const wandMaxDamage = 330;

            Creature* dummyTarget = SpawnCreature();
            //Test regular damage
            TestSpellDamage(player, dummyTarget, ClassSpells::Priest::WAND, wandMinDamage, wandMaxDamage);

            //Test improved damage 5%
            LearnTalent(player, Talents::Priest::WAND_SPEC_RNK_1);
            TestSpellDamage(player, dummyTarget, ClassSpells::Priest::WAND, wandMinDamage * 1.05f, wandMaxDamage * 1.05f);

            //Test improved damage 25%
            LearnTalent(player, Talents::Priest::WAND_SPEC_RNK_5);
            TestSpellDamage(player, dummyTarget, ClassSpells::Priest::WAND, wandMinDamage * 1.25f, wandMaxDamage * 1.25f);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<WandSpecializationTestImpt>();
    }
};

void AddSC_test_talents_priest()
{
    new WandSpecializationTest();
}
