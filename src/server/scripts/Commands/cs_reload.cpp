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

//reload commands
static bool HandleReloadCommand(const char* arg)
{
    // this is error catcher for wrong table name in .reload commands
    handler->PSendSysMessage("Db table with name starting from '%s' not found and can't be reloaded.",arg);
    handler->SetSentErrorMessage(true);
    return false;
}

static bool HandleReloadCreatureTextCommand(const char* /*args*/)
{
    TC_LOG_INFO("command","Re-Loading Creature Texts...");
    sCreatureTextMgr->LoadCreatureTexts();
    sCreatureTextMgr->LoadCreatureTextLocales();
    SendGlobalGMSysMessage("Creature Texts reloaded.");
    return true;
}

static bool HandleReloadCreatureTemplateCommand(const char* /*args*/)
{
    TC_LOG_INFO("command", "Re-Loading Creature Templates...");
    sObjectMgr->LoadCreatureTemplates(true);
    SendGlobalGMSysMessage("Creature Templates reloaded.");
    return true;
}

static bool HandleReloadAllCommand(const char*)
{
    HandleReloadAreaTriggerTeleportCommand("");
    HandleReloadSkillFishingBaseLevelCommand("");

    HandleReloadAllAreaCommand("");
    HandleReloadAllLootCommand("");
    HandleReloadAllNpcCommand("");
    HandleReloadAllQuestCommand("");
    HandleReloadAllSpellCommand("");
    HandleReloadAllItemCommand("");
    HandleReloadAllLocalesCommand("");

    HandleReloadCommandCommand("");
    HandleReloadReservedNameCommand("");
    HandleReloadTrinityStringCommand("");
    HandleReloadGameTeleCommand("");
    return true;
}

static bool HandleReloadAllAreaCommand(const char*)
{
    //HandleReloadQuestAreaTriggersCommand(""); -- reloaded in HandleReloadAllQuestCommand
    HandleReloadAreaTriggerTeleportCommand("");
    HandleReloadAreaTriggerTavernCommand("");
    HandleReloadGameGraveyardZoneCommand("");
    return true;
}

static bool HandleReloadAllLootCommand(const char*)
{
    TC_LOG_INFO("command", "Re-Loading Loot Tables..." );
    LoadLootTables();
    SendGlobalGMSysMessage("DB tables `*_loot_template` reloaded.");
    return true;
}

static bool HandleReloadAllNpcCommand(const char* /*args*/)
{
    HandleReloadCreatureGossipCommand("a");
    HandleReloadGossipMenuOptionCommand("a");
    HandleReloadNpcTrainerCommand("a");
    HandleReloadNpcVendorCommand("a");
    return true;
}

static bool HandleReloadAllQuestCommand(const char* /*args*/)
{
    HandleReloadQuestAreaTriggersCommand("a");
    HandleReloadQuestTemplateCommand("a");

    TC_LOG_INFO("command", "Re-Loading Quests Relations..." );
    sObjectMgr->LoadQuestRelations();
    SendGlobalGMSysMessage("DB tables `*_questrelation` and `*_involvedrelation` reloaded.");
    return true;
}

static bool HandleReloadAllScriptsCommand(const char*)
{
    if(sMapMgr->IsScriptScheduled())
    {
        handler->PSendSysMessage("DB scripts used currently, please attempt reload later.");
        handler->SetSentErrorMessage(true);
        return false;
    }

    TC_LOG_INFO("command", "Re-Loading Scripts..." );
    HandleReloadGameObjectScriptsCommand("a");
    HandleReloadEventScriptsCommand("a");
    HandleReloadQuestEndScriptsCommand("a");
    HandleReloadQuestStartScriptsCommand("a");
    HandleReloadSpellScriptsCommand("a");
    SendGlobalGMSysMessage("DB tables `*_scripts` reloaded.");
    HandleReloadWpScriptsCommand("a");
    return true;
}

