#include "Chat.h"
#include "Language.h"
#include <fstream>
#include "UpdateFieldsDebug.h"
#include "BattleGroundMgr.h"
#include "ChannelMgr.h"
#include "GossipDef.h"
#include "Bag.h"

class debug_commandscript : public CommandScript
{
public:
    debug_commandscript() : CommandScript("debug_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> debugSnapshotTable =
        {
            { "go",             SEC_GAMEMASTER3,  false, &HandleDebugSnapshotGo,              "" },
            { "",               SEC_GAMEMASTER3,  false, &HandleDebugSnapshot,                "" },
        };

        static std::vector<ChatCommand> debugCommandTable =
        {
            { "batchattack",    SEC_GAMEMASTER3,  false, &HandleDebugBatchAttack,             "" },
            { "inarc",          SEC_GAMEMASTER3,  false, &HandleDebugInArcCommand,            "" },
            { "spellfail",      SEC_GAMEMASTER3,  false, &HandleDebugSpellFailCommand,        "" },
            { "setpoi",         SEC_GAMEMASTER3,  false, &HandleDebugSetPoiCommand,           "" },
            { "qpartymsg",      SEC_GAMEMASTER3,  false, &HandleDebugSendQuestPartyMsgCommand,"" },
            { "qinvalidmsg",    SEC_GAMEMASTER3,  false, &HandleDebugSendQuestInvalidMsgCommand,"" },
            { "equiperr",       SEC_GAMEMASTER3,  false, &HandleDebugEquipErrorCommand,       "" },
            { "sellerr",        SEC_GAMEMASTER3,  false, &HandleDebugSellErrorCommand,        "" },
            { "buyerr",         SEC_GAMEMASTER3,  false, &HandleDebugBuyErrorCommand,         "" },
            { "sendopcode",     SEC_GAMEMASTER3,  false, &HandleDebugSendOpcodeCommand,       "" },
            { "uws",            SEC_GAMEMASTER3,  false, &HandleUpdateWorldStateCommand,      "" },
            { "ps",             SEC_GAMEMASTER3,  false, &HandleDebugPlaySound2Command,       "" },
            { "scn",            SEC_GAMEMASTER3,  false, &HandleDebugSendChannelNotifyCommand,"" },
            { "scm",            SEC_GAMEMASTER3,  false, &HandleDebugSendChatMsgCommand,      "" },
            { "getitemstate",   SEC_GAMEMASTER3,  false, &HandleDebugGetItemState,            "" },
            { "playsound",      SEC_GAMEMASTER1,  false, &HandleDebugPlaySoundCommand,        "" },
            { "setvalue",       SEC_ADMINISTRATOR,false, &HandleDebugSetValueCommand,         "" },
            { "getvalue",       SEC_GAMEMASTER3,  false, &HandleDebugGetValueCommand,         "" },
            { "anim",           SEC_GAMEMASTER2,  false, &HandleDebugAnimCommand,             "" },
            { "lootrecipient",  SEC_GAMEMASTER2,  false, &HandleDebugGetLootRecipient,        "" },
            { "arena",          SEC_GAMEMASTER3,  false, &HandleDebugArenaCommand,            "" },
            { "bg",             SEC_GAMEMASTER3,  false, &HandleDebugBattleGroundCommand,     "" },
            { "bgevent",        SEC_GAMEMASTER3,  false, &HandleDebugBattleGroundEventCommand,"" },
            { "threatlist",     SEC_GAMEMASTER2,  false, &HandleDebugThreatListCommand,       "" },
            { "threatunitlist", SEC_GAMEMASTER2,  false, &HandleDebugCombatListCommand,       "" },
            { "threatinfo",     SEC_GAMEMASTER2,  false, &HandleDebugThreatInfoCommand,       "" },
            { "cin",            SEC_GAMEMASTER3,  false, &HandleDebugCinematic,               "" },
            { "getitembypos",   SEC_GAMEMASTER3,  false, &HandleDebugItemByPos,               "" },
            { "getitemlevelsum",SEC_GAMEMASTER3,  false, &HandleDebugItemLevelSum,            "" },
            { "pvpannounce",    SEC_GAMEMASTER3,  false, &HandleDebugPvPAnnounce,             "" },
            { "stealth",        SEC_GAMEMASTER3,  false, &HandleDebugStealthLevel,            "" },
            { "detect",         SEC_GAMEMASTER3,  false, &HandleDebugAttackDistance,          "" },
            { "unloadgrid",     SEC_GAMEMASTER3,  false, &HandleDebugUnloadGrid,              "" },
            { "resetdaily",     SEC_GAMEMASTER3,  true,  &HandleDebugResetDailyQuests,        "" },
            { "attackers",      SEC_GAMEMASTER2,  false, &HandleDebugShowAttackers,           "" },
            { "zoneattack",     SEC_GAMEMASTER3,  false, &HandleDebugSendZoneUnderAttack,     "" },
            { "los",            SEC_GAMEMASTER1,  false, &HandleDebugLoSCommand,              "" },
            { "playerflags",    SEC_GAMEMASTER3,  false, &HandleDebugPlayerFlags,             "" },
            { "opcodetest",     SEC_GAMEMASTER3,  false, &HandleDebugOpcodeTestCommand,       "" },
            { "playemote",      SEC_GAMEMASTER2,  false, &HandleDebugPlayEmoteCommand,        "" },
            { "mapheight",      SEC_GAMEMASTER3,  false, &HandleDebugMapHeight,               "" },
            { "snapshot",       SEC_GAMEMASTER3,  false, nullptr,                             "", debugSnapshotTable },
            { "crash",          SEC_SUPERADMIN,   false, &HandleDebugCrashCommand,            "" },
            { "setzonemusic",   SEC_SUPERADMIN,   false, &HandleDebugZoneMusicCommand,        "" },
            { "setzonelight",   SEC_SUPERADMIN,   false, &HandleDebugZoneLightCommand,        "" },
            { "setzoneweather", SEC_SUPERADMIN,   false, &HandleDebugZoneWeatherCommand,      "" },
            { "setarmor",       SEC_GAMEMASTER3,  false, &HandleDebugSetArmorCommand,         "" },
            { "getarmor",       SEC_GAMEMASTER3,  false, &HandleDebugGetArmorCommand,         "" },
            { "boundary",       SEC_GAMEMASTER3,  false, &HandleDebugBoundaryCommand,         "" },
            //Dev utilities
            { "spawnbatchobjects",SEC_SUPERADMIN, false, &HandleSpawnBatchObjects,            "" },
        };
        static std::vector<ChatCommand> commandTable =
        {
            { "debug",          SEC_GAMEMASTER1,  false, nullptr,                                        "", debugCommandTable },
        };
        return commandTable;
    }

