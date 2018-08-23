#include "Chat.h"
#include "Language.h"
#include "WaypointMovementGenerator.h"
#include "WaypointDefines.h"


class wp_commandscript : public CommandScript
{
public:
    wp_commandscript() : CommandScript("wp_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> wpCommandTable =
        {
            { "show",           SEC_GAMEMASTER3,     false, &HandleWpShowCommand,               "" },
            { "addwp",          SEC_GAMEMASTER3,     false, &HandleWpAddCommand,                "" },
            { "load",           SEC_GAMEMASTER3,     false, &HandleWpLoadPathCommand,           "" },
            { "modify",         SEC_GAMEMASTER3,     false, &HandleWpModifyCommand,             "" },
            { "event",          SEC_GAMEMASTER3,     false, &HandleWpEventCommand,              "" },
            { "unload",         SEC_GAMEMASTER3,     false, &HandleWpUnLoadPathCommand,         "" },
            { "direction",      SEC_GAMEMASTER3,     false, &HandleWpChangePathDirectionCommand,"" },
            { "teleport",       SEC_GAMEMASTER3,     false, &HandleWpTeleportToPathCommand,     "" },
            { "type"     ,      SEC_GAMEMASTER3,     false, &HandleWpChangePathTypeCommand,     "" },
        };
        static std::vector<ChatCommand> commandTable =
        {
            { "path",           SEC_GAMEMASTER2,  false, nullptr,                               "", wpCommandTable },
        };
        return commandTable;
    }

    static bool HandleWpShowCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

        // first arg: on, off, first, last
        char* show_str = strtok((char*)args, " ");
        if (!show_str)
        {
            return false;
        }
        // second arg: GUID (optional, if a creature is selected)
        char* guid_str = strtok((char*)nullptr, " ");

        uint32 pathid = 0;
        Creature* target =  handler->GetSelectedCreature();

        // Did player provide a PathID?

        if (!guid_str)
        {
            // No PathID provided
            // -> Player must have selected a creature
            if(!target)
            {
                handler->SendSysMessage(LANG_SELECT_CREATURE);
                handler->SetSentErrorMessage(true);
                return false;
            }

            pathid = target->GetWaypointPath();
        }

        else
        {
            // PathID provided
            // Warn if player also selected a creature
            // -> Creature selection is ignored <-
            if(target)
            {
                handler->SendSysMessage(LANG_WAYPOINT_CREATSELECTED);
            }

            pathid = atoi((char*)guid_str);
        }

        std::string show = show_str;
        uint32 Maxpoint;

        //PSendSysMessage("wpshow - show: %s", show);

        // Show info for the selected waypoint
        if (show == "info") {
            // Check if the user did specify a visual waypoint
            if (target->GetEntry() != VISUAL_WAYPOINT) {
                handler->PSendSysMessage(LANG_WAYPOINT_VP_SELECT);
                handler->SetSentErrorMessage(true);
                return false;
            }
            QueryResult result = WorldDatabase.PQuery("SELECT id, point, delay, move_type, action, action_chance FROM waypoint_data WHERE wpguid = %u", target->GetGUID().GetCounter());

            if (!result) {
                handler->SendSysMessage(LANG_WAYPOINT_NOTFOUNDDBPROBLEM);
                return true;
            }

            handler->SendSysMessage("|cff00ffffDEBUG: wp show info:|r");

            do {
                Field *fields = result->Fetch();
                pathid = fields[0].GetUInt32();
                uint32 point = fields[1].GetUInt32();
                uint32 delay = fields[2].GetUInt32();
                uint32 flag = fields[3].GetUInt32();
                uint32 ev_id = fields[4].GetUInt32();
                uint32 ev_chance = fields[5].GetUInt32();

                handler->PSendSysMessage("|cff00ff00Show info: for current point: |r|cff00ffff%u|r|cff00ff00, Path ID: |r|cff00ffff%u|r", point, pathid);
                handler->PSendSysMessage("|cff00ff00Show info: delay: |r|cff00ffff%u|r", delay);
                handler->PSendSysMessage("|cff00ff00Show info: Move flag: |r|cff00ffff%u|r", flag);
                handler->PSendSysMessage("|cff00ff00Show info: Waypoint event: |r|cff00ffff%u|r", ev_id);
                handler->PSendSysMessage("|cff00ff00Show info: Event chance: |r|cff00ffff%u|r", ev_chance);
            } while (result->NextRow());

            // Cleanup memory
            return true;
        }

