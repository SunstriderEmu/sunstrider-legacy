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

#include "OutdoorPvPHP.h"
#include "OutdoorPvP.h"
#include "Player.h"
#include "WorldPacket.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Language.h"
#include "MapManager.h"
#include "ScriptMgr.h"

uint32 const HP_LANG_CAPTURE_A[HP_TOWER_NUM] = { TEXT_BROKEN_HILL_TAKEN_ALLIANCE, TEXT_OVERLOOK_TAKEN_ALLIANCE, TEXT_STADIUM_TAKEN_ALLIANCE };
uint32 const HP_LANG_CAPTURE_H[HP_TOWER_NUM] = { TEXT_BROKEN_HILL_TAKEN_HORDE, TEXT_OVERLOOK_TAKEN_HORDE, TEXT_STADIUM_TAKEN_HORDE };

OPvPCapturePointHP::OPvPCapturePointHP(OutdoorPvP *pvp,OutdoorPvPHPTowerType type)
: OPvPCapturePoint(pvp), m_TowerType(type)
{
    SetCapturePointData(HPCapturePoints[type].entry,
        HPCapturePoints[type].map,
        HPCapturePoints[type].x,
        HPCapturePoints[type].y,
        HPCapturePoints[type].z,
        HPCapturePoints[type].o,
        HPCapturePoints[type].rot0,
        HPCapturePoints[type].rot1,
        HPCapturePoints[type].rot2,
        HPCapturePoints[type].rot3);
    AddObject(type,
        HPTowerFlags[type].entry,
        HPTowerFlags[type].map,
        HPTowerFlags[type].x,
        HPTowerFlags[type].y,
        HPTowerFlags[type].z,
        HPTowerFlags[type].o,
        HPTowerFlags[type].rot0,
        HPTowerFlags[type].rot1,
        HPTowerFlags[type].rot2,
        HPTowerFlags[type].rot3);
}

OutdoorPvPHP::OutdoorPvPHP()
{
    m_TypeId = OUTDOOR_PVP_HP;
}

bool OutdoorPvPHP::SetupOutdoorPvP()
{
    m_AllianceTowersControlled = 0;
    m_HordeTowersControlled = 0;
    // add the zones affected by the pvp buff
    for(uint32 OutdoorPvPHPBuffZone : OutdoorPvPHPBuffZones)
        RegisterZone(OutdoorPvPHPBuffZone);

    AddCapturePoint(new OPvPCapturePointHP(this, HP_TOWER_BROKEN_HILL));
    AddCapturePoint(new OPvPCapturePointHP(this, HP_TOWER_OVERLOOK));
    AddCapturePoint(new OPvPCapturePointHP(this, HP_TOWER_STADIUM));

    SetMapFromZone(OutdoorPvPHPBuffZones[0]);

    return true;
}

void OutdoorPvPHP::HandlePlayerEnterZone(Player * plr, uint32 zone)
{
    // add buffs
    if(plr->GetTeam() == ALLIANCE)
    {
        if(m_AllianceTowersControlled >=3)
            plr->CastSpell(plr,AllianceBuff, true);
    }
    else
    {
        if(m_HordeTowersControlled >=3)
            plr->CastSpell(plr,HordeBuff, true);
    }
    OutdoorPvP::HandlePlayerEnterZone(plr,zone);
}

void OutdoorPvPHP::HandlePlayerLeaveZone(Player * plr, uint32 zone)
{
    // remove buffs
    if(plr->GetTeam() == ALLIANCE)
    {
        plr->RemoveAurasDueToSpell(AllianceBuff);
    }
    else
    {
        plr->RemoveAurasDueToSpell(HordeBuff);
    }
    OutdoorPvP::HandlePlayerLeaveZone(plr, zone);
}

bool OutdoorPvPHP::Update(uint32 diff)
{
    bool changed = false;
    if((changed = OutdoorPvP::Update(diff)))
    {
        if(m_AllianceTowersControlled == 3)
            BuffTeam(ALLIANCE);
        else if(m_HordeTowersControlled == 3)
            BuffTeam(HORDE);
        else
            BuffTeam(0);

        SendUpdateWorldState(HP_UI_TOWER_COUNT_A, m_AllianceTowersControlled);
        SendUpdateWorldState(HP_UI_TOWER_COUNT_H, m_HordeTowersControlled);
    }
    return changed;
}