    /** Syntax: .debug batchattack <count> [type]
    With type:
    0 - Basic attack
    1 - Offhand attack
    2 - Ranged attack
    */
    static bool HandleDebugBatchAttack(ChatHandler* handler, char const* args)
    {
        char* cCount = strtok((char*)args, " ");
        if(!cCount)
            return false;

        uint32 count = atoi(cCount);
        if(count == 0)
            return false;

        if(count > 5000)
        {
            handler->PSendSysMessage("Plz stahp abusing");
            return true;
        }

        WeaponAttackType type = BASE_ATTACK;
        if(char* cType = strtok(nullptr, ""))
        {
            type = WeaponAttackType(atoi(cType));
            if(type > RANGED_ATTACK)
                return false;
        }

        Player* p = handler->GetSession()->GetPlayer();
        if(Unit* victim = p->GetVictim())
        {
            if(type != RANGED_ATTACK)
            {
                for(uint32 i = 0; i < count; i++)
                    p->AttackerStateUpdate(victim, type);
            } else 
            {
                for(uint32 i = 0; i < count; i++)
                    p->CastSpell(victim, 75, true); //shoot
            }
        } else 
        {
            handler->PSendSysMessage("No victim");
        }

        return true;
    }

    static bool HandleDebugInArcCommand(ChatHandler* handler, char const* /*args*/)
    {
        Object *obj = handler->GetSelectedUnit();

        if(!obj)
        {
            handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            return true;
        }

        handler->SendSysMessage(LANG_NOT_IMPLEMENTED);

        return true;
    }

    static bool HandleDebugSpellFailCommand(ChatHandler* handler, char const* args)
    {
        char* px = strtok((char*)args, " ");
        if(!px)
            return false;

        uint8 failnum = (uint8)atoi(px);

        WorldPacket data(SMSG_CAST_FAILED, 5);
        data << uint32(133); //spell 133 ?
        data << uint8(failnum);
        handler->GetSession()->SendPacket(&data);

        return true;
    }

    static bool HandleDebugSetPoiCommand(ChatHandler* handler, char const* args)
    {
        Player *pPlayer = handler->GetSession()->GetPlayer();
        Unit* target = handler->GetSelectedUnit();
        if(!target)
        {
            handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            return true;
        }

        char* icon_text = strtok((char*)args, " ");
        char* flags_text = strtok(nullptr, " ");
        if(!icon_text || !flags_text)
            return false;

        uint32 icon = atol(icon_text);
        uint32 flags = atol(flags_text);

        TC_LOG_DEBUG("battleground","Command : POI, NPC = %u, icon = %u flags = %u", target->GetGUID().GetCounter(), icon,flags);
        pPlayer->PlayerTalkClass->SendPointOfInterest(target->GetPositionX(), target->GetPositionY(), Poi_Icon(icon), flags, 30, "Test POI");
        return true;
    }

    static bool HandleDebugSendQuestPartyMsgCommand(ChatHandler* handler, char const* args)
    {
        uint32 msg = atol((char*)args);
        handler->GetSession()->GetPlayer()->SendPushToPartyResponse(handler->GetSession()->GetPlayer(), msg);
        return true;
    }

    static bool HandleDebugSendQuestInvalidMsgCommand(ChatHandler* handler, char const* args)
    {
        uint32 msg = atol((char*)args);
        handler->GetSession()->GetPlayer()->SendCanTakeQuestResponse(msg);
        return true;
    }

    static bool HandleDebugEquipErrorCommand(ChatHandler* handler, char const* args)
    {
        uint8 msg = atoi(args);
        handler->GetSession()->GetPlayer()->SendEquipError(msg, nullptr, nullptr);
        return true;
    }

    static bool HandleDebugSellErrorCommand(ChatHandler* handler, char const* args)
    {
        uint8 msg = atoi(args);
        handler->GetSession()->GetPlayer()->SendSellError(msg, nullptr, ObjectGuid::Empty, 0);
        return true;
    }

    static bool HandleDebugBuyErrorCommand(ChatHandler* handler, char const* args)
    {
        uint8 msg = atoi(args);
        handler->GetSession()->GetPlayer()->SendBuyError(msg, nullptr, 0, 0);
        return true;
    }

    static bool HandleDebugSendOpcodeCommand(ChatHandler* handler, char const* /*args*/)
    {
        Unit *unit = handler->GetSelectedUnit();
        Player *player = nullptr;
        if (!unit || (unit->GetTypeId() != TYPEID_PLAYER))
            player = handler->GetSession()->GetPlayer();
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
        handler->PSendSysMessage(LANG_COMMAND_OPCODESENT, data.GetOpcode(), unit->GetName().c_str());
        return true;
    }

    static bool HandleUpdateWorldStateCommand(ChatHandler* handler, char const* args)
    {
        char* w = strtok((char*)args, " ");
        char* s = strtok(nullptr, " ");

        if (!w || !s)
            return false;

        uint32 world = (uint32)atoi(w);
        uint32 state = (uint32)atoi(s);
        handler->GetSession()->GetPlayer()->SendUpdateWorldState(world, state);
        return true;
    }

    static bool HandleDebugPlaySound2Command(ChatHandler* handler, char const* args)
    {
        uint32 soundid = atoi(args);
        handler->GetSession()->GetPlayer()->PlaySound(soundid, false);
        return true;
    }

    //Send notification in channel
    static bool HandleDebugSendChannelNotifyCommand(ChatHandler* handler, char const* args)
    {
        const char *name = "test";
        uint8 code = atoi(args);

        WorldPacket data(SMSG_CHANNEL_NOTIFY, (1+10));
        data << code;                                           // notify type
        data << name;                                           // channel name
        data << uint32(0);
        data << uint32(0);
        handler->GetSession()->SendPacket(&data);
        return true;
    }

    //Send notification in chat
    static bool HandleDebugSendChatMsgCommand(ChatHandler* handler, char const* args)
    {
        const char *msg = "testtest";
        ChatMsg type = ChatMsg(atoi(args));
        WorldPacket data;
        ChatHandler::BuildChatPacket(data, ChatMsg(type), LANG_UNIVERSAL, handler->GetSession()->GetPlayer(), handler->GetSession()->GetPlayer(), msg);
        handler->GetSession()->SendPacket(&data);
        return true;
    }

    static bool HandleDebugGetItemState(ChatHandler* handler, char const* args)
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

        Player* player = handler->GetSelectedPlayer();
        if (!player) player = handler->GetSession()->GetPlayer();

