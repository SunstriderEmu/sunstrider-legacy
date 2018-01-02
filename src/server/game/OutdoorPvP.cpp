
#include "OutdoorPvP.h"
#include "OutdoorPvPMgr.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Map.h"
#include "MapManager.h"
#include "Group.h"
#include "WorldPacket.h"
#include "Creature.h"

OPvPCapturePoint::OPvPCapturePoint(OutdoorPvP * pvp)
: m_PvP(pvp), m_value(0), m_maxValue(0), m_minValue(0.0f), m_team(TEAM_NEUTRAL),
m_State(0), m_OldState(0), m_capturePoint(nullptr), m_neutralValuePct(0),
m_maxSpeed(0), m_capturePointSpawnId(0)
{
}

bool OPvPCapturePoint::HandlePlayerEnter(Player * player)
{
    if (m_capturePoint)
    {
        player->SendUpdateWorldState(m_capturePoint->GetGOInfo()->capturePoint.worldState1, 1);
        player->SendUpdateWorldState(m_capturePoint->GetGOInfo()->capturePoint.worldstate2, (uint32)ceil((m_value + m_maxValue) / (2 * m_maxValue) * 100.0f));
        player->SendUpdateWorldState(m_capturePoint->GetGOInfo()->capturePoint.worldstate3, m_neutralValuePct);
    }

    //avoid having gamemasters capturing points
    if (player->IsGameMaster())
        return false;

    return m_activePlayers[player->GetTeamId()].insert(player->GetGUID()).second;
}

void OPvPCapturePoint::HandlePlayerLeave(Player* player)
{
    if (m_capturePoint)
        player->SendUpdateWorldState(m_capturePoint->GetGOInfo()->capturePoint.worldState1, 0);

    m_activePlayers[player->GetTeamId()].erase(player->GetGUID());
}

bool OPvPCapturePoint::AddObject(uint32 type, uint32 entry, uint32 map, float x, float y, float z, float o, float rotation0, float rotation1, float rotation2, float rotation3)
{
    if (ObjectGuid::LowType guid = sObjectMgr->AddGameObjectData(entry, map, x, y, z, o, 0, rotation0, rotation1, rotation2, rotation3))
    {
        AddGO(type, guid, entry);
        return true;
    }

    return true;
}

void OPvPCapturePoint::AddGO(uint32 type, ObjectGuid::LowType guid, uint32 entry)
{
    if (!entry)
    {
        GameObjectData const* data = sObjectMgr->GetGameObjectData(guid);
        if (!data)
            return;
        entry = data->id;
    }

    m_Objects[type] = ObjectGuid(HighGuid::GameObject, entry, guid);
    m_ObjectTypes[m_Objects[type]] = type;
}

void OPvPCapturePoint::AddCre(uint32 type, ObjectGuid::LowType guid, uint32 entry)
{
    if (!entry)
    {
        CreatureData const* data = sObjectMgr->GetCreatureData(guid);
        if (!data)
            return;
        entry = data->id;
    }

    m_Creatures[type] = ObjectGuid(HighGuid::Unit, entry, guid);
    m_CreatureTypes[m_Creatures[type]] = type;
}

bool OPvPCapturePoint::AddCreature(uint32 type, uint32 entry, uint32 map, float x, float y, float z, float o, uint32 spawntimedelay)
{
    if (ObjectGuid::LowType guid = sObjectMgr->AddCreatureData(entry, map, x, y, z, o, spawntimedelay))
    {
        AddCre(type, guid, entry);
        return true;
    }

    return false;
}

void OPvPCapturePoint::SendChangePhase()
{
    if (!m_capturePoint)
        return;

    // send this too, sometimes the slider disappears, dunno why :(
    SendUpdateWorldState(m_capturePoint->GetGOInfo()->capturePoint.worldState1, 1);
    // send these updates to only the ones in this objective
    SendUpdateWorldState(m_capturePoint->GetGOInfo()->capturePoint.worldstate2, (uint32)ceil((m_value + m_maxValue) / (2 * m_maxValue) * 100.0f));
    // send this too, sometimes it resets :S
    SendUpdateWorldState(m_capturePoint->GetGOInfo()->capturePoint.worldstate3, m_neutralValuePct);
}

