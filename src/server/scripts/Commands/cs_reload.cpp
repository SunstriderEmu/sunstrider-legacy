#include "Chat.h"
#include "Language.h"
#include "CreatureTextMgr.h"
#include "GameEventMgr.h"
#include "AuctionHouseMgr.h"
#include "SmartScriptMgr.h"
#include "SkillExtraItems.h"
#include "SkillDiscovery.h"
#include "CharacterCache.h"
#include "ItemEnchantmentMgr.h"
#include "WaypointDefines.h"
#include "MapManager.h"

class reload_commandscript : public CommandScript
{
public:
    reload_commandscript() : CommandScript("reload_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> reloadCommandTable =
        {
            { "access_requirement",          SEC_ADMINISTRATOR, true, &HandleReloadAccessRequirementCommand,       "" },
            { "all",                         SEC_ADMINISTRATOR, true, &HandleReloadAllCommand,                     "" },
            { "all_item",                    SEC_ADMINISTRATOR, true, &HandleReloadAllItemCommand,                 "" },
            { "all_locales",                 SEC_ADMINISTRATOR, true, &HandleReloadAllLocalesCommand,              "" },
            { "all_loot",                    SEC_ADMINISTRATOR, true, &HandleReloadAllLootCommand,                 "" },
            { "all_npc",                     SEC_ADMINISTRATOR, true, &HandleReloadAllNpcCommand,                  "" },
            { "all_quest",                   SEC_ADMINISTRATOR, true, &HandleReloadAllQuestCommand,                "" },
            { "all_scripts",                 SEC_ADMINISTRATOR, true, &HandleReloadAllScriptsCommand,              "" },
            { "all_spell",                   SEC_ADMINISTRATOR, true, &HandleReloadAllSpellCommand,                "" },
            { "areatrigger_involvedrelation",SEC_ADMINISTRATOR, true, &HandleReloadQuestAreaTriggersCommand,       "" },
            { "areatrigger_tavern",          SEC_ADMINISTRATOR, true, &HandleReloadAreaTriggerTavernCommand,       "" },
            { "areatrigger_teleport",        SEC_ADMINISTRATOR, true, &HandleReloadAreaTriggerTeleportCommand,     "" },
            { "auctions",                    SEC_ADMINISTRATOR, true, &HandleReloadAuctionsCommand,                "" },
            { "auto_ann_by_time",            SEC_ADMINISTRATOR, true, &HandleReloadAutoAnnounceCommand,            "" },
            { "command",                     SEC_ADMINISTRATOR, true, &HandleReloadCommandCommand,                 "" },
            { "conditions",                  SEC_ADMINISTRATOR, true, &HandleReloadConditions,                     "" },
            { "config",                      SEC_ADMINISTRATOR, true, &HandleReloadConfigCommand,                  "" },
            { "creature_gossip",             SEC_ADMINISTRATOR, true, &HandleReloadCreatureGossipCommand,          "" },
            { "creature_questender",         SEC_ADMINISTRATOR, true, &HandleReloadCreatureQuestEndersCommand,     "" },
            { "creature_loot_template",      SEC_ADMINISTRATOR, true, &HandleReloadLootTemplatesCreatureCommand,   "" },
            { "creature_movement_override",  SEC_ADMINISTRATOR, true, &HandleReloadCreatureMovementOverrideCommand,"" },
            { "creature_model_info",         SEC_ADMINISTRATOR, true, &HandleReloadCreatureModelInfoCommand,       "" },
            { "creature_text",               SEC_ADMINISTRATOR, true, &HandleReloadCreatureTextCommand,            "" },
            { "creature_template",           SEC_ADMINISTRATOR, true, &HandleReloadCreatureTemplateCommand,        "" },
            { "disenchant_loot_template",    SEC_ADMINISTRATOR, true, &HandleReloadLootTemplatesDisenchantCommand, "" },
            { "event_scripts",               SEC_ADMINISTRATOR, true, &HandleReloadEventScriptsCommand,            "" },
            { "fishing_loot_template",       SEC_ADMINISTRATOR, true, &HandleReloadLootTemplatesFishingCommand,    "" },
            { "game_event",                  SEC_ADMINISTRATOR, true, &HandleReloadGameEventCommand,               "" },
            { "game_graveyard_zone",         SEC_ADMINISTRATOR, true, &HandleReloadGameGraveyardZoneCommand,       "" },
            { "game_tele",                   SEC_ADMINISTRATOR, true, &HandleReloadGameTeleCommand,                "" },
            { "gameobject_questender",       SEC_ADMINISTRATOR, true, &HandleReloadGameobjectQuestEndersCommand,   "" },
            { "gameobject_loot_template",    SEC_ADMINISTRATOR, true, &HandleReloadLootTemplatesGameobjectCommand, "" },
            { "gameobject_queststarter",     SEC_ADMINISTRATOR, true, &HandleReloadGameobjectQuestStartersCommand, "" },
            { "gameobject_scripts",          SEC_ADMINISTRATOR, true, &HandleReloadGameObjectScriptsCommand,       "" },
            { "gm_tickets",                  SEC_ADMINISTRATOR, true, &HandleReloadGMTicketCommand,                "" },
            { "item_enchantment_template",   SEC_ADMINISTRATOR, true, &HandleReloadItemEnchantementsCommand,       "" },
            { "item_extended_cost",          SEC_ADMINISTRATOR, true, &HandleReloadItemExtendedCostCommand,        "" },
            { "item_loot_template",          SEC_ADMINISTRATOR, true, &HandleReloadLootTemplatesItemCommand,       "" },
            { "locales_creature",            SEC_ADMINISTRATOR, true, &HandleReloadLocalesCreatureCommand,         "" },
            { "locales_gameobject",          SEC_ADMINISTRATOR, true, &HandleReloadLocalesGameobjectCommand,       "" },
            { "locales_item",                SEC_ADMINISTRATOR, true, &HandleReloadLocalesItemCommand,             "" },
            { "locales_gossip_text",         SEC_ADMINISTRATOR, true, &HandleReloadLocalesGossipTextCommand,       "" },
            { "locales_page_text",           SEC_ADMINISTRATOR, true, &HandleReloadLocalesPageTextCommand,         "" },
            { "locales_quest",               SEC_ADMINISTRATOR, true, &HandleReloadLocalesQuestCommand,            "" },
            { "gossip_menu",                 SEC_ADMINISTRATOR, true, &HandleReloadGossipMenuCommand,              "" },
            { "gossip_menu_option",          SEC_ADMINISTRATOR, true, &HandleReloadGossipMenuOptionCommand,        "" },
            { "gossip_text",                 SEC_ADMINISTRATOR, true, &HandleReloadGossipTextCommand,              "" },
            { "npc_trainer",                 SEC_ADMINISTRATOR, true, &HandleReloadNpcTrainerCommand,              "" },
            { "npc_vendor",                  SEC_ADMINISTRATOR, true, &HandleReloadNpcVendorCommand,               "" },
            { "page_text",                   SEC_ADMINISTRATOR, true, &HandleReloadPageTextsCommand,               "" },
            { "pickpocketing_loot_template", SEC_ADMINISTRATOR, true, &HandleReloadLootTemplatesPickpocketingCommand,"" },
            { "prospecting_loot_template",   SEC_ADMINISTRATOR, true, &HandleReloadLootTemplatesProspectingCommand,"" },
            { "quest_end_scripts",           SEC_ADMINISTRATOR, true, &HandleReloadQuestEndScriptsCommand,         "" },
            { "mail_loot_template",          SEC_ADMINISTRATOR, true, &HandleReloadLootTemplatesMailCommand,       "" },
            { "quest_start_scripts",         SEC_ADMINISTRATOR, true, &HandleReloadQuestStartScriptsCommand,       "" },
            { "quest_template",              SEC_ADMINISTRATOR, true, &HandleReloadQuestTemplateCommand,           "" },
            { "reference_loot_template",     SEC_ADMINISTRATOR, true, &HandleReloadLootTemplatesReferenceCommand,  "" },
            { "reserved_name",               SEC_ADMINISTRATOR, true, &HandleReloadReservedNameCommand,            "" },
            { "skill_discovery_template",    SEC_ADMINISTRATOR, true, &HandleReloadSkillDiscoveryTemplateCommand,  "" },
            { "skill_extra_item_template",   SEC_ADMINISTRATOR, true, &HandleReloadSkillExtraItemTemplateCommand,  "" },
            { "skill_fishing_base_level",    SEC_ADMINISTRATOR, true, &HandleReloadSkillFishingBaseLevelCommand,   "" },
            { "skinning_loot_template",      SEC_ADMINISTRATOR, true, &HandleReloadLootTemplatesSkinningCommand,   "" },
            { "smart_scripts",               SEC_ADMINISTRATOR, true, &HandleReloadSmartAICommand,                 "" },
            { "spell_affect",                SEC_ADMINISTRATOR, true, &HandleReloadSpellAffectCommand,             "" },
            { "spell_area",                  SEC_ADMINISTRATOR, true, &HandleReloadSpellAreaCommand,               "" },
            { "spell_bonus_data",            SEC_ADMINISTRATOR, true, &HandleReloadSpellBonusesCommand,            "" },
            { "spell_disabled",              SEC_ADMINISTRATOR, true, &HandleReloadSpellDisabledCommand,           "" },
            { "spell_elixir",                SEC_ADMINISTRATOR, true, &HandleReloadSpellElixirCommand,             "" },
            { "spell_group",                 SEC_ADMINISTRATOR, true, &HandleReloadSpellGroupsCommand,             "" },
            { "spell_group_stack_rules",     SEC_ADMINISTRATOR, true, &HandleReloadSpellGroupStackRulesCommand,    "" },
            { "spell_learn_spell",           SEC_ADMINISTRATOR, true, &HandleReloadSpellLearnSpellCommand,         "" },
            { "spell_linked_spell",          SEC_ADMINISTRATOR, true, &HandleReloadSpellLinkedSpellCommand,        "" },
            { "spell_pet_auras",             SEC_ADMINISTRATOR, true, &HandleReloadSpellPetAurasCommand,           "" },
            { "spell_proc",                  SEC_ADMINISTRATOR, true, &HandleReloadSpellProcCommand,               "" },
            { "spell_required",              SEC_ADMINISTRATOR, true, &HandleReloadSpellRequiredCommand,           "" },
            { "spell_scripts",               SEC_ADMINISTRATOR, true, &HandleReloadSpellScriptsCommand,            "" },
            { "spell_target_position",       SEC_ADMINISTRATOR, true, &HandleReloadSpellTargetPositionCommand,     "" },
            { "spell_template",              SEC_ADMINISTRATOR, true, &HandleReloadSpellTemplates,                 "" },
            { "spell_threats",               SEC_ADMINISTRATOR, true, &HandleReloadSpellThreatsCommand,            "" },
            { "trinity_string",              SEC_ADMINISTRATOR, true, &HandleReloadTrinityStringCommand,           "" },
            { "waypoint_scripts",            SEC_ADMINISTRATOR, true, &HandleReloadWpScriptsCommand,               "" },
            { "waypoints",                   SEC_ADMINISTRATOR, true, &HandleReloadSmartWaypointsCommand,          "" },
            { "",                            SEC_ADMINISTRATOR, true, &HandleReloadCommand,                        "" },
        };
        static std::vector<ChatCommand> commandTable =
        {
            { "reload",         SEC_ADMINISTRATOR,  true,  nullptr,                                "", reloadCommandTable },
        };
        return commandTable;
    }