        if (!list_queue && !check_all)
        {
            state_str = "The player has the following " + state_str + " items: ";
            handler->SendSysMessage(state_str.c_str());
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
                        handler->PSendSysMessage("bag: 255 slot: %d guid: %d owner: %d", item->GetSlot(), item->GetGUID().GetCounter(), item->GetOwnerGUID().GetCounter());
                }
                else
                {
                    Bag* bag = (Bag*)item;
                    for (uint8 j = 0; j < bag->GetBagSize(); ++j)
                    {
                        item = bag->GetItemByPos(j);
                        if (item && item->GetState() == state)
                            handler->PSendSysMessage("bag: 255 slot: %d guid: %d owner: %d", item->GetSlot(), item->GetGUID().GetCounter(), item->GetOwnerGUID().GetCounter());
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

                handler->PSendSysMessage("Bag: %d slot: %d guid: %d - state: %s", bag_slot, item->GetSlot(), item->GetGUID().GetCounter(), st.c_str());
            }
            if (updateQueue.empty())
                handler->PSendSysMessage("updatequeue empty");
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
                    handler->PSendSysMessage("Item at slot %d, guid %d has an incorrect slot value: %d", i, item->GetGUID().GetCounter(), item->GetSlot());
                    error = true; continue;
                }

                if (item->GetOwnerGUID() != player->GetGUID())
                {
                    handler->PSendSysMessage("For the item at slot %d and itemguid %d, owner's guid (%d) and player's guid (%d) don't match!", item->GetSlot(), item->GetGUID().GetCounter(), item->GetOwnerGUID().GetCounter(), player->GetGUID().GetCounter());
                    error = true; continue;
                }

                if (Bag *container = item->GetContainer())
                {
                    handler->PSendSysMessage("Item at slot: %d guid: %d has a container (slot: %d, guid: %d) but shouldnt!", item->GetSlot(), item->GetGUID().GetCounter(), container->GetSlot(), container->GetGUID().GetCounter());
                    error = true; continue;
                }

                if (item->IsInUpdateQueue())
                {
                    uint16 qp = item->GetQueuePos();
                    if (qp > updateQueue.size())
                    {
                        handler->PSendSysMessage("Item at slot: %d guid: %d has a queuepos (%d) larger than the update queue size! ", item->GetSlot(), item->GetGUID().GetCounter(), qp);
                        error = true; continue;
                    }

                    if (updateQueue[qp] == nullptr)
                    {
                        handler->PSendSysMessage("Item at slot: %d guid: %d has a queuepos (%d) that points to NULL in the queue!", item->GetSlot(), item->GetGUID().GetCounter(), qp);
                        error = true; continue;
                    }

                    if (updateQueue[qp] != item)
                    {
                        handler->PSendSysMessage("Item at slot: %d guid: %d has has a queuepos (%d) that points to another item in the queue (bag: %d, slot: %d, guid: %d)", item->GetSlot(), item->GetGUID().GetCounter(), qp, updateQueue[qp]->GetBagSlot(), updateQueue[qp]->GetSlot(), updateQueue[qp]->GetGUID().GetCounter());
                        error = true; continue;
                    }
                }
                else if (item->GetState() != ITEM_UNCHANGED)
                {
                    handler->PSendSysMessage("Item at slot: %d guid: %d is not in queue but should be (state: %d)!", item->GetSlot(), item->GetGUID().GetCounter(), item->GetState());
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
                            handler->PSendSysMessage("The item in bag %d slot %d, guid %d has an incorrect slot value: %d", bag->GetSlot(), j, item->GetGUID().GetCounter(), item->GetSlot());
                            error = true; continue;
                        }

                        if (item->GetOwnerGUID() != player->GetGUID())
                        {
                            handler->PSendSysMessage("For the item in bag %d at slot %d and itemguid %d, owner's guid (%d) and player's guid (%d) don't match!", bag->GetSlot(), item->GetSlot(), item->GetGUID().GetCounter(), item->GetOwnerGUID().GetCounter(), player->GetGUID().GetCounter());
                            error = true; continue;
                        }

                        Bag *container = item->GetContainer();
                        if (!container)
                        {
                            handler->PSendSysMessage("The item in bag %d at slot %d with guid %d has no container!", bag->GetSlot(), item->GetSlot(), item->GetGUID().GetCounter());
                            error = true; continue;
                        }

                        if (container != bag)
                        {
                            handler->PSendSysMessage("The item in bag %d at slot %d with guid %d has a different container(slot %d guid %d)!", bag->GetSlot(), item->GetSlot(), item->GetGUID().GetCounter(), container->GetSlot(), container->GetGUID().GetCounter());
                            error = true; continue;
                        }

                        if (item->IsInUpdateQueue())
                        {
                            uint16 qp = item->GetQueuePos();
                            if (qp > updateQueue.size())
                            {
                                handler->PSendSysMessage("Item in bag: %d at slot: %d guid: %d has a queuepos (%d) larger than the update queue size! ", bag->GetSlot(), item->GetSlot(), item->GetGUID().GetCounter(), qp);
                                error = true; continue;
                            }

                            if (updateQueue[qp] == nullptr)
                            {
                                handler->PSendSysMessage("Item in bag: %d at slot: %d guid: %d has a queuepos (%d) that points to NULL in the queue!", bag->GetSlot(), item->GetSlot(), item->GetGUID().GetCounter(), qp);
                                error = true; continue;
                            }

                            if (updateQueue[qp] != item)
                            {
                                handler->PSendSysMessage("Item in bag: %d at slot: %d guid: %d has has a queuepos (%d) that points to another item in the queue (bag: %d, slot: %d, guid: %d)", bag->GetSlot(), item->GetSlot(), item->GetGUID().GetCounter(), qp, updateQueue[qp]->GetBagSlot(), updateQueue[qp]->GetSlot(), updateQueue[qp]->GetGUID().GetCounter());
                                error = true; continue;
                            }
                        }
                        else if (item->GetState() != ITEM_UNCHANGED)
                        {
                            handler->PSendSysMessage("Item in bag: %d at slot: %d guid: %d is not in queue but should be (state: %d)!", bag->GetSlot(), item->GetSlot(), item->GetGUID().GetCounter(), item->GetState());
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
                    handler->PSendSysMessage("queue(%u): for the an item (guid %u), the owner's guid (%u) and player's guid (%u) don't match!", uint32(i), item->GetGUID().GetCounter(), item->GetOwnerGUID().GetCounter(), player->GetGUID().GetCounter());
                    error = true; continue;
                }

                if (item->GetQueuePos() != i)
                {
                    handler->PSendSysMessage("queue(%d): for the an item (guid %d), the queuepos doesn't match it's position in the queue!", uint32(i), item->GetGUID().GetCounter());
                    error = true; continue;
                }

                if (item->GetState() == ITEM_REMOVED) continue;
                Item *test = player->GetItemByPos( item->GetBagSlot(), item->GetSlot());

