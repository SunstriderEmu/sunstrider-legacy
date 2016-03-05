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
#include "Language.h"
#include "Database/DatabaseEnv.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Opcodes.h"
#include "Log.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "UpdateMask.h"
#include "Chat.h"
#include "MapManager.h"
#include "GridNotifiersImpl.h"
#include "CellImpl.h"
#include "ChannelMgr.h"
#include "IRCMgr.h"
#include "AccountMgr.h"
#include "LogsDatabaseAccessor.h"

bool ChatHandler::load_command_table = true;

bool ChatHandler::SetDataForCommandInTable(std::vector<ChatCommand>& table, char const* text, uint32 securityLevel, std::string const& help, std::string const& fullcommand, bool allowIrc)
{
    std::string cmd = "";

    //get first word in 'cmd'
    while (*text != ' ' && *text != '\0')
    {
        cmd += *text;
        ++text;
    }

    //make text point on next word
    while (*text == ' ') ++text;

    for (uint32 i = 0; i < table.size(); i++)
    {
        if (!table[i].Name)
            return false;

        // for data fill use full explicit command names
        if (table[i].Name != cmd)
            continue;

        // select subcommand from child commands list (including "")
        if (!table[i].ChildCommands.empty())
        {
            if (SetDataForCommandInTable(table[i].ChildCommands, text, securityLevel, help, fullcommand, allowIrc))
                return true;
            else if (*text)
                return false;

            // fail with "" subcommands, then use normal level up command instead
        }
        // expected subcommand by full name DB content
        else if (*text)
        {
            TC_LOG_ERROR("sql.sql", "Table `command` have unexpected subcommand '%s' in command '%s', skip.", text, fullcommand.c_str());
            return false;
        }

        if (table[i].SecurityLevel != securityLevel)
            TC_LOG_DEBUG("misc", "Table `command` overwrite for command '%s' default permission (%u) by %u", fullcommand.c_str(), table[i].SecurityLevel, securityLevel);

        table[i].SecurityLevel = securityLevel;
        table[i].Help = help;
        //note that command with AllowIRC set to true still aren't allowed on irc if noSessionNeeded is set to false.
        table[i].AllowIRC = allowIrc;
        return true;
    }

    // in case "" command let process by caller
    if (!cmd.empty())
    {
        if (&table == &getCommandTable())
            TC_LOG_ERROR("sql.sql", "Table `command` have not existing command '%s', skip.", cmd.c_str());
        else
            TC_LOG_ERROR("sql.sql", "Table `command` have not existing subcommand '%s' in command '%s', skip.", cmd.c_str(), fullcommand.c_str());
    }
    return true;
}