static bool HandleReloadAllSpellCommand(const char*)
{
    HandleReloadSkillDiscoveryTemplateCommand("a");
    HandleReloadSkillExtraItemTemplateCommand("a");
    HandleReloadSpellAffectCommand("a");
    HandleReloadSpellGroupsCommand("a");
    HandleReloadSpellRequiredCommand("a");
    HandleReloadSpellElixirCommand("a");
    HandleReloadSpellLearnSpellCommand("a");
    HandleReloadSpellLinkedSpellCommand("a");
    HandleReloadSpellProcCommand("a");
    HandleReloadSpellTargetPositionCommand("a");
    HandleReloadSpellThreatsCommand("a");
    HandleReloadSpellPetAurasCommand("a");
    HandleReloadSpellGroupStackRulesCommand("a");
    HandleReloadSpellDisabledCommand("a");
    return true;
}

static bool HandleReloadAllItemCommand(const char*)
{
    HandleReloadPageTextsCommand("a");
    HandleReloadItemEnchantementsCommand("a");
    return true;
}

static bool HandleReloadAllLocalesCommand(const char* /*args*/)
{
    HandleReloadLocalesCreatureCommand("a");
    HandleReloadLocalesGameobjectCommand("a");
    HandleReloadLocalesItemCommand("a");
    HandleReloadLocalesGossipTextCommand("a");
    HandleReloadLocalesPageTextCommand("a");
    HandleReloadLocalesQuestCommand("a");
    return true;
}

static bool HandleReloadConfigCommand(const char* /*args*/)
{
    TC_LOG_INFO("command", "Re-Loading config settings..." );
    sWorld->LoadConfigSettings(true);
    sMapMgr->InitializeVisibilityDistanceInfo();
    SendGlobalGMSysMessage("World config settings reloaded.");
    return true;
}

static bool HandleReloadAreaTriggerTavernCommand(const char*)
{
    TC_LOG_INFO("command", "Re-Loading Tavern Area Triggers..." );
    sObjectMgr->LoadTavernAreaTriggers();
    SendGlobalGMSysMessage("DB table `areatrigger_tavern` reloaded.");
    return true;
}

static bool HandleReloadAreaTriggerTeleportCommand(const char*)
{
    TC_LOG_INFO("command", "Re-Loading AreaTrigger teleport definitions..." );
    sObjectMgr->LoadAreaTriggerTeleports();
    SendGlobalGMSysMessage("DB table `areatrigger_teleport` reloaded.");
    return true;
}

static bool HandleReloadAccessRequirementCommand(const char*)
{
    TC_LOG_INFO("command", "Re-Loading Access Requirement definitions..." );
    sObjectMgr->LoadAccessRequirements();
    SendGlobalGMSysMessage("DB table `access_requirement` reloaded.");
     return true;
 }

static bool HandleReloadCommandCommand(const char*)
{
    ChatHandler::invalidateCommandTable();
    SendGlobalGMSysMessage("DB table `command` will be reloaded at next chat command use.");
    return true;
}

static bool HandleReloadCreatureQuestStartersCommand(const char*)
{
    TC_LOG_INFO("command", "Loading Quests Relations... (`creature_queststarter`)" );
    sObjectMgr->LoadCreatureQuestStarters();
    SendGlobalGMSysMessage("DB table `creature_queststarter` (creature quest givers) reloaded.");
    return true;
}

static bool HandleReloadCreatureQuestEndersCommand(const char*)
{
    TC_LOG_INFO("command", "Loading Quests Relations... (`creature_questender`)" );
    sObjectMgr->LoadCreatureQuestEnders();
    SendGlobalGMSysMessage("DB table `creature_questender` (creature quest takers) reloaded.");
    return true;
}

static bool HandleReloadGameobjectQuestStartersCommand(const char*)
{
    TC_LOG_INFO("command", "Loading Quests Relations... (`gameobject_queststarter`)" );
    sObjectMgr->LoadGameobjectQuestStarters();
    SendGlobalGMSysMessage("DB table `gameobject_queststarter` (gameobject quest givers) reloaded.");
    return true;
}

