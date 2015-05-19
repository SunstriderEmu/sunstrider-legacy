/*
* Copyright (C) 2005-2009 Trinity <http://getmangos.com/>
*
* Copyright (C) 2008-2009 Trinity <http://www.trinitycore.org/>
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
#include "ObjectMgr.h"
#include "AuctionHouseMgr.h"
#include "AccountMgr.h"
#include "PlayerDump.h"
#include "SpellMgr.h"
#include "Player.h"
#include "Opcodes.h"
#include "GameObject.h"
#include "Chat.h"
#include "Log.h"
#include "Guild.h"
#include "ObjectAccessor.h"
#include "MapManager.h"
#include "SpellAuras.h"
#include "ScriptCalls.h"
#include "Language.h"
#include "GridNotifiersImpl.h"
#include "CellImpl.h"
#include "Weather.h"
#include "PointMovementGenerator.h"
#include "TargetedMovementGenerator.h"
#include "WaypointMovementGenerator.h"
#include "SkillDiscovery.h"
#include "SkillExtraItems.h"
#include "SystemConfig.h"
#include "ConfigMgr.h"
#include "Util.h"
#include "ItemEnchantmentMgr.h"
#include "BattleGroundMgr.h"
#include "InstanceSaveMgr.h"
#include "InstanceData.h"
#include "ChannelMgr.h"
#include "ScriptedInstance.h"
#include "CreatureTextMgr.h"
#include "ConditionMgr.h"
#include "SmartAI.h"
#include "GameEvent.h"
#include "IRCMgr.h"

#include "Management/MMapManager.h"                         // for mmap manager
#include "PathGenerator.h"                                  // for mmap commands                                

//reload commands
bool ChatHandler::HandleReloadCommand(const char* arg)
{
    // this is error catcher for wrong table name in .reload commands
    PSendSysMessage("Db table with name starting from '%s' not found and can't be reloaded.",arg);
    SetSentErrorMessage(true);
    return false;
}

bool ChatHandler::HandleReloadCreatureText(const char* /*args*/)
{
    TC_LOG_INFO("command","Re-Loading Creature Texts...");
    sCreatureTextMgr->LoadCreatureTexts();
    SendGlobalGMSysMessage("Creature Texts reloaded.");
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
    if(sWorld->IsScriptScheduled())
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
    HandleReloadDbScriptStringCommand("a");
    HandleReloadWpScriptsCommand("a");
    return true;
}

