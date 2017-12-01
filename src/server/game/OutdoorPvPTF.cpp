
#include "OutdoorPvPTF.h"
#include "OutdoorPvPMgr.h"
#include "WorldPacket.h"
#include "Player.h"
#include "ObjectMgr.h"
#include "Language.h"
#include "World.h"

OutdoorPvPTF::OutdoorPvPTF()
{
    m_TypeId = OUTDOOR_PVP_TF;
}

OPvPCapturePointTF::OPvPCapturePointTF(OutdoorPvP *pvp, OutdoorPvPTF_TowerType type)
: OPvPCapturePoint(pvp), m_TowerType(type), m_TowerState(TF_TOWERSTATE_N)
{
    SetCapturePointData(TFCapturePoints[type].entry,TFCapturePoints[type].map,TFCapturePoints[type].x,TFCapturePoints[type].y,TFCapturePoints[type].z,TFCapturePoints[type].o,TFCapturePoints[type].rot0,TFCapturePoints[type].rot1,TFCapturePoints[type].rot2,TFCapturePoints[type].rot3);
}

void OPvPCapturePointTF::FillInitialWorldStates(WorldPacket &data)
{
    data << uint32(TFTowerWorldStates[m_TowerType].n) << uint32(bool(m_TowerState & TF_TOWERSTATE_N));
    data << uint32(TFTowerWorldStates[m_TowerType].h) << uint32(bool(m_TowerState & TF_TOWERSTATE_H));
    data << uint32(TFTowerWorldStates[m_TowerType].a) << uint32(bool(m_TowerState & TF_TOWERSTATE_A));
}

void OutdoorPvPTF::FillInitialWorldStates(WorldPacket &data)
{
    data << TF_UI_TOWER_SLIDER_POS << uint32(50);
    data << TF_UI_TOWER_SLIDER_N << uint32(100);
    data << TF_UI_TOWER_SLIDER_DISPLAY << uint32(0);

    data << TF_UI_TOWER_COUNT_H << m_HordeTowersControlled;
    data << TF_UI_TOWER_COUNT_A << m_AllianceTowersControlled;
    data << TF_UI_TOWERS_CONTROLLED_DISPLAY << uint32(!m_IsLocked);

    data << TF_UI_LOCKED_TIME_MINUTES_FIRST_DIGIT << first_digit;
    data << TF_UI_LOCKED_TIME_MINUTES_SECOND_DIGIT << second_digit;
    data << TF_UI_LOCKED_TIME_HOURS << hours_left;

    data << TF_UI_LOCKED_DISPLAY_NEUTRAL << uint32(m_IsLocked && !m_HordeTowersControlled && !m_AllianceTowersControlled);
    data << TF_UI_LOCKED_DISPLAY_HORDE << uint32(m_IsLocked && (m_HordeTowersControlled > m_AllianceTowersControlled));
    data << TF_UI_LOCKED_DISPLAY_ALLIANCE << uint32(m_IsLocked && (m_HordeTowersControlled < m_AllianceTowersControlled));

    for(auto & m_OPvPCapturePoint : m_capturePoints)
    {
        m_OPvPCapturePoint.second->FillInitialWorldStates(data);
    }
}

void OutdoorPvPTF::SendRemoveWorldStates(Player * plr)
{
    plr->SendUpdateWorldState(TF_UI_TOWER_SLIDER_POS,uint32(0));
    plr->SendUpdateWorldState(TF_UI_TOWER_SLIDER_N,uint32(0));
    plr->SendUpdateWorldState(TF_UI_TOWER_SLIDER_DISPLAY,uint32(0));

    plr->SendUpdateWorldState(TF_UI_TOWER_COUNT_H,uint32(0));
    plr->SendUpdateWorldState(TF_UI_TOWER_COUNT_A,uint32(0));
    plr->SendUpdateWorldState(TF_UI_TOWERS_CONTROLLED_DISPLAY,uint32(0));

    plr->SendUpdateWorldState(TF_UI_LOCKED_TIME_MINUTES_FIRST_DIGIT,uint32(0));
    plr->SendUpdateWorldState(TF_UI_LOCKED_TIME_MINUTES_SECOND_DIGIT,uint32(0));
    plr->SendUpdateWorldState(TF_UI_LOCKED_TIME_HOURS,uint32(0));

    plr->SendUpdateWorldState(TF_UI_LOCKED_DISPLAY_NEUTRAL,uint32(0));
    plr->SendUpdateWorldState(TF_UI_LOCKED_DISPLAY_HORDE,uint32(0));
    plr->SendUpdateWorldState(TF_UI_LOCKED_DISPLAY_ALLIANCE,uint32(0));

    for(auto TFTowerWorldState : TFTowerWorldStates)
    {
        plr->SendUpdateWorldState(uint32(TFTowerWorldState.n),uint32(0));
        plr->SendUpdateWorldState(uint32(TFTowerWorldState.h),uint32(0));
        plr->SendUpdateWorldState(uint32(TFTowerWorldState.a),uint32(0));
    }
}

