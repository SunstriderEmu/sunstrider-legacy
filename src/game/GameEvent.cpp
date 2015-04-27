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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "GameEvent.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Language.h"
#include "Log.h"
#include "MapManager.h"
#include "Policies/SingletonImp.h"
#include "GossipDef.h"
#include "Player.h"
#include "BattleGroundMgr.h"
#include "GridNotifiers.h"

INSTANTIATE_SINGLETON_1(GameEvent);

bool GameEvent::CheckOneGameEvent(uint16 entry) const
{
    time_t currenttime = time(NULL);
    // if the state is conditions or nextphase, then the event should be active
    if (mGameEvent[entry].state == GAMEEVENT_WORLD_CONDITIONS || mGameEvent[entry].state == GAMEEVENT_WORLD_NEXTPHASE) {
        return true;
    }
    // finished world events are inactive
    else if (mGameEvent[entry].state == GAMEEVENT_WORLD_FINISHED) {
        return false;
    }
    // if inactive world event, check the prerequisite events
    else if (mGameEvent[entry].state == GAMEEVENT_WORLD_INACTIVE)
    {
        for(auto itr : mGameEvent[entry].prerequisite_events)
        {
            if( (mGameEvent[itr].state != GAMEEVENT_WORLD_NEXTPHASE && mGameEvent[itr].state != GAMEEVENT_WORLD_FINISHED) ||   // if prereq not in nextphase or finished state, then can't start this one
                mGameEvent[itr].nextstart > currenttime)               // if not in nextphase state for long enough, can't start this one
                return false;
        }
        // all prerequisite events are met
        // but if there are no prerequisites, this can be only activated through gm command
        return !(mGameEvent[entry].prerequisite_events.empty());
    }

    // Get the event information
    if( mGameEvent[entry].start < currenttime && currenttime < mGameEvent[entry].end &&
        ((currenttime - mGameEvent[entry].start) % (mGameEvent[entry].occurence * MINUTE)) < (mGameEvent[entry].length * MINUTE) )
        return true;
    else
        return false;
}

uint32 GameEvent::NextCheck(uint16 entry) const
{
    time_t currenttime = time(NULL);

    // for NEXTPHASE state world events, return the delay to start the next event, so the followup event will be checked correctly
    if ((mGameEvent[entry].state == GAMEEVENT_WORLD_NEXTPHASE || mGameEvent[entry].state == GAMEEVENT_WORLD_FINISHED) && mGameEvent[entry].nextstart >= currenttime)
        return (mGameEvent[entry].nextstart - currenttime);

    // for CONDITIONS state world events, return the length of the wait period, so if the conditions are met, this check will be called again to set the timer as NEXTPHASE event
    if (mGameEvent[entry].state == GAMEEVENT_WORLD_CONDITIONS)
        return mGameEvent[entry].length ? mGameEvent[entry].length * 60 : max_ge_check_delay;

    // outdated event: we return max
    if (currenttime > mGameEvent[entry].end)
        return max_ge_check_delay;

    // never started event, we return delay before start

    if (mGameEvent[entry].start > currenttime)
        return (mGameEvent[entry].start - currenttime);

    uint32 delay;
    // in event, we return the end of it
    if ((((currenttime - mGameEvent[entry].start) % (mGameEvent[entry].occurence * 60)) < (mGameEvent[entry].length * 60)))
        // we return the delay before it ends
        delay = (mGameEvent[entry].length * MINUTE) - ((currenttime - mGameEvent[entry].start) % (mGameEvent[entry].occurence * MINUTE));
    else                                                    // not in window, we return the delay before next start
        delay = (mGameEvent[entry].occurence * MINUTE) - ((currenttime - mGameEvent[entry].start) % (mGameEvent[entry].occurence * MINUTE));
    // In case the end is before next check
    if (mGameEvent[entry].end  < time_t(currenttime + delay))
        return (mGameEvent[entry].end - currenttime);
    else
        return delay;
}

bool GameEvent::StartEvent( uint16 event_id, bool overwrite )
{
    // Temp hack until game_event dependencies is implemented
    if ((event_id >= 50 && event_id <= 56) && !IsActiveEvent(12))
        return false;

    if(mGameEvent[event_id].state == GAMEEVENT_NORMAL)
    {
        AddActiveEvent(event_id);
        ApplyNewEvent(event_id);
        if(overwrite)
        {
            mGameEvent[event_id].start = time(NULL);
            if(mGameEvent[event_id].end <= mGameEvent[event_id].start)
                mGameEvent[event_id].end = mGameEvent[event_id].start+mGameEvent[event_id].length;
        }
        return false;
    }
    else
    {
        if( mGameEvent[event_id].state == GAMEEVENT_WORLD_INACTIVE )
            // set to conditions phase
            mGameEvent[event_id].state = GAMEEVENT_WORLD_CONDITIONS;

        // add to active events
        AddActiveEvent(event_id);
        // add spawns
        ApplyNewEvent(event_id);

        // check if can go to next state
        bool conditions_met = CheckOneGameEventConditions(event_id);
        // save to db
        SaveWorldEventStateToDB(event_id);
        // force game event update to set the update timer if conditions were met from a command
        // this update is needed to possibly start events dependent on the started one
        // or to scedule another update where the next event will be started
        if(overwrite && conditions_met)
            sWorld->ForceGameEventUpdate();

        return conditions_met;
    }
}

void GameEvent::StopEvent( uint16 event_id, bool overwrite )
{
    bool serverwide_evt = mGameEvent[event_id].state != GAMEEVENT_NORMAL;

    RemoveActiveEvent(event_id);
    UnApplyEvent(event_id);

    if(overwrite && !serverwide_evt)
    {
        mGameEvent[event_id].start = time(NULL) - mGameEvent[event_id].length * MINUTE;
        if(mGameEvent[event_id].end <= mGameEvent[event_id].start)
            mGameEvent[event_id].end = mGameEvent[event_id].start+mGameEvent[event_id].length;
    }
    else if(serverwide_evt)
    {
        // if finished world event, then only gm command can stop it
        if(overwrite || mGameEvent[event_id].state != GAMEEVENT_WORLD_FINISHED)
        {
            // reset conditions
            mGameEvent[event_id].nextstart = 0;
            mGameEvent[event_id].state = GAMEEVENT_WORLD_INACTIVE;
            std::map<uint32 /*condition id*/, GameEventFinishCondition>::iterator itr;
            for(itr = mGameEvent[event_id].conditions.begin(); itr != mGameEvent[event_id].conditions.end(); ++itr)
                itr->second.done = 0;
            SQLTransaction trans = CharacterDatabase.BeginTransaction();
            trans->PAppend("DELETE FROM game_event_save WHERE event_id = '%u'",event_id);
            trans->PAppend("DELETE FROM game_event_condition_save WHERE event_id = '%u'",event_id);
            CharacterDatabase.CommitTransaction(trans);
        }
    }
}

