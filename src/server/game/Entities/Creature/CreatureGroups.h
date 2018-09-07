
#ifndef _FORMATIONS_H
#define _FORMATIONS_H

class CreatureGroup;
enum WaypointPathType : uint32;

enum GroupAI : uint8
{
    GROUP_AI_NONE           = 0, //no follow leader, no automatic mutual support
    GROUP_AI_LEADER_SUPPORT = 1, //follow leader, leader start attacking if a member start attacking
    GROUP_AI_FULL_SUPPORT   = 2, //follow leader, while group start attacking if any member start attacking

    GROUP_AI_TOTAL
};

struct FormationInfo
{
    ObjectGuid::LowType leaderSpawnId = 0;
    float followDist = 0.0f;
    float followAngle = 0.0f;
    GroupAI groupAI = GROUP_AI_FULL_SUPPORT;
    bool respawn = false;
    bool linkedLoot = false;
    uint32 leaderWaypointIDs[2] = { }; //formation does a 180° on these points. // NYI, this is somewhat automated for now

    Position originalHome; //home before being in the formation
};

class TC_GAME_API FormationMgr
{
public:
    typedef std::unordered_map<uint32/*spawnID*/, FormationInfo> CreatureGroupInfoType;

    static FormationMgr* instance();

    void AddCreatureToGroup(ObjectGuid::LowType leaderSpawnId, Creature* creature);
    void RemoveCreatureFromGroup(ObjectGuid::LowType leaderSpawnId, Creature* member);
    void RemoveCreatureFromGroup(CreatureGroup *group, Creature* member);

    //empty group then delete it
    void BreakFormation(Creature* leader);

    void LoadCreatureFormations();
    FormationInfo* GetFormationInfo(ObjectGuid::LowType spawnId);

    void AddFormationMember(ObjectGuid::LowType creature_lowguid, FormationInfo const& group_member);
private:

    FormationMgr();
    ~FormationMgr();

    CreatureGroupInfoType _creatureGroupMap;
};


#define RESPAWN_TIMER 15000

class TC_GAME_API CreatureGroup
{
    friend FormationMgr;
    private:
        Creature* _leader;
        typedef std::unordered_map<Creature*, FormationInfo*>  CreatureGroupMemberType;
        CreatureGroupMemberType _members;

        ObjectGuid::LowType _leaderSpawnId;
        bool _formed;
        bool _engaging;
        bool _justAlive; //group was alive at last update
        uint32 _respawnTimer; //time left before respawning group members with respawn flag (only decreases when out of combat)
    
    public:
        //Group cannot be created empty
        explicit CreatureGroup(uint32 id);
        ~CreatureGroup();
        
        Creature* GetLeader() const { return _leader; }
        ObjectGuid::LowType GetLeaderSpawnId() const { return _leaderSpawnId; }
        bool IsEmpty() const { return _members.empty(); }
        bool IsFormed() const { return _formed; }
        bool IsAlive() const; //true if any member is alive
        bool IsEngaged() const; //true if any member is engaged
        bool IsLootLinked(Creature* c);
        bool IsLeader(Creature const* creature) const { return _leader == creature; }

        bool HasMember(Creature* member) const { return _members.count(member) > 0; }
        //add creature to group, with current relative position to leader as position in formation, OR with position instead if specified
        void AddMember(Creature *member);
        void RemoveMember(Creature *member);
        // Reset movement for formation members (not leader)
        void FormationReset(bool dismiss);
        // Restore UNIT_DYNFLAG_LOOTABLE on creatures with linkedLoot flag
        void SetLootable(bool lootable);

        void LeaderMoveTo(Position const& destination, uint32 id = 0, uint32 moveType = 0, bool orientation = false);
        void MemberEngagingTarget(Creature* member, Unit *target);
        bool CanLeaderStartMoving() const;

        void Respawn();
        void Update(uint32 diff);
        void ForEachMember(std::function<void(Creature*)> const& apply);
        void SetMemberGroupAI(Creature* member, GroupAI ai);

private:
        //remove every members
        void EmptyFormation();
        //depth is there to avoid infinite recursive calls, let it at 0 by default
        Position CalculateMemberDestination(Creature* member, Position const& leaderPos, float followAngle, float followDist, float pathAngle, uint8 depth = 0) const;
};

#define sFormationMgr FormationMgr::instance()

#endif
