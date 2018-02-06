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
bool ChatHandler::HandleReloadCommand(const char* arg)
{
    // this is error catcher for wrong table name in .reload commands
    PSendSysMessage("Db table with name starting from '%s' not found and can't be reloaded.",arg);
    SetSentErrorMessage(true);
    return false;
}

bool ChatHandler::HandleReloadCreatureTextCommand(const char* /*args*/)
{
    TC_LOG_INFO("command","Re-Loading Creature Texts...");
    sCreatureTextMgr->LoadCreatureTexts();
    sCreatureTextMgr->LoadCreatureTextLocales();
    SendGlobalGMSysMessage("Creature Texts reloaded.");
    return true;
}

bool ChatHandler::HandleReloadCreatureTemplateCommand(const char* /*args*/)
{
    TC_LOG_INFO("command", "Re-Loading Creature Templates...");
    sObjectMgr->LoadCreatureTemplates(true);
    SendGlobalGMSysMessage("Creature Templates reloaded.");
    return true;
}

bool ChatHandler::HandleReloadAllCommand(const char*)
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

bool ChatHandler::HandleReloadAllAreaCommand(const char*)
{
    //HandleReloadQuestAreaTriggersCommand(""); -- reloaded in HandleReloadAllQuestCommand
    HandleReloadAreaTriggerTeleportCommand("");
    HandleReloadAreaTriggerTavernCommand("");
    HandleReloadGameGraveyardZoneCommand("");
    return true;
}

