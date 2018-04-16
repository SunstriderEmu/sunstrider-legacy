#ifndef CharacterCache_h__
#define CharacterCache_h__
 
#include "ArenaTeam.h"
#include "Define.h"
#include <string>

struct CharacterCacheEntry
{
    uint32 guidLow;
    uint32 accountId;
    std::string name;
    uint8 race;
    uint8 playerClass;
    uint8 gender;
    uint8 level;
    //NYI uint16 mailCount;
    uint32 guildId;
    //NYI uint32 groupId;
    uint32 arenaTeamId[MAX_ARENA_SLOT];
};

class TC_GAME_API CharacterCache
{
public:
    CharacterCache();
    ~CharacterCache();
    static CharacterCache* instance();

	void LoadCharacterCacheStorage();
    void AddCharacterCacheEntry(ObjectGuid::LowType guid, uint32 accountId, std::string const& name, uint8 gender, uint8 race, uint8 playerClass, uint8 level, /* uint16 mailCount, */ uint32 guildId);
    void DeleteCharacterCacheEntry(ObjectGuid::LowType guid, std::string const& name);

    void UpdateCharacterData(ObjectGuid::LowType guid, uint8 mask, std::string const& name, uint8 gender = GENDER_NONE, uint8 race = RACE_NONE, uint8 playerClass = 0);
    void UpdateCharacterLevel(ObjectGuid::LowType const& guid, uint8 level);
    void UpdateCharacterAccountId(ObjectGuid const& guid, uint32 accountId);
    //NYI void UpdateCharacterMails(ObjectGuid::LowType guid, int16 count, bool add = true);
    void UpdateCharacterGuildId(ObjectGuid::LowType guid, uint32 guildId);
    //NYI void UpdateCharacterGroup(ObjectGuid::LowType guid, uint32 groupId);
    void UpdateCharacterArenaTeamId(ObjectGuid::LowType guid, uint8 slot, uint32 arenaTeamId);

    CharacterCacheEntry const* GetCharacterCacheByGuid(ObjectGuid::LowType guid) const;
    CharacterCacheEntry const* GetCharacterCacheByName(std::string const& name) const;
	bool HasCharacterCacheEntry(ObjectGuid::LowType guidLow) const;

    ObjectGuid GetCharacterGuidByName(std::string const& name) const;
    bool GetCharacterNameByGuid(ObjectGuid guid, std::string& name) const;
    uint32 GetCharacterTeamByGuid(ObjectGuid guid) const;
    uint32 GetCharacterAccountIdByGuid(ObjectGuid guid) const;
    uint32 GetCharacterAccountIdByName(std::string const& name) const;
    uint8 GetCharacterLevelByGuid(ObjectGuid guid) const;
    ObjectGuid::LowType GetCharacterGuildIdByGuid(ObjectGuid guid) const;
    uint32 GetCharacterArenaTeamIdByGuid(ObjectGuid guid, uint8 type) const;
};

#define sCharacterCache CharacterCache::instance()

#endif // CharacterCache_h__