void OutdoorPvPHP::SendRemoveWorldStates(Player *plr)
{
    plr->SendUpdateWorldState(HP_UI_TOWER_DISPLAY_A, WORLD_STATE_REMOVE);
    plr->SendUpdateWorldState(HP_UI_TOWER_DISPLAY_H, WORLD_STATE_REMOVE);
    plr->SendUpdateWorldState(HP_UI_TOWER_COUNT_H, WORLD_STATE_REMOVE);
    plr->SendUpdateWorldState(HP_UI_TOWER_COUNT_A, WORLD_STATE_REMOVE);
    /*plr->SendUpdateWorldState(HP_UI_TOWER_SLIDER_N, WORLD_STATE_REMOVE);
    plr->SendUpdateWorldState(HP_UI_TOWER_SLIDER_POS, WORLD_STATE_REMOVE);
    plr->SendUpdateWorldState(HP_UI_TOWER_SLIDER_DISPLAY, WORLD_STATE_REMOVE);*/
    for (uint8 i = 0; i < HP_TOWER_NUM; ++i)
    {
        plr->SendUpdateWorldState(HP_MAP_N[i], WORLD_STATE_REMOVE);
        plr->SendUpdateWorldState(HP_MAP_A[i], WORLD_STATE_REMOVE);
        plr->SendUpdateWorldState(HP_MAP_H[i], WORLD_STATE_REMOVE);
    }
}

void OutdoorPvPHP::FillInitialWorldStates(WorldPacket &data)
{
    /*
    {
        data << uint32(0x9ba) << uint32(0x1);           // 10 // add ally tower main gui icon       // maybe should be sent only on login?
        data << uint32(0x9b9) << uint32(0x1);           // 11 // add horde tower main gui icon      // maybe should be sent only on login?
        data << uint32(0x9b5) << uint32(0x0);           // 12 // show neutral broken hill icon      // 2485
        data << uint32(0x9b4) << uint32(0x1);           // 13 // show icon above broken hill        // 2484
        data << uint32(0x9b3) << uint32(0x0);           // 14 // show ally broken hill icon         // 2483
        data << uint32(0x9b2) << uint32(0x0);           // 15 // show neutral overlook icon         // 2482
        data << uint32(0x9b1) << uint32(0x1);           // 16 // show the overlook arrow            // 2481
        data << uint32(0x9b0) << uint32(0x0);           // 17 // show ally overlook icon            // 2480
        data << uint32(0x9ae) << uint32(0x0);           // 18 // horde pvp objectives captured      // 2478
        data << uint32(0x9ac) << uint32(0x0);           // 19 // ally pvp objectives captured       // 2476
        data << uint32(2475)  << uint32(100); //: ally / horde slider grey area                              // show only in direct vicinity!
        data << uint32(2474)  << uint32(50);  //: ally / horde slider percentage, 100 for ally, 0 for horde  // show only in direct vicinity!
        data << uint32(2473)  << uint32(0);   //: ally / horde slider display                                // show only in direct vicinity!
        data << uint32(0x9a8) << uint32(0x0);           // 20 // show the neutral stadium icon      // 2472
        data << uint32(0x9a7) << uint32(0x0);           // 21 // show the ally stadium icon         // 2471
        data << uint32(0x9a6) << uint32(0x1);           // 22 // show the horde stadium icon        // 2470
    }
    */
    data << uint32(HP_UI_TOWER_DISPLAY_A) << uint32(WORLD_STATE_ADD);
    data << uint32(HP_UI_TOWER_DISPLAY_H) << uint32(WORLD_STATE_ADD);
    data << uint32(HP_UI_TOWER_COUNT_A) << uint32(m_AllianceTowersControlled);
    data << uint32(HP_UI_TOWER_COUNT_H) << uint32(m_HordeTowersControlled);

    for(auto & m_OPvPCapturePoint : m_capturePoints)
        m_OPvPCapturePoint.second->FillInitialWorldStates(data);
}

uint32 OutdoorPvPHP::GetAllianceTowersControlled() const
{
    return m_AllianceTowersControlled;
}

void OutdoorPvPHP::SetAllianceTowersControlled(uint32 count)
{
    m_AllianceTowersControlled = count;
}

uint32 OutdoorPvPHP::GetHordeTowersControlled() const
{
    return m_HordeTowersControlled;
}

void OutdoorPvPHP::SetHordeTowersControlled(uint32 count)
{
    m_HordeTowersControlled = count;
}

