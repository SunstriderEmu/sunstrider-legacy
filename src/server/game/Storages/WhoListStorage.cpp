#include "WhoListStorage.h"
#include "World.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "WorldSession.h"

WhoListStorageMgr* WhoListStorageMgr::instance()
{
    static WhoListStorageMgr instance;
    return &instance;
}

void WhoListStorageMgr::Update()
{
    // clear current list
    _whoListStorage.clear();
    _whoListStorage.reserve( (size_t) 1000 /*sWorld->GetPlayerCount()+1*/);

    HashMapHolder<Player>::MapType const& m = ObjectAccessor::GetPlayers();
    for (HashMapHolder<Player>::MapType::const_iterator itr = m.begin(); itr != m.end(); ++itr)
    {
        if (!itr->second->FindMap() || itr->second->GetSession()->PlayerLoading())
            continue;

        std::string playerName = itr->second->GetName();
        std::wstring widePlayerName;
        if (!Utf8toWStr(playerName, widePlayerName))
            continue;

        wstrToLower(widePlayerName);

        std::string guildName = sObjectMgr->GetGuildNameById(itr->second->GetGuildId());
        std::wstring wideGuildName;
        if (!Utf8toWStr(guildName, wideGuildName))
            continue;

        wstrToLower(wideGuildName);
        //do not show players in arenas
        uint32 playerZoneId = itr->second->GetZoneId();
        if (playerZoneId == (uint32) 3698 || playerZoneId == (uint32) 3968 || playerZoneId == (uint32) 3702)
        {
            WorldLocation const& loc = itr->second->GetBattlegroundEntryPoint();
            uint32 mapId = loc.GetMapId();
            Map const* map = sMapMgr->FindBaseNonInstanceMap(mapId);
            if (map)
                playerZoneId = map->GetZoneId(loc.GetPositionX(), loc.GetPositionY(), loc.GetPositionZ());
        }

        // Conversion uint32 to uint8 here
        _whoListStorage.emplace_back(itr->second->GetGUID(), itr->second->GetTeam(), itr->second->GetSession()->GetSecurity(), itr->second->GetLevel(), 
            itr->second->GetClass(), itr->second->GetRace(), playerZoneId, itr->second->GetByteValue(PLAYER_BYTES_3, PLAYER_BYTES_3_OFFSET_GENDER), itr->second->IsVisible(),
            widePlayerName, wideGuildName, playerName, guildName);
    }
}