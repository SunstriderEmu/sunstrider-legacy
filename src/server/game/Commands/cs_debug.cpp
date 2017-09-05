#include "Chat.h"
#include "Language.h"
#include <fstream>
#include "UpdateFieldsDebug.h"
#include "BattleGroundMgr.h"
#include "ChannelMgr.h"
#include "GossipDef.h"

//FIXME: not working for float values
void FillSnapshotValues(Unit* target, std::vector<uint32>& values)
{
    uint32 valuesCount = target->GetValuesCount();
    values.clear();
    values.resize(valuesCount);
    for (uint32 i = 0; i < valuesCount; i++)
        values[i] = target->GetUInt32Value(i);
}

bool ChatHandler::HandleDebugUpdateCommand(const char* args)
{
    ARGS_CHECK

    uint32 updateIndex;
    uint32 value;

    char* pUpdateIndex = strtok((char*)args, " ");

    Unit* chr = GetSelectedUnit();
    if (chr == nullptr)
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    if(!pUpdateIndex)
    {
        return true;
    }
    updateIndex = atoi(pUpdateIndex);
    //check updateIndex
    if(chr->GetTypeId() == TYPEID_PLAYER)
    {
        if (updateIndex>=PLAYER_END) return true;
    }
    else
    {
        if (updateIndex>=UNIT_END) return true;
    }

    char*  pvalue = strtok(nullptr, " ");
    if (!pvalue)
    {
        value=chr->GetUInt32Value(updateIndex);

        PSendSysMessage(LANG_UPDATE, chr->GetGUIDLow(),updateIndex,value);
        return true;
    }

    value=atoi(pvalue);

    PSendSysMessage(LANG_UPDATE_CHANGE, chr->GetGUIDLow(),updateIndex,value);

    chr->SetUInt32Value(updateIndex,value);

    return true;
}

/** Syntax: .debug batchattack <count> [type]
With type:
0 - Basic attack
1 - Offhand attack
2 - Ranged attack
*/
bool ChatHandler::HandleDebugBatchAttack(const char* args)
{
    char* cCount = strtok((char*)args, " ");
    if(!cCount)
        return false;

    uint32 count = atoi(cCount);
    if(count == 0)
        return false;

    if(count > 5000)
    {
        PSendSysMessage("Plz stahp abusing");
        return true;
    }

    WeaponAttackType type = BASE_ATTACK;
    if(char* cType = strtok(nullptr, ""))
    {
        type = WeaponAttackType(atoi(cType));
        if(type > RANGED_ATTACK)
            return false;
    }

    Player* p = m_session->GetPlayer();
    if(Unit* victim = p->GetVictim())
    {
        if(type != RANGED_ATTACK)
        {
            for(uint32 i = 0; i < count; i++)
                p->AttackerStateUpdate(victim, type);
        } else {
            for(uint32 i = 0; i < count; i++)
                p->CastSpell(victim, 75, true); //shoot
        }
    } else {
        PSendSysMessage("No victim");
    }

    return true;
}

bool ChatHandler::HandleDebugInArcCommand(const char* /*args*/)
{
    Object *obj = GetSelectedUnit();

    if(!obj)
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        return true;
    }

    SendSysMessage(LANG_NOT_IMPLEMENTED);

    return true;
}

bool ChatHandler::HandleDebugSpellFailCommand(const char* args)
{
    char* px = strtok((char*)args, " ");
    if(!px)
        return false;

    uint8 failnum = (uint8)atoi(px);

    WorldPacket data(SMSG_CAST_FAILED, 5);
    data << uint32(133); //spell 133 ?
    data << uint8(failnum);
    m_session->SendPacket(&data);

    return true;
}

bool ChatHandler::HandleDebugSetPoiCommand(const char* args)
{
    Player *pPlayer = m_session->GetPlayer();
    Unit* target = GetSelectedUnit();
    if(!target)
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        return true;
    }

    char* icon_text = strtok((char*)args, " ");
    char* flags_text = strtok(nullptr, " ");
    if(!icon_text || !flags_text)
        return false;

    uint32 icon = atol(icon_text);
    uint32 flags = atol(flags_text);

    TC_LOG_DEBUG("battleground","Command : POI, NPC = %u, icon = %u flags = %u", target->GetGUIDLow(), icon,flags);
    pPlayer->PlayerTalkClass->SendPointOfInterest(target->GetPositionX(), target->GetPositionY(), Poi_Icon(icon), flags, 30, "Test POI");
    return true;
}

bool ChatHandler::HandleDebugSendQuestPartyMsgCommand(const char* args)
{
    uint32 msg = atol((char*)args);
    m_session->GetPlayer()->SendPushToPartyResponse(m_session->GetPlayer(), msg);
    return true;
}

bool ChatHandler::HandleDebugSendQuestInvalidMsgCommand(const char* args)
{
    uint32 msg = atol((char*)args);
    m_session->GetPlayer()->SendCanTakeQuestResponse(msg);
    return true;
}

bool ChatHandler::HandleDebugEquipErrorCommand(const char* args)
{
    uint8 msg = atoi(args);
    m_session->GetPlayer()->SendEquipError(msg, nullptr, nullptr);
    return true;
}

bool ChatHandler::HandleDebugSellErrorCommand(const char* args)
{
    uint8 msg = atoi(args);
    m_session->GetPlayer()->SendSellError(msg, nullptr, 0, 0);
    return true;
}

bool ChatHandler::HandleDebugBuyErrorCommand(const char* args)
{
    uint8 msg = atoi(args);
    m_session->GetPlayer()->SendBuyError(msg, nullptr, 0, 0);
    return true;
}