bool ChatHandler::HandleReloadAllSpellCommand(const char*)
{
    HandleReloadSkillDiscoveryTemplateCommand("a");
    HandleReloadSkillExtraItemTemplateCommand("a");
    HandleReloadSpellAffectCommand("a");
    HandleReloadSpellRequiredCommand("a");
    HandleReloadSpellElixirCommand("a");
    HandleReloadSpellLearnSpellCommand("a");
    HandleReloadSpellLinkedSpellCommand("a");
    HandleReloadSpellProcEventCommand("a");
    HandleReloadSpellScriptTargetCommand("a");
    HandleReloadSpellTargetPositionCommand("a");
    HandleReloadSpellThreatsCommand("a");
    HandleReloadSpellPetAurasCommand("a");
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
    load_command_table = true;
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

bool ChatHandler::HandleReloadCreatureLinkedRespawnCommand(const char* args)
{
    TC_LOG_INFO("command", "Loading Linked Respawns... (`creature_linked_respawn`)" );
    sObjectMgr->LoadCreatureLinkedRespawn();
    SendGlobalGMSysMessage("DB table `creature_linked_respawn` (creature linked respawns) reloaded.");
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
    SendGlobalGMSysMessage("DB table `npc_trainer` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadNpcVendorCommand(const char*)
{
    TC_LOG_INFO( "command", "Re-Loading `npc_vendor` Table!" );
    sObjectMgr->LoadVendors();
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

bool ChatHandler::HandleReloadSpellAffectCommand(const char*)
{
    TC_LOG_INFO( "command", "Re-Loading SpellAffect definitions..." );
    sSpellMgr->LoadSpellAffects();
    SendGlobalGMSysMessage("DB table `spell_affect` (spell mods apply requirements) reloaded.");
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

bool ChatHandler::HandleReloadSpellProcEventCommand(const char*)
{
    TC_LOG_INFO( "command", "Re-Loading Spell Proc Event conditions..." );
    sSpellMgr->LoadSpellProcEvents();
    SendGlobalGMSysMessage("DB table `spell_proc_event` (spell proc trigger requirements) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadSpellScriptTargetCommand(const char*)
{
    TC_LOG_INFO( "command", "Re-Loading SpellsScriptTarget..." );
    sSpellMgr->LoadSpellScriptTarget();
    SendGlobalGMSysMessage("DB table `spell_script_target` (spell targets selection in case specific creature/GO requirements) reloaded.");
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
    if(sWorld->IsScriptScheduled())
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

bool ChatHandler::HandleReloadInstanceTemplateAddonCommand(const char* arg)
{
    TC_LOG_INFO("command", "Re-Loading Instance Templates Addon..." );
    sObjectMgr->LoadInstanceTemplateAddon();
    SendGlobalGMSysMessage("DB table `instance_template_addon` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadEventScriptsCommand(const char* arg)
{
    if(sWorld->IsScriptScheduled())
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
    if(sWorld->IsScriptScheduled())
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

bool ChatHandler::HandleReloadAutoAnnounceCommand(const char* args)
{
    sWorld->LoadAutoAnnounce();
    return true;
}

bool ChatHandler::HandleReloadQuestEndScriptsCommand(const char* arg)
{
    if(sWorld->IsScriptScheduled())
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
    if(sWorld->IsScriptScheduled())
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
    if(sWorld->IsScriptScheduled())
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

bool ChatHandler::HandleReloadDbScriptStringCommand(const char* arg)
{
    TC_LOG_INFO("command", "Re-Loading Script strings from `db_script_string`...");
    sObjectMgr->LoadDbScriptStrings();
    SendGlobalGMSysMessage("DB table `db_script_string` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadGameEventCommand(const char* args)
{
    TC_LOG_INFO("command", "Re-Loading game events...");

    gameeventmgr.LoadFromDB();

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

bool ChatHandler::HandleLoadScriptsCommand(const char* args)
{
    if(!LoadScriptingModule()) return true;

    sWorld->SendGMText(LANG_SCRIPTS_RELOADED);
    return true;
}

bool ChatHandler::HandleReloadAuctionsCommand(const char* args)
{
    ///- Reload dynamic data tables from the database
    TC_LOG_INFO( "command", "Re-Loading Auctions..." );
    sAHMgr.LoadAuctionItems();
    sAHMgr.LoadAuctions();
    SendGlobalGMSysMessage("Auctions reloaded.");
    return true;
}

bool ChatHandler::HandleAccountSetGmLevelCommand(const char* args)
{
    if (!*args)
    {
        SendSysMessage(LANG_CMD_SYNTAX);
        SetSentErrorMessage(true);
        return false;
    }

    std::string targetAccountName;
    uint32 targetAccountId = 0;
    uint32 targetSecurity = 0;
    uint32 gm = 0;
    char* arg1 = strtok((char*)args, " ");
    char* arg2 = strtok(NULL, " ");
    char* arg3 = strtok(NULL, " ");
    bool isAccountNameGiven = true;

    if (!arg3)
    {
        if (!getSelectedPlayer())
            return false;
        isAccountNameGiven = false;
    }

    // Check for second parameter
    if (!isAccountNameGiven && !arg2)
        return false;

    // Check for account
    if (isAccountNameGiven)
    {
        targetAccountName = arg1;
        if (!AccountMgr::normalizeString(targetAccountName) || !AccountMgr::GetId(targetAccountName))
        {
            PSendSysMessage(LANG_ACCOUNT_NOT_EXIST, targetAccountName.c_str());
            SetSentErrorMessage(true);
            return false;
        }
    }

    // Check for invalid specified GM level.
    gm = (isAccountNameGiven) ? atoi(arg2) : atoi(arg1);
    if (gm > SEC_CONSOLE)
    {
        SendSysMessage(LANG_BAD_VALUE);
        SetSentErrorMessage(true);
        return false;
    }

    // handler->getSession() == NULL only for console
    targetAccountId = (isAccountNameGiven) ? AccountMgr::GetId(targetAccountName) : getSelectedPlayer()->GetSession()->GetAccountId();
    int32 gmRealmID = (isAccountNameGiven) ? atoi(arg3) : atoi(arg2);
    uint32 playerSecurity;
    if (GetSession())
        playerSecurity = AccountMgr::GetSecurity(GetSession()->GetAccountId(), gmRealmID);
    else
        playerSecurity = SEC_CONSOLE;

    // can set security level only for target with less security and to less security that we have
    // This also restricts setting handler's own security.
    targetSecurity = AccountMgr::GetSecurity(targetAccountId, gmRealmID);
    if (targetSecurity >= playerSecurity || gm >= playerSecurity)
    {
        SendSysMessage(LANG_YOURS_SECURITY_IS_LOW);
        SetSentErrorMessage(true);
        return false;
    }

    // Check and abort if the target gm has a higher rank on one of the realms and the new realm is -1
    if (gmRealmID == -1 && !AccountMgr::IsConsoleAccount(playerSecurity))
    {
        PreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_ACCOUNT_ACCESS_GMLEVEL_TEST);

        stmt->setUInt32(0, targetAccountId);
        stmt->setUInt8(1, uint8(gm));

        PreparedQueryResult result = LoginDatabase.Query(stmt);

        if (result)
        {
            SendSysMessage(LANG_YOURS_SECURITY_IS_LOW);
            SetSentErrorMessage(true);
            return false;
        }
    }

    // Check if provided realmID has a negative value other than -1
    if (gmRealmID < -1)
    {
        SendSysMessage("You have not chosen -1 or the current realmID that you are on.");
        SetSentErrorMessage(true);
        return false;
    }

    rbac::RBACData* rbac = nullptr;
    //rbac = isAccountNameGiven ? NULL : getSelectedPlayer()->GetSession()->GetRBACData(); //TODO RBAC
    sAccountMgr->UpdateAccountAccess(rbac, targetAccountId, uint8(gm), gmRealmID);

    PSendSysMessage(LANG_YOU_CHANGE_SECURITY, targetAccountName.c_str(), gm);
    return true;
}

/// Set password for account
bool ChatHandler::HandleAccountSetPasswordCommand(const char* args)
{
    ARGS_CHECK

    ///- Get the command line arguments
    char *szAccount = strtok ((char*)args," ");
    char *szPassword1 =  strtok (NULL," ");
    char *szPassword2 =  strtok (NULL," ");

    if (!szAccount||!szPassword1 || !szPassword2)
        return false;

    std::string account_name = szAccount;
    if(!AccountMgr::normalizeString(account_name))
    {
        PSendSysMessage(LANG_ACCOUNT_NOT_EXIST,account_name.c_str());
        SetSentErrorMessage(true);
        return false;
    }

    uint32 targetAccountId = sAccountMgr->GetId(account_name);
    if (!targetAccountId)
    {
        PSendSysMessage(LANG_ACCOUNT_NOT_EXIST,account_name.c_str());
        SetSentErrorMessage(true);
        return false;
    }

    uint32 targetSecurity = sAccountMgr->GetSecurity(targetAccountId);

    /// m_session==NULL only for console
    uint32 plSecurity = m_session ? m_session->GetSecurity() : SEC_ADMINISTRATOR;

    /// can set password only for target with less security
    /// This is also reject self apply in fact
    if (targetSecurity >= plSecurity)
    {
        SendSysMessage (LANG_YOURS_SECURITY_IS_LOW);
        SetSentErrorMessage (true);
        return false;
    }

    if (strcmp(szPassword1,szPassword2))
    {
        SendSysMessage (LANG_NEW_PASSWORDS_NOT_MATCH);
        SetSentErrorMessage (true);
        return false;
    }

    AccountOpResult result = sAccountMgr->ChangePassword(targetAccountId, szPassword1);

    switch(result)
    {
        case AOR_OK:
            SendSysMessage(LANG_COMMAND_PASSWORD);
            break;
        case AOR_NAME_NOT_EXIST:
            PSendSysMessage(LANG_ACCOUNT_NOT_EXIST,account_name.c_str());
            SetSentErrorMessage(true);
            return false;
        case AOR_PASS_TOO_LONG:
            SendSysMessage(LANG_PASSWORD_TOO_LONG);
            SetSentErrorMessage(true);
            return false;
        default:
            SendSysMessage(LANG_COMMAND_NOTCHANGEPASSWORD);
            SetSentErrorMessage(true);
            return false;
    }

    return true;
}

bool ChatHandler::HandleMaxSkillCommand(const char* /*args*/)
{
    

    Player* SelectedPlayer = getSelectedPlayer();
    if(!SelectedPlayer)
       SelectedPlayer = m_session->GetPlayer();

    // each skills that have max skill value dependent from level seted to current level max skill value
    SelectedPlayer->UpdateSkillsToMaxSkillsForLevel();
    return true;
}

bool ChatHandler::HandleSetSkillCommand(const char* args)
{
    // number or [name] Shift-click form |color|Hskill:skill_id|h[name]|h|r
    char* skill_p = extractKeyFromLink((char*)args,"Hskill");
    if(!skill_p)
        return false;

    char *level_p = strtok (NULL, " ");

    if( !level_p)
        return false;

    char *max_p   = strtok (NULL, " ");

    int32 skill = atoi(skill_p);

    if (skill <= 0)
    {
        PSendSysMessage(LANG_INVALID_SKILL_ID, skill);
        SetSentErrorMessage(true);
        return false;
    }

    int32 level = atol (level_p);

    Player * target = getSelectedPlayer();
    if(!target)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    SkillLineEntry const* sl = sSkillLineStore.LookupEntry(skill);
    if(!sl)
    {
        PSendSysMessage(LANG_INVALID_SKILL_ID, skill);
        SetSentErrorMessage(true);
        return false;
    }

    if(!target->GetSkillValue(skill))
    {
        PSendSysMessage(LANG_SET_SKILL_ERROR, target->GetName().c_str(), skill, sl->name[0]);
        SetSentErrorMessage(true);
        return false;
    }

    int32 max   = max_p ? atol (max_p) : target->GetPureMaxSkillValue(skill);

    if( level <= 0 || level > max || max <= 0 )
        return false;

    target->SetSkill(skill, level, max);
    PSendSysMessage(LANG_SET_SKILL, skill, sl->name[0], target->GetName().c_str(), level, max);

    return true;
}

bool ChatHandler::HandleUnLearnCommand(const char* args)
{
    ARGS_CHECK

    // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r
    uint32 min_id = extractSpellIdFromLink((char*)args);
    if(!min_id)
        return false;

    // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r
    char* tail = strtok(NULL,"");

    uint32 max_id = extractSpellIdFromLink(tail);

    if (!max_id)
    {
        // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r
        max_id =  min_id+1;
    }
    else
    {
        if (max_id < min_id)
            std::swap(min_id,max_id);

        max_id=max_id+1;
    }

    Player* target = getSelectedPlayer();
    if(!target)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    for(uint32 spell=min_id;spell<max_id;spell++)
    {
        if (target->HasSpell(spell))
            target->removeSpell(spell);
        else
            SendSysMessage(LANG_FORGET_SPELL);
    }

    return true;
}

bool ChatHandler::HandleCooldownCommand(const char* args)
{
    

    Player* target = getSelectedPlayer();
    if(!target)
        target = m_session->GetPlayer();

    if (!*args)
    {
        target->RemoveAllSpellCooldown();
        PSendSysMessage(LANG_REMOVEALL_COOLDOWN, target->GetName().c_str());
    }
    else
    {
        // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
        uint32 spell_id = extractSpellIdFromLink((char*)args);
        if(!spell_id)
            return false;

        if(!sSpellMgr->GetSpellInfo(spell_id))
        {
            PSendSysMessage(LANG_UNKNOWN_SPELL, target==m_session->GetPlayer() ? GetTrinityString(LANG_YOU) : target->GetName().c_str());
            SetSentErrorMessage(true);
            return false;
        }

        WorldPacket data( SMSG_CLEAR_COOLDOWN, (4+8) );
        data << uint32(spell_id);
        data << uint64(target->GetGUID());
        target->GetSession()->SendPacket(&data);
        target->RemoveSpellCooldown(spell_id);
        PSendSysMessage(LANG_REMOVE_COOLDOWN, spell_id, target==m_session->GetPlayer() ? GetTrinityString(LANG_YOU) : target->GetName().c_str());
    }
    return true;
}

bool ChatHandler::HandleLearnAllCommand(const char* /*args*/)
{
    

    static const char *allSpellList[] =
    {
        "3365",
        "6233",
        "6247",
        "6246",
        "6477",
        "6478",
        "22810",
        "8386",
        "21651",
        "21652",
        "522",
        "7266",
        "8597",
        "2479",
        "22027",
        "6603",
        "5019",
        "133",
        "168",
        "227",
        "5009",
        "9078",
        "668",
        "203",
        "20599",
        "20600",
        "81",
        "20597",
        "20598",
        "20864",
        "1459",
        "5504",
        "587",
        "5143",
        "118",
        "5505",
        "597",
        "604",
        "1449",
        "1460",
        "2855",
        "1008",
        "475",
        "5506",
        "1463",
        "12824",
        "8437",
        "990",
        "5145",
        "8450",
        "1461",
        "759",
        "8494",
        "8455",
        "8438",
        "6127",
        "8416",
        "6129",
        "8451",
        "8495",
        "8439",
        "3552",
        "8417",
        "10138",
        "12825",
        "10169",
        "10156",
        "10144",
        "10191",
        "10201",
        "10211",
        "10053",
        "10173",
        "10139",
        "10145",
        "10192",
        "10170",
        "10202",
        "10054",
        "10174",
        "10193",
        "12826",
        "2136",
        "143",
        "145",
        "2137",
        "2120",
        "3140",
        "543",
        "2138",
        "2948",
        "8400",
        "2121",
        "8444",
        "8412",
        "8457",
        "8401",
        "8422",
        "8445",
        "8402",
        "8413",
        "8458",
        "8423",
        "8446",
        "10148",
        "10197",
        "10205",
        "10149",
        "10215",
        "10223",
        "10206",
        "10199",
        "10150",
        "10216",
        "10207",
        "10225",
        "10151",
        "116",
        "205",
        "7300",
        "122",
        "837",
        "10",
        "7301",
        "7322",
        "6143",
        "120",
        "865",
        "8406",
        "6141",
        "7302",
        "8461",
        "8407",
        "8492",
        "8427",
        "8408",
        "6131",
        "7320",
        "10159",
        "8462",
        "10185",
        "10179",
        "10160",
        "10180",
        "10219",
        "10186",
        "10177",
        "10230",
        "10181",
        "10161",
        "10187",
        "10220",
        "2018",
        "2663",
        "12260",
        "2660",
        "3115",
        "3326",
        "2665",
        "3116",
        "2738",
        "3293",
        "2661",
        "3319",
        "2662",
        "9983",
        "8880",
        "2737",
        "2739",
        "7408",
        "3320",
        "2666",
        "3323",
        "3324",
        "3294",
        "22723",
        "23219",
        "23220",
        "23221",
        "23228",
        "23338",
        "10788",
        "10790",
        "5611",
        "5016",
        "5609",
        "2060",
        "10963",
        "10964",
        "10965",
        "22593",
        "22594",
        "596",
        "996",
        "499",
        "768",
        "17002",
        "1448",
        "1082",
        "16979",
        "1079",
        "5215",
        "20484",
        "5221",
        "15590",
        "17007",
        "6795",
        "6807",
        "5487",
        "1446",
        "1066",
        "5421",
        "3139",
        "779",
        "6811",
        "6808",
        "1445",
        "5216",
        "1737",
        "5222",
        "5217",
        "1432",
        "6812",
        "9492",
        "5210",
        "3030",
        "1441",
        "783",
        "6801",
        "20739",
        "8944",
        "9491",
        "22569",
        "5226",
        "6786",
        "1433",
        "8973",
        "1828",
        "9495",
        "9006",
        "6794",
        "8993",
        "5203",
        "16914",
        "6784",
        "9635",
        "22830",
        "20722",
        "9748",
        "6790",
        "9753",
        "9493",
        "9752",
        "9831",
        "9825",
        "9822",
        "5204",
        "5401",
        "22831",
        "6793",
        "9845",
        "17401",
        "9882",
        "9868",
        "20749",
        "9893",
        "9899",
        "9895",
        "9832",
        "9902",
        "9909",
        "22832",
        "9828",
        "9851",
        "9883",
        "9869",
        "17406",
        "17402",
        "9914",
        "20750",
        "9897",
        "9848",
        "3127",
        "107",
        "204",
        "9116",
        "2457",
        "78",
        "18848",
        "331",
        "403",
        "2098",
        "1752",
        "11278",
        "11288",
        "11284",
        "6461",
        "2344",
        "2345",
        "6463",
        "2346",
        "2352",
        "775",
        "1434",
        "1612",
        "71",
        "2468",
        "2458",
        "2467",
        "7164",
        "7178",
        "7367",
        "7376",
        "7381",
        "21156",
        "5209",
        "3029",
        "5201",
        "9849",
        "9850",
        "20719",
        "22568",
        "22827",
        "22828",
        "22829",
        "6809",
        "8972",
        "9005",
        "9823",
        "9827",
        "6783",
        "9913",
        "6785",
        "6787",
        "9866",
        "9867",
        "9894",
        "9896",
        "6800",
        "8992",
        "9829",
        "9830",
        "780",
        "769",
        "6749",
        "6750",
        "9755",
        "9754",
        "9908",
        "20745",
        "20742",
        "20747",
        "20748",
        "9746",
        "9745",
        "9880",
        "9881",
        "5391",
        "842",
        "3025",
        "3031",
        "3287",
        "3329",
        "1945",
        "3559",
        "4933",
        "4934",
        "4935",
        "4936",
        "5142",
        "5390",
        "5392",
        "5404",
        "5420",
        "6405",
        "7293",
        "7965",
        "8041",
        "8153",
        "9033",
        "9034",
        //"9036", problems with ghost state
        "16421",
        "21653",
        "22660",
        "5225",
        "9846",
        "2426",
        "5916",
        "6634",
        //"6718", phasing stealth, annoying for learn all case.
        "6719",
        "8822",
        "9591",
        "9590",
        "10032",
        "17746",
        "17747",
        "8203",
        "11392",
        "12495",
        "16380",
        "23452",
        "4079",
        "4996",
        "4997",
        "4998",
        "4999",
        "5000",
        "6348",
        "6349",
        "6481",
        "6482",
        "6483",
        "6484",
        "11362",
        "11410",
        "11409",
        "12510",
        "12509",
        "12885",
        "13142",
        "21463",
        "23460",
        "11421",
        "11416",
        "11418",
        "1851",
        "10059",
        "11423",
        "11417",
        "11422",
        "11419",
        "11424",
        "11420",
        "27",
        "31",
        "33",
        "34",
        "35",
        "15125",
        "21127",
        "22950",
        "1180",
        "201",
        "12593",
        "12842",
        "16770",
        "6057",
        "12051",
        "18468",
        "12606",
        "12605",
        "18466",
        "12502",
        "12043",
        "15060",
        "12042",
        "12341",
        "12848",
        "12344",
        "12353",
        "18460",
        "11366",
        "12350",
        "12352",
        "13043",
        "11368",
        "11113",
        "12400",
        "11129",
        "16766",
        "12573",
        "15053",
        "12580",
        "12475",
        "12472",
        "12953",
        "12488",
        "11189",
        "12985",
        "12519",
        "16758",
        "11958",
        "12490",
        "11426",
        "3565",
        "3562",
        "18960",
        "3567",
        "3561",
        "3566",
        "3563",
        "1953",
        "2139",
        "12505",
        "13018",
        "12522",
        "12523",
        "5146",
        "5144",
        "5148",
        "8419",
        "8418",
        "10213",
        "10212",
        "10157",
        "12524",
        "13019",
        "12525",
        "13020",
        "12526",
        "13021",
        "18809",
        "13031",
        "13032",
        "13033",
        "4036",
        "3920",
        "3919",
        "3918",
        "7430",
        "3922",
        "3923",
        "7411",
        "7418",
        "7421",
        "13262",
        "7412",
        "7415",
        "7413",
        "7416",
        "13920",
        "13921",
        "7745",
        "7779",
        "7428",
        "7457",
        "7857",
        "7748",
        "7426",
        "13421",
        "7454",
        "13378",
        "7788",
        "14807",
        "14293",
        "7795",
        "6296",
        "20608",
        "755",
        "444",
        "427",
        "428",
        "442",
        "447",
        "3578",
        "3581",
        "19027",
        "3580",
        "665",
        "3579",
        "3577",
        "6755",
        "3576",
        "2575",
        "2577",
        "2578",
        "2579",
        "2580",
        "2656",
        "2657",
        "2576",
        "3564",
        "10248",
        "8388",
        "2659",
        "14891",
        "3308",
        "3307",
        "10097",
        "2658",
        "3569",
        "16153",
        "3304",
        "10098",
        "4037",
        "3929",
        "3931",
        "3926",
        "3924",
        "3930",
        "3977",
        "3925",
        "136",
        "228",
        "5487",
        "43",
        "202",
        "0"
    };

    int loop = 0;
    while(strcmp(allSpellList[loop], "0"))
    {
        uint32 spell = atol((char*)allSpellList[loop++]);

        if (m_session->GetPlayer()->HasSpell(spell))
            continue;

        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spell);
        if(!spellInfo || !SpellMgr::IsSpellValid(spellInfo,m_session->GetPlayer()))
        {
            PSendSysMessage(LANG_COMMAND_SPELL_BROKEN,spell);
            continue;
        }

        m_session->GetPlayer()->learnSpell(spell);
    }

    SendSysMessage(LANG_COMMAND_LEARN_MANY_SPELLS);

    return true;
}

bool ChatHandler::HandleLearnAllGMCommand(const char* /*args*/)
{
    

    static const char *gmSpellList[] =
    {
        "24347",                                            // Become A Fish, No Breath Bar
        "35132",                                            // Visual Boom
        "38488",                                            // Attack 4000-8000 AOE
        "38795",                                            // Attack 2000 AOE + Slow Down 90%
        "15712",                                            // Attack 200
        "1852",                                             // GM Spell Silence
        "31899",                                            // Kill
        "31924",                                            // Kill
        "29878",                                            // Kill My Self
        "26644",                                            // More Kill

        "28550",                                            //Invisible 24
        "23452",                                            //Invisible + Target
        "0"
    };

    uint16 gmSpellIter = 0;
    while( strcmp(gmSpellList[gmSpellIter], "0") )
    {
        uint32 spell = atol((char const*)gmSpellList[gmSpellIter++]);

        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spell);
        if(!spellInfo || !SpellMgr::IsSpellValid(spellInfo,m_session->GetPlayer()))
        {
            PSendSysMessage(LANG_COMMAND_SPELL_BROKEN,spell);
            continue;
        }

        m_session->GetPlayer()->learnSpell(spell);
    }

    SendSysMessage(LANG_LEARNING_GM_SKILLS);
    return true;
}

bool ChatHandler::HandleLearnAllMyClassCommand(const char* /*args*/)
{
    HandleLearnAllMySpellsCommand("");
    HandleLearnAllMyTalentsCommand("");
    return true;
}

bool ChatHandler::HandleLearnAllMySpellsCommand(const char* /*args*/)
{
    m_session->GetPlayer()->LearnAllClassSpells();
    SendSysMessage(LANG_COMMAND_LEARN_CLASS_SPELLS);
    return true;
}

static void learnAllHighRanks(Player* player, uint32 spellid)
{
    SpellChainNode const* node;
    do
    {
        node = sSpellMgr->GetSpellChainNode(spellid);
        player->learnSpell(spellid);
        if (!node)
            break;
        spellid=node->next;
    }
    while (node->next);
}

bool ChatHandler::HandleLearnAllMyTalentsCommand(const char* /*args*/)
{
    

    Player* player = m_session->GetPlayer();
    uint32 classMask = player->GetClassMask();

    for (uint32 i = 0; i < sTalentStore.GetNumRows(); i++)
    {
        TalentEntry const *talentInfo = sTalentStore.LookupEntry(i);
        if(!talentInfo)
            continue;

        TalentTabEntry const *talentTabInfo = sTalentTabStore.LookupEntry( talentInfo->TalentTab );
        if(!talentTabInfo)
            continue;

        if( (classMask & talentTabInfo->ClassMask) == 0 )
            continue;

        // search highest talent rank
        uint32 spellid = 0;
        int rank = 4;
        for(; rank >= 0; --rank)
        {
            if(talentInfo->RankID[rank]!=0)
            {
                spellid = talentInfo->RankID[rank];
                break;
            }
        }

        if(!spellid)                                        // ??? none spells in talent
            continue;

        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellid);
        if(!spellInfo || !SpellMgr::IsSpellValid(spellInfo,m_session->GetPlayer(),false))
            continue;

        // learn highest rank of talent
        player->learnSpell(spellid);

        // and learn all non-talent spell ranks (recursive by tree)
        learnAllHighRanks(player,spellid);
    }

    SendSysMessage(LANG_COMMAND_LEARN_CLASS_TALENTS);
    return true;
}

bool ChatHandler::HandleLearnAllLangCommand(const char* /*args*/)
{
    

    // skipping UNIVERSAL language (0)
    for(int i = 1; i < LANGUAGES_COUNT; ++i)
        m_session->GetPlayer()->learnSpell(lang_description[i].spell_id);

    SendSysMessage(LANG_COMMAND_LEARN_ALL_LANG);
    return true;
}

bool ChatHandler::HandleLearnAllDefaultCommand(const char* args)
{
    char* pName = strtok((char*)args, "");
    Player *player = NULL;
    if (pName)
    {
        std::string name = pName;

        if(!normalizePlayerName(name))
        {
            SendSysMessage(LANG_PLAYER_NOT_FOUND);
            SetSentErrorMessage(true);
            return false;
        }

        player = sObjectAccessor->FindConnectedPlayerByName(name.c_str());
    }
    else
        player = getSelectedPlayer();

    if(!player)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    player->learnDefaultSpells();
    player->learnQuestRewardedSpells();

    PSendSysMessage(LANG_COMMAND_LEARN_ALL_DEFAULT_AND_QUEST,player->GetName().c_str());
    return true;
}

bool ChatHandler::HandleLearnCommand(const char* args)
{
    

    Player* targetPlayer = getSelectedPlayer();

    if(!targetPlayer)
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
    uint32 spell = extractSpellIdFromLink((char*)args);
    if(!spell || !sSpellMgr->GetSpellInfo(spell))
        return false;

    if (targetPlayer->HasSpell(spell))
    {
        if(targetPlayer == m_session->GetPlayer())
            SendSysMessage(LANG_YOU_KNOWN_SPELL);
        else
            PSendSysMessage(LANG_TARGET_KNOWN_SPELL,targetPlayer->GetName().c_str());
        SetSentErrorMessage(true);
        return false;
    }

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spell);
    if(!spellInfo || !SpellMgr::IsSpellValid(spellInfo,m_session->GetPlayer()))
    {
        PSendSysMessage(LANG_COMMAND_SPELL_BROKEN,spell);
        SetSentErrorMessage(true);
        return false;
    }

    targetPlayer->learnSpell(spell);

    return true;
}

bool ChatHandler::HandleAddItemCommand(const char* args)
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
                PSendSysMessage(LANG_COMMAND_COULDNOTFIND, citemName+1);
                SetSentErrorMessage(true);
                return false;
            }
            itemId = result->Fetch()->GetUInt16();
        }
        else
            return false;
    }
    else                                                    // item_id or [name] Shift-click form |color|Hitem:item_id:0:0:0|h[name]|h|r
    {
        char* cId = extractKeyFromLink((char*)args,"Hitem");
        if(!cId)
            return false;
        itemId = atol(cId);
    }

    char* ccount = strtok(NULL, " ");

    int32 count = 1;

    if (ccount)
        count = strtol(ccount, NULL, 10);

    if (count == 0)
        count = 1;

    Player* pl = m_session->GetPlayer();
    Player* plTarget = getSelectedPlayer();
    if(!plTarget)
        plTarget = pl;

    TC_LOG_DEBUG("command",GetTrinityString(LANG_ADDITEM), itemId, count);

    ItemTemplate const *pProto = sObjectMgr->GetItemTemplate(itemId);
    if(!pProto)
    {
        PSendSysMessage(LANG_COMMAND_ITEMIDINVALID, itemId);
        SetSentErrorMessage(true);
        return false;
    }

    //Subtract
    if (count < 0)
    {
        plTarget->DestroyItemCount(itemId, -count, true, false);
        PSendSysMessage(LANG_REMOVEITEM, itemId, -count, plTarget->GetName().c_str());
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
        PSendSysMessage(LANG_ITEM_CANNOT_CREATE, itemId, noSpaceForCount );
        SetSentErrorMessage(true);
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
        PSendSysMessage(LANG_ITEM_CANNOT_CREATE, itemId, noSpaceForCount);

    return true;
}

bool ChatHandler::HandleAddItemSetCommand(const char* args)
{
    
    ARGS_CHECK

    char* cId = extractKeyFromLink((char*)args,"Hitemset"); // number or [name] Shift-click form |color|Hitemset:itemset_id|h[name]|h|r
    if (!cId)
        return false;

    uint32 itemsetId = atol(cId);

    // prevent generation all items with itemset field value '0'
    if (itemsetId == 0)
    {
        PSendSysMessage(LANG_NO_ITEMS_FROM_ITEMSET_FOUND,itemsetId);
        SetSentErrorMessage(true);
        return false;
    }

    Player* pl = m_session->GetPlayer();
    Player* plTarget = getSelectedPlayer();
    if(!plTarget)
        plTarget = pl;

    TC_LOG_DEBUG("command",GetTrinityString(LANG_ADDITEMSET), itemsetId);

    bool found = false;

    ItemTemplateContainer const* its = sObjectMgr->GetItemTemplateStore();
    for (ItemTemplateContainer::const_iterator itr = its->begin(); itr != its->end(); ++itr)
    {
        uint32 id = itr->first;
        ItemTemplate const *pProto = &(itr->second);
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
                pl->SendEquipError( msg, NULL, NULL );
                PSendSysMessage(LANG_ITEM_CANNOT_CREATE, pProto->ItemId, 1);
            }
        }
    }

    if (!found)
    {
        PSendSysMessage(LANG_NO_ITEMS_FROM_ITEMSET_FOUND,itemsetId);

        SetSentErrorMessage(true);
        return false;
    }

    return true;
}

bool ChatHandler::HandleListItemCommand(const char* args)
{
    ARGS_CHECK

    char* cId = extractKeyFromLink((char*)args,"Hitem");
    if(!cId)
        return false;

    uint32 item_id = atol(cId);
    if(!item_id)
    {
        PSendSysMessage(LANG_COMMAND_ITEMIDINVALID, item_id);
        SetSentErrorMessage(true);
        return false;
    }

    ItemTemplate const* itemProto = sObjectMgr->GetItemTemplate(item_id);
    if(!itemProto)
    {
        PSendSysMessage(LANG_COMMAND_ITEMIDINVALID, item_id);
        SetSentErrorMessage(true);
        return false;
    }

    char* c_count = strtok(NULL, " ");
    int count = c_count ? atol(c_count) : 10;

    if(count < 0)
        return false;

    QueryResult result;

    // inventory case
    uint32 inv_count = 0;
    result=CharacterDatabase.PQuery("SELECT COUNT(item_template) FROM character_inventory WHERE item_template='%u'",item_id);
    if(result)
    {
        inv_count = (*result)[0].GetUInt32();
    }

    result=CharacterDatabase.PQuery(
    //          0        1             2             3        4                  5
        "SELECT ci.item, cibag.slot AS bag, ci.slot, ci.guid, characters.account,characters.name "
        "FROM character_inventory AS ci LEFT JOIN character_inventory AS cibag ON (cibag.item=ci.bag),characters "
        "WHERE ci.item_template='%u' AND ci.guid = characters.guid LIMIT %u ",
        item_id,uint32(count));

    if(result)
    {
        do
        {
            Field *fields = result->Fetch();
            uint32 item_guid = fields[0].GetUInt32();
            uint32 item_bag = fields[1].GetUInt32();
            uint32 item_slot = fields[2].GetUInt32();
            uint32 owner_guid = fields[3].GetUInt32();
            uint32 owner_acc = fields[4].GetUInt32();
            std::string owner_name = fields[5].GetString();

            char const* item_pos = 0;
            if(Player::IsEquipmentPos(item_bag,item_slot))
                item_pos = "[equipped]";
            else if(Player::IsInventoryPos(item_bag,item_slot))
                item_pos = "[in inventory]";
            else if(Player::IsBankPos(item_bag,item_slot))
                item_pos = "[in bank]";
            else
                item_pos = "";

            PSendSysMessage(LANG_ITEMLIST_SLOT,
                item_guid,owner_name.c_str(),owner_guid,owner_acc,item_pos);
        } while (result->NextRow());

        int64 res_count = result->GetRowCount();

        if(count > res_count)
            count-=res_count;
        else if(count)
            count = 0;
    }

    // mail case
    uint32 mail_count = 0;
    result=CharacterDatabase.PQuery("SELECT COUNT(item_template) FROM mail_items WHERE item_template='%u'", item_id);
    if(result)
    {
        mail_count = (*result)[0].GetUInt32();
    }

    if(count > 0)
    {
        result=CharacterDatabase.PQuery(
        //          0                     1            2              3               4            5               6
            "SELECT mail_items.item_guid, mail.sender, mail.receiver, char_s.account, char_s.name, char_r.account, char_r.name "
            "FROM mail,mail_items,characters as char_s,characters as char_r "
            "WHERE mail_items.item_template='%u' AND char_s.guid = mail.sender AND char_r.guid = mail.receiver AND mail.id=mail_items.mail_id LIMIT %u",
            item_id,uint32(count));
    }
    else
        result = NULL;

    if(result)
    {
        do
        {
            Field *fields = result->Fetch();
            uint32 item_guid        = fields[0].GetUInt32();
            uint32 item_s           = fields[1].GetUInt32();
            uint32 item_r           = fields[2].GetUInt32();
            uint32 item_s_acc       = fields[3].GetUInt32();
            std::string item_s_name = fields[4].GetString();
            uint32 item_r_acc       = fields[5].GetUInt32();
            std::string item_r_name = fields[6].GetString();

            char const* item_pos = "[in mail]";

            PSendSysMessage(LANG_ITEMLIST_MAIL,
                item_guid,item_s_name.c_str(),item_s,item_s_acc,item_r_name.c_str(),item_r,item_r_acc,item_pos);
        } while (result->NextRow());

        int64 res_count = result->GetRowCount();

        if(count > res_count)
            count-=res_count;
        else if(count)
            count = 0;
    }

    // auction case
    uint32 auc_count = 0;
    result=CharacterDatabase.PQuery("SELECT COUNT(item_template) FROM auctionhouse WHERE item_template='%u'",item_id);
    if(result)
    {
        auc_count = (*result)[0].GetUInt32();
    }

    if(count > 0)
    {
        result=CharacterDatabase.PQuery(
        //           0                      1                       2                   3
            "SELECT  auctionhouse.itemguid, auctionhouse.itemowner, characters.account, characters.name "
            "FROM auctionhouse,characters WHERE auctionhouse.item_template='%u' AND characters.guid = auctionhouse.itemowner LIMIT %u",
            item_id,uint32(count));
    }
    else
        result = NULL;

    if(result)
    {
        do
        {
            Field *fields = result->Fetch();
            uint32 item_guid       = fields[0].GetUInt32();
            uint32 owner           = fields[1].GetUInt32();
            uint32 owner_acc       = fields[2].GetUInt32();
            std::string owner_name = fields[3].GetString();

            char const* item_pos = "[in auction]";

            PSendSysMessage(LANG_ITEMLIST_AUCTION, item_guid, owner_name.c_str(), owner, owner_acc,item_pos);
        } while (result->NextRow());
    }

    // guild bank case
    uint32 guild_count = 0;
    result=CharacterDatabase.PQuery("SELECT COUNT(item_entry) FROM guild_bank_item WHERE item_entry='%u'",item_id);
    if(result)
    {
        guild_count = (*result)[0].GetUInt32();
    }

    result=CharacterDatabase.PQuery(
        //      0             1           2
        "SELECT gi.item_guid, gi.guildid, guild.name "
        "FROM guild_bank_item AS gi, guild WHERE gi.item_entry='%u' AND gi.guildid = guild.guildid LIMIT %u ",
        item_id,uint32(count));

    if(result)
    {
        do
        {
            Field *fields = result->Fetch();
            uint32 item_guid = fields[0].GetUInt32();
            uint32 guild_guid = fields[1].GetUInt32();
            std::string guild_name = fields[2].GetString();

            char const* item_pos = "[in guild bank]";

            PSendSysMessage(LANG_ITEMLIST_GUILD,item_guid,guild_name.c_str(),guild_guid,item_pos);
        } while (result->NextRow());

        int64 res_count = result->GetRowCount();

        if(count > res_count)
            count-=res_count;
        else if(count)
            count = 0;
    }

    if(inv_count+mail_count+auc_count+guild_count == 0)
    {
        SendSysMessage(LANG_COMMAND_NOITEMFOUND);
        SetSentErrorMessage(true);
        return false;
    }

    PSendSysMessage(LANG_COMMAND_LISTITEMMESSAGE,item_id,inv_count+mail_count+auc_count+guild_count,inv_count,mail_count,auc_count,guild_count);

    return true;
}

bool ChatHandler::HandleListObjectCommand(const char* args)
{
    ARGS_CHECK

    // number or [name] Shift-click form |color|Hgameobject_entry:go_id|h[name]|h|r
    char* cId = extractKeyFromLink((char*)args,"Hgameobject_entry");
    if(!cId)
        return false;

    uint32 go_id = atol(cId);
    if(!go_id)
    {
        PSendSysMessage(LANG_COMMAND_LISTOBJINVALIDID, go_id);
        SetSentErrorMessage(true);
        return false;
    }

    GameObjectTemplate const * gInfo = sObjectMgr->GetGameObjectTemplate(go_id);
    if(!gInfo)
    {
        PSendSysMessage(LANG_COMMAND_LISTOBJINVALIDID, go_id);
        SetSentErrorMessage(true);
        return false;
    }

    char* c_count = strtok(NULL, " ");
    int count = c_count ? atol(c_count) : 10;

    if(count < 0)
        return false;

    QueryResult result;

    uint32 obj_count = 0;
    result=WorldDatabase.PQuery("SELECT COUNT(guid) FROM gameobject WHERE id='%u'",go_id);
    if(result)
    {
        obj_count = (*result)[0].GetUInt32();
    }

    if(m_session)
    {
        Player* pl = m_session->GetPlayer();
        result = WorldDatabase.PQuery("SELECT guid, position_x, position_y, position_z, map, id, (POW(position_x - '%f', 2) + POW(position_y - '%f', 2) + POW(position_z - '%f', 2)) AS order_ FROM gameobject WHERE id = '%u' ORDER BY order_ ASC LIMIT %u",
            pl->GetPositionX(), pl->GetPositionY(), pl->GetPositionZ(),go_id,uint32(count));
    }
    else
        result = WorldDatabase.PQuery("SELECT guid, position_x, position_y, position_z, map FROM gameobject WHERE id = '%u' LIMIT %u",
            go_id,uint32(count));

    if (result)
    {
        do
        {
            Field *fields = result->Fetch();
            uint32 guid = fields[0].GetUInt32();
            float x = fields[1].GetFloat();
            float y = fields[2].GetFloat();
            float z = fields[3].GetFloat();
            int mapid = fields[4].GetUInt16();
            uint32 id = fields[5].GetUInt32();

            if (m_session)
                PSendSysMessage(LANG_GO_LIST_CHAT, guid, id, guid, gInfo->name.c_str(), x, y, z, mapid);
            else
                PSendSysMessage(LANG_GO_LIST_CONSOLE, guid, gInfo->name.c_str(), x, y, z, mapid);
        } while (result->NextRow());
    }

    PSendSysMessage(LANG_COMMAND_LISTOBJMESSAGE,go_id,obj_count);
    return true;
}

bool ChatHandler::HandleNearObjectCommand(const char* args)
{
    

    float distance = (!*args) ? 10 : atol(args);
    uint32 count = 0;

    Player* pl = m_session->GetPlayer();
    QueryResult result = WorldDatabase.PQuery("SELECT guid, id, position_x, position_y, position_z, map, "
        "(POW(position_x - '%f', 2) + POW(position_y - '%f', 2) + POW(position_z - '%f', 2)) AS order_ "
        "FROM gameobject WHERE map='%u' AND (POW(position_x - '%f', 2) + POW(position_y - '%f', 2) + POW(position_z - '%f', 2)) <= '%f' ORDER BY order_",
        pl->GetPositionX(), pl->GetPositionY(), pl->GetPositionZ(),
        pl->GetMapId(),pl->GetPositionX(), pl->GetPositionY(), pl->GetPositionZ(),distance*distance);

    if (result)
    {
        do
        {
            Field *fields = result->Fetch();
            uint32 guid = fields[0].GetUInt32();
            uint32 entry = fields[1].GetUInt32();
            float x = fields[2].GetFloat();
            float y = fields[3].GetFloat();
            float z = fields[4].GetFloat();
            int mapid = fields[5].GetUInt16();

            GameObjectTemplate const * gInfo = sObjectMgr->GetGameObjectTemplate(entry);

            if(!gInfo)
                continue;

            PSendSysMessage(LANG_GO_LIST_CHAT, guid, entry, guid, gInfo->name.c_str(), x, y, z, mapid);

            ++count;
        } while (result->NextRow());
    }

    PSendSysMessage(LANG_COMMAND_NEAROBJMESSAGE,distance,count);
    return true;
}

bool ChatHandler::HandleObjectStateCommand(const char* args)
{
    // number or [name] Shift-click form |color|Hgameobject:go_id|h[name]|h|r
    char* cId = extractKeyFromLink((char*)args, "Hgameobject");
    if(!cId)
        return false;

    uint32 lowguid = atoi(cId);
    if(!lowguid)
        return false;

    GameObject* gobj = NULL;

    if(GameObjectData const* goData = sObjectMgr->GetGOData(lowguid))
        gobj = GetObjectGlobalyWithGuidOrNearWithDbGuid(lowguid, goData->id);

    if(!gobj)
    {
        PSendSysMessage(LANG_COMMAND_OBJNOTFOUND, lowguid);
        SetSentErrorMessage(true);
        return false;
    }

    char* cstate = strtok(NULL, " ");
    if(!cstate)
        return false;

    int32 state = atoi(cstate);
    if(state < 0)
        gobj->SendObjectDeSpawnAnim(gobj->GetGUID());
    else
        gobj->SetGoState(state);

    return true;

    return true;
}

bool ChatHandler::HandleListCreatureCommand(const char* args)
{
    ARGS_CHECK

    // number or [name] Shift-click form |color|Hcreature_entry:creature_id|h[name]|h|r
    char* cId = extractKeyFromLink((char*)args,"Hcreature_entry");
    if(!cId)
        return false;

    uint32 cr_id = atol(cId);
    if(!cr_id)
    {
        PSendSysMessage(LANG_COMMAND_INVALIDCREATUREID, cr_id);
        SetSentErrorMessage(true);
        return false;
    }

    CreatureTemplate const* cInfo = sObjectMgr->GetCreatureTemplate(cr_id);
    if(!cInfo)
    {
        PSendSysMessage(LANG_COMMAND_INVALIDCREATUREID, cr_id);
        SetSentErrorMessage(true);
        return false;
    }

    char* c_count = strtok(NULL, " ");
    int count = c_count ? atol(c_count) : 10;

    if(count < 0)
        return false;

    QueryResult result;

    uint32 cr_count = 0;
    result=WorldDatabase.PQuery("SELECT COUNT(guid) FROM creature WHERE id='%u'",cr_id);
    if(result)
    {
        cr_count = (*result)[0].GetUInt32();
    }

    if(m_session)
    {
        Player* pl = m_session->GetPlayer();
        result = WorldDatabase.PQuery("SELECT guid, position_x, position_y, position_z, map, (POW(position_x - '%f', 2) + POW(position_y - '%f', 2) + POW(position_z - '%f', 2)) AS order_ FROM creature WHERE id = '%u' ORDER BY order_ ASC LIMIT %u",
            pl->GetPositionX(), pl->GetPositionY(), pl->GetPositionZ(), cr_id,uint32(count));
    }
    else
        result = WorldDatabase.PQuery("SELECT guid, position_x, position_y, position_z, map FROM creature WHERE id = '%u' LIMIT %u",
            cr_id,uint32(count));

    if (result)
    {
        do
        {
            Field *fields = result->Fetch();
            uint32 guid = fields[0].GetUInt32();
            float x = fields[1].GetFloat();
            float y = fields[2].GetFloat();
            float z = fields[3].GetFloat();
            int mapid = fields[4].GetUInt16();

            if  (m_session)
                PSendSysMessage(LANG_CREATURE_LIST_CHAT, guid, guid, cInfo->Name.c_str(), x, y, z, mapid);
            else
                PSendSysMessage(LANG_CREATURE_LIST_CONSOLE, guid, cInfo->Name.c_str(), x, y, z, mapid);
        } while (result->NextRow());
    }

    PSendSysMessage(LANG_COMMAND_LISTCREATUREMESSAGE,cr_id,cr_count);
    return true;
}

bool ChatHandler::HandleLookupItemCommand(const char* args)
{
    ARGS_CHECK

    std::string namepart = args;
    std::wstring wnamepart;

    // converting string that we try to find to lower case
    if(!Utf8toWStr(namepart,wnamepart))
        return false;

    wstrToLower(wnamepart);

    uint32 counter = 0;

    // Search in `item_template`
    ItemTemplateContainer const* its = sObjectMgr->GetItemTemplateStore();
    for (ItemTemplateContainer::const_iterator itr = its->begin(); itr != its->end(); ++itr)
    {
        uint32 id = itr->first;
        ItemTemplate const *pProto = &(itr->second);
        if(!pProto)
            continue;

        LocaleConstant loc_idx = GetSessionDbcLocale();
        if ( loc_idx >= 0 )
        {
            ItemLocale const *il = sObjectMgr->GetItemLocale(pProto->ItemId);
            if (il)
            {
                if (il->Name.size() > loc_idx && !il->Name[loc_idx].empty())
                {
                    std::string name = il->Name[loc_idx];

                    if (Utf8FitTo(name, wnamepart))
                    {
                        if (m_session)
                            PSendSysMessage(LANG_ITEM_LIST_CHAT, id, id, name.c_str());
                        else
                            PSendSysMessage(LANG_ITEM_LIST_CONSOLE, id, name.c_str());
                        ++counter;
                        continue;
                    }
                }
            }
        }

        std::string name = pProto->Name1;
        if(name.empty())
            continue;

        if (Utf8FitTo(name, wnamepart))
        {
            if (m_session)
                PSendSysMessage(LANG_ITEM_LIST_CHAT, id, id, name.c_str());
            else
                PSendSysMessage(LANG_ITEM_LIST_CONSOLE, id, name.c_str());
            ++counter;
        }
    }

    if (counter==0)
        SendSysMessage(LANG_COMMAND_NOITEMFOUND);

    return true;
}

bool ChatHandler::HandleLookupItemSetCommand(const char* args)
{
    ARGS_CHECK

    std::string namepart = args;
    std::wstring wnamepart;

    if(!Utf8toWStr(namepart,wnamepart))
        return false;

    // converting string that we try to find to lower case
    wstrToLower( wnamepart );

    uint32 counter = 0;                                     // Counter for figure out that we found smth.

    // Search in ItemSet.dbc
    for (uint32 id = 0; id < sItemSetStore.GetNumRows(); id++)
    {
        ItemSetEntry const *set = sItemSetStore.LookupEntry(id);
        if(set)
        {
            int loc = GetSessionDbcLocale();
            std::string name = set->name[loc];
            if(name.empty())
                continue;

            if (!Utf8FitTo(name, wnamepart))
            {
                loc = 0;
                for(; loc < TOTAL_LOCALES; ++loc)
                {
                    if(GetSessionDbcLocale())
                        continue;

                    name = set->name[loc];
                    if(name.empty())
                        continue;

                    if (Utf8FitTo(name, wnamepart))
                        break;
                }
            }

            if(loc < TOTAL_LOCALES)
            {
                // send item set in "id - [namedlink locale]" format
                if (m_session)
                    PSendSysMessage(LANG_ITEMSET_LIST_CHAT,id,id,name.c_str(),localeNames[loc]);
                else
                    PSendSysMessage(LANG_ITEMSET_LIST_CONSOLE,id,name.c_str(),localeNames[loc]);
                ++counter;
            }
        }
    }
    if (counter == 0)                                       // if counter == 0 then we found nth
        SendSysMessage(LANG_COMMAND_NOITEMSETFOUND);
    return true;
}

bool ChatHandler::HandleLookupSkillCommand(const char* args)
{
    ARGS_CHECK

    // can be NULL in console call
    Player* target = getSelectedPlayer();

    std::string namepart = args;
    std::wstring wnamepart;

    if(!Utf8toWStr(namepart,wnamepart))
        return false;

    // converting string that we try to find to lower case
    wstrToLower( wnamepart );

    uint32 counter = 0;                                     // Counter for figure out that we found smth.

    // Search in SkillLine.dbc
    for (uint32 id = 0; id < sSkillLineStore.GetNumRows(); id++)
    {
        SkillLineEntry const *skillInfo = sSkillLineStore.LookupEntry(id);
        if(skillInfo)
        {
            int loc = GetSessionDbcLocale();
            std::string name = skillInfo->name[loc];
            if(name.empty())
                continue;

            if (!Utf8FitTo(name, wnamepart))
            {
                loc = 0;
                for(; loc < TOTAL_LOCALES; ++loc)
                {
                    if(GetSessionDbcLocale())
                        continue;

                    name = skillInfo->name[loc];
                    if(name.empty())
                        continue;

                    if (Utf8FitTo(name, wnamepart))
                        break;
                }
            }

            if(loc < TOTAL_LOCALES)
            {
                char const* knownStr = "";
                if(target && target->HasSkill(id))
                    knownStr = GetTrinityString(LANG_KNOWN);

                // send skill in "id - [namedlink locale]" format
                if (m_session)
                    PSendSysMessage(LANG_SKILL_LIST_CHAT,id,id,name.c_str(),localeNames[loc],knownStr);
                else
                    PSendSysMessage(LANG_SKILL_LIST_CONSOLE,id,name.c_str(),localeNames[loc],knownStr);

                ++counter;
            }
        }
    }
    if (counter == 0)                                       // if counter == 0 then we found nth
        SendSysMessage(LANG_COMMAND_NOSKILLFOUND);
    return true;
}

bool ChatHandler::HandleGetSpellInfoCommand(const char* args)
{
    ARGS_CHECK

    // can be NULL at console call
    Player* target = getSelectedPlayer();

    std::string namepart = args;
    std::wstring wnamepart;

    if(!Utf8toWStr(namepart,wnamepart))
        return false;

    // converting string that we try to find to lower case
    wstrToLower( wnamepart );

    uint32 counter = 0;                                     // Counter for figure out that we found smth.

    // Search in Spell.dbc
    //for (uint32 id = 0; id < sSpellStore.GetNumRows(); id++)
    for (uint32 id = 0; id < sObjectMgr->GetSpellStore().size(); id++)
    {
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(id);
        if(spellInfo)
        {
            int loc = GetSessionDbcLocale();
            std::string name = spellInfo->SpellName[loc];
            if(name.empty())
                continue;

            if (!Utf8FitTo(name, wnamepart))
            {
                loc = 0;
                for(; loc < TOTAL_LOCALES; ++loc)
                {
                    if(GetSessionDbcLocale())
                        continue;

                    name = spellInfo->SpellName[loc];
                    if(name.empty())
                        continue;

                    if (Utf8FitTo(name, wnamepart))
                        break;
                }
            }

            if(loc < TOTAL_LOCALES)
            {
                bool known = target && target->HasSpell(id);
                bool learn = (spellInfo->Effects[0].Effect == SPELL_EFFECT_LEARN_SPELL);

                uint32 talentCost = GetTalentSpellCost(id);

                bool talent = (talentCost > 0);
                bool passive = spellInfo->IsPassive();
                bool active = target && (target->HasAura(id,0) || target->HasAura(id,1) || target->HasAura(id,2));

                // unit32 used to prevent interpreting uint8 as char at output
                // find rank of learned spell for learning spell, or talent rank
                uint32 rank = talentCost ? talentCost : sSpellMgr->GetSpellRank(learn ? spellInfo->Effects[0].TriggerSpell : id);

                // send spell in "id - [name, rank N] [talent] [passive] [learn] [known]" format
                std::ostringstream ss;
                if (m_session)
                    ss << id << " - |cffffffff|Hspell:" << id << "|h[" << name;
                else
                    ss << id << " - " << name;

                // include rank in link name
                if(rank)
                    ss << GetTrinityString(LANG_SPELL_RANK) << rank;

                if (m_session)
                    ss << " " << localeNames[loc] << "]|h|r";
                else
                    ss << " " << localeNames[loc];

                if(talent)
                    ss << GetTrinityString(LANG_TALENT);
                if(passive)
                    ss << GetTrinityString(LANG_PASSIVE);
                if(learn)
                    ss << GetTrinityString(LANG_LEARN);
                if(known)
                    ss << GetTrinityString(LANG_KNOWN);
                if(active)
                    ss << GetTrinityString(LANG_ACTIVE);

                SendSysMessage(ss.str().c_str());

                ++counter;
            }
        }
    }
    if (counter == 0)                                       // if counter == 0 then we found nth
        SendSysMessage(LANG_COMMAND_NOSPELLFOUND);
    return true;
}

bool ChatHandler::HandleLookupQuestCommand(const char* args)
{
    ARGS_CHECK

    // can be NULL at console call
    Player* target = getSelectedPlayer();

    std::string namepart = args;
    std::wstring wnamepart;

    // converting string that we try to find to lower case
    if(!Utf8toWStr(namepart,wnamepart))
        return false;

    wstrToLower(wnamepart);

    uint32 counter = 0 ;

    ObjectMgr::QuestMap const& qTemplates = sObjectMgr->GetQuestTemplates();
    for (ObjectMgr::QuestMap::const_iterator iter = qTemplates.begin(); iter != qTemplates.end(); ++iter)
    {
        Quest * qinfo = iter->second;

        LocaleConstant loc_idx = GetSessionDbcLocale();
        if ( loc_idx >= 0 )
        {
            QuestLocale const *il = sObjectMgr->GetQuestLocale(qinfo->GetQuestId());
            if (il)
            {
                if (il->Title.size() > loc_idx && !il->Title[loc_idx].empty())
                {
                    std::string title = il->Title[loc_idx];

                    if (Utf8FitTo(title, wnamepart))
                    {
                        char const* statusStr = "";

                        if(target)
                        {
                            QuestStatus status = target->GetQuestStatus(qinfo->GetQuestId());

                            if(status == QUEST_STATUS_COMPLETE)
                            {
                                if(target->GetQuestRewardStatus(qinfo->GetQuestId()))
                                    statusStr = GetTrinityString(LANG_COMMAND_QUEST_REWARDED);
                                else
                                    statusStr = GetTrinityString(LANG_COMMAND_QUEST_COMPLETE);
                            }
                            else if(status == QUEST_STATUS_INCOMPLETE)
                                statusStr = GetTrinityString(LANG_COMMAND_QUEST_ACTIVE);
                        }

                        if (m_session)
                            PSendSysMessage(LANG_QUEST_LIST_CHAT,qinfo->GetQuestId(),qinfo->GetQuestId(),title.c_str(),statusStr);
                        else
                            PSendSysMessage(LANG_QUEST_LIST_CONSOLE,qinfo->GetQuestId(),title.c_str(),statusStr);

                        QueryResult result = WorldDatabase.PQuery("SELECT bugged, comment FROM quest_bugs WHERE entry = %u", qinfo->GetQuestId());
                        if (result) 
                        {
                            Field* fields = result->Fetch();
                            if (fields[0].GetUInt8())
                                //PSendSysMessage(" -> L'autovalidation est activée pour cette quête.");
                                PSendSysMessage(" -> Auto validation is activated for this quest.");

                            std::string x = "";
                            if(fields[1].GetString() != x)
                                PSendSysMessage(" -> BUG: %s", fields[1].GetString().c_str());
                        }
                        ++counter;
                        continue;
                    }
                }
            }
        }

        std::string title = qinfo->GetTitle();
        if(title.empty())
            continue;

        if (Utf8FitTo(title, wnamepart))
        {
            char const* statusStr = "";

            if(target)
            {
                QuestStatus status = target->GetQuestStatus(qinfo->GetQuestId());

                if(status == QUEST_STATUS_COMPLETE)
                {
                    if(target->GetQuestRewardStatus(qinfo->GetQuestId()))
                        statusStr = GetTrinityString(LANG_COMMAND_QUEST_REWARDED);
                    else
                        statusStr = GetTrinityString(LANG_COMMAND_QUEST_COMPLETE);
                }
                else if(status == QUEST_STATUS_INCOMPLETE)
                    statusStr = GetTrinityString(LANG_COMMAND_QUEST_ACTIVE);
            }

            if (m_session)
                PSendSysMessage(LANG_QUEST_LIST_CHAT,qinfo->GetQuestId(),qinfo->GetQuestId(),title.c_str(),statusStr);
            else
                PSendSysMessage(LANG_QUEST_LIST_CONSOLE,qinfo->GetQuestId(),title.c_str(),statusStr);

            ++counter;
        }
    }

    if (counter==0)
        SendSysMessage(LANG_COMMAND_NOQUESTFOUND);

    return true;
}

bool ChatHandler::HandleLookupCreatureCommand(const char* args)
{
    ARGS_CHECK

    std::string namepart = args;
    std::wstring wnamepart;

    // converting string that we try to find to lower case
    if (!Utf8toWStr (namepart,wnamepart))
        return false;

    wstrToLower (wnamepart);

    uint32 counter = 0;

    CreatureTemplateContainer const* ctc = sObjectMgr->GetCreatureTemplateStore();
    for (CreatureTemplateContainer::const_iterator itr = ctc->begin(); itr != ctc->end(); ++itr)
    {
        uint32 id = itr->second.Entry;
        CreatureTemplate const* cInfo = &(itr->second);
        if(!cInfo)
            continue;

        LocaleConstant loc_idx = GetSessionDbcLocale();
        if (loc_idx >= 0)
        {
            CreatureLocale const *cl = sObjectMgr->GetCreatureLocale (id);
            if (cl)
            {
                if (cl->Name.size() > loc_idx && !cl->Name[loc_idx].empty ())
                {
                    std::string name = cl->Name[loc_idx];

                    if (Utf8FitTo (name, wnamepart))
                    {
                        if (m_session)
                            PSendSysMessage (LANG_CREATURE_ENTRY_LIST_CHAT, id, id, name.c_str ());
                        else
                            PSendSysMessage (LANG_CREATURE_ENTRY_LIST_CONSOLE, id, name.c_str ());
                        ++counter;
                        continue;
                    }
                }
            }
        }

        std::string name = cInfo->Name;
        if (name.empty ())
            continue;

        if (Utf8FitTo(name, wnamepart))
        {
            if (m_session)
                PSendSysMessage (LANG_CREATURE_ENTRY_LIST_CHAT, id, id, name.c_str ());
            else
                PSendSysMessage (LANG_CREATURE_ENTRY_LIST_CONSOLE, id, name.c_str ());
            ++counter;
        }
    }

    if (counter==0)
        SendSysMessage (LANG_COMMAND_NOCREATUREFOUND);

    return true;
}

bool ChatHandler::HandleLookupObjectCommand(const char* args)
{
    ARGS_CHECK

    std::string namepart = args;
    std::wstring wnamepart;

    // converting string that we try to find to lower case
    if(!Utf8toWStr(namepart,wnamepart))
        return false;

    wstrToLower(wnamepart);

    uint32 counter = 0;
    
    
    GameObjectTemplateContainer const* gotc = sObjectMgr->GetGameObjectTemplateStore();
    for (GameObjectTemplateContainer::const_iterator itr = gotc->begin(); itr != gotc->end(); ++itr)
    {
        uint32 id = itr->first;
        GameObjectTemplate const* gInfo = &(itr->second);
        if(!gInfo)
            continue;

        LocaleConstant loc_idx = GetSessionDbcLocale();
        if ( loc_idx >= 0 )
        {
            GameObjectLocale const *gl = sObjectMgr->GetGameObjectLocale(id);
            if (gl)
            {
                if (gl->Name.size() > loc_idx && !gl->Name[loc_idx].empty())
                {
                    std::string name = gl->Name[loc_idx];

                    if (Utf8FitTo(name, wnamepart))
                    {
                        if (m_session)
                            PSendSysMessage(LANG_GO_ENTRY_LIST_CHAT, id, id, name.c_str());
                        else
                            PSendSysMessage(LANG_GO_ENTRY_LIST_CONSOLE, id, name.c_str());
                        ++counter;
                        continue;
                    }
                }
            }
        }

        std::string name = gInfo->name;
        if(name.empty())
            continue;

        if(Utf8FitTo(name, wnamepart))
        {
            if (m_session)
                PSendSysMessage(LANG_GO_ENTRY_LIST_CHAT, id, id, name.c_str());
            else
                PSendSysMessage(LANG_GO_ENTRY_LIST_CONSOLE, id, name.c_str());
            ++counter;
        }
    }

    if(counter==0)
        SendSysMessage(LANG_COMMAND_NOGAMEOBJECTFOUND);

    return true;
}

/** \brief GM command level 3 - Create a guild.
 *
 * This command allows a GM (level 3) to create a guild.
 *
 * The "args" parameter contains the name of the guild leader
 * and then the name of the guild.
 *
 */
bool ChatHandler::HandleGuildCreateCommand(const char* args)
{

    ARGS_CHECK

    char *lname = strtok ((char*)args, " ");
    char *gname = strtok (NULL, "");

    if (!lname)
        return false;

    if (!gname)
    {
        SendSysMessage (LANG_INSERT_GUILD_NAME);
        SetSentErrorMessage (true);
        return false;
    }

    std::string guildname = gname;

    Player* player = sObjectAccessor->FindPlayerByName (lname);
    if (!player)
    {
        SendSysMessage (LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage (true);
        return false;
    }

    if (player->GetGuildId())
    {
        SendSysMessage (LANG_PLAYER_IN_GUILD);
        return true;
    }

    Guild *guild = new Guild;
    if (!guild->create (player->GetGUID (),guildname))
    {
        delete guild;
        SendSysMessage (LANG_GUILD_NOT_CREATED);
        SetSentErrorMessage (true);
        return false;
    }

    sObjectMgr->AddGuild (guild);
    return true;
}

bool ChatHandler::HandleGuildInviteCommand(const char *args)
{
    ARGS_CHECK

    char* par1 = strtok ((char*)args, " ");
    char* par2 = strtok (NULL, "");
    if(!par1 || !par2)
        return false;

    std::string glName = par2;
    Guild* targetGuild = sObjectMgr->GetGuildByName (glName);
    if (!targetGuild)
        return false;

    std::string plName = par1;
    if (!normalizePlayerName (plName))
    {
        SendSysMessage (LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage (true);
        return false;
    }

    uint64 plGuid = 0;
    if (Player* targetPlayer = sObjectAccessor->FindPlayerByName (plName.c_str ()))
        plGuid = targetPlayer->GetGUID ();
    else
        plGuid = sObjectMgr->GetPlayerGUIDByName (plName.c_str ());

    if (!plGuid)
        false;

    // player's guild membership checked in AddMember before add
    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    if (!targetGuild->AddMember (plGuid,targetGuild->GetLowestRank (), trans))
        return false;
    CharacterDatabase.CommitTransaction(trans);

    return true;
}

bool ChatHandler::HandleGuildUninviteCommand(const char *args)
{
    ARGS_CHECK

    char* par1 = strtok ((char*)args, " ");
    if(!par1)
        return false;

    std::string plName = par1;
    if (!normalizePlayerName (plName))
    {
        SendSysMessage (LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage (true);
        return false;
    }

    uint64 plGuid = 0;
    uint32 glId   = 0;
    if (Player* targetPlayer = sObjectAccessor->FindPlayerByName (plName.c_str ()))
    {
        plGuid = targetPlayer->GetGUID ();
        glId   = targetPlayer->GetGuildId ();
    }
    else
    {
        plGuid = sObjectMgr->GetPlayerGUIDByName (plName.c_str ());
        glId = Player::GetGuildIdFromDB (plGuid);
    }

    if (!plGuid || !glId)
        return false;

    Guild* targetGuild = sObjectMgr->GetGuildById (glId);
    if (!targetGuild)
        return false;

    targetGuild->DelMember (plGuid);

    return true;
}

bool ChatHandler::HandleGuildRankCommand(const char *args)
{
    ARGS_CHECK

    char* par1 = strtok ((char*)args, " ");
    char* par2 = strtok (NULL, " ");
    if (!par1 || !par2)
        return false;
    std::string plName = par1;
    if (!normalizePlayerName (plName))
    {
        SendSysMessage (LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage (true);
        return false;
    }

    uint64 plGuid = 0;
    uint32 glId   = 0;
    if (Player* targetPlayer = sObjectAccessor->FindPlayerByName (plName.c_str ()))
    {
        plGuid = targetPlayer->GetGUID ();
        glId   = targetPlayer->GetGuildId ();
    }
    else
    {
        plGuid = sObjectMgr->GetPlayerGUIDByName (plName.c_str ());
        glId = Player::GetGuildIdFromDB (plGuid);
    }

    if (!plGuid || !glId)
        return false;

    Guild* targetGuild = sObjectMgr->GetGuildById (glId);
    if (!targetGuild)
        return false;

    uint32 newrank = uint32 (atoi (par2));
    if (newrank > targetGuild->GetLowestRank ())
        return false;

    targetGuild->ChangeRank (plGuid,newrank);

    return true;
}

bool ChatHandler::HandleGuildDeleteCommand(const char* args)
{
    ARGS_CHECK

    char* par1 = strtok ((char*)args, " ");
    char* par2 = strtok(NULL, " ");
    if (!par1)
        return false;

    std::string gld = par1;

    Guild* targetGuild = NULL;
    if (gld == "id") {
        if (!par2)
            return false;
        targetGuild = sObjectMgr->GetGuildById(atoi(par2));
    } else {
        targetGuild = sObjectMgr->GetGuildByName (gld);
    }

    if (!targetGuild)
        return false;

    targetGuild->Disband ();
    PSendSysMessage("Guild deleted.");

    return true;
}

bool ChatHandler::HandleGetDistanceCommand(const char* /*args*/)
{
    

    Unit* pUnit = getSelectedUnit();

    if(!pUnit)
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    PSendSysMessage(LANG_DISTANCE, m_session->GetPlayer()->GetDistance(pUnit),m_session->GetPlayer()->GetDistance2d(pUnit));
    PSendSysMessage("Exact distance 2d: %f", m_session->GetPlayer()->GetExactDistance2d(pUnit->GetPositionX(), pUnit->GetPositionY()));

    return true;
}

bool ChatHandler::HandleAddWeaponCommand(const char* args)
{
    ARGS_CHECK
 
    Creature *pCreature = getSelectedCreature();
 
    if(!pCreature)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        return true;
    }
 
    char* pItemID = strtok((char*)args, " ");
    char* pSlotID = strtok(NULL, " ");
 
    if (!pItemID)
        return false;
 
    uint32 slotID;
    if (pSlotID)
        slotID = atoi(pSlotID);
    else
        slotID = 1;
   
    uint32 itemID = atoi(pItemID);
    if (itemID == 0)
    {
        pCreature->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY + (slotID-1), 0);
        return true;
    }
   
    const ItemTemplate* proto = sObjectMgr->GetItemTemplate(itemID);
    if (!proto)
    {
        PSendSysMessage(LANG_ITEM_NOT_FOUND,itemID);
        return true;
    }
/*
    PSendSysMessage("Class = %u",proto->Class);
    PSendSysMessage("SubClass = %u",proto->SubClass);
    PSendSysMessage("InventoryType = %u",proto->InventoryType);
*/
    switch(proto->InventoryType)
    {
        case INVTYPE_SHIELD:
            slotID = 2;
            pCreature->SetByteValue(UNIT_FIELD_BYTES_2, 0, SHEATH_STATE_MELEE);
            break;
        case INVTYPE_2HWEAPON:
        case INVTYPE_WEAPONMAINHAND:
        case INVTYPE_WEAPON:
        case INVTYPE_WEAPONOFFHAND:
            if (slotID != 1 && slotID != 2)
            {
                PSendSysMessage("Emplacement %u invalide.",slotID);
                return true;
            }
            pCreature->SetByteValue(UNIT_FIELD_BYTES_2, 0, SHEATH_STATE_MELEE);
            break;
        case INVTYPE_THROWN:
        case INVTYPE_RANGED:
        case INVTYPE_RANGEDRIGHT:
            slotID = 3;
            pCreature->SetByteValue(UNIT_FIELD_BYTES_2, 0, SHEATH_STATE_RANGED);
            break;
        default:
            PSendSysMessage("Objet %u invalide.",itemID);
            return true;
            break;
    }
    uint32 equipinfo = proto->Class + proto->SubClass * 256;
    pCreature->SetUInt32Value(UNIT_VIRTUAL_ITEM_INFO + ((slotID-1) * 2) + 0, equipinfo);
    pCreature->SetUInt32Value(UNIT_VIRTUAL_ITEM_INFO + ((slotID-1) * 2) + 1, proto->InventoryType);
 
    pCreature->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY + (slotID-1), proto->DisplayInfoID);
 
    PSendSysMessage(LANG_ITEM_ADDED_TO_SLOT,itemID,proto->Name1.c_str(),slotID);
    return true;
}

bool ChatHandler::HandleDieCommand(const char* /*args*/)
{
    

    Unit* target = getSelectedUnit();

    if(!target || !m_session->GetPlayer()->GetTarget())
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    if( target->IsAlive() )
    {
        m_session->GetPlayer()->Kill(target);
    }

    return true;
}

bool ChatHandler::HandleDamageCommand(const char * args)
{
    
    ARGS_CHECK

    Unit* target = getSelectedUnit();

    if(!target || !m_session->GetPlayer()->GetTarget())
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    if( !target->IsAlive() )
        return true;

    char* damageStr = strtok((char*)args, " ");
    if(!damageStr)
        return false;

    int32 damage = atoi((char*)damageStr);
    if(damage <=0)
        return true;

    char* schoolStr = strtok((char*)NULL, " ");

    // flat melee damage without resistence/etc reduction
    if(!schoolStr)
    {
        m_session->GetPlayer()->DealDamage(target, damage, NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
        m_session->GetPlayer()->SendAttackStateUpdate (HITINFO_NORMALSWING2, target, 1, SPELL_SCHOOL_MASK_NORMAL, damage, 0, 0, VICTIMSTATE_NORMAL, 0);
        return true;
    }

    uint32 school = schoolStr ? atoi((char*)schoolStr) : SPELL_SCHOOL_NORMAL;
    if(school >= MAX_SPELL_SCHOOL)
        return false;

    SpellSchoolMask schoolmask = SpellSchoolMask(1 << school);

    if ( schoolmask & SPELL_SCHOOL_MASK_NORMAL )
        damage = m_session->GetPlayer()->CalcArmorReducedDamage(target, damage);

    char* spellStr = strtok((char*)NULL, " ");

    // melee damage by specific school
    if(!spellStr)
    {
        uint32 absorb = 0;
        uint32 resist = 0;

        m_session->GetPlayer()->CalcAbsorbResist(target,schoolmask, SPELL_DIRECT_DAMAGE, damage, &absorb, &resist, 0);

        if (damage <= absorb + resist)
            return true;

        damage -= absorb + resist;

        m_session->GetPlayer()->DealDamage(target, damage, NULL, DIRECT_DAMAGE, schoolmask, NULL, false);
        m_session->GetPlayer()->SendAttackStateUpdate (HITINFO_NORMALSWING2, target, 1, schoolmask, damage, absorb, resist, VICTIMSTATE_NORMAL, 0);
        return true;
    }

    // non-melee damage

    // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
    uint32 spellid = extractSpellIdFromLink((char*)args);
    if(!spellid || !sSpellMgr->GetSpellInfo(spellid))
        return false;

    m_session->GetPlayer()->SpellNonMeleeDamageLog(target, spellid, damage, false);
    return true;
}

bool ChatHandler::HandleModifyArenaCommand(const char * args)
{
    ARGS_CHECK

    Player *target = getSelectedPlayer();
    if(!target)
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    int32 amount = (uint32)atoi(args);

    target->ModifyArenaPoints(amount);

    PSendSysMessage(LANG_COMMAND_MODIFY_ARENA, target->GetName().c_str(), target->GetArenaPoints());

    return true;
}

bool ChatHandler::HandleReviveCommand(const char* args)
{
    Player* SelectedPlayer = NULL;

    if (*args)
    {
        std::string name = args;
        if(!normalizePlayerName(name))
        {
            SendSysMessage(LANG_PLAYER_NOT_FOUND);
            SetSentErrorMessage(true);
            return false;
        }

        SelectedPlayer = sObjectAccessor->FindConnectedPlayerByName(name.c_str());
    }
    else
        SelectedPlayer = getSelectedPlayer();

    if(!SelectedPlayer)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    SelectedPlayer->ResurrectPlayer(0.5f);
    SelectedPlayer->SpawnCorpseBones();
    SelectedPlayer->SaveToDB();
    return true;
}

bool ChatHandler::HandleAuraCommand(const char* args)
{
    char* px = strtok((char*)args, " ");
    if (!px)
        return false;

    Unit *target = getSelectedUnit();
    if(!target)
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    uint32 spellID = (uint32)atoi(px);
    SpellInfo const *spellInfo = sSpellMgr->GetSpellInfo( spellID );
    if(spellInfo)
    {
        for(uint32 i = 0;i<3;i++)
        {
            uint8 eff = spellInfo->Effects[i].Effect;
            if (eff>=TOTAL_SPELL_EFFECTS)
                continue;
            if( IsAreaAuraEffect(eff)           ||
                eff == SPELL_EFFECT_APPLY_AURA  ||
                eff == SPELL_EFFECT_PERSISTENT_AREA_AURA )
            {
                Aura *Aur = CreateAura(spellInfo, i, NULL, target);
                target->AddAura(Aur);
            }
        }
    }

    return true;
}

bool ChatHandler::HandleUnAuraCommand(const char* args)
{
    char* px = strtok((char*)args, " ");
    if (!px)
        return false;

    Unit *target = getSelectedUnit();
    if(!target)
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
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

bool ChatHandler::HandleLinkGraveCommand(const char* args)
{
    

    ARGS_CHECK

    char* px = strtok((char*)args, " ");
    if (!px)
        return false;

    uint32 g_id = (uint32)atoi(px);

    uint32 g_team;

    char* px2 = strtok(NULL, " ");

    if (!px2)
        g_team = 0;
    else if (strncmp(px2,"horde",6)==0)
        g_team = TEAM_HORDE;
    else if (strncmp(px2,"alliance",9)==0)
        g_team = TEAM_ALLIANCE;
    else
        return false;

    WorldSafeLocsEntry const* graveyard =  sWorldSafeLocsStore.LookupEntry(g_id);

    if(!graveyard )
    {
        PSendSysMessage(LANG_COMMAND_GRAVEYARDNOEXIST, g_id);
        SetSentErrorMessage(true);
        return false;
    }

    Player* player = m_session->GetPlayer();

    uint32 zoneId = player->GetZoneId();

    AreaTableEntry const *areaEntry = GetAreaEntryByAreaID(zoneId);
    if(!areaEntry || areaEntry->parentArea !=0 )
    {
        PSendSysMessage(LANG_COMMAND_GRAVEYARDWRONGZONE, g_id,zoneId);
        SetSentErrorMessage(true);
        return false;
    }

    if(sObjectMgr->AddGraveYardLink(g_id,player->GetZoneId(),g_team))
        PSendSysMessage(LANG_COMMAND_GRAVEYARDLINKED, g_id,zoneId);
    else
        PSendSysMessage(LANG_COMMAND_GRAVEYARDALRLINKED, g_id,zoneId);

    return true;
}

bool ChatHandler::HandleNearGraveCommand(const char* args)
{
    

    uint32 g_team;

    size_t argslen = strlen(args);

    if(!*args)
        g_team = 0;
    else if (strncmp((char*)args,"horde",argslen)==0)
        g_team = TEAM_HORDE;
    else if (strncmp((char*)args,"alliance",argslen)==0)
        g_team = TEAM_ALLIANCE;
    else
        return false;

    Player* player = m_session->GetPlayer();

    WorldSafeLocsEntry const* graveyard = sObjectMgr->GetClosestGraveYard(
        player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(),player->GetMapId(),g_team);

    if(graveyard)
    {
        uint32 g_id = graveyard->ID;

        GraveYardData const* data = sObjectMgr->FindGraveYardData(g_id,player->GetZoneId());
        if (!data)
        {
            PSendSysMessage(LANG_COMMAND_GRAVEYARDERROR,g_id);
            SetSentErrorMessage(true);
            return false;
        }

        g_team = data->team;

        std::string team_name = GetTrinityString(LANG_COMMAND_GRAVEYARD_NOTEAM);

        if(g_team == 0)
            team_name = GetTrinityString(LANG_COMMAND_GRAVEYARD_ANY);
        else if(g_team == TEAM_HORDE)
            team_name = GetTrinityString(LANG_COMMAND_GRAVEYARD_HORDE);
        else if(g_team == TEAM_ALLIANCE)
            team_name = GetTrinityString(LANG_COMMAND_GRAVEYARD_ALLIANCE);

        PSendSysMessage(LANG_COMMAND_GRAVEYARDNEAREST, g_id,team_name.c_str(),player->GetZoneId());
    }
    else
    {
        std::string team_name;

        if(g_team == 0)
            team_name = GetTrinityString(LANG_COMMAND_GRAVEYARD_ANY);
        else if(g_team == TEAM_HORDE)
            team_name = GetTrinityString(LANG_COMMAND_GRAVEYARD_HORDE);
        else if(g_team == TEAM_ALLIANCE)
            team_name = GetTrinityString(LANG_COMMAND_GRAVEYARD_ALLIANCE);

        if(g_team == ~uint32(0))
            PSendSysMessage(LANG_COMMAND_ZONENOGRAVEYARDS, player->GetZoneId());
        else
            PSendSysMessage(LANG_COMMAND_ZONENOGRAFACTION, player->GetZoneId(),team_name.c_str());
    }

    return true;
}

// .npc playemote #emoteid [#permanent]
bool ChatHandler::HandleNpcPlayEmoteCommand(const char* args)
{
    char* cEmote = strtok((char*)args, " ");
    char* cPermanent = strtok((char*)NULL, " ");

    if(!cEmote) return false;
    uint32 emote = atoi(cEmote);
    uint8 permanent = 0;
    if(cPermanent) permanent = atoi(cPermanent);

    Creature* target = getSelectedCreature();
    if(!target)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    target->SetUInt32Value(UNIT_NPC_EMOTESTATE,emote);
    if(permanent)
    {
        PSendSysMessage("Emote set as permanent (will stay after a reboot)");
        if(emote)
            WorldDatabase.PExecute("REPLACE INTO creature_addon(`guid`,`emote`) VALUES (%u,%u)", target->GetDBTableGUIDLow(), emote);
        else
            WorldDatabase.PExecute("UPDATE creature_addon SET `emote` = 0 WHERE `guid` = %u", target->GetDBTableGUIDLow());
    }
    return true;
}

bool ChatHandler::HandleNpcInfoCommand(const char* /*args*/)
{
    Creature* target = getSelectedCreature();

    if(!target)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    uint32 faction = target->GetFaction();
    uint32 npcflags = target->GetUInt32Value(UNIT_NPC_FLAGS);
    uint32 displayid = target->GetDisplayId();
    uint32 nativeid = target->GetNativeDisplayId();
    uint32 Entry = target->GetEntry();
    CreatureTemplate const* cInfo = target->GetCreatureTemplate();
    CreatureData const* cData = sObjectMgr->GetCreatureData(target->GetDBTableGUIDLow());

    int32 curRespawnDelay = target->GetRespawnTimeEx()-time(NULL);
    if(curRespawnDelay < 0)
        curRespawnDelay = 0;
    std::string curRespawnDelayStr = secsToTimeString(curRespawnDelay,true);
    std::string defRespawnDelayStr = secsToTimeString(target->GetRespawnDelay(),true);

    PSendSysMessage(LANG_NPCINFO_CHAR,  target->GetDBTableGUIDLow(), faction, npcflags, Entry, displayid, nativeid);
    if(cInfo->difficulty_entry_1)
        PSendSysMessage("Heroic Entry: %u", cInfo->difficulty_entry_1);
    else if (target->GetMap()->IsHeroic() && Entry != cInfo->Entry)
        PSendSysMessage("Current id (heroic id ?) : %u", cInfo->Entry);

    PSendSysMessage(LANG_NPCINFO_LEVEL, target->GetLevel());
    PSendSysMessage(LANG_NPCINFO_HEALTH,target->GetCreateHealth(), target->GetMaxHealth(), target->GetHealth());
    PSendSysMessage(LANG_NPCINFO_FLAGS, target->GetUInt32Value(UNIT_FIELD_FLAGS), target->GetUInt32Value(UNIT_DYNAMIC_FLAGS), target->GetFaction());
    PSendSysMessage(LANG_COMMAND_RAWPAWNTIMES, defRespawnDelayStr.c_str(),curRespawnDelayStr.c_str());
    PSendSysMessage(LANG_NPCINFO_LOOT,  cInfo->lootid,cInfo->pickpocketLootId,cInfo->SkinLootId);
    PSendSysMessage(LANG_NPCINFO_DUNGEON_ID, target->GetInstanceId());
    PSendSysMessage(LANG_NPCINFO_POSITION,float(target->GetPositionX()), float(target->GetPositionY()), float(target->GetPositionZ()), float(target->GetOrientation()));
    PSendSysMessage("AIName: %s", target->GetAIName().c_str());
    PSendSysMessage("ScriptName: %s", target->GetScriptName().c_str());
    PSendSysMessage("ScriptName (new): %s", target->getScriptName().c_str());
    PSendSysMessage("Creature Pool ID: %u", target->GetCreaturePoolId());
    PSendSysMessage("Creature linked instance event: %d", int(target->getInstanceEventId()));
    if(const CreatureData* const linked = target->GetLinkedRespawnCreatureData())
        if(CreatureTemplate const *master = sObjectMgr->GetCreatureTemplate(linked->id))
            PSendSysMessage(LANG_NPCINFO_LINKGUID, sObjectMgr->GetLinkedRespawnGuid(target->GetDBTableGUIDLow()), linked->id, master->Name.c_str());

    PSendSysMessage("Mouvement flag: %u", target->GetUnitMovementFlags());
    if ((npcflags & UNIT_NPC_FLAG_VENDOR) )
    {
        SendSysMessage(LANG_NPCINFO_VENDOR);
    }
    if ((npcflags & UNIT_NPC_FLAG_TRAINER) )
    {
        SendSysMessage(LANG_NPCINFO_TRAINER);
    }
    if(target->GetWaypointPathId())
        PSendSysMessage("PathID : %u", target->GetWaypointPathId());

    if(target->GetFormation())
        PSendSysMessage("Creature is member of group %u", target->GetFormation()->GetId());

    return true;
}

bool ChatHandler::HandleExploreCheatCommand(const char* args)
{
    ARGS_CHECK

    int flag = atoi((char*)args);

    Player *chr = getSelectedPlayer();
    if (chr == NULL)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    if (flag != 0)
    {
        PSendSysMessage(LANG_YOU_SET_EXPLORE_ALL, chr->GetName().c_str());
        if (needReportToTarget(chr))
            ChatHandler(chr).PSendSysMessage(LANG_YOURS_EXPLORE_SET_ALL,GetName().c_str());
    }
    else
    {
        PSendSysMessage(LANG_YOU_SET_EXPLORE_NOTHING, chr->GetName().c_str());
        if (needReportToTarget(chr))
            ChatHandler(chr).PSendSysMessage(LANG_YOURS_EXPLORE_SET_NOTHING,GetName().c_str());
    }

    for (uint8 i=0; i<128; i++)
    {
        if (flag != 0)
        {
            m_session->GetPlayer()->SetFlag(PLAYER_EXPLORED_ZONES_1+i,0xFFFFFFFF);
        }
        else
        {
            m_session->GetPlayer()->SetFlag(PLAYER_EXPLORED_ZONES_1+i,0);
        }
    }

    return true;
}

bool ChatHandler::HandleHoverCommand(const char* args)
{
    

    char* px = strtok((char*)args, " ");
    uint32 flag;
    if (!px)
        flag = 1;
    else
        flag = atoi(px);

    m_session->GetPlayer()->SetHover(flag);

    if (flag)
        SendSysMessage(LANG_HOVER_ENABLED);
    else
        SendSysMessage(LANG_HOVER_DISABLED);

    return true;
}

bool ChatHandler::HandleWaterwalkCommand(const char* args)
{
    Player *player = getSelectedPlayer();
    if(!player)
    {
        PSendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    if (strncmp(args, "on", 3) == 0)
        player->SetMovement(MOVE_WATER_WALK);               // ON
    else if (strncmp(args, "off", 4) == 0)
        player->SetMovement(MOVE_LAND_WALK);                // OFF
    else
    {
        SendSysMessage(LANG_USE_BOL);
        return false;
    }

    PSendSysMessage(LANG_YOU_SET_WATERWALK, args, player->GetName().c_str());
    if(needReportToTarget(player))
        ChatHandler(player).PSendSysMessage(LANG_YOUR_WATERWALK_SET, args, GetName().c_str());
    return true;

}

bool ChatHandler::HandleLevelUpCommand(const char* args)
{
    char* px = strtok((char*)args, " ");
    char* py = strtok((char*)NULL, " ");

    // command format parsing
    char* pname = (char*)NULL;
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
    Player *chr = NULL;
    uint64 chr_guid = 0;

    std::string name;

    if(pname)                                               // player by name
    {
        name = pname;
        if(!normalizePlayerName(name))
        {
            SendSysMessage(LANG_PLAYER_NOT_FOUND);
            SetSentErrorMessage(true);
            return false;
        }

        chr = sObjectAccessor->FindConnectedPlayerByName(name.c_str());
        if(!chr)                                            // not in game
        {
            chr_guid = sObjectMgr->GetPlayerGUIDByName(name);
            if (chr_guid == 0)
            {
                SendSysMessage(LANG_PLAYER_NOT_FOUND);
                SetSentErrorMessage(true);
                return false;
            }
        }
    }
    else                                                    // player by selection
    {
        chr = getSelectedPlayer();

        if (chr == NULL)
        {
            SendSysMessage(LANG_NO_CHAR_SELECTED);
            SetSentErrorMessage(true);
            return false;
        }

        name = chr->GetName();
    }

    assert(chr || chr_guid);

    int32 oldlevel = chr ? chr->GetLevel() : Player::GetLevelFromDB(chr_guid);
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
        CharacterDatabase.PExecute("UPDATE characters SET level = '%u', xp = 0 WHERE guid = '%u'", newlevel, GUID_LOPART(chr_guid));
    }

    if(m_session && m_session->GetPlayer() != chr)                       // including chr==NULL
        PSendSysMessage(LANG_YOU_CHANGE_LVL,name.c_str(),newlevel);
    return true;
}

bool ChatHandler::HandleShowAreaCommand(const char* args)
{
    ARGS_CHECK

    int area = atoi((char*)args);

    Player *chr = getSelectedPlayer();
    if (chr == NULL)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    int offset = area / 32;
    uint32 val = (uint32)(1 << (area % 32));

    if(offset >= 128)
    {
        SendSysMessage(LANG_BAD_VALUE);
        SetSentErrorMessage(true);
        return false;
    }

    uint32 currFields = chr->GetUInt32Value(PLAYER_EXPLORED_ZONES_1 + offset);
    chr->SetUInt32Value(PLAYER_EXPLORED_ZONES_1 + offset, (uint32)(currFields | val));

    SendSysMessage(LANG_EXPLORE_AREA);
    return true;
}

bool ChatHandler::HandleHideAreaCommand(const char* args)
{
    ARGS_CHECK

    int area = atoi((char*)args);

    Player *chr = getSelectedPlayer();
    if (chr == NULL)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    int offset = area / 32;
    uint32 val = (uint32)(1 << (area % 32));

    if(offset >= 128)
    {
        SendSysMessage(LANG_BAD_VALUE);
        SetSentErrorMessage(true);
        return false;
    }

    uint32 currFields = chr->GetUInt32Value(PLAYER_EXPLORED_ZONES_1 + offset);
    chr->SetUInt32Value(PLAYER_EXPLORED_ZONES_1 + offset, (uint32)(currFields ^ val));

    SendSysMessage(LANG_UNEXPLORE_AREA);
    return true;
}

bool ChatHandler::HandleUpdateCommand(const char* args)
{
    ARGS_CHECK

    uint32 updateIndex;
    uint32 value;

    char* pUpdateIndex = strtok((char*)args, " ");

    Unit* chr = getSelectedUnit();
    if (chr == NULL)
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

    char*  pvalue = strtok(NULL, " ");
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

bool ChatHandler::HandleBankCommand(const char* /*args*/)
{
    

    m_session->SendShowBank( m_session->GetPlayer()->GetGUID() );

    return true;
}

bool ChatHandler::HandleChangeWeather(const char* args)
{
    
    ARGS_CHECK

    //Weather is OFF
    if (!sWorld->getConfig(CONFIG_WEATHER))
    {
        SendSysMessage(LANG_WEATHER_DISABLED);
        SetSentErrorMessage(true);
        return false;
    }

    //*Change the weather of a cell
    char* px = strtok((char*)args, " ");
    char* py = strtok(NULL, " ");

    if (!px || !py)
        return false;

    uint32 type = (uint32)atoi(px);                         //0 to 3, 0: fine, 1: rain, 2: snow, 3: sand
    float grade = (float)atof(py);                          //0 to 1, sending -1 is instand good weather

    Player *player = m_session->GetPlayer();
    uint32 zoneid = player->GetZoneId();

    Weather* wth = sWorld->FindWeather(zoneid);

    if(!wth)
        wth = sWorld->AddWeather(zoneid);
    if(!wth)
    {
        SendSysMessage(LANG_NO_WEATHER);
        SetSentErrorMessage(true);
        return false;
    }

    wth->SetWeather(WeatherType(type), grade);

    return true;
}

/* Syntax : .debug setvalue #index #value [uint32/uint64/float]*/
bool ChatHandler::HandleSetValueCommand(const char* args)
{
    ARGS_CHECK

    char* cIndex = strtok((char*)args, " ");
    char* cValue = strtok(nullptr, " ");
    char* cType = strtok(nullptr, " ");

    if (!cIndex || !cValue)
        return false;

    Unit* target = getSelectedUnit();
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

/* Syntax : .debug getvalue #index [uint32/uint64/float]*/
bool ChatHandler::HandleGetValueCommand(const char* args)
{
    ARGS_CHECK

    char* cIndex = strtok((char*)args, " ");
    char* cType = strtok(nullptr, " ");

    if (!cIndex)
        return false;

    Unit* target = getSelectedUnit();
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
        uValue = target->GetUInt32Value(index);
        PSendSysMessage(LANG_GET_UINT_FIELD, GUID_LOPART(guid), index, uValue);
        break;
    case 1: //uint64
        uValue = target->GetUInt64Value(index);
        PSendSysMessage("The uint64 value of %u (lowguid) in field %u is " UI64FMTD, GUID_LOPART(guid), index, uValue);
        break;
    case 2: //float
        fValue = target->GetFloatValue(index);
        PSendSysMessage(LANG_GET_FLOAT_FIELD, GUID_LOPART(guid), index, fValue);
        break;
    }

    return true;
}

/* Syntax : .gobject setvalue #guid #index #value [uint32/uint64/float]*/
bool ChatHandler::HandleGobSetValueCommand(const char* args)
{
    ARGS_CHECK

    char* cGUID = strtok((char*)args, " ");
    char* cIndex = strtok(nullptr, " ");
    char* cValue = strtok(nullptr, " ");
    char* cType = strtok(nullptr, " ");

    if (!cGUID || !cIndex || !cValue)
        return false;

    uint64 guid = atoi(cGUID);
    if(!guid) 
        return false;

     GameObject* target = NULL;
    // by DB guid
    if (GameObjectData const* go_data = sObjectMgr->GetGOData(guid))
        target = GetObjectGlobalyWithGuidOrNearWithDbGuid(guid,go_data->id);

    if(!target)
    {
        PSendSysMessage(LANG_COMMAND_OBJNOTFOUND, guid);
        SetSentErrorMessage(true);
        return false;
    }

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
        uValue = (uint32)atoi(cValue);
        target->SetUInt32Value(index,uValue);
        PSendSysMessage(LANG_SET_UINT_FIELD, GUID_LOPART(guid), index, uValue);
        break;
    case 1: //uint64
        uValue = (uint64)atoi(cValue);
        target->SetUInt64Value(index,uValue);
        PSendSysMessage(LANG_SET_UINT_FIELD, GUID_LOPART(guid), index, uValue);
    case 2: //float
        fValue = (float)atof(cValue);
        target->SetFloatValue(index,fValue);
        PSendSysMessage(LANG_SET_FLOAT_FIELD, GUID_LOPART(guid), index, fValue);
        break;
    }

    //update visual
    if(Map* map = sMapMgr->CreateMap(target->GetMapId(),target))
    {
        map->Remove(target,false);
        map->Add(target);
    }

    return true;
}

/* Syntax : .gobject getvalue #guid #index [uint32/uint64/float]*/
bool ChatHandler::HandleGobGetValueCommand(const char * args)
{
    ARGS_CHECK

    char* cGUID = strtok((char*)args, " ");
    char* cIndex = strtok(nullptr, " ");
    char* cType = strtok(nullptr, " ");

    if (!cGUID || !cIndex)
        return false;

    uint64 guid = atoi(cGUID);
    if(!guid) 
        return false;

     GameObject* target = NULL;
    // by DB guid
    if (GameObjectData const* go_data = sObjectMgr->GetGOData(guid))
        target = GetObjectGlobalyWithGuidOrNearWithDbGuid(guid,go_data->id);

    if(!target)
    {
        PSendSysMessage(LANG_COMMAND_OBJNOTFOUND, guid);
        SetSentErrorMessage(true);
        return false;
    }

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
        uValue = target->GetUInt32Value(index);
        PSendSysMessage(LANG_GET_UINT_FIELD, GUID_LOPART(guid), index, uValue);
        break;
    case 1: //uint64
        uValue = target->GetUInt64Value(index);
        PSendSysMessage(LANG_GET_UINT_FIELD, GUID_LOPART(guid), index, uValue);
    case 2: //float
        fValue = target->GetFloatValue(index);
        PSendSysMessage(LANG_GET_FLOAT_FIELD, GUID_LOPART(guid), index, fValue);
        break;
    }

    return true;
}

bool ChatHandler::HandleAddTeleCommand(const char * args)
{
    
    ARGS_CHECK

    Player *player=m_session->GetPlayer();
    if (!player)
        return false;

    std::string name = args;

    if(sObjectMgr->GetGameTele(name))
    {
        SendSysMessage(LANG_COMMAND_TP_ALREADYEXIST);
        SetSentErrorMessage(true);
        return false;
    }

    GameTele tele;
    tele.position_x  = player->GetPositionX();
    tele.position_y  = player->GetPositionY();
    tele.position_z  = player->GetPositionZ();
    tele.orientation = player->GetOrientation();
    tele.mapId       = player->GetMapId();
    tele.name        = name;

    if(sObjectMgr->AddGameTele(tele))
    {
        SendSysMessage(LANG_COMMAND_TP_ADDED);
    }
    else
    {
        SendSysMessage(LANG_COMMAND_TP_ADDEDERR);
        SetSentErrorMessage(true);
        return false;
    }

    return true;
}

bool ChatHandler::HandleDelTeleCommand(const char * args)
{
    ARGS_CHECK

    std::string name = args;

    if(!sObjectMgr->DeleteGameTele(name))
    {
        SendSysMessage(LANG_COMMAND_TELE_NOTFOUND);
        SetSentErrorMessage(true);
        return false;
    }

    SendSysMessage(LANG_COMMAND_TP_DELETED);
    return true;
}

bool ChatHandler::HandleListAurasCommand (const char * /*args*/)
{
    Unit *unit = getSelectedUnit();
    if(!unit)
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    char const* talentStr = GetTrinityString(LANG_TALENT);
    char const* passiveStr = GetTrinityString(LANG_PASSIVE);

    Unit::AuraMap const& uAuras = unit->GetAuras();
    PSendSysMessage(LANG_COMMAND_TARGET_LISTAURAS, uAuras.size());
    for (Unit::AuraMap::const_iterator itr = uAuras.begin(); itr != uAuras.end(); ++itr)
    {
        bool talent = GetTalentSpellCost(itr->second->GetId()) > 0;
        PSendSysMessage(LANG_COMMAND_TARGET_AURADETAIL, itr->second->GetId(), itr->second->GetEffIndex(),
            itr->second->GetModifier()->m_auraname, itr->second->GetAuraDuration(), itr->second->GetAuraMaxDuration(),
            itr->second->GetSpellInfo()->SpellName[GetSessionDbcLocale()],
            (itr->second->IsPassive() ? passiveStr : ""),(talent ? talentStr : ""),
            IS_PLAYER_GUID(itr->second->GetCasterGUID()) ? "player" : "creature",GUID_LOPART(itr->second->GetCasterGUID()));
    }
    for (int i = 0; i < TOTAL_AURAS; i++)
    {
        Unit::AuraList const& uAuraList = unit->GetAurasByType(AuraType(i));
        if (uAuraList.empty()) continue;
        PSendSysMessage(LANG_COMMAND_TARGET_LISTAURATYPE, uAuraList.size(), i);
        for (Unit::AuraList::const_iterator itr = uAuraList.begin(); itr != uAuraList.end(); ++itr)
        {
            bool talent = GetTalentSpellCost((*itr)->GetId()) > 0;
            PSendSysMessage(LANG_COMMAND_TARGET_AURASIMPLE, (*itr)->GetId(), (*itr)->GetEffIndex(),
                (*itr)->GetSpellInfo()->SpellName[GetSessionDbcLocale()],((*itr)->IsPassive() ? passiveStr : ""),(talent ? talentStr : ""),
                IS_PLAYER_GUID((*itr)->GetCasterGUID()) ? "player" : "creature",GUID_LOPART((*itr)->GetCasterGUID()));
        }
    }
    return true;
}

bool ChatHandler::HandleResetHonorCommand (const char * args)
{
    char* pName = strtok((char*)args, "");
    Player *player = NULL;
    if (pName)
    {
        std::string name = pName;
        if(!normalizePlayerName(name))
        {
            SendSysMessage(LANG_PLAYER_NOT_FOUND);
            SetSentErrorMessage(true);
            return false;
        }

        uint64 guid = sObjectMgr->GetPlayerGUIDByName(name.c_str());
        player = sObjectMgr->GetPlayer(guid);
    }
    else
        player = getSelectedPlayer();

    if(!player)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        return true;
    }

    player->SetUInt32Value(PLAYER_FIELD_KILLS, 0);
    player->SetUInt32Value(PLAYER_FIELD_LIFETIME_HONORABLE_KILLS, 0);
    player->SetUInt32Value(PLAYER_FIELD_HONOR_CURRENCY, 0);
    player->SetUInt32Value(PLAYER_FIELD_TODAY_CONTRIBUTION, 0);
    player->SetUInt32Value(PLAYER_FIELD_YESTERDAY_CONTRIBUTION, 0);

    return true;
}

static bool HandleResetStatsOrLevelHelper(Player* player)
{
    PlayerInfo const *info = sObjectMgr->GetPlayerInfo(player->GetRace(), player->GetClass());
    if(!info) return false;

    ChrClassesEntry const* cEntry = sChrClassesStore.LookupEntry(player->GetClass());
    if(!cEntry)
    {
        TC_LOG_ERROR("command","Class %u not found in DBC (Wrong DBC files?)",player->GetClass());
        return false;
    }

    uint8 powertype = cEntry->PowerType;

    uint32 unitfield;
    if(powertype == POWER_RAGE)
        unitfield = 0x1100EE00;
    else if(powertype == POWER_ENERGY)
        unitfield = 0x00000000;
    else if(powertype == POWER_MANA)
        unitfield = 0x0000EE00;
    else
    {
        TC_LOG_ERROR("command","Invalid default powertype %u for player (class %u)",powertype,player->GetClass());
        return false;
    }

    // reset m_form if no aura
    if(!player->HasAuraType(SPELL_AURA_MOD_SHAPESHIFT))
        player->m_form = FORM_NONE;

    player->SetFloatValue(UNIT_FIELD_BOUNDINGRADIUS, DEFAULT_WORLD_OBJECT_SIZE );
    player->SetFloatValue(UNIT_FIELD_COMBATREACH, DEFAULT_COMBAT_REACH );

    player->SetFactionForRace(player->GetRace());

    player->SetUInt32Value(UNIT_FIELD_BYTES_0, ( ( player->GetRace() ) | ( player->GetClass() << 8 ) | ( player->GetGender() << 16 ) | ( powertype << 24 ) ) );

    // reset only if player not in some form;
    if(player->m_form==FORM_NONE)
    {
        switch(player->GetGender())
        {
            case GENDER_FEMALE:
                player->SetDisplayId(info->displayId_f);
                player->SetNativeDisplayId(info->displayId_f);
                break;
            case GENDER_MALE:
                player->SetDisplayId(info->displayId_m);
                player->SetNativeDisplayId(info->displayId_m);
                break;
            default:
                break;
        }
    }

    // set UNIT_FIELD_BYTES_1 to init state but preserve m_form value
    player->SetUInt32Value(UNIT_FIELD_BYTES_1, unitfield);
    player->SetByteValue(UNIT_FIELD_BYTES_2, 1, UNIT_BYTE2_FLAG_SANCTUARY | UNIT_BYTE2_FLAG_UNK5 );
    player->SetByteValue(UNIT_FIELD_BYTES_2, 3, player->m_form);

    player->SetUInt32Value(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP_ATTACKABLE);

    //-1 is default value
    player->SetUInt32Value(PLAYER_FIELD_WATCHED_FACTION_INDEX, uint32(-1));

    //player->SetUInt32Value(PLAYER_FIELD_BYTES, 0xEEE00000 );
    return true;
}

bool ChatHandler::HandleResetLevelCommand(const char * args)
{
    char* pName = strtok((char*)args, "");
    Player *player = NULL;
    if (pName)
    {
        std::string name = pName;
        if(!normalizePlayerName(name))
        {
            SendSysMessage(LANG_PLAYER_NOT_FOUND);
            SetSentErrorMessage(true);
            return false;
        }

        uint64 guid = sObjectMgr->GetPlayerGUIDByName(name.c_str());
        player = sObjectMgr->GetPlayer(guid);
    }
    else
        player = getSelectedPlayer();

    if(!player)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    if(!HandleResetStatsOrLevelHelper(player))
        return false;

    player->SetLevel(1);
    player->InitStatsForLevel(true);
    player->InitTaxiNodesForLevel();
    player->InitTalentForLevel();
    player->SetUInt32Value(PLAYER_XP,0);

    // reset level to summoned pet
    Pet* pet = player->GetPet();
    if(pet && pet->getPetType()==SUMMON_PET)
        pet->InitStatsForLevel(1);

    return true;
}

bool ChatHandler::HandleResetStatsCommand(const char * args)
{
    char* pName = strtok((char*)args, "");
    Player *player = NULL;
    if (pName)
    {
        std::string name = pName;
        if(!normalizePlayerName(name))
        {
            SendSysMessage(LANG_PLAYER_NOT_FOUND);
            SetSentErrorMessage(true);
            return false;
        }

        uint64 guid = sObjectMgr->GetPlayerGUIDByName(name.c_str());
        player = sObjectMgr->GetPlayer(guid);
    }
    else
        player = getSelectedPlayer();

    if(!player)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    if(!HandleResetStatsOrLevelHelper(player))
        return false;

    player->InitStatsForLevel(true);
    player->InitTaxiNodesForLevel();
    player->InitTalentForLevel();

    return true;
}

bool ChatHandler::HandleResetSpellsCommand(const char * args)
{
    

    char* pName = strtok((char*)args, "");
    Player *player = NULL;
    uint64 playerGUID = 0;
    if (pName)
    {
        std::string name = pName;

        if(!normalizePlayerName(name))
        {
            SendSysMessage(LANG_PLAYER_NOT_FOUND);
            SetSentErrorMessage(true);
            return false;
        }

        player = sObjectAccessor->FindConnectedPlayerByName(name.c_str());
        if(!player)
            playerGUID = sObjectMgr->GetPlayerGUIDByName(name.c_str());
    }
    else
        player = getSelectedPlayer();

    if(!player && !playerGUID)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    if(player)
    {
        player->resetSpells();

        ChatHandler(player).SendSysMessage(LANG_RESET_SPELLS);

        if(m_session->GetPlayer()!=player)
            PSendSysMessage(LANG_RESET_SPELLS_ONLINE,player->GetName().c_str());
    }
    else
    {
        CharacterDatabase.PExecute("UPDATE characters SET at_login = at_login | '%u' WHERE guid = '%u'",uint32(AT_LOGIN_RESET_SPELLS), GUID_LOPART(playerGUID));
        PSendSysMessage(LANG_RESET_SPELLS_OFFLINE,pName);
    }

    return true;
}

bool ChatHandler::HandleResetTalentsCommand(const char * args)
{
    

    char* pName = strtok((char*)args, "");
    Player *player = NULL;
    uint64 playerGUID = 0;
    if (pName)
    {
        std::string name = pName;
        if(!normalizePlayerName(name))
        {
            SendSysMessage(LANG_PLAYER_NOT_FOUND);
            SetSentErrorMessage(true);
            return false;
        }

        player = sObjectAccessor->FindConnectedPlayerByName(name.c_str());
        if(!player)
            playerGUID = sObjectMgr->GetPlayerGUIDByName(name.c_str());
    }
    else
        player = getSelectedPlayer();

    if(!player && !playerGUID)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    if(player)
    {
        player->resetTalents(true);

        ChatHandler(player).SendSysMessage(LANG_RESET_TALENTS);

        if(m_session->GetPlayer()!=player)
            PSendSysMessage(LANG_RESET_TALENTS_ONLINE,player->GetName().c_str());
    }
    else
    {
        CharacterDatabase.PExecute("UPDATE characters SET at_login = at_login | '%u' WHERE guid = '%u'",uint32(AT_LOGIN_RESET_TALENTS), GUID_LOPART(playerGUID) );
        PSendSysMessage(LANG_RESET_TALENTS_OFFLINE,pName);
    }

    return true;
}

bool ChatHandler::HandleResetAllCommand(const char * args)
{
    ARGS_CHECK

    std::string casename = args;

    AtLoginFlags atLogin;

    // Command specially created as single command to prevent using short case names
    if(casename=="spells")
    {
        atLogin = AT_LOGIN_RESET_SPELLS;
        sWorld->SendWorldText(LANG_RESETALL_SPELLS);
    }
    else if(casename=="talents")
    {
        atLogin = AT_LOGIN_RESET_TALENTS;
        sWorld->SendWorldText(LANG_RESETALL_TALENTS);
    }
    else
    {
        PSendSysMessage(LANG_RESETALL_UNKNOWN_CASE,args);
        SetSentErrorMessage(true);
        return false;
    }

    CharacterDatabase.PExecute("UPDATE characters SET at_login = at_login | '%u' WHERE (at_login & '%u') = '0'",atLogin,atLogin);
    boost::shared_lock<boost::shared_mutex> lock(*HashMapHolder<Player>::GetLock());
    HashMapHolder<Player>::MapType const& plist = ObjectAccessor::GetPlayers();
    for(HashMapHolder<Player>::MapType::const_iterator itr = plist.begin(); itr != plist.end(); ++itr)
        itr->second->SetAtLoginFlag(atLogin);

    return true;
}

bool ChatHandler::HandleServerShutDownCancelCommand(const char* args)
{
    sWorld->ShutdownCancel();
    return true;
}

bool ChatHandler::HandleServerShutDownCommand(const char* args)
{
    ARGS_CHECK

    char* time_str = strtok ((char*) args, " ");
    char* reason = strtok (NULL, "");

    int32 time = atoi (time_str);

    ///- Prevent interpret wrong arg value as 0 secs shutdown time
    if(time == 0 && (time_str[0]!='0' || time_str[1]!='\0') || time < 0)
        return false;

    //if (exitcode_str)
    if (reason)
    {
        /*int32 exitcode = atoi (exitcode_str);

        // Handle atoi() errors
        if (exitcode == 0 && (exitcode_str[0] != '0' || exitcode_str[1] != '\0'))
            return false;

        // Exit code should be in range of 0-125, 126-255 is used
        // in many shells for their own return codes and code > 255
        // is not supported in many others
        if (exitcode < 0 || exitcode > 125)
            return false;

        sWorld->ShutdownServ (time, 0, exitcode);*/
        
        sWorld->ShutdownServ (time, 0, reason);
    }
    else
        sWorld->ShutdownServ(time,0,"");
    return true;
}

bool ChatHandler::HandleServerRestartCommand(const char* args)
{
    ARGS_CHECK

    char* time_str = strtok ((char*) args, " ");
    char* reason = strtok (NULL, "");

    int32 time = atoi (time_str);

    ///- Prevent interpret wrong arg value as 0 secs shutdown time
    if(time == 0 && (time_str[0]!='0' || time_str[1]!='\0') || time < 0)
        return false;

    if (reason)
        sWorld->ShutdownServ (time, SHUTDOWN_MASK_RESTART, reason);
    else
        sWorld->ShutdownServ(time, SHUTDOWN_MASK_RESTART, "");
        
    return true;
}

bool ChatHandler::HandleServerIdleRestartCommand(const char* args)
{
    ARGS_CHECK

    char* time_str = strtok ((char*) args, " ");
    char* reason = strtok (NULL, "");

    int32 time = atoi (time_str);

    ///- Prevent interpret wrong arg value as 0 secs shutdown time
    if(time == 0 && (time_str[0]!='0' || time_str[1]!='\0') || time < 0)
        return false;

    if (reason)
        sWorld->ShutdownServ (time, SHUTDOWN_MASK_RESTART, reason);
    else
        sWorld->ShutdownServ(time,SHUTDOWN_MASK_RESTART|SHUTDOWN_MASK_IDLE,"");
        
    return true;
}

bool ChatHandler::HandleServerIdleShutDownCommand(const char* args)
{
    ARGS_CHECK

    char* time_str = strtok ((char*) args, " ");
    char* reason = strtok (NULL, "");

    int32 time = atoi (time_str);

    ///- Prevent interpret wrong arg value as 0 secs shutdown time
    if(time == 0 && (time_str[0]!='0' || time_str[1]!='\0') || time < 0)
        return false;

    if (reason)
        sWorld->ShutdownServ (time, SHUTDOWN_MASK_IDLE, reason);
    else
        sWorld->ShutdownServ(time,SHUTDOWN_MASK_IDLE,"");
        
    return true;
}

bool ChatHandler::HandleAddQuest(const char* args)
{
    Player* player = getSelectedPlayer();
    if(!player)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    // .addquest #entry'
    // number or [name] Shift-click form |color|Hquest:quest_id|h[name]|h|r
    char* cId = extractKeyFromLink((char*)args,"Hquest");
    if(!cId)
        return false;

    uint32 entry = atol(cId);

    Quest const* pQuest = sObjectMgr->GetQuestTemplate(entry);

    if(!pQuest)
    {
        PSendSysMessage(LANG_COMMAND_QUEST_NOTFOUND,entry);
        SetSentErrorMessage(true);
        return false;
    }

    // check item starting quest (it can work incorrectly if added without item in inventory)
    ItemTemplateContainer const* its = sObjectMgr->GetItemTemplateStore();
    for (ItemTemplateContainer::const_iterator itr = its->begin(); itr != its->end(); ++itr)
    {
        uint32 id = itr->first;
        ItemTemplate const *pProto = &(itr->second);
        if (!pProto)
            continue;

        if (pProto->StartQuest == entry)
        {
            PSendSysMessage(LANG_COMMAND_QUEST_STARTFROMITEM, entry, pProto->ItemId);
            SetSentErrorMessage(true);
            return false;
        }
    }

    // ok, normal (creature/GO starting) quest
    if( player->CanAddQuest( pQuest, true ) )
    {
        player->AddQuest( pQuest, NULL );

        if ( player->CanCompleteQuest( entry ) )
            player->CompleteQuest( entry );

        if (   sWorld->getConfig(CONFIG_BUGGY_QUESTS_AUTOCOMPLETE)
                && !(pQuest->IsDaily())
                && !(pQuest->GetType() == QUEST_TYPE_RAID)
                && !(pQuest->GetType() == QUEST_TYPE_DUNGEON)
                && pQuest->IsMarkedAsBugged()
                )
        {
            player->AutoCompleteQuest(pQuest);
        }
    }

    return true;
}

bool ChatHandler::HandleRemoveQuest(const char* args)
{
    Player* player = getSelectedPlayer();
    if(!player)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    // .removequest #entry'
    // number or [name] Shift-click form |color|Hquest:quest_id|h[name]|h|r
    char* cId = extractKeyFromLink((char*)args,"Hquest");
    if(!cId)
        return false;

    uint32 entry = atol(cId);

    Quest const* pQuest = sObjectMgr->GetQuestTemplate(entry);

    if(!pQuest)
    {
        PSendSysMessage(LANG_COMMAND_QUEST_NOTFOUND, entry);
        SetSentErrorMessage(true);
        return false;
    }

    // remove all quest entries for 'entry' from quest log
    for(uint8 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot )
    {
        uint32 quest = player->GetQuestSlotQuestId(slot);
        if(quest==entry)
        {
            player->SetQuestSlot(slot,0);

            // we ignore unequippable quest items in this case, its' still be equipped
            player->TakeQuestSourceItem( quest, false );
        }
    }

    // set quest status to not started (will updated in DB at next save)
    player->SetQuestStatus( entry, QUEST_STATUS_NONE);

    // reset rewarded for restart repeatable quest
    player->getQuestStatusMap()[entry].m_rewarded = false;

    SendSysMessage(LANG_COMMAND_QUEST_REMOVED);
    return true;
}

bool ChatHandler::HandleCompleteQuest(const char* args)
{
    Player* player = getSelectedPlayer();
    if(!player)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    // .quest complete #entry
    // number or [name] Shift-click form |color|Hquest:quest_id|h[name]|h|r
    char* cId = extractKeyFromLink((char*)args,"Hquest");
    if(!cId)
        return false;

    bool forceComplete = false;
    uint32 entry = 0;

    //if gm wants to force quest completion
    if( strcmp(cId, "force") == 0 )
    {
        char* tail = strtok(NULL,"");
        if(!tail)
            return false;
        cId = extractKeyFromLink(tail,"Hquest");
        if(!cId)
            return false;

        entry = atoi(cId);
        //TC_LOG_ERROR("command","DEBUG: ID value: %d", tEntry);
        if(!entry)
            return false;
            
        forceComplete = true;
    }
    else
        entry = atol(cId);

    Quest const* pQuest = sObjectMgr->GetQuestTemplate(entry);

    // If player doesn't have the quest
    if(!pQuest || player->GetQuestStatus(entry) == QUEST_STATUS_NONE)
    {
        PSendSysMessage(LANG_COMMAND_QUEST_NOTFOUND, entry);
        SetSentErrorMessage(true);
        return false;
    }
    
    QueryResult result = CharacterDatabase.PQuery("SELECT count FROM completed_quests WHERE guid = %u", player->GetGUID());
    
    uint8 completedQuestsThisWeek;
    if (result)
    {
        Field *fields = result->Fetch();
        completedQuestsThisWeek = fields[0].GetUInt8();
    }
    else //player has no completed quest this week
    {
        completedQuestsThisWeek = 0;
    }
 /* DISABLED
    if (completedQuestsThisWeek >= 2 && !forceComplete) //TODO: set a config option here ?
    {
        //tell the GM that this player has reached the maximum quests complete for this week
        PSendSysMessage(LANG_REACHED_QCOMPLETE_LIMIT, player->GetName().c_str());
        SetSentErrorMessage(true);
        return true;
    }*/
    // Add quest items for quests that require items
    for(uint8 x = 0; x < QUEST_OBJECTIVES_COUNT; ++x)
    {
        uint32 id = pQuest->RequiredItemId[x];
        uint32 count = pQuest->RequiredItemCount[x];
        if(!id || !count)
            continue;

        uint32 curItemCount = player->GetItemCount(id,true);

        ItemPosCountVec dest;
        uint8 msg = player->CanStoreNewItem( NULL_BAG, NULL_SLOT, dest, id, count-curItemCount );
        if( msg == EQUIP_ERR_OK )
        {
            Item* item = player->StoreNewItem( dest, id, true);
            player->SendNewItem(item,count-curItemCount,true,false);
        }
    }

    // All creature/GO slain/casted (not required, but otherwise it will display "Creature slain 0/10")
    for(uint8 i = 0; i < QUEST_OBJECTIVES_COUNT; i++)
    {
        uint32 creature = pQuest->RequiredNpcOrGo[i];
        uint32 creaturecount = pQuest->RequiredNpcOrGoCount[i];

        if(uint32 spell_id = pQuest->ReqSpell[i])
        {
            for(uint16 z = 0; z < creaturecount; ++z)
                player->CastedCreatureOrGO(creature,0,spell_id);
        }
        else if(creature > 0)
        {
            for(uint16 z = 0; z < creaturecount; ++z)
                player->KilledMonster(creature,0);
        }
        else if(creature < 0)
        {
            for(uint16 z = 0; z < creaturecount; ++z)
                player->CastedCreatureOrGO(creature,0,0);
        }
    }

    // If the quest requires reputation to complete
    if(uint32 repFaction = pQuest->GetRepObjectiveFaction())
    {
        uint32 repValue = pQuest->GetRepObjectiveValue();
        uint32 curRep = player->GetReputation(repFaction);
        if(curRep < repValue)
        {
            FactionEntry const *factionEntry = sFactionStore.LookupEntry(repFaction);
            player->SetFactionReputation(factionEntry,repValue);
        }
    }

    // If the quest requires money
    int32 ReqOrRewMoney = pQuest->GetRewOrReqMoney();
    if(ReqOrRewMoney < 0)
        player->ModifyMoney(-ReqOrRewMoney);

    player->CompleteQuest(entry);
    PSendSysMessage(LANG_QCOMPLETE_SUCCESS, entry, player->GetName().c_str()); //tell GM that the quest has been successfully completed
    
    if (completedQuestsThisWeek == 0) //entry does not exist, we have to create it
    {
        CharacterDatabase.PExecute("INSERT INTO completed_quests VALUES(%u, 1)", player->GetGUID());
    }
    else //entry exists, we have just to update it
    {
        CharacterDatabase.PExecute("UPDATE completed_quests SET count = count + 1 WHERE guid = %u", player->GetGUID());
    }
    
    // Check if quest already exists in bugged quests
    QueryResult questbug = WorldDatabase.PQuery("SELECT bugged FROM quest_bugs WHERE entry = %u", entry);
    if (questbug) 
    {
        WorldDatabase.PExecute("UPDATE quest_bugs SET completecount = completecount+1 WHERE entry = %u", entry);
    } else {
        WorldDatabase.PExecute("INSERT INTO quest_bugs VALUES (%u, 1, 0, '')", entry);
    }
    
    return true;
}

bool ChatHandler::HandleReportQuest(const char* args)
{
    ARGS_CHECK
        
    char* questIdStr = strtok((char*)args, " ");
    if (!questIdStr)
        return false;
    uint32 questId = atoi(questIdStr);
    if (!questId)
        return false;
        
    char* comment = strtok(NULL, "");
    if (!comment)
        return false;
        
    WorldDatabase.PQuery("UPDATE quest_bugs SET comment = \"%s\" WHERE entry = %u", comment, questId);
    
    return true;
}

//shows the number of completed quest this week, for selected character
bool ChatHandler::HandleCountCompleteQuest(const char* args)
{
    Player* player;
    uint64 targetGUID;
    if(!*args) //if no name provided, check if we have a player on target
    {
        player = getSelectedPlayer();
        if(!player)
        {
            SendSysMessage(LANG_NO_CHAR_SELECTED);
            SetSentErrorMessage(true);
            return false;
        }
        
        targetGUID = player->GetGUID();
    }
    else
    {
        std::string name = args;

        if(!normalizePlayerName(name))
        {
            SendSysMessage(LANG_PLAYER_NOT_FOUND);
            SetSentErrorMessage(true);
            return false;
        }

        player = sObjectAccessor->FindConnectedPlayerByName(name.c_str());
        if (player)
        {
            targetGUID = player->GetGUID();
        }
        else //player is not online, get GUID with another function
        {
            targetGUID = sObjectMgr->GetPlayerGUIDByName(name);
            if (!targetGUID) //player doesn't exist
            {
                SendSysMessage(LANG_PLAYER_NOT_FOUND);
                SetSentErrorMessage(true);
                return false;
            }
        }
    }
    
    QueryResult result = CharacterDatabase.PQuery("SELECT count FROM completed_quests WHERE guid = %u", targetGUID);
    uint8 completedQuestsThisWeek;
    if (result)
    {
        Field *fields = result->Fetch();
        completedQuestsThisWeek = fields[0].GetUInt8();
    }
    else //player has no completed quest this week
    {
        completedQuestsThisWeek = 0;
    }
    
    std::string displayName;
    if (CharacterNameData const* nameData = sWorld->GetCharacterNameData(targetGUID))
        displayName = nameData->m_name;
    else
        displayName = "<Unknown>";

    PSendSysMessage(LANG_QCOMPLETE_THIS_WEEK, displayName.c_str(), completedQuestsThisWeek);
    return true;
}

//shows the total number of quests completed by all gamemasters this week
bool ChatHandler::HandleTotalCount(const char* args)
{
    QueryResult result = CharacterDatabase.PQuery("SELECT SUM(count) FROM completed_quests");
    uint32 totalQuestsCompletedThisWeek;
    if (result)
    {
        Field* fields = result->Fetch();
        totalQuestsCompletedThisWeek = fields[0].GetUInt32();
    }
    else
        totalQuestsCompletedThisWeek = 0;
        
    PSendSysMessage(LANG_QCOMPLETE_TOTAL, totalQuestsCompletedThisWeek);
    return true;
}

bool ChatHandler::HandleBanAccountCommand(const char* args)
{
    return HandleBanHelper(BAN_ACCOUNT,args);
}

bool ChatHandler::HandleBanCharacterCommand(const char* args)
{
    return HandleBanHelper(BAN_CHARACTER,args);
}

bool ChatHandler::HandleBanIPCommand(const char* args)
{
    return HandleBanHelper(BAN_IP,args);
}

bool ChatHandler::HandleBanHelper(BanMode mode, const char* args)
{
    char* cnameOrIP = strtok ((char*)args, " ");
    if (!cnameOrIP)
        return false;

    std::string nameOrIP = cnameOrIP;

    char* duration = strtok (NULL," ");
    if(!duration || !atoi(duration))
        return false;

    char* reason = strtok (NULL,"");
    if(!reason)
        return false;

    switch(mode)
    {
        case BAN_ACCOUNT:
            if(!AccountMgr::normalizeString(nameOrIP))
            {
                PSendSysMessage(LANG_ACCOUNT_NOT_EXIST,nameOrIP.c_str());
                SetSentErrorMessage(true);
                return false;
            }
            break;
        case BAN_CHARACTER:
            if(!normalizePlayerName(nameOrIP))
            {
                SendSysMessage(LANG_PLAYER_NOT_FOUND);
                SetSentErrorMessage(true);
                return false;
            }
            break;
        case BAN_IP:
            if(!IsIPAddress(nameOrIP.c_str()))
                return false;
            break;
    }

    switch(sWorld->BanAccount(mode, nameOrIP, duration, reason,m_session ? m_session->GetPlayerName() : ""))
    {
        case BAN_SUCCESS:
            if(atoi(duration)>0)
                PSendSysMessage(LANG_BAN_YOUBANNED,nameOrIP.c_str(),secsToTimeString(TimeStringToSecs(duration),true).c_str(),reason);
            else
                PSendSysMessage(LANG_BAN_YOUPERMBANNED,nameOrIP.c_str(),reason);
            break;
        case BAN_SYNTAX_ERROR:
            return false;
        case BAN_NOTFOUND:
            switch(mode)
            {
                default:
                    PSendSysMessage(LANG_BAN_NOTFOUND,"account",nameOrIP.c_str());
                    break;
                case BAN_CHARACTER:
                    PSendSysMessage(LANG_BAN_NOTFOUND,"character",nameOrIP.c_str());
                    break;
                case BAN_IP:
                    PSendSysMessage(LANG_BAN_NOTFOUND,"ip",nameOrIP.c_str());
                    break;
            }
            SetSentErrorMessage(true);
            return false;
    }

    return true;
}

bool ChatHandler::HandleUnBanAccountCommand(const char* args)
{
    return HandleUnBanHelper(BAN_ACCOUNT,args);
}

bool ChatHandler::HandleUnBanCharacterCommand(const char* args)
{
    return HandleUnBanHelper(BAN_CHARACTER,args);
}

bool ChatHandler::HandleUnBanIPCommand(const char* args)
{
    return HandleUnBanHelper(BAN_IP,args);
}

bool ChatHandler::HandleUnBanHelper(BanMode mode, const char* args)
{
    char* cnameOrIP = strtok ((char*)args, " ");
    if(!cnameOrIP)
        return false;

    std::string nameOrIP = cnameOrIP;

    switch(mode)
    {
        case BAN_ACCOUNT:
            if(!AccountMgr::normalizeString(nameOrIP))
            {
                PSendSysMessage(LANG_ACCOUNT_NOT_EXIST,nameOrIP.c_str());
                SetSentErrorMessage(true);
                return false;
            }
            break;
        case BAN_CHARACTER:
            if(!normalizePlayerName(nameOrIP))
            {
                SendSysMessage(LANG_PLAYER_NOT_FOUND);
                SetSentErrorMessage(true);
                return false;
            }
            break;
        case BAN_IP:
            if(!IsIPAddress(nameOrIP.c_str()))
                return false;
            break;
    }

    if(sWorld->RemoveBanAccount(mode,nameOrIP))
        PSendSysMessage(LANG_UNBAN_UNBANNED,nameOrIP.c_str());
    else
        PSendSysMessage(LANG_UNBAN_ERROR,nameOrIP.c_str());

    return true;
}

bool ChatHandler::HandleBanInfoAccountCommand(const char* args)
{
    char* cname = strtok((char*)args, "");
    if(!cname)
        return false;

    std::string account_name = cname;
    if(!AccountMgr::normalizeString(account_name))
    {
        PSendSysMessage(LANG_ACCOUNT_NOT_EXIST,account_name.c_str());
        SetSentErrorMessage(true);
        return false;
    }

    uint32 accountid = sAccountMgr->GetId(account_name);
    if(!accountid)
    {
        PSendSysMessage(LANG_ACCOUNT_NOT_EXIST,account_name.c_str());
        return true;
    }

    return HandleBanInfoHelper(accountid,account_name.c_str());
}

bool ChatHandler::HandleBanInfoCharacterCommand(const char* args)
{
    char* cname = strtok ((char*)args, "");
    if(!cname)
        return false;

    std::string name = cname;
    if(!normalizePlayerName(name))
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    uint32 accountid = sObjectMgr->GetPlayerAccountIdByPlayerName(name);
    if(!accountid)
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    std::string accountname;
    if(!sAccountMgr->GetName(accountid,accountname))
    {
        PSendSysMessage(LANG_BANINFO_NOCHARACTER);
        return true;
    }

    return HandleBanInfoHelper(accountid,accountname.c_str());
}

bool ChatHandler::HandleBanInfoHelper(uint32 accountid, char const* accountname)
{
    QueryResult result = LoginDatabase.PQuery("SELECT FROM_UNIXTIME(bandate), unbandate-bandate, active, unbandate,banreason,bannedby FROM account_banned WHERE id = '%u' ORDER BY bandate ASC",accountid);
    if(!result)
    {
        PSendSysMessage(LANG_BANINFO_NOACCOUNTBAN, accountname);
        return true;
    }

    PSendSysMessage(LANG_BANINFO_BANHISTORY,accountname);
    do
    {
        Field* fields = result->Fetch();

        time_t unbandate = time_t(fields[3].GetUInt64());
        bool active = false;
        if(fields[2].GetBool() && (fields[1].GetUInt64() == (uint64)0 ||unbandate >= time(NULL)) )
            active = true;
        bool permanent = (fields[1].GetUInt64() == (uint64)0);
        std::string bantime = permanent?GetTrinityString(LANG_BANINFO_INFINITE):secsToTimeString(fields[1].GetUInt64(), true);
        PSendSysMessage(LANG_BANINFO_HISTORYENTRY,
            fields[0].GetCString(), bantime.c_str(), active ? GetTrinityString(LANG_BANINFO_YES):GetTrinityString(LANG_BANINFO_NO), fields[4].GetCString(), fields[5].GetCString());
    }while (result->NextRow());

    return true;
}

bool ChatHandler::HandleBanInfoIPCommand(const char* args)
{
    char* cIP = strtok ((char*)args, "");
    if(!cIP)
        return false;

    if (!IsIPAddress(cIP))
        return false;

    std::string IP = cIP;

    LoginDatabase.EscapeString(IP);
    QueryResult result = LoginDatabase.PQuery("SELECT ip, FROM_UNIXTIME(bandate), FROM_UNIXTIME(unbandate), unbandate-UNIX_TIMESTAMP(), banreason,bannedby,unbandate-bandate FROM ip_banned WHERE ip = '%s'",IP.c_str());
    if(!result)
    {
        PSendSysMessage(LANG_BANINFO_NOIP);
        return true;
    }

    Field *fields = result->Fetch();
    bool permanent = !fields[6].GetUInt64();
    PSendSysMessage(LANG_BANINFO_IPENTRY,
        fields[0].GetCString(), fields[1].GetCString(), permanent ? GetTrinityString(LANG_BANINFO_NEVER):fields[2].GetCString(),
        permanent ? GetTrinityString(LANG_BANINFO_INFINITE):secsToTimeString(fields[3].GetUInt64(), true).c_str(), fields[4].GetCString(), fields[5].GetCString());

    return true;
}

bool ChatHandler::HandleMuteInfoAccountCommand(const char* args)
{
    ARGS_CHECK
        
    char* cname = strtok((char*)args, "");
    if(!cname)
        return false;

    std::string account_name = cname;
    if(!AccountMgr::normalizeString(account_name))
    {
        PSendSysMessage(LANG_ACCOUNT_NOT_EXIST,account_name.c_str());
        SetSentErrorMessage(true);
        return false;
    }

    uint32 accountid = sAccountMgr->GetId(account_name);
    if(!accountid)
    {
        PSendSysMessage(LANG_ACCOUNT_NOT_EXIST,account_name.c_str());
        return true;
    }
    
    QueryResult result = LogsDatabase.PQuery("SELECT duration, reason, author, FROM_UNIXTIME(time), time FROM sanctions WHERE acctid = %d AND type = %u", accountid, uint32(SANCTION_MUTE));
    if (!result) {
        PSendSysMessage("No sanction on this account.");
        return true;
    }
    
    do {
        Field* fields = result->Fetch();
        uint32 duration = fields[0].GetUInt32();
        std::string reason = fields[1].GetString();
        uint32 authorGUID = fields[2].GetUInt32();
        std::string unbanstr = fields[3].GetString();
        uint64 unbantimestamp = fields[4].GetUInt64() + (duration * 60);
        std::string authorname;
        std::string displayName;
        if (CharacterNameData const* nameData = sWorld->GetCharacterNameData(authorGUID))
            authorname = nameData->m_name;
        else
            authorname = "<Unknown>";

        PSendSysMessage("Account %u: Mute %s for %s by %s (%s)%s.", accountid, secsToTimeString(fields[0].GetUInt32()).c_str(), reason.c_str(), authorname.c_str(), unbanstr.c_str(), (unbantimestamp > uint64(time(NULL))) ? " (actif)" : "");
    } while (result->NextRow());
    
    return true;
}

bool ChatHandler::HandleMuteInfoCharacterCommand(char const* args)
{
    char* cname = strtok ((char*)args, "");
    if(!cname)
        return false;

    std::string name = cname;
    if(!normalizePlayerName(name))
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    uint32 accountid = sObjectMgr->GetPlayerAccountIdByPlayerName(name);
    if(!accountid)
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    std::string accountname;
    if(!sAccountMgr->GetName(accountid,accountname))
    {
        PSendSysMessage(LANG_BANINFO_NOCHARACTER);
        return true;
    }
    
    QueryResult result = LogsDatabase.PQuery("SELECT duration, reason, author, FROM_UNIXTIME(time), time FROM sanctions WHERE acctid = %d AND type = 0", accountid);
    if (!result) {
        //PSendSysMessage("Aucune sanction pour le compte de ce personnage.");
        PSendSysMessage("No sanction found for this character.");
        return true;
    }
    
    do {
        Field* fields = result->Fetch();
        std::string reason = fields[1].GetString();
        uint32 duration = fields[0].GetUInt32();
        uint32 authorGUID = fields[2].GetUInt32();
        std::string unbanstr = fields[3].GetString();
        uint64 unbantimestamp = fields[4].GetUInt64() + (duration * 60);
        std::string authorname;
        if (CharacterNameData const* nameData = sWorld->GetCharacterNameData(authorGUID))
            authorname = nameData->m_name;
        else
            authorname = "<Unknown>";

        PSendSysMessage("Account %d: Mute %s by \"%s\" for %s (%s)%s.", accountid, secsToTimeString(fields[0].GetUInt32()).c_str(), reason.c_str(), authorname.c_str(), unbanstr.c_str(), (unbantimestamp > uint64(time(NULL))) ? " (actif)" : "");
    } while (result->NextRow());
    
    return true;
}

bool ChatHandler::HandleBanListCharacterCommand(const char* args)
{
    LoginDatabase.Execute("DELETE FROM ip_banned WHERE unbandate<=UNIX_TIMESTAMP() AND unbandate<>bandate");

    char* cFilter = strtok ((char*)args, " ");
    if(!cFilter)
        return false;

    std::string filter = cFilter;
    LoginDatabase.EscapeString(filter);
    QueryResult result = CharacterDatabase.PQuery("SELECT account FROM characters WHERE name " _LIKE_ " " _CONCAT3_ ("'%%'","'%s'","'%%'"),filter.c_str());
    if (!result)
    {
        PSendSysMessage(LANG_BANLIST_NOCHARACTER);
        return true;
    }

    return HandleBanListHelper(result);
}

bool ChatHandler::HandleBanListAccountCommand(const char* args)
{
    LoginDatabase.Execute("DELETE FROM ip_banned WHERE unbandate<=UNIX_TIMESTAMP() AND unbandate<>bandate");

    char* cFilter = strtok((char*)args, " ");
    std::string filter = cFilter ? cFilter : "";
    LoginDatabase.EscapeString(filter);

    QueryResult result;

    if(filter.empty())
    {
        result = LoginDatabase.Query("SELECT account.id, username FROM account, account_banned"
            " WHERE account.id = account_banned.id AND active = 1 GROUP BY account.id");
    }
    else
    {
        result = LoginDatabase.PQuery("SELECT account.id, username FROM account, account_banned"
            " WHERE account.id = account_banned.id AND active = 1 AND username " _LIKE_ " " _CONCAT3_ ("'%%'","'%s'","'%%'")" GROUP BY account.id",
            filter.c_str());
    }

    if (!result)
    {
        PSendSysMessage(LANG_BANLIST_NOACCOUNT);
        return true;
    }

    return HandleBanListHelper(result);
}

bool ChatHandler::HandleBanListHelper(QueryResult result)
{
    PSendSysMessage(LANG_BANLIST_MATCHINGACCOUNT);

    // Chat short output
    if(m_session)
    {
        do
        {
            Field* fields = result->Fetch();
            uint32 accountid = fields[0].GetUInt32();

            QueryResult banresult = LoginDatabase.PQuery("SELECT account.username FROM account,account_banned WHERE account_banned.id='%u' AND account_banned.id=account.id",accountid);
            if(banresult)
            {
                Field* fields2 = banresult->Fetch();
                PSendSysMessage("%s",fields2[0].GetCString());
            }
        } while (result->NextRow());
    }
    // Console wide output
    else
    {
        SendSysMessage(LANG_BANLIST_ACCOUNTS);
        SendSysMessage("===============================================================================");
        SendSysMessage(LANG_BANLIST_ACCOUNTS_HEADER);
        do
        {
            SendSysMessage("-------------------------------------------------------------------------------");
            Field *fields = result->Fetch();
            uint32 account_id = fields[0].GetUInt32 ();

            std::string account_name;

            // "account" case, name can be get in same query
            if(result->GetFieldCount() > 1)
                account_name = fields[1].GetString();
            // "character" case, name need extract from another DB
            else
                sAccountMgr->GetName (account_id,account_name);

            // No SQL injection. id is uint32.
            QueryResult banInfo = LoginDatabase.PQuery("SELECT bandate,unbandate,bannedby,banreason FROM account_banned WHERE id = %u ORDER BY unbandate", account_id);
            if (banInfo)
            {
                Field *fields2 = banInfo->Fetch();
                do
                {
                    time_t t_ban = fields2[0].GetUInt64();
                    tm* aTm_ban = localtime(&t_ban);

                    if (fields2[0].GetUInt64() == fields2[1].GetUInt64())
                    {
                        PSendSysMessage("|%-15.15s|%02d-%02d-%02d %02d:%02d|   permanent  |%-15.15s|%-15.15s|",
                            account_name.c_str(),aTm_ban->tm_year%100, aTm_ban->tm_mon+1, aTm_ban->tm_mday, aTm_ban->tm_hour, aTm_ban->tm_min,
                            fields2[2].GetCString(),fields2[3].GetCString());
                    }
                    else
                    {
                        time_t t_unban = fields2[1].GetUInt64();
                        tm* aTm_unban = localtime(&t_unban);
                        PSendSysMessage("|%-15.15s|%02d-%02d-%02d %02d:%02d|%02d-%02d-%02d %02d:%02d|%-15.15s|%-15.15s|",
                            account_name.c_str(),aTm_ban->tm_year%100, aTm_ban->tm_mon+1, aTm_ban->tm_mday, aTm_ban->tm_hour, aTm_ban->tm_min,
                            aTm_unban->tm_year%100, aTm_unban->tm_mon+1, aTm_unban->tm_mday, aTm_unban->tm_hour, aTm_unban->tm_min,
                            fields2[2].GetCString(),fields2[3].GetCString());
                    }
                }while ( banInfo->NextRow() );
            }
        }while( result->NextRow() );
        SendSysMessage("===============================================================================");
    }

    return true;
}

bool ChatHandler::HandleBanListIPCommand(const char* args)
{
    LoginDatabase.Execute("DELETE FROM ip_banned WHERE unbandate<=UNIX_TIMESTAMP() AND unbandate<>bandate");

    char* cFilter = strtok((char*)args, " ");
    std::string filter = cFilter ? cFilter : "";
    LoginDatabase.EscapeString(filter);

    QueryResult result;

    if(filter.empty())
    {
        result = LoginDatabase.Query ("SELECT ip,bandate,unbandate,bannedby,banreason FROM ip_banned"
            " WHERE (bandate=unbandate OR unbandate>UNIX_TIMESTAMP())"
            " ORDER BY unbandate" );
    }
    else
    {
        result = LoginDatabase.PQuery( "SELECT ip,bandate,unbandate,bannedby,banreason FROM ip_banned"
            " WHERE (bandate=unbandate OR unbandate>UNIX_TIMESTAMP()) AND ip " _LIKE_ " " _CONCAT3_ ("'%%'","'%s'","'%%'")
            " ORDER BY unbandate",filter.c_str() );
    }

    if(!result)
    {
        PSendSysMessage(LANG_BANLIST_NOIP);
        return true;
    }

    PSendSysMessage(LANG_BANLIST_MATCHINGIP);
    // Chat short output
    if(m_session)
    {
        do
        {
            Field* fields = result->Fetch();
            PSendSysMessage("%s",fields[0].GetCString());
        } while (result->NextRow());
    }
    // Console wide output
    else
    {
        SendSysMessage(LANG_BANLIST_IPS);
        SendSysMessage("===============================================================================");
        SendSysMessage(LANG_BANLIST_IPS_HEADER);
        do
        {
            SendSysMessage("-------------------------------------------------------------------------------");
            Field *fields = result->Fetch();
            time_t t_ban = fields[1].GetUInt64();
            tm* aTm_ban = localtime(&t_ban);
            if ( fields[1].GetUInt64() == fields[2].GetUInt64() )
            {
                PSendSysMessage("|%-15.15s|%02d-%02d-%02d %02d:%02d|   permanent  |%-15.15s|%-15.15s|",
                    fields[0].GetCString(), aTm_ban->tm_year%100, aTm_ban->tm_mon+1, aTm_ban->tm_mday, aTm_ban->tm_hour, aTm_ban->tm_min,
                    fields[3].GetCString(), fields[4].GetCString());
            }
            else
            {
                time_t t_unban = fields[2].GetUInt64();
                tm* aTm_unban = localtime(&t_unban);
                PSendSysMessage("|%-15.15s|%02d-%02d-%02d %02d:%02d|%02d-%02d-%02d %02d:%02d|%-15.15s|%-15.15s|",
                    fields[0].GetCString(), aTm_ban->tm_year%100, aTm_ban->tm_mon+1, aTm_ban->tm_mday, aTm_ban->tm_hour, aTm_ban->tm_min,
                    aTm_unban->tm_year%100, aTm_unban->tm_mon+1, aTm_unban->tm_mday, aTm_unban->tm_hour, aTm_unban->tm_min,
                    fields[3].GetCString(), fields[4].GetCString());
            }
        }while( result->NextRow() );
        SendSysMessage("===============================================================================");
    }

    return true;
}

bool ChatHandler::HandleRespawnCommand(const char* /*args*/)
{
    

    Player* pl = m_session->GetPlayer();

    // accept only explicitly selected target (not implicitly self targeting case)
    Unit* target = getSelectedUnit();
    if(pl->GetTarget() && target)
    {
        if(target->GetTypeId()!=TYPEID_UNIT)
        {
            SendSysMessage(LANG_SELECT_CREATURE);
            SetSentErrorMessage(true);
            return false;
        }

        if(target->IsDead())
            (target->ToCreature())->Respawn();
        return true;
    }

    CellCoord p(Trinity::ComputeCellCoord(pl->GetPositionX(), pl->GetPositionY()));
    Cell cell(p);
    cell.data.Part.reserved = ALL_DISTRICT;
    cell.SetNoCreate();

    Trinity::RespawnDo u_do;
    Trinity::WorldObjectWorker<Trinity::RespawnDo> worker(u_do);

    TypeContainerVisitor<Trinity::WorldObjectWorker<Trinity::RespawnDo>, GridTypeMapContainer > obj_worker(worker);
    cell.Visit(p, obj_worker, *pl->GetMap());

    return true;
}

bool ChatHandler::HandleFlyModeCommand(const char* args)
{
    

    Unit *unit = getSelectedUnit();
    if (!unit || (unit->GetTypeId() != TYPEID_PLAYER))
        unit = m_session->GetPlayer();

    WorldPacket data(12);
    if (strncmp(args, "on", 3) == 0)
    {
        ((Player*)(unit))->SetFlying(true);
    }
    else if (strncmp(args, "off", 4) == 0)
    {
        ((Player*)(unit))->SetFlying(false);
    }
    else
    {
        SendSysMessage(LANG_USE_BOL);
        return false;
    }
    PSendSysMessage(LANG_COMMAND_FLYMODE_STATUS, unit->GetName().c_str(), args);
    return true;
}

bool ChatHandler::HandleLoadPDumpCommand(const char *args)
{
    char * file = strtok((char*)args, " ");
    if(!file)
        return false;

    char * account = strtok(NULL, " ");
    if(!account)
        return false;

    std::string account_name = account;
    if(!AccountMgr::normalizeString(account_name))
    {
        PSendSysMessage(LANG_ACCOUNT_NOT_EXIST,account_name.c_str());
        SetSentErrorMessage(true);
        return false;
    }

    uint32 account_id = sAccountMgr->GetId(account_name);
    if(!account_id)
    {
        account_id = atoi(account);                             // use original string
        if(!account_id)
        {
            PSendSysMessage(LANG_ACCOUNT_NOT_EXIST,account_name.c_str());
            SetSentErrorMessage(true);
            return false;
        }
    }

    if(!sAccountMgr->GetName(account_id,account_name))
    {
        PSendSysMessage(LANG_ACCOUNT_NOT_EXIST,account_name.c_str());
        SetSentErrorMessage(true);
        return false;
    }

    char* guid_str = NULL;
    char* name_str = strtok(NULL, " ");

    std::string name;
    if(name_str)
    {
        name = name_str;
        // normalize the name if specified and check if it exists
        if(!normalizePlayerName(name))
        {
            PSendSysMessage(LANG_INVALID_CHARACTER_NAME);
            SetSentErrorMessage(true);
            return false;
        }

        if(!ObjectMgr::CheckPlayerName(name,true))
        {
            PSendSysMessage(LANG_INVALID_CHARACTER_NAME);
            SetSentErrorMessage(true);
            return false;
        }

        guid_str = strtok(NULL, " ");
    }

    uint32 guid = 0;

    if(guid_str)
    {
        guid = atoi(guid_str);
        if(!guid)
        {
            PSendSysMessage(LANG_INVALID_CHARACTER_GUID);
            SetSentErrorMessage(true);
            return false;
        }

        if(sObjectMgr->GetPlayerAccountIdByGUID(guid))
        {
            PSendSysMessage(LANG_CHARACTER_GUID_IN_USE,guid);
            SetSentErrorMessage(true);
            return false;
        }
    }

    switch(PlayerDumpReader().LoadDump(file, account_id, name, guid))
    {
        case DUMP_SUCCESS:
            PSendSysMessage(LANG_COMMAND_IMPORT_SUCCESS);
            break;
        case DUMP_FILE_OPEN_ERROR:
            PSendSysMessage(LANG_FILE_OPEN_FAIL,file);
            SetSentErrorMessage(true);
            return false;
        case DUMP_FILE_BROKEN:
            PSendSysMessage(LANG_DUMP_BROKEN,file);
            SetSentErrorMessage(true);
            return false;
        case DUMP_TOO_MANY_CHARS:
            PSendSysMessage(LANG_ACCOUNT_CHARACTER_LIST_FULL,account_name.c_str(),account_id);
            SetSentErrorMessage(true);
            return false;
        default:
            PSendSysMessage(LANG_COMMAND_IMPORT_FAILED);
            SetSentErrorMessage(true);
            return false;
    }

    return true;
}

bool ChatHandler::HandleNpcChangeEntryCommand(const char *args)
{
    uint32 newEntryNum = atoi(args);
    if(!newEntryNum)
        return false;

    Unit* unit = getSelectedUnit();
    if(!unit || unit->GetTypeId() != TYPEID_UNIT)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }
    Creature* creature = unit->ToCreature();
    if(creature->UpdateEntry(newEntryNum))
        SendSysMessage(LANG_DONE);
    else
        SendSysMessage(LANG_ERROR);
    return true;
}

bool ChatHandler::HandleWritePDumpCommand(const char *args)
{
    char* file = strtok((char*)args, " ");
    char* p2 = strtok(NULL, " ");

    if(!file || !p2)
        return false;

    uint32 guid;
    // character name can't start from number
    if (isNumeric(p2[0]))
        guid = atoi(p2);
    else
    {
        std::string name = p2;

        if (!normalizePlayerName (name))
        {
            SendSysMessage (LANG_PLAYER_NOT_FOUND);
            SetSentErrorMessage (true);
            return false;
        }

        guid = sObjectMgr->GetPlayerGUIDByName(name);
    }

    if(!sObjectMgr->GetPlayerAccountIdByGUID(guid))
    {
        PSendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    switch(PlayerDumpWriter().WriteDump(file, guid))
    {
        case DUMP_SUCCESS:
            PSendSysMessage(LANG_COMMAND_EXPORT_SUCCESS);
            break;
        case DUMP_FILE_OPEN_ERROR:
            PSendSysMessage(LANG_FILE_OPEN_FAIL,file);
            SetSentErrorMessage(true);
            return false;
        default:
            PSendSysMessage(LANG_COMMAND_EXPORT_FAILED);
            SetSentErrorMessage(true);
            return false;
    }

    return true;
}

bool ChatHandler::HandleMovegensCommand(const char* /*args*/)
{
    Unit* unit = getSelectedUnit();
    if(!unit)
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    PSendSysMessage(LANG_MOVEGENS_LIST,(unit->GetTypeId()==TYPEID_PLAYER ? "Player" : "Creature" ),unit->GetGUIDLow());

    MotionMaster* motionMaster = unit->GetMotionMaster();
    float x, y, z;
    motionMaster->GetDestination(x, y, z);

    for (uint8 i = 0; i < MAX_MOTION_SLOT; ++i)
    {
        MovementGenerator* movementGenerator = motionMaster->GetMotionSlot(i);
        if (!movementGenerator)
        {
            SendSysMessage("Empty");
            continue;
        }

        switch (movementGenerator->GetMovementGeneratorType())
        {
            case IDLE_MOTION_TYPE:
                SendSysMessage(LANG_MOVEGENS_IDLE);
                break;
            case RANDOM_MOTION_TYPE:
                SendSysMessage(LANG_MOVEGENS_RANDOM);
                break;
            case WAYPOINT_MOTION_TYPE:
                SendSysMessage(LANG_MOVEGENS_WAYPOINT);
                break;
            case ANIMAL_RANDOM_MOTION_TYPE:
                SendSysMessage(LANG_MOVEGENS_ANIMAL_RANDOM);
                break;
            case CONFUSED_MOTION_TYPE:
                SendSysMessage(LANG_MOVEGENS_CONFUSED);
                break;
            case CHASE_MOTION_TYPE:
            {
                Unit* target = NULL;
                if (unit->GetTypeId() == TYPEID_PLAYER)
                    target = static_cast<ChaseMovementGenerator<Player> const*>(movementGenerator)->GetTarget();
                else
                    target = static_cast<ChaseMovementGenerator<Creature> const*>(movementGenerator)->GetTarget();

                if (!target)
                    SendSysMessage(LANG_MOVEGENS_CHASE_NULL);
                else if (target->GetTypeId() == TYPEID_PLAYER)
                    PSendSysMessage(LANG_MOVEGENS_CHASE_PLAYER, target->GetName().c_str(), target->GetGUIDLow());
                else
                    PSendSysMessage(LANG_MOVEGENS_CHASE_CREATURE, target->GetName().c_str(), target->GetGUIDLow());
                break;
            }
            case FOLLOW_MOTION_TYPE:
            {
                Unit* target = NULL;
                if (unit->GetTypeId() == TYPEID_PLAYER)
                    target = static_cast<FollowMovementGenerator<Player> const*>(movementGenerator)->GetTarget();
                else
                    target = static_cast<FollowMovementGenerator<Creature> const*>(movementGenerator)->GetTarget();

                if (!target)
                    SendSysMessage(LANG_MOVEGENS_FOLLOW_NULL);
                else if (target->GetTypeId() == TYPEID_PLAYER)
                    PSendSysMessage(LANG_MOVEGENS_FOLLOW_PLAYER, target->GetName().c_str(), target->GetGUIDLow());
                else
                    PSendSysMessage(LANG_MOVEGENS_FOLLOW_CREATURE, target->GetName().c_str(), target->GetGUIDLow());
                break;
            }
            case HOME_MOTION_TYPE:
            {
                if (unit->GetTypeId() == TYPEID_UNIT)
                    PSendSysMessage(LANG_MOVEGENS_HOME_CREATURE, x, y, z);
                else
                    SendSysMessage(LANG_MOVEGENS_HOME_PLAYER);
                break;
            }
            case FLIGHT_MOTION_TYPE:
                SendSysMessage(LANG_MOVEGENS_FLIGHT);
                break;
            case POINT_MOTION_TYPE:
                PSendSysMessage(LANG_MOVEGENS_POINT, x, y, z);
                break;
            case FLEEING_MOTION_TYPE:
                SendSysMessage(LANG_MOVEGENS_FEAR);
                break;
            case DISTRACT_MOTION_TYPE:
                SendSysMessage(LANG_MOVEGENS_DISTRACT);
                break;
            case EFFECT_MOTION_TYPE:
                SendSysMessage(LANG_MOVEGENS_EFFECT);
                break;
            default:
                PSendSysMessage(LANG_MOVEGENS_UNKNOWN, movementGenerator->GetMovementGeneratorType());
                break;
        }
    }
    
    return true;
}

bool ChatHandler::HandlePLimitCommand(const char *args)
{
    if(*args)
    {
        char* param = strtok((char*)args, " ");
        if(!param)
            return false;

        int l = strlen(param);

        if(     strncmp(param,"player",l) == 0 )
            sWorld->SetPlayerLimit(-SEC_PLAYER);
        else if(strncmp(param,"moderator",l) == 0 )
            sWorld->SetPlayerLimit(-SEC_GAMEMASTER1);
        else if(strncmp(param,"gamemaster",l) == 0 )
            sWorld->SetPlayerLimit(-SEC_GAMEMASTER2);
        else if(strncmp(param,"administrator",l) == 0 )
            sWorld->SetPlayerLimit(-SEC_GAMEMASTER3);
        else if(strncmp(param,"reset",l) == 0 )
            sWorld->SetPlayerLimit( sConfigMgr->GetIntDefault("PlayerLimit", DEFAULT_PLAYER_LIMIT) );
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

    PSendSysMessage("Player limits: amount %u, min. security level %s.",pLimit,secName);

    return true;
}

bool ChatHandler::HandleCastCommand(const char* args)
{
    
    ARGS_CHECK

    Unit* target = getSelectedUnit();

    if(!target)
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
    uint32 spell = extractSpellIdFromLink((char*)args);
    if(!spell)
        return false;

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spell);
    if(!spellInfo)
        return false;

    if(!SpellMgr::IsSpellValid(spellInfo,m_session->GetPlayer()))
    {
        PSendSysMessage(LANG_COMMAND_SPELL_BROKEN,spell);
        SetSentErrorMessage(true);
        return false;
    }

    char* trig_str = strtok(NULL, " ");
    if(trig_str)
    {
        int l = strlen(trig_str);
        if(strncmp(trig_str,"triggered",l) != 0 )
            return false;
    }

    bool triggered = (trig_str != NULL);

    m_session->GetPlayer()->CastSpell(target,spell,triggered);

    return true;
}

/** args : <spellid> <count> */
bool ChatHandler::HandleCastBatchCommand(const char* args)
{
    Unit* target = getSelectedUnit();

    if(!target)
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r
    // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
    uint32 spell = extractSpellIdFromLink((char*)args);
    if(!spell || !sSpellMgr->GetSpellInfo(spell))
        return false;

    char* countStr = strtok(NULL, " ");
    int count = 0;
    if(countStr)
    {
        count = atoi(countStr);
        if(count == 0)
            return false;
    }

    for(int i = 0; i < count; i++)
        m_session->GetPlayer()->CastSpell(target, spell, true);

    return true;
}

bool ChatHandler::HandleCastBackCommand(const char* args)
{
    Unit* caster = getSelectedUnit();

    if(!caster)
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r
    // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
    uint32 spell = extractSpellIdFromLink((char*)args);
    if(!spell || !sSpellMgr->GetSpellInfo(spell))
        return false;

    char* trig_str = strtok(NULL, " ");
    if(trig_str)
    {
        int l = strlen(trig_str);
        if(strncmp(trig_str,"triggered",l) != 0 )
            return false;
    }

    bool triggered = (trig_str != NULL);

    // update orientation at server
    caster->SetOrientation(caster->GetAngle(m_session->GetPlayer()));

    // and client
    WorldPacket data;
    caster->BuildHeartBeatMsg(&data);
    caster->SendMessageToSet(&data,true);

    caster->CastSpell(m_session->GetPlayer(),spell,triggered);

    return true;
}

bool ChatHandler::HandleCastDistCommand(const char* args)
{
    
    ARGS_CHECK

    // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
    uint32 spell = extractSpellIdFromLink((char*)args);
    if(!spell)
        return false;

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spell);
    if(!spellInfo)
        return false;

    if(!SpellMgr::IsSpellValid(spellInfo,m_session->GetPlayer()))
    {
        PSendSysMessage(LANG_COMMAND_SPELL_BROKEN,spell);
        SetSentErrorMessage(true);
        return false;
    }

    char *distStr = strtok(NULL, " ");

    float dist = 0;

    if(distStr)
        sscanf(distStr, "%f", &dist);

    char* trig_str = strtok(NULL, " ");
    if(trig_str)
    {
        int l = strlen(trig_str);
        if(strncmp(trig_str,"triggered",l) != 0 )
            return false;
    }

    bool triggered = (trig_str != NULL);

    float x,y,z;
    m_session->GetPlayer()->GetClosePoint(x,y,z,dist);

    m_session->GetPlayer()->CastSpell(x,y,z,spell,triggered);
    return true;
}

bool ChatHandler::HandleCastTargetCommand(const char* args)
{
    

    Creature* caster = getSelectedCreature();

    if(!caster)
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    if(!caster->GetVictim())
    {
        SendSysMessage(LANG_SELECTED_TARGET_NOT_HAVE_VICTIM);
        SetSentErrorMessage(true);
        return false;
    }

    // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
    uint32 spell = extractSpellIdFromLink((char*)args);
    if(!spell || !sSpellMgr->GetSpellInfo(spell))
        return false;

    char* trig_str = strtok(NULL, " ");
    if(trig_str)
    {
        int l = strlen(trig_str);
        if(strncmp(trig_str,"triggered",l) != 0 )
            return false;
    }

    bool triggered = (trig_str != NULL);

    // update orientation at server
    caster->SetOrientation(caster->GetAngle(m_session->GetPlayer()));

    // and client
    WorldPacket data;
    caster->BuildHeartBeatMsg(&data);
    caster->SendMessageToSet(&data,true);

    caster->CastSpell(caster->GetVictim(),spell,triggered);

    return true;
}

/*
ComeToMe command REQUIRED for 3rd party scripting library to have access to PointMovementGenerator
Without this function 3rd party scripting library will get linking errors (unresolved external)
when attempting to use the PointMovementGenerator
*/
bool ChatHandler::HandleComeToMeCommand(const char *args)
{
    

    char* newFlagStr = strtok((char*)args, " ");

    if(!newFlagStr)
        return false;

    uint32 newFlags = (uint32)strtoul(newFlagStr, NULL, 0);

    Creature* caster = getSelectedCreature();
    if(!caster)
    {
        m_session->GetPlayer()->SetUnitMovementFlags(newFlags);
        SendSysMessage(LANG_SELECT_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    caster->SetUnitMovementFlags(newFlags);

    Player* pl = m_session->GetPlayer();

    caster->GetMotionMaster()->MovePoint(0, pl->GetPositionX(), pl->GetPositionY(), pl->GetPositionZ());
    return true;
}

bool ChatHandler::HandleCastSelfCommand(const char* args)
{
    
    ARGS_CHECK

    Unit* target = getSelectedUnit();

    if(!target)
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
    uint32 spell = extractSpellIdFromLink((char*)args);
    if(!spell)
        return false;

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spell);
    if(!spellInfo)
        return false;

    if(!SpellMgr::IsSpellValid(spellInfo,m_session->GetPlayer()))
    {
        PSendSysMessage(LANG_COMMAND_SPELL_BROKEN,spell);
        SetSentErrorMessage(true);
        return false;
    }

    target->CastSpell(target,spell,false);

    return true;
}

std::string GetTimeString(uint32 time)
{
    uint16 days = time / DAY, hours = (time % DAY) / HOUR, minute = (time % HOUR) / MINUTE;
    std::ostringstream ss;
    if(days) ss << days << "d ";
    if(hours) ss << hours << "h ";
    ss << minute << "m";
    return ss.str();
}

bool ChatHandler::HandleInstanceListBindsCommand(const char* /*args*/)
{
    Player* player = getSelectedPlayer();
    if (!player) player = m_session->GetPlayer();
    uint32 counter = 0;
    for(uint8 i = 0; i < TOTAL_DIFFICULTIES; i++)
    {
        Player::BoundInstancesMap &binds = player->GetBoundInstances(i);
        for(Player::BoundInstancesMap::iterator itr = binds.begin(); itr != binds.end(); ++itr)
        {
            InstanceSave *save = itr->second.save;
            std::string timeleft = GetTimeString(save->GetResetTime() - time(NULL));
            PSendSysMessage("map: %d inst: %d perm: %s diff: %s canReset: %s TTR: %s", itr->first, save->GetInstanceId(), itr->second.perm ? "yes" : "no",  save->GetDifficulty() == DIFFICULTY_NORMAL ? "normal" : "heroic", save->CanReset() ? "yes" : "no", timeleft.c_str());
            counter++;
        }
    }
    PSendSysMessage("player binds: %d", counter);
    counter = 0;
    Group *group = player->GetGroup();
    if(group)
    {
        for(uint8 i = 0; i < TOTAL_DIFFICULTIES; i++)
        {
            Group::BoundInstancesMap &binds = group->GetBoundInstances(i);
            for(Group::BoundInstancesMap::iterator itr = binds.begin(); itr != binds.end(); ++itr)
            {
                InstanceSave *save = itr->second.save;
                std::string timeleft = GetTimeString(save->GetResetTime() - time(NULL));
                PSendSysMessage("map: %d inst: %d perm: %s diff: %s canReset: %s TTR: %s", itr->first, save->GetInstanceId(), itr->second.perm ? "yes" : "no",  save->GetDifficulty() == DIFFICULTY_NORMAL ? "normal" : "heroic", save->CanReset() ? "yes" : "no", timeleft.c_str());
                counter++;
            }
        }
    }
    PSendSysMessage("group binds: %d", counter);

    return true;
}

bool ChatHandler::HandleInstanceUnbindCommand(const char* args)
{
    
    ARGS_CHECK

    std::string cmd = args;
    if(cmd == "all")
    {
        Player* player = getSelectedPlayer();
        if (!player) player = m_session->GetPlayer();
        uint32 counter = 0;
        for(uint8 i = 0; i < TOTAL_DIFFICULTIES; i++)
        {
            Player::BoundInstancesMap &binds = player->GetBoundInstances(i);
            for(Player::BoundInstancesMap::iterator itr = binds.begin(); itr != binds.end();)
            {
                if(itr->first != player->GetMapId())
                {
                    InstanceSave *save = itr->second.save;
                    std::string timeleft = GetTimeString(save->GetResetTime() - time(NULL));
                    PSendSysMessage("unbinding map: %d inst: %d perm: %s diff: %s canReset: %s TTR: %s", itr->first, save->GetInstanceId(), itr->second.perm ? "yes" : "no",  save->GetDifficulty() == DIFFICULTY_NORMAL ? "normal" : "heroic", save->CanReset() ? "yes" : "no", timeleft.c_str());
                    player->UnbindInstance(itr, i);
                    counter++;
                }
                else
                    ++itr;
            }
        }
        PSendSysMessage("instances unbound: %d", counter);
    }
    return true;
}

bool ChatHandler::HandleInstanceStatsCommand(const char* /*args*/)
{
    PSendSysMessage("instances loaded: %d", sMapMgr->GetNumInstances());
    PSendSysMessage("players in instances: %d", sMapMgr->GetNumPlayersInInstances());
    PSendSysMessage("instance saves: %d", sInstanceSaveMgr->GetNumInstanceSaves());
    PSendSysMessage("players bound: %d", sInstanceSaveMgr->GetNumBoundPlayersTotal());
    PSendSysMessage("groups bound: %d", sInstanceSaveMgr->GetNumBoundGroupsTotal());
    return true;
}

bool ChatHandler::HandleInstanceSaveDataCommand(const char * /*args*/)
{
    

    Player* pl = m_session->GetPlayer();

    Map* map = pl->GetMap();
    if (!map->IsDungeon())
    {
        PSendSysMessage("Map is not a dungeon.");
        SetSentErrorMessage(true);
        return false;
    }

    if (!((InstanceMap*)map)->GetInstanceData())
    {
        PSendSysMessage("Map has no instance data.");
        SetSentErrorMessage(true);
        return false;
    }

    ((InstanceMap*)map)->GetInstanceData()->SaveToDB();
    return true;
}

/// Display the list of GMs
bool ChatHandler::HandleGMListFullCommand(const char* /*args*/)
{
    ///- Get the accounts with GM Level >0
    QueryResult result = LoginDatabase.Query( "SELECT username,gmlevel FROM account WHERE gmlevel > 0" );
    if(result)
    {
        SendSysMessage(LANG_GMLIST);
        SendSysMessage("========================");
        SendSysMessage(LANG_GMLIST_HEADER);
        SendSysMessage("========================");

        ///- Circle through them. Display username and GM level
        do
        {
            Field *fields = result->Fetch();
            PSendSysMessage("|%15s|%6s|", fields[0].GetCString(),fields[1].GetCString());
        }while( result->NextRow() );

        PSendSysMessage("========================");
    }
    else
        PSendSysMessage(LANG_GMLIST_EMPTY);
    return true;
}

/// Define the 'Message of the day' for the realm
bool ChatHandler::HandleServerSetMotdCommand(const char* args)
{
    sWorld->SetMotd(args);
    PSendSysMessage(LANG_MOTD_NEW, args);
    return true;
}

bool ChatHandler::HandleServerSetConfigCommand(const char* args)
{
    ARGS_CHECK

    char* cConfigIndex = strtok((char*)args, " ");
    char* cConfigValue = strtok(NULL, " ");

    if (!cConfigIndex || !cConfigValue)
        return false;

    uint32 configIndex = (uint32)atoi(cConfigIndex);
    if(configIndex > CONFIG_VALUE_COUNT)
    {
        PSendSysMessage("Wrong index");
        return false;
    }

    uint32 configValue = (uint32)atoi(cConfigValue);
    sWorld->setConfig(configIndex,configValue);

    PSendSysMessage("Config %i set to %i",configIndex,configValue);

    return true;
}

/// Set/Unset the expansion level for an account
bool ChatHandler::HandleAccountSetAddonCommand(const char* args)
{
    ///- Get the command line arguments
    char *szAcc = strtok((char*)args," ");
    char *szExp = strtok(NULL," ");

    if(!szAcc)
        return false;

    std::string account_name;
    uint32 account_id;

    if(!szExp)
    {
        Player* player = getSelectedPlayer();
        if(!player)
            return false;

        account_id = player->GetSession()->GetAccountId();
        sAccountMgr->GetName(account_id,account_name);
        szExp = szAcc;
    }
    else
    {
        ///- Convert Account name to Upper Format
        account_name = szAcc;
        if(!AccountMgr::normalizeString(account_name))
        {
            PSendSysMessage(LANG_ACCOUNT_NOT_EXIST,account_name.c_str());
            SetSentErrorMessage(true);
            return false;
        }

        account_id = sAccountMgr->GetId(account_name);
        if(!account_id)
        {
            PSendSysMessage(LANG_ACCOUNT_NOT_EXIST,account_name.c_str());
            SetSentErrorMessage(true);
            return false;
        }
    }

    int lev=atoi(szExp);                                    //get int anyway (0 if error)
    if(lev < 0)
        return false;

    // No SQL injection
    LoginDatabase.PExecute("UPDATE account SET expansion = '%d' WHERE id = '%u'",lev,account_id);
    PSendSysMessage(LANG_ACCOUNT_SETADDON,account_name.c_str(),account_id,lev);
    return true;
}

//Send items by mail
bool ChatHandler::HandleSendItemsCommand(const char* args)
{
    ARGS_CHECK

    // format: name "subject text" "mail text" item1[:count1] item2[:count2] ... item12[:count12]

    char* pName = strtok((char*)args, " ");
    if(!pName)
        return false;

    char* tail1 = strtok(NULL, "");
    if(!tail1)
        return false;

    char* msgSubject;
    if(*tail1=='"')
        msgSubject = strtok(tail1+1, "\"");
    else
    {
        char* space = strtok(tail1, "\"");
        if(!space)
            return false;
        msgSubject = strtok(NULL, "\"");
    }

    if (!msgSubject)
        return false;

    char* tail2 = strtok(NULL, "");
    if(!tail2)
        return false;

    char* msgText;
    if(*tail2=='"')
        msgText = strtok(tail2+1, "\"");
    else
    {
        char* space = strtok(tail2, "\"");
        if(!space)
            return false;
        msgText = strtok(NULL, "\"");
    }

    if (!msgText)
        return false;

    // pName, msgSubject, msgText isn't NUL after prev. check
    std::string name    = pName;
    std::string subject = msgSubject;
    std::string text    = msgText;

    // extract items
    typedef std::pair<uint32,uint32> ItemPair;
    typedef std::list< ItemPair > ItemPairs;
    ItemPairs items;

    // get all tail string
    char* tail = strtok(NULL, "");

    // get from tail next item str
    while(char* itemStr = strtok(tail, " "))
    {
        // and get new tail
        tail = strtok(NULL, "");

        // parse item str
        char* itemIdStr = strtok(itemStr, ":");
        char* itemCountStr = strtok(NULL, " ");

        uint32 item_id = atoi(itemIdStr);
        if(!item_id)
            return false;

        ItemTemplate const* item_proto = sObjectMgr->GetItemTemplate(item_id);
        if(!item_proto)
        {
            PSendSysMessage(LANG_COMMAND_ITEMIDINVALID, item_id);
            SetSentErrorMessage(true);
            return false;
        }

        uint32 item_count = itemCountStr ? atoi(itemCountStr) : 1;
        if(item_count < 1 || item_proto->MaxCount && item_count > item_proto->MaxCount)
        {
            PSendSysMessage(LANG_COMMAND_INVALID_ITEM_COUNT, item_count,item_id);
            SetSentErrorMessage(true);
            return false;
        }

        while(item_count > item_proto->Stackable)
        {
            items.push_back(ItemPair(item_id,item_proto->Stackable));
            item_count -= item_proto->Stackable;
        }

        items.push_back(ItemPair(item_id,item_count));

        if(items.size() > MAX_MAIL_ITEMS)
        {
            PSendSysMessage(LANG_COMMAND_MAIL_ITEMS_LIMIT, MAX_MAIL_ITEMS);
            SetSentErrorMessage(true);
            return false;
        }
    }

    if(!normalizePlayerName(name))
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    uint64 receiver_guid = sObjectMgr->GetPlayerGUIDByName(name);
    if(!receiver_guid)
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    // from console show not existed sender
    uint32 sender_guidlo = m_session ? m_session->GetPlayer()->GetGUIDLow() : 0;

    uint32 messagetype = MAIL_NORMAL;
    uint32 stationery = MAIL_STATIONERY_GM;
    uint32 itemTextId = !text.empty() ? sObjectMgr->CreateItemText( text ) : 0;

    Player *receiver = sObjectMgr->GetPlayer(receiver_guid);

    // fill mail
    MailItemsInfo mi;                                       // item list preparing

    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    for(ItemPairs::const_iterator itr = items.begin(); itr != items.end(); ++itr)
    {
        if(Item* item = Item::CreateItem(itr->first,itr->second,m_session ? m_session->GetPlayer() : 0))
        {
            item->SaveToDB(trans);                               // save for prevent lost at next mail load, if send fail then item will deleted
            mi.AddItem(item->GetGUIDLow(), item->GetEntry(), item);
        }
    }
    CharacterDatabase.CommitTransaction(trans);

    WorldSession::SendMailTo(receiver,messagetype, stationery, sender_guidlo, GUID_LOPART(receiver_guid), subject, itemTextId, &mi, 0, 0, MAIL_CHECK_MASK_NONE);

    PSendSysMessage(LANG_MAIL_SENT, name.c_str());
    return true;
}

///Send money by mail
bool ChatHandler::HandleSendMoneyCommand(const char* args)
{
    ARGS_CHECK

    /// format: name "subject text" "mail text" money

    char* pName = strtok((char*)args, " ");
    if (!pName)
        return false;

    char* tail1 = strtok(NULL, "");
    if (!tail1)
        return false;

    char* msgSubject;
    if (*tail1=='"')
        msgSubject = strtok(tail1+1, "\"");
    else
    {
        char* space = strtok(tail1, "\"");
        if (!space)
            return false;
        msgSubject = strtok(NULL, "\"");
    }

    if (!msgSubject)
        return false;

    char* tail2 = strtok(NULL, "");
    if (!tail2)
        return false;

    char* msgText;
    if (*tail2=='"')
        msgText = strtok(tail2+1, "\"");
    else
    {
        char* space = strtok(tail2, "\"");
        if (!space)
            return false;
        msgText = strtok(NULL, "\"");
    }

    if (!msgText)
        return false;

    char* money_str = strtok(NULL, "");
    int32 money = money_str ? atoi(money_str) : 0;
    if (money <= 0)
        return false;

    // pName, msgSubject, msgText isn't NUL after prev. check
    std::string name    = pName;
    std::string subject = msgSubject;
    std::string text    = msgText;

    if (!normalizePlayerName(name))
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    uint64 receiver_guid = sObjectMgr->GetPlayerGUIDByName(name);
    if (!receiver_guid)
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    uint32 mailId = sObjectMgr->GenerateMailID();

    // from console show not existed sender
    uint32 sender_guidlo = m_session ? m_session->GetPlayer()->GetGUIDLow() : 0;

    uint32 messagetype = MAIL_NORMAL;
    uint32 stationery = MAIL_STATIONERY_GM;
    uint32 itemTextId = !text.empty() ? sObjectMgr->CreateItemText( text ) : 0;

    Player *receiver = sObjectMgr->GetPlayer(receiver_guid);

    WorldSession::SendMailTo(receiver,messagetype, stationery, sender_guidlo, GUID_LOPART(receiver_guid), subject, itemTextId, NULL, money, 0, MAIL_CHECK_MASK_NONE);

    PSendSysMessage(LANG_MAIL_SENT, name.c_str());
    return true;
}

/// Send a message to a player in game
bool ChatHandler::HandleSendMessageCommand(const char* args)
{
    ///- Get the command line arguments
    char* name_str = strtok((char*)args, " ");
    char* msg_str = strtok(NULL, "");

    if(!name_str || !msg_str)
        return false;

    std::string name = name_str;

    if(!normalizePlayerName(name))
        return false;

    ///- Find the player and check that he is not logging out.
    Player *rPlayer = sObjectAccessor->FindConnectedPlayerByName(name.c_str());
    if(!rPlayer)
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    if(rPlayer->GetSession()->isLogingOut())
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    ///- Send the message
    //Use SendAreaTriggerMessage for fastest delivery.
    rPlayer->GetSession()->SendAreaTriggerMessage("%s", msg_str);
    rPlayer->GetSession()->SendAreaTriggerMessage("|cffff0000[Message from administrator]:|r");

    //Confirmation message
    PSendSysMessage(LANG_SENDMESSAGE,name.c_str(),msg_str);
    return true;
}

bool ChatHandler::HandleFlushArenaPointsCommand(const char * /*args*/)
{
    sBattlegroundMgr->DistributeArenaPoints();
    return true;
}

bool ChatHandler::HandleModifyGenderCommand(const char *args)
{
    ARGS_CHECK

    Player *player = getSelectedPlayer();

    if(!player)
    {
        PSendSysMessage(LANG_NO_PLAYER);
        SetSentErrorMessage(true);
        return false;
    }

    char const* gender_str = (char*)args;
    int gender_len = strlen(gender_str);

    uint32 displayId = player->GetNativeDisplayId();
    char const* gender_full = NULL;
    uint32 new_displayId = displayId;
    Gender gender;

    if(!strncmp(gender_str,"male",gender_len))              // MALE
    {
        if(player->GetGender() == GENDER_MALE)
            return true;

        gender_full = "male";
        new_displayId = player->GetRace() == RACE_BLOODELF ? displayId+1 : displayId-1;
        gender = GENDER_MALE;
    }
    else if (!strncmp(gender_str,"female",gender_len))      // FEMALE
    {
        if(player->GetGender() == GENDER_FEMALE)
            return true;

        gender_full = "female";
        new_displayId = player->GetRace() == RACE_BLOODELF ? displayId-1 : displayId+1;
        gender = GENDER_FEMALE;
    }
    else
    {
        SendSysMessage(LANG_MUST_MALE_OR_FEMALE);
        SetSentErrorMessage(true);
        return false;
    }

    // Set gender
    player->SetByteValue(UNIT_FIELD_BYTES_0, 2, gender);
    player->SetByteValue(PLAYER_BYTES_3, 0, gender);

    // Change display ID
    player->SetDisplayId(new_displayId);
    player->SetNativeDisplayId(new_displayId);

    PSendSysMessage(LANG_YOU_CHANGE_GENDER, player->GetName().c_str(),gender_full);
    if (needReportToTarget(player))
        ChatHandler(player).PSendSysMessage(LANG_YOUR_GENDER_CHANGED, gender_full,GetName().c_str());
    return true;
}

/*------------------------------------------
 *-------------TRINITY----------------------
 *-------------------------------------*/

bool ChatHandler::HandlePlayAllCommand(const char* args)
{
    
    ARGS_CHECK

    uint32 soundId = atoi((char*)args);

    if(!sSoundEntriesStore.LookupEntry(soundId))
    {
        PSendSysMessage(LANG_SOUND_NOT_EXIST, soundId);
        SetSentErrorMessage(true);
        return false;
    }

    WorldPacket data(SMSG_PLAY_SOUND, 4);
    data << uint32(soundId) << m_session->GetPlayer()->GetGUID();
    sWorld->SendGlobalMessage(&data);

    PSendSysMessage(LANG_COMMAND_PLAYED_TO_ALL, soundId);
    return true;
}

bool ChatHandler::HandleFreezeCommand(const char *args)
{
    

    std::string name;
    Player* player;
    char* TargetName = strtok((char*)args, " "); //get entered name
    if (!TargetName) //if no name entered use target
    {
        player = getSelectedPlayer();
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
        player = sObjectAccessor->FindConnectedPlayerByName(name.c_str()); //get player by name
    }

    if (!player)
    {
        SendSysMessage(LANG_COMMAND_FREEZE_WRONG);
        return true;
    }

    if (player==m_session->GetPlayer())
    {
        SendSysMessage(LANG_COMMAND_FREEZE_ERROR);
        return true;
    }

    //effect
    if ((player) && (!(player==m_session->GetPlayer())))
    {
        PSendSysMessage(LANG_COMMAND_FREEZE,name.c_str());

        //stop combat + make player unattackable + duel stop + stop some spells
        player->SetFaction(35);
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
                    player->RemovePet(pet,PET_SAVE_NOT_IN_SLOT);
            }
        }

        //stop movement and disable spells
        uint32 spellID = 9454;
        //m_session->GetPlayer()->CastSpell(player,spellID,false);
        SpellInfo const *spellInfo = sSpellMgr->GetSpellInfo( spellID );
        if(spellInfo) //TODO: Change the duration of the aura to -1 instead of 5000000
        {
            for(uint32 i = 0;i<3;i++)
            {
                uint8 eff = spellInfo->Effects[i].Effect;
                if (eff>=TOTAL_SPELL_EFFECTS)
                    continue;
                if( eff == SPELL_EFFECT_APPLY_AREA_AURA_PARTY || eff == SPELL_EFFECT_APPLY_AURA ||
                    eff == SPELL_EFFECT_PERSISTENT_AREA_AURA || eff == SPELL_EFFECT_APPLY_AREA_AURA_FRIEND ||
                    eff == SPELL_EFFECT_APPLY_AREA_AURA_ENEMY)
                {
                    Aura *Aur = CreateAura(spellInfo, i, NULL, player);
                    player->AddAura(Aur);
                }
            }
        }

        //save player
        player->SaveToDB();
    }
    return true;
}

bool ChatHandler::HandleUnFreezeCommand(const char *args)
{
    std::string name;
    Player* player;
    char* TargetName = strtok((char*)args, " "); //get entered name
    if (!TargetName) //if no name entered use target
    {
        player = getSelectedPlayer();
        if (player) //prevent crash with creature as target
        {
            name = player->GetName();
        }
    }

    else // if name entered
    {
        name = TargetName;
        normalizePlayerName(name);
        player = sObjectAccessor->FindConnectedPlayerByName(name.c_str()); //get player by name
    }

    //effect
    if (player)
    {
        PSendSysMessage(LANG_COMMAND_UNFREEZE,name.c_str());

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
                SendSysMessage(LANG_COMMAND_FREEZE_WRONG);
                return true;
            }
            //if player found: delete his freeze aura
            Field *fields=result->Fetch();
            uint64 pguid = fields[0].GetUInt64();
            CharacterDatabase.PQuery("DELETE FROM `character_aura` WHERE character_aura.spell = 9454 AND character_aura.guid = '%u'",pguid);
            PSendSysMessage(LANG_COMMAND_UNFREEZE,name.c_str());
            return true;
        }
        else
        {
            SendSysMessage(LANG_COMMAND_FREEZE_WRONG);
            return true;
        }
    }

    return true;
}

bool ChatHandler::HandleListFreezeCommand(const char* args)
{
    //Get names from DB
    QueryResult result = CharacterDatabase.PQuery("SELECT characters.name FROM `characters` LEFT JOIN `character_aura` ON (characters.guid = character_aura.guid) WHERE character_aura.spell = 9454");
    if(!result)
    {
        SendSysMessage(LANG_COMMAND_NO_FROZEN_PLAYERS);
        return true;
    }
    //Header of the names
    PSendSysMessage(LANG_COMMAND_LIST_FREEZE);

    //Output of the results
    do
    {
        Field *fields = result->Fetch();
        std::string fplayers = fields[0].GetString();
        PSendSysMessage(LANG_COMMAND_FROZEN_PLAYERS,fplayers.c_str());
    } while (result->NextRow());

    return true;
}

bool ChatHandler::HandleGroupLeaderCommand(const char* args)
{
    Player* plr  = NULL;
    Group* group = NULL;
    uint64 guid  = 0;
    char* cname  = strtok((char*)args, " ");

    if(GetPlayerGroupAndGUIDByName(cname, plr, group, guid))
        if(group && group->GetLeaderGUID() != guid)
            group->ChangeLeader(guid);

    return true;
}

bool ChatHandler::HandleGroupDisbandCommand(const char* args)
{
    Player* plr  = NULL;
    Group* group = NULL;
    uint64 guid  = 0;
    char* cname  = strtok((char*)args, " ");

    if(GetPlayerGroupAndGUIDByName(cname, plr, group, guid))
        if(group)
            group->Disband();

    return true;
}

bool ChatHandler::HandleGroupRemoveCommand(const char* args)
{
    Player* plr  = NULL;
    Group* group = NULL;
    uint64 guid  = 0;
    char* cname  = strtok((char*)args, " ");

    if(GetPlayerGroupAndGUIDByName(cname, plr, group, guid, true))
        if(group)
            group->RemoveMember(guid, 0);

    return true;
}

bool ChatHandler::HandlePossessCommand(const char* args)
{
    

    Unit* pUnit = getSelectedUnit();
    if(!pUnit)
        return false;

    m_session->GetPlayer()->CastSpell(pUnit, 530, true);
    return true;
}

bool ChatHandler::HandleUnPossessCommand(const char* args)
{
    

    Unit* pUnit = getSelectedUnit();
    if(!pUnit) pUnit = m_session->GetPlayer();

    pUnit->RemoveAurasByType(SPELL_AURA_MOD_CHARM);
    pUnit->RemoveAurasByType(SPELL_AURA_MOD_POSSESS_PET);
    pUnit->RemoveAurasByType(SPELL_AURA_MOD_POSSESS);

    return true;
}

bool ChatHandler::HandleBindSightCommand(const char* args)
{
    
    
    Unit* pUnit = getSelectedUnit();
    if (!pUnit)
        return false;

    m_session->GetPlayer()->CastSpell(pUnit, 6277, true);
    return true;
}

bool ChatHandler::HandleUnbindSightCommand(const char* args)
{
    

    if (m_session->GetPlayer()->IsPossessing())
        return false;

    m_session->GetPlayer()->StopCastingBindSight();
    return true;
}
bool ChatHandler::HandleZoneBuffCommand(const char* args)
{
    ARGS_CHECK

    char *bufid = strtok((char *)args, " ");
    if (!bufid)
        return false;

    boost::shared_lock<boost::shared_mutex> lock(*HashMapHolder<Player>::GetLock());
    HashMapHolder<Player>::MapType const& players = ObjectAccessor::GetPlayers();
    Player *p;

    for (HashMapHolder<Player>::MapType::const_iterator it = players.begin(); it != players.end(); it++) {
        p = it->second;
        if (p && p->IsInWorld() && p->GetZoneId() == m_session->GetPlayer()->GetZoneId())
            p->CastSpell(p, atoi(bufid), true);
    }

    return true;
}

bool ChatHandler::HandleZoneMorphCommand(const char* args)
{
    ARGS_CHECK

    char *displid = strtok((char *)args, " ");
    if (!displid)
        return false;
    char *factid = strtok(NULL, " ");

    uint16 display_id = (uint16)atoi((char *)args);
    uint8 faction_id = factid ? (uint8)atoi(factid) : 0;

    boost::shared_lock<boost::shared_mutex> lock(*HashMapHolder<Player>::GetLock());
    HashMapHolder<Player>::MapType const& players = ObjectAccessor::GetPlayers();
    Player *p;

    for (HashMapHolder<Player>::MapType::const_iterator it = players.begin(); it != players.end(); it++) {
        p = it->second;
        if (p && p->IsInWorld() && p->GetZoneId() == m_session->GetPlayer()->GetZoneId() &&
            ((faction_id == 1 && p->GetTeam() == TEAM_ALLIANCE) || (faction_id == 2 && p->GetTeam() == TEAM_HORDE) || faction_id == 0))
            p->SetDisplayId(display_id);
    }

    return true;
}

bool ChatHandler::HandleNpcMassFactionIdCommand(const char* args)
{
    

    char *entryid = strtok((char *)args, " ");
    if (!entryid)
        return false;

    char *factid = strtok(NULL, " ");
    if (!factid)
        return false;

    Player *player = m_session->GetPlayer();

    uint32 factionId = (uint32)atoi(factid);
    uint32 entryId = (uint32)atoi(entryid);

    if (!sFactionTemplateStore.LookupEntry(factionId)) {
        PSendSysMessage(LANG_WRONG_FACTION, factionId);
        SetSentErrorMessage(true);
        return false;
    }

    CellCoord p(Trinity::ComputeCellCoord(player->GetPositionX(), player->GetPositionY()));
    Cell cell(p);
    cell.data.Part.reserved = ALL_DISTRICT;

    Trinity::FactionDo u_do(entryId, factionId);
    Trinity::WorldObjectWorker<Trinity::FactionDo> worker(u_do);

    TypeContainerVisitor<Trinity::WorldObjectWorker<Trinity::FactionDo>, GridTypeMapContainer > obj_worker(worker);
    cell.Visit(p, obj_worker, *player->GetMap());

    return true;
}

bool ChatHandler::HandleNpcGoBackHomeCommand(const char* args)
{
    

    if (!*args) {      // Command is applied on selected unit
        Unit* pUnit = getSelectedUnit();
        if (!pUnit || pUnit == m_session->GetPlayer())
            return false;
        
        float x, y, z, o;
        (pUnit->ToCreature())->GetHomePosition(x, y, z, o);
        pUnit->GetMotionMaster()->MovePoint(0, x, y, z);
        return true;
    }
    else {                      // On specified GUID
        char* guid = strtok((char *)args, " ");
        Player* plr = m_session->GetPlayer();
        if (!guid || !plr)
            return false;
        uint64 uintGUID = (uint64)atoll(guid);
        QueryResult result = WorldDatabase.PQuery("SELECT id FROM creature WHERE guid = %u LIMIT 1", uintGUID);
        if (result) {
            Field *fields = result->Fetch();
            uint32 creatureentry = fields[0].GetUInt32();
            uint64 packedguid = MAKE_NEW_GUID(uintGUID, creatureentry, HIGHGUID_UNIT);
            Unit* pUnit = Unit::GetUnit(*plr, packedguid);
            if (!pUnit) {
                PSendSysMessage("No unit found.");
                return true;
            }
            float x, y, z, o;
            (pUnit->ToCreature())->GetHomePosition(x, y, z, o);
            (pUnit->ToCreature())->GetMotionMaster()->MovePoint(0, x, y, z);
            return true;
        }
        PSendSysMessage("No unit found.");
    }
    
    return false;
}

bool ChatHandler::HandleGoATCommand(const char* args)
{
    
    ARGS_CHECK
        
    Player* plr = m_session->GetPlayer();
    if (!plr)
        return false;
        
    char* atIdChar = strtok((char*)args, " ");
    int atId = atoi(atIdChar);
    if (!atId)
        return false;
        
    AreaTriggerEntry const* at = sAreaTriggerStore.LookupEntry(atId);
    if (!at)
        return false;
        
    // Teleport player on at coords
    plr->TeleportTo(at->mapid, at->x, at->y, at->z, plr->GetOrientation());
    return true;
}

bool ChatHandler::HandleNpcSetPoolCommand(const char* args)
{
    ARGS_CHECK
    
    char *chrPoolId = strtok((char *)args, " ");
    if (!chrPoolId)
        return false;
        
    uint32 poolId = (uint32)atoi(chrPoolId);
    if (!poolId)
        return false;
        
    Creature *creature = getSelectedCreature();
    if (!creature)
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        return true;
    }
        
    WorldDatabase.PExecute("UPDATE creature SET pool_id = %u WHERE guid = %u", poolId, creature->GetDBTableGUIDLow());
    creature->SetCreaturePoolId(poolId);
    creature->FindMap()->AddCreatureToPool(creature, poolId);
    PSendSysMessage("Creature (guid: %u) added to pool %u",creature->GetDBTableGUIDLow(),poolId);
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
    HashMapHolder<Player>::MapType& m = ObjectAccessor::GetPlayers();
    for(HashMapHolder<Player>::MapType::iterator itr = m.begin(); itr != m.end(); ++itr)
    {
        if (itr->second && itr->second->GetSession()->GetPlayer() && itr->second->GetSession()->GetPlayer()->IsInWorld())
        {
            if(ChannelMgr* cMgr = channelMgr(itr->second->GetSession()->GetPlayer()->GetTeam()))
            {
                if(Channel *chn = cMgr->GetChannel(channel, itr->second->GetSession()->GetPlayer()))
                {
                    WorldPacket data;
                    ChatHandler::BuildChatPacket(data, CHAT_MSG_CHANNEL, LANG_UNIVERSAL, itr->second->GetSession()->GetPlayer(),itr->second->GetSession()->GetPlayer(), msg, 0, channel);
                    itr->second->GetSession()->SendPacket(&data);
                }
            }
        }
    }
    
    return false;
}

bool ChatHandler::HandleDebugAurasList(const char* args)
{
    

    Unit* unit = getSelectedUnit();
    if (!unit)
        unit = m_session->GetPlayer();
    
    PSendSysMessage("Aura list:");
    Unit::AuraMap& tAuras = unit->GetAuras();
    for (Unit::AuraMap::iterator itr = tAuras.begin(); itr != tAuras.end(); itr++)
    {
        SpellInfo const* spellProto = (*itr).second->GetSpellInfo();
        PSendSysMessage("%u - %s (stack: %u) - Effect %u - Value %u %s", spellProto->Id, spellProto->SpellName[sWorld->GetDefaultDbcLocale()], (*itr).second->GetStackAmount(), (*itr).second->GetEffIndex(), (*itr).second->GetModifierValue(), (*itr).second->IsActive() ? "" : "[inactive]");
    }
    
    return true;
}

bool ChatHandler::HandleGetMoveFlagsCommand(const char* args)
{
    

    Unit* target = getSelectedUnit();
    if (!target)
        target = m_session->GetPlayer();

    PSendSysMessage("Target (%u) moveflags = %u",target->GetGUIDLow(),target->GetUnitMovementFlags());

    return true;
}

bool ChatHandler::HandleSetMoveFlagsCommand(const char* args)
{
    ARGS_CHECK

    Unit* target = getSelectedUnit();
    if (!target)
        target = m_session->GetPlayer();

    if(strcmp(args,"") == 0)
        return false;

    uint32 moveFlags;
    std::stringstream ss(args);
    ss >> moveFlags;

    target->SetUnitMovementFlags(moveFlags);

    PSendSysMessage("Target (%u) moveflags set to %u",target->GetGUIDLow(),moveFlags);

    return true;
}

bool ChatHandler::HandleInstanceSetDataCommand(const char* args)
{
    ARGS_CHECK
        
    char *chrDataId = strtok((char *)args, " ");
    if (!chrDataId)
        return false;
        
    uint32 dataId = uint32(atoi(chrDataId));
    
    char *chrDataValue = strtok(NULL, " ");
    if (!chrDataValue)
        return false;
        
    uint32 dataValue = uint32(atoi(chrDataValue));
    
    Player *plr = m_session->GetPlayer();
    
    if (ScriptedInstance *pInstance = ((ScriptedInstance*)plr->GetInstanceData()))
        pInstance->SetData(dataId, dataValue);
    else {
        PSendSysMessage("You are not in an instance.");
        return false;
    }
    
    return true;
}

bool ChatHandler::HandleInstanceGetDataCommand(const char* args)
{
    ARGS_CHECK
        
    char *chrDataId = strtok((char *)args, " ");
    if (!chrDataId)
        return false;
        
    uint32 dataId = uint32(atoi(chrDataId));
    
    Player *plr = m_session->GetPlayer();
    
    if (ScriptedInstance *pInstance = ((ScriptedInstance*)plr->GetInstanceData()))
        PSendSysMessage("Instance data %u = %u.", dataId, pInstance->GetData(dataId));
    else {
        PSendSysMessage("You are not in an instance.");
        return false;
    }
    
    return true;
}

bool ChatHandler::HandleGetMaxCreaturePoolIdCommand(const char* args)
{
    QueryResult result = WorldDatabase.PQuery("SELECT MAX(pool_id) FROM creature");
    Field *fields = result->Fetch();
    
    uint32 maxId = fields[0].GetUInt32();
    
    PSendSysMessage("Current max creature pool id: %u", maxId);
    
    return true;
}

bool ChatHandler::HandleSetTitleCommand(const char* args)
{
    
    ARGS_CHECK

    uint32 titleId = atoi(args);

    if(CharTitlesEntry const* titleEntry = sCharTitlesStore.LookupEntry(titleId)) {
        if (Player* plr = getSelectedUnit()->ToPlayer())
            plr->SetTitle(titleEntry,true);
        else if (Player* plr = m_session->GetPlayer())
            plr->SetTitle(titleEntry,true);
    }

    return true;
}

bool ChatHandler::HandleRemoveTitleCommand(const char* args)
{
    
    ARGS_CHECK

    uint32 titleId = atoi(args);

    if(CharTitlesEntry const* titleEntry = sCharTitlesStore.LookupEntry(titleId)) {
        if (Player* plr = m_session->GetPlayer())
            if (plr->HasTitle(titleEntry))
                plr->RemoveTitle(titleEntry);
    }

    return true;
}

bool ChatHandler::HandleGMStats(const char* args)
{
    

    uint32 accId = m_session->GetAccountId();
    
    QueryResult timeResult = CharacterDatabase.Query("SELECT GmWeekBeginTime FROM saved_variables");
    if (!timeResult) {
        PSendSysMessage("Error with GmWeekBeginTime.");
        return true;
    }
    
    Field* timeFields = timeResult->Fetch();
    uint64 beginTime = timeFields[0].GetUInt64();

    QueryResult countResult = CharacterDatabase.PQuery("SELECT COUNT(*) FROM gm_tickets WHERE timestamp > " UI64FMTD " AND closed = %u", beginTime, accId);
    if (!countResult) {
        PSendSysMessage("No information found for this account.");
        return true;
    }
    
    Field* countFields = countResult->Fetch();
    uint32 count = countFields[0].GetUInt32();
    
    //PSendSysMessage("Vous avez fermé %u tickets depuis le début de la semaine.", count);
    PSendSysMessage("You closed %u tickets since the beginning of the week.", count);
    
    return true;
}

bool ChatHandler::HandleMmap(const char* args)
{
    bool on;
    if (args)
    {
        std::string argstr = (char*)args;
        if (argstr == "on")
        {
            sWorld->setConfig(CONFIG_BOOL_MMAP_ENABLED, true);
            SendSysMessage("WORLD: mmaps are now ENABLED (individual map settings still in effect)");
        }
        else if (argstr == "off")
        {
            sWorld->setConfig(CONFIG_BOOL_MMAP_ENABLED, false);
            SendSysMessage("WORLD: mmaps are now DISABLED");
        }
        else
        {
            on = sWorld->getConfig(CONFIG_BOOL_MMAP_ENABLED);
            PSendSysMessage("mmaps are %sabled", on ? "en" : "dis");
        }
        return true;
    }

    return true;
}

bool ChatHandler::HandleMmapTestArea(const char* args)
{
    

#ifdef OLDMOV
    float radius = 40.0f;
    //ExtractFloat(&args, radius);

    CellCoord pair(Trinity::ComputeCellCoord( m_session->GetPlayer()->GetPositionX(), m_session->GetPlayer()->GetPositionY()) );
    Cell cell(pair);
    cell.SetNoCreate();

    std::list<Creature*> creatureList;

    Trinity::AnyUnitInObjectRangeCheck go_check(m_session->GetPlayer(), radius);
    Trinity::CreatureListSearcher<Trinity::AnyUnitInObjectRangeCheck> go_search(creatureList, go_check);
    TypeContainerVisitor<Trinity::CreatureListSearcher<Trinity::AnyUnitInObjectRangeCheck>, GridTypeMapContainer> go_visit(go_search);

    // Get Creatures
    cell.Visit(pair, go_visit, *(m_session->GetPlayer()->GetMap()), *(m_session->GetPlayer()), radius);

    if (!creatureList.empty())
    {
        PSendSysMessage("Found %i Creatures.", creatureList.size());

        uint32 paths = 0;
        uint32 uStartTime = GetMSTime();

        float gx,gy,gz;
        m_session->GetPlayer()->GetPosition(gx,gy,gz);
        for (std::list<Creature*>::iterator itr = creatureList.begin(); itr != creatureList.end(); ++itr)
        {
            PathInfo((*itr), gx, gy, gz);
            ++paths;
        }

        uint32 uPathLoadTime = GetMSTimeDiff(uStartTime, GetMSTime());
        PSendSysMessage("Generated %i paths in %i ms", paths, uPathLoadTime);
    }
    else
    {
        PSendSysMessage("No creatures in %f yard range.", radius);
    }
    #endif
    return true;
}

bool ChatHandler::HandleReloadConditions(const char* args)
{
    TC_LOG_INFO("command","Re-Loading Conditions...");
    sConditionMgr.LoadConditions(true);
    SendGlobalGMSysMessage("Conditions reloaded.");
    return true;
}

bool ChatHandler::HandleReloadSmartAI(const char* /*args*/)
{
    TC_LOG_INFO("command","Re-Loading SmartAI Scripts...");
    sSmartScriptMgr->LoadSmartAIFromDB();
    SendGlobalGMSysMessage("SmartAI Scripts reloaded.");
    return true;
}

bool ChatHandler::HandleDebugUnloadGrid(const char* args)
{
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
    return true;
}

bool ChatHandler::HandleNpcSetInstanceEventCommand(const char* args)
{
    ARGS_CHECK
    
    Creature* target = getSelectedCreature();
    if (!target || (target && target->GetTypeId() != TYPEID_UNIT)) {
        PSendSysMessage("Vous devez sélectionner une créature.");
        return true;
    }
    
    char* eventIdStr = strtok((char*)args, " ");
    if (!eventIdStr)
        return false;
        
    int eventId = atoi(eventIdStr);
    
    if (eventId == -1) {
        WorldDatabase.PExecute("DELETE FROM creature_encounter_respawn WHERE guid = %u", target->GetDBTableGUIDLow());
        return true;
    }
    
    WorldDatabase.PExecute("REPLACE INTO creature_encounter_respawn VALUES (%u, %u)", target->GetDBTableGUIDLow(), eventId);
    PSendSysMessage("Creature (%u) respawn linked to event %u.",target->GetDBTableGUIDLow(),eventId);
    
    return true;
}

bool ChatHandler::HandleReloadSpellTemplates(const char* args)
{
    TC_LOG_INFO("command","Re-loading spell templates...");
    sObjectMgr->LoadSpellTemplates();
    sSpellMgr->LoadSpellCustomAttr(); //re apply custom attr
    SendGlobalGMSysMessage("DB table `spell_template` (spell definitions) reloaded.");
    return true;
}

bool ChatHandler::HandleGuildRenameCommand(const char* args)
{
    ARGS_CHECK
        
    char* guildIdStr = strtok((char*)args, " ");
    if (!guildIdStr)
        return false;
        
    char* newName = strtok(NULL, "");
    if (!newName)
        return false;
    
    uint32 guildId = atoi(guildIdStr);
    CharacterDatabase.PExecute("UPDATE guild SET name = '%s' WHERE guildid = %u", newName, guildId);
    
    PSendSysMessage("Guilde renommée !");
    
    return true;
}

bool ChatHandler::HandleEnableEventCommand(const char* args)
{
    
    ARGS_CHECK

    char* eventIdStr = strtok((char*)args, " ");
    if (!eventIdStr)
        return false;
        
    int eventId = atoi(eventIdStr);

    Unit* pUnit = getSelectedUnit();
    if (!pUnit || pUnit == m_session->GetPlayer())
        return false;

    if (pUnit->ToCreature())
        if (pUnit->ToCreature()->getAI())
            pUnit->ToCreature()->getAI()->enableEvent(eventId);

    return true;
}

bool ChatHandler::HandleDisableEventCommand(const char* args)
{
    
    ARGS_CHECK

    char* eventIdStr = strtok((char*)args, " ");
    if (!eventIdStr)
        return false;
        
    int eventId = atoi(eventIdStr);

    Unit* pUnit = getSelectedUnit();
    if (!pUnit || pUnit == m_session->GetPlayer())
        return false;

    if (pUnit->ToCreature())
        if (pUnit->ToCreature()->getAI())
            pUnit->ToCreature()->getAI()->disableEvent(eventId);

    return true;
}

bool ChatHandler::HandleScheduleEventCommand(const char* args)
{
    
    ARGS_CHECK

    char* eventIdStr = strtok((char*)args, " ");
    char* timerStr = strtok(NULL, " ");

    if (!eventIdStr || !timerStr)
        return false;

    int eventId = atoi(eventIdStr);
    int timer = atoi(timerStr);

    Unit* pUnit = getSelectedUnit();
    if (!pUnit || pUnit == m_session->GetPlayer())
        return false;

    if (pUnit->ToCreature())
        if (pUnit->ToCreature()->getAI())
            pUnit->ToCreature()->getAI()->scheduleEvent(eventId, timer);

    return true;
}

bool ChatHandler::HandleNpcSetCombatDistanceCommand(const char* args)
{
    ARGS_CHECK

    char* cDistance = strtok((char*)args, " ");
    if (!cDistance)
        return false;
        
    float distance = (float)atof(cDistance);

    Creature *pCreature = getSelectedCreature();
    if(!pCreature)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        return true;
    }

    if(pCreature->AI())
    {
        pCreature->AI()->SetCombatDistance(distance);
        PSendSysMessage("m_combatDistance set to %f", distance);
    }

    return true;
}