static bool HandleReloadGameobjectQuestEndersCommand(const char*)
{
    TC_LOG_INFO("command", "Loading Quests Relations... (`gameobject_questender`)" );
    sObjectMgr->LoadGameobjectQuestEnders();
    SendGlobalGMSysMessage("DB table `gameobject_questender` (gameobject quest takers) reloaded.");
    return true;
}

static bool HandleReloadQuestAreaTriggersCommand(const char*)
{
    TC_LOG_INFO("command", "Re-Loading Quest Area Triggers..." );
    sObjectMgr->LoadQuestAreaTriggers();
    SendGlobalGMSysMessage("DB table `areatrigger_involvedrelation` (quest area triggers) reloaded.");
    return true;
}

static bool HandleReloadQuestTemplateCommand(const char*)
{
    TC_LOG_INFO("command", "Re-Loading Quest Templates..." );
    sObjectMgr->LoadQuests();
    SendGlobalGMSysMessage("DB table `quest_template` (quest definitions) reloaded.");
    return true;
}

static bool HandleReloadCreatureModelInfoCommand(const char*)
{
    TC_LOG_INFO("command", "Re-Loading `creature_model_info`" );
    sObjectMgr->LoadCreatureModelInfo();
    SendGlobalGMSysMessage("DB table `creature_model_info` reloaded.");
    return true;
}

static bool HandleReloadLootTemplatesCreatureCommand(const char*)
{
    TC_LOG_INFO("command", "Re-Loading Loot Tables... (`creature_loot_template`)" );
    LoadLootTemplates_Creature();
    LootTemplates_Creature.CheckLootRefs();
    SendGlobalGMSysMessage("DB table `creature_loot_template` reloaded.");
    return true;
}

static bool HandleReloadCreatureMovementOverrideCommand(const char*)
{
    TC_LOG_INFO("misc", "Re-Loading Creature movement overrides...");
    sObjectMgr->LoadCreatureMovementOverrides();
    SendGlobalGMSysMessage("DB table `creature_movement_override` reloaded.");
    return true;
}

static bool HandleReloadLootTemplatesDisenchantCommand(const char*)
{
    TC_LOG_INFO( "command","Re-Loading Loot Tables... (`disenchant_loot_template`)" );
    LoadLootTemplates_Disenchant();
    LootTemplates_Disenchant.CheckLootRefs();
    SendGlobalGMSysMessage("DB table `disenchant_loot_template` reloaded.");
    return true;
}

static bool HandleReloadLootTemplatesFishingCommand(const char*)
{
    TC_LOG_INFO( "command","Re-Loading Loot Tables... (`fishing_loot_template`)" );
    LoadLootTemplates_Fishing();
    LootTemplates_Fishing.CheckLootRefs();
    SendGlobalGMSysMessage("DB table `fishing_loot_template` reloaded.");
    return true;
}

static bool HandleReloadLootTemplatesGameobjectCommand(const char*)
{
    TC_LOG_INFO( "command","Re-Loading Loot Tables... (`gameobject_loot_template`)" );
    LoadLootTemplates_Gameobject();
    LootTemplates_Gameobject.CheckLootRefs();
    SendGlobalGMSysMessage("DB table `gameobject_loot_template` reloaded.");
    return true;
}

static bool HandleReloadLootTemplatesItemCommand(const char*)
{
    TC_LOG_INFO( "command","Re-Loading Loot Tables... (`item_loot_template`)" );
    LoadLootTemplates_Item();
    LootTemplates_Item.CheckLootRefs();
    SendGlobalGMSysMessage("DB table `item_loot_template` reloaded.");
    return true;
}

static bool HandleReloadLootTemplatesPickpocketingCommand(const char*)
{
    TC_LOG_INFO( "command", "Re-Loading Loot Tables... (`pickpocketing_loot_template`)" );
    LoadLootTemplates_Pickpocketing();
    LootTemplates_Pickpocketing.CheckLootRefs();
    SendGlobalGMSysMessage("DB table `pickpocketing_loot_template` reloaded.");
    return true;
}