bool OPvPCapturePoint::SetCapturePointData(uint32 entry, uint32 map, float x, float y, float z, float o, float rotation0, float rotation1, float rotation2, float rotation3)
{
    // check info existence
    GameObjectTemplate const* goinfo = sObjectMgr->GetGameObjectTemplate(entry);
    if (!goinfo || goinfo->type != GAMEOBJECT_TYPE_CAPTURE_POINT)
    {
        TC_LOG_ERROR("outdoorpvp", "OutdoorPvP: GO %u is not capture point!", entry);
        return false;
    }

    // create capture point go
    m_capturePointSpawnId = sObjectMgr->AddGameObjectData(entry, map, x, y, z, o, 0, rotation0, rotation1, rotation2, rotation3);
    if (m_capturePointSpawnId == 0)
        return false;

    // get the needed values from goinfo
    m_maxValue = goinfo->capturePoint.maxTime;
    m_maxSpeed = m_maxValue / (goinfo->capturePoint.minTime ? goinfo->capturePoint.minTime : 60);
    m_neutralValuePct = goinfo->capturePoint.neutralPercent;
    m_minValue = CalculatePct(m_maxValue, m_neutralValuePct);

    return true;
}

bool OPvPCapturePoint::DelCreature(uint32 type)
{
    uint32 spawnId = m_Creatures[type];
    if(!m_Creatures[type])
    {
        TC_LOG_ERROR("FIXME","opvp creature type %u was already deleted",type);
        return false;
    }
    
    auto bounds = m_PvP->GetMap()->GetCreatureBySpawnIdStore().equal_range(spawnId);
    for (auto itr = bounds.first; itr != bounds.second;)
    {
        Creature* c = itr->second;
        ++itr;
        // Don't save respawn time
        c->SetRespawnTime(0);
        c->DespawnOrUnsummon();
        c->AddObjectToRemoveList();
    }

    Creature *cr = m_PvP->GetMap()->GetCreature(m_Creatures[type]);
    if(!cr)
    {
        // can happen when closing the core
        m_Creatures[type].Clear();
        return false;
    }

    TC_LOG_DEBUG("outdoorpvp", "deleting opvp creature type %u", type);
    // explicit removal from map
    // beats me why this is needed, but with the recent removal "cleanup" some creatures stay in the map if "properly" deleted
    // so this is a big fat workaround, if AddObjectToRemoveList and DoDelayedMovesAndRemoves worked correctly, this wouldn't be needed
    //if (Map* map = sMapMgr->FindMap(cr->GetMapId()))
    //    map->Remove(cr, false);
    // delete respawn time for this creature
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CREATURE_RESPAWN);
    stmt->setUInt32(0, spawnId);
    stmt->setUInt16(1, m_PvP->GetMap()->GetId());
    stmt->setUInt32(2, 0);  // instance id, always 0 for world maps
    CharacterDatabase.Execute(stmt);

    sObjectMgr->DeleteCreatureData(spawnId);
    m_CreatureTypes[m_Creatures[type]] = 0;
    m_Creatures[type].Clear();
    return true;
}

bool OPvPCapturePoint::DelObject(uint32 type)
{
    if(!m_Objects[type])
        return false;
    
    GameObject *obj = m_PvP->GetMap()->GetGameObject(m_Objects[type]);
    if(!obj)
    {
        m_Objects[type].Clear();
        return false;
    }
    uint32 guid = obj->GetSpawnId();
    obj->SetRespawnTime(0);                                 // not save respawn time
    obj->Delete();
    sObjectMgr->DeleteGameObjectData(guid);
    m_ObjectTypes[m_Objects[type]] = 0;
    m_Objects[type].Clear();
    return true;
}