bool ChatHandler::HandleNpcAllowCombatMovementCommand(const char* args)
{
    ARGS_CHECK

    char* cAllow = strtok((char*)args, " ");
    if (!cAllow)
        return false;
        
    int allow = atoi(cAllow);

    Creature *pCreature = getSelectedCreature();
    if(!pCreature)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        return true;
    }

    if(pCreature->AI())
    {
        pCreature->AI()->SetCombatMovementAllowed(allow);
        PSendSysMessage("m_allowCombatMovement set to %s", allow ? "true" : "false");
    }

    return true;
}

/* if no args given, tell if the selected creature is linked to a game_event. Else usage is .npc linkgameevent #eventid [#guid] (a guid may be given, overiding the selected creature)*/
bool ChatHandler::HandleNpcLinkGameEventCommand(const char* args)
{
    CreatureData const* data = NULL;
    char* cEvent = strtok((char*)args, " ");
    char* cCreatureGUID = strtok(NULL, " ");
    int16 event = 0;
    uint32 creatureGUID = 0;
    bool justShowInfo = false;
    if(!cEvent) // No params given
    {
        justShowInfo = true;
    } else {
        event = atoi(cEvent);
        if(cCreatureGUID) // erase selected creature if guid explicitely given
            creatureGUID = atoi(cCreatureGUID);
    }

    if(!creatureGUID)
    {
        Creature* creature = getSelectedCreature();
        if(creature)
            creatureGUID = creature->GetDBTableGUIDLow();
    }

    data = sObjectMgr->GetCreatureData(creatureGUID);
    if(!data)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        return true;
    }

    int16 currentEventId = gameeventmgr.GetCreatureEvent(creatureGUID);

    if (justShowInfo)
    {
        if(currentEventId)
            //PSendSysMessage("La creature (guid : %u) est liée à l'event %i.",creatureGUID,currentEventId);
            PSendSysMessage("Creature (guid: %u) bound to event %i.",creatureGUID,currentEventId);
        else
            //PSendSysMessage("La creature (guid : %u) n'est liée à aucun event.",creatureGUID);
            PSendSysMessage("Creature (guid : %u) is not bound to an event.",creatureGUID);
    } else {
        if(currentEventId)
        {
           // PSendSysMessage("La creature (guid : %u) est déjà liée à l'event %i.",creatureGUID,currentEventId);
            //PSendSysMessage("La creature est déjà liée à l'event %i.",currentEventId);
            PSendSysMessage("Creature bound to event %i.",currentEventId);
            return true;
        }

        if(gameeventmgr.AddCreatureToEvent(creatureGUID, event))
            //PSendSysMessage("La creature (guid : %u) a été liée à l'event %i.",creatureGUID,event);
            PSendSysMessage("Creature (guid: %u) is now bound to the event %i.",creatureGUID,event);
        else
            //PSendSysMessage("Erreur : La creature (guid : %u) n'a pas pu être liée à l'event %d (event inexistant ?).",creatureGUID,event);
            PSendSysMessage("Error: creature (guid: %u) could not be linked to the event %d (event nonexistent?).",creatureGUID,event);
    }

    return true;
}

