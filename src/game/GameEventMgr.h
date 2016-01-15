/*
 * Copyright (C) 2005-2008 MaNGOS <http://www.mangosproject.org/>
 *
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

#ifndef TRINITY_GAMEEVENT_H
#define TRINITY_GAMEEVENT_H

#include "Define.h"
#include "Creature.h"
#include "GameObject.h"

#define max_ge_check_delay 86400                            // 1 day in seconds

enum GameEventState
{
    GAMEEVENT_NORMAL = 0,   // standard game events
    GAMEEVENT_WORLD_INACTIVE,   // not yet started
    GAMEEVENT_WORLD_CONDITIONS,  // condition matching phase
    GAMEEVENT_WORLD_NEXTPHASE,   // conditions are met, now 'lenght' timer to start next event
    GAMEEVENT_WORLD_FINISHED    // next events are started, unapply this one
};

struct GameEventFinishCondition
{
    float reqNum;  // required number // use float, since some events use percent
    float done;    // done number
    uint32 max_world_state;  // max resource count world state update id
    uint32 done_world_state; // done resource count world state update id
};

struct GameEventQuestToEventConditionNum
{
    uint32 event_id;
    uint32 condition;
    float num;
};

struct GameEventData
{
    GameEventData() : start(1),end(0),nextstart(0),occurence(0),length(0),state(GAMEEVENT_NORMAL) {}
    time_t start;   // occurs after this time
    time_t end;     // occurs before this time
    time_t nextstart; // after this time the follow-up events count this phase completed
    uint32 occurence;   // time between end and start
    uint32 length;  // length of the event (minutes) after finishing all conditions
    GameEventState state;   // state of the game event, these are saved into the game_event table on change!
    std::map<uint32 /*condition id*/, GameEventFinishCondition> conditions;  // conditions to finish
    std::set<uint16 /*gameevent id*/> prerequisite_events;  // events that must be completed before starting this event
    std::string description;

    bool isValid() const { return ((length > 0) || (state > GAMEEVENT_NORMAL)); }
};

struct ModelEquip
{
    uint32 modelid;
    uint32 equipment_id;
    uint32 modelid_prev;
    uint32 equipement_id_prev;
};

struct NPCVendorEntry
{
    uint32 entry;                                           // creature entry
    ItemTemplate const* proto;                             // item id
    uint32 maxcount;                                        // 0 for infinite
    uint32 incrtime;                                        // time for restore items amount if maxcount != 0
    uint32 ExtendedCost;
};

class Player;

class GameEventMgr
{
   private:
        GameEventMgr();
        ~GameEventMgr() { };

    public:
        static GameEventMgr* instance()
        {
            static GameEventMgr instance;
            return &instance;
        }