                if (test == nullptr)
                {
                    handler->PSendSysMessage("queue(%d): the bag(%d) and slot(%d) values for the item with guid %d are incorrect, the player doesn't have an item at that position!", (uint32)i, item->GetBagSlot(), item->GetSlot(), item->GetGUID().GetCounter());
                    error = true; continue;
                }

                if (test != item)
                {
                    handler->PSendSysMessage("queue(%d): the bag(%d) and slot(%d) values for the item with guid %d are incorrect, the item with guid %d is there instead!", (uint32)i, item->GetBagSlot(), item->GetSlot(), item->GetGUID().GetCounter(), test->GetGUID().GetCounter());
                    error = true; continue;
                }
            }
            if (!error)
                handler->SendSysMessage("All OK!");
        }

        return true;
    }

    static bool HandleDebugArenaCommand(ChatHandler* handler, char const* args)
    {
        bool enabled = sBattlegroundMgr->ToggleArenaTesting();
    
        if(enabled)
            handler->SendGlobalGMSysMessage("Arenas are set to minimum 1 player per team for debugging.");
        else
            handler->SendGlobalGMSysMessage("Arenas are set to normal playercount.");

        return true;
    }
    
    static bool HandleDebugBattleGroundEventCommand(ChatHandler* handler, const char* args)
    {
        Player *chr = handler->GetSession()->GetPlayer();
        ASSERT(chr);
        Battleground* bg = chr->GetBattleground();
        if (!bg)
        {
            handler->SendSysMessage("You are not in a battleground");
            handler->SetSentErrorMessage(true);
            return false;
        }
        bg->HandleCommand(chr, handler, args);
        return true;
    }

    static bool HandleDebugBattleGroundCommand(ChatHandler* handler, const char* /*args*/)
    {
        bool enabled = sBattlegroundMgr->ToggleBattleGroundTesting();

        if(enabled)
            handler->SendGlobalGMSysMessage("BattleGrounds will now open with first player joining.");
        else
            handler->SendGlobalGMSysMessage("BattleGrounds are set to normal playercount.");
    
        return true;
    }

    static bool HandleDebugThreatListCommand(ChatHandler* handler, char const* args)
    {
        Unit* target = handler->GetSelectedUnit();
        if (!target)
            target = handler->GetSession()->GetPlayer();

        ThreatManager& mgr = target->GetThreatManager();
        if (!target->IsAlive())
        {
            handler->PSendSysMessage("%s (guid %u) is not alive.", target->GetName().c_str(), target->GetGUID().GetCounter());
            return true;
        }

        uint32 count = 0;
        auto const& threatenedByMe = target->GetThreatManager().GetThreatenedByMeList();
        if (threatenedByMe.empty())
            handler->PSendSysMessage("%s (guid %u) does not threaten any units.", target->GetName().c_str(), target->GetGUID().GetCounter());
        else
        {
            handler->PSendSysMessage("List of units threatened by %s (guid %u)", target->GetName().c_str(), target->GetGUID().GetCounter());
            for (auto const& pair : threatenedByMe)
            {
                Unit* unit = pair.second->GetOwner();
                handler->PSendSysMessage("   %u.   %s   (GUID %u, SpawnID %u)  - threat %f", ++count, unit->GetName().c_str(), unit->GetGUID().GetCounter(), unit->GetTypeId() == TYPEID_UNIT ? unit->ToCreature()->GetSpawnId() : 0, pair.second->GetThreat());
            }
            handler->SendSysMessage("End of threatened-by-me list.");
        }

        if (mgr.CanHaveThreatList())
        {
            if (!mgr.IsThreatListEmpty(true))
            {
                if (mgr.IsEngaged())
                    handler->PSendSysMessage("Threat list of %s (GUID %u, SpawnID %u):", target->GetName().c_str(), target->GetGUID().GetCounter(), target->GetTypeId() == TYPEID_UNIT ? target->ToCreature()->GetSpawnId() : 0);
                else
                    handler->PSendSysMessage("%s (GUID %u, SpawnID %u) is not engaged, but still has a threat list? Well, here it is:", target->GetName().c_str(), target->GetGUID().GetCounter(), target->GetTypeId() == TYPEID_UNIT ? target->ToCreature()->GetSpawnId() : 0);

                count = 0;
                Unit* fixtateVictim = mgr.GetFixateTarget();
                for (ThreatReference const* ref : mgr.GetSortedThreatList())
                {
                    Unit* unit = ref->GetVictim();
                    char const* onlineStr;
                    switch (ref->GetOnlineState())
                    {
                    case ThreatReference::ONLINE_STATE_SUPPRESSED:
                        onlineStr = " [SUPPRESSED]";
                        break;
                    case ThreatReference::ONLINE_STATE_OFFLINE:
                        onlineStr = " [OFFLINE]";
                        break;
                    default:
                        onlineStr = "";
                    }
                    char const* tauntStr;
                    if (unit == fixtateVictim)
                        tauntStr = " [FIXTATE]";
                    else
                        switch (ref->GetTauntState())
                        {
                        case ThreatReference::TAUNT_STATE_TAUNT:
                            tauntStr = " [TAUNT]";
                            break;
                        case ThreatReference::TAUNT_STATE_DETAUNT:
                            tauntStr = " [DETAUNT]";
                            break;
                        default:
                            tauntStr = "";
                        }
                    handler->PSendSysMessage("   %u.   %s   (GUID %u)  - threat %f%s%s", ++count, unit->GetName().c_str(), unit->GetGUID().GetCounter(), ref->GetThreat(), tauntStr, onlineStr);
                }
                handler->SendSysMessage("End of threat list.");
            }
            else if (!mgr.IsEngaged())
                handler->PSendSysMessage("%s (GUID %u, SpawnID %u) is not currently engaged.", target->GetName().c_str(), target->GetGUID().GetCounter(), target->GetTypeId() == TYPEID_UNIT ? target->ToCreature()->GetSpawnId() : 0);
            else
                handler->PSendSysMessage("%s (GUID %u, SpawnID %u) seems to be engaged, but does not have a threat list??", target->GetName().c_str(), target->GetGUID().GetCounter(), target->GetTypeId() == TYPEID_UNIT ? target->ToCreature()->GetSpawnId() : 0);
        }
        else
            handler->PSendSysMessage("%s (GUID %u) cannot have a threat list.", target->GetName().c_str(), target->GetGUID().GetCounter());
        return true;
    }

    static bool HandleDebugCombatListCommand(ChatHandler* handler, char const* args)
    {
        Unit* target = handler->GetSelectedUnit();
        if (!target)
            target = handler->GetSession()->GetPlayer();

        handler->PSendSysMessage("Combat refs: (Combat state: %d | Manager state: %d)", target->IsInCombat(), target->GetCombatManager().HasCombat());
        for (auto const& ref : target->GetCombatManager().GetPvPCombatRefs())
        {
            Unit* unit = ref.second->GetOther(target);
            handler->PSendSysMessage("[PvP] %s (SpawnID %u)", unit->GetName().c_str(), unit->GetTypeId() == TYPEID_UNIT ? unit->ToCreature()->GetSpawnId() : 0);
        }
        for (auto const& ref : target->GetCombatManager().GetPvECombatRefs())
        {
            Unit* unit = ref.second->GetOther(target);
            handler->PSendSysMessage("[PvE] %s (SpawnID %u)", unit->GetName().c_str(), unit->GetTypeId() == TYPEID_UNIT ? unit->ToCreature()->GetSpawnId() : 0);
        }
        return true;
    }


    static bool HandleDebugThreatInfoCommand(ChatHandler* handler, char const* args)
    {
        Unit* target = handler->GetSelectedUnit();
        if (!target)
        {
            handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        handler->PSendSysMessage("Threat info for %s (%s):", target->GetName(), target->GetGUID().ToString().c_str());

        ThreatManager const& mgr = target->GetThreatManager();

        // _singleSchoolModifiers
        {
            auto& mods = mgr._singleSchoolModifiers;
            handler->SendSysMessage(" - Single-school threat modifiers:");
            handler->PSendSysMessage(" |-- Physical: %.2f%%", mods[SPELL_SCHOOL_NORMAL] * 100.0f);
            handler->PSendSysMessage(" |-- Holy    : %.2f%%", mods[SPELL_SCHOOL_HOLY] * 100.0f);
            handler->PSendSysMessage(" |-- Fire    : %.2f%%", mods[SPELL_SCHOOL_FIRE] * 100.0f);
            handler->PSendSysMessage(" |-- Nature  : %.2f%%", mods[SPELL_SCHOOL_NATURE] * 100.0f);
            handler->PSendSysMessage(" |-- Frost   : %.2f%%", mods[SPELL_SCHOOL_FROST] * 100.0f);
            handler->PSendSysMessage(" |-- Shadow  : %.2f%%", mods[SPELL_SCHOOL_SHADOW] * 100.0f);
            handler->PSendSysMessage(" |-- Arcane  : %.2f%%", mods[SPELL_SCHOOL_ARCANE] * 100.0f);
        }

        // _multiSchoolModifiers
        {
            auto& mods = mgr._multiSchoolModifiers;
            handler->PSendSysMessage("- Multi-school threat modifiers (%zu entries):", mods.size());
            for (auto const& pair : mods)
                handler->PSendSysMessage(" |-- Mask 0x%x: %.2f%%", uint32(pair.first), pair.second);
        }

        // _redirectInfo
        {
            auto const& redirectInfo = mgr._redirectInfo;
            if (redirectInfo.empty())
                handler->SendSysMessage(" - No redirects being applied");
            else
            {
                handler->PSendSysMessage(" - %02zu redirects being applied:", redirectInfo.size());
                for (auto const& pair : redirectInfo)
                {
                    Unit* unit = ObjectAccessor::GetUnit(*target, pair.first);
                    handler->PSendSysMessage(" |-- %02u%% to %s", pair.second, unit ? unit->GetName().c_str() : pair.first.ToString().c_str());
                }
            }
        }

        // _redirectRegistry
        {
            auto const& redirectRegistry = mgr._redirectRegistry;
            if (redirectRegistry.empty())
                handler->SendSysMessage(" - No redirects are registered");
            else
            {
                handler->PSendSysMessage(" - %02zu spells may have redirects registered", redirectRegistry.size());
                for (auto const& outerPair : redirectRegistry) // (spellId, (guid, pct))
                {
                    SpellInfo const* const spell = sSpellMgr->GetSpellInfo(outerPair.first);
                    handler->PSendSysMessage(" |-- #%06u %s (%zu entries):", outerPair.first, spell ? spell->SpellName[0] : "<unknown>", outerPair.second.size());
                    for (auto const& innerPair : outerPair.second) // (guid, pct)
                    {
                        Unit* unit = ObjectAccessor::GetUnit(*target, innerPair.first);
                        handler->PSendSysMessage("   |-- %02u%% to %s", innerPair.second, unit ? unit->GetName().c_str() : innerPair.first.ToString().c_str());
                    }
                }
            }
        }
        return true;
    }

    static bool HandleDebugCinematic(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

        WorldPacket data(SMSG_TRIGGER_CINEMATIC, 4);
        data << uint32(atoi(args));
        handler->GetSession()->GetPlayer()->SendDirectMessage(&data);
        return true;
    }

    static bool HandleDebugItemByPos(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK
        
        uint32 pos = uint32(atoi(args));
        Item* item = handler->GetSession()->GetPlayer()->GetItemByPos(pos);
        if (!item) {
            handler->PSendSysMessage("Invalid position.");
            return true;
        }
    
        handler->PSendSysMessage("Item found! Name: %s, level: %u", item->GetTemplate()->Name1.c_str(), item->GetTemplate()->ItemLevel);
        return true;
    }

    static bool HandleDebugItemLevelSum(ChatHandler* handler, char const* args)
    {
        handler->PSendSysMessage("Total stuff level: %u", handler->GetSession()->GetPlayer()->GetEquipedItemsLevelSum());
    
        return true;
    }

    //Play sound
    static bool HandleDebugPlaySoundCommand(ChatHandler* handler, char const* args)
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
                handler->PSendSysMessage(LANG_SOUND_NOT_EXIST, dwSoundId);
                handler->SetSentErrorMessage(true);
                return false;
            }

            handler->GetSession()->GetPlayer()->PlayDistanceSound(dwSoundId, handler->GetSession()->GetPlayer());

            handler->PSendSysMessage(LANG_YOU_HEAR_SOUND, dwSoundId);
            return true;
        }

        return false;
    }

    /* Syntax : .debug setvalue #index #value [uint32/uint64/float]*/
    static bool HandleDebugSetValueCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

        char* cIndex = strtok((char*)args, " ");
        char* cValue = strtok(nullptr, " ");
        char* cType = strtok(nullptr, " ");

        if (!cIndex || !cValue)
            return false;

        Unit* target = handler->GetSelectedUnit();
        if(!target)
        {
            handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        ObjectGuid guid = target->GetGUID();

        uint32 index = (uint32)atoi(cIndex);
        if(index >= target->GetValuesCount())
        {
            handler->PSendSysMessage(LANG_TOO_BIG_INDEX, index, guid.GetCounter(), target->GetValuesCount());
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
            handler->PSendSysMessage(LANG_SET_UINT_FIELD, guid.GetCounter(), index, uValue);
            }
            break;
        case 1: //uint64
            {
            std::stringstream ss(cValue);
            ss >> uValue;
            target->SetUInt64Value(index,uValue);
            handler->PSendSysMessage("You set the uint64 value of %u in %u to " UI64FMTD, guid.GetCounter(), index, uValue);
            }
            break;
        case 2: //float
            fValue = (float)atof(cValue);
            target->SetFloatValue(index,fValue);
            handler->PSendSysMessage(LANG_SET_FLOAT_FIELD, guid.GetCounter(), index, fValue);
            break;
        }

        return true;
    }

    /* Syntax : .debug getvalue #index */
    static bool HandleDebugGetValueCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

        char* cIndex = strtok((char*)args, " ");

        if (!cIndex)
            return false;

        Unit* target = handler->GetSelectedUnit();
        if (!target)
        {
            handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        ObjectGuid guid = target->GetGUID();

        uint32 index = (uint32)atoi(cIndex);
        if (index >= target->GetValuesCount())
        {
            handler->PSendSysMessage(LANG_TOO_BIG_INDEX, index, guid.GetCounter(), target->GetValuesCount());
            return false;
        }

        WorldSession::SnapshotType values;
        UpdateFieldsDebug::FillSnapshotValues(target, values); //todo: no need to fill this for all values

        TypeID type = TypeID(target->GetTypeId());
        std::string fieldName = "[UNKNOWN]";
        UpdateFieldsDebug::GetFieldNameString(type, index, fieldName);
        std::stringstream ss;
        UpdateFieldsDebug::InsertFieldInStream(type, index, values, ss);
        handler->PSendSysMessage("Field %s (%u): %s", fieldName.c_str(), index, ss.str().c_str());

        return true;
    }

    /* Syntax: .debug crash <SIGSEGV|SEGABRT> */
    static bool HandleDebugCrashCommand(ChatHandler* handler, char const* args)
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
    static bool HandleDebugZoneMusicCommand(ChatHandler* handler, char const* args)
    {
        char* cMusicId = strtok((char*)args, " ");

        if (!cMusicId)
            return false;

        uint32 musicId = (uint32)atoi(cMusicId);
        if (musicId == 0)
            return false;

        Player* p = handler->GetSession()->GetPlayer();
        if (!p)
            return false;

        Map* map = p->GetMap();
        if (!map)
            return false;

        map->SetZoneMusic(p->GetZoneId(), musicId);
        handler->PSendSysMessage("Changed zone music to %u", musicId);
        return true;
    }

    /* Syntax: .debug zonelight #newLightId */
    static bool HandleDebugZoneLightCommand(ChatHandler* handler, char const* args)
    {
        char* cLightId = strtok((char*)args, " ");

        if (!cLightId)
            return false;

        uint32 lightId = (uint32)atoi(cLightId);
        if (lightId == 0)
            return false;

        Player* p = handler->GetSession()->GetPlayer();
        if (!p)
            return false;

        Map* map = p->GetMap();
        if (!map)
            return false;

        map->SetZoneOverrideLight(p->GetZoneId(), lightId, 10 * SECOND * IN_MILLISECONDS); //fade over 10 seconds
        handler->PSendSysMessage("Changed zone light to %u", lightId);
        return true;
    }

    /* Syntax: .debug zoneweather #weatherId [#intensity] */
    static bool HandleDebugZoneWeatherCommand(ChatHandler* handler, char const* args)
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

        Player* p = handler->GetSession()->GetPlayer();
        if (!p)
            return false;

        Map* map = p->GetMap();
        if (!map)
            return false;

        map->SetZoneWeather(p->GetZoneId(), WeatherState(weatherId), intensity);
        handler->PSendSysMessage("Changed zone weather to %u", weatherId);
        return true;
    }

    /* Syntax: .debug setarmor #armorValue */
    static bool HandleDebugSetArmorCommand(ChatHandler* handler, char const* args)
    {
        Unit* target = handler->GetSelectedUnit();
        if (!target)
        {
            handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            return true;
        }

        uint32 armor = atoi(args);
        target->SetUInt32Value(UNIT_FIELD_RESISTANCES, armor);

        handler->PSendSysMessage("Set target armor to %u", armor);

        return true;
    }

    /* Syntax: .debug getarmor */
    static bool HandleDebugGetArmorCommand(ChatHandler* handler, char const* args)
    {
        return HandleDebugGetValueCommand(handler, "186"); //186 == UNIT_FIELD_RESISTANCES
    }

    //show animation
    static bool HandleDebugAnimCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

        uint32 anim_id = atoi((char*)args);
        handler->GetSession()->GetPlayer()->HandleEmoteCommand(anim_id);
        return true;
    }

    static bool HandleDebugGetLootRecipient(ChatHandler* handler, char const* /*args*/)
    {
        Creature* target =  handler->GetSelectedCreature();
        if(!target)
            return false;

        handler->PSendSysMessage("Loot recipient: %s", target->hasLootRecipient()?(target->GetLootRecipient()?target->GetLootRecipient()->GetName().c_str() :"offline"):"no loot recipient");
        return true;
    }

    static bool HandleDebugStealthLevel(ChatHandler* handler, char const* args)
    {
        Unit *target = handler->GetSelectedUnit();
        if (!target)
            target = handler->GetSession()->GetPlayer();
        
        float modStealth = target->GetTotalAuraModifier(SPELL_AURA_MOD_STEALTH);
        float modStealthLevel = target->GetTotalAuraModifier(SPELL_AURA_MOD_STEALTH_LEVEL);
        float modDetect = target->GetTotalAuraModifier(SPELL_AURA_MOD_STEALTH_DETECT);

        handler->PSendSysMessage("Stealth: %f - Stealth level: %f - Total: %f (Detect %f)", modStealth, modStealthLevel, (modStealth+modStealthLevel),modDetect);
        return true;
    }

    static bool HandleDebugAttackDistance(ChatHandler* handler, char const* args)
    {
        Unit *target = handler->GetSelectedUnit();
        if (!target || !target->ToCreature())
            return false;
        
        handler->PSendSysMessage("AttackDistance: %f - ModDetectRange: %i", target->ToCreature()->GetAggroRange(handler->GetSession()->GetPlayer()), target->GetTotalAuraModifier(SPELL_AURA_MOD_DETECT_RANGE));
        return true;
    }

    static bool HandleDebugPvPAnnounce(ChatHandler* handler, char const* args)
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

    static bool HandleDebugResetDailyQuests(ChatHandler* handler, char const* args)
    {
        sWorld->ResetDailyQuests();

        return true;
    }

    static bool HandleDebugShowAttackers(ChatHandler* handler, char const* args)
    {
        Unit* target = handler->GetSelectedUnit();
        if (!target)
            return false;
        
        handler->SendSysMessage("Attackers list:");
        char msg[256];
        for (auto itr : target->GetAttackers()) {
            if (itr->GetTypeId() == TYPEID_PLAYER)
                snprintf(msg, 256, "%s (Entry: 0 (Player), GUID: %u, Full GUID:" UI64FMTD ")", itr->GetName().c_str(), itr->GetGUID().GetCounter(), itr->GetGUID().GetRawValue());
            else
                snprintf(msg, 256, "%s (Entry: %u, GUID: %u, Full GUID:" UI64FMTD ")", itr->GetName().c_str(), itr->GetEntry(), itr->ToCreature()->GetSpawnId(), itr->GetGUID().GetRawValue());
            
            handler->SendSysMessage(msg);
        }
    
        return true;
    }

    static bool HandleDebugSendZoneUnderAttack(ChatHandler* handler, char const* args)
    {
        char* zone = strtok((char*)args, " ");
        if (!zone)
            return false;
        
        uint32 zoneId = atoi(zone);
        if (!zoneId)
            return false;
    
        uint32 team = handler->GetSession()->GetPlayer()->GetTeam();
        sWorld->SendZoneUnderAttack(zoneId, (team==ALLIANCE ? HORDE : ALLIANCE));
        return true;
    }

    static bool HandleDebugLoSCommand(ChatHandler* handler, char const* args)
    {
        if (Unit* unit = handler->GetSelectedUnit())
            handler->PSendSysMessage("Unit %s (GuidLow: %u) is %sin LoS", unit->GetName().c_str(), unit->GetGUID().GetCounter(), handler->GetSession()->GetPlayer()->IsWithinLOSInMap(unit) ? "" : "not ");
        
        return true;
    }

    static bool HandleDebugPlayerFlags(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK
        
        int flags = atoi(args);
        if (!flags)
            return true;
        
        if (flags < 0)
            handler->GetSession()->GetPlayer()->RemoveFlag(PLAYER_FLAGS, -flags);
        else
            handler->GetSession()->GetPlayer()->SetFlag(PLAYER_FLAGS, flags);
        
        return true;
    }

    /* Syntax: .debug #mapheight [walkableOnly] [teleport] */
    static bool HandleDebugMapHeight(ChatHandler* handler, char const* args)
    {
        bool walkableOnly = false;
        bool teleport = false;

        char* sWalkableOnly = strtok((char*)args, " ");
        char* sTeleport = strtok(nullptr, " ");
        if(sWalkableOnly)
            walkableOnly = atoi(sWalkableOnly);
        if(sTeleport)
            teleport = atoi(sTeleport);

        Player* p = handler->GetSession()->GetPlayer();
        float mapHeight = p->GetMap()->GetHeight(PHASEMASK_NORMAL, p->GetPositionX(), p->GetPositionY(), p->GetPositionZ(), true, 100.0f, 0.0f, walkableOnly);
        if (mapHeight == INVALID_HEIGHT)
        {
            handler->SendSysMessage("No valid height found within 100 yards below");
            return true;
        }

        handler->PSendSysMessage("Map height found at Z: %f", mapHeight);
        if (teleport)
            p->TeleportTo(p->GetMapId(), p->GetPositionX(), p->GetPositionY(), mapHeight, p->GetOrientation());

        return true;
    }

    /* Syntax: .debug playemote #emoteId */
    static bool HandleDebugPlayEmoteCommand(ChatHandler* handler, char const* args)
    {
        uint32 emote = atoi(args);

        Unit* target = handler->GetSelectedUnit();
        if(!target)
        {
            handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        target->HandleEmoteCommand(emote);
        handler->PSendSysMessage("Playing emote %u on target", emote);
        return true;
    }

    //this is meant to be use with the debugger to compose an arbitrary packet on the fly
    static bool HandleDebugOpcodeTestCommand(ChatHandler* handler, char const* /*args*/)
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

        ObjectGuid pGuid = handler->GetSession()->GetPlayer()->GetGUID();
        uint32 pGuidLow = handler->GetSession()->GetPlayer()->GetGUID().GetCounter();
        PackedGuid pPackGuid = handler->GetSession()->GetPlayer()->GetPackGUID();
    
        ObjectGuid tGuid;
        uint32 tGuidLow = 0;
        PackedGuid tPackGuid;
        if(Unit const* t = handler->GetSelectedUnit())
        {
            tGuid = t->GetGUID();
            tGuidLow = t->GetGUID().GetCounter();
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
            handler->PSendSysMessage("Nothing to send");
            return true;
        }

        handler->GetSession()->SendPacket(&data);
    
        handler->PSendSysMessage("Send opcode %u with size %u.", op, (uint32)data.size());
        return true;
    }


    static bool _HandleDebugSnapshot(ChatHandler* handler, WorldObject* target, std::string arg)
    {
#ifdef TRINITY_DEBUG
        auto& snapshot_values = handler->GetSession()->snapshot_values;

        if (arg == "start")
        {
            UpdateFieldsDebug::FillSnapshotValues(target, snapshot_values);
            handler->PSendSysMessage("Created snapshot for target %s (guid %u)", target->GetName(), target->GetGUID().GetCounter());
            return true;
        }
        else if (arg == "stop")
        {
            WorldSession::SnapshotType newValues;
            UpdateFieldsDebug::FillSnapshotValues(target, newValues);
            if (newValues.size() != snapshot_values.size())
            {
                handler->SendSysMessage("Error: Snapshots sizes do not match");
                return true;
            }
            handler->SendSysMessage("Differences since snapshot:");

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

                    handler->PSendSysMessage("%s", stream.str().c_str());
                }
            }
            handler->PSendSysMessage("Found %u differences", diffCount);
        }
        else if (arg == "dump")
        {
            handler->PSendSysMessage("Dumping value for target %s (lowguid %u): ", target->GetName().c_str(), target->GetGUID().GetCounter());
            WorldSession::SnapshotType dumpValues;
            UpdateFieldsDebug::FillSnapshotValues(target, dumpValues);
            TypeID typeId = TypeID(target->GetTypeId());
            for (uint32 i = 0; i < dumpValues.size(); i++)
            {
                std::stringstream stream;
                std::string fieldName = "[UNKNOWN]";
                UpdateFieldsDebug::GetFieldNameString(typeId, i, fieldName);
                stream << "Index: " << std::setw(4) << i << " | " << fieldName << std::endl;
                uint32 fieldSize = UpdateFieldsDebug::InsertFieldInStream(typeId, i, dumpValues, stream);
                stream << std::endl;
                if (fieldSize > 1)
                    i += (fieldSize - 1); //this will skip next field for UPDATE_FIELD_TYPE_LONG

                handler->PSendSysMessage("%s", stream.str().c_str());
                TC_LOG_DEBUG("snapshot", "%", stream.str().c_str());
            }
        }
        else {
            return false;
        }