/* .npc unlinkgameevent [#guid] */
bool ChatHandler::HandleNpcUnlinkGameEventCommand(const char* args)
{
    Creature* creature = NULL;
    CreatureData const* data = NULL;
    char* cCreatureGUID = strtok((char*)args, " ");
    uint32 creatureGUID = 0;

    if(cCreatureGUID) //Guid given
    {
        creatureGUID = atoi(cCreatureGUID);
    } else { //else, try to get selected creature
        creature = getSelectedCreature();
        if(!creature)
        {
            SendSysMessage(LANG_SELECT_CREATURE);
            return true;
        }           
        creatureGUID = creature->GetGUIDLow();
    }

    data = sObjectMgr->GetCreatureData(creatureGUID);
    if(!data)
    {
        //PSendSysMessage("Creature avec le guid %u introuvable.",creatureGUID);
        PSendSysMessage("Creature with guid %u not found.",creatureGUID);
        return true;
    } 

    int16 currentEventId = gameeventmgr.GetCreatureEvent(creatureGUID);

    if (!currentEventId)
    {
        //PSendSysMessage("La creature (guid : %u) n'est liée à aucun event.",creatureGUID);
        PSendSysMessage("Creature (guid: %u) is not linked to any event.",creatureGUID);
    } else {
        if(gameeventmgr.RemoveCreatureFromEvent(creatureGUID))
            //PSendSysMessage("La creature (guid : %u) n'est plus liée à l'event %i.",creatureGUID,currentEventId);
            PSendSysMessage("Creature (guid: %u) is not anymore linked to the event %i.",creatureGUID,currentEventId);
        else
            //PSendSysMessage("Erreur lors de la suppression de la créature (guid : %u) de l'event %i.",creatureGUID,currentEventId);
            PSendSysMessage("Error on removing creature (guid: %u) from the event %i.",creatureGUID,currentEventId);
    }

    return true;
}

