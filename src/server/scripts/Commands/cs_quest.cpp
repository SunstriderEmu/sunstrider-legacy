#include "Chat.h"
#include "Language.h"
#include "CharacterCache.h"

static bool HandleQuestAddCommand(ChatHandler* handler, char const* args)
{
    Player* player = GetSelectedPlayerOrSelf();
    if(!player)
    {
        handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
        handler->SetSentErrorMessage(true);
        return false;
    }

    // .addquest #entry'
    // number or [name] Shift-click form |color|Hquest:quest_id|h[name]|h|r
    char* cId = extractKeyFromLink((char*)args,"Hquest");
    if(!cId)
        return false;

    uint32 entry = atol(cId);

    Quest const* pQuest = sObjectMgr->GetQuestTemplate(entry);

    if(!pQuest)
    {
        handler->PSendSysMessage(LANG_COMMAND_QUEST_NOTFOUND,entry);
        handler->SetSentErrorMessage(true);
        return false;
    }

    // check item starting quest (it can work incorrectly if added without item in inventory)
    ItemTemplateContainer const& its = sObjectMgr->GetItemTemplateStore();
    for (const auto & it : its)
    {
        ItemTemplate const *pProto = &(it.second);
        if (!pProto)
            continue;

        if (pProto->StartQuest == entry)
        {
            handler->PSendSysMessage(LANG_COMMAND_QUEST_STARTFROMITEM, entry, pProto->ItemId);
            handler->SetSentErrorMessage(true);
            return false;
        }
    }

    // ok, normal (creature/GO starting) quest
    if( player->CanAddQuest( pQuest, true ) )
    {
        player->AddQuest( pQuest, nullptr );

        if ( player->CanCompleteQuest( entry ) )
            player->CompleteQuest( entry );

        if (   sWorld->getConfig(CONFIG_BUGGY_QUESTS_AUTOCOMPLETE)
                && !(pQuest->IsDaily())
                && !(pQuest->GetType() == QUEST_TYPE_RAID)
                && !(pQuest->GetType() == QUEST_TYPE_DUNGEON)
                && pQuest->IsMarkedAsBugged()
                )
        {
            player->AutoCompleteQuest(pQuest);
        }
    }

    return true;
}

static bool HandleQuestRemoveCommand(ChatHandler* handler, char const* args)
{
    Player* player = GetSelectedPlayerOrSelf();
    if(!player)
    {
        handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
        handler->SetSentErrorMessage(true);
        return false;
    }

    // .removequest #entry'
    // number or [name] Shift-click form |color|Hquest:quest_id|h[name]|h|r
    char* cId = extractKeyFromLink((char*)args,"Hquest");
    if(!cId)
        return false;

    uint32 entry = atol(cId);

    Quest const* pQuest = sObjectMgr->GetQuestTemplate(entry);

    if(!pQuest)
    {
        handler->PSendSysMessage(LANG_COMMAND_QUEST_NOTFOUND, entry);
        handler->SetSentErrorMessage(true);
        return false;
    }

    // remove all quest entries for 'entry' from quest log
    for(uint8 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot )
    {
        uint32 quest = player->GetQuestSlotQuestId(slot);
        if(quest==entry)
        {
            player->SetQuestSlot(slot,0);

            // we ignore unequippable quest items in this case, its' still be equipped
            player->TakeQuestSourceItem( quest, false );
        }
    }

    // set quest status to not started (will updated in DB at next save)
    player->SetQuestStatus( entry, QUEST_STATUS_NONE);

    // reset rewarded for restart repeatable quest
    player->getQuestStatusMap()[entry].m_rewarded = false;

    handler->SendSysMessage(LANG_COMMAND_QUEST_REMOVED);
    return true;
}