std::vector<ChatCommand> const& ChatHandler::getCommandTable()
{
    static std::vector<ChatCommand> accountSetCommandTable =
    {
        { "addon",          SEC_GAMEMASTER3,          true,  false, &ChatHandler::HandleAccountSetAddonCommand,     "" },
        { "gmlevel",        SEC_ADMINISTRATOR,        true,  false, &ChatHandler::HandleAccountSetGmLevelCommand,   "" },
        { "password",       SEC_ADMINISTRATOR,        true,  false, &ChatHandler::HandleAccountSetPasswordCommand,  "" },
    };

    static std::vector<ChatCommand> accountCommandTable =
    {
        { "create",         SEC_ADMINISTRATOR,  true,  false, &ChatHandler::HandleAccountCreateCommand,       "" },
        { "delete",         SEC_ADMINISTRATOR,  true,  false, &ChatHandler::HandleAccountDeleteCommand,       "" },
        { "mailchange",     SEC_GAMEMASTER2,    true,  false, &ChatHandler::HandleAccountMailChangeCommand,   "" },
        { "set",            SEC_GAMEMASTER3,    true,  true,  NULL,                                           "", accountSetCommandTable },
        { "",               SEC_PLAYER,         false, false, &ChatHandler::HandleAccountCommand,             "" },
    };

    static std::vector<ChatCommand> serverSetCommandTable =
    {
        { "motd",           SEC_GAMEMASTER3,    true,  false, &ChatHandler::HandleServerSetMotdCommand,       "" },
        { "config",         SEC_SUPERADMIN,     true,  false, &ChatHandler::HandleServerSetConfigCommand,     "" },
    };

    static std::vector<ChatCommand> serverIdleRestartCommandTable =
    {
        { "cancel",         SEC_ADMINISTRATOR , true,  false, &ChatHandler::HandleServerShutDownCancelCommand,"" },
        { ""   ,            SEC_ADMINISTRATOR,  true,  false, &ChatHandler::HandleServerIdleRestartCommand,   "" },
    };

    static std::vector<ChatCommand> serverIdleShutdownCommandTable =
    {
        { "cancel",         SEC_ADMINISTRATOR,  true,  false, &ChatHandler::HandleServerShutDownCancelCommand,"" },
        { ""   ,            SEC_ADMINISTRATOR,  true,  false, &ChatHandler::HandleServerIdleShutDownCommand,  "" },
    };

    static std::vector<ChatCommand> serverRestartCommandTable =
    {
        { "cancel",         SEC_ADMINISTRATOR,  true,  false, &ChatHandler::HandleServerShutDownCancelCommand,"" },
        { ""   ,            SEC_ADMINISTRATOR,  true,  false, &ChatHandler::HandleServerRestartCommand,       "" },
    };

    static std::vector<ChatCommand> serverShutdownCommandTable =
    {
        { "cancel",         SEC_ADMINISTRATOR,  true,  false, &ChatHandler::HandleServerShutDownCancelCommand,"" },
        { ""   ,            SEC_ADMINISTRATOR,  true,  false, &ChatHandler::HandleServerShutDownCommand,      "" },
    };

    static std::vector<ChatCommand> serverCommandTable =
    {
        { "corpses",        SEC_GAMEMASTER2,     true,  false, &ChatHandler::HandleServerCorpsesCommand,       "" },
        { "exit",           SEC_ADMINISTRATOR,   true,  false, &ChatHandler::HandleServerExitCommand,          "" },
        { "idlerestart",    SEC_ADMINISTRATOR,   true,  true,  NULL,                                           "", serverIdleRestartCommandTable },
        { "idleshutdown",   SEC_ADMINISTRATOR,   true,  true,  NULL,                                           "", serverShutdownCommandTable },
        { "info",           SEC_PLAYER,          true,  true,  &ChatHandler::HandleServerInfoCommand,          "" },
        { "motd",           SEC_PLAYER,          true,  true,  &ChatHandler::HandleServerMotdCommand,          "" },
        { "restart",        SEC_ADMINISTRATOR,   true,  true,  NULL,                                           "", serverRestartCommandTable },
        { "shutdown",       SEC_ADMINISTRATOR,   true,  true,  NULL,                                           "", serverShutdownCommandTable },
        { "set",            SEC_ADMINISTRATOR,   true,  true,  NULL,                                           "", serverSetCommandTable },
    };

    static std::vector<ChatCommand> ircCommandtable =
    {
        { "reconnect",      SEC_ADMINISTRATOR,   true,  false, &ChatHandler::HandleIRCReconnectCommand,        "" },
        { "join",           SEC_ADMINISTRATOR,   true,  true,  &ChatHandler::HandleIRCJoinCommand,             "" },
        { "part",           SEC_ADMINISTRATOR,   true,  true,  &ChatHandler::HandleIRCPartCommand,             "" },
        { "quit",           SEC_ADMINISTRATOR,   true,  true,  &ChatHandler::HandleIRCQuitCommand,             "" },
    };
    
    static std::vector<ChatCommand> mmapCommandTable =
    {
        { "path",           SEC_GAMEMASTER3,     false, false, &ChatHandler::HandleMmapPathCommand,            "" },
        { "loc",            SEC_GAMEMASTER3,     false, false, &ChatHandler::HandleMmapLocCommand,             "" },
        { "loadedtiles",    SEC_GAMEMASTER3,     false, false, &ChatHandler::HandleMmapLoadedTilesCommand,     "" },
        { "stats",          SEC_GAMEMASTER3,     false, false, &ChatHandler::HandleMmapStatsCommand,           "" },
        { "testarea",       SEC_GAMEMASTER3,     false, false, &ChatHandler::HandleMmapTestArea,               "" },
        { "",               SEC_SUPERADMIN,      false, false, &ChatHandler::HandleMmap,                       "" },
    };

    static std::vector<ChatCommand> modifyCommandTable =
    {
        { "hp",             SEC_GAMEMASTER1,      false, false, &ChatHandler::HandleModifyHPCommand,            "" },
        { "mana",           SEC_GAMEMASTER1,      false, false, &ChatHandler::HandleModifyManaCommand,          "" },
        { "rage",           SEC_GAMEMASTER1,      false, false, &ChatHandler::HandleModifyRageCommand,          "" },
        { "energy",         SEC_GAMEMASTER1,      false, false, &ChatHandler::HandleModifyEnergyCommand,        "" },
        { "money",          SEC_GAMEMASTER1,      false, false, &ChatHandler::HandleModifyMoneyCommand,         "" },
        { "speed",          SEC_GAMEMASTER1,      false, false, &ChatHandler::HandleModifySpeedCommand,         "" },
        { "swim",           SEC_GAMEMASTER1,      false, false, &ChatHandler::HandleModifySwimCommand,          "" },
        { "scale",          SEC_GAMEMASTER1,      false, false, &ChatHandler::HandleModifyScaleCommand,         "" },
        { "bit",            SEC_GAMEMASTER1,      false, false, &ChatHandler::HandleModifyBitCommand,           "" },
        { "bwalk",          SEC_GAMEMASTER1,      false, false, &ChatHandler::HandleModifyBWalkCommand,         "" },
        { "fly",            SEC_GAMEMASTER1,      false, false, &ChatHandler::HandleModifyFlyCommand,           "" },
        { "aspeed",         SEC_GAMEMASTER1,      false, false, &ChatHandler::HandleModifyASpeedCommand,        "" },
        { "faction",        SEC_GAMEMASTER1,      false, false, &ChatHandler::HandleModifyFactionCommand,       "" },
        { "spell",          SEC_GAMEMASTER2,      false, false, &ChatHandler::HandleModifySpellCommand,         "" },
        { "tp",             SEC_GAMEMASTER1,      false, false, &ChatHandler::HandleModifyTalentCommand,        "" },
        { "titles",         SEC_GAMEMASTER2,      false, false, &ChatHandler::HandleModifyKnownTitlesCommand,   "" },
        { "mount",          SEC_GAMEMASTER1,      false, false, &ChatHandler::HandleModifyMountCommand,         "" },
        { "honor",          SEC_GAMEMASTER2,      false, false, &ChatHandler::HandleModifyHonorCommand,         "" },
        { "rep",            SEC_GAMEMASTER2,      false, false, &ChatHandler::HandleModifyRepCommand,           "" },
        { "arena",          SEC_GAMEMASTER2,      false, false, &ChatHandler::HandleModifyArenaCommand,         "" },
        { "drunk",          SEC_GAMEMASTER1,      false, false, &ChatHandler::HandleDrunkCommand,               "" },
        { "morph",          SEC_GAMEMASTER2,      false, false, &ChatHandler::HandleMorphCommand,               "" },
        { "gender",         SEC_GAMEMASTER3,      false, false, &ChatHandler::HandleModifyGenderCommand,        "" },
    };

    static std::vector<ChatCommand> wpCommandTable =
    {
        { "show",           SEC_GAMEMASTER3,     false, false, &ChatHandler::HandleWpShowCommand,               "" },
        { "addwp",          SEC_GAMEMASTER3,     false, false, &ChatHandler::HandleWpAddCommand,                "" },
        { "load",           SEC_GAMEMASTER3,     false, false, &ChatHandler::HandleWpLoadPathCommand,           "" },
        { "modify",         SEC_GAMEMASTER3,     false, false, &ChatHandler::HandleWpModifyCommand,             "" },
        { "event",          SEC_GAMEMASTER3,     false, false, &ChatHandler::HandleWpEventCommand,              "" },
        { "unload",         SEC_GAMEMASTER3,     false, false, &ChatHandler::HandleWpUnLoadPathCommand,         "" },
        { "direction",      SEC_GAMEMASTER3,     false, false, &ChatHandler::HandleWpChangePathDirectionCommand,"" },
        { "type"     ,      SEC_GAMEMASTER3,     false, false, &ChatHandler::HandleWpChangePathTypeCommand,     "" },
    };


    static std::vector<ChatCommand> banCommandTable =
    {
        { "account",        SEC_GAMEMASTER3,  true,  false, &ChatHandler::HandleBanAccountCommand,          "" },
        { "character",      SEC_GAMEMASTER3,  true,  false, &ChatHandler::HandleBanCharacterCommand,        "" },
        { "ip",             SEC_GAMEMASTER3,  true,  false, &ChatHandler::HandleBanIPCommand,               "" },
    };

    static std::vector<ChatCommand> baninfoCommandTable =
    {
        { "account",        SEC_GAMEMASTER3,  true,  false, &ChatHandler::HandleBanInfoAccountCommand,      "" },
        { "character",      SEC_GAMEMASTER3,  true,  false, &ChatHandler::HandleBanInfoCharacterCommand,    "" },
        { "ip",             SEC_GAMEMASTER3,  true,  false, &ChatHandler::HandleBanInfoIPCommand,           "" },
    };
    
    static std::vector<ChatCommand> muteinfoCommandTable =
    {
        { "account",        SEC_GAMEMASTER3,  true,  false, &ChatHandler::HandleMuteInfoAccountCommand,      "" },
        { "character",      SEC_GAMEMASTER3,  true,  false, &ChatHandler::HandleMuteInfoCharacterCommand,    "" },
    };

    static std::vector<ChatCommand> banlistCommandTable =
    {
        { "account",        SEC_GAMEMASTER3,  true,  false, &ChatHandler::HandleBanListAccountCommand,      "" },
        { "character",      SEC_GAMEMASTER3,  true,  false, &ChatHandler::HandleBanListCharacterCommand,    "" },
        { "ip",             SEC_GAMEMASTER3,  true,  false, &ChatHandler::HandleBanListIPCommand,           "" },
    };

    static std::vector<ChatCommand> unbanCommandTable =
    {
        { "account",        SEC_GAMEMASTER3,  true,  false, &ChatHandler::HandleUnBanAccountCommand,        "" },
        { "character",      SEC_GAMEMASTER3,  true,  false, &ChatHandler::HandleUnBanCharacterCommand,      "" },
        { "ip",             SEC_GAMEMASTER3,  true,  false, &ChatHandler::HandleUnBanIPCommand,             "" },
    };

    static std::vector<ChatCommand> debugCommandTable =
    {
        { "batchattack",    SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleDebugBatchAttack,           "" },
        { "inarc",          SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleDebugInArcCommand,          "" },
        { "spellfail",      SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleDebugSpellFailCommand,      "" },
        { "setpoi",         SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleSetPoiCommand,              "" },
        { "qpartymsg",      SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleSendQuestPartyMsgCommand,   "" },
        { "qinvalidmsg",    SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleSendQuestInvalidMsgCommand, "" },
        { "equiperr",       SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleEquipErrorCommand,          "" },
        { "sellerr",        SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleSellErrorCommand,           "" },
        { "buyerr",         SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleBuyErrorCommand,            "" },
        { "sendopcode",     SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleSendOpcodeCommand,          "" },
        { "uws",            SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleUpdateWorldStateCommand,    "" },
        { "ps",             SEC_GAMEMASTER3,  false, false, &ChatHandler::HandlePlaySound2Command,          "" },
        { "scn",            SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleSendChannelNotifyCommand,   "" },
        { "scm",            SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleSendChatMsgCommand,         "" },
        { "getitemstate",   SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleGetItemState,               "" },
        { "playsound",      SEC_GAMEMASTER1,  false, false, &ChatHandler::HandlePlaySoundCommand,           "" },
        { "update",         SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleUpdateCommand,              "" },
        { "setvalue",       SEC_ADMINISTRATOR,false, false, &ChatHandler::HandleSetValueCommand,            "" },
        { "getvalue",       SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleGetValueCommand,            "" },
        { "anim",           SEC_GAMEMASTER2,  false, false, &ChatHandler::HandleAnimCommand,                "" },
        { "lootrecipient",  SEC_GAMEMASTER2,  false, false, &ChatHandler::HandleGetLootRecipient,           "" },
        { "arena",          SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleDebugArenaCommand,          "" },
        { "bg",             SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleDebugBattleGroundCommand,   "" },
        { "threatlist",     SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleDebugThreatList,            "" },
        { "threatunitlist", SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleDebugHostilRefList,         "" },
        { "cin",            SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleDebugCinematic,             "" },
        { "getitembypos",   SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleDebugItemByPos,             "" },
        { "getitemlevelsum",SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleDebugItemLevelSum,          "" },
        { "removelootitem" ,SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleRemoveLootItem,             "" },
        { "pvpannounce",    SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleDebugPvPAnnounce,           "" },
        { "auralist",       SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleDebugAurasList,             "" },
        { "stealth",        SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleDebugStealthLevel,          "" },
        { "detect",         SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleDebugAttackDistance,        "" },
        { "unloadgrid",     SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleDebugUnloadGrid,            "" },
        { "loadedscripts",  SEC_GAMEMASTER3,  true,  false, &ChatHandler::HandleDebugLoadedScripts,         "" },
        { "resetdaily",     SEC_GAMEMASTER3,  true,  false, &ChatHandler::HandleDebugResetDailyQuests,      "" },
        { "attackers",      SEC_GAMEMASTER2,  false, false, &ChatHandler::HandleDebugShowAttackers,         "" },
        { "zoneattack",     SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleDebugSendZoneUnderAttack,   "" },
        { "los",            SEC_GAMEMASTER1,  false, false, &ChatHandler::HandleDebugLoSCommand,            "" },
        { "playerflags",    SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleDebugPlayerFlags,           "" },
        { "opcodetest",     SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleDebugOpcodeTestCommand,     "" },
        { "playemote",      SEC_GAMEMASTER2,  false, false, &ChatHandler::HandleDebugPlayEmoteCommand,      "" },
        { "mapheight",      SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleDebugMapHeight,             "" },
    };

    static std::vector<ChatCommand> eventCommandTable =
    {
        { "activelist",     SEC_GAMEMASTER2,     true,  false, &ChatHandler::HandleEventActiveListCommand,     "" },
        { "start",          SEC_GAMEMASTER2,     true,  false, &ChatHandler::HandleEventStartCommand,          "" },
        { "stop",           SEC_GAMEMASTER2,     true,  false, &ChatHandler::HandleEventStopCommand,           "" },
        { "create",         SEC_GAMEMASTER2,     true,  false, &ChatHandler::HandleEventCreateCommand,         "" },
        { "info",           SEC_GAMEMASTER2,     true,  false, &ChatHandler::HandleEventInfoCommand,           "" },
    };

    static std::vector<ChatCommand> learnCommandTable =
    {
        { "all_gm",         SEC_GAMEMASTER2,  false, false, &ChatHandler::HandleLearnAllGMCommand,          "" },
        { "all_crafts",     SEC_GAMEMASTER2,  false, false, &ChatHandler::HandleLearnAllCraftsCommand,      "" },
        { "all_default",    SEC_GAMEMASTER1,  false, false, &ChatHandler::HandleLearnAllDefaultCommand,     "" },
        { "all_lang",       SEC_GAMEMASTER1,  false, false, &ChatHandler::HandleLearnAllLangCommand,        "" },
        { "all_myclass",    SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleLearnAllMyClassCommand,     "" },
        { "all_myspells",   SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleLearnAllMySpellsCommand,    "" },
        { "all_mytalents",  SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleLearnAllMyTalentsCommand,   "" },
        { "all_recipes",    SEC_GAMEMASTER2,  false, false, &ChatHandler::HandleLearnAllRecipesCommand,     "" },
        { "",               SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleLearnCommand,               "" },
    };

    static std::vector<ChatCommand> reloadCommandTable =
    {
        { "access_requirement",          SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadAccessRequirementCommand,       "" },
        { "all",                         SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadAllCommand,                     "" },
        { "all_item",                    SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadAllItemCommand,                 "" },
        { "all_locales",                 SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadAllLocalesCommand,              "" },
        { "all_loot",                    SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadAllLootCommand,                 "" },
        { "all_npc",                     SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadAllNpcCommand,                  "" },
        { "all_quest",                   SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadAllQuestCommand,                "" },
        { "all_scripts",                 SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadAllScriptsCommand,              "" },
        { "all_spell",                   SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadAllSpellCommand,                "" },
        { "areatrigger_involvedrelation",SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadQuestAreaTriggersCommand,       "" },
        { "areatrigger_tavern",          SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadAreaTriggerTavernCommand,       "" },
        { "areatrigger_teleport",        SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadAreaTriggerTeleportCommand,     "" },
        { "auctions",                    SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadAuctionsCommand,                "" },
        { "auto_ann_by_time",            SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadAutoAnnounceCommand,            "" },
        { "command",                     SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadCommandCommand,                 "" },
        { "conditions",                  SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadConditions,                     "" },
        { "config",                      SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadConfigCommand,                  "" },
        { "creature_gossip",             SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadCreatureGossipCommand,          "" },  
        { "creature_questender",         SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadCreatureQuestEndersCommand,     "" },
        { "creature_linked_respawn",     SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadCreatureLinkedRespawnCommand,   "" },
        { "creature_loot_template",      SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadLootTemplatesCreatureCommand,   "" },
        { "creature_model_info",         SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadCreatureModelInfoCommand,       "" },
        { "creature_text",               SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadCreatureTextCommand,            "" },
        { "creature_template",           SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadCreatureTemplateCommand,        "" },
        { "disenchant_loot_template",    SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadLootTemplatesDisenchantCommand, "" },
        { "event_scripts",               SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadEventScriptsCommand,            "" },
        { "fishing_loot_template",       SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadLootTemplatesFishingCommand,    "" },
        { "game_event",                  SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadGameEventCommand,               "" },
        { "game_graveyard_zone",         SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadGameGraveyardZoneCommand,       "" },
        { "game_tele",                   SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadGameTeleCommand,                "" },
        { "gameobject_questender",       SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadGameobjectQuestEndersCommand,   "" },
        { "gameobject_loot_template",    SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadLootTemplatesGameobjectCommand, "" },
        { "gameobject_queststarter",     SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadGameobjectQuestStartersCommand, "" },
        { "gameobject_scripts",          SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadGameObjectScriptsCommand,       "" },
        { "gm_tickets",                  SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleGMTicketReloadCommand,                "" },
        { "item_enchantment_template",   SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadItemEnchantementsCommand,       "" },
        { "item_extended_cost",          SEC_ADMINISTRATOR, true,  true,  &ChatHandler::HandleReloadItemExtendedCostCommand,        "" },
        { "item_loot_template",          SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadLootTemplatesItemCommand,       "" },
        { "locales_creature",            SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadLocalesCreatureCommand,         "" },
        { "locales_gameobject",          SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadLocalesGameobjectCommand,       "" },
        { "locales_item",                SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadLocalesItemCommand,             "" },
        { "locales_gossip_text",         SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadLocalesGossipTextCommand,       "" },
        { "locales_page_text",           SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadLocalesPageTextCommand,         "" },
        { "locales_quest",               SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadLocalesQuestCommand,            "" },
        { "gossip_menu",                 SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadGossipMenuCommand,              "" },  
        { "gossip_menu_option",          SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadGossipMenuOptionCommand,        "" },
        { "gossip_text",                 SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadGossipTextCommand,              "" },  
        { "npc_trainer",                 SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadNpcTrainerCommand,              "" },
        { "npc_vendor",                  SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadNpcVendorCommand,               "" },
        { "page_text",                   SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadPageTextsCommand,               "" },
        { "pickpocketing_loot_template", SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadLootTemplatesPickpocketingCommand,""},
        { "prospecting_loot_template",   SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadLootTemplatesProspectingCommand,"" },
        { "quest_end_scripts",           SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadQuestEndScriptsCommand,         "" },
        { "quest_mail_loot_template",    SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadLootTemplatesQuestMailCommand,  "" },
        { "quest_start_scripts",         SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadQuestStartScriptsCommand,       "" },
        { "quest_template",              SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadQuestTemplateCommand,           "" },
        { "reference_loot_template",     SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadLootTemplatesReferenceCommand,  "" },
        { "reserved_name",               SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadReservedNameCommand,            "" },
        { "skill_discovery_template",    SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadSkillDiscoveryTemplateCommand,  "" },
        { "skill_extra_item_template",   SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadSkillExtraItemTemplateCommand,  "" },
        { "skill_fishing_base_level",    SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadSkillFishingBaseLevelCommand,   "" },
        { "skinning_loot_template",      SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadLootTemplatesSkinningCommand,   "" },
        { "smart_scripts",               SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadSmartAICommand,                 "" },
        { "spell_affect",                SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadSpellAffectCommand,             "" },
        { "spell_bonus_data",            SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadSpellBonusesCommand,             "" },
        { "spell_disabled",              SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadSpellDisabledCommand,           "" },
        { "spell_elixir",                SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadSpellElixirCommand,             "" },
        { "spell_learn_spell",           SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadSpellLearnSpellCommand,         "" },
        { "spell_linked_spell",          SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadSpellLinkedSpellCommand,        "" },
        { "spell_pet_auras",             SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadSpellPetAurasCommand,           "" },
        { "spell_proc_event",            SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadSpellProcEventCommand,          "" },
        { "spell_required",              SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadSpellRequiredCommand,           "" },
        { "spell_script_target",         SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadSpellScriptTargetCommand,       "" },
        { "spell_scripts",               SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadSpellScriptsCommand,            "" },
        { "spell_target_position",       SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadSpellTargetPositionCommand,     "" },
        { "spell_template",              SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadSpellTemplates,                 "" },
        { "spell_threats",               SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadSpellThreatsCommand,            "" },
        { "trinity_string",              SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadTrinityStringCommand,           "" },
        { "waypoint_scripts",            SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadWpScriptsCommand,               "" },
        { "waypoints",                   SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadSmartWaypointsCommand,          "" },
        { "",                            SEC_ADMINISTRATOR, true,  false, &ChatHandler::HandleReloadCommand,                        "" },
    };

    static std::vector<ChatCommand> honorCommandTable =
    {
        { "add",            SEC_GAMEMASTER2,     false, false, &ChatHandler::HandleAddHonorCommand,            "" },
        { "addkill",        SEC_GAMEMASTER2,     false, false, &ChatHandler::HandleHonorAddKillCommand,        "" },
        { "update",         SEC_GAMEMASTER2,     false, false, &ChatHandler::HandleUpdateHonorFieldsCommand,   "" },
    };

    static std::vector<ChatCommand> guildCommandTable =
    {
        { "create",         SEC_GAMEMASTER2,     true,  false, &ChatHandler::HandleGuildCreateCommand,         "" },
        { "delete",         SEC_GAMEMASTER2,     true,  false, &ChatHandler::HandleGuildDeleteCommand,         "" },
        { "invite",         SEC_GAMEMASTER2,     true,  false, &ChatHandler::HandleGuildInviteCommand,         "" },
        { "uninvite",       SEC_GAMEMASTER2,     true,  false, &ChatHandler::HandleGuildUninviteCommand,       "" },
        { "rank",           SEC_GAMEMASTER2,     true,  false, &ChatHandler::HandleGuildRankCommand,           "" },
        { "rename",         SEC_GAMEMASTER2,     true,  false, &ChatHandler::HandleGuildRenameCommand,         "" },
    };

    static std::vector<ChatCommand> petCommandTable =
    {
        { "create",         SEC_GAMEMASTER2,     false, false, &ChatHandler::HandleCreatePetCommand,           "" },
        { "learn",          SEC_GAMEMASTER2,     false, false, &ChatHandler::HandlePetLearnCommand,            "" },
        { "unlearn",        SEC_GAMEMASTER2,     false, false, &ChatHandler::HandlePetUnlearnCommand,          "" },
        { "tp",             SEC_GAMEMASTER2,     false, false, &ChatHandler::HandlePetTpCommand,               "" },
        { "rename",         SEC_GAMEMASTER2,     false, false, &ChatHandler::HandlePetRenameCommand,           "" },
    };


    static std::vector<ChatCommand> groupCommandTable =
    {
        { "leader",         SEC_GAMEMASTER3,     false,  false, &ChatHandler::HandleGroupLeaderCommand,         "" },
        { "disband",        SEC_GAMEMASTER3,     false,  false, &ChatHandler::HandleGroupDisbandCommand,        "" },
        { "remove",         SEC_GAMEMASTER3,     false,  false, &ChatHandler::HandleGroupRemoveCommand,         "" },
    };

    static std::vector<ChatCommand> lookupPlayerCommandTable =
    {
        { "ip",             SEC_GAMEMASTER2,     true,  false, &ChatHandler::HandleLookupPlayerIpCommand,       "" },
        { "account",        SEC_GAMEMASTER2,     true,  false, &ChatHandler::HandleLookupPlayerAccountCommand,  "" },
        { "email",          SEC_GAMEMASTER2,     true,  false, &ChatHandler::HandleLookupPlayerEmailCommand,    "" },
    };

    static std::vector<ChatCommand> lookupCommandTable =
    {
        { "area",           SEC_GAMEMASTER1,  true,  false, &ChatHandler::HandleLookupAreaCommand,          "" },
        { "creature",       SEC_GAMEMASTER3,  true,  false, &ChatHandler::HandleLookupCreatureCommand,      "" },
        { "event",          SEC_GAMEMASTER2,  true,  false, &ChatHandler::HandleLookupEventCommand,         "" },
        { "faction",        SEC_GAMEMASTER3,  true,  false, &ChatHandler::HandleLookupFactionCommand,       "" },
        { "item",           SEC_GAMEMASTER3,  true,  false, &ChatHandler::HandleLookupItemCommand,          "" },
        { "itemset",        SEC_GAMEMASTER3,  true,  false, &ChatHandler::HandleLookupItemSetCommand,       "" },
        { "object",         SEC_GAMEMASTER3,  true,  false, &ChatHandler::HandleLookupObjectCommand,        "" },
        { "quest",          SEC_GAMEMASTER3,  true,  false, &ChatHandler::HandleLookupQuestCommand,         "" },
        { "player",         SEC_GAMEMASTER2,  true,  false, NULL,                                           "", lookupPlayerCommandTable },
        { "skill",          SEC_GAMEMASTER3,  true,  false, &ChatHandler::HandleLookupSkillCommand,         "" },
        { "spell",          SEC_GAMEMASTER3,  true,  false, &ChatHandler::HandleGetSpellInfoCommand,        "" },
        { "tele",           SEC_GAMEMASTER1,  true,  false, &ChatHandler::HandleLookupTeleCommand,          "" },
    };

    static std::vector<ChatCommand> resetCommandTable =
    {
        { "honor",          SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleResetHonorCommand,          "" },
        { "level",          SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleResetLevelCommand,          "" },
        { "spells",         SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleResetSpellsCommand,         "" },
        { "stats",          SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleResetStatsCommand,          "" },
        { "talents",        SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleResetTalentsCommand,        "" },
        { "all",            SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleResetAllCommand,            "" },
    };

    static std::vector<ChatCommand> castCommandTable =
    {
        { "back",           SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleCastBackCommand,            "" },
        { "batch",          SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleCastBatchCommand,           "" },
        { "dist",           SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleCastDistCommand,            "" },
        { "self",           SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleCastSelfCommand,            "" },
        { "target",         SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleCastTargetCommand,          "" },
        { "",               SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleCastCommand,                "" },
    };

    static std::vector<ChatCommand> pdumpCommandTable =
    {
        { "load",           SEC_GAMEMASTER3,  true,  false, &ChatHandler::HandleLoadPDumpCommand,           "" },
        { "write",          SEC_GAMEMASTER3,  true,  false, &ChatHandler::HandleWritePDumpCommand,          "" },
    };

    static std::vector<ChatCommand> listCommandTable =
    {
        { "creature",       SEC_GAMEMASTER3,  true,  false, &ChatHandler::HandleListCreatureCommand,        "" },
        { "item",           SEC_GAMEMASTER3,  true,  false, &ChatHandler::HandleListItemCommand,            "" },
        { "object",         SEC_GAMEMASTER3,  true,  false, &ChatHandler::HandleListObjectCommand,          "" },
        { "auras",          SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleListAurasCommand,           "" },
    };

    static std::vector<ChatCommand> teleCommandTable =
    {
        { "add",            SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleAddTeleCommand,             "" },
        { "del",            SEC_GAMEMASTER3,  true,  false, &ChatHandler::HandleDelTeleCommand,             "" },
        { "name",           SEC_GAMEMASTER1,  true,  false, &ChatHandler::HandleNameTeleCommand,            "" },
        { "group",          SEC_GAMEMASTER1,  false, false, &ChatHandler::HandleGroupTeleCommand,           "" },
        { "",               SEC_GAMEMASTER1,  false, false, &ChatHandler::HandleTeleCommand,                "" },
    };

    static std::vector<ChatCommand> npcPathTable =
    {
        { "type",           SEC_GAMEMASTER2,  false, false, &ChatHandler::HandleNpcPathTypeCommand,           "" },
        { "direction",      SEC_GAMEMASTER2,  false, false, &ChatHandler::HandleNpcPathDirectionCommand,      "" },
        { "currentid",      SEC_GAMEMASTER1,  false, false, &ChatHandler::HandleNpcPathCurrentIdCommand,      "" },
    };

    static std::vector<ChatCommand> npcCommandTable =
    {
        { "say",            SEC_GAMEMASTER1,     false, false, &ChatHandler::HandleNpcSayCommand,              "" },
        { "textemote",      SEC_GAMEMASTER1,     false, false, &ChatHandler::HandleNpcTextEmoteCommand,        "" },
        { "add",            SEC_GAMEMASTER2,     false, false, &ChatHandler::HandleNpcAddCommand,              "" },
        { "delete",         SEC_GAMEMASTER2,     false, false, &ChatHandler::HandleNpcDeleteCommand,           "" },
        { "spawndist",      SEC_GAMEMASTER2,     false, false, &ChatHandler::HandleNpcSpawnDistCommand,        "" },
        { "spawntime",      SEC_GAMEMASTER2,     false, false, &ChatHandler::HandleNpcSpawnTimeCommand,        "" },
        { "factionid",      SEC_GAMEMASTER2,     false, false, &ChatHandler::HandleNpcFactionIdCommand,        "" },
        { "setmovetype",    SEC_GAMEMASTER2,     false, false, &ChatHandler::HandleNpcSetMoveTypeCommand,      "" },
        { "move",           SEC_GAMEMASTER2,     false, false, &ChatHandler::HandleNpcMoveCommand,             "" },
        { "changelevel",    SEC_GAMEMASTER2,     false, false, &ChatHandler::HandleChangeLevelCommand,         "" },
        { "setmodel",       SEC_GAMEMASTER2,     false, false, &ChatHandler::HandleNpcSetModelCommand,         "" },
        { "additem",        SEC_GAMEMASTER2,     false, false, &ChatHandler::HandleAddVendorItemCommand,       "" },
        { "delitem",        SEC_GAMEMASTER2,     false, false, &ChatHandler::HandleDelVendorItemCommand,       "" },
        { "flag",           SEC_GAMEMASTER2,     false, false, &ChatHandler::HandleNpcFlagCommand,             "" },
        { "changeentry",    SEC_GAMEMASTER3,     false, false, &ChatHandler::HandleNpcChangeEntryCommand,      "" },
        { "info",           SEC_GAMEMASTER3,     false, false, &ChatHandler::HandleNpcInfoCommand,             "" },
        { "setemotestate",  SEC_GAMEMASTER3,     false, false, &ChatHandler::HandleNpcSetEmoteStateCommand,    "" },
        { "follow",         SEC_GAMEMASTER2,     false, false, &ChatHandler::HandleNpcFollowCommand,           "" },
        { "unfollow",       SEC_GAMEMASTER2,     false, false, &ChatHandler::HandleNpcUnFollowCommand,         "" },
        { "whisper",        SEC_GAMEMASTER1,     false, false, &ChatHandler::HandleNpcWhisperCommand,          "" },
        { "yell",           SEC_GAMEMASTER1,     false, false, &ChatHandler::HandleNpcYellCommand,             "" },
        { "addtemp",        SEC_GAMEMASTER2,     false, false, &ChatHandler::HandleTempAddSpwCommand,          "" },
        { "addformation",   SEC_ADMINISTRATOR,   false, false, &ChatHandler::HandleNpcAddFormationCommand,     "" },
        { "removeformation",SEC_ADMINISTRATOR,   false, false, &ChatHandler::HandleNpcRemoveFormationCommand,  "" },
        { "setlink",        SEC_ADMINISTRATOR,   false, false, &ChatHandler::HandleNpcSetLinkCommand,          "" },
        { "gobackhome",     SEC_GAMEMASTER3,     false, false, &ChatHandler::HandleNpcGoBackHomeCommand,       "" },
        { "setpool",        SEC_GAMEMASTER3,     false, false, &ChatHandler::HandleNpcSetPoolCommand,          "" },
        { "guid",           SEC_GAMEMASTER1,     false, false, &ChatHandler::HandleNpcGuidCommand,             "" },
        { "addweapon",      SEC_GAMEMASTER3,     false, false, &ChatHandler::HandleAddWeaponCommand,           "" },
        { "massfactionid",  SEC_ADMINISTRATOR,   false, false, &ChatHandler::HandleNpcMassFactionIdCommand,    "" },
        { "combatdistance", SEC_ADMINISTRATOR,   false, false, &ChatHandler::HandleNpcSetCombatDistanceCommand,"" },
        { "combatmovallow", SEC_ADMINISTRATOR,   false, false, &ChatHandler::HandleNpcAllowCombatMovementCommand,""},
        { "linkgameevent",  SEC_ADMINISTRATOR,   false, false, &ChatHandler::HandleNpcLinkGameEventCommand,    "" },
        { "unlinkgameevent",SEC_ADMINISTRATOR,   false, false, &ChatHandler::HandleNpcUnlinkGameEventCommand,  "" },
        { "goto",           SEC_GAMEMASTER3,     false, false, &ChatHandler::HandleNpcGotoCommand,             "" },
        { "fly",            SEC_GAMEMASTER3,     false, false, &ChatHandler::HandleNpcFlyCommand,              "" },
        { "near",           SEC_GAMEMASTER3,     false, false, &ChatHandler::HandleNpcNearCommand,             "" },
        { "name",           SEC_GAMEMASTER2,     false, false, &ChatHandler::HandleNameCommand,                "" },

        //{ TODO: fix or remove this commands
        { "seteventid",     SEC_GAMEMASTER3,     false, false, &ChatHandler::HandleNpcSetInstanceEventCommand, "" },
        //}

        { "path",           SEC_GAMEMASTER1,     false, false, NULL,                                           "", npcPathTable },
    };

    static std::vector<ChatCommand> goCommandTable =
    {
        { "grid",           SEC_GAMEMASTER1,     false, false, &ChatHandler::HandleGoGridCommand,              "" },
        { "creature",       SEC_GAMEMASTER2,     false, false, &ChatHandler::HandleGoCreatureCommand,          "" },
        { "object",         SEC_GAMEMASTER2,     false, false, &ChatHandler::HandleGoObjectCommand,            "" },
        { "ticket",         SEC_GAMEMASTER1,     false, false, &ChatHandler::HandleGoTicketCommand,            "" },
        { "trigger",        SEC_GAMEMASTER2,     false, false, &ChatHandler::HandleGoTriggerCommand,           "" },
        { "graveyard",      SEC_GAMEMASTER2,     false, false, &ChatHandler::HandleGoGraveyardCommand,         "" },
        { "zonexy",         SEC_GAMEMASTER1,     false, false, &ChatHandler::HandleGoZoneXYCommand,            "" },
        { "xy",             SEC_GAMEMASTER1,     false, false, &ChatHandler::HandleGoXYCommand,                "" },
        { "xyz",            SEC_GAMEMASTER1,     false, false, &ChatHandler::HandleGoXYZCommand,               "" },
        { "xyzo",           SEC_GAMEMASTER1,     false, false, &ChatHandler::HandleGoXYZOCommand,              "" },
        { "at",             SEC_GAMEMASTER3,     false, false, &ChatHandler::HandleGoATCommand,                "" },
        { "",               SEC_GAMEMASTER1,     false, false, &ChatHandler::HandleGoXYZCommand,               "" },
    };

    static std::vector<ChatCommand> gobjectCommandTable =
    {
        { "add",            SEC_GAMEMASTER3,     false, false, &ChatHandler::HandleGameObjectCommand,          "" },
        { "delete",         SEC_GAMEMASTER3,     false, false, &ChatHandler::HandleDelObjectCommand,           "" },
        { "target",         SEC_GAMEMASTER2,     false, false, &ChatHandler::HandleTargetObjectCommand,        "" },
        { "turn",           SEC_GAMEMASTER3,     false, false, &ChatHandler::HandleTurnObjectCommand,          "" },
        { "move",           SEC_GAMEMASTER3,     false, false, &ChatHandler::HandleMoveObjectCommand,          "" },
        { "near",           SEC_GAMEMASTER3,     false, false, &ChatHandler::HandleNearObjectCommand,          "" },
        { "activate",       SEC_GAMEMASTER2,     false, false, &ChatHandler::HandleActivateObjectCommand,      "" },
        { "addtemp",        SEC_GAMEMASTER3,     false, false, &ChatHandler::HandleTempGameObjectCommand,      "" },
        { "linkgameevent",  SEC_ADMINISTRATOR,   false, false, &ChatHandler::HandleGobLinkGameEventCommand,    "" },
        { "unlinkgameevent",SEC_ADMINISTRATOR,   false, false, &ChatHandler::HandleGobUnlinkGameEventCommand,  "" },
        { "getvalue",       SEC_GAMEMASTER3,     false, false, &ChatHandler::HandleGobGetValueCommand,         "" },
        { "setvalue",       SEC_GAMEMASTER3,     false, false, &ChatHandler::HandleGobSetValueCommand,         "" },
    };

    static std::vector<ChatCommand> questCommandTable =
    {
        { "add",            SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleAddQuest,                   "" },
        { "complete",       SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleCompleteQuest,              "" },
        { "countcomplete",  SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleCountCompleteQuest,         "" },
        { "totalcount",     SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleTotalCount,                 "" },
        { "remove",         SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleRemoveQuest,                "" },
        { "report",         SEC_GAMEMASTER2,  false, false, &ChatHandler::HandleReportQuest,                "" },
    };

    static std::vector<ChatCommand> gmCommandTable =
    {
        { "chat",           SEC_GAMEMASTER1,    false, false, &ChatHandler::HandleGMChatCommand,              "" },
        { "ingame",         SEC_PLAYER,         true,  false, &ChatHandler::HandleGMListIngameCommand,        "" },
        { "list",           SEC_GAMEMASTER3,    true,  false, &ChatHandler::HandleGMListFullCommand,          "" },
        { "visible",        SEC_GAMEMASTER1,    false, false, &ChatHandler::HandleVisibleCommand,             "" },
        { "fly",            SEC_GAMEMASTER3,    false, false, &ChatHandler::HandleFlyModeCommand,             "" },
        { "stats",          SEC_GAMEMASTER1,    false, false, &ChatHandler::HandleGMStats,                    "" },
        { "",               SEC_GAMEMASTER1,    false, false, &ChatHandler::HandleGMmodeCommand,              "" },
        { NULL,             0,                  false, false, NULL,                                           "" }
    };

    static std::vector<ChatCommand> instanceCommandTable =
    {
        { "listbinds",      SEC_GAMEMASTER1,      false, false, &ChatHandler::HandleInstanceListBindsCommand,   "" },
        { "unbind",         SEC_GAMEMASTER1,      false, false, &ChatHandler::HandleInstanceUnbindCommand,      "" },
        { "stats",          SEC_GAMEMASTER1,      true,  false, &ChatHandler::HandleInstanceStatsCommand,       "" },
        { "savedata",       SEC_GAMEMASTER1,      false, false, &ChatHandler::HandleInstanceSaveDataCommand,    "" },
        { "setdata",        SEC_GAMEMASTER1,      false, false, &ChatHandler::HandleInstanceSetDataCommand,     "" },
        { "getdata",        SEC_GAMEMASTER1,      false, false, &ChatHandler::HandleInstanceGetDataCommand,     "" },
    };

    static std::vector<ChatCommand> ticketCommandTable =
    {
        { "list",           SEC_GAMEMASTER1,      false, false, &ChatHandler::HandleGMTicketListCommand,             "" },
        { "onlinelist",     SEC_GAMEMASTER1,      false, false, &ChatHandler::HandleGMTicketListOnlineCommand,       "" },
        { "viewname",       SEC_GAMEMASTER1,      false, false, &ChatHandler::HandleGMTicketGetByNameCommand,        "" },
        { "viewid",         SEC_GAMEMASTER1,      false, false, &ChatHandler::HandleGMTicketGetByIdCommand,          "" },
        { "close",          SEC_GAMEMASTER1,      false, false, &ChatHandler::HandleGMTicketCloseByIdCommand,        "" },
        { "closedlist",     SEC_GAMEMASTER1,      false, false, &ChatHandler::HandleGMTicketListClosedCommand,       "" },
        { "delete",         SEC_GAMEMASTER3,      false, false, &ChatHandler::HandleGMTicketDeleteByIdCommand,       "" },
        { "assign",         SEC_GAMEMASTER1,      false, false, &ChatHandler::HandleGMTicketAssignToCommand,         "" },
        { "unassign",       SEC_GAMEMASTER1,      false, false, &ChatHandler::HandleGMTicketUnAssignCommand,         "" },
        { "comment",        SEC_GAMEMASTER1,      false, false, &ChatHandler::HandleGMTicketCommentCommand,          "" },
    };

    static std::vector<ChatCommand> zoneCommandTable =
    {
        { "buff",           SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleZoneBuffCommand,                 "" },
        { "morph",          SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleZoneMorphCommand,                "" },
    };

    static std::vector<ChatCommand> npcEventCommandTable =
    {
        { "enable",        SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleEnableEventCommand,              "" },
        { "disable",       SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleDisableEventCommand,             "" },
        { "schedule",      SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleScheduleEventCommand,            "" },
    
    };

    static std::vector<ChatCommand> spectateCommandTable =
    {
        { "version",       SEC_PLAYER,      false, false, &ChatHandler::HandleSpectateVersion,                 "" },
        { "watch",         SEC_PLAYER,      false, false, &ChatHandler::HandleSpectateFromCommand,             "" },
        { "leave",         SEC_PLAYER,      false, false, &ChatHandler::HandleSpectateCancelCommand,           "" },
        { "init",          SEC_PLAYER,      false, false, &ChatHandler::HandleSpectateInitCommand,             "" },
    };

    static std::vector<ChatCommand> smartAICommandTable =
    {
        { "debug",         SEC_GAMEMASTER2, true,  true,  &ChatHandler::HandleSmartAIDebugCommand,             "" },
        { "errors",        SEC_GAMEMASTER2, true,  true,  &ChatHandler::HandleSmartAIShowErrorsCommand,        "" },
    };

   static std::vector<ChatCommand> cheatCommandTable =
    {
        { "god",            SEC_GAMEMASTER2,  false, false, &ChatHandler::HandleGodModeCheatCommand,            "" },
        { "casttime",       SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleCasttimeCheatCommand,           "" },
        { "cooldown",       SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleCoolDownCheatCommand,           "" },
        { "power",          SEC_GAMEMASTER3,  false, false, &ChatHandler::HandlePowerCheatCommand,              "" },
        { "taxi",           SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleTaxiCheatCommand,               "" },
        { "explore",        SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleExploreCheatCommand,            "" },
        { "waterwalk",      SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleWaterwalkCheatCommand,          "" },
    };

   /**
    * The values here may be overwritten by the database and are here as defaults.
    */
    static std::vector<ChatCommand> commandTable =
    {
        { "account",        SEC_PLAYER,       true,  false, NULL,                                           "", accountCommandTable },
        { "additem",        SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleAddItemCommand,             "" },
        { "additemset",     SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleAddItemSetCommand,          "" },
        { "announce",       SEC_GAMEMASTER1,  true,  false, &ChatHandler::HandleAnnounceCommand,            "" },
        { "arenarename",    SEC_GAMEMASTER2,  true,  false, &ChatHandler::HandleRenameArenaTeamCommand,     "" },
        { "aura",           SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleAuraCommand,                "" },
        { "ban",            SEC_GAMEMASTER3,  true,  false, NULL,                                           "", banCommandTable },
        { "baninfo",        SEC_GAMEMASTER3,  false, false, NULL,                                           "", baninfoCommandTable },
        { "bank",           SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleBankCommand,                "" },
        { "banlist",        SEC_GAMEMASTER3,  true,  false, NULL,                                           "", banlistCommandTable },
        { "bg",             SEC_PLAYER,       false, false, &ChatHandler::HandleBattlegroundCommand,        "" },
        { "bindsight",      SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleBindSightCommand,           "" },
        { "blink",          SEC_GAMEMASTER1,  false, false, &ChatHandler::HandleBlinkCommand,               "" },
        { "boutique",       SEC_PLAYER,       false, false, &ChatHandler::HandleBuyInShopCommand,           "" },
        { "cast",           SEC_GAMEMASTER3,  false, false, NULL,                                           "", castCommandTable },
        { "chanban",        SEC_GAMEMASTER1,  true,  false, &ChatHandler::HandleChanBan,                    "" },
        { "chaninfoban",    SEC_GAMEMASTER1,  true,  false, &ChatHandler::HandleChanInfoBan,                "" },
        { "chanunban",      SEC_GAMEMASTER1,  true,  false, &ChatHandler::HandleChanUnban,                  "" },
        { "chardelete",     SEC_ADMINISTRATOR,true,  true,  &ChatHandler::HandleCharacterDeleteCommand,     "" },
        { "cheat",          SEC_GAMEMASTER2,  false, false,  NULL,                                          "", cheatCommandTable },
        { "combatstop",     SEC_GAMEMASTER2,  false, false, &ChatHandler::HandleCombatStopCommand,           "" },
        { "cometome",       SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleComeToMeCommand,            "" },
        { "commands",       SEC_PLAYER,       true,  false, &ChatHandler::HandleCommandsCommand,            "" },
        { "cooldown",       SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleCooldownCommand,            "" },
        { "copystuff",      SEC_GAMEMASTER2,  false, false, &ChatHandler::HandleCopyStuffCommand,           "" },
        { "credits",        SEC_PLAYER,       false, false, &ChatHandler::HandleViewCreditsCommand,         "" },
        { "damage",         SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleDamageCommand,              "" },
        { "debug",          SEC_GAMEMASTER1,  false, true,  NULL,                                           "", debugCommandTable },
        { "demorph",        SEC_GAMEMASTER2,  false, false, &ChatHandler::HandleDeMorphCommand,             "" },
        { "die",            SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleDieCommand,                 "" },
        { "dismount",       SEC_PLAYER,       false, false, &ChatHandler::HandleDismountCommand,            "" },
        { "distance",       SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleGetDistanceCommand,         "" },
        { "event",          SEC_GAMEMASTER2,  false, true,  NULL,                                           "", eventCommandTable },
        { "changefaction",  SEC_PLAYER,       false, false, &ChatHandler::HandleRaceOrFactionChange,        "" },
        { "flusharenapoints",SEC_GAMEMASTER3, false, false, &ChatHandler::HandleFlushArenaPointsCommand,    "" },
        { "freeze",         SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleFreezeCommand,              "" },
        { "getmoveflags",   SEC_GAMEMASTER2,  false, false, &ChatHandler::HandleGetMoveFlagsCommand,        "" },
        { "gm",             SEC_GAMEMASTER1,  true,  false, NULL,                                           "", gmCommandTable },
        { "gmannounce",     SEC_GAMEMASTER1,  true,  false, &ChatHandler::HandleGMAnnounceCommand,          "" },
        { "gmnameannounce", SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleGMNameAnnounceCommand,      "" },
        { "gmnotify",       SEC_GAMEMASTER1,  true,  false, &ChatHandler::HandleGMNotifyCommand,            "" },
        { "go",             SEC_GAMEMASTER1,  false, false,  NULL,                                           "", goCommandTable },
        { "gobject",        SEC_GAMEMASTER2,  false, true,  NULL,                                           "", gobjectCommandTable },
        { "goname",         SEC_GAMEMASTER1,  false, false, &ChatHandler::HandleGonameCommand,              "" },
        { "gps",            SEC_GAMEMASTER1,  true,  true,  &ChatHandler::HandleGPSCommand,                 "" },
        { "gpss",           SEC_GAMEMASTER1,  false, false, &ChatHandler::HandleGPSSCommand,                "" },
        { "groupgo",        SEC_GAMEMASTER1,  false, false, &ChatHandler::HandleGroupgoCommand,             "" },
        { "guid",           SEC_GAMEMASTER2,  false, false, &ChatHandler::HandleGUIDCommand,                "" },
        { "guild",          SEC_GAMEMASTER3,  true,  true,  NULL,                                           "", guildCommandTable },
        { "help",           SEC_PLAYER,       true,  false, &ChatHandler::HandleHelpCommand,                "" },
        { "heroday",        SEC_PLAYER,       true,  true,  &ChatHandler::HandleHerodayCommand,             "" },
        { "hidearea",       SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleHideAreaCommand,            "" },
        { "honor",          SEC_GAMEMASTER2,  false, false, NULL,                                           "", honorCommandTable },
        { "hover",          SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleHoverCommand,               "" },
        { "instance",       SEC_GAMEMASTER3,  true,  true,  NULL,                                           "", instanceCommandTable },
        { "irc",            SEC_GAMEMASTER3,  true,  false, NULL,                                           "", ircCommandtable },
        { "itemmove",       SEC_GAMEMASTER2,  false, false, &ChatHandler::HandleItemMoveCommand,            "" },
        { "kick",           SEC_GAMEMASTER2,  true,  false, &ChatHandler::HandleKickPlayerCommand,          "" },
        { "learn",          SEC_GAMEMASTER1,  false, false, NULL,                                           "", learnCommandTable },
        { "levelup",        SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleLevelUpCommand,             "" },
        { "linkgrave",      SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleLinkGraveCommand,           "" },
        { "list",           SEC_GAMEMASTER3,  true,  false, NULL,                                           "", listCommandTable },
        { "listfreeze",     SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleListFreezeCommand,          "" },
        { "loadpath",       SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleReloadAllPaths,             "" },
        { "loadscripts",    SEC_GAMEMASTER3,  true,  false, &ChatHandler::HandleLoadScriptsCommand,         "" },
        { "lockaccount",    SEC_PLAYER,       false, false, &ChatHandler::HandleLockAccountCommand,         "" },
        { "lookup",         SEC_GAMEMASTER3,  true,  false, NULL,                                           "", lookupCommandTable },
        { "maxpool",        SEC_GAMEMASTER3,  true,  true,  &ChatHandler::HandleGetMaxCreaturePoolIdCommand,"" },
        { "maxskill",       SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleMaxSkillCommand,            "" },
        { "mmap",           SEC_GAMEMASTER2,  false, false, NULL,                                           "", mmapCommandTable },
        { "modify",         SEC_GAMEMASTER1,  false, false, NULL,                                           "", modifyCommandTable },
        { "movegens",       SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleMovegensCommand,            "" },
        { "mute",           SEC_GAMEMASTER2,  true,  false, &ChatHandler::HandleMuteCommand,                "" },
        { "muteinfo",       SEC_GAMEMASTER3,  false, false, NULL,                                           "", muteinfoCommandTable },
        { "nameannounce",   SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleNameAnnounceCommand,        "" },
        { "namego",         SEC_GAMEMASTER1,  false, false, &ChatHandler::HandleNamegoCommand,              "" },
        { "neargrave",      SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleNearGraveCommand,           "" },
        { "notify",         SEC_GAMEMASTER1,  true,  false, &ChatHandler::HandleNotifyCommand,              "" },
        { "npc",            SEC_GAMEMASTER1,  false, false, NULL,                                           "", npcCommandTable },
        { "npcevent",       SEC_GAMEMASTER2,  false, false, NULL,                                           "", npcEventCommandTable },
        { "password",       SEC_PLAYER,       false, false, &ChatHandler::HandlePasswordCommand,            "" },
        { "path",           SEC_GAMEMASTER2,  false, false, NULL,                                           "", wpCommandTable },
        { "pdump",          SEC_GAMEMASTER3,  true,  false, NULL,                                           "", pdumpCommandTable },
        { "pet",            SEC_GAMEMASTER2,  false, false, NULL,                                           "", petCommandTable },
        { "pinfo",          SEC_GAMEMASTER2,  true,  true,  &ChatHandler::HandlePInfoCommand,               "" },
        { "playall",        SEC_GAMEMASTER3,  false, false, &ChatHandler::HandlePlayAllCommand,             "" },
        { "plimit",         SEC_GAMEMASTER3,  true,  false, &ChatHandler::HandlePLimitCommand,              "" },
        { "possess",        SEC_GAMEMASTER3,  false, false, &ChatHandler::HandlePossessCommand,             "" },
        { "quest",          SEC_GAMEMASTER3,  false, false, NULL,                                           "", questCommandTable },
        { "recall",         SEC_GAMEMASTER1,  false, false, &ChatHandler::HandleRecallCommand,              "" },
        { "recup",          SEC_PLAYER,       false, false, &ChatHandler::HandleRecupCommand,               "" },
        { "reload",         SEC_ADMINISTRATOR,true,  false, NULL,                                           "", reloadCommandTable },
        { "removetitle"    ,SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleRemoveTitleCommand,         "" },
        { "rename",         SEC_GAMEMASTER2,  true,  false, &ChatHandler::HandleRenameCommand,              "" },
        { "repairitems",    SEC_GAMEMASTER2,  false, false, &ChatHandler::HandleRepairitemsCommand,         "" },
        { "reportlag",      SEC_PLAYER,       false, false, &ChatHandler::HandleReportLagCommand,           "" },
        { "reset",          SEC_GAMEMASTER3,  false, false, NULL,                                           "", resetCommandTable },
        { "reskin",         SEC_PLAYER,       false, false, &ChatHandler::HandleReskinCommand,              "" },
        { "respawn",        SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleRespawnCommand,             "" },
        { "revive",         SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleReviveCommand,              "" },
        { "save",           SEC_PLAYER,       false, false, &ChatHandler::HandleSaveCommand,                "" },
        { "saveall",        SEC_GAMEMASTER1,  true,  false, &ChatHandler::HandleSaveAllCommand,             "" },
        { "senditems",      SEC_GAMEMASTER3,  true,  false, &ChatHandler::HandleSendItemsCommand,           "" },
        { "sendmail",       SEC_GAMEMASTER1,  true,  false, &ChatHandler::HandleSendMailCommand,            "" },
        { "sendmessage",    SEC_GAMEMASTER3,  true,  false, &ChatHandler::HandleSendMessageCommand,         "" },
        { "sendmoney",      SEC_GAMEMASTER3,  true,  false, &ChatHandler::HandleSendMoneyCommand,           "" },
        { "server",         SEC_GAMEMASTER3,  true,  false, NULL,                                           "", serverCommandTable },
        { "setmoveflags",   SEC_GAMEMASTER2,  false, false, &ChatHandler::HandleSetMoveFlagsCommand,        "" },
        { "setskill",       SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleSetSkillCommand,            "" },
        { "settitle"       ,SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleSetTitleCommand,            "" },
        { "showarea",       SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleShowAreaCommand,            "" },
        { "spectator",      SEC_PLAYER,       false, false, NULL,                                           "", spectateCommandTable },
        { "spellinfo",      SEC_GAMEMASTER3,  true,  false, &ChatHandler::HandleSpellInfoCommand,           "" },
        { "smartai",        SEC_GAMEMASTER3,  true,  true,  NULL,                                           "", smartAICommandTable },
        { "start",          SEC_PLAYER,       false, false, &ChatHandler::HandleStartCommand,               "" },
        { "tele",           SEC_GAMEMASTER1,  true,  false, NULL,                                           "", teleCommandTable },
        { "ticket",         SEC_GAMEMASTER1,  false, false, NULL,                                           "", ticketCommandTable },
        { "unaura",         SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleUnAuraCommand,              "" },
        { "unban",          SEC_GAMEMASTER3,  true,  false, NULL,                                           "", unbanCommandTable },
        { "unbindsight",    SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleUnbindSightCommand,         "" },
        { "unfreeze",       SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleUnFreezeCommand,            "" },
        { "unlearn",        SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleUnLearnCommand,             "" },
        { "unmute",         SEC_GAMEMASTER2,  true,  false, &ChatHandler::HandleUnmuteCommand,              "" },
        { "unpossess",      SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleUnPossessCommand,           "" },
        { "updatetitle",    SEC_PLAYER,       false, false, &ChatHandler::HandleUpdatePvPTitleCommand,      "" },
        { "wchange",        SEC_GAMEMASTER3,  false, false, &ChatHandler::HandleChangeWeather,              "" },
        { "whispers",       SEC_GAMEMASTER1,  false, false, &ChatHandler::HandleWhispersCommand,            "" },
        { "yolo",           SEC_GAMEMASTER1,  true,  true,  &ChatHandler::HandleYoloCommand,                "" },
        { "zone",           SEC_GAMEMASTER3,  false, false, NULL,                                           "", zoneCommandTable },
    };

    /**
     * Values in database take precedence over default values in command table
     */
    if(load_command_table)
    {
        load_command_table = false;

        QueryResult result = WorldDatabase.Query("SELECT name, security, help, ircAllowed FROM command");
        if (result)
        {
            do
            {
                Field* fields = result->Fetch();
                std::string fullName = fields[0].GetString();
                
                SetDataForCommandInTable(commandTable, fullName.c_str(), fields[1].GetUInt8(), fields[2].GetString(), fullName, fields[3].GetBool());

            } while(result->NextRow());
        }
    }

    return commandTable;
}

