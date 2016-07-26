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

#ifndef _FORMATIONS_H
#define _FORMATIONS_H

class CreatureGroup;

enum GroupAI
{
    GROUP_AI_NONE           = 0, //no follow leader, no automatic mutual support
    GROUP_AI_LEADER_SUPPORT = 1, //follow leader, leader start attacking if a member start attacking
    GROUP_AI_FULL_SUPPORT   = 2, //follow leader, while group start attacking if any member start attacking
};

struct FormationInfo
{
    uint32 leaderGUID = 0;
    float follow_dist = 0.0f;
    float follow_angle = 0.0f;
    GroupAI groupAI = GROUP_AI_FULL_SUPPORT;
    bool respawn = false;
    bool linkedLoot = false;
};

typedef std::unordered_map<uint32/*memberDBGUID*/, FormationInfo*>   CreatureGroupInfoType;

struct MemberPosition
{
	float follow_angle;
	float follow_dist;
};

class CreatureGroupManager
{
    public:

        static CreatureGroupManager* instance()
        {
            static CreatureGroupManager instance;
            return &instance;
        }

        ~CreatureGroupManager();
        
		//add creature to group (or create it if it does not exists), with current relative position to leader as position in formation, OR with position instead if specified
        void AddCreatureToGroup(uint32 group_id, Creature* member, MemberPosition* position = nullptr);
        void RemoveCreatureFromGroup(uint32 group_id, Creature* member);
        void RemoveCreatureFromGroup(CreatureGroup *group, Creature* member);
        //empty group then delete it
        void BreakFormation(Creature* leader);
        void LoadCreatureFormations();

        void AddGroupMember(uint32 creature_lowguid, FormationInfo* group_member);

        CreatureGroupInfoType const& GetGroupMap()
        {
            return CreatureGroupMap;
        };

        //get guid for storing in group map
        static uint32 GetCreatureGUIDForStore(Creature* member);
    private:
        CreatureGroupInfoType CreatureGroupMap;

        void Clear(); //clear memory from CreatureGroupMap
};


#define RESPAWN_TIMER 15000

class CreatureGroup
{
    friend CreatureGroupManager;
    private:
        Creature* m_leader; //Important do not forget sometimes to work with pointers instead synonims :D:D
        typedef std::map<Creature*, FormationInfo*>  CreatureGroupMemberType;
        CreatureGroupMemberType m_members;

        uint32 m_groupID;
        bool m_Formed;
        bool inCombat;
        bool justAlive;
        uint32 respawnTimer;
    
    public:
        //Group cannot be created empty
        explicit CreatureGroup(uint32 id) : m_groupID(id), m_leader(NULL), m_Formed(false), inCombat(false), justAlive(true), respawnTimer(RESPAWN_TIMER) {}
        ~CreatureGroup();
        
        Creature* getLeader() const { return m_leader; }
        uint32 GetId() const { return m_groupID; }
        bool isEmpty() const { return m_members.empty(); }
        bool isFormed() const { return m_Formed; }
        bool IsAlive() const; //true if any member is alive
        bool isLootLinked(Creature* c);

		//add creature to group, with current relative position to leader as position in formation, OR with position instead if specified
        void AddMember(Creature *member, MemberPosition* pos = nullptr);
        void RemoveMember(Creature *member);
        void FormationReset(bool dismiss);
        void SetLootable(bool lootable);

        void LeaderMoveTo(float x, float y, float z, bool run);
        void MemberAttackStart(Creature* member, Unit *target);

        void UpdateCombat();
        void Respawn();
        void Update(uint32 diff);

private:
        //remove every members
        void EmptyFormation();
        //depth is there to avoid infinite recursive calls, let it at 0 by default
        Position CalculateMemberDestination(Creature* member, Position const& leaderPos, float followAngle, float followDist, float pathAngle, uint8 depth = 0) const;
};

#define sCreatureGroupMgr CreatureGroupManager::instance()

#endif