void GameEvent::LoadFromDB()
{
    //reload case
    
    mGameEvent.clear();
    mGameEventCreatureQuests.clear();
    mGameEventGameObjectQuests.clear();
    mGameEventVendors.clear();
    mGameEventModelEquip.clear();
    mGameEventCreatureGuids.clear();
    mGameEventGameobjectGuids.clear();
    mGameEventBattlegroundHolidays.clear();
    
    {
        QueryResult result = WorldDatabase.Query("SELECT MAX(entry) FROM game_event");
        if( !result )
        {
            TC_LOG_INFO("gameevent",">> Table game_event is empty.");
            TC_LOG_INFO("gameevent"," ");
            return;
        }

        Field *fields = result->Fetch();

        uint32 max_event_id = fields[0].GetUInt16();

        mGameEvent.resize(max_event_id+1);
    }

    QueryResult result = WorldDatabase.Query("SELECT entry,UNIX_TIMESTAMP(start_time),UNIX_TIMESTAMP(end_time),occurence,length,description,world_event FROM game_event");
    if( !result )
    {
        mGameEvent.clear();
        TC_LOG_INFO("gameevent",">> Table game_event is empty:");
        TC_LOG_INFO("gameevent"," ");
        return;
    }

    uint32 count = 0;

    do
    {
        ++count;
        Field *fields = result->Fetch();

        uint16 event_id = fields[0].GetUInt16();
        if(event_id==0)
        {
            TC_LOG_ERROR("gameevent","`game_event` game event id (%i) is reserved and can't be used.",event_id);
            continue;
        }

        GameEventData& pGameEvent = mGameEvent[event_id];
        uint64 starttime        = fields[1].GetUInt64();
        pGameEvent.start        = time_t(starttime);
        uint64 endtime          = fields[2].GetUInt64();
        pGameEvent.end          = time_t(endtime);
        pGameEvent.occurence    = fields[3].GetUInt32();
        pGameEvent.length       = fields[4].GetUInt32();
        pGameEvent.description  = fields[5].GetString();
        pGameEvent.state        = (GameEventState)(fields[6].GetUInt8());
        pGameEvent.nextstart    = 0;

        if(pGameEvent.length==0 && pGameEvent.state == GAMEEVENT_NORMAL)                            // length>0 is validity check
        {
            TC_LOG_ERROR("gameevent","`game_event` game event id (%i) isn't a world event and has length = 0, thus it can't be used.",event_id);
            continue;
        }

    /*    if( pGameEvent.occurence == 0 )
             pGameEvent.occurence = -1; //set a default value instead */

    } while( result->NextRow() );

    TC_LOG_INFO("gameevent"," ");
    TC_LOG_INFO( "gameevent",">> Loaded %u game events", count );

    // load game event saves
    //                                       0         1      2
    result = CharacterDatabase.Query("SELECT event_id, state, UNIX_TIMESTAMP(next_start) FROM game_event_save");

    count = 0;
    if( !result )
    {
        TC_LOG_INFO("gameevent"," ");
        TC_LOG_INFO("server.loading", ">> Loaded %u game event saves in game events", count );
    }
    else
    {
        do
        {
            Field *fields = result->Fetch();

            uint16 event_id = fields[0].GetUInt16();

            if(event_id >= mGameEvent.size())
            {
                TC_LOG_ERROR("gameevent","`game_event_save` game event id (%i) is out of range compared to max event id in `game_event`",event_id);
                continue;
            }

            if(mGameEvent[event_id].state != GAMEEVENT_NORMAL)
            {
                mGameEvent[event_id].state = (GameEventState)(fields[1].GetUInt8());
                mGameEvent[event_id].nextstart    = time_t(fields[2].GetUInt64());
            }
            else
            {
                TC_LOG_ERROR("gameevent","game_event_save includes event save for non-worldevent id %u",event_id);
                continue;
            }

            ++count;

        } while( result->NextRow() );
        TC_LOG_INFO("gameevent"," ");
        TC_LOG_INFO("server.loading",">> Loaded %u game event saves in game events", count );
    }

    // load game event links (prerequisites)
    result = WorldDatabase.Query("SELECT event_id, prerequisite_event FROM game_event_prerequisite");
    if( !result )
    {
        TC_LOG_INFO("gameevent"," ");
        TC_LOG_INFO("server.loading", ">> Loaded %u game event prerequisites in game events", count );
    }
    else
    {
        do
        {
            Field *fields = result->Fetch();

            uint16 event_id = fields[0].GetUInt16();

            if(event_id >= mGameEvent.size())
            {
                TC_LOG_ERROR("gameevent","`game_event_prerequisite` game event id (%i) is out of range compared to max event id in `game_event`",event_id);
                continue;
            }


            if(mGameEvent[event_id].state != GAMEEVENT_NORMAL)
            {
                uint16 prerequisite_event = fields[1].GetUInt16();
                if(prerequisite_event >= mGameEvent.size())
                {
                    TC_LOG_ERROR("gameevent","`game_event_prerequisite` game event prerequisite id (%i) is out of range compared to max event id in `game_event`",prerequisite_event);
                    continue;
                }
                mGameEvent[event_id].prerequisite_events.insert(prerequisite_event);
            }
            else
            {
                TC_LOG_ERROR("gameevent","game_event_prerequisiste includes event entry for non-worldevent id %u",event_id);
                continue;
            }

            ++count;

        } while( result->NextRow() );
        TC_LOG_INFO("gameevent"," ");
        TC_LOG_INFO("server.loading", ">> Loaded %u game event prerequisites in game events", count );
    }

    mGameEventCreatureGuids.resize(mGameEvent.size()*2-1);
    //                                   1              2
    result = WorldDatabase.Query("SELECT creature.guid, game_event_creature.event "
        "FROM creature JOIN game_event_creature ON creature.guid = game_event_creature.guid");

    count = 0;
    if( !result )
    {
        TC_LOG_INFO("gameevent"," ");
        TC_LOG_INFO("server.loading",">> Loaded %u creatures in game events", count );
    }
    else
    {
        do
        {
            Field *fields = result->Fetch();

            uint32 guid    = fields[0].GetUInt32();
            int16 event_id = fields[1].GetInt16();

            int32 internal_event_id = mGameEvent.size() + event_id - 1;

            if(internal_event_id < 0 || internal_event_id >= mGameEventCreatureGuids.size())
            {
                TC_LOG_ERROR("gameevent","`game_event_creature` game event id (%i) is out of range compared to max event id in `game_event`",event_id);
                continue;
            }

            ++count;
            GuidList& crelist = mGameEventCreatureGuids[internal_event_id];
            crelist.push_back(guid);

        } while( result->NextRow() );
        TC_LOG_INFO("gameevent"," ");
        TC_LOG_INFO( "server.loading",">> Loaded %u creatures in game events", count );
    }

    mGameEventGameobjectGuids.resize(mGameEvent.size()*2-1);
    //                                   1                2
    result = WorldDatabase.Query("SELECT gameobject.guid, game_event_gameobject.event "
        "FROM gameobject JOIN game_event_gameobject ON gameobject.guid=game_event_gameobject.guid");

    count = 0;
    if( !result )
    {
        TC_LOG_INFO("gameevent"," ");
        TC_LOG_INFO("server.loading",">> Loaded %u gameobjects in game events", count );
    }
    else
    {
        do
        {
            Field *fields = result->Fetch();

            uint32 guid    = fields[0].GetUInt32();
            int16 event_id = fields[1].GetInt16();

            int32 internal_event_id = mGameEvent.size() + event_id - 1;

            if(internal_event_id < 0 || internal_event_id >= mGameEventGameobjectGuids.size())
            {
                TC_LOG_ERROR("gameevent","`game_event_gameobject` game event id (%i) is out of range compared to max event id in `game_event`",event_id);
                continue;
            }

            ++count;
            GuidList& golist = mGameEventGameobjectGuids[internal_event_id];
            golist.push_back(guid);

        } while( result->NextRow() );
        TC_LOG_INFO("gameevent"," ");
        TC_LOG_INFO("server.loading", ">> Loaded %u gameobjects in game events", count );
    }

    mGameEventModelEquip.resize(mGameEvent.size());
    //                                   0              1                             2
    result = WorldDatabase.Query("SELECT creature.guid, game_event_model_equip.event, game_event_model_equip.modelid,"
    //   3
        "game_event_model_equip.equipment_id "
        "FROM creature JOIN game_event_model_equip ON creature.guid=game_event_model_equip.guid");

    count = 0;
    if( !result )
    {
        TC_LOG_INFO("gameevent"," ");
        TC_LOG_INFO("server.loading",">> Loaded %u model/equipment changes in game events", count );
    }
    else
    {
        do
        {
            Field *fields = result->Fetch();

            uint32 guid     = fields[0].GetUInt32();
            uint16 event_id = fields[1].GetUInt16();

            if(event_id >= mGameEventModelEquip.size())
            {
                TC_LOG_ERROR("gameevent","`game_event_model_equip` game event id (%u) is out of range compared to max event id in `game_event`",event_id);
                continue;
            }

            ++count;
            ModelEquipList& equiplist = mGameEventModelEquip[event_id];
            ModelEquip newModelEquipSet;
            newModelEquipSet.modelid = fields[2].GetUInt32();
            newModelEquipSet.equipment_id = fields[3].GetUInt32();
            newModelEquipSet.equipement_id_prev = 0;
            newModelEquipSet.modelid_prev = 0;

            if(newModelEquipSet.equipment_id > 0)
            {
                if(!sObjectMgr->GetEquipmentInfo(newModelEquipSet.equipment_id))
                {
                    TC_LOG_ERROR("gameevent","Table `game_event_model_equip` have creature (Guid: %u) with equipment_id %u not found in table `creature_equip_template`, set to no equipment.", guid, newModelEquipSet.equipment_id);
                    continue;
                }
            }

            equiplist.push_back(std::pair<uint32, ModelEquip>(guid, newModelEquipSet));

        } while( result->NextRow() );
        TC_LOG_INFO("gameevent"," ");
        TC_LOG_INFO( "server.loading",">> Loaded %u model/equipment changes in game events", count );
    }

    mGameEventCreatureQuests.resize(mGameEvent.size());
    //                                   0   1      2
    result = WorldDatabase.Query("SELECT id, quest, event FROM game_event_creature_quest");

    count = 0;
    if( !result )
    {
        TC_LOG_INFO("gameevent"," ");
        TC_LOG_INFO("server.loading",">> Loaded %u quests additions in game events", count );
    }
    else
    {
        do
        {
            Field *fields = result->Fetch();

            uint32 id       = fields[0].GetUInt32();
            uint32 quest    = fields[1].GetUInt32();
            uint16 event_id = fields[2].GetUInt16();

            if(event_id >= mGameEventCreatureQuests.size())
            {
                TC_LOG_ERROR("gameevent","`game_event_creature_quest` game event id (%u) is out of range compared to max event id in `game_event`",event_id);
                continue;
            }

            ++count;
            QuestRelList& questlist = mGameEventCreatureQuests[event_id];
            questlist.push_back(QuestRelation(id, quest));

        } while( result->NextRow() );
        TC_LOG_INFO("gameevent"," ");
        TC_LOG_INFO("server.loading", ">> Loaded %u quests additions in game events", count );
    }

    mGameEventGameObjectQuests.resize(mGameEvent.size());
    //                                   0   1      2
    result = WorldDatabase.Query("SELECT id, quest, event FROM game_event_gameobject_quest");

    count = 0;
    if( !result )
    {
        TC_LOG_INFO("gameevent"," ");
        TC_LOG_INFO("server.loading",">> Loaded %u go quests additions in game events", count );
    }
    else
    {
        do
        {
            Field *fields = result->Fetch();

            uint32 id       = fields[0].GetUInt32();
            uint32 quest    = fields[1].GetUInt32();
            uint16 event_id = fields[2].GetUInt16();

            if(event_id >= mGameEventGameObjectQuests.size())
            {
                TC_LOG_ERROR("gameevent","`game_event_gameobject_quest` game event id (%u) is out of range compared to max event id in `game_event`",event_id);
                continue;
            }

            ++count;
            QuestRelList& questlist = mGameEventGameObjectQuests[event_id];
            questlist.push_back(QuestRelation(id, quest));

        } while( result->NextRow() );
        TC_LOG_INFO("gameevent"," ");
        TC_LOG_INFO( "server.loading",">> Loaded %u quests additions in game events", count );
    }

    // Load quest to (event,condition) mapping
    //                                   0      1         2             3
    result = WorldDatabase.Query("SELECT quest, event_id, condition_id, num FROM game_event_quest_condition");

    count = 0;
    if( !result )
    {
        TC_LOG_INFO("gameevent"," ");
        TC_LOG_INFO("server.loading",">> Loaded %u quest event conditions in game events", count );
    }
    else
    {
        do
        {
            Field *fields = result->Fetch();

            uint32 quest     = fields[0].GetUInt32();
            uint16 event_id  = fields[1].GetUInt16();
            uint32 condition = fields[2].GetUInt32();
            float num       = fields[3].GetFloat();

            if(event_id >= mGameEvent.size())
            {
                TC_LOG_ERROR("gameevent","`game_event_quest_condition` game event id (%u) is out of range compared to max event id in `game_event`",event_id);
                continue;
            }

            ++count;
            mQuestToEventConditions[quest].event_id = event_id;
            mQuestToEventConditions[quest].condition = condition;
            mQuestToEventConditions[quest].num = num;

        } while( result->NextRow() );
        TC_LOG_INFO("gameevent"," ");
        TC_LOG_INFO("server.loading", ">> Loaded %u quest event conditions in game events", count );
    }

    // load conditions of the events
    //                                   0         1             2        3                      4
    result = WorldDatabase.Query("SELECT event_id, condition_id, req_num, max_world_state_field, done_world_state_field FROM game_event_condition");

    count = 0;
    if( !result )
    {
        TC_LOG_INFO("gameevent"," ");
        TC_LOG_INFO("server.loading",">> Loaded %u conditions in game events", count );
    }
    else
    {
        do
        {
            Field *fields = result->Fetch();

            uint16 event_id  = fields[0].GetUInt16();
            uint32 condition = fields[1].GetUInt32();

            if(event_id >= mGameEvent.size())
            {
                TC_LOG_ERROR("gameevent","`game_event_condition` game event id (%u) is out of range compared to max event id in `game_event`",event_id);
                continue;
            }

            mGameEvent[event_id].conditions[condition].reqNum = fields[2].GetFloat();
            mGameEvent[event_id].conditions[condition].done = 0;
            mGameEvent[event_id].conditions[condition].max_world_state = fields[3].GetUInt32();
            mGameEvent[event_id].conditions[condition].done_world_state = fields[4].GetUInt32();

            ++count;

        } while( result->NextRow() );
        TC_LOG_INFO("gameevent"," ");
        TC_LOG_INFO("server.loading", ">> Loaded %u conditions in game events", count );
    }

    // load condition saves
    //                                       0         1             2
    result = CharacterDatabase.Query("SELECT event_id, condition_id, done FROM game_event_condition_save");

    count = 0;
    if( !result )
    {
        TC_LOG_INFO("gameevent"," ");
        TC_LOG_INFO("server.loading",">> Loaded %u condition saves in game events", count );
    }
    else
    {
        do
        {
            Field *fields = result->Fetch();

            uint16 event_id  = fields[0].GetUInt16();
            uint32 condition = fields[1].GetUInt32();

            if(event_id >= mGameEvent.size())
            {
                TC_LOG_ERROR("gameevent","`game_event_condition_save` game event id (%u) is out of range compared to max event id in `game_event`",event_id);
                continue;
            }

            std::map<uint32, GameEventFinishCondition>::iterator itr = mGameEvent[event_id].conditions.find(condition);
            if(itr != mGameEvent[event_id].conditions.end())
            {
                itr->second.done = fields[2].GetFloat();
            }
            else
            {
                TC_LOG_ERROR("gameevent","game_event_condition_save contains not present condition evt id %u cond id %u",event_id, condition);
                continue;
            }

            ++count;

        } while( result->NextRow() );
        TC_LOG_INFO("server.loading", " ");
        TC_LOG_INFO( "server.loading", ">> Loaded %u condition saves in game events", count );
    }

    mGameEventNPCFlags.resize(mGameEvent.size());
    // load game event npcflag
    //                                   0         1        2
    result = WorldDatabase.Query("SELECT guid, event_id, npcflag FROM game_event_npcflag");

    count = 0;
    if( !result )
    {
        TC_LOG_INFO("gameevent"," ");
        TC_LOG_INFO("server.loading",">> Loaded %u npcflags in game events", count );
    }
    else
    {
        do
        {
            Field *fields = result->Fetch();

            uint32 guid     = fields[0].GetUInt32();
            uint16 event_id = fields[1].GetUInt16();
            uint32 npcflag  = fields[2].GetUInt32();

            if(event_id >= mGameEvent.size())
            {
                TC_LOG_ERROR("gameevent","`game_event_npcflag` game event id (%u) is out of range compared to max event id in `game_event`",event_id);
                continue;
            }

            mGameEventNPCFlags[event_id].push_back(GuidNPCFlagPair(guid,npcflag));

            ++count;

        } while( result->NextRow() );
        TC_LOG_INFO("server.loading", " ");
        TC_LOG_INFO( "server.loading", ">> Loaded %u npcflags in game events", count );
    }

    mGameEventVendors.resize(mGameEvent.size());
    //                                   0      1      2     3         4         5
    result = WorldDatabase.Query("SELECT event, guid, item, maxcount, incrtime, ExtendedCost FROM game_event_npc_vendor");

    count = 0;
    if( !result )
    {
        TC_LOG_INFO("gameevent"," ");
        TC_LOG_INFO("server.loading",">> Loaded %u vendor additions in game events", count );
    }
    else
    {
        do
        {
            Field *fields = result->Fetch();

            uint16 event_id  = fields[0].GetUInt16();

            if(event_id >= mGameEventVendors.size())
            {
                TC_LOG_ERROR("gameevent","`game_event_npc_vendor` game event id (%u) is out of range compared to max event id in `game_event`",event_id);
                continue;
            }

            NPCVendorList& vendors = mGameEventVendors[event_id];
            NPCVendorEntry newEntry;
            uint32 guid = fields[1].GetUInt32();
            uint32 itemId = fields[2].GetUInt32();
            newEntry.proto = sObjectMgr->GetItemTemplate(itemId);
            if(!newEntry.proto)
            {
                TC_LOG_ERROR("gameevent","`game_event_npc_vendor` game event id (%u) has an item with non existing template (%u)",event_id,itemId);
                continue;
            }
            newEntry.maxcount = fields[3].GetUInt32();
            newEntry.incrtime = fields[4].GetUInt32();
            newEntry.ExtendedCost = fields[5].GetUInt32();
            // get the event npc flag for checking if the npc will be vendor during the event or not
            uint32 event_npc_flag = 0;
            for(auto itr : mGameEventNPCFlags[event_id])
            {
                if(itr.first == guid)
                {
                    event_npc_flag = itr.second;
                    break;
                }
            }
            // get creature entry
            newEntry.entry = 0;

            if( CreatureData const* data = sObjectMgr->GetCreatureData(guid) )
                newEntry.entry = data->id;

            // check validity with event's npcflag
            if(!sObjectMgr->IsVendorItemValid(newEntry.entry, newEntry.proto, newEntry.maxcount, newEntry.incrtime, newEntry.ExtendedCost, NULL, NULL, event_npc_flag))
                continue;
            ++count;
            vendors.push_back(newEntry);

        } while( result->NextRow() );
        TC_LOG_INFO("server.loading", " ");
        TC_LOG_INFO("server.loading",  ">> Loaded %u vendor additions in game events", count );
    }

    // set all flags to 0
    mGameEventBattlegroundHolidays.resize(mGameEvent.size(),0);
    // load game event battleground flags
    //                                   0     1
    result = WorldDatabase.Query("SELECT event, bgflag FROM game_event_battleground_holiday");

    count = 0;
    if( !result )
    {
        TC_LOG_INFO("gameevent"," ");
        TC_LOG_INFO("server.loading",">> Loaded %u battleground holidays in game events", count );
    }
    else
    {
        do
        {
            Field *fields = result->Fetch();

            uint16 event_id = fields[0].GetUInt16();

            if(event_id >= mGameEvent.size())
            {
                TC_LOG_ERROR("gameevent","`game_event_battleground_holiday` game event id (%u) is out of range compared to max event id in `game_event`",event_id);
                continue;
            }

            ++count;

            mGameEventBattlegroundHolidays[event_id] = fields[1].GetUInt32();

        } while( result->NextRow() );
        TC_LOG_INFO("server.loading", " ");
        TC_LOG_INFO( "server.loading", ">> Loaded %u battleground holidays in game events", count );
    }
}