bool ChatHandler::HandleDebugSendOpcodeCommand(const char* /*args*/)
{
    Unit *unit = GetSelectedUnit();
    Player *player = nullptr;
    if (!unit || (unit->GetTypeId() != TYPEID_PLAYER))
        player = m_session->GetPlayer();
    else
        player = unit->ToPlayer();
    if(!unit) unit = player;

    std::ifstream ifs("opcode.txt");
    if(ifs.bad())
        return false;

    uint32 opcode;
    ifs >> opcode;

    WorldPacket data(opcode, 0);

    while(!ifs.eof())
    {
        std::string type;
        ifs >> type;

        if(type == "")
            break;

        if(type == "uint8")
        {
            uint16 val1;
            ifs >> val1;
            data << uint8(val1);
        }
        else if(type == "uint16")
        {
            uint16 val2;
            ifs >> val2;
            data << val2;
        }
        else if(type == "uint32")
        {
            uint32 val3;
            ifs >> val3;
            data << val3;
        }
        else if(type == "uint64")
        {
            uint64 val4;
            ifs >> val4;
            data << val4;
        }
        else if(type == "float")
        {
            float val5;
            ifs >> val5;
            data << val5;
        }
        else if(type == "string")
        {
            std::string val6;
            ifs >> val6;
            data << val6;
        }
        else if(type == "pguid")
        {
            data << unit->GetPackGUID();
        }
        else if(type == "myguid")
        {
            data << player->GetPackGUID();
        }
        else if(type == "pos")
        {
            data << unit->GetPositionX();
            data << unit->GetPositionY();
            data << unit->GetPositionZ();
        }
        else if(type == "mypos")
        {
            data << player->GetPositionX();
            data << player->GetPositionY();
            data << player->GetPositionZ();
        }
        else
        {
            break;
        }
    }
    ifs.close();
    TC_LOG_DEBUG("network.opcode","Sending opcode %u", data.GetOpcode());
    data.hexlike();
    (unit->ToPlayer())->SendDirectMessage(&data);
    PSendSysMessage(LANG_COMMAND_OPCODESENT, data.GetOpcode(), unit->GetName().c_str());
    return true;
}

bool ChatHandler::HandleUpdateWorldStateCommand(const char* args)
{
    char* w = strtok((char*)args, " ");
    char* s = strtok(nullptr, " ");

    if (!w || !s)
        return false;

    uint32 world = (uint32)atoi(w);
    uint32 state = (uint32)atoi(s);
    m_session->GetPlayer()->SendUpdateWorldState(world, state);
    return true;
}

bool ChatHandler::HandleDebugPlaySound2Command(const char* args)
{
    uint32 soundid = atoi(args);
    m_session->GetPlayer()->PlaySound(soundid, false);
    return true;
}

//Send notification in channel
bool ChatHandler::HandleDebugSendChannelNotifyCommand(const char* args)
{
    const char *name = "test";
    uint8 code = atoi(args);

    WorldPacket data(SMSG_CHANNEL_NOTIFY, (1+10));
    data << code;                                           // notify type
    data << name;                                           // channel name
    data << uint32(0);
    data << uint32(0);
    m_session->SendPacket(&data);
    return true;
}

//Send notification in chat
bool ChatHandler::HandleDebugSendChatMsgCommand(const char* args)
{
    const char *msg = "testtest";
    ChatMsg type = ChatMsg(atoi(args));
    WorldPacket data;
    ChatHandler::BuildChatPacket(data, ChatMsg(type), LANG_UNIVERSAL, m_session->GetPlayer(), m_session->GetPlayer(), msg);
    m_session->SendPacket(&data);
    return true;
}

