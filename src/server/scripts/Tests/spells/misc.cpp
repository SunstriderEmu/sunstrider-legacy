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
            obj->SetOwnerGUID(ObjectGuid::Empty); //remove owner, environmental traps don't have any

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

class StackingRulesTest : public TestCaseScript
{
public:

    StackingRulesTest() : TestCaseScript("spells rules stacking") { }

    class StackingTestImpl : public TestCase
    {
    public:
        StackingTestImpl() : TestCase(STATUS_PARTIAL, true) {  }

        TestPlayer* p1;
        TestPlayer* p2;
        Creature* target;

        void TestStack(uint32 id1, uint32 id2 = 0, bool _not = false, bool sameCaster = false)
        {
            if (!id2)
                id2 = id1;

            Unit* caster1 = p1;
            Unit* caster2 = sameCaster ? p1 : p2;

            target->RemoveAllAuras();
            Aura* a;
            a = caster1->AddAura(id1, target);
            ASSERT_INFO("Failed to add aura %u", id1);
            TEST_ASSERT(a != nullptr);
            a = caster2->AddAura(id2, target);
            ASSERT_INFO("Failed to add aura %u", id2);
            TEST_ASSERT(a != nullptr);

            if (id1 == id2)
            {
                uint32 auraCount = target->GetAuraCount(id1);
                if (_not)
                {
                    ASSERT_INFO("Testing if %u NOT stacks with %u. Aura %u has wrong count %u (should be 1)", id1, id2, id1, auraCount);
                    TEST_ASSERT(auraCount == 1)
                } 
                else 
                {
                    ASSERT_INFO("Testing if %u stacks with %u. Aura %u has wrong count %u (should be 2)", id1, id2, id1, auraCount);
                    TEST_ASSERT(target->GetAuraCount(id1) == 2)
                }
            }
            else 
            {
                bool hasAura1 = target->HasAura(id1);
                bool hasAura2 = target->HasAura(id2);
                if (_not)
                {
                    ASSERT_INFO("Testing if %u NOT stacks with %u. Target should not have aura %u", id1, id2, id1);
                    TEST_ASSERT(!hasAura1);
                    ASSERT_INFO("Testing if %u NOT stacks with %u. Target should have aura %u", id1, id2, id2);
                    TEST_ASSERT(hasAura2);
                }
                else
                {
                    ASSERT_INFO("Testing if %u stacks with %u. Target should have aura %u", id1, id2, id1);
                    TEST_ASSERT(hasAura1);
                    ASSERT_INFO("Testing if %u stacks with %u. Target should have aura %u", id1, id2, id2);
                    TEST_ASSERT(hasAura2);
                }
            }
        }

        void TestNotStack(uint32 id1, uint32 id2 = 0, bool sameCaster = false)
        {
            TestStack(id1, id2, true, sameCaster);
        }

