#include "Creature.h"
#include "CreatureGroups.h"
#include "ObjectMgr.h"
#include "CreatureAI.h"
#include "MoveSplineInit.h"
#include "WaypointMovementGenerator.h"

/*
Sunstrider: What changed from TC:
We now handle creature summons, for this we had to change:
- Bring back the concept of groupID, which is usually still leader spawn id but may be creature lowguid + SUMMON_OFFSET in summon cases
- The FormationInfo for each member is now stored into CreatureGroup, instead of CreatureGroup referencing _creatureGroupMap.
  _creatureGroupMap still contains the FormationInfo from database and a copy is made whenever we create a non summon creature.
- CreatureGroup::AddMember will create a default FormationInfo is none is found (it will be the case if a summon is added)

We also added concepts off:
- Linked loot: Members with the flag will only be lootable whenever the whole group is dead
- Linked respawn: Members with this flag will respawn after RESPAWN_TIMER if any member of the group is alive and nobody in the group is engaged

Finally:
- Our DB angle are radiant and not degrees
- GroupAI used an index instead of flags. No particular reason, I just didn't upgrade it.
- Some functions return objects to simplify usage, such as AddCreatureToGroup returning the group pointer
- Npc addformation is using the position relative to the leader and instead of the player to generate distance and angle. (We found this more intuitive)
*/

enum SpecialValues
{
    SUMMON_OFFSET = 1000000,
};

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

uint32 FormationMgr::GetInternalGroupId(Creature const* leader)
{
    uint32 internalId = leader->GetSpawnId();
    if (!internalId)
    {
        uint32 guid = leader->GetGUID().GetCounter();
        if (uint64(guid) + SUMMON_OFFSET > std::numeric_limits<ObjectGuid::LowType>::max()) //make sure we're not overflowing
            ABORT("FormationMgr::GetInternalGroupId Group internal id overflow");

        internalId = SUMMON_OFFSET + guid;
    }
    return internalId;
}

CreatureGroup* FormationMgr::AddCreatureToGroup(Creature* leader, Creature* creature)
{
    uint32 internalId = FormationMgr::GetInternalGroupId(leader);
    return AddCreatureToGroup(internalId, creature);
}

CreatureGroup* FormationMgr::AddCreatureToGroup(uint32 groupID, Creature* creature)
{
    if (groupID == 0)
    {
        TC_LOG_ERROR("entities.unit", "AddCreatureToGroup: invalid leaderInternalId %u given", groupID);
        return nullptr;
    }

    Map* map = creature->GetMap();

    auto itr = map->CreatureGroupHolder.find(groupID);
    if (itr != map->CreatureGroupHolder.end())
    {
        //Add member to an existing group
        TC_LOG_DEBUG("entities.unit", "Group found: %u, inserting creature GUID: %u, Group InstanceID %u", groupID, creature->GetGUID().GetCounter(), creature->GetInstanceId());

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
        TC_LOG_DEBUG("entities.unit", "Group not found: %u. Creating new group.", groupID);
        CreatureGroup* group = new CreatureGroup(groupID);
        std::tie(itr, std::ignore) = map->CreatureGroupHolder.emplace(groupID, group);
    }

    itr->second->AddMember(creature);
    return itr->second;
}

void FormationMgr::RemoveCreatureFromGroup(uint32 groupId, Creature *member)
{
    Map* map = member->GetMap();
    auto itr = map->CreatureGroupHolder.find(groupId);
    if (itr == map->CreatureGroupHolder.end())
        return;

    RemoveCreatureFromGroup(itr->second, member);
}

