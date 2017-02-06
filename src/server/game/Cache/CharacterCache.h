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
    void AddCharacterCacheEntry(uint32 guid, uint32 accountId, std::string const& name, uint8 gender, uint8 race, uint8 playerClass, uint8 level, /* uint16 mailCount, */ uint32 guildId);
    void DeleteCharacterCacheEntry(uint32 guid, std::string const& name);

    void UpdateCharacterData(uint32 guid, uint8 mask, std::string const& name, uint8 gender = GENDER_NONE, uint8 race = RACE_NONE, uint8 playerClass = 0);
    void UpdateCharacterLevel(uint32 const& guid, uint8 level);
    void UpdateCharacterAccountId(ObjectGuid const& guid, uint32 accountId);
    //NYI void UpdateCharacterMails(uint32 guid, int16 count, bool add = true);
    void UpdateCharacterGuildId(uint32 guid, uint32 guildId);
    //NYI void UpdateCharacterGroup(uint32 guid, uint32 groupId);
    void UpdateCharacterArenaTeamId(uint32 guid, uint8 slot, uint32 arenaTeamId);

    CharacterCacheEntry const* GetCharacterCacheByGuid(uint32 guid) const;
    CharacterCacheEntry const* GetCharacterCacheByName(std::string const& name) const;
	bool HasCharacterCacheEntry(uint32 guidLow) const;

    uint32 GetCharacterGuidByName(std::string const& name) const;
    bool GetCharacterNameByGuid(uint64 guid, std::string& name) const;
    uint32 GetCharacterTeamByGuid(uint64 guid) const;
    uint32 GetCharacterAccountIdByGuid(uint64 guid) const;
    uint32 GetCharacterAccountIdByName(std::string const& name) const;
    uint8 GetCharacterLevelByGuid(uint64 guid) const;
    uint32 GetCharacterGuildIdByGuid(uint64 guid) const;
    uint32 GetCharacterArenaTeamIdByGuid(uint64 guid, uint8 type) const;
};

#define sCharacterCache CharacterCache::instance()

#endif // CharacterCache_h__