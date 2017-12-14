
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
#include "AccountMgr.h"
#include "LogsDatabaseAccessor.h"
#include "CharacterCache.h"

bool ChatHandler::load_command_table = true;

bool ChatHandler::SetDataForCommandInTable(std::vector<ChatCommand>& table, char const* text, uint32 securityLevel, std::string const& help, std::string const& fullcommand)
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

    for (auto & i : table)
    {
        if (!i.Name)
            return false;

        // for data fill use full explicit command names
        if (i.Name != cmd)
            continue;

        // select subcommand from child commands list (including "")
        if (!i.ChildCommands.empty())
        {
            if (SetDataForCommandInTable(i.ChildCommands, text, securityLevel, help, fullcommand))
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

        if (i.SecurityLevel != securityLevel)
            TC_LOG_DEBUG("misc", "Table `command` overwrite for command '%s' default permission (%u) by %u", fullcommand.c_str(), i.SecurityLevel, securityLevel);

        i.SecurityLevel = securityLevel;
        i.Help = help;
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
        { "addon",          SEC_GAMEMASTER3,          true, &ChatHandler::HandleAccountSetAddonCommand,     "" },
        { "gmlevel",        SEC_ADMINISTRATOR,        true, &ChatHandler::HandleAccountSetGmLevelCommand,   "" },
        { "password",       SEC_ADMINISTRATOR,        true, &ChatHandler::HandleAccountSetPasswordCommand,  "" },
    };

    static std::vector<ChatCommand> accountCommandTable =
    {
        { "create",         SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleAccountCreateCommand,      "" },
        { "delete",         SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleAccountDeleteCommand,      "" },
        { "mailchange",     SEC_GAMEMASTER2,    true,  &ChatHandler::HandleAccountMailChangeCommand,  "" },
        { "set",            SEC_GAMEMASTER3,    true,  nullptr,                                       "", accountSetCommandTable },
        { "",               SEC_PLAYER,         false, &ChatHandler::HandleAccountCommand,            "" },
    };

    static std::vector<ChatCommand> serverSetCommandTable =
    {
        { "motd",           SEC_GAMEMASTER3,    true, &ChatHandler::HandleServerSetMotdCommand,       "" },
        { "config",         SEC_SUPERADMIN,     true, &ChatHandler::HandleServerSetConfigCommand,     "" },
    };

    static std::vector<ChatCommand> serverIdleRestartCommandTable =
    {
        { "cancel",         SEC_ADMINISTRATOR , true, &ChatHandler::HandleServerShutDownCancelCommand,"" },
        { ""   ,            SEC_ADMINISTRATOR,  true, &ChatHandler::HandleServerIdleRestartCommand,   "" },
    };

    static std::vector<ChatCommand> serverIdleShutdownCommandTable =
    {
        { "cancel",         SEC_ADMINISTRATOR,  true, &ChatHandler::HandleServerShutDownCancelCommand,"" },
        { ""   ,            SEC_ADMINISTRATOR,  true, &ChatHandler::HandleServerIdleShutDownCommand,  "" },
    };

    static std::vector<ChatCommand> serverRestartCommandTable =
    {
        { "cancel",         SEC_ADMINISTRATOR,  true, &ChatHandler::HandleServerShutDownCancelCommand,"" },
        { ""   ,            SEC_ADMINISTRATOR,  true, &ChatHandler::HandleServerRestartCommand,       "" },
    };

    static std::vector<ChatCommand> serverShutdownCommandTable =
    {
        { "cancel",         SEC_ADMINISTRATOR,  true, &ChatHandler::HandleServerShutDownCancelCommand,"" },
        { ""   ,            SEC_ADMINISTRATOR,  true, &ChatHandler::HandleServerShutDownCommand,      "" },
    };

    static std::vector<ChatCommand> serverCommandTable =
    {
        { "corpses",        SEC_GAMEMASTER2,     true, &ChatHandler::HandleServerCorpsesCommand,       "" },
        { "exit",           SEC_ADMINISTRATOR,   true, &ChatHandler::HandleServerExitCommand,          "" },
        { "idlerestart",    SEC_ADMINISTRATOR,   true,  nullptr,                                       "", serverIdleRestartCommandTable },
        { "idleshutdown",   SEC_ADMINISTRATOR,   true,  nullptr,                                       "", serverShutdownCommandTable },
        { "info",           SEC_PLAYER,          true,  &ChatHandler::HandleServerInfoCommand,         "" },
        { "motd",           SEC_PLAYER,          true,  &ChatHandler::HandleServerMotdCommand,         "" },
        { "restart",        SEC_ADMINISTRATOR,   true,  nullptr,                                       "", serverRestartCommandTable },
        { "shutdown",       SEC_ADMINISTRATOR,   true,  nullptr,                                       "", serverShutdownCommandTable },
        { "set",            SEC_ADMINISTRATOR,   true,  nullptr,                                       "", serverSetCommandTable },
    };

    static std::vector<ChatCommand> mmapCommandTable =
    {
        { "path",           SEC_GAMEMASTER3,     false, &ChatHandler::HandleMmapPathCommand,            "" },
        { "loc",            SEC_GAMEMASTER3,     false, &ChatHandler::HandleMmapLocCommand,             "" },
        { "loadedtiles",    SEC_GAMEMASTER3,     false, &ChatHandler::HandleMmapLoadedTilesCommand,     "" },
        { "stats",          SEC_GAMEMASTER3,     false, &ChatHandler::HandleMmapStatsCommand,           "" },
        { "testarea",       SEC_GAMEMASTER3,     false, &ChatHandler::HandleMmapTestArea,               "" },
    };

    static std::vector<ChatCommand> modifyCommandTable =
    {
        { "hp",             SEC_GAMEMASTER1,      false, &ChatHandler::HandleModifyHPCommand,            "" },
        { "mana",           SEC_GAMEMASTER1,      false, &ChatHandler::HandleModifyManaCommand,          "" },
        { "rage",           SEC_GAMEMASTER1,      false, &ChatHandler::HandleModifyRageCommand,          "" },
        { "energy",         SEC_GAMEMASTER1,      false, &ChatHandler::HandleModifyEnergyCommand,        "" },
        { "money",          SEC_GAMEMASTER1,      false, &ChatHandler::HandleModifyMoneyCommand,         "" },
        { "speed",          SEC_GAMEMASTER1,      false, &ChatHandler::HandleModifySpeedCommand,         "" },
        { "swim",           SEC_GAMEMASTER1,      false, &ChatHandler::HandleModifySwimCommand,          "" },
        { "scale",          SEC_GAMEMASTER1,      false, &ChatHandler::HandleModifyScaleCommand,         "" },
        { "bit",            SEC_GAMEMASTER1,      false, &ChatHandler::HandleModifyBitCommand,           "" },
        { "bwalk",          SEC_GAMEMASTER1,      false, &ChatHandler::HandleModifyBWalkCommand,         "" },
        { "fly",            SEC_GAMEMASTER1,      false, &ChatHandler::HandleModifyFlyCommand,           "" },
        { "aspeed",         SEC_GAMEMASTER1,      false, &ChatHandler::HandleModifyASpeedCommand,        "" },
        { "faction",        SEC_GAMEMASTER1,      false, &ChatHandler::HandleModifyFactionCommand,       "" },
        { "spell",          SEC_GAMEMASTER2,      false, &ChatHandler::HandleModifySpellCommand,         "" },
        { "tp",             SEC_GAMEMASTER1,      false, &ChatHandler::HandleModifyTalentCommand,        "" },
        { "titles",         SEC_GAMEMASTER2,      false, &ChatHandler::HandleModifyKnownTitlesCommand,   "" },
        { "mount",          SEC_GAMEMASTER1,      false, &ChatHandler::HandleModifyMountCommand,         "" },
        { "honor",          SEC_GAMEMASTER2,      false, &ChatHandler::HandleModifyHonorCommand,         "" },
        { "rep",            SEC_GAMEMASTER2,      false, &ChatHandler::HandleModifyRepCommand,           "" },
        { "arena",          SEC_GAMEMASTER2,      false, &ChatHandler::HandleModifyArenaCommand,         "" },
        { "drunk",          SEC_GAMEMASTER1,      false, &ChatHandler::HandleDrunkCommand,               "" },
        { "morph",          SEC_GAMEMASTER2,      false, &ChatHandler::HandleMorphCommand,               "" },
        { "gender",         SEC_GAMEMASTER3,      false, &ChatHandler::HandleModifyGenderCommand,        "" },
    };

    static std::vector<ChatCommand> wpCommandTable =
    {
        { "show",           SEC_GAMEMASTER3,     false, &ChatHandler::HandleWpShowCommand,               "" },
        { "addwp",          SEC_GAMEMASTER3,     false, &ChatHandler::HandleWpAddCommand,                "" },
        { "load",           SEC_GAMEMASTER3,     false, &ChatHandler::HandleWpLoadPathCommand,           "" },
        { "modify",         SEC_GAMEMASTER3,     false, &ChatHandler::HandleWpModifyCommand,             "" },
        { "event",          SEC_GAMEMASTER3,     false, &ChatHandler::HandleWpEventCommand,              "" },
        { "unload",         SEC_GAMEMASTER3,     false, &ChatHandler::HandleWpUnLoadPathCommand,         "" },
        { "direction",      SEC_GAMEMASTER3,     false, &ChatHandler::HandleWpChangePathDirectionCommand,"" },
        { "teleport",       SEC_GAMEMASTER3,     false, &ChatHandler::HandleWpTeleportToPathCommand,     "" },
        { "type"     ,      SEC_GAMEMASTER3,     false, &ChatHandler::HandleWpChangePathTypeCommand,     "" },
    };


    static std::vector<ChatCommand> banCommandTable =
    {
        { "account",        SEC_GAMEMASTER3,  true, &ChatHandler::HandleBanAccountCommand,          "" },
        { "character",      SEC_GAMEMASTER3,  true, &ChatHandler::HandleBanCharacterCommand,        "" },
        { "ip",             SEC_GAMEMASTER3,  true, &ChatHandler::HandleBanIPCommand,               "" },
    };

    static std::vector<ChatCommand> baninfoCommandTable =
    {
        { "account",        SEC_GAMEMASTER3,  true, &ChatHandler::HandleBanInfoAccountCommand,      "" },
        { "character",      SEC_GAMEMASTER3,  true, &ChatHandler::HandleBanInfoCharacterCommand,    "" },
        { "ip",             SEC_GAMEMASTER3,  true, &ChatHandler::HandleBanInfoIPCommand,           "" },
    };

    static std::vector<ChatCommand> muteinfoCommandTable =
    {
        { "account",        SEC_GAMEMASTER3,  true, &ChatHandler::HandleMuteInfoAccountCommand,      "" },
        { "character",      SEC_GAMEMASTER3,  true, &ChatHandler::HandleMuteInfoCharacterCommand,    "" },
    };

    static std::vector<ChatCommand> banlistCommandTable =
    {
        { "account",        SEC_GAMEMASTER3,  true, &ChatHandler::HandleBanListAccountCommand,      "" },
        { "character",      SEC_GAMEMASTER3,  true, &ChatHandler::HandleBanListCharacterCommand,    "" },
        { "ip",             SEC_GAMEMASTER3,  true, &ChatHandler::HandleBanListIPCommand,           "" },
    };

    static std::vector<ChatCommand> unbanCommandTable =
    {
        { "account",        SEC_GAMEMASTER3,  true, &ChatHandler::HandleUnBanAccountCommand,        "" },
        { "character",      SEC_GAMEMASTER3,  true, &ChatHandler::HandleUnBanCharacterCommand,      "" },
        { "ip",             SEC_GAMEMASTER3,  true, &ChatHandler::HandleUnBanIPCommand,             "" },
    };


    static std::vector<ChatCommand> profilingCommandTable =
    {
        { "start",   SEC_SUPERADMIN,   true,  &ChatHandler::HandleProfilingStartCommand,             "" },
        { "stop",    SEC_SUPERADMIN,   true,  &ChatHandler::HandleProfilingStopCommand,              "" },
        { "status",  SEC_SUPERADMIN,   true,  &ChatHandler::HandleProfilingStatusCommand,            "" },
    };

    static std::vector<ChatCommand> debugCommandTable =
    {
        { "batchattack",    SEC_GAMEMASTER3,  false, &ChatHandler::HandleDebugBatchAttack,             "" },
        { "inarc",          SEC_GAMEMASTER3,  false, &ChatHandler::HandleDebugInArcCommand,            "" },
        { "spellfail",      SEC_GAMEMASTER3,  false, &ChatHandler::HandleDebugSpellFailCommand,        "" },
        { "setpoi",         SEC_GAMEMASTER3,  false, &ChatHandler::HandleDebugSetPoiCommand,           "" },
        { "qpartymsg",      SEC_GAMEMASTER3,  false, &ChatHandler::HandleDebugSendQuestPartyMsgCommand,"" },
        { "qinvalidmsg",    SEC_GAMEMASTER3,  false, &ChatHandler::HandleDebugSendQuestInvalidMsgCommand,"" },
        { "equiperr",       SEC_GAMEMASTER3,  false, &ChatHandler::HandleDebugEquipErrorCommand,       "" },
        { "sellerr",        SEC_GAMEMASTER3,  false, &ChatHandler::HandleDebugSellErrorCommand,        "" },
        { "buyerr",         SEC_GAMEMASTER3,  false, &ChatHandler::HandleDebugBuyErrorCommand,         "" },
        { "sendopcode",     SEC_GAMEMASTER3,  false, &ChatHandler::HandleDebugSendOpcodeCommand,       "" },
        { "uws",            SEC_GAMEMASTER3,  false, &ChatHandler::HandleUpdateWorldStateCommand,      "" },
        { "ps",             SEC_GAMEMASTER3,  false, &ChatHandler::HandleDebugPlaySound2Command,       "" },
        { "scn",            SEC_GAMEMASTER3,  false, &ChatHandler::HandleDebugSendChannelNotifyCommand,"" },
        { "scm",            SEC_GAMEMASTER3,  false, &ChatHandler::HandleDebugSendChatMsgCommand,      "" },
        { "getitemstate",   SEC_GAMEMASTER3,  false, &ChatHandler::HandleDebugGetItemState,            "" },
        { "playsound",      SEC_GAMEMASTER1,  false, &ChatHandler::HandleDebugPlaySoundCommand,        "" },
        { "update",         SEC_GAMEMASTER3,  false, &ChatHandler::HandleDebugUpdateCommand,           "" },
        { "setvalue",       SEC_ADMINISTRATOR,false, &ChatHandler::HandleDebugSetValueCommand,         "" },
        { "getvalue",       SEC_GAMEMASTER3,  false, &ChatHandler::HandleDebugGetValueCommand,         "" },
        { "anim",           SEC_GAMEMASTER2,  false, &ChatHandler::HandleDebugAnimCommand,             "" },
        { "lootrecipient",  SEC_GAMEMASTER2,  false, &ChatHandler::HandleDebugGetLootRecipient,        "" },
        { "arena",          SEC_GAMEMASTER3,  false, &ChatHandler::HandleDebugArenaCommand,            "" },
        { "bg",             SEC_GAMEMASTER3,  false, &ChatHandler::HandleDebugBattleGroundCommand,     "" },
        { "threatlist",     SEC_GAMEMASTER3,  false, &ChatHandler::HandleDebugThreatList,              "" },
        { "threatunitlist", SEC_GAMEMASTER3,  false, &ChatHandler::HandleDebugHostilRefList,           "" },
        { "cin",            SEC_GAMEMASTER3,  false, &ChatHandler::HandleDebugCinematic,               "" },
        { "getitembypos",   SEC_GAMEMASTER3,  false, &ChatHandler::HandleDebugItemByPos,               "" },
        { "getitemlevelsum",SEC_GAMEMASTER3,  false, &ChatHandler::HandleDebugItemLevelSum,            "" },
        { "removelootitem" ,SEC_GAMEMASTER3,  false, &ChatHandler::HandleDebugRemoveLootItem,          "" },
        { "pvpannounce",    SEC_GAMEMASTER3,  false, &ChatHandler::HandleDebugPvPAnnounce,             "" },
        { "auralist",       SEC_GAMEMASTER3,  false, &ChatHandler::HandleDebugAurasList,               "" },
        { "stealth",        SEC_GAMEMASTER3,  false, &ChatHandler::HandleDebugStealthLevel,            "" },
        { "detect",         SEC_GAMEMASTER3,  false, &ChatHandler::HandleDebugAttackDistance,          "" },
        { "unloadgrid",     SEC_GAMEMASTER3,  false, &ChatHandler::HandleDebugUnloadGrid,              "" },
        { "loadedscripts",  SEC_GAMEMASTER3,  true,  &ChatHandler::HandleDebugLoadedScripts,           "" },
        { "resetdaily",     SEC_GAMEMASTER3,  true,  &ChatHandler::HandleDebugResetDailyQuests,        "" },
        { "attackers",      SEC_GAMEMASTER2,  false, &ChatHandler::HandleDebugShowAttackers,           "" },
        { "zoneattack",     SEC_GAMEMASTER3,  false, &ChatHandler::HandleDebugSendZoneUnderAttack,     "" },
        { "los",            SEC_GAMEMASTER1,  false, &ChatHandler::HandleDebugLoSCommand,              "" },
        { "playerflags",    SEC_GAMEMASTER3,  false, &ChatHandler::HandleDebugPlayerFlags,             "" },
        { "opcodetest",     SEC_GAMEMASTER3,  false, &ChatHandler::HandleDebugOpcodeTestCommand,       "" },
        { "playemote",      SEC_GAMEMASTER2,  false, &ChatHandler::HandleDebugPlayEmoteCommand,        "" },
        { "mapheight",      SEC_GAMEMASTER3,  false, &ChatHandler::HandleDebugMapHeight,               "" },
        { "valuessnapshot", SEC_GAMEMASTER3,  false, &ChatHandler::HandleDebugValuesSnapshot,          "" },
        { "crash",          SEC_SUPERADMIN,   false, &ChatHandler::HandleDebugCrashCommand,            "" },
        { "setzonemusic",   SEC_SUPERADMIN,   false, &ChatHandler::HandleDebugZoneMusicCommand,        "" },
        { "setzonelight",   SEC_SUPERADMIN,   false, &ChatHandler::HandleDebugZoneLightCommand,        "" },
        { "setzoneweather", SEC_SUPERADMIN,   false, &ChatHandler::HandleDebugZoneWeatherCommand,      "" },
        { "setarmor",       SEC_GAMEMASTER3,  false, &ChatHandler::HandleDebugSetArmorCommand,         "" },
        { "getarmor",       SEC_GAMEMASTER3,  false, &ChatHandler::HandleDebugGetArmorCommand,         "" },
        { "spawnbatchobjects",SEC_SUPERADMIN, false, &ChatHandler::HandleSpawnBatchObjects,            "" },
        { "boundary",      SEC_GAMEMASTER3,   false, &ChatHandler::HandleDebugBoundaryCommand,         "" },
    };

    static std::vector<ChatCommand> eventCommandTable =
    {
        { "activelist",     SEC_GAMEMASTER2,     true, &ChatHandler::HandleEventActiveListCommand,     "" },
        { "start",          SEC_GAMEMASTER2,     true, &ChatHandler::HandleEventStartCommand,          "" },
        { "stop",           SEC_GAMEMASTER2,     true, &ChatHandler::HandleEventStopCommand,           "" },
        { "create",         SEC_GAMEMASTER2,     true, &ChatHandler::HandleEventCreateCommand,         "" },
        { "info",           SEC_GAMEMASTER2,     true, &ChatHandler::HandleEventInfoCommand,           "" },
    };

    static std::vector<ChatCommand> learnCommandTable =
    {
        { "all_gm",         SEC_GAMEMASTER2,  false,  &ChatHandler::HandleLearnAllGMCommand,          "" },
        { "all_crafts",     SEC_GAMEMASTER2,  false,  &ChatHandler::HandleLearnAllCraftsCommand,      "" },
        { "all_default",    SEC_GAMEMASTER1,  false,  &ChatHandler::HandleLearnAllDefaultCommand,     "" },
        { "all_lang",       SEC_GAMEMASTER1,  false,  &ChatHandler::HandleLearnAllLangCommand,        "" },
        { "all_myclass",    SEC_GAMEMASTER3,  false,  &ChatHandler::HandleLearnAllMyClassCommand,     "" },
        { "all_myproficiencies",SEC_GAMEMASTER3,false,&ChatHandler::HandleLearnAllMyProeficienciesCommand,"" },
        { "all_myspells",   SEC_GAMEMASTER3,  false,  &ChatHandler::HandleLearnAllMySpellsCommand,    "" },
        { "all_mytalents",  SEC_GAMEMASTER3,  false,  &ChatHandler::HandleLearnAllMyTalentsCommand,   "" },
        { "all_recipes",    SEC_GAMEMASTER2,  false,  &ChatHandler::HandleLearnAllRecipesCommand,     "" },
        { "",               SEC_GAMEMASTER3,  false,  &ChatHandler::HandleLearnCommand,               "" },
    };

    static std::vector<ChatCommand> reloadCommandTable =
    {
        { "access_requirement",          SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadAccessRequirementCommand,       "" },
        { "all",                         SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadAllCommand,                     "" },
        { "all_item",                    SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadAllItemCommand,                 "" },
        { "all_locales",                 SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadAllLocalesCommand,              "" },
        { "all_loot",                    SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadAllLootCommand,                 "" },
        { "all_npc",                     SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadAllNpcCommand,                  "" },
        { "all_quest",                   SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadAllQuestCommand,                "" },
        { "all_scripts",                 SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadAllScriptsCommand,              "" },
        { "all_spell",                   SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadAllSpellCommand,                "" },
        { "areatrigger_involvedrelation",SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadQuestAreaTriggersCommand,       "" },
        { "areatrigger_tavern",          SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadAreaTriggerTavernCommand,       "" },
        { "areatrigger_teleport",        SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadAreaTriggerTeleportCommand,     "" },
        { "auctions",                    SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadAuctionsCommand,                "" },
        { "auto_ann_by_time",            SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadAutoAnnounceCommand,            "" },
        { "command",                     SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadCommandCommand,                 "" },
        { "conditions",                  SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadConditions,                     "" },
        { "config",                      SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadConfigCommand,                  "" },
        { "creature_gossip",             SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadCreatureGossipCommand,          "" },
        { "creature_questender",         SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadCreatureQuestEndersCommand,     "" },
        { "creature_linked_respawn",     SEC_ADMINISTRATOR, true, &ChatHandler::HandleLinkedRespawnCommand,   "" },
        { "creature_loot_template",      SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadLootTemplatesCreatureCommand,   "" },
        { "creature_model_info",         SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadCreatureModelInfoCommand,       "" },
        { "creature_text",               SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadCreatureTextCommand,            "" },
        { "creature_template",           SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadCreatureTemplateCommand,        "" },
        { "disenchant_loot_template",    SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadLootTemplatesDisenchantCommand, "" },
        { "event_scripts",               SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadEventScriptsCommand,            "" },
        { "fishing_loot_template",       SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadLootTemplatesFishingCommand,    "" },
        { "game_event",                  SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadGameEventCommand,               "" },
        { "game_graveyard_zone",         SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadGameGraveyardZoneCommand,       "" },
        { "game_tele",                   SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadGameTeleCommand,                "" },
        { "gameobject_questender",       SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadGameobjectQuestEndersCommand,   "" },
        { "gameobject_loot_template",    SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadLootTemplatesGameobjectCommand, "" },
        { "gameobject_queststarter",     SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadGameobjectQuestStartersCommand, "" },
        { "gameobject_scripts",          SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadGameObjectScriptsCommand,       "" },
        { "gm_tickets",                  SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadGMTicketCommand,                "" },
        { "item_enchantment_template",   SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadItemEnchantementsCommand,       "" },
        { "item_extended_cost",          SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadItemExtendedCostCommand,        "" },
        { "item_loot_template",          SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadLootTemplatesItemCommand,       "" },
        { "locales_creature",            SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadLocalesCreatureCommand,         "" },
        { "locales_gameobject",          SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadLocalesGameobjectCommand,       "" },
        { "locales_item",                SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadLocalesItemCommand,             "" },
        { "locales_gossip_text",         SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadLocalesGossipTextCommand,       "" },
        { "locales_page_text",           SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadLocalesPageTextCommand,         "" },
        { "locales_quest",               SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadLocalesQuestCommand,            "" },
        { "gossip_menu",                 SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadGossipMenuCommand,              "" },
        { "gossip_menu_option",          SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadGossipMenuOptionCommand,        "" },
        { "gossip_text",                 SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadGossipTextCommand,              "" },
        { "npc_trainer",                 SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadNpcTrainerCommand,              "" },
        { "npc_vendor",                  SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadNpcVendorCommand,               "" },
        { "page_text",                   SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadPageTextsCommand,               "" },
        { "pickpocketing_loot_template", SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadLootTemplatesPickpocketingCommand,""},
        { "prospecting_loot_template",   SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadLootTemplatesProspectingCommand,"" },
        { "quest_end_scripts",           SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadQuestEndScriptsCommand,         "" },
        { "quest_mail_loot_template",    SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadLootTemplatesQuestMailCommand,  "" },
        { "quest_start_scripts",         SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadQuestStartScriptsCommand,       "" },
        { "quest_template",              SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadQuestTemplateCommand,           "" },
        { "reference_loot_template",     SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadLootTemplatesReferenceCommand,  "" },
        { "reserved_name",               SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadReservedNameCommand,            "" },
        { "skill_discovery_template",    SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadSkillDiscoveryTemplateCommand,  "" },
        { "skill_extra_item_template",   SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadSkillExtraItemTemplateCommand,  "" },
        { "skill_fishing_base_level",    SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadSkillFishingBaseLevelCommand,   "" },
        { "skinning_loot_template",      SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadLootTemplatesSkinningCommand,   "" },
        { "smart_scripts",               SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadSmartAICommand,                 "" },
        { "spell_affect",                SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadSpellAffectCommand,             "" },
        { "spell_area",                  SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadSpellAreaCommand,               "" },
        { "spell_bonus_data",            SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadSpellBonusesCommand,            "" },
        { "spell_disabled",              SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadSpellDisabledCommand,           "" },
        { "spell_elixir",                SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadSpellElixirCommand,             "" },
        { "spell_learn_spell",           SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadSpellLearnSpellCommand,         "" },
        { "spell_linked_spell",          SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadSpellLinkedSpellCommand,        "" },
        { "spell_pet_auras",             SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadSpellPetAurasCommand,           "" },
        { "spell_proc_event",            SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadSpellProcEventCommand,          "" },
        { "spell_required",              SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadSpellRequiredCommand,           "" },
        { "spell_scripts",               SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadSpellScriptsCommand,            "" },
        { "spell_target_position",       SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadSpellTargetPositionCommand,     "" },
        { "spell_template",              SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadSpellTemplates,                 "" },
        { "spell_threats",               SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadSpellThreatsCommand,            "" },
        { "trinity_string",              SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadTrinityStringCommand,           "" },
        { "waypoint_scripts",            SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadWpScriptsCommand,               "" },
        { "waypoints",                   SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadSmartWaypointsCommand,          "" },
        { "",                            SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadCommand,                        "" },
    };

    static std::vector<ChatCommand> honorCommandTable =
    {
        { "add",            SEC_GAMEMASTER2,     false, &ChatHandler::HandleAddHonorCommand,            "" },
        { "addkill",        SEC_GAMEMASTER2,     false, &ChatHandler::HandleHonorAddKillCommand,        "" },
        { "update",         SEC_GAMEMASTER2,     false, &ChatHandler::HandleUpdateHonorFieldsCommand,   "" },
    };

    static std::vector<ChatCommand> guildCommandTable =
    {
        { "create",         SEC_GAMEMASTER2,     true, &ChatHandler::HandleGuildCreateCommand,         "" },
        { "delete",         SEC_GAMEMASTER2,     true, &ChatHandler::HandleGuildDeleteCommand,         "" },
        { "invite",         SEC_GAMEMASTER2,     true, &ChatHandler::HandleGuildInviteCommand,         "" },
        { "uninvite",       SEC_GAMEMASTER2,     true, &ChatHandler::HandleGuildUninviteCommand,       "" },
        { "rank",           SEC_GAMEMASTER2,     true, &ChatHandler::HandleGuildRankCommand,           "" },
        { "rename",         SEC_GAMEMASTER2,     true, &ChatHandler::HandleGuildRenameCommand,         "" },
    };

    static std::vector<ChatCommand> petCommandTable =
    {
        { "create",         SEC_GAMEMASTER2,     false, &ChatHandler::HandleCreatePetCommand,           "" },
        { "learn",          SEC_GAMEMASTER2,     false, &ChatHandler::HandlePetLearnCommand,            "" },
        { "unlearn",        SEC_GAMEMASTER2,     false, &ChatHandler::HandlePetUnlearnCommand,          "" },
        { "tp",             SEC_GAMEMASTER2,     false, &ChatHandler::HandlePetTpCommand,               "" },
        { "rename",         SEC_GAMEMASTER2,     false, &ChatHandler::HandlePetRenameCommand,           "" },
    };


    static std::vector<ChatCommand> groupCommandTable =
    {
        { "leader",         SEC_GAMEMASTER3,     false, &ChatHandler::HandleGroupLeaderCommand,         "" },
        { "disband",        SEC_GAMEMASTER3,     false, &ChatHandler::HandleGroupDisbandCommand,        "" },
        { "remove",         SEC_GAMEMASTER3,     false, &ChatHandler::HandleGroupRemoveCommand,         "" },
    };

    static std::vector<ChatCommand> lookupPlayerCommandTable =
    {
        { "ip",             SEC_GAMEMASTER2,     true, &ChatHandler::HandleLookupPlayerIpCommand,       "" },
        { "account",        SEC_GAMEMASTER2,     true, &ChatHandler::HandleLookupPlayerAccountCommand,  "" },
        { "email",          SEC_GAMEMASTER2,     true, &ChatHandler::HandleLookupPlayerEmailCommand,    "" },
    };

    static std::vector<ChatCommand> lookupCommandTable =
    {
        { "area",           SEC_GAMEMASTER1,  true, &ChatHandler::HandleLookupAreaCommand,          "" },
        { "creature",       SEC_GAMEMASTER3,  true, &ChatHandler::HandleLookupCreatureCommand,      "" },
        { "event",          SEC_GAMEMASTER2,  true, &ChatHandler::HandleLookupEventCommand,         "" },
        { "faction",        SEC_GAMEMASTER3,  true, &ChatHandler::HandleLookupFactionCommand,       "" },
        { "item",           SEC_GAMEMASTER3,  true, &ChatHandler::HandleLookupItemCommand,          "" },
        { "itemset",        SEC_GAMEMASTER3,  true, &ChatHandler::HandleLookupItemSetCommand,       "" },
        { "object",         SEC_GAMEMASTER3,  true, &ChatHandler::HandleLookupObjectCommand,        "" },
        { "quest",          SEC_GAMEMASTER3,  true, &ChatHandler::HandleLookupQuestCommand,         "" },
        { "player",         SEC_GAMEMASTER2,  true, nullptr,                                           "", lookupPlayerCommandTable },
        { "skill",          SEC_GAMEMASTER3,  true, &ChatHandler::HandleLookupSkillCommand,         "" },
        { "spell",          SEC_GAMEMASTER3,  true, &ChatHandler::HandleGetSpellInfoCommand,        "" },
        { "tele",           SEC_GAMEMASTER1,  true, &ChatHandler::HandleLookupTeleCommand,          "" },
    };

    static std::vector<ChatCommand> resetCommandTable =
    {
        { "honor",          SEC_GAMEMASTER3,  false, &ChatHandler::HandleResetHonorCommand,          "" },
        { "level",          SEC_GAMEMASTER3,  false, &ChatHandler::HandleResetLevelCommand,          "" },
        { "spells",         SEC_GAMEMASTER3,  false, &ChatHandler::HandleResetSpellsCommand,         "" },
        { "stats",          SEC_GAMEMASTER3,  false, &ChatHandler::HandleResetStatsCommand,          "" },
        { "talents",        SEC_GAMEMASTER3,  false, &ChatHandler::HandleResetTalentsCommand,        "" },
        { "all",            SEC_GAMEMASTER3,  false, &ChatHandler::HandleResetAllCommand,            "" },
    };

    static std::vector<ChatCommand> castCommandTable =
    {
        { "back",           SEC_GAMEMASTER3,  false, &ChatHandler::HandleCastBackCommand,            "" },
        { "batch",          SEC_GAMEMASTER3,  false, &ChatHandler::HandleCastBatchCommand,           "" },
        { "dist",           SEC_GAMEMASTER3,  false, &ChatHandler::HandleCastDistCommand,            "" },
        { "self",           SEC_GAMEMASTER3,  false, &ChatHandler::HandleCastSelfCommand,            "" },
        { "target",         SEC_GAMEMASTER3,  false, &ChatHandler::HandleCastTargetCommand,          "" },
        { "",               SEC_GAMEMASTER3,  false, &ChatHandler::HandleCastCommand,                "" },
    };

    static std::vector<ChatCommand> pdumpCommandTable =
    {
        { "load",           SEC_GAMEMASTER3,  true, &ChatHandler::HandleLoadPDumpCommand,           "" },
        { "write",          SEC_GAMEMASTER3,  true, &ChatHandler::HandleWritePDumpCommand,          "" },
    };

    static std::vector<ChatCommand> listCommandTable =
    {
        { "creature",       SEC_GAMEMASTER3,  true,  &ChatHandler::HandleListCreatureCommand,        "" },
        { "item",           SEC_GAMEMASTER3,  true,  &ChatHandler::HandleListItemCommand,            "" },
        { "object",         SEC_GAMEMASTER3,  true,  &ChatHandler::HandleListObjectCommand,          "" },
        { "auras",          SEC_GAMEMASTER3,  false, &ChatHandler::HandleListAurasCommand,           "" },
    };

    static std::vector<ChatCommand> teleCommandTable =
    {
        { "add",            SEC_GAMEMASTER3,  false, &ChatHandler::HandleAddTeleCommand,             "" },
        { "del",            SEC_GAMEMASTER3,  true,  &ChatHandler::HandleDelTeleCommand,             "" },
        { "name",           SEC_GAMEMASTER1,  true,  &ChatHandler::HandleNameTeleCommand,            "" },
        { "group",          SEC_GAMEMASTER1,  false, &ChatHandler::HandleGroupTeleCommand,           "" },
        { "",               SEC_GAMEMASTER1,  false, &ChatHandler::HandleTeleCommand,                "" },
    };

    static std::vector<ChatCommand> npcPathTable =
    {
        { "type",           SEC_GAMEMASTER2,  false, &ChatHandler::HandleNpcPathTypeCommand,           "" },
        { "direction",      SEC_GAMEMASTER2,  false, &ChatHandler::HandleNpcPathDirectionCommand,      "" },
        { "currentid",      SEC_GAMEMASTER1,  false, &ChatHandler::HandleNpcPathCurrentIdCommand,      "" },
    };

    static std::vector<ChatCommand> npcCommandTable =
    {
        { "say",            SEC_GAMEMASTER1,     false, &ChatHandler::HandleNpcSayCommand,              "" },
        { "textemote",      SEC_GAMEMASTER1,     false, &ChatHandler::HandleNpcTextEmoteCommand,        "" },
        { "add",            SEC_GAMEMASTER2,     false, &ChatHandler::HandleNpcAddCommand,              "" },
        { "delete",         SEC_GAMEMASTER2,     false, &ChatHandler::HandleNpcDeleteCommand,           "" },
        { "spawndist",      SEC_GAMEMASTER2,     false, &ChatHandler::HandleNpcSpawnDistCommand,        "" },
        { "spawntime",      SEC_GAMEMASTER2,     false, &ChatHandler::HandleNpcSpawnTimeCommand,        "" },
        { "factionid",      SEC_GAMEMASTER2,     false, &ChatHandler::HandleNpcFactionIdCommand,        "" },
        { "setmovetype",    SEC_GAMEMASTER2,     false, &ChatHandler::HandleNpcSetMoveTypeCommand,      "" },
        { "move",           SEC_GAMEMASTER2,     false, &ChatHandler::HandleNpcMoveCommand,             "" },
        { "changelevel",    SEC_GAMEMASTER2,     false, &ChatHandler::HandleNpcChangeLevelCommand,      "" },
        { "setmodel",       SEC_GAMEMASTER2,     false, &ChatHandler::HandleNpcSetModelCommand,         "" },
        { "additem",        SEC_GAMEMASTER2,     false, &ChatHandler::HandleNpcAddVendorItemCommand,    "" },
        { "delitem",        SEC_GAMEMASTER2,     false, &ChatHandler::HandleNpcDelVendorItemCommand,    "" },
        { "flag",           SEC_GAMEMASTER2,     false, &ChatHandler::HandleNpcFlagCommand,             "" },
        { "changeentry",    SEC_GAMEMASTER3,     false, &ChatHandler::HandleNpcChangeEntryCommand,      "" },
        { "info",           SEC_GAMEMASTER3,     false, &ChatHandler::HandleNpcInfoCommand,             "" },
        { "setemotestate",  SEC_GAMEMASTER3,     false, &ChatHandler::HandleNpcSetEmoteStateCommand,    "" },
        { "follow",         SEC_GAMEMASTER2,     false, &ChatHandler::HandleNpcFollowCommand,           "" },
        { "unfollow",       SEC_GAMEMASTER2,     false, &ChatHandler::HandleNpcUnFollowCommand,         "" },
        { "whisper",        SEC_GAMEMASTER1,     false, &ChatHandler::HandleNpcWhisperCommand,          "" },
        { "yell",           SEC_GAMEMASTER1,     false, &ChatHandler::HandleNpcYellCommand,             "" },
        { "addtemp",        SEC_GAMEMASTER2,     false, &ChatHandler::HandleNpcTempAddCommand,          "" },
        { "addformation",   SEC_ADMINISTRATOR,   false, &ChatHandler::HandleNpcAddFormationCommand,     "" },
        { "removeformation",SEC_ADMINISTRATOR,   false, &ChatHandler::HandleNpcRemoveFormationCommand,  "" },
        { "setlink",        SEC_ADMINISTRATOR,   false, &ChatHandler::HandleNpcSetLinkCommand,          "" },
        { "gobackhome",     SEC_GAMEMASTER3,     false, &ChatHandler::HandleNpcGoBackHomeCommand,       "" },
        { "setpool",        SEC_GAMEMASTER3,     false, &ChatHandler::HandleNpcSetPoolCommand,          "" },
        { "guid",           SEC_GAMEMASTER1,     false, &ChatHandler::HandleNpcGuidCommand,             "" },
        { "addweapon",      SEC_GAMEMASTER3,     false, &ChatHandler::HandleNpcAddWeaponCommand,        "" },
        { "massfactionid",  SEC_ADMINISTRATOR,   false, &ChatHandler::HandleNpcMassFactionIdCommand,    "" },
        { "combatdistance", SEC_ADMINISTRATOR,   false, &ChatHandler::HandleNpcSetCombatDistanceCommand,"" },
        { "combatmovallow", SEC_ADMINISTRATOR,   false, &ChatHandler::HandleNpcAllowCombatMovementCommand,""},
        { "linkgameevent",  SEC_ADMINISTRATOR,   false, &ChatHandler::HandleNpcLinkGameEventCommand,    "" },
        { "unlinkgameevent",SEC_ADMINISTRATOR,   false, &ChatHandler::HandleNpcUnlinkGameEventCommand,  "" },
        { "goto",           SEC_GAMEMASTER3,     false, &ChatHandler::HandleNpcGotoCommand,             "" },
        { "fly",            SEC_GAMEMASTER3,     false, &ChatHandler::HandleNpcFlyCommand,              "" },
        { "near",           SEC_GAMEMASTER3,     false, &ChatHandler::HandleNpcNearCommand,             "" },
        { "name",           SEC_GAMEMASTER2,     false, &ChatHandler::HandleNpcNameCommand,                "" },

        //{ TODO: fix or remove this commands
        { "seteventid",     SEC_GAMEMASTER3,     false, &ChatHandler::HandleNpcSetInstanceEventCommand, "" },
        //}

        { "path",           SEC_GAMEMASTER1,     false, nullptr,                                        "", npcPathTable },
    };

    static std::vector<ChatCommand> goCommandTable =
    {
        { "grid",           SEC_GAMEMASTER1,     false, &ChatHandler::HandleGoGridCommand,              "" },
        { "creature",       SEC_GAMEMASTER2,     false, &ChatHandler::HandleGoCreatureCommand,          "" },
        { "object",         SEC_GAMEMASTER2,     false, &ChatHandler::HandleGoObjectCommand,            "" },
        { "ticket",         SEC_GAMEMASTER1,     false, &ChatHandler::HandleGoTicketCommand,            "" },
        { "trigger",        SEC_GAMEMASTER2,     false, &ChatHandler::HandleGoTriggerCommand,           "" },
        { "graveyard",      SEC_GAMEMASTER2,     false, &ChatHandler::HandleGoGraveyardCommand,         "" },
        { "zonexy",         SEC_GAMEMASTER1,     false, &ChatHandler::HandleGoZoneXYCommand,            "" },
        { "xy",             SEC_GAMEMASTER1,     false, &ChatHandler::HandleGoXYCommand,                "" },
        { "xyz",            SEC_GAMEMASTER1,     false, &ChatHandler::HandleGoXYZCommand,               "" },
        { "xyzo",           SEC_GAMEMASTER1,     false, &ChatHandler::HandleGoXYZOCommand,              "" },
        { "at",             SEC_GAMEMASTER3,     false, &ChatHandler::HandleGoATCommand,                "" },
        { "",               SEC_GAMEMASTER1,     false, &ChatHandler::HandleGoXYZCommand,               "" },
    };

    static std::vector<ChatCommand> gobjectCommandTable =
    {
        { "add",            SEC_GAMEMASTER3,     false, &ChatHandler::HandleGameObjectAddCommand,       "" },
        { "delete",         SEC_GAMEMASTER3,     false, &ChatHandler::HandleGameObjectDeleteCommand,    "" },
        { "target",         SEC_GAMEMASTER2,     false, &ChatHandler::HandleTargetObjectCommand,        "" },
        { "turn",           SEC_GAMEMASTER3,     false, &ChatHandler::HandleTurnObjectCommand,          "" },
        { "move",           SEC_GAMEMASTER3,     false, &ChatHandler::HandleMoveObjectCommand,          "" },
        { "near",           SEC_GAMEMASTER3,     false, &ChatHandler::HandleNearObjectCommand,          "" },
        { "activate",       SEC_GAMEMASTER2,     false, &ChatHandler::HandleActivateObjectCommand,      "" },
        { "addtemp",        SEC_GAMEMASTER3,     false, &ChatHandler::HandleTempGameObjectCommand,      "" },
        { "linkgameevent",  SEC_ADMINISTRATOR,   false, &ChatHandler::HandleGobLinkGameEventCommand,    "" },
        { "unlinkgameevent",SEC_ADMINISTRATOR,   false, &ChatHandler::HandleGobUnlinkGameEventCommand,  "" },
        { "getvalue",       SEC_GAMEMASTER3,     false, &ChatHandler::HandleGobGetValueCommand,         "" },
        { "setvalue",       SEC_GAMEMASTER3,     false, &ChatHandler::HandleGobSetValueCommand,         "" },
    };

    static std::vector<ChatCommand> questCommandTable =
    {
        { "add",            SEC_GAMEMASTER3,  false, &ChatHandler::HandleQuestAddCommand,                   "" },
        { "complete",       SEC_GAMEMASTER3,  false, &ChatHandler::HandleQuestCompleteCommand,              "" },
        { "countcomplete",  SEC_GAMEMASTER3,  false, &ChatHandler::HandleQuestCountCompleteCommand,         "" },
        { "totalcount",     SEC_GAMEMASTER3,  false, &ChatHandler::HandleQuestTotalCountCommand,            "" },
        { "remove",         SEC_GAMEMASTER3,  false, &ChatHandler::HandleQuestRemoveCommand,                "" },
        { "report",         SEC_GAMEMASTER2,  false, &ChatHandler::HandleQuestReportCommand,                "" },
    };

    static std::vector<ChatCommand> gmCommandTable =
    {
        { "chat",           SEC_GAMEMASTER1,    false, &ChatHandler::HandleGMChatCommand,              "" },
        { "ingame",         SEC_PLAYER,         true,  &ChatHandler::HandleGMListIngameCommand,        "" },
        { "list",           SEC_GAMEMASTER3,    true,  &ChatHandler::HandleGMListFullCommand,          "" },
        { "visible",        SEC_GAMEMASTER1,    false, &ChatHandler::HandleGMVisibleCommand,           "" },
        { "fly",            SEC_GAMEMASTER3,    false, &ChatHandler::HandleGMFlyModeCommand,           "" },
        { "stats",          SEC_GAMEMASTER1,    false, &ChatHandler::HandleGMStats,                    "" },
        { "",               SEC_GAMEMASTER1,    false, &ChatHandler::HandleGMmodeCommand,              "" },
    };

    static std::vector<ChatCommand> instanceCommandTable =
    {
        { "listbinds",      SEC_GAMEMASTER1,      false, &ChatHandler::HandleInstanceListBindsCommand,   "" },
        { "unbind",         SEC_GAMEMASTER1,      false, &ChatHandler::HandleInstanceUnbindCommand,      "" },
        { "stats",          SEC_GAMEMASTER1,      true,  &ChatHandler::HandleInstanceStatsCommand,       "" },
        { "savedata",       SEC_GAMEMASTER1,      false, &ChatHandler::HandleInstanceSaveDataCommand,    "" },
        { "setdata",        SEC_GAMEMASTER1,      false, &ChatHandler::HandleInstanceSetDataCommand,     "" },
        { "getdata",        SEC_GAMEMASTER1,      false, &ChatHandler::HandleInstanceGetDataCommand,     "" },
    };

    static std::vector<ChatCommand> replayCommandTable =
    {
        { "play",           SEC_ADMINISTRATOR,  false, &ChatHandler::HandleReplayPlayCommand,           "" },
        { "forward",        SEC_ADMINISTRATOR,  false, &ChatHandler::HandleReplayForwardCommand,        "" },
        { "stop",           SEC_ADMINISTRATOR,  false, &ChatHandler::HandleReplayStopCommand,           "" },
        { "record",         SEC_ADMINISTRATOR,  false, &ChatHandler::HandleReplayRecordCommand,         "" },
        { "speed",          SEC_ADMINISTRATOR,  false, &ChatHandler::HandleReplaySpeedCommand,          "" },
    };

    static std::vector<ChatCommand> ticketCommandTable =
    {
        { "list",           SEC_GAMEMASTER1,      false, &ChatHandler::HandleGMTicketListCommand,             "" },
        { "onlinelist",     SEC_GAMEMASTER1,      false, &ChatHandler::HandleGMTicketListOnlineCommand,       "" },
        { "viewname",       SEC_GAMEMASTER1,      false, &ChatHandler::HandleGMTicketGetByNameCommand,        "" },
        { "viewid",         SEC_GAMEMASTER1,      false, &ChatHandler::HandleGMTicketGetByIdCommand,          "" },
        { "close",          SEC_GAMEMASTER1,      false, &ChatHandler::HandleGMTicketCloseByIdCommand,        "" },
        { "closedlist",     SEC_GAMEMASTER1,      false, &ChatHandler::HandleGMTicketListClosedCommand,       "" },
        { "delete",         SEC_GAMEMASTER3,      false, &ChatHandler::HandleGMTicketDeleteByIdCommand,       "" },
        { "assign",         SEC_GAMEMASTER1,      false, &ChatHandler::HandleGMTicketAssignToCommand,         "" },
        { "unassign",       SEC_GAMEMASTER1,      false, &ChatHandler::HandleGMTicketUnAssignCommand,         "" },
        { "comment",        SEC_GAMEMASTER1,      false, &ChatHandler::HandleGMTicketCommentCommand,          "" },
    };

    static std::vector<ChatCommand> zoneCommandTable =
    {
        { "buff",           SEC_GAMEMASTER3,  false, &ChatHandler::HandleZoneBuffCommand,                 "" },
        { "morph",          SEC_GAMEMASTER3,  false, &ChatHandler::HandleZoneMorphCommand,                "" },
    };

    static std::vector<ChatCommand> spectateCommandTable =
    {
        { "version",       SEC_PLAYER,      false, &ChatHandler::HandleSpectateVersion,                 "" },
        { "watch",         SEC_PLAYER,      false, &ChatHandler::HandleSpectateFromCommand,             "" },
        { "leave",         SEC_PLAYER,      false, &ChatHandler::HandleSpectateCancelCommand,           "" },
        { "init",          SEC_PLAYER,      false, &ChatHandler::HandleSpectateInitCommand,             "" },
    };

    static std::vector<ChatCommand> smartAICommandTable =
    {
        { "debug",         SEC_GAMEMASTER2, true,  &ChatHandler::HandleSmartAIDebugCommand,             "" },
        { "errors",        SEC_GAMEMASTER2, true,  &ChatHandler::HandleSmartAIShowErrorsCommand,        "" },
    };

   static std::vector<ChatCommand> cheatCommandTable =
    {
        { "god",            SEC_GAMEMASTER2,  false, &ChatHandler::HandleGodModeCheatCommand,            "" },
        { "casttime",       SEC_GAMEMASTER3,  false, &ChatHandler::HandleCasttimeCheatCommand,           "" },
        { "cooldown",       SEC_GAMEMASTER3,  false, &ChatHandler::HandleCoolDownCheatCommand,           "" },
        { "power",          SEC_GAMEMASTER3,  false, &ChatHandler::HandlePowerCheatCommand,              "" },
        { "taxi",           SEC_GAMEMASTER3,  false, &ChatHandler::HandleTaxiCheatCommand,               "" },
        { "explore",        SEC_GAMEMASTER3,  false, &ChatHandler::HandleExploreCheatCommand,            "" },
        { "waterwalk",      SEC_GAMEMASTER3,  false, &ChatHandler::HandleWaterwalkCheatCommand,          "" },
    };

   static std::vector<ChatCommand> testCommandTable = 
   {
       { "start",          SEC_ADMINISTRATOR, true,  &ChatHandler::HandleTestsStartCommand,                 "" },
       { "list",           SEC_ADMINISTRATOR, true,  &ChatHandler::HandleTestsListCommand,                  "" },
       { "running",        SEC_ADMINISTRATOR, true,  &ChatHandler::HandleTestsRunningCommand,               "" },
       { "cancel",         SEC_ADMINISTRATOR, true,  &ChatHandler::HandleTestsCancelCommand,                "" },
       { "go",             SEC_ADMINISTRATOR, false, &ChatHandler::HandleTestsGoCommand,                    "" },
   };

   /**
    * The values here may be overwritten by the database and are here as defaults.
    */
    static std::vector<ChatCommand> commandTable =
    {
        { "account",        SEC_PLAYER,       true,  nullptr,                                        "", accountCommandTable },
        { "additem",        SEC_GAMEMASTER3,  false, &ChatHandler::HandleAddItemCommand,             "" },
        { "additemset",     SEC_GAMEMASTER3,  false, &ChatHandler::HandleAddItemSetCommand,          "" },
        { "announce",       SEC_GAMEMASTER1,  true,  &ChatHandler::HandleAnnounceCommand,            "" },
        { "arenarename",    SEC_GAMEMASTER2,  true,  &ChatHandler::HandleRenameArenaTeamCommand,     "" },
        { "aura",           SEC_GAMEMASTER3,  false, &ChatHandler::HandleAuraCommand,                "" },
        { "ban",            SEC_GAMEMASTER3,  true,  nullptr,                                        "", banCommandTable },
        { "baninfo",        SEC_GAMEMASTER3,  false, nullptr,                                        "", baninfoCommandTable },
        { "bank",           SEC_GAMEMASTER3,  false, &ChatHandler::HandleBankCommand,                "" },
        { "banlist",        SEC_GAMEMASTER3,  true,  nullptr,                                        "", banlistCommandTable },
        { "bg",             SEC_PLAYER,       false, &ChatHandler::HandleBattlegroundCommand,        "" },
        { "bindsight",      SEC_GAMEMASTER3,  false, &ChatHandler::HandleBindSightCommand,           "" },
        { "blink",          SEC_GAMEMASTER1,  false, &ChatHandler::HandleBlinkCommand,               "" },
        { "boutique",       SEC_PLAYER,       false, &ChatHandler::HandleBuyInShopCommand,           "" },
        { "cast",           SEC_GAMEMASTER3,  false, nullptr,                                        "", castCommandTable },
        { "chanban",        SEC_GAMEMASTER1,  true,  &ChatHandler::HandleChanBan,                    "" },
        { "chaninfoban",    SEC_GAMEMASTER1,  true,  &ChatHandler::HandleChanInfoBan,                "" },
        { "chanunban",      SEC_GAMEMASTER1,  true,  &ChatHandler::HandleChanUnban,                  "" },
        { "chardelete",     SEC_ADMINISTRATOR,true,  &ChatHandler::HandleCharacterDeleteCommand,     "" },
        { "cheat",          SEC_GAMEMASTER2,  false, nullptr,                                        "", cheatCommandTable },
        { "combatstop",     SEC_GAMEMASTER2,  false, &ChatHandler::HandleCombatStopCommand,          "" },
        { "cometome",       SEC_GAMEMASTER3,  false, &ChatHandler::HandleComeToMeCommand,            "" },
        { "commands",       SEC_PLAYER,       true,  &ChatHandler::HandleCommandsCommand,            "" },
        { "cooldown",       SEC_GAMEMASTER3,  false, &ChatHandler::HandleCooldownCommand,            "" },
        { "copystuff",      SEC_GAMEMASTER2,  false, &ChatHandler::HandleCopyStuffCommand,           "" },
        { "credits",        SEC_PLAYER,       false, &ChatHandler::HandleViewCreditsCommand,         "" },
        { "damage",         SEC_GAMEMASTER3,  false, &ChatHandler::HandleDamageCommand,              "" },
        { "debug",          SEC_GAMEMASTER1,  false, nullptr,                                        "", debugCommandTable },
        { "demorph",        SEC_GAMEMASTER2,  false, &ChatHandler::HandleDeMorphCommand,             "" },
        { "die",            SEC_GAMEMASTER3,  false, &ChatHandler::HandleDieCommand,                 "" },
        { "dismount",       SEC_PLAYER,       false, &ChatHandler::HandleDismountCommand,            "" },
        { "distance",       SEC_GAMEMASTER3,  false, &ChatHandler::HandleGetDistanceCommand,         "" },
        { "event",          SEC_GAMEMASTER2,  false,  nullptr,                                        "", eventCommandTable },
        { "changefaction",  SEC_PLAYER,       false, &ChatHandler::HandleRaceOrFactionChange,        "" },
        { "flusharenapoints",SEC_GAMEMASTER3, false, &ChatHandler::HandleFlushArenaPointsCommand,    "" },
        { "freeze",         SEC_GAMEMASTER3,  false, &ChatHandler::HandleFreezeCommand,              "" },
        { "getmoveflags",   SEC_GAMEMASTER2,  false, &ChatHandler::HandleGetMoveFlagsCommand,        "" },
        { "gm",             SEC_GAMEMASTER1,  true,  nullptr,                                        "", gmCommandTable },
        { "gmannounce",     SEC_GAMEMASTER1,  true,  &ChatHandler::HandleGMAnnounceCommand,          "" },
        { "gmnotify",       SEC_GAMEMASTER1,  true,  &ChatHandler::HandleGMNotifyCommand,            "" },
        { "go",             SEC_GAMEMASTER1,  false, nullptr,                                        "", goCommandTable },
        { "gobject",        SEC_GAMEMASTER2,  false, nullptr,                                        "", gobjectCommandTable },
        { "goname",         SEC_GAMEMASTER1,  false, &ChatHandler::HandleGonameCommand,              "" },
        { "gps",            SEC_GAMEMASTER1,  true,  &ChatHandler::HandleGPSCommand,                 "" },
        { "gpss",           SEC_GAMEMASTER1,  false, &ChatHandler::HandleGPSSCommand,                "" },
        { "groupgo",        SEC_GAMEMASTER1,  false, &ChatHandler::HandleGroupgoCommand,             "" },
        { "guid",           SEC_GAMEMASTER2,  false, &ChatHandler::HandleGUIDCommand,                "" },
        { "guild",          SEC_GAMEMASTER3,  true,  nullptr,                                        "", guildCommandTable },
        { "help",           SEC_PLAYER,       true,  &ChatHandler::HandleHelpCommand,                "" },
        { "heroday",        SEC_PLAYER,       true,  &ChatHandler::HandleHerodayCommand,             "" },
        { "hidearea",       SEC_GAMEMASTER3,  false, &ChatHandler::HandleHideAreaCommand,            "" },
        { "honor",          SEC_GAMEMASTER2,  false, nullptr,                                        "", honorCommandTable },
        { "hover",          SEC_GAMEMASTER3,  false, &ChatHandler::HandleHoverCommand,               "" },
        { "instance",       SEC_GAMEMASTER3,  true,  nullptr,                                        "", instanceCommandTable },
        { "itemmove",       SEC_GAMEMASTER2,  false, &ChatHandler::HandleItemMoveCommand,            "" },
        { "kick",           SEC_GAMEMASTER2,  true,  &ChatHandler::HandleKickPlayerCommand,          "" },
        { "learn",          SEC_GAMEMASTER1,  false, nullptr,                                        "", learnCommandTable },
        { "levelup",        SEC_GAMEMASTER3,  false, &ChatHandler::HandleLevelUpCommand,             "" },
        { "linkgrave",      SEC_GAMEMASTER3,  false, &ChatHandler::HandleLinkGraveCommand,           "" },
        { "list",           SEC_GAMEMASTER3,  true,  nullptr,                                        "", listCommandTable },
        { "listfreeze",     SEC_GAMEMASTER3,  false, &ChatHandler::HandleListFreezeCommand,          "" },
        { "loadpath",       SEC_GAMEMASTER3,  false, &ChatHandler::HandleReloadAllPaths,             "" },
        { "lockaccount",    SEC_PLAYER,       false, &ChatHandler::HandleLockAccountCommand,         "" },
        { "lookup",         SEC_GAMEMASTER3,  true,  nullptr,                                        "", lookupCommandTable },
        { "maxpool",        SEC_GAMEMASTER3,  true,  &ChatHandler::HandleGetMaxCreaturePoolIdCommand,"" },
        { "maxskill",       SEC_GAMEMASTER3,  false, &ChatHandler::HandleMaxSkillCommand,            "" },
        { "mmap",           SEC_GAMEMASTER2,  false, nullptr,                                        "", mmapCommandTable },
        { "modify",         SEC_GAMEMASTER1,  false, nullptr,                                        "", modifyCommandTable },
        { "movegens",       SEC_GAMEMASTER3,  false, &ChatHandler::HandleMovegensCommand,            "" },
        { "mute",           SEC_GAMEMASTER2,  true,  &ChatHandler::HandleMuteCommand,                "" },
        { "muteinfo",       SEC_GAMEMASTER3,  false, nullptr,                                        "", muteinfoCommandTable },
        { "namego",         SEC_GAMEMASTER1,  false, &ChatHandler::HandleNamegoCommand,              "" },
        { "neargrave",      SEC_GAMEMASTER3,  false, &ChatHandler::HandleNearGraveCommand,           "" },
        { "notify",         SEC_GAMEMASTER1,  true,  &ChatHandler::HandleNotifyCommand,              "" },
        { "npc",            SEC_GAMEMASTER1,  false, nullptr,                                        "", npcCommandTable },
        { "password",       SEC_PLAYER,       false, &ChatHandler::HandlePasswordCommand,            "" },
        { "path",           SEC_GAMEMASTER2,  false, nullptr,                                        "", wpCommandTable },
        { "pdump",          SEC_GAMEMASTER3,  true,  nullptr,                                        "", pdumpCommandTable },
        { "pet",            SEC_GAMEMASTER2,  false, nullptr,                                        "", petCommandTable },
        { "pinfo",          SEC_GAMEMASTER2,  true,  &ChatHandler::HandlePInfoCommand,               "" },
        { "playall",        SEC_GAMEMASTER3,  false, &ChatHandler::HandlePlayAllCommand,             "" },
        { "plimit",         SEC_GAMEMASTER3,  true,  &ChatHandler::HandlePLimitCommand,              "" },
        { "possess",        SEC_GAMEMASTER3,  false, &ChatHandler::HandlePossessCommand,             "" },
        { "profiling",      SEC_SUPERADMIN,   false, nullptr,                                        "", profilingCommandTable },
        { "quest",          SEC_GAMEMASTER3,  false, nullptr,                                        "", questCommandTable },
        { "recall",         SEC_GAMEMASTER1,  false, &ChatHandler::HandleRecallCommand,              "" },
        { "recup",          SEC_GAMEMASTER3,  false, &ChatHandler::HandleRecupCommand,               "" },
        { "reload",         SEC_ADMINISTRATOR,true,  nullptr,                                        "", reloadCommandTable },
        { "removetitle"    ,SEC_GAMEMASTER3,  false, &ChatHandler::HandleRemoveTitleCommand,         "" },
        { "rename",         SEC_GAMEMASTER2,  true,  &ChatHandler::HandleRenameCommand,              "" },
        { "repairitems",    SEC_GAMEMASTER2,  false, &ChatHandler::HandleRepairitemsCommand,         "" },
        { "replay",         SEC_ADMINISTRATOR,false, nullptr,                                        "", replayCommandTable },
        { "reportlag",      SEC_PLAYER,       false, &ChatHandler::HandleReportLagCommand,           "" },
        { "reset",          SEC_GAMEMASTER3,  false, nullptr,                                        "", resetCommandTable },
        { "reskin",         SEC_PLAYER,       false, &ChatHandler::HandleReskinCommand,              "" },
        { "respawn",        SEC_GAMEMASTER3,  false, &ChatHandler::HandleRespawnCommand,             "" },
        { "revive",         SEC_GAMEMASTER3,  false, &ChatHandler::HandleReviveCommand,              "" },
        { "save",           SEC_PLAYER,       false, &ChatHandler::HandleSaveCommand,                "" },
        { "saveall",        SEC_GAMEMASTER1,  true,  &ChatHandler::HandleSaveAllCommand,             "" },
        { "senditems",      SEC_GAMEMASTER3,  true,  &ChatHandler::HandleSendItemsCommand,           "" },
        { "sendmail",       SEC_GAMEMASTER1,  true,  &ChatHandler::HandleSendMailCommand,            "" },
        { "sendmessage",    SEC_GAMEMASTER3,  true,  &ChatHandler::HandleSendMessageCommand,         "" },
        { "sendmoney",      SEC_GAMEMASTER3,  true,  &ChatHandler::HandleSendMoneyCommand,           "" },
        { "server",         SEC_GAMEMASTER3,  true,  nullptr,                                        "", serverCommandTable },
        { "setmoveflags",   SEC_GAMEMASTER2,  false, &ChatHandler::HandleSetMoveFlagsCommand,        "" },
        { "setskill",       SEC_GAMEMASTER3,  false, &ChatHandler::HandleSetSkillCommand,            "" },
        { "settitle"       ,SEC_GAMEMASTER3,  false, &ChatHandler::HandleSetTitleCommand,            "" },
        { "showarea",       SEC_GAMEMASTER3,  false, &ChatHandler::HandleShowAreaCommand,            "" },
        { "spectator",      SEC_PLAYER,       false, nullptr,                                        "", spectateCommandTable },
        { "spellinfo",      SEC_GAMEMASTER3,  true,  &ChatHandler::HandleSpellInfoCommand,           "" },
        { "smartai",        SEC_GAMEMASTER3,  true,  nullptr,                                        "", smartAICommandTable },
        { "start",          SEC_PLAYER,       false, &ChatHandler::HandleStartCommand,               "" },
        { "tele",           SEC_GAMEMASTER1,  true,  nullptr,                                        "", teleCommandTable },
        { "tests",          SEC_SUPERADMIN,   true,  nullptr,                                        "", testCommandTable },
        { "ticket",         SEC_GAMEMASTER1,  false, nullptr,                                        "", ticketCommandTable },
        { "unaura",         SEC_GAMEMASTER3,  false, &ChatHandler::HandleUnAuraCommand,              "" },
        { "unban",          SEC_GAMEMASTER3,  true,  nullptr,                                        "", unbanCommandTable },
        { "unbindsight",    SEC_GAMEMASTER3,  false, &ChatHandler::HandleUnbindSightCommand,         "" },
        { "unfreeze",       SEC_GAMEMASTER3,  false, &ChatHandler::HandleUnFreezeCommand,            "" },
        { "unlearn",        SEC_GAMEMASTER3,  false, &ChatHandler::HandleUnLearnCommand,             "" },
        { "unmute",         SEC_GAMEMASTER2,  true,  &ChatHandler::HandleUnmuteCommand,              "" },
        { "unpossess",      SEC_GAMEMASTER3,  false, &ChatHandler::HandleUnPossessCommand,           "" },
        { "updatetitle",    SEC_PLAYER,       false, &ChatHandler::HandleUpdatePvPTitleCommand,      "" },
        { "wchange",        SEC_GAMEMASTER3,  false, &ChatHandler::HandleChangeWeather,              "" },
        { "whispers",       SEC_GAMEMASTER1,  false, &ChatHandler::HandleWhispersCommand,            "" },
        { "yolo",           SEC_GAMEMASTER1,  true,  &ChatHandler::HandleYoloCommand,                "" },
        { "zone",           SEC_GAMEMASTER3,  false, nullptr,                                        "", zoneCommandTable },
        //PLAYERBOT
        { "rndbot",         SEC_SUPERADMIN,   true,  &ChatHandler::HandlePlayerbotConsoleCommand,    "" },
        { "bot",            SEC_SUPERADMIN,   true,  &ChatHandler::HandlePlayerbotMgrCommand,        "" },
    };

    /**
     * Values in database take precedence over default values in command table
     */
    if(load_command_table)
    {
        load_command_table = false;

        QueryResult result = WorldDatabase.Query("SELECT name, security, help FROM command");
        if (result)
        {
            do
            {
                Field* fields = result->Fetch();
                std::string fullName = fields[0].GetString();

                SetDataForCommandInTable(commandTable, fullName.c_str(), fields[1].GetUInt8(), fields[2].GetString(), fullName);

            } while(result->NextRow());
        }
    }

    return commandTable;
}