    //reload commands
    static bool HandleReloadCommand(ChatHandler* handler, char const* args)
    {
        // this is error catcher for wrong table name in .reload commands
        handler->PSendSysMessage("Db table with name starting from '%s' not found and can't be reloaded.", args);
        handler->SetSentErrorMessage(true);
        return false;
    }

    static bool HandleReloadCreatureTextCommand(ChatHandler* handler, char const* /*args*/)
    {
        TC_LOG_INFO("command", "Re-Loading Creature Texts...");
        sCreatureTextMgr->LoadCreatureTexts();
        sCreatureTextMgr->LoadCreatureTextLocales();
        handler->SendGlobalGMSysMessage("Creature Texts reloaded.");
        return true;
    }

    static bool HandleReloadCreatureTemplateCommand(ChatHandler* handler, char const* /*args*/)
    {
        TC_LOG_INFO("command", "Re-Loading Creature Templates...");
        sObjectMgr->LoadCreatureTemplates(true);
        handler->SendGlobalGMSysMessage("Creature Templates reloaded.");
        return true;
    }

    static bool HandleReloadAllCommand(ChatHandler* handler, char const* /*args*/)
    {
        HandleReloadAreaTriggerTeleportCommand(handler, "");
        HandleReloadSkillFishingBaseLevelCommand(handler, "");

        HandleReloadAllAreaCommand(handler, "");
        HandleReloadAllLootCommand(handler, "");
        HandleReloadAllNpcCommand(handler, "");
        HandleReloadAllQuestCommand(handler, "");
        HandleReloadAllSpellCommand(handler, "");
        HandleReloadAllItemCommand(handler, "");
        HandleReloadAllLocalesCommand(handler, "");

        HandleReloadCommandCommand(handler, "");
        HandleReloadReservedNameCommand(handler, "");
        HandleReloadTrinityStringCommand(handler, "");
        HandleReloadGameTeleCommand(handler, "");
        return true;
    }

