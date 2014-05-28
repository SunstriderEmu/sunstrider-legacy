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

bool ChatHandler::HandleDebugInArcCommand(const char* /*args*/)
{
    Object *obj = getSelectedUnit();

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
    if(!args)
        return false;

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
    Unit* target = getSelectedUnit();
    if(!target)
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        return true;
    }

    if(!args)
        return false;

    char* icon_text = strtok((char*)args, " ");
    char* flags_text = strtok(NULL, " ");
    if(!icon_text || !flags_text)
        return false;

    uint32 icon = atol(icon_text);
    if ( icon < 0 )
        icon = 0;

    uint32 flags = atol(flags_text);

    sLog.outDetail("Command : POI, NPC = %u, icon = %u flags = %u", target->GetGUIDLow(), icon,flags);
    pPlayer->PlayerTalkClass->SendPointOfInterest(target->GetPositionX(), target->GetPositionY(), Poi_Icon(icon), flags, 30, "Test POI");
    return true;
}

bool ChatHandler::HandleEquipErrorCommand(const char* args)
{
    if(!args)
        return false;

    uint8 msg = atoi(args);
    m_session->GetPlayer()->SendEquipError(msg, 0, 0);
    return true;
}

bool ChatHandler::HandleSellErrorCommand(const char* args)
{
    if(!args)
        return false;

    uint8 msg = atoi(args);
    m_session->GetPlayer()->SendSellError(msg, 0, 0, 0);
    return true;
}

bool ChatHandler::HandleBuyErrorCommand(const char* args)
{
    if(!args)
        return false;

    uint8 msg = atoi(args);
    m_session->GetPlayer()->SendBuyError(msg, 0, 0, 0);
    return true;
}

bool ChatHandler::HandleSendOpcodeCommand(const char* /*args*/)
{
    Unit *unit = getSelectedUnit();
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
            data.append(unit->GetPackGUID());
        }
        else if(type == "myguid")
        {
            data.append(player->GetPackGUID());
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
    sLog.outDebug("Sending opcode %u", data.GetOpcode());
    data.hexlike();
    (unit->ToPlayer())->GetSession()->SendPacket(&data);
    PSendSysMessage(LANG_COMMAND_OPCODESENT, data.GetOpcode(), unit->GetName());
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
    if(!args)
        return false;

    uint32 soundid = atoi(args);
    m_session->GetPlayer()->PlaySound(soundid, false);
    return true;
}

//Send notification in channel
bool ChatHandler::HandleSendChannelNotifyCommand(const char* args)
{
    if(!args)
        return false;

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
    if(!args)
        return false;

    const char *msg = "testtest";
    uint8 type = atoi(args);
    WorldPacket data;
    ChatHandler::FillMessageData(&data, m_session, type, 0, "chan", m_session->GetPlayer()->GetGUID(), msg, m_session->GetPlayer());
    m_session->SendPacket(&data);
    return true;
}

bool ChatHandler::HandleSendQuestPartyMsgCommand(const char* args)
{
    uint32 msg = atol((char*)args);
    if (msg >= 0)
        m_session->GetPlayer()->SendPushToPartyResponse(m_session->GetPlayer(), msg);
    return true;
}

bool ChatHandler::HandleGetLootRecipient(const char* /*args*/)
{
    Creature* target = getSelectedCreature();
    if(!target)
        return false;

    PSendSysMessage("loot recipient: %s", target->hasLootRecipient()?(target->GetLootRecipient()?target->GetLootRecipient()->GetName():"offline"):"no loot recipient");
    return true;
}

bool ChatHandler::HandleSendQuestInvalidMsgCommand(const char* args)
{
    uint32 msg = atol((char*)args);
    if (msg >= 0)
        m_session->GetPlayer()->SendCanTakeQuestResponse(msg);
    return true;
}

