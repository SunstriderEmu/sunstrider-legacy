#include "Chat.h"
#include "Language.h"
#include "Transport.h"
#include "TargetedMovementGenerator.h"
#include "GameEventMgr.h"
#include "WaypointMovementGenerator.h"
#include "GridNotifiersImpl.h"
#include "CellImpl.h"
#include "PoolMgr.h"

bool ChatHandler::HandleNpcGuidCommand(const char* args)
{
    Creature* target = GetSelectedCreature();
    if (!target)
        return false;
        
    PSendSysMessage("GUID: %u", target->GetSpawnId());
    
    return true;
}

bool ChatHandler::HandleNpcSayCommand(const char* args)
{
    ARGS_CHECK

    Creature* pCreature = GetSelectedCreature();
    if(!pCreature)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        SetSentErrorMessage(true);
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

bool ChatHandler::HandleNpcYellCommand(const char* args)
{
    ARGS_CHECK

    Creature* pCreature = GetSelectedCreature();
    if(!pCreature)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    pCreature->Yell(args, LANG_UNIVERSAL, nullptr);

    // make an emote
    pCreature->HandleEmoteCommand(EMOTE_ONESHOT_SHOUT);

    return true;
}

//show text emote by creature in chat
bool ChatHandler::HandleNpcTextEmoteCommand(const char* args)
{
    ARGS_CHECK

    Creature* pCreature = GetSelectedCreature();

    if(!pCreature)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    pCreature->TextEmote(args, nullptr);

    return true;
}

// make npc whisper to player
bool ChatHandler::HandleNpcWhisperCommand(const char* args)
{
    ARGS_CHECK

    char* receiver_str = strtok((char*)args, " ");
    char* text = strtok(nullptr, "");

    ObjectGuid guid = m_session->GetPlayer()->GetTarget();
    Creature* pCreature = ObjectAccessor::GetCreature(*m_session->GetPlayer(), guid);

    if(!pCreature || !receiver_str || !text)
    {
        return false;
    }

    ObjectGuid receiver_guid = ObjectGuid(uint64(atol(receiver_str)));
    Player* targetPlayer = ObjectAccessor::FindPlayer(receiver_guid);
    if(!targetPlayer)
    {
        SendSysMessage("Could not find player with that name.");
        return true;
    }

    pCreature->Whisper(text,LANG_UNIVERSAL,targetPlayer);

    return true;
}

//add spawn of creature
bool ChatHandler::HandleNpcAddCommand(const char* args)
{
    ARGS_CHECK
    char* cId = strtok((char*)args, " ");
    if (!cId)
        return false;

    uint32 id  = atoi(cId);

    Player *chr = m_session->GetPlayer();
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
                SendSysMessage("Error: cannot save creature on transport because trans->GetGOInfo() == NULL");
                return true;
            }
            if (Creature* creature = trans->CreateNPCPassenger(guid, &data))
            {
                sObjectMgr->AddCreatureToGrid(guid, &data);
                creature->SaveToDB(trans->GetGOInfo()->moTransport.mapID, 1 << map->GetSpawnMode());
            }
            else {
                SendSysMessage("Error: cannot create NPC Passenger.");
            }
            return true;
        }
    }

    auto pCreature = new Creature;
    if (!pCreature->Create(sObjectMgr->GenerateCreatureSpawnId(), map, chr->GetPhaseMask(), id, { x, y, z, o }))
    {
        delete pCreature;
        return false;
    }

    if(!pCreature->IsPositionValid())
    {
        TC_LOG_ERROR("command","ERROR: Creature (guidlow %d, entry %d) not created. Suggested coordinates isn't valid (X: %f Y: %f)",pCreature->GetGUID().GetCounter(),pCreature->GetEntry(),pCreature->GetPositionX(),pCreature->GetPositionY());
        delete pCreature;
        return false;
    }

    pCreature->SaveToDB(map->GetId(), (1 << map->GetSpawnMode()));

    ObjectGuid::LowType db_guid = pCreature->GetSpawnId();

    // To call _LoadGoods(); _LoadQuests(); CreateTrainerSpells();
    pCreature->LoadFromDB(db_guid, map, true, false);

    sObjectMgr->AddCreatureToGrid(db_guid, sObjectMgr->GetCreatureData(db_guid));
    return true;
}

bool ChatHandler::HandleNpcDeleteCommand(const char* args)
{
    Creature* unit = nullptr;

    if(*args)
    {
        // number or [name] Shift-click form |color|Hcreature:creature_guid|h[name]|h|r
        char* cId = extractKeyFromLink((char*)args,"Hcreature");
        if(!cId)
            return false;

        ObjectGuid::LowType lowguid = atoi(cId);
        if(!lowguid)
            return false;

        if (CreatureData const* cr_data = sObjectMgr->GetCreatureData(lowguid))
            unit = ObjectAccessor::GetCreature(*m_session->GetPlayer(), ObjectGuid(HighGuid::Unit, cr_data->id, lowguid));
    }
    else
        unit = GetSelectedCreature();

    if(!unit || unit->IsPet() || unit->IsTotem())
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    // Delete the creature
    unit->DeleteFromDB();
    unit->AddObjectToRemoveList();

    SendSysMessage(LANG_COMMAND_DELCREATMESSAGE);

    return true;
}