    static bool HandleReloadAllAreaCommand(ChatHandler* handler, char const* /*args*/)
    {
        //HandleReloadQuestAreaTriggersCommand(handler, ""); -- reloaded in HandleReloadAllQuestCommand
        HandleReloadAreaTriggerTeleportCommand(handler, "");
        HandleReloadAreaTriggerTavernCommand(handler, "");
        HandleReloadGameGraveyardZoneCommand(handler, "");
        return true;
    }

    static bool HandleReloadAllLootCommand(ChatHandler* handler, char const* /*args*/)
    {
        TC_LOG_INFO("command", "Re-Loading Loot Tables...");
        LoadLootTables();
        handler->SendGlobalGMSysMessage("DB tables `*_loot_template` reloaded.");
        return true;
    }

    static bool HandleReloadAllNpcCommand(ChatHandler* handler, char const* /*args*/)
    {
        HandleReloadCreatureGossipCommand(handler, "a");
        HandleReloadGossipMenuOptionCommand(handler, "a");
        HandleReloadNpcTrainerCommand(handler, "a");
        HandleReloadNpcVendorCommand(handler, "a");
        return true;
    }

    static bool HandleReloadAllQuestCommand(ChatHandler* handler, char const* /*args*/)
    {
        HandleReloadQuestAreaTriggersCommand(handler, "a");
        HandleReloadQuestTemplateCommand(handler, "a");

        TC_LOG_INFO("command", "Re-Loading Quests Relations...");
        sObjectMgr->LoadQuestRelations();
        handler->SendGlobalGMSysMessage("DB tables `*_questrelation` and `*_involvedrelation` reloaded.");
        return true;
    }

    static bool HandleReloadAllScriptsCommand(ChatHandler* handler, char const* /*args*/)
    {
        if (sMapMgr->IsScriptScheduled())
        {
            handler->PSendSysMessage("DB scripts used currently, please attempt reload later.");
            handler->SetSentErrorMessage(true);
            return false;
        }

        TC_LOG_INFO("command", "Re-Loading Scripts...");
        HandleReloadGameObjectScriptsCommand(handler, "a");
        HandleReloadEventScriptsCommand(handler, "a");
        HandleReloadQuestEndScriptsCommand(handler, "a");
        HandleReloadQuestStartScriptsCommand(handler, "a");
        HandleReloadSpellScriptsCommand(handler, "a");
        handler->SendGlobalGMSysMessage("DB tables `*_scripts` reloaded.");
        HandleReloadWpScriptsCommand(handler, "a");
        return true;
    }

    static bool HandleReloadAllSpellCommand(ChatHandler* handler, char const* /*args*/)
    {
        HandleReloadSkillDiscoveryTemplateCommand(handler, "a");
        HandleReloadSkillExtraItemTemplateCommand(handler, "a");
        HandleReloadSpellAffectCommand(handler, "a");
        HandleReloadSpellGroupsCommand(handler, "a");
        HandleReloadSpellRequiredCommand(handler, "a");
        HandleReloadSpellElixirCommand(handler, "a");
        HandleReloadSpellLearnSpellCommand(handler, "a");
        HandleReloadSpellLinkedSpellCommand(handler, "a");
        HandleReloadSpellProcCommand(handler, "a");
        HandleReloadSpellTargetPositionCommand(handler, "a");
        HandleReloadSpellThreatsCommand(handler, "a");
        HandleReloadSpellPetAurasCommand(handler, "a");
        HandleReloadSpellGroupStackRulesCommand(handler, "a");
        HandleReloadSpellDisabledCommand(handler, "a");
        return true;
    }

    static bool HandleReloadAllItemCommand(ChatHandler* handler, char const* /*args*/)
    {
        HandleReloadPageTextsCommand(handler, "a");
        HandleReloadItemEnchantementsCommand(handler, "a");
        return true;
    }

    static bool HandleReloadAllLocalesCommand(ChatHandler* handler, char const* /*args*/)
    {
        HandleReloadLocalesCreatureCommand(handler, "a");
        HandleReloadLocalesGameobjectCommand(handler, "a");
        HandleReloadLocalesItemCommand(handler, "a");
        HandleReloadLocalesGossipTextCommand(handler, "a");
        HandleReloadLocalesPageTextCommand(handler, "a");
        HandleReloadLocalesQuestCommand(handler, "a");
        return true;
    }