static bool HandleReloadLootTemplatesProspectingCommand(const char*)
{
    TC_LOG_INFO( "command","Re-Loading Loot Tables... (`prospecting_loot_template`)" );
    LoadLootTemplates_Prospecting();
    LootTemplates_Prospecting.CheckLootRefs();
    SendGlobalGMSysMessage("DB table `prospecting_loot_template` reloaded.");
    return true;
}

static bool HandleReloadLootTemplatesMailCommand(const char*)
{
    TC_LOG_INFO("misc", "Re-Loading Loot Tables... (`mail_loot_template`)");
    LoadLootTemplates_Mail();
    LootTemplates_Mail.CheckLootRefs();
    SendGlobalGMSysMessage("DB table `mail_loot_template` reloaded.");
    sConditionMgr->LoadConditions(true);
    return true;
}

static bool HandleReloadLootTemplatesReferenceCommand(const char*)
{
    TC_LOG_INFO( "command","Re-Loading Loot Tables... (`reference_loot_template`)" );
    LoadLootTemplates_Reference();
    SendGlobalGMSysMessage("DB table `reference_loot_template` reloaded.");
    return true;
}

static bool HandleReloadLootTemplatesSkinningCommand(const char*)
{
    TC_LOG_INFO( "command", "Re-Loading Loot Tables... (`skinning_loot_template`)" );
    LoadLootTemplates_Skinning();
    LootTemplates_Skinning.CheckLootRefs();
    SendGlobalGMSysMessage("DB table `skinning_loot_template` reloaded.");
    return true;
}

static bool HandleReloadTrinityStringCommand(const char*)
{
    TC_LOG_INFO( "command", "Re-Loading trinity_string Table!" );
    sObjectMgr->LoadTrinityStrings();
    SendGlobalGMSysMessage("DB table `trinity_string` reloaded.");
    return true;
}

static bool HandleReloadGossipMenuCommand(ChatHandler* handler, char const* args)
{
    TC_LOG_INFO( "command", "Re-Loading `gossip_menu` Table!" );
    sObjectMgr->LoadGossipMenu();
    SendGlobalGMSysMessage("DB table `gossip_menu` reloaded.");
    return true;
}

static bool HandleReloadGossipTextCommand(ChatHandler* handler, char const* args)
{
    TC_LOG_INFO( "command", "Re-Loading `gossip_text` Table!" );
    sObjectMgr->LoadGossipText();
    sObjectMgr->LoadGossipTextLocales();
    SendGlobalGMSysMessage("DB tables `gossip_text` and `locales_gossip_text` reloaded.");
    return true;
}

static bool HandleReloadCreatureGossipCommand(ChatHandler* handler, char const* args)
{
    TC_LOG_INFO( "command", "Re-Loading `creature_gossip` Table!" );
    sObjectMgr->LoadCreatureGossip();
    SendGlobalGMSysMessage("DB table `creature_gossip` reloaded.");
    return true;
}

static bool HandleReloadGossipMenuOptionCommand(const char*)
{
    TC_LOG_INFO( "command", "Re-Loading `gossip_menu_option` Table!" );
    sObjectMgr->LoadGossipMenuItems();
    sObjectMgr->LoadGossipMenuItemsLocales();
    SendGlobalGMSysMessage("DB tables `gossip_menu_option` and `locales_gossip_menu_option` reloaded.");
    return true;
}

static bool HandleReloadNpcTrainerCommand(const char*)
{
    TC_LOG_INFO( "command", "Re-Loading `npc_trainer` Table!" );
    sObjectMgr->LoadTrainerSpell();
    sGameEventMgr->LoadTrainers();
    SendGlobalGMSysMessage("DB table `npc_trainer` reloaded.");
    return true;
}

static bool HandleReloadNpcVendorCommand(const char*)
{
    TC_LOG_INFO( "command", "Re-Loading `npc_vendor` Table!" );
    sObjectMgr->LoadVendors();
    sGameEventMgr->LoadVendors();
    SendGlobalGMSysMessage("DB table `npc_vendor` reloaded.");
    return true;
}

