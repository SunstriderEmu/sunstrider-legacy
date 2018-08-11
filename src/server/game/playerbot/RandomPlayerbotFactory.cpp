
#include "playerbot.h"
#include "PlayerbotAIConfig.h"
#include "PlayerbotFactory.h"
#include "DatabaseEnv.h"
#include "PlayerbotAI.h"
#include "Player.h"
#include "Guild.h"
//#include "GuildMgr.h"
#include "RandomPlayerbotFactory.h"
#include "TestPlayer.h"

std::map<uint8, std::vector<uint8>> RandomPlayerbotFactory::availableRaces;

std::map<uint8, std::vector<uint8>> const& RandomPlayerbotFactory::GetAvailableRacesForClasses()
{
    static std::mutex initMutex;
    initMutex.lock();
    if (availableRaces.empty())
    {
        availableRaces[CLASS_WARRIOR].push_back(RACE_HUMAN);
        availableRaces[CLASS_WARRIOR].push_back(RACE_NIGHTELF);
        availableRaces[CLASS_WARRIOR].push_back(RACE_GNOME);
        availableRaces[CLASS_WARRIOR].push_back(RACE_DWARF);
        availableRaces[CLASS_WARRIOR].push_back(RACE_ORC);
        availableRaces[CLASS_WARRIOR].push_back(RACE_UNDEAD_PLAYER);
        availableRaces[CLASS_WARRIOR].push_back(RACE_TAUREN);
        availableRaces[CLASS_WARRIOR].push_back(RACE_TROLL);
        availableRaces[CLASS_WARRIOR].push_back(RACE_DRAENEI);

        availableRaces[CLASS_PALADIN].push_back(RACE_HUMAN);
        availableRaces[CLASS_PALADIN].push_back(RACE_DWARF);
        availableRaces[CLASS_PALADIN].push_back(RACE_DRAENEI);
        availableRaces[CLASS_PALADIN].push_back(RACE_BLOODELF);

        availableRaces[CLASS_ROGUE].push_back(RACE_HUMAN);
        availableRaces[CLASS_ROGUE].push_back(RACE_DWARF);
        availableRaces[CLASS_ROGUE].push_back(RACE_NIGHTELF);
        availableRaces[CLASS_ROGUE].push_back(RACE_GNOME);
        availableRaces[CLASS_ROGUE].push_back(RACE_ORC);
        availableRaces[CLASS_ROGUE].push_back(RACE_TROLL);
        availableRaces[CLASS_ROGUE].push_back(RACE_BLOODELF);

        availableRaces[CLASS_PRIEST].push_back(RACE_HUMAN);
        availableRaces[CLASS_PRIEST].push_back(RACE_DWARF);
        availableRaces[CLASS_PRIEST].push_back(RACE_NIGHTELF);
        availableRaces[CLASS_PRIEST].push_back(RACE_DRAENEI);
        availableRaces[CLASS_PRIEST].push_back(RACE_TROLL);
        availableRaces[CLASS_PRIEST].push_back(RACE_UNDEAD_PLAYER);
        availableRaces[CLASS_PRIEST].push_back(RACE_BLOODELF);

        availableRaces[CLASS_MAGE].push_back(RACE_HUMAN);
        availableRaces[CLASS_MAGE].push_back(RACE_GNOME);
        availableRaces[CLASS_MAGE].push_back(RACE_DRAENEI);
        availableRaces[CLASS_MAGE].push_back(RACE_UNDEAD_PLAYER);
        availableRaces[CLASS_MAGE].push_back(RACE_TROLL);
        availableRaces[CLASS_MAGE].push_back(RACE_BLOODELF);

        availableRaces[CLASS_WARLOCK].push_back(RACE_HUMAN);
        availableRaces[CLASS_WARLOCK].push_back(RACE_GNOME);
        availableRaces[CLASS_WARLOCK].push_back(RACE_UNDEAD_PLAYER);
        availableRaces[CLASS_WARLOCK].push_back(RACE_ORC);
        availableRaces[CLASS_WARLOCK].push_back(RACE_BLOODELF);

        availableRaces[CLASS_SHAMAN].push_back(RACE_DRAENEI);
        availableRaces[CLASS_SHAMAN].push_back(RACE_ORC);
        availableRaces[CLASS_SHAMAN].push_back(RACE_TAUREN);
        availableRaces[CLASS_SHAMAN].push_back(RACE_TROLL);

        availableRaces[CLASS_HUNTER].push_back(RACE_DWARF);
        availableRaces[CLASS_HUNTER].push_back(RACE_NIGHTELF);
        availableRaces[CLASS_HUNTER].push_back(RACE_DRAENEI);
        availableRaces[CLASS_HUNTER].push_back(RACE_ORC);
        availableRaces[CLASS_HUNTER].push_back(RACE_TAUREN);
        availableRaces[CLASS_HUNTER].push_back(RACE_TROLL);
        availableRaces[CLASS_HUNTER].push_back(RACE_BLOODELF);

        availableRaces[CLASS_DRUID].push_back(RACE_NIGHTELF);
        availableRaces[CLASS_DRUID].push_back(RACE_TAUREN);
    }
    initMutex.unlock();
    return availableRaces;
}