void ChatHandler::SendMessageWithoutAuthor(char const* channel, const char* msg)
{
    WorldPacket data;
    ChatHandler::BuildChatPacket(data, CHAT_MSG_CHANNEL, LANG_UNIVERSAL, nullptr, nullptr, msg, 0, channel);

    boost::shared_lock<boost::shared_mutex> lock(*HashMapHolder<Player>::GetLock());
    HashMapHolder<Player>::MapType& m = ObjectAccessor::GetPlayers();
    for(HashMapHolder<Player>::MapType::iterator itr = m.begin(); itr != m.end(); ++itr)
    {
        if (itr->second && itr->second->GetSession()->GetPlayer() && itr->second->GetSession()->GetPlayer()->IsInWorld())
        {
            if(ChannelMgr* cMgr = channelMgr(itr->second->GetSession()->GetPlayer()->GetTeam()))
            {
                if(Channel *chn = cMgr->GetChannel(channel, itr->second->GetSession()->GetPlayer()))
                {
                    itr->second->GetSession()->SendPacket(&data);
                }
            }
        }
    }
}

const char* ChatHandler::GetTrinityString(int32 entry) const
{
    if (m_session)
        return m_session->GetTrinityString(entry);
    else
        return sObjectMgr->GetTrinityString(entry, sWorld->GetDefaultDbcLocale());
}

