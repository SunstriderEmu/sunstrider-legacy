
#ifndef OUTDOOR_PVP_MGR_H_
#define OUTDOOR_PVP_MGR_H_

#define OUTDOORPVP_OBJECTIVE_UPDATE_INTERVAL 1000

#include "OutdoorPvP.h"

class Player;
class GameObject;
class Creature;
struct GossipMenuItems;

// class to handle player enter / leave / areatrigger / GO use events
class TC_GAME_API OutdoorPvPMgr
{
private:
    OutdoorPvPMgr();
    ~OutdoorPvPMgr() = default;
public:
    static OutdoorPvPMgr* instance()
    {
        static OutdoorPvPMgr instance;
        return &instance;
    }

    // create outdoor pvp events
    void InitOutdoorPvP();
    // cleanup
    void Die();
    // called when a player enters an outdoor pvp area
    void HandlePlayerEnterZone(Player * plr, uint32 areaflag);
    // called when player leaves an outdoor pvp area
    void HandlePlayerLeaveZone(Player * plr, uint32 areaflag);
	// called when player resurrects
	void HandlePlayerResurrects(Player* player, uint32 areaflag);

    // return assigned outdoor pvp
    OutdoorPvP * GetOutdoorPvPToZoneId(uint32 zoneid);
    // handle custom (non-exist in dbc) spell if registered
    bool HandleCustomSpell(Player * plr, uint32 spellId, GameObject* go);
    // handle custom go if registered
    bool HandleOpenGo(Player * plr, uint64 guid);

	ZoneScript* GetZoneScript(uint32 zoneId);

    void AddZone(uint32 zoneid, OutdoorPvP * handle);

    void Update(uint32 diff);

    void HandleGossipOption(Player * player, uint64 guid, uint32 gossipid);

    bool CanTalkTo(Player * player, Creature * creature, GossipMenuItems const& gso);

    void HandleDropFlag(Player * plr, uint32 spellId);

    typedef std::vector<OutdoorPvP*> OutdoorPvPSet;
    typedef std::map<uint32 /* zoneid */, OutdoorPvP*> OutdoorPvPMap;
private:
    // contains all initiated outdoor pvp events
    // used when initing / cleaning up
    OutdoorPvPSet  m_OutdoorPvPSet;
    // maps the zone ids to an outdoor pvp event
    // used in player event handling
    OutdoorPvPMap   m_OutdoorPvPMap;
    // update interval
    float m_UpdateTimer;
};

#define sOutdoorPvPMgr OutdoorPvPMgr::instance()

#endif /*OUTDOOR_PVP_MGR_H_*/