//move selected creature
bool ChatHandler::HandleNpcMoveCommand(const char* args)
{
    ObjectGuid::LowType lowguid = 0;

    Creature* pCreature = GetSelectedCreature();

    if(!pCreature)
    {
        // number or [name] Shift-click form |color|Hcreature:creature_guid|h[name]|h|r
        char* cId = extractKeyFromLink((char*)args,"Hcreature");
        if(!cId)
            return false;

        lowguid = atoi(cId);

        /* FIXME: impossible without entry
        if(lowguid)
            pCreature = ObjectAccessor::GetCreature(*m_session->GetPlayer(),MAKE_GUID(lowguid,HighGuid::Unit));
        */

        // Attempting creature load from DB data
        if(!pCreature)
        {
            CreatureData const* data = sObjectMgr->GetCreatureData(lowguid);
            if(!data)
            {
                PSendSysMessage(LANG_COMMAND_CREATGUIDNOTFOUND, lowguid);
                SetSentErrorMessage(true);
                return false;
            }

            uint32 map_id = data->spawnPoint.GetMapId();

            if(m_session->GetPlayer()->GetMapId() != map_id)
            {
                PSendSysMessage(LANG_COMMAND_CREATUREATSAMEMAP, lowguid);
                SetSentErrorMessage(true);
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

    if(Transport* trans = m_session->GetPlayer()->GetTransport())
    {
        x = m_session->GetPlayer()->GetTransOffsetX();
        y = m_session->GetPlayer()->GetTransOffsetY();
        z = m_session->GetPlayer()->GetTransOffsetZ();
        o = m_session->GetPlayer()->GetTransOffsetO();
    } else {
        x = m_session->GetPlayer()->GetPositionX();
        y = m_session->GetPlayer()->GetPositionY();
        z = m_session->GetPlayer()->GetPositionZ();
        o = m_session->GetPlayer()->GetOrientation();
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

    PSendSysMessage(LANG_COMMAND_CREATUREMOVED);
    return true;
}

bool ChatHandler::HandleNpcFlyCommand(const char* args)
{
    Creature* pCreature = GetSelectedCreature();
    if(!pCreature)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        return true;
    }

    ARGS_CHECK

    std::string argstr = (char*)args;

    if (argstr == "on")
    {
        pCreature->SetCanFly(true);
        PSendSysMessage("Creature is now fly-capable");
        return true;
    } else if (argstr == "off")
    {
        pCreature->SetCanFly(false);
        PSendSysMessage("Creature is now not fly-capable");
        return true;
    }

    return false;
}

bool ChatHandler::HandleNpcGotoCommand(const char* args)
{
    Creature* pCreature = GetSelectedCreature();
    if(!pCreature)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        return true;
    }

    ARGS_CHECK

    return true;
}

//change level of creature or pet
bool ChatHandler::HandleNpcChangeLevelCommand(const char* args)
{
    ARGS_CHECK

    uint8 lvl = (uint8) atoi((char*)args);
    if ( lvl < 1 || lvl > sWorld->getConfig(CONFIG_MAX_PLAYER_LEVEL) + 3)
    {
        SendSysMessage(LANG_BAD_VALUE);
        SetSentErrorMessage(true);
        return false;
    }

    Creature* pCreature = GetSelectedCreature();
    if(!pCreature)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        SetSentErrorMessage(true);
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
bool ChatHandler::HandleNpcFlagCommand(const char* args)
{
    ARGS_CHECK

    uint32 npcFlags = (uint32) atoi((char*)args);

    Creature* pCreature = GetSelectedCreature();

    if(!pCreature)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    pCreature->SetUInt32Value(UNIT_NPC_FLAGS, npcFlags);

    //WorldDatabase.PExecute("UPDATE creature_template SET npcflag = '%u' WHERE entry = '%u'", npcFlags, pCreature->GetEntry());

    SendSysMessage(LANG_VALUE_SAVED_REJOIN);

    return true;
}

//set model of creature
bool ChatHandler::HandleNpcSetModelCommand(const char* args)
{
    ARGS_CHECK

    uint32 displayId = (uint32) atoi((char*)args);

    Creature *pCreature = GetSelectedCreature();

    if(!pCreature || pCreature->IsPet())
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    pCreature->SetDisplayId(displayId);
    pCreature->SetNativeDisplayId(displayId);

    pCreature->SaveToDB();

    return true;
}

//del item from vendor list
bool ChatHandler::HandleNpcDelVendorItemCommand(const char* args)
{
    ARGS_CHECK

    Creature* vendor = GetSelectedCreature();
    if (!vendor || !vendor->IsVendor())
    {
        SendSysMessage(LANG_COMMAND_VENDORSELECTION);
        SetSentErrorMessage(true);
        return false;
    }

    char* pitem  = extractKeyFromLink((char*)args,"Hitem");
    if (!pitem)
    {
        SendSysMessage(LANG_COMMAND_NEEDITEMSEND);
        SetSentErrorMessage(true);
        return false;
    }
    uint32 itemId = atol(pitem);
    
    ItemTemplate const* pProto = sObjectMgr->GetItemTemplate(itemId);
    if(!pProto)
    {
        PSendSysMessage("Invalid id");
        return true;
    }

    if(!sObjectMgr->RemoveVendorItem(vendor->GetEntry(),pProto))
    {
        PSendSysMessage(LANG_ITEM_NOT_IN_LIST,itemId);
        SetSentErrorMessage(true);
        return false;
    }


    PSendSysMessage(LANG_ITEM_DELETED_FROM_LIST,itemId,pProto->Name1.c_str());
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
bool ChatHandler::HandleNpcSetMoveTypeCommand(const char* args)
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
        pCreature = GetSelectedCreature();
        if(!pCreature || pCreature->IsPet())
            return false;
        lowguid = pCreature->GetSpawnId();
    }
    else                                                    // case .setmovetype #creature_guid $move_type (with selected creature)
    {
        lowguid = atoi((char*)guid_str);

        /* impossible without entry
        if(lowguid)
            pCreature = ObjectAccessor::GetCreature(*m_session->GetPlayer(),MAKE_GUID(lowguid,HighGuid::Unit));
        */

        // attempt check creature existence by DB data
        if(!pCreature)
        {
            CreatureData const* data = sObjectMgr->GetCreatureData(lowguid);
            if(!data)
            {
                PSendSysMessage(LANG_COMMAND_CREATGUIDNOTFOUND, lowguid);
                SetSentErrorMessage(true);
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
        PSendSysMessage(LANG_MOVE_TYPE_SET,type_str);
    }
    else
    {
        PSendSysMessage(LANG_MOVE_TYPE_SET_NODEL,type_str);
    }

    return true;
}                                                           // HandleNpcSetMoveTypeCommand

bool ChatHandler::HandleNpcChangeEntryCommand(const char *args)
{
    uint32 newEntryNum = atoi(args);
    if(!newEntryNum)
        return false;

    Unit* unit = GetSelectedUnit();
    if(!unit || unit->GetTypeId() != TYPEID_UNIT)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }
    Creature* creature = unit->ToCreature();
    if(creature->UpdateEntry(newEntryNum))
        SendSysMessage(LANG_DONE);
    else
        SendSysMessage(LANG_ERROR);
    return true;
}

bool ChatHandler::HandleNpcInfoCommand(const char* /*args*/)
{
    Creature* target = GetSelectedCreature();

    if(!target)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        SetSentErrorMessage(true);
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

    PSendSysMessage(LANG_NPCINFO_CHAR, target->GetSpawnId(), faction, npcflags, Entry, displayid, nativeid);
    if(cInfo->difficulty_entry_1)
        PSendSysMessage("Heroic Entry: %u", cInfo->difficulty_entry_1);
    else if (target->GetMap()->IsHeroic() && Entry != cInfo->Entry)
        PSendSysMessage("Current id (heroic id ?) : %u", cInfo->Entry);

    PSendSysMessage(LANG_NPCINFO_LEVEL, target->GetLevel());
    PSendSysMessage(LANG_NPCINFO_HEALTH,target->GetCreateHealth(), target->GetMaxHealth(), target->GetHealth());
    PSendSysMessage(LANG_NPCINFO_FLAGS, target->GetUInt32Value(UNIT_FIELD_FLAGS), target->GetUInt32Value(UNIT_DYNAMIC_FLAGS), target->GetFaction());
    PSendSysMessage(LANG_COMMAND_RAWPAWNTIMES, defRespawnDelayStr.c_str(),curRespawnDelayStr.c_str());
    PSendSysMessage(LANG_NPCINFO_LOOT,  cInfo->lootid,cInfo->pickpocketLootId,cInfo->SkinLootId);
    PSendSysMessage(LANG_NPCINFO_DUNGEON_ID, target->GetInstanceId());
    PSendSysMessage(LANG_NPCINFO_POSITION,float(target->GetPositionX()), float(target->GetPositionY()), float(target->GetPositionZ()), float(target->GetOrientation()));
    PSendSysMessage("AIName: %s", target->GetAIName().c_str());
    PSendSysMessage("ScriptName: %s", target->GetScriptName().c_str());
    PSendSysMessage("Creature Pool ID: %u", target->GetCreaturePoolId());
    PSendSysMessage("Creature linked instance event: %d", int(target->getInstanceEventId()));
    if(const CreatureData* const linked = target->GetLinkedRespawnCreatureData())
        if(CreatureTemplate const *master = sObjectMgr->GetCreatureTemplate(linked->id))
            PSendSysMessage(LANG_NPCINFO_LINKGUID, sObjectMgr->GetLinkedRespawnGuid(ObjectGuid(HighGuid::Unit, target->GetEntry(), target->GetSpawnId())), linked->id, master->Name.c_str());
    PSendSysMessage("Movement flag: %u", target->GetUnitMovementFlags());
    if ((npcflags & UNIT_NPC_FLAG_VENDOR) )
        SendSysMessage(LANG_NPCINFO_VENDOR);

    if ((npcflags & UNIT_NPC_FLAG_TRAINER) )
        SendSysMessage(LANG_NPCINFO_TRAINER);

    if(target->GetWaypointPath())
        PSendSysMessage("PathID : %u", target->GetWaypointPath());

    if(target->GetFormation())
        PSendSysMessage("Member of formation %u", target->GetFormation()->GetId());

    SpawnData const* data = sObjectMgr->GetSpawnData(SPAWN_TYPE_CREATURE, target->GetSpawnId());
    if (data && data->spawnGroupData)
        PSendSysMessage("Member of spawn group %u", data->spawnGroupData->groupId);

    if(uint32 poolid = sPoolMgr->IsPartOfAPool<Creature>(target->GetSpawnId()))
        PSendSysMessage("Part of pool %u", poolid);

    return true;
}

// .npc setemotestate #emoteid [#permanent]
bool ChatHandler::HandleNpcSetEmoteStateCommand(const char* args)
{
    char* cState = strtok((char*)args, " ");
    char* cPermanent = strtok((char*)nullptr, " ");

    if(!cState) 
        return false;

    uint32 state = atoi(cState);
    uint8 permanent = 0;
    if(cPermanent) 
        permanent = atoi(cPermanent);

    Creature* target = GetSelectedCreature();
    if(!target)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    target->SetEmoteState(state);
    PSendSysMessage("Emote state set to %u", state);

    if(permanent)
    {
        PSendSysMessage("Emote state set as permanent (will stay after a reboot)");
        if(state)
            WorldDatabase.PExecute("INSERT INTO creature_addon(`guid`,`emote`) VALUES (%u,%u) ON DUPLICATE KEY UPDATE emote = %u;", target->GetSpawnId(), state, state);
        else
            WorldDatabase.PExecute("UPDATE creature_addon SET `emote` = 0 WHERE `guid` = %u", target->GetSpawnId());
    }
    return true;
}

bool ChatHandler::HandleNpcFollowCommand(const char* /*args*/)
{
    Player *player = m_session->GetPlayer();
    Creature *creature = GetSelectedCreature();

    if(!creature)
    {
        PSendSysMessage(LANG_SELECT_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    // Follow player - Using pet's default dist and angle
    creature->GetMotionMaster()->MoveFollow(player, PET_FOLLOW_DIST, creature->GetFollowAngle());

    PSendSysMessage(LANG_CREATURE_FOLLOW_YOU_NOW, creature->GetName().c_str());
    return true;
}

bool ChatHandler::HandleNpcUnFollowCommand(const char* /*args*/)
{
    Player *player = m_session->GetPlayer();
    Creature *creature = GetSelectedCreature();

    if(!creature)
    {
        PSendSysMessage(LANG_SELECT_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    if (/*creature->GetMotionMaster()->empty() ||*/
        creature->GetMotionMaster()->GetCurrentMovementGeneratorType ()!=FOLLOW_MOTION_TYPE)
    {
        PSendSysMessage(LANG_CREATURE_NOT_FOLLOW_YOU, creature->GetName().c_str());
        SetSentErrorMessage(true);
        return false;
    }

    FollowMovementGenerator<Creature> const* mgen
        = static_cast<FollowMovementGenerator<Creature> const*>((creature->GetMotionMaster()->top()));

    if(mgen->GetTarget()!=player)
    {
        PSendSysMessage(LANG_CREATURE_NOT_FOLLOW_YOU, creature->GetName().c_str());
        SetSentErrorMessage(true);
        return false;
    }

    // reset movement
    creature->GetMotionMaster()->MovementExpired(true);

    PSendSysMessage(LANG_CREATURE_NOT_FOLLOW_YOU_NOW, creature->GetName().c_str());
    return true;
}

// add creature, temp only
bool ChatHandler::HandleNpcTempAddCommand(const char* args)
{
    ARGS_CHECK
    char* charID = strtok((char*)args, " ");
    if (!charID)
        return false;

    Player *chr = m_session->GetPlayer();

    float x = chr->GetPositionX();
    float y = chr->GetPositionY();
    float z = chr->GetPositionZ();
    float ang = chr->GetOrientation();

    uint32 id = atoi(charID);

    chr->SummonCreature(id,x,y,z,ang, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 1 * MINUTE * IN_MILLISECONDS);

    return true;
}

bool ChatHandler::HandleNpcAddFormationCommand(const char* args)
{
    ARGS_CHECK

        ObjectGuid::LowType leaderGUID = (ObjectGuid::LowType) atoi((char*)args);

    Creature *pCreature = GetSelectedCreature();

    if(!pCreature || !pCreature->GetSpawnId())
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        SetSentErrorMessage(true);
        return true;
    }
    
    if(pCreature->GetFormation())
    {
        PSendSysMessage("Selected creature is already member of group %u.", pCreature->GetFormation()->GetId());
        return true;
    }

    uint32 lowguid = pCreature->GetSpawnId();
    if (!lowguid)
    {
        PSendSysMessage("Creature may not be a summon", leaderGUID);
        return true;
    }

    CreatureData const* data = sObjectMgr->GetCreatureData(leaderGUID);
    if (!data)
    {
        PSendSysMessage("Could not find creature data for guid %u", leaderGUID);
        SetSentErrorMessage(true);
        return false;
    }

    Creature* leader;
    if (pCreature->GetMap()->Instanceable())
        leader = pCreature->GetMap()->GetCreatureBySpawnId(leaderGUID);
    else
        leader = pCreature->GetMap()->GetCreature(ObjectGuid(HighGuid::Unit, data->id, leaderGUID));

    if (!leader)
    {
        PSendSysMessage("Could not find leader (guid %u) in map.", leaderGUID);
        return true;
    }

    if (!leader->GetFormation())
    {
        FormationInfo* group_member;
        group_member = new FormationInfo;
        group_member->leaderGUID = leaderGUID;
        sCreatureGroupMgr->AddGroupMember(leaderGUID, group_member);
        pCreature->SearchFormation();

        WorldDatabase.PExecute("REPLACE INTO `creature_formations` (`leaderGUID`, `memberGUID`, `dist`, `angle`, `groupAI`) VALUES ('%u', '%u', 0, 0, '%u')",
            leaderGUID, leaderGUID, uint32(group_member->groupAI));

        PSendSysMessage("Created formation with leader %u", leaderGUID);
    }

    Player *chr = m_session->GetPlayer();
    FormationInfo* group_member;

    group_member                  = new FormationInfo;
    group_member->follow_angle    = pCreature->GetAngle(chr) - chr->GetOrientation();
    group_member->follow_dist     = sqrtf(pow(chr->GetPositionX() - pCreature->GetPositionX(),int(2))+pow(chr->GetPositionY()-pCreature->GetPositionY(),int(2)));
    group_member->leaderGUID      = leaderGUID;

    sCreatureGroupMgr->AddGroupMember(lowguid, group_member);
    pCreature->SearchFormation();

    WorldDatabase.PExecute("REPLACE INTO `creature_formations` (`leaderGUID`, `memberGUID`, `dist`, `angle`, `groupAI`) VALUES ('%u', '%u','%f', '%f', '%u')",
        leaderGUID, lowguid, group_member->follow_dist, group_member->follow_angle, uint32(group_member->groupAI));

    PSendSysMessage("Creature %u added to formation with leader %u.", lowguid, leaderGUID);

    return true;
 }

bool ChatHandler::HandleNpcRemoveFormationCommand(const char* args)
{
    ARGS_CHECK

    Creature *pCreature = GetSelectedCreature();

    if(!pCreature || !pCreature->GetSpawnId())
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        SetSentErrorMessage(true);
        return true;
    }

    CreatureGroup* formation = pCreature->GetFormation();
    if(!formation)
    {
        PSendSysMessage("Selected creature (%u) is not in a formation.", pCreature->GetGUID().GetCounter());
        return true;
    }

    formation->RemoveMember(pCreature);
    pCreature->SetFormation(nullptr);
    WorldDatabase.PExecute("DELETE ROM `creature_formations` WHERE memberGUID = %u",pCreature->GetGUID().GetCounter());

    PSendSysMessage("Creature removed from formation.");

    return true;
}

bool ChatHandler::HandleNpcSetLinkCommand(const char* args)
{
    ARGS_CHECK

    ObjectGuid::LowType linkguid = (uint32) atoi((char*)args);

    Creature* pCreature = GetSelectedCreature();

    if(!pCreature)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    if(!pCreature->GetSpawnId())
    {
        PSendSysMessage("Selected creature [guidlow:%u] isn't in `creature` table", pCreature->GetGUID().GetCounter());
        SetSentErrorMessage(true);
        return false;
    }

    if(!sObjectMgr->SetCreatureLinkedRespawn(pCreature->GetSpawnId(), linkguid))
    {
        PSendSysMessage("Selected creature can't link with guid '%u'.", linkguid);
        SetSentErrorMessage(true);
        return false;
    }

    PSendSysMessage("LinkGUID '%u' added to creature with DBTableGUID: '%u'.", linkguid, pCreature->GetSpawnId());
    return true;
}

bool ChatHandler::HandleNpcGoBackHomeCommand(const char* args)
{
    if (!*args) {      // Command is applied on selected unit
        Unit* pUnit = GetSelectedUnit();
        if (!pUnit || pUnit == m_session->GetPlayer())
            return false;
        
        float x, y, z, o;
        (pUnit->ToCreature())->GetHomePosition(x, y, z, o);
        pUnit->GetMotionMaster()->MovePoint(0, x, y, z);
        return true;
    }
    else {                      // On specified GUID
        char* guid = strtok((char *)args, " ");
        Player* plr = m_session->GetPlayer();
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
                PSendSysMessage("No unit found.");
                return true;
            }
            float x, y, z, o;
            (pUnit->ToCreature())->GetHomePosition(x, y, z, o);
            (pUnit->ToCreature())->GetMotionMaster()->MovePoint(0, x, y, z);
            return true;
        }
        PSendSysMessage("No unit found.");
    }
    
    return false;
}

bool ChatHandler::HandleNpcSetPoolCommand(const char* args)
{
    ARGS_CHECK
    
    char *chrPoolId = strtok((char *)args, " ");
    if (!chrPoolId)
        return false;
        
    uint32 poolId = (uint32)atoi(chrPoolId);
    if (!poolId)
        return false;
        
    Creature *creature = GetSelectedCreature();
    if (!creature)
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        return true;
    }
        
    WorldDatabase.PExecute("UPDATE creature SET pool_id = %u WHERE guid = %u", poolId, creature->GetSpawnId());
    creature->SetCreaturePoolId(poolId);
    creature->FindMap()->AddCreatureToPool(creature, poolId);
    PSendSysMessage("Creature (guid: %u) added to pool %u",creature->GetSpawnId(),poolId);
    return true;
}

/** Syntax: .npc addweapon #weaponId [#slot] 
#slot = 0 : right hand, 1 : left hand, 2 : ranged
*/
bool ChatHandler::HandleNpcAddWeaponCommand(const char* args)
{
    ARGS_CHECK
 
    Creature *pCreature = GetSelectedCreature();
 
    if(!pCreature)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
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
        PSendSysMessage(LANG_ITEM_NOT_FOUND,itemID);
        return true;
    }
/*
    PSendSysMessage("Class = %u",proto->Class);
    PSendSysMessage("SubClass = %u",proto->SubClass);
    PSendSysMessage("InventoryType = %u",proto->InventoryType);
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
                PSendSysMessage("Given slot %u invalid.",slot);
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
            PSendSysMessage("Invalid object %u.",itemID);
            return true;
            break;
    }
    pCreature->SetWeapon(slot, proto->DisplayInfoID, (ItemSubclassWeapon)proto->SubClass, (InventoryType)proto->InventoryType);
 
    PSendSysMessage(LANG_ITEM_ADDED_TO_SLOT,itemID,proto->Name1.c_str(),slot);
    return true;
}

bool ChatHandler::HandleNpcMassFactionIdCommand(const char* args)
{
    char *entryid = strtok((char *)args, " ");
    if (!entryid)
        return false;

    char *factid = strtok(nullptr, " ");
    if (!factid)
        return false;

    Player *player = m_session->GetPlayer();

    uint32 factionId = (uint32)atoi(factid);
    uint32 entryId = (uint32)atoi(entryid);

    if (!sFactionTemplateStore.LookupEntry(factionId)) {
        PSendSysMessage(LANG_WRONG_FACTION, factionId);
        SetSentErrorMessage(true);
        return false;
    }

    CellCoord p(Trinity::ComputeCellCoord(player->GetPositionX(), player->GetPositionY()));
    Cell cell(p);

    Trinity::FactionDo u_do(entryId, factionId);
    Trinity::WorldObjectWorker<Trinity::FactionDo> worker(player, u_do);
    Cell::VisitGridObjects(player, worker, MAX_SEARCHER_DISTANCE);

    return true;
}

bool ChatHandler::HandleNpcSetCombatDistanceCommand(const char* args)
{
    ARGS_CHECK

    char* cDistance = strtok((char*)args, " ");
    if (!cDistance)
        return false;
        
    float distance = (float)atof(cDistance);

    Creature *pCreature = GetSelectedCreature();
    if(!pCreature)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        return true;
    }

    if(pCreature->AI())
    {
        pCreature->SetCombatDistance(distance);
        PSendSysMessage("m_combatDistance set to %f", distance);
    }

    return true;
}

bool ChatHandler::HandleNpcAllowCombatMovementCommand(const char* args)
{
    ARGS_CHECK

    char* cAllow = strtok((char*)args, " ");
    if (!cAllow)
        return false;
        
    int allow = atoi(cAllow);

    Creature *pCreature = GetSelectedCreature();
    if(!pCreature)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        return true;
    }

    if(pCreature->AI())
    {
        pCreature->AI()->SetCombatMovementAllowed(allow);
        PSendSysMessage("m_allowCombatMovement set to %s", allow ? "true" : "false");
    }

    return true;
}

/* if no args given, tell if the selected creature is linked to a game_event. Else usage is .npc linkgameevent #eventid [#guid] (a guid may be given, overiding the selected creature)*/
bool ChatHandler::HandleNpcLinkGameEventCommand(const char* args)
{
    CreatureData const* data = nullptr;
    char* cEvent = strtok((char*)args, " ");
    char* cCreatureGUID = strtok(nullptr, " ");
    int16 event = 0;
    ObjectGuid::LowType creatureGUID = 0;
    bool justShowInfo = false;
    if(!cEvent) // No params given
    {
        justShowInfo = true;
    } else {
        event = atoi(cEvent);
        if(cCreatureGUID) // erase selected creature if guid explicitely given
            creatureGUID = atoi(cCreatureGUID);
    }

    if(!creatureGUID)
    {
        Creature* creature = GetSelectedCreature();
        if(creature)
            creatureGUID = creature->GetSpawnId();
    }

    data = sObjectMgr->GetCreatureData(creatureGUID);
    if(!data)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        return true;
    }

    ObjectGuid fullGUID = ObjectGuid(HighGuid::Unit, data->id, creatureGUID);
    int16 currentEventId = sGameEventMgr->GetCreatureEvent(fullGUID);

    if (justShowInfo)
    {
        if(currentEventId)
            //PSendSysMessage("La creature (guid : %u) est liée à l'event %i.",creatureGUID,currentEventId);
            PSendSysMessage("Creature (guid: %u) bound to event %i.",creatureGUID, currentEventId);
        else
            //PSendSysMessage("La creature (guid : %u) n'est liée à aucun event.",creatureGUID);
            PSendSysMessage("Creature (guid : %u) is not bound to an event.", creatureGUID);
    } else {
        if(currentEventId)
        {
           // PSendSysMessage("La creature (guid : %u) est déjà liée à l'event %i.",creatureGUID,currentEventId);
            //PSendSysMessage("La creature est déjà liée à l'event %i.",currentEventId);
            PSendSysMessage("Creature bound to event %i.",currentEventId);
            return true;
        }

        if(sGameEventMgr->AddCreatureToEvent(fullGUID, event))
            //PSendSysMessage("La creature (guid : %u) a été liée à l'event %i.",creatureGUID,event);
            PSendSysMessage("Creature (guid: %u) is now bound to the event %i.",creatureGUID,event);
        else
            //PSendSysMessage("Erreur : La creature (guid : %u) n'a pas pu être liée à l'event %d (event inexistant ?).",creatureGUID,event);
            PSendSysMessage("Error: creature (guid: %u) could not be linked to the event %d (event nonexistent?).",creatureGUID,event);
    }

    return true;
}

/* .npc unlinkgameevent [#guid] */
bool ChatHandler::HandleNpcUnlinkGameEventCommand(const char* args)
{
    Creature* creature = nullptr;
    CreatureData const* data = nullptr;
    char* cCreatureGUID = strtok((char*)args, " ");
    ObjectGuid::LowType creatureGUID = 0;

    if(cCreatureGUID) //Guid given
    {
        creatureGUID = atoi(cCreatureGUID);
    } else { //else, try to get selected creature
        creature = GetSelectedCreature();
        if(!creature)
        {
            SendSysMessage(LANG_SELECT_CREATURE);
            SetSentErrorMessage(true);
            return false;
        }           
        creatureGUID = creature->GetGUID().GetCounter();
    }

    data = sObjectMgr->GetCreatureData(creatureGUID);
    if(!data)
    {
        //PSendSysMessage("Creature avec le guid %u introuvable.",creatureGUID);
        PSendSysMessage("Creature with guid %u not found.",creatureGUID);
        return true;
    } 

    ObjectGuid fullGUID = ObjectGuid(HighGuid::Unit, data->id, creatureGUID);
    int16 currentEventId = sGameEventMgr->GetCreatureEvent(fullGUID);

    if (!currentEventId)
    {
        //PSendSysMessage("La creature (guid : %u) n'est liée à aucun event.",creatureGUID);
        PSendSysMessage("Creature (guid: %u) is not linked to any event.",creatureGUID);
    } else {
        if(sGameEventMgr->RemoveCreatureFromEvent(fullGUID))
            //PSendSysMessage("La creature (guid : %u) n'est plus liée à l'event %i.",creatureGUID,currentEventId);
            PSendSysMessage("Creature (guid: %u) is not anymore linked to the event %i.",creatureGUID,currentEventId);
        else
            //PSendSysMessage("Erreur lors de la suppression de la créature (guid : %u) de l'event %i.",creatureGUID,currentEventId);
            PSendSysMessage("Error on removing creature (guid: %u) from the event %i.",creatureGUID,currentEventId);
    }

    return true;
}

bool ChatHandler::HandleNpcNearCommand(const char* args)
{
    float distance = (!*args) ? 10 : atol(args);
    uint32 count = 0;

    Player* pl = m_session->GetPlayer();
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

            PSendSysMessage("guid %u, id %u - |cffffffff|Hcreature:%d|h[%s X:%f Y:%f Z:%f MapId:%d]|h|r", guid, entry, guid, cInfo->Name.c_str(), x, y, z, mapid);
            if (linked_event)
                PSendSysMessage("Linked event: %i", linked_event);

            ++count;
        } while (result->NextRow());
    }

    PSendSysMessage("Found near creatures (distance %f): %u",distance,count);
    return true;
}

