
#include "GameEventMgr.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Language.h"
#include "Log.h"
#include "MapManager.h"
#include "GossipDef.h"
#include "Player.h"
#include "BattleGroundMgr.h"
#include "GridNotifiers.h"
#include "Map.h"
#include "Transport.h"
#include "GameTime.h"

bool GameEventMgr::CheckOneGameEvent(uint16 entry) const
{
    time_t currenttime = GameTime::GetGameTime();
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

uint32 GameEventMgr::NextCheck(uint16 entry) const
{
    time_t currenttime = GameTime::GetGameTime();

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

bool GameEventMgr::StartEvent( uint16 event_id, bool overwrite )
{
    // Temp hack until game_event dependencies is implemented
    if ((event_id >= 50 && event_id <= 56) && !IsActiveEvent(GAME_EVENT_HALLOWS_END))
        return false;

    if(mGameEvent[event_id].state == GAMEEVENT_NORMAL)
    {
        AddActiveEvent(event_id);
        ApplyNewEvent(event_id);
        if(overwrite)
        {
            mGameEvent[event_id].start = GameTime::GetGameTime();
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

void GameEventMgr::StopEvent( uint16 event_id, bool overwrite )
{
    bool serverwide_evt = mGameEvent[event_id].state != GAMEEVENT_NORMAL;

    RemoveActiveEvent(event_id);
    UnApplyEvent(event_id);

    if(overwrite && !serverwide_evt)
    {
        mGameEvent[event_id].start = GameTime::GetGameTime() - mGameEvent[event_id].length * MINUTE;
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

void GameEventMgr::LoadVendors()
{
    mGameEventVendors.clear();
    mGameEventVendors.resize(mGameEvent.size());

    //                                                 0      1      2     3         4         5
    QueryResult result = WorldDatabase.Query("SELECT event, guid, item, maxcount, incrtime, ExtendedCost FROM game_event_npc_vendor");

    uint32 count = 0;
    if (!result)
    {
        TC_LOG_INFO("server.loading", ">> Loaded %u vendor additions in game events", count);
    }
    else
    {
        do
        {
            Field *fields = result->Fetch();

            uint16 event_id = fields[0].GetUInt32();

            if (event_id >= mGameEventVendors.size())
            {
                TC_LOG_ERROR("sql.sql", "`game_event_npc_vendor` game event id (%u) is out of range compared to max event id in `game_event`", event_id);
                continue;
            }

            NPCVendorList& vendors = mGameEventVendors[event_id];
            NPCVendorEntry newEntry;
            ObjectGuid::LowType guid = fields[1].GetUInt32();
            uint32 itemId = fields[2].GetUInt32();
            newEntry.proto = sObjectMgr->GetItemTemplate(itemId);
            if (!newEntry.proto)
            {
                TC_LOG_ERROR("sql.sql", "`game_event_npc_vendor` game event id (%u) has an item with non existing template (%u)", event_id, itemId);
                continue;
            }
            newEntry.maxcount = fields[3].GetUInt32();
            newEntry.incrtime = fields[4].GetUInt32();
            newEntry.ExtendedCost = fields[5].GetUInt32();
            // get the event npc flag for checking if the npc will be vendor during the event or not
            uint32 event_npc_flag = 0;
            for (auto itr : mGameEventNPCFlags[event_id])
            {
                if (itr.first == guid)
                {
                    event_npc_flag = itr.second;
                    break;
                }
            }
            // get creature entry
            newEntry.entry = 0;

            if (CreatureData const* data = sObjectMgr->GetCreatureData(guid))
                newEntry.entry = data->GetFirstSpawnEntry();

            // check validity with event's npcflag
            if (!sObjectMgr->IsVendorItemValid(newEntry.entry, newEntry.proto, newEntry.maxcount, newEntry.incrtime, newEntry.ExtendedCost, nullptr, nullptr, event_npc_flag))
                continue;
            ++count;
            vendors.push_back(newEntry);

        } while (result->NextRow());
        TC_LOG_INFO("server.loading", ">> Loaded %u vendor additions in game events", count);
    }

}

void GameEventMgr::LoadTrainers()
{
    mGameEventTrainers.clear();
    //                                                 0             2                   4                       6
    QueryResult result = WorldDatabase.Query("SELECT event, entry, spell, spellcost, reqskill, reqskillvalue, reqlevel FROM game_event_npc_trainer");
    uint32 count = 0;
    if (!result)
    {
        TC_LOG_INFO("server.loading", ">> Loaded %u trainer spells in game events", count);
    }
    else
    {
        do
        {
            Field *fields = result->Fetch();

            uint16 event_id = fields[0].GetUInt32();

            if (event_id >= mGameEvent.size())
            {
                TC_LOG_ERROR("sql.sql", "`game_event_npc_trainer` game event id (%u) is out of range compared to max event id in `game_event`", event_id);
                continue;
            }

            uint32 creatureId = fields[1].GetUInt32();
            uint32 spellId = fields[2].GetUInt32();

            CreatureTemplate const* cInfo = sObjectMgr->GetCreatureTemplate(creatureId);
            if (!cInfo)
            {
                TC_LOG_ERROR("sql.sql", "Table `game_event_npc_trainer` have entry for not existed creature template (Entry: %u), ignore", creatureId);
                continue;
            }

            if (!(cInfo->npcflag & UNIT_NPC_FLAG_TRAINER))
            {
                TC_LOG_ERROR("sql.sql", "Table `game_event_npc_trainer` have data for not creature template (Entry: %u) without trainer flag, ignore", creatureId);
                continue;
            }

            SpellInfo const *spellinfo = sSpellMgr->GetSpellInfo(spellId);
            if (!spellinfo)
            {
                TC_LOG_ERROR("sql.sql", "Table `npc_trainer` for Trainer (Entry: %u ) has non existing spell %u, ignore", creatureId, spellId);
                continue;
            }

            if (!SpellMgr::IsSpellValid(spellinfo))
            {
                TC_LOG_ERROR("sql.sql", "Table `npc_trainer` for Trainer (Entry: %u) has broken learning spell %u, ignore", creatureId, spellId);
                continue;
            }

            TrainerSpell trainerSpell;
            trainerSpell.spell = spellId;
            trainerSpell.spellcost = fields[3].GetUInt32();
            trainerSpell.reqskill = fields[4].GetUInt16();
            trainerSpell.reqskillvalue = fields[5].GetUInt16();
            trainerSpell.reqlevel = fields[6].GetUInt8();

            if (!trainerSpell.reqlevel)
                trainerSpell.reqlevel = spellinfo->SpellLevel;

            auto itr = mGameEventTrainers.find(event_id);
            if (itr == mGameEventTrainers.end())
            {
                std::multimap<uint32, TrainerSpell> map;
                map.insert(std::make_pair(creatureId, trainerSpell));
                mGameEventTrainers.insert(std::make_pair(event_id, std::move(map)));
            }
            else {
                auto& map = itr->second;
                map.insert(std::make_pair(creatureId, trainerSpell));
            }

            ++count;
        } while (result->NextRow());
        TC_LOG_INFO("server.loading", ">> Loaded %u trainer spells in game events", count);
    }

}

void GameEventMgr::LoadFromDB()
{
    //reload case
    
    mGameEvent.clear();
    mGameEventCreatureQuests.clear();
    mGameEventGameObjectQuests.clear();
    mGameEventModelEquip.clear();
    mGameEventCreatureGuids.clear();
    mGameEventGameobjectGuids.clear();
    mGameEventBattlegroundHolidays.clear();
    
    {
        QueryResult result = WorldDatabase.Query("SELECT MAX(entry) FROM game_event");
        if( !result )
        {
            TC_LOG_INFO("server.loading",">> Table game_event is empty.");
            return;
        }

        Field *fields = result->Fetch();

        uint32 max_event_id = fields[0].GetUInt32();

        mGameEvent.resize(max_event_id+1);
    }

    QueryResult result = WorldDatabase.Query("SELECT entry,UNIX_TIMESTAMP(start_time),UNIX_TIMESTAMP(end_time),occurence,length,description,world_event FROM game_event");
    if( !result )
    {
        mGameEvent.clear();
        TC_LOG_INFO("server.loading",">> Table game_event is empty:");
        return;
    }

    uint32 count = 0;

    do
    {
        ++count;
        Field *fields = result->Fetch();

        uint16 event_id = fields[0].GetUInt32();
        if(event_id==0)
        {
            TC_LOG_ERROR("sql.sql","`game_event` game event id (%i) is reserved and can't be used.",event_id);
            continue;
        }

        GameEventData& pGameEvent = mGameEvent[event_id];
        uint64 starttime        = fields[1].GetUInt64();
        pGameEvent.start        = time_t(starttime);
        uint64 endtime          = fields[2].GetUInt64();
        pGameEvent.end          = time_t(endtime);
        pGameEvent.occurence    = fields[3].GetUInt64();
        pGameEvent.length       = fields[4].GetUInt64();
        pGameEvent.description  = fields[5].GetString();
        pGameEvent.state        = (GameEventState)(fields[6].GetUInt8());
        pGameEvent.nextstart    = 0;

        if(pGameEvent.length==0 && pGameEvent.state == GAMEEVENT_NORMAL)                            // length>0 is validity check
        {
            TC_LOG_ERROR("sql.sql","`game_event` game event id (%i) isn't a world event and has length = 0, thus it can't be used.",event_id);
            continue;
        }

    /*    if( pGameEvent.occurence == 0 )
             pGameEvent.occurence = -1; //set a default value instead */

    } while( result->NextRow() );

    TC_LOG_INFO( "server.loading",">> Loaded %u game events", count );

    // load game event saves
    //                                       0         1      2
    result = CharacterDatabase.Query("SELECT event_id, state, UNIX_TIMESTAMP(next_start) FROM game_event_save");

    count = 0;
    if( !result )
    {
        TC_LOG_INFO("server.loading", ">> Loaded %u game event saves in game events", count );
    }
    else
    {
        do
        {
            Field *fields = result->Fetch();

            uint32 event_id = fields[0].GetUInt32();

            if(event_id >= mGameEvent.size())
            {
                TC_LOG_ERROR("sql.sql","`game_event_save` game event id (%i) is out of range compared to max event id in `game_event`",event_id);
                continue;
            }

            if(mGameEvent[event_id].state != GAMEEVENT_NORMAL)
            {
                mGameEvent[event_id].state = (GameEventState)(fields[1].GetUInt8());
                mGameEvent[event_id].nextstart    = time_t(fields[2].GetUInt64());
            }
            else
            {
                TC_LOG_ERROR("sql.sql","game_event_save includes event save for non-worldevent id %u",event_id);
                continue;
            }

            ++count;

        } while( result->NextRow() );
        TC_LOG_INFO("server.loading",">> Loaded %u game event saves in game events", count );
    }

    // load game event links (prerequisites)
    result = WorldDatabase.Query("SELECT event_id, prerequisite_event FROM game_event_prerequisite");
    if( !result )
    {
        TC_LOG_INFO("server.loading", ">> Loaded %u game event prerequisites in game events", count );
    }
    else
    {
        do
        {
            Field *fields = result->Fetch();

            uint32 event_id = fields[0].GetUInt32();

            if(event_id >= mGameEvent.size())
            {
                TC_LOG_ERROR("sql.sql","`game_event_prerequisite` game event id (%i) is out of range compared to max event id in `game_event`",event_id);
                continue;
            }


            if(mGameEvent[event_id].state != GAMEEVENT_NORMAL)
            {
                uint32 prerequisite_event = fields[1].GetUInt32();
                if(prerequisite_event >= mGameEvent.size())
                {
                    TC_LOG_ERROR("sql.sql","`game_event_prerequisite` game event prerequisite id (%i) is out of range compared to max event id in `game_event`",prerequisite_event);
                    continue;
                }
                mGameEvent[event_id].prerequisite_events.insert(prerequisite_event);
            }
            else
            {
                TC_LOG_ERROR("sql.sql","game_event_prerequisiste includes event entry for non-worldevent id %u",event_id);
                continue;
            }

            ++count;

        } while( result->NextRow() );
        TC_LOG_INFO("server.loading", ">> Loaded %u game event prerequisites in game events", count );
    }

    mGameEventCreatureGuids.resize(mGameEvent.size()*2-1);
    //                                   1                 2
    result = WorldDatabase.Query("SELECT creature.spawnId, game_event_creature.event "
        "FROM creature JOIN game_event_creature ON creature.spawnID = game_event_creature.guid");

    count = 0;
    if( !result )
    {
        TC_LOG_INFO("server.loading",">> Loaded %u creatures in game events", count );
    }
    else
    {
        do
        {
            Field *fields = result->Fetch();

            ObjectGuid::LowType guid    = fields[0].GetUInt32();
            int32 event_id = fields[1].GetInt32();

            int32 internal_event_id = mGameEvent.size() + event_id - 1;

            if(internal_event_id < 0 || internal_event_id >= mGameEventCreatureGuids.size())
            {
                TC_LOG_ERROR("sql.sql","`game_event_creature` game event id (%i) is out of range compared to max event id in `game_event`", event_id);
                continue;
            }

            ++count;
            auto& crelist = mGameEventCreatureGuids[internal_event_id];
            crelist.push_back(guid);

        } while( result->NextRow() );
        TC_LOG_INFO( "server.loading",">> Loaded %u creatures in game events", count );
    }

    mGameEventGameobjectGuids.resize(mGameEvent.size()*2-1);
    //                                   1                2
    result = WorldDatabase.Query("SELECT gameobject.guid, game_event_gameobject.event "
        "FROM gameobject JOIN game_event_gameobject ON gameobject.guid=game_event_gameobject.guid");

    count = 0;
    if( !result )
    {
        TC_LOG_INFO("server.loading"," ");
        TC_LOG_INFO("server.loading",">> Loaded %u gameobjects in game events", count );
    }
    else
    {
        do
        {
            Field *fields = result->Fetch();

            ObjectGuid::LowType guid = fields[0].GetUInt32();
            int16 event_id = fields[1].GetInt16();

            int32 internal_event_id = mGameEvent.size() + event_id - 1;

            if(internal_event_id < 0 || internal_event_id >= mGameEventGameobjectGuids.size())
            {
                TC_LOG_ERROR("sql.sql","`game_event_gameobject` game event id (%i) is out of range compared to max event id in `game_event`",event_id);
                continue;
            }

            ++count;
            auto& golist = mGameEventGameobjectGuids[internal_event_id];
            golist.push_back(guid);

        } while( result->NextRow() );
        TC_LOG_INFO("server.loading", ">> Loaded %u gameobjects in game events", count );
    }

    mGameEventModelEquip.resize(mGameEvent.size());
    //                                   0          1           2             3                  4
    result = WorldDatabase.Query("SELECT c.spawnID, geme.event, geme.modelid, geme.equipment_id, ce.entry "
        "FROM creature c "
        "JOIN creature_entry ce ON ce.spawnID = c.spawnID "
        "JOIN game_event_model_equip geme ON c.spawnID = geme.guid");

    count = 0;
    if( !result )
    {
        TC_LOG_INFO("server.loading"," ");
        TC_LOG_INFO("server.loading",">> Loaded %u model/equipment changes in game events", count );
    }
    else
    {
        do
        {
            Field *fields = result->Fetch();

            ObjectGuid::LowType guid     = fields[0].GetUInt32();
            uint16 event_id = fields[1].GetUInt16();
            uint32 creatureID = fields[4].GetUInt32();

            if(event_id >= mGameEventModelEquip.size())
            {
                TC_LOG_ERROR("sql.sql","`game_event_model_equip` game event id (%u) is out of range compared to max event id in `game_event`",event_id);
                continue;
            }

            ++count;
            ModelEquipList& equiplist = mGameEventModelEquip[event_id];
            ModelEquip newModelEquipSet;
            newModelEquipSet.modelid = fields[2].GetUInt32();
            newModelEquipSet.equipment_id = fields[3].GetUInt8();
            newModelEquipSet.equipement_id_prev = 0;
            newModelEquipSet.modelid_prev = 0;

            if(newModelEquipSet.equipment_id > 0)
            {
                int8 equipId = newModelEquipSet.equipment_id;
                if(!sObjectMgr->GetEquipmentInfo(creatureID, equipId))
                {
                    TC_LOG_ERROR("gameevent","Table `game_event_model_equip` have creature (Guid: %u) with equipment_id %u not found in table `creature_equip_template`, set to no equipment.", guid, newModelEquipSet.equipment_id);
                    continue;
                }
            }

            equiplist.push_back(std::pair<uint32, ModelEquip>(guid, newModelEquipSet));

        } while( result->NextRow() );
        TC_LOG_INFO( "server.loading",">> Loaded %u model/equipment changes in game events", count );
    }

    mGameEventCreatureQuests.resize(mGameEvent.size());
    //                                   0   1      2
    result = WorldDatabase.Query("SELECT id, quest, event FROM game_event_creature_quest");

    count = 0;
    if( !result )
    {
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
                TC_LOG_ERROR("sql.sql","`game_event_creature_quest` game event id (%u) is out of range compared to max event id in `game_event`",event_id);
                continue;
            }

            ++count;
            QuestRelList& questlist = mGameEventCreatureQuests[event_id];
            questlist.push_back(QuestRelation(id, quest));

        } while( result->NextRow() );
        TC_LOG_INFO("server.loading", ">> Loaded %u quests additions in game events", count );
    }

    mGameEventGameObjectQuests.resize(mGameEvent.size());
    //                                   0   1      2
    result = WorldDatabase.Query("SELECT id, quest, event FROM game_event_gameobject_quest");

    count = 0;
    if( !result )
    {
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
                TC_LOG_ERROR("sql.sql","`game_event_gameobject_quest` game event id (%u) is out of range compared to max event id in `game_event`",event_id);
                continue;
            }

            ++count;
            QuestRelList& questlist = mGameEventGameObjectQuests[event_id];
            questlist.push_back(QuestRelation(id, quest));

        } while( result->NextRow() );
        TC_LOG_INFO("server.loading",">> Loaded %u quests additions in game events", count );
    }

    // Load quest to (event,condition) mapping
    //                                   0      1         2             3
    result = WorldDatabase.Query("SELECT quest, event_id, condition_id, num FROM game_event_quest_condition");

    count = 0;
    if( !result )
    {
        TC_LOG_INFO("server.loading",">> Loaded %u quest event conditions in game events", count );
    }
    else
    {
        do
        {
            Field *fields = result->Fetch();

            uint32 quest     = fields[0].GetUInt32();
            uint32 event_id  = fields[1].GetUInt32();
            uint32 condition = fields[2].GetUInt32();
            float num       = fields[3].GetFloat();

            if(event_id >= mGameEvent.size())
            {
                TC_LOG_ERROR("sql.sql","`game_event_quest_condition` game event id (%u) is out of range compared to max event id in `game_event`",event_id);
                continue;
            }

            ++count;
            mQuestToEventConditions[quest].event_id = event_id;
            mQuestToEventConditions[quest].condition = condition;
            mQuestToEventConditions[quest].num = num;

        } while( result->NextRow() );
        TC_LOG_INFO("server.loading", ">> Loaded %u quest event conditions in game events", count );
    }

    // load conditions of the events
    //                                   0         1             2        3                      4
    result = WorldDatabase.Query("SELECT event_id, condition_id, req_num, max_world_state_field, done_world_state_field FROM game_event_condition");

    count = 0;
    if( !result )
    {
        TC_LOG_INFO("server.loading",">> Loaded %u conditions in game events", count );
    }
    else
    {
        do
        {
            Field *fields = result->Fetch();

            uint16 event_id  = fields[0].GetUInt32();
            uint32 condition = fields[1].GetUInt32();

            if(event_id >= mGameEvent.size())
            {
                TC_LOG_ERROR("sql.sql","`game_event_condition` game event id (%u) is out of range compared to max event id in `game_event`",event_id);
                continue;
            }

            mGameEvent[event_id].conditions[condition].reqNum = fields[2].GetFloat();
            mGameEvent[event_id].conditions[condition].done = 0;
            mGameEvent[event_id].conditions[condition].max_world_state = fields[3].GetUInt16();
            mGameEvent[event_id].conditions[condition].done_world_state = fields[4].GetUInt16();

            ++count;

        } while( result->NextRow() );
        TC_LOG_INFO("server.loading", ">> Loaded %u conditions in game events", count );
    }

    // load condition saves
    //                                       0         1             2
    result = CharacterDatabase.Query("SELECT event_id, condition_id, done FROM game_event_condition_save");

    count = 0;
    if( !result )
    {
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
                TC_LOG_ERROR("sql.sql","`game_event_condition_save` game event id (%u) is out of range compared to max event id in `game_event`",event_id);
                continue;
            }

            auto itr = mGameEvent[event_id].conditions.find(condition);
            if(itr != mGameEvent[event_id].conditions.end())
            {
                itr->second.done = fields[2].GetFloat();
            }
            else
            {
                TC_LOG_ERROR("sql.sql","game_event_condition_save contains not present condition evt id %u cond id %u",event_id, condition);
                continue;
            }

            ++count;

        } while( result->NextRow() );
        TC_LOG_INFO("server.loading", ">> Loaded %u condition saves in game events", count );
    }

    mGameEventNPCFlags.resize(mGameEvent.size());
    // load game event npcflag
    //                                   0         1        2
    result = WorldDatabase.Query("SELECT guid, event_id, npcflag FROM game_event_npcflag");

    count = 0;
    if( !result )
    {
        TC_LOG_INFO("server.loading",">> Loaded %u npcflags in game events", count );
    }
    else
    {
        do
        {
            Field *fields = result->Fetch();

            ObjectGuid::LowType guid     = fields[0].GetUInt32();
            uint16 event_id = fields[1].GetUInt32();
            uint32 npcflag  = fields[2].GetUInt32();

            if(event_id >= mGameEvent.size())
            {
                TC_LOG_ERROR("sql.sql","`game_event_npcflag` game event id (%u) is out of range compared to max event id in `game_event`",event_id);
                continue;
            }

            mGameEventNPCFlags[event_id].push_back(GuidNPCFlagPair(guid,npcflag));

            ++count;

        } while( result->NextRow() );
        TC_LOG_INFO("server.loading", ">> Loaded %u npcflags in game events", count );
    }

    LoadVendors();
    LoadTrainers();

    // set all flags to 0
    mGameEventBattlegroundHolidays.resize(mGameEvent.size(),0);
    // load game event battleground flags
    //                                   0     1
    result = WorldDatabase.Query("SELECT event, bgflag FROM game_event_battleground_holiday");

    count = 0;
    if( !result )
    {
        TC_LOG_INFO("server.loading",">> Loaded %u battleground holidays in game events", count );
    }
    else
    {
        do
        {
            Field *fields = result->Fetch();

            uint16 event_id = fields[0].GetUInt32();

            if(event_id >= mGameEvent.size())
            {
                TC_LOG_ERROR("sql.sql","`game_event_battleground_holiday` game event id (%u) is out of range compared to max event id in `game_event`",event_id);
                continue;
            }

            ++count;

            mGameEventBattlegroundHolidays[event_id] = fields[1].GetUInt32();

        } while( result->NextRow() );
        TC_LOG_INFO("server.loading", ">> Loaded %u battleground holidays in game events", count );
    }
}

uint32 GameEventMgr::GetNPCFlag(Creature * cr)
{
    uint32 mask = 0;
    ObjectGuid::LowType guid = cr->GetSpawnId();

    for(auto e_itr : m_ActiveEvents)
    {
        for(auto itr : mGameEventNPCFlags[e_itr])
            if(itr.first == guid)
                mask |= itr.second;
    }

    return mask;
}

uint32 GameEventMgr::Initialize()                              // return the next event delay in ms
{
    m_ActiveEvents.clear();
    uint32 delay = Update();
    TC_LOG_INFO("server.loading","Game Event system initialized." );
    TC_LOG_INFO("server.loading","Initialize: Next event in %u", delay);
    isSystemInit = true;
    return delay;
}

uint32 GameEventMgr::Update()                                  // return the next event delay in ms
{
    time_t currenttime = GameTime::GetGameTime();
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

            //TC_LOG_DEBUG("gameevent","GameEventMgr %u is active",itr->first);
            // queue for activation
            if (!IsActiveEvent(itr))
                activate.insert(itr);
        }
        else
        {
            //TC_LOG_DEBUG("gameevent","GameEventMgr %u is not active",itr->first);
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

void GameEventMgr::UnApplyEvent(uint16 event_id)
{
    TC_LOG_DEBUG("gameevent","GameEventMgr %u \"%s\" removed.", event_id, mGameEvent[event_id].description.c_str());
    WarnAIScripts(event_id, false);

    // un-spawn positive event tagged objects
    GameEventUnspawn(event_id);
    // spawn negative event tagget objects
    int32 event_nid = (-1) * event_id;
    GameEventSpawn(event_nid);
    // restore equipment or model
    ChangeEquipOrModel(event_id, false);
    // Remove quests that are events only to non event npc
    UpdateEventQuests(event_id, false);
    // update npcflags in this event
    UpdateEventNPCFlags(event_id);
    // remove vendor items
    UpdateEventNPCVendor(event_id, false);
    // remove trainer spells
    UpdateEventNPCTrainer(event_id, false);
    // update bg holiday
    UpdateBattlegroundSettings();
}

void GameEventMgr::ApplyNewEvent(uint16 event_id)
{
    switch(sWorld->getConfig(CONFIG_EVENT_ANNOUNCE))
    {
        case 0:                                             // disable
            break;
        case 1:                                             // announce events
            sWorld->SendWorldText(LANG_EVENTMESSAGE, mGameEvent[event_id].description.c_str());
            break;
    }

    TC_LOG_DEBUG("gameevent","GameEventMgr %u \"%s\" started.", event_id, mGameEvent[event_id].description.c_str());
    WarnAIScripts(event_id, true);

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
    // add trainers spells
    UpdateEventNPCTrainer(event_id, true);
    // update bg holiday
    UpdateBattlegroundSettings();
}

void GameEventMgr::UpdateEventNPCFlags(uint16 event_id)
{
    std::unordered_map<uint32, std::unordered_set<uint32>> creaturesByMap;

    // go through the creatures whose npcflags are changed in the event
    for (NPCFlagList::iterator itr = mGameEventNPCFlags[event_id].begin(); itr != mGameEventNPCFlags[event_id].end(); ++itr)
        // get the creature data from the low guid to get the entry, to be able to find out the whole guid
        if (CreatureData const* data = sObjectMgr->GetCreatureData(itr->first))
            creaturesByMap[data->spawnPoint.GetMapId()].insert(itr->first);

    for (auto const& p : creaturesByMap)
    {
        sMapMgr->DoForAllMapsWithMapId(p.first, [this, &p](Map* map)
        {
            for (auto& spawnId : p.second)
            {
                auto creatureBounds = map->GetCreatureBySpawnIdStore().equal_range(spawnId);
                for (auto itr = creatureBounds.first; itr != creatureBounds.second; ++itr)
                {
                    Creature* creature = itr->second;
                    uint32 npcflag = GetNPCFlag(creature);
                    if (CreatureTemplate const* creatureTemplate = creature->GetCreatureTemplate())
                        npcflag |= creatureTemplate->npcflag;

                    creature->SetUInt32Value(UNIT_NPC_FLAGS, npcflag);
                    // reset gossip options, since the flag change might have added / removed some
                    //cr->ResetGossipOptions();
                }
            }
        });
    }
}

void GameEventMgr::UpdateBattlegroundSettings()
{
    uint32 mask = 0;
    for(auto itr : m_ActiveEvents)
        mask |= mGameEventBattlegroundHolidays[itr];
    sBattlegroundMgr->SetHolidayWeekends(mask);
}

void GameEventMgr::UpdateEventNPCTrainer(uint16 event_id, bool activate)
{
    auto itr_pair = mGameEventTrainers.equal_range(event_id);
    auto itr_lower = itr_pair.first;
    auto itr_upper = itr_pair.second;
    for (auto itr = itr_lower; itr != itr_upper; ++itr)
    {
        auto spellsByTrainerMap = itr->second;
        for (auto itr2 : spellsByTrainerMap)
        {
            uint32 creatureId = itr2.first;
            TrainerSpell spell = itr2.second;
            if (activate)
                sObjectMgr->AddTrainerSpell(creatureId, spell);
            else
                sObjectMgr->RemoveTrainerSpell(creatureId, spell.spell);
        }
    }
}

void GameEventMgr::UpdateEventNPCVendor(uint16 event_id, bool activate)
{
    for(auto itr : mGameEventVendors[event_id])
    {
        if(activate)
            sObjectMgr->AddVendorItem(itr.entry, itr.proto, itr.maxcount, itr.incrtime, itr.ExtendedCost, false);
        else
            sObjectMgr->RemoveVendorItem(itr.entry, itr.proto, false);
    }
}

void GameEventMgr::SpawnCreature(uint32 spawnId)
{
    // Add to correct cell
    CreatureData const* data = sObjectMgr->GetCreatureData(spawnId);
    if (data && sObjectMgr->AddCreatureToGrid(spawnId, data))
    {
        // Spawn if necessary (loaded grids only)
        Map* map = const_cast<Map*>(sMapMgr->CreateBaseMap(data->spawnPoint.GetMapId()));
        // We use spawn coords to spawn
        if(!map->Instanceable() && map->IsGridLoaded(data->spawnPoint))
        {
            auto  pCreature = new Creature;
            //TC_LOG_DEBUG("gameevent","Spawning creature %u",*itr);
            if (!pCreature->LoadFromDB(spawnId, map, true, false))
            {
                sObjectMgr->RemoveCreatureFromGrid(spawnId, data);
                delete pCreature;
            }
        }
    }
}

void GameEventMgr::SpawnGameObject(uint32 spawnId)
{
    // Add to correct cell
    GameObjectData const* data = sObjectMgr->GetGameObjectData(spawnId);
    if (data && sObjectMgr->AddGameobjectToGrid(spawnId, data))
    {
        // Spawn if necessary (loaded grids only)
        // this base map checked as non-instanced and then only existed
        Map* map = const_cast<Map*>(sMapMgr->CreateBaseMap(data->spawnPoint.GetMapId()));
        // We use current coords to unspawn, not spawn coords since creature can have changed grid
        if(!map->Instanceable() && map->IsGridLoaded(data->spawnPoint))
        {
            GameObject* pGameobject = sObjectMgr->CreateGameObject(data->id);
            //TC_LOG_DEBUG("gameevent","Spawning gameobject %u", *itr);
            if (!pGameobject->LoadFromDB(spawnId, map, true, false))
            {
                sObjectMgr->RemoveGameobjectFromGrid(spawnId, data);
                delete pGameobject;
            }
        }
    }
}

void GameEventMgr::GameEventSpawn(int16 event_id)
{
    int32 internal_event_id = mGameEvent.size() + event_id - 1;

    if(internal_event_id < 0 || internal_event_id >= mGameEventCreatureGuids.size())
    {
        TC_LOG_ERROR("gameevent","GameEventMgr::GameEventSpawn attempt access to out of range mGameEventCreatureGuids element %i (size: %u)", internal_event_id, (uint32)mGameEventCreatureGuids.size());
        return;
    }

    for(auto itr : mGameEventCreatureGuids[internal_event_id])
        SpawnCreature(itr);

    if(internal_event_id < 0 || internal_event_id >= mGameEventGameobjectGuids.size())
    {
        TC_LOG_ERROR("gameevent", "GameEventMgr::GameEventSpawn attempt access to out of range mGameEventGameobjectGuids element %i (size: %u)", internal_event_id, (uint32)mGameEventGameobjectGuids.size());
        return;
    }

    for(auto itr : mGameEventGameobjectGuids[internal_event_id])
        SpawnGameObject(itr);
}

void GameEventMgr::DespawnCreature(uint32 spawnId, uint16 event_id)
{
    // Remove the creature from grid
    if (CreatureData const* data = sObjectMgr->GetCreatureData(spawnId))
    {
        sObjectMgr->RemoveCreatureFromGrid(spawnId, data);

        sMapMgr->DoForAllMapsWithMapId(data->spawnPoint.GetMapId(), [&](Map* map)
        {
            auto creatureBounds = map->GetCreatureBySpawnIdStore().equal_range(spawnId);
            for (auto itr2 = creatureBounds.first; itr2 != creatureBounds.second;)
            {
                Creature* creature = itr2->second;
                ++itr2;
                creature->AI()->DespawnDueToGameEventEnd(event_id);
                creature->AddObjectToRemoveList();
            }
        });
    }
}

void GameEventMgr::DespawnGameObject(uint32 spawnId)
{
    // Remove the gameobject from grid
    if (GameObjectData const* data = sObjectMgr->GetGameObjectData(spawnId))
    {
        sObjectMgr->RemoveGameobjectFromGrid(spawnId, data);

        sMapMgr->DoForAllMapsWithMapId(data->spawnPoint.GetMapId(), [&spawnId](Map* map)
        {
            auto gameobjectBounds = map->GetGameObjectBySpawnIdStore().equal_range(spawnId);
            for (auto itr2 = gameobjectBounds.first; itr2 != gameobjectBounds.second;)
            {
                GameObject* go = itr2->second;
                ++itr2;
                go->AddObjectToRemoveList();
            }
        });
    }
}

void GameEventMgr::GameEventUnspawn(int16 event_id)
{
    int32 internal_event_id = mGameEvent.size() + event_id - 1;

    if(internal_event_id < 0 || internal_event_id >= mGameEventCreatureGuids.size())
    {
        TC_LOG_ERROR("gameevent", "GameEventMgr::GameEventUnspawn attempt access to out of range mGameEventCreatureGuids element %i (size: %u)", internal_event_id, (uint32)mGameEventCreatureGuids.size());
        return;
    }

    for(auto itr : mGameEventCreatureGuids[internal_event_id])
    {
        // check if it's needed by another event, if so, don't remove
        if( event_id > 0 && hasCreatureActiveEventExcept(itr,event_id) )
            continue;
        DespawnCreature(itr,event_id);
    }

    if(internal_event_id < 0 || internal_event_id >= mGameEventGameobjectGuids.size())
    {
        TC_LOG_ERROR("gameevent", "GameEventMgr::GameEventUnspawn attempt access to out of range mGameEventGameobjectGuids element %i (size: %u)", internal_event_id, (uint32)mGameEventGameobjectGuids.size());
        return;
    }

    for(auto itr : mGameEventGameobjectGuids[internal_event_id])
    {
        // check if it's needed by another event, if so, don't remove
        if( event_id >0 && hasGameObjectActiveEventExcept(itr,event_id) )
            continue;
        DespawnGameObject(itr);
    }
}

Optional<uint32> GameEventMgr::GetEquipmentOverride(uint32 spawnId)
{
    auto itr = mGameEventEquipOverrides.find(spawnId);
    if (itr == mGameEventEquipOverrides.end())
        return {};

    return itr->second;
}

void GameEventMgr::ChangeEquipOrModel(int16 event_id, bool activate)
{
    for(auto& itr : mGameEventModelEquip[event_id]) //sun: added reference, else code below won't work...
    {
        // Remove the creature from grid
        uint32 spawnId = itr.first;
        CreatureData const* data = sObjectMgr->GetCreatureData(spawnId);
        if(!data)
            continue;

        if (activate)
            mGameEventEquipOverrides[spawnId] = itr.second.equipment_id;
        else
            mGameEventEquipOverrides.erase(spawnId);

        // Update if spawned
        sMapMgr->DoForAllMapsWithMapId(data->spawnPoint.GetMapId(), [&](Map* map)
        {
            auto creatureBounds = map->GetCreatureBySpawnIdStore().equal_range(spawnId);
            for (auto itr2 = creatureBounds.first; itr2 != creatureBounds.second; ++itr2)
            {
                Creature* creature = itr2->second;
                if (activate)
                {
                    itr.second.equipement_id_prev = creature->GetCurrentEquipmentId();
                    itr.second.modelid_prev = creature->GetDisplayId();
                    creature->LoadEquipment(itr.second.equipment_id, true);

                    if (itr.second.modelid >0 && itr.second.modelid_prev != itr.second.modelid)
                    {
                        uint32 newModelId = itr.second.modelid; //may be changed at next line
                        CreatureModelInfo const *minfo = sObjectMgr->GetCreatureModelSameGenderAndRaceAs(newModelId, itr.second.modelid_prev);
                        if (minfo)
                        {
                            creature->SetDisplayId(itr.second.modelid);
                            creature->SetNativeDisplayId(itr.second.modelid);
                            creature->SetFloatValue(UNIT_FIELD_BOUNDINGRADIUS, minfo->bounding_radius);
                            creature->SetFloatValue(UNIT_FIELD_COMBATREACH, minfo->combat_reach);
                        }
                    }
                }
                else
                {
                    creature->LoadEquipment(itr.second.equipement_id_prev, true);
                    if (itr.second.modelid_prev >0 && itr.second.modelid_prev != itr.second.modelid)
                    {
                        CreatureModelInfo const *minfo = sObjectMgr->GetCreatureModelInfo(itr.second.modelid_prev);
                        if (minfo)
                        {
                            creature->SetDisplayId(itr.second.modelid_prev);
                            creature->SetNativeDisplayId(itr.second.modelid_prev);
                            creature->SetFloatValue(UNIT_FIELD_BOUNDINGRADIUS, minfo->bounding_radius);
                            creature->SetFloatValue(UNIT_FIELD_COMBATREACH, minfo->combat_reach);
                        }
                    }
                }
            }
        });

        // sun: replace CreatureData modifications with GetEquipmentOverride
    }
}

bool GameEventMgr::hasCreatureQuestActiveEventExcept(uint32 quest_id, uint16 event_id)
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

bool GameEventMgr::hasGameObjectQuestActiveEventExcept(uint32 quest_id, uint16 event_id)
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
bool GameEventMgr::hasCreatureActiveEventExcept(uint32 creature_id, uint16 event_id)
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
bool GameEventMgr::hasGameObjectActiveEventExcept(uint32 go_id, uint16 event_id)
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

//move this to ObjectAccessor ? HACKY HACKY AND HEAVY
void AddQuestFlagToCreatures(uint32 entry)
{
    //add gossip flag to existing creatures if needed
 /* Disabled for now as this seems to freeze player session
    boost::shared_lock<boost::shared_mutex> lock(*HashMapHolder<Creature>::GetLock());
    auto creatures = ObjectAccessor::GetCreatures();
    for (auto itr : creatures)
    {
        Creature* creature = itr.second;
        if (creature->GetEntry() != entry)
            continue;

        creature->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);

        //update questgiver status (exclamation mark over creature heads) for every players having this creature at client
        Map::PlayerList const& players = creature->GetMap()->GetPlayers();
        for (auto itr2 : players)
        {
            Player* player = itr2.GetSource();
            if (player->HaveAtClient(creature))
            {
                WorldPacket fakePacket(CMSG_QUESTGIVER_STATUS_QUERY);
                fakePacket << itr.first;
                player->GetSession()->HandleQuestgiverStatusQueryOpcode(fakePacket);
            }
        }
    }
    */
}

void GameEventMgr::UpdateEventQuests(uint16 event_id, bool Activate)
{
    for (auto itr : mGameEventCreatureQuests[event_id])
    {
        QuestRelations &CreatureQuestMap = sObjectMgr->mCreatureQuestRelations;
        if (Activate)                                       // Add the pair(id,quest) to the multimap
        {
            CreatureQuestMap.insert(QuestRelations::value_type(itr.first, itr.second));
            AddQuestFlagToCreatures(itr.first);
        }
        else
        {
            if(!hasCreatureQuestActiveEventExcept(itr.second,event_id))
            {
                // Remove the pair(id,quest) from the multimap
                auto qitr = CreatureQuestMap.find(itr.first);
                if (qitr == CreatureQuestMap.end())
                    continue;
                auto lastElement = CreatureQuestMap.upper_bound(itr.first);
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
                auto qitr = GameObjectQuestMap.find(itr.first);
                if (qitr == GameObjectQuestMap.end())
                    continue;
                auto lastElement = GameObjectQuestMap.upper_bound(itr.first);
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

GameEventMgr::GameEventMgr()
{
    isSystemInit = false;
}

void GameEventMgr::HandleQuestComplete(uint32 quest_id)
{
    // translate the quest to event and condition
    auto itr = mQuestToEventConditions.find(quest_id);
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

bool GameEventMgr::CheckOneGameEventConditions(uint16 event_id)
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
        time_t currenttime = GameTime::GetGameTime();
        mGameEvent[event_id].nextstart = currenttime + mGameEvent[event_id].length * 60;
    }
    return true;
}

void GameEventMgr::SaveWorldEventStateToDB(uint16 event_id)
{
    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    trans->PAppend("DELETE FROM game_event_save WHERE event_id = '%u'",event_id);
    if(mGameEvent[event_id].nextstart)
        trans->PAppend("INSERT INTO game_event_save (event_id, state, next_start) VALUES ('%u','%u',FROM_UNIXTIME(" UI64FMTD "))",event_id,mGameEvent[event_id].state,(uint64)(mGameEvent[event_id].nextstart));
    else
        trans->PAppend("INSERT INTO game_event_save (event_id, state, next_start) VALUES ('%u','%u','0000-00-00 00:00:00')",event_id,mGameEvent[event_id].state);
    CharacterDatabase.CommitTransaction(trans);
}

void GameEventMgr::SendWorldStateUpdate(Player * plr, uint16 event_id)
{
    for(auto itr : mGameEvent[event_id].conditions)
    {
        if(itr.second.done_world_state)
            plr->SendUpdateWorldState(itr.second.done_world_state, (uint32)(itr.second.done));
        if(itr.second.max_world_state)
            plr->SendUpdateWorldState(itr.second.max_world_state, (uint32)(itr.second.reqNum));
    }
}

class GameEventAIHookWorker
{
public:
    GameEventAIHookWorker(uint16 eventId, bool activate) : _eventId(eventId), _activate(activate) { }

    void Visit(std::unordered_map<ObjectGuid, Creature*>& creatureMap)
    {
        for (auto const& p : creatureMap)
            if (p.second->IsInWorld() && p.second->IsAIEnabled())
                p.second->AI()->OnGameEvent(_activate, _eventId);
    }

    void Visit(std::unordered_map<ObjectGuid, GameObject*>& gameObjectMap)
    {
        for (auto const& p : gameObjectMap)
            if (p.second->IsInWorld() && p.second->AI())
                p.second->AI()->OnGameEvent(_activate, _eventId);
    }

    template<class T>
    void Visit(std::unordered_map<ObjectGuid, T*>&) { }

private:
    uint16 _eventId;
    bool _activate;
};

void GameEventMgr::WarnAIScripts(uint16 event_id, bool activate)
{
    //! Iterate over every supported source type (creature and gameobject)
    //! Not entirely sure how this will affect units in non-loaded grids.
    sMapMgr->DoForAllMaps([event_id, activate](Map* map)
    {
        GameEventAIHookWorker worker(event_id, activate);
        TypeContainerVisitor<GameEventAIHookWorker, MapStoredObjectTypesContainer> visitor(worker);
        visitor.Visit(map->GetObjectsStore());
    });
}

bool GameEventMgr::AddCreatureToEvent(uint32 spawnId, int16 event_id, Map* map /* = nullptr */)
{ 
    if(!spawnId || !event_id)
        return false;

    if ((event_id > 0 && event_id >= int32(mGameEvent.size()))
        || (event_id < 0 && -event_id >= int32(mGameEvent.size())))
        return false;

    //Check if creature already linked to an event
    if (GetCreatureEvent(spawnId))
        return false;

    //Add to gameevent creature map
    int32 internal_event_id = mGameEvent.size() + event_id - 1;
    auto& crelist = mGameEventCreatureGuids[internal_event_id];
    crelist.push_back(spawnId);

    //Save in db
    WorldDatabase.PQuery("REPLACE INTO game_event_creature (guid, event) VALUES (%u,%i)", spawnId, event_id);

    //Spawn/Despawn IG if needed
    CreatureData const* data = sObjectMgr->GetCreatureData(spawnId);
    if(map && data)
    {
        if (ShouldHaveObjectsSpawned(event_id))
            SpawnCreature(spawnId);
        else
            DespawnCreature(spawnId, event_id);
    }

    return true; 
}
bool GameEventMgr::AddGameObjectToEvent(uint32 spawnId, int16 event_id, Map* map /* = nullptr */)
{
    if(!spawnId || !event_id)
        return false;

    if (   (event_id > 0 && event_id >= int32(mGameEvent.size()))
        || (event_id < 0 && -event_id >= int32(mGameEvent.size())))
        return false;

    //Check if creature already linked to an event
    if (GetGameObjectEvent(spawnId))
        return false;

    //Add to gameevent creature map
    int32 internal_event_id = mGameEvent.size() + event_id - 1;
    auto& crelist = mGameEventGameobjectGuids[internal_event_id];
    crelist.push_back(spawnId);

    //Save in db
    WorldDatabase.PQuery("REPLACE INTO game_event_gameobject (guid, event) VALUES (%u,%i)", spawnId, event_id);

    //Spawn/Despawn IG if needed
    GameObjectData const* data = sObjectMgr->GetGameObjectData(spawnId);
    if(map && data)
    {
        if (ShouldHaveObjectsSpawned(event_id))
            SpawnGameObject(spawnId);
        else
            DespawnGameObject(spawnId);
    }

    return true; 
}

bool GameEventMgr::RemoveCreatureFromEvent(uint32 spawnId, Map* map /* = nullptr */)
{ 
    if(!spawnId)
        return false;

    //Check if creature is linked to an event
    uint16 event_id = GetCreatureEvent(spawnId);
    if (!event_id)
        return false;

    //remove to gameevent creature map
    int32 internal_event_id = mGameEvent.size() + event_id - 1;
    if (internal_event_id >= mGameEventCreatureGuids.size())
    {
        TC_LOG_ERROR("gameevent", "RemoveCreatureFromEvent: Tried to access invalid internal_event_id %i", internal_event_id);
        return false;
    }
    auto& crelist = mGameEventCreatureGuids[internal_event_id];
    crelist.remove(spawnId);

    //Remove from DB
    WorldDatabase.PQuery("DELETE FROM game_event_creature WHERE guid = %u;", spawnId);

    //Respawn IG if needed
    CreatureData const* data = sObjectMgr->GetCreatureData(spawnId);
    if(map && data)
        SpawnCreature(spawnId);

    return true;
}
bool GameEventMgr::RemoveGameObjectFromEvent(uint32 spawnId, Map* map /* = nullptr */)
{ 
    if(!spawnId)
        return false;

    //Check if creature is linked to an event
    uint16 event_id = GetGameObjectEvent(spawnId);
    if (!event_id)
        return false;

    //Remove to gameevent creature map
    int32 internal_event_id = mGameEvent.size() + event_id - 1;
    if (internal_event_id >= mGameEventGameobjectGuids.size())
    {
        TC_LOG_ERROR("gameevent", "RemoveGameObjectFromEvent: Tried to access invalid internal_event_id %i", internal_event_id);
        return false;
    }
    auto& crelist = mGameEventGameobjectGuids[internal_event_id];
    crelist.remove(spawnId);

    //Remove from DB
    WorldDatabase.PQuery("DELETE FROM game_event_gameobject WHERE guid = %u;", spawnId);

    //Respawn IG if needed
    GameObjectData const* data = sObjectMgr->GetGameObjectData(spawnId);
    if(map && data)
        SpawnGameObject(spawnId);

    return true;
}

bool GameEventMgr::CreateGameEvent(const char* name,int16& event_id)
{ 
    if(!name)
    {
        TC_LOG_ERROR("gameevent","CreateGameEvent(...) : No name given");
        return false;
    }

    event_id = mGameEvent.size();
    /*
    // crash crash crash. Remember events may not be continguous, probably the problem here

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

int16 GameEventMgr::GetCreatureEvent(uint32 spawnId)
{ 
    if(!spawnId)
        return 0;

    for(int i = -((int)mGameEvent.size())+1; i < (int)mGameEvent.size();i++)
    {
        int32 internal_event_id = mGameEvent.size() + i - 1;
        for(auto itr : mGameEventCreatureGuids[internal_event_id])
        {
            if(itr == spawnId)
                return i;
        }
    }
    return 0;
}

int16 GameEventMgr::GetGameObjectEvent(uint32 spawnId)
{ 
    if(!spawnId)
        return 0;

    for(int i = -((int)mGameEvent.size())+1; i < (int)mGameEvent.size();i++)
    {
        int32 internal_event_id = mGameEvent.size() + i - 1;
        for(auto itr : mGameEventGameobjectGuids[internal_event_id])
        {
            if(itr == spawnId)
                return i;
        }
    }
    return 0;
}

bool GameEventMgr::ShouldHaveObjectsSpawned(int16 event_id)
{
    bool activeEvent = IsActiveEvent(std::abs(event_id));
    return event_id > 0 ? activeEvent : !activeEvent;
}