#else
        handler->PSendSysMessage("Command can only be used when compiled in debug mode");
#endif
        return true;
    }

    /* Syntax: .debug snapshot go <start|stop|dump> */
    static bool HandleDebugSnapshotGo(ChatHandler* handler, char const* args)
    {
        Tokenizer tokens(args, ' ');
        if (tokens.size() < 2)
            return false;

        ObjectGuid::LowType spawnID(uint32(std::stoul(tokens[0])));
        std::string arg = tokens[1];

        Map* map = handler->GetSession()->GetPlayer()->GetMap();
        if (!map)
        {
            handler->SendSysMessage("Map not found");
            return true;
        }

        GameObject* target = map->GetGameObjectBySpawnId(spawnID);
        if (!target)
        {
            handler->SendSysMessage(LANG_COMMAND_GOOBJNOTFOUND);
            return true;
        }

        _HandleDebugSnapshot(handler, target, arg);

        return true;
    }

    /* Syntax: .debug snapshot <start|stop|dump> */
    static bool HandleDebugSnapshot(ChatHandler* handler, char const* args)
    {
        char* cArg = strtok((char*)args, " ");
        if (!cArg)
            return false;

        WorldObject* target = handler->GetSelectedUnit();
        if (!target)
        {
            handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            return true;
        }

        std::string arg(cArg);
        _HandleDebugSnapshot(handler, target, arg);

        return true;
    }

    static bool HandleDebugUnloadGrid(ChatHandler* handler, char const* args)
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
            handler->PSendSysMessage("Cannot find map id %u.", mapid);
            return false;
        }

        bool ret;
        ret = map->UnloadGrid(gx, gy, unloadall);

        handler->PSendSysMessage("Unload grid returned %u", ret);
        */
        return true;
    }

    static bool HandleDebugBoundaryCommand(ChatHandler* handler, char const* args)
    {
        Player* player = handler->GetSession()->GetPlayer();
        if (!player)
            return false;
        Creature* target =  handler->GetSelectedCreature();
        if (!target || !target->IsAIEnabled())
        {
            return false;
        }

        char* fill_str = args ? strtok((char*)args, " ") : nullptr;
        char* duration_str = args ? strtok(nullptr, " ") : nullptr;

        int duration = duration_str ? atoi(duration_str) : -1;
        if (duration <= 0 || duration >= 30 * MINUTE) // arbitary upper limit
            duration = 3 * MINUTE;

        bool doFill = fill_str ? (stricmp(fill_str, "FILL") == 0) : false;

        int32 errMsg = target->AI()->VisualizeBoundary(duration, player, doFill);
        if (errMsg > 0)
            handler->PSendSysMessage(errMsg);

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
    static bool HandleSpawnBatchObjects(ChatHandler* handler, char const* args)
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

        ARGS_CHECK
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
            handler->PSendSysMessage("Failed to open file %s", filename);
            return true;
        }

        //read map id
        std::string map_id_str;
        std::getline(objects_file, map_id_str);
        if (map_id_str == "")
        {
            handler->SendSysMessage("Invalid input file (could not get mapid)");
            return true;
        }

        uint32 mapId = std::stoi(map_id_str);
        if (mapId == 0)
        {
            handler->SendSysMessage("Invalid input file (mapId == 0)");
            return true;
        }

        Player* player = handler->GetSession()->GetPlayer();
        if(player->GetMapId() != mapId)
        {
            handler->PSendSysMessage("You must be in the map indicated in the input file (%u)", mapId);
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
                handler->PSendSysMessage("Failed to read line: %s", line.c_str());
        }

        if (fileObjects.empty())
        {
            handler->SendSysMessage("No objects extracted from file");
            return true;
        }

        //Check validity for each. Also remove from the object list the one we already have on our server
        auto goDataMap = sObjectMgr->GetAllGameObjectData();
        for (std::vector<FileGameObject>::iterator itr2 = fileObjects.begin(); itr2 != fileObjects.end(); )
        {
            FileGameObject& fileObject = *itr2;
            uint32 objectEntry = fileObject.entry;
            const GameObjectTemplate* goI = sObjectMgr->GetGameObjectTemplate(objectEntry);
            if (!goI)
            {
                handler->PSendSysMessage("Ignoring non existing object %s", fileObject.ToString().c_str());
                itr2 = fileObjects.erase(itr2);
                continue;
            }

            bool erase = false;
            for (auto itr : goDataMap)
            {
                auto const& sunstriderObject = itr.second;
                uint32 sunstriderGUID = itr.first;
                if (sunstriderObject.spawnPoint.GetMapId() != mapId)
                    continue;

                //same id and very close position, consider it the same object
                bool sameObject = sunstriderObject.id == objectEntry && sunstriderObject.spawnMask & fileObject.spawnMask && fileObject.position.GetExactDist(sunstriderObject.spawnPoint) < 5.0f;
                if (sameObject)
                {
                    if (sunstriderObject.spawnMask & fileObject.spawnMask && sunstriderObject.spawnMask != fileObject.spawnMask)
                        handler->PSendSysMessage("Warning: Object %s looks like it's corresponding to %u but has different spawn mode. Object will be ignored.", sunstriderGUID, fileObject.ToString().c_str());

                    handler->PSendSysMessage("Ignoring object %s we already have", fileObject.ToString().c_str());
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
            handler->SendSysMessage("All gameobjects from file were already present, stopping here");
            return true;
        }

        //spawn them
        for (auto itr : fileObjects)
        {
            handler->PSendSysMessage("Summoning %s", itr.ToString().c_str());
            player->SummonGameObject(itr.entry, itr.position, itr.rot, 0);
            if (permanent)
            {
                auto pGameObj = new GameObject;
                if (!pGameObj->Create(player->GetMap()->GenerateLowGuid<HighGuid::GameObject>(), itr.entry, player->GetMap(), PHASEMASK_NORMAL, itr.position, itr.rot, 0, GO_STATE_READY))
                {
                    delete pGameObj;
                    handler->PSendSysMessage("Failed to create object %u", itr.entry);
                    continue;
                }
                pGameObj->SetRespawnTime(itr.spawnTimeSecs);
                pGameObj->SaveToDB(player->GetMap()->GetId(), itr.spawnMask);
            }
        }
        if(permanent)
            handler->SendSysMessage("Saved all to DB");

        return true;
    }
};

void AddSC_debug_commandscript()
{
    new debug_commandscript();
}
