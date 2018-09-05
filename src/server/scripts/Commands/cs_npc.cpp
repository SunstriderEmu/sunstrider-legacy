#include "Chat.h"
#include "Language.h"
#include "Transport.h"
#include "FollowMovementGenerator.h"
#include "GameEventMgr.h"
#include "WaypointMovementGenerator.h"
#include "GridNotifiersImpl.h"
#include "CellImpl.h"
#include "PoolMgr.h"
#include <boost/algorithm/string.hpp>    

bool HandleNpcSpawnGroup(ChatHandler* handler, char const* args)
{
    if (!*args)
        return false;

    bool ignoreRespawn = false;
    bool force = false;
    uint32 groupId = 0;

    // Decode arguments
    char* arg = strtok((char*)args, " ");
    while (arg)
    {
        std::string thisArg = arg;
        boost::algorithm::to_lower(thisArg);
        if (thisArg == "ignorerespawn")
            ignoreRespawn = true;
        else if (thisArg == "force")
            force = true;
        else if (thisArg.empty() || !(std::count_if(thisArg.begin(), thisArg.end(), ::isdigit) == (int)thisArg.size()))
            return false;
        else
            groupId = atoi(thisArg.c_str());

        arg = strtok(nullptr, " ");
    }

    Player* player = handler->GetSession()->GetPlayer();

    std::vector <WorldObject*> creatureList;
    if (!player->GetMap()->SpawnGroupSpawn(groupId, ignoreRespawn, force, &creatureList))
    {
        handler->PSendSysMessage("Spawn group %u not found", groupId);
        handler->SetSentErrorMessage(true);
        return false;
    }

    handler->PSendSysMessage("Spawned a total of %zu objects:", creatureList.size());

    return true;
}

bool HandleNpcDespawnGroup(ChatHandler* handler, char const* args)
{
    if (!*args)
        return false;

    bool deleteRespawnTimes = false;
    uint32 groupId = 0;

    // Decode arguments
    char* arg = strtok((char*)args, " ");
    while (arg)
    {
        std::string thisArg = arg;
        boost::algorithm::to_lower(thisArg);
        if (thisArg == "removerespawntime")
            deleteRespawnTimes = true;
        else if (thisArg.empty() || !(std::count_if(thisArg.begin(), thisArg.end(), ::isdigit) == (int)thisArg.size()))
            return false;
        else
            groupId = atoi(thisArg.c_str());

        arg = strtok(nullptr, " ");
    }

    Player* player = handler->GetSession()->GetPlayer();

    size_t n = 0;
    if (!player->GetMap()->SpawnGroupDespawn(groupId, deleteRespawnTimes, &n))
    {
        handler->PSendSysMessage("Spawn group %u not found", groupId);
        handler->SetSentErrorMessage(true);
        return false;
    }
    handler->PSendSysMessage("Despawned a total of %zu objects.", n);

    return true;
}

class npc_commandscript : public CommandScript
{
public:
    npc_commandscript() : CommandScript("npc_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> npcPathTable =
        {
            { "type",           SEC_GAMEMASTER2,  false, &HandleNpcPathTypeCommand,           "" },
            { "direction",      SEC_GAMEMASTER2,  false, &HandleNpcPathDirectionCommand,      "" },
            { "currentid",      SEC_GAMEMASTER1,  false, &HandleNpcPathCurrentIdCommand,      "" },
        };
        static std::vector<ChatCommand> npcCommandTable =
        {
            { "say",            SEC_GAMEMASTER1,     false, &HandleNpcSayCommand,              "" },
            { "textemote",      SEC_GAMEMASTER1,     false, &HandleNpcTextEmoteCommand,        "" },
            { "add",            SEC_GAMEMASTER2,     false, &HandleNpcAddCommand,              "" },
            { "delete",         SEC_GAMEMASTER2,     false, &HandleNpcDeleteCommand,           "" },
            { "spawndist",      SEC_GAMEMASTER2,     false, &HandleNpcSpawnDistCommand,        "" },
            { "spawntime",      SEC_GAMEMASTER2,     false, &HandleNpcSpawnTimeCommand,        "" },
            { "factionid",      SEC_GAMEMASTER2,     false, &HandleNpcFactionIdCommand,        "" },
            { "setmovetype",    SEC_GAMEMASTER2,     false, &HandleNpcSetMoveTypeCommand,      "" },
            { "move",           SEC_GAMEMASTER2,     false, &HandleNpcMoveCommand,             "" },
            { "changelevel",    SEC_GAMEMASTER2,     false, &HandleNpcChangeLevelCommand,      "" },
            { "setmodel",       SEC_GAMEMASTER2,     false, &HandleNpcSetModelCommand,         "" },
            { "additem",        SEC_GAMEMASTER2,     false, &HandleNpcAddVendorItemCommand,    "" },
            { "delitem",        SEC_GAMEMASTER2,     false, &HandleNpcDelVendorItemCommand,    "" },
            { "flag",           SEC_GAMEMASTER2,     false, &HandleNpcFlagCommand,             "" },
            { "changeentry",    SEC_GAMEMASTER3,     false, &HandleNpcChangeEntryCommand,      "" },
            { "info",           SEC_GAMEMASTER3,     false, &HandleNpcInfoCommand,             "" },
            { "setemotestate",  SEC_GAMEMASTER3,     false, &HandleNpcSetEmoteStateCommand,    "" },
            { "follow",         SEC_GAMEMASTER2,     false, &HandleNpcFollowCommand,           "" },
            { "unfollow",       SEC_GAMEMASTER2,     false, &HandleNpcUnFollowCommand,         "" },
            { "whisper",        SEC_GAMEMASTER1,     false, &HandleNpcWhisperCommand,          "" },
            { "yell",           SEC_GAMEMASTER1,     false, &HandleNpcYellCommand,             "" },
            { "addtemp",        SEC_GAMEMASTER2,     false, &HandleNpcTempAddCommand,          "" },
            { "addformation",   SEC_ADMINISTRATOR,   false, &HandleNpcAddFormationCommand,     "" },
            { "removeformation",SEC_ADMINISTRATOR,   false, &HandleNpcRemoveFormationCommand,  "" },
            { "gobackhome",     SEC_GAMEMASTER3,     false, &HandleNpcGoBackHomeCommand,       "" },
            { "setpool",        SEC_GAMEMASTER3,     false, &HandleNpcSetPoolCommand,          "" },
            { "guid",           SEC_GAMEMASTER1,     false, &HandleNpcGuidCommand,             "" },
            { "addweapon",      SEC_GAMEMASTER3,     false, &HandleNpcAddWeaponCommand,        "" },
            { "massfactionid",  SEC_ADMINISTRATOR,   false, &HandleNpcMassFactionIdCommand,    "" },
            { "combatdistance", SEC_ADMINISTRATOR,   false, &HandleNpcSetCombatDistanceCommand,"" },
            { "combatmovallow", SEC_ADMINISTRATOR,   false, &HandleNpcAllowCombatMovementCommand,"" },
            { "linkgameevent",  SEC_ADMINISTRATOR,   false, &HandleNpcLinkGameEventCommand,    "" },
            { "unlinkgameevent",SEC_ADMINISTRATOR,   false, &HandleNpcUnlinkGameEventCommand,  "" },
            { "goto",           SEC_GAMEMASTER3,     false, &HandleNpcGotoCommand,             "" },
            { "fly",            SEC_GAMEMASTER3,     false, &HandleNpcFlyCommand,              "" },
            { "near",           SEC_GAMEMASTER3,     false, &HandleNpcNearCommand,             "" },
            { "name",           SEC_GAMEMASTER2,     false, &HandleNpcNameCommand,             "" },
            { "spawngroup",     SEC_ADMINISTRATOR,   false, &HandleNpcSpawnGroup,              "" },
            { "despawngroup",   SEC_ADMINISTRATOR,   false, &HandleNpcDespawnGroup,            "" },
            { "path",           SEC_GAMEMASTER1,     false, nullptr,                           "", npcPathTable },
        };
        static std::vector<ChatCommand> commandTable =
        {
            { "npc",            SEC_GAMEMASTER1,  false, nullptr,                 "", npcCommandTable },
        };
        return commandTable;
    }

