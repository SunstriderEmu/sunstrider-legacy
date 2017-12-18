#include "Chat.h"
#include "Map.h"
#include "InstanceScript.h"

std::string GetTimeString(uint32 time)
{
    uint16 days = time / DAY, hours = (time % DAY) / HOUR, minute = (time % HOUR) / MINUTE;
    std::ostringstream ss;
    if(days) ss << days << "d ";
    if(hours) ss << hours << "h ";
    ss << minute << "m";
    return ss.str();
}

bool ChatHandler::HandleInstanceListBindsCommand(const char* /*args*/)
{
    Player* player = GetSelectedPlayerOrSelf();
    uint32 counter = 0;
    for(uint8 i = 0; i < MAX_DIFFICULTY; i++)
    {
        Player::BoundInstancesMap &binds = player->GetBoundInstances(Difficulty(i));
        for(auto & bind : binds)
        {
            InstanceSave *save = bind.second.save;
            std::string timeleft = GetTimeString(save->GetResetTime() - time(nullptr));
            PSendSysMessage("map: %d inst: %d perm: %s diff: %s canReset: %s TTR: %s", bind.first, save->GetInstanceId(), bind.second.perm ? "yes" : "no",  save->GetDifficulty() == REGULAR_DIFFICULTY ? "normal" : "heroic", save->CanReset() ? "yes" : "no", timeleft.c_str());
            counter++;
        }
    }
    PSendSysMessage("player binds: %d", counter);
    counter = 0;
    Group *group = player->GetGroup();
    if(group)
    {
        for(uint8 i = 0; i < MAX_DIFFICULTY; i++)
        {
            Group::BoundInstancesMap &binds = group->GetBoundInstances(Difficulty(i));
            for(auto & bind : binds)
            {
                InstanceSave *save = bind.second.save;
                std::string timeleft = GetTimeString(save->GetResetTime() - time(nullptr));
                PSendSysMessage("map: %d inst: %d perm: %s diff: %s canReset: %s TTR: %s", bind.first, save->GetInstanceId(), bind.second.perm ? "yes" : "no",  save->GetDifficulty() == REGULAR_DIFFICULTY ? "normal" : "heroic", save->CanReset() ? "yes" : "no", timeleft.c_str());
                counter++;
            }
        }
    }
    PSendSysMessage("group binds: %d", counter);

    return true;
}

bool ChatHandler::HandleInstanceUnbindCommand(const char* args)
{
    ARGS_CHECK

    std::string cmd = args;
    if(cmd == "all")
    {
        Player* player = GetSelectedPlayerOrSelf();
        uint32 counter = 0;
        for(uint8 i = 0; i < MAX_DIFFICULTY; i++)
        {
            Player::BoundInstancesMap &binds = player->GetBoundInstances(Difficulty(i));
            for(auto itr = binds.begin(); itr != binds.end();)
            {
                if(itr->first != player->GetMapId())
                {
                    InstanceSave *save = itr->second.save;
                    std::string timeleft = GetTimeString(save->GetResetTime() - time(nullptr));
                    PSendSysMessage("unbinding map: %d inst: %d perm: %s diff: %s canReset: %s TTR: %s", itr->first, save->GetInstanceId(), itr->second.perm ? "yes" : "no",  save->GetDifficulty() == REGULAR_DIFFICULTY ? "normal" : "heroic", save->CanReset() ? "yes" : "no", timeleft.c_str());
                    player->UnbindInstance(itr, Difficulty(i));
                    counter++;
                }
                else
                    ++itr;
            }
        }
        PSendSysMessage("instances unbound: %d", counter);
    }
    return true;
}

bool ChatHandler::HandleInstanceStatsCommand(const char* /*args*/)
{
    PSendSysMessage("instances loaded: %d", sMapMgr->GetNumInstances());
    PSendSysMessage("players in instances: %d", sMapMgr->GetNumPlayersInInstances());
    PSendSysMessage("instance saves: %d", sInstanceSaveMgr->GetNumInstanceSaves());
    PSendSysMessage("players bound: %d", sInstanceSaveMgr->GetNumBoundPlayersTotal());
    PSendSysMessage("groups bound: %d", sInstanceSaveMgr->GetNumBoundGroupsTotal());
    return true;
}