RandomPlayerbotFactory::RandomPlayerbotFactory(uint32 accountId) : accountId(accountId)
{
    GetAvailableRacesForClasses();
}

uint32 RandomPlayerbotFactory::CreateRandomBot(uint8 cls)
{
    uint8 race = availableRaces[cls][urand(0, availableRaces[cls].size() - 1)];
    return CreateRandomBot(cls, race);
}

uint32 RandomPlayerbotFactory::CreateRandomBot(uint8 cls, uint8 race)
{
    sLog->outMessage("playerbot", LOG_LEVEL_DEBUG, "Creating new random bot for class %d", cls);

    std::string name = CreateRandomBotName();
    if (name.empty())
        return 0;

    WorldSession* session = new WorldSession(accountId, BUILD_243, "rndbot", NULL, SEC_PLAYER, 1, 0, LOCALE_enUS, 0, false);
    if (!session)
    {
        sLog->outMessage("playerbot", LOG_LEVEL_ERROR, "Couldn't create session for random bot account %d", accountId);
        delete session;
        return 0;
    }

    Player* player = new Player(session);

    CharacterCreateInfo cci;
    cci.RandomizeAppearance();
    cci.Name = name;
    cci.Race = race;
    cci.Class = cls;

    if (!player->Create(sObjectMgr->GetGenerator<HighGuid::Player>().Generate(), &cci))
    {
        player->DeleteFromDB(player->GetGUID(), accountId, true, true);
        delete player;
        delete session; //delete player before session! Player destructor does reference session
        sLog->outMessage("playerbot", LOG_LEVEL_ERROR, "Unable to create random bot for account %d - name: \"%s\"; race: %u; class: %u; gender: %u; skin: %u; face: %u; hairStyle: %u; hairColor: %u; facialHair: %u; outfitId: %u",
                accountId, name.c_str(), race, cls, cci.Gender, cci.Skin, cci.Face, cci.HairStyle, cci.HairColor, cci.FacialHair, cci.OutfitId);
        return 0;
    }

    player->setCinematic(2);
    player->SetAtLoginFlag(AT_LOGIN_NONE);
    player->SaveToDB(true);

    sLog->outMessage("playerbot", LOG_LEVEL_DEBUG, "Random bot created for account %d - name: \"%s\"; race: %u; class: %u; gender: %u; skin: %u; face: %u; hairStyle: %u; hairColor: %u; facialHair: %u; outfitId: %u",
            accountId, name.c_str(), race, cls, cci.Gender, cci.Skin, cci.Face, cci.HairStyle, cci.HairColor, cci.FacialHair, cci.OutfitId);
    
    uint32 guid = player->GetGUID().GetCounter();
    //delete session;
    //delete player;
    return guid;
}

std::string RandomPlayerbotFactory::CreateTestBotName()
{
    static std::mutex queryMutex;
    static std::list<std::string> botNames; // a bit of caching since this request is quite heavy

    queryMutex.lock();
    if (botNames.empty())
    {
        QueryResult results = CharacterDatabase.PQuery("SELECT n.name FROM ai_playerbot_names n ORDER BY RAND() LIMIT 100");
        if (!results)
        {
            TC_LOG_ERROR("test.unit_test", "Could not get name for test bot, no names in ai_playerbot_name table");
            return "";
        }
        do
        {
            Field* fields = results->Fetch();
            botNames.push_back(fields[0].GetString());
        } while (results->NextRow());
    }
    std::string name = botNames.front();
    botNames.pop_front();
    queryMutex.unlock();
    while (ObjectAccessor::FindConnectedPlayerByName(name.c_str()))
    {
        if (botNames.empty())
        {
            TC_LOG_ERROR("test.unit_test", "Could not get name for test bot, none free found");
            return "";
        }
        name = botNames.front();
        botNames.pop_front();
    }

    return name;
}

