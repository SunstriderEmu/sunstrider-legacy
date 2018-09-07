#include "Creature.h"
#include "CreatureGroups.h"
#include "ObjectMgr.h"
#include "CreatureAI.h"
#include "MoveSplineInit.h"
#include "WaypointMovementGenerator.h"

FormationMgr::FormationMgr()
{
}

FormationMgr::~FormationMgr()
{
}

FormationMgr* FormationMgr::instance()
{
    static FormationMgr instance;
    return &instance;
}

void FormationMgr::AddCreatureToGroup(ObjectGuid::LowType leaderSpawnId, Creature* creature)
{
    Map* map = creature->GetMap();

    auto itr = map->CreatureGroupHolder.find(leaderSpawnId);
    if (itr != map->CreatureGroupHolder.end())
    {
        //Add member to an existing group
        TC_LOG_DEBUG("entities.unit", "Group found: %u, inserting creature GUID: %u, Group InstanceID %u", leaderSpawnId, creature->GetGUID().GetCounter(), creature->GetInstanceId());

        // With dynamic spawn the creature may have just respawned
        // we need to find previous instance of creature and delete it from the formation, as it'll be invalidated
        auto bounds = Trinity::Containers::MapEqualRange(map->GetCreatureBySpawnIdStore(), creature->GetSpawnId());
        for (auto const& pair : bounds)
        {
            Creature* other = pair.second;
            if (other == creature)
                continue;

            if (itr->second->HasMember(other))
                itr->second->RemoveMember(other);
        }
    }
    else
    {
        //Create new group
        TC_LOG_DEBUG("entities.unit", "Group not found: %u. Creating new group.", leaderSpawnId);
        CreatureGroup* group = new CreatureGroup(leaderSpawnId);
        std::tie(itr, std::ignore) = map->CreatureGroupHolder.emplace(leaderSpawnId, group);
    }

    itr->second->AddMember(creature);
}

void FormationMgr::RemoveCreatureFromGroup(ObjectGuid::LowType leaderSpawnId, Creature *member)
{
    Map* map = member->GetMap();
    auto itr = map->CreatureGroupHolder.find(leaderSpawnId);
    if (itr == map->CreatureGroupHolder.end())
        return;

    RemoveCreatureFromGroup(itr->second, member);
}

void FormationMgr::RemoveCreatureFromGroup(CreatureGroup *group, Creature *member)
{
    TC_LOG_DEBUG("entities.unit", "Deleting member pointer to GUID: %u from group %u", group->GetLeaderSpawnId(), member->GetSpawnId());
    group->RemoveMember(member);

    if (group->IsEmpty())
    {
        Map* map = member->GetMap();

        TC_LOG_DEBUG("entities.unit", "Deleting group with InstanceID %u", member->GetInstanceId());
        auto itr = map->CreatureGroupHolder.find(group->GetLeaderSpawnId());
        ASSERT(itr != map->CreatureGroupHolder.end(), "Not registered group %u in map %u", group->GetLeaderSpawnId(), map->GetId());
        map->CreatureGroupHolder.erase(itr);
        delete group;
    }
}