        if(show == "on")
        {
            QueryResult result = WorldDatabase.PQuery( "SELECT point, position_x,position_y,position_z FROM waypoint_data WHERE id = '%u'", pathid);

            if(!result)
            {
                handler->SendSysMessage("|cffff33ffPath no found.|r");
                handler->SetSentErrorMessage(true);
                return false;
            }

            handler->PSendSysMessage("|cff00ff00DEBUG: wp on, PathID: |cff00ffff%u|r", pathid);

            // Delete all visuals for this NPC
            QueryResult result2 = WorldDatabase.PQuery( "SELECT wpguid FROM waypoint_data WHERE id = '%u' and wpguid <> 0", pathid);

            if(result2)
            {
                bool hasError = false;
                do
                {
                    Field *fields = result2->Fetch();
                    uint32 wpguid = fields[0].GetUInt32();
                    Creature* pCreature = ObjectAccessor::GetCreature(*handler->GetSession()->GetPlayer(), ObjectGuid(HighGuid::Unit, VISUAL_WAYPOINT, wpguid));

                    if(!pCreature)
                    {
                        handler->PSendSysMessage(LANG_WAYPOINT_NOTREMOVED, wpguid);
                        hasError = true;
                        WorldDatabase.PExecute("DELETE FROM creature WHERE guid = '%u'", wpguid);
                    }
                    else
                    {
                        pCreature->CombatStop();
                        pCreature->DeleteFromDB();
                        pCreature->AddObjectToRemoveList();
                    }

                }while( result2->NextRow() );

                if( hasError )
                {
                    handler->PSendSysMessage(LANG_WAYPOINT_TOOFAR1);
                    handler->PSendSysMessage(LANG_WAYPOINT_TOOFAR2);
                    handler->PSendSysMessage(LANG_WAYPOINT_TOOFAR3);
                }
            }

            do
            {
                Field *fields = result->Fetch();
               // uint32 point    = fields[0].GetUInt32();
                float x         = fields[1].GetFloat();
                float y         = fields[2].GetFloat();
                float z         = fields[3].GetFloat();

                uint32 id = VISUAL_WAYPOINT;

                Player *chr = handler->GetSession()->GetPlayer();
                /*Map *map = chr->GetMap();
                float o = chr->GetOrientation();

                Creature* wpCreature = new Creature;
                if (!wpCreature->Create(sObjectMgr->GenerateLowGuid(HighGuid::Unit, true), map, id, 0))
                {
                    handler->PSendSysMessage(LANG_WAYPOINT_VP_NOTCREATED, id);
                    delete wpCreature;
                    return false;
                }

                wpCreature->Relocate(x, y, z, o);

                if(!wpCreature->IsPositionValid())
                {
                    TC_LOG_ERROR("command","ERROR: Creature (guidlow %d, entry %d) not created. Suggested coordinates isn't valid (X: %f Y: %f)",wpCreature->GetGUID().GetCounter(),wpCreature->GetEntry(),wpCreature->GetPositionX(),wpCreature->GetPositionY());
                    delete wpCreature;
                    return false;
                }

                // set "wpguid" column to the visual waypoint
                WorldDatabase.PExecute("UPDATE waypoint_data SET wpguid = '%u' WHERE id = '%u' and point = '%u'", wpCreature->GetGUID().GetCounter(), pathid, point);

                wpCreature->SaveToDB(map->GetId(), (1 << map->GetSpawnMode()));
                wpCreature->LoadFromDB(wpCreature->GetSpawnId(),map);
                map->Add(wpCreature);
                */
                chr->SummonCreature(id,x,y,z,0,TEMPSUMMON_CORPSE_TIMED_DESPAWN,60000);
            }
            while( result->NextRow() );

            handler->SendSysMessage("|cff00ff00Showing the current creature's path.|r");

            return true;
        }

        if(show == "first")
        {
            handler->PSendSysMessage("|cff00ff00DEBUG: wp first, GUID: %u|r", pathid);

            QueryResult result = WorldDatabase.PQuery( "SELECT position_x,position_y,position_z FROM waypoint_data WHERE point='1' AND id = '%u'",pathid);
            if(!result)
            {
                handler->PSendSysMessage(LANG_WAYPOINT_NOTFOUND, pathid);
                handler->SetSentErrorMessage(true);
                return false;
            }

            Field *fields = result->Fetch();
            float x         = fields[0].GetFloat();
            float y         = fields[1].GetFloat();
            float z         = fields[2].GetFloat();
            uint32 id = VISUAL_WAYPOINT;

            Player *chr = handler->GetSession()->GetPlayer();
            //float o = chr->GetOrientation();
            /*Map *map = chr->GetMap();
        
            Creature* pCreature = new Creature;
            if (!pCreature->Create(sObjectMgr->GenerateLowGuid(HighGuid::Unit,true),map, id, 0))
            {
                handler->PSendSysMessage(LANG_WAYPOINT_VP_NOTCREATED, id);
                delete pCreature;
                return false;
            }

            pCreature->Relocate(x, y, z, o);

            if(!pCreature->IsPositionValid())
            {
                TC_LOG_ERROR("command","ERROR: Creature (guidlow %d, entry %d) not created. Suggested coordinates isn't valid (X: %f Y: %f)",pCreature->GetGUID().GetCounter(),pCreature->GetEntry(),pCreature->GetPositionX(),pCreature->GetPositionY());
                delete pCreature;
                return false;
            }

            pCreature->SaveToDB(map->GetId(), (1 << map->GetSpawnMode()));
            pCreature->LoadFromDB(pCreature->GetSpawnId(), map);
            map->Add(pCreature);

            if(target)
            {
                pCreature->SetDisplayId(target->GetDisplayId());
                pCreature->SetFloatValue(OBJECT_FIELD_SCALE_X, 0.5);
            }
            */
            chr->SummonCreature(id,x,y,z,0,TEMPSUMMON_CORPSE_DESPAWN,10);
            return true;
        }