std::string RandomPlayerbotFactory::CreateRandomBotName()
{
    QueryResult result = CharacterDatabase.Query("SELECT MAX(name_id) FROM ai_playerbot_names");
    if (!result)
    {
        sLog->outMessage("playerbot", LOG_LEVEL_ERROR, "No more names left for random bot");
        return "";
    }

    Field* fields = result->Fetch();

    result = CharacterDatabase.PQuery("SELECT n.name FROM ai_playerbot_names n "
            "LEFT OUTER JOIN characters e ON e.name = n.name "
            "WHERE e.guid IS NULL LIMIT 1");
    if (!result)
    {
        sLog->outMessage("playerbot", LOG_LEVEL_FATAL, "No names in ai_playerbot_name table");
        return "";
    }

    fields = result->Fetch();
    return fields[0].GetString();
}

void RandomPlayerbotFactory::CreateRandomBots()
{
    if (sPlayerbotAIConfig.deleteRandomBotAccounts)
    {
        sLog->outMessage("playerbot", LOG_LEVEL_INFO, "Deleting random bot accounts...");
        QueryResult results = LoginDatabase.PQuery("SELECT id FROM account where username like '%s%%'", sPlayerbotAIConfig.randomBotAccountPrefix.c_str());
        if (results)
        {
            do
            {
                Field* fields = results->Fetch();
                sAccountMgr->DeleteAccount(fields[0].GetUInt32());
            } while (results->NextRow());
        }

        CharacterDatabase.Execute("DELETE FROM ai_playerbot_random_bots");
        sLog->outMessage("playerbot", LOG_LEVEL_INFO, "Random bot accounts deleted");
    }

    for (int accountNumber = 0; accountNumber < sPlayerbotAIConfig.randomBotAccountCount; ++accountNumber)
    {
        std::ostringstream out; out << sPlayerbotAIConfig.randomBotAccountPrefix << accountNumber;
        std::string accountName = out.str();
        QueryResult results = LoginDatabase.PQuery("SELECT id FROM account where username = '%s'", accountName.c_str());
        if (results)
        {
            continue;
        }

        std::string password = "";
        for (int i = 0; i < 10; i++)
        {
            password += (char)urand('!', 'z');
        }
        sAccountMgr->CreateAccount(accountName, password, "playerbot");

        sLog->outMessage("playerbot", LOG_LEVEL_DEBUG, "Account %s created for random bots", accountName.c_str());
    }

    LoginDatabase.PExecute("UPDATE account SET expansion = '%u' where username like '%s%%'", 2, sPlayerbotAIConfig.randomBotAccountPrefix.c_str());

    int totalRandomBotChars = 0;
    for (int accountNumber = 0; accountNumber < sPlayerbotAIConfig.randomBotAccountCount; ++accountNumber)
    {
        std::ostringstream out; out << sPlayerbotAIConfig.randomBotAccountPrefix << accountNumber;
        std::string accountName = out.str();

        QueryResult results = LoginDatabase.PQuery("SELECT id FROM account where username = '%s'", accountName.c_str());
        if (!results)
            continue;

        Field* fields = results->Fetch();
        uint32 accountId = fields[0].GetUInt32();

        sPlayerbotAIConfig.randomBotAccounts.push_back(accountId);

        int count = sAccountMgr->GetCharactersCount(accountId);
        if (count >= 10)
        {
            totalRandomBotChars += count;
            continue;
        }

        RandomPlayerbotFactory factory(accountId);
        for (uint8 cls = CLASS_WARRIOR; cls < MAX_CLASSES; ++cls)
        {
            if (cls != 10 && cls != CLASS_DEATH_KNIGHT)
                factory.CreateRandomBot(cls);
        }

        totalRandomBotChars += sAccountMgr->GetCharactersCount(accountId);
    }

    sLog->outMessage("playerbot", LOG_LEVEL_INFO, "%d random bot accounts with %d characters available", sPlayerbotAIConfig.randomBotAccounts.size(), totalRandomBotChars);
}