/* .gobject linkgameevent #event #guid */
bool ChatHandler::HandleGobLinkGameEventCommand(const char* args)
{
    GameObjectData const* data = NULL;
    char* cEvent = strtok((char*)args, " ");
    char* cGobGUID = strtok(NULL, " ");
    int16 event = 0;
    uint32 gobGUID = 0;

    if(!cEvent || !cGobGUID) 
       return false;

    event = atoi(cEvent);
    gobGUID = atoi(cGobGUID);

    if(!event || !gobGUID)
    {
        //PSendSysMessage("Valeurs incorrectes.");
        PSendSysMessage("Incorrect values.");
        return true;
    }

    data = sObjectMgr->GetGOData(gobGUID);
    if(!data)
    {
        //PSendSysMessage("Gobject (guid : %u) introuvable.",gobGUID);
        PSendSysMessage("Gobject (guid: %u) not found.",gobGUID);
        return true;
    }

    int16 currentEventId = gameeventmgr.GetGameObjectEvent(gobGUID);
    if(currentEventId)
    {
        //PSendSysMessage("Le gobject est déjà lié à l'event %i.",currentEventId);
        PSendSysMessage("Gobject already linked to the event %i.",currentEventId);
        return true;
    }

    if(gameeventmgr.AddGameObjectToEvent(gobGUID, event))
        //PSendSysMessage("Le gobject (guid : %u) a été lié à l'event %i.",gobGUID,event);
        PSendSysMessage("Gobject (guid: %u) is now linked to the event %i.",gobGUID,event);
    else
        //PSendSysMessage("Erreur : Le gobject (guid : %u) n'a pas pu être lié à l'event %d (event inexistant ?).",gobGUID,event);
        PSendSysMessage("Error: gobject (guid: %u) could not be linked to the event %d (event nonexistent).",gobGUID,event);

    return true;
}

