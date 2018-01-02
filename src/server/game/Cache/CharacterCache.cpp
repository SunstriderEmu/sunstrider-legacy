#include "CharacterCache.h"
#include "ArenaTeam.h"
#include "DatabaseEnv.h"
#include "Log.h"
#include "Player.h"
#include "Timer.h"
#include "World.h"
#include "WorldPacket.h"
#include <unordered_map>

namespace
{
    std::unordered_map<uint64, CharacterCacheEntry> _characterCacheStore;
    std::unordered_map<std::string, CharacterCacheEntry*> _characterCacheByNameStore;
}

CharacterCache::CharacterCache()
{
}

CharacterCache::~CharacterCache()
{
}

CharacterCache* CharacterCache::instance()
{
    static CharacterCache instance;
    return &instance;
}

void CharacterCache::LoadCharacterCacheStorage()
{
    uint32 oldMSTime = GetMSTime();

    _characterCacheStore.clear();
    QueryResult result = CharacterDatabase.Query("SELECT guid, account, name, gender, race, class, level FROM characters WHERE deleteDate IS NULL");
    if (!result)
    {
        TC_LOG_ERROR("server.loading",">>  Loaded 0 Players data!");
        return;
    }

    uint32 count = 0;

    /*
    // query to load number of mails by receiver
    std::map<uint32, uint16> _mailCountMap;
    QueryResult mailCountResult = CharacterDatabase.Query("SELECT receiver, COUNT(receiver) FROM mail GROUP BY receiver");
    if (mailCountResult)
    {
        do
        {
            Field* fields = mailCountResult->Fetch();
            _mailCountMap[fields[0].GetUInt32()] = uint16(fields[1].GetUInt64());
        } while (mailCountResult->NextRow());
    }
    */

    do
    {
        Field* fields = result->Fetch();
        ObjectGuid::LowType guidLow = fields[0].GetUInt32();

        // count mails
        /*
        uint16 mailCount = 0;
        std::map<uint32, uint16>::const_iterator itr = _mailCountMap.find(guidLow);
        if (itr != _mailCountMap.end())
            mailCount = itr->second;
        */

        AddCharacterCacheEntry(
            guidLow,               /*guid*/
            fields[1].GetUInt32(), /*accountId*/
            fields[2].GetString(), /*name*/
            fields[3].GetUInt8(),  /*gender*/
            fields[4].GetUInt8(),  /*race*/
            fields[5].GetUInt8(),  /*class*/
            fields[6].GetUInt8(),  /*level*/
            //mailCount,             /*mail count*/
            0                      /*guild id*/);

        ++count;
    } while (result->NextRow());

    TC_LOG_INFO("server.loading", ">> Loaded %d Players data in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void CharacterCache::AddCharacterCacheEntry(ObjectGuid::LowType guid, uint32 accountId, std::string const& name, uint8 gender, uint8 race, uint8 playerClass, uint8 level, /* uint16 mailCount, */ uint32 guildId)
{
    CharacterCacheEntry& data = _characterCacheStore[guid];

    data.guidLow = guid;
    data.accountId = accountId;
    data.name = name;
    data.level = level;
    data.race = race;
    data.playerClass = playerClass;
    data.gender = gender;
    //data.mailCount = mailCount;
    data.guildId = guildId;
    //data.groupId = 0;
    data.arenaTeamId[0] = 0;
    data.arenaTeamId[1] = 0;
    data.arenaTeamId[2] = 0;

    // Fill Name to Guid Store
    _characterCacheByNameStore[name] = &data;
}

void CharacterCache::DeleteCharacterCacheEntry(ObjectGuid::LowType guid, std::string const& name)
{
    _characterCacheStore.erase(guid);
    _characterCacheByNameStore.erase(name);
}

void CharacterCache::UpdateCharacterData(ObjectGuid::LowType guid, uint8 mask, std::string const& name, uint8 gender, uint8 race, uint8 playerClass)
{
    auto itr = _characterCacheStore.find(guid);
    if (itr == _characterCacheStore.end())
        return;

    if (mask & PLAYER_UPDATE_DATA_RACE)
        itr->second.race = race;
    if (mask & PLAYER_UPDATE_DATA_CLASS)
        itr->second.playerClass = playerClass;
    if (mask & PLAYER_UPDATE_DATA_GENDER)
        itr->second.gender = gender;
    if (mask & PLAYER_UPDATE_DATA_NAME)
    {
        std::string oldName = itr->second.name;
        itr->second.name = name;

        // Correct name -> pointer storage
        _characterCacheByNameStore.erase(oldName);
        _characterCacheByNameStore[name] = &itr->second;
    }

    WorldPacket data(SMSG_INVALIDATE_PLAYER, 8);
    data << uint64(ObjectGuid(HighGuid::Player, guid));
    sWorld->SendGlobalMessage(&data);
}

void CharacterCache::UpdateCharacterLevel(ObjectGuid::LowType const& guid, uint8 level)
{
    auto itr = _characterCacheStore.find(guid);
    if (itr == _characterCacheStore.end())
        return;

    itr->second.level = level;
}

void CharacterCache::UpdateCharacterAccountId(ObjectGuid const& guid, uint32 accountId)
{
    auto itr = _characterCacheStore.find(guid);
    if (itr == _characterCacheStore.end())
        return;

    itr->second.accountId = accountId;
}

/*
void CharacterCache::UpdateCharacterMails(ObjectGuid::LowType guid, int16 count, bool add)
{
    auto itr = _characterCacheStore.find(guid);
    if (itr == _characterCacheStore.end())
        return;

    if (!add)
    {
        itr->second.mailCount = count;
        return;
    }

    int16 icount = (int16)itr->second.mailCount;
    if (count < 0 && abs(count) > icount)
        count = -icount;
    itr->second.mailCount = uint16(icount + count); // addition or subtraction
}
*/

void CharacterCache::UpdateCharacterGuildId(ObjectGuid::LowType guid, uint32 guildId)
{
    auto itr = _characterCacheStore.find(guid);
    if (itr == _characterCacheStore.end())
        return;

    itr->second.guildId = guildId;
}

/*
void CharacterCache::UpdateCharacterGroup(ObjectGuid::LowType guid, uint32 groupId)
{
    auto itr = _characterCacheStore.find(guid);
    if (itr == _characterCacheStore.end())
        return;

    itr->second.groupId = groupId;
}
*/

void CharacterCache::UpdateCharacterArenaTeamId(ObjectGuid::LowType guid, uint8 slot, uint32 arenaTeamId)
{
    auto itr = _characterCacheStore.find(guid);
    if (itr == _characterCacheStore.end())
        return;

    itr->second.arenaTeamId[slot] = arenaTeamId;
}

bool CharacterCache::HasCharacterCacheEntry(ObjectGuid::LowType guid) const
{
    return _characterCacheStore.find(guid) != _characterCacheStore.end();
}

CharacterCacheEntry const* CharacterCache::GetCharacterCacheByGuid(ObjectGuid::LowType guid) const
{
    auto itr = _characterCacheStore.find(guid);
    if (itr != _characterCacheStore.end())
        return &itr->second;

    return nullptr;
}

CharacterCacheEntry const* CharacterCache::GetCharacterCacheByName(std::string const& name) const
{
    auto itr = _characterCacheByNameStore.find(name);
    if (itr != _characterCacheByNameStore.end())
        return itr->second;

    return nullptr;
}

ObjectGuid CharacterCache::GetCharacterGuidByName(std::string const& name) const
{
    auto itr = _characterCacheByNameStore.find(name);
    if (itr != _characterCacheByNameStore.end())
        return ObjectGuid(HighGuid::Player, itr->second->guidLow);

    return ObjectGuid::Empty;
}


bool CharacterCache::GetCharacterNameByGuid(ObjectGuid guid, std::string& name) const
{
    auto itr = _characterCacheStore.find(guid);
    if (itr == _characterCacheStore.end())
        return false;

    name = itr->second.name;
    return true;
}

uint32 CharacterCache::GetCharacterTeamByGuid(ObjectGuid guid) const
{
    auto itr = _characterCacheStore.find(guid);
    if (itr == _characterCacheStore.end())
        return 0;

    return Player::TeamForRace(itr->second.race);
}

uint32 CharacterCache::GetCharacterAccountIdByGuid(ObjectGuid guid) const
{
    auto itr = _characterCacheStore.find(guid);
    if (itr == _characterCacheStore.end())
        return 0;

    return itr->second.accountId;
}

uint32 CharacterCache::GetCharacterAccountIdByName(std::string const& name) const
{
    auto itr = _characterCacheByNameStore.find(name);
    if (itr != _characterCacheByNameStore.end())
        return itr->second->accountId;

    return 0;
}

uint8 CharacterCache::GetCharacterLevelByGuid(ObjectGuid guid) const
{
    auto itr = _characterCacheStore.find(guid);
    if (itr == _characterCacheStore.end())
        return 0;

    return itr->second.level;
}

ObjectGuid::LowType CharacterCache::GetCharacterGuildIdByGuid(ObjectGuid guid) const
{
    auto itr = _characterCacheStore.find(guid);
    if (itr == _characterCacheStore.end())
        return 0;

    return itr->second.guildId;
}

uint32 CharacterCache::GetCharacterArenaTeamIdByGuid(ObjectGuid guid, uint8 type) const
{
    auto itr = _characterCacheStore.find(guid);
    if (itr == _characterCacheStore.end())
        return 0;

    return itr->second.arenaTeamId[ArenaTeam::GetSlotByType(type)];
}