bool OPvPCapturePoint::DelCapturePoint()
{
    sObjectMgr->DeleteGameObjectData(m_capturePointSpawnId);
    m_capturePointSpawnId = 0;

    if (m_capturePoint)
    {
        m_capturePoint->SetRespawnTime(0);                                 // not save respawn time
        m_capturePoint->Delete();
    }

    return true;
}

bool OutdoorPvP::HasPlayer(Player const* player) const
{
    GuidSet const &plSet = m_players[player->GetTeamId()];
    return plSet.find(player->GetGUID()) != plSet.end();
}

OPvPCapturePoint * OutdoorPvP::GetCapturePoint(ObjectGuid::LowType guid) const
{
    OutdoorPvP::OPvPCapturePointMap::const_iterator itr = m_capturePoints.find(guid);
    if (itr != m_capturePoints.end())
        return itr->second;
    return NULL;
}

void OPvPCapturePoint::DeleteSpawns()
{
    for(auto & m_Object : m_Objects)
        DelObject(m_Object.first);
    for(auto & m_Creature : m_Creatures)
        DelCreature(m_Creature.first);
    DelCapturePoint();
}

void OutdoorPvP::DeleteSpawns()
{
    // Remove script from any registered gameobjects/creatures
    for (auto itr = m_GoScriptStore.begin(); itr != m_GoScriptStore.end(); ++itr)
    {
        if (GameObject* go = itr->second)
            go->ClearZoneScript();
    }
    m_GoScriptStore.clear();

    for (auto itr = m_CreatureScriptStore.begin(); itr != m_CreatureScriptStore.end(); ++itr)
    {
        if (Creature* creature = itr->second)
            creature->ClearZoneScript();
    }
    m_CreatureScriptStore.clear();

    for (auto & itr : m_capturePoints)
    {
        itr.second->DeleteSpawns();
        delete itr.second;
    }
    m_capturePoints.clear();
}

OutdoorPvP::OutdoorPvP() 
    : m_map(nullptr)
{
}

OutdoorPvP::~OutdoorPvP()
{
    DeleteSpawns();
}

void OutdoorPvP::HandlePlayerEnterZone(Player * plr, uint32 zone)
{
    m_players[plr->GetTeamId()].insert(plr->GetGUID());
}

void OutdoorPvP::HandlePlayerLeaveZone(Player * player, uint32 zone)
{
    // inform the objectives of the leaving
    for (OPvPCapturePointMap::iterator itr = m_capturePoints.begin(); itr != m_capturePoints.end(); ++itr)
        itr->second->HandlePlayerLeave(player);

    // remove the world state information from the player (we can't keep everyone up to date, so leave out those who are not in the concerning zones)
    if(zone != player->GetZoneId())
        SendRemoveWorldStates(player);

     m_players[player->GetTeamId()].erase(player->GetGUID());
}

void OutdoorPvP::HandlePlayerResurrects(Player* /*player*/, uint32 /*zone*/) { }

bool OutdoorPvP::Update(uint32 diff)
{
    bool objective_changed = false;
    for (OPvPCapturePointMap::iterator itr = m_capturePoints.begin(); itr != m_capturePoints.end(); ++itr)
    {
        if (itr->second->Update(diff))
            objective_changed = true;
    }
    return objective_changed;
}

void OPvPCapturePoint::UpdateActivePlayerProximityCheck()
{
    for(auto & m_ActivePlayerGuid : m_activePlayers)
    {
        std::set<ObjectGuid>::iterator itr, next;
        for(itr = m_ActivePlayerGuid.begin(); itr != m_ActivePlayerGuid.end(); itr = next)
        {
            next = itr;
            ++next;
            // if the player is online
            if(Player * pl = ObjectAccessor::FindPlayer(*itr))
            {
                if(!m_capturePoint->IsWithinDistInMap(pl, m_capturePoint->GetGOInfo()->capturePoint.radius))
                    HandlePlayerLeave(pl);
            }
            else
            {
                TC_LOG_ERROR("FIXME","Player (" UI64FMTD ") offline, bit still in outdoor pvp, this should never happen.",(*itr));
            } 
        }
    }
}