        typedef std::set<uint16> ActiveEvents;
        typedef std::vector<GameEventData> GameEventDataMap;
        ActiveEvents const& GetActiveEventList() const { return m_ActiveEvents; }
        GameEventDataMap const& GetEventMap() const { return mGameEvent; }
        bool CheckOneGameEvent(uint16 entry) const;
        uint32 NextCheck(uint16 entry) const;
        void LoadFromDB();
        uint32 Update();
        bool IsActiveEvent(uint16 event_id) { return ( m_ActiveEvents.find(event_id)!=m_ActiveEvents.end()); }
        uint32 Initialize();
        bool StartEvent(uint16 event_id, bool overwrite = false);
        void StopEvent(uint16 event_id, bool overwrite = false);
        void HandleQuestComplete(uint32 quest_id);  // called on world event type quest completions
        uint32 GetNPCFlag(Creature * cr);
        /* Add a creature of given guid to an event (both in DB + in live memory). Return success.*/
        bool AddCreatureToEvent(uint32 guid, uint16 event_id);
        /* Add a gobject of given guid to an event (both in DB + in live memory). Return success. */
        bool AddGameObjectToEvent(uint32 guid, uint16 event_id);
        /* Remove a creature of given guid from all events (both in DB + in live memory). Return success. */
        bool RemoveCreatureFromEvent(uint32 guid);
        /* Remove a gameobject of given guid from all events (both in DB + in live memory). Return success. */
        bool RemoveGameObjectFromEvent(uint32 guid);
        /* Create a new game event, both in database and live memory, return success & the id of the created event in reference */
        bool CreateGameEvent(const char* name,int16& event_id);
        /* Set start time of a game event, both in database and live memory, return success */
        //bool SetEventStartTime(uint16 event_id,time_t startTime);
        /* Set end time of a game event, both in database and live memory, return success */
        //bool SetEventEndTime(uint16 event_id,time_t endTime);
        /* Return an event_id if a given creature is linked to game_event system, null otherwise. Can be negative (despawned when event is enabled and vice versa). */
        int16 GetCreatureEvent(uint32 guid);
        /* Return an event_id if a given gameobject is linked to game_event system, null otherwise. Can be negative (despawned when event is enabled and vice versa). */
        int16 GetGameObjectEvent(uint32 guid);
    private:
        void SendWorldStateUpdate(Player * plr, uint16 event_id);
        void AddActiveEvent(uint16 event_id) { m_ActiveEvents.insert(event_id); }
        void RemoveActiveEvent(uint16 event_id) { m_ActiveEvents.erase(event_id); }
        void ApplyNewEvent(uint16 event_id);
        void UnApplyEvent(uint16 event_id);
        void SpawnCreature(uint32 guid);
        void SpawnGameObject(uint32 guid);
        void GameEventSpawn(int16 event_id);
        void UnspawnCreature(uint32 guid,uint16 event_id); //despawn due to event_id
        void UnspawnGameObject(uint32 guid);
        void GameEventUnspawn(int16 event_id);
        void ChangeEquipOrModel(int16 event_id, bool activate);
        void UpdateEventQuests(uint16 event_id, bool Activate);
        void UpdateEventNPCFlags(uint16 event_id);
        void UpdateEventNPCVendor(uint16 event_id, bool activate);
        void UpdateBattlegroundSettings();
        bool CheckOneGameEventConditions(uint16 event_id);
        void SaveWorldEventStateToDB(uint16 event_id);
        bool hasCreatureQuestActiveEventExcept(uint32 quest_id, uint16 event_id);
        bool hasGameObjectQuestActiveEventExcept(uint32 quest_id, uint16 event_id);
        bool hasCreatureActiveEventExcept(uint32 creature_guid, uint16 event_id);
        bool hasGameObjectActiveEventExcept(uint32 go_guid, uint16 event_id);
    protected:
        typedef std::vector<GuidList> GameEventGuidMap;
        typedef std::pair<uint32, ModelEquip> ModelEquipPair;
        typedef std::list<ModelEquipPair> ModelEquipList;
        typedef std::vector<ModelEquipList> GameEventModelEquipMap;
        typedef std::pair<uint32, uint32> QuestRelation;
        typedef std::list<QuestRelation> QuestRelList;
        typedef std::vector<QuestRelList> GameEventQuestMap;
        typedef std::list<NPCVendorEntry> NPCVendorList;
        typedef std::vector<NPCVendorList> GameEventNPCVendorMap;
        typedef std::map<uint32 /*quest id*/, GameEventQuestToEventConditionNum> QuestIdToEventConditionMap;
        typedef std::pair<uint32 /*guid*/, uint32 /*npcflag*/> GuidNPCFlagPair;
        typedef std::list<GuidNPCFlagPair> NPCFlagList;
        typedef std::vector<NPCFlagList> GameEventNPCFlagMap;
        typedef std::pair<uint16 /*event id*/, uint32 /*menu id*/> EventNPCGossipIdPair;
        typedef std::map<uint32 /*guid*/, EventNPCGossipIdPair> GuidEventNpcGossipIdMap;
        typedef std::vector<uint32> GameEventBitmask;
        GameEventQuestMap mGameEventCreatureQuests;
        GameEventQuestMap mGameEventGameObjectQuests;
        GameEventNPCVendorMap mGameEventVendors;
        GameEventModelEquipMap mGameEventModelEquip;
        GameEventGuidMap  mGameEventCreatureGuids;
        GameEventGuidMap  mGameEventGameobjectGuids;
        GameEventDataMap  mGameEvent;
        GameEventBitmask  mGameEventBattlegroundHolidays;
        QuestIdToEventConditionMap mQuestToEventConditions;
        GameEventNPCFlagMap mGameEventNPCFlags;
        ActiveEvents m_ActiveEvents;
        bool isSystemInit;
};

#define sGameEventMgr GameEventMgr::instance()

#endif

