
#include "OutdoorPvPMgr.h"
#include "Player.h"
#include "MapManager.h"
#include "ScriptMgr.h"

OutdoorPvPMgr::OutdoorPvPMgr()
{
    m_UpdateTimer = 0;
}

void OutdoorPvPMgr::Die()
{
    //TC_LOG_DEBUG("outdoorpvp", "Deleting OutdoorPvPMgr");
    for (auto & itr : m_OutdoorPvPSet)
        delete itr;

    m_OutdoorPvPSet.clear();

    m_OutdoorPvPDatas.fill(0);

    m_OutdoorPvPMap.clear();
}

void OutdoorPvPMgr::InitOutdoorPvP()
{
    uint32 oldMSTime = GetMSTime();

    //                                                 0       1
    QueryResult result = WorldDatabase.Query("SELECT TypeId, ScriptName FROM outdoorpvp_template");
    if (!result)
    {
        TC_LOG_INFO("server.loading", ">> Loaded 0 outdoor PvP definitions. DB table `outdoorpvp_template` is empty.");
        return;
    }

    uint32 count = 0;
    uint32 typeId = 0;

    do
    {
        Field* fields = result->Fetch();

        typeId = fields[0].GetUInt8();

        /*TC if (DisableMgr::IsDisabledFor(DISABLE_TYPE_OUTDOORPVP, typeId, nullptr))
            continue;*/

        if (typeId >= MAX_OUTDOORPVP_TYPES)
        {
            TC_LOG_ERROR("sql.sql", "Invalid OutdoorPvPTypes value %u in outdoorpvp_template; skipped.", typeId);
            continue;
        }

        OutdoorPvPTypes realTypeId = OutdoorPvPTypes(typeId);
        m_OutdoorPvPDatas[realTypeId] = sObjectMgr->GetScriptId(fields[1].GetString());

        ++count;
    } while (result->NextRow());

    OutdoorPvP* pvp;
    for (uint8 i = 1; i < MAX_OUTDOORPVP_TYPES; ++i)
    {
        if (!m_OutdoorPvPDatas[i])
        {
            TC_LOG_ERROR("sql.sql", "Could not initialize OutdoorPvP object for type ID %u; no entry in database.", uint32(i));
            continue;
        }

        pvp = sScriptMgr->CreateOutdoorPvP(m_OutdoorPvPDatas[i]);
        if (!pvp)
        {
            TC_LOG_ERROR("outdoorpvp", "Could not initialize OutdoorPvP object for type ID %u; got NULL pointer from script.", uint32(i));
            continue;
        }

        if (!pvp->SetupOutdoorPvP())
        {
            TC_LOG_ERROR("outdoorpvp", "Could not initialize OutdoorPvP object for type ID %u; SetupOutdoorPvP failed.", uint32(i));
            delete pvp;
            continue;
        }

        m_OutdoorPvPSet.push_back(pvp);
    }

    TC_LOG_INFO("server.loading", ">> Loaded %u outdoor PvP definitions in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void OutdoorPvPMgr::AddZone(uint32 zoneid, OutdoorPvP *handle)
{
    m_OutdoorPvPMap[zoneid] = handle;
}

ZoneScript* OutdoorPvPMgr::GetZoneScript(uint32 zoneId)
{
    OutdoorPvPMap::iterator itr = m_OutdoorPvPMap.find(zoneId);
    if (itr != m_OutdoorPvPMap.end())
        return itr->second;
    else
        return nullptr;
}

void OutdoorPvPMgr::HandlePlayerEnterZone(Player* player, uint32 zoneid)
{
    auto itr = m_OutdoorPvPMap.find(zoneid);
    if(itr == m_OutdoorPvPMap.end())
        return;

    if (itr->second->HasPlayer(player))
        return;

    // add possibly beneficial buffs to plr for zone
    itr->second->HandlePlayerEnterZone(player, zoneid);
}

void OutdoorPvPMgr::HandlePlayerLeaveZone(Player* player, uint32 zoneid)
{
    auto itr = m_OutdoorPvPMap.find(zoneid);
    if(itr == m_OutdoorPvPMap.end())
        return;

    // teleport: remove once in removefromworld, once in updatezone
    if (!itr->second->HasPlayer(player))
        return;

    // inform the OutdoorPvP class of the leaving, it should remove the player from all objectives
    itr->second->HandlePlayerLeaveZone(player, zoneid);
}

OutdoorPvP * OutdoorPvPMgr::GetOutdoorPvPToZoneId(uint32 zoneid)
{
    auto itr = m_OutdoorPvPMap.find(zoneid);
    if(itr == m_OutdoorPvPMap.end())
    {
        // no handle for this zone, return
        return nullptr;
    }
    return itr->second;
}

void OutdoorPvPMgr::Update(uint32 diff)
{
    if(m_UpdateTimer < diff)
    {
        for(auto & itr : m_OutdoorPvPSet)
            itr->Update(diff);

        m_UpdateTimer = OUTDOORPVP_OBJECTIVE_UPDATE_INTERVAL;
    } else m_UpdateTimer -= diff;
}

bool OutdoorPvPMgr::HandleCustomSpell(Player *plr, uint32 spellId, GameObject * go)
{
    for(auto & itr : m_OutdoorPvPSet)
    {
        if(itr->HandleCustomSpell(plr,spellId,go))
            return true;
    }
    return false;
}

bool OutdoorPvPMgr::HandleOpenGo(Player *plr, ObjectGuid guid)
{
    for(auto & itr : m_OutdoorPvPSet)
    {
        if(itr->HandleOpenGo(plr,guid))
            return true;
    }
    return false;
}

void OutdoorPvPMgr::HandleGossipOption(Player *plr, ObjectGuid guid, uint32 gossipid)
{
    for(auto & itr : m_OutdoorPvPSet)
    {
        if(itr->HandleGossipOption(plr,guid,gossipid))
            return;
    }
}

bool OutdoorPvPMgr::CanTalkTo(Player * plr, Creature * c, GossipMenuItems const& gso)
{
    for(auto & itr : m_OutdoorPvPSet)
    {
        if(itr->CanTalkTo(plr,c,gso))
            return true;
    }
    return false;
}

void OutdoorPvPMgr::HandleDropFlag(Player *plr, uint32 spellId)
{
    for(auto & itr : m_OutdoorPvPSet)
    {
        if(itr->HandleDropFlag(plr,spellId))
            return;
    }
}

void OutdoorPvPMgr::HandlePlayerResurrects(Player* player, uint32 zoneid)
{
    auto itr = m_OutdoorPvPMap.find(zoneid);
    if (itr == m_OutdoorPvPMap.end())
        return;

    if (itr->second->HasPlayer(player))
        itr->second->HandlePlayerResurrects(player, zoneid);
}

std::string OutdoorPvPMgr::GetDefenseMessage(uint32 zoneId, uint32 id, LocaleConstant locale) const
{
    if (BroadcastText const* bct = sObjectMgr->GetBroadcastText(id))
        return bct->GetText(locale);

    TC_LOG_ERROR("outdoorpvp", "Can not find DefenseMessage (Zone: %u, Id: %u). BroadcastText (Id: %u) does not exist.", zoneId, id, id);
    return "";
}