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

#include "Common.h"
#include "Database/DatabaseEnv.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include "Player.h"
#include "Opcodes.h"
#include "Chat.h"
#include "Log.h"
#include "Unit.h"
#include "ObjectAccessor.h"
#include "GossipDef.h"
#include "Language.h"
#include "MapManager.h"
#include "BattleGroundMgr.h"
#include <fstream>
#include "ObjectMgr.h"
#include "SpellMgr.h"
#include "SmartScriptMgr.h"
#include "SmartAI.h"
#include "UpdateFieldsDebug.h"

#ifdef PLAYERBOT
#include "playerbot.h"
#include "GuildTaskMgr.h"
#endif

bool ChatHandler::HandleYoloCommand(const char* /* args */)
{
    SendSysMessage(LANG_SWAG);

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
    if(char* cType = strtok(NULL, ""))
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
    data << uint32(133);
    data << uint8(failnum);
    m_session->SendPacket(&data);

    return true;
}

bool ChatHandler::HandleSetPoiCommand(const char* args)
{
    Player *pPlayer = m_session->GetPlayer();
    Unit* target = GetSelectedUnit();
    if(!target)
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        return true;
    }

    char* icon_text = strtok((char*)args, " ");
    char* flags_text = strtok(NULL, " ");
    if(!icon_text || !flags_text)
        return false;

    uint32 icon = atol(icon_text);
    uint32 flags = atol(flags_text);

    TC_LOG_DEBUG("battleground","Command : POI, NPC = %u, icon = %u flags = %u", target->GetGUIDLow(), icon,flags);
    pPlayer->PlayerTalkClass->SendPointOfInterest(target->GetPositionX(), target->GetPositionY(), Poi_Icon(icon), flags, 30, "Test POI");
    return true;
}

bool ChatHandler::HandleEquipErrorCommand(const char* args)
{
    uint8 msg = atoi(args);
    m_session->GetPlayer()->SendEquipError(msg, 0, 0);
    return true;
}

bool ChatHandler::HandleSellErrorCommand(const char* args)
{
    uint8 msg = atoi(args);
    m_session->GetPlayer()->SendSellError(msg, 0, 0, 0);
    return true;
}

bool ChatHandler::HandleBuyErrorCommand(const char* args)
{
    uint8 msg = atoi(args);
    m_session->GetPlayer()->SendBuyError(msg, 0, 0, 0);
    return true;
}

bool ChatHandler::HandleSendOpcodeCommand(const char* /*args*/)
{
    Unit *unit = GetSelectedUnit();
    Player *player = NULL;
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
    char* s = strtok(NULL, " ");

    if (!w || !s)
        return false;

    uint32 world = (uint32)atoi(w);
    uint32 state = (uint32)atoi(s);
    m_session->GetPlayer()->SendUpdateWorldState(world, state);
    return true;
}

bool ChatHandler::HandlePlaySound2Command(const char* args)
{
    uint32 soundid = atoi(args);
    m_session->GetPlayer()->PlaySound(soundid, false);
    return true;
}

//Send notification in channel
bool ChatHandler::HandleSendChannelNotifyCommand(const char* args)
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
bool ChatHandler::HandleSendChatMsgCommand(const char* args)
{
    const char *msg = "testtest";
    ChatMsg type = ChatMsg(atoi(args));
    WorldPacket data;
    ChatHandler::BuildChatPacket(data, ChatMsg(type), LANG_UNIVERSAL, m_session->GetPlayer(), m_session->GetPlayer(), msg);
    m_session->SendPacket(&data);
    return true;
}

bool ChatHandler::HandleSendQuestPartyMsgCommand(const char* args)
{
    uint32 msg = atol((char*)args);
    m_session->GetPlayer()->SendPushToPartyResponse(m_session->GetPlayer(), msg);
    return true;
}

bool ChatHandler::HandleGetLootRecipient(const char* /*args*/)
{
    Creature* target = GetSelectedCreature();
    if(!target)
        return false;

    PSendSysMessage("Loot recipient: %s", target->hasLootRecipient()?(target->GetLootRecipient()?target->GetLootRecipient()->GetName().c_str() :"offline"):"no loot recipient");
    return true;
}