bool ChatHandler::HandleNpcNameCommand(const char* args)
{
    ARGS_CHECK

    uint32 id = atoi(args);

    Creature* pCreature = GetSelectedCreature();
    if(!pCreature)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        return true;
    }

    if (CreatureTemplate const *cinfo = sObjectMgr->GetCreatureTemplate(id))
        pCreature->SetUInt32Value(OBJECT_FIELD_ENTRY, id);
    else
        PSendSysMessage("Creature template with id %u not found", id);

    return true;
}

bool ChatHandler::HandleNpcSetInstanceEventCommand(const char* args)
{
    ARGS_CHECK
    
    Creature* target = GetSelectedCreature();
    if (!target || (target && target->GetTypeId() != TYPEID_UNIT)) {
        PSendSysMessage("Vous devez sélectionner une créature.");
        return true;
    }
    
    char* eventIdStr = strtok((char*)args, " ");
    if (!eventIdStr)
        return false;
        
    int eventId = atoi(eventIdStr);
    
    if (eventId == -1) {
        WorldDatabase.PExecute("DELETE FROM creature_encounter_respawn WHERE guid = %u", target->GetSpawnId());
        return true;
    }
    
    WorldDatabase.PExecute("REPLACE INTO creature_encounter_respawn VALUES (%u, %u)", target->GetSpawnId(), eventId);
    PSendSysMessage("Creature (%u) respawn linked to event %u.",target->GetSpawnId(),eventId);
    
    return true;
}