void RandomPlayerbotFactory::CreateRandomGuilds()
{
    vector<uint32> randomBots;
    QueryResult results = LoginDatabase.PQuery("SELECT id FROM account where username like '%s%%'", sPlayerbotAIConfig.randomBotAccountPrefix.c_str());
    if (results)
    {
        do
        {
            Field* fields = results->Fetch();
            uint32 accountId = fields[0].GetUInt32();

            QueryResult results2 = CharacterDatabase.PQuery("SELECT guid FROM characters where account  = '%u'", accountId);
            if (results2)
            {
                do
                {
                    Field* fields2 = results2->Fetch();
                    uint32 guid = fields2[0].GetUInt32();
                    randomBots.push_back(guid);
                } while (results2->NextRow());
            }

        } while (results->NextRow());
    }

    if (sPlayerbotAIConfig.deleteRandomBotGuilds)
    {
        sLog->outMessage("playerbot", LOG_LEVEL_INFO, "Deleting random bot guilds...");
        for (vector<uint32>::iterator i = randomBots.begin(); i != randomBots.end(); ++i)
        {
            ObjectGuid leader(HighGuid::Player, *i);
            Guild* guild = sObjectMgr->GetGuildByLeader(leader);
            if (guild) 
                guild->Disband();
        }
        sLog->outMessage("playerbot", LOG_LEVEL_INFO, "Random bot guilds deleted");
    }

    int guildNumber = 0;
    vector<ObjectGuid> availableLeaders;
    for (vector<uint32>::iterator i = randomBots.begin(); i != randomBots.end(); ++i)
    {
        ObjectGuid leader(HighGuid::Player, *i);
        Guild* guild = sObjectMgr->GetGuildByLeader(leader);
        if (guild)
        {
            ++guildNumber;
            sPlayerbotAIConfig.randomBotGuilds.push_back(guild->GetId());
        }
        else
        {
            Player* player = ObjectAccessor::FindPlayer(leader);
            if (player)
                availableLeaders.push_back(leader);
        }
    }

    for (; guildNumber < sPlayerbotAIConfig.randomBotGuildCount; ++guildNumber)
    {
        std::string guildName = CreateRandomGuildName();
        if (guildName.empty())
            break;

        if (availableLeaders.empty())
        {
            sLog->outMessage("playerbot", LOG_LEVEL_ERROR, "No leaders for random guilds available");
            break;
        }

        int index = urand(0, availableLeaders.size() - 1);
        ObjectGuid leader = availableLeaders[index];
        Player* player = ObjectAccessor::FindPlayer(leader);
        if (!player)
        {
            sLog->outMessage("playerbot", LOG_LEVEL_ERROR, "Cannot find player for leader %u", leader);
            break;
        }

        Guild* guild = new Guild();
        if (!guild->create(player->GetGUID(), guildName))
        {
            sLog->outMessage("playerbot", LOG_LEVEL_ERROR, "Error creating guild %s", guildName.c_str());
            break;
        }

        sObjectMgr->AddGuild(guild);
        sPlayerbotAIConfig.randomBotGuilds.push_back(guild->GetId());
    }

    sLog->outMessage("playerbot", LOG_LEVEL_INFO, "%d random bot guilds available", guildNumber);
}

string RandomPlayerbotFactory::CreateRandomGuildName()
{
    QueryResult result = CharacterDatabase.Query("SELECT MAX(name_id) FROM ai_playerbot_guild_names");
    if (!result)
    {
        sLog->outMessage("playerbot", LOG_LEVEL_ERROR, "No more names left for random guilds");
        return "";
    }

    Field *fields = result->Fetch();
    uint32 maxId = fields[0].GetUInt32();

    uint32 id = urand(0, maxId);
    result = CharacterDatabase.PQuery("SELECT n.name FROM ai_playerbot_guild_names n "
            "LEFT OUTER JOIN guild e ON e.name = n.name "
            "WHERE e.guildid IS NULL AND n.name_id >= '%u' LIMIT 1", id);
    if (!result)
    {
        sLog->outMessage("playerbot", LOG_LEVEL_ERROR, "No more names left for random guilds");
        return "";
    }

    fields = result->Fetch();
    return fields[0].GetString();
}