bool ChatHandler::HandleSendQuestInvalidMsgCommand(const char* args)
{
    uint32 msg = atol((char*)args);
    m_session->GetPlayer()->SendCanTakeQuestResponse(msg);
    return true;
}

bool ChatHandler::HandleGetItemState(const char* args)
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
        for(size_t i = 0; i < updateQueue.size(); i++)
        {
            Item *item = updateQueue[i];
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

                if (updateQueue[qp] == NULL)
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

                        if (updateQueue[qp] == NULL)
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

            if (test == NULL)
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

bool ChatHandler::HandleRemoveLootItem(const char* args)
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
    float modDetect = target->GetTotalAuraModifier(SPELL_AURA_MOD_DETECT);

    PSendSysMessage("Stealth: %f - Stealth level: %f - Total: %f (Detect %f)", modStealth, modStealthLevel, (modStealth+modStealthLevel),modDetect);
    return true;
}

bool ChatHandler::HandleDebugAttackDistance(const char* args)
{
    Unit *target = GetSelectedUnit();
    if (!target || !target->ToCreature())
        return false;
        
    PSendSysMessage("AttackDistance: %f - ModDetectRange: %i", target->ToCreature()->GetAttackDistance(m_session->GetPlayer()), target->GetTotalAuraModifier(SPELL_AURA_MOD_DETECT_RANGE));
    return true;
}