/* Syntax : .npc path type [PathType] 
No arguments means print current type. This DOES NOT update values in db, use .path type to do so.

Possible types :
0 - WP_PATH_TYPE_LOOP
1 - WP_PATH_TYPE_ONCE
2 - WP_PATH_TYPE_ROUND_TRIP
*/
bool ChatHandler::HandleNpcPathTypeCommand(const char* args)
{
    Creature* target = GetSelectedCreature();
    if(!target)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        return true;
    }

    if(target->GetMotionMaster()->GetCurrentMovementGeneratorType() != WAYPOINT_MOTION_TYPE)
    {
        SendSysMessage("Creature is not using waypoint movement generator.");
        return true;
    }

    auto movGenerator = dynamic_cast<WaypointMovementGenerator<Creature>*>(target->GetMotionMaster()->top());
    if(!movGenerator)
    {
        SendSysMessage("Could not get movement generator.");
        return true;
    }

    if(!*args)
    { //getter
        WaypointPathType type = movGenerator->GetPathType();
        std::string pathTypeStr = GetWaypointPathTypeName(type);
        PSendSysMessage("Creature waypoint movement type : %s (%u).", pathTypeStr.c_str(), type);
    } else 
    { //setter
        uint32 type = (uint32)atoi(args);
        bool ok = movGenerator->SetPathType(WaypointPathType(type));
        if(!ok)
        {
            PSendSysMessage("Wrong type given : %u.", type);
            return false;
        }
        std::string pathTypeStr = GetWaypointPathTypeName(WaypointPathType(type));
        PSendSysMessage("Target creature path type set to %s (%u).", pathTypeStr.c_str(), type);
    }
    return true;
}

