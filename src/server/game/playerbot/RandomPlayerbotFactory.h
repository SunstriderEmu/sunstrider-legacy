#ifndef _RandomPlayerbotFactory_H
#define _RandomPlayerbotFactory_H

#include "Common.h"
#include "PlayerbotAIBase.h"

class WorldPacket;
class Player;
class Unit;
class Object;
class Item;

using namespace std;

class RandomPlayerbotFactory
{
    public:
        RandomPlayerbotFactory(uint32 accountId);
        virtual ~RandomPlayerbotFactory() {}

    public:
        //return bot guid 
        uint32 CreateRandomBot(uint8 cls);
        //return bot guid
        uint32 CreateRandomBot(uint8 cls, uint8 race, bool testBot = false);
        static void CreateRandomBots();
        static void CreateRandomGuilds();

        static std::map<uint8, std::vector<uint8>> const& GetAvailableRacesForClasses();

        static std::string CreateTestBotName(); //just get a random name, don't care if it's used or not
        static std::string CreateRandomBotName();
        static std::string CreateRandomGuildName();

    private:
        uint32 accountId;
        static map<uint8, vector<uint8> > availableRaces;
};

#endif