uint32 GameEvent::GetNPCFlag(Creature * cr)
{
    uint32 mask = 0;
    uint32 guid = cr->GetDBTableGUIDLow();

    for(auto e_itr : m_ActiveEvents)
    {
        for(auto itr : mGameEventNPCFlags[e_itr])
            if(itr.first == guid)
                mask |= itr.second;
    }

    return mask;
}

uint32 GameEvent::Initialize()                              // return the next event delay in ms
{
    m_ActiveEvents.clear();
    uint32 delay = Update();
    TC_LOG_INFO("server.loading","Game Event system initialized." );
    TC_LOG_INFO("server.loading","Initialize: Next event in %u", delay);
    isSystemInit = true;
    return delay;
}

uint32 GameEvent::Update()                                  // return the next event delay in ms
{
    time_t currenttime = time(NULL);
    uint32 nextEventDelay = max_ge_check_delay;             // 1 day
    uint32 calcDelay;
    std::set<uint16> activate, deactivate;
    for (uint16 itr = 1; itr < mGameEvent.size(); ++itr)
    {
        // must do the activating first, and after that the deactivating
        // so first queue it
        //TC_LOG_ERROR("gameevent","Checking event %u",itr);
        if (CheckOneGameEvent(itr))
        {
            // if the world event is in NEXTPHASE state, and the time has passed to finish this event, then do so
            if (mGameEvent[itr].state == GAMEEVENT_WORLD_NEXTPHASE && mGameEvent[itr].nextstart <= currenttime)
            {
                // set this event to finished, null the nextstart time
                mGameEvent[itr].state = GAMEEVENT_WORLD_FINISHED;
                mGameEvent[itr].nextstart = 0;
                // save the state of this gameevent
                SaveWorldEventStateToDB(itr);
                // queue for deactivation
                if(IsActiveEvent(itr))
                    deactivate.insert(itr);
                // go to next event, this no longer needs an event update timer
                continue;
            }
            else if (mGameEvent[itr].state == GAMEEVENT_WORLD_CONDITIONS && CheckOneGameEventConditions(itr))
                // changed, save to DB the gameevent state, will be updated in next update cycle
                SaveWorldEventStateToDB(itr);

            //TC_LOG_DEBUG("gameevent","GameEvent %u is active",itr->first);
            // queue for activation
            if (!IsActiveEvent(itr))
                activate.insert(itr);
        }
        else
        {
            //TC_LOG_DEBUG("gameevent","GameEvent %u is not active",itr->first);
            if (IsActiveEvent(itr))
                deactivate.insert(itr);
            else
            {
                if (!isSystemInit)
                {
                    int16 event_nid = (-1) * (itr);
                    // spawn all negative ones for this event
                    GameEventSpawn(event_nid);
                }
            }
        }
        calcDelay = NextCheck(itr);
        if (calcDelay < nextEventDelay)
            nextEventDelay = calcDelay;
    }
    // now activate the queue
    // a now activated event can contain a spawn of a to-be-deactivated one
    // following the activate - deactivate order, deactivating the first event later will leave the spawn in (wont disappear then reappear clientside)
    for(auto itr : activate)
        // start the event
        // returns true the started event completed
        // in that case, initiate next update in 1 second
        if(StartEvent(itr))
            nextEventDelay = 0;
    for(auto itr : deactivate)
        StopEvent(itr);
    TC_LOG_DEBUG("battleground","Next game event check in %u seconds.", nextEventDelay + 1);
    return (nextEventDelay + 1) * 1000;                     // Add 1 second to be sure event has started/stopped at next call
}