static bool HandleReloadReservedNameCommand(const char*)
{
    TC_LOG_INFO( "server.loading","Loading ReservedNames... (`reserved_name`)" );
    sObjectMgr->LoadReservedPlayersNames();
    SendGlobalGMSysMessage("DB table `reserved_name` (player reserved names) reloaded.");
    return true;
}

static bool HandleReloadSkillDiscoveryTemplateCommand(const char* /*args*/)
{
    TC_LOG_INFO( "server.loading","Re-Loading Skill Discovery Table..." );
    LoadSkillDiscoveryTable();
    SendGlobalGMSysMessage("DB table `skill_discovery_template` (recipes discovered at crafting) reloaded.");
    return true;
}

static bool HandleReloadSkillExtraItemTemplateCommand(const char* /*args*/)
{
    TC_LOG_INFO( "server.loading", "Re-Loading Skill Extra Item Table..." );
    LoadSkillExtraItemTable();
    SendGlobalGMSysMessage("DB table `skill_extra_item_template` (extra item creation when crafting) reloaded.");
    return true;
}

static bool HandleReloadSkillFishingBaseLevelCommand(const char* /*args*/)
{
    TC_LOG_INFO( "command", "Re-Loading Skill Fishing base level requirements..." );
    sObjectMgr->LoadFishingBaseSkillLevel();
    SendGlobalGMSysMessage("DB table `skill_fishing_base_level` (fishing base level for zone/subzone) reloaded.");
    return true;
}

static bool HandleReloadSpellBonusesCommand(const char*)
{
    TC_LOG_INFO("command", "Re-Loading Spell Bonuses definitions...");
    sSpellMgr->LoadSpellBonuses();
    SendGlobalGMSysMessage("DB table `spell_bonus_data` reloaded.");
    return true;
}

static bool HandleReloadSpellAffectCommand(const char*)
{
    TC_LOG_INFO( "command", "Re-Loading SpellAffect definitions..." );
    sSpellMgr->LoadSpellAffects(true);
    SendGlobalGMSysMessage("DB table `spell_affect` (spell mods apply requirements) reloaded.");
    return true;
}

static bool HandleReloadSpellAreaCommand(const char*)
{
    TC_LOG_INFO("command", "Re-Loading spell_area...");
    sSpellMgr->LoadSpellAreas();
    SendGlobalGMSysMessage("DB table `spell_area` reloaded.");
    return true;
}

static bool HandleReloadSpellRequiredCommand(const char*)
{
    TC_LOG_INFO( "command", "Re-Loading Spell Required Data... " );
    sSpellMgr->LoadSpellRequired();
    SendGlobalGMSysMessage("DB table `spell_required` reloaded.");
    return true;
}

static bool HandleReloadSpellElixirCommand(const char*)
{
    TC_LOG_INFO( "command", "Re-Loading Spell Elixir types..." );
    sSpellMgr->LoadSpellElixirs();
    SendGlobalGMSysMessage("DB table `spell_elixir` (spell elixir types) reloaded.");
    return true;
}

static bool HandleReloadSpellGroupsCommand(ChatHandler* handler, char const* args)
{
    TC_LOG_INFO("misc", "Re-Loading Spell Groups...");
    sSpellMgr->LoadSpellGroups();
    SendGlobalGMSysMessage("DB table `spell_group` (spell groups) reloaded.");
    return true;
}

static bool HandleReloadSpellGroupStackRulesCommand(ChatHandler* handler, char const* args)
{
    TC_LOG_INFO("misc", "Re-Loading Spell Group Stack Rules...");
    sSpellMgr->LoadSpellGroupStackRules();
    SendGlobalGMSysMessage("DB table `spell_group_stack_rules` (spell stacking definitions) reloaded.");
    return true;
}

static bool HandleReloadSpellLearnSpellCommand(const char*)
{
    TC_LOG_INFO( "command", "Re-Loading Spell Learn Spells..." );
    sSpellMgr->LoadSpellLearnSpells();
    SendGlobalGMSysMessage("DB table `spell_learn_spell` reloaded.");
    return true;
}