void FormationMgr::LoadCreatureFormations()
{
    uint32 oldMSTime = GetMSTime();

    //Get group data
    QueryResult result = WorldDatabase.PQuery("SELECT `leaderGUID`, `memberGUID`, `dist`, `angle`, `groupAI`, `respawn`, `linkedloot` FROM `creature_formations` ORDER BY `leaderGUID`");
    if(!result)
    {
        TC_LOG_INFO("server.loading", ">>  Loaded 0 creatures in formations. DB table `creature_formations` is empty!");
        return;
    }

    uint32 count = 0;
    std::unordered_set<ObjectGuid::LowType> leaderSpawnIds;

    do
    {
        Field* fields = result->Fetch();

        //Load group member data
        FormationInfo member;
        uint32 index = 0;
        member.leaderSpawnId              = fields[index++].GetUInt32();
        ObjectGuid::LowType memberSpawnId = fields[index++].GetUInt32();
        member.followDist                 = 0.f;
        member.followAngle                = 0.f;
        member.followDist                 = fields[index++].GetFloat();
        member.followAngle                = fields[index++].GetFloat() * float(M_PI) / 180.0f;
        member.groupAI                    = GroupAI(fields[index++].GetUInt32());
        member.respawn                    = fields[index++].GetBool();
        member.linkedLoot                 = fields[index++].GetBool();

        // check data correctness
        {
            if (member.groupAI >= GROUP_AI_TOTAL)
            {
                TC_LOG_ERROR("sql.sql", "LoadCreatureFormations: invalid groupAI %u for creature with SpawnId %u", uint32(member.groupAI), memberSpawnId);
                continue;
            }

            if (!sObjectMgr->GetCreatureData(member.leaderSpawnId))
            {
                TC_LOG_ERROR("sql.sql", "creature_formations table leader guid %u incorrect (not exist)", member.leaderSpawnId);
                continue;
            }

            if (!sObjectMgr->GetCreatureData(memberSpawnId))
            {
                TC_LOG_ERROR("sql.sql", "creature_formations table member guid %u incorrect (not exist)", memberSpawnId);
                continue;
            }

            leaderSpawnIds.insert(member.leaderSpawnId);
        }

        _creatureGroupMap.emplace(memberSpawnId, std::move(member));
        count++;
    }
    while(result->NextRow()) ;

    TC_LOG_INFO("server.loading", ">> Loaded %u creatures in formations in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

FormationInfo* FormationMgr::GetFormationInfo(ObjectGuid::LowType spawnId)
{
    return Trinity::Containers::MapGetValuePtr(_creatureGroupMap, spawnId);
}

void FormationMgr::AddFormationMember(ObjectGuid::LowType spawnId, FormationInfo const& group_member)
{
    _creatureGroupMap[spawnId] = group_member;
}

void FormationMgr::BreakFormation(Creature* leader)
{
    CreatureGroup* group = leader->GetFormation();
    if (!group)
        return;

    group->EmptyFormation();
    leader->GetMap()->CreatureGroupHolder.erase(group->GetLeaderSpawnId());
    delete group;
}

CreatureGroup::CreatureGroup(uint32 id) :
    _leader(nullptr),
    _leaderSpawnId(id),
    _formed(false), 
    _engaging(false),
    _justAlive(true),
    _respawnTimer(RESPAWN_TIMER)
{}

CreatureGroup::~CreatureGroup()
{
    EmptyFormation();
}

void CreatureGroup::EmptyFormation()
{
    _leader = nullptr;

    while (_members.size())
        RemoveMember(_members.begin()->first);
}

void CreatureGroup::AddMember(Creature* member)
{
    TC_LOG_DEBUG("entities.unit", "CreatureGroup::AddMember: Adding unit GUID: %u.", member->GetGUID().GetCounter());

    //Check if it is a leader
    if (member->GetSpawnId() == _leaderSpawnId)
    {
        TC_LOG_DEBUG("entities.unit", "Unit GUID: %u is formation leader. Adding group.", member->GetGUID().GetCounter());
        _leader = member;
    }

    // formation must be registered at this point
    FormationInfo* formationInfo = ASSERT_NOTNULL(sFormationMgr->GetFormationInfo(member->GetSpawnId()));
    _members.emplace(member, formationInfo);
    member->SetFormation(this);
    formationInfo->originalHome = member->GetHomePosition();
}

void CreatureGroup::RemoveMember(Creature *member)
{
    if (_leader == member)
        _leader = nullptr;

    auto itr = _members.find(member);
    if (itr != _members.end())
    {
        //restore original home (for ex: DarkPortalEventDemonAI rely on this)
        member->SetHomePosition(itr->second->originalHome);
        _members.erase(member);
    }
    member->SetFormation(nullptr);
}

void CreatureGroup::MemberEngagingTarget(Creature *member, Unit *target)
{
    // used to prevent recursive calls
    if (_engaging)
        return;

    GroupAI groupAI = ASSERT_NOTNULL(sFormationMgr->GetFormationInfo(member->GetSpawnId()))->groupAI;
    if (!groupAI)
        return;

    switch (groupAI)
    {
    case GROUP_AI_NONE:
        return;
    case GROUP_AI_LEADER_SUPPORT:
        if (member != _leader)
            return;
    case GROUP_AI_FULL_SUPPORT:
        break; //continue
    }

    _engaging = true;

    for(auto& pair : _members)
    {
        Creature* other = pair.first;
        if(other == member)
            continue;

        if(!other->IsAlive())
            continue;

        if(other->CanCreatureAttack(target) == CAN_ATTACK_RESULT_OK)
            other->EngageWithTarget(target);
    }

    _engaging = false;
}

void CreatureGroup::FormationReset(bool dismiss)
{
    for (auto const& pair : _members)
    {
        if (pair.first != _leader && pair.first->IsAlive())
        {
            if (dismiss)
                pair.first->GetMotionMaster()->Initialize();
            else
                pair.first->GetMotionMaster()->MoveIdle();
            TC_LOG_DEBUG("entities.unit", "CreatureGroup::FormationReset: Set %s movement for member GUID: %u", dismiss ? "default" : "idle", pair.first->GetGUID().GetCounter());
        }
    }

    _formed = !dismiss;
}

bool CreatureGroup::CanLeaderStartMoving() const
{
    for (auto const& pair : _members)
    {
        if (pair.first != _leader && pair.first->IsAlive())
        {
            if (pair.first->IsEngaged() || pair.first->IsReturningHome())
                return false;
        }
    }

    return true;
}

Position CreatureGroup::CalculateMemberDestination(Creature* member, Position const& leaderPos, float followAngle, float followDist, float pathAngle, uint8 depth) const
{
    //no valid position found after 3 iterations, return leader position instead
    if (depth > 3)
        return leaderPos;

    Position dest;
    dest.m_positionX = leaderPos.GetPositionX() + cos(followAngle + pathAngle) * followDist;
    dest.m_positionY = leaderPos.GetPositionY() + sin(followAngle + pathAngle) * followDist;
    float height;
    if (!member->IsFlying())
        height = member->GetMap()->GetHeight(dest.m_positionX, dest.m_positionY, leaderPos.GetPositionZ() + 5.0f, true, 10.0f, member->GetCollisionHeight(), true);
    else
        height = leaderPos.GetPositionZ();

    if (height != INVALID_HEIGHT)
        dest.m_positionZ = height;
    else //no valid height found, try closer
        return CalculateMemberDestination(member, leaderPos, followAngle, followDist / 2.0f, pathAngle, ++depth);

    Trinity::NormalizeMapCoord(dest.m_positionX);
    Trinity::NormalizeMapCoord(dest.m_positionY);

    /* No ground pos update since we already do GetHeight call before
    if (!member->CanFly() && !member->IsFlying())
        member->UpdateGroundPositionZ(dest.m_positionX, dest.m_positionY, dest.m_positionZ);*/

    return dest;
}

/*
void CreatureGroup::LeaderStartPath(WaypointPath const* path, WaypointPathType type)
{
    // Generate leaderWaypointIDs (sun: this is not autoamtic and in db at TC, with point_1 and point_2 columns)

    // nothing to do on a too short path
    if (path->nodes.size() <= 3)
        return;

    auto GetAngle = [](WaypointNode const& a, WaypointNode const& b) {
        return  (a.x - b.x / a.y - b.y);
    };

    if (type == WP_PATH_TYPE_LOOP)
    {
        WaypointNode const& first = path->nodes[0];
        WaypointNode const& second = path->nodes[1];
        WaypointNode const& beforeLast = path->nodes[path->nodes.size() - 2];
        WaypointNode const& last = path->nodes[path->nodes.size() - 1];

        float firstPointAngle = GetAngle(last, first) - GetAngle(first, second);
        float lastPointAngle = GetAngle(beforeLast, last) - GetAngle(last, first);

        //If angle too great
        //Then what?
    }
    //other types?
}
*/

void CreatureGroup::LeaderMoveTo(Position const& destination, uint32 id /*= 0*/, uint32 moveType /*= 0*/, bool orientation /*= false*/)
{
    //! To do: This should probably get its own movement generator or use WaypointMovementGenerator.
    //! If the leader's path is known, member's path can be plotted as well using formation offsets.
    if (!_leader)
        return;

    Position pos(destination);
    float pathDist = _leader->GetExactDist(destination);
    float pathAngle = _leader->GetAbsoluteAngle(destination);

    for(auto & m_member : _members)
    {
        Creature* member = m_member.first;
        FormationInfo* formationInfo = m_member.second;
        if(member == _leader || !member->IsAlive() || member->IsEngaged() || formationInfo->groupAI == GROUP_AI_NONE)
            continue;

        if (formationInfo->leaderWaypointIDs[0])
        {
            for (uint8 i = 0; i < 2; ++i)
            {
                if (_leader->GetCurrentWaypointInfo().first == formationInfo->leaderWaypointIDs[i])
                {
                    formationInfo->followAngle = float(M_PI) * 2.f - formationInfo->followAngle;
                    break;
                }
            }
        }

        // sunwell: this should be automatized, if turn angle is greater than PI/2 (90°) we should swap formation angle
        if (M_PI - fabs(fabs(_leader->GetOrientation() - pathAngle) - M_PI) > M_PI*0.50f)
        {
            // sunwell: in both cases should be 2*M_PI - follow_angle
            // sunwell: also, GetCurrentWaypointID() returns 0..n-1, while point_1 must be > 0, so +1
            // sunwell: db table waypoint_data shouldn't have point id 0 and shouldn't have any gaps for this to work!
            // if (m_leader->GetCurrentWaypointID()+1 == itr->second->point_1 || m_leader->GetCurrentWaypointID()+1 == itr->second->point_2)
            formationInfo->followAngle = Position::NormalizeOrientation(formationInfo->followAngle + M_PI); //(2 * M_PI) - itr->second->follow_angle;
        }

        Position memberDest = CalculateMemberDestination(member, destination, formationInfo->followAngle, formationInfo->followDist, pathAngle);

        if (moveType == WAYPOINT_MOVE_TYPE_WALK)
            member->SetWalk(true);
        else
            member->SetWalk(false);
            
        // sunwell: change members speed basing on distance - if too far speed up, if too close slow down
        UnitMoveType mtype = Movement::SelectSpeedType(member->GetUnitMovementFlags());
        member->SetSpeedRate(mtype, _leader->GetSpeedRate(mtype) * member->GetExactDist(POSITION_GET_X_Y_Z(&memberDest)) / pathDist);

        member->SetHomePosition(POSITION_GET_X_Y_Z(&memberDest), pathAngle);
        member->GetMotionMaster()->MovePoint(0, POSITION_GET_X_Y_Z(&memberDest), true, orientation ? destination.GetOrientation() : Optional<float>());
    }
}

bool CreatureGroup::IsEngaged() const
{
    return std::any_of(_members.begin(), _members.end(), [](auto const& i) { return i.first->IsEngaged(); });
}

void CreatureGroup::Respawn()
{
    for(auto itr : _members)
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
    return std::any_of(_members.begin(), _members.end(), [](auto const& i) { return i.first->IsAlive(); });
}

void CreatureGroup::Update(uint32 diff)
{
    if(IsAlive())
    {
        if(!IsEngaged())
        {
            if(_respawnTimer < diff)
            {
                Respawn();//Respawn dead members marked as respawnable
                _respawnTimer = RESPAWN_TIMER;
            } else _respawnTimer -= diff;
        } else {
            if(_respawnTimer < RESPAWN_TIMER)
                _respawnTimer = RESPAWN_TIMER;
        }
        _justAlive = true;

        //reset corpse despawn time if loot linked
        for(auto itr : _members)
            if (itr.first->IsDead() && itr.second->linkedLoot == true)
                itr.first->ResetCorpseRemoveTime();
    } else 
    {
        if(_justAlive)
        {
            SetLootable(true);
            _justAlive = false;
        }
    }
}

void CreatureGroup::SetLootable(bool lootable)
{
    for(auto itr : _members)
        if(itr.second->linkedLoot == true)
            itr.first->SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);
}

bool CreatureGroup::IsLootLinked(Creature* member)
{
    auto itr = _members.find(member);
    if(itr != _members.end())
        return itr->second->linkedLoot;

    return false;
}

void CreatureGroup::ForEachMember(std::function<void(Creature*)> const& apply)
{
    for (auto itr : _members)
    {
        Creature* member = itr.first;
        apply(member);
    }
}

void CreatureGroup::SetMemberGroupAI(Creature* member, GroupAI ai)
{
    auto itr = _members.find(member);
    if (itr != _members.end())
        itr->second->groupAI = ai;
}