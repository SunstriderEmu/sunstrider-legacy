#include "Chat.h"
#include "Language.h"
#include "CharacterCache.h"
#include "LogsDatabaseAccessor.h"
#include "ChaseMovementGenerator.h"
#include "FollowMovementGenerator.h"
#include "BattleGroundMgr.h"
#include "ArenaTeamMgr.h"
#include "Weather.h"
#include "GridNotifiersImpl.h"
#include "CellImpl.h"
#include "AccountMgr.h"
#include "ScriptMgr.h"
#include "SpellHistory.h"
#include "MovementDefines.h"
#include "Mail.h"
#include "Config.h"
#include "Pet.h"

#ifdef PLAYERBOT
#include "playerbot.h"
#include "GuildTaskMgr.h"
#endif
#ifdef TESTS
#include "TestMgr.h"
#endif

class misc_commandscript : public CommandScript
{
public:
    misc_commandscript() : CommandScript("misc_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> commandTable =
        {
            { "kick",           SEC_GAMEMASTER2,  true,  &HandleKickPlayerCommand,          "" },
            { "itemmove",       SEC_GAMEMASTER2,  false, &HandleItemMoveCommand,            "" },
            { "guid",           SEC_GAMEMASTER2,  false, &HandleGUIDCommand,                "" },
            { "unmute",         SEC_GAMEMASTER2,  true,  &HandleUnmuteCommand,              "" },
            { "mute",           SEC_GAMEMASTER2,  true,  &HandleMuteCommand,                "" },
            { "maxskill",       SEC_GAMEMASTER3,  false, &HandleMaxSkillCommand,            "" },
            { "setskill",       SEC_GAMEMASTER3,  false, &HandleSetSkillCommand,            "" },
            { "cooldown",       SEC_GAMEMASTER3,  false, &HandleCooldownCommand,            "" },
            { "additem",        SEC_GAMEMASTER3,  false, &HandleAddItemCommand,             "" },
            { "additemset",     SEC_GAMEMASTER3,  false, &HandleAddItemSetCommand,          "" },
            { "die",            SEC_GAMEMASTER3,  false, &HandleDieCommand,                 "" },
            { "distance",       SEC_GAMEMASTER3,  false, &HandleGetDistanceCommand,         "" },
            { "damage",         SEC_GAMEMASTER3,  false, &HandleDamageCommand,              "" },
            { "revive",         SEC_GAMEMASTER3,  false, &HandleReviveCommand,              "" },
            { "aura",           SEC_GAMEMASTER3,  false, &HandleAuraCommand,                "" },
            { "unaura",         SEC_GAMEMASTER3,  false, &HandleUnAuraCommand,              "" },
            { "linkgrave",      SEC_GAMEMASTER3,  false, &HandleLinkGraveCommand,           "" },
            { "neargrave",      SEC_GAMEMASTER3,  false, &HandleNearGraveCommand,           "" },
            { "hover",          SEC_GAMEMASTER3,  false, &HandleHoverCommand,               "" },
            { "levelup",        SEC_GAMEMASTER3,  false, &HandleLevelUpCommand,             "" },
            { "showarea",       SEC_GAMEMASTER3,  false, &HandleShowAreaCommand,            "" },
            { "bank",           SEC_GAMEMASTER3,  false, &HandleBankCommand,                "" },
            { "wchange",        SEC_GAMEMASTER3,  false, &HandleChangeWeather,              "" },
            { "respawn",        SEC_GAMEMASTER3,  false, &HandleRespawnCommand,             "" },
            { "movegens",       SEC_GAMEMASTER3,  false, &HandleMovegensCommand,            "" },
            { "plimit",         SEC_GAMEMASTER3,  true,  &HandlePLimitCommand,              "" },
            { "cometome",       SEC_GAMEMASTER3,  false, &HandleComeToMeCommand,            "" },
            { "flusharenapoints",SEC_GAMEMASTER3, false, &HandleFlushArenaPointsCommand,    "" },
            { "playall",        SEC_GAMEMASTER3,  false, &HandlePlayAllCommand,             "" },
            { "freeze",         SEC_GAMEMASTER3,  false, &HandleFreezeCommand,              "" },
            { "unfreeze",       SEC_GAMEMASTER3,  false, &HandleUnFreezeCommand,            "" },
            { "listfreeze",     SEC_GAMEMASTER3,  false, &HandleListFreezeCommand,          "" },
            { "possess",        SEC_GAMEMASTER3,  false, &HandlePossessCommand,             "" },
            { "unpossess",      SEC_GAMEMASTER3,  false, &HandleUnPossessCommand,           "" },
            { "bindsight",      SEC_GAMEMASTER3,  false, &HandleBindSightCommand,           "" },
            { "unbindsight",    SEC_GAMEMASTER3,  false, &HandleUnbindSightCommand,         "" },
            { "getmoveflags",   SEC_GAMEMASTER2,  false, &HandleGetMoveFlagsCommand,        "" },
            { "setmoveflags",   SEC_GAMEMASTER2,  false, &HandleSetMoveFlagsCommand,        "" },
            { "maxpool",        SEC_GAMEMASTER3,  true,  &HandleGetMaxCreaturePoolIdCommand,"" },
            { "settitle"       ,SEC_GAMEMASTER3,  false, &HandleSetTitleCommand,            "" },
            { "removetitle"    ,SEC_GAMEMASTER3,  false, &HandleRemoveTitleCommand,         "" },
            { "yolo",           SEC_GAMEMASTER1,  true,  &HandleYoloCommand,                "" },
            { "spellinfo",      SEC_GAMEMASTER3,  true,  &HandleSpellInfoCommand,           "" },
            { "hidearea",       SEC_GAMEMASTER3,  false, &HandleHideAreaCommand,            "" },
            //PLAYERBOT
            { "rndbot",         SEC_SUPERADMIN,   true,  &HandlePlayerbotConsoleCommand,    "" },
            { "bot",            SEC_SUPERADMIN,   true,  &HandlePlayerbotMgrCommand,        "" },
        };
        return commandTable;
    }