    static bool HandleReloadConfigCommand(ChatHandler* handler, char const* /*args*/)
    {
        TC_LOG_INFO("command", "Re-Loading config settings...");
        sWorld->LoadConfigSettings(true);
        sMapMgr->InitializeVisibilityDistanceInfo();
        handler->SendGlobalGMSysMessage("World config settings reloaded.");
        return true;
    }

    static bool HandleReloadAreaTriggerTavernCommand(ChatHandler* handler, char const* /*args*/)
    {
        TC_LOG_INFO("command", "Re-Loading Tavern Area Triggers...");
        sObjectMgr->LoadTavernAreaTriggers();
        handler->SendGlobalGMSysMessage("DB table `areatrigger_tavern` reloaded.");
        return true;
    }

    static bool HandleReloadAreaTriggerTeleportCommand(ChatHandler* handler, char const* /*args*/)
    {
        TC_LOG_INFO("command", "Re-Loading AreaTrigger teleport definitions...");
        sObjectMgr->LoadAreaTriggerTeleports();
        handler->SendGlobalGMSysMessage("DB table `areatrigger_teleport` reloaded.");
        return true;
    }

    static bool HandleReloadAccessRequirementCommand(ChatHandler* handler, char const* /*args*/)
    {
        TC_LOG_INFO("command", "Re-Loading Access Requirement definitions...");
        sObjectMgr->LoadAccessRequirements();
        handler->SendGlobalGMSysMessage("DB table `access_requirement` reloaded.");
        return true;
    }

    static bool HandleReloadCommandCommand(ChatHandler* handler, char const* /*args*/)
    {
        ChatHandler::invalidateCommandTable();
        handler->SendGlobalGMSysMessage("DB table `command` will be reloaded at next chat command use.");
        return true;
    }

    static bool HandleReloadCreatureQuestStartersCommand(ChatHandler* handler, char const* /*args*/)
    {
        TC_LOG_INFO("command", "Loading Quests Relations... (`creature_queststarter`)");
        sObjectMgr->LoadCreatureQuestStarters();
        handler->SendGlobalGMSysMessage("DB table `creature_queststarter` (creature quest givers) reloaded.");
        return true;
    }

    static bool HandleReloadCreatureQuestEndersCommand(ChatHandler* handler, char const* /*args*/)
    {
        TC_LOG_INFO("command", "Loading Quests Relations... (`creature_questender`)");
        sObjectMgr->LoadCreatureQuestEnders();
        handler->SendGlobalGMSysMessage("DB table `creature_questender` (creature quest takers) reloaded.");
        return true;
    }

    static bool HandleReloadGameobjectQuestStartersCommand(ChatHandler* handler, char const* /*args*/)
    {
        TC_LOG_INFO("command", "Loading Quests Relations... (`gameobject_queststarter`)");
        sObjectMgr->LoadGameobjectQuestStarters();
        handler->SendGlobalGMSysMessage("DB table `gameobject_queststarter` (gameobject quest givers) reloaded.");
        return true;
    }

    static bool HandleReloadGameobjectQuestEndersCommand(ChatHandler* handler, char const* /*args*/)
    {
        TC_LOG_INFO("command", "Loading Quests Relations... (`gameobject_questender`)");
        sObjectMgr->LoadGameobjectQuestEnders();
        handler->SendGlobalGMSysMessage("DB table `gameobject_questender` (gameobject quest takers) reloaded.");
        return true;
    }

    static bool HandleReloadQuestAreaTriggersCommand(ChatHandler* handler, char const* /*args*/)
    {
        TC_LOG_INFO("command", "Re-Loading Quest Area Triggers...");
        sObjectMgr->LoadQuestAreaTriggers();
        handler->SendGlobalGMSysMessage("DB table `areatrigger_involvedrelation` (quest area triggers) reloaded.");
        return true;
    }

    static bool HandleReloadQuestTemplateCommand(ChatHandler* handler, char const* /*args*/)
    {
        TC_LOG_INFO("command", "Re-Loading Quest Templates...");
        sObjectMgr->LoadQuests();
        handler->SendGlobalGMSysMessage("DB table `quest_template` (quest definitions) reloaded.");
        return true;
    }

    static bool HandleReloadCreatureModelInfoCommand(ChatHandler* handler, char const* /*args*/)
    {
        TC_LOG_INFO("command", "Re-Loading `creature_model_info`");
        sObjectMgr->LoadCreatureModelInfo();
        handler->SendGlobalGMSysMessage("DB table `creature_model_info` reloaded.");
        return true;
    }

    static bool HandleReloadLootTemplatesCreatureCommand(ChatHandler* handler, char const* /*args*/)
    {
        TC_LOG_INFO("command", "Re-Loading Loot Tables... (`creature_loot_template`)");
        LoadLootTemplates_Creature();
        LootTemplates_Creature.CheckLootRefs();
        handler->SendGlobalGMSysMessage("DB table `creature_loot_template` reloaded.");
        return true;
    }

    static bool HandleReloadCreatureMovementOverrideCommand(ChatHandler* handler, char const* /*args*/)
    {
        TC_LOG_INFO("misc", "Re-Loading Creature movement overrides...");
        sObjectMgr->LoadCreatureMovementOverrides();
        handler->SendGlobalGMSysMessage("DB table `creature_movement_override` reloaded.");
        return true;
    }

    static bool HandleReloadLootTemplatesDisenchantCommand(ChatHandler* handler, char const* /*args*/)
    {
        TC_LOG_INFO("command", "Re-Loading Loot Tables... (`disenchant_loot_template`)");
        LoadLootTemplates_Disenchant();
        LootTemplates_Disenchant.CheckLootRefs();
        handler->SendGlobalGMSysMessage("DB table `disenchant_loot_template` reloaded.");
        return true;
    }