void GameEvent::UnApplyEvent(uint16 event_id)
{
    TC_LOG_DEBUG("gameevent","GameEvent %u \"%s\" removed.", event_id, mGameEvent[event_id].description.c_str());
    // un-spawn positive event tagged objects
    GameEventUnspawn(event_id);
    // spawn negative event tagget objects
    int16 event_nid = (-1) * event_id;
    GameEventSpawn(event_nid);
    // restore equipment or model
    ChangeEquipOrModel(event_id, false);
    // Remove quests that are events only to non event npc
    UpdateEventQuests(event_id, false);
    // update npcflags in this event
    UpdateEventNPCFlags(event_id);
    // remove vendor items
    UpdateEventNPCVendor(event_id, false);
    // update bg holiday
    UpdateBattlegroundSettings();
}

void GameEvent::ApplyNewEvent(uint16 event_id)
{
    switch(sWorld->getConfig(CONFIG_EVENT_ANNOUNCE))
    {
        case 0:                                             // disable
            break;
        case 1:                                             // announce events
            sWorld->SendWorldText(LANG_EVENTMESSAGE, mGameEvent[event_id].description.c_str());
            break;
    }

    TC_LOG_DEBUG("gameevent","GameEvent %u \"%s\" started.", event_id, mGameEvent[event_id].description.c_str());

    // spawn positive event tagget objects
    GameEventSpawn(event_id);
    // un-spawn negative event tagged objects
    int16 event_nid = (-1) * event_id;
    GameEventUnspawn(event_nid);
    // Change equipement or model
    ChangeEquipOrModel(event_id, true);
    // Add quests that are events only to non event npc
    UpdateEventQuests(event_id, true);
    // update npcflags in this event
    UpdateEventNPCFlags(event_id);
    // add vendor items
    UpdateEventNPCVendor(event_id, true);
    // update bg holiday
    UpdateBattlegroundSettings();
}