static bool HandleQuestCompleteCommand(ChatHandler* handler, char const* args)
{
    Player* player = GetSelectedPlayerOrSelf();
    if(!player)
    {
        handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
        handler->SetSentErrorMessage(true);
        return false;
    }

    // .quest complete #entry
    // number or [name] Shift-click form |color|Hquest:quest_id|h[name]|h|r
    char* cId = extractKeyFromLink((char*)args,"Hquest");
    if(!cId)
        return false;

   // bool forceComplete = false;
    uint32 entry = 0;

    //if gm wants to force quest completion
    /*if( strcmp(cId, "force") == 0 )
    {
        char* tail = strtok(NULL,"");
        if(!tail)
            return false;
        cId = extractKeyFromLink(tail,"Hquest");
        if(!cId)
            return false;

        entry = atoi(cId);
        //TC_LOG_ERROR("command","DEBUG: ID value: %d", tEntry);
        if(!entry)
            return false;
            
        forceComplete = true;
    }
    else */
        entry = atol(cId);

    Quest const* pQuest = sObjectMgr->GetQuestTemplate(entry);

    // If player doesn't have the quest
    if(!pQuest || player->GetQuestStatus(entry) == QUEST_STATUS_NONE)
    {
        handler->PSendSysMessage(LANG_COMMAND_QUEST_NOTFOUND, entry);
        handler->SetSentErrorMessage(true);
        return false;
    }
    
    QueryResult result = CharacterDatabase.PQuery("SELECT count FROM completed_quests WHERE guid = %u", player->GetGUID());
    
    uint8 completedQuestsThisWeek;
    if (result)
    {
        Field *fields = result->Fetch();
        completedQuestsThisWeek = fields[0].GetUInt8();
    }
    else //player has no completed quest this week
    {
        completedQuestsThisWeek = 0;
    }
 /* Limit validations per week : DISABLED
    if (completedQuestsThisWeek >= 2 && !forceComplete) //TODO: set a config option here ?
    {
        //tell the GM that this player has reached the maximum quests complete for this week
        handler->PSendSysMessage(LANG_REACHED_QCOMPLETE_LIMIT, player->GetName().c_str());
        handler->SetSentErrorMessage(true);
        return true;
    }*/
    // Add quest items for quests that require items
    for(uint8 x = 0; x < QUEST_OBJECTIVES_COUNT; ++x)
    {
        uint32 id = pQuest->RequiredItemId[x];
        uint32 count = pQuest->RequiredItemCount[x];
        if(!id || !count)
            continue;

        uint32 curItemCount = player->GetItemCount(id,true);

        ItemPosCountVec dest;
        uint8 msg = player->CanStoreNewItem( NULL_BAG, NULL_SLOT, dest, id, count-curItemCount );
        if( msg == EQUIP_ERR_OK )
        {
            Item* item = player->StoreNewItem( dest, id, true);
            player->SendNewItem(item,count-curItemCount,true,false);
        }
    }

    // All creature/GO slain/casted (not required, but otherwise it will display "Creature slain 0/10")
    for(uint8 i = 0; i < QUEST_OBJECTIVES_COUNT; i++)
    {
        int32 creatureOrGo = pQuest->RequiredNpcOrGo[i];
        uint32 creaturecount = pQuest->RequiredNpcOrGoCount[i];

        if(uint32 spell_id = pQuest->ReqSpell[i])
        {
            for(uint16 z = 0; z < creaturecount; ++z)
                player->CastedCreatureOrGO(creatureOrGo, ObjectGuid::Empty,spell_id);
        }
        else if(creatureOrGo > 0)
        {
            for(uint16 z = 0; z < creaturecount; ++z)
                player->KilledMonsterCredit(creatureOrGo, ObjectGuid::Empty);
        }
        else if(creatureOrGo < 0)
        {
            for(uint16 z = 0; z < creaturecount; ++z)
                player->CastedCreatureOrGO(creatureOrGo, ObjectGuid::Empty,0);
        }
    }

    // If the quest requires reputation to complete
    if(uint32 repFaction = pQuest->GetRepObjectiveFaction())
    {
        uint32 repValue = pQuest->GetRepObjectiveValue();
        uint32 curRep = player->GetReputation(repFaction);
        if(curRep < repValue)
        {
            FactionEntry const *factionEntry = sFactionStore.LookupEntry(repFaction);
            player->SetFactionReputation(factionEntry,repValue);
        }
    }

    // If the quest requires money
    int32 ReqOrRewMoney = pQuest->GetRewOrReqMoney();
    if(ReqOrRewMoney < 0)
        player->ModifyMoney(-ReqOrRewMoney);

    player->CompleteQuest(entry);
    handler->PSendSysMessage(LANG_QCOMPLETE_SUCCESS, entry, player->GetName().c_str()); //tell GM that the quest has been successfully completed
    
    if (completedQuestsThisWeek == 0) //entry does not exist, we have to create it
    {
        CharacterDatabase.PExecute("INSERT INTO completed_quests VALUES(%u, 1)", player->GetGUID());
    }
    else //entry exists, we have just to update it
    {
        CharacterDatabase.PExecute("UPDATE completed_quests SET count = count + 1 WHERE guid = %u", player->GetGUID());
    }
    
    // Check if quest already exists in bugged quests
    QueryResult questbug = WorldDatabase.PQuery("SELECT bugged FROM quest_bugs WHERE entry = %u", entry);
    if (questbug) 
    {
        WorldDatabase.PExecute("UPDATE quest_bugs SET completecount = completecount+1 WHERE entry = %u", entry);
    } else {
        WorldDatabase.PExecute("INSERT INTO quest_bugs VALUES (%u, 1, 0, '')", entry);
    }
    
    return true;
}