static bool HandleReloadSpellLinkedSpellCommand(const char*)
{
    TC_LOG_INFO( "command", "Re-Loading Spell Linked Spells..." );
    sSpellMgr->LoadSpellLinked();
    SendGlobalGMSysMessage("DB table `spell_linked_spell` reloaded.");
    return true;
}

static bool HandleReloadSpellProcCommand(const char*)
{
    TC_LOG_INFO( "command", "Re-Loading Spell Proc Event conditions..." );
    sSpellMgr->LoadSpellProcs();
    SendGlobalGMSysMessage("DB table `spell_proc` (spell proc trigger requirements) reloaded.");
    return true;
}

static bool HandleReloadSpellTargetPositionCommand(const char*)
{
    TC_LOG_INFO( "command", "Re-Loading Spell target coordinates..." );
    sSpellMgr->LoadSpellTargetPositions();
    SendGlobalGMSysMessage("DB table `spell_target_position` (destination coordinates for spell targets) reloaded.");
    return true;
}

static bool HandleReloadSpellThreatsCommand(const char*)
{
    TC_LOG_INFO( "command", "Re-Loading Aggro Spells Definitions...");
    sSpellMgr->LoadSpellThreats();
    SendGlobalGMSysMessage("DB table `spell_threat` (spell aggro definitions) reloaded.");
    return true;
}

static bool HandleReloadSpellPetAurasCommand(const char*)
{
    TC_LOG_INFO( "command", "Re-Loading Spell pet auras...");
    sSpellMgr->LoadSpellPetAuras();
    SendGlobalGMSysMessage("DB table `spell_pet_auras` reloaded.");
    return true;
}

static bool HandleReloadPageTextsCommand(const char*)
{
    TC_LOG_INFO( "command", "Re-Loading Page Texts..." );
    sObjectMgr->LoadPageTexts();
    SendGlobalGMSysMessage("DB table `page_texts` reloaded.");
    return true;
}

static bool HandleReloadItemEnchantementsCommand(const char*)
{
    TC_LOG_INFO( "command", "Re-Loading Item Random Enchantments Table..." );
    LoadRandomEnchantmentsTable();
    SendGlobalGMSysMessage("DB table `item_enchantment_template` reloaded.");
    return true;
}

static bool HandleReloadGameObjectScriptsCommand(const char* arg)
{
    if(sMapMgr->IsScriptScheduled())
    {
        handler->SendSysMessage("DB scripts used currently, please attempt reload later.");
        handler->SetSentErrorMessage(true);
        return false;
    }

    if(*arg!='a')
        TC_LOG_INFO("command", "Re-Loading Scripts from `gameobject_scripts`...");

    sObjectMgr->LoadGameObjectScripts();

    if(*arg!='a')
        SendGlobalGMSysMessage("DB table `gameobject_scripts` reloaded.");

    return true;
}

static bool HandleReloadEventScriptsCommand(const char* arg)
{
    if(sMapMgr->IsScriptScheduled())
    {
        handler->SendSysMessage("DB scripts used currently, please attempt reload later.");
        handler->SetSentErrorMessage(true);
        return false;
    }

    if(*arg!='a')
        TC_LOG_INFO("command", "Re-Loading Scripts from `event_scripts`...");

    sObjectMgr->LoadEventScripts();

    if(*arg!='a')
        SendGlobalGMSysMessage("DB table `event_scripts` reloaded.");

    return true;
}

static bool HandleReloadWpScriptsCommand(const char* arg)
{
    if(sMapMgr->IsScriptScheduled())
    {
        handler->SendSysMessage("DB scripts used currently, please attempt reload later.");
        handler->SetSentErrorMessage(true);
        return false;
    }

    if(*arg!='a')
        TC_LOG_INFO("command", "Re-Loading Scripts from `waypoint_scripts`...");

    sObjectMgr->LoadWaypointScripts();

    if(*arg!='a')
        SendGlobalGMSysMessage("DB table `waypoint_scripts` reloaded.");

    return true;
}