        void Test() override
        {
            p1 = SpawnRandomPlayer();
            p2 = SpawnRandomPlayer();
            target = SpawnCreature();
            
            //well fed buffs
            uint32 const herbBakedEgg = 19705;
            uint32 const windblossomBerries = 18191;
            uint32 const rumseyRum = 25804;
            uint32 const blackenedBasilick = 33263;
            uint32 const fishermanFeast = 33257;
            TestNotStack(herbBakedEgg, windblossomBerries);
            TestNotStack(windblossomBerries, rumseyRum);
            TestNotStack(rumseyRum, blackenedBasilick);
            TestNotStack(blackenedBasilick, fishermanFeast);
            TestNotStack(fishermanFeast, herbBakedEgg);

            //endurance group
            uint32 const scrollOfStamina = 8099;
            uint32 const PWFortitude = 1243;
            uint32 const prayerFortitude = 21562;
            TestNotStack(scrollOfStamina, PWFortitude);
            TestNotStack(PWFortitude, prayerFortitude);
            TestNotStack(prayerFortitude, scrollOfStamina);

            //spirit group
            uint32 const prayerSpirit = 27681;
            uint32 const divineSpirit = 14752;
            uint32 const scrollSpirit = 8112;
            TestNotStack(prayerSpirit, divineSpirit);
            TestNotStack(divineSpirit, scrollSpirit);
            TestNotStack(scrollSpirit, prayerSpirit);

            //intellect group
            uint32 const arcaneBrilliance = 23028;
            uint32 const arcaneIntellect = 1459;
            uint32 const scrollIntellect = 8096;
            TestNotStack(arcaneBrilliance, arcaneIntellect);
            TestNotStack(arcaneIntellect, scrollIntellect);
            TestNotStack(scrollIntellect, arcaneBrilliance);

            //blessings
            uint32 const wisdomBlessing = 19742;
            uint32 const greaterWisdom = 25894;
            uint32 const mightBlessing = 19740;
            uint32 const greaterMight = 25782;
            uint32 const kingsBlessing = 20217;
            uint32 const greaterKings = 25898;
            uint32 const lightBlessing = 19977;
            uint32 const greaterLight = 25890;
            // test blessing vs greater
            TestNotStack(wisdomBlessing, greaterWisdom);
            TestNotStack(mightBlessing, greaterMight);
            TestNotStack(kingsBlessing, greaterKings);
            TestNotStack(lightBlessing, greaterLight);
            // test between different blessings when same caster
            TestNotStack(wisdomBlessing, mightBlessing, true);
            TestNotStack(mightBlessing, greaterKings, true);
            TestNotStack(greaterLight, greaterWisdom, true);

            //haste group
            uint32 const heroism = 32182;
            uint32 const bloodlust = 41185;
            uint32 const powerInfusion = 10060;
            uint32 const icyVeins = 12472;

            TestNotStack(heroism, bloodlust);
            TestNotStack(bloodlust, powerInfusion);
            TestNotStack(powerInfusion, icyVeins);
            TestNotStack(icyVeins, heroism);
            TestNotStack(heroism, powerInfusion);
            TestNotStack(bloodlust, icyVeins);

            //earth shield
            TestNotStack(974, 974);
            //todo: also need to test more powerful spell here

            //armor debuffs
            uint32 const sunderArmor = 7386;
            uint32 const exposeArmor = 8647;
            uint32 const crystalYield = 15235;
            TestNotStack(sunderArmor, exposeArmor);
            TestNotStack(sunderArmor, crystalYield);
            TestNotStack(exposeArmor, crystalYield);

            //totems http://www.twentytotems.com/2007/11/stackable-totems-guide_27.html
            uint32 const healingStreamTotemEffect = 5672;
            TestStack(healingStreamTotemEffect);
            uint32 const manaStreamTotemEffect = 5677;
            TestNotStack(manaStreamTotemEffect);
            uint32 const tranquilAirEffect = 25909;
            TestNotStack(tranquilAirEffect);
            uint32 const wrathOfAirEffect = 2895;
            TestNotStack(wrathOfAirEffect);
            uint32 const natureResistanceEffect = 10596;
            TestNotStack(natureResistanceEffect);
            uint32 const windWallEffect = 15108;
            TestNotStack(windWallEffect);
            uint32 const graceOfAirEffect = 8836;
            TestNotStack(graceOfAirEffect);
            uint32 const fireResistanceTotem = 8185;
            TestNotStack(fireResistanceTotem);
            uint32 const frostResistanceTotem = 8182;
            TestNotStack(frostResistanceTotem);
            uint32 const stoneSkinTotem = 8072;
            TestNotStack(stoneSkinTotem);
            uint32 const earthBindTotemEffect = 3600;
            TestNotStack(earthBindTotemEffect);
            uint32 const strenghtOfEarthTotem = 8076;
            TestNotStack(strenghtOfEarthTotem);
            uint32 const totemOfWrath = 30708;
            TestStack(totemOfWrath); //does stack!

            //misc spells
            TestStack(15407); //Mind flay
            TestStack(6074); //Renew
            TestNotStack(33876, 33878); //Mangle cat + bear
            TestNotStack(19977); //blessing of light
            TestNotStack(31944); // Doomfire DoT - only one per target
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<StackingTestImpl>();
    }
};
void AddSC_test_spells_misc()
{
    new EnvironmentalTrapTest();
    new StackingRulesTest();
}