        if(show == "last")
        {
            handler->PSendSysMessage("|cff00ff00DEBUG: wp last, PathID: |r|cff00ffff%u|r", pathid);

            QueryResult result = WorldDatabase.PQuery( "SELECT MAX(point) FROM waypoint_data WHERE id = '%u'",pathid);
            if( result )
            {
                Maxpoint = (*result)[0].GetUInt32();
            }
            else
                Maxpoint = 0;

            result = WorldDatabase.PQuery( "SELECT position_x,position_y,position_z FROM waypoint_data WHERE point ='%u' AND id = '%u'",Maxpoint, pathid);
            if(!result)
            {
                handler->PSendSysMessage(LANG_WAYPOINT_NOTFOUNDLAST, pathid);
                handler->SetSentErrorMessage(true);
                return false;
            }
            Field *fields = result->Fetch();
            float x         = fields[0].GetFloat();
            float y         = fields[1].GetFloat();
            float z         = fields[2].GetFloat();
            uint32 id = VISUAL_WAYPOINT;

            Player *chr = handler->GetSession()->GetPlayer();
            //float o = chr->GetOrientation();
            /*Map *map = chr->GetMap();
        
            Creature* pCreature = new Creature;
            if (!pCreature->Create(sObjectMgr->GenerateLowGuid(HighGuid::Unit,true), map, id, 0))
            {
                handler->PSendSysMessage(LANG_WAYPOINT_NOTCREATED, id);
                delete pCreature;
                return false;
            }

            pCreature->Relocate(x, y, z, o);

            if(!pCreature->IsPositionValid())
            {
                TC_LOG_ERROR("command","ERROR: Creature (guidlow %d, entry %d) not created. Suggested coordinates isn't valid (X: %f Y: %f)",pCreature->GetGUID().GetCounter(),pCreature->GetEntry(),pCreature->GetPositionX(),pCreature->GetPositionY());
                delete pCreature;
                return false;
            }

            pCreature->SaveToDB(map->GetId(), (1 << map->GetSpawnMode()));
            pCreature->LoadFromDB(pCreature->GetSpawnId(), map);
            map->Add(pCreature);

            if(target)
            {
                pCreature->SetDisplayId(target->GetDisplayId());
                pCreature->SetFloatValue(OBJECT_FIELD_SCALE_X, 0.5);
            }
            */
            chr->SummonCreature(id,x,y,z,0,TEMPSUMMON_CORPSE_DESPAWN,10);
            return true;
        }

        if(show == "off")
        {
            QueryResult result = WorldDatabase.PQuery("SELECT guid FROM creature WHERE id = '%u'", 1);
            if(!result)
            {
                handler->SendSysMessage(LANG_WAYPOINT_VP_NOTFOUND);
                handler->SetSentErrorMessage(true);
                return false;
            }
            bool hasError = false;
            do
            {
                Field *fields = result->Fetch();
                uint32 guid = fields[0].GetUInt32();
                Creature* pCreature = ObjectAccessor::GetCreature(*handler->GetSession()->GetPlayer(), ObjectGuid(HighGuid::Unit ,VISUAL_WAYPOINT, guid));

                if(!pCreature)
                {
                    handler->PSendSysMessage(LANG_WAYPOINT_NOTREMOVED, guid);
                    hasError = true;
                    WorldDatabase.PExecute("DELETE FROM creature WHERE guid = '%u'", guid);
                }
                else
                {
                    pCreature->CombatStop();
                    pCreature->DeleteFromDB();
                    pCreature->AddObjectToRemoveList();
                }
            }while(result->NextRow());
            // set "wpguid" column to "empty" - no visual waypoint spawned
            WorldDatabase.PExecute("UPDATE waypoint_data SET wpguid = '0'");
            //WorldDatabase.PExecute("UPDATE creature_movement SET wpguid = '0' WHERE wpguid <> '0'");

            if( hasError )
            {
                handler->PSendSysMessage(LANG_WAYPOINT_TOOFAR1);
                handler->PSendSysMessage(LANG_WAYPOINT_TOOFAR2);
                handler->PSendSysMessage(LANG_WAYPOINT_TOOFAR3);
            }

            handler->SendSysMessage(LANG_WAYPOINT_VP_ALLREMOVED);

            return true;
        }

        handler->PSendSysMessage("|cffff33ffDEBUG: wpshow - no valid command found|r");

