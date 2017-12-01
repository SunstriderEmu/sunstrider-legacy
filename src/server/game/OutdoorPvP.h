
#ifndef OUTDOOR_PVP_H_
#define OUTDOOR_PVP_H_

#include <map>
#include <set>
#include "ZoneScript.h"
#include "Position.h"

#define OPVP_TRIGGER_CREATURE_ENTRY 12999

enum ObjectiveStates{
    OBJECTIVESTATE_NEUTRAL = 0,
    OBJECTIVESTATE_ALLIANCE = 1,
    OBJECTIVESTATE_HORDE = 2,
    OBJECTIVESTATE_NEUTRAL_ALLIANCE_CHALLENGE = 3,
    OBJECTIVESTATE_NEUTRAL_HORDE_CHALLENGE = 4,
    OBJECTIVESTATE_ALLIANCE_HORDE_CHALLENGE = 5,
    OBJECTIVESTATE_HORDE_ALLIANCE_CHALLENGE = 6
};

enum OutdoorPvPTypes{
    OUTDOOR_PVP_HP = 1,
    OUTDOOR_PVP_NA = 2,
    OUTDOOR_PVP_TF = 3,
    OUTDOOR_PVP_ZM = 4,
    OUTDOOR_PVP_SI = 5,
    OUTDOOR_PVP_EP = 6
};

#define MAX_OUTDOORPVP_TYPES 7

// struct for go spawning
struct go_type{
    uint32 entry;
    uint32 map;
    float x;
    float y;
    float z;
    float o;
    float rot0;
    float rot1;
    float rot2;
    float rot3;
};

// struct for creature spawning
struct creature_type{
    uint32 entry;
    uint32 teamval;
    uint32 map;
    float x;
    float y;
    float z;
    float o;
};

// some class predefs
class Player;
class GameObject;
class WorldPacket;
class Creature;
class Unit;
struct GossipMenuItems;

class OutdoorPvP;
class TC_GAME_API OPvPCapturePoint 
{
public:
    OPvPCapturePoint(OutdoorPvP * pvp);
    virtual ~OPvPCapturePoint() {}

    virtual void FillInitialWorldStates(WorldPacket & data) {}

    // send world state update to all players present
    virtual void SendUpdateWorldState(uint32 field, uint32 value);
    // send kill notify to players in the controlling faction
    virtual void SendObjectiveComplete(uint32 id, uint64 guid);

    // used when player is activated/inactivated in the area
    bool HandlePlayerEnter(Player * plr);
    void HandlePlayerLeave(Player * plr);

    // checks if player is in range of a capture credit marker
    virtual bool IsInsideObjective(Player * plr);

    virtual bool HandleCustomSpell(Player *plr, uint32 spellId, GameObject * go);
    virtual int32 HandleOpenGo(Player *plr, uint64 guid);

    // returns true if the state of the objective has changed, in this case, the OutdoorPvP must send a world state ui update.
    virtual bool Update(uint32 diff);

	virtual void ChangeState() = 0;

	virtual void ChangeTeam(TeamId /*oldTeam*/) { }

	virtual void SendChangePhase();

    virtual bool HandleGossipOption(Player *plr, uint64 guid, uint32 gossipid);

    virtual bool CanTalkTo(Player * plr, Creature * c, GossipMenuItems const& gso);

    virtual bool HandleDropFlag(Player * plr, uint32 spellId);

    virtual void DeleteSpawns();

	ObjectGuid::LowType m_capturePointSpawnId;
	GameObject* m_capturePoint;

	void AddGO(uint32 type, ObjectGuid::LowType guid, uint32 entry = 0);
	void AddCre(uint32 type, ObjectGuid::LowType guid, uint32 entry = 0);

protected:

    virtual bool SetCapturePointData(uint32 entry, uint32 map, float x, float y, float z, float o, float rotation0, float rotation1, float rotation2, float rotation3);
    virtual bool AddObject(uint32 type, uint32 entry, uint32 map, float x, float y, float z, float o, float rotation0, float rotation1, float rotation2, float rotation3);
    virtual bool AddCreature(uint32 type, uint32 entry, uint32 map, float x, float y, float z, float o, uint32 spawntimedelay = 0);

    virtual bool DelCreature(uint32 type);
    virtual bool DelObject(uint32 type);
    virtual bool DelCapturePoint();

    virtual void UpdateActivePlayerProximityCheck();
	
protected:
    // active players in the area of the objective, 0 - alliance, 1 - horde
    std::set<uint64> m_activePlayers[2];
    // total shift needed to capture the objective
    float m_maxValue;
	float m_minValue;