void OPvPCapturePointTF::UpdateTowerState()
{
    m_PvP->SendUpdateWorldState(uint32(TFTowerWorldStates[m_TowerType].n),uint32(bool(m_TowerState & TF_TOWERSTATE_N)));
    m_PvP->SendUpdateWorldState(uint32(TFTowerWorldStates[m_TowerType].h),uint32(bool(m_TowerState & TF_TOWERSTATE_H)));
    m_PvP->SendUpdateWorldState(uint32(TFTowerWorldStates[m_TowerType].a),uint32(bool(m_TowerState & TF_TOWERSTATE_A)));
}

void OutdoorPvPTF::BuffTeam(uint32 team)
{
    if(team == ALLIANCE)
    {
        for(uint64 itr : m_players[0])
        {
            if(Player * plr = sObjectMgr->GetPlayer(itr))
                if(plr->IsInWorld()) plr->CastSpell(plr,TF_CAPTURE_BUFF, TRIGGERED_FULL_MASK);
        }
        for(uint64 itr : m_players[1])
        {
            if(Player * plr = sObjectMgr->GetPlayer(itr))
                if(plr->IsInWorld()) plr->RemoveAurasDueToSpell(TF_CAPTURE_BUFF);
        }
    }
    else if(team == HORDE)
    {
        for(uint64 itr : m_players[1])
        {
            if(Player * plr = sObjectMgr->GetPlayer(itr))
                if(plr->IsInWorld()) plr->CastSpell(plr,TF_CAPTURE_BUFF, TRIGGERED_FULL_MASK);
        }
        for(uint64 itr : m_players[0])
        {
            if(Player * plr = sObjectMgr->GetPlayer(itr))
                if(plr->IsInWorld()) plr->RemoveAurasDueToSpell(TF_CAPTURE_BUFF);
        }
    }
    else
    {
        for(uint64 itr : m_players[0])
        {
            if(Player * plr = sObjectMgr->GetPlayer(itr))
                if(plr->IsInWorld()) plr->RemoveAurasDueToSpell(TF_CAPTURE_BUFF);
        }
        for(uint64 itr : m_players[1])
        {
            if(Player * plr = sObjectMgr->GetPlayer(itr))
                if(plr->IsInWorld()) plr->RemoveAurasDueToSpell(TF_CAPTURE_BUFF);
        }
    }
}

void OPvPCapturePointTF::RewardDailyQuest(uint32 team)
{
    if (team == ALLIANCE)
    {
        for(uint64 itr : m_activePlayers[0])
        {
            if(Player * plr = sObjectMgr->GetPlayer(itr)) {
                if(plr->IsInWorld() && plr->GetQuestStatus(11505) == QUEST_STATUS_INCOMPLETE)
                    plr->AreaExploredOrEventHappens(11505);
            }
        }
    }
    else if (team == HORDE)
    {
        for(uint64 itr : m_activePlayers[1])
        {
            if(Player * plr = sObjectMgr->GetPlayer(itr)) {
                if(plr->IsInWorld() && plr->GetQuestStatus(11506) == QUEST_STATUS_INCOMPLETE)
                    plr->AreaExploredOrEventHappens(11506);
            }
        }
    }
    else
        TC_LOG_ERROR("FIXME","OutdoorPvPTF::RewardDailyQuest() invalid team ID: %u", team);
}