bool ChatHandler::HandleDebugGetItemState(const char* args)
{
    std::string state_str = args;

    ItemUpdateState state = ITEM_UNCHANGED;
    bool list_queue = false, check_all = false;
    if (state_str == "unchanged") state = ITEM_UNCHANGED;
    else if (state_str == "changed") state = ITEM_CHANGED;
    else if (state_str == "new") state = ITEM_NEW;
    else if (state_str == "removed") state = ITEM_REMOVED;
    else if (state_str == "queue") list_queue = true;
    else if (state_str == "check_all") check_all = true;
    else return false;

    Player* player = GetSelectedPlayer();
    if (!player) player = m_session->GetPlayer();

    if (!list_queue && !check_all)
    {
        state_str = "The player has the following " + state_str + " items: ";
        SendSysMessage(state_str.c_str());
        for (uint8 i = PLAYER_SLOT_START; i < PLAYER_SLOT_END; i++)
        {
            if(i >= BUYBACK_SLOT_START && i < BUYBACK_SLOT_END)
                continue;

            Item *item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i);
            if (!item) 
                continue;

            if (!item->IsBag())
            {
                if (item->GetState() == state)
                    PSendSysMessage("bag: 255 slot: %d guid: %d owner: %d", item->GetSlot(), item->GetGUIDLow(), GUID_LOPART(item->GetOwnerGUID()));
            }
            else
            {
                Bag* bag = (Bag*)item;
                for (uint8 j = 0; j < bag->GetBagSize(); ++j)
                {
                    item = bag->GetItemByPos(j);
                    if (item && item->GetState() == state)
                        PSendSysMessage("bag: 255 slot: %d guid: %d owner: %d", item->GetSlot(), item->GetGUIDLow(), GUID_LOPART(item->GetOwnerGUID()));
                }
            }
        }
    }

    if (list_queue)
    {
        std::vector<Item *> &updateQueue = player->GetItemUpdateQueue();
        for(auto item : updateQueue)
        {
            if(!item) continue;

            Bag *container = item->GetContainer();
            uint8 bag_slot = container ? container->GetSlot() : uint8(INVENTORY_SLOT_BAG_0);

            std::string st;
            switch(item->GetState())
            {
                case ITEM_UNCHANGED: st = "unchanged"; break;
                case ITEM_CHANGED: st = "changed"; break;
                case ITEM_NEW: st = "new"; break;
                case ITEM_REMOVED: st = "removed"; break;
            }

            PSendSysMessage("Bag: %d slot: %d guid: %d - state: %s", bag_slot, item->GetSlot(), item->GetGUIDLow(), st.c_str());
        }
        if (updateQueue.empty())
            PSendSysMessage("updatequeue empty");
    }

    if (check_all)
    {
        bool error = false;
        std::vector<Item *> &updateQueue = player->GetItemUpdateQueue();
        for (uint8 i = PLAYER_SLOT_START; i < PLAYER_SLOT_END; i++)
        {
            if(i >= BUYBACK_SLOT_START && i < BUYBACK_SLOT_END)
                continue;

            Item *item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i);
            if (!item) 
                continue;

            if (item->GetSlot() != i)
            {
                PSendSysMessage("Item at slot %d, guid %d has an incorrect slot value: %d", i, item->GetGUIDLow(), item->GetSlot());
                error = true; continue;
            }

            if (item->GetOwnerGUID() != player->GetGUID())
            {
                PSendSysMessage("For the item at slot %d and itemguid %d, owner's guid (%d) and player's guid (%d) don't match!", item->GetSlot(), item->GetGUIDLow(), GUID_LOPART(item->GetOwnerGUID()), player->GetGUIDLow());
                error = true; continue;
            }

            if (Bag *container = item->GetContainer())
            {
                PSendSysMessage("Item at slot: %d guid: %d has a container (slot: %d, guid: %d) but shouldnt!", item->GetSlot(), item->GetGUIDLow(), container->GetSlot(), container->GetGUIDLow());
                error = true; continue;
            }

            if (item->IsInUpdateQueue())
            {
                uint16 qp = item->GetQueuePos();
                if (qp > updateQueue.size())
                {
                    PSendSysMessage("Item at slot: %d guid: %d has a queuepos (%d) larger than the update queue size! ", item->GetSlot(), item->GetGUIDLow(), qp);
                    error = true; continue;
                }

                if (updateQueue[qp] == nullptr)
                {
                    PSendSysMessage("Item at slot: %d guid: %d has a queuepos (%d) that points to NULL in the queue!", item->GetSlot(), item->GetGUIDLow(), qp);
                    error = true; continue;
                }

                if (updateQueue[qp] != item)
                {
                    PSendSysMessage("Item at slot: %d guid: %d has has a queuepos (%d) that points to another item in the queue (bag: %d, slot: %d, guid: %d)", item->GetSlot(), item->GetGUIDLow(), qp, updateQueue[qp]->GetBagSlot(), updateQueue[qp]->GetSlot(), updateQueue[qp]->GetGUIDLow());
                    error = true; continue;
                }
            }
            else if (item->GetState() != ITEM_UNCHANGED)
            {
                PSendSysMessage("Item at slot: %d guid: %d is not in queue but should be (state: %d)!", item->GetSlot(), item->GetGUIDLow(), item->GetState());
                error = true; continue;
            }

            if(item->IsBag())
            {
                Bag *bag = (Bag*)item;
                for (uint8 j = 0; j < bag->GetBagSize(); ++j)
                {
                    item = bag->GetItemByPos(j);
                    if (!item) 
                        continue;

                    if (item->GetSlot() != j)
                    {
                        PSendSysMessage("The item in bag %d slot %d, guid %d has an incorrect slot value: %d", bag->GetSlot(), j, item->GetGUIDLow(), item->GetSlot());
                        error = true; continue;
                    }

                    if (item->GetOwnerGUID() != player->GetGUID())
                    {
                        PSendSysMessage("For the item in bag %d at slot %d and itemguid %d, owner's guid (%d) and player's guid (%d) don't match!", bag->GetSlot(), item->GetSlot(), item->GetGUIDLow(), GUID_LOPART(item->GetOwnerGUID()), player->GetGUIDLow());
                        error = true; continue;
                    }

                    Bag *container = item->GetContainer();
                    if (!container)
                    {
                        PSendSysMessage("The item in bag %d at slot %d with guid %d has no container!", bag->GetSlot(), item->GetSlot(), item->GetGUIDLow());
                        error = true; continue;
                    }

                    if (container != bag)
                    {
                        PSendSysMessage("The item in bag %d at slot %d with guid %d has a different container(slot %d guid %d)!", bag->GetSlot(), item->GetSlot(), item->GetGUIDLow(), container->GetSlot(), container->GetGUIDLow());
                        error = true; continue;
                    }

                    if (item->IsInUpdateQueue())
                    {
                        uint16 qp = item->GetQueuePos();
                        if (qp > updateQueue.size())
                        {
                            PSendSysMessage("Item in bag: %d at slot: %d guid: %d has a queuepos (%d) larger than the update queue size! ", bag->GetSlot(), item->GetSlot(), item->GetGUIDLow(), qp);
                            error = true; continue;
                        }

                        if (updateQueue[qp] == nullptr)
                        {
                            PSendSysMessage("Item in bag: %d at slot: %d guid: %d has a queuepos (%d) that points to NULL in the queue!", bag->GetSlot(), item->GetSlot(), item->GetGUIDLow(), qp);
                            error = true; continue;
                        }

                        if (updateQueue[qp] != item)
                        {
                            PSendSysMessage("Item in bag: %d at slot: %d guid: %d has has a queuepos (%d) that points to another item in the queue (bag: %d, slot: %d, guid: %d)", bag->GetSlot(), item->GetSlot(), item->GetGUIDLow(), qp, updateQueue[qp]->GetBagSlot(), updateQueue[qp]->GetSlot(), updateQueue[qp]->GetGUIDLow());
                            error = true; continue;
                        }
                    }
                    else if (item->GetState() != ITEM_UNCHANGED)
                    {
                        PSendSysMessage("Item in bag: %d at slot: %d guid: %d is not in queue but should be (state: %d)!", bag->GetSlot(), item->GetSlot(), item->GetGUIDLow(), item->GetState());
                        error = true; continue;
                    }
                }
            }
        }

        for(size_t i = 0; i < updateQueue.size(); i++)
        {
            Item *item = updateQueue[i];
            if(!item) continue;

            if (item->GetOwnerGUID() != player->GetGUID())
            {
                PSendSysMessage("queue(%u): for the an item (guid %u), the owner's guid (%u) and player's guid (%u) don't match!", uint32(i), item->GetGUIDLow(), GUID_LOPART(item->GetOwnerGUID()), player->GetGUIDLow());
                error = true; continue;
            }

            if (item->GetQueuePos() != i)
            {
                PSendSysMessage("queue(%d): for the an item (guid %d), the queuepos doesn't match it's position in the queue!", uint32(i), item->GetGUIDLow());
                error = true; continue;
            }

            if (item->GetState() == ITEM_REMOVED) continue;
            Item *test = player->GetItemByPos( item->GetBagSlot(), item->GetSlot());

            if (test == nullptr)
            {
                PSendSysMessage("queue(%d): the bag(%d) and slot(%d) values for the item with guid %d are incorrect, the player doesn't have an item at that position!", (uint32)i, item->GetBagSlot(), item->GetSlot(), item->GetGUIDLow());
                error = true; continue;
            }

            if (test != item)
            {
                PSendSysMessage("queue(%d): the bag(%d) and slot(%d) values for the item with guid %d are incorrect, the item with guid %d is there instead!", (uint32)i, item->GetBagSlot(), item->GetSlot(), item->GetGUIDLow(), test->GetGUIDLow());
                error = true; continue;
            }
        }
        if (!error)
            SendSysMessage("All OK!");
    }

    return true;
}