bool ChatHandler::HandleGetItemState(const char* args)
{
    if (!args)
        return false;

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

    Player* player = getSelectedPlayer();
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
            if (!item) continue;
            if (!item->IsBag())
            {
                if (item->GetState() == state)
                    PSendSysMessage("bag: 255 slot: %d guid: %d owner: %d", item->GetSlot(), item->GetGUIDLow(), GUID_LOPART(item->GetOwnerGUID()));
            }
            else
            {
                Bag *bag = (Bag*)item;
                for (uint8 j = 0; j < bag->GetBagSize(); ++j)
                {
                    Item* item = bag->GetItemByPos(j);
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

            PSendSysMessage("bag: %d slot: %d guid: %d - state: %s", bag_slot, item->GetSlot(), item->GetGUIDLow(), st.c_str());
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
            if (!item) continue;

            if (item->GetSlot() != i)
            {
                PSendSysMessage("item at slot %d, guid %d has an incorrect slot value: %d", i, item->GetGUIDLow(), item->GetSlot());
                error = true; continue;
            }

            if (item->GetOwnerGUID() != player->GetGUID())
            {
                PSendSysMessage("for the item at slot %d and itemguid %d, owner's guid (%d) and player's guid (%d) don't match!", item->GetSlot(), item->GetGUIDLow(), GUID_LOPART(item->GetOwnerGUID()), player->GetGUIDLow());
                error = true; continue;
            }

            if (Bag *container = item->GetContainer())
            {
                PSendSysMessage("item at slot: %d guid: %d has a container (slot: %d, guid: %d) but shouldnt!", item->GetSlot(), item->GetGUIDLow(), container->GetSlot(), container->GetGUIDLow());
                error = true; continue;
            }

            if (item->IsInUpdateQueue())
            {
                uint16 qp = item->GetQueuePos();
                if (qp > updateQueue.size())
                {
                    PSendSysMessage("item at slot: %d guid: %d has a queuepos (%d) larger than the update queue size! ", item->GetSlot(), item->GetGUIDLow(), qp);
                    error = true; continue;
                }

                if (updateQueue[qp] == NULL)
                {
                    PSendSysMessage("item at slot: %d guid: %d has a queuepos (%d) that points to NULL in the queue!", item->GetSlot(), item->GetGUIDLow(), qp);
                    error = true; continue;
                }

                if (updateQueue[qp] != item)
                {
                    PSendSysMessage("item at slot: %d guid: %d has has a queuepos (%d) that points to another item in the queue (bag: %d, slot: %d, guid: %d)", item->GetSlot(), item->GetGUIDLow(), qp, updateQueue[qp]->GetBagSlot(), updateQueue[qp]->GetSlot(), updateQueue[qp]->GetGUIDLow());
                    error = true; continue;
                }
            }
            else if (item->GetState() != ITEM_UNCHANGED)
            {
                PSendSysMessage("item at slot: %d guid: %d is not in queue but should be (state: %d)!", item->GetSlot(), item->GetGUIDLow(), item->GetState());
                error = true; continue;
            }

            if(item->IsBag())
            {
                Bag *bag = (Bag*)item;
                for (uint8 j = 0; j < bag->GetBagSize(); ++j)
                {
                    Item* item = bag->GetItemByPos(j);
                    if (!item) continue;

                    if (item->GetSlot() != j)
                    {
                        PSendSysMessage("the item in bag %d slot %d, guid %d has an incorrect slot value: %d", bag->GetSlot(), j, item->GetGUIDLow(), item->GetSlot());
                        error = true; continue;
                    }

                    if (item->GetOwnerGUID() != player->GetGUID())
                    {
                        PSendSysMessage("for the item in bag %d at slot %d and itemguid %d, owner's guid (%d) and player's guid (%d) don't match!", bag->GetSlot(), item->GetSlot(), item->GetGUIDLow(), GUID_LOPART(item->GetOwnerGUID()), player->GetGUIDLow());
                        error = true; continue;
                    }

                    Bag *container = item->GetContainer();
                    if (!container)
                    {
                        PSendSysMessage("the item in bag %d at slot %d with guid %d has no container!", bag->GetSlot(), item->GetSlot(), item->GetGUIDLow());
                        error = true; continue;
                    }

                    if (container != bag)
                    {
                        PSendSysMessage("the item in bag %d at slot %d with guid %d has a different container(slot %d guid %d)!", bag->GetSlot(), item->GetSlot(), item->GetGUIDLow(), container->GetSlot(), container->GetGUIDLow());
                        error = true; continue;
                    }

                    if (item->IsInUpdateQueue())
                    {
                        uint16 qp = item->GetQueuePos();
                        if (qp > updateQueue.size())
                        {
                            PSendSysMessage("item in bag: %d at slot: %d guid: %d has a queuepos (%d) larger than the update queue size! ", bag->GetSlot(), item->GetSlot(), item->GetGUIDLow(), qp);
                            error = true; continue;
                        }

                        if (updateQueue[qp] == NULL)
                        {
                            PSendSysMessage("item in bag: %d at slot: %d guid: %d has a queuepos (%d) that points to NULL in the queue!", bag->GetSlot(), item->GetSlot(), item->GetGUIDLow(), qp);
                            error = true; continue;
                        }

                        if (updateQueue[qp] != item)
                        {
                            PSendSysMessage("item in bag: %d at slot: %d guid: %d has has a queuepos (%d) that points to another item in the queue (bag: %d, slot: %d, guid: %d)", bag->GetSlot(), item->GetSlot(), item->GetGUIDLow(), qp, updateQueue[qp]->GetBagSlot(), updateQueue[qp]->GetSlot(), updateQueue[qp]->GetGUIDLow());
                            error = true; continue;
                        }
                    }
                    else if (item->GetState() != ITEM_UNCHANGED)
                    {
                        PSendSysMessage("item in bag: %d at slot: %d guid: %d is not in queue but should be (state: %d)!", bag->GetSlot(), item->GetSlot(), item->GetGUIDLow(), item->GetState());
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
                PSendSysMessage("queue(%d): for the an item (guid %d), the owner's guid (%d) and player's guid (%d) don't match!", i, item->GetGUIDLow(), GUID_LOPART(item->GetOwnerGUID()), player->GetGUIDLow());
                error = true; continue;
            }

            if (item->GetQueuePos() != i)
            {
                PSendSysMessage("queue(%d): for the an item (guid %d), the queuepos doesn't match it's position in the queue!", i, item->GetGUIDLow());
                error = true; continue;
            }

            if (item->GetState() == ITEM_REMOVED) continue;
            Item *test = player->GetItemByPos( item->GetBagSlot(), item->GetSlot());

            if (test == NULL)
            {
                PSendSysMessage("queue(%d): the bag(%d) and slot(%d) values for the item with guid %d are incorrect, the player doesn't have an item at that position!", i, item->GetBagSlot(), item->GetSlot(), item->GetGUIDLow());
                error = true; continue;
            }

            if (test != item)
            {
                PSendSysMessage("queue(%d): the bag(%d) and slot(%d) values for the item with guid %d are incorrect, the item with guid %d is there instead!", i, item->GetBagSlot(), item->GetSlot(), item->GetGUIDLow(), test->GetGUIDLow());
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
    sBattleGroundMgr.ToggleArenaTesting();
    return true;
}

bool ChatHandler::HandleDebugThreatList(const char * args)
{
    Creature* target = getSelectedCreature();
    if(!target || target->isTotem() || target->IsPet())
        return false;

    uint32 limit = 0;
    if(args)
        limit = (uint32)atoi(args);

    std::list<HostilReference*>& tlist = target->getThreatManager().getThreatList();
    std::list<HostilReference*>::iterator itr;
    uint32 cnt = 0;
    PSendSysMessage("Threat list of %s (guid %u)",target->GetName(), target->GetGUIDLow());
    for(itr = tlist.begin(); itr != tlist.end(); ++itr)
    {
        Unit* unit = (*itr)->getTarget();
        if(!unit)
            continue;
        ++cnt;
        PSendSysMessage("   %u.   %s   (guid %u) - (entry %u) - threat %f",cnt,unit->GetName(), unit->GetGUIDLow(), unit->GetEntry(), (*itr)->getThreat());

        if (limit && cnt >= limit)
            break;
    }
    SendSysMessage("End of threat list.");
    return true;
}

bool ChatHandler::HandleDebugHostilRefList(const char * /*args*/)
{
    Unit* target = getSelectedUnit();
    if(!target)
        target = m_session->GetPlayer();
    HostilReference* ref = target->getHostilRefManager().getFirst();
    uint32 cnt = 0;
    PSendSysMessage("Hostil reference list of %s (guid %u)",target->GetName(), target->GetGUIDLow());
    while(ref)
    {
        if(Unit * unit = ref->getSource()->getOwner())
        {
            ++cnt;
            PSendSysMessage("   %u.   %s   (guid %u) - (entry %u) - threat %f",cnt,unit->GetName(), unit->GetGUIDLow(), unit->GetEntry(), ref->getThreat());
        }
        ref = ref->next();
    }
    SendSysMessage("End of hostil reference list.");
    return true;
}

bool ChatHandler::HandleDebugCinematic(const char* args)
{
    if (!args || !*args)
        return true;
    WorldPacket data(SMSG_TRIGGER_CINEMATIC, 4);
    data << uint32(atoi(args));
    m_session->GetPlayer()->GetSession()->SendPacket(&data);
    return true;
}

bool ChatHandler::HandleDebugItemByPos(const char* args)
{
    if (!args || !*args)
        return false;
        
    uint32 pos = uint32(atoi(args));
    Item* item = m_session->GetPlayer()->GetItemByPos(pos);
    if (!item) {
        PSendSysMessage("Invalid position.");
        return true;
    }
    
    PSendSysMessage("Item found! Name: %s, level: %u", item->GetProto()->Name1, item->GetProto()->ItemLevel);
    return true;
}

bool ChatHandler::HandleDebugItemLevelSum(const char* args)
{
    PSendSysMessage("Total stuff level: %u", m_session->GetPlayer()->GetEquipedItemsLevelSum());
    
    return true;
}

bool ChatHandler::HandleRemoveLootItem(const char* args)
{
    Creature* target = getSelectedCreature();
    
    if (!args || !*args || !target)
        return false;
        
    uint32 itemId = uint32(atoi(args));
    target->loot.RemoveItem(itemId);
    PSendSysMessage("Item removed.");
        
    return true;
}

bool ChatHandler::HandleDebugStealthLevel(const char* args)
{
    Unit *target = getSelectedUnit();
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
    Unit *target = getSelectedUnit();
    if (!target || !target->ToCreature())
        return false;
        
    PSendSysMessage("AttackDistance: %f - ModDetectRange: %f", target->ToCreature()->GetAttackDistance(m_session->GetPlayer()), target->GetTotalAuraModifier(SPELL_AURA_MOD_DETECT_RANGE));
    return true;
}

bool ChatHandler::HandleSpellInfoCommand(const char* args)
{
    if (!args || !*args)
        return false;
    
    uint32 spellId = uint32(atoi(args));
    if (!spellId)
        return false;
        
    const SpellEntry* spell = spellmgr.LookupSpell(spellId);
    if (!spell)
        return false;
        
    PSendSysMessage("## Spell %u (%s) ##", spell->Id, spell->SpellName[sWorld.GetDefaultDbcLocale()]);
    PSendSysMessage("Icon: %u - Visual: %u", spell->SpellIconID, spell->SpellVisual);
    PSendSysMessage("Attributes: %x %x %x %x %x %x", spell->Attributes, spell->AttributesEx, spell->AttributesEx2, spell->AttributesEx3, spell->AttributesEx4, spell->AttributesEx5);
    PSendSysMessage("Stack amount: %u", spell->StackAmount);
    PSendSysMessage("SpellFamilyName: %u (%x)", spell->SpellFamilyName, spell->SpellFamilyName);
    PSendSysMessage("SpellFamilyFlags: " I64FMTD " (%x)", spell->SpellFamilyFlags, spell->SpellFamilyFlags);
    
    return true;
}

bool ChatHandler::HandleDebugLoadedScripts(const char* args)
{
    uint32 entries, guids;
    objmgr.GetLoadedScriptsStats(entries, guids);
    
    PSendSysMessage("Loaded scripts stats: %u by entry, %u by GUID", entries, guids);
    
    return true;
}

bool ChatHandler::HandleDebugResetDailyQuests(const char* args)
{
    sWorld.ResetDailyQuests();

    return true;
}

bool ChatHandler::HandleDebugShowAttackers(const char* args)
{
    Unit* target = getSelectedUnit();
    if (!target)
        return false;
        
    SendSysMessage("Attackers list :");
    char msg[256];
    for (Unit::AttackerSet::const_iterator itr = target->getAttackers().begin(); itr != target->getAttackers().end(); ++itr) {
        if ((*itr)->GetTypeId() == TYPEID_PLAYER)
            snprintf(msg, 256, "%s (Entry: 0 (Player), GUID: %u, Full GUID:" I64FMTD")", (*itr)->GetName(), (*itr)->GetGUIDLow(), (*itr)->GetGUID());
        else
            snprintf(msg, 256, "%s (Entry: %u, GUID: %u, Full GUID:" I64FMTD")", (*itr)->GetName(), (*itr)->GetEntry(), (*itr)->ToCreature()->GetDBTableGUIDLow(), (*itr)->GetGUID());
            
        SendSysMessage(msg);
    }
    
    return true;
}

bool ChatHandler::HandleDebugSendZoneUnderAttack(const char* args)
{
    if (!args)
        return false;
        
    char* zone = strtok((char*)args, " ");
    if (!zone)
        return false;
        
    uint32 zoneId = atoi(zone);
    if (!zoneId)
        return false;
    
    uint32 team = m_session->GetPlayer()->GetTeam();
    WorldPacket data(SMSG_ZONE_UNDER_ATTACK,4);
    data << zoneId;
    sWorld.SendGlobalMessage(&data,NULL,(team==ALLIANCE ? HORDE : ALLIANCE));
    return true;
}

bool ChatHandler::HandleDebugLoSCommand(const char* args)
{
    if (Unit* unit = getSelectedUnit())
        PSendSysMessage("Unit %s (GuidLow: %u) is %sin LoS", unit->GetName(), unit->GetGUIDLow(), m_session->GetPlayer()->IsWithinLOSInMap(unit) ? "" : "not ");
    
    /*AreaTableEntry const* area;
    for (uint32 i = 0; i <= 4140; i++) {
        area = GetAreaEntryByAreaID(i);
        if (area) {
            if (area->flags & AREA_FLAG_OUTSIDE)
                sLog.outString("Area %u (%s) is outdoor", i, area->area_name[2]);
            else if (area->flags & AREA_FLAG_INSIDE)
                sLog.outString("Area %u (%s) is indoor", i, area->area_name[2]);
            else
                sLog.outString("Area %u (%s) is detected by wmo flags", i, area->area_name[2]);
        }
    }*/
    
    return true;
}

bool ChatHandler::HandleDebugPlayerFlags(const char* args)
{
    if (!args || !*args)
        return false;
        
    int flags = atoi(args);
    if (!flags)
        return true;
        
    if (flags < 0)
        m_session->GetPlayer()->RemoveFlag(PLAYER_FLAGS, -flags);
    else
        m_session->GetPlayer()->SetFlag(PLAYER_FLAGS, flags);
        
    return true;
}

bool ChatHandler::HandleDebugDumpProfilingCommand(const char* args)
{
    PSendSysMessage("Dump done.");
    sLog.outString(sProfilerMgr.dump().c_str());
    return true;
}

bool ChatHandler::HandleDebugClearProfilingCommand(const char* args)
{
    PSendSysMessage("Profiling data cleared.");
    sProfilerMgr.clear();
    return true;
}