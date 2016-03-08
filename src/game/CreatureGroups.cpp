/*
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 *
 * Copyright (C) 2008-2009 Trinity <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "Creature.h"
#include "CreatureGroups.h"
#include "ObjectMgr.h"
#include "CreatureAI.h"
#include "MoveSplineInit.h"

CreatureGroupManager::~CreatureGroupManager()
{
    Clear();
}

void CreatureGroupManager::Clear()
{
    for (auto itr : CreatureGroupMap)
        delete itr.second;

    //Clear existing map
    CreatureGroupMap.clear();
}

void CreatureGroupManager::AddGroupMember(uint32 creature_lowguid, FormationInfo* group_member)
{
    CreatureGroupMap[creature_lowguid] = group_member;
}

void CreatureGroupManager::AddCreatureToGroup(uint32 groupId, Creature *member)
{
    Map *map = member->FindMap();
    if(!map)
        return;

    CreatureGroupHolderType::iterator itr = map->CreatureGroupHolder.find(groupId);

    //Add member to an existing group
    if(itr != map->CreatureGroupHolder.end())
        itr->second->AddMember(member);
    //Create new group
    else
    {
        CreatureGroup* group = new CreatureGroup(groupId);
        map->CreatureGroupHolder[groupId] = group;
        group->AddMember(member);
    }
}

void CreatureGroupManager::RemoveCreatureFromGroup(uint32 groupId, Creature *member)
{
    Map *map = member->FindMap();
    if (!map)
        return;

    CreatureGroupHolderType::iterator itr = map->CreatureGroupHolder.find(groupId);
    if (itr == map->CreatureGroupHolder.end())
        return;

    CreatureGroup *group = itr->second;

    group->RemoveMember(member);

    if (group->isEmpty())
    {
        map->CreatureGroupHolder.erase(group->GetId());
        delete group;
    }
}

void CreatureGroupManager::RemoveCreatureFromGroup(CreatureGroup *group, Creature *member)
{
    group->RemoveMember(member);

    if(group->isEmpty())
    {
        Map *map = member->FindMap();
        if(!map)
            return;

        map->CreatureGroupHolder.erase(group->GetId());
        delete group;
    }
}

void CreatureGroup::EmptyFormation()
{
    for (auto itr : m_members)
    {
        Creature* member = itr.first;
        member->SetFormation(NULL);
        m_members.erase(member);
    }
}

void CreatureGroupManager::BreakFormation(Creature* leader)
{
    CreatureGroup* group = leader->GetFormation();
    if (!group)
        return;

    group->EmptyFormation();
    leader->GetMap()->CreatureGroupHolder.erase(group->GetId());
    delete group;
}

void CreatureGroupManager::LoadCreatureFormations()
{
    Clear();

    //Check Integrity of the table
    QueryResult result = WorldDatabase.PQuery("SELECT MAX(`leaderGUID`) FROM `creature_formations`");

    if(!result)
    {
        TC_LOG_ERROR("sql.sql"," ...an error occured while loading the table `creature_formations` ( maybe it doesn't exist ?)\n");
        return;
    }

    //Get group data
    result = WorldDatabase.PQuery("SELECT `leaderGUID`, `memberGUID`, `dist`, `angle`, `groupAI`,`respawn`,`linkedloot` FROM `creature_formations` ORDER BY `leaderGUID`");

    if(!result)
    {
        TC_LOG_ERROR("sql.sql","The table `creature_formations` is empty or corrupted");
        return;
    }

    Field *fields;
    uint32 total_records = result->GetRowCount();

    FormationInfo* group_member;
    //Loading data...
    do
    {
        fields = result->Fetch();

        //Load group member data
        group_member                        = new FormationInfo;
        group_member->leaderGUID            = fields[0].GetUInt32();
        uint32 memberGUID = fields[1].GetUInt32();
        //If creature is group leader we may skip loading of dist/angle
        if(group_member->leaderGUID != memberGUID)
        {
            group_member->follow_dist             = fields[2].GetFloat();
            group_member->follow_angle            = fields[3].GetFloat();
        }

        group_member->groupAI = GroupAI(fields[4].GetUInt32());
        group_member->respawn = fields[5].GetBool();
        group_member->linkedLoot = fields[6].GetBool();

        // check data correctness
        const CreatureData* leader = sObjectMgr->GetCreatureData(group_member->leaderGUID);
        const CreatureData* member = sObjectMgr->GetCreatureData(memberGUID);
        if(!leader || !member || leader->mapid != member->mapid)
        {
            TC_LOG_ERROR("sql.sql","Table `creature_formations` has an invalid record (leaderGUID: '%u', memberGUID: '%u')", group_member->leaderGUID, memberGUID);
            delete group_member;
            continue;
        }

        CreatureGroupMap[memberGUID] = group_member;
    }
    while(result->NextRow()) ;

    TC_LOG_INFO("sql.sql", ">> Loaded %u creatures in formations", total_records );
    //Free some heap
}

void CreatureGroup::AddMember(Creature *member)
{
    if (member->GetFormation())
    {
        TC_LOG_ERROR("misc", "Tried to add a member (tableguid: %u, entry %u) already in a formation to formation", member->GetDBTableGUIDLow(), member->GetEntry());
        return;
    }

    //Check if it is a leader
    if(member->GetDBTableGUIDLow() == m_groupID)
        m_leader = member;

    // Create formation info if needed
    FormationInfo* fInfo = nullptr;
    auto itr = sCreatureGroupMgr->GetGroupMap().find(member->GetDBTableGUIDLow());
    if (itr == sCreatureGroupMgr->GetGroupMap().end())
    {
        //get leader to calc angle and distance from his current pos
        if (!m_leader)
            m_leader = member->GetMap()->GetCreature(MAKE_PAIR64(m_groupID, HIGHGUID_UNIT));
       
        if (!m_leader)
            return;

        fInfo = new FormationInfo;
        if (m_leader != member) //next infos not needed if we're leader
        {
            fInfo->follow_angle = member->GetAngle(m_leader) - m_leader->GetOrientation();
            fInfo->follow_dist = sqrtf(pow(m_leader->GetPositionX() - member->GetPositionX(), int(2)) + pow(m_leader->GetPositionY() - member->GetPositionY(), int(2)));
        }
        fInfo->leaderGUID = m_leader->GetGUIDLow();
        sCreatureGroupMgr->AddGroupMember(member->GetDBTableGUIDLow(), fInfo);
    }
    else {
        fInfo = sCreatureGroupMgr->GetGroupMap().find(member->GetDBTableGUIDLow())->second;
    }


    m_members[member] = fInfo;
    member->SetFormation(this);
}

void CreatureGroup::RemoveMember(Creature *member)
{
    if(m_leader == member)
        m_leader = NULL;

    m_members.erase(member);
    member->SetFormation(NULL);
}

void CreatureGroup::MemberAttackStart(Creature *member, Unit *target)
{
    if(!member || !target)
        return;

    CreatureGroupInfoType const& groupMap = sCreatureGroupMgr->GetGroupMap();
    CreatureGroupInfoType::const_iterator fInfo = groupMap.find(member->GetDBTableGUIDLow());
    if (fInfo == groupMap.end() || !fInfo->second)
        return;

    GroupAI groupAI = fInfo->second->groupAI;
    if (groupAI == GROUP_AI_NONE)
        return;

    if (groupAI == GROUP_AI_LEADER_SUPPORT && member != m_leader)
        return;

    for(CreatureGroupMemberType::iterator itr = m_members.begin(); itr != m_members.end(); ++itr)
    {
        //Skip one check
        if(!itr->first || itr->first == member)
            continue;

        if(!itr->first->IsAlive())
            continue;

        if(itr->first->GetVictim())
            continue;

        if(itr->first->CanAttack(target) == CAN_ATTACK_RESULT_OK) {
            itr->first->AI()->AttackStart(target);
        }
    }
}

void CreatureGroup::FormationReset(bool dismiss)
{
    if (m_members.size() && m_members.begin()->second->groupAI == GROUP_AI_NONE)
        return;

    for(CreatureGroupMemberType::iterator itr = m_members.begin(); itr != m_members.end(); ++itr)
    {
        if(itr->first != m_leader && itr->first->IsAlive())
        {
            if(dismiss)
                itr->first->GetMotionMaster()->Initialize();
            else
                itr->first->GetMotionMaster()->MoveIdle();
        }
    }
    m_Formed = !dismiss;
}

void CreatureGroup::LeaderMoveTo(float x, float y, float z, bool run)
{
    //! To do: This should probably get its own movement generator or use WaypointMovementGenerator.
    //! If the leader's path is known, member's path can be plotted as well using formation offsets.
    if(!m_leader)
        return;

    uint8 groupAI = sCreatureGroupMgr->GetGroupMap().at(m_leader->GetDBTableGUIDLow())->groupAI;
    if (groupAI == GROUP_AI_NONE)
        return;

    float pathDist = m_leader->GetExactDist(x, y, z);
    float pathAngle = m_leader->GetAngle(x, y);

    for(CreatureGroupMemberType::iterator itr = m_members.begin(); itr != m_members.end(); ++itr)
    {
        Creature *member = itr->first;
        if(member == m_leader || !member->IsAlive() || member->GetVictim())
            continue;

        // Xinef: If member is stunned / rooted etc don't allow to move him
        if (member->HasUnitState(UNIT_STATE_NOT_MOVE))
            continue;

        // Xinef: this should be automatized, if turn angle is greater than PI/2 (90°) we should swap formation angle
        if (M_PI - fabs(fabs(m_leader->GetOrientation() - pathAngle) - M_PI) > M_PI*0.50f)
        {
            // pussywizard: in both cases should be 2*M_PI - follow_angle
            // pussywizard: also, GetCurrentWaypointID() returns 0..n-1, while point_1 must be > 0, so +1
            // pussywizard: db table waypoint_data shouldn't have point id 0 and shouldn't have any gaps for this to work!
            // if (m_leader->GetCurrentWaypointID()+1 == itr->second->point_1 || m_leader->GetCurrentWaypointID()+1 == itr->second->point_2)
            itr->second->follow_angle = Position::NormalizeOrientation(itr->second->follow_angle + M_PI); //(2 * M_PI) - itr->second->follow_angle;
        }


        float followAngle = itr->second->follow_angle;
        float followDist = itr->second->follow_dist;

        float dx = x + cos(followAngle + pathAngle) * followDist;
        float dy = y + sin(followAngle + pathAngle) * followDist;
        float dz = z;

        Trinity::NormalizeMapCoord(dx);
        Trinity::NormalizeMapCoord(dy);

        member->UpdateGroundPositionZ(dx, dy, dz);

        member->SetUnitMovementFlags(m_leader->GetUnitMovementFlags());
        // pussywizard: setting the same movementflags is not enough, spline decides whether leader walks/runs, so spline param is now passed as "run" parameter to this function
        if (run && member->IsWalking())
            member->RemoveUnitMovementFlag(MOVEMENTFLAG_WALKING);
        else if (!run && !member->IsWalking())
            member->AddUnitMovementFlag(MOVEMENTFLAG_WALKING);
            
        // xinef: if we move members to position without taking care of sizes, we should compare distance without sizes
        // xinef: change members speed basing on distance - if too far speed up, if too close slow down
        UnitMoveType mtype = Movement::SelectSpeedType(member->GetUnitMovementFlags());
        member->SetSpeedRate(mtype, m_leader->GetSpeedRate(mtype) * member->GetExactDist(dx, dy, dz) / pathDist);

        member->GetMotionMaster()->MovePoint(0, dx, dy, dz);
        member->SetHomePosition(dx, dy, dz, pathAngle);
    }
}

void CreatureGroup::UpdateCombat()
{
    inCombat = false;
    for(auto itr : m_members)
    {
        if(itr.first->IsInCombat())
            inCombat = true;
    }
}

void CreatureGroup::Respawn()
{
    for(auto itr : m_members)
    {
        if(!itr.second->respawn)
            continue;
        Creature* member = itr.first;
        if(member->IsAlive())
            continue;
        member->Respawn();
        member->GetMotionMaster()->MoveTargetedHome();
    }
}

bool CreatureGroup::IsAlive() const
{
    for(auto itr : m_members)
        if(itr.first->IsAlive())
            return true;

    return false;
}

void CreatureGroup::Update(uint32 diff)
{
    if(IsAlive())
    {
        UpdateCombat();
        if(!inCombat) 
        {
            if(respawnTimer < diff)
            {
                Respawn();//Respawn dead members marked as respawnable
                respawnTimer = RESPAWN_TIMER;
            } else respawnTimer -= diff;
        } else {
            if(respawnTimer < RESPAWN_TIMER)
                respawnTimer = RESPAWN_TIMER;
        }
        justAlive = true;

        //reset corpse despawn time if loot linked
        for(auto itr : m_members)
        {
            if(itr.second->linkedLoot == true)
                itr.first->SetCorpseRemoveTime(time(NULL) + itr.first->GetCorpseDelay());
        }
    } else {
        if(justAlive)
        {
            SetLootable(true);
            justAlive = false;
        }
    }
}

void CreatureGroup::SetLootable(bool lootable)
{
    for(auto itr : m_members)
        if(itr.second->linkedLoot == true)
            itr.first->SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);
}

bool CreatureGroup::isLootLinked(Creature* c)
{
    CreatureGroupMemberType::iterator itr = m_members.find(c);
    if(itr != m_members.end())
    {
        return itr->second->linkedLoot;
    }
    return false;
}