void GameEvent::UpdateEventNPCFlags(uint16 event_id)
{
    // go through the creatures whose npcflags are changed in the event
    for(auto itr : mGameEventNPCFlags[event_id])
    {
        // get the creature data from the low guid to get the entry, to be able to find out the whole guid
        if( CreatureData const* data = sObjectMgr->GetCreatureData(itr.first) )
        {
            Creature * cr = HashMapHolder<Creature>::Find(MAKE_NEW_GUID(itr.first,data->id,HIGHGUID_UNIT));
            // if we found the creature, modify its npcflag
            if(cr)
            {
                uint32 npcflag = GetNPCFlag(cr);
                if(const CreatureTemplate * ci = cr->GetCreatureTemplate())
                    npcflag |= ci->npcflag;

                cr->SetUInt32Value(UNIT_NPC_FLAGS,npcflag);
                // reset gossip options, since the flag change might have added / removed some
               // cr->ResetGossipOptions();
                // update to world
                cr->SendUpdateObjectToAllExcept(NULL);
            }
            // if we didn't find it, then the npcflag will be updated when the creature is loaded
        }
    }
}

void GameEvent::UpdateBattlegroundSettings()
{
    uint32 mask = 0;
    for(auto itr : m_ActiveEvents)
        mask |= mGameEventBattlegroundHolidays[itr];
    sBattlegroundMgr->SetHolidayWeekends(mask);
}

void GameEvent::UpdateEventNPCVendor(uint16 event_id, bool activate)
{
    for(auto itr : mGameEventVendors[event_id])
    {
        if(activate)
            sObjectMgr->AddVendorItem(itr.entry, itr.proto, itr.maxcount, itr.incrtime, itr.ExtendedCost, false);
        else
            sObjectMgr->RemoveVendorItem(itr.entry, itr.proto, false);
    }
}

void GameEvent::SpawnCreature(uint32 guid)
{
    // Add to correct cell
    CreatureData const* data = sObjectMgr->GetCreatureData(guid);
    if (data)
    {
        sObjectMgr->AddCreatureToGrid(guid, data);

        // Spawn if necessary (loaded grids only)
        Map* map = const_cast<Map*>(sMapMgr->GetBaseMap(data->mapid));
        // We use spawn coords to spawn
        if(!map->Instanceable() && !map->IsRemovalGrid(data->posX,data->posY))
        {
            Creature* pCreature = new Creature;
            //TC_LOG_DEBUG("gameevent","Spawning creature %u",*itr);
            if (!pCreature->LoadFromDB(guid, map))
            {
                delete pCreature;
            }
            else
            {
                map->Add(pCreature);
            }
        }
    }
}