bool ChatHandler::HandleSpellInfoCommand(const char* args)
{
    ARGS_CHECK
    
    uint32 spellId = uint32(atoi(args));
    if (!spellId)
        return false;
        
    const SpellInfo* spell = sSpellMgr->GetSpellInfo(spellId);
    if (!spell)
        return false;
        
    PSendSysMessage("## Spell %u (%s) ##", spell->Id, spell->SpellName[(uint32)sWorld->GetDefaultDbcLocale()]);
    PSendSysMessage("Icon: %u - Visual: %u", spell->SpellIconID, spell->SpellVisual);
    PSendSysMessage("Attributes: %x %x %x %x %x %x", spell->Attributes, spell->AttributesEx, spell->AttributesEx2, spell->AttributesEx3, spell->AttributesEx4, spell->AttributesEx5);
    PSendSysMessage("Stack amount: %u", spell->StackAmount);
    PSendSysMessage("SpellFamilyName: %u (%x)", spell->SpellFamilyName, spell->SpellFamilyName);
    PSendSysMessage("SpellFamilyFlags: " UI64FMTD " (" UI64FMTD ")", spell->SpellFamilyFlags, spell->SpellFamilyFlags);
    
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
    for (Unit::AttackerSet::const_iterator itr = target->GetAttackers().begin(); itr != target->GetAttackers().end(); ++itr) {
        if ((*itr)->GetTypeId() == TYPEID_PLAYER)
            snprintf(msg, 256, "%s (Entry: 0 (Player), GUID: %u, Full GUID:" UI64FMTD ")", (*itr)->GetName().c_str(), (*itr)->GetGUIDLow(), (*itr)->GetGUID());
        else
            snprintf(msg, 256, "%s (Entry: %u, GUID: %u, Full GUID:" UI64FMTD ")", (*itr)->GetName().c_str(), (*itr)->GetEntry(), (*itr)->ToCreature()->GetDBTableGUIDLow(), (*itr)->GetGUID());
            
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
    sWorld->SendZoneUnderAttack(zoneId, (team==TEAM_ALLIANCE ? TEAM_HORDE : TEAM_ALLIANCE));
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

/** Syntax: .smartai errors [entryOrGuid] */
bool ChatHandler::HandleSmartAIShowErrorsCommand(const char* args)
{
    uint32 entry = 0;
    uint32 guid = 0;
    Creature* target = nullptr;

    if (!*args)
    {
        //if no arguments given, try getting selected creature
        target = GetSelectedCreature();

        if (!target)
        {
            SendSysMessage("Select a creature or give an entry or a guid (as a negative value).");
            return true;
        }

        guid = target->GetGUIDLow();
        entry = target->GetEntry();
    } else {
        //arguments given, check if guid or entry
        int entryOrGuid = atoi(args);
        if(entryOrGuid > 0)
            entry = entryOrGuid;
        else
            guid = -entryOrGuid;
    }

    SendSysMessage("SmartAI errors :");
    auto errorList = sSmartScriptMgr->GetErrorList(-int32(guid)); //negative guid in argument
    for(auto itr : errorList)
        PSendSysMessage("%s", itr.c_str());

    errorList = sSmartScriptMgr->GetErrorList(entry);
    for(auto itr : errorList)
        PSendSysMessage("%s", itr.c_str());

    return true;
}

/** Syntax: .smartai debug [entryOrGuid] */
bool ChatHandler::HandleSmartAIDebugCommand(const char* args)
{
    uint32 entry = 0;
    uint32 guid = 0;
    Creature* target = nullptr;

    if (!*args)
    {
        //if no arguments given, try getting selected creature
        target = GetSelectedCreature();

        if (!target)
        {
            SendSysMessage("Select a creature or give an entry or a guid (as a negative value).");
            return true;
        }

        guid = target->GetGUIDLow();
        entry = target->GetEntry();
    } else {
        //arguments given, check if guid or entry
        int entryOrGuid = atoi(args);
        if(entryOrGuid > 0)
            entry = entryOrGuid;
        else
            guid = -entryOrGuid;
    }

    SendSysMessage("SmartAI infos :");
    if(target)
    {
        if(target->GetAIName() == SMARTAI_AI_NAME)
        {
            if(SmartScript* smartScript = dynamic_cast<SmartAI*>(target->AI())->GetScript())
            {
                uint32 phase = smartScript->GetPhase();
                PSendSysMessage("Current phase: %u", phase);

                uint32 lastProcessedActionId = smartScript->GetLastProcessedActionId();
                PSendSysMessage("Last processed action: %u", lastProcessedActionId);
            }
        } else {
            SendSysMessage("Not SmartAI creature.");
        }
    } else {
        SendSysMessage("No target selected.");
        //TODO: try getting AI with args
    }

    return true;
}

/* Syntax: .debug mapheight [walkableOnly] [teleport] */
bool ChatHandler::HandleDebugMapHeight(const char* args)
{
    bool walkableOnly = false;
    bool teleport = false;

    char* sWalkableOnly = strtok((char*)args, " ");
    char* sTeleport = strtok(NULL, " ");
    if(sWalkableOnly)
        walkableOnly = atoi(sWalkableOnly);
    if(sTeleport)
        teleport = atoi(sTeleport);

    Player* p = GetSession()->GetPlayer();
    float mapHeight = p->GetMap()->GetHeight(PhaseMask(1), p->GetPositionX(), p->GetPositionY(), p->GetPositionZ(), true, 100.0f, walkableOnly);
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

/* Syntax: .debug playemote <emoteId> */
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
bool ChatHandler::HandleDebugOpcodeTestCommand(const char* args)
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

    GetSession()->SendPacket(&data);
    
    PSendSysMessage("Send opcode %u with size %u.", op, (uint32)data.size());
    return true;
}

void FillSnapshotValues(Unit* target, std::vector<uint32>& values)
{
    uint32 valuesCount = target->GetValuesCount();
    values.clear();
    values.resize(valuesCount);
    for (uint32 i = 0; i < valuesCount; i++)
        values[i] = target->GetUInt32Value(i);
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
                GetFieldNameString(typeId, i, fieldName);
                stream << "Index: " << i << " | " << fieldName << std::endl;
                stream << "  New value:  ";
                uint32 fieldSize = InsertFieldInStream(typeId, i, newValues, stream);
                stream << std::endl;
                stream << "  Old value:   ";
                InsertFieldInStream(typeId, i, snapshot_values, stream);
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

/* Syntax : .debug getvalue #index */
bool ChatHandler::HandleGetValueCommand(const char* args)
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
    FillSnapshotValues(target, values);

    TypeID type = TypeID(target->GetTypeId());
    std::string fieldName = "[UNKNOWN]";
    GetFieldNameString(type, index, fieldName);
    std::stringstream ss;
    InsertFieldInStream(type, index, values, ss);
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

bool ChatHandler::HandlePlayerbotConsoleCommand(const char* args)
{
    return RandomPlayerbotMgr::HandlePlayerbotConsoleCommand(this, args);
}

bool ChatHandler::HandlePlayerbotMgrCommand(const char* args)
{
    return PlayerbotMgr::HandlePlayerbotMgrCommand(this, args);
}