        return true;
    }

    static bool HandleWpUnLoadPathCommand(ChatHandler* handler, char const* args)
    {
        Creature* target =  handler->GetSelectedCreature();

        if(!target)
        {
            handler->PSendSysMessage("%s%s|r", "|cff33ffff", "You must select target.");
            return true;
        }

        uint32 guidlow = target->GetSpawnId();
        if(!guidlow)
        {
            handler->PSendSysMessage("%s%s|r", "|cff33ffff", "Creature is not permanent.");
            return true;
        }

        if(target->GetCreatureAddon())
        {
            if(target->GetCreatureAddon()->path_id != 0)
            {
                WorldDatabase.PExecute("UPDATE creature_addon SET path_id = 0 WHERE guid = %u", guidlow);
                WorldDatabase.PExecute("UPDATE creature SET MovementType = '%u' WHERE guid = '%u'", IDLE_MOTION_TYPE, guidlow);
                target->LoadPath(0);
                target->SetDefaultMovementType(IDLE_MOTION_TYPE);
                target->GetMotionMaster()->MoveTargetedHome();
                target->GetMotionMaster()->Initialize();
                target->Say("Path unloaded.",LANG_UNIVERSAL,nullptr);
                return true;
            }
            handler->PSendSysMessage("%s%s|r", "|cffff33ff", "Target have no loaded path. (1)");
            return true;
        }
        handler->PSendSysMessage("%s%s|r", "|cffff33ff", "Target have no loaded path. (2)");
        return true;
    }

    static bool HandleWpEventCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

        char* show_str = strtok((char*)args, " ");

        std::string show = show_str;

        // Check
        if( (show != "add") && (show != "mod") && (show != "del") && (show != "listid")) return false;


        if(show == "add")
        {
        uint32 id = 0;
        char* arg_id = strtok(nullptr, " ");

        if(arg_id)
            id = atoi(arg_id);

        if(id)
        {
            QueryResult result = WorldDatabase.PQuery( "SELECT `id` FROM waypoint_scripts WHERE guid = %u", id);

            if( !result )
            {
                WorldDatabase.PExecute("INSERT INTO waypoint_scripts(guid)VALUES(%u)", id);
                handler->PSendSysMessage("%s%s%u|r", "|cff00ff00", "Wp Event: New waypoint event added: ", id);
            }
            else
            {
                handler->PSendSysMessage("|cff00ff00Wp Event: you have choosed an existing waypoint script guid: %u|r", id);
            }
        }
        else
        {
            if(QueryResult result = WorldDatabase.PQuery( "SELECT MAX(guid) FROM waypoint_scripts"))
            {
                id = result->Fetch()->GetUInt32();
                WorldDatabase.PExecute("INSERT INTO waypoint_scripts(guid)VALUES(%u)", id+1);
                handler->PSendSysMessage("%s%s%u|r", "|cff00ff00","Wp Event: New waypoint event added: |r|cff00ffff", id+1);
            }
        }

        return true;
        }


        if(show == "listid")
        {
            uint32 id;
            char* arg_id = strtok(nullptr, " ");

            if(!arg_id)
            {
                handler->PSendSysMessage("%s%s|r", "|cff33ffff","Wp Event: you must provide waypoint script id.");
                return true;
            }

            id = atoi(arg_id);

            uint32 a2, a3, a4, a5, a6;
            float a8, a9, a10, a11;
            char const* a7;

            QueryResult result = WorldDatabase.PQuery( "SELECT `guid`, `delay`, `command`, `datalong`, `datalong2`, `dataint`, `x`, `y`, `z`, `o` FROM waypoint_scripts WHERE id = %u", id);

            if( !result )
            {
                handler->PSendSysMessage("%s%s%u|r", "|cff33ffff", "Wp Event: no waypoint scripts found on id: ", id);
                return true;
            }

            Field *fields;

            do
            {
                fields = result->Fetch();
                a2 = fields[0].GetUInt32();
                a3 = fields[1].GetUInt32();
                a4 = fields[2].GetUInt32();
                a5 = fields[3].GetUInt32();
                a6 = fields[4].GetUInt32();
                a7 = fields[5].GetCString();
                a8 = fields[6].GetFloat();
                a9 = fields[7].GetFloat();
                a10 = fields[8].GetFloat();
                a11 = fields[9].GetFloat();

                handler->PSendSysMessage("|cffff33ffid:|r|cff00ffff %u|r|cff00ff00, guid: |r|cff00ffff%u|r|cff00ff00, delay: |r|cff00ffff%u|r|cff00ff00, command: |r|cff00ffff%u|r|cff00ff00, datalong: |r|cff00ffff%u|r|cff00ff00, datalong2: |r|cff00ffff%u|r|cff00ff00, datatext: |r|cff00ffff%s|r|cff00ff00, posx: |r|cff00ffff%f|r|cff00ff00, posy: |r|cff00ffff%f|r|cff00ff00, posz: |r|cff00ffff%f|r|cff00ff00, orientation: |r|cff00ffff%f|r", id, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
            } while(result->NextRow());
        }

        if(show == "del")
        {
            char* arg_id = strtok(nullptr, " ");
            uint32 id = atoi(arg_id);

            QueryResult result = WorldDatabase.PQuery( "SELECT `guid` FROM waypoint_scripts WHERE guid = %u", id);

            if( result )
            {

               WorldDatabase.PExecute("DELETE FROM waypoint_scripts WHERE guid = %u", id);
               handler->PSendSysMessage("%s%s%u|r","|cff00ff00","Wp Event: waypoint script removed: ", id);
            }
            else
                handler->PSendSysMessage("|cffff33ffWp Event: ERROR: you have selected a non existing script: %u|r", id);

            return true;
        }


        if(show == "mod")
        {
            char* arg_1 = strtok(nullptr," ");

            if(!arg_1)
            {
                handler->SendSysMessage("|cffff33ffERROR: waypoint script guid not present.|r");
                return true;
            }

            uint32 id = atoi(arg_1);

            if(!id)
            {
                handler->SendSysMessage("|cffff33ffERROR: no valid waypoint script id not present.|r");
                return true;
            }

            char* arg_2 = strtok(nullptr," ");

            if(!arg_2)
            {   
                handler->SendSysMessage("|cffff33ffERROR: no argument present.|r");
                return true;
            }

            std::string arg_string  = arg_2;

            if( (arg_string != "setid") && (arg_string != "delay") && (arg_string != "command")
            && (arg_string != "datalong") && (arg_string != "datalong2") && (arg_string != "dataint") && (arg_string != "posx")
            && (arg_string != "posy") && (arg_string != "posz") && (arg_string != "orientation") )
            { 
                handler->SendSysMessage("|cffff33ffERROR: no valid argument present.|r");
                return true;
            }

            char* arg_3;
            std::string arg_str_2 = arg_2;
            arg_3 = strtok(nullptr," ");

            if(!arg_3)
            {
                handler->SendSysMessage("|cffff33ffERROR: no additional argument present.|r");
                return true;
            }

            float coord;

            if(arg_str_2 == "setid")
            {
                uint32 newid = atoi(arg_3);
                handler->PSendSysMessage("%s%s|r|cff00ffff%u|r|cff00ff00%s|r|cff00ffff%u|r","|cff00ff00","Wp Event: waypoint script guid: ", newid," id changed: ", id);
                WorldDatabase.PExecute("UPDATE waypoint_scripts SET id='%u' WHERE guid='%u'",newid, id); 
                return true;
            }
            else
            {
                QueryResult result = WorldDatabase.PQuery("SELECT id FROM waypoint_scripts WHERE guid='%u'",id);

                if(!result)
                {
                    handler->SendSysMessage("|cffff33ffERROR: you have selected a nonexistent waypoint script guid.|r");
                    return true;
                }

                if(arg_str_2 == "posx")
                {
                    coord = atof(arg_3);
                    WorldDatabase.PExecute("UPDATE waypoint_scripts SET x='%f' WHERE guid='%u'",
                        coord, id);
                    handler->PSendSysMessage("|cff00ff00Waypoint script:|r|cff00ffff %u|r|cff00ff00 position_x updated.|r", id);
                    return true;
                }else if(arg_str_2 == "posy")
                {
                    coord = atof(arg_3);
                    WorldDatabase.PExecute("UPDATE waypoint_scripts SET y='%f' WHERE guid='%u'",
                        coord, id);
                    handler->PSendSysMessage("|cff00ff00Waypoint script: %u position_y updated.|r", id);
                    return true;
                } else if(arg_str_2 == "posz")
                {
                    coord = atof(arg_3);
                    WorldDatabase.PExecute("UPDATE waypoint_scripts SET z='%f' WHERE guid='%u'",
                        coord, id);
                    handler->PSendSysMessage("|cff00ff00Waypoint script: |r|cff00ffff%u|r|cff00ff00 position_z updated.|r", id);
                    return true;
                } else if(arg_str_2 == "orientation")
                {
                    coord = atof(arg_3);
                    WorldDatabase.PExecute("UPDATE waypoint_scripts SET o='%f' WHERE guid='%u'",
                        coord, id);
                    handler->PSendSysMessage("|cff00ff00Waypoint script: |r|cff00ffff%u|r|cff00ff00 orientation updated.|r", id);
                    return true;
                } else if(arg_str_2 == "dataint")
                {
                        WorldDatabase.PExecute("UPDATE waypoint_scripts SET %s='%u' WHERE guid='%u'",
                        arg_2, atoi(arg_3), id);
                        handler->PSendSysMessage("|cff00ff00Waypoint script: |r|cff00ffff%u|r|cff00ff00 dataint updated.|r", id);
                        return true;
                }else
                {
                        std::string arg_str_3 = arg_3;
                        WorldDatabase.EscapeString(arg_str_3);
                        WorldDatabase.PExecute("UPDATE waypoint_scripts SET %s='%s' WHERE guid='%u'",
                        arg_2, arg_str_3.c_str(), id);
                }
            }
            handler->PSendSysMessage("%s%s|r|cff00ffff%u:|r|cff00ff00 %s %s|r","|cff00ff00","Waypoint script:", id, arg_2,"updated.");
        }
        return true;
    }


    static bool HandleWpModifyCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

        // first arg: add del text emote spell waittime move
        char* show_str = strtok((char*)args, " ");
        if (!show_str)
        {
            return false;
        }

        std::string show = show_str;
        // Check
        // Remember: "show" must also be the name of a column!
        if( (show != "delay") && (show != "action") && (show != "action_chance")
            && (show != "move_type") && (show != "del") && (show != "move") && (show != "wpadd")
           )
        {
            return false;
        }

        // Next arg is: <PATHID> <WPNUM> <ARGUMENT>
        char* arg_str = nullptr;

        // Did user provide a GUID
        // or did the user select a creature?
        // -> variable lowguid is filled with the GUID of the NPC
        uint32 pathid = 0;
        uint32 point = 0;
        uint32 wpGuid = 0;
        Creature* target =  handler->GetSelectedCreature();

        if(!target || target->GetEntry() != VISUAL_WAYPOINT)
        {
            handler->SendSysMessage("|cffff33ffERROR: you must select a waypoint.|r");
            return false;
        }

        // The visual waypoint
        Creature* wpCreature = nullptr;
        wpGuid = target->GetGUID().GetCounter();

        // Did the user select a visual spawnpoint?
        if(wpGuid)
            wpCreature = ObjectAccessor::GetCreature(*handler->GetSession()->GetPlayer(), ObjectGuid(HighGuid::Unit, VISUAL_WAYPOINT, wpGuid));
        // attempt check creature existence by DB data
        else
        {
            handler->PSendSysMessage(LANG_WAYPOINT_CREATNOTFOUND, wpGuid);
            return false;
        }
        // User did select a visual waypoint?
        // Check the creature
        if (wpCreature->GetEntry() == VISUAL_WAYPOINT )
        {
            QueryResult result =
            WorldDatabase.PQuery( "SELECT id, point FROM waypoint_data WHERE wpguid = %u", wpGuid);

            if(!result)
            {
                handler->PSendSysMessage(LANG_WAYPOINT_NOTFOUNDSEARCH, target->GetGUID().GetCounter());
                // Select waypoint number from database
                // Since we compare float values, we have to deal with
                // some difficulties.
                // Here we search for all waypoints that only differ in one from 1 thousand
                // (0.001) - There is no other way to compare C++ floats with mySQL floats
                // See also: http://dev.mysql.com/doc/refman/5.0/en/problems-with-float.html
                const char* maxDIFF = "0.01";
                result = WorldDatabase.PQuery( "SELECT id, point FROM waypoint_data WHERE (abs(position_x - %f) <= %s ) and (abs(position_y - %f) <= %s ) and (abs(position_z - %f) <= %s )",
                wpCreature->GetPositionX(), maxDIFF, wpCreature->GetPositionY(), maxDIFF, wpCreature->GetPositionZ(), maxDIFF);
                if(!result)
                {
                        handler->PSendSysMessage(LANG_WAYPOINT_NOTFOUNDDBPROBLEM, wpGuid);
                        return true;
                }
            }

            do
            {
                Field *fields = result->Fetch();
                pathid = fields[0].GetUInt32();
                point  = fields[1].GetUInt32();
            }
            while( result->NextRow() );

            // We have the waypoint number and the GUID of the "master npc"
            // Text is enclosed in "<>", all other arguments not
            arg_str = strtok((char*)nullptr, " ");
        }

        // Check for argument
        if(show != "del" && show != "move" && arg_str == nullptr)
        {
            handler->PSendSysMessage(LANG_WAYPOINT_ARGUMENTREQ, show_str);
            return false;
        }

        if(show == "del" && target)
        {
            handler->PSendSysMessage("|cff00ff00DEBUG: wp modify del, PathID: |r|cff00ffff%u|r", pathid);

            // wpCreature
            wpCreature = nullptr;

            if( wpGuid != 0 )
            {
                wpCreature = ObjectAccessor::GetCreature(*handler->GetSession()->GetPlayer(), ObjectGuid(HighGuid::Unit, VISUAL_WAYPOINT, wpGuid));
                wpCreature->CombatStop();
                wpCreature->DeleteFromDB();
                wpCreature->AddObjectToRemoveList();
            }

            SQLTransaction trans = WorldDatabase.BeginTransaction();
            trans->PAppend("DELETE FROM waypoint_data WHERE id='%u' AND point='%u'",
                pathid, point);
            trans->PAppend("UPDATE waypoint_data SET point=point-1 WHERE id='%u' AND point>'%u'",
                pathid, point);
            WorldDatabase.CommitTransaction(trans);

            handler->PSendSysMessage(LANG_WAYPOINT_REMOVED);
            return true;
        }                                                       // del

        if(show == "move" && target)
        {
            handler->PSendSysMessage("|cff00ff00DEBUG: wp move, PathID: |r|cff00ffff%u|r", pathid);

            Player *chr = handler->GetSession()->GetPlayer();
            Map *map = chr->GetMap();
            {
                // wpCreature
                wpCreature = nullptr;
                // What to do:
                // Move the visual spawnpoint
                // Respawn the owner of the waypoints
                if( wpGuid != 0 )
                {
                    wpCreature = ObjectAccessor::GetCreature(*handler->GetSession()->GetPlayer(), ObjectGuid(HighGuid::Unit , VISUAL_WAYPOINT, wpGuid));
                    wpCreature->CombatStop();
                    wpCreature->DeleteFromDB();
                    wpCreature->AddObjectToRemoveList();
                    // re-create
                    auto wpCreature2 = new Creature;
                    if (!wpCreature2->Create(sObjectMgr->GenerateCreatureSpawnId(), map, chr->GetPhaseMask(), VISUAL_WAYPOINT, { chr->GetPositionX(), chr->GetPositionY(), chr->GetPositionZ(), chr->GetOrientation() }))
                    {
                        handler->PSendSysMessage(LANG_WAYPOINT_VP_NOTCREATED, VISUAL_WAYPOINT);
                        delete wpCreature2;
                        return false;
                    }

                    if(!wpCreature2->IsPositionValid())
                    {
                        TC_LOG_ERROR("command","ERROR: creature (guidlow %d, entry %d) not created. Suggested coordinates aren't valid (X: %f Y: %f)",wpCreature2->GetGUID().GetCounter(),wpCreature2->GetEntry(),wpCreature2->GetPositionX(),wpCreature2->GetPositionY());
                        delete wpCreature2;
                        return false;
                    }

                    wpCreature2->SaveToDB(map->GetId(), (1 << map->GetSpawnMode()));
                    // To call _LoadGoods(); _LoadQuests(); CreateTrainerSpells();
                    wpCreature2->LoadFromDB(wpCreature2->GetSpawnId(), map, true, false);
                }

                WorldDatabase.PExecute("UPDATE waypoint_data SET position_x = '%f',position_y = '%f',position_z = '%f' where id = '%u' AND point='%u'",
                    chr->GetPositionX(), chr->GetPositionY(), chr->GetPositionZ(), pathid, point );

                handler->PSendSysMessage(LANG_WAYPOINT_CHANGED);
            }
            return true;
        }                                                       // move


        const char *text = arg_str;

        if( text == nullptr )
        {
            // show_str check for present in list of correct values, no sql injection possible
            WorldDatabase.PExecute("UPDATE waypoint_data SET %s=NULL WHERE id='%u' AND point='%u'",
                show_str, pathid, point);
        }
        else
        {
            // show_str check for present in list of correct values, no sql injection possible
            std::string text2 = text;
            WorldDatabase.EscapeString(text2);
            WorldDatabase.PExecute("UPDATE waypoint_data SET %s='%s' WHERE id='%u' AND point='%u'",
                show_str, text2.c_str(), pathid, point);
        }

        handler->PSendSysMessage(LANG_WAYPOINT_CHANGED_NO, show_str);

        return true;
    }


    /* Syntax : .path type <pathid> [type] 
    Note that this doesn't update creatures already using this path.

    Possible types :
    0 - WP_PATH_TYPE_LOOP
    1 - WP_PATH_TYPE_ONCE
    2 - WP_PATH_TYPE_ROUND_TRIP
    */
    static bool HandleWpChangePathTypeCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

        char* pathIdStr = strtok((char*)args, " ");
        uint32 pathId = uint32(atoi(pathIdStr));
        if(!pathId)
            return false;

        QueryResult result = WorldDatabase.PQuery( "SELECT 0 FROM waypoint_data WHERE id = '%u' LIMIT 1",pathId); 
        if(!result)
        {
            handler->PSendSysMessage("No path of given id (%u) found", pathId);
            handler->SetSentErrorMessage(true);
            return false;
        }

        char* typeStr = strtok(nullptr, " ");
        if(typeStr) //if the second argument was given
        { //setter
            uint32 type = uint32(atoi(typeStr));
            if(type >= WP_PATH_TYPE_TOTAL)
            {
                handler->PSendSysMessage("Wrong type given : %u", type);
                handler->SetSentErrorMessage(true);
                return true;
            }
        
            //change in db
            PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_REP_WAYPOINT_PATH_TYPE);
            stmt->setUInt32(0, pathId);
            stmt->setUInt16(1, type);
            WorldDatabase.Execute(stmt);

            //change in memory
            WaypointPath* path = (WaypointPath*)sWaypointMgr->GetPath(pathId);
            path->pathType = type;

            std::string pathTypeStr = GetWaypointPathTypeName(WaypointPathType(type));
            handler->PSendSysMessage("Changed path %u type to %s (%u)", pathId, pathTypeStr.c_str(), type);
        } else 
        { //getter
            // check db value
            PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_WAYPOINT_PATH_TYPE);
            stmt->setUInt32(0, pathId);
            PreparedQueryResult result_ = WorldDatabase.Query(stmt);
        
            if(result_)
            {
                uint32 type = result_->Fetch()->GetUInt16();
                std::string pathTypeStr = GetWaypointPathTypeName(WaypointPathType(type));
                handler->PSendSysMessage("DB : Path id %u has type set to %s (%u)", pathId, pathTypeStr.c_str(), type);
            } else {
                handler->PSendSysMessage("No db entry found for path id %u", pathId);
                handler->SetSentErrorMessage(true);
                return false;
            }
            // check memory value
            WaypointPath const* path = sWaypointMgr->GetPath(pathId);
            if(path)
            {
                uint8 type = path->pathType;
                std::string pathTypeStr = GetWaypointPathTypeName(WaypointPathType(type));
                handler->PSendSysMessage("Memory : Path id %u has type set to %s (%u)", pathId, pathTypeStr.c_str(), type);
            }
        }
        return true;
    }

    /** .path teleport #path_id [#point_id] **/
    static bool HandleWpTeleportToPathCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

        Player* p = handler->GetSession() ? handler->GetSession()->GetPlayer() : nullptr;
        if (!p)
            return false;

        char* pathIdStr = strtok((char*)args, " ");
        if (!pathIdStr)
            return false;

        uint32 pathId = uint32(atoi(pathIdStr));
        if (!pathId)
            return false;

        uint32 pointId = 0;
        if (char* pointStr = strtok(nullptr, " "))
            pointId = atoi(pointStr);

        WaypointPath* path = (WaypointPath*)sWaypointMgr->GetPath(pathId);
        if (!path)
        {
            handler->PSendSysMessage("Could not find path %u", pathId);
            handler->SetSentErrorMessage(true);
            return false;
        }

        for (auto itr : path->nodes)
        {
            if (itr.id == pointId)
            {
                p->TeleportTo(p->GetMapId(), itr.x, itr.y, itr.z, itr.orientation ? itr.orientation : p->GetOrientation());
                return true;
            }
        }

        handler->PSendSysMessage("Could not find point with id %u in path %u", pointId, pathId);
        handler->SetSentErrorMessage(true);
        return false;
    }

    /* Syntax : .path direction <pathid> [dir] 
    Note that this doesn't update creatures already using this path.

    Possible directions :
    0 - WP_PATH_DIRECTION_NORMAL
    1 - WP_PATH_DIRECTION_REVERSE
    2 - WP_PATH_DIRECTION_RANDOM
    */
    static bool HandleWpChangePathDirectionCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

        char* pathIdStr = strtok((char*)args, " ");
        uint32 pathId = uint32(atoi(pathIdStr));
        if(!pathId)
            return false;

        QueryResult result = WorldDatabase.PQuery( "SELECT 0 FROM waypoint_data WHERE id = '%u' LIMIT 1",pathId); 
        if(!result)
        {
            handler->PSendSysMessage("No path of given id (%u) found", pathId);
            handler->SetSentErrorMessage(true);
            return false;
        }

        char* dirStr = strtok(nullptr, " ");
        if(dirStr) //if the second argument was given
        { //setter
            uint32 dir = uint32(atoi(dirStr));
            if(dir >= WP_PATH_DIRECTION_TOTAL)
            {
                handler->PSendSysMessage("Wrong direction given : %u", dir);
                handler->SetSentErrorMessage(true);
                return true;
            }
        
            //change in db
            PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_REP_WAYPOINT_PATH_DIRECTION);
            stmt->setUInt32(0, pathId);
            stmt->setUInt16(1, dir);
            WorldDatabase.Execute(stmt);

            //change in memory
            WaypointPath* path = (WaypointPath*)sWaypointMgr->GetPath(pathId);
            path->pathDirection = dir;

            std::string pathDirStr = GetWaypointPathDirectionName(WaypointPathDirection(dir));
            handler->PSendSysMessage("Changed path %u direction to %s (%u)", pathId, pathDirStr.c_str(), dir);
        } else 
        { //getter
            // check db value
            PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_WAYPOINT_PATH_DIRECTION);
            stmt->setUInt32(0, pathId);
            PreparedQueryResult result_ = WorldDatabase.Query(stmt);
        
            if(result_)
            {
                uint32 dir = result_->Fetch()->GetUInt16();
                std::string pathDirStr = GetWaypointPathDirectionName(WaypointPathDirection(dir));
                handler->PSendSysMessage("DB : Path id %u has direction set to %s (%u)", pathId, pathDirStr.c_str(), dir);
            } else {
                handler->PSendSysMessage("No db entry found for path id %u", pathId);
                handler->SetSentErrorMessage(true);
                return false;
            }
            // check memory value
            WaypointPath const* path = sWaypointMgr->GetPath(pathId);
            if(path)
            {
                uint8 dir = path->pathDirection;
                std::string pathDirStr = GetWaypointPathDirectionName(WaypointPathDirection(dir));
                handler->PSendSysMessage("Current path value (in memory) : Path id %u has direction set to %s (%u)", pathId, pathDirStr.c_str(), dir);
            }
        }
        return true;
    }

    static bool HandleWpAddCommand(ChatHandler* handler, char const* args)
    {
    
        // optional
        char* path_number = nullptr;
        uint32 pathid = 0;

        if(*args)
            path_number = strtok((char*)args, " ");

        uint32 point = 0;
        Creature* target =  handler->GetSelectedCreature();

        if (!path_number)
            {
            if(target)
                pathid = target->GetWaypointPath();
                    else
                    {
                        QueryResult result = WorldDatabase.PQuery( "SELECT MAX(id) FROM waypoint_data");
                        if(result)
                        {
                            uint32 maxpathid = result->Fetch()->GetInt32();
                            pathid = maxpathid+1;
                            handler->PSendSysMessage("%s%s|r", "|cff00ff00", "New path started.");
                        }
                    }
            }
            else
                pathid = atoi(path_number);

        // path_id -> ID of the Path
        // point   -> number of the waypoint (if not 0)

        if(!pathid)
        {
            handler->PSendSysMessage("%s%s|r", "|cffff33ff", "Current creature haven't loaded path.");
            return true;
        }

        QueryResult result = WorldDatabase.PQuery( "SELECT MAX(point) FROM waypoint_data WHERE id = '%u'",pathid);

        if( result )
        {
            point = (*result)[0].GetUInt32();
        }

        Player* player = handler->GetSession()->GetPlayer();

        WorldDatabase.PExecute("INSERT INTO waypoint_data (id, point, position_x, position_y, position_z) VALUES ('%u','%u','%f', '%f', '%f')",
            pathid, point+1, player->GetPositionX(), player->GetPositionY(), player->GetPositionZ());

            handler->PSendSysMessage("%s%s%u%s%u%s|r", "|cff00ff00", "PathID: |r|cff00ffff", pathid, "|r|cff00ff00: Waypoint |r|cff00ffff", point,"|r|cff00ff00 created. ");

        return true;
    }                                                           // HandleWpAddCommand

    static bool HandleWpLoadPathCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

        // optional
        char* path_number = nullptr;

        if(*args)
            path_number = strtok((char*)args, " ");


        uint32 pathid = 0;
        uint32 guidlow = 0;
        Creature* target =  handler->GetSelectedCreature();

        if(!target)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if(target->GetEntry() == 1)
        {
            handler->PSendSysMessage("%s%s|r", "|cffff33ff", "You want to load path to a waypoint? Aren't you?");
            handler->SetSentErrorMessage(true);
            return false;
        }

        pathid = atoi(path_number);

        if(!pathid)
        {
            handler->PSendSysMessage("%s%s|r", "|cffff33ff", "No valid path number provided.");
            return true;
        }

        guidlow = target->GetSpawnId();
        QueryResult result = WorldDatabase.PQuery( "SELECT guid FROM creature_addon WHERE guid = '%u'",guidlow);

        if( result )
        {
            WorldDatabase.PExecute("UPDATE creature_addon SET path_id = '%u' WHERE guid = '%u'", pathid, guidlow);
        }
        else
        {
            //if there's any creature_template_addon, let's base ourserlves on it
            result = WorldDatabase.PQuery( "SELECT mount,bytes0,bytes1,bytes2,emote,moveflags,auras FROM creature_template_addon WHERE entry = '%u'",target->GetEntry());
            if(result)
            {
                uint32 mount = (*result)[0].GetUInt32();
                uint32 bytes0 = (*result)[1].GetUInt32();
                uint32 bytes1 = (*result)[2].GetUInt32();
                uint32 bytes2 = (*result)[3].GetUInt32();
                uint32 emote = (*result)[4].GetUInt32();
                uint32 moveflags = (*result)[5].GetUInt32();
                const char* auras = (*result)[6].GetCString();
                WorldDatabase.PExecute("INSERT INTO creature_addon(guid,path_id,mount,bytes0,bytes1,bytes2,emote,moveflags,auras) VALUES \
                                       ('%u','%u','%u','%u','%u','%u','%u','%u','%s')", guidlow, pathid,mount,bytes0,bytes1,bytes2,emote,moveflags,auras);
            } else { //else just create a new entry
                WorldDatabase.PExecute("INSERT INTO creature_addon(guid,path_id) VALUES ('%u','%u')", guidlow, pathid);
            }
        }

        WorldDatabase.PExecute("UPDATE creature SET MovementType = '%u' WHERE guid = '%u'", WAYPOINT_MOTION_TYPE, guidlow);

        target->LoadPath(pathid);
        target->SetDefaultMovementType(WAYPOINT_MOTION_TYPE);
        target->GetMotionMaster()->Initialize();
        target->Say("Path loaded.",LANG_UNIVERSAL,nullptr);

        return true;
    }
};

void AddSC_wp_commandscript()
{
    new wp_commandscript();
}