    static bool HandleNpcGuidCommand(ChatHandler* handler, char const* args)
    {
        Creature* target =  handler->GetSelectedCreature();
        if (!target)
            return false;
        
        handler->PSendSysMessage("SpawnId: %u", target->GetSpawnId());
    
        return true;
    }

    static bool HandleNpcSayCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

        Creature* pCreature =  handler->GetSelectedCreature();
        if(!pCreature)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        pCreature->Say(args, LANG_UNIVERSAL, nullptr);

        // make some emotes
        char lastchar = args[strlen(args) - 1];
        switch(lastchar)
        {
            case '?':   pCreature->HandleEmoteCommand(EMOTE_ONESHOT_QUESTION);      break;
            case '!':   pCreature->HandleEmoteCommand(EMOTE_ONESHOT_EXCLAMATION);   break;
            default:    pCreature->HandleEmoteCommand(EMOTE_ONESHOT_TALK);          break;
        }

        return true;
    }

    static bool HandleNpcYellCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

        Creature* pCreature =  handler->GetSelectedCreature();
        if(!pCreature)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        pCreature->Yell(args, LANG_UNIVERSAL, nullptr);

        // make an emote
        pCreature->HandleEmoteCommand(EMOTE_ONESHOT_SHOUT);

        return true;
    }

    //show text emote by creature in chat
    static bool HandleNpcTextEmoteCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

        Creature* pCreature =  handler->GetSelectedCreature();

        if(!pCreature)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        pCreature->TextEmote(args, nullptr);

        return true;
    }

    // make npc whisper to player
    static bool HandleNpcWhisperCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

        char* receiver_str = strtok((char*)args, " ");
        char* text = strtok(nullptr, "");

        ObjectGuid guid = handler->GetSession()->GetPlayer()->GetTarget();
        Creature* pCreature = ObjectAccessor::GetCreature(*handler->GetSession()->GetPlayer(), guid);

        if(!pCreature || !receiver_str || !text)
        {
            return false;
        }

        ObjectGuid receiver_guid = ObjectGuid(uint64(atol(receiver_str)));
        Player* targetPlayer = ObjectAccessor::FindPlayer(receiver_guid);
        if(!targetPlayer)
        {
            handler->SendSysMessage("Could not find player with that name.");
            return true;
        }

        pCreature->Whisper(text,LANG_UNIVERSAL,targetPlayer);

        return true;
    }

    //add spawn of creature
    static bool HandleNpcAddCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK
        char* cId = strtok((char*)args, " ");
        if (!cId)
            return false;

        uint32 id  = atoi(cId);

        Player *chr = handler->GetSession()->GetPlayer();
        float x = chr->GetPositionX();
        float y = chr->GetPositionY();
        float z = chr->GetPositionZ();
        float o = chr->GetOrientation();
        Map *map = chr->GetMap();

        if (Transport* tt = chr->GetTransport())
        {
            if (MotionTransport* trans = tt->ToMotionTransport())
            {
                ObjectGuid::LowType guid = sObjectMgr->GenerateCreatureSpawnId();
                CreatureData& data = sObjectMgr->NewOrExistCreatureData(guid);
                data.id = id;
                data.spawnPoint.m_positionX = chr->GetTransOffsetX();
                data.spawnPoint.m_positionY = chr->GetTransOffsetY();
                data.spawnPoint.m_positionZ = chr->GetTransOffsetZ();
                data.spawnPoint.m_orientation = chr->GetTransOffsetO();
                data.displayid = 0;
                data.equipmentId = 0;
                data.spawntimesecs = 300;
                data.spawndist = 0;
                data.movementType = 1;
                data.spawnMask = 1;

                if (!trans->GetGOInfo())
                {
                    handler->SendSysMessage("Error: cannot save creature on transport because trans->GetGOInfo() == NULL");
                    return true;
                }
                if (Creature* creature = trans->CreateNPCPassenger(guid, &data))
                {
                    sObjectMgr->AddCreatureToGrid(guid, &data);
                    creature->SaveToDB(trans->GetGOInfo()->moTransport.mapID, 1 << map->GetSpawnMode());
                }
                else {
                    handler->SendSysMessage("Error: cannot create NPC Passenger.");
                }
                return true;
            }
        }

        auto creature = new Creature;
        if (!creature->Create(map->GenerateLowGuid<HighGuid::Unit>(), map, chr->GetPhaseMask(), id, { x, y, z, o }))
        {
            delete creature;
            return false;
        }

        if(!creature->IsPositionValid())
        {
            TC_LOG_ERROR("command","ERROR: Creature (guidlow %d, entry %d) not created. Suggested coordinates isn't valid (X: %f Y: %f)", creature->GetGUID().GetCounter(), creature->GetEntry(), creature->GetPositionX(), creature->GetPositionY());
            delete creature;
            return false;
        }

        creature->SaveToDB(map->GetId(), (1 << map->GetSpawnMode()));

        ObjectGuid::LowType db_guid = creature->GetSpawnId(); //spawn id gets generated in SaveToDB

        // To call _LoadGoods(); _LoadQuests(); CreateTrainerSpells();
        // current "creature" variable is deleted and created fresh new, otherwise old values might trigger asserts or cause undefined behavior
        creature->CleanupsBeforeDelete();
        delete creature;
        creature = new Creature();
        if (!creature->LoadFromDB(db_guid, map, true, true))
        {
            delete creature;
            return false;
        }

        sObjectMgr->AddCreatureToGrid(db_guid, sObjectMgr->GetCreatureData(db_guid));
        return true;
    }

    static bool HandleNpcDeleteCommand(ChatHandler* handler, char const* args)
    {
        Creature* unit = nullptr;

        if(*args)
        {
            // number or [name] Shift-click form |color|Hcreature:creature_guid|h[name]|h|r
            char* cId = handler->extractKeyFromLink((char*)args,"Hcreature");
            if(!cId)
                return false;

            ObjectGuid::LowType lowguid = atoi(cId);
            if(!lowguid)
                return false;

            // force respawn to make sure we find something
            handler->GetSession()->GetPlayer()->GetMap()->ForceRespawn(SPAWN_TYPE_CREATURE, lowguid);
            unit = handler->GetCreatureFromPlayerMapByDbGuid(lowguid);
        }
        else
            unit =  handler->GetSelectedCreature();

        if(!unit || unit->IsPet() || unit->IsTotem())
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // Delete the creature
        unit->DeleteFromDB();
        unit->AddObjectToRemoveList();

        handler->SendSysMessage(LANG_COMMAND_DELCREATMESSAGE);

        return true;
    }

    //move selected creature
    static bool HandleNpcMoveCommand(ChatHandler* handler, char const* args)
    {
        ObjectGuid::LowType lowguid = 0;

        Creature* pCreature =  handler->GetSelectedCreature();

        if(!pCreature)
        {
            // number or [name] Shift-click form |color|Hcreature:creature_guid|h[name]|h|r
            char* cId = handler->extractKeyFromLink((char*)args,"Hcreature");
            if(!cId)
                return false;

            lowguid = atoi(cId);

            /* FIXME: impossible without entry
            if(lowguid)
                pCreature = ObjectAccessor::GetCreature(*handler->GetSession()->GetPlayer(),MAKE_GUID(lowguid,HighGuid::Unit));
            */

            // Attempting creature load from DB data
            if(!pCreature)
            {
                CreatureData const* data = sObjectMgr->GetCreatureData(lowguid);
                if(!data)
                {
                    handler->PSendSysMessage(LANG_COMMAND_CREATGUIDNOTFOUND, lowguid);
                    handler->SetSentErrorMessage(true);
                    return false;
                }

                uint32 map_id = data->spawnPoint.GetMapId();

                if(handler->GetSession()->GetPlayer()->GetMapId() != map_id)
                {
                    handler->PSendSysMessage(LANG_COMMAND_CREATUREATSAMEMAP, lowguid);
                    handler->SetSentErrorMessage(true);
                    return false;
                }
            }
            else
            {
                lowguid = pCreature->GetSpawnId();
            }
        }
        else
        {
            lowguid = pCreature->GetSpawnId();
        }
        float x,y,z,o;

        if(Transport* trans = handler->GetSession()->GetPlayer()->GetTransport())
        {
            x = handler->GetSession()->GetPlayer()->GetTransOffsetX();
            y = handler->GetSession()->GetPlayer()->GetTransOffsetY();
            z = handler->GetSession()->GetPlayer()->GetTransOffsetZ();
            o = handler->GetSession()->GetPlayer()->GetTransOffsetO();
        } else {
            x = handler->GetSession()->GetPlayer()->GetPositionX();
            y = handler->GetSession()->GetPlayer()->GetPositionY();
            z = handler->GetSession()->GetPlayer()->GetPositionZ();
            o = handler->GetSession()->GetPlayer()->GetOrientation();
        }

        if (pCreature)
        {
            if(CreatureData const* data = sObjectMgr->GetCreatureData(pCreature->GetSpawnId()))
                const_cast<CreatureData*>(data)->spawnPoint.Relocate(x, y, z, o);

            pCreature->UpdatePosition(x, y, z, o);
            pCreature->InitCreatureAddon(true);
            pCreature->GetMotionMaster()->Initialize();
            if(pCreature->IsAlive())                            // dead creature will reset movement generator at respawn
            {
                pCreature->SetDeathState(JUST_DIED);
                pCreature->Respawn();
            }
        }

        WorldDatabase.PExecute("UPDATE creature SET position_x = '%f', position_y = '%f', position_z = '%f', orientation = '%f' WHERE guid = '%u'", x, y, z, o, lowguid);

        handler->PSendSysMessage(LANG_COMMAND_CREATUREMOVED);
        return true;
    }

    static bool HandleNpcFlyCommand(ChatHandler* handler, char const* args)
    {
        Creature* pCreature =  handler->GetSelectedCreature();
        if(!pCreature)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            return true;
        }

        ARGS_CHECK

        std::string argstr = (char*)args;

        if (argstr == "on")
        {
            pCreature->SetCanFly(true);
            handler->PSendSysMessage("Creature is now fly-capable");
            return true;
        } else if (argstr == "off")
        {
            pCreature->SetCanFly(false);
            handler->PSendSysMessage("Creature is now not fly-capable");
            return true;
        }

        return false;
    }

    static bool HandleNpcGotoCommand(ChatHandler* handler, char const* args)
    {
        Creature* pCreature =  handler->GetSelectedCreature();
        if(!pCreature)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            return true;
        }

        ARGS_CHECK

        return true;
    }

    //change level of creature or pet
    static bool HandleNpcChangeLevelCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

        uint8 lvl = (uint8) atoi((char*)args);
        if ( lvl < 1 || lvl > sWorld->getConfig(CONFIG_MAX_PLAYER_LEVEL) + 3)
        {
            handler->SendSysMessage(LANG_BAD_VALUE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Creature* pCreature =  handler->GetSelectedCreature();
        if(!pCreature)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if(pCreature->IsPet())
        {
            ((Pet*)pCreature)->GivePetLevel(lvl);
        }
        else
        {
            pCreature->SetMaxHealth( 100 + 30*lvl);
            pCreature->SetHealth( 100 + 30*lvl);
            pCreature->SetLevel( lvl);
            pCreature->SaveToDB();
        }

        return true;
    }

    //set npcflag of creature
    static bool HandleNpcFlagCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

        uint32 npcFlags = (uint32) atoi((char*)args);

        Creature* pCreature =  handler->GetSelectedCreature();

        if(!pCreature)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        pCreature->SetUInt32Value(UNIT_NPC_FLAGS, npcFlags);

        //WorldDatabase.PExecute("UPDATE creature_template SET npcflag = '%u' WHERE entry = '%u'", npcFlags, pCreature->GetEntry());

        handler->SendSysMessage(LANG_VALUE_SAVED_REJOIN);

        return true;
    }

    //set model of creature
    static bool HandleNpcSetModelCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

        uint32 displayId = (uint32) atoi((char*)args);

        Creature *pCreature =  handler->GetSelectedCreature();

        if(!pCreature || pCreature->IsPet())
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        pCreature->SetDisplayId(displayId);
        pCreature->SetNativeDisplayId(displayId);

        pCreature->SaveToDB();

        return true;
    }

    //del item from vendor list
    static bool HandleNpcDelVendorItemCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

        Creature* vendor =  handler->GetSelectedCreature();
        if (!vendor || !vendor->IsVendor())
        {
            handler->SendSysMessage(LANG_COMMAND_VENDORSELECTION);
            handler->SetSentErrorMessage(true);
            return false;
        }

        char* pitem  = handler->extractKeyFromLink((char*)args,"Hitem");
        if (!pitem)
        {
            handler->SendSysMessage(LANG_COMMAND_NEEDITEMSEND);
            handler->SetSentErrorMessage(true);
            return false;
        }
        uint32 itemId = atol(pitem);
    
        ItemTemplate const* pProto = sObjectMgr->GetItemTemplate(itemId);
        if(!pProto)
        {
            handler->PSendSysMessage("Invalid id");
            return true;
        }

        if(!sObjectMgr->RemoveVendorItem(vendor->GetEntry(),pProto))
        {
            handler->PSendSysMessage(LANG_ITEM_NOT_IN_LIST,itemId);
            handler->SetSentErrorMessage(true);
            return false;
        }


        handler->PSendSysMessage(LANG_ITEM_DELETED_FROM_LIST,itemId,pProto->Name1.c_str());
        return true;
    }

    /**
     * Set the movement type for an NPC.<br/>
     * <br/>
     * Valid movement types are:
     * <ul>
     * <li> stay - NPC wont move </li>
     * <li> random - NPC will move randomly according to the spawndist </li>
     * <li> way - NPC will move with given waypoints set </li>
     * </ul>
     * additional parameter: NODEL - so no waypoints are deleted, if you
     *                       change the movement type
     */
    static bool HandleNpcSetMoveTypeCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

        // 3 arguments:
        // GUID (optional - you can also select the creature)
        // stay|random|way (determines the kind of movement)
        // NODEL (optional - tells the system NOT to delete any waypoints)
        //        this is very handy if you want to do waypoints, that are
        //        later switched on/off according to special events (like escort
        //        quests, etc)
        char* guid_str = strtok((char*)args, " ");
        char* type_str = strtok((char*)nullptr, " ");
        char* dontdel_str = strtok((char*)nullptr, " ");

        bool doNotDelete = false;

        if(!guid_str)
            return false;

        ObjectGuid::LowType lowguid = 0;
        Creature* pCreature = nullptr;

        if( dontdel_str )
        {
            //TC_LOG_ERROR("command","DEBUG: All 3 params are set");

            // All 3 params are set
            // GUID
            // type
            // doNotDEL
            if( stricmp( dontdel_str, "NODEL" ) == 0 )
            {
                //TC_LOG_ERROR("command","DEBUG: doNotDelete = true;");
                doNotDelete = true;
            }
        }
        else
        {
            // Only 2 params - but maybe NODEL is set
            if( type_str )
            {
                TC_LOG_ERROR("command","DEBUG: Only 2 params ");
                if( stricmp( type_str, "NODEL" ) == 0 )
                {
                    //TC_LOG_ERROR("command","DEBUG: type_str, NODEL ");
                    doNotDelete = true;
                    type_str = nullptr;
                }
            }
        }

        if(!type_str)                                           // case .setmovetype $move_type (with selected creature)
        {
            type_str = guid_str;
            pCreature =  handler->GetSelectedCreature();
            if(!pCreature || pCreature->IsPet())
                return false;
            lowguid = pCreature->GetSpawnId();
        }
        else                                                    // case .setmovetype #creature_guid $move_type (with selected creature)
        {
            lowguid = atoi((char*)guid_str);

            /* impossible without entry
            if(lowguid)
                pCreature = ObjectAccessor::GetCreature(*handler->GetSession()->GetPlayer(),MAKE_GUID(lowguid,HighGuid::Unit));
            */

            // attempt check creature existence by DB data
            if(!pCreature)
            {
                CreatureData const* data = sObjectMgr->GetCreatureData(lowguid);
                if(!data)
                {
                    handler->PSendSysMessage(LANG_COMMAND_CREATGUIDNOTFOUND, lowguid);
                    handler->SetSentErrorMessage(true);
                    return false;
                }
            }
            else
            {
                lowguid = pCreature->GetSpawnId();
            }
        }

        // now lowguid is low guid really existed creature
        // and pCreature point (maybe) to this creature or NULL

        MovementGeneratorType move_type;

        std::string type = type_str;

        if(type == "stay")
            move_type = IDLE_MOTION_TYPE;
        else if(type == "random")
            move_type = RANDOM_MOTION_TYPE;
        else if(type == "way")
            move_type = WAYPOINT_MOTION_TYPE;
        else
            return false;

        if(pCreature)
        {
            // update movement type
            if(doNotDelete == false)
                pCreature->LoadPath(0);

            pCreature->SetDefaultMovementType(move_type);
            pCreature->GetMotionMaster()->Initialize();
            if(pCreature->IsAlive())                            // dead creature will reset movement generator at respawn
            {
                pCreature->SetDeathState(JUST_DIED);
                pCreature->Respawn();
            }
            pCreature->SaveToDB();
        }
        if( doNotDelete == false )
        {
            handler->PSendSysMessage(LANG_MOVE_TYPE_SET,type_str);
        }
        else
        {
            handler->PSendSysMessage(LANG_MOVE_TYPE_SET_NODEL,type_str);
        }

        return true;
    }                                                           // HandleNpcSetMoveTypeCommand

    static bool HandleNpcChangeEntryCommand(ChatHandler* handler, char const* args)
    {
        uint32 newEntryNum = atoi(args);
        if(!newEntryNum)
            return false;

        Unit* unit = handler->GetSelectedUnit();
        if(!unit || unit->GetTypeId() != TYPEID_UNIT)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }
        Creature* creature = unit->ToCreature();
        if(creature->UpdateEntry(newEntryNum))
            handler->SendSysMessage(LANG_DONE);
        else
            handler->SendSysMessage(LANG_ERROR);
        return true;
    }

    static bool HandleNpcInfoCommand(ChatHandler* handler, char const* /*args*/)
    {
        Creature* target = handler->GetSelectedCreature();

        if(!target)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        uint32 faction = target->GetFaction();
        uint32 npcflags = target->GetUInt32Value(UNIT_NPC_FLAGS);
        uint32 displayid = target->GetDisplayId();
        uint32 nativeid = target->GetNativeDisplayId();
        uint32 Entry = target->GetEntry();
        CreatureTemplate const* cInfo = target->GetCreatureTemplate();

        int32 curRespawnDelay = target->GetRespawnTimeEx()-time(nullptr);
        if(curRespawnDelay < 0)
            curRespawnDelay = 0;
        std::string curRespawnDelayStr = secsToTimeString(curRespawnDelay,true);
        std::string defRespawnDelayStr = secsToTimeString(target->GetRespawnDelay(),true);

        handler->PSendSysMessage(LANG_NPCINFO_CHAR, target->GetSpawnId(), faction, npcflags, Entry, displayid, nativeid);
        if(cInfo->difficulty_entry_1)
            handler->PSendSysMessage("Heroic Entry: %u", cInfo->difficulty_entry_1);
        else if (target->GetMap()->IsHeroic() && Entry != cInfo->Entry)
            handler->PSendSysMessage("Current id (heroic id ?) : %u", cInfo->Entry);

        handler->PSendSysMessage(LANG_NPCINFO_LEVEL, target->GetLevel());
        handler->PSendSysMessage(LANG_NPCINFO_HEALTH,target->GetCreateHealth(), target->GetMaxHealth(), target->GetHealth());
        handler->PSendSysMessage(LANG_NPCINFO_FLAGS, target->GetUInt32Value(UNIT_FIELD_FLAGS), target->GetUInt32Value(UNIT_DYNAMIC_FLAGS), target->GetFaction());
        handler->PSendSysMessage(LANG_COMMAND_RAWPAWNTIMES, defRespawnDelayStr.c_str(),curRespawnDelayStr.c_str());
        handler->PSendSysMessage(LANG_NPCINFO_LOOT,  cInfo->lootid,cInfo->pickpocketLootId,cInfo->SkinLootId);
        handler->PSendSysMessage(LANG_NPCINFO_DUNGEON_ID, target->GetInstanceId());
        handler->PSendSysMessage(LANG_NPCINFO_POSITION,float(target->GetPositionX()), float(target->GetPositionY()), float(target->GetPositionZ()), float(target->GetOrientation()));
        handler->PSendSysMessage("AIName: %s", target->GetAIName().c_str());
        handler->PSendSysMessage("ScriptName: %s", target->GetScriptName().c_str());
        handler->PSendSysMessage(LANG_NPCINFO_MOVEMENT_DATA, target->GetMovementTemplate().ToString().c_str());
        handler->PSendSysMessage("Creature Pool ID: %u", target->GetCreaturePoolId());


        if ((npcflags & UNIT_NPC_FLAG_VENDOR) )
            handler->SendSysMessage(LANG_NPCINFO_VENDOR);

        if ((npcflags & UNIT_NPC_FLAG_TRAINER) )
            handler->SendSysMessage(LANG_NPCINFO_TRAINER);

        if(target->GetWaypointPath())
            handler->PSendSysMessage("PathID : %u", target->GetWaypointPath());

        if(target->GetFormation())
            handler->PSendSysMessage("Member of formation %u", target->GetFormation()->GetId());

        SpawnData const* data = sObjectMgr->GetSpawnData(SPAWN_TYPE_CREATURE, target->GetSpawnId());
        if (data && data->spawnGroupData)
            handler->PSendSysMessage("Member of spawn group %u", data->spawnGroupData->groupId);

        if(uint32 poolid = sPoolMgr->IsPartOfAPool<Creature>(target->GetSpawnId()))
            handler->PSendSysMessage("Part of pool %u", poolid);

        return true;
    }

    // .npc setemotestate #emoteid [#permanent]
    static bool HandleNpcSetEmoteStateCommand(ChatHandler* handler, char const* args)
    {
        char* cState = strtok((char*)args, " ");
        char* cPermanent = strtok((char*)nullptr, " ");

        if(!cState) 
            return false;

        uint32 state = atoi(cState);
        uint8 permanent = 0;
        if(cPermanent) 
            permanent = atoi(cPermanent);

        Creature* target =  handler->GetSelectedCreature();
        if(!target)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        target->SetEmoteState(state);
        handler->PSendSysMessage("Emote state set to %u", state);

        if(permanent)
        {
            handler->PSendSysMessage("Emote state set as permanent (will stay after a reboot)");
            if(state)
                WorldDatabase.PExecute("INSERT INTO creature_addon(`guid`,`emote`) VALUES (%u,%u) ON DUPLICATE KEY UPDATE emote = %u;", target->GetSpawnId(), state, state);
            else
                WorldDatabase.PExecute("UPDATE creature_addon SET `emote` = 0 WHERE `guid` = %u", target->GetSpawnId());
        }
        return true;
    }

    static bool HandleNpcFollowCommand(ChatHandler* handler, char const* /*args*/)
    {
        Player *player = handler->GetSession()->GetPlayer();
        Creature *creature =  handler->GetSelectedCreature();

        if(!creature)
        {
            handler->PSendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // Follow player - Using pet's default dist and angle
        creature->GetMotionMaster()->MoveFollow(player, PET_FOLLOW_DIST, creature->GetFollowAngle());

        handler->PSendSysMessage(LANG_CREATURE_FOLLOW_YOU_NOW, creature->GetName().c_str());
        return true;
    }

    static bool HandleNpcUnFollowCommand(ChatHandler* handler, char const* /*args*/)
    {
        Player *player = handler->GetSession()->GetPlayer();
        Creature *creature =  handler->GetSelectedCreature();

        if(!creature)
        {
            handler->PSendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        MovementGenerator* movement = creature->GetMotionMaster()->GetMovementGenerator([player](MovementGenerator const* a) -> bool
        {
            if (a->GetMovementGeneratorType() == FOLLOW_MOTION_TYPE)
            {
                FollowMovementGenerator const* followMovement = dynamic_cast<FollowMovementGenerator const*>(a);
                return followMovement && followMovement->GetTarget() == player;
            }
            return false;
        });

        if (!movement)
        {
            handler->PSendSysMessage(LANG_CREATURE_NOT_FOLLOW_YOU, creature->GetName().c_str());
            handler->SetSentErrorMessage(true);
            return false;
        }

        // reset movement
        creature->GetMotionMaster()->Remove(movement);

        handler->PSendSysMessage(LANG_CREATURE_NOT_FOLLOW_YOU_NOW, creature->GetName().c_str());
        return true;
    }

    // add creature, temp only
    static bool HandleNpcTempAddCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK
        char* charID = strtok((char*)args, " ");
        if (!charID)
            return false;

        Player *chr = handler->GetSession()->GetPlayer();

        float x = chr->GetPositionX();
        float y = chr->GetPositionY();
        float z = chr->GetPositionZ();
        float ang = chr->GetOrientation();

        uint32 id = atoi(charID);

        chr->SummonCreature(id,x,y,z,ang, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 1 * MINUTE * IN_MILLISECONDS);

        return true;
    }

    static bool HandleNpcAddFormationCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

        ObjectGuid::LowType leaderSpawnID = (ObjectGuid::LowType) atoi((char*)args);

        Creature* pCreature = handler->GetSelectedCreature();
        if(!pCreature || !pCreature->GetSpawnId())
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return true;
        }
    
        if(pCreature->GetFormation())
        {
            handler->PSendSysMessage("Selected creature is already member of group %u.", pCreature->GetFormation()->GetId());
            return true;
        }

        uint32 targetSpawnId = pCreature->GetSpawnId();
        if (!targetSpawnId)
        {
            handler->SendSysMessage("Target creature may not be a summon");
            return true;
        }

        CreatureData const* data = sObjectMgr->GetCreatureData(leaderSpawnID);
        if (!data)
        {
            handler->PSendSysMessage("Could not find creature data for spawnID %u", leaderSpawnID);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Creature* leader = pCreature->GetMap()->GetCreatureBySpawnId(leaderSpawnID);
        if (!leader)
        {
            handler->PSendSysMessage("Could not find leader (spawnID %u) in map.", leaderSpawnID);
            return true;
        }

        if (!leader->GetFormation())
        {
            FormationInfo* group_member;
            group_member = new FormationInfo;
            group_member->leaderGUID = leader->GetGUID();
            sCreatureGroupMgr->AddGroupMember(leaderSpawnID, group_member);
            pCreature->SearchFormation();

            WorldDatabase.PExecute("REPLACE INTO `creature_formations` (`leaderGUID`, `memberGUID`, `dist`, `angle`, `groupAI`) VALUES ('%u', '%u', 0, 0, '%u')",
                leaderSpawnID, leaderSpawnID, uint32(group_member->groupAI));

            handler->PSendSysMessage("Created formation with leader %u", leaderSpawnID);
        }

        Player* chr = handler->GetSession()->GetPlayer();
        FormationInfo* group_member;

        group_member                  = new FormationInfo;
        group_member->follow_angle    = pCreature->GetAbsoluteAngle(chr) - chr->GetOrientation();
        group_member->follow_dist     = sqrtf(pow(chr->GetPositionX() - pCreature->GetPositionX(),int(2))+pow(chr->GetPositionY()-pCreature->GetPositionY(),int(2)));
        group_member->leaderGUID      = leader->GetGUID();

        sCreatureGroupMgr->AddGroupMember(targetSpawnId, group_member);
        pCreature->SearchFormation();

        WorldDatabase.PExecute("REPLACE INTO `creature_formations` (`leaderGUID`, `memberGUID`, `dist`, `angle`, `groupAI`) VALUES ('%u', '%u','%f', '%f', '%u')",
            leaderSpawnID, targetSpawnId, group_member->follow_dist, group_member->follow_angle, uint32(group_member->groupAI));

        handler->PSendSysMessage("Creature %u added to formation with leader %u.", targetSpawnId, leaderSpawnID);

        return true;
     }

    static bool HandleNpcRemoveFormationCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

        Creature *pCreature =  handler->GetSelectedCreature();

        if(!pCreature || !pCreature->GetSpawnId())
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return true;
        }

        CreatureGroup* formation = pCreature->GetFormation();
        if(!formation)
        {
            handler->PSendSysMessage("Selected creature (%u) is not in a formation.", pCreature->GetSpawnId());
            return true;
        }

        formation->RemoveMember(pCreature);
        pCreature->SetFormation(nullptr);
        WorldDatabase.PExecute("DELETE ROM `creature_formations` WHERE memberGUID = %u", pCreature->GetSpawnId());

        handler->PSendSysMessage("Creature removed from formation.");

        return true;
    }

    static bool HandleNpcGoBackHomeCommand(ChatHandler* handler, char const* args)
    {
        if (!*args) {      // Command is applied on selected unit
            Unit* pUnit = handler->GetSelectedUnit();
            if (!pUnit || pUnit == handler->GetSession()->GetPlayer())
                return false;
        
            float x, y, z, o;
            (pUnit->ToCreature())->GetHomePosition(x, y, z, o);
            pUnit->GetMotionMaster()->MovePoint(0, x, y, z);
            return true;
        }
        else {                      // On specified GUID
            char* guid = strtok((char *)args, " ");
            Player* plr = handler->GetSession()->GetPlayer();
            if (!guid || !plr)
                return false;
            ObjectGuid uintGUID = ObjectGuid(uint64(atoll(guid)));
            QueryResult result = WorldDatabase.PQuery("SELECT id FROM creature WHERE guid = %u LIMIT 1", uintGUID);
            if (result) {
                Field *fields = result->Fetch();
                uint32 creatureentry = fields[0].GetUInt32();
                ObjectGuid fullguid = ObjectGuid(HighGuid::Unit, creatureentry, uint32(uintGUID));
                Unit* pUnit = ObjectAccessor::GetUnit(*plr, fullguid);
                if (!pUnit) {
                    handler->PSendSysMessage("No unit found.");
                    return true;
                }
                float x, y, z, o;
                (pUnit->ToCreature())->GetHomePosition(x, y, z, o);
                (pUnit->ToCreature())->GetMotionMaster()->MovePoint(0, x, y, z);
                return true;
            }
            handler->PSendSysMessage("No unit found.");
        }
    
        return false;
    }

    static bool HandleNpcSetPoolCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK
    
        char *chrPoolId = strtok((char *)args, " ");
        if (!chrPoolId)
            return false;
        
        uint32 poolId = (uint32)atoi(chrPoolId);
        if (!poolId)
            return false;
        
        Creature *creature =  handler->GetSelectedCreature();
        if (!creature)
        {
            handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            return true;
        }
        
        WorldDatabase.PExecute("UPDATE creature SET pool_id = %u WHERE guid = %u", poolId, creature->GetSpawnId());
        creature->SetCreaturePoolId(poolId);
        creature->FindMap()->AddCreatureToPool(creature, poolId);
        handler->PSendSysMessage("Creature (guid: %u) added to pool %u",creature->GetSpawnId(),poolId);
        return true;
    }

    /** Syntax: .npc addweapon #weaponId [#slot] 
    #slot = 0 : right hand, 1 : left hand, 2 : ranged
    */
    static bool HandleNpcAddWeaponCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK
 
        Creature *pCreature =  handler->GetSelectedCreature();
 
        if(!pCreature)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            return true;
        }
 
        char* pItemID = strtok((char*)args, " ");
        char* pSlot = strtok(nullptr, " ");
 
        if (!pItemID)
            return false;
 
        WeaponSlot slot = WEAPON_SLOT_MAINHAND;
        if (pSlot)
        {
            slot = WeaponSlot(atoi(pSlot));
            if(slot > WEAPON_SLOT_RANGED)
                return false;
        }

        uint32 itemID = atoi(pItemID);
        if (itemID == 0)
        {
            pCreature->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY + slot, 0);
            return true;
        }
   
        const ItemTemplate* proto = sObjectMgr->GetItemTemplate(itemID);
        if (!proto)
        {
            handler->PSendSysMessage(LANG_ITEM_NOT_FOUND,itemID);
            return true;
        }
    /*
        handler->PSendSysMessage("Class = %u",proto->Class);
        handler->PSendSysMessage("SubClass = %u",proto->SubClass);
        handler->PSendSysMessage("InventoryType = %u",proto->InventoryType);
    */
        switch(proto->InventoryType)
        {
            case INVTYPE_SHIELD:
                slot = WEAPON_SLOT_OFFHAND;
                pCreature->SetSheath(SHEATH_STATE_MELEE);
                break;
            case INVTYPE_2HWEAPON:
            case INVTYPE_WEAPONMAINHAND:
            case INVTYPE_WEAPON:
            case INVTYPE_WEAPONOFFHAND:
                if (slot != WEAPON_SLOT_MAINHAND && slot != WEAPON_SLOT_OFFHAND)
                {
                    handler->PSendSysMessage("Given slot %u invalid.",slot);
                    return true;
                }

                pCreature->SetSheath(SHEATH_STATE_MELEE);
                break;
            case INVTYPE_THROWN:
            case INVTYPE_RANGED:
            case INVTYPE_RANGEDRIGHT:
                slot = WEAPON_SLOT_RANGED;
                pCreature->SetSheath(SHEATH_STATE_RANGED);
                break;
            default:
                handler->PSendSysMessage("Invalid object %u.",itemID);
                return true;
                break;
        }
        pCreature->SetWeapon(slot, proto->DisplayInfoID, (ItemSubclassWeapon)proto->SubClass, (InventoryType)proto->InventoryType);
 
        handler->PSendSysMessage(LANG_ITEM_ADDED_TO_SLOT,itemID,proto->Name1.c_str(),slot);
        return true;
    }

    static bool HandleNpcMassFactionIdCommand(ChatHandler* handler, char const* args)
    {
        char *entryid = strtok((char *)args, " ");
        if (!entryid)
            return false;

        char *factid = strtok(nullptr, " ");
        if (!factid)
            return false;

        Player *player = handler->GetSession()->GetPlayer();

        uint32 factionId = (uint32)atoi(factid);
        uint32 entryId = (uint32)atoi(entryid);

        if (!sFactionTemplateStore.LookupEntry(factionId)) {
            handler->PSendSysMessage(LANG_WRONG_FACTION, factionId);
            handler->SetSentErrorMessage(true);
            return false;
        }

        CellCoord p(Trinity::ComputeCellCoord(player->GetPositionX(), player->GetPositionY()));
        Cell cell(p);

        Trinity::FactionDo u_do(entryId, factionId);
        Trinity::WorldObjectWorker<Trinity::FactionDo> worker(player, u_do);
        Cell::VisitGridObjects(player, worker, MAX_SEARCHER_DISTANCE);

        return true;
    }

    static bool HandleNpcSetCombatDistanceCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

        char* cDistance = strtok((char*)args, " ");
        if (!cDistance)
            return false;
        
        float distance = (float)atof(cDistance);

        Creature *pCreature =  handler->GetSelectedCreature();
        if(!pCreature)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            return true;
        }

        if(pCreature->AI())
        {
            pCreature->SetCombatDistance(distance);
            handler->PSendSysMessage("m_combatDistance set to %f", distance);
        }

        return true;
    }

    static bool HandleNpcAllowCombatMovementCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

        char* cAllow = strtok((char*)args, " ");
        if (!cAllow)
            return false;
        
        int allow = atoi(cAllow);

        Creature *pCreature =  handler->GetSelectedCreature();
        if(!pCreature)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            return true;
        }

        if(pCreature->AI())
        {
            pCreature->AI()->SetCombatMovementAllowed(allow);
            handler->PSendSysMessage("m_allowCombatMovement set to %s", allow ? "true" : "false");
        }

        return true;
    }

    /* if no args given, tell if the selected creature is linked to a game_event. Else usage is .npc linkgameevent #eventid [#guid] (a guid may be given, overiding the selected creature)*/
    static bool HandleNpcLinkGameEventCommand(ChatHandler* handler, char const* args)
    {
        CreatureData const* data = nullptr;
        char* cEvent = strtok((char*)args, " ");
        char* cCreatureSpawnId = strtok(nullptr, " ");
        int16 event = 0;
        ObjectGuid::LowType creatureSpawnId = 0;
        bool justShowInfo = false;
        if(!cEvent) // No params given
        {
            justShowInfo = true;
        } else {
            event = atoi(cEvent);
            if(cCreatureSpawnId) // erase selected creature if guid explicitely given
                creatureSpawnId = atoi(cCreatureSpawnId);
        }

        if(!creatureSpawnId)
        {
            Creature* creature =  handler->GetSelectedCreature();
            if(creature)
                creatureSpawnId = creature->GetSpawnId();
        }

        data = sObjectMgr->GetCreatureData(creatureSpawnId);
        if(!data)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            return true;
        }

        int16 currentEventId = sGameEventMgr->GetCreatureEvent(creatureSpawnId);

        if (justShowInfo)
        {
            if(currentEventId)
                handler->PSendSysMessage("Creature (spawnID: %u) bound to event %i.", creatureSpawnId, currentEventId);
            else
                handler->PSendSysMessage("Creature (spawnID : %u) is not bound to an event.", creatureSpawnId);
        } else {
            if(currentEventId)
            {
               // handler->PSendSysMessage("La creature (guid : %u) est déjà liée à l'event %i.",creatureGUID,currentEventId);
                //PSendSysMessage("La creature est déjà liée à l'event %i.",currentEventId);
                handler->PSendSysMessage("Creature bound to event %i.",currentEventId);
                return true;
            }

            if(sGameEventMgr->AddCreatureToEvent(creatureSpawnId, event))
                handler->PSendSysMessage("Creature (spawnID: %u) is now bound to the event %i.", creatureSpawnId, event);
            else
                handler->PSendSysMessage("Error: creature (spawnID: %u) could not be linked to the event %d (event nonexistent?).", creatureSpawnId, event);
        }

        return true;
    }

    /* .npc unlinkgameevent [#guid] */
    static bool HandleNpcUnlinkGameEventCommand(ChatHandler* handler, char const* args)
    {
        Creature* creature = nullptr;
        CreatureData const* data = nullptr;
        char* cCreatureSpawnID = strtok((char*)args, " ");
        ObjectGuid::LowType creatureSpawnId = 0;

        if(cCreatureSpawnID) //Guid given
        {
            creatureSpawnId = atoi(cCreatureSpawnID);
        } else { //else, try to get selected creature
            creature =  handler->GetSelectedCreature();
            if(!creature)
            {
                handler->SendSysMessage(LANG_SELECT_CREATURE);
                handler->SetSentErrorMessage(true);
                return false;
            }           
            creatureSpawnId = creature->GetSpawnId();
        }

        data = sObjectMgr->GetCreatureData(creatureSpawnId);
        if(!data)
        {
            handler->PSendSysMessage("Creature with spawnId %u not found.", creatureSpawnId);
            return true;
        } 

        int16 currentEventId = sGameEventMgr->GetCreatureEvent(creatureSpawnId);
        if (!currentEventId)
        {
            handler->PSendSysMessage("Creature (spawnId: %u) is not linked to any event.", creatureSpawnId);
        } else {
            if(sGameEventMgr->RemoveCreatureFromEvent(creatureSpawnId))
                handler->PSendSysMessage("Creature (spawnId: %u) is not anymore linked to the event %i.", creatureSpawnId, currentEventId);
            else
                handler->PSendSysMessage("Error on removing creature (spawnId: %u) from the event %i.", creatureSpawnId, currentEventId);
        }

        return true;
    }

    static bool HandleNpcNearCommand(ChatHandler* handler, char const* args)
    {
        float distance = (!*args) ? 10 : atol(args);
        uint32 count = 0;

        Player* pl = handler->GetSession()->GetPlayer();
        QueryResult result = WorldDatabase.PQuery("SELECT c.guid, id, position_x, position_y, position_z, map, gec.event, "
            "(POW(position_x - '%f', 2) + POW(position_y - '%f', 2) + POW(position_z - '%f', 2)) AS order_ "
            "FROM creature c " 
            "LEFT JOIN game_event_creature gec ON c.guid = gec.guid "
            "WHERE map='%u' AND (POW(position_x - '%f', 2) + POW(position_y - '%f', 2) + POW(position_z - '%f', 2)) <= '%f' " 
            "ORDER BY order_",
            pl->GetPositionX(), pl->GetPositionY(), pl->GetPositionZ(),
            pl->GetMapId(),pl->GetPositionX(), pl->GetPositionY(), pl->GetPositionZ(),distance*distance);

        if (result)
        {
            do
            {
                Field *fields = result->Fetch();
                ObjectGuid::LowType guid = fields[0].GetUInt32();
                uint32 entry = fields[1].GetUInt32();
                float x = fields[2].GetFloat();
                float y = fields[3].GetFloat();
                float z = fields[4].GetFloat();
                int mapid = fields[5].GetUInt16();
                int linked_event = fields[6].GetInt16();

                CreatureTemplate const * cInfo = sObjectMgr->GetCreatureTemplate(entry);
                if(!cInfo)
                    continue;

                handler->PSendSysMessage("guid %u, id %u - |cffffffff|Hcreature:%d|h[%s X:%f Y:%f Z:%f MapId:%d]|h|r", guid, entry, guid, cInfo->Name.c_str(), x, y, z, mapid);
                if (linked_event)
                    handler->PSendSysMessage("Linked event: %i", linked_event);

                ++count;
            } while (result->NextRow());
        }

        handler->PSendSysMessage("Found near creatures (distance %f): %u",distance,count);
        return true;
    }

    static bool HandleNpcNameCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

        uint32 id = atoi(args);

        Creature* pCreature =  handler->GetSelectedCreature();
        if(!pCreature)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            return true;
        }

        if (CreatureTemplate const *cinfo = sObjectMgr->GetCreatureTemplate(id))
            pCreature->SetUInt32Value(OBJECT_FIELD_ENTRY, id);
        else
            handler->PSendSysMessage("Creature template with id %u not found", id);

        return true;
    }

    /* Syntax : .npc path type [PathType] 
    No arguments means print current type. This DOES NOT update values in db, use .path type to do so.

    Possible types :
    0 - WP_PATH_TYPE_LOOP
    1 - WP_PATH_TYPE_ONCE
    2 - WP_PATH_TYPE_ROUND_TRIP
    */
    static bool HandleNpcPathTypeCommand(ChatHandler* handler, char const* args)
    {
        Creature* target =  handler->GetSelectedCreature();
        if(!target)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            return true;
        }

        if(target->GetMotionMaster()->GetCurrentMovementGeneratorType() != WAYPOINT_MOTION_TYPE)
        {
            handler->SendSysMessage("Creature is not using waypoint movement generator.");
            return true;
        }

        MovementGenerator* baseGenerator = target->GetMotionMaster()->GetCurrentMovementGenerator();
        WaypointMovementGenerator<Creature>* movGenerator = static_cast<WaypointMovementGenerator<Creature>*>(baseGenerator);
        if(!movGenerator)
        {
            handler->SendSysMessage("Could not get movement generator.");
            return true;
        }

        if(!*args)
        { //getter
            WaypointPathType type = movGenerator->GetPathType();
            std::string pathTypeStr = GetWaypointPathTypeName(type);
            handler->PSendSysMessage("Creature waypoint movement type : %s (%u).", pathTypeStr.c_str(), type);
        } else 
        { //setter
            uint32 type = (uint32)atoi(args);
            bool ok = movGenerator->SetPathType(WaypointPathType(type));
            if(!ok)
            {
                handler->PSendSysMessage("Wrong type given : %u.", type);
                return false;
            }
            std::string pathTypeStr = GetWaypointPathTypeName(WaypointPathType(type));
            handler->PSendSysMessage("Target creature path type set to %s (%u).", pathTypeStr.c_str(), type);
        }
        return true;
    }

    /* Syntax : .npc path direction [PathDirection]
    No arguments means print current direction. This DOES NOT update values in db, use .path direction to do so.

    Possible directions :
    0 - WP_PATH_DIRECTION_NORMAL
    1 - WP_PATH_DIRECTION_REVERSE
    2 - WP_PATH_DIRECTION_RANDOM
    */
    static bool HandleNpcPathDirectionCommand(ChatHandler* handler, char const* args)
    {
        Creature* target =  handler->GetSelectedCreature();
        if(!target)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            return true;
        }

        if(target->GetMotionMaster()->GetCurrentMovementGeneratorType() != WAYPOINT_MOTION_TYPE)
        {
            handler->SendSysMessage("Creature is not using waypoint movement generator.");
            return true;
        }

        MovementGenerator* baseGenerator = target->GetMotionMaster()->GetCurrentMovementGenerator();
        WaypointMovementGenerator<Creature>* movGenerator = static_cast<WaypointMovementGenerator<Creature>*>(baseGenerator);
        if (!movGenerator)
        {
            handler->SendSysMessage("Could not get movement generator.");
            return true;
        }

        if(!*args)
        { //getter
            WaypointPathDirection dir = movGenerator->GetPathDirection();
            std::string pathDirStr = GetWaypointPathDirectionName(WaypointPathDirection(dir));
            handler->PSendSysMessage("Creature waypoint movement direction : %s (%u).", pathDirStr.c_str(), dir);
        } else 
        { //setter
            uint32 dir = (uint32)atoi(args);
            bool ok = movGenerator->SetDirection(WaypointPathDirection(dir));
            if(!ok)
            {
                handler->PSendSysMessage("Wrong direction given : %u.", dir);
                return false;
            }
            std::string pathDirStr = GetWaypointPathDirectionName(WaypointPathDirection(dir));
            handler->PSendSysMessage("Target creature path direction set to %s (%u).", pathDirStr.c_str(), dir);
        }
        return true;
    }

    /* Syntax : .npc path currentid */
    static bool HandleNpcPathCurrentIdCommand(ChatHandler* handler, char const* args)
    {
        Creature* target =  handler->GetSelectedCreature();
        if(!target)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            return true;
        }

        uint32 pathId = target->GetWaypointPath();
        handler->PSendSysMessage("Target creature current path id : %u.", pathId);

        return true;
    }

    //set spawn dist of creature
    static bool HandleNpcSpawnDistCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

        float option = atof((char*)args);
        if (option < 0.0f)
        {
            handler->SendSysMessage(LANG_BAD_VALUE);
            return false;
        }

        MovementGeneratorType mtype = IDLE_MOTION_TYPE;
        if (option >0.0f)
            mtype = RANDOM_MOTION_TYPE;

        Creature* pCreature = handler->GetSelectedCreature();
        if (!pCreature)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }
        ObjectGuid::LowType u_guidlow = pCreature->GetSpawnId();

        //sun: also update creature data, new spawns also need to use the correct type (updating the current creature is not enough in case of non compat respawn)
        if (CreatureData const* _data = sObjectMgr->GetCreatureData(pCreature->GetSpawnId()))
            if (CreatureData* data = const_cast<CreatureData*>(_data))
            {
                data->movementType = mtype;
                data->spawndist = option;
            }

        pCreature->SetRespawnRadius((float)option);
        pCreature->SetDefaultMovementType(mtype);
        pCreature->GetMotionMaster()->Initialize();
        pCreature->Respawn(true);

        WorldDatabase.PExecute("UPDATE creature SET spawndist=%f, MovementType=%i WHERE guid=%u", option, mtype, u_guidlow);
        handler->PSendSysMessage(LANG_COMMAND_SPAWNDIST, option);
        return true;
    }

    static bool HandleNpcSpawnTimeCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

        char* stime = strtok((char*)args, " ");

        if (!stime)
            return false;

        int i_stime = atoi((char*)stime);

        if (i_stime < 0)
        {
            handler->SendSysMessage(LANG_BAD_VALUE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Creature *pCreature =  handler->GetSelectedCreature();
        ObjectGuid::LowType u_guidlow = 0;

        if (pCreature)
            u_guidlow = pCreature->GetSpawnId();
        else
            return false;

        WorldDatabase.PExecute("UPDATE creature SET spawntimesecs=%i WHERE guid=%u",i_stime,u_guidlow);
        pCreature->SetRespawnDelay((uint32)i_stime);
        handler->PSendSysMessage(LANG_COMMAND_SPAWNTIME,i_stime);

        return true;
    }

    //set faction of creature
    static bool HandleNpcFactionIdCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

        uint32 factionId = (uint32) atoi((char*)args);

        if (!sFactionTemplateStore.LookupEntry(factionId))
        {
            handler->PSendSysMessage(LANG_WRONG_FACTION, factionId);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Creature* pCreature =  handler->GetSelectedCreature();

        if(!pCreature)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        pCreature->SetFaction(factionId);

        // faction is set in creature_template - not inside creature

        // update in memory
        if(CreatureTemplate const *cinfo = pCreature->GetCreatureTemplate())
        {
            const_cast<CreatureTemplate*>(cinfo)->faction = factionId;
        }

        // and DB
        //WorldDatabase.PExecute("UPDATE creature_template SET faction = '%u' WHERE entry = '%u'", factionId, pCreature->GetEntry());

        return true;
    }

    //add item in vendorlist
    static bool HandleNpcAddVendorItemCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

        char* pitem  = handler->extractKeyFromLink((char*)args,"Hitem");
        if (!pitem)
        {
            handler->SendSysMessage(LANG_COMMAND_NEEDITEMSEND);
            handler->SetSentErrorMessage(true);
            return false;
        }

        uint32 itemId = atol(pitem);

        char* fmaxcount = strtok(nullptr, " ");                    //add maxcount, default: 0
        uint32 maxcount = 0;
        if (fmaxcount)
            maxcount = atol(fmaxcount);

        char* fincrtime = strtok(nullptr, " ");                    //add incrtime, default: 0
        uint32 incrtime = 0;
        if (fincrtime)
            incrtime = atol(fincrtime);

        char* fextendedcost = strtok(nullptr, " ");                //add ExtendedCost, default: 0
        uint32 extendedcost = fextendedcost ? atol(fextendedcost) : 0;

        Creature* vendor =  handler->GetSelectedCreature();

        uint32 vendor_entry = vendor ? vendor->GetEntry() : 0;

        ItemTemplate const* pProto = sObjectMgr->GetItemTemplate(itemId);
        if(!pProto)
        {
            handler->PSendSysMessage("Invalid id");
            return true;
        }

        if(!sObjectMgr->IsVendorItemValid(vendor_entry,pProto,maxcount,incrtime,extendedcost,handler->GetSession()->GetPlayer()))
        {
            handler->SetSentErrorMessage(true);
            return false;
        }

        sObjectMgr->AddVendorItem(vendor_entry,pProto,maxcount,incrtime,extendedcost);


        handler->PSendSysMessage(LANG_ITEM_ADDED_TO_LIST,itemId,pProto->Name1.c_str(),maxcount,incrtime,extendedcost);
        return true;
    }
};

void AddSC_npc_commandscript()
{
    new npc_commandscript();
}