bool ChatHandler::HandleDebugArenaCommand(const char * /*args*/)
{
    bool enabled = sBattlegroundMgr->ToggleArenaTesting();
    
    if(enabled)
        SendGlobalGMSysMessage("Arenas are set to minimum 1 player per team for debugging.");
    else
        SendGlobalGMSysMessage("Arenas are set to normal playercount.");

    return true;
}

bool ChatHandler::HandleDebugBattleGroundCommand(const char* )
{
    bool enabled = sBattlegroundMgr->ToggleBattleGroundTesting();

    if(enabled)
        SendGlobalGMSysMessage("BattleGrounds will now open with first player joining.");
    else
        SendGlobalGMSysMessage("BattleGrounds are set to normal playercount.");
    
    return true;
}

bool ChatHandler::HandleDebugThreatList(const char * args)
{
    Creature* target = GetSelectedCreature();
    if(!target || target->IsTotem() || target->IsPet())
        return false;

    uint32 limit = 0;
    if(args)
        limit = (uint32)atoi(args);

    std::list<HostileReference*>& tlist = target->getThreatManager().getThreatList();
    std::list<HostileReference*>::iterator itr;
    uint32 cnt = 0;
    PSendSysMessage("Threat list of %s (guid %u)",target->GetName().c_str(), target->GetGUIDLow());
    for(itr = tlist.begin(); itr != tlist.end(); ++itr)
    {
        Unit* unit = (*itr)->getTarget();
        if(!unit)
            continue;
        ++cnt;
        PSendSysMessage("   %u.   %s   (guid %u) - (entry %u) - threat %f",cnt,unit->GetName().c_str(), unit->GetGUIDLow(), unit->GetEntry(), (*itr)->getThreat());

        if (limit && cnt >= limit)
            break;
    }
    SendSysMessage("End of threat list.");
    return true;
}

bool ChatHandler::HandleDebugHostilRefList(const char * /*args*/)
{
    Unit* target = GetSelectedUnit();
    if(!target)
        target = m_session->GetPlayer();
    HostileReference* ref = target->GetHostileRefManager().getFirst();
    uint32 cnt = 0;
    PSendSysMessage("Hostil reference list of %s (guid %u)",target->GetName().c_str(), target->GetGUIDLow());
    while(ref)
    {
        if(Unit * unit = ref->GetSource()->GetOwner())
        {
            ++cnt;
            PSendSysMessage("   %u.   %s   (guid %u) - (entry %u) - threat %f",cnt,unit->GetName().c_str(), unit->GetGUIDLow(), unit->GetEntry(), ref->getThreat());
        }
        ref = ref->next();
    }
    SendSysMessage("End of hostil reference list.");
    return true;
}

bool ChatHandler::HandleDebugCinematic(const char* args)
{
    ARGS_CHECK

    WorldPacket data(SMSG_TRIGGER_CINEMATIC, 4);
    data << uint32(atoi(args));
    m_session->GetPlayer()->SendDirectMessage(&data);
    return true;
}

bool ChatHandler::HandleDebugItemByPos(const char* args)
{
    ARGS_CHECK
        
    uint32 pos = uint32(atoi(args));
    Item* item = m_session->GetPlayer()->GetItemByPos(pos);
    if (!item) {
        PSendSysMessage("Invalid position.");
        return true;
    }
    
    PSendSysMessage("Item found! Name: %s, level: %u", item->GetTemplate()->Name1.c_str(), item->GetTemplate()->ItemLevel);
    return true;
}

bool ChatHandler::HandleDebugItemLevelSum(const char* args)
{
    PSendSysMessage("Total stuff level: %u", m_session->GetPlayer()->GetEquipedItemsLevelSum());
    
    return true;
}

//Play sound
bool ChatHandler::HandleDebugPlaySoundCommand(const char* args)
{
    // USAGE: .debug playsound #soundid
    // #soundid - ID decimal number from SoundEntries.dbc (1st column)
    // this file have about 5000 sounds.
    // In this realization only caller can hear this sound.
    if( *args )
    {
        uint32 dwSoundId = atoi((char*)args);

        if( !sSoundEntriesStore.LookupEntry(dwSoundId) )
        {
            PSendSysMessage(LANG_SOUND_NOT_EXIST, dwSoundId);
            SetSentErrorMessage(true);
            return false;
        }

        m_session->SendSoundFromObject(m_session->GetPlayer()->GetGUID(), dwSoundId);

        PSendSysMessage(LANG_YOU_HEAR_SOUND, dwSoundId);
        return true;
    }

    return false;
}

/* Syntax : .debug setvalue #index #value [uint32/uint64/float]*/
bool ChatHandler::HandleDebugSetValueCommand(const char* args)
{
    ARGS_CHECK

    char* cIndex = strtok((char*)args, " ");
    char* cValue = strtok(nullptr, " ");
    char* cType = strtok(nullptr, " ");

    if (!cIndex || !cValue)
        return false;

    Unit* target = GetSelectedUnit();
    if(!target)
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    uint64 guid = target->GetGUID();

    uint32 index = (uint32)atoi(cIndex);
    if(index >= target->GetValuesCount())
    {
        PSendSysMessage(LANG_TOO_BIG_INDEX, index, GUID_LOPART(guid), target->GetValuesCount());
        return false;
    }
    uint64 uValue;
    float fValue;
    uint8 type = 0;
    if(cType)
    {
        if( strcmp(cType, "float") == 0 )
            type = 2;
        else if ( strcmp(cType, "uint64") == 0 )
            type = 1;
        else if ( strcmp(cType, "uint32") == 0 )
            type = 0;
    }

    switch(type)
    {
    case 0: //uint32
        {
        std::stringstream ss(cValue);
        ss >> uValue;
        target->SetUInt32Value(index,uValue);
        PSendSysMessage(LANG_SET_UINT_FIELD, GUID_LOPART(guid), index, uValue);
        }
        break;
    case 1: //uint64
        {
        std::stringstream ss(cValue);
        ss >> uValue;
        target->SetUInt64Value(index,uValue);
        PSendSysMessage("You set the uint64 value of %u in %u to " UI64FMTD, GUID_LOPART(guid), index, uValue);
        }
        break;
    case 2: //float
        fValue = (float)atof(cValue);
        target->SetFloatValue(index,fValue);
        PSendSysMessage(LANG_SET_FLOAT_FIELD, GUID_LOPART(guid), index, fValue);
        break;
    }

    return true;
}