/*.gobject unlinkgameevent #guid*/
bool ChatHandler::HandleGobUnlinkGameEventCommand(const char* args)
{
    GameObjectData const* data = NULL;
    char* cGobGUID = strtok((char*)args, " ");
    uint32 gobGUID = 0;

    if(!cGobGUID)
        return false;

    gobGUID = atoi(cGobGUID);

    data = sObjectMgr->GetGOData(gobGUID);
    if(!data)
    {
        //PSendSysMessage("Gobject avec le guid %u introuvable.",gobGUID);
        PSendSysMessage("Gobject with guid %u not found.",gobGUID);
        return true;
    } 

    int16 currentEventId = gameeventmgr.GetGameObjectEvent(gobGUID);
    if (!currentEventId)
    {
        //PSendSysMessage("Le gobject (guid : %u) n'est lié à aucun event.",gobGUID);
        PSendSysMessage("Gobject (guid: %u) is not linked to any event.",gobGUID);
    } else {
        if(gameeventmgr.RemoveGameObjectFromEvent(gobGUID))
            //PSendSysMessage("Le gobject (guid : %u) n'est plus lié à l'event %i.",gobGUID,currentEventId);
            PSendSysMessage("Gobject (guid: %u) is not linked anymore to the event %i.",gobGUID,currentEventId);
        else
            //PSendSysMessage("Erreur lors de la suppression du gobject (guid : %u) de l'event %i.",gobGUID,currentEventId);
            PSendSysMessage("Error on removing gobject (guid: %u) from the event %i.",gobGUID,currentEventId);
    }

    return true;
}