    //kick player
    static bool HandleKickPlayerCommand(ChatHandler* handler, char const* args)
    {
        const char* kickName = strtok((char*)args, " ");
        char* kickReason = strtok(nullptr, "\n");
        std::string reason = "No Reason";
        std::string kicker = "Console";
        if(kickReason)
            reason = kickReason;
        if(handler->GetSession())
            kicker = handler->GetSession()->GetPlayer()->GetName();

        if(!kickName)
         {
            Player* player = handler->GetSelectedPlayerOrSelf();
            if(!player)
            {
                handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
                handler->SetSentErrorMessage(true);
                return false;
            }

            if(player == handler->GetSession()->GetPlayer())
            {
                handler->SendSysMessage(LANG_COMMAND_KICKSELF);
                handler->SetSentErrorMessage(true);
                return false;
            }

            // check online security
            if (handler->HasLowerSecurity(player, ObjectGuid::Empty))
                return false;

            if(sWorld->getConfig(CONFIG_SHOW_KICK_IN_WORLD) == 1)
                sWorld->SendWorldText(LANG_COMMAND_KICKMESSAGE, player->GetName().c_str(), kicker.c_str(), reason.c_str());
            else
                handler->PSendSysMessage(LANG_COMMAND_KICKMESSAGE, player->GetName().c_str(), kicker.c_str(), reason.c_str());

            player->GetSession()->KickPlayer();
        }
        else
        {
            std::string name = kickName;
            if(!normalizePlayerName(name))
            {
                handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
                handler->SetSentErrorMessage(true);
                return false;
            }

            if(handler->GetSession() && name==handler->GetSession()->GetPlayer()->GetName())
            {
                handler->SendSysMessage(LANG_COMMAND_KICKSELF);
                handler->SetSentErrorMessage(true);
                return false;
            }

            Player* player = ObjectAccessor::FindConnectedPlayerByName(kickName);
            if(!player)
            {
                handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
                handler->SetSentErrorMessage(true);
                return false;
            }

            if(handler->GetSession() && player->GetSession()->GetSecurity() > handler->GetSession()->GetSecurity())
            {
                handler->SendSysMessage(LANG_YOURS_SECURITY_IS_LOW); //maybe replacement string for this later on
                handler->SetSentErrorMessage(true);
                return false;
            }

            if(sWorld->KickPlayer(name.c_str()))
            {
                if(sWorld->getConfig(CONFIG_SHOW_KICK_IN_WORLD) == 1)
                {

                    sWorld->SendWorldText(LANG_COMMAND_KICKMESSAGE, name.c_str(), kicker.c_str(), reason.c_str());
                }
                else
                {
                    handler->PSendSysMessage(LANG_COMMAND_KICKMESSAGE, name.c_str(), kicker.c_str(), reason.c_str());
                }
            }
            else
            {
                handler->PSendSysMessage(LANG_COMMAND_KICKNOTFOUNDPLAYER, name.c_str());
                return false;
            }
        }
        return true;
    }

    //move item to other slot
    static bool HandleItemMoveCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

        uint8 srcslot, dstslot;

        char* pParam1 = strtok((char*)args, " ");
        if (!pParam1)
            return false;

        char* pParam2 = strtok(nullptr, " ");
        if (!pParam2)
            return false;

        srcslot = (uint8)atoi(pParam1);
        dstslot = (uint8)atoi(pParam2);

        if(srcslot==dstslot)
            return true;

        if(!handler->GetSession()->GetPlayer()->IsValidPos(INVENTORY_SLOT_BAG_0,srcslot))
            return false;

        if(!handler->GetSession()->GetPlayer()->IsValidPos(INVENTORY_SLOT_BAG_0,dstslot))
            return false;

        uint16 src = ((INVENTORY_SLOT_BAG_0 << 8) | srcslot);
        uint16 dst = ((INVENTORY_SLOT_BAG_0 << 8) | dstslot);

        handler->GetSession()->GetPlayer()->SwapItem( src, dst );

