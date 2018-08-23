#include "Chat.h"
#include "Map.h"
#include "InstanceScript.h"
#include "InstanceSaveMgr.h"
#include "MapManager.h"

class instance_commandscript : public CommandScript
{
public:
    instance_commandscript() : CommandScript("instance_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> instanceCommandTable =
        {
            { "listbinds",      SEC_GAMEMASTER1,      false, &HandleInstanceListBindsCommand,   "" },
            { "unbind",         SEC_GAMEMASTER1,      false, &HandleInstanceUnbindCommand,      "" },
            { "stats",          SEC_GAMEMASTER1,      true,  &HandleInstanceStatsCommand,       "" },
            { "savedata",       SEC_GAMEMASTER1,      false, &HandleInstanceSaveDataCommand,    "" },
            { "setdata",        SEC_GAMEMASTER1,      false, &HandleInstanceSetDataCommand,     "" },
            { "getdata",        SEC_GAMEMASTER1,      false, &HandleInstanceGetDataCommand,     "" },
            { "setbossstate",   SEC_GAMEMASTER1,      false, &HandleInstanceSetBossStateCommand,"" },
            { "getbossstate",   SEC_GAMEMASTER1,      false, &HandleInstanceGetBossStateCommand,"" },
        };
        static std::vector<ChatCommand> commandTable =
        {
            { "instance",       SEC_GAMEMASTER3,      true,  nullptr,                           "", instanceCommandTable },
        };
        return commandTable;
    }

    static std::string GetTimeString(uint32 time)
    {
        uint16 days = time / DAY, hours = (time % DAY) / HOUR, minute = (time % HOUR) / MINUTE;
        std::ostringstream ss;
        if (days) ss << days << "d ";
        if (hours) ss << hours << "h ";
        ss << minute << "m";
        return ss.str();
    }

    static bool HandleInstanceListBindsCommand(ChatHandler* handler, char const* /*args*/)
    {
        Player* player = handler->GetSelectedPlayerOrSelf();
        uint32 counter = 0;
        for (uint8 i = 0; i < MAX_DIFFICULTY; i++)
        {
            Player::BoundInstancesMap &binds = player->GetBoundInstances(Difficulty(i));
            for (auto & bind : binds)
            {
                InstanceSave *save = bind.second.save;
                std::string timeleft = GetTimeString(save->GetResetTime() - time(nullptr));
                handler->PSendSysMessage("map: %d inst: %d perm: %s diff: %s canReset: %s TTR: %s", bind.first, save->GetInstanceId(), bind.second.perm ? "yes" : "no", save->GetDifficulty() == REGULAR_DIFFICULTY ? "normal" : "heroic", save->CanReset() ? "yes" : "no", timeleft.c_str());
                counter++;
            }
        }
        handler->PSendSysMessage("player binds: %d", counter);
        counter = 0;
        Group *group = player->GetGroup();
        if (group)
        {
            for (uint8 i = 0; i < MAX_DIFFICULTY; i++)
            {
                Group::BoundInstancesMap &binds = group->GetBoundInstances(Difficulty(i));
                for (auto & bind : binds)
                {
                    InstanceSave *save = bind.second.save;
                    std::string timeleft = GetTimeString(save->GetResetTime() - time(nullptr));
                    handler->PSendSysMessage("map: %d inst: %d perm: %s diff: %s canReset: %s TTR: %s", bind.first, save->GetInstanceId(), bind.second.perm ? "yes" : "no", save->GetDifficulty() == REGULAR_DIFFICULTY ? "normal" : "heroic", save->CanReset() ? "yes" : "no", timeleft.c_str());
                    counter++;
                }
            }
        }
        handler->PSendSysMessage("group binds: %d", counter);

        return true;
    }

    static bool HandleInstanceUnbindCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