/* Syntax : .debug getvalue #index */
bool ChatHandler::HandleDebugGetValueCommand(const char* args)
{
    ARGS_CHECK

    char* cIndex = strtok((char*)args, " ");

    if (!cIndex)
        return false;

    Unit* target = GetSelectedUnit();
    if (!target)
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    uint64 guid = target->GetGUID();

    uint32 index = (uint32)atoi(cIndex);
    if (index >= target->GetValuesCount())
    {
        PSendSysMessage(LANG_TOO_BIG_INDEX, index, GUID_LOPART(guid), target->GetValuesCount());
        return false;
    }

    std::vector<uint32> values;
    FillSnapshotValues(target, values); //todo: no need to fill this for all values

    TypeID type = TypeID(target->GetTypeId());
    std::string fieldName = "[UNKNOWN]";
    UpdateFieldsDebug::GetFieldNameString(type, index, fieldName);
    std::stringstream ss;
    UpdateFieldsDebug::InsertFieldInStream(type, index, values, ss);
    PSendSysMessage("Field %s (%u): %s", fieldName.c_str(), index, ss.str().c_str());

    return true;
}

/* Syntax: .debug crash <SIGSEGV|SEGABRT> */
bool ChatHandler::HandleDebugCrashCommand(const char* args)
{
    char* cSignal = strtok((char*)args, " ");
    if (!cSignal)
        return false;

    std::string signal(cSignal);

    if (signal == "SIGSEGV")
        raise(SIGSEGV);
    else if (signal == "SIGABRT")
        raise(SIGABRT);
    else
        return false;

    return true;
}

/* Syntax: .debug zonemusic #musicId */
bool ChatHandler::HandleDebugZoneMusicCommand(const char* args)
{
    char* cMusicId = strtok((char*)args, " ");

    if (!cMusicId)
        return false;

    uint32 musicId = (uint32)atoi(cMusicId);
    if (musicId == 0)
        return false;

    Player* p = GetSession()->GetPlayer();
    if (!p)
        return false;

    Map* map = p->GetMap();
    if (!map)
        return false;

    map->SetZoneMusic(p->GetZoneId(), musicId);
    PSendSysMessage("Changed zone music to %u", musicId);
    return true;
}

/* Syntax: .debug zonelight #newLightId */
bool ChatHandler::HandleDebugZoneLightCommand(const char* args)
{
    char* cLightId = strtok((char*)args, " ");

    if (!cLightId)
        return false;

    uint32 lightId = (uint32)atoi(cLightId);
    if (lightId == 0)
        return false;

    Player* p = GetSession()->GetPlayer();
    if (!p)
        return false;

    Map* map = p->GetMap();
    if (!map)
        return false;

    map->SetZoneOverrideLight(p->GetZoneId(), lightId, 10 * SECOND * IN_MILLISECONDS); //fade over 10 seconds
    PSendSysMessage("Changed zone light to %u", lightId);
    return true;
}

/* Syntax: .debug zoneweather #weatherId [#intensity] */
bool ChatHandler::HandleDebugZoneWeatherCommand(const char* args)
{
    char* sWeatherId = strtok((char*)args, " ");
    char* sIntensity = strtok(nullptr, " ");

    uint32 weatherId = 0;
    float intensity = 0.999f;

    if (!sWeatherId)
        return false;

    weatherId = atoi(sWeatherId);
    if (weatherId == 0)
        return false;

    if (sIntensity)
        intensity = atof(sIntensity);

    Player* p = GetSession()->GetPlayer();
    if (!p)
        return false;

    Map* map = p->GetMap();
    if (!map)
        return false;

    map->SetZoneWeather(p->GetZoneId(), WeatherState(weatherId), intensity);
    PSendSysMessage("Changed zone weather to %u", weatherId);
    return true;
}

/* Syntax: .debug setarmor #armorValue */
bool ChatHandler::HandleDebugSetArmorCommand(const char* args)
{
    Unit* target = GetSelectedUnit();
    if (!target)
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        return true;
    }

    uint32 armor = atoi(args);
    target->SetUInt32Value(UNIT_FIELD_RESISTANCES, armor);

    PSendSysMessage("Set target armor to %u", armor);

    return true;
}

/* Syntax: .debug getarmor */
bool ChatHandler::HandleDebugGetArmorCommand(const char* args)
{
    return HandleDebugGetValueCommand("186"); //186 == UNIT_FIELD_RESISTANCES
}

//show animation
bool ChatHandler::HandleDebugAnimCommand(const char* args)
{
    ARGS_CHECK

    uint32 anim_id = atoi((char*)args);
    m_session->GetPlayer()->HandleEmoteCommand(anim_id);
    return true;
}

bool ChatHandler::HandleDebugGetLootRecipient(const char* /*args*/)
{
    Creature* target = GetSelectedCreature();
    if(!target)
        return false;

    PSendSysMessage("Loot recipient: %s", target->hasLootRecipient()?(target->GetLootRecipient()?target->GetLootRecipient()->GetName().c_str() :"offline"):"no loot recipient");
    return true;
}

bool ChatHandler::HandleDebugRemoveLootItem(const char* args)
{
    ARGS_CHECK
    Creature* target = GetSelectedCreature();
    if (!target)
        return false;
        
    uint32 itemId = uint32(atoi(args));
    target->loot.RemoveItem(itemId);
    PSendSysMessage("Item removed.");
        
    return true;
}