ChatHandler::ChatHandler(WorldSession* session)
    : m_session(session)
{}

ChatHandler::ChatHandler(Player* player)
    : m_session(player->GetSession())
{}

std::string ChatHandler::GetNameLink(Player* chr) const
{
    return playerLink(chr->GetName());
}

void ChatHandler::SendMessageWithoutAuthor(char const* channel, const char* msg)
{
    WorldPacket data;
    ChatHandler::BuildChatPacket(data, CHAT_MSG_CHANNEL, LANG_UNIVERSAL, nullptr, nullptr, msg, 0, channel);

    boost::shared_lock<boost::shared_mutex> lock(*HashMapHolder<Player>::GetLock());
    HashMapHolder<Player>::MapType const& m = ObjectAccessor::GetPlayers();
    for(auto const & itr : m)
    {
        if (itr.second && itr.second->GetSession()->GetPlayer() && itr.second->GetSession()->GetPlayer()->IsInWorld())
        {
            if(ChannelMgr* cMgr = channelMgr(itr.second->GetSession()->GetPlayer()->GetTeam()))
            {
                if(Channel *chn = cMgr->GetChannel(channel, itr.second->GetSession()->GetPlayer()))
                {
                    itr.second->SendDirectMessage(&data);
                }
            }
        }
    }
}