bool OPvPCapturePoint::Update(uint32 diff)
{
    if (!m_capturePoint)
        return false;

    float radius = (float)m_capturePoint->GetGOInfo()->capturePoint.radius;

    UpdateActivePlayerProximityCheck();

    std::list<Player*> players;
    Trinity::AnyPlayerInObjectRangeCheck checker(m_capturePoint, radius);
    Trinity::PlayerListSearcher<Trinity::AnyPlayerInObjectRangeCheck> searcher(m_capturePoint, players, checker);
    Cell::VisitWorldObjects(m_capturePoint, searcher, radius);

    for (std::list<Player*>::iterator itr = players.begin(); itr != players.end(); ++itr)
    {
        Player* const player = *itr;
        if (player->IsOutdoorPvPActive())
        {
            if (m_activePlayers[player->GetTeamId()].insert(player->GetGUID()).second)
                HandlePlayerEnter(*itr);
        }
    }

    // get the difference of numbers
    float fact_diff = ((float)m_activePlayers[0].size() - (float)m_activePlayers[1].size());

    uint32 Challenger = 0;

    if(fact_diff < 0)
    {
        // horde is in majority, but it's already horde-controlled -> no change
        if (m_State == OBJECTIVESTATE_HORDE && m_value == -m_maxValue)
            return false;

        if(fact_diff < - m_maxSpeed)
            fact_diff = - m_maxSpeed;

        Challenger = HORDE;
    }
    else if(fact_diff > 0)
    {
        // ally is in majority, but it's already ally-controlled -> no change
        if (m_State == OBJECTIVESTATE_ALLIANCE && m_value == m_maxValue)
            return false;

        if(fact_diff > m_maxSpeed)
            fact_diff = m_maxSpeed;

        Challenger = ALLIANCE;
    }
    else /*if(fact_diff==0)*/ // no change
        return false;

    float oldValue = m_value;
    TeamId oldTeam = m_team;

    m_OldState = m_State;

    m_value += fact_diff;

    // check limits, these are over the grey part
    if(m_value <= -m_minValue) // red
    {
        if(m_value <= - m_maxValue)
            m_value = - m_maxValue;
        m_State = OBJECTIVESTATE_HORDE;
        m_team = TEAM_HORDE;
    }
    else if(m_value >= m_minValue) //blue
    {
        if(m_value >= m_maxValue)
            m_value = m_maxValue;
        m_State = OBJECTIVESTATE_ALLIANCE;
        m_team = TEAM_ALLIANCE;
    } else if (oldValue * m_value <=0) // grey, go through mid point
    {
        // if challenger is ally, then n->a challenge
        if(Challenger == ALLIANCE)
            m_State = OBJECTIVESTATE_NEUTRAL_ALLIANCE_CHALLENGE;
        // if challenger is horde, then n->h challenge
        else if(Challenger == HORDE)
            m_State = OBJECTIVESTATE_NEUTRAL_HORDE_CHALLENGE;
        m_team = TEAM_NEUTRAL;
    }
    else // grey, did not go through mid point
    {
        // old phase and current are on the same side, so one team challenges the other
        if(Challenger == ALLIANCE && (m_OldState == OBJECTIVESTATE_HORDE || m_OldState == OBJECTIVESTATE_NEUTRAL_HORDE_CHALLENGE))
            m_State = OBJECTIVESTATE_HORDE_ALLIANCE_CHALLENGE;
        else if(Challenger == HORDE && (m_OldState == OBJECTIVESTATE_ALLIANCE || m_OldState == OBJECTIVESTATE_NEUTRAL_ALLIANCE_CHALLENGE))
            m_State = OBJECTIVESTATE_ALLIANCE_HORDE_CHALLENGE;
        m_team = TEAM_NEUTRAL;
    }

    if (m_value != oldValue)
        SendChangePhase();

    if (m_OldState != m_State)
    {
        //TC_LOG_ERROR("outdoorpvp", "%u->%u", m_OldState, m_State);
        if (oldTeam != m_team)
            ChangeTeam(oldTeam);
        ChangeState();
        return true;
    }

    return false;
}