std::string ChatHandler::GetTrinityStringVA(int32 entry, ...) const
{
    const char *format = GetTrinityString(entry);
    va_list ap;
    char str [1024];
    va_start(ap, entry);
    vsnprintf(str, 1024, format, ap );
    va_end(ap);
    return str;
}

bool ChatHandler::isAvailable(ChatCommand const& cmd) const
{
    return m_session->GetSecurity() >= cmd.SecurityLevel;
}

bool ChatHandler::hasStringAbbr(const char* name, const char* part) const
{
    if (!name)
        return false;

    // non "" command
    if( *name )
    {
        // "" part from non-"" command
        if( !*part )
            return false;

        for(;;)
        {
            if( !*part )
                return true;
            else if( !*name )
                return false;
            else if( tolower( *name ) != tolower( *part ) )
                return false;
            ++name; ++part;
        }
    }
    // allow with any for ""

    return true;
}

void ChatHandler::SendSysMessage(const char *str, bool escapeCharacters)
{
    WorldPacket data;

    // need copy to prevent corruption by strtok call in LineFromMessage original string
    char* buf;
    char* pos;

    if (escapeCharacters && strchr(str, '|'))
    {
        size_t startPos = 0;
        std::ostringstream o;
        while (const char* charPos = strchr(str + startPos, '|'))
        {
            o.write(str + startPos, charPos - str - startPos);
            o << "||";
            startPos = charPos - str + 1;
        }
        o.write(str + startPos, strlen(str) - startPos);
        buf = strdup(o.str().c_str());
    }
    else
    {
        buf = strdup(str);
    }

    pos = buf;

    while (char* line = LineFromMessage(pos))
    {
        BuildChatPacket(data, CHAT_MSG_SYSTEM, LANG_UNIVERSAL, nullptr, nullptr, line);
        m_session->SendPacket(&data);
    }

    free(buf);
}