bool OutdoorPvPTF::Update(uint32 diff)
{
    bool changed = false;

    if((changed = OutdoorPvP::Update(diff)))
    {
        if(m_AllianceTowersControlled == TF_TOWER_NUM)
        {
            BuffTeam(ALLIANCE);
            m_IsLocked = true;
            SendUpdateWorldState(TF_UI_LOCKED_DISPLAY_NEUTRAL,uint32(0));
            SendUpdateWorldState(TF_UI_LOCKED_DISPLAY_HORDE,uint32(0));
            SendUpdateWorldState(TF_UI_LOCKED_DISPLAY_ALLIANCE,uint32(1));
            SendUpdateWorldState(TF_UI_TOWERS_CONTROLLED_DISPLAY, uint32(0));
        }
        else if(m_HordeTowersControlled == TF_TOWER_NUM)
        {
            BuffTeam(HORDE);
            m_IsLocked = true;
            SendUpdateWorldState(TF_UI_LOCKED_DISPLAY_NEUTRAL,uint32(0));
            SendUpdateWorldState(TF_UI_LOCKED_DISPLAY_HORDE,uint32(1));
            SendUpdateWorldState(TF_UI_LOCKED_DISPLAY_ALLIANCE,uint32(0));
            SendUpdateWorldState(TF_UI_TOWERS_CONTROLLED_DISPLAY, uint32(0));
        }
        else
            BuffTeam(0);

        SendUpdateWorldState(TF_UI_TOWER_COUNT_A, m_AllianceTowersControlled);
        SendUpdateWorldState(TF_UI_TOWER_COUNT_H, m_HordeTowersControlled);
    }
    if(m_IsLocked)
    {
        // lock timer is down, release lock
        if(m_LockTimer < diff)
        {
            m_LockTimer = TF_LOCK_TIME;
            m_LockTimerUpdate = 0;
            m_IsLocked = false;
            SendUpdateWorldState(TF_UI_TOWERS_CONTROLLED_DISPLAY, uint32(1));
            SendUpdateWorldState(TF_UI_LOCKED_DISPLAY_NEUTRAL,uint32(0));
            SendUpdateWorldState(TF_UI_LOCKED_DISPLAY_HORDE,uint32(0));
            SendUpdateWorldState(TF_UI_LOCKED_DISPLAY_ALLIANCE,uint32(0));
        }
        else
        {
            // worldstateui update timer is down, update ui with new time data
            if(m_LockTimerUpdate < diff)
            {
                m_LockTimerUpdate = TF_LOCK_TIME_UPDATE;
                uint32 minutes_left = m_LockTimer / 60000;
                hours_left = minutes_left / 60;
                minutes_left -= hours_left * 60;
                second_digit = minutes_left % 10;
                first_digit = minutes_left / 10;

                SendUpdateWorldState(TF_UI_LOCKED_TIME_MINUTES_FIRST_DIGIT,first_digit);
                SendUpdateWorldState(TF_UI_LOCKED_TIME_MINUTES_SECOND_DIGIT,second_digit);
                SendUpdateWorldState(TF_UI_LOCKED_TIME_HOURS,hours_left);
            } else m_LockTimerUpdate -= diff;
            m_LockTimer -= diff;
        }
    }
    return changed;
}


void OutdoorPvPTF::HandlePlayerEnterZone(Player * plr, uint32 zone)
{
    if(plr->GetTeam() == ALLIANCE)
    {
        if(m_AllianceTowersControlled >= TF_TOWER_NUM)
            plr->CastSpell(plr,TF_CAPTURE_BUFF, TRIGGERED_FULL_MASK);
    }
    else
    {
        if(m_HordeTowersControlled >= TF_TOWER_NUM)
            plr->CastSpell(plr,TF_CAPTURE_BUFF, TRIGGERED_FULL_MASK);
    }
    OutdoorPvP::HandlePlayerEnterZone(plr,zone);
}

void OutdoorPvPTF::HandlePlayerLeaveZone(Player * plr, uint32 zone)
{
    // remove buffs
    plr->RemoveAurasDueToSpell(TF_CAPTURE_BUFF);
    OutdoorPvP::HandlePlayerLeaveZone(plr, zone);
}

bool OutdoorPvPTF::SetupOutdoorPvP()
{
    m_AllianceTowersControlled = 0;
    m_HordeTowersControlled = 0;

    m_IsLocked = false;
    m_LockTimer = TF_LOCK_TIME;
    m_LockTimerUpdate = 0;
    hours_left = 6;
    second_digit = 0;
    first_digit = 0;

    SetMapFromZone(OutdoorPvPTFBuffZones[0]);

    // add the zones affected by the pvp buff
    for(uint32 OutdoorPvPTFBuffZone : OutdoorPvPTFBuffZones)
        sOutdoorPvPMgr->AddZone(OutdoorPvPTFBuffZone,this);

    AddCapturePoint(new OPvPCapturePointTF(this,TF_TOWER_NW));
    AddCapturePoint(new OPvPCapturePointTF(this,TF_TOWER_N));
    AddCapturePoint(new OPvPCapturePointTF(this,TF_TOWER_NE));
    AddCapturePoint(new OPvPCapturePointTF(this,TF_TOWER_SE));
    AddCapturePoint(new OPvPCapturePointTF(this,TF_TOWER_S));

    return true;
}