/* Syntax : .npc path direction [PathDirection]
No arguments means print current direction. This DOES NOT update values in db, use .path direction to do so.

Possible directions :
0 - WP_PATH_DIRECTION_NORMAL
1 - WP_PATH_DIRECTION_REVERSE
*/
bool ChatHandler::HandleNpcPathDirectionCommand(const char* args)
{
    Creature* target = GetSelectedCreature();
    if(!target)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        return true;
    }

    if(target->GetMotionMaster()->GetCurrentMovementGeneratorType() != WAYPOINT_MOTION_TYPE)
    {
        SendSysMessage("Creature is not using waypoint movement generator.");
        return true;
    }

    auto movGenerator = dynamic_cast<WaypointMovementGenerator<Creature>*>(target->GetMotionMaster()->top());
    if(!movGenerator)
    {
        SendSysMessage("Could not get movement generator.");
        return true;
    }

    if(!*args)
    { //getter
        WaypointPathDirection dir = movGenerator->GetPathDirection();
        std::string pathDirStr = GetWaypointPathDirectionName(WaypointPathDirection(dir));
        PSendSysMessage("Creature waypoint movement direction : %s (%u).", pathDirStr.c_str(), dir);
    } else 
    { //setter
        uint32 dir = (uint32)atoi(args);
        bool ok = movGenerator->SetDirection(WaypointPathDirection(dir));
        if(!ok)
        {
            PSendSysMessage("Wrong direction given : %u.", dir);
            return false;
        }
        std::string pathDirStr = GetWaypointPathDirectionName(WaypointPathDirection(dir));
        PSendSysMessage("Target creature path direction set to %s (%u).", pathDirStr.c_str(), dir);
    }
    return true;
}