void ChatHandler::SendGlobalSysMessage(const char *str)
{
    // Chat output
    WorldPacket data;

    // need copy to prevent corruption by strtok call in LineFromMessage original string
    char* buf = strdup(str);
    char* pos = buf;

    while(char* line = LineFromMessage(pos))
    {
        BuildChatPacket(data, CHAT_MSG_SYSTEM, LANG_UNIVERSAL, nullptr, nullptr, line);
        sWorld->SendGlobalMessage(&data);
    }

    free(buf);

    if (sWorld->getConfig(CONFIG_IRC_ENABLED))
    {
        std::string msg(str);
        sIRCMgr->sendGlobalMsgToIRC(msg);
    }
}

void ChatHandler::SendGlobalGMSysMessage(const char *str)
{
    // Chat output
    WorldPacket data;

    // need copy to prevent corruption by strtok call in LineFromMessage original string
    char* buf = strdup(str);
    char* pos = buf;

    while(char* line = LineFromMessage(pos))
    {
        BuildChatPacket(data, CHAT_MSG_SYSTEM, LANG_UNIVERSAL, nullptr, nullptr, line);
        sWorld->SendGlobalGMMessage(&data);
    }
    free(buf);

    if (sWorld->getConfig(CONFIG_IRC_ENABLED))
    {
        std::string msg(str);
        sIRCMgr->sendGlobalMsgToIRC(msg);
    }
}