    // maximum speed of capture
    float m_maxSpeed;
    // the status of the objective
    float m_value;

	TeamId m_team;

    // objective states
    uint32 m_OldState;
    uint32 m_State;
    // neutral value on capture bar
    uint32 m_neutralValuePct;

    // pointer to the OutdoorPvP this objective belongs to
    OutdoorPvP* m_PvP;

    // map to store the various gameobjects and creatures spawned by the objective
    //        type , guid
    std::map<uint32,uint64> m_Objects;
    std::map<uint32,uint64> m_Creatures;
    std::map<uint64,uint32> m_ObjectTypes;
    std::map<uint64,uint32> m_CreatureTypes;

    //uint64 m_CapturePointCreature;
};

// base class for specific outdoor pvp handlers
class TC_GAME_API OutdoorPvP : public ZoneScript
{
	friend class OutdoorPvPMgr;

public:
    // ctor
    OutdoorPvP();
    // dtor
    ~OutdoorPvP();
    // deletes all gos/creatures spawned by the pvp
    void DeleteSpawns();

	typedef std::map<ObjectGuid::LowType/*spawnId*/, OPvPCapturePoint*> OPvPCapturePointMap;
	typedef std::pair<ObjectGuid::LowType, GameObject*> GoScriptPair;
	typedef std::pair<ObjectGuid::LowType, Creature*> CreatureScriptPair;

    // called from Player::UpdateZone to add / remove buffs given by outdoor pvp events
    virtual void HandlePlayerEnterZone(Player * plr, uint32 zone);
    virtual void HandlePlayerLeaveZone(Player * plr, uint32 zone);
	virtual void HandlePlayerResurrects(Player* player, uint32 zone);

    // called when a player triggers an areatrigger
    virtual bool HandleAreaTrigger(Player * plr, uint32 trigger);
    // called on custom spell
    virtual bool HandleCustomSpell(Player *plr, uint32 spellId, GameObject * go);
    // called on go use
    virtual bool HandleOpenGo(Player *plr, uint64 guid);

    // setup stuff
    virtual bool SetupOutdoorPvP() {return true;}

	void OnGameObjectCreate(GameObject* go) override;
	void OnGameObjectRemove(GameObject* go) override;
	void OnCreatureCreate(Creature*) override;
	void OnCreatureRemove(Creature*) override;

    // world state stuff
    virtual void SendRemoveWorldStates(Player * plr) {}
    virtual void FillInitialWorldStates(WorldPacket & data) {}

    // send world state update to all players present
    virtual void SendUpdateWorldState(uint32 field, uint32 value);

    // called by OutdoorPvPMgr, updates the objectives and if needed, sends new worldstateui information
    virtual bool Update(uint32 diff);

    // handle npc/player kill
    virtual void HandleKill(Player * killer, Unit * killed);
    virtual void HandleKillImpl(Player * killer, Unit * killed) {}

    // checks if player is in range of a capture credit marker
    virtual bool IsInsideObjective(Player * plr);

    // awards rewards for player kill
    virtual void AwardKillBonus(Player * plr) {}

    uint32 GetTypeId() {return m_TypeId;}

    virtual bool HandleDropFlag(Player * plr, uint32 spellId);

    virtual bool HandleGossipOption(Player *plr, uint64 guid, uint32 gossipid);

    virtual bool CanTalkTo(Player * plr, Creature * c, GossipMenuItems const& gso);

	void SendDefenseMessage(uint32 zoneId, uint32 id);

	Map* GetMap() const { return m_map; }

protected:
    // the map of the objectives belonging to this outdoorpvp
    OPvPCapturePointMap m_capturePoints;
    // players in the zones of this outdoorpvp, 0 - alliance, 1 - horde
    GuidSet m_players[2];
    uint32 m_TypeId;

	bool HasPlayer(Player const* player) const;

	OPvPCapturePoint * GetCapturePoint(ObjectGuid::LowType guid) const;

	void AddCapturePoint(OPvPCapturePoint* cp)
	{
		m_capturePoints[cp->m_capturePointSpawnId] = cp;
	}

	// Hack to store map because this code is just shit
	void SetMapFromZone(uint32 zone);
	std::map<ObjectGuid::LowType, GameObject*> m_GoScriptStore;
	std::map<ObjectGuid::LowType, Creature*> m_CreatureScriptStore;

	Map* m_map;
};

#endif /*OUTDOOR_PVP_H_*/