/* Syntax : .npc path currentid */
bool ChatHandler::HandleNpcPathCurrentIdCommand(const char* args)
{
    Creature* target = GetSelectedCreature();
    if(!target)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        return true;
    }

    uint32 pathId = target->GetWaypointPath();
    PSendSysMessage("Target creature current path id : %u.", pathId);

    return true;
}

//set spawn dist of creature
bool ChatHandler::HandleNpcSpawnDistCommand(const char* args)
{
    ARGS_CHECK

    float option = atof((char*)args);
    if (option < 0.0f)
    {
        SendSysMessage(LANG_BAD_VALUE);
        return false;
    }

    MovementGeneratorType mtype = IDLE_MOTION_TYPE;
    if (option >0.0f)
        mtype = RANDOM_MOTION_TYPE;

    Creature *pCreature = GetSelectedCreature();
    ObjectGuid::LowType u_guidlow = 0;

    if (pCreature)
        u_guidlow = pCreature->GetSpawnId();
    else
        return false;

    pCreature->SetRespawnRadius((float)option);
    pCreature->SetDefaultMovementType(mtype);
    pCreature->GetMotionMaster()->Initialize();
    if(pCreature->IsAlive())                                // dead creature will reset movement generator at respawn
    {
        pCreature->SetDeathState(JUST_DIED);
        pCreature->Respawn();
    }

    WorldDatabase.PExecute("UPDATE creature SET spawndist=%f, MovementType=%i WHERE guid=%u",option,mtype,u_guidlow);
    PSendSysMessage(LANG_COMMAND_SPAWNDIST,option);
    return true;
}