bool ChatHandler::HandleInstanceSaveDataCommand(const char * /*args*/)
{
    Player* pl = m_session->GetPlayer();

    Map* map = pl->GetMap();
    if (!map->IsDungeon())
    {
        PSendSysMessage("Map is not a dungeon.");
        SetSentErrorMessage(true);
        return false;
    }

    if (!((InstanceMap*)map)->GetInstanceScript())
    {
        PSendSysMessage("Map has no instance data.");
        SetSentErrorMessage(true);
        return false;
    }

    ((InstanceMap*)map)->GetInstanceScript()->SaveToDB();
    return true;
}

bool ChatHandler::HandleInstanceSetDataCommand(const char* args)
{
    ARGS_CHECK
        
    char *chrDataId = strtok((char *)args, " ");
    if (!chrDataId)
        return false;
        
    uint32 dataId = uint32(atoi(chrDataId));
    
    char *chrDataValue = strtok(nullptr, " ");
    if (!chrDataValue)
        return false;
        
    uint32 dataValue = uint32(atoi(chrDataValue));
    
    Player *plr = m_session->GetPlayer();
    
    if (InstanceScript *pInstance = ((InstanceScript*)plr->GetInstanceScript()))
        pInstance->SetData(dataId, dataValue);
    else {
        PSendSysMessage("You are not in an instance.");
        return false;
    }
    
    return true;
}

bool ChatHandler::HandleInstanceGetDataCommand(const char* args)
{
    ARGS_CHECK
        
    char *chrDataId = strtok((char *)args, " ");
    if (!chrDataId)
        return false;
        
    uint32 dataId = uint32(atoi(chrDataId));
    
    Player *plr = m_session->GetPlayer();
    
    if (InstanceScript *pInstance = ((InstanceScript*)plr->GetInstanceScript()))
        PSendSysMessage("Instance data %u = %u.", dataId, pInstance->GetData(dataId));
    else {
        PSendSysMessage("You are not in an instance.");
        return false;
    }
    
    return true;
}

bool ChatHandler::HandleInstanceSetBossStateCommand(const char* args)
{
    ARGS_CHECK

        char *chrDataId = strtok((char *)args, " ");
    if (!chrDataId)
        return false;

    uint32 encounterId = uint32(atoi(chrDataId));
    char *chrDataValue = strtok(nullptr, " ");
    if (!chrDataValue)
        return false;
    uint32 state = uint32(atoi(chrDataValue));

    Player *plr = m_session->GetPlayer();

    if (InstanceScript* pInstance = ((InstanceScript*)plr->GetInstanceScript()))
    {
        if (encounterId > pInstance->GetEncounterCount())
        {
            //PSendSysMessage(LANG_BAD_VALUE);
            SendSysMessage("Too high");
            SetSentErrorMessage(true);
            return false;
        }
        pInstance->SetBossState(encounterId, EncounterState(state));
        PSendSysMessage("Set instance boss state %u = %u (%s).", encounterId, state, InstanceScript::GetBossStateName(encounterId));
    }
    else
    {
        PSendSysMessage("You are not in an instance.");
        return false;
    }

    return true;
}


bool ChatHandler::HandleInstanceGetBossStateCommand(const char* args)
{
    ARGS_CHECK

        char *chrDataId = strtok((char *)args, " ");
    if (!chrDataId)
        return false;

    uint32 encounterId = uint32(atoi(chrDataId));

    Player *plr = m_session->GetPlayer();

    if (InstanceScript* pInstance = ((InstanceScript*)plr->GetInstanceScript()))
    {
        if (encounterId > pInstance->GetEncounterCount())
        {
            //PSendSysMessage(LANG_BAD_VALUE);
            SendSysMessage("Too high");
            SetSentErrorMessage(true);
            return false;
        }
        uint32 state = pInstance->GetBossState(encounterId);
        PSendSysMessage("Instance boss state %u = %u (%s).", encounterId, state, InstanceScript::GetBossStateName(encounterId));
    }
    else 
    {
        PSendSysMessage("You are not in an instance.");
        return false;
    }

    return true;
}