bool ChatHandler::HandleDebugStealthLevel(const char* args)
{
    Unit *target = GetSelectedUnit();
    if (!target)
        target = m_session->GetPlayer();
        
    float modStealth = target->GetTotalAuraModifier(SPELL_AURA_MOD_STEALTH);
    float modStealthLevel = target->GetTotalAuraModifier(SPELL_AURA_MOD_STEALTH_LEVEL);
    float modDetect = target->GetTotalAuraModifier(SPELL_AURA_MOD_STEALTH_DETECT);

    PSendSysMessage("Stealth: %f - Stealth level: %f - Total: %f (Detect %f)", modStealth, modStealthLevel, (modStealth+modStealthLevel),modDetect);
    return true;
}

bool ChatHandler::HandleDebugAttackDistance(const char* args)
{
    Unit *target = GetSelectedUnit();
    if (!target || !target->ToCreature())
        return false;
        
    PSendSysMessage("AttackDistance: %f - ModDetectRange: %i", target->ToCreature()->GetAggroRange(m_session->GetPlayer()), target->GetTotalAuraModifier(SPELL_AURA_MOD_STEALTH_DETECT_RANGE));
    return true;
}

bool ChatHandler::HandleDebugPvPAnnounce(const char* args)
{
    ARGS_CHECK
        
    char *msg = strtok((char *)args, " ");
    if (!msg)
        return false;
        
    char const* channel = "pvp";
    
    boost::shared_lock<boost::shared_mutex> lock(*HashMapHolder<Player>::GetLock());
    HashMapHolder<Player>::MapType const& m = ObjectAccessor::GetPlayers();
    for(auto & itr : m)
    {
        if (itr.second && itr.second->GetSession()->GetPlayer() && itr.second->GetSession()->GetPlayer()->IsInWorld())
        {
            if(ChannelMgr* cMgr = channelMgr(itr.second->GetSession()->GetPlayer()->GetTeam()))
            {
                if(Channel *chn = cMgr->GetChannel(channel, itr.second->GetSession()->GetPlayer()))
                {
                    WorldPacket data;
                    ChatHandler::BuildChatPacket(data, CHAT_MSG_CHANNEL, LANG_UNIVERSAL, itr.second->GetSession()->GetPlayer(),itr.second->GetSession()->GetPlayer(), msg, 0, channel);
                    itr.second->SendDirectMessage(&data);
                }
            }
        }
    }
    
    return false;
}

bool ChatHandler::HandleDebugAurasList(const char* args)
{
    Unit* unit = GetSelectedUnit();
    if (!unit)
        unit = m_session->GetPlayer();
    
    PSendSysMessage("Aura list:");
    Unit::AuraMap& tAuras = unit->GetAuras();
    for (auto & tAura : tAuras)
    {
        SpellInfo const* spellProto = tAura.second->GetSpellInfo();
        PSendSysMessage("%u - %s (stack: %u) - Effect %u - Value %u %s", spellProto->Id, spellProto->SpellName[sWorld->GetDefaultDbcLocale()], tAura.second->GetStackAmount(), tAura.second->GetEffIndex(), tAura.second->GetModifierValue(), tAura.second->IsActive() ? "" : "[inactive]");
    }
    
    return true;
}

bool ChatHandler::HandleDebugLoadedScripts(const char* args)
{
    uint32 entries, guids;
    sObjectMgr->GetLoadedScriptsStats(entries, guids);
    
    PSendSysMessage("Loaded scripts stats: %u by entry, %u by GUID", entries, guids);
    
    return true;
}

bool ChatHandler::HandleDebugResetDailyQuests(const char* args)
{
    sWorld->ResetDailyQuests();

    return true;
}

bool ChatHandler::HandleDebugShowAttackers(const char* args)
{
    Unit* target = GetSelectedUnit();
    if (!target)
        return false;
        
    SendSysMessage("Attackers list:");
    char msg[256];
    for (auto itr : target->GetAttackers()) {
        if (itr->GetTypeId() == TYPEID_PLAYER)
            snprintf(msg, 256, "%s (Entry: 0 (Player), GUID: %u, Full GUID:" UI64FMTD ")", itr->GetName().c_str(), itr->GetGUIDLow(), itr->GetGUID());
        else
            snprintf(msg, 256, "%s (Entry: %u, GUID: %u, Full GUID:" UI64FMTD ")", itr->GetName().c_str(), itr->GetEntry(), itr->ToCreature()->GetSpawnId(), itr->GetGUID());
            
        SendSysMessage(msg);
    }
    
    return true;
}

bool ChatHandler::HandleDebugSendZoneUnderAttack(const char* args)
{
    char* zone = strtok((char*)args, " ");
    if (!zone)
        return false;
        
    uint32 zoneId = atoi(zone);
    if (!zoneId)
        return false;
    
    uint32 team = m_session->GetPlayer()->GetTeam();
    sWorld->SendZoneUnderAttack(zoneId, (team==ALLIANCE ? HORDE : ALLIANCE));
    return true;
}

bool ChatHandler::HandleDebugLoSCommand(const char* args)
{
    if (Unit* unit = GetSelectedUnit())
        PSendSysMessage("Unit %s (GuidLow: %u) is %sin LoS", unit->GetName().c_str(), unit->GetGUIDLow(), m_session->GetPlayer()->IsWithinLOSInMap(unit) ? "" : "not ");
        
    return true;
}

bool ChatHandler::HandleDebugPlayerFlags(const char* args)
{
    ARGS_CHECK
        
    int flags = atoi(args);
    if (!flags)
        return true;
        
    if (flags < 0)
        m_session->GetPlayer()->RemoveFlag(PLAYER_FLAGS, -flags);
    else
        m_session->GetPlayer()->SetFlag(PLAYER_FLAGS, flags);
        
    return true;
}