bool ChatHandler::HandleNpcSpawnTimeCommand(const char* args)
{
    ARGS_CHECK

    char* stime = strtok((char*)args, " ");

    if (!stime)
        return false;

    int i_stime = atoi((char*)stime);

    if (i_stime < 0)
    {
        SendSysMessage(LANG_BAD_VALUE);
        SetSentErrorMessage(true);
        return false;
    }

    Creature *pCreature = GetSelectedCreature();
    ObjectGuid::LowType u_guidlow = 0;

    if (pCreature)
        u_guidlow = pCreature->GetSpawnId();
    else
        return false;

    WorldDatabase.PExecute("UPDATE creature SET spawntimesecs=%i WHERE guid=%u",i_stime,u_guidlow);
    pCreature->SetRespawnDelay((uint32)i_stime);
    PSendSysMessage(LANG_COMMAND_SPAWNTIME,i_stime);

    return true;
}

//set faction of creature
bool ChatHandler::HandleNpcFactionIdCommand(const char* args)
{
    ARGS_CHECK

    uint32 factionId = (uint32) atoi((char*)args);

    if (!sFactionTemplateStore.LookupEntry(factionId))
    {
        PSendSysMessage(LANG_WRONG_FACTION, factionId);
        SetSentErrorMessage(true);
        return false;
    }

    Creature* pCreature = GetSelectedCreature();

    if(!pCreature)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        SetSentErrorMessage(true);
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
bool ChatHandler::HandleNpcAddVendorItemCommand(const char* args)
{
    ARGS_CHECK

    char* pitem  = extractKeyFromLink((char*)args,"Hitem");
    if (!pitem)
    {
        SendSysMessage(LANG_COMMAND_NEEDITEMSEND);
        SetSentErrorMessage(true);
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

    Creature* vendor = GetSelectedCreature();

    uint32 vendor_entry = vendor ? vendor->GetEntry() : 0;

    ItemTemplate const* pProto = sObjectMgr->GetItemTemplate(itemId);
    if(!pProto)
    {
        PSendSysMessage("Invalid id");
        return true;
    }

    if(!sObjectMgr->IsVendorItemValid(vendor_entry,pProto,maxcount,incrtime,extendedcost,m_session->GetPlayer()))
    {
        SetSentErrorMessage(true);
        return false;
    }

    sObjectMgr->AddVendorItem(vendor_entry,pProto,maxcount,incrtime,extendedcost);


    PSendSysMessage(LANG_ITEM_ADDED_TO_LIST,itemId,pProto->Name1.c_str(),maxcount,incrtime,extendedcost);
    return true;
}