void FormationMgr::RemoveCreatureFromGroup(CreatureGroup* group, Creature* member)
{
    TC_LOG_DEBUG("entities.unit", "Deleting member pointer to GUID: %u from group %u", group->GetGroupId(), member->GetSpawnId());
    group->RemoveMember(member);

    if (group->IsEmpty())
    {
        Map* map = member->GetMap();

        TC_LOG_DEBUG("entities.unit", "Deleting group with InstanceID %u", member->GetInstanceId());
        auto itr = map->CreatureGroupHolder.find(group->GetGroupId());
        ASSERT(itr != map->CreatureGroupHolder.end(), "Not registered group %u in map %u", group->GetGroupId(), map->GetId());
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
    do
    {
        Field* fields = result->Fetch();

        //Load group member data
        FormationInfo member;
        uint32 index = 0;
        member.groupID                    = fields[index++].GetUInt32();
        ObjectGuid::LowType memberSpawnId = fields[index++].GetUInt32();
        member.followDist                 = fields[index++].GetFloat();
        member.followAngle                = fields[index++].GetFloat();
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

            if (!sObjectMgr->GetCreatureData(member.groupID))
            {
                TC_LOG_ERROR("sql.sql", "creature_formations table leader guid %u incorrect (not exist)", member.groupID);
                continue;
            }

            if (!sObjectMgr->GetCreatureData(memberSpawnId))
            {
                TC_LOG_ERROR("sql.sql", "creature_formations table member guid %u incorrect (not exist)", memberSpawnId);
                continue;
            }
        }

        _creatureGroupMap.emplace(memberSpawnId, std::move(member));
        count++;
    }
    while(result->NextRow()) ;

    TC_LOG_INFO("server.loading", ">> Loaded %u creatures in formations in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

FormationInfo* FormationMgr::GetFormationInfo(uint32 spawnId)
{
    return Trinity::Containers::MapGetValuePtr(_creatureGroupMap, spawnId);
}

void FormationMgr::AddFormationMember(uint32 internalId, FormationInfo const& group_member)
{
    _creatureGroupMap[internalId] = group_member;
}

void FormationMgr::BreakFormation(Creature* leader)
{
    CreatureGroup* group = leader->GetFormation();
    if (!group)
        return;

    group->EmptyFormation();
    leader->GetMap()->CreatureGroupHolder.erase(group->GetGroupId());
    delete group;
}

CreatureGroup::CreatureGroup(uint32 id) :
    _leader(nullptr),
    _groupId(id),
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

FormationInfo& CreatureGroup::AddMember(Creature* member)
{
    TC_LOG_DEBUG("entities.unit", "CreatureGroup::AddMember: Adding unit GUID: %u.", member->GetGUID().GetCounter());
    
    uint32 groupID = FormationMgr::GetInternalGroupId(member);

    //Check if it is a leader
    if (groupID == _groupId)
    {
        TC_LOG_DEBUG("entities.unit", "Unit GUID: %u is formation leader. Adding group.", member->GetGUID().GetCounter());
        _leader = member;
    }

    FormationInfo* dbFormationInfo = sFormationMgr->GetFormationInfo(groupID);
    FormationInfo formationInfo;
    if (dbFormationInfo)
        formationInfo = *dbFormationInfo;
    else
    { //generate default from current creature position. This is currently used by SMART_ACTION_ADD_TO_FORMATION
        if (_leader)
        {
            formationInfo.followAngle = _leader->GetAbsoluteAngle(member) - _leader->GetOrientation();
            formationInfo.followDist = sqrtf(pow(_leader->GetPositionX() - member->GetPositionX(), int(2)) + pow(_leader->GetPositionY() - member->GetPositionY(), int(2)));
        } else
            TC_LOG_WARN("entities.unit", "CreatureGroup::AddMember Adding unit (GUID: %u) to group (%u) which has currently no leader. No follow dist/angles were generated.", member->GetGUID().GetCounter(), groupID);
    }
    //else we keep default values from FormationInfo constructor

    formationInfo.originalHome = member->GetHomePosition();
    member->SetFormation(this);
    //std::map::emplace: Returns a pair consisting of an iterator to the inserted element, or the already-existing element if no insertion happened, and a bool denoting whether the insertion took place. True for Insertion, False for No Insertion.
    auto newElementItr = _members.emplace(member, std::move(formationInfo)).first;
    return newElementItr->second;
}

void CreatureGroup::RemoveMember(Creature *member)
{
    if (_leader == member)
        _leader = nullptr;

    auto itr = _members.find(member);
    if (itr != _members.end())
    {
        //restore original home (for ex: DarkPortalEventDemonAI rely on this)
        member->SetHomePosition(itr->second.originalHome);
        _members.erase(member);
    }
    member->SetFormation(nullptr);
}

void CreatureGroup::MemberEngagingTarget(Creature *member, Unit *target)
{
    // used to prevent recursive calls
    if (_engaging)
        return;
    
    auto itr = _members.find(member);
    if (itr == _members.end())
        return;

    GroupAI groupAI = itr->second.groupAI;
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

        if(other->_CanCreatureAttack(target) == CAN_ATTACK_RESULT_OK)
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

Creature* followedCreature2 = nullptr;

void CreatureGroup::LeaderMoveTo(Position const& destination, uint32 id /*= 0*/, uint32 moveType /*= 0*/, bool orientation /*= false*/)
{
    if (!_leader)
        return;

    /*Position pos(destination);
    float pathDist = _leader->GetExactDist(destination);
    float pathAngle = _leader->GetAbsoluteAngle(destination);*/

    for(auto & m_member : _members)
    {
        Creature* member = m_member.first;
        FormationInfo& formationInfo = m_member.second;
        if(member == _leader || !member->IsAlive() || member->IsEngaged() || formationInfo.groupAI == GROUP_AI_NONE)
            continue;

        /*TC if (formationInfo.leaderWaypointIDs[0])
        {
            for (uint8 i = 0; i < 2; ++i)
            {
                if (_leader->GetCurrentWaypointInfo().first == formationInfo.leaderWaypointIDs[i])
                {
                    formationInfo.followAngle = float(M_PI) * 2.f - formationInfo.followAngle;
                    break;
                }
            }
        }*/

        if (member == followedCreature2)
            member = followedCreature2;

        FormationMoveSegment path(_leader->GetPosition(), destination, moveType, orientation, formationInfo.followAngle, formationInfo.followDist);
        member->GetMotionMaster()->MoveFormation(id, path, _leader);
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
        if(!itr.second.respawn)
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
            if (itr.first->IsDead() && itr.second.linkedLoot == true)
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
        if(itr.second.linkedLoot == true)
            itr.first->SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);
}

bool CreatureGroup::IsLootLinked(Creature* member)
{
    auto itr = _members.find(member);
    if(itr != _members.end())
        return itr->second.linkedLoot;

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
        itr->second.groupAI = ai;
}