/* Syntax: .debug #mapheight [walkableOnly] [teleport] */
bool ChatHandler::HandleDebugMapHeight(const char* args)
{
    bool walkableOnly = false;
    bool teleport = false;

    char* sWalkableOnly = strtok((char*)args, " ");
    char* sTeleport = strtok(nullptr, " ");
    if(sWalkableOnly)
        walkableOnly = atoi(sWalkableOnly);
    if(sTeleport)
        teleport = atoi(sTeleport);

    Player* p = GetSession()->GetPlayer();
    float mapHeight = p->GetMap()->GetHeight(PHASEMASK_NORMAL, p->GetPositionX(), p->GetPositionY(), p->GetPositionZ(), true, 100.0f, walkableOnly);
    if (mapHeight == INVALID_HEIGHT)
    {
        SendSysMessage("No valid height found within 100 yards below");
        return true;
    }

    PSendSysMessage("Map height found at Z: %f", mapHeight);
    if (teleport)
        p->TeleportTo(p->GetMapId(), p->GetPositionX(), p->GetPositionY(), mapHeight, p->GetOrientation());

    return true;
}

/* Syntax: .debug playemote #emoteId */
bool ChatHandler::HandleDebugPlayEmoteCommand(const char* args)
{
    uint32 emote = atoi(args);

    Unit* target = GetSelectedUnit();
    if(!target)
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    target->HandleEmoteCommand(emote);
    PSendSysMessage("Playing emote %u on target", emote);
    return true;
}

//this is meant to be use with the debugger to compose an arbitrary packet on the fly
bool ChatHandler::HandleDebugOpcodeTestCommand(const char* /* args */)
{
    Opcodes op = MSG_NULL_ACTION;
    bool boo = false;
    uint16 un16 = 0;
    int16 in16 = 0;
    uint32 un32 = 0;
    int32 in32 = 0;
    uint64 un64 = 0;
    int64 in64 = 0;
    float floa = 0.0f;
    uint8 byte = 0;
    int8 sbyte = 0;
    std::string str = "abcd";

    uint64 pGuid = GetSession()->GetPlayer()->GetGUID();
    uint32 pGuidLow = GetSession()->GetPlayer()->GetGUIDLow();
    PackedGuid pPackGuid = GetSession()->GetPlayer()->GetPackGUID();
    
    uint64 tGuid = 0;
    uint32 tGuidLow = 0;
    PackedGuid tPackGuid;
    if(Unit const* t = GetSelectedUnit())
    {
        tGuid = t->GetGUID();
        tGuidLow = t->GetGUIDLow();
        tPackGuid = t->GetPackGUID();
    }

    WorldPacket data(op);

    if (false)
        data << boo;
    if (false)
        data << un16;
    if (false)
        data << in16;
    if (false)
        data << un32;
    if (false)
        data << in32;
    if (false)
        data << un64;
    if (false)
        data << in64;
    if (false)
        data << floa;
    if (false)
        data << byte;
    if (false)
        data << sbyte;
    if (false)
        data << str;
    if (false)
        data << pGuid;
    if (false)
        data << pGuidLow;
    if (false)
        data << pPackGuid;
    if (false)
        data << tGuid;
    if (false)
        data << tGuidLow;
    if (false)
        data << tPackGuid;

    if (op == MSG_NULL_ACTION)
    {
        PSendSysMessage("Nothing to send");
        return true;
    }

    GetSession()->SendPacket(&data);
    
    PSendSysMessage("Send opcode %u with size %u.", op, (uint32)data.size());
    return true;
}

/* Syntax: .debug valuessnapshots <start|stop> */
bool ChatHandler::HandleDebugValuesSnapshot(const char* args)
{
#ifdef TRINITY_DEBUG
    Unit* target = GetSelectedUnit();
    if (!target)
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        return true;
    }

    char* cArg = strtok((char*)args, " ");
    if (!cArg)
        return false;

    std::string arg(cArg);

    auto& snapshot_values = GetSession()->snapshot_values;

    if (arg == "start")
    {
        FillSnapshotValues(target, snapshot_values);
        PSendSysMessage("Created snapshot for target %s (guid %u)", target->GetName(), target->GetGUIDLow());
        return true;
    }
    else if (arg == "stop")
    {
        std::vector<uint32> newValues;
        FillSnapshotValues(target, newValues);
        if (newValues.size() != snapshot_values.size())
        {
            SendSysMessage("Error: Snapshots sizes do not match");
            return true;
        }
        SendSysMessage("Differences since snapshot:");

        uint32 diffCount = 0;
        TypeID typeId = TypeID(target->GetTypeId());
        for (uint32 i = 0; i < snapshot_values.size(); i++)
        {
            if (newValues[i] != snapshot_values[i])
            {
                std::stringstream stream;
                std::string fieldName = "[UNKNOWN]";
                UpdateFieldsDebug::GetFieldNameString(typeId, i, fieldName);
                stream << "Index: " << i << " | " << fieldName << std::endl;
                stream << "  New value:  ";
                uint32 fieldSize = UpdateFieldsDebug::InsertFieldInStream(typeId, i, newValues, stream);
                stream << std::endl;
                stream << "  Old value:   ";
                UpdateFieldsDebug::InsertFieldInStream(typeId, i, snapshot_values, stream);
                stream << std::endl;
                diffCount++;
                if (fieldSize > 1)
                    i += (fieldSize - 1); //this will skip next field for UPDATE_FIELD_TYPE_LONG

                PSendSysMessage("%s", stream.str().c_str());
            }
        }
        PSendSysMessage("Found %u differences", diffCount);
    }
    else {
        return false;
    }
#else
    PSendSysMessage("Command can only be used when compiled in debug mode");
#endif
    return true;
}

bool ChatHandler::HandleDebugUnloadGrid(const char* args)
{
    /*
    ARGS_CHECK

    char* mapidstr = strtok((char*)args, " ");
    if (!mapidstr || !*mapidstr)
        return false;

    char* gxstr = strtok(NULL, " ");
    if (!gxstr || !*gxstr)
        return false;

    char* gystr = strtok(NULL, " ");
    if (!gystr || !*gystr)
        return false;

    char* unloadallstr = strtok(NULL, " ");
    if (!unloadallstr || !*unloadallstr)
        return false;

    int mapid, gx, gy;
    bool unloadall;

    mapid = atoi(mapidstr);
    gx = atoi(gxstr);
    gy = atoi(gystr);
    unloadall = atoi(unloadallstr);

    Map* map = sMapMgr->FindBaseNonInstanceMap(mapid);
    if (!map)
    {
        PSendSysMessage("Cannot find map id %u.", mapid);
        return false;
    }

    bool ret;
    ret = map->UnloadGrid(gx, gy, unloadall);

    PSendSysMessage("Unload grid returned %u", ret);
    */
    return true;
}