            std::string cmd = args;
        if (cmd == "all")
        {
            Player* player = handler->GetSelectedPlayerOrSelf();
            uint32 counter = 0;
            for (uint8 i = 0; i < MAX_DIFFICULTY; i++)
            {
                Player::BoundInstancesMap &binds = player->GetBoundInstances(Difficulty(i));
                for (auto itr = binds.begin(); itr != binds.end();)
                {
                    if (itr->first != player->GetMapId())
                    {
                        InstanceSave *save = itr->second.save;
                        std::string timeleft = GetTimeString(save->GetResetTime() - time(nullptr));
                        handler->PSendSysMessage("unbinding map: %d inst: %d perm: %s diff: %s canReset: %s TTR: %s", itr->first, save->GetInstanceId(), itr->second.perm ? "yes" : "no", save->GetDifficulty() == REGULAR_DIFFICULTY ? "normal" : "heroic", save->CanReset() ? "yes" : "no", timeleft.c_str());
                        player->UnbindInstance(itr, Difficulty(i));
                        counter++;
                    }
                    else
                        ++itr;
                }
            }
            handler->PSendSysMessage("instances unbound: %d", counter);
        }
        return true;
    }

    static bool HandleInstanceStatsCommand(ChatHandler* handler, char const* /*args*/)
    {
        handler->PSendSysMessage("instances loaded: %d", sMapMgr->GetNumInstances());
        handler->PSendSysMessage("players in instances: %d", sMapMgr->GetNumPlayersInInstances());
        handler->PSendSysMessage("instance saves: %d", sInstanceSaveMgr->GetNumInstanceSaves());
        handler->PSendSysMessage("players bound: %d", sInstanceSaveMgr->GetNumBoundPlayersTotal());
        handler->PSendSysMessage("groups bound: %d", sInstanceSaveMgr->GetNumBoundGroupsTotal());
        return true;
    }

    static bool HandleInstanceSaveDataCommand(ChatHandler* handler, char const* args)
    {
        Player* pl = handler->GetSession()->GetPlayer();

        Map* map = pl->GetMap();
        if (!map->IsDungeon())
        {
            handler->PSendSysMessage("Map is not a dungeon.");
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (!((InstanceMap*)map)->GetInstanceScript())
        {
            handler->PSendSysMessage("Map has no instance data.");
            handler->SetSentErrorMessage(true);
            return false;
        }

        ((InstanceMap*)map)->GetInstanceScript()->SaveToDB();
        return true;
    }

    static bool HandleInstanceSetDataCommand(ChatHandler* handler, char const* args)
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

        Player *plr = handler->GetSession()->GetPlayer();

        if (InstanceScript *pInstance = ((InstanceScript*)plr->GetInstanceScript()))
            pInstance->SetData(dataId, dataValue);
        else {
            handler->PSendSysMessage("You are not in an instance.");
            return false;
        }

        return true;
    }

    static bool HandleInstanceGetDataCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

            char *chrDataId = strtok((char *)args, " ");
        if (!chrDataId)
            return false;

        uint32 dataId = uint32(atoi(chrDataId));

        Player *plr = handler->GetSession()->GetPlayer();

        if (InstanceScript *pInstance = ((InstanceScript*)plr->GetInstanceScript()))
            handler->PSendSysMessage("Instance data %u = %u.", dataId, pInstance->GetData(dataId));
        else {
            handler->PSendSysMessage("You are not in an instance.");
            return false;
        }

        return true;
    }

    static bool HandleInstanceSetBossStateCommand(ChatHandler* handler, char const* args)
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

        Player *plr = handler->GetSession()->GetPlayer();

        if (InstanceScript* pInstance = ((InstanceScript*)plr->GetInstanceScript()))
        {
            if (encounterId > pInstance->GetEncounterCount())
            {
                //PSendSysMessage(LANG_BAD_VALUE);
                handler->SendSysMessage("Too high");
                handler->SetSentErrorMessage(true);
                return false;
            }
            pInstance->SetBossState(encounterId, EncounterState(state));
            handler->PSendSysMessage("Set instance boss state %u = %u (%s).", encounterId, state, InstanceScript::GetBossStateName(state));
        }
        else
        {
            handler->PSendSysMessage("You are not in an instance.");
            return false;
        }

        return true;
    }


    static bool HandleInstanceGetBossStateCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

            char *chrDataId = strtok((char *)args, " ");
        if (!chrDataId)
            return false;

        uint32 encounterId = uint32(atoi(chrDataId));

        Player *plr = handler->GetSession()->GetPlayer();

        if (InstanceScript* pInstance = ((InstanceScript*)plr->GetInstanceScript()))
        {
            if (encounterId > pInstance->GetEncounterCount())
            {
                //PSendSysMessage(LANG_BAD_VALUE);
                handler->SendSysMessage("Too high");
                handler->SetSentErrorMessage(true);
                return false;
            }
            uint32 state = pInstance->GetBossState(encounterId);
            handler->PSendSysMessage("Instance boss state %u = %u (%s).", encounterId, state, InstanceScript::GetBossStateName(state));
        }
        else
        {
            handler->PSendSysMessage("You are not in an instance.");
            return false;
        }

        return true;
    }
};

void AddSC_instance_commandscript()
{
    new instance_commandscript();
}