void GameEvent::SpawnGameObject(uint32 guid)
{
    // Add to correct cell
    GameObjectData const* data = sObjectMgr->GetGOData(guid);
    if (data)
    {
        sObjectMgr->AddGameobjectToGrid(guid, data);
        // Spawn if necessary (loaded grids only)
        // this base map checked as non-instanced and then only existed
        Map* map = const_cast<Map*>(sMapMgr->GetBaseMap(data->mapid));
        // We use current coords to unspawn, not spawn coords since creature can have changed grid
        if(!map->Instanceable() && !map->IsRemovalGrid(data->posX, data->posY))
        {
            GameObject* pGameobject = new GameObject;
            //TC_LOG_DEBUG("gameevent","Spawning gameobject %u", *itr);
            if (!pGameobject->LoadFromDB(guid, map))
            {
                delete pGameobject;
            }
            else
            {
                if(pGameobject->isSpawnedByDefault())
                    map->Add(pGameobject);
            }
        }
    }
}

void GameEvent::GameEventSpawn(int16 event_id)
{
    int32 internal_event_id = mGameEvent.size() + event_id - 1;

    if(internal_event_id < 0 || internal_event_id >= mGameEventCreatureGuids.size())
    {
        TC_LOG_ERROR("gameevent","GameEvent::GameEventSpawn attempt access to out of range mGameEventCreatureGuids element %i (size: %u)", internal_event_id, (uint32)mGameEventCreatureGuids.size());
        return;
    }

    for(auto itr : mGameEventCreatureGuids[internal_event_id])
        SpawnCreature(itr);

    if(internal_event_id < 0 || internal_event_id >= mGameEventGameobjectGuids.size())
    {
        TC_LOG_ERROR("gameevent", "GameEvent::GameEventSpawn attempt access to out of range mGameEventGameobjectGuids element %i (size: %u)", internal_event_id, (uint32)mGameEventGameobjectGuids.size());
        return;
    }

    for(auto itr : mGameEventGameobjectGuids[internal_event_id])
        SpawnGameObject(itr);
}

void GameEvent::UnspawnCreature(uint32 guid,uint16 event_id)
{
    // Remove the creature from grid
    if( CreatureData const* data = sObjectMgr->GetCreatureData(guid) )
    {
        sObjectMgr->RemoveCreatureFromGrid(guid, data);

        if( Creature* pCreature = sObjectAccessor->GetObjectInWorld(MAKE_NEW_GUID(guid, data->id, HIGHGUID_UNIT), (Creature*)NULL) )
        {
            pCreature->AI()->DespawnDueToGameEventEnd(event_id);
            pCreature->AddObjectToRemoveList();
        }
    }
}

void GameEvent::UnspawnGameObject(uint32 guid)
{
    // Remove the gameobject from grid
    if(GameObjectData const* data = sObjectMgr->GetGOData(guid))
    {
        sObjectMgr->RemoveGameobjectFromGrid(guid, data);

        if( GameObject* pGameobject = sObjectAccessor->GetObjectInWorld(MAKE_NEW_GUID(guid, data->id, HIGHGUID_GAMEOBJECT), (GameObject*)NULL) )
            pGameobject->AddObjectToRemoveList();
    }
}

void GameEvent::GameEventUnspawn(int16 event_id)
{
    int32 internal_event_id = mGameEvent.size() + event_id - 1;

    if(internal_event_id < 0 || internal_event_id >= mGameEventCreatureGuids.size())
    {
        TC_LOG_ERROR("gameevent", "GameEvent::GameEventUnspawn attempt access to out of range mGameEventCreatureGuids element %i (size: %u)", internal_event_id, (uint32)mGameEventCreatureGuids.size());
        return;
    }

    for(auto itr : mGameEventCreatureGuids[internal_event_id])
    {
        // check if it's needed by another event, if so, don't remove
        if( event_id > 0 && hasCreatureActiveEventExcept(itr,event_id) )
            continue;
        UnspawnCreature(itr,event_id);
    }

    if(internal_event_id < 0 || internal_event_id >= mGameEventGameobjectGuids.size())
    {
        TC_LOG_ERROR("gameevent", "GameEvent::GameEventUnspawn attempt access to out of range mGameEventGameobjectGuids element %i (size: %u)", internal_event_id, (uint32)mGameEventGameobjectGuids.size());
        return;
    }

    for(auto itr : mGameEventGameobjectGuids[internal_event_id])
    {
        // check if it's needed by another event, if so, don't remove
        if( event_id >0 && hasGameObjectActiveEventExcept(itr,event_id) )
            continue;
        UnspawnGameObject(itr);
    }
}

void GameEvent::ChangeEquipOrModel(int16 event_id, bool activate)
{
    for(auto itr : mGameEventModelEquip[event_id])
    {
        // Remove the creature from grid
        CreatureData const* data = sObjectMgr->GetCreatureData(itr.first);
        if(!data)
            continue;

        // Update if spawned
        Creature* pCreature = sObjectAccessor->GetObjectInWorld(MAKE_NEW_GUID(itr.first, data->id,HIGHGUID_UNIT), (Creature*)NULL);
        if (pCreature)
        {
            if (activate)
            {
                itr.second.equipement_id_prev = pCreature->GetCurrentEquipmentId();
                itr.second.modelid_prev = pCreature->GetDisplayId();
                pCreature->LoadEquipment(itr.second.equipment_id, true);
                if (itr.second.modelid >0 && itr.second.modelid_prev != itr.second.modelid)
                {
                    CreatureModelInfo const *minfo = sObjectMgr->GetCreatureModelInfo(itr.second.modelid);
                    if (minfo)
                    {
                        pCreature->SetDisplayId(itr.second.modelid);
                        pCreature->SetNativeDisplayId(itr.second.modelid);
                        pCreature->SetFloatValue(UNIT_FIELD_BOUNDINGRADIUS,minfo->bounding_radius);
                        pCreature->SetFloatValue(UNIT_FIELD_COMBATREACH,minfo->combat_reach );
                    }
                }
            }
            else
            {
                pCreature->LoadEquipment(itr.second.equipement_id_prev, true);
                if (itr.second.modelid_prev >0 && itr.second.modelid_prev != itr.second.modelid)
                {
                    CreatureModelInfo const *minfo = sObjectMgr->GetCreatureModelInfo(itr.second.modelid_prev);
                    if (minfo)
                    {
                        pCreature->SetDisplayId(itr.second.modelid_prev);
                        pCreature->SetNativeDisplayId(itr.second.modelid_prev);
                        pCreature->SetFloatValue(UNIT_FIELD_BOUNDINGRADIUS,minfo->bounding_radius);
                        pCreature->SetFloatValue(UNIT_FIELD_COMBATREACH,minfo->combat_reach );
                    }
                }
            }
        }
        else                                                // If not spawned
        {
            CreatureData const* data = sObjectMgr->GetCreatureData(itr.first);
            if (data && activate)
            {
                CreatureTemplate const *cinfo = sObjectMgr->GetCreatureTemplate(data->id);
                uint32 display_id = sObjectMgr->ChooseDisplayId(0,cinfo,data);
                CreatureModelInfo const *minfo = sObjectMgr->GetCreatureModelRandomGender(&display_id);
                if (data->equipmentId == 0)
                    itr.second.equipement_id_prev = cinfo->equipmentId;
                else if (data->equipmentId != -1)
                    itr.second.equipement_id_prev = data->equipmentId;
                itr.second.modelid_prev = display_id;
            }
        }
        // now last step: put in data
                                                            // just to have write access to it
        CreatureData& data2 = sObjectMgr->NewOrExistCreatureData(itr.first);
        if (activate)
        {
            data2.displayid = itr.second.modelid;
            data2.equipmentId = itr.second.equipment_id;
        }
        else
        {
            data2.displayid = itr.second.modelid_prev;
            data2.equipmentId = itr.second.equipement_id_prev;
        }
    }
}