        return true;
    }

    static bool HandleGUIDCommand(ChatHandler* handler, char const* /*args*/)
    {
        ObjectGuid guid = handler->GetSession()->GetPlayer()->GetTarget();

        if (guid == 0)
        {
            handler->SendSysMessage(LANG_NO_SELECTION);
            handler->SetSentErrorMessage(true);
            return false;
        }

        handler->PSendSysMessage(LANG_OBJECT_GUID, guid.ToString().c_str());
        return true;
    }

    //unmute player
    static bool HandleUnmuteCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

        char *charname = strtok((char*)args, " ");
        if (!charname)
            return false;

        std::string cname = charname;

        if(!normalizePlayerName(cname))
        {
            handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
            handler->SetSentErrorMessage(true);
            return false;
        }

        ObjectGuid guid = sCharacterCache->GetCharacterGuidByName(cname.c_str());
        if(!guid)
        {
            handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Player *chr = ObjectAccessor::FindPlayer(guid);

        // check security
        uint32 account_id = 0;
        uint32 security = 0;

        if (chr)
        {
            account_id = chr->GetSession()->GetAccountId();
            security = chr->GetSession()->GetSecurity();
        }
        else
        {
            account_id = sCharacterCache->GetCharacterAccountIdByGuid(guid);
            security = sAccountMgr->GetSecurity(account_id);
        }

        // must have strong lesser security level
        if (handler->HasLowerSecurity(chr, guid, true))
            return false;


        if (chr)
        {
            if(chr->CanSpeak())
            {
                handler->SendSysMessage(LANG_CHAT_ALREADY_ENABLED);
                handler->SetSentErrorMessage(true);
                return false;
            }

            chr->GetSession()->m_muteTime = 0;
        }

        LoginDatabase.PExecute("UPDATE account SET mutetime = '0' WHERE id = '%u'", account_id );
        LogsDatabaseAccessor::RemoveSanction(handler->GetSession(), account_id, 0, "", SANCTION_MUTE_ACCOUNT);

        if(chr)
            ChatHandler(chr).PSendSysMessage(LANG_YOUR_CHAT_ENABLED);

        handler->PSendSysMessage(LANG_YOU_ENABLE_CHAT, cname.c_str());
        return true;
    }

    //mute player for some times
    static bool HandleMuteCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

        char* charname = strtok((char*)args, " ");
        if (!charname)
            return false;

        std::string cname = charname;

        char* timetonotspeak = strtok(nullptr, " ");
        if(!timetonotspeak)
            return false;

        char* mutereason = strtok(nullptr, "");
        std::string mutereasonstr;
        if (!mutereason)
            return false;
    
        mutereasonstr = mutereason;
        
        uint32 notspeaktime = (uint32) atoi(timetonotspeak);

        if(!normalizePlayerName(cname))
        {
            handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
            handler->SetSentErrorMessage(true);
            return false;
        }

        ObjectGuid guid = sCharacterCache->GetCharacterGuidByName(cname.c_str());
        if(!guid)
        {
            handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Player *chr = ObjectAccessor::FindPlayer(guid);

        // check security
        uint32 account_id = 0;
        uint32 security = 0;

        if (chr)
        {
            account_id = chr->GetSession()->GetAccountId();
            security = chr->GetSession()->GetSecurity();
        }
        else
        {
            account_id = sCharacterCache->GetCharacterAccountIdByGuid(guid);
            security = sAccountMgr->GetSecurity(account_id);
        }

        if(handler->GetSession() && security >= handler->GetSession()->GetSecurity())
        {
            handler->SendSysMessage(LANG_YOURS_SECURITY_IS_LOW);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // must have strong lesser security level
        if (handler->HasLowerSecurity(chr, guid, true))
            return false;

        uint32 duration = notspeaktime*MINUTE;
        time_t mutetime = time(nullptr) + duration;

        if (chr)
            chr->GetSession()->m_muteTime = mutetime;
        
        // Prevent SQL injection
        LogsDatabase.EscapeString(mutereasonstr);
        LoginDatabase.PExecute("UPDATE account SET mutetime = " UI64FMTD " WHERE id = '%u'", uint64(mutetime), account_id );

        LogsDatabaseAccessor::Sanction(handler->GetSession(), account_id, 0, SANCTION_MUTE_ACCOUNT, duration, mutereasonstr);

        if(chr)
            ChatHandler(chr).PSendSysMessage(LANG_YOUR_CHAT_DISABLED, notspeaktime, mutereasonstr.c_str());

        handler->PSendSysMessage(LANG_YOU_DISABLE_CHAT, cname.c_str(), notspeaktime, mutereasonstr.c_str());

        return true;
    }

    static bool HandleMaxSkillCommand(ChatHandler* handler, char const* /*args*/)
    {
        Player* SelectedPlayer = handler->GetSelectedPlayerOrSelf();

        // each skills that have max skill value dependent from level seted to current level max skill value
        SelectedPlayer->UpdateSkillsToMaxSkillsForLevel();
        handler->SendSysMessage("Max skills set to target");
        return true;
    }

    static bool HandleSetSkillCommand(ChatHandler* handler, char const* args)
    {
        // number or [name] Shift-click form |color|Hskill:skill_id|h[name]|h|r
        char* skill_p = handler->extractKeyFromLink((char*)args,"Hskill");
        if(!skill_p)
            return false;

        char *level_p = strtok (nullptr, " ");

        if( !level_p)
            return false;

        char *max_p   = strtok (nullptr, " ");

        int32 skill = atoi(skill_p);

        if (skill <= 0)
        {
            handler->PSendSysMessage(LANG_INVALID_SKILL_ID, skill);
            handler->SetSentErrorMessage(true);
            return false;
        }

        int32 level = atol(level_p);
        if (level == 0)
            level = 1;

        Player * target = handler->GetSelectedPlayerOrSelf();
        if(!target)
        {
            handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        SkillLineEntry const* sl = sSkillLineStore.LookupEntry(skill);
        if(!sl)
        {
            handler->PSendSysMessage(LANG_INVALID_SKILL_ID, skill);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if(!target->GetSkillValue(skill))
        {
            handler->PSendSysMessage(LANG_SET_SKILL_ERROR, target->GetName().c_str(), skill, sl->name[0]);
            handler->SetSentErrorMessage(true);
            return false;
        }

        int32 max   = max_p ? atol (max_p) : target->GetPureMaxSkillValue(skill);

        if( level > max || max <= 0 )
            return false;

        uint16 step = (level - 1) / 75;
        target->SetSkill(skill, step, level, max); //remove skill if level == 0
        handler->PSendSysMessage(LANG_SET_SKILL, skill, sl->name[0], target->GetName().c_str(), level, max);

        return true;
    }

    static bool HandleCooldownCommand(ChatHandler* handler, char const* args)
    {
        Player* target = handler->GetSelectedPlayerOrSelf();

        if (!*args)
        {
            target->GetSpellHistory()->ResetAllCooldowns();
            handler->PSendSysMessage(LANG_REMOVEALL_COOLDOWN, target->GetName().c_str());
        }
        else
        {
            // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
            uint32 spell_id =  handler->extractSpellIdFromLink((char*)args);
            if(!spell_id)
                return false;

            if(!sSpellMgr->GetSpellInfo(spell_id))
            {
                handler->PSendSysMessage(LANG_UNKNOWN_SPELL, target==handler->GetSession()->GetPlayer() ? handler->GetTrinityString(LANG_YOU) : target->GetName().c_str());
                handler->SetSentErrorMessage(true);
                return false;
            }

            target->GetSpellHistory()->ResetCooldown(spell_id, true);
            handler->PSendSysMessage(LANG_REMOVE_COOLDOWN, spell_id, target==handler->GetSession()->GetPlayer() ? handler->GetTrinityString(LANG_YOU) : target->GetName().c_str());
        }
        return true;
    }

    static bool HandleAddItemCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

        uint32 itemId = 0;

        if(args[0]=='[')                                        // [name] manual form
        {
            char* citemName = citemName = strtok((char*)args, "]");

            if(citemName && citemName[0])
            {
                std::string itemName = citemName+1;
                WorldDatabase.EscapeString(itemName);
                QueryResult result = WorldDatabase.PQuery("SELECT entry FROM item_template WHERE name = '%s'", itemName.c_str());
                if (!result)
                {
                    handler->PSendSysMessage(LANG_COMMAND_COULDNOTFIND, citemName+1);
                    handler->SetSentErrorMessage(true);
                    return false;
                }
                itemId = result->Fetch()->GetUInt16();
            }
            else
                return false;
        }
        else                                                    // item_id or [name] Shift-click form |color|Hitem:item_id:0:0:0|h[name]|h|r
        {
            char* cId = handler->extractKeyFromLink((char*)args,"Hitem");
            if(!cId)
                return false;
            itemId = atol(cId);
        }

        char* ccount = strtok(nullptr, " ");

        int32 count = 1;

        if (ccount)
            count = strtol(ccount, nullptr, 10);

        if (count == 0)
            count = 1;

        Player* pl = handler->GetSession()->GetPlayer();
        Player* plTarget = handler->GetSelectedPlayerOrSelf();

        TC_LOG_DEBUG("command",handler->GetTrinityString(LANG_ADDITEM), itemId, count);

        ItemTemplate const *pProto = sObjectMgr->GetItemTemplate(itemId);
        if(!pProto)
        {
            handler->PSendSysMessage(LANG_COMMAND_ITEMIDINVALID, itemId);
            handler->SetSentErrorMessage(true);
            return false;
        }

        //Subtract
        if (count < 0)
        {
            plTarget->DestroyItemCount(itemId, -count, true, false);
            handler->PSendSysMessage(LANG_REMOVEITEM, itemId, -count, plTarget->GetName().c_str());
            return true;
        }

        //Adding items
        uint32 noSpaceForCount = 0;

        // check space and find places
        ItemPosCountVec dest;
        uint8 msg = plTarget->CanStoreNewItem( NULL_BAG, NULL_SLOT, dest, itemId, count, &noSpaceForCount );
        if( msg != EQUIP_ERR_OK )                               // convert to possible store amount
            count -= noSpaceForCount;

        if( count == 0 || dest.empty())                         // can't add any
        {
            handler->PSendSysMessage(LANG_ITEM_CANNOT_CREATE, itemId, noSpaceForCount );
            handler->SetSentErrorMessage(true);
            return false;
        }

        Item* item = plTarget->StoreNewItem( dest, itemId, true, Item::GenerateItemRandomPropertyId(itemId));

        // remove binding (let GM give it to another player later)
        if(pl==plTarget)
            for(ItemPosCountVec::const_iterator itr = dest.begin(); itr != dest.end(); ++itr)
                if(Item* item1 = pl->GetItemByPos(itr->pos))
                    item1->SetBinding( false );

        if(count > 0 && item)
        {
            pl->SendNewItem(item,count,false,true);
            if(pl!=plTarget)
                plTarget->SendNewItem(item,count,true,false);
        }

        if(noSpaceForCount > 0)
            handler->PSendSysMessage(LANG_ITEM_CANNOT_CREATE, itemId, noSpaceForCount);

        return true;
    }

    static bool HandleAddItemSetCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

        char* cId = handler->extractKeyFromLink((char*)args,"Hitemset"); // number or [name] Shift-click form |color|Hitemset:itemset_id|h[name]|h|r
        if (!cId)
            return false;

        uint32 itemsetId = atol(cId);

        // prevent generation all items with itemset field value '0'
        if (itemsetId == 0)
        {
            handler->PSendSysMessage(LANG_NO_ITEMS_FROM_ITEMSET_FOUND,itemsetId);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Player* pl = handler->GetSession()->GetPlayer();
        Player* plTarget = handler->GetSelectedPlayerOrSelf();

        TC_LOG_DEBUG("command",handler->GetTrinityString(LANG_ADDITEMSET), itemsetId);

        bool found = false;

        ItemTemplateContainer const& its = sObjectMgr->GetItemTemplateStore();
        for (const auto & it : its)
        {
            ItemTemplate const *pProto = &(it.second);
            if (!pProto)
                continue;

            if (pProto->ItemSet == itemsetId)
            {
                found = true;
                ItemPosCountVec dest;
                uint8 msg = plTarget->CanStoreNewItem( NULL_BAG, NULL_SLOT, dest, pProto->ItemId, 1 );
                if (msg == EQUIP_ERR_OK)
                {
                    Item* item = plTarget->StoreNewItem( dest, pProto->ItemId, true);

                    // remove binding (let GM give it to another player later)
                    if (pl==plTarget)
                        item->SetBinding( false );

                    pl->SendNewItem(item,1,false,true);
                    if (pl!=plTarget)
                        plTarget->SendNewItem(item,1,true,false);
                }
                else
                {
                    pl->SendEquipError( msg, nullptr, nullptr );
                    handler->PSendSysMessage(LANG_ITEM_CANNOT_CREATE, pProto->ItemId, 1);
                }
            }
        }

        if (!found)
        {
            handler->PSendSysMessage(LANG_NO_ITEMS_FROM_ITEMSET_FOUND,itemsetId);

            handler->SetSentErrorMessage(true);
            return false;
        }

        return true;
    }

    static bool HandleDieCommand(ChatHandler* handler, char const* /*args*/)
    {
        Unit* target = handler->GetSelectedUnit();

        if(!target || !handler->GetSession()->GetPlayer()->GetTarget())
        {
            handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (Player* player = target->ToPlayer())
            if (handler->HasLowerSecurity(player, ObjectGuid::Empty, false))
                return false;

        if(target->IsAlive())
            Unit::Kill(handler->GetSession()->GetPlayer(), target);

        return true;
    }

    static bool HandleGetDistanceCommand(ChatHandler* handler, char const* /*args*/)
    {
        Unit* pUnit = handler->GetSelectedUnit();

        if (!pUnit)
        {
            handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        handler->PSendSysMessage(LANG_DISTANCE, handler->GetSession()->GetPlayer()->GetDistance(pUnit), handler->GetSession()->GetPlayer()->GetDistance2d(pUnit));
        handler->PSendSysMessage("Exact distance 2d: %f", handler->GetSession()->GetPlayer()->GetExactDistance2d(pUnit->GetPositionX(), pUnit->GetPositionY()));

        return true;
    }

    static bool HandleDamageCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

        Unit* target = handler->GetSelectedUnit();

        if(!target || !handler->GetSession()->GetPlayer()->GetTarget())
        {
            handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (Player* player = target->ToPlayer())
            if (handler->HasLowerSecurity(player, ObjectGuid::Empty, false))
                return false;

        if( !target->IsAlive() )
            return true;

        char* damageStr = strtok((char*)args, " ");
        if(!damageStr)
            return false;

        int32 damage = atoi((char*)damageStr);
        if(damage <=0)
            return true;

        char* schoolStr = strtok((char*)nullptr, " ");

        // flat melee damage without resistence/etc reduction
        if(!schoolStr)
        {
            Unit::DealDamage(handler->GetSession()->GetPlayer(), target, damage, nullptr, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, nullptr, false);
            handler->GetSession()->GetPlayer()->SendAttackStateUpdate(HITINFO_NORMALSWING2, target, 1, SPELL_SCHOOL_MASK_NORMAL, damage, 0, 0, VICTIMSTATE_NORMAL, 0);
            return true;
        }

        uint32 school = schoolStr ? atoi((char*)schoolStr) : SPELL_SCHOOL_NORMAL;
        if(school >= MAX_SPELL_SCHOOL)
            return false;

        SpellSchoolMask schoolmask = SpellSchoolMask(1 << school);

        if (Unit::IsDamageReducedByArmor(schoolmask))
            damage = Unit::CalcArmorReducedDamage(handler->GetSession()->GetPlayer(), target, damage, nullptr, BASE_ATTACK);

        char* spellStr = strtok((char*)nullptr, " ");

        // melee damage by specific school
        if(!spellStr)
        {
            DamageInfo damageInfo(handler->GetSession()->GetPlayer(), target, damage, nullptr, schoolmask, SPELL_DIRECT_DAMAGE, BASE_ATTACK);
            Unit::CalcAbsorbResist(damageInfo);

            Unit::DealDamage(handler->GetSession()->GetPlayer(), target, damageInfo.GetDamage(), nullptr, DIRECT_DAMAGE, schoolmask, nullptr, false);
            handler->GetSession()->GetPlayer()->SendAttackStateUpdate(HITINFO_NORMALSWING2, target, 1, schoolmask, damageInfo.GetDamage(), damageInfo.GetAbsorb(), damageInfo.GetResist(), VICTIMSTATE_NORMAL, 0);
            return true;
        }

        // non-melee damage

        // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
        uint32 spellid =  handler->extractSpellIdFromLink((char*)args);
        if(!spellid)
            return false;

        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellid);
        if (!spellInfo)
            return false;

        Player* attacker = handler->GetSession()->GetPlayer();
        SpellNonMeleeDamage dmgInfo(attacker, target, spellid, spellInfo->GetSchoolMask());
        Unit::DealDamageMods(dmgInfo.target, dmgInfo.damage, &dmgInfo.absorb);
        attacker->SendSpellNonMeleeDamageLog(&dmgInfo);
        attacker->DealSpellDamage(&dmgInfo, true);
        return true;
    }

    static bool HandleReviveCommand(ChatHandler* handler, char const* args)
    {
        Player* SelectedPlayer = nullptr;

        if (*args)
        {
            std::string name = args;
            if(!normalizePlayerName(name))
            {
                handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
                handler->SetSentErrorMessage(true);
                return false;
            }

            SelectedPlayer = ObjectAccessor::FindConnectedPlayerByName(name.c_str());
        }
        else
            SelectedPlayer = handler->GetSelectedPlayerOrSelf();

        if(!SelectedPlayer)
        {
            handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        SelectedPlayer->ResurrectPlayer(0.5f);
        SelectedPlayer->SpawnCorpseBones();
        SelectedPlayer->SaveToDB();
        return true;
    }

    static bool HandleAuraCommand(ChatHandler* handler, char const* args)
    {
        char* px = strtok((char*)args, " ");
        if (!px)
            return false;

        Unit *target = handler->GetSelectedUnit();
        if(!target)
        {
            handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
        uint32 spellID =  handler->extractSpellIdFromLink((char*)args);

        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellID);
        if (spellInfo)
        {
             AuraCreateInfo createInfo(spellInfo, MAX_EFFECT_MASK, target);
            createInfo.SetCaster(target);

            Aura::TryRefreshStackOrCreate(createInfo);
        }

        return true;
    }

    static bool HandleUnAuraCommand(ChatHandler* handler, char const* args)
    {
        char* px = strtok((char*)args, " ");
        if (!px)
            return false;

        Unit *target = handler->GetSelectedUnit();
        if(!target)
        {
            handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        std::string argstr = args;
        if (argstr == "all")
        {
            target->RemoveAllAuras();
            return true;
        }

        uint32 spellID = (uint32)atoi(px);
        target->RemoveAurasDueToSpell(spellID);

        return true;
    }

    static bool HandleLinkGraveCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

        char* px = strtok((char*)args, " ");
        if (!px)
            return false;

        uint32 g_id = (uint32)atoi(px);

        uint32 g_team;

        char* px2 = strtok(nullptr, " ");

        if (!px2)
            g_team = 0;
        else if (strncmp(px2,"horde",6)==0)
            g_team = HORDE;
        else if (strncmp(px2,"alliance",9)==0)
            g_team = ALLIANCE;
        else
            return false;

        WorldSafeLocsEntry const* graveyard =  sWorldSafeLocsStore.LookupEntry(g_id);

        if(!graveyard )
        {
            handler->PSendSysMessage(LANG_COMMAND_GRAVEYARDNOEXIST, g_id);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Player* player = handler->GetSession()->GetPlayer();

        uint32 zoneId = player->GetZoneId();

        AreaTableEntry const *areaEntry = sAreaTableStore.LookupEntry(zoneId);
        if(!areaEntry || areaEntry->zone !=0 )
        {
            handler->PSendSysMessage(LANG_COMMAND_GRAVEYARDWRONGZONE, g_id,zoneId);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if(sObjectMgr->AddGraveYardLink(g_id,player->GetZoneId(),g_team))
            handler->PSendSysMessage(LANG_COMMAND_GRAVEYARDLINKED, g_id,zoneId);
        else
            handler->PSendSysMessage(LANG_COMMAND_GRAVEYARDALRLINKED, g_id,zoneId);

        return true;
    }

    static bool HandleNearGraveCommand(ChatHandler* handler, char const* args)
    {
        uint32 g_team;

        size_t argslen = strlen(args);

        if(!*args)
            g_team = 0;
        else if (strncmp((char*)args,"horde",argslen)==0)
            g_team = HORDE;
        else if (strncmp((char*)args,"alliance",argslen)==0)
            g_team = ALLIANCE;
        else
            return false;

        Player* player = handler->GetSession()->GetPlayer();

        WorldSafeLocsEntry const* graveyard = sObjectMgr->GetClosestGraveYard(
            player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(),player->GetMapId(),g_team);

        if(graveyard)
        {
            uint32 g_id = graveyard->ID;

            GraveYardData const* data = sObjectMgr->FindGraveYardData(g_id,player->GetZoneId());
            if (!data)
            {
                handler->PSendSysMessage(LANG_COMMAND_GRAVEYARDERROR,g_id);
                handler->SetSentErrorMessage(true);
                return false;
            }

            g_team = data->team;

            std::string team_name = handler->GetTrinityString(LANG_COMMAND_GRAVEYARD_NOTEAM);

            if(g_team == 0)
                team_name = handler->GetTrinityString(LANG_COMMAND_GRAVEYARD_ANY);
            else if(g_team == HORDE)
                team_name = handler->GetTrinityString(LANG_COMMAND_GRAVEYARD_HORDE);
            else if(g_team == ALLIANCE)
                team_name = handler->GetTrinityString(LANG_COMMAND_GRAVEYARD_ALLIANCE);

            handler->PSendSysMessage(LANG_COMMAND_GRAVEYARDNEAREST, g_id,team_name.c_str(),player->GetZoneId());
        }
        else
        {
            std::string team_name;

            if(g_team == 0)
                team_name = handler->GetTrinityString(LANG_COMMAND_GRAVEYARD_ANY);
            else if(g_team == HORDE)
                team_name = handler->GetTrinityString(LANG_COMMAND_GRAVEYARD_HORDE);
            else if(g_team == ALLIANCE)
                team_name = handler->GetTrinityString(LANG_COMMAND_GRAVEYARD_ALLIANCE);

            if(g_team == ~uint32(0))
                handler->PSendSysMessage(LANG_COMMAND_ZONENOGRAVEYARDS, player->GetZoneId());
            else
                handler->PSendSysMessage(LANG_COMMAND_ZONENOGRAFACTION, player->GetZoneId(),team_name.c_str());
        }

        return true;
    }

    static bool HandleHoverCommand(ChatHandler* handler, char const* args)
    {
        char* px = strtok((char*)args, " ");
        uint32 flag;
        if (!px)
            flag = 1;
        else
            flag = atoi(px);

        handler->GetSession()->GetPlayer()->SetHover(flag);

        if (flag)
            handler->SendSysMessage(LANG_HOVER_ENABLED);
        else
            handler->SendSysMessage(LANG_HOVER_DISABLED);

        return true;
    }

    static bool HandleLevelUpCommand(ChatHandler* handler, char const* args)
    {
        char* px = strtok((char*)args, " ");
        char* py = strtok((char*)nullptr, " ");

        // command format parsing
        char* pname = (char*)nullptr;
        int addlevel = 1;

        if(px && py)                                            // .levelup name level
        {
            addlevel = atoi(py);
            pname = px;
        }
        else if(px && !py)                                      // .levelup name OR .levelup level
        {
            if(isalpha(px[0]))                                  // .levelup name
                pname = px;
            else                                                // .levelup level
                addlevel = atoi(px);
        }
        // else .levelup - nothing do for preparing

        // player
        Player *chr = nullptr;
        ObjectGuid chr_guid;

        std::string name;

        if(pname)                                               // player by name
        {
            name = pname;
            if(!normalizePlayerName(name))
            {
                handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
                handler->SetSentErrorMessage(true);
                return false;
            }

            chr = ObjectAccessor::FindConnectedPlayerByName(name.c_str());
            if(!chr)                                            // not in game
            {
                chr_guid = sCharacterCache->GetCharacterGuidByName(name);
                if (chr_guid == 0)
                {
                    handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
                    handler->SetSentErrorMessage(true);
                    return false;
                }
            }
        }
        else                                                    // player by selection
        {
            chr = handler->GetSelectedPlayerOrSelf();

            if (chr == nullptr)
            {
                handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
                handler->SetSentErrorMessage(true);
                return false;
            }

            name = chr->GetName();
        }

        assert(chr || chr_guid);

        int32 oldlevel = chr ? chr->GetLevel() : Player::GetLevelFromStorage(chr_guid);
        int32 newlevel = oldlevel + addlevel;
        if(newlevel < 1)
            newlevel = 1;
        if(newlevel > STRONG_MAX_LEVEL)                         // hardcoded maximum level
            newlevel = STRONG_MAX_LEVEL;

        if(chr)
        {
            chr->GiveLevel(newlevel);
            chr->InitTalentForLevel();
            chr->SetUInt32Value(PLAYER_XP,0);

            if(oldlevel == newlevel)
                ChatHandler(chr).SendSysMessage(LANG_YOURS_LEVEL_PROGRESS_RESET);
            else
            if(oldlevel < newlevel)
                ChatHandler(chr).PSendSysMessage(LANG_YOURS_LEVEL_UP,newlevel-oldlevel);
            else
            if(oldlevel > newlevel)
                ChatHandler(chr).PSendSysMessage(LANG_YOURS_LEVEL_DOWN,newlevel-oldlevel);
        }
        else
        {
            // update level and XP at level, all other will be updated at loading
            CharacterDatabase.PExecute("UPDATE characters SET level = '%u', xp = 0 WHERE guid = '%u'", newlevel, chr_guid.GetCounter());
        }

        sCharacterCache->UpdateCharacterLevel(chr_guid.GetCounter(), newlevel);

        if(handler->GetSession() && handler->GetSession()->GetPlayer() != chr)                       // including chr==NULL
            handler->PSendSysMessage(LANG_YOU_CHANGE_LVL,name.c_str(),newlevel);
        return true;
    }

    static bool HandleShowAreaCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

        int area = atoi((char*)args);

        Player *chr = handler->GetSelectedPlayerOrSelf();
        if (chr == nullptr)
        {
            handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        int offset = area / 32;
        uint32 val = (uint32)(1 << (area % 32));

        if(offset >= 128)
        {
            handler->SendSysMessage(LANG_BAD_VALUE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        uint32 currFields = chr->GetUInt32Value(PLAYER_EXPLORED_ZONES_1 + offset);
        chr->SetUInt32Value(PLAYER_EXPLORED_ZONES_1 + offset, (uint32)(currFields | val));

        handler->SendSysMessage(LANG_EXPLORE_AREA);
        return true;
    }

    static bool HandleHideAreaCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

        int area = atoi((char*)args);

        Player *chr = handler->GetSelectedPlayerOrSelf();
        if (chr == nullptr)
        {
            handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        int offset = area / 32;
        uint32 val = (uint32)(1 << (area % 32));

        if(offset >= 128)
        {
            handler->SendSysMessage(LANG_BAD_VALUE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        uint32 currFields = chr->GetUInt32Value(PLAYER_EXPLORED_ZONES_1 + offset);
        chr->SetUInt32Value(PLAYER_EXPLORED_ZONES_1 + offset, (uint32)(currFields ^ val));

        handler->SendSysMessage(LANG_UNEXPLORE_AREA);
        return true;
    }

    static bool HandleBankCommand(ChatHandler* handler, char const* /*args*/)
    {
        handler->GetSession()->SendShowBank( handler->GetSession()->GetPlayer()->GetGUID() );

        return true;
    }

    static bool HandleChangeWeather(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

        //Weather is OFF
        if (!sWorld->getConfig(CONFIG_WEATHER))
        {
            handler->SendSysMessage(LANG_WEATHER_DISABLED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        //*Change the weather of a cell
        char* px = strtok((char*)args, " ");
        char* py = strtok(nullptr, " ");

        if (!px || !py)
            return false;

        uint32 type = (uint32)atoi(px);                         //0 to 3, 0: fine, 1: rain, 2: snow, 3: sand
        float grade = (float)atof(py);                          //0 to 1, sending -1 is instand good weather

        Player *player = handler->GetSession()->GetPlayer();
        uint32 zoneid = player->GetZoneId();

        Weather* wth = sWorld->FindWeather(zoneid);

        if(!wth)
            wth = sWorld->AddWeather(zoneid);
        if(!wth)
        {
            handler->SendSysMessage(LANG_NO_WEATHER);
            handler->SetSentErrorMessage(true);
            return false;
        }

        wth->SetWeather(WeatherType(type), grade);

        return true;
    }

    static bool HandleRespawnCommand(ChatHandler* handler, char const* /*args*/)
    {
        Player* player = handler->GetSession()->GetPlayer();

        // accept only explicitly selected target (not implicitly self targeting case)
        Unit* target = handler->GetSelectedUnit();
        if(player->GetTarget() && target)
        {
            if(target->GetTypeId()!=TYPEID_UNIT)
            {
                handler->SendSysMessage(LANG_SELECT_CREATURE);
                handler->SetSentErrorMessage(true);
                return false;
            }

            if(target->IsDead())
                (target->ToCreature())->Respawn(true);
            return true;
        }

        // First handle any creatures that still have a corpse around
        Trinity::RespawnDo u_do;
        Trinity::WorldObjectWorker<Trinity::RespawnDo> worker(player, u_do);
        Cell::VisitGridObjects(player, worker, player->GetGridActivationRange());

        // Now handle any that had despawned, but had respawn time logged.
        std::vector<RespawnInfo*> data;
        player->GetMap()->GetRespawnInfo(data, SPAWN_TYPEMASK_ALL, 0);
        if (!data.empty())
        {
            uint32 const gridId = Trinity::ComputeGridCoord(player->GetPositionX(), player->GetPositionY()).GetId();
            for (RespawnInfo* info : data)
                if (info->gridId == gridId)
                    player->GetMap()->RemoveRespawnTime(info, true);
        }

        return true;
    }

    static bool HandleMovegensCommand(ChatHandler* handler, char const* /*args*/)
    {
        Unit* unit = handler->GetSelectedUnit();
        if(!unit)
        {
            handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        handler->PSendSysMessage(LANG_MOVEGENS_LIST,(unit->GetTypeId()==TYPEID_PLAYER ? "Player" : "Creature" ),unit->GetGUID().GetCounter());

        if (unit->GetMotionMaster()->Empty())
        {
            handler->SendSysMessage("Empty");
            return true;
        }

        MotionMaster* motionMaster = unit->GetMotionMaster();
        float x, y, z;
        motionMaster->GetDestination(x, y, z);

        std::vector<MovementGeneratorInformation> list = unit->GetMotionMaster()->GetMovementGeneratorsInformation();
        for (MovementGeneratorInformation info : list)
        {
            switch (info.Type)
            {
                case IDLE_MOTION_TYPE:
                    handler->SendSysMessage(LANG_MOVEGENS_IDLE);
                    break;
                case RANDOM_MOTION_TYPE:
                    handler->SendSysMessage(LANG_MOVEGENS_RANDOM);
                    break;
                case WAYPOINT_MOTION_TYPE:
                    handler->SendSysMessage(LANG_MOVEGENS_WAYPOINT);
                    break;
                case ANIMAL_RANDOM_MOTION_TYPE:
                    handler->SendSysMessage(LANG_MOVEGENS_ANIMAL_RANDOM);
                    break;
                case CONFUSED_MOTION_TYPE:
                    handler->SendSysMessage(LANG_MOVEGENS_CONFUSED);
                    break;
                case CHASE_MOTION_TYPE:
                {
                    if (info.TargetGUID.IsEmpty())
                        handler->SendSysMessage(LANG_MOVEGENS_CHASE_NULL);
                    else if (info.TargetGUID.IsPlayer())
                        handler->PSendSysMessage(LANG_MOVEGENS_CHASE_PLAYER, info.TargetName.c_str(), info.TargetGUID.GetCounter());
                    else
                        handler->PSendSysMessage(LANG_MOVEGENS_CHASE_CREATURE, info.TargetName.c_str(), info.TargetGUID.GetCounter());
                    break;
                }
                case FOLLOW_MOTION_TYPE:
                {
                    if(info.TargetGUID.IsEmpty())
                        handler->SendSysMessage(LANG_MOVEGENS_FOLLOW_NULL);
                    else if (info.TargetGUID.IsPlayer())
                        handler->PSendSysMessage(LANG_MOVEGENS_FOLLOW_PLAYER, info.TargetName.c_str(), info.TargetGUID.GetCounter());
                    else
                        handler->PSendSysMessage(LANG_MOVEGENS_FOLLOW_CREATURE, info.TargetName.c_str(), info.TargetGUID.GetCounter());
                    break;
                }
                case HOME_MOTION_TYPE:
                {
                    if (unit->GetTypeId() == TYPEID_UNIT)
                        handler->PSendSysMessage(LANG_MOVEGENS_HOME_CREATURE, x, y, z);
                    else
                        handler->SendSysMessage(LANG_MOVEGENS_HOME_PLAYER);
                    break;
                }
                case FLIGHT_MOTION_TYPE:
                    handler->SendSysMessage(LANG_MOVEGENS_FLIGHT);
                    break;
                case POINT_MOTION_TYPE:
                    handler->PSendSysMessage(LANG_MOVEGENS_POINT, x, y, z);
                    break;
                case FLEEING_MOTION_TYPE:
                    handler->SendSysMessage(LANG_MOVEGENS_FEAR);
                    break;
                case DISTRACT_MOTION_TYPE:
                    handler->SendSysMessage(LANG_MOVEGENS_DISTRACT);
                    break;
                case EFFECT_MOTION_TYPE:
                    handler->SendSysMessage(LANG_MOVEGENS_EFFECT);
                    break;
                case FORMATION_MOTION_TYPE:
                    handler->SendSysMessage(LANG_MOVEGENS_FORMATION);
                    break;
                default:
                    handler->PSendSysMessage(LANG_MOVEGENS_UNKNOWN, info.Type);
                    break;
            }
        }
    
        return true;
    }

    static bool HandlePLimitCommand(ChatHandler* handler, char const* args)
    {
        if(*args)
        {
            char* param = strtok((char*)args, " ");
            if(!param)
                return false;

            int l = strlen(param);

            if(strncmp(param,"player",l) == 0 )
                sWorld->SetPlayerLimit(-SEC_PLAYER);
            else if(strncmp(param,"moderator",l) == 0 )
                sWorld->SetPlayerLimit(-SEC_GAMEMASTER1);
            else if(strncmp(param,"gamemaster",l) == 0 )
                sWorld->SetPlayerLimit(-SEC_GAMEMASTER2);
            else if(strncmp(param,"administrator",l) == 0 )
                sWorld->SetPlayerLimit(-SEC_GAMEMASTER3);
            else if(strncmp(param,"reset",l) == 0 )
                sWorld->SetPlayerLimit(sConfigMgr->GetIntDefault("PlayerLimit", DEFAULT_PLAYER_LIMIT) );
            else
            {
                int val = atoi(param);
                if(val < -SEC_GAMEMASTER3) val = -SEC_GAMEMASTER3;

                sWorld->SetPlayerLimit(val);
            }

            // kick all low security level players
            if(sWorld->GetPlayerAmountLimit() > SEC_PLAYER)
                sWorld->KickAllLess(sWorld->GetPlayerSecurityLimit());
        }

        uint32 pLimit = sWorld->GetPlayerAmountLimit();
        AccountTypes allowedAccountType = sWorld->GetPlayerSecurityLimit();
        char const* secName = "";
        switch(allowedAccountType)
        {
            case SEC_PLAYER:        secName = "Player";        break;
            case SEC_GAMEMASTER1:     secName = "Moderator";     break;
            case SEC_GAMEMASTER2:    secName = "Gamemaster";    break;
            case SEC_GAMEMASTER3: secName = "Administrator"; break;
            default:                secName = "<unknown>";     break;
        }

        handler->PSendSysMessage("Player limits: amount %u, min. security level %s.",pLimit,secName);

        return true;
    }

    static bool HandleComeToMeCommand(ChatHandler* handler, char const* args)
    {
        char* newFlagStr = strtok((char*)args, " ");

        if(!newFlagStr)
            return false;

        uint32 newFlags = (uint32)strtoul(newFlagStr, nullptr, 0);

        Creature* caster =  handler->GetSelectedCreature();
        if(!caster)
        {
            handler->GetSession()->GetPlayer()->SetUnitMovementFlags(newFlags);
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        caster->SetUnitMovementFlags(newFlags);

        Player* pl = handler->GetSession()->GetPlayer();

        caster->GetMotionMaster()->MovePoint(0, pl->GetPositionX(), pl->GetPositionY(), pl->GetPositionZ());
        return true;
    }

    static bool HandleFlushArenaPointsCommand(ChatHandler* handler, char const* args)
    {
        sArenaTeamMgr->DistributeArenaPoints();
        return true;
    }

    static bool HandlePlayAllCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

        uint32 soundId = atoi((char*)args);

        if(!sSoundEntriesStore.LookupEntry(soundId))
        {
            handler->PSendSysMessage(LANG_SOUND_NOT_EXIST, soundId);
            handler->SetSentErrorMessage(true);
            return false;
        }

        WorldPacket data(SMSG_PLAY_SOUND, 12);
        data << uint32(soundId) << handler->GetSession()->GetPlayer()->GetGUID();
        sWorld->SendGlobalMessage(&data);

        handler->PSendSysMessage(LANG_COMMAND_PLAYED_TO_ALL, soundId);
        return true;
    }

    static bool HandleFreezeCommand(ChatHandler* handler, char const* args)
    {
        std::string name;
        Player* player;
        char* TargetName = strtok((char*)args, " "); //get entered name
        if (!TargetName) //if no name entered use target
        {
            player = handler->GetSelectedPlayer();
            if (player) //prevent crash with creature as target
            {
                name = player->GetName();
                normalizePlayerName(name);
            }
        }
        else // if name entered
        {
            name = TargetName;
            normalizePlayerName(name);
            player = ObjectAccessor::FindConnectedPlayerByName(name.c_str()); //get player by name
        }

        if (!player)
        {
            handler->SendSysMessage(LANG_COMMAND_FREEZE_WRONG);
            return true;
        }

        if (player==handler->GetSession()->GetPlayer())
        {
            handler->SendSysMessage(LANG_COMMAND_FREEZE_ERROR);
            return true;
        }

        //effect
        if ((player) && (!(player==handler->GetSession()->GetPlayer())))
        {
            handler->PSendSysMessage(LANG_COMMAND_FREEZE,name.c_str());

            //stop combat + make player unattackable + duel stop + stop some spells
            player->SetFaction(FACTION_FRIENDLY);
            player->CombatStop();
            if(player->IsNonMeleeSpellCast(true))
                player->InterruptNonMeleeSpells(true);
            player->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            //player->SetUInt32Value(PLAYER_DUEL_TEAM, 1);

            //if player class = hunter || warlock remove pet if alive
            if((player->GetClass() == CLASS_HUNTER) || (player->GetClass() == CLASS_WARLOCK))
            {
                if(Pet* pet = player->GetPet())
                {
                    pet->SavePetToDB(PET_SAVE_AS_CURRENT);
                    // not let dismiss dead pet
                    if(pet && pet->IsAlive())
                        player->RemovePet(pet, PET_SAVE_NOT_IN_SLOT);
                }
            }

            Aura* freeze = player->AddAura(9454, player);
            if (freeze)
            {
                freeze->SetDuration(-1);
                handler->PSendSysMessage(LANG_COMMAND_FREEZE, player->GetName().c_str());
                // save player
                player->SaveToDB();
                return true;
            }
        }
        return true;
    }

    static bool HandleUnFreezeCommand(ChatHandler* handler, char const* args)
    {
        std::string name;
        Player* player;
        char* TargetName = strtok((char*)args, " "); //get entered name
        if (!TargetName) //if no name entered use target
        {
            player = handler->GetSelectedPlayer();
            if (player) //prevent crash with creature as target
            {
                name = player->GetName();
            }
        }

        else // if name entered
        {
            name = TargetName;
            normalizePlayerName(name);
            player = ObjectAccessor::FindConnectedPlayerByName(name.c_str()); //get player by name
        }

        //effect
        if (player)
        {
            handler->PSendSysMessage(LANG_COMMAND_UNFREEZE,name.c_str());

            //Reset player faction + allow combat + allow duels
            player->SetFactionForRace(player->GetRace());
            player->RemoveFlag (UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);

            //allow movement and spells
            uint32 spellID = 9454;
            player->RemoveAurasDueToSpell(spellID);

            //save player
            player->SaveToDB();
        }

        if (!player)
        {
            if (TargetName)
            {
                //check for offline players
                QueryResult result = CharacterDatabase.PQuery("SELECT characters.guid FROM `characters` WHERE characters.name = '%s'",name.c_str());
                if(!result)
                {
                    handler->SendSysMessage(LANG_COMMAND_FREEZE_WRONG);
                    return true;
                }
                //if player found: delete his freeze aura
                Field *fields=result->Fetch();
                ObjectGuid pguid = ObjectGuid(fields[0].GetUInt64());
                CharacterDatabase.PQuery("DELETE FROM `character_aura` WHERE character_aura.spell = 9454 AND character_aura.guid = '%u'",pguid);
                handler->PSendSysMessage(LANG_COMMAND_UNFREEZE,name.c_str());
                return true;
            }
            else
            {
                handler->SendSysMessage(LANG_COMMAND_FREEZE_WRONG);
                return true;
            }
        }

        return true;
    }

    static bool HandleListFreezeCommand(ChatHandler* handler, char const* args)
    {
        //Get names from DB
        QueryResult result = CharacterDatabase.PQuery("SELECT characters.name FROM `characters` LEFT JOIN `character_aura` ON (characters.guid = character_aura.guid) WHERE character_aura.spell = 9454");
        if(!result)
        {
            handler->SendSysMessage(LANG_COMMAND_NO_FROZEN_PLAYERS);
            return true;
        }
        //Header of the names
        handler->PSendSysMessage(LANG_COMMAND_LIST_FREEZE);

        //Output of the results
        do
        {
            Field *fields = result->Fetch();
            std::string fplayers = fields[0].GetString();
            handler->PSendSysMessage(LANG_COMMAND_FROZEN_PLAYERS,fplayers.c_str());
        } while (result->NextRow());

        return true;
    }

    static bool HandlePossessCommand(ChatHandler* handler, char const* args)
    {
        Unit* pUnit = handler->GetSelectedUnit();
        if(!pUnit)
            return false;

        handler->GetSession()->GetPlayer()->CastSpell(pUnit, 530, true);
        return true;
    }

    static bool HandleUnPossessCommand(ChatHandler* handler, char const* args)
    {
        Unit* pUnit = handler->GetSelectedUnit();
        if(!pUnit) pUnit = handler->GetSession()->GetPlayer();

        pUnit->RemoveAurasByType(SPELL_AURA_MOD_CHARM);
        pUnit->RemoveAurasByType(SPELL_AURA_MOD_POSSESS_PET);
        pUnit->RemoveAurasByType(SPELL_AURA_MOD_POSSESS);

        return true;
    }

    static bool HandleBindSightCommand(ChatHandler* handler, char const* args)
    {
        Unit* pUnit = handler->GetSelectedUnit();
        if (!pUnit)
            return false;

        handler->GetSession()->GetPlayer()->CastSpell(pUnit, 6277, true);
        return true;
    }

    static bool HandleUnbindSightCommand(ChatHandler* handler, char const* args)
    {
        if (handler->GetSession()->GetPlayer()->IsPossessing())
            return false;

        handler->GetSession()->GetPlayer()->StopCastingBindSight();
        return true;
    }

    static bool HandleGetMoveFlagsCommand(ChatHandler* handler, char const* args)
    {
        Unit* target = handler->GetSelectedUnit();
        if (!target)
            target = handler->GetSession()->GetPlayer();

        std::stringstream stream;
        stream << std::hex << target->GetUnitMovementFlags();

        handler->PSendSysMessage("Target (%u) moveflags = 0x%s", 
            target->GetGUID().GetCounter(), 
            stream.str().c_str());

        return true;
    }

    static bool HandleSetMoveFlagsCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

        Unit* target = handler->GetSelectedUnit();
        if (!target)
            target = handler->GetSession()->GetPlayer();

        if(strcmp(args,"") == 0)
            return false;

        bool useHex = true;
        if (args[0] != '0' || args[1] != 'x')
            useHex = false;

        uint32 moveFlags;
        std::stringstream ss;
        if (useHex)
            ss << std::hex << std::string(args).substr(2);
        else
            ss << args;

        ss >> moveFlags;

        target->SetUnitMovementFlags(moveFlags);

        std::stringstream stream;
        stream << std::hex << target->GetUnitMovementFlags();
        handler->PSendSysMessage("Target (%u) moveflags set to 0x%s", target->GetGUID().GetCounter(), stream.str().c_str());

        return true;
    }

    static bool HandleGetMaxCreaturePoolIdCommand(ChatHandler* handler, char const* args)
    {
        QueryResult result = WorldDatabase.PQuery("SELECT MAX(pool_id) FROM creature");
        Field *fields = result->Fetch();
    
        uint32 maxId = fields[0].GetUInt32();
    
        handler->PSendSysMessage("Current max creature pool id: %u", maxId);
    
        return true;
    }

    static bool HandleSetTitleCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

        uint32 titleId = atoi(args);

        if(CharTitlesEntry const* titleEntry = sCharTitlesStore.LookupEntry(titleId)) {
            if (Player* plr = handler->GetSelectedUnit()->ToPlayer())
                plr->SetTitle(titleEntry,true);
            else if (Player* _plr = handler->GetSession()->GetPlayer())
                _plr->SetTitle(titleEntry,true);
        }

        return true;
    }

    static bool HandleRemoveTitleCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

        uint32 titleId = atoi(args);

        if(CharTitlesEntry const* titleEntry = sCharTitlesStore.LookupEntry(titleId)) {
            if (Player* plr = handler->GetSession()->GetPlayer())
                if (plr->HasTitle(titleEntry))
                    plr->RemoveTitle(titleEntry);
        }

        return true;
    }

    static bool HandleYoloCommand(ChatHandler* handler, char const* /*args*/)
    {
        handler->SendSysMessage(LANG_SWAG);

        return true;
    }

    static bool HandleSpellInfoCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK
    
        uint32 spellId = uint32(atoi(args));
        if (!spellId)
            return false;
        
        const SpellInfo* spell = sSpellMgr->GetSpellInfo(spellId);
        if (!spell)
            return false;
        
        handler->PSendSysMessage("## Spell %u (%s) ##", spell->Id, spell->SpellName[(uint32)sWorld->GetDefaultDbcLocale()]);
        handler->PSendSysMessage("Icon: %u - Visual: %u", spell->SpellIconID, spell->SpellVisual);
        handler->PSendSysMessage("Attributes: %x %x %x %x %x %x", spell->Attributes, spell->AttributesEx, spell->AttributesEx2, spell->AttributesEx3, spell->AttributesEx4, spell->AttributesEx5);
        handler->PSendSysMessage("Stack amount: %u", spell->StackAmount);
        handler->PSendSysMessage("SpellFamilyName: %u (%x)", spell->SpellFamilyName, spell->SpellFamilyName);
        handler->PSendSysMessage("SpellFamilyFlags: " UI64FMTD " (" UI64FMTD ")", spell->SpellFamilyFlags, spell->SpellFamilyFlags);
    
        return true;
    }

    static bool HandlePlayerbotConsoleCommand(ChatHandler* handler, char const* args)
    {
    #ifdef PLAYERBOT
        return RandomPlayerbotMgr::HandlePlayerbotConsoleCommand(handler, args);
    #else
        handler->SendSysMessage("Core not build with playerbot");
        return true;
    #endif
    }

    static bool HandlePlayerbotMgrCommand(ChatHandler* handler, char const* args)
    {
    #ifdef PLAYERBOT
        return PlayerbotMgr::HandlePlayerbotMgrCommand(handler, args);
    #else
        handler->SendSysMessage("Core not build with playerbot");
        return true;
    #endif
    }
};

void AddSC_misc_commandscript()
{
    new misc_commandscript();
}