void ChatHandler::SendSysMessage(int32 entry)
{
    SendSysMessage(GetTrinityString(entry));
}

bool ChatHandler::ExecuteCommandInTable(std::vector<ChatCommand> const& table, const char* text, const std::string& fullcmd)
{
    char const* oldtext = text;
    std::string cmd = "";

    while (*text != ' ' && *text != '\0')
    {
        cmd += *text;
        ++text;
    }

    while (*text == ' ') ++text;

    for (uint32 i = 0; i < table.size(); ++i)
    {
        if( !hasStringAbbr(table[i].Name, cmd.c_str()) )
            continue;

        bool match = false;
        if (strlen(table[i].Name) > cmd.length())
        {
            for (uint32 j = 0; j < table.size(); ++j)
            {
                if (!hasStringAbbr(table[j].Name, cmd.c_str()))
                    continue;

                if (strcmp(table[j].Name, cmd.c_str()) == 0)
                {
                    match = true;
                    break;
                }
            }
        }
        if (match)
            continue;

        // select subcommand from child commands list
        if(!table[i].ChildCommands.empty())
        {
            if(!ExecuteCommandInTable(table[i].ChildCommands, text, fullcmd))
            {
                if(text && text[0] != '\0')
                    SendSysMessage(LANG_NO_SUBCMD);
                else
                    SendSysMessage(LANG_CMD_SYNTAX);

                ShowHelpForCommand(table[i].ChildCommands,text);
            }

            return true;
        }

        // must be available and have handler
        if(!table[i].Handler || !isAvailable(table[i]))
            continue;

        SetSentErrorMessage(false);
        // table[i].Name == "" is special case: send original command to handler
        if((this->*(table[i].Handler))(strlen(table[i].Name)!=0 ? text : oldtext))
        {
            //log command
            Unit const* target = m_session ? (m_session->GetPlayer() ? m_session->GetPlayer()->GetSelectedUnit() : nullptr) : nullptr;
            LogsDatabaseAccessor::GMCommand(m_session, target, fullcmd);
        }
        // some commands have custom error messages. Don't send the default one in these cases.
        else if(!sentErrorMessage)
        {
            if(!table[i].Help.empty())
                SendSysMessage(table[i].Help.c_str());
            else
                SendSysMessage(LANG_CMD_SYNTAX);
        }

        return true;
    }

    return false;
}

int ChatHandler::ParseCommands(const char* text)
{
    ASSERT(text);
    ASSERT(*text);

    std::string fullcmd = text;

    /// chat case (.command or !command format)
    if(m_session)
    {
        if( (text[0] != '!') && (text[0] != '.') )
            return 0;
    }

    /// ignore single . or ! in line
    if(strlen(text) < 2)
        return 0;
    // original `text` can't be used. It content destroyed in command code processing.

    /// ignore messages starting with many . or !
    if((text[0] == '.' && text[1] == '.') || (text[0] == '!' && text[1] == '!'))
        return 0;

    /// skip first . or ! (in console allowed use command with . and ! and without its)
    if(text[0] == '!' || text[0] == '.')
        ++text;

    if(!ExecuteCommandInTable(getCommandTable(), text, fullcmd))
    {
        if(m_session && m_session->GetSecurity() == SEC_PLAYER)
            return 0;

        SendSysMessage(LANG_NO_CMD);
    }
    return 1;
}

bool ChatHandler::ShowHelpForSubCommands(std::vector<ChatCommand> const& table, char const* cmd, char const* subcmd)
{
    std::string list;
    for (uint32 i = 0; i < table.size(); ++i)
    {
        // must be available (ignore handler existence for show command with possibe avalable subcomands
        if(!isAvailable(table[i]))
            continue;

        // for empty subcmd show all available
        if( *subcmd && !hasStringAbbr(table[i].Name, subcmd))
            continue;

        if(m_session)
            list += "\n    ";
        else
            list += "\n\r    ";

        list += table[i].Name;

        if(!table[i].ChildCommands.empty())
            list += " ...";
    }

    if(list.empty())
        return false;

    if (&table == &getCommandTable())
    {
        SendSysMessage(LANG_AVAILABLE_CMD);
        PSendSysMessage("%s",list.c_str());
    }
    else
        PSendSysMessage(LANG_SUBCMDS_LIST,cmd,list.c_str());

    return true;
}