static bool HandleQuestReportCommand(ChatHandler* handler, char const* args)
{
    ARGS_CHECK
        
    char* questIdStr = strtok((char*)args, " ");
    if (!questIdStr)
        return false;
    uint32 questId = atoi(questIdStr);
    if (!questId)
        return false;
        
    char* comment = strtok(nullptr, "");
    if (!comment)
        return false;
        
    WorldDatabase.PQuery("UPDATE quest_bugs SET comment = \"%s\" WHERE entry = %u", comment, questId);
    
    return true;
}

//shows the number of completed quest this week, for selected character
static bool HandleQuestCountCompleteCommand(ChatHandler* handler, char const* args)
{
    Player* player;
    ObjectGuid targetGUID;
    if(!*args) //if no name provided, check if we have a player on target
    {
        player = GetSelectedPlayerOrSelf();
        if(!player)
        {
            handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
            handler->SetSentErrorMessage(true);
            return false;
        }
        
        targetGUID = player->GetGUID();
    }
    else
    {
        std::string name = args;

        if(!normalizePlayerName(name))
        {
            handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
            handler->SetSentErrorMessage(true);
            return false;
        }

        player = ObjectAccessor::FindConnectedPlayerByName(name.c_str());
        if (player)
        {
            targetGUID = player->GetGUID();
        }
        else //player is not online, get GUID with another function
        {
            targetGUID = sCharacterCache->GetCharacterGuidByName(name);
            if (!targetGUID) //player doesn't exist
            {
                handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
                handler->SetSentErrorMessage(true);
                return false;
            }
        }
    }
    
    QueryResult result = CharacterDatabase.PQuery("SELECT count FROM completed_quests WHERE guid = %u", targetGUID);
    uint8 completedQuestsThisWeek;
    if (result)
    {
        Field *fields = result->Fetch();
        completedQuestsThisWeek = fields[0].GetUInt8();
    }
    else //player has no completed quest this week
    {
        completedQuestsThisWeek = 0;
    }

    std::string displayName;
    bool nameResult = sCharacterCache->GetCharacterNameByGuid(targetGUID, displayName);
    if (!nameResult)
        displayName = "<Unknown>";

    handler->PSendSysMessage(LANG_QCOMPLETE_THIS_WEEK, displayName.c_str(), completedQuestsThisWeek);
    return true;
}

//shows the total number of quests completed by all gamemasters this week
static bool HandleQuestTotalCountCommand(ChatHandler* handler, char const* args)
{
    QueryResult result = CharacterDatabase.PQuery("SELECT SUM(count) FROM completed_quests");
    uint32 totalQuestsCompletedThisWeek;
    if (result)
    {
        Field* fields = result->Fetch();
        totalQuestsCompletedThisWeek = fields[0].GetUInt32();
    }
    else
        totalQuestsCompletedThisWeek = 0;
        
    handler->PSendSysMessage(LANG_QCOMPLETE_TOTAL, totalQuestsCompletedThisWeek);
    return true;
}