/* event create #id $name */
bool ChatHandler::HandleEventCreateCommand(const char* args)
{
    /*
    ARGS_CHECK

    if(strcmp(args,"") == 0)
        return false;

    int16 createdEventId = 0;
    bool success = gameeventmgr.CreateGameEvent(args,createdEventId);
    if(success)
        PSendSysMessage("L'event \"%s\" (id: %i) a été créé.",args,createdEventId);
    else
        PSendSysMessage("Erreur : L'event \"%s\" (id: %i) n'a pas pu être créé.",args,createdEventId);
 */
    return true;
}

/* Syntax : .path direction <pathid> [dir] 
Note that this doesn't update creatures already using this path.

Possible directions :
0 - WP_PATH_DIRECTION_NORMAL
1 - WP_PATH_DIRECTION_REVERSE
2 - WP_PATH_DIRECTION_RANDOM
*/
bool ChatHandler::HandleWpChangePathDirectionCommand(const char* args)
{
    ARGS_CHECK

    char* pathIdStr = strtok((char*)args, " ");
    uint32 pathId = uint32(atoi(pathIdStr));
    if(!pathId)
        return false;

    QueryResult result = WorldDatabase.PQuery( "SELECT 0 FROM waypoint_data WHERE id = '%u' LIMIT 1",pathId); 
    if(!result)
    {
        PSendSysMessage("No path of given id (%u) found", pathId);
        return true;
    }

    char* dirStr = strtok(NULL, " ");
    if(dirStr) //if the second argument was given
    { //setter
        uint32 dir = uint32(atoi(dirStr));
        if(dir >= WP_PATH_DIRECTION_TOTAL)
        {
            PSendSysMessage("Wrong direction given : %u", dir);
            return false;
        }
        
        //change in db
        PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_REP_WAYPOINT_PATH_DIRECTION);
        stmt->setUInt32(0, pathId);
        stmt->setUInt16(1, dir);
        WorldDatabase.Execute(stmt);

        //change in memory
        WaypointPath* path = (WaypointPath*)sWaypointMgr->GetPath(pathId);
        path->pathDirection = dir;

        std::string pathDirStr = GetWaypointPathDirectionName(WaypointPathDirection(dir));
        PSendSysMessage("Changed path %u direction to %s (%u)", pathId, pathDirStr.c_str(), dir);
    } else 
    { //getter
        // check db value
        PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_WAYPOINT_PATH_DIRECTION);
        stmt->setUInt32(0, pathId);
        PreparedQueryResult result = WorldDatabase.Query(stmt);
        
        if(result)
        {
            uint32 dir = result->Fetch()->GetUInt16();
            std::string pathDirStr = GetWaypointPathDirectionName(WaypointPathDirection(dir));
            PSendSysMessage("DB : Path id %u has direction set to %s (%u)", pathId, pathDirStr.c_str(), dir);
        } else {
            PSendSysMessage("No db entry found for path id %u", pathId);
        }
        // check memory value
        WaypointPath const* path = sWaypointMgr->GetPath(pathId);
        if(path)
        {
            uint8 dir = path->pathDirection;
            std::string pathDirStr = GetWaypointPathDirectionName(WaypointPathDirection(dir));
            PSendSysMessage("Current path value (in memory) : Path id %u has direction set to %s (%u)", pathId, pathDirStr.c_str(), dir);
        }
    }
    return true;
}

