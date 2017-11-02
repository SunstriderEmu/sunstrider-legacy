#include "test_case.h"

#include "Player.h"
#include "Creature.h"
#include "PlayerbotAI.h"
#include "RandomPlayerbotMgr.h"
#include "MapManager.h"

#define TEST_CREATURE_ENTRY 8

/*
void Testing::PrepareCasterAndTarget(Player*& caster, Creature*& target)
{
    //corner in testing map
    static const WorldLocation testLoc = WorldLocation(13, -128.0f, -128.0f, -144.7f, 0.4f);

    uint32 bot = sRandomPlayerbotMgr.AddRandomBot();
    REQUIRE(bot != 0);

    Player* playerBot = sRandomPlayerbotMgr.AddPlayerBot(bot, 0);
    REQUIRE(playerBot != nullptr);

    PlayerbotAI* ai = playerBot->GetPlayerbotAI();
    REQUIRE(ai != nullptr);

    //handle bot position
    bool teleportOK = playerBot->TeleportTo(testLoc);
    REQUIRE(teleportOK);
    ai->HandleTeleportAck(); //immediately handle teleport packet

                             //spawn target creature
    Position inFront(playerBot);
    playerBot->MovePosition(inFront, 10.0f, playerBot->GetOrientation());
    Creature* targetCreature = playerBot->SummonCreature(10, inFront); // Creature ID 10 = Training dummy
    REQUIRE(targetCreature);

    caster = playerBot;
    target = targetCreature;
}

void Testing::PrepareCasterAndTarget_cleanup(Player*& caster, Creature*& target)
{
    caster->GetSession()->KickPlayer(); //not working for bots ?
    target->DespawnOrUnsummon();
}

TestCase::TestCase(bool mapIsolated)
    : TestCase(mapIsolated, WorldLocation(13, -128.0f, -128.0f, -144.7f, 0.0f)) //default test location
{
}

TestCase::TestCase(bool mapIsolated, WorldLocation specificPosition) : 
    _testLocation(specificPosition),
    _mapIsolated(mapIsolated)
{
    _testMap = sMapMgr->CreateBaseMap(specificPosition.GetMapId());
}

Player* TestCase::SpawnRandomPlayer()
{
    Player* playerBot = sRandomPlayerbotMgr.CreateRandomTestBot(_testLocation);
    //REQUIRE(playerBot != 0);
    return playerBot;
}

Creature* TestCase::SpawnCreature(uint32 entry, bool spawnInFront)
{
    Position spawnPosition(_testLocation);
    if (spawnInFront)
    {
        //get a position in front of default location
        spawnPosition.m_positionX = spawnPosition.m_positionX + 10.0f * std::cos(spawnPosition.m_orientation);
        spawnPosition.m_positionY = spawnPosition.m_positionY + 10.0f * std::sin(spawnPosition.m_orientation);
    }

    return SpawnCreatureWithPosition(spawnPosition, entry);
}

Creature* TestCase::SpawnCreatureWithPosition(Position spawnPosition, uint32 entry)
{
    //spawnPosition. si 0 on ignore
    uint32 creatureEntry = entry ? entry : TEST_CREATURE_ENTRY;

    return _testMap->SummonCreature(creatureEntry, spawnPosition);

    //Creature* targetCreature = playerBot->SummonCreature(TEST_CREATURE_ENTRY, inFront);
}

Player* TestCase::SpawnPlayer(Classes _class, Races _race, uint32 level, Position spawnPosition)
{
    //TODO;
    return nullptr;
}

TestCase::~TestCase()
{
    for (auto object : _spawnedObjects)
    {
        switch (object->GetTypeId())
        {
        case TYPEID_PLAYER:
            object->ToPlayer()->GetSession()->KickPlayer(); //not working for bots ?
            break;
        case TYPEID_UNIT:
            object->ToCreature()->DespawnOrUnsummon();
            break;
        case TYPEID_GAMEOBJECT:
            object->ToGameObject()->Delete();
            break;
        default:
            TC_LOG_ERROR("testing", "Testing: Failed to destroy object of unknown type %u", object->GetTypeId());
        }
    }
}

void TestCase::TestStacksCount(Player* caster, Unit* target, uint32 talent, uint32 castSpell, uint32 testSpell, uint32 requireCount)
{
    caster->LearnSpell(castSpell, false);
    caster->LearnSpell(talent, false);

    uint32 castCount = 0;
    while (castCount < requireCount*5) { //cast a bunch more
        caster->CastSpell(target, castSpell, true);
        castCount++;
    }
    uint32 auraCount = target->GetAuraCount(testSpell);
    //CHECK(auraCount == requireCount);
}

*/