bool ChatHandler::ShowHelpForCommand(std::vector<ChatCommand> const& table, const char* cmd)
{
    if(*cmd)
    {
        for (uint32 i = 0; i < table.size(); ++i)
        {
            // must be available (ignore handler existence for show command with possibe avalable subcomands
            if(!isAvailable(table[i]))
                continue;

            if( !hasStringAbbr(table[i].Name, cmd) )
                continue;

            // have subcommand
            char const* subcmd = (*cmd) ? strtok(NULL, " ") : "";

            if(!table[i].ChildCommands.empty() && subcmd && *subcmd)
            {
                if(ShowHelpForCommand(table[i].ChildCommands, subcmd))
                    return true;
            }

            if(!table[i].Help.empty())
                SendSysMessage(table[i].Help.c_str());

            if(!table[i].ChildCommands.empty())
                if(ShowHelpForSubCommands(table[i].ChildCommands,table[i].Name,subcmd ? subcmd : ""))
                    return true;

            return !table[i].Help.empty();
        }
    }
    else
    {
        for (uint32 i = 0; i < table.size(); ++i)
        {
            // must be available (ignore handler existence for show command with possibe avalable subcomands
            if(!isAvailable(table[i]))
                continue;

            if(strlen(table[i].Name))
                continue;

            if(!table[i].Help.empty())
                SendSysMessage(table[i].Help.c_str());

            if(!table[i].ChildCommands.empty())
                if(ShowHelpForSubCommands(table[i].ChildCommands,"",""))
                    return true;

            return !table[i].Help.empty();
        }
    }

    return ShowHelpForSubCommands(table,"",cmd);
}

size_t ChatHandler::BuildChatPacket(WorldPacket& data, ChatMsg chatType, Language language, uint64 senderGUID, uint64 receiverGUID, std::string const& message, uint8 chatTag,
                                  std::string const& senderName /*= ""*/, std::string const& receiverName /*= ""*/,
                                  uint32 achievementId /*= 0*/, bool gmMessage /*= false*/, std::string const& channelName /*= ""*/)
{
    
#ifndef LICH_KING
    gmMessage = false;  // SMSG_GM_MESSAGECHAT seems to have a different structure on BC, disable it for now
#endif
    size_t receiverGUIDPos = 0;
    data.Initialize(!gmMessage ? SMSG_MESSAGECHAT : SMSG_GM_MESSAGECHAT);
    data << uint8(chatType);
    data << int32(language);
    data << uint64(senderGUID);
    data << uint32(0); // some flags
    switch (chatType)
    {
        case CHAT_MSG_MONSTER_SAY:
        case CHAT_MSG_MONSTER_PARTY:
        case CHAT_MSG_MONSTER_YELL:
        case CHAT_MSG_MONSTER_WHISPER:
        case CHAT_MSG_MONSTER_EMOTE:
        case CHAT_MSG_RAID_BOSS_EMOTE:
        case CHAT_MSG_RAID_BOSS_WHISPER:
#ifdef LICH_KING
        case CHAT_MSG_BATTLENET:
#endif
            data << uint32(senderName.length() + 1);
            data << senderName;
            receiverGUIDPos = data.wpos();
            data << uint64(receiverGUID);
            if (receiverGUID && !IS_PLAYER_GUID(receiverGUID) && !IS_PET_GUID(receiverGUID))
            {
                data << uint32(receiverName.length() + 1);
                data << receiverName;
            }
            break;
#ifdef LICH_KING
        case CHAT_MSG_WHISPER_FOREIGN:
            data << uint32(senderName.length() + 1);
            data << senderName;
            receiverGUIDPos = data.wpos();
            data << uint64(receiverGUID);
            break;
#endif
        case CHAT_MSG_BG_SYSTEM_NEUTRAL:
        case CHAT_MSG_BG_SYSTEM_ALLIANCE:
        case CHAT_MSG_BG_SYSTEM_HORDE:
            receiverGUIDPos = data.wpos();
            data << uint64(receiverGUID);
            if (receiverGUID && !IS_PLAYER_GUID(receiverGUID))
            {
                data << uint32(receiverName.length() + 1);
                data << receiverName;
            }
            break;
#ifdef LICH_KING
        case CHAT_MSG_ACHIEVEMENT:
        case CHAT_MSG_GUILD_ACHIEVEMENT:
            receiverGUIDPos = data.wpos();
            data << uint64(receiverGUID);
            break;
#endif
        default:
            if (gmMessage)
            {
                data << uint32(senderName.length() + 1);
                data << senderName;
            }

            if (chatType == CHAT_MSG_CHANNEL)
            {
                ASSERT(channelName.length() > 0);
                data << channelName;
            }

            receiverGUIDPos = data.wpos();
            data << uint64(receiverGUID);
            break;
    }

    data << uint32(message.length() + 1);
    data << message;
    data << uint8(chatTag);

#ifdef LICH_KING
    if (chatType == CHAT_MSG_ACHIEVEMENT || chatType == CHAT_MSG_GUILD_ACHIEVEMENT)
        data << uint32(achievementId);
#endif

    return receiverGUIDPos;
}

size_t ChatHandler::BuildChatPacket(WorldPacket& data, ChatMsg chatType, Language language, WorldObject const* sender, WorldObject const* receiver, std::string const& message,
                                  uint32 achievementId /*= 0*/, std::string const& channelName /*= ""*/, LocaleConstant locale /*= DEFAULT_LOCALE*/)
{
    uint64 senderGUID = 0;
    std::string senderName = "";
    uint8 chatTag = 0;
    bool gmMessage = false;
    uint64 receiverGUID = 0;
    std::string receiverName = "";
    if (sender)
    {
        senderGUID = sender->GetGUID();
        senderName = sender->GetNameForLocaleIdx(locale);
        if (Player const* playerSender = sender->ToPlayer())
        {
            chatTag = playerSender->GetChatTag();

            //TC RBAC gmMessage = playerSender->GetSession()->HasPermission(rbac::RBAC_PERM_COMMAND_GM_CHAT);
            gmMessage = playerSender->IsGMChat();
        }
    }

    if (receiver)
    {
        receiverGUID = receiver->GetGUID();
        receiverName = receiver->GetNameForLocaleIdx(locale);
    }

    return BuildChatPacket(data, chatType, language, senderGUID, receiverGUID, message, chatTag, senderName, receiverName, achievementId, gmMessage, channelName);
}

Player * ChatHandler::GetSelectedPlayer() const
{
    if(!m_session)
        return nullptr;

    uint64 guid  = m_session->GetPlayer()->GetTarget();

    if (guid == 0)
        return m_session->GetPlayer();

    return sObjectMgr->GetPlayer(guid);
}

Player* ChatHandler::GetSelectedPlayerOrSelf() const
{
    if (!m_session)
        return nullptr;

    uint64 selected = m_session->GetPlayer()->GetTarget();
    if (!selected)
        return m_session->GetPlayer();

    // first try with selected target
    Player* targetPlayer = ObjectAccessor::FindPlayer(selected);
    // if the target is not a player, then return self
    if (!targetPlayer)
        targetPlayer = m_session->GetPlayer();

    return targetPlayer;
}

Unit* ChatHandler::GetSelectedUnit() const
{
    if(!m_session)
        return nullptr;

    uint64 guid = m_session->GetPlayer()->GetTarget();

    if (guid == 0)
        return m_session->GetPlayer();

    return ObjectAccessor::GetUnit(*m_session->GetPlayer(),guid);
}

Creature* ChatHandler::GetSelectedCreature() const
{
    if(!m_session)
        return nullptr;

    return ObjectAccessor::GetCreatureOrPetOrVehicle(*m_session->GetPlayer(),m_session->GetPlayer()->GetTarget());
}

char* ChatHandler::extractKeyFromLink(char* text, char const* linkType, char** something1)
{
    // skip empty
    if(!text)
        return nullptr;

    // skip spaces
    while(*text==' '||*text=='\t'||*text=='\b')
        ++text;

    if(!*text)
        return nullptr;

    // return non link case
    if(text[0]!='|')
        return strtok(text, " ");

    // [name] Shift-click form |color|linkType:key|h[name]|h|r
    // or
    // [name] Shift-click form |color|linkType:key:something1:...:somethingN|h[name]|h|r

    char* check = strtok(text, "|");                        // skip color
    if(!check)
        return nullptr;                                        // end of data

    char* cLinkType = strtok(NULL, ":");                    // linktype
    if(!cLinkType)
        return nullptr;                                        // end of data

    if(strcmp(cLinkType,linkType) != 0)
    {
        strtok(NULL, " ");                                  // skip link tail (to allow continue strtok(NULL,s) use after retturn from function
        SendSysMessage(LANG_WRONG_LINK_TYPE);
        return nullptr;
    }

    char* cKeys = strtok(NULL, "|");                        // extract keys and values
    char* cKeysTail = strtok(NULL, "");

    char* cKey = strtok(cKeys, ":|");                       // extract key
    if(something1)
        *something1 = strtok(NULL, ":|");                   // extract something

    strtok(cKeysTail, "]");                                 // restart scan tail and skip name with possible spaces
    strtok(NULL, " ");                                      // skip link tail (to allow continue strtok(NULL,s) use after retturn from function
    return cKey;
}

char* ChatHandler::extractKeyFromLink(char* text, char const* const* linkTypes, int* found_idx, char** something1)
{
    // skip empty
    if(!text)
        return NULL;

    // skip spaces
    while(*text==' '||*text=='\t'||*text=='\b')
        ++text;

    if(!*text)
        return NULL;

    // return non link case
    if(text[0]!='|')
        return strtok(text, " ");

    // [name] Shift-click form |color|linkType:key|h[name]|h|r
    // or
    // [name] Shift-click form |color|linkType:key:something1:...:somethingN|h[name]|h|r

    char* check = strtok(text, "|");                        // skip color
    if(!check)
        return NULL;                                        // end of data

    char* cLinkType = strtok(NULL, ":");                    // linktype
    if(!cLinkType)
        return NULL;                                        // end of data

    for(int i = 0; linkTypes[i]; ++i)
    {
        if(strcmp(cLinkType,linkTypes[i]) == 0)
        {
            char* cKeys = strtok(NULL, "|");                // extract keys and values
            char* cKeysTail = strtok(NULL, "");

            char* cKey = strtok(cKeys, ":|");               // extract key
            if(something1)
                *something1 = strtok(NULL, ":|");           // extract something

            strtok(cKeysTail, "]");                         // restart scan tail and skip name with possible spaces
            strtok(NULL, " ");                              // skip link tail (to allow continue strtok(NULL,s) use after return from function
            if(found_idx)
                *found_idx = i;
            return cKey;
        }
    }

    strtok(NULL, " ");                                      // skip link tail (to allow continue strtok(NULL,s) use after return from function
    SendSysMessage(LANG_WRONG_LINK_TYPE);
    return NULL;
}

char const *fmtstring( char const *format, ... )
{
    va_list        argptr;
    #define    MAX_FMT_STRING    32000
    static char        temp_buffer[MAX_FMT_STRING];
    static char        string[MAX_FMT_STRING];
    static int        index = 0;
    char    *buf;
    int len;

    va_start(argptr, format);
    vsnprintf(temp_buffer,MAX_FMT_STRING, format, argptr);
    va_end(argptr);

    len = strlen(temp_buffer);

    if( len >= MAX_FMT_STRING )
        return "ERROR";

    if (len + index >= MAX_FMT_STRING-1)
    {
        index = 0;
    }

    buf = &string[index];
    memcpy( buf, temp_buffer, len+1 );

    index += len + 1;

    return buf;
}