void OutdoorPvP::SendUpdateWorldState(uint32 field, uint32 value)
{
    // send to both factions
    for(auto & m_PlayerGuid : m_players)
    {
        // send to all players present in the area
        for(auto itr = m_PlayerGuid.begin(); itr != m_PlayerGuid.end(); ++itr)
        {
            Player * plr = sObjectMgr->GetPlayer(*itr);
            if(plr)
            {
                plr->SendUpdateWorldState(field,value);
            }
        }
    }
}

void OPvPCapturePoint::SendUpdateWorldState(uint32 field, uint32 value)
{
    for(auto & m_ActivePlayerGuid : m_activePlayers)
    {
        // send to all players present in the area
        for(auto itr = m_ActivePlayerGuid.begin(); itr != m_ActivePlayerGuid.end(); ++itr)
        {
            Player * plr = sObjectMgr->GetPlayer(*itr);
            if(plr)
            {
                plr->SendUpdateWorldState(field,value);
            }
        }
    }
}

void OPvPCapturePoint::SendObjectiveComplete(uint32 id,ObjectGuid guid)
{
    uint32 team;
    switch(m_State)
    {
    case OBJECTIVESTATE_ALLIANCE:
        team = 0;
        break;
    case OBJECTIVESTATE_HORDE:
        team = 1;
        break;
    default:
        return;
        break;
    }

    // send to all players present in the area
    for(uint64 itr : m_activePlayers[team])
        if(Player* plr = sObjectMgr->GetPlayer(itr))
            plr->KilledMonsterCredit(id,guid);
}

void OutdoorPvP::HandleKill(Player *killer, Unit * killed)
{
    if(Group * pGroup = killer->GetGroup())
    {
        for(GroupReference *itr = pGroup->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player *pGroupGuy = itr->GetSource();

            if(!pGroupGuy)
                continue;

            // skip if too far away
            if(!pGroupGuy->IsAtGroupRewardDistance(killed))
                continue;

            // creature kills must be notified, even if not inside objective / not outdoor pvp active
            // player kills only count if active and inside objective
            if(( pGroupGuy->IsOutdoorPvPActive() && IsInsideObjective(pGroupGuy) ) || killed->GetTypeId() == TYPEID_UNIT)
            {
                HandleKillImpl(pGroupGuy, killed);
            }
        }
    }
    else
    {
        // creature kills must be notified, even if not inside objective / not outdoor pvp active
        if(killer && (( killer->IsOutdoorPvPActive() && IsInsideObjective(killer) ) || killed->GetTypeId() == TYPEID_UNIT))
        {
            HandleKillImpl(killer, killed);
        }
    }
}

bool OutdoorPvP::IsInsideObjective(Player* player)
{
    for (OPvPCapturePointMap::const_iterator itr = m_capturePoints.begin(); itr != m_capturePoints.end(); ++itr)
        if (itr->second->IsInsideObjective(player))
            return true;

    return false;
}

bool OPvPCapturePoint::IsInsideObjective(Player *plr)
{
    auto itr = m_activePlayers[plr->GetTeamId()].find(plr->GetGUID());
    return itr != m_activePlayers[plr->GetTeamId()].end();
}

bool OutdoorPvP::HandleCustomSpell(Player *player, uint32 spellId, GameObject * go)
{
    for (OPvPCapturePointMap::iterator itr = m_capturePoints.begin(); itr != m_capturePoints.end(); ++itr)
        if (itr->second->HandleCustomSpell(player, spellId, go))
            return true;

    return false;
}

bool OPvPCapturePoint::HandleCustomSpell(Player *player, uint32 spellId, GameObject * go)
{
    if(!player->IsOutdoorPvPActive())
        return false;
    return false;
}