bool GameEvent::hasCreatureQuestActiveEventExcept(uint32 quest_id, uint16 event_id)
{
    for(auto e_itr : m_ActiveEvents)
    {
        if(e_itr != event_id)
            for(auto itr : mGameEventCreatureQuests[e_itr])
                if(itr.second == quest_id)
                    return true;
    }
    return false;
}

bool GameEvent::hasGameObjectQuestActiveEventExcept(uint32 quest_id, uint16 event_id)
{
    for(auto e_itr : m_ActiveEvents)
    {
        if(e_itr != event_id)
            for(auto itr : mGameEventGameObjectQuests[e_itr])
                if(itr.second == quest_id)
                    return true;
    }
    return false;
}
bool GameEvent::hasCreatureActiveEventExcept(uint32 creature_id, uint16 event_id)
{
    for(auto e_itr : m_ActiveEvents)
    {
        if(e_itr != event_id)
        {
            int32 internal_event_id = mGameEvent.size() + e_itr - 1;
            for(auto itr : mGameEventCreatureGuids[internal_event_id])
                if(itr == creature_id)
                    return true;
        }
    }
    return false;
}
bool GameEvent::hasGameObjectActiveEventExcept(uint32 go_id, uint16 event_id)
{
    for(auto e_itr : m_ActiveEvents)
    {
        if(e_itr != event_id)
        {
            int32 internal_event_id = mGameEvent.size() + e_itr - 1;
            for(auto itr : mGameEventGameobjectGuids[internal_event_id])
                if(itr == go_id)
                    return true;
        }
    }
    return false;
}

void GameEvent::UpdateEventQuests(uint16 event_id, bool Activate)
{
    for (auto itr : mGameEventCreatureQuests[event_id])
    {
        QuestRelations &CreatureQuestMap = sObjectMgr->mCreatureQuestRelations;
        if (Activate)                                       // Add the pair(id,quest) to the multimap
            CreatureQuestMap.insert(QuestRelations::value_type(itr.first, itr.second));
        else
        {
            if(!hasCreatureQuestActiveEventExcept(itr.second,event_id))
            {
                // Remove the pair(id,quest) from the multimap
                QuestRelations::iterator qitr = CreatureQuestMap.find(itr.first);
                if (qitr == CreatureQuestMap.end())
                    continue;
                QuestRelations::iterator lastElement = CreatureQuestMap.upper_bound(itr.first);
                for ( ;qitr != lastElement;++qitr)
                {
                    if (qitr->second == itr.second)
                    {
                        CreatureQuestMap.erase(qitr);           // iterator is now no more valid
                        break;                                  // but we can exit loop since the element is found
                    }
                }
            }
        }
    }
    for (auto itr : mGameEventGameObjectQuests[event_id])
    {
        QuestRelations &GameObjectQuestMap = sObjectMgr->mGOQuestRelations;
        if (Activate)                                       // Add the pair(id,quest) to the multimap
            GameObjectQuestMap.insert(QuestRelations::value_type(itr.first, itr.second));
        else
        {
            if(!hasGameObjectQuestActiveEventExcept(itr.second,event_id))
            {
                // Remove the pair(id,quest) from the multimap
                QuestRelations::iterator qitr = GameObjectQuestMap.find(itr.first);
                if (qitr == GameObjectQuestMap.end())
                    continue;
                QuestRelations::iterator lastElement = GameObjectQuestMap.upper_bound(itr.first);
                for ( ;qitr != lastElement;++qitr)
                {
                    if (qitr->second == itr.second)
                    {
                        GameObjectQuestMap.erase(qitr);           // iterator is now no more valid
                        break;                                  // but we can exit loop since the element is found
                    }
                }
            }
        }
    }}

GameEvent::GameEvent()
{
    isSystemInit = false;
}

void GameEvent::HandleQuestComplete(uint32 quest_id)
{
    // translate the quest to event and condition
    QuestIdToEventConditionMap::iterator itr = mQuestToEventConditions.find(quest_id);
    // quest is registered
    if(itr != mQuestToEventConditions.end())
    {
        uint16 event_id = itr->second.event_id;
        uint32 condition = itr->second.condition;
        float num = itr->second.num;

        // the event is not active, so return, don't increase condition finishes
        if(!IsActiveEvent(event_id))
            return;
        // not in correct phase, return
        if(mGameEvent[event_id].state != GAMEEVENT_WORLD_CONDITIONS)
            return;
        auto citr = mGameEvent[event_id].conditions.find(condition);
        // condition is registered
        if(citr != mGameEvent[event_id].conditions.end())
        {
            // increase the done count, only if less then the req
            if(citr->second.done < citr->second.reqNum)
            {
                citr->second.done += num;
                // check max limit
                if(citr->second.done > citr->second.reqNum)
                    citr->second.done = citr->second.reqNum;
                // save the change to db
                SQLTransaction trans = CharacterDatabase.BeginTransaction();
                trans->PAppend("DELETE FROM game_event_condition_save WHERE event_id = '%u' AND condition_id = '%u'",event_id,condition);
                trans->PAppend("INSERT INTO game_event_condition_save (event_id, condition_id, done) VALUES (%u,%u,%f)",event_id,condition,citr->second.done);
                CharacterDatabase.CommitTransaction(trans);
                // check if all conditions are met, if so, update the event state
                if(CheckOneGameEventConditions(event_id))
                {
                    // changed, save to DB the gameevent state
                    SaveWorldEventStateToDB(event_id);
                    // force update events to set timer
                    sWorld->ForceGameEventUpdate();
                }
            }
        }
    }
}

bool GameEvent::CheckOneGameEventConditions(uint16 event_id)
{
    for(auto itr : mGameEvent[event_id].conditions)
        if(itr.second.done < itr.second.reqNum)
            // return false if a condition doesn't match
            return false;
    // set the phase
    mGameEvent[event_id].state = GAMEEVENT_WORLD_NEXTPHASE;
    // set the followup events' start time
    if(!mGameEvent[event_id].nextstart)
    {
        time_t currenttime = time(NULL);
        mGameEvent[event_id].nextstart = currenttime + mGameEvent[event_id].length * 60;
    }
    return true;
}

void GameEvent::SaveWorldEventStateToDB(uint16 event_id)
{
    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    trans->PAppend("DELETE FROM game_event_save WHERE event_id = '%u'",event_id);
    if(mGameEvent[event_id].nextstart)
        trans->PAppend("INSERT INTO game_event_save (event_id, state, next_start) VALUES ('%u','%u',FROM_UNIXTIME(" UI64FMTD "))",event_id,mGameEvent[event_id].state,(uint64)(mGameEvent[event_id].nextstart));
    else
        trans->PAppend("INSERT INTO game_event_save (event_id, state, next_start) VALUES ('%u','%u','0000-00-00 00:00:00')",event_id,mGameEvent[event_id].state);
    CharacterDatabase.CommitTransaction(trans);
}