GameObject* ChatHandler::GetObjectGlobalyWithGuidOrNearWithDbGuid(uint32 lowguid,uint32 entry)
{
    if(!m_session)
        return NULL;

    Player* pl = m_session->GetPlayer();

    GameObject* obj = ObjectAccessor::GetGameObject(*pl, MAKE_NEW_GUID(lowguid, entry, HIGHGUID_GAMEOBJECT));

    if(!obj && sObjectMgr->GetGOData(lowguid))                   // guid is DB guid of object
    {
        // search near player then
        CellCoord p(Trinity::ComputeCellCoord(pl->GetPositionX(), pl->GetPositionY()));
        Cell cell(p);
        cell.data.Part.reserved = ALL_DISTRICT;

        Trinity::GameObjectWithDbGUIDCheck go_check(*pl,lowguid);
        Trinity::GameObjectSearcher<Trinity::GameObjectWithDbGUIDCheck> checker(obj,go_check);

        TypeContainerVisitor<Trinity::GameObjectSearcher<Trinity::GameObjectWithDbGUIDCheck>, GridTypeMapContainer > object_checker(checker);
        cell.Visit(p, object_checker, *pl->GetMap());
    }

    return obj;
}

static char const* const spellTalentKeys[] = {
    "Hspell",
    "Htalent",
    0
};

uint32 ChatHandler::extractSpellIdFromLink(char* text)
{
    // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r
    // number or [name] Shift-click form |color|Htalent:talent_id,rank|h[name]|h|r
    int type = 0;
    char* rankS = NULL;
    char* idS = extractKeyFromLink(text,spellTalentKeys,&type,&rankS);
    if(!idS)
        return 0;

    uint32 id = (uint32)atol(idS);

    // spell
    if(type==0)
        return id;

    // talent
    TalentEntry const* talentEntry = sTalentStore.LookupEntry(id);
    if(!talentEntry)
        return 0;

    int32 rank = rankS ? (uint32)atol(rankS) : 0;
    if(rank >= 5)
        return 0;

    if(rank < 0)
        rank = 0;

    return talentEntry->RankID[rank];
}

GameTele const* ChatHandler::extractGameTeleFromLink(char* text)
{
    // id, or string, or [name] Shift-click form |color|Htele:id|h[name]|h|r
    char* cId = extractKeyFromLink(text,"Htele");
    if(!cId)
        return nullptr;

    // id case (explicit or from shift link)
    if(cId[0] >= '0' || cId[0] >= '9')
        if(uint32 id = atoi(cId))
            return sObjectMgr->GetGameTele(id);

    return sObjectMgr->GetGameTele(cId);
}

std::string const ChatHandler::GetName() const
{
    return m_session->GetPlayer()->GetName();
}

bool ChatHandler::needReportToTarget(Player* chr) const
{
    Player* pl = m_session->GetPlayer();
    return pl != chr && pl->IsVisibleGloballyFor(chr);
}

LocaleConstant ChatHandler::GetSessionDbcLocale() const
{
    return m_session ? m_session->GetSessionDbcLocale() : LOCALE_enUS;
}

const char *CliHandler::GetTrinityString(int32 entry) const
{
    return sObjectMgr->GetTrinityStringForDBCLocale(entry);
}

bool CliHandler::isAvailable(ChatCommand const& cmd) const
{
    // skip non-console commands in console case
    return cmd.noSessionNeeded;
}

void CliHandler::SendSysMessage(const char *str, bool escapeCharacters)
{
    m_print(m_callbackArg, str);
    m_print(m_callbackArg, "\r\n");
}

std::string CliHandler::GetNameLink() const
{
    return GetTrinityString(LANG_CONSOLE_COMMAND);
}

bool CliHandler::needReportToTarget(Player* /*chr*/) const
{
    return true;
}

LocaleConstant CliHandler::GetSessionDbcLocale() const
{
    return sWorld->GetDefaultDbcLocale();
}

bool ChatHandler::GetPlayerGroupAndGUIDByName(const char* cname, Player* &plr, Group* &group, uint64 &guid, bool offline)
{
    plr  = NULL;
    guid = 0;

    if(cname)
    {
        std::string name = cname;
        if(!name.empty())
        {
            if(!normalizePlayerName(name))
            {
                PSendSysMessage(LANG_PLAYER_NOT_FOUND);
                SetSentErrorMessage(true);
                return false;
            }

            plr = sObjectAccessor->FindConnectedPlayerByName(name.c_str());
            if(offline)
                guid = sObjectMgr->GetPlayerGUIDByName(name.c_str());
        }
    }

    if(plr)
    {
        group = plr->GetGroup();
        if(!guid || !offline)
            guid = plr->GetGUID();
    }
    else
    {
        if(GetSelectedPlayer())
            plr = GetSelectedPlayer();
        else
            plr = m_session->GetPlayer();

        if(!guid || !offline)
            guid  = plr->GetGUID();
        group = plr->GetGroup();
    }

    return true;
}

std::string ChatHandler::extractPlayerNameFromLink(char* text)
{
    // |color|Hplayer:name|h[name]|h|r
    char* name_str = extractKeyFromLink(text, "Hplayer");
    if (!name_str)
        return "";

    std::string name = name_str;
    if (!normalizePlayerName(name))
        return "";

    return name;
}

bool ChatHandler::extractPlayerTarget(char* args, Player** player, uint64* player_guid /*=NULL*/, std::string* player_name /*= NULL*/)
{
    if (args && *args)
    {
        std::string name = extractPlayerNameFromLink(args);
        if (name.empty())
        {
            SendSysMessage(LANG_PLAYER_NOT_FOUND);
            SetSentErrorMessage(true);
            return false;
        }

        Player* pl = sObjectAccessor->FindPlayerByName(name.c_str());

        // if allowed player pointer
        if (player)
            *player = pl;

        // if need guid value from DB (in name case for check player existence)
        uint64 guid = !pl && (player_guid || player_name) ? sObjectMgr->GetPlayerGUIDByName(name) : 0;

        // if allowed player guid (if no then only online players allowed)
        if (player_guid)
            *player_guid = pl ? pl->GetGUID() : guid;

        if (player_name)
            *player_name = pl || guid ? name : "";
    }
    else
    {
        Player* pl = GetSelectedPlayer();
        // if allowed player pointer
        if (player)
            *player = pl;
        // if allowed player guid (if no then only online players allowed)
        if (player_guid)
            *player_guid = pl ? pl->GetGUID() : 0;

        if (player_name)
            *player_name = pl ? pl->GetName() : "";
    }

    // some from req. data must be provided (note: name is empty if player not exist)
    if ((!player || !*player) && (!player_guid || !*player_guid) && (!player_name || player_name->empty()))
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    return true;
}

/// Exit the realm
bool ChatHandler::HandleServerExitCommand(const char* args)
{
    SendSysMessage(LANG_COMMAND_EXIT);
    World::StopNow(SHUTDOWN_EXIT_CODE);
    return true;
}

bool ChatHandler::HandleCharacterDeleteCommand(const char* args)
{
    ARGS_CHECK

    char *character_name_str = strtok((char*)args," ");
    if(!character_name_str)
        return false;

    std::string character_name = character_name_str;
    if(!normalizePlayerName(character_name))
        return false;

    uint64 character_guid;
    uint32 account_id;

    Player *player = sObjectAccessor->FindConnectedPlayerByName(character_name.c_str());
    if(player)
    {
        character_guid = player->GetGUID();
        account_id = player->GetSession()->GetAccountId();
        player->GetSession()->KickPlayer();
    }
    else
    {
        character_guid = sObjectMgr->GetPlayerGUIDByName(character_name);
        if(!character_guid)
        {
            PSendSysMessage(LANG_NO_PLAYER,character_name.c_str());
            SetSentErrorMessage(true);
            return false;
        }

        account_id = sObjectMgr->GetPlayerAccountIdByGUID(character_guid);
    }

    std::string account_name;
    sAccountMgr->GetName (account_id,account_name);

    Player::DeleteFromDB(character_guid, account_id, true);
    PSendSysMessage(LANG_CHARACTER_DELETED,character_name.c_str(),GUID_LOPART(character_guid),account_name.c_str(), account_id);
    return true;
}

/// Delete a user account and all associated characters in this realm
/// \todo This function has to be enhanced to respect the login/realm split (delete char, delete account chars in realm, delete account chars in realm then delete account
bool ChatHandler::HandleAccountDeleteCommand(const char* args)
{
    ARGS_CHECK

    ///- Get the account name from the command line
    char *account_name_str=strtok ((char*)args," ");
    if (!account_name_str)
        return false;

    std::string account_name = account_name_str;
    if(!AccountMgr::normalizeString(account_name))
    {
        PSendSysMessage(LANG_ACCOUNT_NOT_EXIST,account_name.c_str());
        SetSentErrorMessage(true);
        return false;
    }

    uint32 account_id = sAccountMgr->GetId(account_name);
    if(!account_id)
    {
        PSendSysMessage(LANG_ACCOUNT_NOT_EXIST,account_name.c_str());
        SetSentErrorMessage(true);
        return false;
    }

    /// Commands not recommended call from chat, but support anyway
    if(m_session)
    {
        uint32 targetSecurity = sAccountMgr->GetSecurity(account_id);

        /// can delete only for account with less security
        /// This is also reject self apply in fact
        if (targetSecurity >= m_session->GetSecurity())
        {
            SendSysMessage (LANG_YOURS_SECURITY_IS_LOW);
            SetSentErrorMessage (true);
            return false;
        }
    }

    AccountOpResult result = sAccountMgr->DeleteAccount(account_id);
    switch(result)
    {
        case AOR_OK:
            PSendSysMessage(LANG_ACCOUNT_DELETED,account_name.c_str());
            break;
        case AOR_NAME_NOT_EXIST:
            PSendSysMessage(LANG_ACCOUNT_NOT_EXIST,account_name.c_str());
            SetSentErrorMessage(true);
            return false;
        case AOR_DB_INTERNAL_ERROR:
            PSendSysMessage(LANG_ACCOUNT_NOT_DELETED_SQL_ERROR,account_name.c_str());
            SetSentErrorMessage(true);
            return false;
        default:
            PSendSysMessage(LANG_ACCOUNT_NOT_DELETED,account_name.c_str());
            SetSentErrorMessage(true);
            return false;
    }

    return true;
}

/// Create an account
bool ChatHandler::HandleAccountCreateCommand(char const* args)
{
    ARGS_CHECK

    std::string email;

    ///- %Parse the command line arguments
    char* accountName = strtok((char*)args, " ");
    char* password = strtok(NULL, " ");
    char* possibleEmail = strtok(NULL, " ' ");
    if (possibleEmail)
        email = possibleEmail;

    if (!accountName || !password)
        return false;

    AccountOpResult result = sAccountMgr->CreateAccount(std::string(accountName), std::string(password), email);
    switch (result)
    {
        case AOR_OK:
            PSendSysMessage(LANG_ACCOUNT_CREATED, accountName);
            if (GetSession())
            {
                TC_LOG_INFO("entities.player.character", "Account: %d (IP: %s) Character:[%s] (GUID: %u) created Account %s (Email: '%s')",
                    GetSession()->GetAccountId(), GetSession()->GetRemoteAddress().c_str(),
                    GetSession()->GetPlayer()->GetName().c_str(), GetSession()->GetPlayer()->GetGUIDLow(),
                    accountName, email.c_str());
            }
            break;
        case AOR_NAME_TOO_LONG:
            SendSysMessage(LANG_ACCOUNT_TOO_LONG);
            SetSentErrorMessage(true);
            return false;
        case AOR_NAME_ALREADY_EXIST:
            SendSysMessage(LANG_ACCOUNT_ALREADY_EXIST);
            SetSentErrorMessage(true);
            return false;
        case AOR_DB_INTERNAL_ERROR:
            PSendSysMessage(LANG_ACCOUNT_NOT_CREATED_SQL_ERROR, accountName);
            SetSentErrorMessage(true);
            return false;
        default:
            PSendSysMessage(LANG_ACCOUNT_NOT_CREATED, accountName);
            SetSentErrorMessage(true);
            return false;
    }

    return true;
}