    static bool HandleReloadLootTemplatesFishingCommand(ChatHandler* handler, char const* /*args*/)
    {
        TC_LOG_INFO("command", "Re-Loading Loot Tables... (`fishing_loot_template`)");
        LoadLootTemplates_Fishing();
        LootTemplates_Fishing.CheckLootRefs();
        handler->SendGlobalGMSysMessage("DB table `fishing_loot_template` reloaded.");
        return true;
    }

    static bool HandleReloadLootTemplatesGameobjectCommand(ChatHandler* handler, char const* /*args*/)
    {
        TC_LOG_INFO("command", "Re-Loading Loot Tables... (`gameobject_loot_template`)");
        LoadLootTemplates_Gameobject();
        LootTemplates_Gameobject.CheckLootRefs();
        handler->SendGlobalGMSysMessage("DB table `gameobject_loot_template` reloaded.");
        return true;
    }

    static bool HandleReloadLootTemplatesItemCommand(ChatHandler* handler, char const* /*args*/)
    {
        TC_LOG_INFO("command", "Re-Loading Loot Tables... (`item_loot_template`)");
        LoadLootTemplates_Item();
        LootTemplates_Item.CheckLootRefs();
        handler->SendGlobalGMSysMessage("DB table `item_loot_template` reloaded.");
        return true;
    }

    static bool HandleReloadLootTemplatesPickpocketingCommand(ChatHandler* handler, char const* /*args*/)
    {
        TC_LOG_INFO("command", "Re-Loading Loot Tables... (`pickpocketing_loot_template`)");
        LoadLootTemplates_Pickpocketing();
        LootTemplates_Pickpocketing.CheckLootRefs();
        handler->SendGlobalGMSysMessage("DB table `pickpocketing_loot_template` reloaded.");
        return true;
    }

    static bool HandleReloadLootTemplatesProspectingCommand(ChatHandler* handler, char const* /*args*/)
    {
        TC_LOG_INFO("command", "Re-Loading Loot Tables... (`prospecting_loot_template`)");
        LoadLootTemplates_Prospecting();
        LootTemplates_Prospecting.CheckLootRefs();
        handler->SendGlobalGMSysMessage("DB table `prospecting_loot_template` reloaded.");
        return true;
    }

    static bool HandleReloadLootTemplatesMailCommand(ChatHandler* handler, char const* /*args*/)
    {
        TC_LOG_INFO("misc", "Re-Loading Loot Tables... (`mail_loot_template`)");
        LoadLootTemplates_Mail();
        LootTemplates_Mail.CheckLootRefs();
        handler->SendGlobalGMSysMessage("DB table `mail_loot_template` reloaded.");
        sConditionMgr->LoadConditions(true);
        return true;
    }

    static bool HandleReloadLootTemplatesReferenceCommand(ChatHandler* handler, char const* /*args*/)
    {
        TC_LOG_INFO("command", "Re-Loading Loot Tables... (`reference_loot_template`)");
        LoadLootTemplates_Reference();
        handler->SendGlobalGMSysMessage("DB table `reference_loot_template` reloaded.");
        return true;
    }

    static bool HandleReloadLootTemplatesSkinningCommand(ChatHandler* handler, char const* /*args*/)
    {
        TC_LOG_INFO("command", "Re-Loading Loot Tables... (`skinning_loot_template`)");
        LoadLootTemplates_Skinning();
        LootTemplates_Skinning.CheckLootRefs();
        handler->SendGlobalGMSysMessage("DB table `skinning_loot_template` reloaded.");
        return true;
    }

    static bool HandleReloadTrinityStringCommand(ChatHandler* handler, char const* /*args*/)
    {
        TC_LOG_INFO("command", "Re-Loading trinity_string Table!");
        sObjectMgr->LoadTrinityStrings();
        handler->SendGlobalGMSysMessage("DB table `trinity_string` reloaded.");
        return true;
    }

    static bool HandleReloadGossipMenuCommand(ChatHandler* handler, char const* args)
    {
        TC_LOG_INFO("command", "Re-Loading `gossip_menu` Table!");
        sObjectMgr->LoadGossipMenu();
        handler->SendGlobalGMSysMessage("DB table `gossip_menu` reloaded.");
        return true;
    }

    static bool HandleReloadGossipTextCommand(ChatHandler* handler, char const* args)
    {
        TC_LOG_INFO("command", "Re-Loading `gossip_text` Table!");
        sObjectMgr->LoadGossipText();
        sObjectMgr->LoadGossipTextLocales();
        handler->SendGlobalGMSysMessage("DB tables `gossip_text` and `locales_gossip_text` reloaded.");
        return true;
    }

    static bool HandleReloadCreatureGossipCommand(ChatHandler* handler, char const* args)
    {
        TC_LOG_INFO("command", "Re-Loading `creature_gossip` Table!");
        sObjectMgr->LoadCreatureGossip();
        handler->SendGlobalGMSysMessage("DB table `creature_gossip` reloaded.");
        return true;
    }

    static bool HandleReloadGossipMenuOptionCommand(ChatHandler* handler, char const* /*args*/)
    {
        TC_LOG_INFO("command", "Re-Loading `gossip_menu_option` Table!");
        sObjectMgr->LoadGossipMenuItems();
        sObjectMgr->LoadGossipMenuItemsLocales();
        handler->SendGlobalGMSysMessage("DB tables `gossip_menu_option` and `locales_gossip_menu_option` reloaded.");
        return true;
    }

    static bool HandleReloadNpcTrainerCommand(ChatHandler* handler, char const* /*args*/)
    {
        TC_LOG_INFO("command", "Re-Loading `npc_trainer` Table!");
        sObjectMgr->LoadTrainerSpell();
        sGameEventMgr->LoadTrainers();
        handler->SendGlobalGMSysMessage("DB table `npc_trainer` reloaded.");
        return true;
    }

    static bool HandleReloadNpcVendorCommand(ChatHandler* handler, char const* /*args*/)
    {
        TC_LOG_INFO("command", "Re-Loading `npc_vendor` Table!");
        sObjectMgr->LoadVendors();
        sGameEventMgr->LoadVendors();
        handler->SendGlobalGMSysMessage("DB table `npc_vendor` reloaded.");
        return true;
    }