void ChatHandler::invalidateCommandTable()
{
    //TC commandTableCache.reset();
    load_command_table = true;
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
            for (const auto & j : table)
            {
                if (!hasStringAbbr(j.Name, cmd.c_str()))
                    continue;

                if (strcmp(j.Name, cmd.c_str()) == 0)
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
    for (const auto & i : table)
    {
        // must be available (ignore handler existence for show command with possibe avalable subcomands
        if(!isAvailable(i))
            continue;

        // for empty subcmd show all available
        if( *subcmd && !hasStringAbbr(i.Name, subcmd))
            continue;

        if(m_session)
            list += "\n    ";
        else
            list += "\n\r    ";

        list += i.Name;

        if(!i.ChildCommands.empty())
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
        for (const auto & i : table)
        {
            // must be available (ignore handler existence for show command with possibe avalable subcomands
            if(!isAvailable(i))
                continue;

            if( !hasStringAbbr(i.Name, cmd) )
                continue;

            // have subcommand
            char const* subcmd = (*cmd) ? strtok(nullptr, " ") : "";

            if(!i.ChildCommands.empty() && subcmd && *subcmd)
            {
                if(ShowHelpForCommand(i.ChildCommands, subcmd))
                    return true;
            }

            if(!i.Help.empty())
                SendSysMessage(i.Help.c_str());

            if(!i.ChildCommands.empty())
                if(ShowHelpForSubCommands(i.ChildCommands,i.Name,subcmd ? subcmd : ""))
                    return true;

            return !i.Help.empty();
        }
    }
    else
    {
        for (const auto & i : table)
        {
            // must be available (ignore handler existence for show command with possibe avalable subcomands
            if(!isAvailable(i))
                continue;

            if(strlen(i.Name))
                continue;

            if(!i.Help.empty())
                SendSysMessage(i.Help.c_str());

            if(!i.ChildCommands.empty())
                if(ShowHelpForSubCommands(i.ChildCommands,"",""))
                    return true;

            return !i.Help.empty();
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

    char* cLinkType = strtok(nullptr, ":");                    // linktype
    if(!cLinkType)
        return nullptr;                                        // end of data

    if(strcmp(cLinkType,linkType) != 0)
    {
        strtok(nullptr, " ");                                  // skip link tail (to allow continue strtok(NULL,s) use after retturn from function
        SendSysMessage(LANG_WRONG_LINK_TYPE);
        return nullptr;
    }

    char* cKeys = strtok(nullptr, "|");                        // extract keys and values
    char* cKeysTail = strtok(nullptr, "");

    char* cKey = strtok(cKeys, ":|");                       // extract key
    if(something1)
        *something1 = strtok(nullptr, ":|");                   // extract something

    strtok(cKeysTail, "]");                                 // restart scan tail and skip name with possible spaces
    strtok(nullptr, " ");                                      // skip link tail (to allow continue strtok(NULL,s) use after retturn from function
    return cKey;
}

char* ChatHandler::extractKeyFromLink(char* text, char const* const* linkTypes, int* found_idx, char** something1)
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

    char* cLinkType = strtok(nullptr, ":");                    // linktype
    if(!cLinkType)
        return nullptr;                                        // end of data

    for(int i = 0; linkTypes[i]; ++i)
    {
        if(strcmp(cLinkType,linkTypes[i]) == 0)
        {
            char* cKeys = strtok(nullptr, "|");                // extract keys and values
            char* cKeysTail = strtok(nullptr, "");

            char* cKey = strtok(cKeys, ":|");               // extract key
            if(something1)
                *something1 = strtok(nullptr, ":|");           // extract something

            strtok(cKeysTail, "]");                         // restart scan tail and skip name with possible spaces
            strtok(nullptr, " ");                              // skip link tail (to allow continue strtok(NULL,s) use after return from function
            if(found_idx)
                *found_idx = i;
            return cKey;
        }
    }

    strtok(nullptr, " ");                                      // skip link tail (to allow continue strtok(NULL,s) use after return from function
    SendSysMessage(LANG_WRONG_LINK_TYPE);
    return nullptr;
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

GameObject* ChatHandler::GetNearbyGameObject()
{
    if (!m_session)
        return nullptr;

    Player* pl = m_session->GetPlayer();
    GameObject* obj = nullptr;
    Trinity::NearestGameObjectCheck check(*pl);
    Trinity::GameObjectLastSearcher<Trinity::NearestGameObjectCheck> searcher(pl, obj, check);
    Cell::VisitGridObjects(pl, searcher, SIZE_OF_GRIDS);
    return obj;
}

GameObject* ChatHandler::GetObjectFromPlayerMapByDbGuid(ObjectGuid::LowType lowguid)
{
    if (!m_session)
        return nullptr;
    auto bounds = m_session->GetPlayer()->GetMap()->GetGameObjectBySpawnIdStore().equal_range(lowguid);
    if (bounds.first != bounds.second)
        return bounds.first->second;
    return nullptr;
}

Creature* ChatHandler::GetCreatureFromPlayerMapByDbGuid(ObjectGuid::LowType lowguid)
{
    if (!m_session)
        return nullptr;
    // Select the first alive creature or a dead one if not found
    Creature* creature = nullptr;
    auto bounds = m_session->GetPlayer()->GetMap()->GetCreatureBySpawnIdStore().equal_range(lowguid);
    for (auto it = bounds.first; it != bounds.second; ++it)
    {
        creature = it->second;
        if (it->second->IsAlive())
            break;
    }
    return creature;
}

static char const* const spellTalentKeys[] = {
    "Hspell",
    "Htalent",
    nullptr
};

uint32 ChatHandler::extractSpellIdFromLink(char* text)
{
    // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r
    // number or [name] Shift-click form |color|Htalent:talent_id,rank|h[name]|h|r
    int type = 0;
    char* rankS = nullptr;
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
    plr  = nullptr;
    guid = 0;

    if(cname)
    {
        std::string name = cname;
        if(!name.empty())
        {
            if(!normalizePlayerName(name))
            {
                SendSysMessage(LANG_PLAYER_NOT_FOUND);
                SetSentErrorMessage(true);
                return false;
            }

            plr = ObjectAccessor::FindConnectedPlayerByName(name.c_str());
            if(offline)
                guid = sCharacterCache->GetCharacterGuidByName(name.c_str());
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

        Player* pl = ObjectAccessor::FindPlayerByName(name.c_str());

        // if allowed player pointer
        if (player)
            *player = pl;

        // if need guid value from DB (in name case for check player existence)
        uint64 guid = !pl && (player_guid || player_name) ? sCharacterCache->GetCharacterGuidByName(name) : 0;

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