bool ChatHandler::HandleIRCReconnectCommand(const char* /* args */)
{
   PSendSysMessage("Trying to reconnect IRC...");
   sIRCMgr->connect();

   return true;
}

/* Syntax : .path type <pathid> [type] 
Note that this doesn't update creatures already using this path.

Possible types :
0 - WP_PATH_TYPE_LOOP
1 - WP_PATH_TYPE_ONCE
2 - WP_PATH_TYPE_ROUND_TRIP
*/
bool ChatHandler::HandleWpChangePathTypeCommand(const char* args)
{
    ARGS_CHECK

    char* pathIdStr = strtok((char*)args, " ");
    uint32 pathId = uint32(atoi(pathIdStr));
    if(!pathId)
        return false;

    QueryResult result = WorldDatabase.PQuery( "SELECT 0 FROM waypoint_data WHERE id = '%u' LIMIT 1",pathId); 
    if(!result)
    {
        PSendSysMessage("No path of given id (%u) found", pathId);
        return true;
    }

    char* typeStr = strtok(NULL, " ");
    if(typeStr) //if the second argument was given
    { //setter
        uint32 type = uint32(atoi(typeStr));
        if(type >= WP_PATH_TYPE_TOTAL)
        {
            PSendSysMessage("Wrong type given : %u", type);
            return false;
        }
        
        //change in db
        PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_REP_WAYPOINT_PATH_TYPE);
        stmt->setUInt32(0, pathId);
        stmt->setUInt16(1, type);
        WorldDatabase.Execute(stmt);

        //change in memory
        WaypointPath* path = (WaypointPath*)sWaypointMgr->GetPath(pathId);
        path->pathType = type;

        std::string pathTypeStr = GetWaypointPathTypeName(WaypointPathType(type));
        PSendSysMessage("Changed path %u type to %s (%u)", pathId, pathTypeStr.c_str(), type);
    } else 
    { //getter
        // check db value
        PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_WAYPOINT_PATH_TYPE);
        stmt->setUInt32(0, pathId);
        PreparedQueryResult result = WorldDatabase.Query(stmt);
        
        if(result)
        {
            uint32 type = result->Fetch()->GetUInt16();
            std::string pathTypeStr = GetWaypointPathTypeName(WaypointPathType(type));
            PSendSysMessage("DB : Path id %u has type set to %s (%u)", pathId, pathTypeStr.c_str(), type);
        } else {
            PSendSysMessage("No db entry found for path id %u", pathId);
        }
        // check memory value
        WaypointPath const* path = sWaypointMgr->GetPath(pathId);
        if(path)
        {
            uint8 type = path->pathType;
            std::string pathTypeStr = GetWaypointPathTypeName(WaypointPathType(type));
            PSendSysMessage("Memory : Path id %u has type set to %s (%u)", pathId, pathTypeStr.c_str(), type);
        }
    }
    return true;
}