static bool HandleReloadSmartWaypointsCommand(ChatHandler* handler, char const* args)
{
    sSmartWaypointMgr->LoadFromDB();
    SendGlobalGMSysMessage("DB table `waypoints` reloaded. (SmartAI waypoints)");
    return true;
}

static bool HandleReloadAutoAnnounceCommand(ChatHandler* handler, char const* args)
{
    sWorld->LoadAutoAnnounce();
    return true;
}

static bool HandleReloadQuestEndScriptsCommand(const char* arg)
{
    if(sMapMgr->IsScriptScheduled())
    {
        handler->SendSysMessage("DB scripts used currently, please attempt reload later.");
        handler->SetSentErrorMessage(true);
        return false;
    }

    if(*arg!='a')
        TC_LOG_INFO("command", "Re-Loading Scripts from `quest_end_scripts`...");

    sObjectMgr->LoadQuestEndScripts();

    if(*arg!='a')
        SendGlobalGMSysMessage("DB table `quest_end_scripts` reloaded.");

    return true;
}

static bool HandleReloadQuestStartScriptsCommand(const char* arg)
{
    if(sMapMgr->IsScriptScheduled())
    {
        handler->SendSysMessage("DB scripts used currently, please attempt reload later.");
        handler->SetSentErrorMessage(true);
        return false;
    }

    if(*arg!='a')
        TC_LOG_INFO("command", "Re-Loading Scripts from `quest_start_scripts`...");

    sObjectMgr->LoadQuestStartScripts();

    if(*arg!='a')
        SendGlobalGMSysMessage("DB table `quest_start_scripts` reloaded.");

    return true;
}

static bool HandleReloadSpellScriptsCommand(const char* arg)
{
    if(sMapMgr->IsScriptScheduled())
    {
        handler->SendSysMessage("DB scripts used currently, please attempt reload later.");
        handler->SetSentErrorMessage(true);
        return false;
    }

    if(*arg!='a')
        TC_LOG_INFO("command", "Re-Loading Scripts from `spell_scripts`...");

    sObjectMgr->LoadSpellScripts();

    if(*arg!='a')
        SendGlobalGMSysMessage("DB table `spell_scripts` reloaded.");

    return true;
}

static bool HandleReloadGameEventCommand(ChatHandler* handler, char const* args)
{
    TC_LOG_INFO("command", "Re-Loading game events...");

    sGameEventMgr->LoadFromDB();

    SendGlobalGMSysMessage("DB table `game_event` reloaded.");

    return true;
}

static bool HandleReloadGameGraveyardZoneCommand(const char* /*arg*/)
{
    TC_LOG_INFO("command", "Re-Loading Graveyard-zone links...");

    sObjectMgr->LoadGraveyardZones();

    SendGlobalGMSysMessage("DB table `game_graveyard_zone` reloaded.");

    return true;
}

static bool HandleReloadGameTeleCommand(const char* /*arg*/)
{
    TC_LOG_INFO("command", "Re-Loading Game Tele coordinates...");

    sObjectMgr->LoadGameTele();

    SendGlobalGMSysMessage("DB table `game_tele` reloaded.");

    return true;
}

static bool HandleReloadSpellDisabledCommand(const char* /*arg*/)
{
    TC_LOG_INFO("command", "Re-Loading spell disabled table...");

    sObjectMgr->LoadSpellDisabledEntrys();

    SendGlobalGMSysMessage("DB table `spell_disabled` reloaded.");

    return true;
}

static bool HandleReloadLocalesCreatureCommand(const char* /*arg*/)
{
    TC_LOG_INFO("command", "Re-Loading Locales Creature ...");
    sObjectMgr->LoadCreatureLocales();
    SendGlobalGMSysMessage("DB table `locales_creature` reloaded.");
    return true;
}

static bool HandleReloadLocalesGameobjectCommand(const char* /*arg*/)
{
    TC_LOG_INFO("command", "Re-Loading Locales Gameobject ... ");
    sObjectMgr->LoadGameObjectLocales();
    SendGlobalGMSysMessage("DB table `locales_gameobject` reloaded.");
    return true;
}