    static bool HandleReloadReservedNameCommand(ChatHandler* handler, char const* /*args*/)
    {
        TC_LOG_INFO("server.loading", "Loading ReservedNames... (`reserved_name`)");
        sObjectMgr->LoadReservedPlayersNames();
        handler->SendGlobalGMSysMessage("DB table `reserved_name` (player reserved names) reloaded.");
        return true;
    }

    static bool HandleReloadSkillDiscoveryTemplateCommand(ChatHandler* handler, char const* /*args*/)
    {
        TC_LOG_INFO("server.loading", "Re-Loading Skill Discovery Table...");
        LoadSkillDiscoveryTable();
        handler->SendGlobalGMSysMessage("DB table `skill_discovery_template` (recipes discovered at crafting) reloaded.");
        return true;
    }

    static bool HandleReloadSkillExtraItemTemplateCommand(ChatHandler* handler, char const* /*args*/)
    {
        TC_LOG_INFO("server.loading", "Re-Loading Skill Extra Item Table...");
        LoadSkillExtraItemTable();
        handler->SendGlobalGMSysMessage("DB table `skill_extra_item_template` (extra item creation when crafting) reloaded.");
        return true;
    }

    static bool HandleReloadSkillFishingBaseLevelCommand(ChatHandler* handler, char const* /*args*/)
    {
        TC_LOG_INFO("command", "Re-Loading Skill Fishing base level requirements...");
        sObjectMgr->LoadFishingBaseSkillLevel();
        handler->SendGlobalGMSysMessage("DB table `skill_fishing_base_level` (fishing base level for zone/subzone) reloaded.");
        return true;
    }

    static bool HandleReloadSpellBonusesCommand(ChatHandler* handler, char const* /*args*/)
    {
        TC_LOG_INFO("command", "Re-Loading Spell Bonuses definitions...");
        sSpellMgr->LoadSpellBonuses();
        handler->SendGlobalGMSysMessage("DB table `spell_bonus_data` reloaded.");
        return true;
    }

    static bool HandleReloadSpellAffectCommand(ChatHandler* handler, char const* /*args*/)
    {
        TC_LOG_INFO("command", "Re-Loading SpellAffect definitions...");
        sSpellMgr->LoadSpellAffects(true);
        handler->SendGlobalGMSysMessage("DB table `spell_affect` (spell mods apply requirements) reloaded.");
        return true;
    }

    static bool HandleReloadSpellAreaCommand(ChatHandler* handler, char const* /*args*/)
    {
        TC_LOG_INFO("command", "Re-Loading spell_area...");
        sSpellMgr->LoadSpellAreas();
        handler->SendGlobalGMSysMessage("DB table `spell_area` reloaded.");
        return true;
    }

    static bool HandleReloadSpellRequiredCommand(ChatHandler* handler, char const* /*args*/)
    {
        TC_LOG_INFO("command", "Re-Loading Spell Required Data... ");
        sSpellMgr->LoadSpellRequired();
        handler->SendGlobalGMSysMessage("DB table `spell_required` reloaded.");
        return true;
    }

    static bool HandleReloadSpellElixirCommand(ChatHandler* handler, char const* args)
    {
        TC_LOG_INFO("command", "Re-Loading Spell Elixir types...");
        sSpellMgr->LoadSpellElixirs();
        handler->SendGlobalGMSysMessage("DB table `spell_elixir` (spell elixir types) reloaded.");
        return true;
    }

    static bool HandleReloadSpellGroupsCommand(ChatHandler* handler, char const* args)
    {
        TC_LOG_INFO("misc", "Re-Loading Spell Groups...");
        sSpellMgr->LoadSpellGroups();
        handler->SendGlobalGMSysMessage("DB table `spell_group` (spell groups) reloaded.");
        return true;
    }

    static bool HandleReloadSpellGroupStackRulesCommand(ChatHandler* handler, char const* args)
    {
        TC_LOG_INFO("misc", "Re-Loading Spell Group Stack Rules...");
        sSpellMgr->LoadSpellGroupStackRules();
        handler->SendGlobalGMSysMessage("DB table `spell_group_stack_rules` (spell stacking definitions) reloaded.");
        return true;
    }

    static bool HandleReloadSpellLearnSpellCommand(ChatHandler* handler, char const* /*args*/)
    {
        TC_LOG_INFO("command", "Re-Loading Spell Learn Spells...");
        sSpellMgr->LoadSpellLearnSpells();
        handler->SendGlobalGMSysMessage("DB table `spell_learn_spell` reloaded.");
        return true;
    }

    static bool HandleReloadSpellLinkedSpellCommand(ChatHandler* handler, char const* /*args*/)
    {
        TC_LOG_INFO("command", "Re-Loading Spell Linked Spells...");
        sSpellMgr->LoadSpellLinked();
        handler->SendGlobalGMSysMessage("DB table `spell_linked_spell` reloaded.");
        return true;
    }

    static bool HandleReloadSpellProcCommand(ChatHandler* handler, char const* /*args*/)
    {
        TC_LOG_INFO("command", "Re-Loading Spell Proc Event conditions...");
        sSpellMgr->LoadSpellProcs();
        handler->SendGlobalGMSysMessage("DB table `spell_proc` (spell proc trigger requirements) reloaded.");
        return true;
    }

    static bool HandleReloadSpellTargetPositionCommand(ChatHandler* handler, char const* /*args*/)
    {
        TC_LOG_INFO("command", "Re-Loading Spell target coordinates...");
        sSpellMgr->LoadSpellTargetPositions();
        handler->SendGlobalGMSysMessage("DB table `spell_target_position` (destination coordinates for spell targets) reloaded.");
        return true;
    }

    static bool HandleReloadSpellThreatsCommand(ChatHandler* handler, char const* /*args*/)
    {
        TC_LOG_INFO("command", "Re-Loading Aggro Spells Definitions...");
        sSpellMgr->LoadSpellThreats();
        handler->SendGlobalGMSysMessage("DB table `spell_threat` (spell aggro definitions) reloaded.");
        return true;
    }