bool OPvPCapturePointTF::Update(uint32 diff)
{
    // can update even in locked state if gathers the controlling faction
    bool canupdate = ((((OutdoorPvPTF*)m_PvP)->m_AllianceTowersControlled > 0) && this->m_activePlayers[0].size() > this->m_activePlayers[1].size()) ||
            ((((OutdoorPvPTF*)m_PvP)->m_HordeTowersControlled > 0) && this->m_activePlayers[0].size() < this->m_activePlayers[1].size());
    // if gathers the other faction, then only update if the pvp is unlocked
    canupdate = canupdate || !((OutdoorPvPTF*)m_PvP)->m_IsLocked;
    return canupdate && OPvPCapturePoint::Update(diff);
}

void OPvPCapturePointTF::ChangeState()
{
    // if changing from controlling alliance to horde
    if (m_OldState == OBJECTIVESTATE_ALLIANCE)
    {
        if (((OutdoorPvPTF*)m_PvP)->m_AllianceTowersControlled)
            ((OutdoorPvPTF*)m_PvP)->m_AllianceTowersControlled--;
        sWorld->SendZoneText(OutdoorPvPTFBuffZones[0], sObjectMgr->GetTrinityStringForDBCLocale(LANG_OPVP_TF_LOOSE_A));
        //m_PvP->SendDefenseMessage(OutdoorPvPTFBuffZones[0], TEXT_SPIRIT_TOWER_LOSE_ALLIANCE);
    }
    // if changing from controlling horde to alliance
    else if (m_OldState == OBJECTIVESTATE_HORDE)
    {
        if (((OutdoorPvPTF*)m_PvP)->m_HordeTowersControlled)
            ((OutdoorPvPTF*)m_PvP)->m_HordeTowersControlled--;
        sWorld->SendZoneText(OutdoorPvPTFBuffZones[0], sObjectMgr->GetTrinityStringForDBCLocale(LANG_OPVP_TF_LOOSE_H));
        //m_PvP->SendDefenseMessage(OutdoorPvPTFBuffZones[0], TEXT_SPIRIT_TOWER_LOSE_HORDE);
    }

    uint32 artkit = 21;

    switch (m_State)
    {
    case OBJECTIVESTATE_ALLIANCE:
        m_TowerState = TF_TOWERSTATE_A;
        artkit = 2;
        if (((OutdoorPvPTF*)m_PvP)->m_AllianceTowersControlled<TF_TOWER_NUM)
            ((OutdoorPvPTF*)m_PvP)->m_AllianceTowersControlled++;
        //m_PvP->SendDefenseMessage(OutdoorPvPTFBuffZones[0], TEXT_SPIRIT_TOWER_TAKEN_ALLIANCE);
        sWorld->SendZoneText(OutdoorPvPTFBuffZones[0], sObjectMgr->GetTrinityStringForDBCLocale(LANG_OPVP_TF_CAPTURE_A));
        RewardDailyQuest(ALLIANCE);
        break;
    case OBJECTIVESTATE_HORDE:
        m_TowerState = TF_TOWERSTATE_H;
        artkit = 1;
        if (((OutdoorPvPTF*)m_PvP)->m_HordeTowersControlled<TF_TOWER_NUM)
            ((OutdoorPvPTF*)m_PvP)->m_HordeTowersControlled++;
        sWorld->SendZoneText(OutdoorPvPTFBuffZones[0], sObjectMgr->GetTrinityStringForDBCLocale(LANG_OPVP_TF_CAPTURE_H));
        //m_PvP->SendDefenseMessage(OutdoorPvPTFBuffZones[0], TEXT_SPIRIT_TOWER_TAKEN_HORDE);
        RewardDailyQuest(HORDE);
        break;
    case OBJECTIVESTATE_NEUTRAL:
    case OBJECTIVESTATE_NEUTRAL_ALLIANCE_CHALLENGE:
    case OBJECTIVESTATE_NEUTRAL_HORDE_CHALLENGE:
    case OBJECTIVESTATE_ALLIANCE_HORDE_CHALLENGE:
    case OBJECTIVESTATE_HORDE_ALLIANCE_CHALLENGE:
        m_TowerState = TF_TOWERSTATE_N;
        break;
    }

    auto bounds = sMapMgr->FindMap(530, 0)->GetGameObjectBySpawnIdStore().equal_range(m_capturePointSpawnId);
    for (auto itr = bounds.first; itr != bounds.second; ++itr)
        itr->second->SetGoArtKit(artkit);

    UpdateTowerState();
}
