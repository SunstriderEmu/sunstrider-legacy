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
#include "CreatureAINew.h"

#define MAX_DESYNC 1.5f

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

void CreatureGroupManager::LoadCreatureFormations()
{
    Clear();

    //Check Integrity of the table
    QueryResult result = WorldDatabase.PQuery("SELECT MAX(`leaderGUID`) FROM `creature_formations`");

    if(!result)
    {
        TC_LOG_ERROR("FIXME"," ...an error occured while loading the table `creature_formations` ( maybe it doesn't exist ?)\n");
        return;
    }

    //Get group data
    result = WorldDatabase.PQuery("SELECT `leaderGUID`, `memberGUID`, `dist_min`, `dist_max`, `angle`, `groupAI`,`respawn`,`linkedloot` FROM `creature_formations` ORDER BY `leaderGUID`");

    if(!result)
    {
        TC_LOG_ERROR("FIXME","The table `creature_formations` is empty or corrupted");
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
        group_member->groupAI                = fields[5].GetUInt32();
        group_member->respawn                = fields[6].GetBool();
        group_member->linkedLoot             = fields[7].GetBool();
        //If creature is group leader we may skip loading of dist/angle
        if(group_member->leaderGUID != memberGUID)
        {
            group_member->follow_dist_min         = fields[2].GetFloat();
            group_member->follow_dist_max         = fields[3].GetFloat();   //FIXME: Add a check to ensure that dist_min <= dist_max
            group_member->follow_angle            = fields[4].GetFloat();
        }

        // check data correctness
        const CreatureData* leader = sObjectMgr->GetCreatureData(group_member->leaderGUID);
        const CreatureData* member = sObjectMgr->GetCreatureData(memberGUID);
        if(!leader || !member || leader->mapid != member->mapid)
        {
            TC_LOG_ERROR("FIXME","Table `creature_formations` has an invalid record (leaderGUID: '%u', memberGUID: '%u')", group_member->leaderGUID, memberGUID);
            delete group_member;
            continue;
        }

        CreatureGroupMap[memberGUID] = group_member;
    }
    while(result->NextRow()) ;

    TC_LOG_INFO("FIXME"," ");
    TC_LOG_INFO("FIXME", ">> Loaded %u creatures in formations", total_records );
    TC_LOG_INFO("FIXME"," ");
    //Free some heap
}

void CreatureGroup::AddMember(Creature *member)
{
    //Check if it is a leader
    if(member->GetDBTableGUIDLow() == m_groupID)
        m_leader = member;

    m_members[member] = sCreatureGroupMgr->GetGroupMap().find(member->GetDBTableGUIDLow())->second;
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
    if(fInfo == groupMap.end() || !fInfo->second)
        return;

    uint8 groupAI = fInfo->second->groupAI;
    if(!groupAI)
        return;

    if(groupAI == 1 && member != m_leader)
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
            if (itr->first->getAI())
                itr->first->getAI()->attackStart(target);
        }
    }
}

void CreatureGroup::FormationReset(bool dismiss)
{
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

void CreatureGroup::LeaderMoveTo(float x, float y, float z)
{
    //! To do: This should probably get its own movement generator or use WaypointMovementGenerator.
    //! If the leader's path is known, member's path can be plotted as well using formation offsets.
    if(!m_leader)
        return;

    float pathangle = atan2(m_leader->GetPositionY() - y, m_leader->GetPositionX() - x);

    for(CreatureGroupMemberType::iterator itr = m_members.begin(); itr != m_members.end(); ++itr)
    {
        Creature *member = itr->first;
        if(member == m_leader || !member->IsAlive() || member->GetVictim())
            continue;

        //TODO: if at last of first waypoint, consider inverting follow angle
        float angle = itr->second->follow_angle;
        float dist_min = itr->second->follow_dist_min;
        float dist_max = itr->second->follow_dist_max;

        float dx = x + cos(angle + pathangle) * dist_min;
        float dy = y + sin(angle + pathangle) * dist_min;
        float dz = z;

        Trinity::NormalizeMapCoord(dx);
        Trinity::NormalizeMapCoord(dy);

        //member->UpdateGroundPositionZ(dx, dy, dz);

        /*if (member->GetDistance(m_leader) > dist_min)
            member->SetUnitMovementFlags(m_leader->GetUnitMovementFlags());
        else
            member->RemoveUnitMovementFlag(MOVEMENTFLAG_WALKING);*/
            
        if (member->GetDistance(m_leader) < dist_min)               // Too close... Slow down buddy!
            member->AddUnitMovementFlag(MOVEMENTFLAG_WALKING);
        else if (member->GetDistance(m_leader) > dist_max)          // HURRY UP, HE'S LEAVING WITHOUT YA!
            member->RemoveUnitMovementFlag(MOVEMENTFLAG_WALKING);
        else                                                        // We're good, synchronize with leader
            member->SetUnitMovementFlags(m_leader->GetUnitMovementFlags());

        member->GetMotionMaster()->MovePoint(0, dx, dy, dz);
        member->SetHomePosition(dx, dy, dz, pathangle);
    }
}

void CreatureGroup::CheckLeaderDistance(Creature* member)
{
    if (!m_leader)
        return;
        
    if (!m_leaderX || !m_leaderY || !m_leaderZ)
        return;
        
    float angle = 0, dist_min = 0, dist_max = 0;
        
    for(CreatureGroupMemberType::iterator itr = m_members.begin(); itr != m_members.end(); ++itr)
    {
        Creature *memberItr = itr->first;
        if(memberItr == m_leader || !member->IsAlive() || member->GetVictim())
            continue;
            
        if (memberItr != member)
            continue;
            
        angle = itr->second->follow_angle;
        dist_min = itr->second->follow_dist_min;
        dist_max = itr->second->follow_dist_max;
        break;
    }
        
        
    if (member->GetDistance(m_leader) < dist_min) {
        member->AddUnitMovementFlag(MOVEMENTFLAG_WALKING);
        return;
        //member->GetMotionMaster()->MovePoint(0, m_leaderX, m_leaderY, m_leaderZ);
    }
    else if (member->GetDistance(m_leader) > dist_max) {
        member->RemoveUnitMovementFlag(MOVEMENTFLAG_WALKING);
        //member->GetMotionMaster()->MovePoint(0, m_leaderX, m_leaderY, m_leaderZ);
    }
    else
        member->SetUnitMovementFlags(m_leader->GetUnitMovementFlags());
        
    member->GetMotionMaster()->MoveFollow(m_leader, dist_max, angle);

    // Force move to GetNearPoint(dist, angle) here?
    //member->GetMotionMaster()->MovePoint(0, m_leaderX, m_leaderY, m_leaderZ);
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