static bool HandleReloadLocalesItemCommand(const char* /*arg*/)
{
    TC_LOG_INFO("command", "Re-Loading Locales Item ... ");
    sObjectMgr->LoadItemLocales();
    SendGlobalGMSysMessage("DB table `locales_item` reloaded.");
    return true;
}

static bool HandleReloadLocalesGossipTextCommand(const char* /*arg*/)
{
    TC_LOG_INFO("command", "Re-Loading Locales Gossip texts ... ");
    sObjectMgr->LoadGossipTextLocales();
    SendGlobalGMSysMessage("DB table `locales_gossip_text` reloaded.");
    return true;
}

static bool HandleReloadLocalesPageTextCommand(const char* /*arg*/)
{
    TC_LOG_INFO("command", "Re-Loading Locales Page Text ... ");
    sObjectMgr->LoadPageTextLocales();
    SendGlobalGMSysMessage("DB table `locales_page_text` reloaded.");
    return true;
}

static bool HandleReloadLocalesQuestCommand(const char* /*arg*/)
{
    TC_LOG_INFO("command", "Re-Loading Locales Quest ... ");
    sObjectMgr->LoadQuestLocales();
    SendGlobalGMSysMessage("DB table `locales_quest` reloaded.");
    return true;
}

static bool HandleReloadAuctionsCommand(ChatHandler* handler, char const* args)
{
    ///- Reload dynamic data tables from the database
    TC_LOG_INFO( "command", "Re-Loading Auctions..." );
    sAuctionMgr->LoadAuctionItems();
    sAuctionMgr->LoadAuctions();
    SendGlobalGMSysMessage("Auctions reloaded.");
    return true;
}

static bool HandleReloadConditions(ChatHandler* handler, char const* args)
{
    TC_LOG_INFO("command","Re-Loading Conditions...");
    sConditionMgr->LoadConditions(true);
    SendGlobalGMSysMessage("Conditions reloaded.");
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

    TC_LOG_INFO("command","Re-Loading SmartAI Scripts... with reloadExistingCreatures = %u", uint32(reloadExistingCreatures));
    sSmartScriptMgr->LoadSmartAIFromDB();
    SendGlobalGMSysMessage("SmartAI Scripts reloaded.");

    if (reloadExistingCreatures)
    {
        if (Creature* target = GetSelectedCreature())
        {
            if (target->GetAIName() == SMARTAI_AI_NAME)
                target->AIM_Initialize();
            handler->PSendSysMessage("Reloaded SmartAI scripts for targeted creature (%s)", target->GetName().c_str());
        } else if (Player* player = GetSession()->GetPlayer()) {
            sSmartScriptMgr->ReloadCreaturesScripts(player->GetMap());
            SendGlobalGMSysMessage("Reloaded SmartAI scripts for all existing creatures in current map. (NYI)");
        }
    }
    return true;
}

static bool HandleReloadGMTicketCommand(const char*)
{
    sObjectMgr->LoadGMTickets();
    return true;
}

static bool HandleReloadSpellTemplates(ChatHandler* handler, char const* args)
{
    TC_LOG_INFO("command","Re-loading spell templates...");
    sObjectMgr->LoadSpellTemplates();
    sSpellMgr->LoadSpellInfoStore(true);
    sSpellMgr->LoadSpellInfoCorrections();
    sSpellMgr->LoadSpellInfoCustomAttributes();
    //also reload those tables as they can alter spell info too
    sSpellMgr->LoadSpellLinked();
    sSpellMgr->LoadSpellAffects();
    sSpellMgr->LoadSpellTalentRanks();

    SendGlobalGMSysMessage("DB table `spell_template` (spell definitions) reloaded.");
    return true;
}

static bool HandleReloadItemExtendedCostCommand(const char*)
{
    TC_LOG_INFO( "command", "Re-Loading Item Extended Cost Table..." );
    sObjectMgr->LoadItemExtendedCost();
    SendGlobalGMSysMessage("DB table `item_extended_cost` reloaded.");
    return true;
}