    static bool HandleReloadSpellPetAurasCommand(ChatHandler* handler, char const* /*args*/)
    {
        TC_LOG_INFO("command", "Re-Loading Spell pet auras...");
        sSpellMgr->LoadSpellPetAuras();
        handler->SendGlobalGMSysMessage("DB table `spell_pet_auras` reloaded.");
        return true;
    }

    static bool HandleReloadPageTextsCommand(ChatHandler* handler, char const* /*args*/)
    {
        TC_LOG_INFO("command", "Re-Loading Page Texts...");
        sObjectMgr->LoadPageTexts();
        handler->SendGlobalGMSysMessage("DB table `page_texts` reloaded.");
        return true;
    }

    static bool HandleReloadItemEnchantementsCommand(ChatHandler* handler, char const* /*args*/)
    {
        TC_LOG_INFO("command", "Re-Loading Item Random Enchantments Table...");
        LoadRandomEnchantmentsTable();
        handler->SendGlobalGMSysMessage("DB table `item_enchantment_template` reloaded.");
        return true;
    }

    static bool HandleReloadGameObjectScriptsCommand(ChatHandler* handler, char const* args)
    {
        if (sMapMgr->IsScriptScheduled())
        {
            handler->SendSysMessage("DB scripts used currently, please attempt reload later.");
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (*args != 'a')
            TC_LOG_INFO("command", "Re-Loading Scripts from `gameobject_scripts`...");

        sObjectMgr->LoadGameObjectScripts();

        if (*args != 'a')
            handler->SendGlobalGMSysMessage("DB table `gameobject_scripts` reloaded.");

        return true;
    }

    static bool HandleReloadEventScriptsCommand(ChatHandler* handler, char const* args)
    {
        if (sMapMgr->IsScriptScheduled())
        {
            handler->SendSysMessage("DB scripts used currently, please attempt reload later.");
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (*args != 'a')
            TC_LOG_INFO("command", "Re-Loading Scripts from `event_scripts`...");

        sObjectMgr->LoadEventScripts();

        if (*args != 'a')
            handler->SendGlobalGMSysMessage("DB table `event_scripts` reloaded.");

        return true;
    }

    static bool HandleReloadWpScriptsCommand(ChatHandler* handler, char const* args)
    {
        if (sMapMgr->IsScriptScheduled())
        {
            handler->SendSysMessage("DB scripts used currently, please attempt reload later.");
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (*args != 'a')
            TC_LOG_INFO("command", "Re-Loading Scripts from `waypoint_scripts`...");

        sObjectMgr->LoadWaypointScripts();

        if (*args != 'a')
            handler->SendGlobalGMSysMessage("DB table `waypoint_scripts` reloaded.");

        return true;
    }

    static bool HandleReloadSmartWaypointsCommand(ChatHandler* handler, char const* args)
    {
        sSmartWaypointMgr->LoadFromDB();
        handler->SendGlobalGMSysMessage("DB table `waypoints` reloaded. (SmartAI waypoints)");
        return true;
    }

    static bool HandleReloadAutoAnnounceCommand(ChatHandler* handler, char const* args)
    {
        sWorld->LoadAutoAnnounce();
        return true;
    }

    static bool HandleReloadQuestEndScriptsCommand(ChatHandler* handler, char const* args)
    {
        if (sMapMgr->IsScriptScheduled())
        {
            handler->SendSysMessage("DB scripts used currently, please attempt reload later.");
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (*args != 'a')
            TC_LOG_INFO("command", "Re-Loading Scripts from `quest_end_scripts`...");

        sObjectMgr->LoadQuestEndScripts();

        if (*args != 'a')
            handler->SendGlobalGMSysMessage("DB table `quest_end_scripts` reloaded.");

        return true;
    }

    static bool HandleReloadQuestStartScriptsCommand(ChatHandler* handler, char const* args)
    {
        if (sMapMgr->IsScriptScheduled())
        {
            handler->SendSysMessage("DB scripts used currently, please attempt reload later.");
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (*args != 'a')
            TC_LOG_INFO("command", "Re-Loading Scripts from `quest_start_scripts`...");

        sObjectMgr->LoadQuestStartScripts();

        if (*args != 'a')
            handler->SendGlobalGMSysMessage("DB table `quest_start_scripts` reloaded.");

        return true;
    }

    static bool HandleReloadSpellScriptsCommand(ChatHandler* handler, char const* args)
    {
        if (sMapMgr->IsScriptScheduled())
        {
            handler->SendSysMessage("DB scripts used currently, please attempt reload later.");
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (*args != 'a')
            TC_LOG_INFO("command", "Re-Loading Scripts from `spell_scripts`...");

        sObjectMgr->LoadSpellScripts();

        if (*args != 'a')
            handler->SendGlobalGMSysMessage("DB table `spell_scripts` reloaded.");

        return true;
    }

    static bool HandleReloadGameEventCommand(ChatHandler* handler, char const* args)
    {
        TC_LOG_INFO("command", "Re-Loading game events...");

        sGameEventMgr->LoadFromDB();

        handler->SendGlobalGMSysMessage("DB table `game_event` reloaded.");

        return true;
    }

    static bool HandleReloadGameGraveyardZoneCommand(ChatHandler* handler, char const* /*args*/)
    {
        TC_LOG_INFO("command", "Re-Loading Graveyard-zone links...");

        sObjectMgr->LoadGraveyardZones();

        handler->SendGlobalGMSysMessage("DB table `game_graveyard_zone` reloaded.");

        return true;
    }

    static bool HandleReloadGameTeleCommand(ChatHandler* handler, char const* /*args*/)
    {
        TC_LOG_INFO("command", "Re-Loading Game Tele coordinates...");

        sObjectMgr->LoadGameTele();

        handler->SendGlobalGMSysMessage("DB table `game_tele` reloaded.");

        return true;
    }

    static bool HandleReloadSpellDisabledCommand(ChatHandler* handler, char const* /*args*/)
    {
        TC_LOG_INFO("command", "Re-Loading spell disabled table...");

        sObjectMgr->LoadSpellDisabledEntrys();

        handler->SendGlobalGMSysMessage("DB table `spell_disabled` reloaded.");

        return true;
    }

    static bool HandleReloadLocalesCreatureCommand(ChatHandler* handler, char const* /*args*/)
    {
        TC_LOG_INFO("command", "Re-Loading Locales Creature ...");
        sObjectMgr->LoadCreatureLocales();
        handler->SendGlobalGMSysMessage("DB table `locales_creature` reloaded.");
        return true;
    }

    static bool HandleReloadLocalesGameobjectCommand(ChatHandler* handler, char const* /*args*/)
    {
        TC_LOG_INFO("command", "Re-Loading Locales Gameobject ... ");
        sObjectMgr->LoadGameObjectLocales();
        handler->SendGlobalGMSysMessage("DB table `locales_gameobject` reloaded.");
        return true;
    }

    static bool HandleReloadLocalesItemCommand(ChatHandler* handler, char const* /*args*/)
    {
        TC_LOG_INFO("command", "Re-Loading Locales Item ... ");
        sObjectMgr->LoadItemLocales();
        handler->SendGlobalGMSysMessage("DB table `locales_item` reloaded.");
        return true;
    }

    static bool HandleReloadLocalesGossipTextCommand(ChatHandler* handler, char const* /*args*/)
    {
        TC_LOG_INFO("command", "Re-Loading Locales Gossip texts ... ");
        sObjectMgr->LoadGossipTextLocales();
        handler->SendGlobalGMSysMessage("DB table `locales_gossip_text` reloaded.");
        return true;
    }

    static bool HandleReloadLocalesPageTextCommand(ChatHandler* handler, char const* /*args*/)
    {
        TC_LOG_INFO("command", "Re-Loading Locales Page Text ... ");
        sObjectMgr->LoadPageTextLocales();
        handler->SendGlobalGMSysMessage("DB table `locales_page_text` reloaded.");
        return true;
    }

    static bool HandleReloadLocalesQuestCommand(ChatHandler* handler, char const* /*args*/)
    {
        TC_LOG_INFO("command", "Re-Loading Locales Quest ... ");
        sObjectMgr->LoadQuestLocales();
        handler->SendGlobalGMSysMessage("DB table `locales_quest` reloaded.");
        return true;
    }

    static bool HandleReloadAuctionsCommand(ChatHandler* handler, char const* args)
    {
        ///- Reload dynamic data tables from the database
        TC_LOG_INFO("command", "Re-Loading Auctions...");
        sAuctionMgr->LoadAuctionItems();
        sAuctionMgr->LoadAuctions();
        handler->SendGlobalGMSysMessage("Auctions reloaded.");
        return true;
    }

    static bool HandleReloadConditions(ChatHandler* handler, char const* args)
    {
        TC_LOG_INFO("command", "Re-Loading Conditions...");
        sConditionMgr->LoadConditions(true);
        handler->SendGlobalGMSysMessage("Conditions reloaded.");
        return true;
    }

    /* Usage: .reload smart_scripts [reload existing creatures]
    [reload existing creatures] default to false
    */
    static bool HandleReloadSmartAICommand(ChatHandler* handler, char const* args)
    {
        bool reloadExistingCreatures = false;
        char* cReloadExistingCreatures = strtok((char*)args, "");
        if (cReloadExistingCreatures)
            reloadExistingCreatures = bool(atoi(cReloadExistingCreatures));

        TC_LOG_INFO("command", "Re-Loading SmartAI Scripts... with reloadExistingCreatures = %u", uint32(reloadExistingCreatures));
        sSmartScriptMgr->LoadSmartAIFromDB();
        handler->SendGlobalGMSysMessage("SmartAI Scripts reloaded.");

        if (reloadExistingCreatures)
        {
            if (Creature* target = handler->GetSelectedCreature())
            {
                if (target->GetAIName() == SMARTAI_AI_NAME)
                    target->AIM_Initialize();
                handler->PSendSysMessage("Reloaded SmartAI scripts for targeted creature (%s)", target->GetName().c_str());
            }
            else if (Player* player = handler->GetSession()->GetPlayer()) {
                sSmartScriptMgr->ReloadCreaturesScripts(player->GetMap());
                handler->SendGlobalGMSysMessage("Reloaded SmartAI scripts for all existing creatures in current map. (NYI)");
            }
        }
        return true;
    }

    static bool HandleReloadGMTicketCommand(ChatHandler* handler, char const* /*args*/)
    {
        sObjectMgr->LoadGMTickets();
        return true;
    }

    static bool HandleReloadSpellTemplates(ChatHandler* handler, char const* args)
    {
        TC_LOG_INFO("command", "Re-loading spell templates...");
        sObjectMgr->LoadSpellTemplates();
        sSpellMgr->LoadSpellInfoStore(true);
        sSpellMgr->LoadSpellInfoCorrections();
        sSpellMgr->LoadSpellInfoCustomAttributes();
        //also reload those tables as they can alter spell info too
        sSpellMgr->LoadSpellLinked();
        sSpellMgr->LoadSpellAffects();
        sSpellMgr->LoadSpellTalentRanks();

        handler->SendGlobalGMSysMessage("DB table `spell_template` (spell definitions) reloaded.");
        return true;
    }

    static bool HandleReloadItemExtendedCostCommand(ChatHandler* handler, char const* /*args*/)
    {
        TC_LOG_INFO("command", "Re-Loading Item Extended Cost Table...");
        sObjectMgr->LoadItemExtendedCost();
        handler->SendGlobalGMSysMessage("DB table `item_extended_cost` reloaded.");
        return true;
    }
};

void AddSC_reload_commandscript()
{
    new reload_commandscript();
}
