/*
 * Copyright (C) 2008 Trinity <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "OutdoorPvPMgr.h"
#include "OutdoorPvPHP.h"
#include "OutdoorPvPNA.h"
#include "OutdoorPvPTF.h"
#include "OutdoorPvPZM.h"
#include "OutdoorPvPSI.h"
#include "OutdoorPvPEP.h"
#include "Player.h"

OutdoorPvPMgr::OutdoorPvPMgr()
{
    m_UpdateTimer = 0;
}

void OutdoorPvPMgr::Die()
{
    //TC_LOG_DEBUG("outdoorpvp", "Deleting OutdoorPvPMgr");
    for (OutdoorPvPSet::iterator itr = m_OutdoorPvPSet.begin(); itr != m_OutdoorPvPSet.end(); ++itr)
        delete *itr;
    /*
    for (OutdoorPvPDataMap::iterator itr = m_OutdoorPvPDatas.begin(); itr != m_OutdoorPvPDatas.end(); ++itr)
        delete itr->second;*/
}

void OutdoorPvPMgr::InitOutdoorPvP()
{
    // create new opvp
    OutdoorPvP * pOP = new OutdoorPvPHP;
    // respawn, init variables
    if(!pOP->SetupOutdoorPvP())
    {
        TC_LOG_ERROR("FIXME","OutdoorPvP : HP init failed.");
        delete pOP;
    }
    else
    {
        m_OutdoorPvPSet.push_back(pOP);
    }


    pOP = new OutdoorPvPNA;
    // respawn, init variables
    if(!pOP->SetupOutdoorPvP())
    {
        TC_LOG_ERROR("FIXME","OutdoorPvP : NA init failed.");
        delete pOP;
    }
    else
    {
        m_OutdoorPvPSet.push_back(pOP);
    }


    pOP = new OutdoorPvPTF;
    // respawn, init variables
    if(!pOP->SetupOutdoorPvP())
    {
        TC_LOG_ERROR("FIXME","OutdoorPvP : TF init failed.");
        delete pOP;
    }
    else
    {
        m_OutdoorPvPSet.push_back(pOP);
    }

    pOP = new OutdoorPvPZM;
    // respawn, init variables
    if(!pOP->SetupOutdoorPvP())
    {
        TC_LOG_ERROR("FIXME","OutdoorPvP : ZM init failed.");
        delete pOP;
    }
    else
    {
        m_OutdoorPvPSet.push_back(pOP);
    }

    pOP = new OutdoorPvPSI;
    // respawn, init variables
    if(!pOP->SetupOutdoorPvP())
    {
        TC_LOG_ERROR("FIXME","OutdoorPvP : SI init failed.");
        delete pOP;
    }
    else
    {
        m_OutdoorPvPSet.push_back(pOP);
    }

    pOP = new OutdoorPvPEP;
    // respawn, init variables
    if(!pOP->SetupOutdoorPvP())
    {
        TC_LOG_ERROR("FIXME","OutdoorPvP : EP init failed.");
        delete pOP;
    }
    else
    {
        m_OutdoorPvPSet.push_back(pOP);
    }
}

void OutdoorPvPMgr::AddZone(uint32 zoneid, OutdoorPvP *handle)
{
    m_OutdoorPvPMap[zoneid] = handle;
}


void OutdoorPvPMgr::HandlePlayerEnterZone(Player *plr, uint32 zoneid)
{
    OutdoorPvPMap::iterator itr = m_OutdoorPvPMap.find(zoneid);
    if(itr == m_OutdoorPvPMap.end())
    {
        // no handle for this zone, return
        return;
    }
    // add possibly beneficial buffs to plr for zone
    itr->second->HandlePlayerEnterZone(plr, zoneid);
    plr->SendInitWorldStates();
}

void OutdoorPvPMgr::HandlePlayerLeaveZone(Player *plr, uint32 zoneid)
{
    OutdoorPvPMap::iterator itr = m_OutdoorPvPMap.find(zoneid);
    if(itr == m_OutdoorPvPMap.end())
    {
        // no handle for this zone, return
        return;
    }
    // inform the OutdoorPvP class of the leaving, it should remove the player from all objectives
    itr->second->HandlePlayerLeaveZone(plr, zoneid);
}

OutdoorPvP * OutdoorPvPMgr::GetOutdoorPvPToZoneId(uint32 zoneid)
{
    OutdoorPvPMap::iterator itr = m_OutdoorPvPMap.find(zoneid);
    if(itr == m_OutdoorPvPMap.end())
    {
        // no handle for this zone, return
        return NULL;
    }
    return itr->second;
}

void OutdoorPvPMgr::Update(uint32 diff)
{
    if(m_UpdateTimer < diff)
    {
        for(OutdoorPvPSet::iterator itr = m_OutdoorPvPSet.begin(); itr != m_OutdoorPvPSet.end(); ++itr)
        {
            (*itr)->Update(diff);
        }
        m_UpdateTimer = OUTDOORPVP_OBJECTIVE_UPDATE_INTERVAL;
    } else m_UpdateTimer -= diff;
}

bool OutdoorPvPMgr::HandleCustomSpell(Player *plr, uint32 spellId, GameObject * go)
{
    for(OutdoorPvPSet::iterator itr = m_OutdoorPvPSet.begin(); itr != m_OutdoorPvPSet.end(); ++itr)
    {
        if((*itr)->HandleCustomSpell(plr,spellId,go))
            return true;
    }
    return false;
}

bool OutdoorPvPMgr::HandleOpenGo(Player *plr, uint64 guid)
{
    for(OutdoorPvPSet::iterator itr = m_OutdoorPvPSet.begin(); itr != m_OutdoorPvPSet.end(); ++itr)
    {
        if((*itr)->HandleOpenGo(plr,guid))
            return true;
    }
    return false;
}

bool OutdoorPvPMgr::HandleCaptureCreaturePlayerMoveInLos(Player * plr, Creature * c)
{
    for(OutdoorPvPSet::iterator itr = m_OutdoorPvPSet.begin(); itr != m_OutdoorPvPSet.end(); ++itr)
    {
        if((*itr)->HandleCaptureCreaturePlayerMoveInLos(plr,c))
            return true;
    }
    return false;
}

void OutdoorPvPMgr::HandleGossipOption(Player *plr, uint64 guid, uint32 gossipid)
{
    for(OutdoorPvPSet::iterator itr = m_OutdoorPvPSet.begin(); itr != m_OutdoorPvPSet.end(); ++itr)
    {
        if((*itr)->HandleGossipOption(plr,guid,gossipid))
            return;
    }
}

bool OutdoorPvPMgr::CanTalkTo(Player * plr, Creature * c, GossipMenuItems const& gso)
{
    for(OutdoorPvPSet::iterator itr = m_OutdoorPvPSet.begin(); itr != m_OutdoorPvPSet.end(); ++itr)
    {
        if((*itr)->CanTalkTo(plr,c,gso))
            return true;
    }
    return false;
}

void OutdoorPvPMgr::HandleDropFlag(Player *plr, uint32 spellId)
{
    for(OutdoorPvPSet::iterator itr = m_OutdoorPvPSet.begin(); itr != m_OutdoorPvPSet.end(); ++itr)
    {
        if((*itr)->HandleDropFlag(plr,spellId))
            return;
    }
}