bool OutdoorPvP::HandleOpenGo(Player *player, ObjectGuid guid)
{
    for(auto & m_OPvPCapturePoint : m_capturePoints)
    {
        if(m_OPvPCapturePoint.second->HandleOpenGo(player,guid) >= 0)
            return true;
    }
    return false;
}

bool OutdoorPvP::HandleGossipOption(Player * plr, ObjectGuid guid, uint32 id)
{
    for(auto & m_OPvPCapturePoint : m_capturePoints)
    {
        if(m_OPvPCapturePoint.second->HandleGossipOption(plr, guid, id))
            return true;
    }
    return false;
}

bool OutdoorPvP::CanTalkTo(Player * plr, Creature * c, GossipMenuItems const& gso)
{
    for(auto & m_OPvPCapturePoint : m_capturePoints)
    {
        if(m_OPvPCapturePoint.second->CanTalkTo(plr, c, gso))
            return true;
    }
    return false;
}

bool OutdoorPvP::HandleDropFlag(Player * plr, uint32 id)
{
    for(auto & m_OPvPCapturePoint : m_capturePoints)
    {
        if(m_OPvPCapturePoint.second->HandleDropFlag(plr, id))
            return true;
    }
    return false;
}

bool OPvPCapturePoint::HandleGossipOption(Player * plr, ObjectGuid guid, uint32 id)
{
    return false;
}

bool OPvPCapturePoint::CanTalkTo(Player * plr, Creature * c, GossipMenuItems const& gso)
{
    return false;
}

bool OPvPCapturePoint::HandleDropFlag(Player * plr, uint32 id)
{
    return false;
}

int32 OPvPCapturePoint::HandleOpenGo(Player *plr, ObjectGuid guid)
{
    auto itr = m_ObjectTypes.find(guid);
    if(itr != m_ObjectTypes.end())
    {
        return itr->second;
    }
    return -1;
}

bool OutdoorPvP::HandleAreaTrigger(Player *plr, uint32 trigger)
{
    return false;
}

void OutdoorPvP::SetMapFromZone(uint32 zone)
{
    AreaTableEntry const* areaTable = sAreaTableStore.LookupEntry(zone);
    ASSERT(areaTable);
    Map* map = sMapMgr->CreateBaseMap(areaTable->mapid);
    ASSERT(!map->Instanceable());
    m_map = map;
}


void OutdoorPvP::OnGameObjectCreate(GameObject* go)
{
    GoScriptPair sp(go->GetGUID().GetCounter(), go);
    m_GoScriptStore.insert(sp);
    if (go->GetGoType() != GAMEOBJECT_TYPE_CAPTURE_POINT)
        return;

    if (OPvPCapturePoint *cp = GetCapturePoint(go->GetSpawnId()))
        cp->m_capturePoint = go;
}

void OutdoorPvP::OnGameObjectRemove(GameObject* go)
{
    m_GoScriptStore.erase(go->GetGUID().GetCounter());

    if (go->GetGoType() != GAMEOBJECT_TYPE_CAPTURE_POINT)
        return;

    if (OPvPCapturePoint *cp = GetCapturePoint(go->GetSpawnId()))
        cp->m_capturePoint = nullptr;
}

void OutdoorPvP::OnCreatureCreate(Creature* creature)
{
    CreatureScriptPair sp(creature->GetGUID().GetCounter(), creature);
    m_CreatureScriptStore.insert(sp);
}

void OutdoorPvP::OnCreatureRemove(Creature* creature)
{
    m_CreatureScriptStore.erase(creature->GetGUID().GetCounter());
}

void OutdoorPvP::SendDefenseMessage(uint32 zoneId, uint32 id)
{
/*NYI
    DefenseMessageBuilder builder(zoneId, id);
    Trinity::LocalizedPacketDo<DefenseMessageBuilder> localizer(builder);
    BroadcastWorker(localizer, zoneId);
    */
}