void OPvPCapturePointHP::ChangeState()
{
    uint32 field = 0;
    switch (m_OldState)
    {
    case OBJECTIVESTATE_NEUTRAL:
        field = HP_MAP_N[m_TowerType];
        break;
    case OBJECTIVESTATE_ALLIANCE:
        field = HP_MAP_A[m_TowerType];
        if (uint32 alliance_towers = ((OutdoorPvPHP*)m_PvP)->GetAllianceTowersControlled())
            ((OutdoorPvPHP*)m_PvP)->SetAllianceTowersControlled(--alliance_towers);
        break;
    case OBJECTIVESTATE_HORDE:
        field = HP_MAP_H[m_TowerType];
        if (uint32 horde_towers = ((OutdoorPvPHP*)m_PvP)->GetHordeTowersControlled())
            ((OutdoorPvPHP*)m_PvP)->SetHordeTowersControlled(--horde_towers);
        break;
    case OBJECTIVESTATE_NEUTRAL_ALLIANCE_CHALLENGE:
        field = HP_MAP_N[m_TowerType];
        break;
    case OBJECTIVESTATE_NEUTRAL_HORDE_CHALLENGE:
        field = HP_MAP_N[m_TowerType];
        break;
    case OBJECTIVESTATE_ALLIANCE_HORDE_CHALLENGE:
        field = HP_MAP_A[m_TowerType];
        break;
    case OBJECTIVESTATE_HORDE_ALLIANCE_CHALLENGE:
        field = HP_MAP_H[m_TowerType];
        break;
    }

    // send world state update
    if (field)
    {
        m_PvP->SendUpdateWorldState(field, WORLD_STATE_REMOVE);
        field = 0;
    }
    uint32 artkit = 21;
    uint32 artkit2 = HP_TowerArtKit_N[m_TowerType];
    switch (m_State)
    {
    case OBJECTIVESTATE_NEUTRAL:
        field = HP_MAP_N[m_TowerType];
        break;
    case OBJECTIVESTATE_ALLIANCE:
    {
        field = HP_MAP_A[m_TowerType];
        artkit = 2;
        artkit2 = HP_TowerArtKit_A[m_TowerType];
        uint32 alliance_towers = ((OutdoorPvPHP*)m_PvP)->GetAllianceTowersControlled();
        if (alliance_towers < 3)
            ((OutdoorPvPHP*)m_PvP)->SetAllianceTowersControlled(++alliance_towers);
        m_PvP->SendDefenseMessage(OutdoorPvPHPBuffZones[0], HP_LANG_CAPTURE_A[m_TowerType]);
        break;
    }
    case OBJECTIVESTATE_HORDE:
    {
        field = HP_MAP_H[m_TowerType];
        artkit = 1;
        artkit2 = HP_TowerArtKit_H[m_TowerType];
        uint32 horde_towers = ((OutdoorPvPHP*)m_PvP)->GetHordeTowersControlled();
        if (horde_towers < 3)
            ((OutdoorPvPHP*)m_PvP)->SetHordeTowersControlled(++horde_towers);
        m_PvP->SendDefenseMessage(OutdoorPvPHPBuffZones[0], HP_LANG_CAPTURE_H[m_TowerType]);
        break;
    }
    case OBJECTIVESTATE_NEUTRAL_ALLIANCE_CHALLENGE:
        field = HP_MAP_N[m_TowerType];
        break;
    case OBJECTIVESTATE_NEUTRAL_HORDE_CHALLENGE:
        field = HP_MAP_N[m_TowerType];
        break;
    case OBJECTIVESTATE_ALLIANCE_HORDE_CHALLENGE:
        field = HP_MAP_A[m_TowerType];
        artkit = 2;
        artkit2 = HP_TowerArtKit_A[m_TowerType];
        break;
    case OBJECTIVESTATE_HORDE_ALLIANCE_CHALLENGE:
        field = HP_MAP_H[m_TowerType];
        artkit = 1;
        artkit2 = HP_TowerArtKit_H[m_TowerType];
        break;
    }

    Map* map = sMapMgr->FindMap(530, 0);
    auto bounds = map->GetGameObjectBySpawnIdStore().equal_range(m_capturePointSpawnId);
    for (auto itr = bounds.first; itr != bounds.second; ++itr)
        itr->second->SetGoArtKit(artkit);

    bounds = map->GetGameObjectBySpawnIdStore().equal_range(m_Objects[m_TowerType]);
    for (auto itr = bounds.first; itr != bounds.second; ++itr)
        itr->second->SetGoArtKit(artkit2);

    // send world state update
    if (field)
        m_PvP->SendUpdateWorldState(field, WORLD_STATE_ADD);

    // complete quest objective
    if (m_State == OBJECTIVESTATE_ALLIANCE || m_State == OBJECTIVESTATE_HORDE)
        SendObjectiveComplete(HP_CREDITMARKER[m_TowerType], ObjectGuid::Empty);
}