/* Spawn a bunch of gameobjects objects from given file. This command will check wheter a close object is found on this server and ignore the new one if one is found.
A preview gobject is spawned.
Syntax: .debug spawnbatchobjects <filename> [permanent(0|1)]
    filename: source file
        First line must be map id
        Each line in file must have format :
        <entry> <x> <y> <z> <rot0> <rot1> <rot2> <rot3> <spawntime> <spawnmode>
    permanent: Spawn them as permanent (write to db)
*/
bool ChatHandler::HandleSpawnBatchObjects(const char* args)
{
    struct FileGameObject
    {
        FileGameObject(std::string const& line)
        {
            Tokenizer tokens(line, ' ');
            if (tokens.size() != 10)
                return;

            entry = std::atoi(tokens[0]);
            position = Position(std::atof(tokens[1]), std::atof(tokens[2]), std::atof(tokens[3]));
            rot = G3D::Quat(std::atof(tokens[4]), std::atof(tokens[5]), std::atof(tokens[6]), std::atof(tokens[7]));
            spawnTimeSecs = std::atoi(tokens[8]);
            spawnMask = std::atoi(tokens[9]);

            if (!position.IsPositionValid())
                return;

            _init = true;
        };

        std::string ToString()
        {
            std::stringstream str;
            str << entry << " (" << position.GetPositionX() << " " << position.GetPositionY() << " " << position.GetPositionZ() << ")";
            return str.str();
        }
        
        uint32 entry;
        Position position;
        G3D::Quat rot;
        uint32 spawnTimeSecs;
        uint32 spawnMask;
        bool _init = false; //true if successfully initialized
    };

    Tokenizer tokens(args, ' ');
    if (tokens.size() < 1)
        return false;

    const char* filename = tokens[0];
    bool permanent = false;
    if (tokens.size() >= 2)
        permanent = bool(atoi(tokens[1]));

    //open file
    std::ifstream objects_file(filename);
    if (objects_file.fail())
    {
        PSendSysMessage("Failed to open file %s", filename);
        return true;
    }

    //read map id
    std::string map_id_str;
    std::getline(objects_file, map_id_str);
    if (map_id_str == "")
    {
        SendSysMessage("Invalid input file (could not get mapid)");
        return true;
    }

    uint32 mapId = std::stoi(map_id_str);
    if (mapId == 0)
    {
        SendSysMessage("Invalid input file (mapId == 0)");
        return true;
    }

    Player* player = m_session->GetPlayer();
    if(player->GetMapId() != mapId)
    {
        PSendSysMessage("You must be in the map indicated in the input file (%u)", mapId);
        return true;
    }

    //read gameobjects
    std::string line;
    std::vector<FileGameObject> fileObjects;
    while (std::getline(objects_file, line))
    {
        FileGameObject object(line);
        if (object._init == true)
            fileObjects.push_back(std::move(object));
        else
            PSendSysMessage("Failed to read line: %s", line.c_str());
    }

    if (fileObjects.empty())
    {
        SendSysMessage("No objects extracted from file");
        return true;
    }

    //Check validity for each. Also remove from the object list the one we already have on our server
    auto goDataMap = sObjectMgr->GetGODataMap();
    for (std::vector<FileGameObject>::iterator itr2 = fileObjects.begin(); itr2 != fileObjects.end(); )
    {
        FileGameObject& fileObject = *itr2;
        uint32 objectEntry = fileObject.entry;
        const GameObjectTemplate* goI = sObjectMgr->GetGameObjectTemplate(objectEntry);
        if (!goI)
        {
            PSendSysMessage("Ignoring non existing object %s", fileObject.ToString().c_str());
            itr2 = fileObjects.erase(itr2);
            continue;
        }

        bool erase = false;
        for (auto itr : goDataMap)
        {
            auto const& sunstriderObject = itr.second;
            uint32 sunstriderGUID = itr.first;
            if (sunstriderObject.mapid != mapId)
                continue;

            Position sunPosition(sunstriderObject.posX, sunstriderObject.posY, sunstriderObject.posZ);
            //same id and very close position, consider it the same object
            bool sameObject = sunstriderObject.id == objectEntry && sunstriderObject.spawnMask & fileObject.spawnMask && fileObject.position.GetExactDist(sunPosition) < 5.0f;
            if (sameObject)
            {
                if (sunstriderObject.spawnMask & fileObject.spawnMask && sunstriderObject.spawnMask != fileObject.spawnMask)
                    PSendSysMessage("Warning: Object %s looks like it's corresponding to %u but has different spawn mode. Object will be ignored.", sunstriderGUID, fileObject.ToString().c_str());

                PSendSysMessage("Ignoring object %s we already have", fileObject.ToString().c_str());
                erase = true;
                break; // No need to check others
            }
        }
        if(erase)
            itr2 = fileObjects.erase(itr2);
        else
            itr2++;
    }

    if (fileObjects.empty())
    {
        SendSysMessage("All gameobjects from file were already present, stopping here");
        return true;
    }

    //spawn them
    for (auto itr : fileObjects)
    {
        PSendSysMessage("Summoning %s", itr.ToString().c_str());
        player->SummonGameObject(itr.entry, itr.position, itr.rot, 0);
        if (permanent)
        {
            auto pGameObj = new GameObject;
            if (!pGameObj->Create(player->GetMap()->GenerateLowGuid<HighGuid::GameObject>(), itr.entry, player->GetMap(), PHASEMASK_NORMAL, itr.position, itr.rot, 0, GO_STATE_READY))
            {
                delete pGameObj;
                PSendSysMessage("Failed to create object %u", itr.entry);
                continue;
            }
            pGameObj->SetRespawnTime(itr.spawnTimeSecs);
            pGameObj->SaveToDB(player->GetMap()->GetId(), itr.spawnMask);
        }
    }
    if(permanent)
        SendSysMessage("Saved all to DB");

    return true;
}