void GameEvent::SendWorldStateUpdate(Player * plr, uint16 event_id)
{
    for(auto itr : mGameEvent[event_id].conditions)
    {
        if(itr.second.done_world_state)
            plr->SendUpdateWorldState(itr.second.done_world_state, (uint32)(itr.second.done));
        if(itr.second.max_world_state)
            plr->SendUpdateWorldState(itr.second.max_world_state, (uint32)(itr.second.reqNum));
    }
}

bool GameEvent::AddCreatureToEvent(uint32 guid, uint16 event_id)
{ 
    if(!guid || !event_id || event_id >= mGameEvent.size())
        return false;

    //Check if creature already linked to an event
    if (GetCreatureEvent(guid))
        return false;

    //Add to gameevent creature map
    int32 internal_event_id = mGameEvent.size() + event_id - 1;
    GuidList& crelist = mGameEventCreatureGuids[internal_event_id];
    crelist.push_back(guid);

    //Save in db
    WorldDatabase.PQuery("REPLACE INTO game_event_creature VALUES (%u,%u)",guid,event_id);

    //Spawn/Despawn IG if needed
    CreatureData const* data = sObjectMgr->GetCreatureData(guid);
    if(data)
    {
        Creature* c = sObjectAccessor->GetObjectInWorld(MAKE_NEW_GUID(guid, data->id, HIGHGUID_UNIT), (Creature*)NULL);
        if(IsActiveEvent(event_id))
        {
            if(!c || !c->IsInWorld())
                SpawnCreature(guid);
        } else {
            if(c && c->IsInWorld())
                UnspawnCreature(guid,event_id);
        }
    }

    return true; 
}
bool GameEvent::AddGameObjectToEvent(uint32 guid, uint16 event_id)
{
    if(!guid || !event_id || event_id >= mGameEvent.size())
        return false;

    //Check if creature already linked to an event
    if (GetGameObjectEvent(guid))
        return false;

    //Add to gameevent creature map
    int32 internal_event_id = mGameEvent.size() + event_id - 1;
    GuidList& crelist = mGameEventGameobjectGuids[internal_event_id];
    crelist.push_back(guid);

    //Save in db
    WorldDatabase.PQuery("REPLACE INTO game_event_gameobject VALUES (%u,%u)",guid,event_id);

    //Spawn/Despawn IG if needed
    GameObjectData const* data = sObjectMgr->GetGOData(guid);
    if(data)
    {
        GameObject* go = sObjectAccessor->GetObjectInWorld(MAKE_NEW_GUID(guid, data->id, HIGHGUID_GAMEOBJECT), (GameObject*)NULL);
        if(IsActiveEvent(event_id))
        {
            if(!go || !go->IsInWorld())
                SpawnGameObject(guid);
        } else {
            if(go && go->IsInWorld())
                UnspawnGameObject(guid);
        }
    }

    return true; 
}

bool GameEvent::RemoveCreatureFromEvent(uint32 guid)
{ 
    if(!guid)
        return false;

    //Check if creature is linked to an event
    uint16 event_id = GetCreatureEvent(guid);
    if (!event_id)
        return false;

    //Add to gameevent creature map
    int32 internal_event_id = mGameEvent.size() + event_id - 1;
    GuidList& crelist = mGameEventCreatureGuids[internal_event_id];
    crelist.remove(guid);

    //Remove from DB
    WorldDatabase.PQuery("DELETE FROM game_event_creature WHERE guid = %u;",guid);

    //Respawn IG if needed
    CreatureData const* data = sObjectMgr->GetCreatureData(guid);
    if(data)
    {
        Creature* c = sObjectAccessor->GetObjectInWorld(MAKE_NEW_GUID(guid, data->id, HIGHGUID_UNIT), (Creature*)NULL);
        if(!IsActiveEvent(event_id) && (!c || !c->IsInWorld()))
        {
            SpawnCreature(guid);
        }
    }

    return true;
}
bool GameEvent::RemoveGameObjectFromEvent(uint32 guid)
{ 
    if(!guid)
        return false;

    //Check if creature is linked to an event
    uint16 event_id = GetGameObjectEvent(guid);
    if (!event_id)
        return false;

    //Add to gameevent creature map
    int32 internal_event_id = mGameEvent.size() + event_id - 1;
    GuidList& crelist = mGameEventGameobjectGuids[internal_event_id];
    crelist.remove(guid);

    //Remove from DB
    WorldDatabase.PQuery("DELETE FROM game_event_gameobject WHERE guid = %u;",guid);

    //Respawn IG if needed
    GameObjectData const* data = sObjectMgr->GetGOData(guid);
    if(data)
    {
        GameObject* go = sObjectAccessor->GetObjectInWorld(MAKE_NEW_GUID(guid, data->id, HIGHGUID_GAMEOBJECT), (GameObject*)NULL);
        if(!IsActiveEvent(event_id) && (!go || !go->IsInWorld()))
        {
            SpawnGameObject(guid);
        }
    }

    return true;
}

bool GameEvent::CreateGameEvent(const char* name,int16& event_id)
{ 
    if(!name)
    {
        TC_LOG_ERROR("gameevent","CreateGameEvent(...) : No name given");
        return false;
    }

    event_id = mGameEvent.size();
    /*
    // crash crash crash

    // add to map
    mGameEvent.resize(mGameEvent.size() + 1);
    GameEventData& pGameEvent = mGameEvent[event_id];
    pGameEvent.start        = time_t(NULL);
    pGameEvent.end          = time_t(1609394400); // 31/12/2020
    pGameEvent.occurence    = 5184000;
    pGameEvent.length       = 2592000;
    pGameEvent.description  = name;
    pGameEvent.state        = GAMEEVENT_NORMAL;
    pGameEvent.nextstart    = 0;
    
    // save in db
    WorldDatabase.PQuery("INSERT INTO game_event (entry,start_time,end_time,occurence,lenght,description,world_event) VALUES (%u,%u,%u,%u,%u,%s,%u);",
        event_id,
        (uint64)pGameEvent.start,
        (uint64)pGameEvent.end,
        (uint32)pGameEvent.occurence,
        (uint32)pGameEvent.length,
        pGameEvent.description,
        (uint8)pGameEvent.state);
        */
       
    return false; 
}

int16 GameEvent::GetCreatureEvent(uint32 guid)
{ 
    if(!guid)
        return 0;

    for(int i = -((int)mGameEvent.size())+1; i < (int)mGameEvent.size();i++)
    {
        int32 internal_event_id = mGameEvent.size() + i - 1;
        for(auto itr : mGameEventCreatureGuids[internal_event_id])
        {
            if(itr == guid)
                return i;
        }
    }
    return 0;
}

int16 GameEvent::GetGameObjectEvent(uint32 guid)
{ 
    if(!guid)
        return 0;

    for(int i = -((int)mGameEvent.size())+1; i < (int)mGameEvent.size();i++)
    {
        int32 internal_event_id = mGameEvent.size() + i - 1;
        for(auto itr : mGameEventGameobjectGuids[internal_event_id])
        {
            if(itr == guid)
                return i;
        }
    }
    return 0;
}