void OPvPCapturePointHP::FillInitialWorldStates(WorldPacket &data)
{
    switch(m_State)
    {
        case OBJECTIVESTATE_ALLIANCE:
        case OBJECTIVESTATE_ALLIANCE_HORDE_CHALLENGE:
            data << uint32(HP_MAP_N[m_TowerType]) << uint32(WORLD_STATE_REMOVE);
            data << uint32(HP_MAP_A[m_TowerType]) << uint32(WORLD_STATE_ADD);
            data << uint32(HP_MAP_H[m_TowerType]) << uint32(WORLD_STATE_REMOVE);
            break;
        case OBJECTIVESTATE_HORDE:
        case OBJECTIVESTATE_HORDE_ALLIANCE_CHALLENGE:
            data << uint32(HP_MAP_N[m_TowerType]) << uint32(WORLD_STATE_REMOVE);
            data << uint32(HP_MAP_A[m_TowerType]) << uint32(WORLD_STATE_REMOVE);
            data << uint32(HP_MAP_H[m_TowerType]) << uint32(WORLD_STATE_ADD);
            break;
        case OBJECTIVESTATE_NEUTRAL:
        case OBJECTIVESTATE_NEUTRAL_ALLIANCE_CHALLENGE:
        case OBJECTIVESTATE_NEUTRAL_HORDE_CHALLENGE:
        default:
            data << uint32(HP_MAP_N[m_TowerType]) << uint32(WORLD_STATE_ADD);
            data << uint32(HP_MAP_A[m_TowerType]) << uint32(WORLD_STATE_REMOVE);
            data << uint32(HP_MAP_H[m_TowerType]) << uint32(WORLD_STATE_REMOVE);
            break;
    }
}

void OutdoorPvPHP::BuffTeam(uint32 team)
{
    if(team == ALLIANCE)
    {
        for(ObjectGuid itr : m_players[0])
        {
            if(Player * plr = ObjectAccessor::FindPlayer(itr))
                if(plr->IsInWorld()) plr->CastSpell(plr,AllianceBuff, true);
        }
        for(ObjectGuid itr : m_players[1])
        {
            if(Player * plr = ObjectAccessor::FindPlayer(itr))
                if(plr->IsInWorld()) plr->RemoveAurasDueToSpell(HordeBuff);
        }
    }
    else if(team == HORDE)
    {
        for(ObjectGuid itr : m_players[1])
        {
            if(Player * plr = ObjectAccessor::FindPlayer(itr))
                if(plr->IsInWorld()) plr->CastSpell(plr,HordeBuff, true);
        }
        for(ObjectGuid itr : m_players[0])
        {
            if(Player * plr = ObjectAccessor::FindPlayer(itr))
                if(plr->IsInWorld()) plr->RemoveAurasDueToSpell(AllianceBuff);
        }
    }
    else
    {
        for(ObjectGuid itr : m_players[0])
        {
            if(Player * plr = ObjectAccessor::FindPlayer(itr))
                if(plr->IsInWorld()) plr->RemoveAurasDueToSpell(AllianceBuff);
        }
        for(ObjectGuid itr : m_players[1])
        {
            if(Player * plr = ObjectAccessor::FindPlayer(itr))
                if(plr->IsInWorld()) plr->RemoveAurasDueToSpell(HordeBuff);
        }
    }
}

void OutdoorPvPHP::HandleKillImpl(Player *plr, Unit * killed)
{
    if(killed->GetTypeId() != TYPEID_PLAYER)
        return;

    if(plr->GetTeam() == ALLIANCE && (killed->ToPlayer())->GetTeam() != ALLIANCE)
        plr->CastSpell(plr,AlliancePlayerKillReward, true);
    else if(plr->GetTeam() == HORDE && (killed->ToPlayer())->GetTeam() != HORDE)
        plr->CastSpell(plr,HordePlayerKillReward, true);
}

class OutdoorPvP_hellfire_peninsula : public OutdoorPvPScript
{
    public:
        OutdoorPvP_hellfire_peninsula() : OutdoorPvPScript("outdoorpvp_hp") { }

        OutdoorPvP* GetOutdoorPvP() const override
        {
            return new OutdoorPvPHP();
        }
};

void AddSC_outdoorpvp_hp()
{
    new OutdoorPvP_hellfire_peninsula();
}