bool ChatHandler::HandleReloadAllLootCommand(const char*)
{
    TC_LOG_INFO("command", "Re-Loading Loot Tables..." );
    LoadLootTables();
    SendGlobalGMSysMessage("DB tables `*_loot_template` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadAllNpcCommand(const char* /*args*/)
{
    HandleReloadCreatureGossipCommand("a");
    HandleReloadGossipMenuOptionCommand("a");
    HandleReloadNpcTrainerCommand("a");
    HandleReloadNpcVendorCommand("a");
    return true;
}

bool ChatHandler::HandleReloadAllQuestCommand(const char* /*args*/)
{
    HandleReloadQuestAreaTriggersCommand("a");
    HandleReloadQuestTemplateCommand("a");

    TC_LOG_INFO("command", "Re-Loading Quests Relations..." );
    sObjectMgr->LoadQuestRelations();
    SendGlobalGMSysMessage("DB tables `*_questrelation` and `*_involvedrelation` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadAllScriptsCommand(const char*)
{
    if(sMapMgr->IsScriptScheduled())
    {
        PSendSysMessage("DB scripts used currently, please attempt reload later.");
        SetSentErrorMessage(true);
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

bool ChatHandler::HandleReloadAllSpellCommand(const char*)
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

bool ChatHandler::HandleReloadAllItemCommand(const char*)
{
    HandleReloadPageTextsCommand("a");
    HandleReloadItemEnchantementsCommand("a");
    return true;
}

bool ChatHandler::HandleReloadAllLocalesCommand(const char* /*args*/)
{
    HandleReloadLocalesCreatureCommand("a");
    HandleReloadLocalesGameobjectCommand("a");
    HandleReloadLocalesItemCommand("a");
    HandleReloadLocalesGossipTextCommand("a");
    HandleReloadLocalesPageTextCommand("a");
    HandleReloadLocalesQuestCommand("a");
    return true;
}

bool ChatHandler::HandleReloadConfigCommand(const char* /*args*/)
{
    TC_LOG_INFO("command", "Re-Loading config settings..." );
    sWorld->LoadConfigSettings(true);
    sMapMgr->InitializeVisibilityDistanceInfo();
    SendGlobalGMSysMessage("World config settings reloaded.");
    return true;
}

bool ChatHandler::HandleReloadAreaTriggerTavernCommand(const char*)
{
    TC_LOG_INFO("command", "Re-Loading Tavern Area Triggers..." );
    sObjectMgr->LoadTavernAreaTriggers();
    SendGlobalGMSysMessage("DB table `areatrigger_tavern` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadAreaTriggerTeleportCommand(const char*)
{
    TC_LOG_INFO("command", "Re-Loading AreaTrigger teleport definitions..." );
    sObjectMgr->LoadAreaTriggerTeleports();
    SendGlobalGMSysMessage("DB table `areatrigger_teleport` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadAccessRequirementCommand(const char*)
{
    TC_LOG_INFO("command", "Re-Loading Access Requirement definitions..." );
    sObjectMgr->LoadAccessRequirements();
    SendGlobalGMSysMessage("DB table `access_requirement` reloaded.");
     return true;
 }

bool ChatHandler::HandleReloadCommandCommand(const char*)
{
    ChatHandler::invalidateCommandTable();
    SendGlobalGMSysMessage("DB table `command` will be reloaded at next chat command use.");
    return true;
}

bool ChatHandler::HandleReloadCreatureQuestStartersCommand(const char*)
{
    TC_LOG_INFO("command", "Loading Quests Relations... (`creature_queststarter`)" );
    sObjectMgr->LoadCreatureQuestStarters();
    SendGlobalGMSysMessage("DB table `creature_queststarter` (creature quest givers) reloaded.");
    return true;
}

bool ChatHandler::HandleLinkedRespawnCommand(const char* args)
{
    TC_LOG_INFO("command", "Loading Linked Respawns... (`linked_respawn`)" );
    sObjectMgr->LoadLinkedRespawn();
    SendGlobalGMSysMessage("DB table `linked_respawn` (linked respawns) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadCreatureQuestEndersCommand(const char*)
{
    TC_LOG_INFO("command", "Loading Quests Relations... (`creature_questender`)" );
    sObjectMgr->LoadCreatureQuestEnders();
    SendGlobalGMSysMessage("DB table `creature_questender` (creature quest takers) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadGameobjectQuestStartersCommand(const char*)
{
    TC_LOG_INFO("command", "Loading Quests Relations... (`gameobject_queststarter`)" );
    sObjectMgr->LoadGameobjectQuestStarters();
    SendGlobalGMSysMessage("DB table `gameobject_queststarter` (gameobject quest givers) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadGameobjectQuestEndersCommand(const char*)
{
    TC_LOG_INFO("command", "Loading Quests Relations... (`gameobject_questender`)" );
    sObjectMgr->LoadGameobjectQuestEnders();
    SendGlobalGMSysMessage("DB table `gameobject_questender` (gameobject quest takers) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadQuestAreaTriggersCommand(const char*)
{
    TC_LOG_INFO("command", "Re-Loading Quest Area Triggers..." );
    sObjectMgr->LoadQuestAreaTriggers();
    SendGlobalGMSysMessage("DB table `areatrigger_involvedrelation` (quest area triggers) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadQuestTemplateCommand(const char*)
{
    TC_LOG_INFO("command", "Re-Loading Quest Templates..." );
    sObjectMgr->LoadQuests();
    SendGlobalGMSysMessage("DB table `quest_template` (quest definitions) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadCreatureModelInfoCommand(const char*)
{
    TC_LOG_INFO("command", "Re-Loading `creature_model_info`" );
    sObjectMgr->LoadCreatureModelInfo();
    SendGlobalGMSysMessage("DB table `creature_model_info` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadLootTemplatesCreatureCommand(const char*)
{
    TC_LOG_INFO("command", "Re-Loading Loot Tables... (`creature_loot_template`)" );
    LoadLootTemplates_Creature();
    LootTemplates_Creature.CheckLootRefs();
    SendGlobalGMSysMessage("DB table `creature_loot_template` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadLootTemplatesDisenchantCommand(const char*)
{
    TC_LOG_INFO( "command","Re-Loading Loot Tables... (`disenchant_loot_template`)" );
    LoadLootTemplates_Disenchant();
    LootTemplates_Disenchant.CheckLootRefs();
    SendGlobalGMSysMessage("DB table `disenchant_loot_template` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadLootTemplatesFishingCommand(const char*)
{
    TC_LOG_INFO( "command","Re-Loading Loot Tables... (`fishing_loot_template`)" );
    LoadLootTemplates_Fishing();
    LootTemplates_Fishing.CheckLootRefs();
    SendGlobalGMSysMessage("DB table `fishing_loot_template` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadLootTemplatesGameobjectCommand(const char*)
{
    TC_LOG_INFO( "command","Re-Loading Loot Tables... (`gameobject_loot_template`)" );
    LoadLootTemplates_Gameobject();
    LootTemplates_Gameobject.CheckLootRefs();
    SendGlobalGMSysMessage("DB table `gameobject_loot_template` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadLootTemplatesItemCommand(const char*)
{
    TC_LOG_INFO( "command","Re-Loading Loot Tables... (`item_loot_template`)" );
    LoadLootTemplates_Item();
    LootTemplates_Item.CheckLootRefs();
    SendGlobalGMSysMessage("DB table `item_loot_template` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadLootTemplatesPickpocketingCommand(const char*)
{
    TC_LOG_INFO( "command", "Re-Loading Loot Tables... (`pickpocketing_loot_template`)" );
    LoadLootTemplates_Pickpocketing();
    LootTemplates_Pickpocketing.CheckLootRefs();
    SendGlobalGMSysMessage("DB table `pickpocketing_loot_template` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadLootTemplatesProspectingCommand(const char*)
{
    TC_LOG_INFO( "command","Re-Loading Loot Tables... (`prospecting_loot_template`)" );
    LoadLootTemplates_Prospecting();
    LootTemplates_Prospecting.CheckLootRefs();
    SendGlobalGMSysMessage("DB table `prospecting_loot_template` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadLootTemplatesQuestMailCommand(const char*)
{
    TC_LOG_INFO( "command", "Re-Loading Loot Tables... (`quest_mail_loot_template`)" );
    LoadLootTemplates_QuestMail();
    LootTemplates_QuestMail.CheckLootRefs();
    SendGlobalGMSysMessage("DB table `quest_mail_loot_template` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadLootTemplatesReferenceCommand(const char*)
{
    TC_LOG_INFO( "command","Re-Loading Loot Tables... (`reference_loot_template`)" );
    LoadLootTemplates_Reference();
    SendGlobalGMSysMessage("DB table `reference_loot_template` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadLootTemplatesSkinningCommand(const char*)
{
    TC_LOG_INFO( "command", "Re-Loading Loot Tables... (`skinning_loot_template`)" );
    LoadLootTemplates_Skinning();
    LootTemplates_Skinning.CheckLootRefs();
    SendGlobalGMSysMessage("DB table `skinning_loot_template` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadTrinityStringCommand(const char*)
{
    TC_LOG_INFO( "command", "Re-Loading trinity_string Table!" );
    sObjectMgr->LoadTrinityStrings();
    SendGlobalGMSysMessage("DB table `trinity_string` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadGossipMenuCommand(const char* args)
{
    TC_LOG_INFO( "command", "Re-Loading `gossip_menu` Table!" );
    sObjectMgr->LoadGossipMenu();
    SendGlobalGMSysMessage("DB table `gossip_menu` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadGossipTextCommand(const char* args)
{
    TC_LOG_INFO( "command", "Re-Loading `gossip_text` Table!" );
    sObjectMgr->LoadGossipText();
    sObjectMgr->LoadGossipTextLocales();
    SendGlobalGMSysMessage("DB tables `gossip_text` and `locales_gossip_text` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadCreatureGossipCommand(const char* args)
{
    TC_LOG_INFO( "command", "Re-Loading `creature_gossip` Table!" );
    sObjectMgr->LoadCreatureGossip();
    SendGlobalGMSysMessage("DB table `creature_gossip` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadGossipMenuOptionCommand(const char*)
{
    TC_LOG_INFO( "command", "Re-Loading `gossip_menu_option` Table!" );
    sObjectMgr->LoadGossipMenuItems();
    sObjectMgr->LoadGossipMenuItemsLocales();
    SendGlobalGMSysMessage("DB tables `gossip_menu_option` and `locales_gossip_menu_option` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadNpcTrainerCommand(const char*)
{
    TC_LOG_INFO( "command", "Re-Loading `npc_trainer` Table!" );
    sObjectMgr->LoadTrainerSpell();
    sGameEventMgr->LoadTrainers();
    SendGlobalGMSysMessage("DB table `npc_trainer` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadNpcVendorCommand(const char*)
{
    TC_LOG_INFO( "command", "Re-Loading `npc_vendor` Table!" );
    sObjectMgr->LoadVendors();
    sGameEventMgr->LoadVendors();
    SendGlobalGMSysMessage("DB table `npc_vendor` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadReservedNameCommand(const char*)
{
    TC_LOG_INFO( "server.loading","Loading ReservedNames... (`reserved_name`)" );
    sObjectMgr->LoadReservedPlayersNames();
    SendGlobalGMSysMessage("DB table `reserved_name` (player reserved names) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadSkillDiscoveryTemplateCommand(const char* /*args*/)
{
    TC_LOG_INFO( "server.loading","Re-Loading Skill Discovery Table..." );
    LoadSkillDiscoveryTable();
    SendGlobalGMSysMessage("DB table `skill_discovery_template` (recipes discovered at crafting) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadSkillExtraItemTemplateCommand(const char* /*args*/)
{
    TC_LOG_INFO( "server.loading", "Re-Loading Skill Extra Item Table..." );
    LoadSkillExtraItemTable();
    SendGlobalGMSysMessage("DB table `skill_extra_item_template` (extra item creation when crafting) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadSkillFishingBaseLevelCommand(const char* /*args*/)
{
    TC_LOG_INFO( "command", "Re-Loading Skill Fishing base level requirements..." );
    sObjectMgr->LoadFishingBaseSkillLevel();
    SendGlobalGMSysMessage("DB table `skill_fishing_base_level` (fishing base level for zone/subzone) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadSpellBonusesCommand(const char*)
{
    TC_LOG_INFO("command", "Re-Loading Spell Bonuses definitions...");
    sSpellMgr->LoadSpellBonusess();
    SendGlobalGMSysMessage("DB table `spell_bonus_data` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadSpellAffectCommand(const char*)
{
    TC_LOG_INFO( "command", "Re-Loading SpellAffect definitions..." );
    sSpellMgr->LoadSpellAffects();
    SendGlobalGMSysMessage("DB table `spell_affect` (spell mods apply requirements) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadSpellAreaCommand(const char*)
{
    TC_LOG_INFO("command", "Re-Loading spell_area...");
    sSpellMgr->LoadSpellAreas();
    SendGlobalGMSysMessage("DB table `spell_area` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadSpellRequiredCommand(const char*)
{
    TC_LOG_INFO( "command", "Re-Loading Spell Required Data... " );
    sSpellMgr->LoadSpellRequired();
    SendGlobalGMSysMessage("DB table `spell_required` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadSpellElixirCommand(const char*)
{
    TC_LOG_INFO( "command", "Re-Loading Spell Elixir types..." );
    sSpellMgr->LoadSpellElixirs();
    SendGlobalGMSysMessage("DB table `spell_elixir` (spell elixir types) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadSpellGroupsCommand(const char* args)
{
    TC_LOG_INFO("misc", "Re-Loading Spell Groups...");
    sSpellMgr->LoadSpellGroups();
    SendGlobalGMSysMessage("DB table `spell_group` (spell groups) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadSpellGroupStackRulesCommand(const char* args)
{
    TC_LOG_INFO("misc", "Re-Loading Spell Group Stack Rules...");
    sSpellMgr->LoadSpellGroupStackRules();
    SendGlobalGMSysMessage("DB table `spell_group_stack_rules` (spell stacking definitions) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadSpellLearnSpellCommand(const char*)
{
    TC_LOG_INFO( "command", "Re-Loading Spell Learn Spells..." );
    sSpellMgr->LoadSpellLearnSpells();
    SendGlobalGMSysMessage("DB table `spell_learn_spell` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadSpellLinkedSpellCommand(const char*)
{
    TC_LOG_INFO( "command", "Re-Loading Spell Linked Spells..." );
    sSpellMgr->LoadSpellLinked();
    SendGlobalGMSysMessage("DB table `spell_linked_spell` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadSpellProcCommand(const char*)
{
    TC_LOG_INFO( "command", "Re-Loading Spell Proc Event conditions..." );
    sSpellMgr->LoadSpellProcs();
    SendGlobalGMSysMessage("DB table `spell_proc` (spell proc trigger requirements) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadSpellTargetPositionCommand(const char*)
{
    TC_LOG_INFO( "command", "Re-Loading Spell target coordinates..." );
    sSpellMgr->LoadSpellTargetPositions();
    SendGlobalGMSysMessage("DB table `spell_target_position` (destination coordinates for spell targets) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadSpellThreatsCommand(const char*)
{
    TC_LOG_INFO( "command", "Re-Loading Aggro Spells Definitions...");
    sSpellMgr->LoadSpellThreats();
    SendGlobalGMSysMessage("DB table `spell_threat` (spell aggro definitions) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadSpellPetAurasCommand(const char*)
{
    TC_LOG_INFO( "command", "Re-Loading Spell pet auras...");
    sSpellMgr->LoadSpellPetAuras();
    SendGlobalGMSysMessage("DB table `spell_pet_auras` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadPageTextsCommand(const char*)
{
    TC_LOG_INFO( "command", "Re-Loading Page Texts..." );
    sObjectMgr->LoadPageTexts();
    SendGlobalGMSysMessage("DB table `page_texts` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadItemEnchantementsCommand(const char*)
{
    TC_LOG_INFO( "command", "Re-Loading Item Random Enchantments Table..." );
    LoadRandomEnchantmentsTable();
    SendGlobalGMSysMessage("DB table `item_enchantment_template` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadGameObjectScriptsCommand(const char* arg)
{
    if(sMapMgr->IsScriptScheduled())
    {
        SendSysMessage("DB scripts used currently, please attempt reload later.");
        SetSentErrorMessage(true);
        return false;
    }

    if(*arg!='a')
        TC_LOG_INFO("command", "Re-Loading Scripts from `gameobject_scripts`...");

    sObjectMgr->LoadGameObjectScripts();

    if(*arg!='a')
        SendGlobalGMSysMessage("DB table `gameobject_scripts` reloaded.");

    return true;
}

bool ChatHandler::HandleReloadEventScriptsCommand(const char* arg)
{
    if(sMapMgr->IsScriptScheduled())
    {
        SendSysMessage("DB scripts used currently, please attempt reload later.");
        SetSentErrorMessage(true);
        return false;
    }

    if(*arg!='a')
        TC_LOG_INFO("command", "Re-Loading Scripts from `event_scripts`...");

    sObjectMgr->LoadEventScripts();

    if(*arg!='a')
        SendGlobalGMSysMessage("DB table `event_scripts` reloaded.");

    return true;
}

bool ChatHandler::HandleReloadWpScriptsCommand(const char* arg)
{
    if(sMapMgr->IsScriptScheduled())
    {
        SendSysMessage("DB scripts used currently, please attempt reload later.");
        SetSentErrorMessage(true);
        return false;
    }

    if(*arg!='a')
        TC_LOG_INFO("command", "Re-Loading Scripts from `waypoint_scripts`...");

    sObjectMgr->LoadWaypointScripts();

    if(*arg!='a')
        SendGlobalGMSysMessage("DB table `waypoint_scripts` reloaded.");

    return true;
}

bool ChatHandler::HandleReloadSmartWaypointsCommand(const char* args)
{
    sSmartWaypointMgr->LoadFromDB();
    SendGlobalGMSysMessage("DB table `waypoints` reloaded. (SmartAI waypoints)");
    return true;
}

bool ChatHandler::HandleReloadAutoAnnounceCommand(const char* args)
{
    sWorld->LoadAutoAnnounce();
    return true;
}

bool ChatHandler::HandleReloadQuestEndScriptsCommand(const char* arg)
{
    if(sMapMgr->IsScriptScheduled())
    {
        SendSysMessage("DB scripts used currently, please attempt reload later.");
        SetSentErrorMessage(true);
        return false;
    }

    if(*arg!='a')
        TC_LOG_INFO("command", "Re-Loading Scripts from `quest_end_scripts`...");

    sObjectMgr->LoadQuestEndScripts();

    if(*arg!='a')
        SendGlobalGMSysMessage("DB table `quest_end_scripts` reloaded.");

    return true;
}

bool ChatHandler::HandleReloadQuestStartScriptsCommand(const char* arg)
{
    if(sMapMgr->IsScriptScheduled())
    {
        SendSysMessage("DB scripts used currently, please attempt reload later.");
        SetSentErrorMessage(true);
        return false;
    }

    if(*arg!='a')
        TC_LOG_INFO("command", "Re-Loading Scripts from `quest_start_scripts`...");

    sObjectMgr->LoadQuestStartScripts();

    if(*arg!='a')
        SendGlobalGMSysMessage("DB table `quest_start_scripts` reloaded.");

    return true;
}

bool ChatHandler::HandleReloadSpellScriptsCommand(const char* arg)
{
    if(sMapMgr->IsScriptScheduled())
    {
        SendSysMessage("DB scripts used currently, please attempt reload later.");
        SetSentErrorMessage(true);
        return false;
    }

    if(*arg!='a')
        TC_LOG_INFO("command", "Re-Loading Scripts from `spell_scripts`...");

    sObjectMgr->LoadSpellScripts();

    if(*arg!='a')
        SendGlobalGMSysMessage("DB table `spell_scripts` reloaded.");

    return true;
}

bool ChatHandler::HandleReloadGameEventCommand(const char* args)
{
    TC_LOG_INFO("command", "Re-Loading game events...");

    sGameEventMgr->LoadFromDB();

    SendGlobalGMSysMessage("DB table `game_event` reloaded.");

    return true;
}

bool ChatHandler::HandleReloadGameGraveyardZoneCommand(const char* /*arg*/)
{
    TC_LOG_INFO("command", "Re-Loading Graveyard-zone links...");

    sObjectMgr->LoadGraveyardZones();

    SendGlobalGMSysMessage("DB table `game_graveyard_zone` reloaded.");

    return true;
}

bool ChatHandler::HandleReloadGameTeleCommand(const char* /*arg*/)
{
    TC_LOG_INFO("command", "Re-Loading Game Tele coordinates...");

    sObjectMgr->LoadGameTele();

    SendGlobalGMSysMessage("DB table `game_tele` reloaded.");

    return true;
}

bool ChatHandler::HandleReloadSpellDisabledCommand(const char* /*arg*/)
{
    TC_LOG_INFO("command", "Re-Loading spell disabled table...");

    sObjectMgr->LoadSpellDisabledEntrys();

    SendGlobalGMSysMessage("DB table `spell_disabled` reloaded.");

    return true;
}

bool ChatHandler::HandleReloadLocalesCreatureCommand(const char* /*arg*/)
{
    TC_LOG_INFO("command", "Re-Loading Locales Creature ...");
    sObjectMgr->LoadCreatureLocales();
    SendGlobalGMSysMessage("DB table `locales_creature` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadLocalesGameobjectCommand(const char* /*arg*/)
{
    TC_LOG_INFO("command", "Re-Loading Locales Gameobject ... ");
    sObjectMgr->LoadGameObjectLocales();
    SendGlobalGMSysMessage("DB table `locales_gameobject` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadLocalesItemCommand(const char* /*arg*/)
{
    TC_LOG_INFO("command", "Re-Loading Locales Item ... ");
    sObjectMgr->LoadItemLocales();
    SendGlobalGMSysMessage("DB table `locales_item` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadLocalesGossipTextCommand(const char* /*arg*/)
{
    TC_LOG_INFO("command", "Re-Loading Locales Gossip texts ... ");
    sObjectMgr->LoadGossipTextLocales();
    SendGlobalGMSysMessage("DB table `locales_gossip_text` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadLocalesPageTextCommand(const char* /*arg*/)
{
    TC_LOG_INFO("command", "Re-Loading Locales Page Text ... ");
    sObjectMgr->LoadPageTextLocales();
    SendGlobalGMSysMessage("DB table `locales_page_text` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadLocalesQuestCommand(const char* /*arg*/)
{
    TC_LOG_INFO("command", "Re-Loading Locales Quest ... ");
    sObjectMgr->LoadQuestLocales();
    SendGlobalGMSysMessage("DB table `locales_quest` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadAuctionsCommand(const char* args)
{
    ///- Reload dynamic data tables from the database
    TC_LOG_INFO( "command", "Re-Loading Auctions..." );
    sAuctionMgr->LoadAuctionItems();
    sAuctionMgr->LoadAuctions();
    SendGlobalGMSysMessage("Auctions reloaded.");
    return true;
}

bool ChatHandler::HandleReloadConditions(const char* args)
{
    TC_LOG_INFO("command","Re-Loading Conditions...");
    sConditionMgr->LoadConditions(true);
    SendGlobalGMSysMessage("Conditions reloaded.");
    return true;
}

/* Usage: .reload smart_scripts [reload existing creatures]
[reload existing creatures] default to false
*/
bool ChatHandler::HandleReloadSmartAICommand(const char* args)
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
            PSendSysMessage("Reloaded SmartAI scripts for targeted creature (%s)", target->GetName().c_str());
        } else if (Player* player = GetSession()->GetPlayer()) {
            sSmartScriptMgr->ReloadCreaturesScripts(player->GetMap());
            SendGlobalGMSysMessage("Reloaded SmartAI scripts for all existing creatures in current map. (NYI)");
        }
    }
    return true;
}

bool ChatHandler::HandleReloadGMTicketCommand(const char*)
{
    sObjectMgr->LoadGMTickets();
    return true;
}

bool ChatHandler::HandleReloadSpellTemplates(const char* args)
{
    TC_LOG_INFO("command","Re-loading spell templates...");
    sObjectMgr->LoadSpellTemplates();
    sSpellMgr->LoadSpellInfoStore(true);
    //also reload those tables as they can alter spell info too
    sSpellMgr->LoadSpellLinked();
    sSpellMgr->LoadSpellAffects();
    sSpellMgr->LoadSpellTalentRanks()

    SendGlobalGMSysMessage("DB table `spell_template` (spell definitions) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadItemExtendedCostCommand(const char*)
{
    TC_LOG_INFO( "command", "Re-Loading Item Extended Cost Table..." );
    sObjectMgr->LoadItemExtendedCost();
    SendGlobalGMSysMessage("DB table `item_extended_cost` reloaded.");
    return true;
}
