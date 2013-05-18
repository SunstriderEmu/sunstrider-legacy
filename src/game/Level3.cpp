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
#include "SkillDiscovery.h"
#include "SkillExtraItems.h"
#include "SystemConfig.h"
#include "Config/ConfigEnv.h"
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
#include "IRC.h"

#include "MoveMap.h"                                        // for mmap manager
#include "PathFinder.h"                                     // for mmap commands                                

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
    sLog.outString("Re-Loading Creature Texts...");
    sCreatureTextMgr.LoadCreatureTexts();
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
    sLog.outString( "Re-Loading Loot Tables..." );
    LoadLootTables();
    SendGlobalGMSysMessage("DB tables `*_loot_template` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadAllNpcCommand(const char* /*args*/)
{
    HandleReloadNpcGossipCommand("a");
    HandleReloadNpcOptionCommand("a");
    HandleReloadNpcTrainerCommand("a");
    HandleReloadNpcVendorCommand("a");
    return true;
}

bool ChatHandler::HandleReloadAllQuestCommand(const char* /*args*/)
{
    HandleReloadQuestAreaTriggersCommand("a");
    HandleReloadQuestTemplateCommand("a");

    sLog.outString( "Re-Loading Quests Relations..." );
    objmgr.LoadQuestRelations();
    SendGlobalGMSysMessage("DB tables `*_questrelation` and `*_involvedrelation` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadAllScriptsCommand(const char*)
{
    if(sWorld.IsScriptScheduled())
    {
        PSendSysMessage("DB scripts used currently, please attempt reload later.");
        SetSentErrorMessage(true);
        return false;
    }

    sLog.outString( "Re-Loading Scripts..." );
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
    HandleReloadLocalesNpcTextCommand("a");
    HandleReloadLocalesPageTextCommand("a");
    HandleReloadLocalesQuestCommand("a");
    return true;
}

bool ChatHandler::HandleReloadConfigCommand(const char* /*args*/)
{
    sLog.outString( "Re-Loading config settings..." );
    sWorld.LoadConfigSettings(true);
    MapManager::Instance().InitializeVisibilityDistanceInfo();
    sIRC.LoadConfigs();
    SendGlobalGMSysMessage("World and IRC config settings reloaded.");
    return true;
}

bool ChatHandler::HandleReloadAreaTriggerTavernCommand(const char*)
{
    sLog.outString( "Re-Loading Tavern Area Triggers..." );
    objmgr.LoadTavernAreaTriggers();
    SendGlobalGMSysMessage("DB table `areatrigger_tavern` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadAreaTriggerTeleportCommand(const char*)
{
    sLog.outString( "Re-Loading AreaTrigger teleport definitions..." );
    objmgr.LoadAreaTriggerTeleports();
    SendGlobalGMSysMessage("DB table `areatrigger_teleport` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadAccessRequirementCommand(const char*)
{
    sLog.outString( "Re-Loading Access Requirement definitions..." );
    objmgr.LoadAccessRequirements();
    SendGlobalGMSysMessage("DB table `access_requirement` reloaded.");
     return true;
 }

bool ChatHandler::HandleReloadCommandCommand(const char*)
{
    load_command_table = true;
    SendGlobalGMSysMessage("DB table `command` will be reloaded at next chat command use.");
    return true;
}

bool ChatHandler::HandleReloadCreatureQuestRelationsCommand(const char*)
{
    sLog.outString( "Loading Quests Relations... (`creature_questrelation`)" );
    objmgr.LoadCreatureQuestRelations();
    SendGlobalGMSysMessage("DB table `creature_questrelation` (creature quest givers) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadCreatureLinkedRespawnCommand(const char* args)
{
    sLog.outString( "Loading Linked Respawns... (`creature_linked_respawn`)" );
    objmgr.LoadCreatureLinkedRespawn();
    SendGlobalGMSysMessage("DB table `creature_linked_respawn` (creature linked respawns) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadCreatureQuestInvRelationsCommand(const char*)
{
    sLog.outString( "Loading Quests Relations... (`creature_involvedrelation`)" );
    objmgr.LoadCreatureInvolvedRelations();
    SendGlobalGMSysMessage("DB table `creature_involvedrelation` (creature quest takers) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadGOQuestRelationsCommand(const char*)
{
    sLog.outString( "Loading Quests Relations... (`gameobject_questrelation`)" );
    objmgr.LoadGameobjectQuestRelations();
    SendGlobalGMSysMessage("DB table `gameobject_questrelation` (gameobject quest givers) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadGOQuestInvRelationsCommand(const char*)
{
    sLog.outString( "Loading Quests Relations... (`gameobject_involvedrelation`)" );
    objmgr.LoadGameobjectInvolvedRelations();
    SendGlobalGMSysMessage("DB table `gameobject_involvedrelation` (gameobject quest takers) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadQuestAreaTriggersCommand(const char*)
{
    sLog.outString( "Re-Loading Quest Area Triggers..." );
    objmgr.LoadQuestAreaTriggers();
    SendGlobalGMSysMessage("DB table `areatrigger_involvedrelation` (quest area triggers) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadQuestTemplateCommand(const char*)
{
    sLog.outString( "Re-Loading Quest Templates..." );
    objmgr.LoadQuests();
    SendGlobalGMSysMessage("DB table `quest_template` (quest definitions) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadLootTemplatesCreatureCommand(const char*)
{
    sLog.outString( "Re-Loading Loot Tables... (`creature_loot_template`)" );
    LoadLootTemplates_Creature();
    LootTemplates_Creature.CheckLootRefs();
    SendGlobalGMSysMessage("DB table `creature_loot_template` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadLootTemplatesDisenchantCommand(const char*)
{
    sLog.outString( "Re-Loading Loot Tables... (`disenchant_loot_template`)" );
    LoadLootTemplates_Disenchant();
    LootTemplates_Disenchant.CheckLootRefs();
    SendGlobalGMSysMessage("DB table `disenchant_loot_template` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadLootTemplatesFishingCommand(const char*)
{
    sLog.outString( "Re-Loading Loot Tables... (`fishing_loot_template`)" );
    LoadLootTemplates_Fishing();
    LootTemplates_Fishing.CheckLootRefs();
    SendGlobalGMSysMessage("DB table `fishing_loot_template` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadLootTemplatesGameobjectCommand(const char*)
{
    sLog.outString( "Re-Loading Loot Tables... (`gameobject_loot_template`)" );
    LoadLootTemplates_Gameobject();
    LootTemplates_Gameobject.CheckLootRefs();
    SendGlobalGMSysMessage("DB table `gameobject_loot_template` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadLootTemplatesItemCommand(const char*)
{
    sLog.outString( "Re-Loading Loot Tables... (`item_loot_template`)" );
    LoadLootTemplates_Item();
    LootTemplates_Item.CheckLootRefs();
    SendGlobalGMSysMessage("DB table `item_loot_template` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadLootTemplatesPickpocketingCommand(const char*)
{
    sLog.outString( "Re-Loading Loot Tables... (`pickpocketing_loot_template`)" );
    LoadLootTemplates_Pickpocketing();
    LootTemplates_Pickpocketing.CheckLootRefs();
    SendGlobalGMSysMessage("DB table `pickpocketing_loot_template` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadLootTemplatesProspectingCommand(const char*)
{
    sLog.outString( "Re-Loading Loot Tables... (`prospecting_loot_template`)" );
    LoadLootTemplates_Prospecting();
    LootTemplates_Prospecting.CheckLootRefs();
    SendGlobalGMSysMessage("DB table `prospecting_loot_template` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadLootTemplatesQuestMailCommand(const char*)
{
    sLog.outString( "Re-Loading Loot Tables... (`quest_mail_loot_template`)" );
    LoadLootTemplates_QuestMail();
    LootTemplates_QuestMail.CheckLootRefs();
    SendGlobalGMSysMessage("DB table `quest_mail_loot_template` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadLootTemplatesReferenceCommand(const char*)
{
    sLog.outString( "Re-Loading Loot Tables... (`reference_loot_template`)" );
    LoadLootTemplates_Reference();
    SendGlobalGMSysMessage("DB table `reference_loot_template` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadLootTemplatesSkinningCommand(const char*)
{
    sLog.outString( "Re-Loading Loot Tables... (`skinning_loot_template`)" );
    LoadLootTemplates_Skinning();
    LootTemplates_Skinning.CheckLootRefs();
    SendGlobalGMSysMessage("DB table `skinning_loot_template` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadTrinityStringCommand(const char*)
{
    sLog.outString( "Re-Loading trinity_string Table!" );
    objmgr.LoadTrinityStrings();
    SendGlobalGMSysMessage("DB table `trinity_string` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadNpcOptionCommand(const char*)
{
    sLog.outString( "Re-Loading `npc_option` Table!" );
    objmgr.LoadNpcOptions();
    SendGlobalGMSysMessage("DB table `npc_option` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadNpcGossipCommand(const char*)
{
    sLog.outString( "Re-Loading `npc_gossip` Table!" );
    objmgr.LoadNpcTextId();
    SendGlobalGMSysMessage("DB table `npc_gossip` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadNpcTrainerCommand(const char*)
{
    sLog.outString( "Re-Loading `npc_trainer` Table!" );
    objmgr.LoadTrainerSpell();
    SendGlobalGMSysMessage("DB table `npc_trainer` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadNpcVendorCommand(const char*)
{
    sLog.outString( "Re-Loading `npc_vendor` Table!" );
    objmgr.LoadVendors();
    SendGlobalGMSysMessage("DB table `npc_vendor` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadReservedNameCommand(const char*)
{
    sLog.outString( "Loading ReservedNames... (`reserved_name`)" );
    objmgr.LoadReservedPlayersNames();
    SendGlobalGMSysMessage("DB table `reserved_name` (player reserved names) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadSkillDiscoveryTemplateCommand(const char* /*args*/)
{
    sLog.outString( "Re-Loading Skill Discovery Table..." );
    LoadSkillDiscoveryTable();
    SendGlobalGMSysMessage("DB table `skill_discovery_template` (recipes discovered at crafting) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadSkillExtraItemTemplateCommand(const char* /*args*/)
{
    sLog.outString( "Re-Loading Skill Extra Item Table..." );
    LoadSkillExtraItemTable();
    SendGlobalGMSysMessage("DB table `skill_extra_item_template` (extra item creation when crafting) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadSkillFishingBaseLevelCommand(const char* /*args*/)
{
    sLog.outString( "Re-Loading Skill Fishing base level requirements..." );
    objmgr.LoadFishingBaseSkillLevel();
    SendGlobalGMSysMessage("DB table `skill_fishing_base_level` (fishing base level for zone/subzone) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadSpellAffectCommand(const char*)
{
    sLog.outString( "Re-Loading SpellAffect definitions..." );
    spellmgr.LoadSpellAffects();
    SendGlobalGMSysMessage("DB table `spell_affect` (spell mods apply requirements) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadSpellRequiredCommand(const char*)
{
    sLog.outString( "Re-Loading Spell Required Data... " );
    spellmgr.LoadSpellRequired();
    SendGlobalGMSysMessage("DB table `spell_required` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadSpellElixirCommand(const char*)
{
    sLog.outString( "Re-Loading Spell Elixir types..." );
    spellmgr.LoadSpellElixirs();
    SendGlobalGMSysMessage("DB table `spell_elixir` (spell elixir types) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadSpellLearnSpellCommand(const char*)
{
    sLog.outString( "Re-Loading Spell Learn Spells..." );
    spellmgr.LoadSpellLearnSpells();
    SendGlobalGMSysMessage("DB table `spell_learn_spell` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadSpellLinkedSpellCommand(const char*)
{
    sLog.outString( "Re-Loading Spell Linked Spells..." );
    spellmgr.LoadSpellLinked();
    SendGlobalGMSysMessage("DB table `spell_linked_spell` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadSpellProcEventCommand(const char*)
{
    sLog.outString( "Re-Loading Spell Proc Event conditions..." );
    spellmgr.LoadSpellProcEvents();
    SendGlobalGMSysMessage("DB table `spell_proc_event` (spell proc trigger requirements) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadSpellScriptTargetCommand(const char*)
{
    sLog.outString( "Re-Loading SpellsScriptTarget..." );
    spellmgr.LoadSpellScriptTarget();
    SendGlobalGMSysMessage("DB table `spell_script_target` (spell targets selection in case specific creature/GO requirements) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadSpellTargetPositionCommand(const char*)
{
    sLog.outString( "Re-Loading Spell target coordinates..." );
    spellmgr.LoadSpellTargetPositions();
    SendGlobalGMSysMessage("DB table `spell_target_position` (destination coordinates for spell targets) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadSpellThreatsCommand(const char*)
{
    sLog.outString( "Re-Loading Aggro Spells Definitions...");
    spellmgr.LoadSpellThreats();
    SendGlobalGMSysMessage("DB table `spell_threat` (spell aggro definitions) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadSpellPetAurasCommand(const char*)
{
    sLog.outString( "Re-Loading Spell pet auras...");
    spellmgr.LoadSpellPetAuras();
    SendGlobalGMSysMessage("DB table `spell_pet_auras` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadPageTextsCommand(const char*)
{
    sLog.outString( "Re-Loading Page Texts..." );
    objmgr.LoadPageTexts();
    SendGlobalGMSysMessage("DB table `page_texts` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadItemEnchantementsCommand(const char*)
{
    sLog.outString( "Re-Loading Item Random Enchantments Table..." );
    LoadRandomEnchantmentsTable();
    SendGlobalGMSysMessage("DB table `item_enchantment_template` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadGameObjectScriptsCommand(const char* arg)
{
    if(sWorld.IsScriptScheduled())
    {
        SendSysMessage("DB scripts used currently, please attempt reload later.");
        SetSentErrorMessage(true);
        return false;
    }

    if(*arg!='a')
        sLog.outString( "Re-Loading Scripts from `gameobject_scripts`...");

    objmgr.LoadGameObjectScripts();

    if(*arg!='a')
        SendGlobalGMSysMessage("DB table `gameobject_scripts` reloaded.");

    return true;
}

bool ChatHandler::HandleReloadEventScriptsCommand(const char* arg)
{
    if(sWorld.IsScriptScheduled())
    {
        SendSysMessage("DB scripts used currently, please attempt reload later.");
        SetSentErrorMessage(true);
        return false;
    }

    if(*arg!='a')
        sLog.outString( "Re-Loading Scripts from `event_scripts`...");

    objmgr.LoadEventScripts();

    if(*arg!='a')
        SendGlobalGMSysMessage("DB table `event_scripts` reloaded.");

    return true;
}

bool ChatHandler::HandleReloadWpScriptsCommand(const char* arg)
{
    if(sWorld.IsScriptScheduled())
    {
        SendSysMessage("DB scripts used currently, please attempt reload later.");
        SetSentErrorMessage(true);
        return false;
    }

    if(*arg!='a')
        sLog.outString( "Re-Loading Scripts from `waypoint_scripts`...");

    objmgr.LoadWaypointScripts();

    if(*arg!='a')
        SendGlobalGMSysMessage("DB table `waypoint_scripts` reloaded.");

    return true;
}

bool ChatHandler::HandleReloadQuestEndScriptsCommand(const char* arg)
{
    if(sWorld.IsScriptScheduled())
    {
        SendSysMessage("DB scripts used currently, please attempt reload later.");
        SetSentErrorMessage(true);
        return false;
    }

    if(*arg!='a')
        sLog.outString( "Re-Loading Scripts from `quest_end_scripts`...");

    objmgr.LoadQuestEndScripts();

    if(*arg!='a')
        SendGlobalGMSysMessage("DB table `quest_end_scripts` reloaded.");

    return true;
}

bool ChatHandler::HandleReloadQuestStartScriptsCommand(const char* arg)
{
    if(sWorld.IsScriptScheduled())
    {
        SendSysMessage("DB scripts used currently, please attempt reload later.");
        SetSentErrorMessage(true);
        return false;
    }

    if(*arg!='a')
        sLog.outString( "Re-Loading Scripts from `quest_start_scripts`...");

    objmgr.LoadQuestStartScripts();

    if(*arg!='a')
        SendGlobalGMSysMessage("DB table `quest_start_scripts` reloaded.");

    return true;
}

bool ChatHandler::HandleReloadSpellScriptsCommand(const char* arg)
{
    if(sWorld.IsScriptScheduled())
    {
        SendSysMessage("DB scripts used currently, please attempt reload later.");
        SetSentErrorMessage(true);
        return false;
    }

    if(*arg!='a')
        sLog.outString( "Re-Loading Scripts from `spell_scripts`...");

    objmgr.LoadSpellScripts();

    if(*arg!='a')
        SendGlobalGMSysMessage("DB table `spell_scripts` reloaded.");

    return true;
}

bool ChatHandler::HandleReloadDbScriptStringCommand(const char* arg)
{
    sLog.outString( "Re-Loading Script strings from `db_script_string`...");
    objmgr.LoadDbScriptStrings();
    SendGlobalGMSysMessage("DB table `db_script_string` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadGameGraveyardZoneCommand(const char* /*arg*/)
{
    sLog.outString( "Re-Loading Graveyard-zone links...");

    objmgr.LoadGraveyardZones();

    SendGlobalGMSysMessage("DB table `game_graveyard_zone` reloaded.");

    return true;
}

bool ChatHandler::HandleReloadGameTeleCommand(const char* /*arg*/)
{
    sLog.outString( "Re-Loading Game Tele coordinates...");

    objmgr.LoadGameTele();

    SendGlobalGMSysMessage("DB table `game_tele` reloaded.");

    return true;
}

bool ChatHandler::HandleReloadSpellDisabledCommand(const char* /*arg*/)
{
    sLog.outString( "Re-Loading spell disabled table...");

    objmgr.LoadSpellDisabledEntrys();

    SendGlobalGMSysMessage("DB table `spell_disabled` reloaded.");

    return true;
}

bool ChatHandler::HandleReloadLocalesCreatureCommand(const char* /*arg*/)
{
    sLog.outString( "Re-Loading Locales Creature ...");
    objmgr.LoadCreatureLocales();
    SendGlobalGMSysMessage("DB table `locales_creature` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadLocalesGameobjectCommand(const char* /*arg*/)
{
    sLog.outString( "Re-Loading Locales Gameobject ... ");
    objmgr.LoadGameObjectLocales();
    SendGlobalGMSysMessage("DB table `locales_gameobject` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadLocalesItemCommand(const char* /*arg*/)
{
    sLog.outString( "Re-Loading Locales Item ... ");
    objmgr.LoadItemLocales();
    SendGlobalGMSysMessage("DB table `locales_item` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadLocalesNpcTextCommand(const char* /*arg*/)
{
    sLog.outString( "Re-Loading Locales NPC Text ... ");
    objmgr.LoadNpcTextLocales();
    SendGlobalGMSysMessage("DB table `locales_npc_text` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadLocalesPageTextCommand(const char* /*arg*/)
{
    sLog.outString( "Re-Loading Locales Page Text ... ");
    objmgr.LoadPageTextLocales();
    SendGlobalGMSysMessage("DB table `locales_page_text` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadLocalesQuestCommand(const char* /*arg*/)
{
    sLog.outString( "Re-Loading Locales Quest ... ");
    objmgr.LoadQuestLocales();
    SendGlobalGMSysMessage("DB table `locales_quest` reloaded.");
    return true;
}

bool ChatHandler::HandleLoadScriptsCommand(const char* args)
{
    if(!LoadScriptingModule()) return true;

    sWorld.SendGMText(LANG_SCRIPTS_RELOADED);
    return true;
}

bool ChatHandler::HandleReloadAuctionsCommand(const char* args)
{
    ///- Reload dynamic data tables from the database
    sLog.outString( "Re-Loading Auctions..." );
    sAHMgr.LoadAuctionItems();
    sAHMgr.LoadAuctions();
    SendGlobalGMSysMessage("Auctions reloaded.");
    return true;
}

bool ChatHandler::HandleAccountSetGmLevelCommand(const char* args)
{
    if(!*args)
        return false;

    std::string targetAccountName;
    uint32 targetAccountId = 0;
    uint32 targetSecurity = 0;
    uint32 gm = 0;
    char* arg1 = strtok((char*)args, " ");
    char* arg2 = strtok(NULL, " ");

    if(getSelectedPlayer() && arg1 && !arg2)
    {
        targetAccountId = getSelectedPlayer()->GetSession()->GetAccountId();
        sAccountMgr.GetName(targetAccountId, targetAccountName);
        Player* targetPlayer = getSelectedPlayer();
        gm = atoi(arg1);

        // Check for invalid specified GM level.
        if ( (gm < SEC_PLAYER || gm > SEC_ADMINISTRATOR) )
        {
            SendSysMessage(LANG_BAD_VALUE);
            SetSentErrorMessage(true);
            return false;
        }

        // Check if targets GM level and specified GM level is not higher than current gm level
        targetSecurity = targetPlayer->GetSession()->GetSecurity();
        if(targetSecurity >= m_session->GetSecurity() || gm >= m_session->GetSecurity() )
        {
            SendSysMessage(LANG_YOURS_SECURITY_IS_LOW);
            SetSentErrorMessage(true);
            return false;
        }

        // Decide which string to show
        if(m_session->GetPlayer()!=targetPlayer)
        {
            PSendSysMessage(LANG_YOU_CHANGE_SECURITY, targetAccountName.c_str(), gm);
        }else{
            PSendSysMessage(LANG_YOURS_SECURITY_CHANGED, m_session->GetPlayer()->GetName(), gm);
        }

        LoginDatabase.PExecute("UPDATE account SET gmlevel = '%d' WHERE id = '%u'", gm, targetAccountId);
        return true;
    }else
    {
        // Check for second parameter
        if(!arg2)
            return false;

        // Check for account
        targetAccountName = arg1;
        if(!AccountMgr::normalizeString(targetAccountName))
        {
            PSendSysMessage(LANG_ACCOUNT_NOT_EXIST,targetAccountName.c_str());
            SetSentErrorMessage(true);
            return false;
        }

        // Check for invalid specified GM level.
        gm = atoi(arg2);
        if ( (gm < SEC_PLAYER || gm > SEC_ADMINISTRATOR) )
        {
            SendSysMessage(LANG_BAD_VALUE);
            SetSentErrorMessage(true);
            return false;
        }

        targetAccountId = sAccountMgr.GetId(arg1);
        /// m_session==NULL only for console
        uint32 plSecurity = m_session ? m_session->GetSecurity() : SEC_CONSOLE;

        /// can set security level only for target with less security and to less security that we have
        /// This is also reject self apply in fact
        targetSecurity = sAccountMgr.GetSecurity(targetAccountId);
        if(targetSecurity >= plSecurity || gm >= plSecurity )
        {
            SendSysMessage(LANG_YOURS_SECURITY_IS_LOW);
            SetSentErrorMessage(true);
            return false;
        }

        PSendSysMessage(LANG_YOU_CHANGE_SECURITY, targetAccountName.c_str(), gm);
        LoginDatabase.PExecute("UPDATE account SET gmlevel = '%d' WHERE id = '%u'", gm, targetAccountId);
        return true;
    }
}

/// Set password for account
bool ChatHandler::HandleAccountSetPasswordCommand(const char* args)
{
    if(!*args)
        return false;

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

    uint32 targetAccountId = sAccountMgr.GetId(account_name);
    if (!targetAccountId)
    {
        PSendSysMessage(LANG_ACCOUNT_NOT_EXIST,account_name.c_str());
        SetSentErrorMessage(true);
        return false;
    }

    uint32 targetSecurity = sAccountMgr.GetSecurity(targetAccountId);

    /// m_session==NULL only for console
    uint32 plSecurity = m_session ? m_session->GetSecurity() : SEC_CONSOLE;

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

    AccountOpResult result = sAccountMgr.ChangePassword(targetAccountId, szPassword1);

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
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

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
        PSendSysMessage(LANG_SET_SKILL_ERROR, target->GetName(), skill, sl->name[0]);
        SetSentErrorMessage(true);
        return false;
    }

    int32 max   = max_p ? atol (max_p) : target->GetPureMaxSkillValue(skill);

    if( level <= 0 || level > max || max <= 0 )
        return false;

    target->SetSkill(skill, level, max);
    PSendSysMessage(LANG_SET_SKILL, skill, sl->name[0], target->GetName(), level, max);

    return true;
}

bool ChatHandler::HandleUnLearnCommand(const char* args)
{
    if (!*args)
        return false;

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
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    if (!*args)
    {
        target->RemoveAllSpellCooldown();
        PSendSysMessage(LANG_REMOVEALL_COOLDOWN, target->GetName());
    }
    else
    {
        // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
        uint32 spell_id = extractSpellIdFromLink((char*)args);
        if(!spell_id)
            return false;

        if(!spellmgr.LookupSpell(spell_id))
        {
            PSendSysMessage(LANG_UNKNOWN_SPELL, target==m_session->GetPlayer() ? GetTrinityString(LANG_YOU) : target->GetName());
            SetSentErrorMessage(true);
            return false;
        }

        WorldPacket data( SMSG_CLEAR_COOLDOWN, (4+8) );
        data << uint32(spell_id);
        data << uint64(target->GetGUID());
        target->GetSession()->SendPacket(&data);
        target->RemoveSpellCooldown(spell_id);
        PSendSysMessage(LANG_REMOVE_COOLDOWN, spell_id, target==m_session->GetPlayer() ? GetTrinityString(LANG_YOU) : target->GetName());
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

        SpellEntry const* spellInfo = spellmgr.LookupSpell(spell);
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
        uint32 spell = atol((char*)gmSpellList[gmSpellIter++]);

        SpellEntry const* spellInfo = spellmgr.LookupSpell(spell);
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
    ChrClassesEntry const* clsEntry = sChrClassesStore.LookupEntry(m_session->GetPlayer()->getClass());
    if(!clsEntry)
        return true;
    uint32 family = clsEntry->spellfamily;

    //for (uint32 i = 0; i < sSpellStore.GetNumRows(); i++)
    for (std::map<uint32, SpellEntry*>::iterator itr = objmgr.GetSpellStore()->begin(); itr != objmgr.GetSpellStore()->end(); itr++)
    {
        uint32 i = itr->first;
        SpellEntry const *spellInfo = spellmgr.LookupSpell(i);
        if(!spellInfo)
            continue;

        // skip wrong class/race skills
        if(!m_session->GetPlayer()->IsSpellFitByClassAndRace(spellInfo->Id))
            continue;

        // skip other spell families
        if( spellInfo->SpellFamilyName != family)
            continue;

        //TODO: skip triggered spells

        // skip spells with first rank learned as talent (and all talents then also)
        uint32 first_rank = spellmgr.GetFirstSpellInChain(spellInfo->Id);
        if(GetTalentSpellCost(first_rank) > 0 )
            continue;

        // skip broken spells
        if(!SpellMgr::IsSpellValid(spellInfo,m_session->GetPlayer(),false))
            continue;

        m_session->GetPlayer()->learnSpell(i);
    }

    SendSysMessage(LANG_COMMAND_LEARN_CLASS_SPELLS);
    return true;
}

static void learnAllHighRanks(Player* player, uint32 spellid)
{
    SpellChainNode const* node;
    do
    {
        node = spellmgr.GetSpellChainNode(spellid);
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
    uint32 classMask = player->getClassMask();

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

        SpellEntry const* spellInfo = spellmgr.LookupSpell(spellid);
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

        player = objmgr.GetPlayer(name.c_str());
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

    PSendSysMessage(LANG_COMMAND_LEARN_ALL_DEFAULT_AND_QUEST,player->GetName());
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
    if(!spell || !spellmgr.LookupSpell(spell))
        return false;

    if (targetPlayer->HasSpell(spell))
    {
        if(targetPlayer == m_session->GetPlayer())
            SendSysMessage(LANG_YOU_KNOWN_SPELL);
        else
            PSendSysMessage(LANG_TARGET_KNOWN_SPELL,targetPlayer->GetName());
        SetSentErrorMessage(true);
        return false;
    }

    SpellEntry const* spellInfo = spellmgr.LookupSpell(spell);
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
    if (!*args)
        return false;

    uint32 itemId = 0;

    if(args[0]=='[')                                        // [name] manual form
    {
        char* citemName = citemName = strtok((char*)args, "]");

        if(citemName && citemName[0])
        {
            std::string itemName = citemName+1;
            WorldDatabase.escape_string(itemName);
            QueryResult *result = WorldDatabase.PQuery("SELECT entry FROM item_template WHERE name = '%s'", itemName.c_str());
            if (!result)
            {
                PSendSysMessage(LANG_COMMAND_COULDNOTFIND, citemName+1);
                SetSentErrorMessage(true);
                return false;
            }
            itemId = result->Fetch()->GetUInt16();
            delete result;
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

    sLog.outDetail(GetTrinityString(LANG_ADDITEM), itemId, count);

    ItemPrototype const *pProto = objmgr.GetItemPrototype(itemId);
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
        PSendSysMessage(LANG_REMOVEITEM, itemId, -count, plTarget->GetName());
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
    if (!*args)
        return false;

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

    sLog.outDetail(GetTrinityString(LANG_ADDITEMSET), itemsetId);

    bool found = false;
    for (uint32 id = 0; id < sItemStorage.MaxEntry; id++)
    {
        ItemPrototype const *pProto = sItemStorage.LookupEntry<ItemPrototype>(id);
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
    if(!*args)
        return false;

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

    ItemPrototype const* itemProto = objmgr.GetItemPrototype(item_id);
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

    QueryResult *result;

    // inventory case
    uint32 inv_count = 0;
    result=CharacterDatabase.PQuery("SELECT COUNT(item_template) FROM character_inventory WHERE item_template='%u'",item_id);
    if(result)
    {
        inv_count = (*result)[0].GetUInt32();
        delete result;
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
            std::string owner_name = fields[5].GetCppString();

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

        delete result;

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
        delete result;
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
            std::string item_s_name = fields[4].GetCppString();
            uint32 item_r_acc       = fields[5].GetUInt32();
            std::string item_r_name = fields[6].GetCppString();

            char const* item_pos = "[in mail]";

            PSendSysMessage(LANG_ITEMLIST_MAIL,
                item_guid,item_s_name.c_str(),item_s,item_s_acc,item_r_name.c_str(),item_r,item_r_acc,item_pos);
        } while (result->NextRow());

        int64 res_count = result->GetRowCount();

        delete result;

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
        delete result;
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
            std::string owner_name = fields[3].GetCppString();

            char const* item_pos = "[in auction]";

            PSendSysMessage(LANG_ITEMLIST_AUCTION, item_guid, owner_name.c_str(), owner, owner_acc,item_pos);
        } while (result->NextRow());

        delete result;
    }

    // guild bank case
    uint32 guild_count = 0;
    result=CharacterDatabase.PQuery("SELECT COUNT(item_entry) FROM guild_bank_item WHERE item_entry='%u'",item_id);
    if(result)
    {
        guild_count = (*result)[0].GetUInt32();
        delete result;
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
            std::string guild_name = fields[2].GetCppString();

            char const* item_pos = "[in guild bank]";

            PSendSysMessage(LANG_ITEMLIST_GUILD,item_guid,guild_name.c_str(),guild_guid,item_pos);
        } while (result->NextRow());

        int64 res_count = result->GetRowCount();

        delete result;

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
    if(!*args)
        return false;

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

    GameObjectInfo const * gInfo = objmgr.GetGameObjectInfo(go_id);
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

    QueryResult *result;

    uint32 obj_count = 0;
    result=WorldDatabase.PQuery("SELECT COUNT(guid) FROM gameobject WHERE id='%u'",go_id);
    if(result)
    {
        obj_count = (*result)[0].GetUInt32();
        delete result;
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
                PSendSysMessage(LANG_GO_LIST_CHAT, guid, id, guid, gInfo->name, x, y, z, mapid);
            else
                PSendSysMessage(LANG_GO_LIST_CONSOLE, guid, gInfo->name, x, y, z, mapid);
        } while (result->NextRow());

        delete result;
    }

    PSendSysMessage(LANG_COMMAND_LISTOBJMESSAGE,go_id,obj_count);
    return true;
}

bool ChatHandler::HandleNearObjectCommand(const char* args)
{
    float distance = (!*args) ? 10 : atol(args);
    uint32 count = 0;

    Player* pl = m_session->GetPlayer();
    QueryResult *result = WorldDatabase.PQuery("SELECT guid, id, position_x, position_y, position_z, map, "
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

            GameObjectInfo const * gInfo = objmgr.GetGameObjectInfo(entry);

            if(!gInfo)
                continue;

            PSendSysMessage(LANG_GO_LIST_CHAT, guid, entry, guid, gInfo->name, x, y, z, mapid);

            ++count;
        } while (result->NextRow());

        delete result;
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

    if(GameObjectData const* goData = objmgr.GetGOData(lowguid))
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
    if(!*args)
        return false;

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

    CreatureInfo const* cInfo = objmgr.GetCreatureTemplate(cr_id);
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

    QueryResult *result;

    uint32 cr_count = 0;
    result=WorldDatabase.PQuery("SELECT COUNT(guid) FROM creature WHERE id='%u'",cr_id);
    if(result)
    {
        cr_count = (*result)[0].GetUInt32();
        delete result;
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
                PSendSysMessage(LANG_CREATURE_LIST_CHAT, guid, guid, cInfo->Name, x, y, z, mapid);
            else
                PSendSysMessage(LANG_CREATURE_LIST_CONSOLE, guid, cInfo->Name, x, y, z, mapid);
        } while (result->NextRow());

        delete result;
    }

    PSendSysMessage(LANG_COMMAND_LISTCREATUREMESSAGE,cr_id,cr_count);
    return true;
}

bool ChatHandler::HandleLookupItemCommand(const char* args)
{
    if(!*args)
        return false;

    std::string namepart = args;
    std::wstring wnamepart;

    // converting string that we try to find to lower case
    if(!Utf8toWStr(namepart,wnamepart))
        return false;

    wstrToLower(wnamepart);

    uint32 counter = 0;

    // Search in `item_template`
    for (uint32 id = 0; id < sItemStorage.MaxEntry; id++)
    {
        ItemPrototype const *pProto = sItemStorage.LookupEntry<ItemPrototype >(id);
        if(!pProto)
            continue;

        int loc_idx = m_session ? m_session->GetSessionDbLocaleIndex() : objmgr.GetDBCLocaleIndex();
        if ( loc_idx >= 0 )
        {
            ItemLocale const *il = objmgr.GetItemLocale(pProto->ItemId);
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
    if(!*args)
        return false;

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
            int loc = m_session ? m_session->GetSessionDbcLocale() : sWorld.GetDefaultDbcLocale();
            std::string name = set->name[loc];
            if(name.empty())
                continue;

            if (!Utf8FitTo(name, wnamepart))
            {
                loc = 0;
                for(; loc < MAX_LOCALE; ++loc)
                {
                    if(m_session && loc==m_session->GetSessionDbcLocale())
                        continue;

                    name = set->name[loc];
                    if(name.empty())
                        continue;

                    if (Utf8FitTo(name, wnamepart))
                        break;
                }
            }

            if(loc < MAX_LOCALE)
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
    if(!*args)
        return false;

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
            int loc = m_session ? m_session->GetSessionDbcLocale() : sWorld.GetDefaultDbcLocale();
            std::string name = skillInfo->name[loc];
            if(name.empty())
                continue;

            if (!Utf8FitTo(name, wnamepart))
            {
                loc = 0;
                for(; loc < MAX_LOCALE; ++loc)
                {
                    if(m_session && loc==m_session->GetSessionDbcLocale())
                        continue;

                    name = skillInfo->name[loc];
                    if(name.empty())
                        continue;

                    if (Utf8FitTo(name, wnamepart))
                        break;
                }
            }

            if(loc < MAX_LOCALE)
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

bool ChatHandler::HandleLookupSpellCommand(const char* args)
{
    if(!*args)
        return false;

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
    for (std::map<uint32, SpellEntry*>::iterator itr = objmgr.GetSpellStore()->begin(); itr != objmgr.GetSpellStore()->end(); itr++)
    {
        uint32 id = itr->first;
        SpellEntry const *spellInfo = spellmgr.LookupSpell(id);
        if(spellInfo)
        {
            int loc = m_session ? m_session->GetSessionDbcLocale() : sWorld.GetDefaultDbcLocale();
            std::string name = spellInfo->SpellName[loc];
            if(name.empty())
                continue;

            if (!Utf8FitTo(name, wnamepart))
            {
                loc = 0;
                for(; loc < MAX_LOCALE; ++loc)
                {
                    if(m_session && loc==m_session->GetSessionDbcLocale())
                        continue;

                    name = spellInfo->SpellName[loc];
                    if(name.empty())
                        continue;

                    if (Utf8FitTo(name, wnamepart))
                        break;
                }
            }

            if(loc < MAX_LOCALE)
            {
                bool known = target && target->HasSpell(id);
                bool learn = (spellInfo->Effect[0] == SPELL_EFFECT_LEARN_SPELL);

                uint32 talentCost = GetTalentSpellCost(id);

                bool talent = (talentCost > 0);
                bool passive = IsPassiveSpell(id);
                bool active = target && (target->HasAura(id,0) || target->HasAura(id,1) || target->HasAura(id,2));

                // unit32 used to prevent interpreting uint8 as char at output
                // find rank of learned spell for learning spell, or talent rank
                uint32 rank = talentCost ? talentCost : spellmgr.GetSpellRank(learn ? spellInfo->EffectTriggerSpell[0] : id);

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
    if(!*args)
        return false;

    // can be NULL at console call
    Player* target = getSelectedPlayer();

    std::string namepart = args;
    std::wstring wnamepart;

    // converting string that we try to find to lower case
    if(!Utf8toWStr(namepart,wnamepart))
        return false;

    wstrToLower(wnamepart);

    uint32 counter = 0 ;

    ObjectMgr::QuestMap const& qTemplates = objmgr.GetQuestTemplates();
    for (ObjectMgr::QuestMap::const_iterator iter = qTemplates.begin(); iter != qTemplates.end(); ++iter)
    {
        Quest * qinfo = iter->second;

        int loc_idx = m_session ? m_session->GetSessionDbLocaleIndex() : objmgr.GetDBCLocaleIndex();
        if ( loc_idx >= 0 )
        {
            QuestLocale const *il = objmgr.GetQuestLocale(qinfo->GetQuestId());
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
                        QueryResult* result = WorldDatabase.PQuery("SELECT bugged, comment FROM quest_bugs WHERE entry = %u", qinfo->GetQuestId());
                        if (result) {
                            Field* fields = result->Fetch();
                            if (fields[0].GetUInt8())
                                PSendSysMessage(" -> BUG: %s", fields[1].GetCppString().c_str());
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
    if (!*args)
        return false;

    std::string namepart = args;
    std::wstring wnamepart;

    // converting string that we try to find to lower case
    if (!Utf8toWStr (namepart,wnamepart))
        return false;

    wstrToLower (wnamepart);

    uint32 counter = 0;

    for (uint32 id = 0; id< sCreatureStorage.MaxEntry; ++id)
    {
        CreatureInfo const* cInfo = sCreatureStorage.LookupEntry<CreatureInfo> (id);
        if(!cInfo)
            continue;

        int loc_idx = m_session ? m_session->GetSessionDbLocaleIndex() : objmgr.GetDBCLocaleIndex();
        if (loc_idx >= 0)
        {
            CreatureLocale const *cl = objmgr.GetCreatureLocale (id);
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
    if(!*args)
        return false;

    std::string namepart = args;
    std::wstring wnamepart;

    // converting string that we try to find to lower case
    if(!Utf8toWStr(namepart,wnamepart))
        return false;

    wstrToLower(wnamepart);

    uint32 counter = 0;

    for (uint32 id = 0; id< sGOStorage.MaxEntry; id++ )
    {
        GameObjectInfo const* gInfo = sGOStorage.LookupEntry<GameObjectInfo>(id);
        if(!gInfo)
            continue;

        int loc_idx = m_session ? m_session->GetSessionDbLocaleIndex() : objmgr.GetDBCLocaleIndex();
        if ( loc_idx >= 0 )
        {
            GameObjectLocale const *gl = objmgr.GetGameObjectLocale(id);
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

    if (!*args)
        return false;

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

    Player* player = ObjectAccessor::Instance ().FindPlayerByName (lname);
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

    objmgr.AddGuild (guild);
    return true;
}

bool ChatHandler::HandleGuildInviteCommand(const char *args)
{
    if (!*args)
        return false;

    char* par1 = strtok ((char*)args, " ");
    char* par2 = strtok (NULL, "");
    if(!par1 || !par2)
        return false;

    std::string glName = par2;
    Guild* targetGuild = objmgr.GetGuildByName (glName);
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
    if (Player* targetPlayer = ObjectAccessor::Instance ().FindPlayerByName (plName.c_str ()))
        plGuid = targetPlayer->GetGUID ();
    else
        plGuid = objmgr.GetPlayerGUIDByName (plName.c_str ());

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
    if (!*args)
        return false;

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
    if (Player* targetPlayer = ObjectAccessor::Instance ().FindPlayerByName (plName.c_str ()))
    {
        plGuid = targetPlayer->GetGUID ();
        glId   = targetPlayer->GetGuildId ();
    }
    else
    {
        plGuid = objmgr.GetPlayerGUIDByName (plName.c_str ());
        glId = Player::GetGuildIdFromDB (plGuid);
    }

    if (!plGuid || !glId)
        return false;

    Guild* targetGuild = objmgr.GetGuildById (glId);
    if (!targetGuild)
        return false;

    targetGuild->DelMember (plGuid);

    return true;
}

bool ChatHandler::HandleGuildRankCommand(const char *args)
{
    if (!*args)
        return false;

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
    if (Player* targetPlayer = ObjectAccessor::Instance ().FindPlayerByName (plName.c_str ()))
    {
        plGuid = targetPlayer->GetGUID ();
        glId   = targetPlayer->GetGuildId ();
    }
    else
    {
        plGuid = objmgr.GetPlayerGUIDByName (plName.c_str ());
        glId = Player::GetGuildIdFromDB (plGuid);
    }

    if (!plGuid || !glId)
        return false;

    Guild* targetGuild = objmgr.GetGuildById (glId);
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
    if (!*args)
        return false;

    char* par1 = strtok ((char*)args, " ");
    char* par2 = strtok(NULL, " ");
    if (!par1)
        return false;

    std::string gld = par1;

    Guild* targetGuild = NULL;
    if (gld == "id") {
        if (!par2)
            return false;
        targetGuild = objmgr.GetGuildById(atoi(par2));
    } else {
        targetGuild = objmgr.GetGuildByName (gld);
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
    if (!*args)
        return false;
 
    Creature *pCreature = getSelectedCreature();
 
    if(!pCreature)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        return false;
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
   
    const ItemPrototype* proto = objmgr.GetItemPrototype(itemID);
    if (!proto)
    {
        PSendSysMessage(LANG_ITEM_NOT_FOUND,itemID);
        return false;
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
                return false;
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
            return false;
            break;
    }
    uint32 equipinfo = proto->Class + proto->SubClass * 256;
    pCreature->SetUInt32Value(UNIT_VIRTUAL_ITEM_INFO + ((slotID-1) * 2) + 0, equipinfo);
    pCreature->SetUInt32Value(UNIT_VIRTUAL_ITEM_INFO + ((slotID-1) * 2) + 1, proto->InventoryType);
 
    pCreature->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY + (slotID-1), proto->DisplayInfoID);
 
    PSendSysMessage(LANG_ITEM_ADDED_TO_SLOT,itemID,proto->Name1,slotID);
    return true;
}

bool ChatHandler::HandleDieCommand(const char* /*args*/)
{
    Unit* target = getSelectedUnit();

    if(!target || !m_session->GetPlayer()->GetSelection())
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    if( target->isAlive() )
    {
        //m_session->GetPlayer()->DealDamage(target, target->GetHealth(), NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
        m_session->GetPlayer()->Kill(target);
    }

    return true;
}

bool ChatHandler::HandleDamageCommand(const char * args)
{
    if (!*args)
        return false;

    Unit* target = getSelectedUnit();

    if(!target || !m_session->GetPlayer()->GetSelection())
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    if( !target->isAlive() )
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
    if(!spellid || !spellmgr.LookupSpell(spellid))
        return false;

    m_session->GetPlayer()->SpellNonMeleeDamageLog(target, spellid, damage, false);
    return true;
}

bool ChatHandler::HandleModifyArenaCommand(const char * args)
{
    if (!*args)
        return false;

    Player *target = getSelectedPlayer();
    if(!target)
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    int32 amount = (uint32)atoi(args);

    target->ModifyArenaPoints(amount);

    PSendSysMessage(LANG_COMMAND_MODIFY_ARENA, target->GetName(), target->GetArenaPoints());

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

        SelectedPlayer = objmgr.GetPlayer(name.c_str());
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
    SpellEntry const *spellInfo = spellmgr.LookupSpell( spellID );
    if(spellInfo)
    {
        for(uint32 i = 0;i<3;i++)
        {
            uint8 eff = spellInfo->Effect[i];
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
    if(!*args)
        return false;

    char* px = strtok((char*)args, " ");
    if (!px)
        return false;

    uint32 g_id = (uint32)atoi(px);

    uint32 g_team;

    char* px2 = strtok(NULL, " ");

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
        PSendSysMessage(LANG_COMMAND_GRAVEYARDNOEXIST, g_id);
        SetSentErrorMessage(true);
        return false;
    }

    Player* player = m_session->GetPlayer();

    uint32 zoneId = player->GetZoneId();

    AreaTableEntry const *areaEntry = GetAreaEntryByAreaID(zoneId);
    if(!areaEntry || areaEntry->zone !=0 )
    {
        PSendSysMessage(LANG_COMMAND_GRAVEYARDWRONGZONE, g_id,zoneId);
        SetSentErrorMessage(true);
        return false;
    }

    if(objmgr.AddGraveYardLink(g_id,player->GetZoneId(),g_team))
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
        g_team = HORDE;
    else if (strncmp((char*)args,"alliance",argslen)==0)
        g_team = ALLIANCE;
    else
        return false;

    Player* player = m_session->GetPlayer();

    WorldSafeLocsEntry const* graveyard = objmgr.GetClosestGraveYard(
        player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(),player->GetMapId(),g_team);

    if(graveyard)
    {
        uint32 g_id = graveyard->ID;

        GraveYardData const* data = objmgr.FindGraveYardData(g_id,player->GetZoneId());
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
        else if(g_team == HORDE)
            team_name = GetTrinityString(LANG_COMMAND_GRAVEYARD_HORDE);
        else if(g_team == ALLIANCE)
            team_name = GetTrinityString(LANG_COMMAND_GRAVEYARD_ALLIANCE);

        PSendSysMessage(LANG_COMMAND_GRAVEYARDNEAREST, g_id,team_name.c_str(),player->GetZoneId());
    }
    else
    {
        std::string team_name;

        if(g_team == 0)
            team_name = GetTrinityString(LANG_COMMAND_GRAVEYARD_ANY);
        else if(g_team == HORDE)
            team_name = GetTrinityString(LANG_COMMAND_GRAVEYARD_HORDE);
        else if(g_team == ALLIANCE)
            team_name = GetTrinityString(LANG_COMMAND_GRAVEYARD_ALLIANCE);

        if(g_team == ~uint32(0))
            PSendSysMessage(LANG_COMMAND_ZONENOGRAVEYARDS, player->GetZoneId());
        else
            PSendSysMessage(LANG_COMMAND_ZONENOGRAFACTION, player->GetZoneId(),team_name.c_str());
    }

    return true;
}

//play npc emote
bool ChatHandler::HandleNpcPlayEmoteCommand(const char* args)
{
    uint32 emote = atoi((char*)args);

    Creature* target = getSelectedCreature();
    if(!target)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    target->SetUInt32Value(UNIT_NPC_EMOTESTATE,emote);
    WorldDatabase.PExecute("UPDATE creature_addon SET emote = '%u' WHERE guid = '%u'", emote, target->GetDBTableGUIDLow());

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

    uint32 faction = target->getFaction();
    uint32 npcflags = target->GetUInt32Value(UNIT_NPC_FLAGS);
    uint32 displayid = target->GetDisplayId();
    uint32 nativeid = target->GetNativeDisplayId();
    uint32 Entry = target->GetEntry();
    CreatureInfo const* cInfo = target->GetCreatureInfo();
    CreatureData const* cData = objmgr.GetCreatureData(target->GetDBTableGUIDLow());

    int32 curRespawnDelay = target->GetRespawnTimeEx()-time(NULL);
    if(curRespawnDelay < 0)
        curRespawnDelay = 0;
    std::string curRespawnDelayStr = secsToTimeString(curRespawnDelay,true);
    std::string defRespawnDelayStr = secsToTimeString(target->GetRespawnDelay(),true);

    PSendSysMessage(LANG_NPCINFO_CHAR,  target->GetDBTableGUIDLow(), faction, npcflags, Entry, displayid, nativeid);
    PSendSysMessage(LANG_NPCINFO_LEVEL, target->getLevel());
    PSendSysMessage(LANG_NPCINFO_HEALTH,target->GetCreateHealth(), target->GetMaxHealth(), target->GetHealth());
    PSendSysMessage(LANG_NPCINFO_FLAGS, target->GetUInt32Value(UNIT_FIELD_FLAGS), target->GetUInt32Value(UNIT_DYNAMIC_FLAGS), target->getFaction());
    PSendSysMessage(LANG_COMMAND_RAWPAWNTIMES, defRespawnDelayStr.c_str(),curRespawnDelayStr.c_str());
    PSendSysMessage(LANG_NPCINFO_LOOT,  cInfo->lootid,cInfo->pickpocketLootId,cInfo->SkinLootId);
    PSendSysMessage(LANG_NPCINFO_DUNGEON_ID, target->GetInstanceId());
    PSendSysMessage(LANG_NPCINFO_POSITION,float(target->GetPositionX()), float(target->GetPositionY()), float(target->GetPositionZ()));
    PSendSysMessage("AIName: %s", target->GetAIName().c_str());
    PSendSysMessage("ScriptName: %s", target->GetScriptName().c_str());
    PSendSysMessage("ScriptName (new): %s", target->getScriptName().c_str());
    PSendSysMessage("Creature Pool ID: %u", target->GetCreaturePoolId());
    PSendSysMessage("Creature linked instance event: %d", int(target->getInstanceEventId()));
    if(const CreatureData* const linked = target->GetLinkedRespawnCreatureData())
        if(CreatureInfo const *master = GetCreatureInfo(linked->id))
            PSendSysMessage(LANG_NPCINFO_LINKGUID, objmgr.GetLinkedRespawnGuid(target->GetDBTableGUIDLow()), linked->id, master->Name);

    if ((npcflags & UNIT_NPC_FLAG_VENDOR) )
    {
        SendSysMessage(LANG_NPCINFO_VENDOR);
    }
    if ((npcflags & UNIT_NPC_FLAG_TRAINER) )
    {
        SendSysMessage(LANG_NPCINFO_TRAINER);
    }

    return true;
}

bool ChatHandler::HandleExploreCheatCommand(const char* args)
{
    if (!*args)
        return false;

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
        PSendSysMessage(LANG_YOU_SET_EXPLORE_ALL, chr->GetName());
        if (needReportToTarget(chr))
            ChatHandler(chr).PSendSysMessage(LANG_YOURS_EXPLORE_SET_ALL,GetName());
    }
    else
    {
        PSendSysMessage(LANG_YOU_SET_EXPLORE_NOTHING, chr->GetName());
        if (needReportToTarget(chr))
            ChatHandler(chr).PSendSysMessage(LANG_YOURS_EXPLORE_SET_NOTHING,GetName());
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
    if(!args)
        return false;

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

    PSendSysMessage(LANG_YOU_SET_WATERWALK, args, player->GetName());
    if(needReportToTarget(player))
        ChatHandler(player).PSendSysMessage(LANG_YOUR_WATERWALK_SET, args, GetName());
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

        chr = objmgr.GetPlayer(name.c_str());
        if(!chr)                                            // not in game
        {
            chr_guid = objmgr.GetPlayerGUIDByName(name);
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

    int32 oldlevel = chr ? chr->getLevel() : Player::GetLevelFromDB(chr_guid);
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

    if(m_session->GetPlayer() != chr)                       // including chr==NULL
        PSendSysMessage(LANG_YOU_CHANGE_LVL,name.c_str(),newlevel);
    return true;
}

bool ChatHandler::HandleShowAreaCommand(const char* args)
{
    if (!*args)
        return false;

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
    if (!*args)
        return false;

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

bool ChatHandler::HandleUpdate(const char* args)
{
    if(!*args)
        return false;

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
    if(!*args)
        return false;

    //Weather is OFF
    if (!sWorld.getConfig(CONFIG_WEATHER))
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

    Weather* wth = sWorld.FindWeather(zoneid);

    if(!wth)
        wth = sWorld.AddWeather(zoneid);
    if(!wth)
    {
        SendSysMessage(LANG_NO_WEATHER);
        SetSentErrorMessage(true);
        return false;
    }

    wth->SetWeather(WeatherType(type), grade);

    return true;
}

bool ChatHandler::HandleSetValue(const char* args)
{
    if(!*args)
        return false;

    char* px = strtok((char*)args, " ");
    char* py = strtok(NULL, " ");
    char* pz = strtok(NULL, " ");

    if (!px || !py)
        return false;

    Unit* target = getSelectedUnit();
    if(!target)
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    uint64 guid = target->GetGUID();

    uint32 Opcode = (uint32)atoi(px);
    if(Opcode >= target->GetValuesCount())
    {
        PSendSysMessage(LANG_TOO_BIG_INDEX, Opcode, GUID_LOPART(guid), target->GetValuesCount());
        return false;
    }
    uint32 iValue;
    float fValue;
    bool isint32 = true;
    if(pz)
        isint32 = (bool)atoi(pz);
    if(isint32)
    {
        iValue = (uint32)atoi(py);
        sLog.outDebug(GetTrinityString(LANG_SET_UINT), GUID_LOPART(guid), Opcode, iValue);
        target->SetUInt32Value( Opcode , iValue );
        PSendSysMessage(LANG_SET_UINT_FIELD, GUID_LOPART(guid), Opcode,iValue);
    }
    else
    {
        fValue = (float)atof(py);
        sLog.outDebug(GetTrinityString(LANG_SET_FLOAT), GUID_LOPART(guid), Opcode, fValue);
        target->SetFloatValue( Opcode , fValue );
        PSendSysMessage(LANG_SET_FLOAT_FIELD, GUID_LOPART(guid), Opcode,fValue);
    }

    return true;
}

bool ChatHandler::HandleGetValue(const char* args)
{
    if(!*args)
        return false;

    char* px = strtok((char*)args, " ");
    char* pz = strtok(NULL, " ");

    if (!px)
        return false;

    Unit* target = getSelectedUnit();
    if(!target)
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    uint64 guid = target->GetGUID();

    uint32 Opcode = (uint32)atoi(px);
    if(Opcode >= target->GetValuesCount())
    {
        PSendSysMessage(LANG_TOO_BIG_INDEX, Opcode, GUID_LOPART(guid), target->GetValuesCount());
        return false;
    }
    uint32 iValue;
    float fValue;
    bool isint32 = true;
    if(pz)
        isint32 = (bool)atoi(pz);

    if(isint32)
    {
        iValue = target->GetUInt32Value( Opcode );
        sLog.outDebug(GetTrinityString(LANG_GET_UINT), GUID_LOPART(guid), Opcode, iValue);
        PSendSysMessage(LANG_GET_UINT_FIELD, GUID_LOPART(guid), Opcode,    iValue);
    }
    else
    {
        fValue = target->GetFloatValue( Opcode );
        sLog.outDebug(GetTrinityString(LANG_GET_FLOAT), GUID_LOPART(guid), Opcode, fValue);
        PSendSysMessage(LANG_GET_FLOAT_FIELD, GUID_LOPART(guid), Opcode, fValue);
    }

    return true;
}

bool ChatHandler::HandleSet32Bit(const char* args)
{
    if(!*args)
        return false;

    char* px = strtok((char*)args, " ");
    char* py = strtok(NULL, " ");

    if (!px || !py)
        return false;

    uint32 Opcode = (uint32)atoi(px);
    uint32 Value = (uint32)atoi(py);
    if (Value > 32)                                         //uint32 = 32 bits
        return false;

    sLog.outDebug(GetTrinityString(LANG_SET_32BIT), Opcode, Value);

    m_session->GetPlayer( )->SetUInt32Value( Opcode , 2^Value );

    PSendSysMessage(LANG_SET_32BIT_FIELD, Opcode,1);
    return true;
}

bool ChatHandler::HandleMod32Value(const char* args)
{
    if(!*args)
        return false;

    char* px = strtok((char*)args, " ");
    char* py = strtok(NULL, " ");

    if (!px || !py)
        return false;

    uint32 Opcode = (uint32)atoi(px);
    int Value = atoi(py);

    if(Opcode >= m_session->GetPlayer()->GetValuesCount())
    {
        PSendSysMessage(LANG_TOO_BIG_INDEX, Opcode, m_session->GetPlayer()->GetGUIDLow(), m_session->GetPlayer( )->GetValuesCount());
        return false;
    }

    sLog.outDebug(GetTrinityString(LANG_CHANGE_32BIT), Opcode, Value);

    int CurrentValue = (int)m_session->GetPlayer( )->GetUInt32Value( Opcode );

    CurrentValue += Value;
    m_session->GetPlayer( )->SetUInt32Value( Opcode , (uint32)CurrentValue );

    PSendSysMessage(LANG_CHANGE_32BIT_FIELD, Opcode,CurrentValue);

    return true;
}

bool ChatHandler::HandleAddTeleCommand(const char * args)
{
    if(!*args)
        return false;

    Player *player=m_session->GetPlayer();
    if (!player)
        return false;

    std::string name = args;

    if(objmgr.GetGameTele(name))
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

    if(objmgr.AddGameTele(tele))
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
    if(!*args)
        return false;

    std::string name = args;

    if(!objmgr.DeleteGameTele(name))
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
            itr->second->GetSpellProto()->SpellName[m_session->GetSessionDbcLocale()],
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
                (*itr)->GetSpellProto()->SpellName[m_session->GetSessionDbcLocale()],((*itr)->IsPassive() ? passiveStr : ""),(talent ? talentStr : ""),
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

        uint64 guid = objmgr.GetPlayerGUIDByName(name.c_str());
        player = objmgr.GetPlayer(guid);
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
    PlayerInfo const *info = objmgr.GetPlayerInfo(player->getRace(), player->getClass());
    if(!info) return false;

    ChrClassesEntry const* cEntry = sChrClassesStore.LookupEntry(player->getClass());
    if(!cEntry)
    {
        sLog.outError("Class %u not found in DBC (Wrong DBC files?)",player->getClass());
        return false;
    }

    uint8 powertype = cEntry->powerType;

    uint32 unitfield;
    if(powertype == POWER_RAGE)
        unitfield = 0x1100EE00;
    else if(powertype == POWER_ENERGY)
        unitfield = 0x00000000;
    else if(powertype == POWER_MANA)
        unitfield = 0x0000EE00;
    else
    {
        sLog.outError("Invalid default powertype %u for player (class %u)",powertype,player->getClass());
        return false;
    }

    // reset m_form if no aura
    if(!player->HasAuraType(SPELL_AURA_MOD_SHAPESHIFT))
        player->m_form = FORM_NONE;

    player->SetFloatValue(UNIT_FIELD_BOUNDINGRADIUS, DEFAULT_WORLD_OBJECT_SIZE );
    player->SetFloatValue(UNIT_FIELD_COMBATREACH, DEFAULT_COMBAT_REACH );

    player->setFactionForRace(player->getRace());

    player->SetUInt32Value(UNIT_FIELD_BYTES_0, ( ( player->getRace() ) | ( player->getClass() << 8 ) | ( player->getGender() << 16 ) | ( powertype << 24 ) ) );

    // reset only if player not in some form;
    if(player->m_form==FORM_NONE)
    {
        switch(player->getGender())
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

        uint64 guid = objmgr.GetPlayerGUIDByName(name.c_str());
        player = objmgr.GetPlayer(guid);
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

        uint64 guid = objmgr.GetPlayerGUIDByName(name.c_str());
        player = objmgr.GetPlayer(guid);
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

        player = objmgr.GetPlayer(name.c_str());
        if(!player)
            playerGUID = objmgr.GetPlayerGUIDByName(name.c_str());
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
            PSendSysMessage(LANG_RESET_SPELLS_ONLINE,player->GetName());
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

        player = objmgr.GetPlayer(name.c_str());
        if(!player)
            playerGUID = objmgr.GetPlayerGUIDByName(name.c_str());
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
            PSendSysMessage(LANG_RESET_TALENTS_ONLINE,player->GetName());
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
    if(!*args)
        return false;

    std::string casename = args;

    AtLoginFlags atLogin;

    // Command specially created as single command to prevent using short case names
    if(casename=="spells")
    {
        atLogin = AT_LOGIN_RESET_SPELLS;
        sWorld.SendWorldText(LANG_RESETALL_SPELLS);
    }
    else if(casename=="talents")
    {
        atLogin = AT_LOGIN_RESET_TALENTS;
        sWorld.SendWorldText(LANG_RESETALL_TALENTS);
    }
    else
    {
        PSendSysMessage(LANG_RESETALL_UNKNOWN_CASE,args);
        SetSentErrorMessage(true);
        return false;
    }

    CharacterDatabase.PExecute("UPDATE characters SET at_login = at_login | '%u' WHERE (at_login & '%u') = '0'",atLogin,atLogin);
    HashMapHolder<Player>::MapType const& plist = ObjectAccessor::Instance().GetPlayers();
    for(HashMapHolder<Player>::MapType::const_iterator itr = plist.begin(); itr != plist.end(); ++itr)
        itr->second->SetAtLoginFlag(atLogin);

    return true;
}

bool ChatHandler::HandleServerShutDownCancelCommand(const char* args)
{
    sWorld.ShutdownCancel();
    return true;
}

bool ChatHandler::HandleServerShutDownCommand(const char* args)
{
    if(!*args)
        return false;

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

        sWorld.ShutdownServ (time, 0, exitcode);*/
        
        sWorld.ShutdownServ (time, 0, reason);
    }
    else
        sWorld.ShutdownServ(time,0,"");
    return true;
}

bool ChatHandler::HandleServerRestartCommand(const char* args)
{
    if(!*args)
        return false;

    char* time_str = strtok ((char*) args, " ");
    char* reason = strtok (NULL, "");

    int32 time = atoi (time_str);

    ///- Prevent interpret wrong arg value as 0 secs shutdown time
    if(time == 0 && (time_str[0]!='0' || time_str[1]!='\0') || time < 0)
        return false;

    if (reason)
        sWorld.ShutdownServ (time, SHUTDOWN_MASK_RESTART, reason);
    else
        sWorld.ShutdownServ(time, SHUTDOWN_MASK_RESTART, "");
        
    return true;
}

bool ChatHandler::HandleServerIdleRestartCommand(const char* args)
{
    if(!*args)
        return false;

    char* time_str = strtok ((char*) args, " ");
    char* reason = strtok (NULL, "");

    int32 time = atoi (time_str);

    ///- Prevent interpret wrong arg value as 0 secs shutdown time
    if(time == 0 && (time_str[0]!='0' || time_str[1]!='\0') || time < 0)
        return false;

    if (reason)
        sWorld.ShutdownServ (time, SHUTDOWN_MASK_RESTART, reason);
    else
        sWorld.ShutdownServ(time,SHUTDOWN_MASK_RESTART|SHUTDOWN_MASK_IDLE,"");
        
    return true;
}

bool ChatHandler::HandleServerIdleShutDownCommand(const char* args)
{
    if(!*args)
        return false;

    char* time_str = strtok ((char*) args, " ");
    char* reason = strtok (NULL, "");

    int32 time = atoi (time_str);

    ///- Prevent interpret wrong arg value as 0 secs shutdown time
    if(time == 0 && (time_str[0]!='0' || time_str[1]!='\0') || time < 0)
        return false;

    if (reason)
        sWorld.ShutdownServ (time, SHUTDOWN_MASK_IDLE, reason);
    else
        sWorld.ShutdownServ(time,SHUTDOWN_MASK_IDLE,"");
        
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

    Quest const* pQuest = objmgr.GetQuestTemplate(entry);

    if(!pQuest)
    {
        PSendSysMessage(LANG_COMMAND_QUEST_NOTFOUND,entry);
        SetSentErrorMessage(true);
        return false;
    }

    // check item starting quest (it can work incorrectly if added without item in inventory)
    for (uint32 id = 0; id < sItemStorage.MaxEntry; id++)
    {
        ItemPrototype const *pProto = sItemStorage.LookupEntry<ItemPrototype>(id);
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

    Quest const* pQuest = objmgr.GetQuestTemplate(entry);

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
        //sLog.outError("DEBUG: ID value: %d", tEntry);
        if(!entry)
            return false;
            
        forceComplete = true;
    }
    else
        entry = atol(cId);

    Quest const* pQuest = objmgr.GetQuestTemplate(entry);

    // If player doesn't have the quest
    if(!pQuest || player->GetQuestStatus(entry) == QUEST_STATUS_NONE)
    {
        PSendSysMessage(LANG_COMMAND_QUEST_NOTFOUND, entry);
        SetSentErrorMessage(true);
        return false;
    }
    
    QueryResult* result = CharacterDatabase.PQuery("SELECT count FROM completed_quests WHERE guid = %u", player->GetGUID());
    
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
    if (completedQuestsThisWeek >= 2 && !forceComplete) //TODO: set a config option here ?
    {
        //tell the GM that this player has reached the maximum quests complete for this week
        PSendSysMessage(LANG_REACHED_QCOMPLETE_LIMIT, player->GetName());
        SetSentErrorMessage(true);
        return true;
    }
    // Add quest items for quests that require items
    for(uint8 x = 0; x < QUEST_OBJECTIVES_COUNT; ++x)
    {
        uint32 id = pQuest->ReqItemId[x];
        uint32 count = pQuest->ReqItemCount[x];
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
        uint32 creature = pQuest->ReqCreatureOrGOId[i];
        uint32 creaturecount = pQuest->ReqCreatureOrGOCount[i];

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
    PSendSysMessage(LANG_QCOMPLETE_SUCCESS, entry, player->GetName()); //tell GM that the quest has been successfully completed
    
    if (completedQuestsThisWeek == 0) //entry does not exist, we have to create it
    {
        CharacterDatabase.PExecute("INSERT INTO completed_quests VALUES(%u, 1)", player->GetGUID());
    }
    else //entry exists, we have just to update it
    {
        CharacterDatabase.PExecute("UPDATE completed_quests SET count = count + 1 WHERE guid = %u", player->GetGUID());
    }
    
    // Check if quest already exists in bugged quests
    QueryResult* questbug = WorldDatabase.PQuery("SELECT bugged FROM quest_bugs WHERE entry = %u", entry);
    if (questbug)
        WorldDatabase.PExecute("UPDATE quest_bugs SET completecount = completecount+1 WHERE entry = %u", entry);
    else
        WorldDatabase.PExecute("INSERT INTO quest_bugs VALUES (%u, 1, 1, '')", entry);
    
    return true;
}

bool ChatHandler::HandleReportQuest(const char* args)
{
    if (!args)
        return false;
        
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

        player = objmgr.GetPlayer(name.c_str());
        if (player)
        {
            targetGUID = player->GetGUID();
        }
        else //player is not online, get GUID with another function
        {
            targetGUID = objmgr.GetPlayerGUIDByName(name);
            if (!targetGUID) //player doesn't exist
            {
                SendSysMessage(LANG_PLAYER_NOT_FOUND);
                SetSentErrorMessage(true);
                return false;
            }
        }
    }
    
    QueryResult* result = CharacterDatabase.PQuery("SELECT count FROM completed_quests WHERE guid = %u", targetGUID);
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
    objmgr.GetPlayerNameByGUID(targetGUID, displayName);
    PSendSysMessage(LANG_QCOMPLETE_THIS_WEEK, displayName.c_str(), completedQuestsThisWeek);
    return true;
}

//shows the total number of quests completed by all gamemasters this week
bool ChatHandler::HandleTotalCount(const char* args)
{
    QueryResult* result = CharacterDatabase.PQuery("SELECT SUM(count) FROM completed_quests");
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
    if(!args)
        return false;

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

    switch(sWorld.BanAccount(mode, nameOrIP, duration, reason,m_session ? m_session->GetPlayerName() : ""))
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
    if(!args)
        return false;

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

    if(sWorld.RemoveBanAccount(mode,nameOrIP))
        PSendSysMessage(LANG_UNBAN_UNBANNED,nameOrIP.c_str());
    else
        PSendSysMessage(LANG_UNBAN_ERROR,nameOrIP.c_str());

    return true;
}

bool ChatHandler::HandleBanInfoAccountCommand(const char* args)
{
    if(!args)
        return false;

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

    uint32 accountid = sAccountMgr.GetId(account_name);
    if(!accountid)
    {
        PSendSysMessage(LANG_ACCOUNT_NOT_EXIST,account_name.c_str());
        return true;
    }

    return HandleBanInfoHelper(accountid,account_name.c_str());
}

bool ChatHandler::HandleBanInfoCharacterCommand(const char* args)
{
    if(!args)
        return false;

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

    uint32 accountid = objmgr.GetPlayerAccountIdByPlayerName(name);
    if(!accountid)
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    std::string accountname;
    if(!sAccountMgr.GetName(accountid,accountname))
    {
        PSendSysMessage(LANG_BANINFO_NOCHARACTER);
        return true;
    }

    return HandleBanInfoHelper(accountid,accountname.c_str());
}

bool ChatHandler::HandleBanInfoHelper(uint32 accountid, char const* accountname)
{
    QueryResult *result = LoginDatabase.PQuery("SELECT FROM_UNIXTIME(bandate), unbandate-bandate, active, unbandate,banreason,bannedby FROM account_banned WHERE id = '%u' ORDER BY bandate ASC",accountid);
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
            fields[0].GetString(), bantime.c_str(), active ? GetTrinityString(LANG_BANINFO_YES):GetTrinityString(LANG_BANINFO_NO), fields[4].GetString(), fields[5].GetString());
    }while (result->NextRow());

    delete result;
    return true;
}

bool ChatHandler::HandleBanInfoIPCommand(const char* args)
{
    if(!args)
        return false;

    char* cIP = strtok ((char*)args, "");
    if(!cIP)
        return false;

    if (!IsIPAddress(cIP))
        return false;

    std::string IP = cIP;

    LoginDatabase.escape_string(IP);
    QueryResult *result = LoginDatabase.PQuery("SELECT ip, FROM_UNIXTIME(bandate), FROM_UNIXTIME(unbandate), unbandate-UNIX_TIMESTAMP(), banreason,bannedby,unbandate-bandate FROM ip_banned WHERE ip = '%s'",IP.c_str());
    if(!result)
    {
        PSendSysMessage(LANG_BANINFO_NOIP);
        return true;
    }

    Field *fields = result->Fetch();
    bool permanent = !fields[6].GetUInt64();
    PSendSysMessage(LANG_BANINFO_IPENTRY,
        fields[0].GetString(), fields[1].GetString(), permanent ? GetTrinityString(LANG_BANINFO_NEVER):fields[2].GetString(),
        permanent ? GetTrinityString(LANG_BANINFO_INFINITE):secsToTimeString(fields[3].GetUInt64(), true).c_str(), fields[4].GetString(), fields[5].GetString());
    delete result;
    return true;
}

bool ChatHandler::HandleMuteInfoAccountCommand(const char* args)
{
    if (!args)
        return false;
        
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

    uint32 accountid = sAccountMgr.GetId(account_name);
    if(!accountid)
    {
        PSendSysMessage(LANG_ACCOUNT_NOT_EXIST,account_name.c_str());
        return true;
    }
    
    QueryResult* result = LogsDatabase.PQuery("SELECT duration, reason, author, FROM_UNIXTIME(time), time FROM sanctions WHERE acctid = %d AND type = %u", accountid, uint32(SANCTION_MUTE));
    if (!result) {
        PSendSysMessage("Aucune sanction pour ce compte.");
        return true;
    }
    
    do {
        Field* fields = result->Fetch();
        uint32 authorGUID = fields[2].GetUInt32();
        uint32 duration = fields[0].GetUInt32();
        std::string unbanstr = fields[3].GetCppString();
        uint64 unbantimestamp = fields[4].GetUInt64() + (duration * 60);
        std::string authorname;
        if (!objmgr.GetPlayerNameByLowGUID(authorGUID, authorname))
            authorname = "<Inconnu>";
        PSendSysMessage("Account %d: Mute %s pour \"%s\" par %s (%s).", accountid, secsToTimeString(fields[0].GetUInt32()).c_str(), fields[1].GetCppString().c_str(), authorname.c_str(), unbanstr.c_str(), (unbantimestamp > uint64(time(NULL))) ? " (actif)" : "");
    } while (result->NextRow());
    
    delete result;
    return true;
}

bool ChatHandler::HandleMuteInfoCharacterCommand(char const* args)
{
    if(!args)
        return false;

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

    uint32 accountid = objmgr.GetPlayerAccountIdByPlayerName(name);
    if(!accountid)
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    std::string accountname;
    if(!sAccountMgr.GetName(accountid,accountname))
    {
        PSendSysMessage(LANG_BANINFO_NOCHARACTER);
        return true;
    }
    
    QueryResult* result = LogsDatabase.PQuery("SELECT duration, reason, author, FROM_UNIXTIME(time), time FROM sanctions WHERE acctid = %d AND type = 0", accountid);
    if (!result) {
        PSendSysMessage("Aucune sanction pour le compte de ce personnage.");
        return true;
    }
    
    do {
        Field* fields = result->Fetch();
        uint32 authorGUID = fields[2].GetUInt32();
        uint32 duration = fields[0].GetUInt32();
        std::string unbanstr = fields[3].GetCppString();
        uint64 unbantimestamp = fields[4].GetUInt64() + (duration * 60);
        std::string authorname;
        if (!objmgr.GetPlayerNameByLowGUID(authorGUID, authorname))
            authorname = "<Inconnu>";
        PSendSysMessage("Account %d: Mute %s pour \"%s\" par %s (%s).", accountid, secsToTimeString(fields[0].GetUInt32()).c_str(), fields[1].GetCppString().c_str(), authorname.c_str(), unbanstr.c_str(), (unbantimestamp > uint64(time(NULL))) ? " (actif)" : "");
    } while (result->NextRow());
    
    delete result;
    return true;
}

bool ChatHandler::HandleBanListCharacterCommand(const char* args)
{
    LoginDatabase.Execute("DELETE FROM ip_banned WHERE unbandate<=UNIX_TIMESTAMP() AND unbandate<>bandate");

    char* cFilter = strtok ((char*)args, " ");
    if(!cFilter)
        return false;

    std::string filter = cFilter;
    LoginDatabase.escape_string(filter);
    QueryResult* result = CharacterDatabase.PQuery("SELECT account FROM characters WHERE name "_LIKE_" "_CONCAT3_("'%%'","'%s'","'%%'"),filter.c_str());
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
    LoginDatabase.escape_string(filter);

    QueryResult* result;

    if(filter.empty())
    {
        result = LoginDatabase.Query("SELECT account.id, username FROM account, account_banned"
            " WHERE account.id = account_banned.id AND active = 1 GROUP BY account.id");
    }
    else
    {
        result = LoginDatabase.PQuery("SELECT account.id, username FROM account, account_banned"
            " WHERE account.id = account_banned.id AND active = 1 AND username "_LIKE_" "_CONCAT3_("'%%'","'%s'","'%%'")" GROUP BY account.id",
            filter.c_str());
    }

    if (!result)
    {
        PSendSysMessage(LANG_BANLIST_NOACCOUNT);
        return true;
    }

    return HandleBanListHelper(result);
}

bool ChatHandler::HandleBanListHelper(QueryResult* result)
{
    PSendSysMessage(LANG_BANLIST_MATCHINGACCOUNT);

    // Chat short output
    if(m_session)
    {
        do
        {
            Field* fields = result->Fetch();
            uint32 accountid = fields[0].GetUInt32();

            QueryResult* banresult = LoginDatabase.PQuery("SELECT account.username FROM account,account_banned WHERE account_banned.id='%u' AND account_banned.id=account.id",accountid);
            if(banresult)
            {
                Field* fields2 = banresult->Fetch();
                PSendSysMessage("%s",fields2[0].GetString());
                delete banresult;
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
                account_name = fields[1].GetCppString();
            // "character" case, name need extract from another DB
            else
                sAccountMgr.GetName (account_id,account_name);

            // No SQL injection. id is uint32.
            QueryResult *banInfo = LoginDatabase.PQuery("SELECT bandate,unbandate,bannedby,banreason FROM account_banned WHERE id = %u ORDER BY unbandate", account_id);
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
                            fields2[2].GetString(),fields2[3].GetString());
                    }
                    else
                    {
                        time_t t_unban = fields2[1].GetUInt64();
                        tm* aTm_unban = localtime(&t_unban);
                        PSendSysMessage("|%-15.15s|%02d-%02d-%02d %02d:%02d|%02d-%02d-%02d %02d:%02d|%-15.15s|%-15.15s|",
                            account_name.c_str(),aTm_ban->tm_year%100, aTm_ban->tm_mon+1, aTm_ban->tm_mday, aTm_ban->tm_hour, aTm_ban->tm_min,
                            aTm_unban->tm_year%100, aTm_unban->tm_mon+1, aTm_unban->tm_mday, aTm_unban->tm_hour, aTm_unban->tm_min,
                            fields2[2].GetString(),fields2[3].GetString());
                    }
                }while ( banInfo->NextRow() );
                delete banInfo;
            }
        }while( result->NextRow() );
        SendSysMessage("===============================================================================");
    }

    delete result;
    return true;
}

bool ChatHandler::HandleBanListIPCommand(const char* args)
{
    LoginDatabase.Execute("DELETE FROM ip_banned WHERE unbandate<=UNIX_TIMESTAMP() AND unbandate<>bandate");

    char* cFilter = strtok((char*)args, " ");
    std::string filter = cFilter ? cFilter : "";
    LoginDatabase.escape_string(filter);

    QueryResult* result;

    if(filter.empty())
    {
        result = LoginDatabase.Query ("SELECT ip,bandate,unbandate,bannedby,banreason FROM ip_banned"
            " WHERE (bandate=unbandate OR unbandate>UNIX_TIMESTAMP())"
            " ORDER BY unbandate" );
    }
    else
    {
        result = LoginDatabase.PQuery( "SELECT ip,bandate,unbandate,bannedby,banreason FROM ip_banned"
            " WHERE (bandate=unbandate OR unbandate>UNIX_TIMESTAMP()) AND ip "_LIKE_" "_CONCAT3_("'%%'","'%s'","'%%'")
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
            PSendSysMessage("%s",fields[0].GetString());
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
                    fields[0].GetString(), aTm_ban->tm_year%100, aTm_ban->tm_mon+1, aTm_ban->tm_mday, aTm_ban->tm_hour, aTm_ban->tm_min,
                    fields[3].GetString(), fields[4].GetString());
            }
            else
            {
                time_t t_unban = fields[2].GetUInt64();
                tm* aTm_unban = localtime(&t_unban);
                PSendSysMessage("|%-15.15s|%02d-%02d-%02d %02d:%02d|%02d-%02d-%02d %02d:%02d|%-15.15s|%-15.15s|",
                    fields[0].GetString(), aTm_ban->tm_year%100, aTm_ban->tm_mon+1, aTm_ban->tm_mday, aTm_ban->tm_hour, aTm_ban->tm_min,
                    aTm_unban->tm_year%100, aTm_unban->tm_mon+1, aTm_unban->tm_mday, aTm_unban->tm_hour, aTm_unban->tm_min,
                    fields[3].GetString(), fields[4].GetString());
            }
        }while( result->NextRow() );
        SendSysMessage("===============================================================================");
    }

    delete result;
    return true;
}

bool ChatHandler::HandleRespawnCommand(const char* /*args*/)
{
    Player* pl = m_session->GetPlayer();

    // accept only explicitly selected target (not implicitly self targeting case)
    Unit* target = getSelectedUnit();
    if(pl->GetSelection() && target)
    {
        if(target->GetTypeId()!=TYPEID_UNIT)
        {
            SendSysMessage(LANG_SELECT_CREATURE);
            SetSentErrorMessage(true);
            return false;
        }

        if(target->isDead())
            (target->ToCreature())->Respawn();
        return true;
    }

    CellPair p(Trinity::ComputeCellPair(pl->GetPositionX(), pl->GetPositionY()));
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
    if(!args)
        return false;

    Unit *unit = getSelectedUnit();
    if (!unit || (unit->GetTypeId() != TYPEID_PLAYER))
        unit = m_session->GetPlayer();

    WorldPacket data(12);
    if (strncmp(args, "on", 3) == 0)
    {
        data.SetOpcode(SMSG_MOVE_SET_CAN_FLY);
        ((Player*)(unit))->SetCanFly(true);
    }
    else if (strncmp(args, "off", 4) == 0)
    {
        data.SetOpcode(SMSG_MOVE_UNSET_CAN_FLY);
        ((Player*)(unit))->SetCanFly(false);
    }
    else
    {
        SendSysMessage(LANG_USE_BOL);
        return false;
    }
    data.append(unit->GetPackGUID());
    data << uint32(0);                                      // unknown
    unit->SendMessageToSet(&data, true);
    PSendSysMessage(LANG_COMMAND_FLYMODE_STATUS, unit->GetName(), args);
    return true;
}

bool ChatHandler::HandleLoadPDumpCommand(const char *args)
{
    if(!args)
        return false;

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

    uint32 account_id = sAccountMgr.GetId(account_name);
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

    if(!sAccountMgr.GetName(account_id,account_name))
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

        if(!ObjectMgr::IsValidName(name,true))
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

        if(objmgr.GetPlayerAccountIdByGUID(guid))
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
    if(!args)
        return false;

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
    if(!args)
        return false;

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

        guid = objmgr.GetPlayerGUIDByName(name);
    }

    if(!objmgr.GetPlayerAccountIdByGUID(guid))
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

    MotionMaster* mm = unit->GetMotionMaster();
    for(int i = 0; i < MAX_MOTION_SLOT; ++i)
    {
        MovementGenerator* mg = mm->GetMotionSlot(i);
        if(!mg)
        {
            SendSysMessage("Empty");
            continue;
        }
        switch(mg->GetMovementGeneratorType())
        {
            case IDLE_MOTION_TYPE:          SendSysMessage(LANG_MOVEGENS_IDLE);          break;
            case RANDOM_MOTION_TYPE:        SendSysMessage(LANG_MOVEGENS_RANDOM);        break;
            case WAYPOINT_MOTION_TYPE:      SendSysMessage(LANG_MOVEGENS_WAYPOINT);      break;
            case ANIMAL_RANDOM_MOTION_TYPE: SendSysMessage(LANG_MOVEGENS_ANIMAL_RANDOM); break;
            case CONFUSED_MOTION_TYPE:      SendSysMessage(LANG_MOVEGENS_CONFUSED);      break;
            case TARGETED_MOTION_TYPE:
            {
                if(unit->GetTypeId()==TYPEID_PLAYER)
                {
                    TargetedMovementGenerator<Player> const* mgen = static_cast<TargetedMovementGenerator<Player> const*>(mg);
                    Unit* target = mgen->GetTarget();
                    if(target)
                        PSendSysMessage(LANG_MOVEGENS_TARGETED_PLAYER,target->GetName(),target->GetGUIDLow());
                    else
                        SendSysMessage(LANG_MOVEGENS_TARGETED_NULL);
                }
                else
                {
                    TargetedMovementGenerator<Creature> const* mgen = static_cast<TargetedMovementGenerator<Creature> const*>(mg);
                    Unit* target = mgen->GetTarget();
                    if(target)
                        PSendSysMessage(LANG_MOVEGENS_TARGETED_CREATURE,target->GetName(),target->GetGUIDLow());
                    else
                        SendSysMessage(LANG_MOVEGENS_TARGETED_NULL);
                }
                break;
            }
            case HOME_MOTION_TYPE:
                if(unit->GetTypeId()==TYPEID_UNIT)
                {
                    float x,y,z;
                    mg->GetDestination(x,y,z);
                    PSendSysMessage(LANG_MOVEGENS_HOME_CREATURE,x,y,z);
                }
                else
                    SendSysMessage(LANG_MOVEGENS_HOME_PLAYER);
                break;
            case FLIGHT_MOTION_TYPE:   SendSysMessage(LANG_MOVEGENS_FLIGHT);  break;
            case POINT_MOTION_TYPE:
            {
                float x,y,z;
                mg->GetDestination(x,y,z);
                PSendSysMessage(LANG_MOVEGENS_POINT,x,y,z);
                break;
            }
            case FLEEING_MOTION_TYPE:  SendSysMessage(LANG_MOVEGENS_FEAR);    break;
            case DISTRACT_MOTION_TYPE: SendSysMessage(LANG_MOVEGENS_DISTRACT);  break;
            default:
                PSendSysMessage(LANG_MOVEGENS_UNKNOWN,mg->GetMovementGeneratorType());
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
            sWorld.SetPlayerLimit(-SEC_PLAYER);
        else if(strncmp(param,"moderator",l) == 0 )
            sWorld.SetPlayerLimit(-SEC_MODERATOR);
        else if(strncmp(param,"gamemaster",l) == 0 )
            sWorld.SetPlayerLimit(-SEC_GAMEMASTER);
        else if(strncmp(param,"administrator",l) == 0 )
            sWorld.SetPlayerLimit(-SEC_ADMINISTRATOR);
        else if(strncmp(param,"reset",l) == 0 )
            sWorld.SetPlayerLimit( sConfig.GetIntDefault("PlayerLimit", DEFAULT_PLAYER_LIMIT) );
        else
        {
            int val = atoi(param);
            if(val < -SEC_ADMINISTRATOR) val = -SEC_ADMINISTRATOR;

            sWorld.SetPlayerLimit(val);
        }

        // kick all low security level players
        if(sWorld.GetPlayerAmountLimit() > SEC_PLAYER)
            sWorld.KickAllLess(sWorld.GetPlayerSecurityLimit());
    }

    uint32 pLimit = sWorld.GetPlayerAmountLimit();
    AccountTypes allowedAccountType = sWorld.GetPlayerSecurityLimit();
    char const* secName = "";
    switch(allowedAccountType)
    {
        case SEC_PLAYER:        secName = "Player";        break;
        case SEC_MODERATOR:     secName = "Moderator";     break;
        case SEC_GAMEMASTER:    secName = "Gamemaster";    break;
        case SEC_ADMINISTRATOR: secName = "Administrator"; break;
        default:                secName = "<unknown>";     break;
    }

    PSendSysMessage("Player limits: amount %u, min. security level %s.",pLimit,secName);

    return true;
}

bool ChatHandler::HandleCastCommand(const char* args)
{
    if(!*args)
        return false;

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

    SpellEntry const* spellInfo = spellmgr.LookupSpell(spell);
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

bool ChatHandler::HandleCastBackCommand(const char* args)
{
    Creature* caster = getSelectedCreature();

    if(!caster)
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r
    // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
    uint32 spell = extractSpellIdFromLink((char*)args);
    if(!spell || !spellmgr.LookupSpell(spell))
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
    if(!*args)
        return false;

    // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
    uint32 spell = extractSpellIdFromLink((char*)args);
    if(!spell)
        return false;

    SpellEntry const* spellInfo = spellmgr.LookupSpell(spell);
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

    if(!caster->getVictim())
    {
        SendSysMessage(LANG_SELECTED_TARGET_NOT_HAVE_VICTIM);
        SetSentErrorMessage(true);
        return false;
    }

    // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
    uint32 spell = extractSpellIdFromLink((char*)args);
    if(!spell || !spellmgr.LookupSpell(spell))
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

    caster->CastSpell(caster->getVictim(),spell,triggered);

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
    if(!*args)
        return false;

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

    SpellEntry const* spellInfo = spellmgr.LookupSpell(spell);
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
    if(!*args)
        return false;

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
    PSendSysMessage("instances loaded: %d", MapManager::Instance().GetNumInstances());
    PSendSysMessage("players in instances: %d", MapManager::Instance().GetNumPlayersInInstances());
    PSendSysMessage("instance saves: %d", sInstanceSaveManager.GetNumInstanceSaves());
    PSendSysMessage("players bound: %d", sInstanceSaveManager.GetNumBoundPlayersTotal());
    PSendSysMessage("groups bound: %d", sInstanceSaveManager.GetNumBoundGroupsTotal());
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
    QueryResult *result = LoginDatabase.Query( "SELECT username,gmlevel FROM account WHERE gmlevel > 0" );
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
            PSendSysMessage("|%15s|%6s|", fields[0].GetString(),fields[1].GetString());
        }while( result->NextRow() );

        PSendSysMessage("========================");
        delete result;
    }
    else
        PSendSysMessage(LANG_GMLIST_EMPTY);
    return true;
}

/// Define the 'Message of the day' for the realm
bool ChatHandler::HandleServerSetMotdCommand(const char* args)
{
    sWorld.SetMotd(args);
    PSendSysMessage(LANG_MOTD_NEW, args);
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
        sAccountMgr.GetName(account_id,account_name);
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

        account_id = sAccountMgr.GetId(account_name);
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
    if(!*args)
        return false;

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

        ItemPrototype const* item_proto = objmgr.GetItemPrototype(item_id);
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

    uint64 receiver_guid = objmgr.GetPlayerGUIDByName(name);
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
    uint32 itemTextId = !text.empty() ? objmgr.CreateItemText( text ) : 0;

    Player *receiver = objmgr.GetPlayer(receiver_guid);

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
    if (!*args)
        return false;

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

    uint64 receiver_guid = objmgr.GetPlayerGUIDByName(name);
    if (!receiver_guid)
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    uint32 mailId = objmgr.GenerateMailID();

    // from console show not existed sender
    uint32 sender_guidlo = m_session ? m_session->GetPlayer()->GetGUIDLow() : 0;

    uint32 messagetype = MAIL_NORMAL;
    uint32 stationery = MAIL_STATIONERY_GM;
    uint32 itemTextId = !text.empty() ? objmgr.CreateItemText( text ) : 0;

    Player *receiver = objmgr.GetPlayer(receiver_guid);

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
    Player *rPlayer = objmgr.GetPlayer(name.c_str());
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
    sBattleGroundMgr.DistributeArenaPoints();
    return true;
}

bool ChatHandler::HandleModifyGenderCommand(const char *args)
{
    if(!*args)
        return false;

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
        if(player->getGender() == GENDER_MALE)
            return true;

        gender_full = "male";
        new_displayId = player->getRace() == RACE_BLOODELF ? displayId+1 : displayId-1;
        gender = GENDER_MALE;
    }
    else if (!strncmp(gender_str,"female",gender_len))      // FEMALE
    {
        if(player->getGender() == GENDER_FEMALE)
            return true;

        gender_full = "female";
        new_displayId = player->getRace() == RACE_BLOODELF ? displayId-1 : displayId+1;
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

    PSendSysMessage(LANG_YOU_CHANGE_GENDER, player->GetName(),gender_full);
    if (needReportToTarget(player))
        ChatHandler(player).PSendSysMessage(LANG_YOUR_GENDER_CHANGED, gender_full,GetName());
    return true;
}

/*------------------------------------------
 *-------------TRINITY----------------------
 *-------------------------------------*/

bool ChatHandler::HandlePlayAllCommand(const char* args)
{
    if(!*args)
        return false;

    uint32 soundId = atoi((char*)args);

    if(!sSoundEntriesStore.LookupEntry(soundId))
    {
        PSendSysMessage(LANG_SOUND_NOT_EXIST, soundId);
        SetSentErrorMessage(true);
        return false;
    }

    WorldPacket data(SMSG_PLAY_SOUND, 4);
    data << uint32(soundId) << m_session->GetPlayer()->GetGUID();
    sWorld.SendGlobalMessage(&data);

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
        player = objmgr.GetPlayer(name.c_str()); //get player by name
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
        player->setFaction(35);
        player->CombatStop();
        if(player->IsNonMeleeSpellCasted(true))
            player->InterruptNonMeleeSpells(true);
        player->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        //player->SetUInt32Value(PLAYER_DUEL_TEAM, 1);

        //if player class = hunter || warlock remove pet if alive
        if((player->getClass() == CLASS_HUNTER) || (player->getClass() == CLASS_WARLOCK))
        {
            if(Pet* pet = player->GetPet())
            {
                pet->SavePetToDB(PET_SAVE_AS_CURRENT);
                // not let dismiss dead pet
                if(pet && pet->isAlive())
                    player->RemovePet(pet,PET_SAVE_NOT_IN_SLOT);
            }
        }

        //stop movement and disable spells
        uint32 spellID = 9454;
        //m_session->GetPlayer()->CastSpell(player,spellID,false);
        SpellEntry const *spellInfo = spellmgr.LookupSpell( spellID );
        if(spellInfo) //TODO: Change the duration of the aura to -1 instead of 5000000
        {
            for(uint32 i = 0;i<3;i++)
            {
                uint8 eff = spellInfo->Effect[i];
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
        player = objmgr.GetPlayer(name.c_str()); //get player by name
    }

    //effect
    if (player)
    {
        PSendSysMessage(LANG_COMMAND_UNFREEZE,name.c_str());

        //Reset player faction + allow combat + allow duels
        player->setFactionForRace(player->getRace());
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
            QueryResult *result = CharacterDatabase.PQuery("SELECT characters.guid FROM `characters` WHERE characters.name = '%s'",name.c_str());
            if(!result)
            {
                SendSysMessage(LANG_COMMAND_FREEZE_WRONG);
                return true;
            }
            //if player found: delete his freeze aura
            Field *fields=result->Fetch();
            uint64 pguid = fields[0].GetUInt64();
            delete result;
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
    QueryResult *result = CharacterDatabase.PQuery("SELECT characters.name FROM `characters` LEFT JOIN `character_aura` ON (characters.guid = character_aura.guid) WHERE character_aura.spell = 9454");
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
        std::string fplayers = fields[0].GetCppString();
        PSendSysMessage(LANG_COMMAND_FROZEN_PLAYERS,fplayers.c_str());
    } while (result->NextRow());

    delete result;
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

    pUnit->RemoveSpellsCausingAura(SPELL_AURA_MOD_CHARM);
    pUnit->RemoveSpellsCausingAura(SPELL_AURA_MOD_POSSESS_PET);
    pUnit->RemoveSpellsCausingAura(SPELL_AURA_MOD_POSSESS);

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

bool ChatHandler::HandleIRCJoinCommand(const char *args)
{
    if(!*args)
        return false;
    
    if(!sIRC.m_connected)
    {
        SendSysMessage("Can't perform command, not connected to an IRC server.");
        return true;
    }
    
    char* channel = strtok((char*)args, " \r");
    char* password = strtok(NULL, " \r");
    
    sIRC.DoJoin(channel, password);
    return true;
}

bool ChatHandler::HandleIRCPrivmsgCommand(const char *args)
{
    if(!*args)
        return false;
    
    if(!sIRC.m_connected)
    {
        SendSysMessage("Can't perform command, not connected to an IRC server.");
        return true;
    }
    
    char* target = strtok((char*)args, " \r");
    char* text = strtok(NULL, "\r");
    if(!text)
        return false;
    
    sIRC.DoPrivmsg(target, text);
    return true;
}

bool ChatHandler::HandleIRCNoticeCommand(const char *args)
{
    if(!*args)
        return false;
    
    if(!sIRC.m_connected)
    {
        SendSysMessage("Can't perform command, not connected to an IRC server.");
        return true;
    }
    
    char* target = strtok((char*)args, " \r");
    char* text = strtok(NULL, "\r");
    if(!text)
        return false;
    
    sIRC.DoNotice(target, text);
    return true;
}

bool ChatHandler::HandleIRCKickCommand(const char *args)
{
    if(!*args)
        return false;
    
    if(!sIRC.m_connected)
    {
        SendSysMessage("Can't perform command, not connected to an IRC server.");
        return true;
    }
    
    char* channel = strtok((char*)args, " ");
    char* user = strtok(NULL, " ");
    if(!user)
        return false;
        
    char* message = strtok(NULL, "\r");
    std::string msg;
    if(!message)
        msg = "Console kick.";
    else
        msg = message;    
        
    sIRC.DoKick(channel, user, msg.c_str());
    return true;
}

bool ChatHandler::HandleIRCQuitCommand(const char* args)
{
    if(!sIRC.m_connected)
    {
        SendSysMessage("Can't perform command, not connected to an IRC server.");
        return true;
    }
    
    char* message = strtok((char*)args, "\r\n");
    std::string msg;
    if(!message)
    {
        msg = "TrinityIRC DoQuit initiated by ";
        if(m_session)
        {
            msg.append(m_session->GetPlayer()->GetName());
        }
        else
            msg.append("console.");    
    }
    sIRC.DoQuit(msg.c_str());
    return true;
}

bool ChatHandler::HandleIRCPartCommand(const char* args)
{
    if(!*args)
        return false;
    
    if(!sIRC.m_connected)
    {
        SendSysMessage("Can't perform command, not connected to an IRC server.");
        return true;
    }
    
    char* channel = strtok((char*)args, " ");
    sIRC.DoPart(channel);
    return true;
}

bool ChatHandler::HandleIRCWhoCommand(const char* args)
{
    if (!sIRC.m_connected) {
        SendSysMessage("Non connecté à IRC.");
        return true;
    }

    std::string nicks = sIRC.nicklist();
    PSendSysMessage("Connectés: %s", nicks.c_str());
    return true;
}

bool ChatHandler::HandleUnbindSightCommand(const char* args)
{
    if (m_session->GetPlayer()->isPossessing())
        return false;

    m_session->GetPlayer()->StopCastingBindSight();
    return true;
}
bool ChatHandler::HandleZoneBuffCommand(const char* args)
{
    if (!*args)
        return false;

    char *bufid = strtok((char *)args, " ");
    if (!bufid)
        return false;

    HashMapHolder<Player>::MapType const& players = ObjectAccessor::Instance().GetPlayers();
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
    if (!*args)
        return false;

    char *displid = strtok((char *)args, " ");
    if (!displid)
        return false;
    char *factid = strtok(NULL, " ");

    uint16 display_id = (uint16)atoi((char *)args);
    uint8 faction_id = factid ? (uint8)atoi(factid) : 0;

    HashMapHolder<Player>::MapType const& players = ObjectAccessor::Instance().GetPlayers();
    Player *p;

    for (HashMapHolder<Player>::MapType::const_iterator it = players.begin(); it != players.end(); it++) {
        p = it->second;
        if (p && p->IsInWorld() && p->GetZoneId() == m_session->GetPlayer()->GetZoneId() &&
            ((faction_id == 1 && p->GetTeam() == ALLIANCE) || (faction_id == 2 && p->GetTeam() == HORDE) || faction_id == 0))
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

    CellPair p(Trinity::ComputeCellPair(player->GetPositionX(), player->GetPositionY()));
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
    if (!args || !*args) {      // Command is applied on selected unit
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
        QueryResult *result = WorldDatabase.PQuery("SELECT id FROM creature WHERE guid = %u LIMIT 1", uintGUID);
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
    if (!args || !*args)
        return false;
        
    Player* plr = m_session->GetPlayer();
    if (!plr)   // How the hell could that happen? o_O
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
    if (!args || !*args)
        return false;
    
    char *chrPoolId = strtok((char *)args, " ");
    if (!chrPoolId)
        return false;
        
    uint32 poolId = (uint32)atoi(chrPoolId);
    if (!poolId)
        return false;
        
    Unit *creature = getSelectedUnit();
    if (!creature || creature->GetTypeId() == TYPEID_PLAYER)
        return false;
        
    if (!creature->ToCreature())
        return false;
        
    WorldDatabase.PExecute("UPDATE creature SET pool_id = %u WHERE guid = %u", poolId, creature->ToCreature()->GetDBTableGUIDLow());
    return true;
}

bool ChatHandler::HandleDebugPvPAnnounce(const char* args)
{
	if (!args || !*args)
		return false;
		
	/*if(ChannelMgr* cMgr = channelMgr(HORDE)) {
		std::string channelname = "pvp";
		std::string what = "CALU";
		Player *p = m_session->GetPlayer();
		uint32 messageLength = strlen(what.c_str()) + 1;
        if(Channel *chn = cMgr->GetChannel(channelname)) {
            WorldPacket data(SMSG_MESSAGECHAT, 1+4+8+4+channelname.size()+1+8+4+messageLength+1);
			data << (uint8)CHAT_MSG_CHANNEL;
			data << (uint32)LANG_UNIVERSAL;
			data << p->GetGUID();                               // 2.1.0
			data << uint32(0);                                  // 2.1.0
			data << channelname;
			data << p->GetGUID();
			data << messageLength;
			data << what;
			data << uint8(4);            
            
            chn->SendToAll(&data);
            
            return true;
		}
        
        return false;
    }*/
    
    char *msg = strtok((char *)args, " ");
    if (!msg)
        return false;
        
	char *channel = "pvp";
    
    HashMapHolder<Player>::MapType& m = ObjectAccessor::Instance().GetPlayers();
    for(HashMapHolder<Player>::MapType::iterator itr = m.begin(); itr != m.end(); ++itr)
    {
        if (itr->second && itr->second->GetSession()->GetPlayer() && itr->second->GetSession()->GetPlayer()->IsInWorld())
        {
            if(ChannelMgr* cMgr = channelMgr(itr->second->GetSession()->GetPlayer()->GetTeam()))
            {
                if(Channel *chn = cMgr->GetChannel(channel, itr->second->GetSession()->GetPlayer()))
                {
                    WorldPacket data;
                    data.Initialize(SMSG_MESSAGECHAT);
                    data << (uint8)CHAT_MSG_CHANNEL;
                    data << (uint32)LANG_UNIVERSAL;
                    data << (uint64)(itr->second->GetSession()->GetPlayer()->GetGUID());
                    data << (uint32)0;
                    data << channel;
                    data << (uint64)(itr->second->GetSession()->GetPlayer()->GetGUID());
                    data << (uint32)(strlen(msg) + 1);
                    data << msg;
                    data << (uint8)4;
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
        SpellEntry const* spellProto = (*itr).second->GetSpellProto();
        PSendSysMessage("%u - %s (stack: %u) - Slot %u", spellProto->Id, spellProto->SpellName[sWorld.GetDefaultDbcLocale()], (*itr).second->GetStackAmount(), (*itr).second->GetAuraSlot());
    }
    
    return true;
}

bool ChatHandler::HandleInstanceSetDataCommand(const char* args)
{
    if (!args)
        return false;
        
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
    if (!args)
        return false;
        
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
    QueryResult *result = WorldDatabase.PQuery("SELECT MAX(pool_id) FROM creature");
    Field *fields = result->Fetch();
    
    uint32 maxId = fields[0].GetUInt32();
    
    PSendSysMessage("Current max creature pool id: %u", maxId);
    
    return true;
}

bool ChatHandler::HandleSetTitleCommand(const char* args)
{
    if (!args || !*args)
        return false;

    uint32 titleId = atoi(args);

    if(CharTitlesEntry const* titleEntry = sCharTitlesStore.LookupEntry(titleId)) {
        if (Player* plr = getSelectedUnit()->ToPlayer())
            plr->SetTitle(titleEntry);
        else if (Player* plr = m_session->GetPlayer())
            plr->SetTitle(titleEntry);
    }

    return true;
}

bool ChatHandler::HandleRemoveTitleCommand(const char* args)
{
    if (!args || !*args)
        return false;

    uint32 titleId = atoi(args);

    if(CharTitlesEntry const* titleEntry = sCharTitlesStore.LookupEntry(titleId)) {
        if (Player* plr = m_session->GetPlayer())
            if (plr->HasTitle(titleEntry))
                plr->RemoveTitle(titleEntry);
    }

    return true;
}

bool ChatHandler::HandleNpcSetScriptCommand(const char* args)
{
    Unit* target = getSelectedUnit();
    
    if (!target || target->GetTypeId() != TYPEID_UNIT || target->isPet())
        return false;
        
    if (!args)
        return false;
    
    char* scriptname = strtok((char*)args, " ");
        
    WorldDatabase.PExecute("UPDATE creature SET scriptname = '%s'", scriptname);
    
    return true;
}

bool ChatHandler::HandleGMStats(const char* args)
{
    uint32 accId = m_session->GetAccountId();
    
    QueryResult* timeResult = CharacterDatabase.Query("SELECT GmWeekBeginTime FROM saved_variables");
    if (!timeResult) {
        PSendSysMessage("Error with GmWeekBeginTime.");
        return true;
    }
    
    Field* timeFields = timeResult->Fetch();
    uint64 beginTime = timeFields[0].GetUInt64();
    
    QueryResult* countResult = CharacterDatabase.PQuery("SELECT COUNT(*) FROM gm_tickets WHERE timestamp > "I64FMTD" AND closed = %u", beginTime, accId);
    if (!countResult) {
        PSendSysMessage("No information found for this account.");
        return true;
    }
    
    Field* countFields = countResult->Fetch();
    uint32 count = countFields[0].GetUInt32();
    
    PSendSysMessage("Vous avez fermé %u tickets depuis le début de la semaine.", count);
    
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
            sWorld.setConfig(CONFIG_BOOL_MMAP_ENABLED, true);
            SendSysMessage("WORLD: mmaps are now ENABLED (individual map settings still in effect)");
        }
        else if (argstr == "off")
        {
            sWorld.setConfig(CONFIG_BOOL_MMAP_ENABLED, false);
            SendSysMessage("WORLD: mmaps are now DISABLED");
        }
        else
        {
            on = sWorld.getConfig(CONFIG_BOOL_MMAP_ENABLED);
            PSendSysMessage("mmaps are %sabled", on ? "en" : "dis");
        }
        return true;
    }

    return true;
}

bool ChatHandler::HandleMmapTestArea(const char* args)
{
    float radius = 40.0f;
    //ExtractFloat(&args, radius);

    CellPair pair(Trinity::ComputeCellPair( m_session->GetPlayer()->GetPositionX(), m_session->GetPlayer()->GetPositionY()) );
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
        uint32 uStartTime = getMSTime();

        float gx,gy,gz;
        m_session->GetPlayer()->GetPosition(gx,gy,gz);
        for (std::list<Creature*>::iterator itr = creatureList.begin(); itr != creatureList.end(); ++itr)
        {
            PathInfo((*itr), gx, gy, gz);
            ++paths;
        }

        uint32 uPathLoadTime = getMSTimeDiff(uStartTime, getMSTime());
        PSendSysMessage("Generated %i paths in %i ms", paths, uPathLoadTime);
    }
    else
    {
        PSendSysMessage("No creatures in %f yard range.", radius);
    }

    return true;
}

bool ChatHandler::HandleReloadConditions(const char* args)
{
    sLog.outString("Re-Loading Conditions...");
    sConditionMgr.LoadConditions(true);
    SendGlobalGMSysMessage("Conditions reloaded.");
    return true;
}

bool ChatHandler::HandleReloadSmartAI(const char* /*args*/)
{
    sLog.outString("Re-Loading SmartAI Scripts...");
    sSmartScriptMgr.LoadSmartAIFromDB();
    SendGlobalGMSysMessage("SmartAI Scripts reloaded.");
    return true;
}

bool ChatHandler::HandleDebugUnloadGrid(const char* args)
{
    if (!args || !*args)
        return false;

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

    Map* map = MapManager::Instance().FindMap(mapid);
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
    if (!args || !*args)
        return false;
    
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
    
    return true;
}

bool ChatHandler::HandleReloadSpellTemplates(const char* args)
{
    sLog.outString("Re-loading spell templates...");
    objmgr.LoadSpellTemplates();
    SendGlobalGMSysMessage("DB table `spell_template` (spell definitions) reloaded.");
    return true;
}

bool ChatHandler::HandleGuildRenameCommand(const char* args)
{
    if (!args || !*args)
        return false;
        
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
    if (!args || !*args)
        return false;

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
    if (!args || !*args)
        return false;

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
    if (!args || !*args)
        return false;

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

/*
#define LIST_SIZE 41564
unsigned int ownerguid[LIST_SIZE] = {
20,20,192,192,192,192,192,192,192,192,192,3174,3174,3174,3174,3174,3174,3174,3174,3174,3174,3174,3174,3174,3174,3174,3174,3174,3174,3174,3174,3174,3174,3174,3174,3174,3174,3174,3174,3174,3174,3174,3174,3174,3174,3174,3174,3174,3174,3174,3174,3174,3174,3174,3174,3174,3174,3174,3174,3174,3174,3174,3174,3174,3174,3174,3174,3174,3174,3174,3174,3174,3174,3174,3174,3174,3174,3174,3174,3174,3174,3174,3174,3174,3174,3174,5698,5698,5698,5698,5698,5698,5698,5698,5698,5698,5698,5698,5698,5698,5698,5698,5698,5698,5698,5698,5698,5698,7457,8352,8352,8352,8352,8352,8352,8352,8352,8352,8352,8352,8352,8352,8352,8352,8352,8352,8352,8352,8352,8352,8352,8352,8352,8352,8352,8352,11707,11707,11707,11707,11707,11707,11707,11707,11707,11707,11707,11707,11707,11707,11707,11707,11707,11707,14527,14527,14527,14527,14527,14527,14527,14527,14527,14527,14527,14527,14527,14527,15841,15841,18195,18195,18195,19243,19681,19681,19681,19681,19681,19681,19681,19681,26533,26533,26533,26533,26533,26533,26533,26533,26533,26533,26533,26533,26533,27742,27742,27742,27742,27742,27742,28476,28476,28476,28476,28476,28476,28476,28476,28476,28476,28476,28476,28476,30075,32865,32865,32865,32865,35518,35518,35518,35518,35518,35518,35518,35518,35518,35518,35518,35518,35518,35518,35518,35518,35518,35518,35518,35518,35518,35518,35518,35518,35518,35518,35518,35518,35518,35518,35518,35518,35518,35518,35518,36516,36516,36516,36516,36516,37195,37195,37195,37195,37195,37195,37195,37195,37195,37195,37195,37195,37195,37195,37195,37195,37406,37406,37406,37406,37406,37406,38606,38606,38606,38606,38606,38606,38606,38606,38606,40276,40276,40276,40276,40276,40276,40276,40276,40276,40276,40276,46288,46288,46288,46288,47366,47366,48562,48562,48562,48562,48562,48562,48638,48638,52443,52443,52443,52443,52443,52443,52443,52443,52443,52443,52443,52443,52443,52443,52443,52443,52443,52443,54408,54408,54408,54408,54408,54408,54408,54408,54408,54408,54408,54408,54408,54408,56470,56470,56470,56470,56470,56470,56470,56470,56470,56470,56470,56470,56470,56470,56470,56470,56470,56470,56470,56822,58094,58094,58094,58094,58094,58094,58094,58094,58094,58094,58094,58094,58094,58094,58094,58094,58094,58094,58094,58094,58094,58094,58094,58094,58094,58094,58094,58094,58094,58094,58094,58094,58094,58094,58094,58094,58094,58094,58094,58094,58094,58094,58094,58094,58094,58094,58094,58094,58094,58094,58094,58094,58094,58094,58094,58094,58094,58094,58094,58094,58094,58094,58094,58094,58094,60342,61347,61347,61347,61347,61347,61347,61347,61347,61347,61347,61347,61347,61347,61347,61347,61347,61347,61347,61347,61347,61347,61347,61347,61347,61347,61347,61347,61347,61347,61347,61347,61347,61347,61496,62482,62482,62482,62482,62482,62482,62482,62482,62482,62482,63273,63273,63273,63273,63273,63273,63273,63273,63273,63273,63273,63273,63273,63273,63273,63273,63273,63273,63273,63273,63273,63273,63273,63273,63273,63273,63273,63273,63273,63273,63273,63273,63273,63273,63273,63273,63839,66325,66325,66325,66325,66325,66325,66325,66325,66325,67759,67759,69383,69383,69383,69383,69383,69383,69383,69383,69383,69383,69383,69383,69383,69383,69383,69383,69383,69383,69383,69383,69383,69383,69383,69383,69383,69383,69383,69383,69383,69383,69383,69383,69571,69571,69571,69571,69571,69571,69571,69571,70934,70934,70934,70934,70934,70934,70934,70934,70934,70934,70934,70934,70934,70934,70934,70934,70934,70934,70934,70934,70934,70934,70934,70934,70934,70934,70934,70934,70934,70934,70934,70934,71013,71013,71013,71013,71013,71013,71013,71013,71013,71013,71013,71013,71013,71013,71013,71013,71013,71013,71013,71013,71013,71013,71013,71013,71013,71013,71013,71013,71013,71013,71013,71013,71013,71013,71013,71013,71013,71013,71832,71832,71832,71832,71832,71832,71832,71832,71832,71832,71832,71832,71832,71832,71832,71832,71832,71832,71832,71832,71832,71832,71832,71832,71832,71832,71832,71832,71832,71832,72184,74147,74147,74147,75551,75645,75645,75645,75645,75645,75645,75645,75645,75645,75645,75925,75932,75932,75932,75932,75932,75932,75932,75932,75932,75932,75932,75932,75932,75932,75932,75932,75932,75932,77274,77358,77358,77358,77358,80886,80886,80886,80886,80886,80886,80886,80886,80886,80886,80886,80886,80886,80886,80886,80886,80886,80886,80886,80886,80886,80886,80886,80886,80886,80886,80886,80886,80886,82641,82641,83743,83743,83743,83743,83743,83743,85087,85087,85087,85087,85087,85087,85087,85087,85087,85985,85985,85985,85985,86091,86091,86091,86091,86091,86100,86200,86966,86966,86966,86966,86966,86966,86966,86966,86966,86966,86966,86966,86966,86966,86966,86966,86966,86966,86966,86966,86987,86987,86987,86987,86987,86987,87036,87036,87036,87205,87205,87205,87205,87205,87205,87205,87205,87307,87307,87307,87307,87603,87603,87967,87967,88173,88420,88992,88992,88992,88992,88992,88992,91813,91813,92189,92189,92189,92189,92189,92189,92189,92189,92189,92189,92189,92189,92189,92189,92189,92189,92189,92189,92189,92189,92189,92189,92189,92189,92189,92189,92189,92189,92189,92189,92189,92189,92189,92189,92189,92189,92189,92189,92189,92189,92189,92189,92189,92189,92189,92189,92189,92189,92189,92189,92189,92189,92189,92189,92189,92189,92189,92974,92974,92974,92974,92974,92974,92974,92974,92974,92974,92974,92974,92974,92974,92974,92974,92974,92974,92974,92974,92974,93018,93018,93018,93018,93018,93018,93018,93018,93018,93018,93018,93018,93018,94472,94472,95369,95369,95369,95369,95369,95369,95369,95369,95369,95369,95369,96340,96340,96340,96340,96340,97198,97198,97198,97198,97198,97198,97198,98203,98203,100783,100783,101206,101206,101206,101206,101206,101206,101206,101206,101206,101206,101206,101206,101206,101206,101485,101485,101485,102005,102005,102005,102005,102005,102005,102005,102005,102005,102355,102355,102355,102360,102364,102364,102364,102364,102364,102364,102364,102364,102364,102364,102364,102364,102364,102364,102364,102364,102364,104182,104182,104182,104182,104182,104182,104182,104182,104182,104182,104182,104182,104182,104182,104182,104182,104182,104182,104182,104182,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,104184,105794,105794,105794,105794,105794,105794,105794,105794,105794,105794,105794,105794,105794,105794,105794,105794,105794,105794,105794,105794,105794,105794,105794,105794,105794,105794,105794,106167,106167,106167,106167,106167,106167,106167,106167,106167,106167,106167,106167,106167,106167,106167,106167,106167,106167,106167,106537,106537,106537,106537,106537,106537,106537,106537,106537,106537,106537,106537,106537,106537,106537,106537,106537,106537,106537,106537,106537,106537,106537,106537,106537,106537,106537,106537,106537,106537,106537,106537,106539,106539,106539,106539,106539,106539,106539,106539,106539,106539,106539,106539,106539,106539,106539,106539,106539,106539,106729,106729,106729,106729,106729,106729,106729,106729,106729,106729,106729,106729,106729,106729,108001,108001,108682,108682,108682,108682,108682,108682,108682,108682,108682,108682,108682,108682,108682,108682,108682,108682,108682,108682,108682,108682,108682,108682,108682,108682,108682,108682,108682,108682,108682,108682,108682,108682,108682,108682,108682,108682,108682,108682,108682,108682,108682,108682,108682,108682,108682,108682,108682,108682,108682,108682,108682,108682,108682,108682,108682,108682,108682,108682,108682,108682,108682,108682,108682,108682,108682,108682,108682,108682,108682,108682,108682,108682,108682,108682,108682,108682,108682,108682,109420,109420,109420,109420,109420,109420,109420,109420,109420,109420,109420,109420,109420,109420,109420,109420,109420,109420,109420,109420,109420,109420,109420,109420,109420,109420,109420,109420,109420,109420,109420,109420,109420,109420,109420,109420,109420,109420,109420,109420,109420,109420,109420,109420,109420,109420,109420,109420,109420,109420,109420,109420,109420,109420,109420,109420,109420,109719,109719,109719,109719,109719,109719,109719,109719,110057,110057,110057,110057,110505,110505,110505,110505,110505,110505,110505,110505,110505,111993,111993,111993,111993,111993,112657,112657,112657,112657,112657,112657,112657,112657,112657,112657,112657,112657,112657,112657,112657,112657,112657,112657,112657,112657,112657,112657,112657,112657,112657,112657,112657,112657,112657,112657,112657,112657,112657,112657,112657,112657,112657,112657,112657,112657,112657,112657,112657,114095,114095,114095,114095,114095,114095,114095,116533,116533,116533,116533,116533,116533,116533,116533,116533,116533,116533,116533,116661,116661,116661,116661,116661,116661,116661,116661,116661,116661,116661,116661,116661,116661,116661,116661,116661,116661,116661,116661,116661,116661,116661,116661,116698,117564,117564,117564,117564,117564,117564,117564,117564,117564,117564,117564,117564,117564,117564,117564,117564,117564,117564,117564,117564,117564,117564,117564,117564,117564,117564,117564,117564,117564,117564,117564,117564,117564,117564,117564,117564,117564,117564,117564,117564,117564,117564,117564,117564,117564,117564,117564,117564,117564,117564,117564,117564,117564,117564,117564,117564,117564,117564,117564,117564,117564,117564,117564,117564,117564,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117620,117748,117748,117748,117748,117748,122936,122936,122936,124322,124322,124322,124322,124322,124322,124322,124322,124322,126268,126268,126268,126268,126268,126268,126268,126268,126268,126268,126268,126268,126268,126268,126268,126268,126268,126268,126268,126911,127030,127030,127030,127030,127030,127030,127795,127795,127795,127795,127795,127795,127795,127795,127795,127795,127795,127795,127795,127795,127795,127795,127795,127795,127854,128929,128929,128929,128929,128929,128929,128929,128929,128929,128929,128929,128929,128929,128929,128929,128929,128929,128929,128929,128929,128929,128929,128929,128929,128929,128929,128929,128929,128929,128929,128929,128929,128929,128929,128929,128929,128984,128984,128984,128984,129066,129066,129066,129066,129066,129066,129066,129066,129066,129066,129066,129066,129066,129066,129066,129066,129066,129066,129066,129066,129066,129066,129066,129066,129066,129066,129066,129066,129066,129066,129066,129066,129066,129066,129066,129066,129066,129066,129066,129072,129072,129072,129097,129097,129097,129097,129097,129097,129097,129097,129097,129097,129097,129097,129097,129097,129097,129097,129097,129097,129097,129097,129097,129097,129097,129097,129097,129097,129097,129097,129097,129097,129097,129097,129097,129097,129097,129097,129097,129097,129097,129097,129097,129097,129097,129097,129097,129097,129097,129097,129097,129097,129097,129097,129097,129097,129097,129097,129097,129097,129097,129097,129097,129097,129097,129097,129097,129357,129653,129653,129653,129653,129653,129653,129653,129930,129930,129930,129930,129930,129930,129930,129930,129930,129930,129930,129930,130193,130193,130193,130193,130193,130193,130193,130193,130193,130193,130193,130193,130193,130193,130780,130780,130780,130780,130780,130780,130780,130780,130898,131614,131810,131898,131898,131898,131898,131898,131898,131898,131898,131898,131936,131936,131936,131936,131936,131936,131936,131936,131936,131936,131936,131936,131936,131936,131936,131936,131936,131936,131936,131936,131936,131936,131936,131936,131936,131936,131936,131936,131936,131936,131936,131936,131936,131936,131936,131936,131936,131936,131936,131936,131936,131936,131936,131936,131936,131936,131936,131936,131940,131940,131940,131940,131940,131940,131940,131940,131940,131940,131940,131940,131940,132156,132156,132156,132156,132156,132156,132156,132156,132156,132156,132156,132567,132567,132567,133350,133350,133350,133350,133350,133350,133350,133350,133350,133350,133350,133543,133543,133543,133543,133543,133543,133543,133543,133543,133543,133543,133543,133543,133543,133543,133543,133543,133543,133543,133566,133566,133566,133566,133566,133870,133870,133870,133870,133870,133870,133870,133870,133870,133870,133870,133870,133870,133870,133870,133870,133870,133870,133870,133870,133870,133870,133870,133870,133870,133870,135808,135808,135808,135808,137227,137227,137227,137227,137227,137635,137635,137635,137866,137866,137866,138829,139462,139462,139462,139462,140219,140219,140219,141064,141064,141064,141064,141064,142060,142060,142060,142060,142060,142060,142060,142060,142060,142111,142111,142111,142111,142111,142111,142111,142111,142111,142111,142111,142111,142111,142111,142111,142111,142111,142111,142111,142111,142111,142111,142111,142518,142518,142518,142518,142518,142518,142518,142518,142518,142518,142518,143404,143404,143404,143404,143404,143404,144033,144033,144033,144033,144976,144976,144976,144976,144976,144976,144976,144976,144976,144976,144976,144976,144976,144976,144976,145203,145203,145203,145203,145203,145203,145203,145203,145203,145203,145203,145203,145203,145203,145203,145203,145203,145203,145203,145203,145203,145203,145203,145203,145203,145203,145203,145203,145203,145203,145203,145203,145203,145203,145203,145203,145203,145203,145240,145240,145240,145240,145240,145240,145240,145240,145240,145240,146579,146579,146579,146579,146579,147818,147818,147818,147818,147818,147818,147818,147818,147818,147918,147918,147918,147918,147918,147918,147918,147918,147918,147918,147918,147918,147918,147918,147918,147918,147918,147918,147918,147918,147918,148015,148015,148015,148015,148288,148292,148292,148292,148292,148292,148292,148292,149919,149919,149919,149919,149919,149919,149919,149919,149919,149919,149919,149919,149919,149919,149919,149919,149919,149919,149919,149919,149919,149919,151386,151386,151386,151386,151386,151386,153078,153078,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154725,154813,156624,156624,156624,156624,156624,156624,156624,156624,157684,160309,160508,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163672,163754,163754,163754,163754,163754,163754,163929,163929,163929,163929,163929,163929,163929,163929,163929,163929,163929,163929,163929,163929,163929,163929,163929,163929,163929,163929,163929,163929,163929,163929,163929,163929,163929,163929,163929,163929,163929,163929,163929,163929,163929,163929,163929,163929,164331,164331,164331,164331,164331,164331,164331,164331,164331,164331,164331,164331,164543,164543,164543,164543,164543,164543,164543,164570,164570,164570,164570,164570,164570,164570,164570,164570,164570,164570,164570,164570,164570,164570,164570,164570,164570,164570,164570,164570,164570,164570,164570,164570,164570,164570,164570,164570,164570,164570,164570,164570,164570,164570,164570,164570,164570,164570,164570,164570,164570,164570,164570,165214,165214,165214,165214,165214,165214,165214,165214,165214,165214,165214,165214,165214,165214,165214,165214,165214,165214,165214,165214,165214,165664,165966,165966,165966,165966,165966,165966,165966,165966,167289,167289,167289,167289,167289,167289,167289,167289,167289,167289,167289,167289,167289,167289,167289,167289,167289,167289,167289,167289,167289,167289,167289,167549,167549,167549,167549,167549,167549,167549,167549,167549,167549,167549,167549,167549,167549,167549,167549,167549,167549,167549,167549,167549,167549,167549,167549,167549,167549,167549,167549,167549,167549,167549,167549,167549,167549,167549,167549,167549,167549,167549,167549,167549,167549,167549,167549,167549,167549,169136,169136,169136,169136,169136,169136,169136,169136,169136,169136,169136,169136,169136,169136,169136,169136,169136,169136,169136,169136,169548,169548,169548,169548,169548,169548,169548,169548,169548,169978,170562,170562,170562,170562,170562,170562,170562,170562,170562,170562,171193,171193,171193,171193,171193,171248,171248,171248,171248,171248,171248,171248,171248,171248,171248,171248,171248,171248,171248,171248,171248,171248,171248,171248,171248,171248,171248,171248,171248,171248,171248,171248,171248,171248,171248,171248,171248,171248,171248,171248,171248,171248,171248,172921,175159,176821,176821,176821,176821,176821,176821,176924,178855,179642,179642,179642,179642,179642,179642,179642,179642,179642,179642,179642,179701,179701,179701,179701,179701,179701,179701,179701,179701,181182,181247,181789,181789,182193,182193,182193,182193,182454,182454,182454,182454,182454,182454,182454,182454,182454,182454,182454,182454,182454,182454,182454,182454,182454,182454,182454,182454,182454,182454,182454,182454,182454,182454,182454,182454,182454,182454,182454,182454,182454,183761,183761,184716,184716,184716,184716,184716,184716,184716,184716,184716,185063,185063,185063,185063,185063,185063,185063,185063,185063,185931,185931,185931,185931,185931,185931,185931,185931,185931,185931,185931,185931,185931,185931,185931,185931,185931,185931,185931,185931,185931,185931,185931,185931,185931,185931,185931,185931,185931,185964,185964,185964,186059,186059,186479,186580,186580,186580,186580,186580,186580,186580,186580,186580,186580,186580,186580,187928,187928,187928,187928,187928,187928,187928,187928,187928,187928,187928,187928,187928,187928,187928,187928,187928,187928,187928,187928,187928,188148,188148,189198,189198,189198,189198,189198,189198,189198,189198,189198,189198,189198,189198,189198,189198,189198,189198,189198,189198,189315,189315,189315,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192204,192839,192839,192839,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194247,194659,194659,194659,194659,195321,196465,196465,196465,196465,196524,196524,196524,196524,196524,196524,196524,196524,196524,196524,196524,197672,198313,198313,198313,198313,198313,198313,198313,198313,198313,198369,198369,198369,198369,198369,198719,198719,198719,198719,198719,200494,200494,201110,201110,201110,201110,201110,201191,201191,201191,203583,203583,203583,203583,203583,203583,203583,203597,203597,203597,203597,203597,203597,203597,203597,203752,203752,203752,203752,203752,204408,204408,204408,204408,204408,204408,204408,204486,204953,204953,204953,204953,205699,205699,205699,205699,205699,207510,207510,208608,208608,208608,208608,209041,209041,209041,209041,209047,209047,209047,209047,209047,209047,209047,209047,209047,209047,210841,210841,210841,210841,211947,211947,211947,211947,211947,214536,215506,215506,215506,215506,215506,215506,215506,215506,215506,215707,215707,215802,215963,215963,215963,215963,215963,215963,215963,215963,215963,215963,215963,215963,215963,215963,215963,215963,215963,215963,215963,215963,215963,215963,215963,215963,215963,215963,215963,215963,217633,217633,218126,220585,220585,220585,220585,220585,220585,220585,220585,220585,220585,220585,221090,221090,222029,222029,222030,222030,222030,222030,222030,222030,222030,222030,222030,222030,222030,222030,222030,222030,222030,222030,222030,222030,222030,222030,222030,222030,222030,222030,222030,222030,222030,222030,222030,222030,222030,222030,223850,223850,223917,223917,224173,224173,224173,224313,224313,224313,224313,224723,224723,224723,224723,224723,224723,224723,224723,224723,224723,224723,224723,225499,225499,225499,225499,225499,225499,225499,225654,227386,227386,227386,227386,227386,227386,227386,227386,227386,227386,227386,227386,227386,227386,227386,227386,227386,227386,227386,227386,227515,227515,227515,227515,227515,227515,227515,227515,227515,227515,227515,227515,227515,227515,227515,227515,227515,227515,227515,227552,227830,227830,227830,227830,227830,227830,227830,228732,228732,228732,228732,228732,228732,228732,228732,228732,228732,228732,228876,228876,229207,229207,229207,229207,229800,229800,229800,230234,231699,231699,231699,231699,231699,231699,231699,231893,232339,232339,232339,232383,233156,233156,234095,234095,234095,234095,234095,234095,234095,234095,234095,234961,234961,234961,234961,234961,234961,234961,234961,234961,234961,234961,234961,234961,234961,234961,234961,234961,234961,234961,234961,234961,234961,234961,234961,234961,234961,234961,234961,234961,234961,234961,234961,235733,235733,235733,235733,235733,235733,237229,237424,237424,237424,238980,238980,238980,238980,238980,238980,238980,239875,239875,239875,239875,239875,239875,239875,239875,239985,239985,239985,239985,239985,239985,239985,239985,239985,240164,240164,240164,240164,240164,240164,240164,240164,240164,240164,240164,240164,240164,240164,240164,240164,240164,240164,240630,240630,240630,240630,240630,240630,240630,240630,240630,241690,242054,242054,242054,242315,242913,242913,242913,242913,242959,243103,243103,243103,243103,243103,243103,243103,243103,243103,243103,243103,243103,243103,243103,243103,243103,243103,243103,244749,244749,244749,244749,244749,244749,244749,244749,244749,244749,244749,244749,245118,245118,245118,245118,246593,246593,246593,246593,246593,246593,246593,246593,246593,246593,246593,246593,246593,246593,246593,246593,246593,246593,246593,247227,247227,247232,247232,247232,247232,247989,247989,247989,247989,247989,247989,247989,247989,247989,247989,247989,247989,247989,247989,247989,247989,247989,247989,247989,247989,247989,247989,247989,247989,247989,247989,247989,247989,247989,247989,247989,247989,247989,247989,247989,247989,247989,247989,248605,248605,248605,248605,248605,248605,248605,248605,248605,248605,248605,248605,248605,248605,248605,248605,248605,248605,248605,248605,248605,248605,248618,248618,248618,248618,248618,248618,248618,248618,248618,248618,248618,248618,248618,248618,248618,248618,248618,248618,248618,248618,248618,248618,248618,248618,248618,248618,248618,248618,248618,248618,248618,248618,248618,248618,248618,248618,248618,249147,249147,249147,249147,249147,249147,249147,249147,249147,249147,249147,249147,249147,249147,249147,249147,249147,249147,249147,249147,249147,249147,249147,249147,249147,249147,249147,249147,249150,249150,249150,249150,249641,249641,249641,249641,249922,249922,249986,251043,252200,253153,253153,253153,254564,254743,254743,254743,254743,254743,254743,254743,256606,256606,256606,256606,256606,256606,256606,256606,256606,256606,256606,256606,256606,256606,256606,256606,256606,256606,256606,256606,256606,256606,256606,256606,256606,256606,256606,256606,256606,256606,256606,256606,256606,256606,256606,256606,256606,256606,256606,256606,256606,256606,257325,257325,257325,257325,257325,257325,257325,257325,257325,257325,257325,257325,257325,257325,257325,257325,257325,257325,258559,258559,258559,258559,258559,258603,258858,259161,259161,259161,259161,259161,259161,259161,259161,259161,259161,259161,259161,259161,259161,259161,259161,259161,259161,259161,259161,259161,259161,259161,259161,259161,259161,259161,259161,259161,259161,259161,259161,259161,259161,261153,261153,261153,261153,261153,261153,261153,262097,262097,262097,262097,262097,262097,262718,262718,262718,262718,262718,262718,262718,262718,262718,262718,264725,264725,265169,265169,265169,265169,265169,265169,265169,265169,265169,265964,265964,266093,266407,266407,266407,266407,266407,266407,269071,269071,269338,272315,272315,272315,272367,272367,272367,272662,272662,272662,272662,273394,273652,274478,274478,274478,274478,274478,274478,274478,274478,274478,274818,274818,274818,274818,274818,274818,274818,274818,274818,274818,274818,274818,275693,275693,275693,276545,277233,277233,277233,277233,280171,280171,280286,280286,280286,280286,280286,280286,280286,280286,280286,280286,280286,280286,280286,280286,280286,280286,280548,280548,280548,280548,280548,280548,280548,280548,280548,280548,280548,280548,280548,280548,280548,280548,280548,280548,280570,280788,280911,281991,281991,281991,281991,281991,281991,281991,281991,281991,281991,281991,281991,281991,281991,281991,281991,281991,281991,281991,281991,281991,281991,281991,281991,281991,281991,281991,281991,281991,281991,281991,281991,281991,281991,281991,281991,281991,281991,281991,282153,282153,282153,282153,282153,283851,283967,283967,283967,283967,283967,283967,283967,283967,283967,283967,283967,283967,283967,283967,283967,283967,283967,283967,283967,283967,283967,283967,283967,283967,283967,283967,283967,283967,283967,283967,283967,283967,283967,283967,283967,283967,283967,283967,283967,284077,284592,284592,284592,284592,284592,284592,284592,284592,284592,284592,284592,284891,284891,284891,284891,284891,284891,284891,284891,284891,284891,284891,285166,285166,285720,286662,286662,286662,286662,286662,286662,286662,286662,286662,286662,286662,286662,286662,286662,286662,286662,286662,286662,286662,286662,286662,286662,286662,286662,286662,286662,286662,286662,286662,286662,286662,286662,286662,286662,286662,286662,286662,287100,288952,288952,288952,288952,288952,289231,289272,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290216,290779,290779,290779,290779,290779,290779,290779,292334,292702,292702,292702,292702,292702,293487,293487,293487,293539,293539,293539,293539,293539,297284,297284,297284,297284,297284,297284,297284,297284,297284,297284,297284,297284,297284,297284,297329,297329,297329,297329,297329,297329,297329,297329,297329,297329,297329,297329,297329,297329,297329,297329,297713,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297822,297932,297932,297991,298300,298300,298300,298300,298300,298300,298300,298300,298300,298300,298300,298300,300347,300347,300347,300347,300347,300347,300347,300347,300347,300347,300347,300347,300347,300347,300347,300347,300347,300347,300347,300347,300347,300347,301440,301440,301716,301716,302466,302466,302466,302466,302466,302466,302466,302466,302466,302466,302466,302466,302466,302466,302466,302466,302466,302466,302466,302466,302466,302466,302466,302466,302466,302466,302466,302466,302466,302466,302466,302466,302466,302466,302849,302849,302849,302849,302849,302849,302849,302849,302849,302849,302889,302889,302889,302889,302889,302889,302889,302889,302891,302891,302891,302891,302891,302891,302891,302891,302891,302891,302891,302891,302891,302891,302983,302983,302983,302983,302983,303451,303451,303451,303451,303451,303451,303451,303451,303451,303451,304447,304447,304447,304447,304447,304447,304676,305697,305697,305697,305697,305697,305697,305697,305697,305697,305697,305697,305697,305697,305697,305697,305697,305697,305697,305851,305851,305851,306277,306277,306826,306826,306826,306826,306826,306826,306990,306990,306990,307113,307759,307759,307937,307937,307937,307937,307937,307937,307937,307937,307990,307990,307990,307990,307990,307990,307990,307990,307990,307990,307990,307990,307990,307990,307990,307990,307990,307990,307990,307990,307990,307990,307990,307990,307990,307990,307990,307990,307990,307990,307990,307990,307990,307990,307990,307990,307990,307990,307990,307990,307990,307990,307990,307990,307990,307990,307990,307990,307990,307990,307990,307990,307990,307990,307990,307990,307990,307990,307990,307990,307990,307990,307990,307990,307990,307990,307990,307990,307990,307990,308099,308099,308099,308099,308099,308099,308099,308099,308099,308099,310298,310298,310298,310298,310298,310298,310298,310298,310888,312024,312024,312359,312359,312359,312385,313037,313037,313037,313037,313037,313037,313037,313037,313037,313037,313037,313037,313037,313037,313037,313037,313037,313037,313037,313037,313037,313037,313037,313037,313037,313037,313037,313037,313363,313363,313363,313363,313363,313363,313363,313363,313363,313363,313363,313363,313363,313363,313363,313363,313363,313363,313363,313363,313363,313363,313363,313363,313363,313363,313363,313363,313363,313363,313363,313363,313363,313363,313363,313363,313363,313363,313601,313620,313620,313620,313620,313642,313642,313642,313856,314116,314719,314719,314719,314719,314719,314719,314719,314719,314719,314719,314719,314719,314719,314719,314719,314719,314719,314719,314719,314719,314719,314719,314719,314719,314719,315999,315999,315999,315999,315999,315999,315999,315999,315999,315999,315999,316157,316157,316157,316157,316157,316157,316157,316157,316157,316157,316157,316157,316157,316157,316157,316157,316157,316157,316572,316572,316572,318342,318342,318342,318342,318342,318342,318342,318342,318342,318461,318529,318529,318529,318529,318529,318529,318529,318529,318529,318529,318529,318529,318529,318529,318529,318529,318529,318529,318529,318662,319228,319228,319228,319228,319228,319228,319228,319228,319228,319228,319228,319228,319228,319228,319228,319228,319228,319899,319899,319899,319899,320121,320121,320597,320597,320597,320597,321702,321702,321702,321702,321851,322042,322042,322042,322042,322042,322042,322427,324201,324201,324201,324201,324208,324208,324208,324208,324208,324208,324208,324208,324208,324208,324208,324208,324208,324208,324208,324208,324208,324208,324208,324208,324208,324208,324208,324208,324208,324208,324208,324208,324208,324208,324208,324208,324208,324208,324208,324208,324208,324208,324208,324208,324208,324208,324208,324208,324208,324208,324208,324208,324208,324208,324208,324208,324208,324208,324831,324831,324831,324831,324831,324831,324831,324831,324831,324831,324831,324831,324831,324831,324831,324831,324831,324831,324831,324831,324831,324831,324831,324831,324831,324831,324831,324831,324831,324831,325561,325561,325561,325561,325561,325561,325561,325561,325561,325561,325561,325561,325592,325592,325592,325592,325592,325592,325592,325592,327157,327296,327296,327296,327296,327296,327910,327910,327910,327910,327910,327910,327910,327910,327910,328154,328154,328154,328154,328154,328154,328154,328154,328154,328154,328154,328154,328154,328154,328231,328231,328231,328231,328231,328231,328231,328231,328231,328231,328231,328231,328231,328231,328923,328923,328923,328923,328923,328923,328923,328923,328923,328923,328923,328923,328923,328923,328923,328923,328923,328923,328923,328923,328923,328923,328923,328923,328923,328923,328923,328923,328923,328923,328923,328923,328923,328923,328923,328923,328923,328923,328923,328923,328923,328923,328923,328923,328923,328923,328923,328923,328923,328923,328923,328923,328923,328923,328923,328923,328923,328923,328923,328923,328923,328923,328923,328923,328923,328923,328923,328923,329764,329764,332069,332905,332905,332905,332905,334028,334028,334260,334260,334260,334260,334260,334260,334260,334260,334260,334675,334675,334678,334678,336272,336272,336272,336272,336272,336272,336272,336272,336272,337498,337498,337498,337498,337498,337498,337498,337605,337789,337789,337789,337789,337789,337789,337789,337789,337789,337789,337789,337789,337789,337789,337789,337789,337789,337789,337789,337789,337789,337789,337789,337789,337789,337789,337789,337789,337789,337789,337789,337789,337789,337789,337789,337789,337789,337789,337789,337789,337789,337789,337789,337789,337789,337789,337789,337789,337789,337789,337789,337832,337832,338474,338474,338474,338474,338474,338474,338474,338474,338474,338474,338474,338474,338474,338474,338474,338474,338474,338474,338474,338474,338474,338474,338474,338474,338474,338844,338844,338844,338844,338844,338844,338844,338844,338844,338844,338844,338844,338844,338844,338844,338844,338844,338844,338844,338844,338844,338844,338844,338844,338844,338844,338844,338844,338844,338844,338844,338844,338844,338844,338844,338844,338844,339240,339240,339616,339616,340266,340713,340901,340901,341838,341838,341838,341838,341838,341838,341838,341838,341838,341838,341838,341838,341838,341838,341838,341838,341932,344476,344476,344476,344476,344476,344476,344476,344476,344476,344476,344476,344476,344476,344476,344476,344503,344834,344834,344834,344834,344834,344834,344834,344834,344834,344834,344834,344834,344834,344834,344834,344926,344926,344926,344926,344926,344926,344926,344926,344926,344926,344926,344926,344926,344926,344926,344926,344926,344926,344926,344926,344926,344926,344926,344994,345923,345923,345923,345923,346508,346508,347895,347895,347895,347895,347895,347895,347895,347895,347895,347895,347895,347895,347895,347895,348419,348419,348419,348419,348419,348419,348544,349357,349883,350266,350266,350535,350535,350535,350535,350535,350535,350535,350535,350535,350535,350535,351499,351499,351561,351561,351561,351561,351561,351561,351561,351561,351561,351561,351561,351561,351561,351561,351561,351561,351561,351561,351561,351561,351561,351561,351561,351561,351561,351561,351561,351561,351561,351561,351561,351561,351561,351561,351561,351561,351561,351561,351932,351932,351932,351932,351932,351932,351932,351932,351932,351932,351932,352060,352060,352060,352060,352060,352060,352186,352186,352186,352186,352186,352186,352186,352186,352186,352186,352416,352416,352416,352416,352416,352416,352416,352416,352416,352416,352416,352416,352446,352446,352446,352446,352446,352446,352446,352446,352446,352446,352446,352446,352446,352446,352446,352446,352446,352446,352446,352446,352446,352446,352446,352446,352446,352446,352446,352446,352446,352446,352446,352446,352446,352446,352446,352930,352930,354453,355412,355412,355412,355412,355412,355412,355412,355528,355528,356094,356094,356094,356094,356094,356094,356094,356094,356094,356094,356094,356094,356094,356094,356094,356094,356094,356094,356094,356094,356094,356094,356094,356094,356094,356094,356094,356094,356094,356094,356094,356094,356094,356094,356094,356094,356094,356094,356094,356094,356094,356094,356094,356094,356094,356094,356094,356094,356094,356094,356094,356094,356094,356510,356534,356534,356534,356534,356534,356534,356534,356534,357286,357286,357286,357286,357286,357286,357286,358697,359718,359718,359718,360961,361151,362041,362041,362691,362691,362691,362691,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,363525,364330,365343,365343,365343,365606,366188,367302,367302,367302,367302,367302,367302,367302,367302,367302,367302,367302,367302,367302,367302,367302,367302,367302,367302,367302,367302,367302,367302,367302,367302,367302,367302,367748,367748,367748,367748,367748,367826,367826,367826,367826,367826,367826,367826,367826,367826,367826,367826,367826,367826,367826,368186,368186,368186,368186,368186,368186,368186,368186,368186,368186,368186,368186,368186,368186,368186,368186,368186,368186,368186,368186,368186,368186,368186,368186,368186,368186,368186,368186,368186,368186,368186,368186,368186,368186,368186,368186,368186,368186,368186,368186,368186,368186,368186,368186,368186,368186,368186,368186,368386,368386,368386,368386,368386,368386,368386,368386,368386,368386,368386,368437,368437,368437,368723,368723,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,368777,369382,369382,369382,369752,369793,370167,370167,370167,370512,370512,370512,370512,370512,370512,370512,370512,370512,370559,370559,370559,370559,370559,370559,370559,370569,370569,370569,370569,370569,370569,370569,370569,370569,370569,370569,370569,370569,370610,370610,370610,370610,370610,370610,370610,370610,370664,370664,370664,370664,371191,371191,371191,371437,371437,371437,371437,371437,371437,371437,371437,371437,371437,371437,371437,371437,371437,371437,371437,371437,371437,371437,371437,371437,371437,371437,371448,371448,371448,371448,371448,371448,371448,371448,371448,371448,371448,371448,371448,371448,371448,371448,371448,371448,371448,371448,371448,371448,371448,371448,371448,371448,371448,371448,371448,371448,371448,371448,371448,371448,371448,371448,371448,371448,373198,373198,374218,374218,374218,374218,374218,374218,374218,374218,374978,374978,374978,374978,374978,374978,374978,374978,375225,375225,375235,375235,375235,375235,375235,375235,375235,375235,375235,375235,375398,375977,375977,375977,375977,375977,375977,375977,375977,375977,375977,375977,375977,375977,375977,375977,375977,375977,375977,375977,375977,375977,375977,375977,375977,375977,375977,375977,375977,375977,375977,375977,375977,375977,375977,375977,375977,375977,375977,375977,376051,376051,376186,376186,376186,376186,376186,376186,376292,376866,376866,376866,376866,376866,376866,376883,377835,377835,377835,377835,377835,377835,379045,379045,379045,379045,379045,379045,379045,379045,379045,379045,379045,379045,379045,379045,380308,380308,380308,380308,380308,380308,380308,380308,380308,380308,380308,380308,380308,381263,381263,381263,381263,381969,381969,381969,381969,381969,381969,381969,381969,381969,381969,381969,381969,381969,381969,381969,381969,381969,381969,381969,381969,381969,381969,381969,382518,382518,382862,382862,382862,382862,383407,383407,383407,383407,383407,383407,383407,383407,383407,383407,383407,383407,383407,383407,386241,386241,386241,386241,386241,387120,387540,387540,387540,387540,387540,387540,387540,387540,387540,387540,387540,387540,387540,387655,387655,387655,387655,388569,388569,388569,388569,388569,388569,388569,388569,388569,388705,388705,389666,389666,389666,389666,389666,389666,389666,389666,389666,389666,389666,389666,389666,389666,389666,389711,389711,389711,389711,389711,389711,389711,389711,389711,389711,389711,389711,389711,389711,389711,389711,389711,389711,389711,389711,389711,389711,389711,389711,390098,390098,390098,390098,390098,390098,390098,390098,390119,390306,390306,391286,391286,391286,391286,391745,391745,391745,391745,391745,391745,391745,391745,391745,391745,391745,391745,391745,391745,391745,391745,391745,391745,391745,391745,391745,392310,392497,392527,392740,392740,392740,393342,393342,393342,393342,393342,393342,393342,393342,393342,393342,393342,393342,393342,393342,393342,393342,393342,393717,394025,394025,394025,394025,395837,395837,395837,395837,395837,395837,395837,395837,396020,396866,396866,398550,398550,398550,398550,398550,398550,398550,398550,398550,398550,398550,398550,398550,398550,398550,398550,398550,398550,398550,398724,398724,398724,398724,398724,398724,398724,398724,398724,398724,398724,398724,398724,398724,398724,398724,398724,398724,399127,399127,399127,399127,399127,399127,399127,399127,399127,399127,399293,399293,399293,399293,399293,399293,399293,399293,399293,399293,399293,399293,399293,399293,399293,399293,399813,400324,400324,400324,400324,400580,400580,400580,400580,400580,400580,400580,400580,400580,400580,400580,400580,400580,400580,400746,400746,400746,400746,401223,401223,401223,401774,401774,401774,401774,404616,404756,404756,404756,404756,405162,405162,405162,405162,405162,405162,405162,405162,405162,405162,405162,405642,405642,406070,406070,406709,406709,406709,407451,407451,407451,407451,407451,407451,407451,407451,407451,407451,407451,407451,407451,407451,407451,407451,407451,407451,407451,407451,407451,407451,407451,407451,407451,407451,407451,407451,407451,407451,407451,407451,407451,407451,407451,407451,407451,407451,407451,407451,407451,407451,407451,407451,407451,407451,407451,407451,407451,407451,407451,407451,407451,407451,407451,407451,407451,407451,407451,407451,407451,407451,407451,407451,407451,407451,407451,407451,407451,407598,407598,407598,407598,407598,407598,407598,407859,407859,407859,407859,407859,407859,407859,407859,407859,407859,407859,407859,407859,407859,407859,407859,407859,407859,407859,407859,407859,407859,407859,407859,407859,407859,407859,407859,407859,407859,407859,407859,407859,407859,407859,407859,407859,407859,407859,407859,407859,407859,407859,407859,407859,407859,407859,407859,407859,407859,407859,407859,408409,408409,408536,408536,408536,408536,408536,408536,408536,408536,408536,408536,408536,408536,408536,408536,408536,408536,408536,408536,408536,408536,408536,408536,408536,408536,408536,408997,408997,408997,408997,408997,408997,408997,408997,408997,408997,408997,408997,408997,408997,408997,408997,408997,408997,408997,408997,408997,408997,408997,408997,408997,408997,408997,408997,408997,408997,408997,408997,408997,408997,408997,408997,408997,408997,408997,408997,408997,408997,408997,408997,408997,409756,409756,409756,409756,409756,409756,409756,409756,409756,409756,409756,409760,409760,409760,409760,409760,409760,409760,409760,409760,409760,409760,409760,409760,409760,409760,409937,409937,409937,410150,410150,410150,410913,411347,411988,411988,411988,412362,412362,412362,412362,412362,413026,413147,413147,413174,413174,413174,413174,413174,413174,413174,413174,413174,414379,414379,414379,415683,415683,415683,415683,415683,416466,416466,416904,416904,416904,416904,416904,416904,416904,416904,416904,416904,417307,417307,417307,417307,417307,417307,417307,417307,417307,417307,417307,417307,417307,417307,417307,417307,417307,417307,417307,417307,417307,417307,417307,417307,417307,417307,417307,417307,417307,417307,417307,417307,417307,417307,417307,417307,417307,417307,417307,417307,417307,417307,417307,417885,417885,417885,417885,417885,417885,417885,417885,417885,418186,418186,418366,418366,418366,418366,418366,418366,418366,418366,418495,418495,418495,418495,418495,418495,418495,418495,418495,418495,418495,418495,418495,419835,419835,419835,419835,419835,419835,420088,420088,420088,420088,420088,420088,420088,420088,420132,420132,420132,420132,420132,420132,420132,420132,420493,420493,420493,420493,420493,420493,420493,420493,420493,420493,420493,420493,420493,420493,420493,420493,420493,420493,420493,420493,420493,420493,420493,420493,420493,420493,420708,420708,420708,420708,420708,420708,420708,420708,420708,420708,420708,420708,420708,420708,420708,420708,420708,420708,420708,420708,420708,420708,420708,420708,420708,420708,420708,420708,420714,420714,420714,420714,420714,420714,420714,420714,421576,421576,421576,421576,421576,421576,421576,421726,422609,422609,422609,422609,422609,422609,422609,422609,422609,422609,422609,422617,422617,422617,422617,422617,422617,422617,422617,422617,422617,422617,422617,422617,422617,422617,422617,422617,422617,423592,423592,423592,423592,423592,423592,423592,423592,423592,423592,423592,423592,423592,423799,423799,423799,423799,423799,423799,423799,423799,423799,423799,423799,423799,423799,423799,423799,423799,423799,423799,423799,423799,423799,424553,424553,424553,424553,424553,424553,424553,424553,424553,424954,424954,424954,425309,425788,425788,425788,425788,425788,425788,426036,426080,426080,426080,426080,426080,427056,427056,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427424,427593,427593,427593,427866,427866,427866,427866,427866,427866,427866,427866,427866,427866,427866,427866,427866,427866,427866,427866,427866,427866,427866,427866,427866,427866,427866,427866,427866,427866,427866,427866,427866,427866,427866,428328,428578,428919,428919,428919,429189,429189,429189,429189,429968,429968,429968,429968,429968,429968,429968,429968,429968,429968,429968,429968,429968,429968,429968,429968,429997,429997,429997,429997,429997,429997,430699,430699,430699,430699,430699,430699,430699,430699,430699,430699,430699,430699,430699,430930,430930,430930,430930,430930,430930,431535,431535,431535,431535,431535,431535,431535,431535,431535,431847,431847,431847,431847,431847,431847,431847,431847,431847,431847,431847,431847,431847,431847,431847,431847,431847,431847,431847,431847,431847,431847,431847,431847,431847,431847,431847,432106,432106,433137,433137,433351,433351,433351,433351,433351,433351,433351,433351,433351,433351,433351,433351,433351,433351,433351,433351,433351,433351,433351,433351,433351,433351,433351,433351,433351,433351,433351,433351,433351,433351,433351,433351,433351,433351,433351,433351,433351,433351,433351,433351,433351,433351,433351,433351,433351,433351,433425,433425,433425,433425,433425,433568,433568,433568,433568,433613,433613,433613,433613,433613,433613,433613,433613,433613,433613,433613,433613,433613,433613,433613,433613,433613,433613,433613,433613,433613,433613,433613,433613,433613,433613,433613,433613,433613,433613,433613,433613,433613,433613,433613,433613,433613,433613,433613,433613,433613,433613,433613,433613,433613,433613,433613,433613,433613,433613,433613,433613,433613,433613,433613,433613,433613,434479,434479,434531,434531,434531,434628,434628,434628,434628,434628,434628,434628,434628,434628,434628,434628,434628,434628,434628,434628,434628,434628,434628,434628,434628,434628,434628,434628,434628,434628,434628,434628,434628,434939,435109,435109,435109,435109,435109,435109,435109,435109,436609,436609,436609,436609,436609,436609,436609,436609,436609,436609,436609,436609,436609,436609,436609,436609,436609,436609,436609,436609,436609,437437,437437,437437,437437,437437,437437,437437,437437,437437,437748,437748,437748,437748,437748,437748,437748,437748,437748,437748,437780,437780,437780,437780,437780,437780,437780,437780,438477,438751,438851,438851,438851,438851,438942,439867,439867,439914,439914,439914,440154,440154,440154,441949,441949,441949,441949,441949,441949,441964,442125,442125,442125,442125,442490,442490,442490,442490,442490,442490,443209,444023,444198,444847,444847,445466,445466,445466,445466,445466,445466,445466,445466,445466,445466,445466,445466,445466,445466,445902,446317,447494,447494,447494,447494,447494,447755,447755,447891,447891,447891,447891,447891,447891,447891,447891,447891,447891,447891,447891,447891,447891,447891,447891,447891,447891,447891,447891,447891,447891,447891,447891,448383,448383,449501,449501,449501,449501,449501,449501,449501,449501,449501,450140,450140,450140,450140,450140,450140,450140,450140,450140,450140,450140,450140,450140,450140,450140,450140,450140,450140,450140,450140,450140,450140,450140,450140,450140,450140,450140,450140,450140,450140,450140,450140,450140,450140,450140,450140,451505,451601,451601,451601,451627,451627,451627,451627,451627,451627,451627,451627,451627,451627,451627,451627,451627,451687,451687,451687,451687,451687,451687,451687,451687,451687,451687,451729,452095,452095,452106,452106,452106,452106,452106,452106,452106,452106,452106,452106,452106,452106,452106,452480,452480,452480,452480,452480,453155,453456,453456,453456,453557,453557,453557,453557,453772,453772,453772,453772,453772,453772,453772,453772,453772,453772,453772,453772,453772,453772,453772,453772,453772,453772,453772,453772,453772,453772,453772,453772,453772,453772,453772,453772,453772,453772,453772,453772,453772,453772,453772,453772,453772,453772,453772,453772,453772,453772,453772,453772,453772,453772,453772,453772,453772,453772,453772,454044,454165,454165,454165,454165,454165,454165,454165,454165,454165,454165,454165,454165,454165,454165,454165,454165,454165,454165,454165,454165,454165,454165,454165,454165,454165,454165,454165,454165,454165,454165,454165,454165,454165,454165,454165,454165,454165,454165,454165,454165,454165,454236,454236,454236,454236,454236,454490,454490,454490,454697,454697,455266,455266,455266,455266,455333,455333,455333,455333,455333,455333,455596,455596,455681,455681,455681,455681,455681,455681,455681,455681,455681,455681,455681,455681,455681,455681,455681,455681,457196,457201,457201,457201,457201,457201,457201,457201,457201,457201,457201,457201,457766,457766,457766,457766,457766,457766,457766,457766,457766,457766,457766,457766,457766,457766,457766,457766,457766,457766,457766,457766,457766,457766,457766,457766,457766,457766,457766,457766,457766,457766,457766,457766,457766,457766,457766,457766,457766,457766,457766,457995,458021,458749,458749,458749,458749,458749,458749,458749,458749,458749,458749,458749,458749,458749,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,458774,459039,459039,459039,459039,459578,459578,459578,459578,459578,459578,459578,459578,459578,459578,459578,459578,459578,459578,459578,459578,459578,459578,459578,459578,459578,459578,459578,459578,459578,459578,459578,459578,459578,459578,459578,459578,460237,460237,460237,460237,460237,460237,460237,460237,460237,460237,460237,460237,460237,460237,460237,460237,460834,460834,460834,461629,461629,461629,461629,461629,461629,461629,461629,461629,461698,461698,461698,461698,461698,461698,461698,462397,462397,462585,462585,462585,462585,463485,463485,463485,463485,463489,463503,463503,463503,463542,463542,464425,464425,465306,465306,465306,465634,465634,465634,465634,465634,465634,465634,465634,465816,465816,465816,465816,465816,465816,465816,465816,465816,465816,465816,465816,465816,466085,466978,466978,466978,466978,466978,466978,466978,466978,466978,466978,466978,466978,466978,466978,466978,466978,466978,466978,466978,466978,466978,466978,466978,466978,466978,466978,466978,466978,466978,466978,466978,466978,466978,466978,466978,466978,466978,466978,466978,466978,466978,466978,466978,466978,466978,466978,466978,466978,466978,466978,466978,466978,466978,466978,466978,467020,468602,469099,469099,469099,470547,470547,470547,470547,470547,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,471559,472174,472297,472599,472599,472947,472947,472947,472947,472947,472947,472947,472947,472947,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,473611,474727,474727,474727,474727,474727,474727,474727,474727,475860,475860,475860,475860,475860,475860,475860,475860,475860,475860,475860,476329,476841,477166,477166,477166,477166,477166,477166,477264,477264,477264,477264,477264,477264,477264,477264,477264,477264,477264,477264,477264,477264,477264,477461,477461,477461,477461,477461,477461,477461,477461,477461,477461,477461,477472,478043,478043,478043,478043,478043,478043,478043,478043,478043,478043,478043,478043,478043,478043,478043,478043,478043,478043,478043,478043,478043,478043,478043,478043,478043,478043,478043,478043,478043,478043,478043,478043,478043,478043,478043,478043,478043,478043,478043,478043,478043,478043,478043,478043,478043,478043,478043,478043,478043,478043,478043,478043,478043,478043,478043,478043,478043,478043,478043,478043,478043,478043,478043,478043,478043,478043,478043,478043,478043,478043,478043,478043,478043,478043,478043,478043,478254,478254,478254,478254,478254,478254,478254,478254,478254,478254,478254,478254,478254,478254,478254,478254,478254,478254,478254,478254,478254,478254,478254,478254,478632,478632,478672,478672,478829,479489,479489,479489,479489,479489,479489,479489,479489,479489,479489,479489,479489,479489,479489,479489,479539,479840,480239,480239,480341,480341,480341,480341,480341,480341,480341,480584,480584,480819,480819,480819,480819,480819,480819,480819,480819,480819,480819,480819,480819,480819,480819,480819,480819,480819,480819,480819,480819,480819,480819,480819,480819,480819,480819,480819,480819,480819,480819,480819,480819,480819,480819,480819,480819,480819,480819,480819,480819,480819,480819,480819,480819,480819,480895,480895,480895,480895,480895,480895,480895,480895,480895,480895,480895,481161,481161,481161,481161,481161,481161,481161,481161,481161,481161,481161,481161,481161,481161,481161,481161,481161,481161,481161,481161,481161,481161,481161,481161,481161,481161,481161,481161,481161,481161,481161,481161,481161,481161,481161,481161,481161,481161,481161,481161,481161,481161,481161,481161,481161,481161,481161,481161,481161,481161,481161,481161,481161,481161,481161,481161,481161,481161,481242,481242,481242,481242,481242,481242,481242,481242,481242,481242,481242,481242,481242,481242,481242,481242,481242,481242,481242,481242,481242,481242,481242,481242,481242,481242,481242,481242,481242,481242,481242,481242,481242,481242,481242,481242,481242,481242,481242,481242,481242,481242,481242,481242,481242,481242,481242,481242,481242,481242,481242,481242,481242,481242,481242,481242,481242,481242,481242,481242,481242,481242,481242,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481802,481803,481803,481803,481803,481803,481803,481806,481806,481806,481806,481806,481848,481848,481989,481989,481989,481989,481989,481989,481989,482230,482230,482735,482870,482870,484389,484389,484770,485339,485339,485339,485339,485339,485339,485339,485339,485339,485339,485339,485339,485339,485339,485339,485339,485339,485339,485339,485339,485339,485339,485339,485339,485339,485339,485339,485339,485434,485434,485434,485434,485434,485434,485434,485434,485434,485434,485434,485434,485434,485434,485434,485434,485434,485434,485434,485434,485434,485434,485434,485434,485434,485434,485434,485890,485890,485890,485890,485890,485890,485890,485890,485890,485890,485890,485890,485890,485890,485890,485890,485890,485936,485936,485936,485936,485936,485936,485936,485936,485936,485936,485936,485936,485936,485936,485936,485936,485936,485936,485936,485936,485936,485936,485936,485936,485936,485936,485936,485936,485936,485936,485936,485936,485936,485936,485936,485936,485936,485936,485936,485936,485936,485936,485936,486548,486548,486548,486548,486548,486548,487017,487017,487017,487017,487017,487017,487017,487017,487017,487017,487017,487195,487195,487195,487195,487195,487195,487195,487195,487195,487195,487195,487195,487195,487195,487195,487195,487195,487195,487195,487195,487195,487195,487195,487195,487195,487195,487195,487292,487292,487383,487383,487383,487383,487383,487383,487383,487383,487383,487383,487383,487383,487383,487383,487383,487383,487383,487383,487383,487383,487383,487383,487383,487383,487383,487383,487383,487383,487383,487383,487383,487383,487397,487397,487513,488147,488147,488173,488173,488173,488173,488173,488173,488173,488173,488469,488480,488480,488480,488649,488649,488649,489091,489091,489124,489477,489477,489477,489477,489477,489477,489477,489477,489477,489477,489477,489477,489477,489477,489477,489477,489477,489477,489477,489477,489477,489477,489477,489477,489477,489477,489477,489477,489477,489477,489477,489477,489477,489477,489477,489477,489477,489477,489477,489477,489477,489477,489477,489477,489477,489477,489477,489477,489477,489477,489477,489477,489477,489477,489477,489477,489477,489477,489477,489477,489543,489543,489543,489543,489543,489543,489543,489543,489543,489543,490086,490086,490086,490086,490086,490086,490086,490086,490086,490086,490086,490086,490086,490086,490086,490086,490086,490086,490086,490086,490086,490086,490086,490086,490086,490086,490086,490086,490086,490086,490086,490086,490086,490324,490324,490324,490324,490324,490668,490668,490668,490668,490668,490668,490668,490668,490668,490668,490668,490668,490668,490668,490668,490668,491144,491436,491436,491531,491531,491585,491585,491585,491760,492053,492053,492053,492432,492667,492667,492667,492667,493611,493611,493611,493611,493611,493611,493611,493611,493611,493611,493611,493611,493611,493611,493611,493611,493611,493611,493611,493611,493611,493611,493611,493611,493611,493611,493611,494983,494983,494983,495083,495083,495083,495814,495814,495898,495898,495898,495898,496345,496384,496384,496682,496682,496682,496682,496682,496682,496682,496682,496682,496682,496682,496682,496682,496682,496682,496682,496682,496682,496682,496682,496682,497141,497141,497141,497141,497281,497281,497281,497281,497281,497281,497281,497281,497281,498403,498403,498403,498403,498403,498403,498403,498403,498403,498403,498403,498403,498403,498403,498403,498403,498403,498403,498403,498403,498403,498403,498403,498403,498403,498403,498456,498456,498456,498456,498456,498456,498456,498456,498456,498456,498456,498456,498456,498456,498456,498492,498492,498492,498492,498492,498492,498492,498492,498492,498694,498694,498694,498772,498772,498772,498772,498772,498772,498772,498772,498772,498772,498772,498772,498772,498772,498772,498772,498772,498772,498772,498772,498772,498772,498772,498772,498772,498772,498772,498772,498772,498772,498772,498772,498772,498772,498772,498772,498772,498772,498772,498772,498772,498772,498772,498772,498772,498772,498772,498772,498772,498772,498772,498772,498772,498772,498772,498772,498772,498772,498772,499918,500250,500250,500250,500250,500250,500250,500250,500250,500250,500250,500250,500365,500365,500365,500365,500365,500365,500519,500519,500519,500519,500519,500519,500519,500519,500519,500519,500519,500519,500519,500519,500519,501163,501163,501163,501163,501163,501163,501163,501392,501392,501392,501392,501392,501598,501598,501598,501598,501598,501598,501598,501598,501598,501598,501598,501598,501598,501598,501598,501598,501598,501598,501598,501598,501598,501598,501598,501598,501598,501598,501598,501598,501598,501598,501598,501598,501598,501598,502011,502011,502011,502011,502011,502011,502011,502011,502011,502011,502011,502011,502011,502011,502011,502011,502011,502011,502011,502011,502011,502011,502011,502011,502011,502011,502096,502096,502096,502612,502671,502671,502671,503041,503041,503041,503857,503963,504057,504057,504057,504057,504057,504057,504057,504057,504057,504057,504057,504057,504057,504057,504057,504057,504057,504057,504057,504057,504057,504206,504206,504206,504206,504206,504206,504206,504206,504313,504313,504313,505564,505564,505564,506087,506087,506151,506151,506151,506151,506151,506151,506151,506151,506151,506151,506151,506151,506151,506151,506151,506151,506151,506151,506151,506151,506151,506151,506151,506151,506151,506151,506151,506151,506151,506151,506151,506151,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506574,506644,506644,506644,506644,506644,506644,506644,506644,506644,506644,506644,506644,506644,506644,506644,506644,506644,506644,506644,506644,506644,506644,506644,506644,506644,506644,506644,506644,508504,508504,508504,508599,508763,508763,508763,508763,508763,508763,508766,509560,509560,509560,509560,509621,509621,509621,509621,509949,509949,509949,509949,509949,509949,509949,509973,509973,509973,509973,509973,509973,509973,509973,509973,509973,509973,509973,509973,509973,509973,509973,509973,509973,510102,510624,510628,511091,511337,511720,511720,511720,511720,511720,511720,511720,512044,512584,512584,512584,512584,512584,512584,512584,512584,512584,512584,512584,512584,512584,512840,512840,513113,513113,513113,513113,513573,513573,514044,514126,514126,514126,514126,514126,514126,514126,514126,514126,514126,514126,514126,514126,514494,514494,514494,514494,514494,514494,514494,514494,514494,514494,514494,514494,514495,514495,514495,514495,514689,514689,514689,515275,515275,515275,515275,515275,515275,515275,515275,515275,515275,515275,515275,515275,515275,515275,515275,515275,515275,515275,516409,516409,516409,516409,516409,516409,516409,516409,516409,516409,516461,516461,516461,516461,516461,516461,516461,516461,516461,516461,516461,516461,516461,516801,516801,516801,517234,517860,517860,517860,517860,517860,517860,517860,517860,517860,517860,517860,517860,517860,517860,517860,517860,517860,517860,517860,517860,518279,518279,518279,518279,518279,518279,518279,518279,518279,518279,518279,519542,519542,519542,519542,519542,519542,519542,521462,521462,521462,521462,521462,521462,521462,521462,521462,521462,521462,521462,521462,521462,521462,521462,521462,521462,521462,521462,521462,521462,521462,521462,521462,521462,521462,521462,521462,521462,521462,521462,521462,521462,521462,521462,521462,521462,521462,521462,521532,521532,521532,521532,521532,521532,521939,521939,521939,521939,521939,521939,521939,521939,521939,521939,521939,521939,521939,521939,521939,521939,521939,521939,521939,521939,521939,521939,521939,521939,521939,521939,521939,521939,521939,521939,521939,521939,521939,521939,521939,521939,521939,521939,521939,521939,521939,521939,521939,521939,521939,521939,521939,521939,521939,521939,521939,521939,521939,521939,521939,521939,521939,521939,521939,521939,521939,521939,521939,521939,521939,521939,522227,522227,522227,522227,522227,522227,522227,522227,522227,522227,522227,522227,522227,522227,522227,522227,522227,522227,522227,522227,522227,522227,522227,522227,522227,522227,522227,522227,522227,522227,522227,522227,522227,522227,522227,522227,522227,522227,522227,522227,522227,522227,522227,522227,522227,522227,522227,522227,522227,522227,522227,522227,522227,522227,522227,522939,522939,522939,522939,523215,523514,523514,523533,524029,524029,524434,524434,524434,524434,524434,524434,524434,524434,524434,524434,524434,524434,524434,524434,524434,524434,524434,524434,524434,524434,524434,524434,524434,524434,524434,524434,524434,524434,524434,524434,524434,524434,524434,524434,524434,524434,524434,524434,524434,524434,524434,524434,524434,524434,524434,524434,524434,524434,524434,524434,524434,524434,524434,524434,524434,524434,524434,524434,524434,524434,524434,524434,524434,524434,524434,524434,524434,524434,524434,524434,524434,524434,524440,524440,524440,524447,524957,524957,524957,524957,524957,524957,524957,525083,525091,525091,525091,525091,525091,525091,525216,525216,525216,525216,525216,525216,525216,525216,525219,525219,525219,525219,525219,525219,525219,525219,525219,525219,525219,525478,525478,525478,525478,525478,525478,525478,525478,525478,525557,525557,525557,525557,525557,525557,525557,525557,525557,525557,525557,525557,525557,525557,525557,525557,525557,525557,525557,525557,525557,525557,525557,525557,525557,525557,525557,525557,525557,525557,525557,525557,525557,525557,525557,525557,525557,525557,525557,525766,525766,525766,525766,525766,525766,525766,525766,525766,525766,525766,525766,525766,525766,525766,525766,525766,525766,525766,525766,525766,525766,525766,525766,525766,525766,525766,525766,525766,525766,525766,525766,525766,525766,525766,525926,525926,525926,526639,526639,526639,526639,526639,526639,526639,526639,526639,526639,526639,527106,527106,527106,527106,527106,527106,527106,527106,527106,527106,527106,527106,527106,527106,527284,527284,527284,527284,527284,527284,527388,527388,527388,527388,527388,527388,527388,527388,527388,527388,527416,527416,527416,527416,527416,527416,527416,527416,527416,527416,527416,527416,527416,527416,527416,527416,527416,527416,527416,527416,527416,527416,527416,527831,527831,527831,527831,527831,527831,527831,527831,527831,527831,527831,527831,527831,527831,527831,527831,527831,527831,527869,528721,529107,529107,529362,529362,529362,529531,530145,530145,530145,530686,530686,530686,530686,530686,530686,530686,530686,530686,530686,530686,530686,530803,530803,530803,530803,530803,530803,530803,530803,530803,530803,530803,531413,531413,531413,531413,531413,531413,531413,531413,532189,532189,532600,532600,532600,533567,533567,533582,533582,533622,534335,534335,534335,534335,534335,534335,534335,534335,534335,534335,534335,534335,534335,534335,534335,534335,534335,534335,534335,534335,534335,534335,534335,534335,534335,534335,534335,534335,534335,534335,534335,534335,534335,534335,534335,534335,534335,534335,534335,534335,534335,534335,534335,534335,534335,534335,534335,534335,534335,534335,534335,534335,534335,534335,534335,534335,534335,534335,534335,534335,534335,534335,534335,534335,534335,534335,534335,534335,534335,534335,534335,534335,534335,534335,534335,534335,534335,534335,534335,534432,534432,534432,534432,534432,534432,534432,534432,534432,534432,534432,534432,534432,534432,534432,534432,534432,534432,534432,534432,534432,534432,534432,534432,534432,534432,534432,534432,535062,535062,535062,535062,535062,535062,535062,535062,535062,535062,535062,535062,535062,535062,535062,535062,535062,535062,535062,535062,535062,535062,535062,535062,535112,535112,535112,535112,535112,535112,535112,535112,535128,535806,536394,536394,536394,536394,536394,536415,536415,536415,536415,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536499,536655,536655,536655,536655,536655,536655,536655,536655,536739,536739,536739,536739,536739,536739,536739,536739,536739,536739,536739,537379,537379,537379,537379,538811,538811,538876,538876,538876,538876,538876,538876,538876,539146,539309,539309,540655,540925,540925,540925,542154,542154,542154,542154,542154,542154,542229,542229,542229,542229,542229,542229,542229,542229,542229,542229,542229,542229,542229,542229,542229,542229,542229,542229,542229,542229,542229,542229,542229,542229,542229,542229,542229,542229,542229,542229,542229,542229,542229,542229,542229,542229,542229,542229,542229,542229,542229,542229,542229,542229,542229,542229,542229,542229,542238,542238,542238,542238,542803,542963,542963,542963,542963,542963,542963,542963,542963,542963,542963,542963,542963,542963,543124,543124,543124,543427,543427,543427,543685,543685,543685,543685,543685,543685,543812,543812,543812,543812,543812,543812,543812,543828,543828,543828,543828,543828,543828,543828,544819,544819,544819,544819,544819,544819,544973,544973,546493,546493,546493,546493,546493,547204,547204,547204,547204,547204,547204,547204,547204,547204,547204,547204,547204,547260,547260,547260,547260,547260,547260,547260,547260,547649,549683,549751,549751,549751,549751,549751,549751,549751,549751,549751,549914,549914,549914,549914,549914,549914,549914,549914,549914,549914,549914,549914,549914,549914,549914,549914,549914,549914,549914,549914,549914,549914,549914,549914,549914,549914,549914,549914,549914,549914,549914,549914,549914,549914,549914,549914,549914,549914,549914,549914,549914,549914,549914,549914,550168,550374,550374,550374,551370,551370,551370,551405,551405,551405,551405,551441,551441,551441,551441,551441,551441,551441,551441,551441,551441,551441,551441,551441,551441,551441,551441,551441,551441,551441,551441,551441,551441,551441,551441,551441,551441,551441,551441,551441,551441,551441,551441,551441,551441,551441,551441,551441,551500,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552167,552207,552207,552207,552207,552207,552207,552207,552207,552758,552932,552932,552932,552932,554223,554223,554223,554223,554223,554223,554223,554223,554223,554223,554223,554223,554223,554223,554223,554333,554375,554375,554375,554657,554657,554657,554657,554657,554657,554657,554657,554657,554657,554657,554657,555078,555078,555078,555078,555078,555078,555078,555078,555078,555078,555078,555078,555078,555078,555078,555078,555078,555078,555078,555078,555078,555078,555078,555078,555078,555078,555078,555078,555078,555078,555078,555078,555078,555078,555633,555633,555773,555773,555773,555773,555773,555773,555773,555773,555773,555773,555773,555773,555773,555773,555773,555773,555773,555773,555773,555773,555773,555773,555773,555773,555773,555773,555773,555773,555773,555773,556294,556294,556294,556294,556294,556294,556294,556294,556294,556294,556604,557079,557079,557079,557079,557080,557080,557080,557080,557080,557080,557080,557080,557080,557080,557080,557080,557080,557080,557080,557080,557080,557080,557080,557080,557080,557080,557356,557399,557399,557399,557399,557399,557399,557399,557399,557399,557399,557399,557399,557399,557567,557567,557567,557567,557567,557567,557567,557567,557567,557567,557567,557567,557567,557567,557567,557567,557567,557567,557567,557567,557567,557567,557567,557769,557769,557769,557769,557769,557769,557769,557769,557769,557769,557769,557769,557769,557769,557769,557769,557769,557769,558094,558094,558094,558094,558609,558609,558609,558690,558690,558690,558690,558697,558697,558697,558697,558697,558697,558697,558697,558697,558697,558697,558697,558697,558697,558697,558697,558697,558697,558697,558697,558697,558697,558697,558697,558697,558697,558697,558697,558697,558697,558697,558697,558830,558830,558830,558830,558830,558830,558830,558830,558830,558830,558830,558830,558830,558830,558830,558830,558832,558832,558832,558832,558832,558832,558832,558832,558832,558832,558832,558832,558832,558832,558832,558832,559119,559119,559119,559119,559119,559119,559119,559264,559264,559264,559264,559264,559264,559264,559264,559264,559264,559264,559264,559264,559264,559264,559264,559264,559264,559264,559264,559264,559264,559264,559264,559264,559264,559264,559264,559264,559264,559264,559264,559337,559337,559337,559337,559337,559337,559337,559337,559337,559337,559337,559337,559337,559337,559337,559337,559337,559337,559337,559407,559407,559681,559965,559965,560051,560051,560051,560051,560051,560339,560339,560339,560339,560339,560339,561160,561160,561160,561160,561160,561160,561160,561160,561160,561160,561160,561160,561160,561160,561160,561160,561160,561160,561160,561160,561160,561160,561160,561160,561160,561160,561160,561160,561160,561160,561160,561160,561160,561160,561160,561160,561160,561160,561160,561160,561160,561160,561160,561160,561160,561160,561160,561160,561160,561160,561160,561160,561160,561160,561160,561160,561160,561160,561160,561160,561160,561160,561737,561737,561737,561737,561737,561737,561737,561737,561737,561737,561737,561737,561737,561737,561737,561737,561737,561737,561737,561737,561737,562003,562003,562003,562003,562003,562003,562003,562003,562003,562003,562003,562003,562003,562003,562003,562003,562003,562003,562003,562003,562003,562003,562003,562003,562003,562003,562003,562003,562003,562003,562003,562003,562003,562003,562003,562003,562003,562003,562003,562003,562003,562003,562003,562519,562519,562519,562519,562519,562519,562519,562521,562521,562521,562521,562521,562521,562521,563714,563714,563776,563776,563776,563776,563776,565094,565094,565094,565094,565094,565094,565094,565140,565140,566321,566664,566938,566938,566938,566938,566938,566938,566938,566938,566938,566938,566938,566938,566938,566938,566938,566938,566938,566938,566938,566938,566938,566938,566938,566938,566938,566938,566938,566938,566938,566938,566938,566938,566938,566938,566938,566938,566938,566938,566938,566938,566938,566938,566938,566938,566938,566938,566938,566938,566938,566938,566938,566938,566938,566938,566938,566938,566938,566938,566938,566938,566938,566938,566938,566938,566938,566938,566938,568417,568417,568417,568417,568417,568417,568417,568417,568417,568417,568417,568417,568417,568417,568417,568417,568417,568417,568417,568417,568567,568567,568567,568567,568567,568828,568828,568828,568828,568828,568896,569061,569061,569061,569061,569061,569061,569061,569061,569061,569061,569061,569061,569061,569061,569061,569061,569061,569061,569061,569061,569061,569061,569061,569061,569061,569061,569061,569061,569061,569061,569061,569061,569358,569358,569358,569358,569358,569358,569358,569358,569358,569358,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,569803,570747,571087,571087,571087,571087,571087,571087,571087,571087,571087,571087,571087,571087,571087,571087,571087,571087,571087,571087,571087,571087,571087,571087,571087,571087,571087,571087,571087,571087,571087,571087,571087,571087,571087,571087,571087,571087,571087,571087,571087,571087,571087,571087,571087,571087,571087,571087,571087,571087,571087,571087,571087,571087,571087,571087,571087,571087,571087,571087,571087,571087,571087,571087,571087,571087,571087,571087,571087,571087,571087,571087,571087,571522,571522,571522,571522,571522,571522,571522,571522,571522,571522,571935,571935,571935,571935,572719,572719,572719,572719,572719,572719,572719,572719,572719,572719,572719,572719,572719,572719,572719,572719,572838,572838,572838,572838,574123,574123,574373,574373,574373,574373,574373,574373,574373,574373,574373,574562,575574,575574,575574,575574,575574,575574,575574,575574,575574,576059,576059,576059,576059,576059,576059,576059,576102,576102,576102,576102,576102,576726,576726,576726,576743,576743,576789,576789,576789,576789,576789,576789,576789,577097,577097,577539,577539,578742,578742,578742,578742,578742,578742,578742,578742,578742,578742,578742,578742,579325,580794,580794,580794,580794,580794,580794,580794,580794,580794,580794,580794,580794,580794,580794,580794,580794,580794,580794,580794,580794,580794,580794,580794,580794,580794,580794,581822,581822,581822,581822,581822,581946,581946,582295,582295,582906,582906,582906,582906,582934,583059,583059,583092,583092,583092,583092,583092,583092,583092,583092,583092,583159,583159,583159,583383,583383,583383,583383,583383,583383,583383,583383,583383,583383,583383,583383,583383,583383,583383,583383,583383,583383,583383,583383,583938,583938,583938,583938,583938,583938,583938,583938,583938,583938,583938,583938,583938,584299,584299,584545,584545,584928,584928,584928,584928,584928,584928,584928,584928,584928,584928,584928,584928,584928,584928,584928,584928,584928,584928,584928,584928,584928,584928,584928,584928,584928,584928,584928,584928,584928,584928,584928,584928,584928,584928,584928,584928,584928,584928,584928,585612,585612,585612,585612,585612,585803,585859,585859,585859,585859,585859,585859,585859,585859,585859,585859,585859,585859,585859,585859,585859,585859,585859,585859,585859,585859,585859,585859,585859,585859,585859,586003,586003,586003,586121,586185,586185,586185,586405,586489,586489,586489,586489,586489,586489,586489,586489,586489,586489,586489,586489,586489,586489,586489,586489,586489,586489,586489,586489,586489,586489,586489,586489,586489,586489,586489,586489,586489,586489,586489,586489,586489,586489,586489,586489,586489,586489,586489,586489,586489,586489,586489,586489,586489,586489,586489,586489,586489,586783,586783,586783,586783,586783,586783,586783,586783,586783,586783,586783,586783,586783,586783,586783,586783,586783,586783,586783,586833,586833,586925,587089,587089,588270,588270,588270,588270,588270,588270,588270,588270,588270,588270,588270,588270,588270,588270,588270,588270,588270,588270,588270,588270,588270,588270,588270,588270,588270,588270,588270,588270,588270,588270,588270,588270,588270,588270,588270,588270,588270,588270,588270,588270,588584,588584,588584,588584,588584,588584,588584,588584,588584,588584,588584,588584,588584,588932,588932,588932,588932,589012,589012,589012,589012,589012,589012,589012,589012,589012,589012,589012,589012,589012,589012,589012,589012,589012,589012,589036,589154,589458,589458,589528,589625,589625,589625,589625,589625,589625,589625,589625,589625,589625,589625,589890,589956,590468,590771,590771,590771,590771,590771,590771,590771,590771,590771,590771,590771,590771,590771,590771,590771,590771,590771,590771,590771,590771,590771,590771,590771,590771,590771,590771,590771,590771,590771,590771,590771,590771,590771,590771,590771,590771,590771,590771,590771,590836,590836,590836,590836,590836,590836,590836,591367,591367,591367,591367,591367,591367,591613,591613,591613,591613,591613,591613,591613,591613,591613,591613,591644,591644,591644,591644,591644,591644,591644,591644,592067,592067,592067,592067,592067,592067,592067,592067,592067,592067,592067,592067,592067,592067,592067,592067,592067,592445,592464,592464,592464,592464,592464,592464,592464,592464,592464,592464,592464,592464,592464,592464,592464,592464,592464,592464,592464,592464,592464,592464,592464,592464,592464,592464,592464,592464,592464,592538,592538,592538,592538,592538,592538,592538,592538,592538,592538,592538,592538,592538,592538,592538,592538,592597,592597,592597,592597,592597,592597,592597,592613,592727,592727,592727,592727,592727,592727,592727,592727,592727,592727,592727,592727,592727,592727,592727,592727,592727,592727,592727,592727,592727,592727,592727,592727,592727,592727,592727,592727,592727,592727,592727,592727,592727,592727,592727,592727,592727,592727,592998,592998,592998,592998,592998,593003,593003,593003,593003,593295,593295,594618,594618,594618,594618,594618,594618,594618,594618,594618,595015,595015,595015,595405,595405,595607,595607,595656,595699,595699,595699,595699,595699,595699,595699,595699,595699,595699,595699,595699,595699,595699,595699,595699,595699,595699,595699,595927,595927,595927,596475,596475,597204,597204,597204,597204,597204,597204,597204,597204,597359,597608,597731,597731,597731,597731,597731,597731,597731,597731,597731,597731,597731,597731,597731,597731,597731,598133,598133,598133,598133,598133,598133,598133,598133,598133,598133,598133,598133,598133,598133,598133,598133,598133,598133,598133,598251,598251,598251,598251,598251,598251,598251,598251,598325,598325,598325,598325,598325,598325,598541,598541,598541,598541,598541,598541,598541,598541,598541,598541,598541,598541,598541,598541,598541,598541,598541,598541,598541,598743,598743,598743,598743,598743,598743,598743,598886,598886,598886,598886,598886,598886,598886,598886,600579,600579,600629,600629,600629,600629,600629,600629,600629,600629,600629,600629,600629,600629,600629,600629,600629,600629,600629,600629,600629,600629,600629,600629,600629,600629,601176,601176,601176,601176,601176,601176,601176,601176,601176,601176,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,601267,602150,602150,602150,602150,602150,602150,602150,602150,602150,602150,602150,602306,602306,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602363,602932,602988,602988,602988,602988,602988,602988,602988,602988,602988,602988,602988,602988,602988,602988,602988,602988,602988,602988,602988,602988,602988,602988,603172,603172,603341,603341,603341,603341,603341,603341,603341,603341,603341,603341,603341,603341,603341,603341,604219,604219,604219,604219,604219,604219,604219,604219,604219,604219,604219,604219,604219,604219,604219,604219,604219,604219,604219,604219,604219,604750,604750,604750,604750,604750,604750,604750,604812,604812,604812,604812,604812,605265,605265,605346,605346,605346,605346,605346,605346,605346,605346,605346,605346,605346,605346,605346,605346,605346,605346,605346,605346,605346,605390,605529,605529,605790,605790,605790,605790,605790,606478,606478,606478,606478,606478,606478,606478,606478,606478,606478,606478,606478,606478,606478,606478,606478,606478,606478,606478,606478,606478,606478,606478,606478,606478,606478,606478,606478,606478,606607,606869,606869,606869,606869,606869,606869,606869,606895,606895,606895,606895,606895,606895,606895,606895,606895,607227,607703,607703,607703,607703,607703,607703,607703,607703,607703,607703,607703,607703,607703,607703,607703,608020,608020,608020,608020,608020,608020,608020,608020,608020,608020,608020,608020,608020,608020,608020,608020,608020,608020,608020,608020,608020,608020,608020,608020,608020,608020,608020,608020,609102,609102,609102,609102,609102,609102,609263,609263,609263,609263,609823,609823,610887,611175,611175,611175,611238,611238,611238,611238,611238,611238,611238,611238,611238,611238,611238,611238,611238,611238,611238,611238,611238,611238,611238,611238,611238,611238,611238,611238,611238,611730,612027,612198,612198,612198,613287,613287,613396,613396,613396,613396,613396,613396,613396,613396,613396,613396,613396,613396,613396,613396,613396,613396,613396,613396,613396,613396,613396,613396,613396,613396,613396,613396,613396,613396,613396,613396,613396,613396,613396,613396,613396,613396,613396,613396,613396,613396,613396,613396,613396,613396,613396,613396,613396,613396,613726,613726,613726,613726,613726,613726,613726,613726,613726,614271,614271,614690,614690,614690,614690,614690,614690,614690,614690,614690,614690,614690,614690,614690,614690,614690,614690,614690,614690,614690,614690,614690,614690,614851,614851,614851,614851,615279,615279,615279,615279,616740,616804,617030,617030,617030,617030,617030,617030,617030,617835,617835,617835,617835,617835,618007,618007,618007,618007,618007,618007,618007,618007,618381,618527,618527,618527,618527,618527,618527,618527,618527,618575,618575,618586,618783,618783,618783,618783,618783,618783,618783,618783,618960,618960,618960,618960,618960,618960,618960,619983,619983,619983,619983,620155,620155,620517,620517,620517,620517,620517,620517,620517,620517,620517,620517,620517,620517,620517,620517,620609,620609,620609,620609,620609,620609,620609,620609,620645,620645,620780,620780,620780,620850,620953,620953,620953,621110,621528,621528,621528,621528,621528,621528,621528,621528,621528,621528,621528,621528,621528,621528,621528,621528,622127,622260,622260,622476,622476,622476,622476,622476,622476,622476,622476,622476,622476,622476,622476,622476,622476,622476,622476,622476,622476,622476,622476,622476,622476,622476,622476,622476,622476,622476,622847,622847,622847,622847,622847,622847,622847,622847,622847,622847,622847,622847,622847,622847,622871,622871,622871,622871,622871,622871,623000,623000,623000,623000,623179,623179,623179,623179,623245,623245,623245,623245,623245,623245,623245,623245,623245,623245,623245,623245,623257,623379,623379,623379,623379,623379,623379,623379,623661,623661,623830,623830,623830,623830,623830,623830,623830,623830,623830,623830,623830,623830,623830,623830,623830,623830,623830,623830,623830,623830,623830,623830,623830,624110,624282,624282,624282,624282,624282,624282,624282,624282,624282,624282,624282,624282,624282,624282,624282,624282,624282,624282,624282,624282,624282,624282,624282,624282,624282,624282,624282,624282,624282,624282,624282,624282,625335,625344,625344,625344,625344,625344,625344,625344,625344,625344,625344,625344,625344,625344,625344,625344,625344,625344,625344,625344,625344,625344,625344,625344,625344,625344,625344,625344,625344,625344,625344,625344,626194,626194,626577,626721,626721,626721,626734,626734,626734,626734,626734,626734,626734,626955,626955,626991,626991,626993,626993,626993,626993,626993,626993,627155,627186,627186,627221,627221,627221,627221,627221,627221,627221,627528,627528,627528,627528,627709,629132,629132,629132,629132,629132,629132,629132,629132,629132,629132,629132,629132,629132,629132,629132,629364,629364,629364,629364,629364,629364,629364,629364,629364,629364,629364,629364,629364,629364,629364,629395,629395,629395,629395,629395,629395,629395,629395,629395,629623,629623,629801,629864,630119,630119,630119,630119,630252,630252,630252,630252,630252,630252,630252,630252,630252,630252,630252,630252,630252,630252,630252,630532,630765,630765,630765,630765,630765,630765,630765,630765,630765,630765,630765,630765,630765,630765,630765,630765,630765,630765,630765,630765,630765,630765,630765,630765,630765,630765,630765,630765,630765,630765,630765,630765,630765,630765,630765,630765,630765,630765,630765,630765,630765,630765,630765,630765,630765,630765,630765,630765,630765,630765,630765,630765,630765,631267,631267,631267,631267,631267,631267,631267,632259,632259,632259,632259,632259,632259,632259,632259,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632437,632711,632719,632719,632719,632719,632881,632881,632881,632881,632881,632881,632881,632881,632881,632881,632881,632881,632881,632881,632881,632881,632881,632881,632881,632881,632881,632881,632881,632881,632881,632881,632881,632881,632881,632881,632881,632881,633201,633295,633305,633305,633305,633305,633305,633305,633365,633365,633365,633365,633365,633365,633365,633365,633365,633592,633592,633592,633592,633592,633592,633592,633592,633592,633592,633592,633592,633592,633592,633592,633592,633592,633749,633749,633749,633749,633749,633977,633977,634466,634466,634477,634538,634538,634538,634538,634538,634538,634538,634623,634623,634623,634623,634623,634623,634623,634623,634623,634623,634623,634623,634842,635583,635875,636200,636200,636200,636200,636200,636200,636200,636200,636200,636200,636200,636200,636200,636200,636200,636200,636200,636200,636200,636200,636200,636200,636200,636200,637435,637538,637538,637830,637830,637830,638071,638071,638071,638071,638071,638071,638071,638078,638078,638078,638078,638078,638078,638078,638287,638287,638287,638287,638287,638934,639723,639962,639973,639973,639973,640182,640182,640182,640188,640188,640188,640188,640316,640502,640508,640508,640508,640508,640508,640508,640508,640508,640508,640508,640508,640508,640508,640508,640508,640508,640508,640508,640508,640508,640508,640508,640789,640967,640967,640967,640967,640967,640967,640967,640967,640967,640967,640967,640967,640967,640967,640967,640967,640967,640967,640967,640967,640967,640967,640967,640967,640967,640967,640967,640967,641253,641253,641253,641253,641253,641253,641253,641253,641253,641253,641253,641253,641253,641253,641253,641253,641253,641253,641253,641253,641253,641253,641253,641253,641253,641253,641253,641253,641253,641253,641253,641253,641253,641253,641253,641253,641253,641253,641253,641253,641253,641253,641253,641253,641253,641253,641253,641253,641253,641253,641253,641253,641253,641253,641253,641611,641611,641611,641611,642021,642021,642021,642021,642021,642021,642021,642021,642021,642021,642021,642021,642021,642021,642021,642021,642021,642021,642021,642021,642220,642220,642298,642298,642298,642298,642298,642298,642298,642298,642298,642298,642298,643026,643026,643026,643026,643026,643026,643026,643026,643026,644122,644122,644122,644122,644122,644122,644122,644122,644122,644122,644122,644122,644122,644122,644122,644122,644122,644122,644122,644122,644122,644122,644122,644122,644329,644329,644329,644329,644329,644329,644329,644329,644329,644329,644329,644374,644374,644374,644374,644374,644374,644374,644374,644374,644374,644374,644528,644528,644560,644560,644560,644560,644613,644613,644613,644613,644613,644613,644613,644613,644613,644619,644619,644619,644619,644619,644619,644619,644619,644619,644619,644619,644619,645152,645152,645152,645152,645152,645152,645152,645152,645152,645152,645152,645152,645152,645152,645152,645152,645152,645152,645152,645152,645152,645152,645152,645152,645152,645152,645152,645152,645152,645152,645152,645152,645152,645152,645152,645152,645152,645518,645518,645518,645549,645549,645549,645549,645549,645549,645549,645549,645549,645549,645549,645549,645549,645549,645895,645895,645895,645895,645895,645895,645895,645895,645895,645895,645895,645895,645895,645895,645895,645895,645895,645895,645895,645895,645895,646239,646239,646239,646239,646239,646277,646277,646277,646277,646277,646277,646277,646277,646277,646277,647122,647122,647122,647122,647122,647122,647122,647122,647413,647413,647413,647413,647413,647413,647413,647413,647413,647413,647413,647413,647413,647413,647413,647413,647413,647413,647413,647413,647413,647413,647413,647413,647413,647413,647413,647413,647413,647413,647413,647413,647413,647413,647413,647413,647413,647413,647413,647413,647413,647582,647582,647582,647582,647582,647582,647582,647817,648107,648107,648107,648107,648107,648107,648374,648374,648518,648518,648518,648518,648518,648518,648518,648518,648518,648530,648530,648530,648530,648530,648530,648676,649102,649102,649110,649119,649119,649119,649119,649119,649119,649119,649119,649119,649119,649119,649119,649119,649119,649119,649119,649119,649119,649119,649119,649119,649119,649119,649119,649119,649119,649119,649119,649119,649119,649119,649119,649119,649119,649119,649119,649119,649119,649119,649119,649119,649119,649119,649119,649119,649119,649119,649119,649119,649119,649119,649264,649264,649264,649264,649264,649264,649264,649264,649264,649264,649320,649320,649512,650001,650155,650155,650155,650155,650155,650155,650155,650155,650155,650155,650155,650155,650155,650363,650363,650363,650363,650363,650363,650465,650465,650465,650465,650465,650465,650465,650465,650465,650465,650465,650465,650465,650465,650465,650465,651044,651044,651044,651044,651044,651044,651044,651391,651391,651505,651505,651505,651505,651505,651505,651505,651505,651505,651505,651505,651656,651909,651909,651909,651909,651909,651909,651909,651909,651909,651909,651909,651909,651909,651909,651909,651909,651909,651909,651909,651909,651909,651909,651909,651909,651909,651909,651909,651909,651909,651909,651909,651909,651909,651909,651909,651909,651909,651909,651909,651909,651909,651909,651909,651909,651909,651909,651909,651909,651909,651909,651909,651909,651909,651909,651909,651909,651909,652246,652246,652246,652246,652246,652246,652246,652246,652246,652339,652339,652339,652339,652339,652543,653148,653148,653148,653148,653148,654043,654043,654043,654043,654043,654043,654043,654043,654043,654043,654043,654043,654043,654043,654043,654043,654228,654344,654693,654693,654693,654693,654693,654693,654693,654693,654693,654693,654693,654693,654693,654693,654693,654693,654693,654693,654693,654693,654693,654708,654719,654719,654719,654719,654738,654738,654738,654738,654738,654738,654738,654738,654738,654738,654977,654977,654977,654977,654977,654977,654977,654977,654977,654977,654977,654977,654977,654977,654977,654977,654977,654977,654977,654977,654977,654977,654977,654977,654977,654977,654977,654977,654977,654977,654977,654977,654977,654977,654977,654977,654977,654977,654977,654977,654977,654977,654977,654977,654977,654977,654977,654977,654977,655242,655242,655363,655363,655363,655363,655366,655366,655366,655366,655366,655366,655366,655366,655366,655366,655370,655513,655513,655513,655513,655658,655658,655658,655658,655658,655782,655815,655815,655815,655815,656442,656442,656442,656442,656442,656442,656442,656442,656442,656442,656442,656442,656442,656442,656442,656442,656442,656442,656442,656442,656442,656442,656442,656442,656442,657031,657031,657031,657031,657031,657189,657189,657189,657310,657310,657642,657642,657642,657642,657642,657642,657642,657642,657642,657642,657852,657852,657852,657852,657852,657852,657852,657852,657852,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657856,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657952,657963,657996,657996,657996,657996,657996,658247,658247,658263,658486,658486,658763,658936,658936,658936,658936,658936,658936,658936,658936,658936,658936,658936,658936,658936,658936,658936,658936,658936,658936,658936,658936,658936,658936,658936,658989,658989,658989,658989,658989,658989,658989,659005,659005,659005,659005,659005,659005,659005,659005,659005,659005,659012,659012,659012,659012,659012,659012,659012,659012,659012,659012,659267,659267,659267,659267,659267,659267,659267,659267,659267,659267,659267,659267,659267,659267,659267,659267,659267,659267,659267,659267,659267,659267,659267,659705,659705,659705,659705,659705,659705,659705,659705,659705,659705,659705,659705,659705,659705,659705,659705,660627,660652,660789,660789,660789,660789,660789,660789,660789,660789,660789,660789,660809,660809,660809,660809,660809,660809,660809,660809,660809,660809,661034,661034,661375,661375,661375,662022,662022,662022,662022,662022,662022,662022,662022,662022,662022,662086,662120,662120,662241,662241,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662318,662726,662800,662800,662800,662800,662800,662800,662803,662803,662803,662803,662803,662803,662803,662803,662803,662803,662803,662803,662803,662803,663186,663186,664089,664089,664089,664089,664089,664089,664089,664089,664089,664089,664089,664089,664089,664089,664089,664089,664089,664089,664089,664089,664089,664089,664089,664089,664089,664089,664089,664089,664089,664089,664089,664089,664463,664987,664987,665088,665129,665129,665129,665129,665129,665129,665129,665129,665129,665129,665129,665129,665129,665129,666555,666555,666555,666555,666555,666555,666555,666555,666555,666555,666555,666555,666555,666555,666555,666555,666555,666555,666555,666555,666555,666555,666555,666555,666555,666555,666555,666555,666555,666555,666555,666555,666555,666555,666555,666555,666555,666555,666555,666555,666555,666555,666555,666555,666555,666555,666555,666555,666555,666555,666555,666555,666555,666555,666555,666555,666555,666555,666555,666555,666555,666555,666654,666684,666684,667103,667103,667103,667103,667688,668340,668340,668340,668340,668340,668340,668340,668340,668340,668340,668340,668340,668340,668340,668340,668340,668681,668681,668681,668681,668681,668681,668681,668681,668770,668770,668770,668770,668838,668838,668838,668838,668838,669238,669238,669238,669639,669639,669639,669639,669639,669639,670520,670520,670520,670520,670520,670520,670520,670520,670520,670728,670728,670728,670728,670728,670728,670728,670728,670728,670728,670728,670728,670728,670728,671052,671052,671052,671052,671052,671052,671052,671052,671052,671052,671052,671052,671052,671052,671052,671052,671052,671052,671052,671052,671052,671052,671052,671052,671052,671052,671052,671052,671052,671052,671052,671052,671052,671052,671052,671052,671052,671052,671052,671052,671052,671052,671405,671405,671405,671405,671405,671405,671405,671405,671405,671405,671405,671405,671405,671405,671405,671405,671405,671405,671405,671405,671405,671405,671405,671405,671405,671405,671405,671405,671405,671405,671405,671405,671405,671405,671405,671405,671405,671405,671405,671405,671405,671405,671405,671405,671405,671405,671405,671405,671405,671405,671405,671868,672064,672064,672064,672850,672850,672850,673181,673181,673181,673181,673181,673181,673181,673181,673181,673181,673181,673181,673181,673181,673181,673521,673521,673521,673521,673521,673521,673521,673521,673521,673521,673521,673521,673521,673521,673521,673521,673521,673521,673521,673521,673521,673521,673521,673521,673521,673521,673521,673521,673521,673594,673594,673594,673594,673594,673594,673594,673723,673723,673756,673757,673757,673757,673757,673757,673757,673757,673757,673757,673757,673757,673757,673757,673757,673757,673757,673757,673757,673757,673757,673757,673757,673757,673758,673758,673903,673903,673903,673903,673903,673903,673903,673903,673903,673903,673903,674066,674066,674066,674066,674066,674066,674066,674066,674066,674079,674079,674079,674100,674100,674100,674100,674100,674100,674100,674100,674100,674100,674295,674295,674295,674295,674295,674295,674295,674295,674295,674295,674295,674295,674295,674295,674295,674295,674295,674295,674295,674295,674295,674295,674295,674295,674295,674295,674295,674295,674295,674295,674630,674630,674785,675393,675393,675393,675393,675393,675393,675393,675393,675393,675393,675393,675393,675393,675393,675393,675393,675393,676084,676084,676084,676224,676224,676224,676224,676224,676224,676224,676224,676224,676224,676224,676224,676224,676224,676224,676224,676224,676224,676224,676224,676224,676224,676224,676224,676224,676224,676224,676224,676224,676224,676224,676224,676224,676224,676290,676705,676941,676941,678374,678374,678910,678911,678911,678911,678911,678911,678911,678911,678911,678911,678911,678911,678911,678911,679013,679013,679013,679013,679013,679013,679566,679566,679566,679680,679680,679680,679680,679680,679680,679680,679680,679680,679680,679680,679680,679680,679680,679680,679680,679680,679680,679680,679765,679765,679765,679765,679765,679765,679765,679765,679821,679821,680074,680074,680074,680074,680074,680074,680586,680586,680586,681421,681421,681431,681431,681431,681431,681431,681431,681431,681620,681620,681656,681656,681656,681656,681656,681656,681656,681656,681656,681656,681656,681656,682015,682015,682879,682879,682879,682879,682879,682879,682879,682879,682879,682879,682879,682879,682880,683198,684139,684425,685159,685159,685159,685159,685159,685159,685159,685213,685213,685213,685213,685213,685213,685213,685213,685213,685213,685213,685213,685213,685213,685395,685395,685395,685395,685434,685434,685826,685826,685826,685826,686045,686045,686045,686045,686045,686045,686045,686045,686045,686045,686045,686045,686045,686045,686045,686045,686045,686045,686045,686045,686045,686045,686045,686045,686045,686045,686045,686045,686045,686045,686045,686045,686045,686045,686045,686045,686045,686045,686045,686045,686045,686045,686045,686045,686045,686045,686045,686045,686045,686045,686045,686045,686045,686045,686045,686045,686045,686045,686045,686045,686045,686045,686045,686234,686234,686234,686234,686234,686831,686831,686831,686831,686912,686912,686912,687058,687058,687058,687058,687058,687562,687562,687562,687562,687562,687562,687562,687562,687562,687562,687562,687727,687727,687727,687727,687727,687727,687951,687951,687951,687951,687951,687955,687955,688181,688181,688182,688182,688182,688182,688182,688260,688520,688520,688611,688611,689013,689013,689013,689013,689013,689379,689379,689379,689379,689644,690178,690178,690178,690178,690178,690178,690178,690178,690178,690178,690178,690178,690178,690178,690178,690178,690178,690178,690178,690178,690178,690178,690223,690234,690452,690453,690825,690825,690825,690825,690825,690825,690825,690825,690825,690917,690917,690967,690967,690967,690967,690967,690967,690967,691294,691294,691294,691352,691493,691493,691493,691493,691493,691493,691493,691493,691493,691568,691694,691694,691694,691694,691694,691694,691694,691694,691694,691694,691694,691694,691707,691719,691719,691719,691719,691719,691719,691978,691978,691978,691978,691978,692378,692378,692378,692378,692378,692378,692378,693072,693678,693678,693678,693697,693697,693697,693904,693904,693904,693904,693904,694415,694415,694415,694415,694415,694415,694415,694415,694415,694415,694415,694415,694415,694415,694415,694415,694415,694415,694415,694415,694415,694415,694415,694415,694415,694415,694415,694415,694415,694415,694415,694415,694415,694415,694541,694541,694541,694541,694541,694541,694678,694882,694882,694882,694882,694882,694951,694951,694951,694951,694951,694951,694951,694951,694951,694951,694951,694951,694951,694951,694951,694951,694951,694951,694951,694951,694951,695085,695085,695460,695460,695460,695460,695604,695604,695604,695604,695604,695604,695840,695840,696050,696050,696050,696050,696050,696050,696378,696378,696378,696378,696498,696602,696602,696602,696602,696602,696602,696602,696602,696602,696602,696602,696602,696602,696602,696649,697402,697402,697402,697402,697402,697402,697491,697491,697491,697781,697781,697781,697781,697781,697781,697781,697781,697781,697781,697781,697781,697781,697781,697781,697781,697781,697781,697781,697781,697781,697781,697781,697781,697781,697781,697781,697781,697836,697923,697923,697923,698216,698216,698216,698216,698216,698216,698216,698216,698216,698216,698216,698275,698275,698275,698275,698275,698275,698275,698275,698376,698376,698376,698376,698376,698376,698376,698376,698376,698376,698376,698376,698376,698533,698692,698692,698692,698692,698692,698692,698692,698692,698692,698701,698701,698701,698701,698701,698701,698911,698911,698911,698911,698911,698911,698911,698911,698911,698911,698911,698911,698911,698911,698911,698911,698911,698911,698911,698911,698911,698911,698911,698911,698911,698946,698946,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699166,699194,699194,699658,699658,699658,699859,699859,699859,699859,699859,699859,699859,699859,699940,699940,699940,700014,700014,700014,700070,700070,700070,700070,700070,700070,700070,700070,700070,700070,700070,700070,700127,700127,700127,700127,700646,700849,700849,700849,700849,700849,700849,701216,701216,701216,701216,701216,701216,701216,701216,701216,701216,701216,701216,701216,701216,701216,701216,701216,701216,701216,701216,701216,701216,701216,701216,701216,701216,701216,701216,701216,701216,701216,701216,701216,701216,701216,701216,701216,701231,701231,701242,701242,701242,701242,701315,701315,701315,701315,701408,701408,701408,701536,701536,701536,701536,701536,701536,701536,701536,701536,701536,701536,701536,701536,701536,701536,701536,701536,701536,701536,701536,701536,701536,701536,701536,701957,701957,701957,701957,701957,701957,701957,701957,701957,701957,701957,701957,701957,701957,701957,701957,701957,701957,701957,701957,701957,701957,701957,701957,701957,701957,701957,701957,701957,701957,701957,701957,701957,701957,701957,701957,701957,701957,701957,701957,701957,701957,701957,701957,701957,701957,701957,701957,702038,702038,702038,702038,702038,702038,702038,702038,702038,702079,702079,702079,702079,702079,702079,702079,702079,702079,702079,702079,702079,702079,702079,702079,702079,702079,702079,702079,702079,702079,702079,702079,702079,702079,702079,702079,702079,702079,702079,702079,702079,702079,702079,702079,702079,702079,702079,702079,702079,702079,702079,702079,702079,702079,702079,702079,702079,702079,702079,702079,702079,702079,703575,703575,703575,703575,703575,703575,703575,703575,703575,703575,703575,703672,703677,703972,704021,704021,704021,704021,704021,704021,704021,704021,704021,704021,704120,704120,704120,704120,704120,704120,704120,704120,704120,704120,704120,704120,704120,704120,704120,704120,704120,704120,704236,704339,704339,704339,704339,704339,704339,704961,704961,704961,704961,705036,705116,705225,705225,705225,705225,705225,705415,705415,705415,705415,705415,705415,705415,705415,705517,705517,705517,705517,705517,705517,705517,705517,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,705874,706194,706194,706194,706194,706194,706194,706194,706194,706194,706194,706194,706194,706194,706194,706194,706194,706194,706194,706267,706267,706267,706267,706769,706769,706769,706769,706769,706769,706769,706769,706769,706769,706769,706769,706769,706769,706769,706769,706769,706769,706769,706769,706769,706769,706769,706769,706769,706769,706769,706769,706769,706769,706769,706769,706769,706769,706769,706769,706769,706769,706769,706769,706769,706769,706769,706769,706769,706769,706769,706769,706769,706769,706769,707034,707034,707034,707918,707918,708382,708382,708382,708382,708382,708382,708382,708382,708382,708382,708382,708382,708382,708382,708421,708421,708550,708550,708550,708601,708601,708601,708603,708675,708675,708675,708675,708675,708675,708675,708789,708970,708970,708970,708970,708970,708970,708970,708970,708970,708970,708970,708970,708970,708970,708970,708970,708970,708970,708970,708970,708970,708970,708970,708970,708970,708970,708970,708970,708970,708970,708970,708970,708970,708970,708970,708970,708999,708999,708999,708999,708999,708999,708999,708999,708999,708999,708999,708999,708999,708999,708999,709373,709373,709373,709796,709796,709796,709796,709796,709796,709796,709796,709796,709796,709796,709796,709796,709796,709796,709796,709796,709796,709796,709796,709796,709796,709796,709796,709796,709796,709833,709833,709833,709833,709833,709833,709833,709833,709833,709833,709833,709833,709833,709833,709833,709833,709833,709833,709833,709833,709833,709833,709833,709833,709833,709833,709833,709833,709833,709833,709833,709833,709833,709833,709833,709833,709833,709833,709833,709833,709833,709833,709833,709833,709833,709833,709833,709833,709833,709833,709833,710414,710414,710566,710566,710566,710566,710566,710566,710566,710566,710566,710599,710660,710660,710660,710660,710660,710774,710774,710774,710774,710774,710774,711005,711005,711005,711367,711367,711367,711367,711367,711367,711367,711367,711367,711367,711367,711367,711367,711367,711367,711367,711416,711416,711416,711416,711416,711416,711416,711639,711639,711639,711639,711639,711639,711778,711778,711778,711778,711778,711778,711778,711778,711778,711778,711778,711778,711778,711778,711778,711778,711778,711778,711778,711778,711778,711807,711807,711807,711807,711807,711807,711807,711807,711807,711843,711843,711843,711843,711843,711979,711979,711979,711979,711979,711979,711979,711979,711979,711979,711979,711979,711979,711979,711979,711979,711979,711979,711979,712026,712026,712026,712026,712026,712026,712026,712026,712026,712026,712026,712026,712026,712026,712031,712031,712031,712031,712240,712240,712240,712240,712240,712240,712277,712277,712432,712432,712908,712908,712908,712908,712908,712908,712908,712908,712908,712908,712908,712908,712908,712908,712908,712908,712908,712908,712908,712908,712908,712991,713058,713058,713058,713058,713058,713058,713058,713058,713058,713058,713058,713058,713058,713058,713058,713058,713058,713058,713058,713058,713058,713058,713058,713058,713058,713058,713058,713058,713058,713058,713058,713581,713581,713581,713581,713581,713581,713581,713605,713605,713805,714059,714059,714201,714201,714201,714201,714201,714201,714201,714201,714201,714201,714201,714201,714201,714201,714201,714201,714201,714201,714201,714201,714201,714201,714201,714201,714201,714201,714201,714201,714201,714201,714201,714201,714201,714201,714201,714201,714201,714201,714201,714201,714201,714201,714201,714201,714201,714201,714201,714201,714404,714404,714404,714404,714404,714404,714404,714404,714404,714510,714510,714510,714510,714510,714563,714563,714563,714563,714563,714563,714563,714563,714563,714563,714563,714563,714563,714563,714563,714563,714563,714563,714563,714678,714678,714678,714678,714678,714678,714678,714755,714755,714859,714859,714859,714859,714859,714881,715262,715262,715262,715262,715262,715262,715262,715262,715262,715262,715262,715262,715262,715262,715262,715262,715262,715262,715262,715262,715262,715262,715262,715262,715262,715262,715262,715262,715262,715262,715262,715262,715262,715262,715262,715262,715262,715262,715262,715262,715262,715262,715262,715262,715262,715262,715262,715262,715262,715262,715262,715262,715262,715262,715411,715411,715520,715520,715788,715788,715852,715852,715852,715852,715852,715852,715852,715852,715852,715852,715852,715852,715852,715852,715852,715852,715983,715983,715983,715983,715983,715983,715983,715983,715983,715983,715983,715983,715983,715983,715983,715983,715983,715983,715983,715983,715983,715983,715983,715983,715983,715983,715983,716067,716067,716067,716067,716067,716067,716067,716140,716140,716140,716140,716140,716140,716140,716140,716140,716140,716140,716391,716391,716391,716391,716437,716437,716437,716437,716437,716437,716437,716437,716437,716437,716437,716437,716437,716437,716437,716437,716437,716437,716437,716437,716437,716744,716999,716999,716999,716999,716999,716999,716999,717437,717437,717437,717437,717437,717437,717437,717437,717437,717437,717437,717437,717437,717437,717437,717437,717437,717437,717437,717437,717437,717437,717437,717437,717437,717437,717437,717437,717437,717437,717437,717437,717437,717437,717437,717437,717437,717437,717831,717831,717831,717840,717840,717840,717840,717840,717840,717840,717840,717840,717840,717840,717840,717840,717840,717840,717840,717840,717840,717840,717840,717840,717840,717840,717840,717840,717840,717840,717840,717840,717840,717840,717840,717840,717840,717840,717840,717840,717840,717840,717840,717840,717840,717840,717840,717840,717976,717976,718534,718534,718534,718534,718534,718534,718534,718723,718723,718723,718723,718723,718723,718723,718723,718723,718723,718723,718723,718723,718723,718723,718723,718723,718723,718723,718723,718723,718723,718723,718723,718723,718723,718723,718723,718723,718723,718723,718723,718723,718723,718723,718723,718723,718723,719256,719256,719256,719256,719256,719256,720288,720288,720288,720288,720558,720968,720968,720968,720968,720968,720968,720968,720968,720968,720968,720968,720968,720968,720968,720968,720968,720990,720990,721101,721101,721101,721101,721101,721101,721101,721101,721101,721354,721354,721354,721354,721403,721403,721403,721403,721403,721403,721403,721403,721403,721403,721403,721882,721882,721882,721882,721882,721882,721882,721882,721882,721882,721882,721882,721882,721882,721901,721901,721901,722167,722167,722167,722167,722167,722167,722167,722167,722167,722167,722167,722167,722167,722167,722167,722167,722167,722167,722167,722167,722167,722167,722167,722167,722167,722167,722167,722167,722167,722511,722511,722511,722511,722511,722511,722511,722511,722511,722511,722511,722511,722511,722511,722511,722511,722511,722511,722511,722511,722511,722511,722532,722532,722532,722532,722532,722532,722532,722532,722532,722532,722532,722532,722532,722714,722714,722714,722752,722863,722863,723036,723036,723036,723036,723036,723036,723036,723036,723036,723036,723036,723322,723322,723322,723322,723322,723448,723448,723448,724303,724303,724303,724303,724303,724303,724303,724303,724303,724303,724303,724303,724303,724303,724303,724303,724303,724303,724303,724303,724303,724303,724303,724303,724303,724303,724303,724303,724303,724303,724303,724303,724303,724303,724303,724303,724303,724303,724303,724303,724303,724303,724303,724303,724303,724303,724303,724303,724303,724303,724303,724303,724303,724303,724303,724303,724303,724303,724303,724303,724353,724353,724353,724353,724353,724353,724353,724353,724353,724353,724353,724353,724353,724353,724353,724353,724353,724353,724353,724353,724353,724353,724353,724353,724353,724353,724353,724353,724353,724353,724353,724353,724353,724353,724739,724739,724739,724739,724777,724777,724777,724873,725359,725664,725704,725704,725704,725704,725858,725858,725878,725878,725973,725973,726442,726442,726442,726442,726442,726442,726442,726559,726559,726559,726559,726559,726559,726978,726978,726978,726978,726978,726978,727011,727061,727232,727232,727232,727232,727232,727232,727232,727232,727232,727232,727232,727494,727494,727494,727494,727494,727494,727494,727971,727971,727971,727971,727971,727971,727971,727971,727971,727971,727971,727971,727971,727971,727971,727971,727971,727971,727971,727971,727971,727971,727971,727971,727971,727971,727971,727971,727971,727971,727971,727971,727971,727971,728319,728319,728319,728319,728319,728389,728389,728389,728389,728954,729070,729208,729208,729225,729225,729225,729225,729225,729225,729225,729225,729225,729225,729225,729225,729225,729225,729225,729225,729225,729310,729310,729364,729364,729364,729364,729364,729364,729364,729364,729364,729750,729903,729903,730557,730557,730557,730557,730557,730557,730557,730557,730557,730557,730557,730557,730557,731231,731231,731290,731290,731290,731649,731649,731649,731649,731765,731765,731765,731882,731882,731882,731882,731882,731882,731882,731882,731882,731882,731882,731882,731882,731882,731882,731882,731982,731982,731982,731982,731982,731982,731982,731982,731982,731982,731982,731982,731982,732003,732003,732003,732003,732003,732003,732003,732003,732003,732003,732057,732057,732057,732057,732057,732057,732057,732057,732057,732057,732057,732057,732057,732057,732057,732057,732057,732057,732057,732057,732057,732057,732057,732057,732057,732057,732126,732126,732126,732126,732126,732126,732126,732341,732341,732341,732341,732341,732680,732969,733157,733157,733157,733157,733284,733284,733955,733955,733955,733955,733955,733955,733955,733955,733955,733955,733955,733955,733955,733955,733955,733955,733991,733991,733991,733991,733991,733991,733991,733991,734138,734138,734138,734138,734138,734138,734138,734138,734138,734138,734138,734138,734163,734163,734163,734163,734163,734163,734163,734163,734163,734163,734163,734163,734163,734163,734163,734163,734163,734163,734694,734694,734694,734694,734694,734694,734878,735051,735051,735051,735166,735166,735166,735166,735166,735330,735330,735330,735330,735330,735330,735330,735330,735330,735330,735330,735330,735330,735330,735330,735330,735330,735330,735330,735330,735330,735330,735663,735663,735663,735663,735663,735663,735663,735771,735771,735771,735771,735771,735771,735771,735771,735884,735884,735884,735884,735884,735884,735895,736000,736000,736000,736000,736000,736000,736000,736000,736000,736000,736528,736528,736528,736751,736751,736751,736751,736751,736751,736751,736751,736751,736751,736751,736751,736856,736856,736856,737184,737429,737748,737748,737925,738239,738239,738239,738239,738239,738239,738239,738239,738239,738239,738239,738239,738239,738239,738239,738239,738239,738239,738239,738239,738951,738951,738951,738951,738951,739269,739269,739269,739269,739269,739269,739269,739444,739444,739444,739444,739444,739444,739444,739444,739444,739444,739444,739444,739444,739444,739444,739444,739444,739444,739444,739444,739625,739936,739936,739936,739936,740751,740779,740779,740779,740779,740779,740779,740779,740779,740779,740794,740794,740794,740794,740794,740794,740794,740794,740794,740794,740794,740794,740794,740794,740794,740794,740794,740913,740913,740913,740913,740913,740913,740913,740913,741103,741103,741103,741103,741103,741103,741103,741103,741103,741199,741199,741207,741207,741207,741207,741207,741207,741207,741207,741207,741207,741207,741207,741207,741268,741268,741268,741268,741268,741268,741268,741268,741268,741268,741268,741268,741268,741268,741268,741268,741361,741361,741361,741361,741361,741361,741361,741361,741361,741361,741361,741417,741417,741417,741417,741417,741417,741417,741417,741417,741417,741417,741417,741417,741417,741417,741417,741417,741417,741417,741417,741417,741417,741417,741417,741417,741417,741417,741417,741417,741417,741417,741417,741417,741417,741417,741417,741417,741417,741417,741417,741417,741417,741417,741417,741733,741733,741733,741733,741733,741733,741733,741733,741733,741733,741762,741762,741762,741762,741762,741762,741762,741762,741762,741762,741762,741762,741762,741762,741762,741762,741762,741762,741762,741762,741762,741762,741762,741762,741762,741762,741762,741762,741762,741762,741762,741762,741762,741762,741762,741762,741762,741762,741762,741762,741762,741762,741762,741762,741762,741762,741762,741762,741762,741762,741762,741762,741767,741862,741862,741862,741866,741866,741866,741866,742375,742630,742630,742630,742630,742630,742630,742630,742630,742630,742984,742984,742984,742984,742984,743005,743005,743005,743005,743005,743005,743005,743005,743005,743005,743005,743005,743005,743005,743005,743005,743126,743126,743126,743126,743126,743126,743126,743126,743126,743126,743126,743126,743126,743126,743126,743126,743126,743126,743126,743126,743126,743168,743168,743168,743168,743168,743168,743168,743168,743168,743168,743168,743168,743168,743168,743168,743168,743168,743168,743168,743168,743208,743208,743208,743208,743208,743208,743208,743208,743226,743835,743835,743835,743835,743835,743835,743835,743956,743956,743975,743975,743975,743975,744011,744100,744100,744100,744685,744796,744796,744796,744796,744796,744796,744796,744796,744796,744796,744796,744796,744796,744796,744796,744796,744796,744796,744796,744796,744796,744796,744796,744796,744796,744796,745063,745066,745066,745066,745066,745066,745066,745066,745095,745105,745454,745454,745454,745454,745454,745454,745562,745562,745562,745700,746132,746132,746132,746132,746132,746132,746132,746132,746132,746132,746132,746132,746132,746132,746132,746132,746132,746132,746132,746132,746132,746132,746132,746132,746132,746132,746132,746132,746132,746132,746132,746132,746132,746132,746132,746132,746259,746259,746259,746259,746259,746259,746259,746259,746259,746259,746259,746259,746259,746259,746259,746259,746259,746259,746259,746259,746259,746259,746324,746324,746324,746324,746324,746324,746363,746363,746378,746378,746378,746378,746378,746378,746378,746378,746379,746379,746379,746379,746379,746379,746379,746379,746379,746379,746379,746379,746379,746379,746379,746379,746379,746379,746379,746379,746468,746468,746673,746673,746673,746673,746673,746673,746673,746673,746673,746673,746673,746673,746673,746673,746673,746673,746673,746673,746673,746673,746673,746673,746673,746673,746673,746673,746673,746673,746673,746673,746673,746673,746673,746673,746673,746673,746673,746673,746673,746673,746673,746673,746673,746673,746673,746673,746673,746673,746673,746673,746673,746673,746673,746673,746673,747279,747279,748208,748208,748208,748208,748208,748461,748513,748513,748513,748513,748514,748637,748637,748637,748637,748637,748659,748659,748659,748659,748659,748659,748659,748659,748659,748659,748659,748659,748659,748659,748659,748659,748659,748659,748659,748659,748659,748659,748659,748660,748660,748660,748660,748660,748660,748660,748660,748940,748940,748940,748940,748940,748979,748979,748979,748979,748979,748979,749002,749002,749002,749002,749002,749002,749002,749002,749002,749002,749002,749002,749002,749002,749002,749002,749002,749643,749685,749685,749685,749685,749685,749685,749841,749841,749841,749841,749841,750574,750574,750574,750574,750574,750574,750574,750574,750574,750574,750574,750574,750574,750574,750574,750574,750574,750574,750574,750574,750574,750574,750574,750574,750574,750574,750574,750595,750640,750640,750640,750640,750640,750640,750640,750640,750640,750640,750640,750640,750640,750640,750640,750695,750870,750870,750870,750870,750870,750870,750870,750870,750870,750870,750870,750870,750870,750870,750870,750870,750870,750870,750870,750870,750870,750870,750870,750870,750870,750870,750870,750870,750870,750870,750870,750870,750870,750870,750870,751506,751705,751705,752208,752208,752208,752208,752208,752208,752208,752208,752208,752208,752208,752208,752208,752208,752208,752208,752208,752208,752208,752208,752208,752208,752208,752208,752208,752208,752208,752208,752208,752208,752208,752208,752208,752208,752208,752225,752225,752367,752367,752367,752367,752367,752367,752367,752367,752367,752367,752367,752367,752367,752367,752367,752367,752367,752367,752367,752777,752777,752777,752777,752777,752777,752777,752777,752777,752777,752777,752777,752777,752777,752798,752798,752798,752798,752798,752873,753915,754234,754234,754234,754234,754234,754234,754234,754589,754589,754589,754589,754589,754726,754775,754775,754775,754907,754907,754907,754907,754907,754907,754961,754961,754961,754961,754961,754961,754961,754961,754961,754961,754961,754961,754961,754961,754961,754961,754961,754961,754961,754961,754961,754961,754961,754961,754961,754961,754961,754961,755015,755015,755015,755015,755015,755015,755015,755015,755015,755015,755015,755015,755015,755015,755015,755015,755015,755048,755219,755219,755219,755219,755485,755485,755485,755485,755485,755485,755485,755485,755485,756509,756509,756509,756509,756509,756509,757043,757043,757338,757338,757338,757562,758200,758200,758200,758200,758200,758200,758200,758200,758200,758455,758480,758480,758822,758860,758860,758860,758860,758860,758860,758860,758860,758860,758860,758860,758860,758860,758860,759249,759249,759249,759390,759390,759390,759390,759390,759390,759390,759390,759390,759390,759390,759390,759390,759390,759390,759390,759390,759390,759390,759390,759390,759390,759390,759580,759580,759580,759580,759580,759580,759596,759596,759596,759596,759596,759596,759596,759596,759596,760204,760218,760218,760218,760219,760219,760219,760219,760219,760219,760360,760360,760360,760360,760360,760360,760360,760360,760927,760927,760927,760927,760927,760927,760927,760927,760950,760950,760950,760950,760950,760950,760950,760950,760950,760950,760950,760950,760950,760950,760950,760950,760950,760950,760950,760950,760950,760950,760950,760950,760950,760950,760950,760950,760950,760950,760950,760950,760950,760950,760950,760950,760950,760950,760950,760950,760950,760950,760950,760950,760950,760950,760950,760950,760950,760950,760950,760950,760950,760950,760950,760950,760950,760950,760950,760950,761241,761362,761362,761362,761467,761467,761467,761472,761472,761472,761472,761472,761472,761472,761568,761568,761568,761568,761568,761568,761568,761568,761568,761568,761568,761568,761568,761568,761634,761634,761634,761634,761634,761634,761634,761634,761634,761634,761634,761634,761634,761634,761634,761634,761634,761634,761952,761952,761952,762150,762150,762150,762795,762795,762795,762871,762871,762871,762871,762871,762871,762871,762871,762871,762871,762871,762871,762871,762871,762871,762871,762871,762871,762871,762871,762871,762871,762871,763073,763131,763131,763131,763131,763131,763131,763131,763131,763131,763131,763131,763277,763756,764139,764232,764232,764622,764622,764622,764622,764622,764622,764622,764622,764622,764622,764622,764622,764961,764961,764961,764961,764961,764961,764961,764961,764961,764961,764961,764961,764961,764961,764961,764961,764961,764961,764961,764961,764961,764961,764961,764961,764961,764961,764961,764961,764961,764961,764961,764961,764961,764961,764961,764961,764961,764961,764961,764961,764961,764961,764961,764961,764961,764961,764961,764961,764961,764961,765244,765244,765244,765244,765244,765244,765244,765244,765244,765244,765244,765244,765244,765244,765244,765244,765244,765244,765244,765244,765244,765244,765244,765244,765244,765244,765244,765244,765244,765244,765244,765244,765244,765244,765244,765244,765244,765244,765244,765244,765244,765244,765244,765244,765244,765244,765244,765244,765244,765244,765244,765244,765244,765244,765244,765244,765244,765244,765244,765244,765244,765244,765244,765244,765244,765244,765244,765244,765244,765244,765244,765244,765244,765244,765244,765244,765244,765244,765290,765750,765750,765750,765750,765750,765750,765750,765750,765750,766295,766295,766295,766295,766295,766295,766295,766295,766295,766295,766418,766418,766418,766418,766418,766418,766418,766418,766418,766600,766600,766600,766600,766600,766600,766600,766600,766600,767192,767192,767192,767192,767192,767192,767192,767192,767192,767192,767192,767192,767192,767192,767192,767192,767192,767192,767192,767192,767192,767192,767224,767224,767224,767224,767408,767408,767408,767633,767633,767633,767633,767633,767633,767633,767633,767633,767633,767633,767642,767863,767863,767863,767863,767863,767863,767863,767863,767863,767863,767863,767863,767863,767863,767863,767863,767863,767863,767863,767863,767863,767863,767863,767863,767863,767863,767863,767863,767863,767863,767863,767863,767863,767863,767863,767863,767863,767863,767863,767863,767863,767863,767863,767863,768202,768202,768202,768202,768202,768202,768202,768202,768202,768202,768202,768202,768221,768221,768221,768221,768221,768221,768221,768221,768221,768222,768222,768222,768222,768222,768222,768222,768222,768222,768222,768222,768222,768222,768222,768222,768222,768222,768222,768222,768222,768222,768222,768222,768222,768222,768222,768222,768222,768222,768222,768222,768222,768222,768222,768679,769495,769495,769495,769495,769495,769495,769848,769848,769848,769889,769889,770083,770083,770592,770592,770592,770592,770592,770592,770592,770592,770592,770592,770788,770788,770788,770935,770935,770935,770935,770935,770935,770935,770935,770935,770935,770935,770935,770935,770935,770935,770935,770935,770935,770935,770935,770935,770935,770935,770935,770935,770935,770935,770935,770935,770935,770935,771118,771118,771118,771118,771118,771118,771246,771246,771246,771246,771246,771246,771246,771675,771675,771675,771675,771675,771675,771675,771675,771675,771675,771675,771675,771675,771675,771675,771675,771675,771675,771876,771907,771907,771907,771907,771907,772003,772003,772003,772003,772003,772562,772562,772562,772562,772562,772562,772562,772562,772706,772987,772987,772987,772987,773478,773883,773883,773883,773902,773902,773902,773902,773902,773902,773902,773902,773902,773902,773902,773902,773902,773902,773902,773902,773902,773929,773929,773929,773929,773929,773929,773929,773929,773929,773929,773929,773929,773929,774354,774412,774412,774464,774464,774464,774464,774721,774721,774721,774721,774721,774721,774721,774721,774721,774721,774721,774721,774721,774721,774721,774721,774721,774721,774721,774721,774721,774721,774721,774784,774784,774784,774784,774784,774784,774784,774784,774784,774784,774784,774784,774784,774784,774784,774784,774784,774784,774784,774784,774784,775003,775003,775003,775003,775003,775003,775003,775003,775003,775003,775003,775003,775003,775003,775003,775003,775003,775003,775003,775003,775003,775003,775003,775003,775003,775003,775003,775003,775003,775046,775046,775046,775046,775051,775051,775051,775051,775051,775051,775051,775051,775051,775051,775051,775051,775051,775051,775051,775051,775713,775713,775713,775713,775713,775713,775713,775713,775713,775713,775713,775713,775713,775713,775713,775713,775713,775713,775713,775713,775713,775713,775713,775713,775713,775713,775713,775940,775946,776217,776217,776455,776455,776455,776455,776512,776810,776810,776810,776810,776810,776810,776810,776810,776810,776810,776810,776810,776810,776810,776810,776810,776810,776810,776810,776810,776810,776941,776941,776941,776941,776941,776941,776941,777141,777141,777141,777141,777141,777141,777141,777141,777141,777141,777141,777141,777141,777141,777141,777205,777205,777205,777205,777205,777205,777205,777205,777205,777205,777205,777600,777600,777600,777600,777600,777600,777600,777600,777600,777600,777600,777600,777600,777600,777600,777600,777600,777600,777600,777600,777600,777600,777600,777600,777600,777600,777600,777600,777600,777600,777600,777764,777764,777764,777764,777764,777764,777764,777764,777764,777764,777764,777764,777764,777764,777764,777764,777764,777764,777764,777764,777764,777764,777764,777764,777764,777764,777764,777764,777764,777764,777850,777850,777850,777850,777850,777850,777850,777850,777850,777876,777876,778147,778147,778147,778147,778318,778318,778318,778318,778318,778318,778318,778318,778318,778318,778411,778411,778411,778411,778411,778411,778411,778411,778411,778411,778411,778411,778479,778479,778479,778479,778479,778621,778621,778621,778621,778621,778621,778621,778621,778621,778621,778621,778621,778621,778621,778621,778621,778621,778621,778621,778621,778621,778621,778621,778621,778621,778621,778621,778621,778621,778621,778621,778621,778940,778940,779011,779011,779011,779011,779011,779011,779011,779011,779011,779011,779011,779011,779011,779011,779011,779011,779011,779295,779295,779332,779332,779336,779336,779336,779336,779336,779336,779336,779336,779520,779602,779602,779602,779602,779602,779602,779602,779602,779602,779602,779602,779602,779602,779602,779602,779602,779602,779602,779602,779602,779602,779777,779777,779777,779777,779777,779777,779777,779777,779777,779948,780338,780519,780519,781395,781395,781395,781395,781509,781509,781653,781653,781653,782168,782168,782168,782168,782168,782168,782168,782195,782195,782195,782195,782195,782195,782195,782195,782195,782195,782195,782215,782215,782215,782215,782215,782215,782215,782397,782397,782397,782397,782397,782397,782397,782397,782397,782397,782397,782397,782397,782397,782397,782397,782397,782397,782418,782418,782418,782418,782418,783229,783229,783229,783229,783229,783229,783229,783229,783229,783229,783229,783229,783229,783229,783229,783229,783229,783229,783229,783229,783229,783229,783229,783229,783229,783234,783234,783234,783234,783234,783582,783582,783582,783582,783592,783592,783592,783592,783592,783592,783592,783592,783592,783592,783592,783592,783592,783592,783592,783608,783717,783717,783717,783717,783717,783717,783717,783879,783879,783879,783879,783879,783879,783879,783879,783879,783879,783879,783879,783879,783879,783879,783879,783879,783879,783879,783879,783879,783879,783879,783879,783879,783879,783879,783879,783879,784042,784042,784050,784050,785151,785151,785151,785151,785151,785158,785158,785158,785158,785214,785244,785244,785244,785244,785244,785244,785244,785244,785244,785244,785244,785244,785244,785244,785339,785661,785661,785661,785661,785661,785661,785661,785849,785850,785850,785850,786816,786816,786816,786816,786816,786816,787779,787779,787779,787779,788256,788256,788256,788256,788256,788256,788541,788541,788541,788541,788541,788541,788541,788541,788541,788541,789003,789003,789048,789048,789048,789048,789200,789200,789200,789200,789200,789200,789200,789200,789200,789200,789200,789200,789200,789200,789200,789200,789200,789200,789200,789200,789200,789200,789200,789200,789200,789200,789200,789200,789200,789200,789200,789200,789200,789200,789849,789849,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,789984,790043,790329,790329,790329,790329,790329,790329,790392,790392,790392,790392,790392,790744,790744,790872,790872,790872,790872,790872,790872,790872,790872,790872,791058,791058,791058,791415,791415,791415,791755,791893,791949,792236,792236,792236,792236,792247,792247,792314,792314,792314,792369,792388,792388,792388,792388,792388,792388,792388,792388,792388,792388,792388,792388,792388,792388,792388,792388,792388,792388,792388,792388,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,792545,793228,793228,793228,793228,793228,793228,793228,793228,793228,793228,793259,793259,793259,793259,793259,793259,793259,793259,793259,793259,793259,793259,793259,793259,793259,793259,793259,793259,793259,793259,793259,793259,793259,793259,793259,793281,793281,793281,793281,793281,793281,793281,793281,793281,793760,793809,793809,793809,793809,793809,793850,793850,793972,793972,793972,793972,793972,793972,793972,793972,793972,793972,793972,793972,793972,793972,793972,793972,793972,793972,794037,794684,794684,794968,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,794982,795190,795211,795211,795211,795211,795211,795211,795211,795211,795211,795211,795211,795211,795211,795211,795211,795211,795211,795211,795211,795211,795211,795211,795211,795211,795211,795211,795211,795211,795211,795211,795211,795211,795211,795211,795211,795211,795211,795211,795211,795211,795211,795211,795211,795211,795211,795211,795211,795211,795211,795249,795249,795249,795249,795249,795249,795249,795249,795249,795249,795249,795249,795249,795249,795461,795510,795510,795622,795622,795622,795761,795761,795761,795761,795761,795761,795761,795761,795761,795761,795761,795761,795761,795761,795761,795761,795761,795761,795761,795761,795761,795761,795761,795761,795775,795912,795912,795912,795912,796132,796132,796132,796132,796132,796132,796132,796132,796132,796132,796132,796132,796132,796132,796132,796132,796132,796132,796132,796132,796132,796132,796132,796132,796132,796132,796132,796132,796132,796132,796132,796132,796132,796132,796132,796132,796132,796132,796132,796132,796132,796231,796231,796231,796231,796244,796304,796304,796304,796304,796304,796304,796304,796454,796454,796792,796792,796792,796792,796898,796898,796898,796898,796898,796898,796898,796898,796966,796966,796966,796966,796966,796966,796966,796966,796966,796966,796966,796966,796966,796966,796966,796966,796966,796966,796966,796966,796966,796966,796966,796966,796966,796966,796966,796966,796966,796966,796966,796966,796966,796966,796966,796966,796966,796966,796966,796966,796966,796966,796966,796966,796966,796966,796966,796966,796966,796966,796966,796966,797061,797061,797061,797061,797061,797061,797061,797061,797061,797061,797061,797061,797061,797061,797419,797419,797697,797697,797697,797697,797697,797697,797938,797938,797938,797938,798202,798202,798260,798260,798260,798260,798260,798260,798260,798260,798260,798260,798260,798260,798260,798260,798260,798260,798260,798260,798260,798260,798260,798260,798260,798260,798260,798260,798260,798260,798260,798260,798260,798260,798260,798260,798260,798260,798260,798260,798275,798275,798275,798275,798275,798275,798275,798275,798275,798275,798275,798275,798275,798275,798275,798275,798275,798275,798275,798275,798275,798275,798275,798275,798275,798275,798275,798275,798275,798275,798275,798275,798275,798275,798275,798275,798275,798275,798275,798275,798275,798275,798275,798275,798275,798275,798275,798275,798275,798275,798275,798275,798275,798275,798275,798948,798948,798948,798948,798948,798948,798948,798948,798948,798948,798948,798948,798948,798948,799033,799033,799033,799033,799033,799033,799033,799033,799033,799207,799207,799207,799207,799207,799207,799207,799207,799207,799207,799207,799207,799207,799207,799207,799207,799207,799207,799207,799207,799207,799207,799207,799207,799207,799475,799475,799475,799475,799475,799475,799475,799475,799475,799475,799475,799475,799475,799475,799475,799475,799475,799475,799475,799550,799550,799550,799550,799550,799550,799627,799627,799627,799627,799627,799627,799627,799627,799627,799627,799627,799627,799627,799627,799627,799627,799627,799627,799627,799627,799627,799627,799627,799627,799627,799627,799627,799627,799627,799627,799627,799627,799627,799627,799627,799627,799627,799627,799627,799627,799716,799716,799716,799716,799795,799795,799795,799795,799795,799795,799795,799795,799795,799795,799795,799795,799795,799795,799795,799795,799795,799795,799795,799795,799795,799795,799795,799795,799795,799795,799795,799795,799795,799795,799795,799795,799795,799795,799795,799917,799917,799917,799917,799917,799917,799917,800188,800188,800610,800610,800610,800861,800861,800861,800861,800861,800861,801160,801160,801160,801160,801160,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801357,801477,801947,801947,801947,801987,801987,802269,802269,802269,802269,802269,802269,802269,802269,802269,802269,802269,802269,802269,802269,802269,802338,802338,802338,802338,802338,802338,802338,802338,802338,802338,802338,802338,802338,802338,802338,802338,802338,802338,802338,802338,802338,802338,802338,802338,802338,802338,802338,802338,802338,802338,802338,802338,802338,802338,802338,802338,802338,802338,802338,802338,802338,802338,802338,802338,802338,802338,802338,802338,802338,802338,802338,802338,802338,802338,802338,802338,802338,802338,802338,802338,802338,802397,802397,802397,802404,802404,802404,802404,802404,802404,802404,802404,802404,802404,802492,802492,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804071,804243,804243,804243,804243,804243,804243,804243,804243,804243,804243,804243,804243,804243,804243,804243,804243,804574,804574,804574,804574,804574,804574,804574,804574,804574,804574,804574,804574,804574,804574,804574,804574,804574,804574,804574,804574,804574,804574,804574,804574,804574,804574,804574,804574,804574,804574,804574,804574,804574,804574,804574,804574,804574,804574,804615,804615,804615,804615,804615,804615,804849,804849,804849,804849,804849,804849,804849,804849,804849,804849,804849,804849,804984,804984,804984,804984,804984,804984,804984,804984,804984,804984,805057,805057,805057,805057,805057,805057,805057,805057,805057,805057,805057,805057,805057,805057,805057,805057,805155,805155,805155,805155,805155,805155,805155,805155,805155,805155,805155,805155,805155,805155,805155,805155,805155,805155,805155,805155,805155,805155,805155,805314,805314,805314,805314,805314,805314,805314,805560,805560,805782,805782,805782,805782,805782,805782,805782,805782,805782,805782,805910,805910,805910,805910,805910,805910,805910,805910,805910,805910,805910,805910,806067,806503,806503,806503,806503,806503,806503,806503,806503,806503,806503,806503,806503,806503,806503,806503,806503,806503,806503,806503,806503,806503,806503,806503,806503,806503,806503,806503,806503,806503,806503,806503,806503,806503,806503,806503,806503,806503,806503,806503,806503,806503,806503,806503,806503,806503,806503,806503,806503,806503,806503,806503,806503,806503,806503,806503,806503,806503,806503,806503,806503,806503,806503,806503,806503,806503,806559,806871,806871,806871,806871,806871,806871,806871,806871,806874,806874,806874,806874,806874,806874,806874,806874,806874,806874,806874,806874,806874,806874,806874,806874,806874,806874,806874,806874,806874,806874,806874,806874,806874,806874,806874,806874,806971,806988,806988,806988,806988,807031,807031,807031,807325,807325,807325,807325,807325,807325,807325,807418,807418,807418,807418,807418,807418,807418,807418,807418,807418,807418,807418,807418,807418,807418,807418,807418,807418,807418,807418,807418,807418,807418,807460,807460,807460,807460,807460,807460,807460,807460,807460,807460,807460,807460,807460,807704,807704,807704,807704,807704,807704,807750,807750,807750,807750,807750,807750,807750,807750,807750,807750,807750,807750,807750,807750,807787,807833,807833,807833,807833,807833,807833,807833,807833,807833,807833,807833,807958,807958,807969,807969,807969,808064,808185,808205,808205,808205,808205,808205,808222,808222,808222,808222,808222,808843,808843,808843,808843,808843,808843,808843,808843,808959,808959,808959,809200,809209,809209,809209,809209,809209,809209,809209,809209,809209,809209,809209,809209,809209,809209,809209,809209,809209,809209,809209,809209,809209,809209,809209,809209,809209,809209,809209,809209,809209,809209,809209,809209,809209,809209,809209,809336,809426,809426,809426,809426,809426,809426,809426,809426,809426,809426,809426,809426,809426,809426,809426,809426,809673,809673,809673,809673,809673,809673,809673,809673,809673,809673,809673,809673,809673,809735,809735,809735,809735,809735,809735,809735,809735,809737,809737,809737,809737,809737,809737,809737,809737,809737,809737,809737,809737,809737,809737,809737,809915,809915,809915,809915,809915,809915,809915,809915,809915,809915,809915,809915,809915,809915,809915,809915,809915,809915,809915,810001,810190,810190,810190,810190,810375,810375,810375,810510,810510,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810526,810564,810564,810564,810564,810564,810564,810564,810564,810564,810564,810564,810564,810564,810564,810564,810564,810564,810564,810564,810564,810564,810564,810564,810564,810564,810564,810564,810564,810564,810564,810564,810564,810564,810564,810852,810852,810852,810852,810999,810999,810999,810999,810999,810999,810999,811167,811167,811167,811167,811167,811167,811167,811167,811167,811167,811167,811167,811167,811167,811358,811358,811358,811358,811358,811358,811358,811358,811358,811358,811358,811358,811361,811416,811579,811579,811579,811579,811579,811579,811579,811579,811579,811579,811579,811579,811579,811579,811579,811579,811579,811579,811579,811579,811579,811579,811579,811579,811579,811579,811579,811579,811579,811907,811907,811907,811907,811907,811907,811907,811942,811942,811942,811942,811942,811942,811942,811942,811942,811942,811942,811942,811962,811996,811996,811996,811996,811996,811996,812093,812093,812093,812093,812093,812093,812093,812093,812093,812592,812592,812592,812592,812592,812592,812592,812592,812592,812592,812592,812592,812592,812665,812665,812665,812665,812665,812680,812680,812680,812680,812680,812680,812952,812952,812952,812952,812952,812952,812952,812952,812952,812952,812952,812952,812952,813133,813213,813213,813213,813213,813213,813213,813213,813213,813213,813591,813591,813591,813591,813591,813591,813591,813591,813591,813591,813591,813591,813591,813591,813591,813591,813591,813591,813591,813591,813591,813591,813591,813591,813591,813591,813591,813591,813591,813591,813591,813591,813591,813591,813591,813591,813591,813591,813591,813591,813591,813591,813591,813591,813591,813591,813591,813591,813591,813591,813591,813591,813591,813591,813591,813670,813670,813670,813670,813670,813670,813670,813670,813670,813670,813670,813670,813670,813670,813670,813670,813670,813670,813670,813670,813670,813670,813670,813670,813670,813670,813670,813670,813670,814103,814103,814103,814551,814551,814551,814551,814551,814551,814551,814551,814563,814563,814563,814563,814563,814563,814563,814563,814563,814563,814563,814563,814563,814563,814563,814563,814563,814563,814563,814563,814563,814563,814563,814563,814563,814563,814563,814649,814649,814649,814649,814649,814649,814649,814649,814720,814758,814802,814802,814802,814802,814802,814802,814802,814802,814802,814802,814802,814809,814809,814809,814809,814809,814809,814809,814809,814809,814809,814809,814809,814809,814809,814809,814809,814809,814809,814809,814809,814809,814809,814809,814809,814809,814809,814809,814809,814809,814809,814809,814809,814809,814809,814809,814809,814809,814809,814809,814809,814809,814809,814809,814809,814809,814809,814809,814809,814809,814809,814809,814809,814809,814809,814809,814809,814809,814809,814809,814809,814809,814809,814809,814809,814809,814809,814809,814809,814809,815024,815024,815024,815024,815190,815190,815230,815230,815230,815230,815230,815230,815230,815230,815230,815230,815300,815300,815300,815300,815300,815300,815300,815300,815599,815599,815599,815599,815599,815599,815599,815599,815599,815599,815720,815720,815720,816002,816002,816002,816002,816002,816002,816002,816002,816002,816002,816002,816002,816002,816002,816002,816002,816002,816002,816002,816002,816002,816002,816002,816002,816002,816002,816002,816002,816002,816002,816002,816002,816002,816002,816002,816002,816002,816002,816088,816088,816088,816088,816088,816088,816088,816088,816088,816088,816088,816088,816088,816088,816088,816088,816088,816088,816088,816088,816088,816088,816088,816088,816088,816088,816088,816088,816088,816088,816088,816088,816088,816088,816088,816088,816096,816096,816096,816096,816233,816233,816318,816318,816409,816409,816409,816409,816774,817331,817331,817353,817353,817378,817378,817378,817452,817452,817452,817452,817452,817452,817452,817452,817593,817593,817593,817593,817593,817593,817593,817593,817593,817593,817791,817791,817791,817791,817791,817791,817791,817791,817791,817798,817798,817841,817841,817841,817841,817841,817842,817842,817842,817842,817842,817842,817842,817842,817842,817842,817842,817842,817842,817842,817842,817842,817842,817842,817842,818147,818147,818147,818147,818147,818147,818231,818231,818231,818231,818231,818231,818231,818231,818231,818231,818231,818231,818231,818231,818231,818231,818231,818231,818231,818231,818231,818231,818231,818231,818231,818231,818231,818231,818231,818231,818231,818231,818231,818231,818231,818231,818231,818410,818410,818410,818410,818410,818410,818410,818543,818543,818543,818593,818593,818593,818593,818593,818593,818593,818593,818593,818765,818765,818765,818765,818765,818765,818765,818765,818765,818765,818765,818765,818765,818765,818765,818765,818765,818765,818765,819166,819166,819166,819166,819166,819166,819316,819629,819629,819629,819629,819629,819629,819629,819629,819629,819629,820069,820069,820441,820441,820441,820441,820441,820441,820441,820441,820441,820441,820477,820477,820477,820477,820477,820477,820477,820477,820477,820477,820477,820477,820477,820477,820477,820477,820477,820477,820477,820477,820477,820477,820477,820477,820477,820477,820477,820477,820477,820477,820477,820477,820477,820477,820483,820483,820483,820483,820483,820483,820483,820483,820483,820513,820513,820513,820513,820525,820525,820525,820525,820525,820525,820525,820525,820525,820525,820525,820525,820525,820525,820525,820525,820525,820525,820525,820525,820525,820525,820525,820525,820525,820525,820551,820551,820551,820551,820551,820551,820551,820551,820551,820551,820551,820551,820551,820551,820551,820551,820551,820551,820551,820551,820551,820551,820551,820551,820551,820551,820551,820551,820551,820551,820551,820551,820551,820551,820551,820551,820551,820551,820551,820551,820551,820551,820585,820585,820585,820585,820585,820585,820585,820585,820585,820585,820585,820585,820585,820585,820585,820585,820585,820585,820703,820703,820703,820703,820703,820703,820703,820703,820703,820703,820703,820703,820703,820703,820703,820703,820703,820703,820703,820703,820703,820703,820703,820703,820703,820703,820882,820882,820882,820882,820882,820882,820882,820882,820882,820882,821092,821092,821092,821092,821092,821092,821092,821092,821092,821092,821092,821092,821092,821092,821138,821138,821138,821138,821138,821138,821138,821138,821138,821138,821138,821138,821138,821138,821138,821138,821138,821138,821138,821402,821458,821458,821458,821620,821620,821620,821699,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821710,821921,821921,821921,821921,821921,821921,821921,821921,821921,821921,821921,821921,821921,821921,821921,821921,821921,821921,821921,821921,821921,821921,821921,821921,821921,821921,821921,821921,821921,821921,821922,821956,822022,822336,822336,822336,822336,822336,822336,822619,822622,822622,822622,822792,822792,822792,822792,822792,822792,822792,822792,822792,822792,822792,822792,822792,822792,822792,822792,822792,822918,822949,822963,822969,822969,822969,822969,822969,823052,823052,823052,823052,823052,823052,823052,823052,823052,823052,823052,823052,823052,823052,823052,823052,823052,823052,823052,823052,823052,823052,823052,823052,823052,823052,823052,823052,823052,823052,823052,823052,823052,823052,823052,823052,823052,823052,823487,823487,823487,823487,823487,823487,823487,823487,823487,823487,823487,823487,823487,823487,823487,823487,823487,823487,823487,823487,823487,823487,823487,823487,823487,823487,823487,823487,823487,823487,823487,823487,823487,823487,823487,823487,823487,823487,823487,823487,823668,823668,823704,823704,823704,823704,823704,823857,823857,823857,823857,823868,823868,823945,823997,823997,823997,823997,823997,823997,823997,823997,823997,823997,823997,823997,823997,823997,823997,823997,824024,824070,824070,824070,824070,824070,824070,824070,824070,824070,824070,824070,824070,824070,824070,824070,824070,824070,824070,824070,824070,824070,824070,824070,824076,824146,824146,824268,824318,824318,824318,824318,824318,824318,824318,824318,824318,824318,824318,824318,824321,824321,824326,824326,824326,824326,824326,824326,824326,824326,824326,824326,824326,824326,824326,824326,824326,824326,824326,824326,824326,824326,824326,824398,824398,824398,824398,824398,824398,824398,824398,824398,824398,824398,824398,824398,824398,824398,824398,824398,824398,824398,824398,824398,824398,824418,824418,824418,824418,824418,824418,824418,824418,824418,824418,824418,824418,824501,824511,824604,824604,824763,824763,824926,824963,825085,825085,825085,825085,825085,825085,825085,825085,825085,825085,825085,825085,825085,825085,825085,825085,825085,825085,825085,825085,825085,825085,825085,825085,825085,825110,825110,825110,825110,825110,825110,825110,825110,825110,825110,825110,825110,825110,825110,825110,825110,825110,825110,825110,825110,825110,825110,825110,825110,825110,825110,825110,825110,825110,825110,825110,825110,825110,825110,825110,825126,825126,825189,825189,825189,825189,825189,825189,825189,825189,825189,825189,825189,825189,825189,825189,825189,825189,825189,825460,825619,825665,825665,825665,825665,825665,825665,825665,825665,825665,825665,825665,825665,825665,825665,825665,825665,825665,825665,825665,825665,825665,825665,825665,825665,825665,825665,825665,825665,825665,825665,825665,825665,825665,825665,825665,825665,825665,825665,825665,825665,825665,825665,825665,825665,825665,825665,825665,825665,825665,825665,825665,825665,825665,825665,825665,825665,825665,825665,825665,825665,825665,825665,825665,825665,825665,825665,825665,825665,825665,825665,825665,825665,825665,825665,825665,825734,825734,825802,825802,825964,825964,825964,825964,826107,826107,826107,826115,826115,826115,826115,826115,826115,826115,826115,826115,826115,826144,826144,826144,826144,826144,826144,826478,826478,826478,826693,826693,826693,826693,826769,826769,826769,826769,826769,826769,826769,826769,826769,826810,826810,826810,826810,826810,826810,826810,826810,826810,826810,826810,826810,826810,826810,826810,826810,826810,826810,826810,826810,826810,826810,826810,826810,826810,826810,826810,827069,827145,827675,827675,827675,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827756,827804,827804,827806,827806,827806,827806,827806,827806,827806,827806,827806,827806,827806,827806,827806,827806,827806,827806,827806,827806,827806,827806,827806,827806,827806,827806,828189,828189,828189,828189,828189,828189,828189,828189,828189,828189,828189,828189,828189,828189,828189,828189,828189,828189,828189,828189,828189,828390,828390,828390,828390,828390,828390,828390,828390,828390,828390,828390,828390,828390,828390,828390,828390,828553,828766,828766,828766,828766,828766,828766,828766,828766,828766,828766,828766,828766,828766,828766,828766,828766,828766,828766,828766,828766,828766,828766,828766,828766,828766,828766,828766,828766,828766,828766,828766,828766,828878,828878,828878,828878,828878,828909,828909,828909,828918,828918,828918,828918,828918,828918,828918,828918,828936,828936,828936,829154,829154,829154,829154,829154,829154,829327,829327,829327,829327,829327,829327,829327,829327,829327,829327,829327,829327,829327,829327,829327,829327,829327,829327,829327,829327,829327,829327,829327,829327,829327,829416,829416,829416,829416,829416,829416,829416,829416,829416,829416,829430,829430,829430,829430,829430,829430,829473,829473,829473,829473,829473,829473,829473,829473,829473,829473,829473,829473,829473,829473,829473,829473,829473,829473,829473,829473,829473,829473,829729,829729,829729,829729,829729,829729,829729,829898,829898,829898,829898,829898,829898,829898,829898,829898,829898,829898,829898,829898,829898,829898,829898,829898,830045,830045,830045,830226,830226,830226,830226,830226,831151,831151,831151,831151,831151,831151,831151,831151,831151,831151,831151,831151,831151,831151,831151,831151,831151,831151,831151,831151,831151,831151,831151,831151,831186,831186,831186,831186,831186,831186,831186,831186,831186,831186,831186,831186,831186,831186,831186,831186,831186,831186,831389,831389,831389,831389,831845,831845,831845,831845,831845,831845,831845,831845,831845,831845,831845,831845,831845,831845,831845,831879,831879,831879,831879,831879,831879,831879,832171,832171,832171,832268,832268,832268,832268,832268,832280,832280,832280,832280,832280,832280,832280,832280,832280,832280,832280,832280,832280,832514,832514,832514,832514,832737,832737,832737,833663,833663,833696,833696,833696,833696,833696,833696,833696,833696,833696,833696,833696,833696,833696,833696,833701,833795,833795,833795,833795,833795,833795,833795,833812,833812,833812,833812,833812,833812,834010,834010,834010,834277,834277,834277,834356,834370,834370,834370,834370,834370,834370,834370,834370,834370,834370,834370,834370,834370,834370,834370,834370,834370,834370,834602,834602,834602,834602,834602,834602,834602,834602,834602,834602,834602,834602,834602,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834758,834800,834800,834800,834800,834800,834800,834800,834800,834800,834800,834800,834800,834800,834800,834881,834881,834881,834881,834881,834881,834881,834881,834881,834881,834881,834881,834881,834881,834881,834881,834881,834881,834881,834881,834881,834881,834881,834881,834881,834881,834881,834881,834881,834881,834881,834881,834881,834881,834881,834881,834881,834881,834881,834881,834881,834881,834881,834881,834881,834898,834898,834898,834898,834898,834898,834898,834898,834898,834898,834898,834898,834898,834898,834898,834898,834898,834898,834898,834898,834898,834898,834898,834898,834898,834898,834898,834898,834898,834898,834898,834898,834898,834898,834898,834898,834898,834898,834898,834898,834898,834898,834898,834898,834898,834898,834898,834898,834898,834898,834898,834898,834898,834898,834898,835255,835480,835480,835480,835606,835606,835715,835715,835715,835715,835715,835715,835739,835739,835739,835739,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835884,835934,836065,836142,836279,836279,836279,836279,836279,836324,836324,836324,836324,836324,836324,836324,836455,836455,836455,836455,836455,836455,836455,836455,836455,836455,836455,836455,836462,836462,836462,836462,836462,836469,836469,836469,836503,836503,836503,836659,836659,836659,836659,836659,836659,836659,836659,836659,836659,836659,836659,836659,836659,836659,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836748,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836831,836960,836960,836960,836960,836960,836963,836963,836963,836963,836963,836963,837038,837038,837038,837038,837090,837111,837111,837190,837190,837190,837208,837208,837208,837208,837318,837318,837318,837320,837320,837320,837320,837320,837320,837320,837320,837320,837320,837320,837320,837352,837352,837355,837355,837355,837477,837477,837477,837477,837477,837477,837477,837477,837477,837477,837477,837477,837624,837624,837624,837624,837624,837624,837624,837624,837625,837625,837625,837625,837625,837625,837625,837625,837700,837700,837700,837700,837700,837700,837700,837700,837732,837878,837878,837878,837878,837878,837878,837878,837878,837878,837878,837878,837878,837916,837916,837916,837946,838009,838111,838111,838111,838111,838111,838111,838111,838111,838111,838114,838114,838114,838114,838114,838114,838114,838114,838114,838114,838114,838115,838115,838116,838116,838116,838116,838116,838116,838117,838117,838117,838117,838117,838123,838123,838123,838123,838123,838123,838123,838123,838123,838123,838123,838123,838123,838123,838123,838123,838123,838123,838123,838123,838123,838123,838123,838123,838123,838123,838123,838123,838147,838147,838147,838147,838147,838147,838147,838147,838147,838147,838147,838147,838147,838147,838147,838147,838147,838147,838147,838147,838147,838147,838147,838147,838147,838147,838147,838147,838147,838147,838147,838147,838147,838147,838455,838455,838806,838846,838846,838846,838846,838846,838940,839040,839040,839040,839040,839040,839040,839040,839040,839040,839123,839123,839123,839123,839123,839123,839123,839369,839369,839369,839369,839369,839369,839369,839369,839369,839369,839369,839369,839369,839369,839369,839369,839369,839369,839369,839369,839369,839487,839540,839540,839540,839758,839758,839758,839758,839762,839762,839762,839762,839762,839762,839762,839762,839762,839762,839762,839762,839762,839762,839762,839762,839762,839762,839762,839762,839762,839762,839762,839762,839762,839762,839762,839762,839762,839762,839762,839762,839762,839762,839762,839762,839762,839762,839762,839762,839762,839762,839762,839762,839762,839762,839762,839762,839855,839855,839855,839855,839855,839855,840075,840075,840075,840075,840075,840075,840075,840075,840075,840075,840075,840075,840075,840075,840075,840075,840110,840110,840110,840110,840110,840115,840115,840115,840115,840115,840115,840115,840115,840115,840115,840115,840115,840115,840115,840115,840115,840115,840115,840115,840115,840115,840115,840115,840115,840115,840115,840115,840115,840115,840115,840115,840115,840115,840115,840168,840168,840168,840168,840168,840168,840168,840168,840168,840168,840170,840170,840170,840170,840170,840170,840187,840187,840187,840187,840187,840187,840187,840187,840187,840187,840187,840187,840187,840187,840187,840187,840187,840187,840187,840187,840187,840239,840239,840239,840239,840335,840335,840335,840335,840335,840335,840335,840335,840335,840356,840356,840356,840578,840578,840578,840578,840578,840578,840578,840578,840578,840578,840578,840791,840791,840791,840791,840791,840850,840850,840950,840950,840950,840950,840950,840950,840950,840950,840956,840956,840956,840956,841001,841001,841001,841001,841001,841001,841001,841001,841001,841001,841001,841001,841001,841001,841001,841001,841001,841001,841001,841001,841001,841001,841001,841001,841001,841001,841001,841001,841001,841001,841001,841001,841001,841001,841001,841017,841017,841017,841017,841017,841017,841017,841017,841017,841017,841017,841017,841017,841017,841017,841062,841062,841062,841062,841062,841062,841062,841062,841062,841062,841062,841062,841062,841062,841062,841062,841062,841062,841062,841062,841062,841062,841062,841062,841062,841062,841435,841435,841435,841435,841435,841435,841638,841638,841638,841638,841638,841638,841638,841638,841638,841638,841638,841638,841638,841638,841638,841638,841638,841638,841638,841638,841638,841638,841638,841638,841638,841638,841638,841638,841638,841638,841638,841639,841639,841639,841639,841639,841639,841639,841639,841639,841639,841639,841639,841639,841639,841678,841678,841678,841678,841678,841678,841678,841678,841678,841723,841723,841723,841723,841723,841870,841870,841870,841870,841870,842000,842009,842009,842015,842112,842152,842152,842152,842152,842152,842152,842152,842152,842152,842152,842152,842152,842152,842152,842152,842152,842152,842152,842152,842152,842152,842152,842152,842152,842152,842152,842152,842152,842152,842152,842152,842152,842152,842152,842152,842152,842152,842152,842152,842152,842152,842152,842152,842152,842152,842152,842152,842152,842152,842152,842152,842152,842152,842152,842152,842152,842152,842152,842152,842152,842152,842152,842152,842152,842152,842152,842152,842179,842187,842187,842187,842187,842187,842187,842187,842209,842209,842209,842209,842209,842209,842209,842209,842209,842209,842209,842209,842209,842209,842209,842209,842209,842209,842209,842209,842209,842209,842209,842209,842209,842209,842209,842209,842209,842209,842209,842209,842209,842209,842209,842209,842209,842209,842209,842209,842264,842719,842719,842719,842783,842783,842783,842876,842876,842876,842876,842876,842876,842876,842876,842876,842876,842876,842876,842876,842876,842876,842876,842876,842876,842876,842876,842876,842876,842876,842876,842876,842876,842876,842876,842876,842876,842876,842876,842876,842876,842876,842876,842876,842876,842876,842876,842876,842876,842876,842876,842876,842876,842876,842876,842876,842876,842876,842876,842876,842876,842876,842876,842876,842876,842915,842915,842915,842915,842915,842915,842915,842915,842989,843640,843640,843693,843693,843693,843693,843716,843716,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843755,843799,843799,843807,844176,844176,844176,844176,844176,844176,844176,844176,844176,844176,844176,844176,844176,844176,844176,844176,844176,844176,844176,844274,844274,844274,844274,844274,844274,844274,844274,844274,844274,844274,844274,844274,844274,844274,844274,844436,844438,844565,844627,844627,844627,844627,844627,844639,844639,844639,844639,844639,844639,844639,844639,844639,844639,844639,844639,844639,844639,844639,844639,844639,844639,844639,844639,844639,844639,844639,844639,844639,844639,844639,844639,844672,844672,844672,844672,844672,844672,844672,844672,844672,844672,844672,844672,844971,844971,844971,844971,844971,844971,844971,844971,844971,844971,844971,844971,844971,844971,844971,844971,844971,844971,844971,844971,844971,844971,844971,844971,844971,844971,844971,844971,844971,844971,845190,845279,845279,845279,845421,845421,845421,845421,845421,845421,845421,845421,845421,845421,845421,845559,845559,845559,845559,845559,845559,845559,845559,845559,845559,845559,845559,845559,845559,845559,845559,845559,845559,845559,845559,845559,845559,845559,845559,845559,845559,845559,845559,845559,845559,845559,845559,845559,845559,845559,845559,845559,845559,845559,845559,845559,845559,845559,845559,845592,845592,845592,845592,845592,845592,845592,845592,845592,845592,845592,845592,845592,845592,845592,845592,845592,845592,845592,845592,845592,845592,845592,845805,845805,845805,845805,845884,845884,845888,845888,845888,845948,845948,845948,845948,845948,845948,845948,845948,845948,845948,845948,845948,845948,845948,845948,845948,845948,845948,845972,845972,845972,845972,845972,845972,845972,845972,845972,845972,846057,846521,846521,846521,846521,846521,846521,846521,846521,846521,846521,846521,846521,846610,846610,846610,846610,846610,846610,846610,846610,846610,846610,846610,846610,846610,846610,846610,846610,846610,846610,846610,846610,846610,846610,846610,846610,846610,846610,846610,846610,846610,846610,846610,846610,846610,846610,846610,846610,846610,846610,846610,846610,846610,846610,846610,846610,846610,846610,846610,846610,846610,846610,846610,846610,846610,846610,846610,846610,846635,846635,846635,846642,846642,846677,846862,846919,846919,846983,846983,846983,846983,846983,846983,846983,846983,846983,846983,846983,846983,846983,846983,846983,846983,846983,846983,846983,847017,847017,847017,847017,847017,847017,847017,847017,847017,847017,847017,847041,847041,847041,847041,847041,847041,847174,847174,847174,847174,847174,847174,847261,847261,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847303,847311,847370,847370,847440,847576,847795,847795,847795,847815,847815,847815,847815,847815,847815,847815,847815,847815,847815,847815,847815,847815,847815,847815,847815,847815,847815,847815,847815,847815,847815,847820,847820,847820,847820,847820,847820,847820,847820,847820,847820,847820,847820,847820,847820,847820,847820,847820,847820,847820,847820,847820,847820,847820,847824,847824,847824,847824,847824,847824,847824,847824,847899,848247,848247,848247,848308,848308,848308,848308,848308,848308,848308,848308,848308,848308,848308,848308,848308,848308,848308,848308,848308,848308,848308,848308,848308,848308,848308,848308,848308,848308,848308,848308,848308,848308,848308,848308,848315,848620,848644,848644,848644,848654,848654,848654,848856,848856,848856,848856,848856,848856,848856,848856,848856,848856,848856,848856,848856,848856,848856,848856,848856,848856,848859,848859,848909,848909,848909,848909,848909,848909,848909,848955,848955,848955,848955,848955,848955,848955,848955,848955,848955,848955,848955,848955,848955,848955,848955,848955,848955,848958,848958,848958,848958,848963,848963,848963,848963,848963,848963,848963,849009,849009,849009,849009,849009,849009,849009,849009,849009,849021,849021,849060,849060,849247,849247,849247,849276,849276,849276,849276,849276,849276,849276,849276,849276,849276,849276,849276,849276,849276,849276,849276,849276,849276,849276,849304,849304,849304,849304,849309,849309,849309,849338,849471,849471,849471,849471,849471,849471,849471,849471,849471,849471,849471,849471,849471,849471,849471,849471,849471,849471,849471,849471,849471,849471,849471,849471,849471,849471,849471,849471,849471,849471,849471,849471,849471,849471,849471,849471,849471,849471,849471,849471,849471,849471,849471,849471,849471,849471,849471,849471,849471,849471,849471,849496,849496,849496,849496,849496,849496,849496,849496,849496,849496,849496,849496,849496,849496,849496,849496,849496,849496,849496,849496,849496,849496,849496,849496,849496,849496,849496,849496,849496,849496,849496,849496,849496,849496,849496,849496,849496,849496,849496,849496,849496,849496,849496,849496,849496,849496,849496,849496,849496,849496,849496,849496,849496,849496,849532,849532,849532,849532,849532,849532,849532,849532,849532,849532,849532,849532,849532,849532,849532,849532,849532,849532,849532,849532,849532,849532,849532,849532,849532,849532,849532,849532,849532,849563,849563,849563,849563,849563,849563,849563,849563,849563,849563,849563,849563,849563,849563,849563,849563,849563,849563,849563,849563,849563,849563,849563,849563,849563,849563,849563,849563,849563,849563,849563,849563,849563,849563,849563,849563,849563,849563,849563,849563,849563,849563,849616,849626,849626,849626,849626,849626,849626,849626,849626,849626,849626,849626,849626,849626,849626,849626,849626,849626,849626,849626,849857,849857,849857,849857,849857,849857,849857,849970,849970,849970,849970,849970,849970,849970,849970,849972,850067,850067,850316,850316,850316,850358,850358,850421,850606,850606,850655,850655,850655,850840,850840,850840,850840,850840,850840,850840,850840,850840,850840,850840,850840,850840,850840,850840,850840,850840,850840,850840,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850881,850920,850920,850920,850920,850920,850920,850920,850920,850920,851298,851298,851298,851298,851298,851298,851298,851298,851351,851351,851351,851351,851351,851351,851351,851351,851351,851351,851351,851351,851351,851351,851378,851378,851378,851378,851378,851378,851378,851378,851378,851378,851378,851378,851378,851378,851378,851378,851378,851642,851642,851642,851642,851680,851721,851721,851721,851721,851721,851721,851795,851795,851795,851795,851795,851795,851795,851795,851795,851795,851795,851800,851800,851800,851800,851800,851800,851800,851800,851800,851800,851800,851800,851913,851913,851913,851913,851913,851913,851913,852032,852032,852032,852032,852032,852032,852032,852032,852032,852032,852032,852032,852032,852032,852032,852032,852032,852032,852032,852032,852032,852032,852032,852032,852032,852032,852032,852032,852032,852032,852032,852032,852032,852032,852032,852062,852062,852062,852062,852062,852062,852062,852062,852062,852062,852062,852062,852062,852101,852101,852101,852101,852101,852101,852101,852251,852354,852354,852354,852354,852354,852354,852354,852354,852354,852354,852390,852390,852390,852390,852390,852390,852390,852390,852390,852390,852390,852390,852390,852390,852390,852390,852390,852522,852522,852711,852711,852711,852711,852711,852711,852711,852711,852711,852711,852711,852711,852711,852711,852711,852711,852711,852711,852711,852711,852711,852711,852711,852711,852711,852711,852711,852711,852711,852711,852711,852711,852711,852711,852711,852711,852711,852711,852711,852711,852711,852711,852711,852711,852711,852711,852711,852845,852877,852877,852877,852877,852877,852877,852877,852969,852969,852969,852969,852969,852969,852969,853024,853024,853024,853024,853024,853024,853024,853024,853024,853170,853257,853308,853308,853308,853308,853308,853308,853308,853308,853308,853308,853308,853308,853308,853308,853308,853332,853332,853332,853332,853332,853332,853342,853342,853342,853342,853342,853342,853342,853342,853342,853342,853342,853342,853342,853342,853342,853342,853342,853342,853342,853342,853342,853342,853342,853342,853342,853342,853342,853342,853342,853342,853342,853342,853342,853342,853342,853342,853342,853342,853342,853342,853342,853342,853342,853342,853342,853342,853342,853342,853342,853342,853342,853342,853342,853342,853342,853342,853342,853342,853342,853347,853347,853347,853347,853347,853347,853347,853347,853347,853347,853347,853347,853347,853347,853347,853347,853349,853349,853349,853349,853349,853426,853600,853600,853600,853600,853600,853600,853600,853600,853600,853600,853653,853653,853653,853653,853653,853653,853653,853669,853669,853727,853727,853727,853727,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853731,853802,853802,853802,853802,853802,853802,853802,853802,853802,853802,853802,853802,853802,853802,853802,853802,853802,853802,853802,853802,853802,853802,853802,853802,853802,853802,853802,853802,853802,853802,853802,853802,853802,853802,853802,853919,853919,853919,853940,853940,853940,853940,853987,853987,854025,854025,854025,854025,854025,854101,854119,854119,854119,854173,854173,854174,854174,854174,854174,854201,854201,854270,854270,854280,854280,854280,854280,854280,854280,854280,854280,854328,854328,854328,854328,854328,854350,854350,854350,854350,854350,854350,854350,854350,854350,854350,854350,854350,854350,854350,854350,854350,854370,854370,854370,854370,854370,854370,854370,854370,854370,854370,854370,854370,854370,854370,854370,854370,854370,854370,854370,854370,854370,854370,854370,854370,854370,854370,854370,854370,854370,854370,854370,854370,854370,854370,854370,854370,854370,854370,854370,854370,854370,854370,854370,854370,854370,854374,854374,854374,854374,854374,854374,854379,854379,854379,854379,854379,854379,854379,854379,854379,854379,854379,854379,854379,854379,854480,854480,854480,854480,854480,854480,854480,854480,854480,854480,854480,854591,854591,854607,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854649,854669,854669,854836,854836,854876,854876,854876,854876,854876,854876,854920,855035,855035,855035,855035,855035,855035,855092,855102,855102,855102,855102,855102,855102,855102,855102,855102,855102,855102,855102,855104,855104,855104,855209,855209,855209,855209,855209,855209,855209,855209,855209,855209,855209,855300,855300,855321,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855374,855412,855412,855412,855412,855412,855412,855412,855412,855412,855412,855412,855412,855412,855412,855534,855534,855534,855534,855534,855534,855534,855534,855534,855534,855534,855534,855534,855534,855534,855549,855549,855549,855600,855600,855600,855600,855600,855600,855600,855600,855600,855600,855600,855600,855600,855600,855600,855600,855600,855650,855650,855650,855692,855692,855692,855692,855692,855692,855692,855692,855716,855716,855716,855716,855716,855716,855716,855716,855783,855806,855806,855806,855806,855806,855806,855914,855914,855914,855914,855914,855914,855914,855914,855914,855914,855914,855914,855914,855914,855914,855914,855914,855914,855914,855914,855914,855914,855914,855914,855914,855914,855914,855914,855914,855914,855914,855914,855914,855914,855914,855914,855914,855914,855914,855914,855914,855914,855914,855914,855914,855914,855914,855914,855914,855914,855914,855914,855914,855914,855914,855914,855914,856026,856026,856026,856026,856026,856026,856026,856026,856045,856045,856045,856045,856045,856045,856045,856045,856055,856055,856055,856055,856055,856055,856055,856055,856055,856055,856055,856055,856055,856055,856068,856068,856068,856068,856068,856068,856068,856068,856068,856068,856068,856068,856068,856068,856068,856070,856070,856070,856070,856070,856070,856070,856093,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856153,856370,856370,856370,856434,856434,856434,856434,856434,856434,856434,856434,856434,856528,856528,856528,856528,856528,856528,856528,856528,856528,856528,856528,856528,856528,856528,856528,856528,856528,856528,856528,856528,856528,856528,856528,856528,856528,856528,856528,856528,856528,856528,856528,856528,856528,856528,856528,856528,856547,856547,856547,856689,856689,856689,856689,856689,856689,856689,856689,856689,856689,856689,856689,856689,856689,856689,856689,856689,856689,856689,856689,856689,856689,856689,856689,856689,856689,856689,856689,856689,856689,856689,856689,856689,856689,856697,856697,856697,856697,856775,856775,856775,856775,856775,856775,856775,856775,856775,856775,856815,856815,856815,856815,856815,856815,856815,856815,856815,856815,856815,856815,856815,856815,856815,856815,856815,856815,856815,856815,856815,856815,856815,856815,856815,856815,856815,856815,856815,856815,856815,856815,856815,856815,856815,856815,856815,856815,856815,856815,856815,856815,856815,856815,856815,856815,856815,856815,856885,856885,856905,856905,856985,856985,856985,857046,857046,857046,857054,857054,857054,857054,857054,857054,857054,857054,857054,857054,857054,857054,857054,857054,857054,857054,857054,857054,857054,857054,857054,857054,857054,857054,857054,857054,857054,857054,857054,857054,857054,857054,857054,857054,857054,857054,857054,857054,857054,857054,857068,857068,857068,857068,857068,857068,857068,857068,857068,857068,857068,857068,857068,857068,857068,857068,857068,857068,857068,857068,857068,857068,857159,857159,857159,857159,857159,857159,857159,857159,857203,857203,857203,857203,857203,857203,857203,857203,857203,857203,857203,857203,857203,857203,857203,857203,857300,857328,857328,857371,857371,857371,857418,857418,857418,857418,857418,857418,857418,857418,857418,857418,857418,857418,857418,857418,857418,857418,857418,857418,857418,857418,857418,857418,857418,857418,857418,857418,857418,857418,857418,857418,857418,857418,857418,857425,857432,857432,857447,857452,857452,857452,857452,857452,857452,857452,857452,857452,857452,857452,857452,857452,857452,857452,857452,857483,857483,857483,857483,857483,857536,857536,857536,857536,857536,857536,857536,857536,857536,857536,857536,857566,857566,857566,857566,857566,857566,857566,857566,857566,857566,857566,857566,857566,857566,857566,857566,857566,857566,857566,857566,857566,857566,857576,857576,857576,857576,857576,857576,857576,857576,857576,857576,857576,857576,857576,857576,857576,857576,857969,857969,857969,857969,857969,858042,858042,858042,858042,858042,858137,858137,858137,858137,858345,858345,858354,858369,858369,858369,858369,858369,858369,858369,858369,858369,858369,858369,858491,858505,858505,858505,858505,858530,858530,858530,858530,858530,858530,858530,858530,858530,858530,858530,858530,858530,858530,858530,858530,858758,858758,858847,858847,858847,858847,858847,858847,858847,858847,858847,858875,858875,858875,858875,858875,858875,858875,858875,858875,858924,858924,858924,858924,858924,858924,858924,858924,858924,858924,858924,858924,858924,858924,858924,858924,858924,858924,858924,858924,858937,858937,858937,858937,858937,859014,859014,859014,859014,859014,859014,859014,859014,859014,859014,859014,859014,859101,859101,859101,859101,859101,859101,859101,859101,859101,859101,859101,859101,859101,859101,859101,859101,859101,859101,859101,859101,859101,859103,859103,859103,859103,859103,859103,859103,859103,859103,859103,859103,859103,859103,859103,859223,859223,859223,859223,859223,859223,859223,859223,859223,859223,859223,859223,859223,859223,859223,859223,859223,859223,859223,859223,859223,859223,859223,859223,859223,859223,859223,859223,859223,859223,859223,859223,859223,859223,859223,859236,859236,859236,859236,859236,859236,859236,859236,859236,859236,859236,859280,859280,859280,859280,859280,859280,859280,859280,859280,859280,859280,859280,859280,859280,859280,859280,859280,859280,859280,859335,859335,859335,859335,859335,859452,859452,859452,859452,859452,859452,859452,859452,859452,859452,859452,859452,859452,859452,859452,859452,859452,859452,859452,859454,859543,859543,859543,859652,859652,859652,859710,859710,859710,859710,859710,859710,859710,859710,859710,859710,859710,859710,859710,859710,859710,859710,859710,859710,859710,859710,859710,859710,859710,859710,859710,859710,859710,859710,859710,859710,859710,859710,859710,859710,859710,859749,859749,859749,859749,859749,859749,859749,859749,859749,859749,859749,859749,859754,859754,859754,859754,859754,859754,859754,859754,859754,859754,859754,859754,859755,859755,859755,859755,859755,859755,859755,859755,859755,859755,859755,859755,859755,859755,859755,859755,859759,859759,859759,859759,859759,859759,859759,859759,859759,859759,859759,859759,859759,859769,859803,859803,859803,859854,859854,859888,859888,859888,859888,859888,859888,859888,859888,859888,859888,859888,859888,859888,859888,859888,859888,859888,859888,859888,859888,859888,859888,859888,859888,859888,859888,859888,859888,859888,859888,859888,859888,860023,860030,860030,860030,860030,860030,860030,860030,860030,860030,860030,860030,860030,860030,860030,860030,860030,860030,860053,860053,860053,860053,860053,860053,860053,860053,860053,860053,860053,860053,860053,860053,860053,860053,860053,860053,860053,860053,860053,860093,860093,860093,860093,860093,860093,860093,860093,860093,860093,860093,860093,860093,860093,860093,860093,860093,860093,860093,860093,860093,860093,860093,860093,860093,860093,860093,860093,860093,860093,860093,860093,860093,860093,860093,860093,860093,860093,860093,860093,860093,860093,860093,860093,860093,860093,860093,860093,860093,860093,860093,860093,860093,860093,860093,860093,860093,860093,860093,860093,860093,860093,860093,860093,860188,860188,860188,860188,860188,860188,860188,860188,860188,860188,860188,860210,860221,860243,860251,860251,860251,860390,860424,860424,860424,860424,860424,860424,860424,860424,860424,860424,860424,860424,860424,860424,860424,860424,860424,860424,860424,860424,860424,860424,860424,860424,860432,860486,860574,860574,860994,860994,860994,860994,860994,860994,860994,860994,860994,860994,860994,860994,860994,860994,860994,860994,860994,860994,860994,860994,860994,860994,860994,860994,860994,860994,860994,860994,860994,860994,860994,860994,860994,860994,860994,860994,860994,860994,860994,860994,860994,860994,860994,860994,860994,860994,860994,860994,860994,860994,860994,860994,860994,860994,860994,860994,860994,860994,860994,860994,860994,860994,860994,860994,860994,860994,860994,860994,860994,860994,860994,860994,860994,860994,860994,860994,861051,861051,861051,861051,861051,861051,861051,861051,861051,861051,861051,861051,861051,861051,861051,861051,861051,861051,861051,861051,861051,861051,861051,861051,861051,861051,861051,861051,861061,861061,861061,861061,861061,861061,861061,861061,861061,861186,861186,861210,861210,861287,861287,861287,861378,861378,861378,861378,861378,861378,861378,861378,861435,861435,861627,861627,861770,861804,861804,861804,861804,861806,861806,861806,861806,861806,861806,861806,861806,861806,861806,861828,861889,861912,861912,861912,861912,861912,861912,861912,861912,861912,861912,861912,861912,861912,861912,861912,861912,861989,861989,861989,861989,861989,861989,861989,861989,861989,861989,861989,861989,861989,861989,861989,861989,861989,861989,861989,861989,861989,861989,861989,861989,861989,861989,861989,861998,861998,861998,861998,861998,861998,861998,862129,862129,862129,862129,862129,862129,862129,862129,862129,862129,862129,862129,862129,862129,862129,862129,862129,862196,862196,862196,862196,862196,862196,862196,862196,862196,862196,862196,862196,862196,862196,862196,862196,862196,862196,862213,862213,862213,862213,862213,862213,862213,862248,862248,862248,862248,862248,862248,862248,862248,862248,862268,862268,862268,862268,862268,862268,862268,862268,862268,862268,862268,862268,862268,862268,862268,862268,862268,862268,862268,862268,862268,862268,862268,862268,862268,862268,862268,862277,862277,862277,862350,862350,862350,862350,862350,862358,862358,862358,862358,862358,862358,862358,862358,862358,862358,862358,862358,862358,862358,862358,862358,862591,862684,862684,862684,862684,862684,862684,862684,862684,862684,862684,862843,862843,862843,862843,862843,862843,862843,862911,862911,862911,862911,862911,862911,862911,862911,862933,862933,862933,862933,862933,862933,862933,862933,862933,862933,862933,862933,862933,862933,862933,862933,862933,862933,862933,862933,862933,862933,862933,862933,862933,862933,862933,862933,862933,862933,862933,862933,862933,862933,862933,862933,862933,862968,862968,862968,862968,862968,862999,862999,862999,862999,863068,863068,863068,863262,863262,863262,863262,863262,863262,863262,863262,863262,863262,863262,863262,863262,863262,863262,863262,863262,863262,863262,863262,863274,863274,863274,863274,863274,863274,863274,863274,863274,863274,863274,863308,863308,863308,863308,863308,863308,863308,863308,863308,863308,863308,863355,863355,863355,863355,863355,863355,863355,863355,863355,863430,863430,863430,863556,863556,863556,863556,863679,863681,863681,863681,863681,863681,863681,863681,863705,863705,863705,863705,863707,863707,863707,863707,863707,863707,863707,863926,863926,863926,863926,863926,863926,863926,863926,863926,863926,863926,863926,863967,863967,863967,863989,864081,864215,864253,864253,864253,864253,864253,864253,864253,864253,864253,864253,864299,864299,864299,864299,864299,864299,864299,864299,864299,864299,864299,864299,864299,864299,864299,864299,864299,864299,864299,864299,864299,864299,864299,864299,864299,864299,864299,864299,864299,864299,864299,864299,864299,864299,864332,864332,864332,864332,864332,864332,864332,864332,864332,864332,864332,864332,864332,864332,864332,864332,864332,864332,864332,864332,864332,864332,864332,864332,864332,864332,864332,864332,864332,864332,864332,864332,864550,864550,864550,864550,864550,864550,864550,864550,864550,864550,864604,864604,864604,864604,864627,864809,864809,864862,864869,864869,864869,864869,864869,864869,864869,864892,864892,864892,864892,864892,864892,864892,864995,864995,864995,864995,864995,864995,864995,865001,865001,865001,865036,865074,865074,865074,865074,865074,865074,865074,865074,865074,865074,865074,865074,865074,865074,865074,865111,865111,865111,865111,865111,865111,865111,865111,865111,865111,865111,865111,865111,865111,865111,865111,865111,865111,865111,865111,865111,865111,865111,865111,865111,865111,865111,865111,865111,865111,865127,865127,865127,865127,865127,865127,865127,865127,865127,865127,865127,865127,865127,865127,865127,865127,865127,865127,865127,865127,865127,865153,865153,865153,865153,865153,865153,865153,865173,865181,865181,865196,865196,865196,865196,865196,865196,865196,865196,865196,865196,865196,865196,865196,865196,865200,865200,865200,865200,865200,865200,865200,865200,865200,865200,865200,865200,865200,865200,865200,865200,865200,865200,865213,865213,865213,865213,865213,865213,865213,865213,865213,865213,865213,865213,865213,865213,865213,865213,865213,865213,865217,865217,865217,865217,865217,865217,865217,865217,865217,865217,865217,865275,865275,865275,865275,865286,865286,865286,865286,865286,865286,865286,865286,865286,865286,865288,865448,865448,865448,865448,865448,865448,865448,865448,865448,865448,865448,865448,865448,865448,865448,865448,865448,865448,865448,865448,865448,865448,865448,865469,865469,865469,865469,865469,865469,865469,865469,865469,865469,865469,865469,865469,865555,865555,865555,865555,865555,865555,865555,865555,865555,865555,865555,865555,865555,865555,865555,865555,865555,865555,865555,865559,865559,865559,865559,865559,865559,865559,865559,865559,865559,865559,865559,865559,865559,865559,865559,865559,865559,865559,865559,865559,865559,865559,865559,865559,865559,865559,865559,865559,865559,865559,865559,865559,865559,865559,865559,865559,865559,865559,865559,865559,865559,865559,865559,865559,865560,865560,865560,865560,865560,865560,865560,865560,865560,865560,865560,865560,865560,865560,865560,865560,865561,865561,865561,865561,865561,865561,865561,865561,865561,865561,865561,865561,865561,865561,865561,865561,865561,865561,865561,865561,865561,865561,865561,865561,865561,865561,865561,865561,865561,865561,865561,865561,865561,865561,865561,865561,865561,865561,865561,865561,865561,865561,865561,865561,865561,865561,865561,865561,865561,865561,865561,865561,865561,865561,865561,865561,865561,865561,865561,865561,865561,865561,865561,865561,865561,865561,865561,865561,865561,865561,865581,865581,865581,865581,865614,865675,865675,865675,865706,865826,865826,865826,865860,865860,865860,865860,865860,865860,865860,865860,865860,865860,865860,865860,865860,865918,865918,865918,865918,865930,865930,865930,865930,865930,865930,865930,865930,865930,865930,865930,865930,865930,865930,865930,865930,866045,866045,866045,866045,866045,866045,866045,866045,866045,866045,866045,866045,866045,866045,866045,866045,866093,866093,866093,866093,866093,866093,866093,866093,866093,866093,866093,866093,866093,866093,866093,866093,866093,866093,866099,866099,866111,866111,866111,866111,866111,866152,866152,866152,866152,866152,866152,866152,866152,866152,866152,866152,866152,866152,866152,866152,866152,866239,866239,866239,866239,866239,866239,866239,866239,866239,866239,866239,866239,866239,866239,866239,866239,866239,866239,866239,866239,866239,866341,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866507,866672,866672,866672,866672,866672,866672,866672,866672,866672,866672,866672,866672,866691,866691,866781,866781,866781,866781,866781,866781,866781,866781,866781,866781,866781,866781,866781,866781,866781,866781,866781,866781,866781,866781,866781,866781,866781,866781,866781,866781,866781,866829,866829,866834,866834,866834,866834,866834,866834,866834,866834,866857,866857,866857,866898,866898,866898,866899,866899,866899,866899,866899,866899,866899,866899,866899,866899,866899,866899,866899,866899,866899,866899,866899,866899,866899,866899,866899,866899,866899,866899,866899,866899,866933,866933,866933,866933,866933,866933,866933,866933,866933,866933,866933,866933,866933,866933,866933,866933,866933,866933,866933,866933,866933,866933,866933,866933,866933,866933,866933,866933,866933,866933,866933,866933,866933,866933,866933,866933,866933,866933,866933,866933,866933,866933,866933,866933,866933,866933,867057,867057,867057,867057,867057,867057,867057,867057,867057,867057,867057,867057,867057,867057,867057,867057,867057,867057,867058,867058,867058,867058,867058,867077,867077,867078,867078,867078,867078,867148,867148,867148,867148,867148,867148,867148,867148,867148,867148,867148,867148,867148,867148,867222,867222,867222,867222,867222,867222,867222,867222,867222,867222,867222,867222,867222,867222,867222,867222,867222,867222,867222,867222,867222,867222,867222,867222,867222,867222,867222,867222,867222,867222,867222,867225,867225,867225,867225,867225,867225,867225,867225,867225,867225,867225,867225,867225,867225,867225,867225,867225,867225,867225,867225,867225,867225,867225,867225,867225,867225,867225,867275,867275,867275,867275,867275,867275,867275,867275,867275,867275,867275,867275,867275,867275,867275,867275,867275,867275,867275,867275,867275,867275,867275,867275,867275,867275,867275,867275,867275,867275,867275,867275,867309,867309,867309,867309,867386,867386,867388,867388,867388,867388,867388,867388,867388,867388,867451,867451,867451,867451,867458,867458,867458,867458,867458,867458,867458,867458,867458,867458,867458,867462,867462,867462,867462,867462,867462,867462,867470,867540,867540,867540,867540,867540,867540,867540,867540,867540,867540,867540,867540,867540,867540,867540,867540,867540,867540,867540,867540,867540,867540,867540,867540,867540,867540,867540,867540,867540,867540,867540,867540,867540,867540,867540,867592,867592,867605,867617,867625,867625,867625,867625,867625,867625,867625,867625,867625,867625,867640,867640,867640,867640,867726,867726,867726,867726,867726,867726,867726,867731,867731,867808,867808,867808,867808,867808,867808,867808,867808,867808,867808,867808,867808,867808,867808,867808,867808,867808,867808,867808,867808,867808,867808,867808,867808,867808,867808,867808,867808,867808,867808,867808,867808,867808,867808,867808,867808,867808,867808,867808,867808,867808,867808,867808,867808,867808,867808,867808,867808,867808,867808,867808,867808,867808,867808,867808,867808,867808,867808,867906,867906,867906,867906,867906,867906,867906,867906,867906,867906,867906,867906,867906,867906,867906,867906,867906,867906,867906,867906,867906,867906,867906,867906,867906,867906,867906,867906,867906,867906,867906,867906,867906,867906,867906,867906,867906,867906,867906,867906,867906,867936,867936,867936,867936,867936,867936,867977,867977,867977,867977,868066,868079,868201,868222,868222,868222,868222,868222,868222,868222,868222,868222,868222,868222,868222,868222,868222,868254,868254,868254,868254,868254,868254,868254,868254,868254,868254,868254,868266,868390,868457,868457,868461,868461,868461,868461,868464,868464,868464,868464,868464,868464,868464,868655,868655,868655,868655,868655,868655,868681,868765,868848,868848,868848,868848,868848,868848,868906,868906,868986,868986,868988,868988,868988,868988,869046,869046,869046,869046,869046,869046,869046,869046,869046,869046,869046,869046,869115,869115,869115,869152,869152,869211,869211,869211,869211,869211,869211,869211,869211,869211,869211,869223,869223,869223,869223,869223,869223,869223,869223,869223,869223,869223,869223,869223,869223,869223,869223,869223,869223,869223,869272,869282,869282,869282,869282,869282,869282,869282,869361,869410,869410,869410,869410,869410,869410,869410,869410,869410,869410,869410,869410,869410,869410,869410,869410,869410,869410,869410,869410,869410,869410,869410,869410,869410,869410,869410,869410,869410,869410,869410,869410,869410,869410,869410,869410,869410,869410,869410,869410,869410,869410,869410,869410,869410,869410,869410,869410,869410,869410,869410,869442,869442,869442,869442,869442,869442,869442,869442,869442,869453,869453,869453,869457,869457,869469,869469,869469,869469,869469,869469,869469,869469,869469,869469,869469,869469,869469,869469,869469,869469,869469,869469,869469,869469,869469,869469,869469,869469,869469,869469,869469,869469,869469,869469,869469,869469,869469,869469,869469,869469,869469,869469,869469,869469,869469,869469,869469,869469,869469,869469,869469,869469,869469,869469,869469,869469,869469,869469,869469,869469,869469,869483,869483,869483,869483,869483,869483,869483,869483,869483,869483,869483,869483,869483,869483,869483,869483,869483,869483,869483,869483,869483,869483,869483,869483,869483,869483,869483,869483,869483,869483,869483,869483,869483,869483,869483,869483,869483,869483,869483,869483,869483,869483,869483,869483,869483,869483,869483,869483,869483,869483,869483,869483,869483,869483,869483,869483,869483,869483,869483,869483,869483,869483,869483,869507,869507,869529,869529,869529,869529,869529,869529,869529,869529,869529,869529,869529,869529,869529,869529,869529,869529,869529,869529,869529,869529,869529,869529,869529,869529,869529,869529,869529,869529,869529,869529,869529,869529,869529,869529,869529,869529,869529,869529,869529,869529,869529,869529,869529,869529,869529,869529,869529,869529,869529,869529,869529,869529,869529,869530,869530,869535,869535,869538,869538,869538,869538,869538,869538,869538,869538,869538,869538,869538,869538,869538,869538,869538,869538,869538,869538,869619,869619,869619,869619,869627,869627,869665,869665,869665,869665,869731,869749,869749,869749,869749,869749,869749,869749,869749,869749,869749,869749,869749,869749,869749,869749,869749,869749,869749,869749,869749,869749,869749,869749,869749,869758,869758,869758,869985,869985,869985,869985,869985,870020,870020,870020,870026,870026,870026,870026,870026,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870071,870086,870086,870086,870086,870086,870086,870094,870094,870094,870226,870226,870226,870226,870226,870226,870226,870226,870226,870226,870226,870226,870226,870226,870226,870226,870226,870226,870226,870226,870226,870226,870226,870226,870226,870226,870226,870226,870254,870254,870254,870349,870349,870349,870349,870349,870349,870349,870349,870349,870349,870349,870349,870349,870349,870349,870349,870349,870349,870349,870349,870349,870349,870349,870349,870349,870349,870349,870349,870349,870349,870349,870349,870349,870349,870349,870349,870349,870349,870349,870349,870349,870349,870349,870349,870349,870349,870349,870349,870349,870349,870349,870349,870349,870349,870349,870349,870349,870396,870396,870396,870396,870396,870396,870396,870396,870396,870396,870396,870396,870396,870396,870396,870396,870396,870396,870396,870396,870396,870396,870396,870396,870396,870396,870396,870396,870396,870396,870396,870407,870429,870429,870438,870446,870446,870446,870446,870446,870469,870469,870469,870469,870469,870520,870520,870520,870520,870520,870556,870561,870580,870584,870653,870653,870653,870653,870653,870653,870653,870653,870653,870653,870653,870653,870653,870653,870653,870653,870653,870653,870653,870653,870653,870653,870653,870653,870653,870653,870653,870653,870653,870716,870735,870735,870735,870735,870735,870735,870735,870735,870735,870735,870735,870735,870735,870735,870735,870735,870735,870735,870744,870744,870744,870744,870744,870744,870744,870744,870744,870744,870744,870744,870744,870744,870744,870744,870744,870744,870744,870744,870744,870744,870744,870744,870744,870744,870744,870744,870744,870744,870744,870744,870744,870744,870744,870744,870744,870744,870744,870744,870744,870744,870744,870744,870744,870744,870744,870744,870744,870744,870744,870744,870744,870744,870744,870744,870810,870852,870852,870852,870852,870852,870852,870852,870852,870852,870852,870852,870852,870852,870852,870852,870853,870853,870853,870853,870853,870857,870857,870857,870857,870857,870857,870857,870857,870857,870857,870857,870857,870857,870857,870857,870857,870857,870929,870966,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871082,871188,871188,871188,871299,871299,871299,871299,871299,871299,871299,871299,871299,871299,871299,871299,871299,871299,871299,871299,871313,871313,871347,871347,871347,871366,871366,871366,871366,871366,871366,871366,871394,871430,871430,871430,871430,871430,871430,871491,871498,871498,871498,871498,871498,871498,871548,871548,871548,871548,871548,871548,871548,871622,871622,871622,871622,871622,871622,871622,871622,871622,871622,871622,871622,871622,871622,871622,871622,871622,871650,871650,871650,871650,871650,871650,871650,871650,871658,871658,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871675,871703,871720,871741,871741,871760,871760,871760,871760,871760,871760,871760,871813,871813,871857,871857,871857,871857,871857,871857,871857,871857,871857,871857,871857,871857,871857,871857,871857,871930,871930,871930,871930,871930,871930,871930,871930,871930,871930,871930,872091,872091,872091,872091,872091,872091,872091,872091,872091,872091,872091,872091,872118,872118,872118,872118,872118,872118,872118,872118,872118,872118,872118,872118,872118,872118,872118,872118,872118,872118,872118,872135,872135,872135,872169,872185,872303,872303,872303,872303,872404,872404,872404,872404,872404,872404,872404,872404,872404,872404,872433,872433,872503,872503,872503,872503,872503,872522,872522,872522,872522,872522,872522,872522,872522,872522,872522,872560,872560,872560,872594,872594,872594,872594,872594,872594,872636,872657,872661,872661,872661,872661,872661,872661,872661,872661,872661,872661,872661,872692,872692,872692,872692,872692,872692,872692,872692,872692,872692,872692,872692,872692,872692,872692,872692,872692,872692,872692,872692,872692,872692,872692,872692,872692,872692,872692,872692,872692,872692,872692,872692,872692,872692,872692,872692,872724,872746,872746,872746,872746,872746,872746,872746,872746,872746,872746,872746,872788,872814,872814,872814,872814,872814,872814,872869,872869,872869,872869,872869,872869,872869,872869,872869,872869,872869,872869,872869,872869,872869,872869,872869,872869,872869,872869,872869,872869,872869,872869,872869,872869,872869,872869,872869,872869,872869,872869,872869,872869,872869,872869,872869,872869,872869,872869,872869,872869,872869,872869,872869,872869,872869,872869,872869,872869,872869,872869,872883,872883,872886,873081,873081,873081,873081,873081,873081,873081,873081,873081,873081,873081,873081,873081,873081,873081,873081,873081,873081,873081,873081,873081,873081,873081,873081,873081,873081,873081,873081,873081,873081,873081,873081,873081,873081,873081,873081,873081,873081,873081,873081,873081,873081,873081,873081,873081,873081,873081,873081,873081,873081,873081,873081,873081,873081,873081,873081,873081,873081,873081,873081,873081,873081,873081,873081,873081,873081,873081,873081,873081,873081,873081,873081,873118,873118,873118,873118,873327,873327,873327,873327,873327,873327,873327,873327,873327,873327,873327,873327,873327,873327,873327,873327,873327,873327,873327,873327,873327,873378,873381,873413,873413,873413,873413,873413,873413,873413,873413,873413,873413,873413,873413,873413,873413,873413,873413,873413,873413,873413,873413,873413,873413,873413,873413,873413,873413,873413,873413,873413,873413,873413,873413,873413,873413,873413,873413,873413,873413,873413,873459,873459,873459,873459,873459,873459,873459,873459,873459,873459,873459,873559,873559,873559,873559,873559,873559,873559,873559,873559,873559,873559,873559,873559,873559,873559,873559,873559,873559,873559,873559,873559,873559,873559,873559,873559,873559,873559,873559,873559,873559,873559,873559,873559,873559,873559,873559,873559,873559,873559,873559,873559,873559,873559,873559,873559,873559,873559,873559,873559,873559,873559,873559,873559,873559,873559,873559,873559,873563,873563,873563,873563,873563,873563,873563,873563,873563,873563,873563,873563,873563,873563,873563,873563,873563,873563,873563,873563,873609,873609,873609,873609,873609,873609,873609,873609,873609,873609,873609,873609,873609,873609,873609,873609,873609,873609,873609,873609,873609,873609,873609,873609,873609,873609,873630,873630,873630,873630,873763,873779,873779,873852,873852,873852,873852,873852,873852,873852,873852,873852,873852,873852,873852,873852,873852,873852,873852,873852,873852,873852,873852,873852,873852,873852,873852,873852,873852,873852,873852,873852,873852,873875,873875,873887,873887,873887,873887,873887,873887,873887,873887,873887,873887,873887,873887,873887,873887,873887,873887,873887,873887,873887,873887,873887,873887,873887,873887,873887,873887,873887,873887,873887,873887,873887,873887,873887,873887,873996,873996,874021,874021,874021,874021,874021,874021,874031,874044,874044,874044,874044,874044,874044,874044,874044,874044,874044,874044,874044,874044,874044,874044,874044,874044,874044,874044,874044,874044,874044,874044,874044,874044,874044,874044,874044,874044,874085,874085,874085,874085,874085,874085,874085,874085,874085,874085,874092,874092,874092,874092,874092,874092,874092,874092,874092,874092,874092,874092,874092,874092,874092,874092,874092,874092,874092,874092,874092,874092,874092,874092,874092,874092,874092,874092,874092,874180,874180,874180,874180,874180,874180,874180,874180,874180,874180,874180,874180,874180,874180,874180,874180,874180,874180,874180,874180,874180,874180,874180,874180,874180,874180,874180,874180,874180,874180,874180,874180,874180,874180,874180,874180,874180,874180,874180,874180,874180,874180,874180,874180,874180,874180,874180,874180,874180,874180,874180,874180,874202,874202,874202,874220,874220,874220,874220,874220,874220,874220,874220,874220,874220,874221,874221,874221,874221,874221,874221,874221,874221,874221,874221,874221,874221,874221,874221,874221,874221,874221,874221,874221,874221,874221,874221,874221,874221,874221,874221,874221,874221,874221,874221,874221,874221,874221,874240,874240,874240,874240,874240,874240,874240,874240,874240,874240,874240,874240,874240,874240,874240,874240,874240,874240,874240,874240,874240,874240,874240,874240,874240,874240,874240,874240,874240,874240,874240,874240,874240,874240,874240,874240,874240,874240,874240,874240,874240,874240,874240,874240,874240,874240,874240,874240,874240,874240,874240,874240,874240,874240,874240,874240,874240,874240,874240,874240,874240,874240,874240,874240,874240,874240,874240,874240,874240,874240,874240,874240,874240,874240,874240,874240,874240,874241,874241,874241,874241,874241,874241,874241,874241,874241,874241,874241,874241,874241,874241,874241,874241,874241,874241,874241,874347,874347,874358,874363,874371,874371,874371,874371,874371,874371,874371,874371,874371,874371,874371,874371,874371,874371,874371,874371,874371,874371,874371,874371,874371,874371,874371,874371,874371,874371,874371,874371,874371,874371,874371,874371,874371,874371,874371,874371,874371,874371,874371,874371,874371,874371,874404,874404,874404,874404,874470,874470,874470,874470,874470,874470,874470,874470,874470,874470,874470,874470,874470,874470,874470,874470,874470,874470,874470,874470,874470,874486,874490,874490,874490,874490,874511,874521,874521,874521,874521,874521,874533,874533,874533,874533,874533,874533,874533,874565,874565,874582,874582,874582,874582,874582,874582,874582,874582,874582,874582,874589,874589,874589,874589,874589,874589,874589,874589,874589,874589,874589,874589,874589,874589,874589,874589,874589,874589,874589,874589,874589,874589,874589,874590,874590,874590,874590,874590,874590,874590,874590,874590,874590,874591,874591,874614,874614,874614,874614,874614,874614,874624,874624,874624,874624,874635,874635,874635,874635,874635,874635,874635,874635,874635,874635,874635,874635,874635,874635,874635,874635,874635,874635,874635,874635,874635,874635,874635,874635,874635,874635,874635,874635,874635,874635,874635,874635,874635,874635,874635,874635,874635,874729,874808,874898,874898,874898,874898,874898,874898,874898,874898,874898,874898,874898,874898,874898,874898,874898,874898,874898,874898,874898,874898,874898,874898,874898,874898,874898,874898,874898,874898,874898,874898,874898,874898,874898,874898,874898,874898,874898,874898,874898,874898,874898,874898,874898,874898,874898,874898,874898,874898,874898,874898,874898,874898,874898,874898,874898,874898,874898,874898,874898,874898,874898,874898,874965,874965,874965,874965,874965,874965,874965,874965,874965,874965,874965,874965,874965,874965,874965,874965,874965,874965,874965,874965,874965,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,874990,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875081,875095,875095,875095,875095,875095,875095,875095,875095,875095,875095,875095,875095,875095,875128,875128,875128,875128,875128,875128,875128,875128,875128,875128,875130,875130,875130,875130,875130,875130,875130,875130,875130,875130,875130,875130,875130,875130,875130,875130,875130,875149,875149,875149,875149,875149,875149,875149,875149,875149,875149,875149,875149,875149,875149,875149,875149,875149,875149,875149,875149,875156,875156,875156,875156,875204,875204,875204,875204,875215,875215,875215,875215,875215,875215,875215,875215,875216,875216,875216,875216,875216,875216,875216,875216,875216,875216,875216,875216,875216,875216,875216,875246,875246,875246,875246,875246,875246,875246,875246,875246,875246,875246,875246,875246,875246,875246,875246,875246,875262,875262,875262,875262,875262,875262,875262,875262,875262,875262,875271,875271,875271,875271,875271,875271,875271,875271,875271,875271,875271,875271,875271,875288,875300,875300,875300,875300,875300,875300,875300,875300,875300,875300,875300,875300,875300,875319,875319,875319,875319,875319,875319,875319,875319,875319,875319,875319,875319,875355,875364,875364,875422,875422,875422,875422,875422,875422,875422,875461,875461,875461,875461,875461,875461,875461,875461,875461,875461,875471,875481,875481,875481,875481,875481,875481,875481,875481,875481,875481,875481,875514,875514,875514,875514,875514,875514,875514,875514,875514,875514,875514,875514,875514,875514,875514,875514,875514,875514,875514,875514,875514,875514,875514,875514,875514,875526,875526,875526,875526,875585,875585,875585,875585,875585,875585,875585,875585,875585,875585,875585,875585,875585,875585,875585,875585,875585,875585,875585,875585,875608,875718,875718,875718,875718,875718,875718,875724,875724,875724,875724,875724,875724,875724,875724,875724,875724,875724,875724,875724,875724,875724,875724,875724,875724,875724,875724,875724,875724,875724,875724,875724,875724,875910,875910,875910,875910,875910,875910,875910,875910,875910,875910,875910,875910,875910,875978,876064,876064,876064,876064,876064,876064,876064,876064,876064,876064,876064,876064,876064,876064,876064,876064,876064,876064,876064,876064,876064,876064,876064,876064,876064,876064,876064,876064,876064,876064,876064,876064,876064,876064,876064,876064,876064,876064,876103,876103,876103,876153,876173,876173,876173,876173,876173,876173,876173,876173,876173,876173,876173,876173,876173,876173,876173,876173,876173,876173,876173,876173,876173,876173,876173,876173,876173,876173,876173,876173,876173,876173,876173,876173,876173,876173,876173,876173,876173,876173,876173,876173,876173,876180,876180,876241,876241,876241,876241,876241,876241,876241,876241,876241,876241,876241,876269,876269,876269,876269,876269,876269,876269,876269,876269,876269,876269,876269,876269,876269,876269,876269,876269,876269,876269,876269,876269,876269,876269,876269,876269,876269,876269,876269,876269,876269,876269,876269,876282,876301,876301,876301,876301,876301,876301,876301,876301,876301,876301,876301,876301,876301,876301,876301,876301,876334,876334,876334,876334,876334,876334,876334,876334,876334,876355,876366,876366,876366,876366,876366,876366,876366,876366,876366,876366,876366,876366,876366,876366,876366,876366,876366,876366,876366,876366,876366,876366,876366,876366,876366,876366,876366,876366,876366,876366,876366,876366,876366,876366,876366,876377,876377,876395,876395,876395,876395,876395,876395,876395,876395,876395,876395,876395,876395,876395,876395,876395,876395,876395,876395,876395,876395,876395,876395,876395,876395,876395,876395,876395,876395,876395,876395,876395,876395,876395,876395,876395,876395,876395,876395,876395,876427,876427,876427,876427,876427,876427,876427,876427,876456,876456,876683,876693,876693,876693,876693,876693,876693,876693,876693,876693,876693,876693,876693,876748,876748,876748,876748,876748,876748,876748,876748,876748,876748,876748,876748,876748,876748,876748,876748,876748,876748,876748,876748,876748,876748,876748,876748,876748,876748,876748,876748,876748,876748,876748,876748,876748,876748,876748,876748,876748,876748,876748,876748,876748,876748,876748,876748,876748,876748,876748,876777,876777,876777,876777,876777,876777,876777,876777,876777,876777,876777,876777,876777,876777,876777,876777,876777,876777,876777,876777,876777,876777,876777,876777,876777,876777,876777,876777,876777,876777,876777,876777,876777,876777,876777,876777,876777,876777,876777,876777,876777,876777,876777,876777,876777,876777,876777,876777,876777,876777,876777,876777,876777,876777,876777,876777,876777,876777,876777,876777,876777,876777,876777,876777,876777,876777,876777,876777,876777,876777,876777,876834,876834,876834,876834,876834,876834,876834,876838,876889,876889,876889,876889,876889,876889,876889,876889,876889,876889,876889,876889,876889,876889,876889,876889,876889,876889,876889,876889,876889,876889,876890,876890,876892,876892,876892,876892,876892,876892,876892,876892,876892,876892,876892,876892,876892,876892,876892,876892,876892,876904,876904,876904,876904,876904,876904,876904,876914,876914,876914,876914,876914,876914,876916,876916,876916,876946,876946,876946,876946,876946,876952,876961,876961,876961,876961,876961,876961,876961,876961,877004,877004,877004,877004,877004,877004,877004,877004,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877007,877017,877017,877017,877017,877017,877017,877017,877017,877017,877017,877017,877027,877027,877070,877070,877070,877070,877070,877070,877070,877077,877077,877077,877077,877077,877077,877077,877077,877077,877077,877077,877093,877093,877093,877093,877093,877093,877093,877093,877093,877093,877093,877093,877093,877093,877093,877093,877093,877093,877093,877093,877093,877093,877093,877093,877093,877109,877136,877136,877136,877136,877136,877136,877139,877139,877167,877167,877189,877189,877189,877189,877189,877201,877339,877339,877339,877339,877339,877339,877339,877339,877339,877339,877339,877339,877339,877339,877339,877339,877339,877339,877339,877339,877339,877339,877339,877339,877339,877339,877339,877360,877360,877360,877360,877371,877371,877371,877371,877371,877371,877371,877376,877376,877376,877376,877376,877376,877376,877376,877376,877376,877376,877376,877376,877376,877378,877378,877379,877379,877379,877379,877379,877388,877388,877388,877388,877388,877388,877388,877388,877388,877388,877388,877388,877388,877450,877450,877450,877450,877450,877450,877516,877516,877516,877516,877516,877516,877516,877516,877516,877516,877516,877516,877516,877532,877532,877532,877549,877549,877549,877549,877549,877549,877549,877549,877549,877549,877549,877549,877549,877549,877583,877583,877583,877583,877583,877583,877583,877583,877583,877583,877583,877583,877583,877583,877583,877583,877583,877583,877583,877583,877583,877603,877603,877603,877603,877603,877603,877603,877618,877618,877618,877618,877618,877618,877618,877637,877637,877637,877637,877637,877644,877656,877656,877656,877656,877656,877656,877656,877656,877656,877656,877656,877656,877673,877673,877673,877673,877673,877673,877673,877673,877673,877673,877673,877673,877673,877673,877673,877673,877673,877673,877673,877673,877673,877673,877673,877673,877673,877673,877673,877673,877673,877673,877673,877673,877673,877673,877673,877673,877673,877673,877673,877673,877673,877673,877673,877673,877673,877673,877673,877673,877673,877673,877673,877673,877673,877673,877673,877673,877673,877673,877673,877673,877673,877673,877673,877673,877673,877673,877673,877673,877673,877673,877673,877757,877757,877757,877757,877757,877757,877757,877763,877763,877763,877763,877763,877763,877763,877763,877763,877763,877763,877763,877763,877763,877763,877763,877763,877763,877763,877763,877763,877763,877763,877763,877763,877763,877763,877763,877763,877763,877763,877763,877763,877765,877781,877781,877781,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877800,877811,877811,877811,877811,877811,877811,877811,877811,877811,877835,877835,877845,877845,877845,877845,877845,877845,877845,877845,877845,877845,877845,877845,877845,877845,877845,877845,877845,877874,877891,877891,877891,877891,877891,877891,877891,877891,877891,877891,877907,877907,877907,877907,877907,877907,877907,877907,877907,877907,877907,877907,877907,877907,877907,877907,877907,877907,877909,877909,877909,877909,877909,877909,877909,877909,877909,877909,877909,877909,877909,877909,877909,877909,877909,877909,877909,877924,877924,877924,877924,877924,877924,877924,877924,877924,877924,877924,877924,877924,877924,877924,877924,877924,877924,877924,877924,877924,877924,877924,877924,877976,877976,877976,877976,877976,877976,877976,877976,877976,877976,877976,877976,877976,877976,877976,877976,877976,877976,877976,877976,877976,877976,877976,877976,877976,877976,877976,877976,877976,877976,877976,877976,877998,878002,878002,878015,878015,878015,878015,878015,878015,878015,878015,878015,878015,878015,878015,878015,878015,878015,878015,878015,878015,878015,878015,878015,878015,878015,878015,878015,878015,878015,878015,878015,878015,878015,878015,878015,878015,878015,878015,878015,878015,878015,878015,878015,878015,878015,878015,878015,878015,878015,878015,878015,878018,878018,878018,878018,878018,878018,878018,878018,878019,878019,878019,878019,878019,878019,878019,878019,878019,878019,878019,878019,878023,878025,878025,878025,878025,878025,878025,878025,878025,878025,878025,878025,878029,878029,878029,878029,878029,878029,878029,878029,878029,878029,878029,878029,878029,878029,878029,878029,878036,878036,878083,878083,878083,878112,878112,878112,878112,878112,878112,878112,878112,878112,878112,878112,878112,878112,878112,878112,878112,878112,878112,878112,878112,878112,878112,878112,878112,878112,878112,878150,878150,878150,878150,878150,878150,878150,878150,878150,878151,878151,878151,878151,878162,878162,878162,878162,878162,878162,878162,878162,878162,878162,878162,878162,878162,878162,878162,878162,878162,878162,878162,878162,878162,878162,878176,878176,878176,878176,878176,878176,878176,878176,878176,878176,878176,878176,878286,878289,878289,878289,878290,878290,878291,878435,878435,878435,878494,878494,878494,878494,878494,878494,878494,878494,878494,878494,878547,878568,878568,878568,878582,878612,878631,878631,878631,878641,878641,878641,878653,878653,878653,878653,878653,878653,878653,878653,878653,878653,878653,878653,878653,878663,878663,878663,878663,878663,878663,878663,878663,878723,878723,878723,878723,878760,878760,878760,878760,878760,878770,878770,878770,878770,878770,878770,878770,878770,878770,878770,878770,878770,878813,878813,878813,878813,878813,878813,878857,878857,878857,878857,878857,878857,878857,878857,878857,878857,878857,878857,878857,878857,878857,878857,878857,878857,878857,878857,878881,878881,878881,878881,878886,878886,878886,878886,878886,878886,878886,878886,878886,878886,878886,878886,878886,878886,878886,878886,878886,878886,878886,878886,878886,878886,878886,878886,878886,878886,878886,878886,878886,878886,878886,878886,878886,878886,878886,878886,878886,878886,878886,878886,878886,878886,878886,878886,878886,878886,878886,878886,878887,878934,878934,878934,878934,878934,878934,878934,878934,878934,878947,878947,878947,878947,878947,878947,878947,878949,878949,878949,878949,878949,878949,878949,878949,878949,878949,878949,878949,878949,878952,878952,878952,878952,878952,878952,878952,878952,878952,878952,878952,878952,878952,878952,878952,878952,878952,878963,878963,878963,878963,878971,878971,878971,878971,878971,878995,878995,878995,878995,879013,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879016,879067,879067,879067,879067,879067,879067,879067,879067,879067,879067,879067,879067,879067,879067,879067,879067,879067,879067,879067,879067,879067,879067,879067,879067,879067,879067,879067,879067,879067,879067,879067,879067,879067,879067,879067,879067,879067,879067,879067,879085,879085,879085,879085,879085,879089,879089,879089,879089,879089,879089,879089,879093,879093,879093,879093,879154,879154,879154,879154,879154,879154,879154,879154,879177,879177,879177,879177,879177,879177,879177,879177,879177,879207,879207,879207,879207,879207,879207,879207,879207,879207,879253,879253,879253,879253,879253,879253,879253,879253,879253,879253,879253,879253,879253,879253,879253,879253,879253,879253,879353,879353,879353,879353,879353,879390,879451,879464,879464,879489,879489,879489,879489,879503,879503,879503,879503,879574,879583,879583,879583,879583,879583,879583,879583,879583,879583,879583,879583,879583,879583,879583,879583,879608,879608,879608,879608,879608,879608,879608,879608,879608,879608,879608,879608,879608,879608,879608,879608,879608,879608,879608,879608,879608,879608,879608,879608,879608,879608,879608,879608,879608,879608,879608,879608,879608,879608,879608,879608,879608,879675,879675,879675,879675,879675,879675,879675,879675,879675,879686,879686,879686,879686,879686,879686,879686,879686,879719,879719,879747,879747,879747,879747,879747,879747,879747,879747,879747,879747,879747,879747,879747,879747,879747,879761,879761,879761,879761,879761,879761,879761,879785,879785,879785,879829,879846,879846,879846,879877,879877,879877,879877,879877,879877,879877,879877,879877,879877,879877,879877,879877,879877,879877,879877,879877,879877,879942,879966,879966,879973,879992,879992,879992,879992,879992,879992,879992,879992,879992,879992,879992,880053,880054,880143,880143,880143,880143,880143,880143,880143,880143,880143,880143,880143,880143,880143,880143,880143,880143,880143,880143,880143,880143,880151,880151,880151,880151,880151,880151,880151,880151,880151,880151,880151,880151,880151,880151,880151,880151,880151,880151,880151,880151,880151,880151,880151,880151,880151,880151,880151,880151,880151,880151,880151,880151,880162,880162,880162,880165,880165,880165,880211,880221,880221,880221,880221,880221,880221,880221,880221,880221,880239,880239,880239,880246,880246,880246,880246,880264,880264,880264,880264,880264,880264,880264,880264,880264,880264,880264,880264,880264,880264,880264,880264,880264,880264,880264,880264,880264,880264,880320,880325,880325,880326,880326,880326,880326,880326,880326,880326,880423,880432,880432,880432,880432,880432,880432,880432,880432,880432,880432,880469,880469,880469,880469,880512,880512,880512,880512,880512,880512,880512,880512,880512,880512,880512,880512,880512,880512,880512,880512,880512,880512,880512,880512,880512,880512,880512,880512,880512,880512,880512,880512,880512,880512,880512,880512,880512,880512,880512,880512,880512,880512,880512,880512,880512,880512,880512,880512,880512,880512,880512,880512,880512,880512,880512,880512,880512,880512,880512,880512,880512,880512,880512,880512,880512,880512,880512,880512,880512,880512,880512,880512,880522,880522,880522,880547,880547,880547,880547,880547,880547,880547,880547,880547,880547,880547,880547,880547,880547,880569,880569,880569,880569,880578,880693,880693,880712,880712,880712,880712,880712,880712,880712,880712,880712,880712,880712,880712,880712,880742,880742,880742,880742,880742,880742,880742,880742,880815,880815,880815,880815,880842,880842,880842,880869,880903,880903,880903,880903,880903,880903,880903,880903,880903,880903,880903,880903,880903,880903,880903,880903,880903,880903,880903,880903,880903,880903,880903,880903,880903,880903,880903,880903,880903,880903,880903,880903,880903,880903,880903,880903,880903,880903,880903,880903,880903,880903,880903,880903,880903,880903,880903,880903,880903,880903,880903,880903,880903,880903,880903,880903,880903,880903,880903,880903,880903,880903,880903,880903,880903,880903,880903,880903,880903,880903,880903,880903,880903,880903,880999,881014,881014,881014,881014,881014,881014,881014,881014,881014,881014,881014,881014,881014,881014,881014,881014,881014,881014,881014,881014,881014,881014,881014,881015,881015,881015,881015,881015,881015,881015,881015,881017,881017,881017,881017,881017,881017,881017,881017,881017,881017,881017,881017,881017,881017,881017,881017,881017,881017,881017,881017,881017,881017,881017,881017,881017,881017,881017,881017,881017,881017,881017,881017,881017,881017,881017,881017,881017,881017,881017,881017,881017,881017,881017,881017,881017,881017,881017,881017,881017,881017,881017,881017,881017,881017,881017,881017,881017,881017,881017,881017,881017,881017,881017,881017,881017,881017,881017,881017,881017,881017,881017,881017,881017,881017,881017,881017,881017,881029,881029,881029,881029,881029,881029,881029,881029,881029,881029,881029,881029,881029,881029,881029,881029,881029,881029,881029,881029,881029,881029,881029,881029,881029,881029,881052,881052,881052,881052,881052,881052,881052,881106,881106,881106,881106,881106,881106,881106,881106,881106,881106,881106,881106,881106,881106,881106,881106,881106,881106,881106,881106,881106,881106,881106,881106,881106,881106,881106,881106,881106,881106,881106,881106,881106,881106,881106,881106,881106,881106,881106,881106,881146,881163,881163,881163,881163,881163,881163,881163,881163,881163,881163,881163,881163,881163,881163,881163,881163,881163,881163,881163,881163,881163,881163,881163,881163,881163,881163,881163,881163,881163,881163,881163,881163,881163,881163,881163,881163,881163,881163,881163,881163,881163,881163,881163,881163,881163,881163,881163,881163,881163,881163,881163,881163,881163,881163,881163,881163,881163,881163,881163,881163,881163,881163,881163,881163,881233,881233,881233,881233,881233,881233,881241,881241,881241,881257,881257,881257,881257,881257,881257,881257,881277,881277,881277,881277,881277,881277,881277,881277,881277,881277,881277,881278,881278,881278,881278,881278,881278,881278,881281,881281,881281,881281,881281,881281,881293,881293,881302,881302,881302,881302,881302,881302,881302,881302,881302,881302,881302,881302,881319,881319,881319,881319,881319,881398,881398,881398,881402,881402,881402,881402,881402,881402,881402,881402,881402,881402,881402,881402,881402,881402,881402,881402,881402,881402,881402,881402,881402,881402,881402,881402,881402,881402,881402,881402,881402,881402,881402,881402,881402,881402,881402,881402,881402,881402,881402,881402,881402,881402,881402,881402,881402,881402,881402,881402,881407,881435,881435,881435,881435,881435,881435,881435,881435,881435,881435,881435,881435,881435,881435,881435,881435,881435,881435,881435,881435,881435,881435,881440,881516,881516,881516,881516,881516,881516,881516,881516,881516,881516,881516,881516,881516,881516,881516,881528,881528,881528,881528,881528,881528,881528,881528,881528,881528,881528,881528,881528,881528,881528,881528,881528,881528,881528,881528,881528,881528,881528,881528,881528,881528,881528,881528,881528,881528,881528,881528,881528,881528,881528,881528,881528,881528,881530,881530,881530,881530,881530,881584,881584,881638,881638,881638,881679,881679,881679,881679,881679,881679,881679,881679,881679,881679,881679,881679,881679,881679,881679,881679,881679,881679,881679,881679,881679,881679,881679,881679,881679,881679,881679,881679,881679,881679,881709,881709,881709,881709,881714,881715,881715,881715,881715,881715,881715,881715,881715,881715,881814,881863,881863,881863,881863,881863,881863,881863,881863,881863,881863,881863,881872,881910,881970,881970,881970,881970,881970,881970,881970,881970,881970,881970,881970,881970,881970,881970,881970,881970,881970,881970,881977,881977,881977,881977,882029,882029,882029,882029,882029,882124,882124,882124,882124,882130,882160,882160,882160,882160,882160,882160,882160,882160,882160,882160,882202,882202,882269,882269,882269,882269,882269,882269,882269,882284,882284,882284,882284,882284,882284,882284,882284,882284,882284,882284,882284,882284,882306,882306,882306,882348,882348,882348,882348,882348,882348,882348,882348,882348,882348,882348,882348,882348,882348,882349,882400,882400,882421,882421,882421,882421,882421,882421,882421,882421,882424,882438,882438,882438,882438,882438,882438,882438,882438,882438,882438,882438,882438,882438,882438,882438,882506,882506,882506,882506,882506,882506,882506,882506,882527,882527,882527,882527,882527,882527,882530,882530,882530,882530,882530,882530,882530,882530,882530,882530,882530,882530,882557,882557,882557,882563,882563,882563,882563,882588,882588,882588,882588,882588,882588,882588,882591,882624,882624,882624,882631,882631,882631,882631,882631,882631,882631,882667,882667,882673,882696,882696,882696,882713,882713,882713,882713,882713,882713,882713,882713,882713,882713,882713,882713,882713,882713,882713,882713,882713,882713,882713,882713,882713,882713,882713,882713,882713,882713,882713,882713,882713,882713,882713,882713,882713,882713,882713,882713,882713,882713,882713,882713,882713,882713,882713,882713,882713,882713,882713,882713,882713,882713,882713,882713,882713,882713,882713,882713,882713,882713,882713,882713,882713,882713,882713,882713,882713,882713,882713,882713,882713,882713,882713,882713,882713,882713,882713,882713,882713,882713,882713,882713,882730,882743,882743,882743,882743,882743,882743,882743,882743,882743,882743,882743,882743,882743,882743,882743,882743,882743,882743,882743,882743,882743,882743,882748,882748,882748,882748,882748,882748,882748,882748,882760,882760,882760,882760,882760,882760,882760,882760,882760,882760,882760,882760,882774,882774,882808,882808,882808,882808,882808,882808,882809,882817,882817,882817,882817,882817,882817,882817,882817,882817,882817,882828,882883,882886,882886,882886,882886,882886,882891,882891,882891,882891,882891,882891,882891,882891,882891,882891,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882897,882944,882944,882944,882944,882944,882944,882944,882944,882944,882944,882944,882944,882944,882944,882944,882944,882944,882944,882944,882944,882944,882944,882944,882944,882944,882944,882944,882944,882969,882969,882969,882974,882974,883088,883088,883088,883088,883088,883088,883088,883088,883088,883088,883088,883088,883088,883088,883088,883088,883088,883088,883088,883088,883088,883088,883088,883088,883088,883088,883088,883088,883088,883094,883094,883094,883094,883094,883094,883094,883163,883163,883163,883163,883163,883163,883163,883163,883163,883163,883168,883168,883168,883191,883191,883191,883191,883191,883191,883191,883191,883191,883191,883191,883191,883191,883191,883191,883191,883191,883191,883191,883191,883191,883191,883191,883191,883191,883191,883191,883191,883191,883191,883191,883191,883191,883191,883191,883191,883191,883191,883191,883191,883197,883197,883197,883204,883204,883204,883204,883204,883204,883204,883204,883204,883204,883204,883218,883218,883218,883218,883218,883218,883218,883227,883227,883227,883227,883227,883230,883230,883233,883254,883254,883254,883254,883254,883254,883254,883286,883286,883309,883309,883309,883309,883309,883309,883309,883309,883309,883309,883309,883309,883309,883309,883309,883309,883368,883368,883368,883368,883368,883368,883368,883368,883410,883410,883410,883410,883418,883471,883496,883496,883496,883496,883496,883496,883496,883496,883496,883496,883496,883496,883496,883496,883496,883496,883496,883496,883496,883496,883496,883512,883512,883512,883512,883512,883513,883538,883646,883646,883646,883720,883720,883720,883720,883720,883720,883720,883720,883720,883720,883720,883720,883720,883720,883720,883747,883747,883747,883747,883747,883747,883747,883747,883747,883757,883757,883757,883757,883757,883757,883757,883757,883757,883757,883757,883757,883757,883757,883768,883768,883768,883770,883770,883770,883770,883770,883770,883770,883770,883770,883770,883770,883770,883770,883770,883770,883770,883770,883770,883770,883770,883770,883770,883770,883770,883770,883770,883770,883786,883786,883786,883786,883786,883786,883786,883786,883786,883786,883786,883786,883786,883786,883786,883786,883797,883797,883797,883797,883797,883797,883797,883797,883797,883797,883797,883797,883797,883797,883797,883797,883797,883797,883797,883797,883797,883870,883870,883870,883870,883870,883870,883870,883870,883893,883907,883931,883971,883971,883971,883971,883971,883994,883994,883994,883994,883994,883994,883994,883994,883994,883994,883994,883994,883994,883994,883994,883994,883994,883994,883994,883994,883994,883994,883994,883994,883994,883994,883994,883994,884025,884025,884025,884025,884025,884025,884026,884026,884026,884026,884026,884124,884124,884194,884194,884194,884219,884219,884219,884219,884219,884219,884233,884233,884233,884233,884233,884233,884233,884254,884264,884264,884264,884264,884264,884264,884264,884349,884349,884349,884412,884412,884412,884412,884412,884412,884412,884412,884412,884421,884421,884421,884421,884421,884421,884421,884455,884455,884455,884455,884455,884455,884455,884455,884455,884455,884455,884455,884455,884455,884455,884461,884461,884461,884461,884461,884497,884542,884542,884542,884542,884542,884542,884542,884542,884542,884542,884596,884596,884618,884618,884618,884618,884618,884618,884618,884618,884618,884642,884642,884642,884642,884644,884644,884644,884677,884677,884677,884677,884677,884677,884677,884677,884677,884677,884677,884677,884705,884705,884705,884705,884705,884705,884732,884759,884759,884759,884760,884760,884760,884760,884760,884760,884760,884760,884760,884760,884760,884760,884760,884760,884772,884772,884772,884772,884772,884772,884772,884772,884772,884772,884772,884772,884772,884772,884772,884772,884772,884772,884772,884774,884816,884818,884818,884818,884818,884818,884818,884818,884818,884818,884818,884818,884818,884820,884820,884820,884820,884820,884820,884820,884820,884820,884826,884845,884845,884845,884845,884845,884845,884845,884864,884864,884885,884885,884885,884885,884885,884901,884903,884903,884926,884926,884926,884926,884926,884926,884939,884939,884939,884939,884939,884939,884939,884939,884939,884939,884939,884939,884939,884939,884939,884939,884939,884939,884939,884939,884939,884939,884939,884939,884939,884939,884965,885015,885015,885015,885015,885015,885028,885032,885034,885061,885061,885061,885061,885061,885061,885062,885064,885085,885085,885085,885085,885085,885085,885085,885085,885085,885085,885085,885085,885085,885085,885085,885085,885085,885085,885085,885085,885085,885085,885085,885085,885085,885085,885085,885085,885085,885085,885085,885085,885085,885085,885085,885085,885117,885117,885117,885117,885117,885117,885117,885117,885117,885117,885117,885117,885117,885117,885117,885117,885117,885117,885117,885117,885117,885117,885119,885119,885119,885119,885119,885119,885119,885119,885119,885119,885180,885180,885180,885180,885180,885180,885251,885251,885251,885251,885251,885251,885270,885270,885270,885270,885270,885270,885270,885270,885270,885270,885270,885270,885270,885270,885270,885270,885270,885270,885270,885270,885270,885270,885270,885270,885270,885270,885270,885270,885270,885270,885270,885270,885270,885270,885270,885270,885270,885270,885270,885270,885270,885323,885323,885323,885323,885323,885323,885323,885323,885323,885323,885323,885323,885323,885323,885323,885323,885323,885329,885358,885358,885358,885358,885358,885435,885453,885528,885528,885528,885580,885580,885580,885580,885580,885580,885612,885612,885612,885612,885612,885646,885646,885646,885646,885646,885646,885646,885646,885646,885646,885659,885664,885708,885708,885708,885708,885708,885708,885708,885708,885708,885708,885708,885730,885730,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885774,885784,885784,885784,885784,885784,885784,885784,885784,885809,885809,885809,885843,885843,885869,885869,885869,885869,885869,885869,885869,885869,885869,885869,885869,885869,885869,885869,885869,885869,885869,885869,885869,885869,885869,885869,885869,885869,885869,885869,885869,885869,885869,885869,885869,885869,885869,885869,885869,885869,885869,885869,885869,885869,885869,885869,885869,885869,885869,885869,885869,885869,885869,885869,885869,885869,885869,885869,885869,885869,885910,885910,885912,885912,885963,885963,885984,885984,885984,885984,885984,885984,885984,885984,885984,885984,885984,885984,885984,885984,885984,885984,885984,885984,885984,885984,885984,885984,885984,885984,885984,885984,885984,885984,885984,885984,885984,885984,886009,886009,886009,886009,886009,886009,886009,886022,886030,886030,886030,886030,886030,886064,886064,886064,886101,886101,886105,886105,886105,886105,886105,886105,886105,886105,886105,886105,886106,886108,886108,886108,886108,886108,886108,886108,886108,886115,886115,886115,886115,886115,886115,886115,886115,886138,886138,886138,886138,886138,886138,886138,886138,886138,886138,886138,886156,886156,886159,886200,886200,886200,886200,886200,886200,886200,886200,886214,886214,886266,886286,886286,886286,886286,886286,886286,886286,886286,886286,886286,886303,886314,886314,886314,886314,886314,886314,886314,886314,886314,886314,886314,886314,886314,886314,886314,886314,886314,886314,886314,886314,886314,886314,886314,886314,886314,886314,886314,886314,886314,886314,886314,886314,886314,886314,886314,886314,886314,886314,886314,886314,886314,886314,886314,886314,886314,886314,886314,886314,886314,886314,886314,886314,886314,886314,886314,886314,886314,886314,886314,886314,886314,886314,886314,886314,886314,886314,886314,886314,886314,886314,886322,886322,886325,886347,886367,886367,886367,886367,886367,886367,886367,886367,886367,886391,886391,886391,886405,886405,886405,886405,886405,886405,886437,886437,886437,886437,886437,886437,886437,886437,886437,886463,886463,886463,886463,886463,886463,886463,886463,886463,886463,886463,886463,886463,886463,886463,886463,886463,886463,886463,886463,886467,886467,886467,886467,886467,886467,886467,886467,886467,886467,886467,886467,886467,886467,886467,886467,886467,886467,886467,886467,886467,886467,886467,886467,886467,886467,886467,886467,886467,886502,886502,886502,886545,886545,886545,886593,886614,886614,886614,886614,886614,886614,886614,886614,886614,886614,886635,886635,886635,886635,886635,886635,886635,886649,886649,886649,886649,886649,886649,886661,886661,886685,886685,886685,886685,886685,886685,886685,886685,886685,886685,886685,886685,886685,886685,886685,886685,886707,886707,886707,886707,886707,886721,886721,886721,886721,886733,886776,886776,886776,886776,886776,886776,886776,886776,886776,886776,886776,886776,886776,886776,886776,886776,886776,886802,886802,886802,886802,886802,886802,886802,886802,886802,886802,886802,886802,886802,886802,886802,886802,886802,886802,886802,886802,886802,886802,886802,886802,886802,886802,886802,886802,886802,886802,886810,886810,886810,886810,886810,886810,886810,886810,886810,886810,886810,886810,886810,886810,886817,886817,886830,886879,886889,886889,886889,886889,886892,886892,886892,886930,886930,886930,886930,886930,886930,886930,886930,886930,886930,886930,886930,886930,886930,886992,886998,886998,887048,887048,887057,887057,887057,887093,887093,887131,887131,887148,887148,887282,887282,887282,887282,887282,887282,887282,887282,887282,887282,887282,887282,887282,887282,887292,887292,887366,887394,887394,887394,887394,887394,887394,887394,887394,887394,887394,887394,887398,887403,887403,887403,887403,887403,887403,887403,887403,887403,887403,887403,887403,887403,887403,887403,887403,887403,887403,887403,887403,887403,887403,887482,887482,887482,887482,887482,887482,887482,887482,887482,887521,887525,887525,887531,887550,887550,887550,887550,887550,887550,887550,887550,887550,887550,887550,887550,887550,887550,887554,887554,887554,887556,887556,887556,887556,887556,887556,887556,887556,887556,887571,887575,887575,887582,887585,887585,887585,887585,887585,887585,887585,887585,887585,887585,887585,887585,887585,887585,887585,887585,887585,887585,887585,887585,887585,887585,887676,887680,887694,887694,887694,887694,887694,887694,887700,887700,887700,887706,887706,887706,887706,887706,887706,887726,887726,887726,887726,887726,887726,887726,887726,887726,887726,887726,887726,887726,887726,887726,887726,887726,887726,887726,887726,887726,887726,887726,887726,887726,887726,887726,887726,887726,887726,887726,887726,887726,887726,887726,887726,887726,887726,887726,887726,887726,887726,887726,887726,887726,887726,887726,887726,887726,887726,887728,887731,887731,887731,887731,887731,887731,887731,887731,887731,887774,887774,887778,887778,887778,887778,887778,887778,887779,887795,887795,887795,887795,887795,887811,887811,887811,887811,887846,887846,887846,887846,887846,887862,887863,887863,887863,887868,887883,887921,887921,887922,887972,887972,887972,887972,887972,887972,888046,888052,888052,888090,888090,888138,888196,888253,888253,888253,888341,888367,888554,888554,888554,888554,888636,888636,888636,888636,888636,888636,888636,888636,888638,888638,888638,888638,888638,888649,888705,888727,888784,888909,888909,888986,889064,889064,889064,889064,889064,889064
};
unsigned int itemguid[LIST_SIZE]  = {
832463171,833529420,820305871,820503305,820508480,821195879,821196480,821196486,821623663,821623675,822051010,822940354,822941492,822951361,822952693,822952708,822952726,822965370,823733934,823733935,823733936,823733937,823735354,823830181,823830279,823831506,823831508,823866076,823871016,823883169,824403276,824410259,824441360,825165772,825952593,825953101,825953189,826025189,826025338,826447224,827256491,827272639,828560619,829574614,829579178,829800216,829884381,830391366,830403949,830443598,830451913,830687349,830694654,830912312,830950032,831459819,831463663,831463746,831465524,832324071,832843606,832898780,833451685,833458659,833458763,833459078,833537002,833555174,833566966,833793244,834044031,834044628,834283013,834778859,834810043,834845067,835189706,835197089,835280525,835905717,836050634,836221967,836271563,836416551,836417855,836505725,821947225,822286939,822296679,822307457,822313980,822324546,822333790,824396316,824399780,824417731,824966609,825920533,825935191,829298379,829789433,829792315,829803849,829809298,836428631,836434128,836664986,836672092,822531004,822946977,823829725,824415040,824859884,824859885,824859886,824868997,825918012,827249401,827885468,829574271,829915464,829953046,830178220,830710575,830733450,830952440,831459690,831668580,831687212,831688936,831902940,832779034,833261742,833448452,835192719,835291810,822580558,822581268,822581331,824200547,824337567,824397523,824809721,824809736,824862961,824862995,824863043,824863058,824863190,824863382,824863394,831722199,833893493,833968833,821197000,822269372,822846704,823053724,824420510,824427722,824437420,825165847,827635513,828878718,828910558,829410576,829450132,832650779,835955794,835959242,828456976,828456997,828457041,825752809,820242251,821755839,821755860,821755882,821755896,824307200,824307222,824307353,820110196,820123993,820132892,820175272,820403757,820857453,826492751,827612856,827612872,828159668,830368376,831807805,832193718,832328376,832328393,832328396,833421988,834116238,834946546,820231891,826217752,826405820,827579465,827781522,827781551,828828588,828828886,828828910,828828919,828935953,829319418,829427746,820108557,828637881,832275466,832275487,832275503,824400277,824420290,824435419,824437939,824438426,824447165,824450267,824464094,824681622,824700507,824712119,824720481,824958882,827646603,827957156,828038165,828044607,828054148,828073096,828107023,828115957,828361876,828369644,828906909,828912497,828942704,828948529,829371564,829375327,829719499,830351057,830708229,830867552,830876732,830884370,827660605,834467332,834477997,835359563,835374510,824470411,825379221,828496385,830338972,831513569,831520709,831523131,831714031,831714149,832092603,832092728,832092849,832092974,832093086,832093189,832093277,824119051,824119074,824119880,824119902,824119933,824120002,824854859,824854950,825202648,825276987,825277000,825277015,825277035,825277058,825277070,824861432,833301816,833309542,833405207,833407225,833411794,833606906,833606931,833698025,833715682,833999224,822893903,826351034,832718992,832732103,830617032,832342140,824862074,824863221,824863948,824866973,824893513,828641851,827614537,834766535,820774252,820774263,820774272,821175104,821329319,821329334,821329341,821329609,821329620,821843865,821844124,827390496,827390512,827460007,828024348,828304576,829631734,833920073,824830771,827490913,828194595,828194616,828194646,829107962,831012823,832178980,832784414,833595700,833888328,833888355,833888381,835301294,822739291,823210340,829228093,829414654,829414657,829415175,829508977,829564509,829763197,829763810,829823714,831578217,831584195,831589300,835220849,835682472,835682663,835690747,835691070,835154575,820057397,820057410,820058944,820058954,820652377,821487348,821489281,821489756,821489909,821515286,821529825,821828201,822807162,823313233,823316983,823317184,823319290,826052322,826358040,827002066,827002101,827031626,827034833,827039841,827627645,827842998,827843000,827843013,827843018,828128456,828131490,828198090,828389851,828389905,828390011,828500304,828500311,828501647,828532725,828533103,828745267,828745283,828745310,828745384,828745399,828745420,828922571,828922588,829122916,829122919,829122922,829122932,829122940,829122953,829508744,829510251,829511497,829511498,831612796,831617685,831730322,833102212,833145304,833258372,835022451,834153856,822474428,822480458,826330605,826683740,826686371,827791431,828547003,828547005,828549164,828553421,828558745,828561529,828828877,829323834,829431906,829432535,829433464,829436663,829439173,829514995,829514996,829699006,829713095,829780656,830929909,832138725,833516247,833609374,834458020,834736444,834790053,836284107,836412958,834779764,820423354,821102403,821364261,821409313,821423850,821427753,821635042,821891367,822230930,835427962,822140477,822719095,829512129,829768849,829878625,829909306,829910707,829917982,829920402,829920406,829947643,829947978,830429137,830429164,830429178,830740935,831046567,831047298,831625823,832335287,835282528,835282578,835282798,835282815,835282818,835282836,835282860,835282923,835282961,835283068,835283070,835283276,835283324,835283344,835283391,835283409,834950535,821992403,829135526,829135542,829135572,829135581,829135593,829135617,829135649,829490462,820140197,820221329,820152046,822677231,822694314,822694489,822694505,822694517,822694530,822694651,822694684,822694707,823603472,823965333,823967294,824007709,824190272,824396317,824481848,824962128,826003493,828030024,829133961,829702974,829780082,829797748,829802745,829812610,835423577,835659755,835659763,835659769,835659780,835659787,829151020,829314053,829502608,829508346,829823915,830021598,832647220,833781858,823425222,824859726,828079294,829018884,829918849,830054446,830558364,830638400,831150819,831449202,831449204,832121636,832121701,833164529,833263943,833443831,833443851,833451083,833679940,833694443,833694454,834041154,834041168,834267598,834267628,835191315,835266607,835270737,835296468,836039309,836051298,836408959,826532741,826675095,826675440,826675470,827205549,827385156,827385168,827385254,827385302,827922505,827936460,827938092,827978555,828772292,829680340,829680363,829680383,829680401,829680415,829680437,829680457,829680480,829680499,829680528,829681142,829681220,829681256,830983037,831241560,831241575,831241598,831241620,831405752,831690880,831691940,831691951,831691970,831692354,821350616,821632196,821632199,821985021,822359041,822526738,824089476,825721419,825760353,825775993,825784001,825800538,825861349,825865457,825913147,825914663,825914704,825924599,825948600,826155798,826157048,826162335,826169422,826181341,826209840,826665740,827243271,830663356,833538384,833539719,821327832,821104602,830737120,830737337,832341719,825909638,829514846,829515433,830418489,830418550,830998381,831030286,831139282,833939668,833939781,822657909,820532577,821008020,821662099,822090812,823095329,824107473,825276999,825345982,826212258,826227022,826741040,828892021,828910836,828940829,831050955,832633138,832680980,832681354,832712693,824206896,824244297,824293293,827882889,820155219,829518371,829684650,829687778,829689251,830801078,831622005,831659770,831659798,832162059,832658622,832663062,833208098,833674344,833755139,833755416,833761539,833762581,833763142,834254480,834254683,834256989,834258523,834258882,834260214,834261296,834283396,835071559,835087902,820291452,836299912,822343617,822645462,832183368,832183760,832185331,832185962,826125364,826125368,826125370,828535814,828535830,830604667,831413845,831421997,831530845,828840298,829299534,829490852,829751118,831234235,831235071,831235091,832760950,832760984,832334349,823863465,820320173,820325009,821902708,822817304,823003902,823012471,825203128,826448274,826448298,826620168,826620188,826620196,826620207,826620215,826620224,826620353,826620410,826620439,829485976,830195591,823526232,824713952,824938473,829540116,829551209,829998740,833799611,833802847,834095884,829184573,829192047,829210348,829213975,829603225,834773552,836218551,836426807,829826301,829861288,829861293,829861319,822999618,823023642,820971716,822734575,836143873,820973114,820294380,820294415,820294418,820294421,820294428,820294437,823790664,830971797,823887660,828701181,828967813,828967814,828967815,828967816,828967817,828967818,828987389,828987390,828987391,828987392,828987393,828988123,828988124,828988125,828988126,829001698,829001699,829001700,829030005,829030006,829030007,829030009,829030010,829031134,829031135,829031136,829031137,829040973,829040974,829040975,829040976,829040977,829040978,829042829,829042830,829042831,829042833,829042834,829044640,829044642,829044646,829044647,829048259,829048260,829048261,829048262,829048263,829062133,829062134,829062136,829091687,830476400,830969017,831240719,835574220,820298526,822189196,822200177,822211540,822218221,822223770,822235894,822245716,823475821,823481299,823484926,823489497,824221189,824231396,824391577,824664235,824670836,824692096,827472168,828932456,828942706,822516906,822516976,828864029,828867981,832107173,833564992,833565161,833863017,834584310,835184544,835832783,835875991,836089262,820784062,820784120,825541639,829376762,832781345,835122932,835122955,835122966,835123061,835123071,835123109,835123120,835713744,822181553,822333314,822333325,827290368,835259890,827645828,830867259,830867275,831241526,831827762,831829788,831829806,821673329,821673530,822814783,825224040,822053387,822053445,822053552,822055426,823849824,823856151,824910630,824966355,825503065,830475110,830481183,831131337,834711022,835420452,832672014,832672030,832672275,820144095,828642822,829920053,831498353,832239319,833878455,834555190,835195076,835633452,822142690,822950851,829538068,823970731,821326163,821623417,821623418,821623420,821712951,821712976,821713000,822050567,822260397,822260418,822260459,822260526,822260550,822260559,822260580,826730027,826730195,829513638,830849866,833231515,833574052,834253773,834535676,834570973,834576377,834888136,834888146,834888190,835153331,835157776,835159771,835306428,835321455,835571207,835678269,836084140,836084153,820063175,820258591,820481955,820488636,820490654,820498795,820509960,820512499,820518355,820591158,820901828,820928710,820936359,821422705,821425958,821545212,821803494,821812520,821824913,821877006,821972955,821979987,821990165,822200475,822203063,822211541,822218222,822223771,822224317,822274576,822546785,822549189,822743542,822743550,822743679,823131491,823420256,823433920,823704685,823997014,823997025,823997441,823997479,824216718,824227199,824230312,824234284,824315208,824319536,824339736,824356119,824397988,824398109,824398131,824747355,826784045,826784060,826784085,826910568,827164158,827166073,827175021,827176476,827180268,827219987,827241916,827244107,827251472,827262323,827264212,827565873,827732230,827734510,827972798,827983802,828005312,828029588,829102179,829102674,829110735,829469141,829474616,829476575,829481705,829483933,829486933,829878824,829957231,829966990,832782046,832788765,832803435,832807368,832822168,832825049,832928135,832932859,832936250,832940529,824231398,824409106,824410688,824712120,824919242,825161719,825165881,825176943,825181996,825226536,826692032,828033743,828037953,828043687,829922581,829924883,829932179,830483104,830995638,833036456,833036493,833036526,833042419,833042708,833042789,833627758,833627970,821424082,821859083,830809620,834063418,834212640,835415575,836063653,836063713,836064717,836065569,836066080,836079774,836081189,836081689,836082801,836083027,836084316,836084409,836085281,827983803,828005313,828086902,828089993,828167631,828170647,828183985,828190633,828194864,828206913,828345635,828360576,828372401,828374217,828376198,828384716,828396934,828403628,828416225,828435446,828473236,828485095,828501194,828503629,828520356,828540125,828553473,828652654,828671542,828686275,828739041,828749046,821278826,821278869,821281333,821292001,821294454,821330623,821330624,821926977,824007652,824007667,825599990,825600004,825601130,826939800,826944599,826947310,828713355,828729911,834530485,834823827,834823891,834823996,834853937,835121946,835122944,835123037,835123086,835123138,835123151,835123211,835123241,835123824,829248746,830868792,822275655,827892141,830311610,830422061,830986942,832220903,832999068,833353232,833353327,834063008,834063182,834093821,834172443,834172475,834172505,834172522,834172538,834172556,834172578,834172598,834172621,834172645,834172664,834172696,834172714,834172742,834172759,834172775,834172807,834172832,834172855,834173674,834173691,834173717,834173764,834173797,834173820,834173841,834173858,834173872,834173885,834173909,834173932,834173955,834173969,834173993,834174036,834174055,834174215,834174242,834174287,834174318,834174354,834174435,834174558,834175546,834175575,834175600,834175626,834175645,834175671,834175679,834175694,834175711,834175730,834175744,834175762,834175773,834175798,834175822,834175844,834175856,834175873,834290812,834305278,834305336,834305685,835424987,821283252,821283276,821327826,821327855,821327880,821766630,821766639,821766650,822000128,822000149,822000161,822000178,822041513,822465973,822516445,822516459,822516481,822516496,822516506,822516519,822516530,822516554,822601764,822662668,822662745,822662769,822662781,822662786,822662800,823766946,824237240,824460780,824460804,824460854,824460924,824460978,824461002,824461045,824461080,824461107,824461138,824461162,824461259,824461282,824461298,824461316,824461367,824461398,825396703,826130058,826675101,826826748,826826774,826827136,826827139,826827143,826827251,822045021,827838772,828261157,829165765,831059012,831201937,832126235,833457036,826604672,826608598,826999471,827029059,821288486,821288841,821288850,821288867,821288877,821288885,822432396,828982508,829611827,828123728,828842114,829126320,829253630,829254164,825223661,826260517,826268147,826574561,827295797,828037239,828037317,828037349,828864579,828864615,828864677,828864703,828864765,828864783,828876188,828898322,829757888,829758160,829902415,829903301,829911664,829934748,829937584,829953852,829989531,829989547,830626364,831015406,831986513,831986528,832295058,832343691,832394041,834087601,834988836,835381307,835389327,835389502,835391534,835391934,835394149,836081380,836576244,821066124,822487628,825249955,825255122,825255138,834569104,834603626,823465046,824395505,825253467,825984561,827892780,830786514,831527809,831527869,833227322,833227337,833364776,833482469,820636601,824044353,824044756,824045096,824601281,825599318,825599813,826311003,826628323,829587013,829896820,829896823,830380561,830899901,830899902,832315046,832315109,832952103,833455390,834017235,835419832,835447730,835454057,835454394,824968640,821637640,822683197,823290229,824126261,824388803,824683594,824683648,824923158,825158338,825171860,825222542,825222790,825222854,825243070,825656332,825656386,825665658,825665781,825683975,825684055,825861320,825887184,825939011,825977719,826071450,826148708,826168334,826480410,826480512,826690340,826690414,826735143,826735222,826735285,826739617,827241736,827341412,827784861,827784899,827786789,827789985,827804408,827804441,827804569,827804727,828339035,828339097,828339547,828819254,828819285,828930885,829495983,829642064,829802395,829923023,829935502,829935583,829945731,829949157,829953709,833470180,833758551,834781087,834839029,836271216,820030180,820103971,820458908,820740290,820745583,821184379,821592274,821613086,821777902,821832246,821861273,822009464,822018060,822018212,822018798,822018806,822208416,822462444,822477678,822478553,822562927,822947048,822947056,822947062,823196229,823423229,823428201,823649895,823995941,823999773,824051696,824051697,824051699,824051700,824051701,824238937,824579840,825096354,825137744,825257965,825325627,825624621,825624631,826302287,826330609,826362348,826608750,826641282,826987192,826987208,827478511,827481668,827531565,827749198,827763220,827768804,828554686,828616723,828690683,829388633,829401900,829401903,829401911,829401920,829401922,829401923,829403656,829403658,829403665,829403666,829586170,829760184,829896242,829896291,829897445,830363013,830379869,830380169,830874161,830874170,830874189,832872004,833066702,833069580,833462157,834016096,834026646,834026933,834447880,834806840,834807224,834808106,834865481,834867335,834867362,834867389,834867412,834867442,834867465,834867491,834867510,834867526,835102522,835102585,835344007,835348943,835349036,835612573,836010125,836010154,836010178,836010331,836010875,836013784,820805156,820805598,824889008,825423300,826181572,823797956,827278881,827282832,822985657,831026771,831281082,832509687,832548193,832548509,835812086,835859614,835878946,821933671,822148080,822413559,823146595,823676182,823692635,823711776,823717179,823740377,824018201,824024703,826253504,826266086,826288830,826293169,826305349,826937812,826956175,827458609,827079144,824786911,825597839,826595319,827165583,827165614,827481626,822896097,822896736,827388145,827418388,827514312,827514335,827514341,827514367,827514417,827514434,827514448,827514528,827515025,827515075,827515088,827515106,831118062,833768254,829996577,821425959,822734779,822752772,822768298,823787273,823826207,825088624,825090266,825944476,825963370,827711960,828220941,828228568,828376647,828404560,828553609,828558965,828571004,828580200,828599091,830115234,830118349,830128547,830129510,830728003,830827162,830832563,830861402,831372510,831381279,831555688,831603401,831611057,831807963,835936887,836001349,827415877,829891066,832846666,834891418,824792997,825473111,826459470,826459490,826532567,826583805,826882007,827158016,827259836,827259868,827342746,827358074,827425648,827452921,827476867,827489622,827494898,827514791,827516303,827517570,827542852,827570940,827637592,827780473,828285581,828285584,828285586,828545247,828983024,829752910,830348970,830410216,830543416,830543459,830694581,830694615,830694641,830757021,830929085,831702339,831786817,835973718,821803413,821812521,821882225,821893249,821901648,821967182,821970604,822565018,822566829,822569746,822577072,822579516,822583668,822586857,822596749,822607982,822619842,823259356,823607816,823736783,823738846,823750995,823834114,825068283,827713789,830006300,830698645,830704100,830985140,831224279,831245939,831347813,831366968,831367002,831367024,831367038,831367052,831367070,831367088,831526126,831526587,831526611,831526620,831526630,831526641,831526655,831526710,831526729,831526742,831526752,831526781,831526790,831526805,831526837,831600289,831600302,831600388,831600604,831601856,831602033,831611393,831697827,831697845,831896720,831896728,823836235,822043911,824337582,825385442,825385609,834004387,834005353,834007081,820739369,821156326,822985344,824072149,824095811,824273282,824420309,824895158,825218477,825535937,827352456,829515696,821545909,821545927,821545940,821546048,821546192,821546203,823805592,825128244,825130799,826472923,826719264,826719279,826719325,826719362,820873110,821380086,822172662,822517622,822523451,822527746,822534079,825807553,832688150,834130639,835344413,820395466,827057005,828948560,828953731,828953844,830251524,831012479,832466482,832473554,820160046,820557740,823180204,823182689,823189757,823204312,824929103,824932095,824952977,824962130,824964327,824976276,825392917,825618901,825622111,825949632,825986044,826000657,826031005,826042857,826097222,826274793,826276346,826278856,826285720,826292008,826301020,826308558,826317714,826328096,827303140,827323661,827332605,827351248,829983508,831823235,831824126,832313730,832313731,832898667,835257024,835263138,835279148,835682855,835692847,835696596,835697560,835698283,820073913,820077305,820110936,820115167,823349656,829371565,829375329,829379992,829380779,829392409,830008540,830010811,830033016,828417933,828418304,828759669,832201424,832243027,833096708,833100571,833110877,833116015,833173854,833174728,830308073,830414679,836323202,821292707,825521173,828228570,830291831,830294036,830336214,830351060,830845758,834535034,834538268,834552439,829847762,830436247,831372325,831589311,831691911,831691995,831909994,831918859,831919003,832070758,832070790,832070820,834405603,834563501,834563714,834563724,834563728,834563748,834702781,830576731,830583623,832817685,833660587,833667945,827274852,827644247,828718378,828779298,828782536,828782904,828784340,828784468,828787187,828797987,828797993,828797995,830390211,830391664,830391670,830391698,830942874,830998284,831083487,831642650,831642884,832337330,832695217,833389512,833479927,836010514,827254642,827254656,827429081,827429114,827776693,828815795,828815802,829481109,829482565,823618112,823618156,824084345,835385195,835402615,835403679,832518667,830455286,830459285,831886094,832343037,833316245,834165927,834170625,820358297,820429750,820841625,820847451,825467476,835795837,835829193,835899832,835908539,835923563,835931266,835950194,835958390,836281547,820032376,820065536,820074116,820258091,820259298,820386320,820868016,820871854,821253778,821373864,821638809,821649017,821681441,822021615,822022169,824934326,824961480,825769625,825784603,825797625,826236831,826543424,830517702,821934467,821939409,821940092,821944449,821965759,821973551,823244628,823698240,825983510,826354656,826835617,820305696,820306099,821937039,822295517,823731192,823734571,820032814,820033049,829617486,831735146,832367427,832367457,833513117,833513138,833513147,833513165,833513175,833513188,833513211,833513228,833513257,834700859,834701243,834701276,834701479,821965930,824311134,824311164,824320316,824320329,824326501,824340773,824341247,824341259,824348824,824348849,824355757,824355771,824384218,824384229,824390771,824391145,824391167,824396461,824396746,824403720,824403744,824411123,824411140,824411492,824421285,824422153,824437720,824438007,828339691,828339698,830695048,833768267,833770097,833771561,833775052,833775977,833776346,820684948,821070317,821942503,822089573,823082734,823314308,826955458,827316880,831129152,833554842,832251371,832379746,833271621,836074816,836081144,831808375,833272903,833272951,833272995,833273008,833273034,833274406,833274426,833274456,825114744,825114748,825114756,825114761,825115333,825115339,825115357,825115375,825182557,825184179,825184202,825184218,825184246,825184259,825184281,825184303,825184309,825184317,826159418,826159429,826159445,833063146,833382845,834220423,835421381,834011611,824100374,824687336,824770815,828452328,828452329,828452330,829485879,820771373,822143287,822145756,822684939,822686276,822686284,822686300,823715668,823718385,823719178,823721369,823721793,827238248,827238475,827251322,827251440,827252109,834779652,834780510,834780531,834780550,834780569,820174191,828671971,829206837,829206861,829212363,830251159,825041308,833447544,820401917,820583215,820583256,820739241,820739247,820739378,820739386,820739394,820739404,820739420,820739487,820739494,821088538,821595511,821595528,821595698,821595721,821664232,821664252,821707514,821759425,821795287,821829154,821829181,821829196,821829241,821829264,821829286,821829321,821833196,821833358,821994723,821994732,821994746,822085746,822281530,822349944,822385259,822508286,822508324,822513410,822708826,822715420,822715457,822715506,822848838,822853712,823393449,823832702,823832737,823832838,824145069,824619048,824806511,824881715,824882256,825217869,825398168,825443442,825446282,825446309,825446317,825446341,825446347,825446369,825446386,825965363,825975580,826132051,826241080,826246503,826246537,826246547,826246561,826246593,826246608,826246628,826396298,826711737,826855381,826855441,826959852,826965417,826994626,826996720,826997537,827022884,827025424,827120951,827172481,827201660,827204844,827204846,827204888,827204893,827204902,827204909,827205538,827395977,827398535,827406896,827451432,827486762,827974546,827983452,827988872,828001836,828014719,828020324,828099624,828146469,828146479,828150020,828242012,828242030,828242048,828242069,828242098,828242110,828494097,828504457,828514357,828524754,828526383,828526402,828594405,828609802,828609831,828609846,828731958,828782275,828782292,828782313,828828242,828828248,828828257,828828584,828828597,828828599,828828751,828828756,828828761,828828775,828828784,828828791,828839704,828839907,828839916,828839929,828840066,828840082,828870952,828909802,828942167,828945790,828945804,828945831,828945851,828945868,828945886,828945921,828946819,828946834,828946856,828946889,828946909,828946926,828946947,828947219,828947243,828953017,828959737,828995109,829054621,829059083,829098888,829148376,829223066,829223091,829223110,829223276,829223319,829223445,829223461,829301360,829301373,829301585,830109243,830109336,830109392,830167575,830282249,830282266,830282277,830282289,830282302,830282324,830282701,830282708,830335758,830335841,830580943,831093009,831371278,831371384,831371399,831384445,831385703,831601586,831601601,831601619,831601629,831601646,831601657,831601672,831601694,831601706,831602086,831604886,831608403,831915528,832252946,832254272,832287152,832503395,832507252,832560703,832570514,832624756,832719281,832751556,832753457,832837013,832846223,832846245,832947113,832968185,833049477,833049479,833220714,833228697,833848666,833921153,833974525,833987653,833992187,834000835,834000842,834000886,834000915,834000972,834000977,834000986,834000999,834001118,834001138,834001145,834001715,834328336,834472394,834862738,834960030,835257062,835257071,835257085,835257101,835257114,835257127,835257364,835257387,835257399,835257420,835782278,835863874,835863895,832339675,830428022,830477889,830477973,830478010,830478047,830505357,830748610,832837225,834785812,835150494,835173772,820295003,820295011,820295033,820295069,820295167,820295175,820295182,820295184,820295232,820295234,820295254,820295265,820295274,820295287,820295301,820298528,820306100,821098848,821397149,821397271,821397390,821397498,821397528,821397589,821397620,821397676,821397730,821397772,821397819,821397850,821397868,821397900,821397960,821693281,822122901,822305224,823106357,823106375,823106406,823106433,823106460,823106492,823106536,823106551,823106883,823106904,823106937,823106951,823106970,823106993,823107018,823206338,823229757,823668941,823668956,823668986,823669036,823669060,823669100,823669141,823669164,823669200,823669241,823669262,823669297,823669333,823669357,823858848,823879979,823883592,824338590,824897694,825520619,825541991,825542006,825542031,825542067,825542085,825542097,825542131,825542158,825542187,825543999,825908052,825913304,825921439,825928007,825934036,825937850,826343790,826347424,826347443,826347464,826347482,826347514,826347535,826347585,827234142,827234161,827234170,827587191,828884483,829247371,829594941,830043084,830412760,829224302,829225292,829231239,832944399,832944401,836081636,826764972,826765099,828039184,828607620,829908545,829927527,831653992,831847269,832101992,832174190,832414195,832802149,833815452,833815473,833815857,833815892,834002179,834002194,834010870,834010873,834011065,834011067,834011072,834049365,834049381,834376134,834476372,834476388,834477778,834477933,834480649,834483320,834483449,834484229,834484245,834484287,834484303,834484328,828233554,829690008,829691538,829693168,832722035,833369273,833372440,833375338,833381440,834210170,834879795,835740293,823151798,823152704,827230834,829116775,829116835,829116891,836288711,824893635,827339250,827382224,828044723,828047666,828161954,828162002,828162083,828162111,828162135,828162152,828162169,828162183,828162224,828162734,828162843,828162861,828162878,828162898,828162921,828164183,828164220,828984974,828984995,833352445,833352462,833352482,833481206,834154065,835524934,835937966,836067843,836076626,836078178,836078851,836079740,836081007,836081856,836082923,836083852,836085387,836088737,836088880,836089542,827386705,828742296,829046776,829423673,829433877,829436760,829436768,829436779,829437603,829437645,829437850,829437859,829437870,829437887,829437897,829437904,829437913,829437916,829437925,829437932,829437942,833784080,821537726,823038306,823050908,824014243,825040889,825088764,829109998,831987944,821351278,821714827,821819179,822078655,822243758,822261343,822561795,822746956,822955292,822964099,823651012,824368523,824639881,825203129,825289898,825665498,825681725,825684972,827006055,827021357,827043842,827128034,827323662,826884840,826885971,826886467,826886990,826949630,827109448,830562966,832177448,832192981,833907425,834062922,834504739,834864772,834879519,834887376,834888582,834889304,835303173,835303494,835304362,835306524,835309984,835511209,835535160,835535407,835535879,835536129,835539005,835539214,835539442,835539634,835541704,835541908,835542085,835542315,835542556,835555442,835556493,835557281,835576614,835576798,835576936,835577024,835577145,835669912,836322743,831854738,831863840,831969882,831972580,831977448,831996597,832011175,832013398,832946213,832949204,832959169,833067238,833127667,833132065,833135161,833334595,833563758,833602443,833621190,833781181,830958219,831366286,831600848,831600882,831748316,831763344,831763391,832440736,832442340,820296436,828091245,828677312,828688805,828689546,828692306,828692877,828693664,828880952,828889518,829247584,823050762,827014600,829159589,835925373,836327743,823328044,823347737,823400605,823893998,824170213,824187966,824204108,824501007,825618903,826048732,826069386,826319768,826322729,826337248,826340319,826347721,826361168,826537001,826563409,826567299,826573158,826575684,826585936,826956864,827054508,827144538,827167420,827175022,827620700,830079803,830086584,830121656,834473851,834484964,834534985,834944070,835015860,835021596,829301088,823057106,829840821,829840835,829967944,829967951,829968019,830747434,833583750,820491293,820262068,820262135,820262158,821310690,821338103,821338123,821680358,821823179,823719466,826428739,826428757,821937881,823485559,823485567,825064581,825246702,826855454,827491289,827701944,827820402,822478281,824343629,827007482,827036340,821833291,822508304,827234303,830214477,830532667,830543970,830558994,830574595,830595265,830615065,831173898,831190589,831194110,831281039,831310328,831656634,832045017,832046036,832059529,833279390,833281675,833288911,833777313,833785719,835075954,835466670,835768706,835768726,835768736,835768749,835792775,836083420,836247435,836247439,836258275,836358997,836383667,832156819,832159845,821533845,821535793,824659051,824814440,824819401,828129604,831748009,831756455,834058333,826702978,826751959,826753535,826753560,828251978,832141714,832338820,832341765,832344624,820399689,820640780,821727412,821727443,821727478,823025654,823489494,823489546,823489581,823489599,823489667,823489678,823489684,823489704,823489721,823489734,823489738,823489749,823489761,823489767,823489780,823489799,823489808,823600986,824374706,826879401,829166263,829512599,829515783,825922029,827205897,827425351,832815938,832821824,821503704,821568730,821607346,821650035,821870621,821870772,821968298,822527775,822529568,822531154,822531722,822535949,836539005,823717408,826549606,826937206,827749665,827750370,827751319,827751560,827756751,833956493,833956583,833956889,833956973,833959884,833962415,833972723,833972754,833972773,834401384,834401452,835567274,835572573,825081915,825082014,823067101,835912020,835917365,835933060,835933893,835943382,835943426,835943464,835943521,836240235,836245554,836245722,836595139,836595633,836595833,836597019,836597078,836597975,832270845,832271398,832338413,820317618,820326926,820333265,820430608,820441678,820442129,820442155,820807053,820807073,820807084,820807108,820807121,820807138,820807153,820807175,820807479,820807496,820807510,820807525,820808395,820808415,820808426,820808439,820808467,821290288,821325687,821337819,821337873,821337918,821337930,821337946,821391619,821391641,821391676,821391701,821391736,821391753,821444634,821545476,821545491,821545506,821547090,821547107,821547132,821594134,821611042,821615598,821624775,821632202,821632245,821632258,821632265,821632279,821632297,821632331,821660638,821660734,821660741,821660745,821660746,821660748,821660757,821660767,821660772,821660892,821660933,821660938,821660949,821660958,821660964,821660973,821660986,821660997,821661105,821661114,821786308,821786402,821786421,821786864,821786871,821999257,822079220,822144155,822293733,822293758,822293866,822293962,822293987,822294009,822294093,822294107,822294119,822294137,822294249,822294282,822294320,822294379,822294474,822294492,822294504,822294517,822294620,822294627,822294636,822569321,822782894,822782921,822784575,822784719,822784739,822784748,822784848,822784864,822784883,822784971,822785000,822785025,829635889,831440410,831441205,822877620,822886584,822886619,825727595,825727616,825727687,827405906,827419001,827419031,827419258,827419281,827481606,827482320,827482624,827482689,827593894,828032676,828032787,828032845,828042616,828042639,828071263,828084612,828121622,828382652,828382678,828382693,828552845,828552877,828552902,828594880,828594902,828594919,828594940,828594967,829156954,829202697,829202715,829202729,829202745,829202769,829493028,829493036,829493048,829493060,829493067,829493082,829495973,829678728,829678961,829678974,829709179,829709201,829731292,830151908,830151935,830462397,831203494,831203546,831203590,832333472,832333473,832333480,832334188,832334201,832334215,832334228,832713658,833033596,833033648,833035896,833035921,833035945,833086584,833107995,833108016,833108054,833325115,833333662,833566433,833566718,833846816,834039006,834039007,834039010,834039054,834039059,834630583,834630598,834630622,834630630,834630641,834630920,834796981,834801169,834801263,834801296,834801334,834965508,835001454,835001465,835001475,835001486,835254228,835254245,835254276,835254809,835254816,835494468,835494768,835834421,835834432,835835190,835835222,835836386,835836509,835838332,835838467,835838578,835878187,835889650,832952458,834345626,834353441,834353661,822215475,835558520,835651133,836184327,836189123,824137134,827390919,827663328,827663364,827663381,827663395,827663413,827663428,827663460,827779931,827779943,821035690,824355452,824368524,827391871,827394609,827407030,827407738,827408962,830156170,831226167,820098187,820099188,820101763,820115597,820324034,822305578,829741899,831280143,831280178,833151847,829252160,834429294,827076189,832079985,832359589,832416330,832544894,823713164,834819909,834842319,834445294,834467581,834477221,834477835,834477848,834477858,834477881,826419162,826715256,826743328,826756792,826762305,826769412,826783118,826830944,820755343,822480250,824050453,824054606,824606114,820333713,820338915,827900473,827906063,833821111,834134895,834157415,824615389,821492747,828946893,831007096,831431409,820247731,821609690,828793561,830081889,830081904,821072859,821072916,823104612,823105920,823106854,823819824,828874329,834061897,834082992,834209137,823736493,823743678,823759972,823764035,824153926,824165710,824165813,824172498,824233055,824233480,829071824,829115254,834255920,834256203,820701885,830292889,832857666,833012178,836118505,834679698,833527568,833528517,833554718,833604788,833687097,833715180,833788937,833855047,834051877,829611437,831326278,827692206,827234643,827498143,827501276,827504410,827504491,827508353,827510305,827513930,827516812,827533256,827535466,827537118,827538219,827539317,827596445,827598315,827599763,827610239,828084777,828085847,828088884,828090206,828090931,828093522,828093694,828098814,828100529,828103802,820745162,820746875,834719709,823985397,825105347,825112622,825246585,825778393,825958213,826124533,826590990,826621412,833160707,836644264,820277533,820857963,825291043,825387073,828566130,830257314,831815445,835461101,835468038,835473629,835512228,835512270,835512318,835512377,835512447,835512499,835513399,835513424,835513577,835513604,835513869,835513907,835513951,835513974,835513988,835514001,835514020,835514038,835514059,835514076,835514090,835514098,835514127,836076807,836076823,836076847,828077631,829653962,827828330,827828688,828617617,828952643,835123198,832207378,832207417,832207446,832309990,825988501,826003496,832839452,832844075,833438247,833439250,833441299,833446642,833448088,833598809,834552442,834585334,820244654,823662138,824808894,825709652,825865912,826340321,827169075,820897081,820088130,820372456,820386828,820466328,820481958,822296009,823102359,823107895,824020976,827795716,827795733,827999541,829285546,831046167,831046185,831048132,831052443,831058737,831064587,831072793,820797943,821546755,822122902,822355303,823489919,824190579,824983222,825184825,825188172,825196385,825201376,825208240,825214215,825282735,825289197,825505426,825530799,825681726,825896986,829607870,822052036,826559740,827009836,827010169,827569883,827680509,832214366,822123945,823015044,824164502,828520677,828824622,829728775,830133504,832380647,832570726,832602826,832655583,820310039,822596429,821722125,827038161,827045416,828586141,831913128,831916039,831920144,831040521,830107179,834535722,834535733,834535750,834535774,834535915,834535948,831093448,822080262,824567229,826072504,835551254,831366846,831366953,820948583,823245400,823479419,824681462,824692618,827777676,828935981,829420407,832964709,820591494,820591534,820706979,820969021,821619925,822286240,822501214,822501220,822990868,823398391,823853767,824910251,824923554,824940481,824940512,826003244,827253859,827253899,828478028,828478083,829493604,829918473,829918500,831848093,831853816,832214553,832239523,833180237,834178309,834807675,835654495,836078628,828385773,828637758,831487764,835199873,835778389,836597782,832339009,820936416,830652271,832863808,833197847,833568882,835118556,835118603,835125978,835126009,835126026,822566991,824335882,829602017,831688697,832838108,833404645,833411798,833441300,830355119,830368643,831028762,831030684,831175926,831178070,831182649,831183310,832978167,821348737,821390895,821730131,821796707,821832039,821832058,821845322,821845811,821845845,821845875,821845902,821845953,821845996,821846032,821846107,824161389,827508501,836513604,825825135,827508204,827612442,829249942,829595021,830078845,830540353,831403720,831403723,828642173,831019363,831025997,831030016,828990106,829806483,831011404,831398764,831841083,831765885,827684376,827684478,827684507,827684564,827684968,827685000,827685054,827685123,827685216,830532680,832696913,833310430,833310517,833310554,833310609,835855745,835855766,835855808,820594423,821037775,821548177,822333772,825385198,828680894,830483746,831031096,831031915,833921159,834622157,834701140,820032735,833249466,833249481,833249512,826115385,826440446,826440467,826440576,826440594,826440619,826440638,826440655,826440670,826440686,826440691,826440731,826440757,826440838,826589201,826589297,826589351,832468832,832468862,834423818,836332683,828123837,834416491,834417363,834417989,825957550,825972918,826414651,826425018,828435447,828454401,828852267,829102180,829292103,834170054,834179994,834184377,834964953,834975295,835021597,835040878,835063306,835495212,835511956,835517571,836076418,836106618,836106637,836107147,836107461,836107484,836107932,836107943,836109197,836109397,836109433,836109605,836115336,836115352,836115530,836115689,836115841,836115906,828644496,831395715,831816429,831816437,831816449,831816663,831816678,831816685,833796610,835186368,835186423,835192234,835565568,835871037,835872353,835873065,835879807,835880421,835885536,835885570,835885793,835886062,824790644,825287246,827114125,828567616,828704652,829841313,829841391,829841424,829841440,829841453,830091799,830091821,830091842,830121364,830121408,830121436,830121462,830121498,830121727,830121782,830121835,830121984,830122022,830122044,830122139,830122193,830161061,830161137,830161160,830161176,830161197,830161223,830161240,830161305,831022712,831457179,831457188,828113421,828403598,828403608,831783939,833527637,833921815,833921820,833921826,833957581,833986929,833986945,833986952,833986972,833986986,833987015,834041065,834151095,834151135,834151148,834151169,834151191,834177749,834177770,834177801,834177820,834177840,834178163,834178190,829403933,829814946,829814958,829815003,826284002,827491246,827491328,827491365,827866888,829492649,835114603,827571979,826972626,826278857,826287234,826400668,836072576,825379862,829534758,831338073,831639645,832017863,833030528,834073938,825036762,825258534,825385219,828300961,829026562,829026563,829026564,829026566,829026567,829030011,829030013,829030014,829030015,829040979,829082219,830405283,830421370,830693776,830693802,831007793,831011056,831031091,831235419,831235440,831235464,831235563,831235863,831235886,831235901,831235917,833048769,834047102,834047199,834047440,834047468,834048676,834048789,834048958,834299683,835863155,835863393,835863527,826028514,826624375,827020102,827384488,827384529,827384641,827384656,827384685,829103430,830489910,830512015,831106746,832261801,832261867,832261895,832261937,833503625,833506233,825787925,832111319,832727798,832751329,835314530,824899948,831615269,824647545,824670581,825678024,825763604,826285025,826395308,828469957,828484125,831707395,831810922,831810926,832032890,834676526,835005261,835005291,835005350,835005374,835005425,835005458,835005595,835005666,835005702,835005735,835005782,835005803,835005831,835005879,835005909,835005942,835006404,835006419,835006444,835006470,836639935,830984675,830985005,830985024,830985048,830985074,830985096,830985278,825246652,825254283,825259402,826500058,826507058,826768549,827063918,827713844,827730758,829343857,829364091,830294038,830793892,830793963,831326297,831326315,829588127,829588151,826227009,826980118,826986053,833549528,834097856,834924186,834936363,834941566,834944322,829551042,835052331,831669109,825321734,828330525,829123114,829476927,829929411,834292020,831307482,831653071,828616495,833432783,833433974,833433975,826246031,830667858,831783240,830358704,830649403,830649440,830649466,830945414,831263674,831337116,832098987,832968307,833477332,833477336,833477340,835784619,835785659,835787832,824910744,825380301,825387621,825400313,829438238,830073976,831154270,831512813,831513480,831917785,834837656,834883455,833250374,834609361,835822448,828695658,824650894,829026224,831026445,831026466,829332858,831178985,833835610,836053681,836053883,836053885,836054434,836055522,836056240,836056937,836056974,836057679,836060082,836060117,836060289,836060400,836086383,836087332,825540114,826382400,827652654,828155266,828155269,828164540,828164555,828164781,828164965,828164979,828164995,833721386,834375758,836456035,836479809,836484409,836486873,836486930,835673002,827293776,832854385,826604673,826606541,826608600,829635498,829639379,829641453,829652659,830181737,830258419,830351063,830459527,830463392,830469664,830876734,831146228,831325829,831337636,831339316,831341949,831342512,831348256,831381281,831382768,831385402,831393586,831399595,832951488,832954680,832971780,832990823,833338110,833367964,835582180,835668183,835671429,835686669,835693366,835702697,835890965,833744511,834004923,835294180,835613431,835613743,827066620,825680680,825680744,826080770,829450688,829747711,829780693,829780695,829780696,829827668,830149735,830149736,830696495,830742420,830912257,830912496,831459643,831460161,831519080,831519081,831884708,831897324,831897325,831897405,832827291,832905468,832905987,833540312,833574821,833589427,833589428,833634021,833745290,834283014,835192871,835193736,835294369,835639188,836650979,836664706,835496994,825224932,825477530,825480227,826172207,830181508,830541753,831062091,833669331,834064528,834066339,836640295,825844799,829316211,829577958,829577973,829577994,829578022,829578047,829578071,829578094,829578122,829578165,835227408,835790574,825791661,824751152,825254284,825259403,825261174,825263158,825274751,825454787,825465147,825470174,825478383,825697685,825709653,825756526,825759581,825781676,826036158,826585938,826591734,826592343,826601136,826824931,826828919,826830946,826837616,827360502,827381167,829803854,829809303,829813110,829841550,829858582,835321457,835401649,835758939,836013149,836108425,836118809,826143970,828641350,834258423,834265052,834265375,835652327,826986149,824884499,824471518,824567209,824570518,824570536,824570541,824570551,824570556,824570558,824570563,824570813,824570828,824570836,824570842,824570904,824570914,824570917,824570997,824571001,824571010,824571011,824571152,824575580,824591326,824596152,824596154,824596156,824596158,824596163,824596166,824618653,824618665,824618672,824618711,824618719,824618725,824618730,824618771,824618775,824618779,824618783,824618795,824619020,824619039,824691409,824722850,824788673,824788690,824788698,824788725,824793208,824793221,824793234,824793709,824793722,824793746,824793764,824793769,824793778,824819450,824850781,824851264,824855297,824855314,824855325,824855341,824855656,824855670,824855680,824861827,824871430,824871436,824871454,824871492,824871505,824871520,824884258,824906831,824913906,824929364,825115511,825115529,825115548,825242139,825242146,825242154,825242167,825242183,825242187,825242199,825242213,825243584,825245169,825245184,825245192,825480501,825480524,825480533,825506426,825506439,825506444,825506451,825534491,825534504,825538273,825538319,825538346,825540094,825540106,825540143,825636633,825747343,825837082,825975907,825975913,826365359,826385409,826393093,826399765,826399779,826399793,826402052,826570914,826583790,826583816,826583821,826584827,826584836,826584841,826665363,826776801,826794146,826852724,826852857,826852875,826852900,826852918,827354614,827590991,827751057,827849719,827941664,828146313,828146318,828146326,828159572,828159582,828159595,828444807,828493875,828494775,828494785,828495620,828495633,828495851,828495859,828495861,828495877,828495889,828495903,828495927,828495937,828495960,828495988,828496151,828496156,828496168,828496178,828496196,828496217,828496227,828497411,828497510,828498801,828674393,828674398,828674406,828674418,828674421,828674433,828682427,828688842,828696063,828753594,828753614,828753622,828753627,828753636,828753646,828753660,828753916,828753923,828907285,829039821,829039914,829039991,829042078,829042203,829042246,829042253,829042279,829042627,829082902,829082968,829083014,829173037,829220127,829252578,829252602,829252729,829279329,829336858,829336871,829336886,829336907,829336929,829336945,829374514,829374526,829392590,829393883,829394270,829394272,829394359,829395029,829395047,829395049,829395126,829395129,829395135,829395137,829395139,829395140,829405383,829509667,829512769,829516520,829516545,829516595,829516613,829610072,829713432,829713443,829713450,829744724,829744739,829744747,829744761,829744772,829744777,829744782,829744800,829744807,829744823,829746530,829746537,829746556,829746592,829746607,829746622,829746628,829748266,829748271,829748278,829748283,829748289,829748323,829748339,829748349,829814984,829815031,829815065,829816069,829816080,829816100,830214440,830214446,830214453,830214461,830214467,830214471,830214484,830214579,830214992,830215000,830215006,830215014,830215026,830215048,830396105,830502239,830818962,830868022,830868029,830868039,830868051,830868054,830868074,830868103,830868111,831201322,831247644,831247652,831405366,831405476,831405479,831405485,831405492,831405495,831405527,831405595,831405602,831405610,831405612,831405615,831405632,831405648,831405659,831410733,831410781,831835169,831835196,831850491,831850496,831850508,831850514,831850531,831850746,831850750,831850764,831890519,831890521,831890523,831890526,831890528,831890529,831890536,832145926,832283437,832457612,832458417,832458437,832458449,832458465,832458564,832458573,832458578,832786043,832786048,832811032,833076346,833076357,833127403,833184624,833308400,833308410,833308441,833473736,833619793,833639008,833678237,833678259,833839277,833891710,833928000,833981877,834011143,834011213,834011221,834011224,834011228,834011232,834011367,834012506,834012510,834022918,834022919,834022921,834085401,834091909,834213320,834219835,834536138,834879549,835132911,835367710,835515398,835541571,835548107,835548118,835548125,835548131,835548134,835548144,835548150,835548159,835548167,835548172,835548180,835548224,835548229,835548237,835548243,835548250,835548257,835548267,835548276,835554431,835554436,835556137,835556144,835556149,835556159,835556165,835556191,835556199,835556209,835556220,835556227,835556250,835556252,835556258,835556262,835556268,835556273,835556284,835556288,835556295,835556625,835556631,835556635,835556646,835556665,835556851,835556853,835556857,835556864,835556873,835556882,835556887,835556896,835556903,835556918,835556927,835627222,835672764,835672782,835672790,835682426,835725248,835725272,835725315,835901973,835901982,835972991,835973000,835973015,835973025,835973040,835990654,836513918,836740074,824499239,824699848,828010251,828580000,829073559,829143385,832395898,829051917,834091785,834091840,834092154,834092206,834092289,833253956,833291526,833682791,825385343,828982587,829082321,831655365,831909818,825609022,826106597,826220010,826792165,829639175,829641279,831763283,831768110,833331351,834450802,835318109,835318258,835973393,836353891,828670652,828971596,830417288,831031185,831031931,834045164,834104272,834104276,834701157,834701160,835495110,835572609,835572731,835572755,835572783,835572800,835733429,824660633,824664249,824674237,824683202,825201636,825206764,825219348,825228381,825238312,825671239,825673955,825681751,825700346,826148486,826150563,826154784,826158007,826346565,826695366,826698242,826703371,826711554,827356534,827522783,827532056,827538765,827546544,827776573,827778245,827779174,827780197,828313914,828317386,828375600,828379364,828888489,828891832,828894738,828993472,829061409,829073020,829083784,829419190,829486337,829490817,829496752,829967339,829981628,830066110,830070719,830468590,830473949,830482754,830512405,830536956,830951604,830962266,830974117,830977904,831444015,831445479,831447704,831448826,831883481,831886267,831930704,831936325,832266452,832347838,832357028,832364623,832364693,832369347,832926818,832961738,832970993,832974381,833158223,833158244,833506070,833525660,833530732,833540361,834124104,834172180,834178724,834184395,834640699,834647040,834651265,834662122,834717131,835194890,835196507,835198216,835200828,835644181,835647337,835648636,836085837,836152466,836177854,836562360,836566806,836569653,826381180,832037161,833956690,826980267,826980277,826980294,826980330,826980649,826980672,826980690,826980968,826981129,826981141,826981161,826981353,826584847,827959986,827960003,827960012,828314621,828315612,828315784,828316204,828316907,828316908,828316909,828316911,828316914,828320120,828322277,828328156,832568623,832569373,832584610,832585941,833829023,834578361,830592170,832226150,832408068,832412629,826110853,826180792,826188137,826197152,827830384,827834056,827880570,827890721,828029833,828030026,828040199,828045916,828078136,828083952,828609045,828973031,829476697,829503712,829508350,829520420,829523014,829524364,829531259,829533380,829535274,829537779,829541133,829550101,829552998,829563978,829648914,829957232,830069412,831245206,825525632,826076303,826076512,826080253,826603575,826604612,826605736,832427379,832432542,833428128,824968196,825433381,826032805,826036687,826498529,826962328,827681883,828716764,826056446,826056488,826506755,826603461,827024582,827040280,828173012,828173396,830357527,830357595,830362535,832302331,835099407,835160868,824550637,826592824,827517540,829032257,829826856,824896114,831601111,831601120,831601150,831601158,831601186,832987279,832987372,832988373,832988466,829483307,833642645,833819734,833836349,833836365,833865318,828848699,830446255,830454787,830456801,830456808,830497212,830497274,830531347,830559317,830561263,830561300,830593504,831258457,831258523,831472904,831472915,832702964,834264001,834338704,832078762,832138391,836640375,831166417,831169175,826155257,826802874,827736050,827736132,829719682,832246919,825385362,831031216,834085410,832540314,834507537,836192924,831371443,832246392,832246406,832246425,832246436,835328731,835564974,835564987,824725868,824733638,824993869,825388379,825393440,825570536,825712333,825712351,825712361,825712370,825712383,825712400,825712410,825712424,825712439,825712823,825712838,825712847,825712858,826175723,826175912,826178670,826262433,826404206,826424883,826424927,826424950,826424959,826424969,826424993,826425030,826427272,826427300,826427319,826427328,826427921,826427923,826427947,826427973,826427987,826428043,826428057,826428341,826428348,826428699,826428707,826428730,827166923,827169438,827789057,827791327,827856211,827917809,828060100,828060116,828060130,828060146,828886910,828887385,828955630,829098718,830547861,830770361,831089130,831089891,831099847,831099919,831099986,834318043,834318071,828966107,828966136,829020387,831392309,832097226,832211218,832250740,832251109,832251122,833061007,834830445,835566901,835566919,835566952,835566962,835566968,835566977,835567006,826515770,830526636,830546603,824984134,824989312,824991353,828140113,827127198,832866778,833846291,833846299,834066061,834066133,834066142,834066210,834066222,834066287,834066303,834066411,834124994,834125046,834125355,834239705,834239736,834239748,834239760,834239786,834239820,834239845,834239926,834239952,834341797,834341881,834341939,834531531,826322730,826331813,826343796,826388489,826390624,826398038,826400466,826455013,826458340,826465455,826486023,827587195,827601142,827646239,827674492,827717373,827932681,827983805,828037512,828043155,828048119,828059215,828089994,828106024,828134500,828190637,828200079,828402455,828473237,828485098,828489463,828501892,828917294,828923401,829773589,829789438,829811704,830162446,829459976,832070778,832535698,833191779,833196158,825014676,825975359,828119880,829221286,825392184,824910766,826007471,826012379,826012473,826012594,826013314,826022130,826025328,828537652,830378613,830378628,830378641,830378650,830378671,830378708,830378838,830378866,830378893,830378906,831550453,831869406,831869425,831869440,831869462,831869479,824679877,825719799,826429652,826429669,827465494,827632226,828399202,828399759,828399840,829343287,832528493,827054511,827478210,829217848,829715494,830043086,830056193,830086586,832617398,832631504,833759138,833867536,833882787,833901676,833907460,834210912,834834110,834848449,835430741,828360781,828360800,828413030,834217445,835228373,835229296,835230909,835231332,835238751,835338834,835338835,835339015,826730138,824661946,824793789,825622534,825622537,825622546,825622553,825622562,827831299,827970225,828468292,828468308,830745291,830909143,830909150,830909151,835286043,835986818,835986971,835987057,827417837,824884673,824889816,829177742,829179754,829201342,829676046,829699365,830771984,830779312,835397999,835402009,835831550,835844014,835851600,835852631,835868257,835868288,826384234,829222931,829982790,832898002,828260495,828440180,829528693,829530411,829536442,834650021,827870933,827905535,827906431,827919612,828314036,825661478,825895781,826350538,828095499,828099355,829458377,835073684,824669040,825028797,825032846,829211348,825461651,834038138,834038192,834038536,834038618,834038787,834040118,834040783,834040829,834040933,834041081,834041188,834041397,834041499,834041962,834042598,834043153,834043188,834043243,834043391,834043999,834044349,834046955,834047008,834047031,834048167,834048398,834048979,834051299,834051509,834051560,834067750,834507904,834508295,834631312,834631451,834633247,834635944,834638239,834640350,834641308,834725188,835060901,835060913,835435229,835435538,835439208,835883969,835884144,836098254,836103909,836106650,836108941,836115696,827270697,827639154,827891566,827911100,828438097,828641939,828649245,828649263,828649297,828649345,828649392,828649413,828649482,828691693,828937334,828972900,829024941,829142594,829182316,829507784,829507826,829507877,829507979,829507991,829790649,829931917,829946101,830203924,830204104,830970387,824509800,824571162,824571170,824571177,824571182,824571187,824571196,830144136,830438290,830857519,834021636,834375637,831922617,831924367,831925247,831925270,831926219,831926255,831929568,832620482,830560881,827251474,828344180,830497301,831520492,831598190,825030818,826309454,827026727,828470608,828539194,830938233,833282104,833282338,835642142,834342512,834342548,834342564,834342580,834342603,834343019,834343059,834343196,834343233,834343277,834343328,834343388,834343439,834343620,826372715,826372740,826372861,826372879,826372979,826373124,826373152,826373171,826373207,826373589,826373606,828206226,829542910,829999035,825571046,825571060,825571067,825571073,825571087,825571109,825571117,825571134,825595188,825599479,825602153,825602157,826074111,826074125,826074137,826074144,826074163,826074176,826074212,826074219,826074237,826074704,826074709,826074720,826074734,826074766,826074771,826074795,826074805,826074821,828276068,830335635,830342297,830343939,830515756,830698378,830947809,830989341,830994026,831770020,831770055,831772573,831773504,831773534,831773563,832371232,832372284,832372297,833859366,833859372,833859379,833859383,833859392,833896379,833896399,833896407,834097671,834913639,835326564,835326575,835326593,835326617,835326628,836279694,836279713,836279732,836350527,836565381,826889512,826925830,828485477,827343512,827419379,827475170,827653286,836643343,836643454,824790352,824946570,825865473,825865523,826249015,826285597,827093012,827094973,827102276,829089760,830570340,826088425,826097224,825491059,825492163,825587774,827240453,827302177,827396693,831456984,834609940,834942394,826151940,826160646,829073035,829602424,830465580,832385210,834651667,835583082,825535745,825555921,828640061,828646774,828654457,828681275,828681925,828682476,828980735,828990395,828991931,828992915,829005706,829007932,829020110,829039445,829049999,829056506,829067784,829090832,829256881,829334674,830470690,830702232,831655390,831669686,831678585,832476707,832481098,833056847,833093777,834045385,834072256,834091710,834095559,834100967,834104504,834108170,834124582,834624367,834625132,834629855,834636766,834638785,834661783,835468357,835469216,835495416,835509995,835566738,835798176,832250717,832250720,824569592,826307244,826320848,826409384,826416779,829424104,829424298,830002135,830002272,830002540,830003980,830004311,830038957,830322791,830323243,830341580,830446913,832578142,833045287,833045606,833045717,833045787,833045868,833045931,833407023,827647174,829341390,829606957,829879398,830618804,830795925,830909249,830909251,830909254,830909256,830918204,830918236,830950728,830983983,830984053,830984082,830984120,830984159,830984192,830984228,830984262,830984323,830984347,831026556,831026588,831050886,831050922,831051154,831051193,831128764,831239225,831297336,831297452,831336201,832793616,832801692,832846213,827197863,828216452,829971909,831066869,836704500,830475594,824552116,825112866,827814786,828234241,830093838,830093856,830093881,830093928,830094351,830110170,830110208,830110248,830110309,830118706,830209722,830332502,832792681,834034147,834213243,829681471,831343234,831343249,831343263,831343269,831565183,831565194,831565781,832749000,832789604,832841069,832841077,832841190,832841204,833810524,828529587,824499241,824500215,824523001,824633247,824639882,824651344,825564444,825656826,825661617,827451125,827594616,828043157,829749069,829917687,832053067,829326939,831443289,831446374,831446389,831453484,831456983,833636790,834608968,834608971,834608976,834608985,834608987,834622319,834622360,834622729,834628707,834630100,834632474,834989065,834989111,834989475,835164397,835167854,828491567,830931113,830931171,830950519,832042235,833415440,834118101,825310234,825318237,825320028,825551518,826309217,829192992,829550102,829563979,829576109,829954778,829966994,830980871,831004317,831789002,828344126,828344287,828344493,828361640,828375740,831611597,830535691,828458242,836215184,832293345,833409526,825255239,825255268,826138604,827372283,827416397,828200718,828681572,829950800,831228512,833877002,835116802,828632687,828714911,824593427,824597495,824944276,824962413,825221430,825262685,825263694,825276463,825276569,827038377,827038693,827039574,827043705,827050983,827340611,827340978,827566299,827566460,827566465,829273647,829300961,830231410,830548968,830549024,830618305,830647068,830663719,830676616,831172445,831172446,831172491,831291959,832430718,832489104,832497476,833313118,833319355,834471092,824546244,824646551,824648055,824801564,824801574,824801726,824801781,825025480,825081707,825081735,830788491,827780195,828380252,832267292,832348649,832943909,833540427,827047289,827488438,828207265,833925848,834770760,834770784,835637046,835637175,835710796,835710819,830159281,830276008,831595753,831748209,831765025,831933871,831953998,831954694,832362489,832934983,833014056,834135736,824633645,824640996,825195310,826242769,827260876,827360056,827383273,827412583,829364432,829443434,829554643,830072338,831919653,831922006,832923848,833594508,833780402,834060747,834659401,835205057,835208255,835651742,835652521,835652661,835652670,835662159,835662306,835662312,835662867,835691839,836267830,836267842,836267853,836441995,836514576,833363565,833558530,826460035,830346741,830347076,830813218,834461545,834520058,834520105,834520177,831354111,831731990,824471414,824490198,824543130,824553946,824563467,824577249,824926377,825181997,825189417,825192645,825205999,825226538,825244437,825246592,825256914,825259006,825274471,825275759,825282740,825298159,825310614,825312790,825326613,825481749,825567921,825572976,825988502,826274227,826274402,826638367,826640249,826814465,827360088,827646606,827674493,827678206,827690008,828362867,828580201,828682237,828695609,828710880,828713398,828726606,828734061,829842493,829850020,829867370,830327822,832377617,834525568,834540420,834593201,824475130,825049922,825060947,826390785,826419228,826474786,826737241,826742749,827053434,836178951,836179030,836568759,836570529,836572416,836691904,836721338,831285152,824851161,832046500,833729216,831095128,832675429,830893714,832500951,825085443,830932215,836367705,836372606,825581937,825581967,825582786,826091510,826091526,827181431,827181464,831842486,833449566,833449609,833449646,833449672,833449743,833449769,833450059,833450210,833453485,833453508,833453535,833453554,833453572,833453639,833453665,833453696,833453716,833453750,833453794,833453850,833453961,833454171,833454209,833454237,833454267,833454317,833454353,833454597,833454632,833454663,833454685,833454747,833454815,833454844,833454872,833454900,833454948,833455018,833455040,833455063,833455097,833455129,833455162,833455187,833455215,833455247,833455277,833456365,833456388,833456416,833456451,833456482,833456535,833456557,833456583,833456616,833456658,833456684,833456733,833456764,833456802,833456827,833456853,833456867,834518113,834518202,834521588,834521694,834521775,834521908,834522033,834522156,834522241,835820102,835039074,830276468,830276973,830277067,832408847,830921552,830439821,830441983,830550931,832108956,834648026,834651554,834659380,834667426,834669004,834669006,834671688,834673162,834676337,834786736,834865983,834866287,834867609,834884072,834885576,835455827,836534137,836534247,836538165,836546797,836548888,836550823,826675445,829410036,829410574,829411104,829411137,827831615,831345356,831348258,831372513,831474082,831933234,832032057,832927958,832940115,832965149,833316189,833316200,833538916,833538948,824527813,825044671,825149313,825150170,825470331,825480472,825551966,825997721,826026321,826044365,826045788,826136990,826138101,826143403,826149669,826512099,826515393,826670127,826670787,826698006,826699269,826781246,827048468,827655821,827991532,827997041,828122777,828299038,828302211,828832980,829753387,829761563,829910456,829965179,830085882,830118321,830278300,830405673,830405922,830435860,831451346,831452366,831452570,831453062,833482362,833928266,833938431,834255471,825478385,826311163,826314709,826850121,826898076,827323665,828139288,828870830,829569199,829733609,830248083,826604197,830791231,831749340,832927351,832956534,824467563,824467664,824467693,824467833,824467856,824467876,824467891,824468037,824884060,824884177,825368640,825368668,825368705,825368729,825371051,825375981,825377895,825377943,825535157,825538055,825555382,825555922,827679366,828070870,828142226,828142249,828542542,828559097,828559117,828559621,828559661,828559684,828562289,828563089,828563122,828563155,828563190,828563208,828564416,828564438,828564456,828564481,828646264,828646775,828654458,828657688,828661332,828661664,828662238,828662441,828663600,828663602,828663616,828663617,828663698,828663699,828663700,828663701,828663703,828663704,828663708,828663709,828663710,828663711,828663745,828663746,828663747,828663750,828665247,828666512,828668275,828669618,828669686,828670499,828670559,828672770,828674479,828674913,828680881,828681927,828682668,828684111,828684333,828687922,828689802,828690390,828691082,828691553,828691554,828698562,828700015,828872200,828954501,828954586,828954739,828954851,828972214,828980729,828983390,828991932,828995572,828997381,829005156,829006329,829006741,829007373,829018634,829025546,829039048,829050000,829055755,829056507,829069001,829103713,829104188,829257692,829348324,829351460,829352812,829533563,829636068,829636089,829636107,829636136,829636544,829636577,829638466,829638488,829638508,829638860,829639099,829639118,829642491,829644503,829742648,829809853,829809943,829810058,829810097,829845554,829900700,829926750,829926760,829926765,829926770,829926777,829926789,829926796,829926823,829927024,829932026,829979925,829980001,830010168,830071274,830114579,830406005,830406019,830406028,830408250,830408261,830408532,830408552,830408570,830408606,830408620,830408741,830424767,830424825,830435482,830438842,830440117,830445613,830460980,830507649,830508009,830508111,830701956,830702462,830702957,830703499,830704032,830705754,830970483,830970494,830970511,830970519,830970615,830970688,830970892,830970920,830971955,830972253,830972265,830972284,830972293,830972313,830972329,830972341,830972362,830972374,830972393,830972408,831024116,831031913,831035599,831037997,831050276,831050381,831432966,831468062,831468069,831468076,831468082,831468085,831468091,831468151,831468154,831468164,831468262,831468270,831468274,831468282,831468297,831468300,831468306,831468311,831670004,831672805,831904617,831907015,831909730,831909777,831915483,831915499,831918350,831919636,831919644,832463149,832463163,832463179,832463194,832465572,832465586,832465605,832465760,832480440,832537021,833047492,833047949,833078854,833097329,833234646,833234665,833234703,833234721,833234738,833234760,833234779,833235058,833235068,833235080,833235093,833235124,833235147,833235165,833235179,833235197,833235448,833263031,833263093,833804104,833804285,833919205,833919224,833919940,833919979,833920004,833921169,834043408,834043412,834043417,834043435,834044139,834045387,834072257,834073934,834074996,834075064,834075576,834076379,834091316,834091939,834101360,834104505,834105058,834105366,834107866,834108171,834111400,834111907,834134827,834135089,834138694,834138793,834139141,834620922,834621240,834621253,834621259,834621262,834621271,834621717,834621720,834621729,834624368,834625371,834636767,834638540,834638786,835362542,835362960,835362976,835362988,835363137,835363153,835364279,835364296,835364316,835364343,835366068,835366082,835366104,835366146,835366528,835366545,835366613,835366647,835370785,835370800,835370815,835370824,835370844,835383748,835383841,835383919,835384008,835384076,835384167,835384254,835384339,835468359,835470393,835491043,835494973,835496072,835497861,835498613,835501806,835507227,835507630,835509996,835510493,835511740,835511899,835511900,835512136,835512982,835517099,835517767,835520719,835566340,835566739,835575617,835789835,835790059,835790234,835861017,835872000,835872949,835873078,835873098,835873139,835873162,835873182,835873234,835874384,835874440,835874490,835874518,835874555,835874582,835874603,835875009,835875048,835875061,835875086,835882741,835887755,836668321,836668330,836668348,824481856,829318947,829789440,831180604,834270397,827721441,829095162,829226706,824959231,824963946,824972178,825011132,826338936,826342703,826346746,831648303,835467676,827405457,828588778,828590894,830904711,832318323,832318564,832896146,827029987,829660560,829666453,832981735,833254966,833464906,833464907,833464914,833465237,833465241,833465242,833465247,833581987,826635843,828407463,828766736,828817058,828817059,829693288,831450223,832351939,826058622,831127388,831129253,835415936,830856471,831236677,836572196,828060046,828060067,828060076,828214882,828387115,832700153,832737494,835961302,835961336,835961365,835961382,835961391,835961409,835961422,835961437,835961454,835961469,835961493,835961516,835961528,835961554,835961570,835961582,824664021,824683365,825217288,825287419,825700525,826285349,826702166,826719342,827520872,827777205,827778086,828894943,829066365,829486108,829981455,830488777,830962031,831031555,831031562,831031573,831031586,831031640,831031643,831444910,831444911,831444914,831444918,831444926,831444930,831446662,831885573,832944046,833150447,833520857,834651538,834661859,836172116,836568708,826205386,830467591,825843017,826484030,827035839,827038502,828636800,828770151,830052169,835427289,824719325,825372900,826218748,826233474,826492948,826508755,826540299,833556862,827820019,830506512,827467904,827491180,827509270,827561612,827569562,827732233,827761761,827857533,828038168,828220943,830113658,824479258,824495694,825518932,825519037,826770236,826770267,827394501,828017018,828017074,828039315,828060442,829136601,829787528,831781854,832202562,833038059,833057944,833058530,833175121,833207572,833207962,833309988,833310040,833310117,833323877,833330723,833350029,834099257,834111200,834112367,834121723,834122596,834677836,834897636,834899782,834932066,834939510,835409619,835447594,826722157,827028148,825981892,825981933,826178740,826341065,828997792,834125385,827792739,826675912,827025682,828643203,828717164,828891165,828891201,828637759,825933921,825934877,827887095,829929127,829929425,829969151,828603474,828614120,828626920,828687219,828695611,828697677,828708728,828736644,829752392,829789441,829792317,829794259,829799852,829809306,825276797,825468277,825809282,825814871,825821971,825830331,825843336,827864003,831063017,831071294,831087336,832665720,833247677,833319905,836176265,836177257,836280154,824818297,824819835,827233028,827471958,828785995,829119004,829894408,829926146,829961479,833882833,834195586,834195724,834196052,834196177,835831958,835832085,835832479,835833211,835834828,835889133,835889262,836196000,836582235,824810493,826110761,825015477,825060644,825093385,825258602,825552420,825559002,826041551,826057289,826253578,826319772,826786284,827346574,827407036,828618136,830813954,831022480,831022499,831022521,826147640,826152838,828815399,828815408,836147414,828421103,824537705,830905568,831514053,831882700,832144437,832318725,832319776,832320415,832892723,832894681,832947188,832947252,833196375,825937525,826616384,826616400,826616423,834909670,834909691,834909735,834909765,834909798,834909839,834910289,834910311,834910334,834671694,834721248,824535858,824536116,824536638,824536654,824536700,824546876,824774640,824776083,824777475,824777557,824778013,824778053,824830808,824830980,824831090,834399404,834634006,835436011,835800003,836000420,836056257,836076861,836076869,836076940,836076950,836076963,836076975,836077438,836077456,836077466,836077595,836077602,836077609,836077617,836078407,836078412,836078419,836078429,836078436,827118007,827687946,832551201,832737116,834002192,834002199,834002207,834565742,832639144,824568087,824568113,831512726,831796250,832728776,833469919,826196081,826196110,826384085,827892723,827963219,828306394,830309876,830397499,830462380,830506562,830904550,831010140,831010156,831010174,831201888,831201911,832029972,832046379,833274808,833275659,836661902,833558581,829833759,835864651,832369355,836322223,836322411,824999608,824999650,824999686,824999750,824999780,824999808,824999871,826714243,826768551,827381169,827674495,827678207,827828349,827828363,827828406,827828423,829931830,835048397,826431383,830789694,830794281,830836816,829376732,829598008,829605719,829621737,829733610,829740071,829754894,829813596,835979839,831763345,835126266,831485672,831485917,831488639,831490536,831612581,831716489,832071335,832544127,832544831,832547603,832547607,834613030,834614081,834614088,834614310,834614717,835659326,835659387,835660673,831039187,832398618,832979762,832992164,832992238,832994252,832994866,832994907,832994961,832994990,832995073,833093010,833096521,833096571,833096626,833096689,833096722,833096806,831804600,831967773,833562671,833562747,833562766,835056502,835056549,835056597,835056617,835056632,826322277,826322300,826322324,826322347,826322359,826322362,826322371,826322385,827588329,827588340,827588354,827588369,827588380,827588612,829612435,829615933,836122048,826615228,833677968,835271926,835281734,827656302,827803465,828377319,831342143,831508743,831713181,832262944,832276930,833550817,833550822,835972176,835973356,835998664,835998674,832283342,832510702,833257746,834966484,826354000,826931798,827018603,834100207,834204200,834681556,834783966,836407825,824467561,824818716,826360059,826458852,828616498,829027307,829223098,830032448,830088081,830088266,831028906,832571807,833071251,833149025,833660313,834086288,834086517,828831220,828831230,830405708,830405961,830409625,829202371,829202567,829202594,829202621,829202650,829202671,830422813,830422831,830422842,830953757,830953781,830953803,830953811,830953823,830953840,830953853,830953875,830953902,830953911,830953926,830953941,830953965,830954349,830954359,830954370,830954386,830954407,830954418,830954435,830954450,830954467,830954488,830954510,830954530,830954556,830954571,830954579,830954906,830954946,830955427,830955438,830955812,830955817,830955832,830955847,830955855,830955867,830956022,830956073,830956093,830956111,830956115,830956126,830956142,830956153,830956167,830956181,830956201,830956220,830956237,830956262,830956272,830956297,830956310,830956332,830956361,830956819,830956828,830956855,825200639,825200668,832353329,833804469,834145773,834643729,834649039,834360122,834362719,834362744,834362922,834362931,834362969,834363090,834363112,834363149,834363377,834363393,834363405,834363425,834363642,834363668,834363682,834363690,834363705,834363985,834363997,834364019,834364041,834364079,834364401,834364476,834364513,834364540,834369492,834369542,834369560,834369717,834369753,834369792,834369840,834370141,834370388,834370518,834370536,834370574,834371527,834371555,834371590,834371911,834371945,834371961,834371982,834372287,834372309,834372329,834406115,834406446,834406873,828070902,828070992,825032379,825032836,825032864,825032890,825032910,825032953,825032978,826096190,826112271,827491171,827491205,827491216,828788118,828833112,828833123,828833131,828833144,830075483,830075592,830112633,830203902,830892185,832222805,832222820,832222847,824471448,824479372,824490580,825073637,825077298,825372901,825413281,825451330,825455645,825483767,825488032,825499658,825917375,825970709,825976705,825995004,826025468,826357894,826374552,826941849,826973473,828553476,828560518,828578604,828586143,830635365,830637102,830637837,830649146,830660155,830667710,830673704,830684933,830688267,830703753,830728009,831223578,831226171,831231766,831239476,831255336,831312851,831329259,831330296,831335805,833276833,834059232,834062443,834072804,834074587,834141840,834421119,834427187,834435566,834445204,834451326,824668233,825687325,825687418,825890490,826312896,826313171,827837672,827935630,828360100,828401144,828401185,828401210,829607358,831272007,831944964,825701965,829601350,830506541,831050679,831166929,831171553,834319290,832339969,827979436,827979572,828107414,827288088,827340361,827340386,827340422,827533193,825590200,833073173,833682673,829587903,829598322,829604891,829619081,829836052,830352334,831822197,831831406,832522490,825792151,827860717,827860731,826108561,826302757,826368819,827672498,833016044,828066015,829882211,828817137,830397367,830412672,830413075,832981734,834634971,834635000,834857422,835039033,835039053,824535958,824560861,824564033,824564041,824568876,824568884,824568895,824568927,825144534,825920940,826358120,826613410,826613447,826613455,826674396,827632064,827668625,827678280,827974889,828615982,829322841,829400656,829401046,829402219,829402225,829402708,829402713,829402718,829402720,829402727,829402729,829402731,829402735,829402737,829402738,829402751,829402756,829883163,830256234,831529395,831789518,831800107,833661493,825980915,825994661,826252099,827330755,827614794,829716583,832809541,834972356,835129099,824993755,825001370,824669419,824686466,824738203,825207094,827379732,830211799,832783134,835914914,824953901,825004891,826025913,826030587,826477355,828097105,829504790,829762026,830175165,832672557,832686634,832712983,833497593,826569904,828016712,828384719,829109074,829492912,829833466,825921687,829856170,829856295,829856324,829856416,829856431,829856438,832327877,824696403,825065689,826417451,826545703,828936648,830582292,830605764,832944431,831580980,832625220,833636513,833920227,834475313,834475325,834475336,834475349,834475376,834475991,834476010,834476029,834476117,834609431,834684839,834684854,834684875,834684890,834684896,834684913,834684928,834684946,834684960,834718096,834718113,834718121,824730724,825871698,826128417,826664102,826907608,827062103,827226898,827227187,827227188,827232579,827431382,827461185,830382101,830917830,833470975,833657042,834101258,835019129,835182034,835186214,835186246,836122585,836437661,836437670,836497635,836497640,836498367,836498369,824954447,825105128,831386902,831790969,831791117,832239593,832422544,835860669,828266936,833373892,833373902,833373915,833373973,833373985,833624312,832576013,825227959,825234615,825284658,825325207,827459248,827500576,829488166,832278587,832501741,833076868,833159347,824956031,825012196,827095323,828096818,830182876,831308041,831310495,831718367,831721856,831729480,831737833,831771131,831771388,832642576,832648355,832715200,833498312,834710987,828233284,829114984,829114986,829117244,829120437,829121033,829124013,829127223,829314896,829780659,833072401,833931971,833932449,826925831,827707048,827713427,827871621,827879104,827890724,828418968,828423305,828431074,828697678,828708730,828960136,828967971,828970776,828978419,829234817,829823917,829825857,829831597,830133505,830139084,825395492,825395494,825927640,826448563,826449337,826640700,827330565,827331860,830191085,833586959,833586977,833586990,836582709,825479463,826810182,826893061,828005191,835378095,835378176,827084444,826132071,833787851,833793671,833799948,833806126,833362021,833362162,827340929,827461365,827462489,827466078,827466107,828980003,830018742,830206651,830296069,830296084,830296103,830296117,830296155,830296177,830296197,830623706,830958147,830958163,830958850,830958862,830958886,830958906,830958947,830958985,830959007,831174196,833700160,833701819,833741345,833741375,833742039,833742104,833742116,833742155,833742178,833742196,833742209,833742233,833742251,833743230,833743267,833743294,833743320,833743746,833743898,833744471,833744489,833744528,834863143,834863169,834863348,834863558,834863588,834864802,834864822,834864844,834864866,834864923,834864952,834864979,834865750,834865774,834866866,834866881,834866926,835993058,835993072,835993084,835993093,835993104,835993132,835993143,835993156,835993180,835993201,835993233,835993267,835993300,835993316,835993868,835993873,835993886,828604270,830736410,832339382,824689842,824699850,824739229,825693313,825734759,825744742,825747028,825766305,826398044,826415690,826420299,826732303,826745378,826751245,826766237,826771721,826776569,826798156,826802942,826807611,826981898,826988127,827389047,827403945,827419415,827858433,827871622,828484228,828954765,830162014,830162034,835118383,825851768,830267765,830269274,830552442,825235790,825285714,825325631,825379093,824482057,824977334,825996080,828113922,828122657,828126130,828132220,830190096,830229358,830461855,830495320,830717970,830737062,830741600,831713186,831721007,831451229,831651983,832424343,832946129,833415502,833687248,825320331,826242154,827022172,827231861,827232357,827855304,827920014,828022519,829421329,829421332,829429601,829504172,834244268,828431785,828433705,828434847,828453491,828454672,828470455,825675858,826054127,826111735,826342474,828189747,829171257,830556277,832692447,833031598,825241380,825354177,825563172,825699880,825700697,825701313,825705859,825748191,825766994,825873046,825873337,825887480,826201503,826331208,826759548,826761012,826982675,827041004,827043288,828371681,828378245,828429403,828430825,828481330,828505433,829063146,829602141,828955584,835459614,836331879,836646851,825574532,827659982,828067268,828067326,828067851,829845353,829854159,830812955,831115364,831507819,831886677,831886688,831990597,833558516,833558520,835344796,835358054,835365201,835480500,835506389,835753822,836093618,836093638,836093661,836093668,836093700,836094114,836094131,836094149,836094165,836094173,836094187,836094196,836094219,836094229,836094243,836094260,836094278,836094300,836094319,836094334,836094632,836094647,836094663,836094674,836095479,827785102,828166723,828167535,831563357,835959333,833099465,833403484,833403514,833403540,824520417,824523880,824527572,824543132,824545419,824547645,824553949,824561855,826097226,826567228,826569905,826575688,826577736,826583951,826591739,826608604,826614232,826617142,826633691,827030936,827045419,827054514,827055140,827061452,827087313,827093014,827102865,827118027,827122893,827128039,827137727,827146898,827569563,827589703,827595968,827602260,827677985,827694175,828174236,828183992,828228572,828768443,828779340,828783474,828785635,829343858,829836053,829842495,829850022,829858586,829868613,829871718,829873712,829875028,830860782,830861091,830862948,828116450,835880544,828431826,828451627,833674223,824962501,825428926,825562251,826029220,826041877,826041955,826076640,826495218,826514146,826604417,826955712,827401974,827401977,827678062,828622398,828713188,830335179,831743762,831799266,831821590,832270751,832307356,832311848,832427583,834702621,834947462,834947496,835594734,832514410,827973442,827973480,828779796,831449440,831449465,831449648,831449659,831449715,832347245,833858332,833858420,833870932,833871444,833881535,833907168,833907648,834084567,834922139,834922678,834940666,834958519,834960033,834960621,835061375,835070335,835070485,835070970,835071612,835477939,825974445,826617748,827019020,828930672,831329037,831703690,832399617,834135215,835369365,825521815,825525434,828028477,828842656,828843195,829632311,829926513,829944810,835954588,836365286,832130687,832193559,832222727,832222742,832222758,832222766,832222779,832259824,833653898,826041553,831449193,832266985,832567653,835329357,826568004,826927797,834433343,824943297,828801048,834889358,832472175,832983579,832983580,826766629,826786286,830086587,830088169,830528541,830719208,833129618,825487727,825700887,825995005,828889260,825843018,825865916,826474375,827124918,828588576,828609049,836508174,832945421,831909803,828629102,828629178,824788738,826344970,826382831,827965634,827980710,828096493,828173611,828327925,829578571,830653659,831059232,831790061,832240903,834484524,833263482,830653949,831408880,832276429,833490845,836675390,836675520,831319117,835460899,824655021,824655095,824655114,824655126,824954482,825033812,825033844,825033862,825033881,825033901,825034153,825034166,825034177,825034181,825034189,825034198,825040478,829756439,832853919,833473737,833473741,833621570,833755019,833755039,831854576,835425580,827163479,831382770,831744333,832280527,832300259,835782575,835982156,835983535,835986737,824499177,825405210,826047685,826088071,826105633,826559879,826626472,826626491,826626492,826626499,826626509,826626513,826626536,826837683,827544802,827556415,827556438,827556458,827556479,827586892,827656023,827968489,827970666,828828258,828828347,828828349,828828363,828828366,828828379,828828381,828828449,828828456,828828463,828828470,828828481,828828490,827524185,829978737,829983513,829984086,827879316,827879332,828036929,831587680,832171283,832171474,832272249,832272359,833153976,833154052,833154146,833249298,833982210,827355569,827355733,827526624,827526712,827526778,827527067,827527677,829077416,829077623,829369549,827638493,833759394,833846172,824855264,825786930,825786947,825786959,825788466,825788582,827079383,828187789,828361882,828363792,829976313,832867549,834148696,830067794,830074741,832201783,835318968,835501281,828839000,831252797,833516934,833523256,830134285,830206487,831485643,835315961,824798215,825276681,825549524,825595716,826132061,826132066,826132103,826174460,827771432,829015047,829170630,829680237,829848682,830059312,830348259,830364768,830410425,830645049,830645482,830645570,830645685,831440172,833207368,833207631,833207766,833229771,833253273,834434551,834435763,834435782,834454157,834460987,834499289,835098472,835098487,835098527,835098541,835098563,835098611,835098624,835098640,835098675,835878311,835878396,835878445,835878489,835878579,835878601,835878893,835878917,835879141,828818973,825062362,825289901,825318240,825336595,825343979,825349816,825359346,825639850,825822807,825852382,825863374,825876031,825890635,826240765,826253582,826262446,826272033,828267483,828344181,828351867,828356486,828361883,828369649,828597657,828852271,828856543,828865060,828908900,829107209,829111176,829347183,829354330,829563981,829705898,830101088,830224243,830258422,830570583,831718388,831764871,831766099,833322552,834585889,834989626,835559406,835572723,827083893,830171362,830172350,832339958,832342094,824854301,825559050,827004659,828200037,828633043,828910061,828912028,829478100,829478629,829533950,824807370,824809332,825976706,827167424,827646609,827694176,828162006,828181695,828188581,829223724,829337532,829343398,829348061,830841197,830846681,830852789,830861379,830877116,828902135,825137241,825137320,825138140,828175155,828520763,828889977,828926949,828927004,830455591,830458602,830956793,825531716,825543977,826610820,826617735,827241591,828369467,828621011,829061504,829150565,829281020,831061662,831062761,831347265,831495463,832475619,832531276,832826390,833431223,833563207,833991171,833991694,834144235,834144318,834144364,834946892,834946952,834947172,834947593,834947696,834972020,834989100,835015475,835068745,835068848,835371573,835848078,835848143,835862481,835862482,825505433,833862777,829619824,831192964,831205165,831621971,831632708,831707902,831744292,831746739,832063851,832067395,832068297,832068722,832117549,826496141,826496152,826496199,826496205,828387314,828387325,828387502,828674327,829071239,829071265,829071276,829071301,829071337,829071346,829071362,829071415,829071446,829080440,829087271,829104980,829105019,829105033,829105049,829105060,829105068,829105078,829105101,829105117,829105127,829105151,829105167,829105195,829105209,829105226,829105235,829105247,829105269,829105297,829105306,829105314,829105327,829105361,829105470,829105479,829105489,829105501,829105523,829105533,829105553,829105571,829184197,830162748,830162769,830162783,830166343,830166357,830166366,830166378,830166408,830166429,830166433,830166459,830166468,830166504,830166596,830166613,830166623,830166632,830169255,830169271,830169320,830169338,830169350,830169365,830169373,830169398,830169408,830169422,830169426,830169442,830169460,830169471,830169490,830169505,830169513,830169528,830171687,830171705,830171720,832162954,832162961,832163010,832283925,832283935,832897595,833235567,833421001,833421012,833534493,833534701,833534864,833535242,833535438,833602719,833602732,833602767,833664494,833664506,833664520,833664609,833664626,833664646,833664661,833664741,834032099,834034031,834039401,834039405,834039410,834039413,834039416,834039435,834039443,834041765,834042005,834042007,834042011,834042014,834042016,834042017,834042018,834042019,834042022,834042025,834042031,834042034,834042041,834042046,834042051,834042053,834042055,834042058,834042065,834042084,834042092,834042099,834042103,834042108,834042111,834042118,834042121,834042127,834042129,834042136,834042142,834042144,834042145,834042149,834042152,834042155,834042156,834042159,834052565,834700334,834700352,834751707,834776685,834780533,835414249,835414284,835414291,835414311,835414342,835414368,835414398,835414421,835414438,835414461,835414502,835414515,835414540,835414564,835414588,835414609,835414653,835414676,835414695,835414719,835414755,835414774,835415315,835415349,835415384,835415399,835415425,835415441,835415459,835415487,835415520,835415539,835415571,835415607,835415628,835415651,835415981,835416080,835416105,835461473,835464155,835470074,835888250,835888276,835888962,835888980,835888990,835889008,835889026,835889064,835889117,835889135,835889149,835889176,835889189,835889207,835889220,835889240,835889265,827316551,834320622,834320669,834956492,829616078,829616101,829616817,829616852,833834532,833894514,833895074,833902274,834771369,834772139,834773864,834775992,834777214,834779527,834781063,834782263,834785527,834786668,834801633,834844540,835095649,835124081,835126519,835126737,835126755,835138123,835140021,835252843,835397249,835887541,835887568,835887585,829529823,832412470,832414571,833828428,833854864,833861667,833869937,833965874,833983312,834776580,834843087,834844539,834875768,834878637,834996694,835100995,828711952,831370875,831370901,824751486,824752151,824761047,825452763,825890522,830480303,830493306,830495495,830497833,826899310,827872597,828023017,828570090,828570136,828631034,828756275,827531176,827561730,830431509,830532470,830869749,830869764,825543197,826520307,830311147,830911784,832244624,826072853,831387625,831543420,828884488,832338914,825430035,833518093,829912488,829912494,829916310,831213647,831213670,834836022,834840348,835842361,836094800,836094911,836094933,826411311,826419167,826437338,826746551,826769415,826809402,826906277,826926186,827266694,827266713,827397775,827457892,828087522,826184426,825280966,826090010,826200715,826251973,826633891,826642957,827149989,827152869,828109558,828110518,828110745,828191023,828669737,828846670,828846729,828953986,829232874,829632034,829696189,829696447,830106083,830106664,830117031,830154100,830160789,830161008,830497192,830544132,830730136,832002160,832032209,832693700,832694409,832792204,833119548,833874917,833892159,833920052,833920067,834401088,834816109,834848396,834955877,834968437,834986045,835504371,835742187,835743693,835819213,836171008,836577525,836604130,836604895,836623460,836653465,828484100,836129987,826585381,827465506,827805481,832732577,835633771,835634130,835644846,835770551,824697713,825622916,825644506,826304764,826305441,826306997,827000815,827009433,827036728,827211779,827418645,827644778,827655669,827661707,827661722,827661736,827684400,827684416,827685629,827685874,827685894,827685951,827686304,827686341,827686411,827686636,827686678,827686714,828864110,829098578,829539857,829539965,829542168,829542267,829542612,829542728,829542828,829542899,829542990,829638898,831204511,831204534,831204559,831204588,831204608,831204637,831204666,831204691,831204737,831204753,831204777,831204807,831204835,831204944,831205005,831637518,831996489,831996571,831997196,831997357,831997449,831997646,831997726,831997955,831998435,832005734,832005764,832005851,832005894,832005961,832006066,832662525,832662603,832969173,832969432,832969566,832969732,832969828,832969961,832970647,832983312,832983539,832989615,832989616,832990010,832990164,832990165,832990177,832990376,832990377,835982630,835982680,835982745,835982883,835983131,835983201,835983396,835983597,835983671,827877589,829858588,825537641,827970451,827150505,827205407,827205426,827205517,827781536,827781571,827931295,828200623,829064882,825142910,825142972,825142976,825142980,825143149,825143400,825143920,825144099,825144449,825144486,825144915,825146358,825146739,825146914,825146916,825147039,825147476,825147517,825147721,825147742,825147877,825148491,825149359,825149427,825150292,825150298,825150429,825150432,825150441,825154882,825156080,825157582,825169518,825453674,826125179,826125643,826139781,826139801,826140287,826140290,826141954,826144107,826148465,826148626,826149744,826153861,826153882,826155591,826155867,826165665,826168177,826168545,827024061,827028364,827030802,827032473,827213322,827213367,827213611,827213876,827214687,827217439,827217445,827217813,827217817,827219033,827220034,827220172,827220245,827220246,827220277,827220608,827220900,827222374,827222475,827222501,827222827,827225447,827225449,827225647,827225670,827226763,827226770,827228300,827228553,827228687,827228786,827229774,827231447,827234451,827234616,827236270,827250088,827250683,827250863,827251041,827255922,827256182,827260097,827262026,827265744,827440705,827441528,827442106,827442159,827442600,827442964,827443129,827449699,827472941,827478049,827486858,827487081,827493344,827496144,827515421,827515510,827518011,827526224,827530572,827531400,827535903,827536551,827536561,827545047,827550240,827880889,827993817,828040875,828042036,828042148,828043949,828047063,828048603,828276963,828277364,828277398,828277576,828278113,828279374,828281643,828281667,828291046,828291055,828291198,828291954,828292439,828295427,828295752,828299991,828300946,828301944,828302294,828307272,828319224,828319625,828320108,828320968,828322266,828325098,828325463,828325505,828333706,828333851,828336854,828340419,828340948,828341613,828346086,828346812,828352039,828352695,828359729,828360395,828376709,828377177,828377225,828388218,828389000,828798716,828798867,828798935,828799135,828799314,828799879,828800452,828800468,828800503,828801629,828802478,828802550,828803166,828804272,828804306,828804378,828804446,828804755,828805640,828805727,828805743,828806106,828806107,828806947,828811306,828812408,828813955,828815252,828994009,829090177,829404394,829404401,829404428,829404457,829404864,829617424,829673407,829900195,829900346,829901481,829901673,829902234,829902255,829902496,830390491,830390756,830390911,830390918,830390919,830391271,830392265,830393627,830397924,830401240,830401258,830410872,830413440,830413690,830417295,830422074,830423381,830423407,830429929,830429970,830435546,830732025,830907542,830907543,830910389,830910393,830910922,830910925,830912263,831869740,831869920,831870477,831870603,831870724,831872539,831872707,831872768,831873171,831874601,831874783,831874800,831874819,831874823,831876311,831878819,831878834,831879151,831879191,831879523,831879935,831880180,831880301,831880302,831880342,831882834,831883133,831883156,831884314,832782258,832783471,832783793,832783833,832787551,832791528,832791567,832796687,832800212,832888547,832888585,832891253,833458764,833463047,833464147,833464270,833464283,833464530,833464640,833464656,833464661,833464865,833467218,833467219,833467880,834026895,834028499,834030167,834031086,834031117,834430325,834431505,834431522,834436719,834436729,834445006,834464136,834465734,834465753,834468605,834470200,834470951,834471374,834475228,834477742,834606993,834608528,834608928,834614858,834616353,834616580,834616672,834619074,834619318,834621752,835176849,835176951,835616655,835620084,835633301,836415384,836494616,836495739,836498396,836498401,836498652,836499937,836500415,836502856,836504146,836506152,826507028,826729091,830646394,830892831,831546022,835082533,835082560,835082578,828930089,828935744,829139027,829139028,829518642,829534496,832804509,832847819,832945415,833061736,833217187,831866626,825485068,833972888,833993017,834382127,835258944,835267115,835632225,835724176,835724698,835724805,835724892,835725004,835725173,836095553,836095882,836095891,836095902,836099139,836100851,836100892,836100967,836101053,825154251,825155088,825155369,826150204,826486980,826670134,826670448,828833000,830405986,833930109,833932888,828558421,825288386,825296233,826774428,826922411,826937817,826956918,827879568,827932034,827934672,827946295,827951135,827958553,827968709,828000475,828034765,828416105,828418969,828431075,828439400,828453954,829110737,829125851,829638628,829651223,829655165,829661057,829668904,829728855,829940190,829947368,829971658,829983516,829985316,829990716,830002637,830013859,830028256,830414838,830416328,830428598,830440429,830445601,830452819,830459209,830508961,830511441,830515439,830521189,830526064,830529765,830541500,831017410,831021660,831026947,831471726,831474312,831475648,831480594,831483875,832054138,832076515,832386846,832392518,832430255,832977943,832981796,833003781,833030179,833154550,833167708,833205300,833228777,833257678,833546297,833782168,834228557,825273563,827033932,827375759,827391684,827577934,828021583,830303406,830505210,831075451,831234348,831339873,832090710,832156001,832188447,832786611,832802511,832803850,832805967,832843111,832844549,832845472,832852485,832853665,834796578,831565730,834342321,825681609,825706674,824863030,824677555,825193573,825665095,826694894,826699138,826702178,826703794,828485103,828501198,828520682,828528401,828533621,828560522,828581293,830402451,826192085,834426263,824923177,828261044,828434389,828894831,828948037,829114588,834750699,835980061,835980130,833803367,834269222,825004736,827144966,828142043,828718351,829037671,829103146,829793013,834609364,834609380,834609382,834609385,834610065,834610067,834610072,834610073,834610075,834610076,834610162,834610164,834610165,834610170,834610176,834610181,834610201,834610214,834610222,834610232,834610250,834610253,834610254,834610255,834610260,834610263,834610264,834610266,834610268,834610276,834610279,835657497,835661475,835663920,835675281,835677987,835796768,835796781,826439504,828283894,829462348,829469679,829489944,829493446,831170045,831197780,831415749,831416631,831420929,824685129,824752898,824767411,825152690,825153161,825166050,825166051,826236024,826605898,827212992,827414990,827513679,827560139,827776398,827902167,828100473,828101920,828185373,828574410,828703168,828891757,828896817,828903868,828903878,829361103,829410372,829411134,829483699,829483739,829537832,829554825,829649224,829649240,829649266,829649297,829649312,829649332,829738450,829958372,829958379,829958608,829962393,829966549,830064458,830304576,830315311,830326917,830350175,830851068,830938218,832207363,832325629,833491893,833816682,834544439,834911646,834911908,835140220,824526037,824631450,824631491,824688442,826427829,827526833,827527020,827527077,827527097,827527140,827826673,827836136,828057953,828058018,828059061,828059080,828059097,828059116,828059140,828059159,828194521,828216075,828360232,828365329,828485605,829276545,829708870,830066426,830071996,830080540,830127189,830159393,830507205,830507237,830508261,830508485,830889446,831061166,831061198,831365896,831566222,831671515,831696108,831738372,831888219,832032843,832049175,832050662,832059138,832060520,832094894,832098187,832117374,832151835,832152344,832152914,832153420,832153919,832154989,832155492,832156192,833361979,834258935,825280443,825280467,825280595,825280782,825280867,825280880,825280898,825280907,825596789,825596913,825654376,825657669,825657737,825657850,825658046,825659681,825660140,825660256,825660411,826441584,826441607,826441632,826441746,826441762,826442133,827237150,827257051,827257837,827258043,827258289,827258614,827258695,827258790,827258872,827259075,827272139,827272374,827295030,827297626,827298592,827338581,827574851,827588940,827589413,827899049,828284884,828284891,828284901,828284906,828284995,828285005,828285007,828285031,828294668,828296089,828296110,828323813,828355924,828358396,829261651,829269496,831424096,831866381,832322085,833423081,833447869,833449933,833450150,833455624,833635767,834176932,834176986,834403940,834897501,834997632,835012002,835091674,835137288,835170327,835180478,835181200,835182321,835182337,835415372,835415569,835600119,835600128,835616861,836030481,836037726,836057761,828290191,834011160,835418175,835418196,835418217,835418243,828291231,828341999,830402457,831858294,831955018,825019241,827383891,824549202,824960242,830387669,832805421,832821857,833425555,833799018,829957233,831913134,836654338,834686536,836462423,835643580,835644659,835071948,826107569,826108577,828292291,828338499,828339845,828341706,828342004,828347692,828347708,828351018,828826256,828835734,828836387,828861707,828874632,829513653,829651098,829692693,830494148,830513657,830815571,830877380,831963590,832319997,832319998,832320104,833419970,833439999,825957032,825957094,826718612,826784542,826785625,827305957,828079166,828946216,829153131,829153132,829527275,831213847,832651653,832742861,832820811,832820812,832854412,832963132,832963441,832977352,833155297,833239375,833255084,833325162,833327010,834182272,834861431,825141994,825421696,825422484,825438095,825673671,825675138,825731302,825743662,825744402,826654316,826658380,826659363,827230512,827230563,828296193,828296194,828825715,826191755,826392100,826550383,826983436,827212759,827264253,827264281,827280519,827318257,827323349,827481324,827488089,827590790,827637313,827873604,827885051,829088076,829318399,829684656,829684699,830043755,830704727,831703660,831933835,831951236,832579749,832959020,833123148,833415809,833415823,833694080,833694120,833960405,833989175,834106986,834521709,834552373,834609434,834645354,834955688,834994358,835072827,835072902,826491448,828188348,828218006,829217768,833555917,834049484,824496470,825032399,825033440,826274800,826480276,827186685,827423934,827915046,827950742,828206921,829266466,825161937,825181064,827898821,827899641,828288382,828288792,828288862,828289274,828289396,828289615,828289711,828290434,828290460,828290519,828291152,828291171,828291182,828291379,828291454,828291551,828291701,828292290,828304200,828316442,828324281,828324676,829423833,831764389,831797787,824573227,824573236,824573252,824573259,824573266,824573276,824573287,824573300,824635890,824636371,824648328,824717703,824914600,824927514,825680737,826165276,826381210,827587585,827674050,829132853,829256126,829272531,829562314,829562315,829592846,829995266,830551038,830559654,832455247,833502259,833512226,834398242,834721142,835466066,825336190,832796491,834317224,830758519,831494028,832056254,832074088,832074108,832074156,832100076,833121046,835169571,825028642,825995007,826990142,827913543,832606191,835110629,834458074,834458123,834950171,824481861,824920475,824929107,824939963,824953843,825491956,825505435,825506889,825521179,825977953,825982165,825986053,826366380,826466470,826499457,826731583,827097524,827434365,827533578,827560959,827620822,827644389,828101108,828404566,828433037,828477664,828484230,828490890,828671478,828814949,828814958,828827598,828827629,828828952,828833220,828835101,828848063,828864155,828924395,829875444,830153054,830157613,830412764,830427047,830441462,831472956,832198247,832559172,833530693,834478595,834478619,834479092,834479113,834996602,835013326,835040883,835955349,835962705,836500268,836645437,824591926,825262842,825625475,829880809,831366867,831700814,831834034,832071857,832568478,833972091,825408208,825452698,825788490,827267858,827381172,827484122,827717379,827878273,828042878,828056296,828075448,828436476,828631381,829220774,829220804,829220846,829255762,829954782,829955094,829966997,829974209,830074742,830077717,830083559,830975914,831130433,831131184,831163771,832653355,833024951,833159649,834165267,834245631,832282248,832283370,832283382,832283413,832283427,828022193,828023310,828023330,828023348,828023362,828023384,828023407,828023439,828023451,828023470,828023487,828023515,828023522,828023533,828023561,828023570,833315716,834126176,835819546,833658079,834104011,825304073,826398266,830519338,828821738,824650560,824650578,824650736,828936435,826339398,829761265,831295023,831751068,824772565,824862773,825986054,828589859,829535086,829646970,829665806,832259810,832259817,832448880,832494658,834198884,834209746,834506744,834834025,834834080,834834131,834834178,835115010,835509328,835509521,835509957,835510244,835510965,835794880,835794969,835795183,825314197,825519691,830393251,827317546,827362294,828566104,830911372,830911584,825446867,826093956,827546182,827611632,826364222,824957422,824968862,824507042,824513408,824523881,824559704,829849793,829849820,830096851,830251079,830322828,830334591,830336464,830352336,830846683,830850301,831447042,831836134,832252712,832252808,834050593,834050610,834050623,825320224,825331842,825371571,827507180,831168303,831198384,831481357,832112350,832208340,832208406,833827089,836014288,836631157,834572379,835067357,835067384,835067411,835067444,835067471,835067493,835067517,835067537,835067588,835067607,835067629,835067640,835067768,835067862,835068214,835068226,835068252,835068260,835068284,835068311,835068336,835068352,835068370,835068396,835068408,827228440,827554121,828280239,828280267,828280272,828280277,828280281,828280283,828366752,828366755,828366767,828366772,828366778,829996160,830115763,826719630,826719850,827714396,827817931,827817947,830254001,830254022,836452027,836452894,829578203,832772082,833882526,824994883,825210945,825210981,825211004,825211030,825211063,825211082,825211102,825211124,825211155,825211170,825211193,825211214,825211238,826112406,826128807,826137014,826675448,826675461,826782478,827234970,827234981,827234985,827234996,827235004,827235014,827235025,827235029,827323172,827323177,827323190,827323195,827323371,827637356,827637631,827640897,827640906,827640925,827640986,827641652,827717767,827979653,828023642,828023659,828034555,828034715,828034751,828034802,828034936,828034951,828034992,828071429,828144507,828200754,828200922,828201380,829376134,829386342,829386345,833956007,825997497,826508763,828163493,829019074,829023189,829086916,829095164,829260918,829619400,829628198,829651224,826292013,826301022,826339495,829619082,829664854,832540319,827672805,827673427,828666826,828976666,829415246,829600128,829657009,829679152,829698981,829699003,829699040,829699063,829716635,829716654,829805011,831564799,834242098,834477796,834525832,834555932,834567603,836449114,830199916,831653351,832811149,835387832,835955276,824664116,824664149,825158337,825198011,825198041,825198049,825198063,825198096,825198109,825198136,825198521,825198532,825198546,825198553,825198559,825198565,826343626,826713636,826713642,827163321,827807526,832396784,833624074,833624094,833624116,833688472,833688573,833688773,833890459,833890476,833890495,834503954,834543970,836099728,824662193,824678592,824681960,824686806,824728023,824732483,824734286,825203133,826197156,826266090,826293173,826702180,827253764,827263338,828850168,828875816,828878203,828892448,828902137,831123927,833512759,833512766,833512783,833512796,833513314,833515090,827460605,828717714,828717716,827906282,831256743,831256780,833477472,831896949,831896958,831896972,832330835,830031933,830465994,830468254,831227002,831232048,831237555,831803479,831805326,831805351,831955662,831965383,832104231,832116458,832118426,832125738,833577049,833591670,834808939,834813604,834815424,834816282,834816284,828149959,828151939,828935939,829557084,835136931,835137263,835137530,835138126,834762379,836019344,836106134,826142047,829555260,829779987,832358661,833847359,825054003,825208352,825211523,825616129,827145328,827224055,827294863,828207411,828868065,830665034,830723772,830725194,830725234,830725255,830725286,831315128,831315168,831315191,832010477,834571725,834576296,834738610,834752101,834783137,834843582,835813025,835813038,835813062,835813109,835863403,835869727,835921431,824507929,824507963,824507988,824508001,824508016,824650492,824650495,824650987,824651000,824651005,824651033,824651058,824651076,824651088,824651099,824651115,824651120,824651135,824651148,824651161,824651175,824651194,824651214,824651225,824651324,826003065,826003093,826003138,826003260,826003349,826003366,826003488,826003528,826003547,826003566,826003588,826003636,826003647,827253548,827253565,827253582,827253616,827253638,827253664,827253703,827253741,827253838,827253868,828498758,828498798,828498827,828498844,828498869,828498957,828498989,828499015,828499036,828499070,828499108,828499137,828499160,831242702,831242741,831242770,831242801,831242840,831242853,831242878,831242889,831242913,831242940,831242955,834779174,834779205,834779228,834779249,834779324,834779341,834779360,834779408,836242072,827418250,828745846,828746998,828747012,828935299,829538343,829538355,829538372,829538385,829551726,829967650,829968975,829969043,829969092,832097492,832147946,832189013,832189101,832189261,832333939,832333949,832333971,832333981,833224304,833227252,833227282,833588814,833589846,825971658,829497479,830059270,831626980,824860172,827089893,827391651,827909967,829501166,829959587,835269552,827002401,828759557,830340816,832782570,831819856,833428417,833428632,833432094,825623713,825794410,826289428,826702601,826706004,827666203,830505128,824854933,825336102,825494651,825513759,825795370,825826423,826030984,829985416,831218110,831455977,831490561,831571007,831845664,831966520,834584238,835314889,835348375,835354160,829396634,827204306,832764372,827783513,833806943,824710286,830008852,830846602,830973624,831046970,831950848,835563677,836078787,824777567,827308252,827310652,827314301,827316228,827316249,827320066,827360067,829578722,829603723,832306399,832599427,835524746,831934003,831942270,828520132,828955023,829036394,830959681,831054763,833724064,829506292,830810596,832104317,832104706,832105185,832543468,835977545,835977556,835977569,835977835,835977860,835977874,835977889,836190348,824583163,826192087,826491697,826568117,827205142,827205144,827205163,827205178,827218238,829916234,830439782,834038298,829318870,833274536,833288542,835045522,824929334,825425906,825453534,827724095,828045938,828046095,828046128,828046376,828046378,828046380,828046381,828046387,829197806,829197811,829197817,829197833,832177007,832208434,832284582,836101606,836101609,836101655,825255434,826369465,828729295,830856597,830861753,830966501,830981635,833708211,833717276,833717304,828512761,831314347,831314348,831836267,832809526,832809533,832809578,832886542,834305547,834305556,834310350,835344366,835347161,825660560,825660592,829224549,828514529,825890637,828344182,828351868,828356487,828361884,828369650,828380201,828526648,828554305,828997806,830996424,834810350,834871065,835246562,835249816,835669286,835670332,835675845,835678694,835679896,830904678,831418613,831958298,832028408,832712643,832712667,832713303,833302991,833730753,834350403,834364124,825281302,825281317,825281333,825281346,829244103,829261073,829266948,824833456,825176889,825273846,825975092,826132199,826150266,826236017,826634142,826647271,826649516,826649561,826649607,826650502,826653728,826817994,828072015,828072169,828072237,828306934,828354254,828361400,828598807,829182987,829338661,829655483,829735859,829735861,829735862,829765023,829872129,830091071,830108309,830108436,831780435,832430889,835445953,835446300,835471193,835471195,835944174,832659498,832660790,833820127,833829859,833831888,833838477,824720302,824756595,825176949,825181814,825631843,825773206,826860132,827224990,827224992,827225000,827225012,827225026,827225030,827225146,827225155,827225156,827295025,827299222,827347024,827907654,827963225,828200082,828215343,828815389,828815403,828815649,828815653,829370048,829380781,829605724,829606987,829619083,829630406,829635501,829879391,829879403,829879410,829879417,829879418,829879430,829879576,829879580,829879590,829915217,829915219,829987400,829993376,829996086,830043056,830759110,830759267,831131824,831202483,831220056,831223581,831247349,831261351,831279451,831283298,831312696,831314941,831471727,831575056,832130594,834823709,835857893,832117443,832360997,832372644,832378398,832645372,832947259,832952196,832955770,832966374,832980903,832982039,832984440,832988441,832991419,833505634,833506534,833506597,833508374,833509160,833509511,833516220,834059944,834067417,834279486,834287363,834287842,834288166,834298706,834319177,835196619,835196785,835196791,835197433,835197543,835197733,835199133,835199401,835200594,835201913,835202501,835202512,835203443,835666189,835666190,835666508,835667641,835685079,835685452,835685542,835687061,835687215,835688966,835690512,835690513,835690515,831455206,831455210,831455213,832731387,830557237,828268067,832355906,831215154,835998647,836103804,824762200,825086384,826061714,827543843,829138743,829717990,829860450,829930091,830333187,830501203,830789656,831287835,831446866,831446892,831446901,831446912,831446919,831715880,831833697,831836855,831839949,831841801,831842069,831842559,831843246,831843289,831843339,831843387,832603649,832604913,832825065,832837525,833000734,833047165,833212926,833212950,833320796,833359821,833472536,833892301,833892320,833892384,833892415,833892435,833892478,833892501,835093057,835219706,835280824,835281788,835282118,835860318,835969210,835995720,836040682,836040684,836040686,836052077,836052339,836052659,836052663,836052666,836100440,836278176,836278192,836278215,836278224,836278235,836278266,836278289,836417203,836537378,827290655,833844730,833847392,828579788,827335795,831473179,831525687,831525729,831525803,831525813,831525827,832523062,825520137,828393720,828834683,832797814,832806146,834071143,833761233,836280001,836280026,836280179,836642051,836642076,836642082,836642100,824711201,825112408,825113449,825140886,825142036,825143604,825145425,825148125,825149045,826129653,826130507,824977361,826024923,826024932,826024958,826290720,826727145,826727160,826727195,826740740,830004079,830043057,830101089,830262202,832442189,834848900,834855731,834855751,834855770,834858783,834859737,834859766,835275002,835282576,835287475,835293826,835297831,835339538,835339545,835339666,835447085,835718154,835730690,835748527,835748638,835748715,835748780,835748852,835748937,835748991,835749145,835749250,835749346,835749481,835749625,835749748,835773303,835773514,835886141,834162521,834342504,835004964,835005301,835005317,835005341,835018764,835026780,835256440,835262578,835489895,836051522,836051550,836051552,836051556,836051557,836051562,836051572,836051579,836051588,836051617,836051634,836053022,836053866,836053873,836053888,836087967,836088276,836088296,836088304,836088310,836088322,836088350,836088360,836088562,827453307,827453451,827453610,825495943,826054216,826423628,826424493,827181825,828124814,828666295,829765948,835211468,835211806,836513418,824855690,825394970,826037768,826651184,827458143,828604794,829090683,829090707,829090730,829090752,829964389,832928145,833979779,835456204,827919479,828365310,829014437,829983519,832534769,832969175,825146427,825155006,825955957,826165593,827235589,827254984,827529503,828310938,828812245,830422770,825531561,825681729,825684979,825690627,825700889,825740469,825773207,825782167,825807760,826240766,826311164,826699140,826708554,826712721,826715258,826804076,826818028,826826196,826832173,827055144,827061455,830107402,831701057,827126171,827890443,829342438,829827131,830717884,830722611,830835478,830835686,830835767,835078468,835078478,835086213,835098256,835214957,835219957,835222370,835239064,835314905,828754652,825033382,834248729,834427190,831757367,834384637,834384698,833505234,827775284,829440368,832286712,830162133,830162236,830162266,831205377,831205394,831205436,831205456,834792016,834793305,834793325,834793339,834793385,824755364,824960905,825256978,827342177,827342200,827342241,827342261,835067315,835067334,835140195,835638574,825616187,828238451,828278803,828796724,829878929,829946947,831839583,831873064,829617189,830696871,827454978,827893317,828142844,825413765,827676583,825612671,832662673,830957298,824854887,824854914,824854926,825250596,825250609,825335837,825335935,825335949,825336007,826093814,826094785,827731803,827954647,827969365,827969771,827985474,828004713,828170520,828173721,828187068,828904136,828904149,828904197,828904221,828904238,828904264,828912536,828970999,828974156,829143640,829267225,829699620,829975565,829975579,829975592,829975769,829975778,830035565,830053605,830057546,830098575,830576812,830577021,830577053,830577121,830577167,830577177,830577190,830577327,830577342,830577351,830577394,831220638,831220665,831220676,831224982,831232277,831249244,831748983,831749138,831749148,832123781,832123802,832124119,832124188,832141218,832288115,833077525,833137738,833736219,833742168,833744931,833748873,833755767,833825393,833835852,835355713,835355734,835355753,825557801,827902787,829168502,829326102,831676182,831999258,832006178,832011415,832071717,832078768,832093931,832121216,832121266,832131870,832146029,832147811,832285902,832307610,832645402,832786257,832823420,832856821,833215634,834966666,835169521,835449084,835449232,836481322,824864439,827994181,828492810,828497945,828567461,829037357,829062346,829093796,829096597,829238621,829250202,829298414,829299183,830066570,830297439,830298520,830301313,830324317,830558272,830559199,830560803,833296443,834782739,835323002,832262750,832262903,832339373,832745227,833228544,833228560,833229191,833996489,825694269,828828871,832081969,832082280,832082370,832082815,835456001,829553443,830518291,831121757,831290804,828935983,828951114,829019993,829028319,830700260,831099526,831202514,831202533,833842162,833842643,833845960,833845969,833845985,833846087,833846111,833846115,833846121,833846305,833846318,833846322,833846327,833846329,833846373,833846455,833846457,833846468,833846473,833846480,833846484,833846489,833846497,833846503,833846509,833846515,833846521,833850878,833850979,833850987,833850990,833850998,833851004,833851009,833851016,833851023,833851025,833851027,833851036,833851043,833851046,833851050,833851055,833851060,833851068,833851077,833855044,833855916,833858327,833858331,833858337,833858341,833858351,833858360,833858369,833858373,833858382,833858387,833858396,833858401,833858405,833858414,833858418,833858425,833858427,833858441,833858452,833858459,833858536,833858543,833858545,833858549,833858551,833858558,833858562,833858566,833858573,833858869,833858896,833858913,833858925,833858931,833858940,833969033,833969074,833969125,834641972,834642064,834642073,834642077,834642380,834642384,834644925,834644970,834644975,834644982,834644987,834644995,834644997,834644999,834645000,834645001,834645005,834645012,834645016,834645021,834645028,834645042,834645045,834645051,834645058,834645067,834645346,834645395,834645400,834645401,834645405,834645409,834645415,834645417,834645422,834645424,834645428,834645434,834646035,834646040,834646045,834646057,834646066,834646075,835053201,835081915,835082444,835209594,835209596,835209963,835210104,835210106,835210108,835210119,835210120,835210121,835210122,835210125,835210128,835210131,835210138,835210140,835210141,835210143,835210147,835210152,835210153,835210158,835210162,835210169,835210172,835210180,835210185,835210186,835211356,835211359,835211360,835211361,835211363,835211367,835211374,835211375,835211583,835211586,835211589,835211590,835211592,835211593,835211601,835211604,835211615,835211616,835211621,835211623,835211629,835211636,835211637,835211638,835516774,835516788,836082594,836082900,836082903,836082904,836082906,836082914,836082921,836082925,836082930,836082937,836082939,836082944,836082947,836082952,836082957,836082963,836082965,836082967,836082971,836082981,836082994,836082995,836082998,836083000,836083001,836083094,836083126,836083131,836083135,836083139,836083611,836083665,836083667,836083670,836083674,836083677,836083680,836083682,836083687,836083692,836083695,836083698,836083701,836083702,836083704,836083708,836083716,836083938,836083973,836083974,836083975,836083978,836083982,825808823,825808925,827642950,829635898,830320808,830320810,835107334,835107364,824970722,824970758,824970798,824970818,825289166,825289180,825289227,826336276,826336319,826336344,826336386,833259746,835335339,835344238,835398538,826144371,830250173,824636572,825225683,825702065,828346952,831931999,835208883,835580429,826604843,825924029,826928069,830247257,834215643,835346729,835360655,826967736,827441509,827907376,829112034,829629502,834643676,825252699,825252712,825252739,825285069,825369134,825517031,825518890,825521227,825521795,825699672,825772351,825772857,825844991,825881469,826175466,826179756,826192529,826195046,826216782,826286812,826473165,827370775,827576970,827642807,827876615,828978786,828979187,829043693,829123210,829123251,829123295,829710384,829710898,829958288,829958318,829958371,830335472,830335487,830335512,830335541,830335564,830335594,830335628,830616310,831165072,831447231,831742118,832099012,826196655,828682645,828779060,830279553,836580912,828069379,831202518,832141259,832941770,833581804,833983281,834317103,834317120,834317135,835015775,836080220,836084345,836084495,831103397,831127057,831341983,825522252,831219331,831282004,825205338,825205657,827084276,830064240,832454864,834113915,830687841,832461411,833880536,833881932,833900248,834897201,834991206,826034150,826567500,826604766,827652525,831187513,831191546,831191592,829841927,829845725,829852036,829862900,829864602,829865172,827239662,828635210,824650851,825235337,825235364,825247931,828238116,824862237,824863397,824863759,824863795,827836986,828262960,828279904,828629456,834397804,834598494,834665083,834861177,825254286,826567502,826604767,830783904,830820510,830827175,830829069,830831797,833982330,827164050,829337831,829525378,831565271,831565590,831565610,831565852,831672502,831675222,831675238,829593689,830867266,830867283,833960050,834382561,834702699,834702759,835068693,835365297,835401727,835540517,835581966,835660579,835705486,835745108,835754811,835765125,835899059,835910946,835911918,835937671,835944857,835953442,835973909,835978612,836064123,836066860,836066978,836067456,836072807,836074294,836077035,836080627,836083737,836085487,836087786,836092377,836092386,836099401,836099415,836099528,836099533,836099560,836099575,832466859,836631056,836631069,836631107,834867369,834924141,834928295,834589102,834589105,834589108,834589110,826570423,827749787,827749933,827749943,827749978,827794249,829043725,829122797,829122820,829122834,829122855,829122886,829122912,829122952,829122992,829123012,829459903,829480885,830281313,831045819,831045863,831191783,835428045,835428340,835675991,835675999,835676014,836118456,836118482,836118516,836118538,836122114,836122379,836122397,836122419,836122433,836122446,832292414,824662792,824663607,824668125,824669996,824670030,824670087,824672153,824672174,824672184,824672209,825175451,825529118,825529145,825529199,825529227,825529282,825529622,825529665,825529690,825529743,825529783,825529815,825529832,825653151,825653156,825655044,825891589,825891603,826151083,826151093,826151124,826151134,826151145,826178663,826197522,826567213,826685585,826685601,827194517,827565998,827566084,827587495,827647595,827674798,827674817,827675157,827675473,827675488,827675500,827675672,827675706,827675743,827675797,827675831,827676362,827676427,827676476,827676500,827676532,827676581,827925502,827925927,827925953,827926047,827926072,827926097,827926301,828178520,828197761,828707172,828707208,828707257,828707291,828707327,828719245,828866531,828867268,828867530,828868090,828868122,828868137,828869479,828869517,828869559,828869595,828869614,829068182,829437374,829595959,829757924,829757976,829757986,829758001,829758050,829758059,829758104,829758178,829759092,829759303,829885498,829910480,829934622,829970412,829970426,830214112,830214360,830214417,830214480,830214556,830214625,830215399,830215415,830215431,830215452,830215496,830215514,830215529,830216010,830216035,830216052,830216074,830216086,830216103,830216173,830430552,830430563,830430579,830430600,830430623,830430646,830431145,830431179,830431580,830431588,830431622,830431704,831072032,831145743,831147693,831222131,831222661,831222711,831446623,831446629,831446638,831446644,831446648,831446653,831446658,831446758,831446767,831446772,831446775,831446778,831446781,831447026,831447033,831447048,831447053,831447139,831448134,831448157,832667985,832668025,832668059,832671968,832671987,832672001,832672761,832672775,832673118,832677130,832685102,832685565,832685600,832685740,832685767,832685795,832685819,832685841,832685873,832686872,832686893,832900954,832900966,832901136,832901147,832901154,832902761,833345270,833592806,833602145,833602166,833602184,833602201,833796870,833845579,833845914,833845917,833845930,833845939,833845948,833969992,833970018,833974055,833997784,834050356,834050366,834050373,834050380,834050387,834791934,834791947,834791987,834792033,834792054,834792076,834792091,834792134,834792197,834792584,834792594,834792617,834792655,834792667,834792678,834792690,834792718,834865891,834937921,834971677,834971701,835012246,835119557,835119583,835119611,835134662,835135738,835142563,835200801,825063944,825129693,825129704,825129712,825315528,833472783,834583349,834872861,825403942,826336379,827963683,828726792,833752551,834089509,834232071,834254843,834254968,835123607,835194973,835195003,835195011,835195028,835528603,835556806,835936781,835938385,835951851,835953203,830761902,836222611,836222624,836222637,829139217,829426922,830779690,831498845,831498940,833991722,834258962,834559969,834821792,834821828,834821843,834913140,824500683,824509115,824509161,824529385,824529395,824529400,825345531,825716489,825754203,825921848,826436453,826444529,826446694,826453071,826663651,826710177,826715213,826731314,826823572,826948516,827505074,827506362,827622691,827771380,827794491,827796310,828217590,828220396,828224695,828359978,828587873,828827713,828933615,830035450,825527719,832790049,827291954,827622281,827911096,828053515,829102648,829271094,829660948,829840520,829840534,829840543,830237299,830646442,831668655,831698350,831698376,831698400,831698435,831698453,831698468,833981284,834599460,834620991,834686756,834738542,834738546,834768074,834773346,834775583,836123958,836407619,826100232,831117411,831394842,831423874,831938298,831976643,832084083,832879324,832887520,833557385,830135878,824763622,824968496,826399798,831088808,824538202,824926418,824926783,824927022,824933125,824935046,824949235,825017568,825017671,825110306,829712662,831194921,832241487,832743640,833719279,833719468,833732687,833747263,833753532,833754320,834180342,836007965,833728945,826491009,826491029,826492271,826775319,826775370,826775395,826775415,826775441,826775456,827013047,827020073,832146992,832520699,834741260,834908359,835104957,835510722,835511073,835511632,835525774,835537620,835539310,835549775,835551586,835553081,835556987,835681435,835682436,835685584,835687172,835688169,835690577,835690767,835899001,835911137,835911959,826453184,826453208,826453242,826453345,826453375,827653164,827844519,827847058,829359959,829359974,829359987,829360008,829360018,829360039,829360247,829556239,829564936,830331264,824505888,824505928,831527553,831527566,827680031,827680057,829933892,829723119,829723132,829723154,829723164,824493694,825122350,825205086,825205102,825234191,825234202,825234216,825234240,825274660,825285038,825313811,825313831,825314176,825314210,828415278,828415330,828415359,828415386,828415406,828415565,828415589,830079867,830354670,830363879,830363926,830613978,830614579,830734521,830735204,830914842,830920506,830920520,826187098,826400375,826796637,826814181,826875394,827568517,828774916,828780580,833796386,833796423,833888247,833897738,833899379,833905044,833966600,835577017,825957408,826858431,826858594,826859945,826860248,826861483,826882909,828191877,828197562,829167532,829168767,829169888,829187826,829519058,829519060,836382860,829983521,830695148,830751609,830783905,830829071,830831798,832652048,834130862,834130874,834130904,834130929,834130962,834130981,834130999,834131044,834131149,834131158,834840723,835246262,835246274,835246282,835246296,835246308,835246332,835246344,835246352,835246355,835246366,835246471,835246475,835246483,835246488,835246503,835246509,835246520,835246528,835246537,836082091,836082431,825342610,825342755,827602872,832177473,832194830,832553206,832553283,832553330,832553675,832553850,832554535,832554590,832554652,832554720,832554752,832555046,832555111,832555532,832555591,826262157,826264241,830496246,825491732,828438690,827763411,829899833,830896228,831426517,835602274,825788088,828312886,834643774,834643812,835055060,835063300,829052687,829053975,829054151,829062341,829063885,829080930,829082194,829086467,829226561,829305512,829306052,829308628,829324100,829325244,829332361,829366448,829366731,829367557,829369766,829371968,829372184,829372321,829372761,829393982,829394427,829394444,829395093,829395155,829395397,829395399,829395503,829397008,829397859,829398682,829448184,829448953,829449236,829449237,829450119,829450896,829453227,829713818,829772526,829772643,830295637,830502772,830504760,830505769,830506314,830510122,830510467,830523877,830661201,830673018,830700421,830796045,830853856,830856082,831792959,831852606,833556294,833581317,825145439,825145443,825961365,826028884,826029497,826029520,826029535,828066165,829655846,829656611,829656652,829656667,829656681,829656700,829657024,829657039,829657061,829657079,829657119,830857080,830857092,824514429,824516663,824518143,824519303,824519898,824532861,824534779,824549811,824550694,824552794,826646149,826648295,827668528,827671727,827672010,828631728,828743660,830661101,830661138,830688829,830689438,830694398,830700975,830769270,830769566,830770411,830778333,830794599,830848989,831199446,831204366,831393628,831398006,831399678,831404821,831819883,831824220,831833933,831836745,832694272,834451245,835452636,836306299,830377814,830377815,830377816,830377967,830378226,830378768,830380011,826282754,826283179,826283794,826284324,826284924,834017919,834783448,827546537,829408068,824919555,825353370,826507064,827357322,827983399,832202793,834430022,834458664,834462531,834462532,834718967,834725889,829314062,833020073,833068128,831998219,824645330,824657194,824660526,824665140,824676318,825701384,825708663,825714046,825719227,825724512,826686992,826697888,826699610,826709385,826722597,827243446,827245255,827248615,827250766,827293299,827308594,827324193,827334779,827347401,827354405,827394971,827402912,827521945,827525865,827540787,827860404,827871502,827874734,827883307,829051921,829057200,829073562,829081957,829085560,829154328,829442084,829523019,829551588,829587907,829598326,829922884,830062446,830070752,830096854,830623163,830629316,830785924,830787953,830794710,830936657,830939054,830954475,830957501,830968680,834114193,834130321,835642846,835644818,835647109,835668188,836223116,836228129,825686755,825687576,825688297,825688457,825688641,825693961,825708640,825712564,826199386,826207998,826242270,826242310,826244828,826244829,828065061,828066691,828073746,831019180,831145938,831511610,828369652,828376651,829730342,829983032,833401756,835202141,835202172,835204158,835206588,835973199,825656087,825754278,825763342,825766270,825768823,825876034,826468821,826769419,826945415,826952243,826956781,826973477,826990145,827097525,827110772,827428543,827431453,827434303,827436852,827446758,827472175,827532651,828063710,828080779,828570160,828578607,828586147,828675109,828683987,828688792,829538761,829551216,830181743,828110445,828180461,828185464,830010170,830964049,831173754,833378510,833381530,833538576,833538592,824579921,824586972,824593565,824595772,824599342,824599344,824599601,824599622,824599631,824599634,824599636,824599642,824599890,824599893,824599897,824599906,825121654,825126889,825128817,825210207,825264681,825264701,825264717,825264743,825267669,825274019,825276361,825323067,825611534,825626121,825692299,825692318,825692338,825692363,825692382,825692418,825692438,825692456,825693012,825693487,825693505,825693519,825693536,825693557,826106338,826781551,826781584,826781613,826781627,826781650,826782365,826782392,826782425,826782475,826782500,826782517,826782582,826783069,827545669,827545688,827545733,827546467,827546495,827546567,827546936,828278464,828282347,828282879,828283271,828283273,828364950,828383962,828795393,828798237,828918079,828921326,828969437,829437456,829873451,829883145,829947158,829947172,829947184,829947198,829947209,829947715,829947724,830578484,830596570,830865544,830865643,830865883,830879287,830880294,830880301,830880307,830880315,830880326,830880619,830880668,830880685,831424088,831424148,831424685,831431088,831552386,831834992,831872306,831873281,832006958,832006968,832007013,832020352,832033460,832041886,832064640,832499649,832549404,832567997,833366971,833369414,833388779,833406965,833573108,833573141,833573172,833573267,833574429,833974012,834257433,834257476,835598367,836015500,831905260,824633171,824637273,824644551,824649529,824650420,824656919,824665538,825225681,825234505,825245190,825376578,825411013,825666581,825672586,826745591,827257824,827261962,827263527,827360057,827366193,827822734,827827604,827834704,827840989,827847385,827925494,827934210,828343093,828349611,828357299,828365955,828375859,828399854,828532822,829443435,829554578,829561390,830072339,831923007,831924445,831929678,831931944,831936186,831943797,832922892,832926381,832928358,832937039,832941294,833555784,833571209,833582562,833589914,833609112,834059373,834065585,834071698,834082190,834659305,834668059,834678568,835217920,835221637,835227719,835233972,835662158,835667942,835672202,835674331,835675127,835680437,825377780,828531046,829063544,829361016,832463701,832473440,832474828,833191006,833456989,833457096,827940886,832174346,832181317,832279139,829351022,829580328,830071444,830423078,830600755,830701099,830701139,830701775,830736681,830763123,830992403,830992456,831013963,831014001,831406454,831406470,826654257,826654264,826654267,826654271,833209760,834752034,824574648,826141233,826279978,827706607,829259937,829348841,829348919,831220403,831702548,830006365,826837463,828433665,830399011,832952099,834296350,834317021,835217331,835908290,835919933,824707542,824725046,824740974,824882217,824887082,825887946,825887999,830953265,834981439,835160709,835166411,835174970,825411811,827251369,836174434,834557121,835506624,830043089,830051660,830070753,830084291,831567052,831577002,831585243,829354624,831028514,831446131,831448505,825621310,826360805,827605655,827605665,827605697,827605745,827605952,827605998,828585081,828585101,828585111,829891221,832680825,825639745,825639792,825671042,825678265,827137035,827137900,827998632,827998670,831456234,831534923,835487213,835557339,835566191,835573944,835584175,835936824,835938377,835938402,835938420,835938431,835938441,835938827,835939237,835939264,835939295,835939327,825532551,828516351,832214322,833017715,835673214,828758396,831470488,833098016,833494620,824793702,824793704,825324030,830204138,835115503,826212905,828786600,825274100,825274137,825274172,825358808,825363454,826225080,826227646,828262861,828267485,825446926,825683502,827590582,824915388,824915524,824915608,826309051,826309087,827172262,827174153,828385355,828559724,828735357,828891814,829040148,829866845,829958446,830296073,831833695,831837179,832281797,832282132,832285585,824748799,825068290,825082632,826500979,829845917,829858590,830033021,830042013,830066651,830334594,830336465,830341122,832066419,834913376,835799704,830558912,830560895,824787567,825166895,826068994,826515904,826684155,826981682,827472135,827477990,827944072,828448588,829355460,830330928,830519464,830545420,830703833,830703879,830703897,830835586,830885034,830885381,831034323,831034622,831034641,831034693,831034714,831034751,831034778,831034829,831034859,831267443,831400290,832231324,832236262,832774934,833012808,833015132,833406732,833439419,834359062,825281536,825281604,832339536,832339668,832345180,824672841,828004044,828004073,828004318,828004342,828004355,828004366,828004390,828004400,828004418,828004676,828004701,828004716,828406125,828782682,831805890,831856326,831856331,831856336,831856404,831856408,831856413,832102472,832117287,832129847,832132329,827880914,832765549,833275482,825263162,826034254,826059027,829355925,834167488,826063990,826382453,826473411,826757776,827037410,827173101,828661634,828989993,829266989,829780213,830720586,831837847,832207611,832793829,832795803,833270326,833279052,833302507,833323297,833472246,833843005,833932550,834100077,834135693,834136549,834155402,834236225,834239384,834246598,834259732,834313385,834322743,835042419,835042721,835043576,835043641,835088103,835089109,835091532,835190944,835196244,835196989,835205568,835208463,835269407,835911599,836356804,836406325,836637053,833681778,833850662,833858978,833859021,833859045,833859054,833859305,833859314,833859323,833859326,833859328,833859334,833859341,833859345,833859348,833859350,833859354,833859360,833893430,829876398,833661126,825622512,825792665,830128556,824875496,825300739,826450813,826574520,826588263,826660027,826660028,827113306,827115520,827115540,827115555,827116377,827116391,827367683,827628017,827628031,827628038,827628056,827628064,827628072,827628084,827628092,827628109,827628126,827628142,827628159,827628172,827628212,827628228,827628240,827628253,827628282,829958398,829958445,829958490,830032853,832714750,832714795,834890306,834890428,828435577,830947257,831279114,831427225,831459760,831513982,831565436,831565517,831565618,831601081,831691825,831973212,833233167,826026168,826071046,826074871,826074941,826369679,826836115,832278428,832285414,832299607,832302776,832309507,832503147,833372723,833533309,835129472,835307717,835480529,836077747,836078838,836079027,836082211,836092553,825324908,832346212,826995076,827620706,825702253,827563482,827880644,829196933,831544666,831660270,831739662,831745627,832117533,833823351,834314642,836713433,829869910,833587272,828141362,824524874,825221828,825702701,826164633,826548445,826722557,826751945,826752742,826753064,826759884,827135472,827177821,827255696,827325294,827328770,827329327,827405165,827408610,827408634,827421784,827422218,827437744,827953549,827953879,828071513,828071598,828073247,828078749,828078793,828703632,828915769,828923383,829913879,829917647,830659984,831480383,833290988,833381597,834786260,829767171,830452480,830452697,830649452,830649464,830649473,830649484,824651859,824681604,824734086,824956122,824961212,825071298,824581151,825224199,825388773,828376428,828376453,828376479,828376543,829040300,829042370,833146492,826314649,826654247,826654250,829595644,831446873,831446879,833386991,835593572,825248032,827878049,828293350,828295336,828296240,829954894,830037872,831440342,831493205,832063835,832122766,832137620,833349081,834616594,834616650,836506641,836506669,829073415,826371277,827098467,827522532,829678147,829678661,831642532,832132047,833127014,833130656,833412995,833413038,833413073,833413118,833779538,833779812,833779969,833780007,833780053,833936766,833941736,833978507,833978872,833980327,833984202,833986168,834331493,834417579,834418795,835724777,826496069,826785899,826826850,826978794,827050179,827054824,827055024,827073854,827082114,827082763,827082769,827082953,827085170,827126876,828571663,832025163,834887267,835008956,835012724,835118877,835118909,835136605,836099594,832113672,824591715,824591725,824591798,824591822,824591962,824591971,824593370,824593828,824594084,824596401,824596403,824596742,824596743,824596814,824622149,824625362,824627582,824629250,824630017,824630525,824639937,824652640,825710282,825710363,825713564,825723609,825724266,825724654,825729971,826360279,828321012,828323909,828325964,828358115,828393194,828560039,829949896,831928834,826043976,829463821,832020452,833349944,833350232,835512218,835791428,835922270,835923348,833151881,833696578,826105800,826628169,826628550,828328173,828340580,828349856,828399859,828403804,830060390,825744884,827564533,829438999,836526458,836526981,825642503,825642507,835635601,826923520,834419007,834424399,834444248,834563472,834563473,834568061,834577096,834577541,834577586,834583856,834587514,834805465,834806393,835084529,835148478,835747665,835824122,835830782,830393382,831222251,831847232,830236169,830291970,832522762,833348922,833397883,833397891,833397912,833785295,834054057,834073517,836012239,830200518,824481548,825287340,830447365,830562918,830859038,832337634,834116769,834332258,834486676,834498860,834801977,834815842,834828647,834875586,834911610,824507952,824669813,824724102,825234769,825293381,825311284,825452821,825467164,825467194,825882703,826136613,826156437,826167976,826175154,826175455,826194666,826211613,826285591,826357110,828141214,830671420,830942290,831750699,832346671,832985136,835074767,836833056,826215305,826834379,827879723,829577466,831239080,835164668,825249369,825252187,825255187,825255284,825255322,825255338,825255362,825255402,825255420,825255443,825255573,825255604,825255629,825255659,825255687,825255840,825255861,825255880,827684779,824710106,829657100,830857045,830894130,830934084,830934115,830934142,827253766,827465459,827814061,829476581,830013861,830297760,830301709,830315748,833640485,835905287,825812542,825925730,826440880,826984804,827832269,830010090,830852194,834505472,834580476,834937639,835195090,835215570,835273511,835313765,835331054,835361732,835503389,835645138,835662450,835665105,835738570,835836830,836074824,836087151,826073364,827204856,827254050,828790364,829015579,830681072,830687420,831465098,832465727,832669590,825727035,825729207,826402922,826647585,826647923,826647990,826647996,826742158,827237139,827435524,827435941,827604539,827615646,827789406,827790746,827791796,827791814,827879296,827920734,828155919,828193499,828193592,828252567,828361328,828361339,828498104,828520601,828562846,828639398,828646151,828704777,828884302,828892209,828907931,828914353,828914380,828954922,828956204,829006611,829006792,829027029,829064575,829516351,829637137,829637197,829637232,829637261,829637280,829637301,829637333,829637363,829637382,829637408,829637424,829637453,829637480,829637511,829637544,829637568,829637596,829637609,829637629,829637650,829637699,829637728,829637748,829637761,829637799,829644751,829698910,829698921,829699018,829841131,829841166,829841204,829841379,829841411,829856138,829856151,829856187,829856339,830015257,835250315,835656493,835685628,825796528,826700627,826700690,826700696,827052932,827052940,827115671,827254616,827386186,827386223,830235928,825581771,828070556,824996342,824996360,825010858,825385329,825518886,828365886,828430785,828534893,828617472,828973736,828979574,828979576,829042832,829169886,829679613,829679909,829813760,829988383,830428204,830527471,830839847,831219819,831223404,831238578,831245239,831325232,831325246,831325272,831325287,831325301,831325475,831325496,831325530,831325559,831325581,831325732,831325757,831325775,831325788,831325809,831325820,831325950,831325977,831326009,831326028,831326057,831326088,831326327,831326347,831327575,831327597,831327625,831328012,831328332,831328454,831329018,831329905,831330577,831335680,831338098,831338807,831338826,831338841,831338847,831338868,831338958,831338974,831339001,831339023,831339038,831339060,831339125,831339163,831339189,831339222,831339247,831339399,831339416,831339456,831339739,831340037,831340052,831340069,831340081,831340095,831340128,831340147,831340171,831340390,831340414,831340449,831340570,831340594,831340615,831340659,831340685,831340705,831341370,831341531,831341556,831341579,831496507,831496877,831575525,831592871,831903692,831909825,832066929,832468439,832553877,832912840,832915115,832949747,832949768,832949800,832949820,832949874,832949898,832949950,832949974,832950384,832950409,832950794,832950805,832950865,832950879,832950903,832950925,832950947,832951002,832951016,832951038,832951117,832951161,832951536,832951573,832951663,832951696,832951715,832951744,832951769,832951843,832951870,832951893,832951919,832951940,832951960,832952018,832952026,832952048,832952072,832952086,832952908,832953550,832953610,832953643,832953680,832953695,832953731,832953749,832953759,832953825,832953843,832953938,832953956,832953979,832954003,832954057,832954124,832954138,832954375,832954430,832954483,832954509,832954535,832954572,832954587,832961213,833173790,834043995,834045848,834045958,834074530,834081476,834632366,834636064,835367736,835368836,835373495,835383039,835444879,835445138,835460570,835547609,835835934,835849323,835959458,836319194,836708386,831487979,826526900,826587961,827097191,827133233,828324223,828660538,828821712,831634110,831864280,831865635,832548210,832576173,833034512,833110746,833112555,833595916,833855115,834339393,834340685,834563684,835349232,835689126,829147184,829934838,824967579,824988681,825100476,825533408,825533764,825589776,827688152,831847828,831860760,831860854,831864640,835381115,835853921,836019671,824614412,824614422,824622040,824732967,824914619,824939050,824976849,824981011,825026846,826253699,828393493,828451331,828456489,828456524,828824034,828825469,829154866,829227056,830157041,831209829,832375341,828116641,828174955,828317158,828321559,828326967,828390017,832717204,832608937,832611865,833072824,833278397,833620563,830733985,834277732,830740516,830740648,830740764,830740921,830741062,830742446,830742513,830742598,830742884,830742936,830742992,830743432,830743549,830743768,830743957,830743976,830744003,830744201,830744224,835624572,825497159,836863852,827411778,827441976,836143400,836363128,836367952,827351139,827385289,827544523,827674110,831588703,831708386,832741881,832845348,832845359,832845546,833097372,833097389,833097451,833153245,833153272,833232712,833233060,833233083,833245241,833249226,833249450,833315672,833559764,833676142,833680132,834891699,835410131,835410147,835410181,835324845,828942145,828944524,828944535,828944546,828977372,828977391,828977408,825456997,826015130,826024081,826031852,826888165,829040681,830402254,832674375,832728037,829661536,826279164,826279182,826279199,826279224,826279242,826279271,826279290,826279306,826279329,828244588,828244603,828244613,828244624,828244638,828244649,824568522,829544883,829560315,829568311,829569301,829569485,829579539,830055296,830058150,830417450,831449934,831884143,831884254,831885442,831911291,831913625,831914289,831915171,832374952,832903364,833265589,833651200,833653196,833661852,833715118,834122467,834123246,835296527,824948060,828365236,828468101,833347328,833601674,834841561,824921013,824921061,824921113,831526580,824503090,824503162,834703447,831028806,831035061,835923430,829532870,830292825,830294272,830301814,830306494,830353325,830357566,830591865,830597852,830609266,830617603,830631502,830832997,831709783,831710539,831713342,833103607,833368009,833732536,833741567,833752313,833796594,833800831,834240788,834346767,830626220,829872567,826410297,828815415,830099002,825240259,825242010,825318811,825521181,825531564,825754280,826175381,826188146,826249398,826275329,826278861,827086211,827644391,827789528,827792880,827879572,827932686,828324008,828338540,828631383,828639261,828652077,828670712,828675110,829193043,829228441,829243900,829262432,829280035,829520428,829694722,829703236,829718043,829794266,829803861,829809313,829955099,830028259,830042346,830810936,830824227,830852793,830861382,830867557,830876738,831114451,831121014,831605277,831809784,832021340,824652592,824652652,826167749,826245473,827260804,827267533,827269603,827269766,828910931,830938293,832173715,825127963,825141298,825550046,825550053,825618360,825618362,828282897,831625102,831685279,831727086,831823251,832426395,833415977,833424728,833439312,833630723,833670667,833693859,833703198,833710230,835934718,836004754,828244290,828248547,829005384,834018551,831338670,831338691,831748500,835065793,827327700,830301470,825019930,825389960,825938995,827912046,827983890,831421602,833317364,828116495,830213865,831333790,831734253,832676662,826608606,827142917,828431081,828433399,829437135,829615936,830621267,830627763,829090088,824624576,826742955,826742987,826883141,827646539,827646753,827830589,827853095,825554347,825586115,827789304,825257319,825623161,825623239,828251586,828265775,831209587,833655913,833725182,831297847,831304941,831315687,831856217,832809491,832809677,832886457,825441069,826505989,827061068,827434395,824849390,829274455,824765726,826770305,827512353,827548751,827562805,827669637,830981584,831337415,831588410,831593765,831632926,833452107,835391262,835573038,831886660,831886662,831886673,831886697,831886702,831886791,831886801,831886803,833634007,835816600,827450267,827482664,827772323,834762966,825431671,828003713,830506967,825259342,835260701,835260702,835260703,835260704,835260705,835260706,835261135,835261137,836510931,836510932,836510933,836510934,836510935,836510936,836510937,836510938,832946504,828175964,833037113,827085240,827088527,827088559,827095255,827108226,827118168,827121435,827135085,827137310,827152150,827517599,828303618,830252954,831947420,831947839,831948403,831950806,833227026,833979202,833980587,833981484,834064762,834064835,834301740,834306781,834413871,834415596,833247384,833249828,833249859,833249882,833249948,833249981,833250357,833250387,835714549,835714561,835714569,835714584,835714595,835714759,828304277,828307142,828317476,828321129,828322555,828323445,827942432,835137511,835137527,835137546,826885640,827840094,830898005,836241404,824563470,830110725,830551511,830761407,830763071,830765288,830829075,831118512,836632689,836634083,836649720,836652191,825130295,830423297,830423315,830423335,830423346,830423358,830846272,831408648,826143850,828505874,828342893,829978624,830032141,830113065,830420528,830420535,830420750,830420802,830420840,830420853,830420870,830420917,830420936,830420938,830421005,830421016,830421028,830421103,830421131,830421208,830421211,830421271,831338654,826879795,824594090,824609705,824632017,824922918,825030483,825182064,825246675,825251166,825412066,825620189,825640713,826367653,827416027,827419358,827536215,829867732,830658937,830997308,831045615,831240551,831616270,832285644,832286259,832346249,832426774,832617830,833060964,833088467,833138712,833331239,834621692,834621694,836026565,825114289,826033124,826767477,826767478,827049017,827071992,827072613,827158621,827163337,827163350,827288813,827708890,827710983,827720414,827720690,827720694,827720700,827724613,827760802,827941806,828479578,828479579,828938629,833427597,833428913,833514749,835169940,835332849,835332850,835750032,836472023,827027072,827376038,834939277,829960256,833461439,833658693,827781339,828440227,828440236,828440345,833833007,834129861,836369069,830985985,830986021,827643097,828743079,828037373,828241289,828283309,829429746,833588527,835370719,836797786,827743972,835093096,827749272,828795543,829899491,830356116,830360508,830369681,832880463,831460206,834397422,835051530,835646747,826040152,828609740,828645803,830697439,830699633,830705637,830939055,830953471,830978583,831247350,831250621,831598844,831615733,831901459,832083509,832399896,826489846,826619961,826620482,826623398,826849535,826851594,829371637,834173821,836293355,836296922,836296946,836382118,836478904,836524935,836649214,825855567,828644369,829263524,830126005,830389373,831251659,832104300,833801065,835856307,830756271,830857095,827326778,828347752,824467937,824467994,824468024,824468046,828576172,828578777,829048542,829228442,829234209,829244283,829579648,829587909,829598331,830062448,830087320,830114031,830745955,830968413,830968455,836239169,825085053,825085076,825085238,825371835,825916098,825916527,825916543,825916837,825916858,825917307,827491424,827498898,827552975,827672604,828009122,828009173,829284965,829664299,829664321,829664354,829664382,830486125,830697904,830788539,830788638,830788662,830789048,830791874,830791891,830792404,830792415,830794844,830794920,830795186,830795207,830795263,830796568,830797490,830798052,830798067,835803913,835803956,835803983,835804294,835804332,835804356,835804385,835809129,835809516,835809539,835810002,835810258,835813297,832323804,832323814,832323817,832323822,832323825,832323833,832323836,825047282,825529776,828225292,828400204,828402692,828893839,828939210,828941586,824760919,824845126,824845233,824879391,825834566,825853318,825853379,825856408,825856426,825856435,825856444,825856451,825856473,825856486,825856497,825856503,825856518,825856537,825856555,825856579,825856593,825856618,825856625,825856644,826091840,826100927,826462078,826484229,826542256,826542519,826608646,826608647,826933859,828011346,828084660,828127594,828428508,828429397,829030285,829030371,829030422,829030501,829030530,829030682,829030706,829030723,829030756,829113034,829588173,830178666,830551684,831050984,831149808,831211048,831216698,831216812,831216968,831226988,831240663,831461797,831484556,831487773,831873629,831873635,831873647,831873651,831983681,831984590,831984623,831984650,831984669,831985351,831985373,831985387,835302312,835302335,835302369,835302435,835302564,835303621,835303629,835305298,835306744,835307050,835307058,835307066,835307077,835307089,835307105,835307358,835307369,835307378,835307813,835307822,835307838,835307916,835307932,835307952,835307968,835307997,835308015,835308039,831189118,826701098,826710337,834142037,836662357,827148054,827148118,829372747,829374157,829374162,829374164,829374173,829374185,829374192,829374208,829724955,831208050,831208073,831208090,831208106,831208123,831208174,831208953,831209013,831209133,831209192,831209224,832222667,832222676,832265786,834437337,834437379,834786485,834802908,835390480,835424796,835723828,829269450,835097210,824504870,824512677,825559008,826461523,826847285,828080781,829261841,829262058,829262091,829281642,829281685,829281748,829281772,831393771,831397356,826644747,826981227,830280530,830758432,830765749,830772373,830875046,831408644,831411480,831849479,832088629,832809544,832810635,833367617,834007049,834010724,834433625,836039606,836039607,836039614,836039618,836039619,826603002,826603024,835072734,835869212,827909309,830916352,831061479,831061508,831061550,831061577,831879778,834612613,824670569,824760330,825188534,826924770,827090692,827365001,829142005,829142043,829145669,830822048,831550896,835898144,832716176,832720071,836771215,825425650,825425848,826968628,827467572,828943012,829140999,830080530,830230996,831240236,831590892,831590905,831860377,831924863,832081448,832081747,832087350,832113581,832176153,832944194,833633913,833840217,834330369,834330389,834342602,836107270,828454406,830149997,826532243,826610402,826611370,824577292,824594786,824595342,824807663,824823525,824845820,825592058,834644689,835388571,835769911,835930619,835930823,835931119,835931388,827695339,834694292,834694557,834694593,835266159,826218381,831514777,828401836,830418081,830834173,831644913,827974542,831723410,833172117,824607545,834186187,834206970,836200607,830330888,832576898,824794755,825184796,825192035,825197558,825419875,825844944,826154342,826154344,827012874,827013773,827311727,830257103,835354942,835806487,836075075,836135325,836135543,836565829,836602582,836604988,836759722,836763623,827817576,824556546,824559706,824561858,824564577,824573181,824577253,824755183,824762111,824764334,824769789,824772691,824773105,824776412,824782997,824788289,824792913,824800013,824801078,824808899,824824691,824966621,824980749,825077302,828396945,829576114,829579649,829587910,829598333,824842310,824913103,824958886,824962222,825446058,825572982,825592865,825599924,825607340,825834286,825851775,826086803,826940438,827381178,827391874,827408968,827496184,828081208,828083663,828266485,828271923,828273737,828371779,828784351,829469168,829811981,829823921,829836061,830123698,830184007,830184655,830200324,830205673,830217251,830246723,830246749,830858939,830883754,830960727,831159758,831159950,831373203,831373444,831380547,831397392,831896665,831896811,833420296,833421580,833425346,833429896,833513022,834528006,835837024,836574264,827777816,828375136,831103890,831279355,824800391,824800605,830740804,830740847,831481814,831702062,831702108,831702392,831819179,831819198,832154322,832154354,832154445,833567617,833567800,835382335,835382660,835744688,835744814,835744856,833256599,833701718,825083762,825084248,825084532,825121744,825121770,825262479,825264129,825271600,830168361,830168396,830346983,824970578,831422956,831423098,834920711,834920722,834922446,834924348,835401902,835590377,824468167,824468206,824468234,824468359,824468369,824536563,824536585,824536615,824536685,824537292,824537302,824761352,825042236,825043036,825243537,825243548,825243552,825243559,825243562,825243571,825243576,825356109,825427462,835728496,827365326,829377790,829378310,829378318,829378341,832310423,836020543,836020556,836020561,836020566,836020574,830253115,830640628,830645947,831733727,831940982,836153295,836153322,836302588,836302635,836302671,836302693,832849325,833106828,826434198,826439652,830746728,830746953,831669849,831670242,831670257,831670280,831670312,831670338,831670355,831670378,832466075,826021354,827772045,828216242,828225811,829106618,829113810,833052228,833334585,833550974,834313523,834336539,834338964,826487069,826544170,826544192,826544208,826544458,826544474,826544500,826545964,826545973,826545986,826546006,826546029,826546375,827320116,827890835,827890852,827891550,827892186,827892199,827892214,827951059,827951443,827951515,827952364,828434526,828442721,828633381,829441288,829680555,829706115,830176632,830176678,830236619,830236623,830440568,830440944,830455190,832954705,834220857,834320708,826741066,828972033,829001090,829016772,829184478,829615257,829960853,830165927,830167124,830651099,830651423,830950372,830958633,831065867,829843675,829843688,829843707,833012187,834372724,835241301,835241364,835241428,835241455,835241489,835241512,835241552,835241896,835241925,835241934,835243858,835243866,835243872,835243874,835243880,835243887,826005826,834780388,836093268,836641830,836641862,824651047,825391304,825391439,827250772,827250805,827252714,828657702,828673264,832503340,834318531,824718688,825105663,827432552,829125559,830467320,830471915,832523893,833728086,831820703,831820716,831820900,834979410,835403887,835784439,835787372,835845438,835864371,835936266,835945224,835946306,835947278,835954134,835957599,835981317,835983168,835983429,835983448,835983485,835983514,835983722,835984155,835984225,835984367,835985188,835985440,835985523,835992031,835992392,835992758,835992762,835992774,835992780,835992786,835992888,836105572,836105881,836106996,836107003,836107021,825111669,825112061,825113143,825113476,825897916,829546615,833982384,835568911,828173402,831138483,832509160,833179143,834348386,834562446,827388410,836063073,829082233,830486516,830666590,830671635,830671729,830678820,830679042,830679119,833836525,829983192,830414868,830476068,830479382,832424766,833640383,830293710,831053754,832275647,827568322,827419846,830845281,832138596,832649783,832692173,832836565,832838674,832859544,832859800,832861344,832877283,832896958,832904671,832905262,832905488,832931215,832935371,833041479,833067148,833068906,833214792,833216374,833219440,833343983,833607137,833621805,833965680,833994655,834013105,834014886,834034041,834042888,834303443,834307627,834334130,834399004,834400488,834416106,834483984,834485476,834826423,834879036,835434673,835627800,835629126,835636764,835850403,835965888,836204341,836205358,836682425,829757956,829758010,829758037,829758994,829759054,831992782,836024507,836211082,836226972,836578485,835678186,835681492,836572460,836543346,825773571,825944699,826688023,826690886,827241263,827996084,828955434,829109002,829211773,833688406,833702401,835660730,835661617,832319175,832322014,836040373,836045313,836048205,836490895,836067096,836067250,836070703,836070710,836071805,836071811,836072881,836072928,836075667,836075724,836079366,836079367,836084511,836085460,836086372,836087570,826523648,826523692,826523957,826524020,826524056,827453137,828937165,828913948,830688378,826283983,826289700,826296045,826688356,826688551,827683720,827904286,827923561,828125187,830404780,833604526,828569363,824613760,824616691,824966167,825154384,825449467,825449781,825643291,826859115,826859134,826861864,827131517,827136016,827136838,827136940,827235629,827777853,827782008,828294113,828294637,828296168,828679106,828679302,828680124,828682086,828685015,828685016,829040683,829214168,829216294,829216327,829216505,829216564,829414721,829911163,830229988,830264557,830402695,830403898,830403979,830803937,830804013,830804088,830804234,830804287,830832521,830832836,830833752,830922382,830923240,832700799,832729988,832747435,832911186,833077715,833079470,833838306,835069094,829723644,830059970,830059993,830737329,830737350,830737356,830737373,831154182,831171406,825028101,829005099,831265203,835234511,835664680,831879223,827078933,827101486,828188582,829347187,830351067,824672844,824675530,824681965,824707543,824710298,824725048,824741585,824743319,824800014,824939965,825193140,825196241,825198629,825465999,825565081,828581295,828117895,825532169,824973253,824994851,826838075,827961051,827969040,828706257,828921572,829112172,829175650,829203534,829374140,829886675,830172009,830685079,830686870,830858619,831325147,831341105,831349422,831668055,831693937,832529832,825770516,828996742,829038279,835721156,824650937,824809750,824809760,824878779,824905424,825245757,825247990,825248004,825248018,825248040,825012423,825012448,825013017,825312941,825312956,825312966,825312986,825313012,825313042,825313218,825313266,825313281,825313359,825313382,825313411,825313649,825314446,825314495,825314637,825314653,825326090,825584348,825586610,825586621,825586640,826813242,826813256,826813279,826813298,826813524,826813541,826814446,833427070,834937951,834937975,835007570,835007598,835007726,835007746,835007770,835007905,835007923,835007939,835008030,835008041,836292667,836292705,836292750,836292803,824923227,826523759,834172856,834909162,835663912,836655373,831844270,834573171,834852411,834869946,835067873,835067892,835067930,835067983,835068003,835068021,833597838,829483435,829483443,829483457,830448770,830231056,830762453,830902858,831013302,831013313,830887781,826969600,828073448,836502081,836502082,830015481,830507829,831253529,831611896,833033876,833037610,833255424,833285800,834990785,835090078,835236378,835429228,835430624,835683952,835755201,835781927,835797445,835831377,835926845,835927163,835934817,835941377,835953327,836120114,836409683,824487185,824766882,824990526,826002198,826278581,829590359,832055984,832061575,832239010,832968039,826159488,826222711,826338248,826666708,826764464,826803660,827037653,827037752,830399492,830951005,826175800,829971600,829980311,833327392,833327393,835638166,835638167,836549740,836634438,824598396,824656739,824656753,824656778,824667539,824678421,824721660,824919877,824941844,824941860,824941874,824941922,825055584,825055601,825055627,825055644,825136199,825136202,825136203,825136268,825136270,825136281,825136283,825136284,825136286,825136291,825159167,825159200,825171619,825411805,825480699,825492397,825569963,825614885,825731447,825810333,825855606,825975787,826115040,826143422,826143427,826143437,826143447,826143451,826143458,826143462,826143469,826143980,826209090,826240351,826240365,826240382,826240407,826240420,826240432,826240443,826240450,826365371,826436849,826436877,826436897,826436915,826436938,826436952,826436964,826436990,826545421,826545448,826556957,826560041,826560066,826560083,826560101,826560115,826560787,826560855,826560894,826560934,826560995,826561153,826591776,826665733,826685267,826689997,826694671,826696130,826797304,826918622,827096544,827099006,827191329,827229171,827287821,827327195,827332838,827471866,827504170,827593850,827594217,827637217,827641066,827650963,827730092,827730094,827730110,827730119,827730145,827878069,827878708,827892903,827966553,827966579,827966600,827966613,827966645,827966667,827966684,827966703,827966723,827966751,827966781,827976661,828168754,828168791,828168828,828168851,828200548,828200638,828200682,828200748,828200781,828201801,828201829,828201861,828201906,828201945,828201991,828202030,828202061,828359444,828473755,828498886,828552974,828674438,828684383,828744684,828754820,828860204,828860229,828860241,828860290,828860307,828860885,828860925,828860984,828861014,828861042,828861064,828864520,828864658,828864702,828864748,828877330,828905435,828905443,828905451,828929685,828933953,828934951,828940431,828940472,828944406,828944417,828945192,829294835,829302847,829316309,829316321,829399532,829402089,829403356,829411472,829411484,829411485,829411486,829411488,829411490,829411492,829411493,829411494,829411498,829411504,829411506,829412381,829412385,829412438,829412497,829412500,829413879,829415239,829415240,829415284,829415289,829415295,829415303,829486897,829497504,829497524,829497537,829533561,829795983,829825241,829839477,829845002,829845036,829846691,829846711,829846737,829846750,829846772,829846804,829846829,829847036,829856703,829856716,829856748,829856911,829856918,829856923,829856935,829857027,829857043,829857060,829888871,829890556,829900046,829906063,829906064,829906066,829906074,829906082,829906086,829906087,829906098,829906099,829906101,829906104,829906105,829906107,829906114,829906119,829906120,829906124,829906127,829906202,829906207,829906213,829906217,829906218,829906221,829906226,829906228,829906231,829906232,829906233,829906234,829906236,829906238,829906242,829906247,829906249,829906251,829949882,829950000,829950628,829950659,829950699,829950727,829950759,829968586,830071260,830071475,830071565,830071581,830071594,830071620,830071648,830071669,830109144,830109176,830126116,830126134,830126148,830409974,830410001,830411005,830431311,830444661,830510364,830535816,831133066,831218010,831218081,831218221,831218298,831218921,831219997,831220043,831263355,831307747,831307759,831307779,831307795,831307803,831307814,831307832,831307843,831307859,831307871,831307889,831307900,831307993,831448671,831449169,831449259,831692357,831811966,831966139,832032483,832082786,832342860,832343758,832343767,832343776,832343786,832345598,832362834,832661728,832661771,832661815,832661850,832664777,832664905,832665203,832665222,832665235,832665272,832665550,832666829,832666853,832667455,832667521,832667551,832667581,832667630,832667655,832668028,832668213,832845483,832898602,832902801,832973430,833028806,833048403,833231931,833323814,833324140,833324943,833328399,833330822,833479099,833479101,833847312,833847335,833958776,833965918,834044959,834044988,834045005,834045023,834045255,834045272,834045283,834045295,834045307,834163451,834185138,834280457,834571502,834616535,834616539,834716950,834727663,834765306,834988268,834993682,834993693,835114977,835115294,835115325,835116843,835116874,835196128,835196139,835197101,835197106,835197122,835197126,835197131,835197138,835197146,835637312,835637392,835637713,835639175,835954354,836174387,836260463,836662264,836662301,836662333,836662378,836667991,836668014,836668090,836841711,836841724,836891938,836899405,824497480,824497829,824953870,824953896,825056174,825272617,825278165,825278167,825456379,825460920,825462402,825555020,825557301,825650906,825732780,825787542,825860163,825860185,825872972,825883630,825895649,825972188,826334831,826338057,826409787,826479555,826518170,826520906,826520918,826538989,826718365,826797456,826808345,826808364,826992288,826998161,826998980,827058632,827149583,827309966,827326735,827429466,827512534,827628514,827635407,827746702,827752076,827876815,827876833,827877751,827877769,827878086,828189364,828193495,828199220,828276449,828395708,828395715,828468749,828492535,828664494,828667543,828667792,828933555,829109118,829494907,829722503,829722517,829722538,829722576,829722591,830092903,830092908,830092926,830092966,830092978,830093045,830093055,830093944,830093975,830094070,830094087,830206783,830214382,830369840,830754347,830754393,830754439,830754837,830755186,830755233,830755287,830755339,830755380,830757633,830781520,830783341,830950908,831104587,831370970,831373830,831375873,831524914,831524925,831906923,832058916,832059117,832059152,832059242,832059296,832059366,832132035,832133378,832133499,832135086,832167050,832167908,832203873,832214025,832214037,832214053,832736128,832736145,832736155,833284369,833285827,834139927,834354616,834355308,834358923,834457832,834881197,834988737,835035423,835047026,835233757,835272158,835272167,835272178,835272185,835272200,835272729,835272732,836297475,836297495,836593316,836909247,825708690,828402464,828578609,828953946,830043092,827958111,828820413,830892285,833766181,834075963,830888982,825113945,826039087,827442672,827923411,828127082,828559456,828749938,828749986,829806439,829806464,829806476,829806500,829806533,829806548,829806563,829806592,829806612,829806623,829806644,830271139,830368148,832147472,832981707,825907162,827277026,827277055,827277100,827277138,830236431,835908303,825335832,825742500,825752826,827552801,830128530,830227195,835271994,835272472,835278935,835290329,824976394,824987992,825339673,828053584,828642081,835521319,835927804,836108400,836780768,836851217,824546796,824546812,824546829,825651843,825651898,825651901,825652212,827783222,827795737,829566721,829597211,829597264,829628557,829629489,829846305,831577046,832268317,832268332,835035190,835035208,835287700,835626460,835626467,828236113,828455003,828487924,828798033,829223687,830855936,830875510,831680398,833487539,833500536,835186842,835186872,835345584,836009908,836318561,836318827,833468735,833720072,830942878,831443174,831443177,831443179,831443180,831443184,831443191,831443192,831443246,835300925,825481397,825500215,827079985,830177072,830270666,831113641,831309171,831358490,832809831,832939678,830926699,831830543,834511476,834722740,834731508,824774164,827733634,828315250,828315458,828315781,828316992,828317844,828528371,830604526,832002344,835314260,828682174,828682302,835736634,835737800,824600769,824953877,825050034,825239910,825272007,825293210,825309266,825418561,825642696,825643490,825643906,825650752,825650907,825733276,825930567,825941620,826219184,826281547,826281939,826305740,826339893,826550421,826720091,827275381,827322160,827329604,827348328,827363137,827378083,827378595,827447482,827448920,827586137,827640039,827645409,827709601,827718464,827730525,827731913,827742687,827747820,827752199,827817825,827817849,827817859,828121155,828190757,828274550,828279683,828282878,828283740,828469357,828657079,828765731,828812883,828812903,828812927,828812931,828812939,828813025,828813035,828813041,828813137,828956607,829033711,829033859,829294502,829311164,829317905,829371488,829676990,829746571,829773613,829888917,829892610,829977936,829978019,830160088,830994401,831036795,831195619,831204863,831204893,831204919,831204983,832306110,832635970,833462098,833466881,833466895,833919850,833919902,833919920,833958781,834012372,834063331,834063364,834700825,834701266,834701309,835118546,835118566,835118602,835118615,835118797,836357146,836357243,836357301,836357372,836454120,836479410,836643407,827931352,832309544,832841119,833386993,833433007,833433455,833998395,824529915,824619170,824619343,826127177,826487412,826580793,827459193,828277292,829901884,829901950,830186793,830382441,833869497,834591837,833755694,833755710,824785518,825102309,825103582,825209795,825902699,826760196,827067006,827067014,827067020,827067023,827537260,827720576,827720577,827720578,827720677,827747096,827747099,827747100,827747244,827840315,828515848,830005622,830091497,830192661,830846910,831776522,836101493,836101500,836102874,836465656,836472385,836472386,830598953,828195622,828195642,825668275,825095963,825095977,825096004,827067458,827067496,827067506,827716005,827716015,827747293,827835790,827835796,829402917,830522918,830542106,825037729,825049904,825239214,825274763,825289908,825413287,825420298,825438068,825440590,825484965,825494893,825497664,825521183,825525728,825564450,825950762,826000663,826274046,826445185,826499464,826762315,826786291,826902526,826925836,826980253,827036347,827046232,827089212,827127609,827532654,827972937,827983814,827991378,828003805,828038174,828204778,828503603,828553617,828580204,828670714,828675111,828680440,828688835,829355114,829420939,829612669,829621740,829660516,829983034,830004496,830008546,830100193,830185328,830341128,830357046,830529677,831407793,832458974,832477702,832655589,833330688,833838118,830812781,831207075,831207080,828671700,830175214,830175234,831296204,825862444,826649274,827016587,827016688,828150362,829171396,829478850,830756994,830757064,830789635,830904503,830971877,831027435,831083631,831124440,832191743,832313202,824689477,824709855,827368442,828961728,829612219,829813136,830067418,832601331,827630890,832184537,832184612,832223186,825627117,826123219,830492718,831179251,831628896,826972520,829695853,835332429,824672858,828863991,828865156,834276052,834311721,835319197,826941392,826946299,829287669,829619783,829623585,829623662,829627341,829644605,832281753,826143993,826730813,828401805,828401849,828401863,831981449,831985022,831986419,832704777,835467812,836450061,836450078,836450087,836450088,825921888,826543978,826544690,826613423,826887150,826887179,826887208,826887229,826887263,826985997,826986017,826986069,826986083,827042596,827215733,827216085,827216090,827259844,827319355,827319376,827319391,827319403,827475214,827555522,827588830,827588843,827588860,827588874,829971514,832133359,832734424,832734441,833758200,833758511,833758541,834133755,834354251,834354663,834354697,834611784,834611794,834770324,824921291,826088572,826742434,826888045,828434139,830281542,830999581,830999630,830999834,830999894,830999935,831000033,831000115,831208086,831347851,831371640,832089083,832284609,832284622,835954896,835955701,835956083,835956517,835956896,835957269,835957674,835958106,835958387,835958728,835959091,835959413,835959715,835960051,835960361,835960628,835964092,835964545,835964917,835965329,835965673,835966009,835966337,835966787,835967154,835967473,835967785,835968122,835968430,835968748,835968999,835969333,831551399,829744815,830765632,835037928,832136577,832547563,835045469,826614038,826616412,826616456,827116431,829429467,829431901,834463276,834465723,834514476,834726044,834728986,834731613,835144567,835210487,835522447,824828853,825028741,825028785,825885410,825885580,825885618,825885654,825885670,825885687,825887577,826097555,826097567,826097605,826097629,826866966,826985819,826985846,826985899,826985936,826985950,826986180,826986195,826986252,828037303,828368183,828534997,828891200,831975069,832404337,824671513,827574322,828411542,828528406,833043250,836676428,836733080,827329667,834761928,835943416,824982735,825362804,825363448,825363597,828037087,828037762,828047129,828048589,830223538,830223721,830223886,830223938,830224009,830224069,830267130,835258629,835258829,835280400,835345162,835345339,835345532,835345712,835741446,829673084,829673775,824684848,827360426,827360572,827360611,827360666,827360696,827360713,827360962,827360994,828936439,830805984,828339622,828339633,828341960,829963423,830408144,831525698,831525782,831525845,831637150,829949107,829958225,831174045,826124224,826812867,827259941,827298602,829135734,829469798,829949022,831975028,836098933,836099404,825656832,825665040,825986060,825988310,826031009,826044432,826056516,826059145,826064453,826079808,826617145,826930697,827139071,828437798,829437625,829533327,829718046,829841559,829858595,829860142,829864593,830360792,831372294,833151803,834273711,834300660,834300684,834300700,834300726,834300752,828903595,828903732,835068951,832019561,832067286,832246404,832823231,833231480,833231505,833231525,833231537,833231557,833231581,833231604,834426671,834426695,834426712,834426727,834426757,837026793,826694283,829331414,829331423,825295949,825295965,825841877,827229054,827229058,828341145,828643080,829948739,831432313,831433507,831562543,831564793,831566773,831570608,831571297,831764406,831764421,832751377,832869149,832869157,833605583,833721346,834002939,834008426,834156142,834886223,834915204,834922706,835133150,835163610,835178956,835182113,835385212,835385239,836553168,835868020,828200275,830880747,833888290,833888305,833415089,825325123,825555828,828413005,829012588,829044078,832637652,832976887,834365849,834515987,834983487,835006606,835202357,835728762,826428027,827837730,828545410,828939116,833502134,833652279,829620530,831985431,831985441,825611222,825965287,825967230,826640291,826775676,826779815,828278381,829358442,829386835,830385994,830795934,830885054,831374597,831901141,832328825,833486490,834046448,834046615,834058762,826826966,826826980,826827149,826827154,827013307,827013324,827369857,827380467,827460624,829191453,827353116,828939574,830660772,830664233,832916634,833265416,832666908,836073574,836106230,832503004,835867328,829329449,831166300,831193827,835253262,835285960,835285970,835788989,826874897,826875159,826317210,826780636,827225842,827228162,827229260,828870054,829127589,830419429,830464156,831022048,833516021,834483064,825643690,830830649,826491807,826636611,826674447,826674525,827151138,827180922,828120007,828120040,828120060,829442319,829442331,829442350,828048325,828101460,824867823,830461377,831328399,832222645,832288094,832288132,833187085,833339492,834720845,824485567,824485594,824596295,824645089,824645095,824646310,825723607,825726971,828321951,828328536,828332425,828526312,828543514,829946853,827130469,827130477,828126241,828169359,828244112,828246965,826278594,826278638,826278674,826278715,824668712,824786465,825188442,825659661,825972159,826025428,826025454,826025507,826025649,826025666,826719246,826719309,826719593,826719649,826719667,826719686,826719708,826719732,826720039,826720064,826720105,826720210,826720220,826720226,826720230,826720300,826720313,826720321,826720327,826720424,826720442,826720450,826720462,826720553,826720562,826720572,829985391,829985399,832846270,832995076,832995098,832995122,833082079,833083457,833083494,833112273,833112307,833706488,834309930,834437235,834627168,834660201,834697934,834703240,834773570,835048048,835220975,835360603,835360837,835360858,835360930,835360936,835360984,829161379,829177116,829184583,829192054,829603231,827773513,828808187,828808199,829514044,824967437,824967457,834166623,827103100,829212951,829213703,829213907,830211691,827637244,827637264,827637305,827637459,827637489,827637534,827637553,827637694,827637731,827637757,827637772,829262023,829648327,831382050,831899909,833417469,835111046,833981114,835270081,835270140,835270413,836604561,825004287,827546619,835714406,836623485,832713736,832715220,835857400,835860149,835870118,824647767,827470377,830880715,830958189,830961412,828672401,828954064,828954093,828954111,828954133,828136829,830780988,830781034,830781076,832232437,834906317,835112576,835134146,835134683,835176600,835593548,835597687,836000863,836001337,836008252,836010880,836011360,836017968,836018008,836018037,836018065,836028297,836029126,836031856,836032222,836032591,836032596,834120830,827686954,834721451,827571808,825120089,825307060,825307078,825307089,825307097,825307104,831546464,832597635,834906703,834596947,834598303,831621208,831933037,831952959,831964118,832137935,832143700,832165933,825695868,825703036,825704007,830594400,826985521,826985551,826985589,826985672,829684170,830953431,831750253,832406307,834643634,834587840,827215295,827488453,828321013,828456969,828457057,828457087,828457121,828457157,828457190,829487551,829492473,836520040,831327658,827549682,828559296,829903631,830075525,830075694,834613217,826172117,826172508,826172548,829316502,831657160,827255991,833277276,833277285,833277568,833277595,833277612,833277626,826768561,829073748,829073749,829143620,824748604,824925752,831848327,828924683,835978464,835978505,835978542,835978580,825385869,825903095,826509088,827059865,828865543,830850892,832306243,832306289,832306337,832306354,832346608,833205584,833540827,833684226,833716854,833716875,833722034,833722285,833754973,833755023,833841864,833972706,833972752,833972788,833972846,833977432,833998652,834628741,834733675,834760925,834866936,835405780,836555231,836581016,827363683,827833385,827872905,834318234,835109453,835364647,826391875,825953404,826919157,826983820,829110448,830394073,825193935,825884055,826191436,826207580,830024088,830672036,830672192,830708502,830845712,831567443,831610374,831610412,831845473,831982088,832790515,833153563,833449065,834184737,835582560,835636151,836565993,832462284,835975690,826131846,828834506,833513602,833519406,824595315,828278679,828371575,830869109,831552385,832006945,834908016,835021569,829882914,833435663,834581419,834587169,835160148,835160156,826954261,828590389,830100217,830100263,835319426,824880255,825858252,825860899,825878084,825981871,827798209,829719724,829720052,829720067,829720176,829720273,829720291,829720305,830608981,833268174,828407236,833232710,833678068,833678126,834679880,835387301,824908100,834874680,835099505,824711283,824739157,824791232,824814393,825285220,825379241,825995726,825995747,825998106,826834649,826835734,827030351,827337541,828532745,828949766,830205880,830705167,830852546,831166708,831197880,831224964,831224983,832224003,832420331,833691097,834735740,836115467,836625971,829064548,827994953,827998132,828018857,826738538,826787499,826839325,826839507,826839727,826900367,827051905,827051920,830516568,830649154,834591344,826601621,828629121,829415375,829415382,829415385,829566399,836822629,836844934,835141992,835990583,835998799,836001652,836008712,836010987,836015872,836015886,836015907,836015924,836015958,836015984,836016022,829958402,825294676,825294762,825294791,825294807,825325220,825325231,825325252,825325277,827866413,830851907,832782053,833179087,833207871,833229426,833835757,827518884,828252495,828703416,828703431,828703444,828703460,828703470,829149843,831235108,831444634,831444789,831444794,831521581,831521759,831700066,831880184,831880186,831880187,831880190,831945154,831945158,832090004,832090015,832091198,833419807,827502330,830544798,824637455,824967584,824967607,824967629,824967665,824967686,824967705,824967722,824967769,824967789,824967820,824967842,824967861,824967869,824967895,824967943,824967976,824968160,824968176,824968195,824968218,824968235,824968251,824968275,824968282,824968295,824968323,824968342,824968370,824968396,824968415,824968435,824968487,824968510,824968525,825008982,825047465,825200995,825291076,825440078,825440108,825440141,825440170,825440191,825440217,825440240,825440269,825440296,825440317,825440343,825440370,825440385,825440405,825440426,825597273,825597549,825597618,825597658,825597682,825597728,825597856,825597886,825597932,825597978,825598028,825599059,825606077,826033548,826047403,826047414,826047426,826047447,826047504,826047554,826047584,826047608,826047628,826047643,826047741,826047759,826047774,826047811,826047822,826047838,826047874,826047888,826047906,826047929,826047945,826047959,826047971,826129932,826491107,826491161,826491206,826491233,826491261,826491280,826491349,826491378,826491401,826491431,826491476,826491507,826491524,826491548,826491588,826491604,826491626,826491644,826491679,826491724,826491750,826491782,826491894,826491956,826491968,826491989,826492056,826492079,826492089,826492124,826492160,826492179,826492199,826492221,826492298,826492324,826492339,826609601,826609616,826609809,826609820,826609855,826609870,826609910,826609926,826609938,826609944,826611854,826611859,826611872,826611877,826611883,826611893,826611901,826611907,826611916,826611922,826611937,826611943,826611949,826611972,826611983,826611996,826612008,826612018,826612023,826612035,826612048,826612056,826612079,826612089,826612097,826612114,826612127,826612148,826612159,826612170,826612183,826612190,826612201,826612220,826612228,826612237,826612250,826612254,826612266,826624346,826624356,826624379,826624392,826624401,826624407,826624416,827130605,827130631,827130651,827130666,827130681,827130735,827130761,827130782,827130815,827130849,827130863,827130925,827130938,827130961,827130972,827130980,827131013,827131027,827131044,827131075,827131094,827131104,827656073,827656095,827656116,827656136,827656155,827656175,827656182,827656226,827656244,827656267,827656288,827656309,827656341,827656356,827656380,827656397,827656423,827656474,827656491,827656505,827656528,827656550,827656579,827656621,827656642,827656657,827656695,827656718,827656747,827656766,827656788,827656804,828259348,828637463,828637501,828637525,828637557,828637580,828637601,828637633,828637656,828637678,828637699,828637716,828637745,828637868,828637902,828637928,828637940,828637959,828637977,828637998,828638015,828638039,828638052,828638079,828638097,828638119,828638140,828638605,828638624,828638669,828638692,828638742,828638769,828638808,828638817,828638827,828638844,828638924,828638942,828638961,828638991,828639010,828639028,828639058,828639073,828639091,828639121,828639139,828639153,828639167,828639196,828639229,828639278,828639305,828639327,828639356,828639381,828639428,828639450,828639520,828639534,828639550,828666391,828667183,828667598,828669017,828704256,828704278,828704296,828704312,828704328,828704352,828704365,828704380,828704392,828704410,828704425,828704439,828704465,828704498,828704519,828704579,828704598,828704611,828704621,828704635,828704684,828704697,828704749,828704770,828704794,828704807,828925656,828955282,828955303,828955317,828955331,828955412,828955446,828955467,828955608,828955654,828955700,828955735,829026782,829026827,829026882,829027128,829207593,829517742,829538320,829638244,829839644,829839654,829839664,829839689,829839707,829839724,829839737,829839754,829839773,829839793,829839815,829839849,829839864,829839881,829839902,829839926,829839943,829839961,829839986,829840023,829840042,829840051,829840069,829840096,829840116,829840133,829840159,829840702,829840716,829840735,829840769,829840784,829840797,829840853,829840878,829840899,829840930,829840942,829840952,829840973,829840992,829841006,829841029,829841044,829841063,829841080,829841088,829841098,829841115,829841189,829841224,829841239,829841256,829841281,829841294,830078392,830079041,830079066,830079084,830079106,830079123,830079139,830104407,830360803,830360816,830624632,830982393,830983926,830983993,830984037,830984051,830984064,830984088,830984114,830984146,830984169,831053226,831053250,831053286,831053309,831053325,831053347,831053397,831053416,831053441,831053466,831053491,831053515,831053536,831053557,831053602,831053630,831053667,831053699,831053735,831053778,831053808,831053827,831053852,831053878,831054299,831054321,831054349,831054393,831054415,831054437,831054452,831054482,831054497,831054569,831054589,831054606,831054693,831054721,831054752,831054796,831054816,831054839,831054871,831054888,831054970,831058895,831058924,831058948,831058969,831058996,831059013,831059033,831059054,831059079,831059095,831059116,831059144,831059173,831059191,831059209,831059229,831059251,831059271,831059291,831059309,831067893,831453849,831691576,831901830,832236343,832275665,832275681,832275697,832275732,832275752,832275764,832275777,832275809,832275824,832275837,832275856,832275867,832275877,832275887,832275900,832275917,832275929,832275939,832275955,832275973,832275985,832276002,832276024,832276035,832276053,832276068,832276088,832276102,832276118,832276141,832276155,832276170,832276183,832276196,832276208,832276459,832276470,832276485,832276516,832276531,832276553,832276566,832276588,832276602,832276633,832276646,832276666,832276680,832276693,832276701,832276728,832276739,832276750,832276801,832276817,832892663,832892708,833368578,833368596,833368612,833368628,833368657,833368696,833368714,833368734,833368746,833368766,833368785,833368810,833368822,833638642,833722135,833722149,833722196,833722221,833722244,833722274,833722314,833722329,833722353,833722378,833722415,833722437,833722468,833722489,833722518,833722547,833722579,833722612,833722638,833722659,833722677,833722713,833722776,833722794,833722819,833722849,833722869,833722899,833722917,833722974,833723001,833723030,833723064,833723533,833723647,833723684,833723708,833723730,833723782,833723815,833723835,833723883,833723929,833723966,833724008,833784810,833830118,833869184,833965282,833965293,833965302,833965315,833965327,834138801,834299134,835113148,835113168,835113190,835113217,835113235,835113267,835113279,835113294,835113318,835113346,835113375,835113401,835113427,835113464,835113486,835113505,835113530,835113558,835113574,835113591,835113601,835113625,835113642,835113652,835113675,835113703,835113729,835113742,835113755,835113778,835113789,835113803,835113828,835113850,835113861,835113883,835113909,835113950,835113970,835113990,835114007,835114025,835114039,835114053,835114073,835114352,835114371,835114380,835114412,835114436,835114443,835114464,835114489,835114498,835114526,835114541,835114559,835570415,835584664,835584687,835657473,835677022,835679119,835795922,835796071,835796138,835969029,835980443,835980453,835981302,836004024,836004039,836004042,836004057,836004068,836004071,836004082,836004092,836004102,836004106,836004115,836004121,836004132,836004137,836004147,836004154,836004176,836004193,836004210,836004224,836004244,836004264,836004271,836004279,836004287,836004294,836004302,836004311,836004331,836004345,836004357,836004371,836004384,836004389,836004400,836004403,836004413,836004419,836004422,836004430,836004446,836004453,836004460,836004475,836004490,836004507,836004520,836004532,836004548,836004562,836004574,836004584,836004594,836004607,836004618,836004631,836004638,836004654,836004660,836004665,836004679,836004689,836004698,836004706,836004724,836004729,836004745,836004766,836004781,836004795,836004806,836004817,836004830,836004847,836004855,836004860,836005157,836005164,836005185,836005190,836005202,836005215,836005227,836005232,836005258,836005272,836005283,836005304,836005325,836005333,836005355,836005361,836005378,836005403,836005413,836005420,836005429,836005436,836005447,836005460,836005466,836005474,836005492,836005499,836005512,836005533,836005539,836005547,836005562,836005571,836005579,836005590,836005601,836005609,836005632,836005638,836005641,836005654,836005660,836005673,827893579,829680574,832167870,832846758,835109449,826600108,827486321,827661359,829153125,833270798,835369602,836171077,836312171,827920051,828302221,835579248,824492467,830240954,830476293,828751032,828896406,830585144,831247642,831750719,832403719,835373779,835418740,835419157,835419532,835419858,835420309,829464180,829600669,829806418,829806431,828487100,824499575,825320496,825977548,825978301,825989585,827197302,824822981,824946703,824946813,826531062,827880881,829075204,829898706,830064800,830081642,830081664,830081681,830173202,830238942,830238973,830359526,830360131,830360360,830360368,830360373,830360379,830360387,830482386,830705007,830786555,831763848,831987143,832167445,832354815,833015724,834576946,834822255,834834853,835417976,835418023,835418056,835953345,836112861,831198566,836304158,825155461,830852097,830852108,830904745,827430927,828042574,830783333,832212373,828520124,830063039,830116389,829899488,830356558,830379633,830379655,830382072,830382127,830382816,830870573,830871045,830877953,830892853,830893173,830893974,832311164,832874770,832877826,832880363,832881179,833444992,833460061,834980497,835100550,835164406,835174469,826078454,826082832,826083738,826085797,826090175,826105642,826105650,826105656,826105666,828112561,831725090,831725109,832460664,834563566,834700872,834715534,834718927,835072723,835073364,835073427,835073448,835073462,835073781,835073812,835073836,835076236,835082790,835082944,835083083,835093035,835093151,835097402,835097503,835320334,835329628,835364567,835425198,835425981,835426204,835506550,835507063,835811492,835811648,835811786,835933655,835933796,835933936,835934029,828873280,828953947,829008304,829041262,829049347,829057205,829523023,831948276,833528160,827870800,828251287,828335693,828457019,828457068,828457086,828457097,828457107,828457122,828457439,828457461,828457472,828457492,828457507,828457521,828457532,828457550,828457565,828457577,828457800,828457810,829949995,829950009,829950032,829950048,829950065,829950076,829950091,829950123,829950142,829950151,829950175,830076921,830080493,830080507,830080549,830080577,830080599,830080649,830450758,831590514,831590530,831590547,831590560,831590631,831590645,831590663,831590694,831590872,831591174,831591216,831591267,831591284,826491824,827968457,828070547,828070572,828070587,828070594,828119550,829617444,829617466,829675384,829675401,831289145,834450309,831677703,829946023,834863207,834863276,834863307,834863522,834913060,834913105,834913571,834913609,835248309,824579235,826100673,826737729,826867238,826869897,827126418,827393430,827522177,827584271,830132109,830132141,830132171,831234172,831234537,831234583,833817259,833817276,833817292,828012870,824791304,826058451,826394229,826420570,827014053,828360485,824704991,824708716,826974449,827303921,835942252,825565083,834721124,834721156,834721182,834721217,834721259,824535921,824968076,824968096,824968115,825392072,825392089,825392103,836182027,826149827,827799100,828183435,829847098,831550764,831777741,831796296,832764649,824679664,824690759,824691397,824693256,824698888,824720263,824729014,825154520,825262341,825269738,825299587,826008020,826407797,826407825,826456366,827627664,827630127,827665737,828363215,828369180,828410798,828591704,828934657,829019389,829036479,829450836,829461945,829461946,829466092,831500780,831501719,831515642,832987334,833217938,833233425,833233528,833233629,833233644,833233669,833264633,833415231,834212094,834237266,834294731,834467361,834471030,834473165,834516308,834529896,834568436,834574060,834715774,834718163,834718580,834718608,834718638,834723093,834751193,834762668,834789690,834812479,834822380,834996216,835053337,835197735,835204855,835379207,835477360,835562669,835564059,835572668,835614932,835831931,835979323,835980357,836072198,836079976,836080695,836081765,836089846,836089868,836089903,836090064,836090086,836090108,836280408,836599233,836600280,836652374,827726152,827884823,827922210,827922829,827923220,827934214,827934231,827934256,827934283,827934292,827934620,827934629,827934639,827934778,827934792,827934808,827934832,827934846,825890782,826717477,826717504,828088426,824592479,824592552,824592559,824592612,824592658,824593231,824593485,824594256,824596193,824596196,824596206,824597090,824622104,824622358,825707978,825708227,825716382,825723232,825723554,826176651,826176865,826176972,826258306,826258487,826311626,827040928,827471523,827649692,828325567,828325744,828325946,828327035,828338461,828359894,828360736,828361934,828362781,828371144,828526080,828526313,828529008,828560287,828563593,828822554,828822941,829249898,829282251,829294927,829947378,829947426,830731144,828683742,828683745,828745441,829805402,829840481,824507351,826724107,826861701,827309390,827309414,827309497,827309521,827309563,827687199,828578578,831328076,831328416,831328428,835965382,827061900,833892459,824700638,827602023,833433507,827108709,828229212,830501467,827528493,825292891,825292908,829524629,829524971,829524983,829524996,829525011,825226116,824999717,824999822,825256787,827622801,827808759,827998413,828078515,828448951,828463933,828463945,828463957,828464051,828577103,829642943,829676101,829874704,829875387,829875404,829875426,829875433,829875989,829876078,829876086,830300657,830423900,830423906,830423925,830423937,830423948,830423963,830423974,831191493,831215373,831529835,831529847,831529869,826005385,826647660,826732044,827729708,828245379,828263022,828822028,829905432,830785751,831418223,831418461,831897582,832225822,832237206,833417591,830243935,833321300,835051292,824650572,824650592,824650611,824650623,824650640,824650792,825245416,825245594,826737402,826838935,826838997,826951512,827545708,828283257,828482658,828487833,828732797,828732811,829437613,830106811,830806462,832449654,832982576,832991910,833968990,835266401,824574955,826235381,826247922,826510167,826512719,826766508,827803363,828170744,828171971,828303317,828303476,828305223,828825957,829351980,829370176,829410624,829411131,829901381,829901871,829901918,829902068,829902070,829902072,829902103,829902119,829902124,829902127,829902582,829903242,830784779,830794294,830859758,830863667,830949233,831438295,832260210,832260736,832260752,832260760,832318698,832325181,832325862,832922166,833302535,833455250,833463647,833466357,833524748,834606975,835186630,835187065,830867359,830867364,833775487,833781210,833785945,833811231,833818426,833968223,834126308,834132619,834140511,824772212,826180954,826341004,826685410,826685462,826685507,828904011,829261007,829261014,829261045,829261108,829415620,824820776,828935645,830551703,824969702,824998635,828313937,828316242,829977652,830079808,830086591,830156494,831454743,831454746,831454747,831454757,831454767,831454772,831454774,836690478,831640980,831641002,831641045,831641071,831641127,831641159,831641191,825478390,825527231,825530804,825660773,825925747,828408260,829630103,830517138,830517933,830568100,830587157,830808506,830808809,830809474,830809575,830809657,830809751,830809863,830810008,830810100,830810266,833637417,833637548,833637617,834120789,834188446,834188541,830448476,831464600,831895987,831895989,834191852,834205603,834739394,834743978,834744284,829911948,831597028,831598847,831606981,834612662,825282672,829200606,832200615,832200667,832200707,832201481,832201514,833203290,833279131,834931193,835270578,835270579,835365335,835762412,835999753,835999786,836002267,836002696,836002716,830106953,830106991,830107026,830107056,830107094,830107139,830107169,830107252,830107288,830108020,830108101,830108144,830108237,830108280,836452361,836452379,836452386,836452389,829068137,831809128,832295012,833982402,833987029,833987044,833956411,833959942,825066668,835560994,824523008,824657868,824660530,824686812,825176952,825872836,827408970,827446763,828477261,828700521,828736651,828864160,828870841,828875823,828878205,828892452,828902145,830156495,830315752,830327827,835395255,830856281,825655681,825656509,825656520,825656527,825656540,825656573,825656580,826717376,826717411,826717437,826717469,826735509,826735571,827250757,827250825,827250856,828215347,828306398,829451611,830579238,830579277,832358768,832358779,832358887,832359210,833539525,833540040,833540456,833540650,834726097,834726141,831103579,836475946,836477576,836477662,836479890,836959164,836959361,834402247,834661532,830694218,828104700,828105036,828395267,829264818,830534728,830534983,830880666,830894251,831257657,832083424,832184413,832304138,832305095,832305103,832305106,832305124,832305130,832305149,832305545,832305556,832305640,832305644,832305652,832305660,832305844,832305848,832305858,832305864,832305867,832305878,832305883,832305891,832305899,832306120,832308791,832480990,832881451,833217282,833467014,833467225,834066465,834066485,834404713,834505536,834506065,834683105,835042765,835292139,836245158,836245187,824648620,824649817,824649853,825304024,830530788,831951181,831958166,832135268,832157518,825294667,829261172,834861334,835047706,835128402,824917779,825098460,825267341,826581146,826581306,826595611,826595814,827748759,828204079,828528259,829752815,830096640,830156960,830411438,830946188,830947710,832209937,834113764,834690719,825182003,825190813,825272405,825274119,825275761,829794268,830497307,835301815,835301823,824482117,824870222,826228878,828602975,831247114,830389389,825946916,826794692,827860814,827860827,827860842,827860856,827860862,827860878,827860896,827860917,827860935,827860954,827860964,827860978,827860992,827861009,827861019,827861023,827861629,827861641,827861657,827861670,827861678,827861693,827861700,827861711,827861719,827861733,827861746,829232621,829232641,829232659,829232673,829232707,829232734,829232746,829232762,829232778,829232788,829232801,829232816,829232845,829232864,829232889,829232907,829232925,829232954,829234093,829234121,829234133,829234176,829234196,829234240,829234259,831679943,831725398,829301619,829386104,827673054,827674076,835842995,835850935,835880398,835902277,835911454,835913705,835913993,835914324,835915310,835915395,835918012,835922084,835923813,835924101,835925167,835930904,824609127,824609130,824609135,824609137,824609167,824609172,825640726,826554035,826554048,826554054,826976874,833558747,834609354,834609390,834609391,834609393,836216076,836216144,836499975,836499978,836503273,836503643,836503646,836509500,836643432,836661247,836676532,831344164,831344192,833243514,833830735,833850027,834303278,834305675,824577679,824578124,824671875,824671876,825594475,825610862,825716669,826287004,830063674,830296826,832409287,825049129,825069270,825069273,828653705,831854950,831863835,831922949,831934767,831936760,831937086,832200590,832219418,833322247,833473876,833473892,833474132,835774502,835808101,835901074,835962396,835962494,835964738,836102534,836174760,836526788,829843660,824830997,824831095,824833316,825805297,825806773,826672688,826674038,826247881,826955297,827145171,827145233,827188006,827980805,827991992,829339845,829606711,830264597,831153436,831176003,831191275,831194335,831374608,831460646,831463880,831784633,831793948,831802637,831819016,832185943,832212178,832220438,832265920,832698226,832704630,832730489,832804553,834106447,834107709,834220543,834223236,834229161,834469892,834505289,834505751,834506145,825656228,825661535,825662500,825075805,825077305,825916111,826431773,826445189,826453721,826457629,826554826,826557089,826567509,826569913,826575695,826577738,826585947,826591745,826604773,827093023,827102869,827118035,827124924,827137734,827146908,827569569,827586786,827589709,827595975,828614129,828776335,828776620,828779344,828780646,828783477,829694724,829702977,829718048,829831601,829842076,829850028,829858597,829860143,829864594,829868617,829871725,829873717,829875030,828460774,828666220,828454412,828503605,828520686,828541549,828553484,828559546,829048546,825167920,825167926,826148768,826158267,826496170,826496184,827377684,827377703,827467412,827535547,827538061,827540114,827540128,827540159,827540183,827734038,827734051,827734056,827734197,827734202,827734206,827765145,827765313,827769381,827769384,827773656,827773789,827889574,827889584,827889592,827908282,827989191,828099312,828641509,828641526,828641551,828898248,829697878,825214169,826319630,827286891,827318089,834814454,834814471,828534213,828534268,828534386,828534412,835192760,826820054,826821254,831655859,831771785,833344272,833356897,833721646,833726744,836052919,836054788,836057060,836107703,836109914,836112204,836115264,836115796,825243254,826390199,824833709,824833834,824833889,824833941,825345601,831701461,831989445,832866827,833539263,826155571,827490201,832766490,832910815,827556509,828197839,828651765,828696924,828860168,828860265,828891903,828891924,828957183,829477449,829824050,824671617,824671688,824691303,825214233,825214245,825214251,825214267,825267680,825335640,827458550,827554053,827554120,827554178,834615673,825809107,825813054,829715455,826420614,826724423,827207685,827586118,827920042,827920063,828021650,828184252,828184264,828188228,829384263,830588729,831047978,831091230,831163348,831163399,831163492,831431545,831431550,831431558,831431575,831440606,831440607,831471850,831867929,832071131,832376569,832508084,833121252,830249878,830970328,830970343,830970353,830970367,830970380,830970400,831258520,831258539,831258738,831258754,831258832,831258848,831258929,831258942,831496107,833640092,833640218,833640254,833640271,833731459,835635209,830748173,833420606,833420614,833420622,833420635,833420650,833420664,833420675,833420684,833420695,833420708,833420722,833420743,826437345,828458742,829192056,824650508,834999653,835440569,824500651,824711883,824782238,827180365,827207823,829975333,830065239,830065573,832175906,833002295,833005214,826206362,826448613,826493340,830989310,831814223,828736474,832722165,835658755,826141714,827342324,828151596,828151623,828523730,829041499,829064034,829141194,829342084,829565069,829565112,829716611,829717042,829717056,829717080,829717097,829717112,829817769,829873871,830000632,830141099,830214541,830214591,830214641,831086502,831086522,831086555,831086577,832309018,832309033,832309946,832507795,832757134,832939891,832939903,832939927,832939955,832939974,832939987,832940001,832940026,833109568,833110865,833110883,833110925,833110955,833111002,833111032,833111061,833111108,833189773,834450274,834450372,834450397,834450908,834450969,834451276,834451677,834452070,835881286,824826563,824864311,826443960,826444122,828238317,828254225,829383612,830982690,831254684,831946296,831949538,832252734,832252745,832252766,832252778,832252785,832624634,833077093,833250154,833573816,833573902,833574023,833574039,834277539,834782765,834888218,834888232,834888250,835580045,835580299,835979005,836035872,836035909,836036017,835565813,835568582,835568613,835568633,827001199,827001368,829201252,829799477,825275746,829839590,828278754,829154365,830496204,831369249,827999912,828199198,826422858,826422941,824589058,825997503,824534146,824536296,824597300,828117373,828611684,829565648,829900051,835390459,835391947,835403381,835410811,835412040,835412041,825404655,825409777,827447609,827898299,829582407,829593837,825649422,826204283,826958990,830857920,831495040,831495046,831495175,831788681,831925026,831925052,832074244,832096161,832096173,825839621,825840066,825840070,825840422,832486814,832980311,836106446,824816128,828807335,829903651,830701671,830701689,830701698,830701711,830701728,830701736,830701761,830701789,830701820,830719099,830748621,830867018,830867035,831443122,831443131,831443134,831546051,834641268,835006052,835006090,835006135,835006211,835006244,835006298,835006325,835006354,835006383,835613034,835613040,835613045,835613050,832329599,832332760,832915312,832916826,832921203,836958143,836962028,836963797,836964931,829244243,829478939,824806877,824806953,828695449,830501229,830517250,830517286,830517308,830517326,830517350,831647715,833267194,833267203,833267214,833267233,833267248,833267257,833267306,834617993,836913244,824859606,836266139,826547585,830078569,833705354,834151811,834859382,834940903,835127583,835638033,836068415,830586809,833317001,833341757,826456216,827569919,827569979,827570047,827570139,827672794,828055105,833518652,833518681,833518699,833518714,834077906,834937117,828494232,828898933,826314784,826340049,827389911,830801119,830809132,831451236,831451503,831380911,836273983,836284055,825576521,825692741,825699794,825699807,825700545,825700553,826841509,826841650,826842179,826910871,836117088,836117107,836117128,836117378,836117398,836117409,825167555,825167608,825167623,825168023,825168038,826480127,826480152,826480194,826480221,826480252,826480272,826481179,836668810,825340276,828623664,828630321,828631315,828631320,828633591,828633598,828888739,828996420,830829952,827624971,827625013,827625046,827625074,827625089,827625104,827625177,827625216,827625237,827625270,827625293,827625353,827626389,827626410,827626432,827626445,827626468,827626570,835039767,835039781,835039802,835039825,835039860,835039877,835039900,835039921,826314846,828197034,828566859,829894482,831279007,831863022,835963355,827545407,827630565,827630582,827848968,828004991,834194617,825085530,826069975,827192199,827895598,827959686,836056539,836103727,826731589,826934422,826934467,826934498,826934522,828906919,829142429,829531267,829741788,829742190,830073663,830166717,830794629,832981371,833671868,833956084,833340556,834050348,835113272,835114114,835114131,835114185,835114202,835114221,825940691,825940870,825948549,827739903,827758877,830227260,830596113,830602372,830648349,830817251,832237099,833201202,824956969,825639839,825671043,827136461,827684387,828306953,831473228,831475457,831482929,831486627,831566903,831951359,832454334,833117312,833818601,834862723,835139076,835591739,826616343,830706472,831239818,831248326,832684224,835736461,836332340,830189824,830190320,830192523,832049634,832820770,834056841,835544610,835550176,824589059,825049908,825610980,825834289,825878986,826638371,826640254,826660838,827020090,827061460,827181271,827190966,827428550,827713432,828040207,828073105,828174245,828215349,828228579,828285995,831403938,831606336,834532234,834584298,834584409,834584418,835383671,835747178,835867339,826445138,829625804,830727883,831230939,831706813,832527057,832620407,836267521,825171322,833521848,833997584,834983354,836018283,836461589,834568605,824948267,826913658,826920248,826920514,827935612,827936069,827936866,827937815,827938049,836010924,830907296,831251533,833724043,827970945,830746903,831375888,831760531,831760553,831868840,832040915,832040926,832040966,832130555,832214513,832214563,828796776,829380784,829385526,836268891,826448612,828389380,830694558,829729820,826504375,826684590,826684611,827092436,827103877,827128907,827133788,827153459,827154104,828037337,828303448,828311127,831623930,831947923,831949506,832126784,833421182,833421849,833978767,833987515,834356123,834965204,835621497,835783668,835783837,824501068,824501085,824501123,824501132,824501185,824572551,826384074,826107098,826222728,826801833,826801861,827603510,828248162,828248279,829011330,829014575,831436290,831706019,833319303,834272076,835318724,835320487,835506540,835959198,836362897,836776343,836786540,826314276,829273075,829379188,829379693,829552089,835084589,826658274,826658275,828785438,829313947,829369300,831879112,831879700,834630384,835159618,825686216,826024383,826119030,826120996,826228737,826229686,826565078,826590471,826632282,826632333,826646815,826879377,827211799,828170142,828808023,830856779,831876643,828223240,834953207,835133015,835133866,835133961,835134036,835134087,835134152,824573691,824573696,824573700,824573707,824573712,826064936,828499551,833179854,835898596,828948011,829596792,827051822,827526106,827526121,827526141,827526159,827526365,827526385,827526426,827526459,827526513,827526737,829656152,834460205,834145286,835007458,835007475,835007492,835007552,835257160,835672892,835673043,836078890,836078963,836079486,836079503,836079511,836079760,836079785,836079806,824650671,824650687,824650704,824650719,824650864,824650885,824650916,824896000,825268562,826900325,827051934,830340665,830369505,830411100,830496628,830597015,830602239,830613145,830623991,830643778,830646040,830659939,830670661,830670831,830707323,831181788,831183545,831193390,831264095,831398792,831497126,831544522,831563985,831705907,831711857,831823872,832302977,832416966,833543962,833630641,833630642,833637430,833644804,834004832,834120957,834337891,834347991,834399024,834703756,835683450,836451890,836555517,836565001,836565002,836724103,824615567,827988385,829438289,830172541,833264153,833470257,833470272,833470275,833531502,833558985,825087899,825836081,826653734,827741729,828220298,828499014,830735948,830736520,830747135,830915815,830915976,830917004,831133505,831273986,831419401,831440512,831562823,831564184,831590461,831723887,831723916,831727018,831738856,831853322,831853340,831860087,831863251,831863384,831865058,831866429,831871611,831871864,831878151,832010261,832056281,832126012,832127397,832128352,832131326,832318918,832319349,832553376,832656472,832906724,832912156,833381274,833386484,834536041,835974701,836189077,836189723,836288524,828450685,824787495,825665126,830461738,828440246,828440255,828440308,828440320,824774661,824792423,825162035,825242074,826693271,826719989,827339115,827390606,828672634,830064214,828031633,828278853,828376671,828460630,829636493,825741203,825741215,827013846,827161657,827161713,827533961,827590430,828729578,828790141,831729879,831730641,831730755,832107999,832183253,835064794,835064846,824544246,824557535,825805180,827744343,827746062,827928343,827982245,828013464,828777150,829120366,829741469,829741471,833988482,833999889,834003204,834013192,834013193,834013579,834303889,834564245,836244439,824628902,824681999,824711735,824972298,824972355,824972546,825255436,825262247,825291565,825317743,825476685,825709355,831973325,832158011,832164519,832173835,833477787,833478124,834122038,834122242,824867230,825929463,825932977,831201342,831205573,831767423,835043806,835049718,825644535,825234383,826024876,827024598,828039710,829457287,833188004,835380011,828606570,833597855,824920931,825251817,828963413,830187379,832848081,828091035,828099886,828101354,831283922,827536851,827548697,827551863,828407092,828414365,828606805,828892772,828892842,829020477,829252195,829317476,830999453,831107300,831132364,831136248,832268101,832268108,832404594,832429680,832434787,832975782,833415242,833712732,833836310,833854844,834551867,836218401,832525473,832794415,834537223,834655983,834746904,834799846,835841185,829053738,827322379,824572537,824573299,824573331,824573340,824573343,824573359,828648056,828682892,828732773,834600702,824677911,827312971,829138444,829156637,830130927,830507549,830507575,830507590,831571436,831572261,832236113,832236122,832236138,832236162,832236184,832236200,832236236,832236248,832239306,832240319,832961621,832962570,833936605,833937167,834148574,834148644,834148669,834148979,834148999,834149026,835422998,835423566,835423603,835423624,835664315,836565370,824469516,824664448,824718549,824794500,824799979,824800928,824920215,824921818,824928349,824931075,824942458,824942459,825090556,825091897,825094728,825095229,825100239,829839614,829840572,829844160,830109292,834254554,824837857,825281398,825281631,825281666,836643316,836643362,834891597,836284452,827782335,829802987,832216439,832325447,832325450,832325458,832325461,835236599,825515638,835624507,835624515,836054924,836067379,836067401,836067430,836067448,836067497,836067516,836067541,836067555,836067569,836067588,836067616,836067629,836067642,836067654,836067665,836067689,831657180,831688397,833571503,833575154,833575178,833575206,833575221,833575247,833575271,833575303,833575321,833575341,833575359,833575375,833575411,833575435,833575447,833575485,833576264,833576278,833576308,833576331,833576351,833576372,833576390,833576433,833576469,833576486,833576511,833576534,833576561,833576586,833576623,834403995,834404082,834404157,834639414,834821923,834821937,835124019,835124149,835124158,835124171,835124182,835124202,835124220,835124237,835124255,835578143,835578151,835578153,835578160,835578171,835578181,835578196,835578206,835578213,828689736,834050630,825392696,828501907,828666812,828706304,828710800,830659148,834191704,834405451,834623934,834639166,835251625,827563786,828526489,828947952,832584131,835347174,824544640,824556815,824777229,824777356,824777378,825090534,825092963,825271084,825404500,826053998,826054010,826370666,826423611,826554648,826592691,826592709,826749557,826749571,826749582,826968900,827576092,827576129,827978827,832219478,832236481,832239297,832239325,832239343,832239456,832240023,832240044,830410612,831181714,836127681,836129394,836233848,830409686,830413476,830413503,830415140,830415575,830419135,825066609,825227855,825242053,825242059,825398133,825398142,825398158,825398169,826058208,826791804,826791841,826843237,826843816,826848678,826926774,826945927,827281836,826683449,826348109,827309454,827309462,827309478,827309540,827309868,835934347,836127149,836130449,836440715,836440717,830304098,830578145,830606657,830663631,830786227,830791242,830807781,830838952,830846687,830861413,830867564,830876741,830884380,831233918,831360796,831393591,831460752,831467457,831468714,831477325,831483877,831488852,831495475,831615739,831643196,831664628,831706882,825322207,825626504,826099723,826730609,826730633,827684267,831452481,831470969,831644832,831719878,831720090,831720101,831720169,831720181,835597481,836749049,824721499,825150297,828597423,828597425,828789192,828789610,828789882,830105165,830105176,830105195,830105214,830105230,830105254,830105267,830105287,830105946,830106004,830106027,830106069,830106090,830106816,830106856,830106908,830106931,830106943,830107364,830107375,830107456,830107484,830107514,830107521,830107540,830107568,834558702,834558715,834558748,830353012,826461083,835307615,826472811,829231619,832911789,833208101,833208142,833208167,833208190,833208261,833208307,833208327,833208360,833208385,833213910,833213949,833214038,833214061,833214366,833214395,833520952,833520965,833520994,833521012,833521024,833521048,833521073,833521112,833521124,833521134,833521156,833521195,833521260,833521289,833521319,833521431,833521499,827684600,828057995,824496858,824753276,824770107,824780136,824785524,824789020,824825683,824849048,824977506,825044479,825065277,825320054,825340440,825356244,825372930,825451356,825455660,826575698,827089794,830942847,830942865,831014367,831014507,831014606,831014627,831014637,831014655,831416411,832265794,832357960,832359489,834475426,834904412,824510342,824565350,826151248,830428644,830726438,828778246,835083159,827910052,832976440,834235266,834729369,835823265,835824226,835824658,824672241,824695864,824697348,826716224,826746320,825979046,827827479,827829676,830479056,833965158,833965183,833965202,833965219,833965235,833965249,825982855,825995012,826022177,826033204,826037900,827118038,827124880,827131401,827133631,827135772,827146910,827565889,828142156,828175989,828181699,828190561,829989485,830431565,830792661,830813105,830827192,830830631,831231224,831744345,831782055,831783218,831785411,833930252,825982856,825995013,827105537,827124881,827133632,827565891,828142157,828175990,828181700,828190562,830813106,830830632,830843765,831162407,831782057,833374999,833377071,830246441,825120579,831312517,831312562,831312602,825539650,834253750,834304775,834873706,835904343,835906363,835909891,835914645,835915045,828643615,833512973,833678078,833678097,833678147,833678852,830668432,833665128,829878698,831518783,831522696,826495861,830438444,830954551,830954572,830954589,830954983,830955022,830955033,830955044,830970580,824507974,835933244,835933382,834458313,826684253,827799511,827809913,829993308,829995299,830001488,830001754,830001798,830001828,830010520,830401045,830462320,833940793,835659076,835971788,836880311,836881028,825182815,825345194,827968395,827968408,827968422,827968440,828510669,828560604,828885228,828885243,828885251,828885267,828885288,828886066,828886089,830459473,831605702,832047261,832048104,832052849,832056705,832059500,832423920,824471415,824800019,824808906,825318245,825404521,825775406,835819074,836100219,836100236,836100250,836445003,836446487,836446514,836446536,836446574,830642602,825821569,830521671,830521703,826345077,826345123,827393285,827550154,828867328,828867421,832056605,832743195,835653609,835653726,835678612,835678956,835679776,835684511,825307950,828265704,831864670,835173829,836395023,836442698,836442706,836442709,825013002,825610532,825817221,825820018,826922200,827018400,827023906,827131072,827160502,827179659,827188468,827711304,827714477,827753214,828124296,828461265,828951490,829006788,829457262,829576248,829742461,829769715,829809628,829809673,829875547,830022514,830191089,830207011,830243090,830471376,830481780,830534793,831087808,831699726,831699894,831702977,831704451,831705389,832543987,832752918,833188006,833981910,833983384,833983409,833983475,834003201,834008482,834010840,834214932,834723491,835299086,835299136,835308583,835521209,835744764,835744900,835910263,836310768,836915418,836930068,833640101,832776848,833256470,833256474,827881878,828655553,832145110,824967926,826028359,826028477,826028496,826037165,826047327,826047339,827427958,830032451,831007027,832112762,832112823,832112926,832112957,832113003,832113029,833386895,833386910,833386953,833387023,834119275,825273562,826229900,829693799,829746307,831718254,831718283,832140786,833165821,833473872,833548052,833817981,835751256,835751278,835751303,835751315,835751343,835751384,835751442,826064976,827306468,827338362,831714792,831730566,831974007,827614525,831809164,833802095,824650780,824650793,824650811,824650827,824896118,825151749,825237498,825237521,825237534,825237552,825237577,825286752,826342977,826346248,826346312,826685868,826686042,826686052,826686056,826686063,826686074,827249670,827487997,830196965,830684944,830699751,830713224,830772790,830774553,830876742,831277241,831825776,831831289,831837713,831848600,826761378,832180397,827808630,831054639,836194404,826732802,827702958,829691027,832436404,835899462,835901786,835902159,835903007,835904599,835905698,835911672,836425814,827101490,827250712,827262332,827561624,827576047,827589747,827646614,827818954,827822266,828912507,828924166,829317428,829347191,829502683,829921518,830300766,830550880,830994559,831121435,831137472,831239516,831309926,831313537,831321609,831327822,831348265,831352658,831854753,831900697,831998226,832543808,832736357,832788739,833171072,833195475,833766080,833999230,834010352,834018799,834020793,834023167,834049811,834062447,834072814,835067486,835090547,835112105,835647572,835649672,835733328,824971782,825455261,825556252,825560770,826043221,826055610,826066341,826069272,826075420,826471666,826483441,826488614,826488639,826488653,826488665,826488683,826488708,826488725,826488749,826488775,826506751,826603220,827038766,828086467,828652018,828652051,828652088,828652116,828652139,828652165,828652181,828652194,829799390,829799405,829799437,829799454,829799508,829799539,829799560,829799589,830501477,831036335,831036373,831036386,831036408,831036440,831036462,831036484,831036510,831036525,831821591,832273793,832273808,832273820,832273834,832273859,832273879,832273890,832273905,832273918,832427081,832461966,833427002,833427036,833427048,833427069,833427076,833427088,833427095,833428134,835111703,835111739,835111756,835111802,835111823,835111866,835156637,835244100,825476249,824934215,824936491,825983352,826751406,827111407,828835299,830922685,831150014,833971585,825063929,827328408,827546968,827547037,827808524,830970408,830970426,830970448,831017901,831020534,824502048,825107033,825107042,825107055,825107126,825107141,825135584,826406073,827859256,830983320,830983517,831519522,831918915,832334367,832334387,832334400,832932289,834094503,825072975,825189618,825195324,831099855,831127366,831127549,831127984,831462269,831462479,831462597,831470677,831470689,831470699,831470706,831470760,831473312,831473450,831473454,831473468,831473478,831473694,832033136,826012323,829941322,830463799,830632853,831320382,831320420,831320436,824802767,825208541,828146309,831112111,831466296,831466301,831466308,831466309,831466310,831466319,835478019,831568388,824640421,824648672,824714349,824790638,824793134,824793151,825642868,825643359,825683489,825687525,825694842,825802532,825809357,826131886,826132037,826137364,826143118,826144572,826153327,826182770,826182771,826291727,826295977,826376825,827143379,827181413,827311156,827313250,827995542,828004693,828014615,828015796,828469768,829181370,829935109,829938175,830460938,830682833,831467595,831560283,831573477,831953233,836091687,836093990,826575291,835133974,836606825,836608028,836608436,836619815,836620214,836698542,836721591,836726710,836776467,836779125,824926405,825285311,825327392,825585831,826083754,826270421,826843352,829166262,829546248,826481563,826913234,826914675,826919044,827044268,827428594,827910003,829575816,829575871,832110271,832813984,832814795,833136957,833139835,833145088,833150795,833151125,833168807,834805437,834809732,834810096,834812674,834813280,834895710,834896267,834960654,834965189,834973196,834979889,834980606,834986505,835296121,836292881,836791814,827889428,833428750,834168779,835315211,835463869,835464340,835933864,825063712,825095681,825947235,827648566,833792623,827257686,827654869,827101493,827117057,828011130,828029598,829451942,830971730,831984407,831986540,832310541,835584386,826107007,829871733,831978950,824490234,824490254,824557512,824565348,825003058,825424509,825433323,825442290,825444050,825444601,825896635,825938165,827535833,827544594,827550765,829540484,831045466,831649037,831673116,832149387,832845366,832845399,832845413,833224149,833235111,833479018,833479079,834080851,834316877,834316900,834316963,827097681,829574687,830444898,831275063,834120918,836089656,832750727,832798253,833334251,834011280,834015289,834130326,834149108,824784647,824873636,825640002,825944604,827680616,827776610,827776972,829606827,829947736,830586580,830987284,831095551,833739788,834503144,834503354,834698891,835215005,835292818,836190648,825495530,825502910,826021491,826988073,828088401,825671477,827641619,827641620,828544877,828933859,827135078,830983580,834885952,835484209,835485556,836054129,836066637,836080306,825053271,830473087,831466717,831976848,831985363,827802714,829698117,830101748,830126316,833287422,833293672,833299449,833304112,835704612,835706238,835706866,835706875,835707772,835708455,835709351,835711155,835711267,835711290,835711327,835803216,836379278,824836443,824907770,825130572,825719037,826678317,826678341,826678445,826680486,826768292,826768680,826774298,827026795,827047022,836669383,825964577,826585576,826645232,826978814,827028011,828225260,826720988,826721006,826721036,826721301,826721375,826721406,826721430,826721464,826721494,827481914,827481934,827481945,827481990,827482003,828193563,828193697,828193721,828472898,828473281,828473301,828473314,828473328,828473342,826453533,826453534,826456373,827581117,830698178,830781809,831933281,832352090,832687274,832688885,832743431,832968573,832969020,832969247,833373059,833373211,833406130,833407266,833550574,835947532,836722294,827648301,827648305,827648307,827648308,827648314,827648316,827665759,827665760,827665761,827665762,827665763,827682182,827682189,827682226,827682272,827682277,829755356,829755357,829755360,829755361,829755362,830272032,830272033,830272035,830272036,830272037,833315286,833319190,835886910,829116557,829116579,829116595,830950279,825137705,825854396,826632129,830804627,831440412,831443007,831443132,831783588,831784421,833395321,833442151,835381425,835589201,835626725,835732600,835758664,828379500,830360069,830360073,830360076,831440265,831440445,831440449,835408026,835408027,835408029,835408030,835408036,835408054,835408060,835408062,835414826,835414830,835414835,835414839,835414882,835414886,835414890,835414905,835414927,835414929,835414931,835414940,826176747,833492796,827194797,831167787,825776332,825782739,830284399,830590146,829078575,826313748,826332989,831313952,831317733,831319866,831724065,831795069,831992098,831998529,832539697,833388188,835049562,835074656,835277705,835277868,835278057,836157786,836165894,836166182,836629728,836634876,831568309,833445509,833715609,833715617,835275429,835275436,835275526,835600100,835600763,835600826,835697296,835701271,835794372,835825150,835827561,835839699,835854263,835858529,835980416,835984632,836008792,836010578,830332494,833482470,835082605,835140438,835140467,835471141,836032414,836032428,836032442,836574026,836574065,824634057,824640453,824643285,824648721,824649137,825643497,825643911,825807425,826135852,826136483,826144738,826730998,826733472,827997699,828458291,829189957,829934973,830127585,830687285,831273101,831464857,831464869,831464874,831465116,831573295,831574012,831574013,831574655,831577154,831951527,831951533,825198030,825198080,825198156,825199671,826111580,827463960,828361370,828361382,828361405,829290551,829377943,832276780,832297366,832625058,832875541,833368400,833723500,833967568,834001353,834001362,834001367,834001537,834001545,834001565,834043474,834540095,834546442,835614108,835977340,835988176,826110179,826122708,826298330,826362460,826603473,827743849,827745320,828188618,831784393,835307349,835315639,824482082,824654738,827238473,827238480,826849214,826855306,826855329,826855396,826855412,828031316,828031527,828471882,828904019,828904026,825319061,826715630,826742014,827359659,829133958,829851584,829855655,829856705,829942983,832125408,834843607,835928512,830966476,830966545,830966564,830966876,830966898,826029305,827790517,828316311,829281216,830176828,830442832,831564738,833493826,833593917,833968566,833996925,834411835,834688785,834688973,834688985,834688997,835506762,836561604,836571415,836571644,836895925,836895945,836899534,836972208,836973600,836974148,836979915,836980068,836980843,837001742,837065417,837066351,824488332,828649199,827205560,828179905,828387611,828387637,828387661,828638872,828638889,829040505,829040522,829064592,832234228,832276446,833277784,833277799,833277810,833277852,833723596,825138991,831443111,824947162,835381636,825137699,825416919,828777703,829346305,829736202,831334908,832287499,835726730,827491310,831212713,832802280,834609376,835456784,835457202,835457667,835458050,836106127,836106136,836106141,836106150,836106255,836106266,836106268,836106282,836106290,836106303,836106316,836106328,836106341,836106349,824513321,825635064,826196105,826887050,826919307,831536916,831541826,835626425,835626547,825710922,835811637,835690244,836370138,824780889,824922685,824922729,824922896,825212627,825404901,825706231,836150422,836150438,824735641,824735668,824735699,827720040,827720452,827720462,829637831,825102887,827117284,828139750,828717812,828717839,830773424,832192261,833341468,833343124,834510435,834546173,827640493,828030033,828035946,828916666,828957044,829813126,831494665,824568259,824673606,824673621,824673643,828541823,829991075,830827667,830932009,832243853,832243865,832243880,833078240,833745104,833745536,833748420,834327397,834328880,834329717,827689878,829370142,829405154,830382059,830428911,826893060,826893318,827047359,827198413,827198444,827200522,827636163,827636172,827636173,831280760,831821743,831821756,831848991,831871301,831871407,832030554,832030809,832129115,832129156,834033645,834033880,834033947,834034415,834035380,834035462,828198497,828198629,828198821,828692107,830122496,827601996,827602206,830951598,831000938,825385212,826063003,828381519,828483880,828483901,828483927,828701256,828743438,828983274,830145713,830145722,830411385,830411390,830411392,831050030,833106652,827973206,832497766,834478465,834478486,834478523,834478649,834990370,830117848,830117946,830948246,830948346,831089168,831209256,831449144,831904677,832416262,832416695,832416855,833273933,833277394,833277424,833724340,833725055,834117517,834120524,834120847,834120900,834633943,834633989,834634305,834634443,834645501,834654034,834906717,835648435,835648842,828933806,829591456,827409518,828838954,824563477,824700519,824738710,824783002,825887894,824911135,825110780,826484124,826587547,835366720,826743138,826919707,826919974,826920071,826920665,826922253,826941337,826942923,826944266,826944267,826945050,826946343,826947347,826947464,825600669,833331796,833860136,833860161,833860246,833860311,833860349,833860377,825082575,830431563,830431636,830431663,824744395,824901899,825950850,825957567,827015876,827016590,832437141,832485805,835668030,836158802,828523197,828545346,828550109,828558442,828559387,828560559,825239916,825566013,826157719,827116059,830533464,830616970,830945583,834070079,834071830,834072750,829733921,836097338,832198809,833375905,835565598,835565607,824527250,824527257,824527277,824527284,824527314,824527345,824527362,824527883,824527892,824527903,824527917,824527942,824527951,824529881,824529887,824529900,824529911,824529921,824529928,824531232,824531352,824531360,824531796,824531814,824531841,825560030,825560039,825560047,825560052,825560772,827096866,827347937,827372245,827552828,832567998,832569685,827614735,828201430,828500318,828500327,828500355,828500382,828500395,828500406,828500706,828500716,828500735,828500754,828500774,828501601,828501612,828501622,831161041,831161075,831161084,831161113,831161133,831161148,831161182,831161196,831161213,831161228,831161268,831161294,831161301,831161339,831161360,831162248,831162256,831162280,831162307,831162322,831162356,831162440,831162459,831162476,831162500,831162522,831162537,831162547,831162560,831162578,831162592,831162609,831163339,831163378,831163401,831163410,831163504,831163518,831163533,831163558,831163571,831163587,831163600,831163612,831163971,831164185,831164199,831164207,831164224,831164230,831164241,831164507,831164515,831164548,831164560,831164568,831164581,831164591,831164605,831164617,831164643,831164656,832528275,832528286,832528295,832528301,832528316,832528573,832528708,832528718,832528727,832528739,832528749,832528763,832554830,832554842,832554852,832554862,832554867,832554876,832554886,825325305,824958240,824967392,824967414,824967519,824967536,824967550,824529761,825979466,827370860,829124622,832240423,833079183,835304284,825157761,826747894,827286636,828060480,828061376,829461227,829919636,834375752,835236580,832367748,832367841,832367895,826920230,835767924,836530093,828743077,826062554,831169011,825275209,827772212,831440750,835844802,828184793,828184803,836191026,836195266,836200528,830479060,824527238,824529973,827359862,827386018,829314229,829317167,829333301,829754969,829760966,830134274,830138878,830144540,832647614,832664034,832729522,832729641,832737719,832737924,832748969,832760486,824540575,825047294,825047303,825047317,825047325,825047338,825047357,825047384,825048017,825389343,825391901,825391920,825391936,825391957,825391964,825391975,825391989,825391998,825392010,825392028,825392050,825400016,825400408,825400942,826137054,826137064,826207851,826207860,826207872,826207895,826207912,826215883,826554039,826554046,826594847,826951158,827369671,827369687,827369744,827552964,827553076,827565814,827839201,827894400,827894407,828363624,828363870,828831211,828831215,829582995,829583012,829583037,829583053,829583075,829583091,829583120,829909793,829909794,829909795,829909796,829909798,829909801,829909802,829909805,829909806,829909808,829909809,830317964,830500608,831098479,831098490,831098526,831472074,831472150,831472167,831472175,831472197,831472263,831472274,831516821,831517734,831517851,831517969,832510390,832510404,832510430,832510445,832512068,832523924,832523953,832523971,832524007,832524021,832866835,832866888,832866892,833322445,833523903,833725563,833725580,833727216,833727587,834170563,834170766,834174026,834174293,834174772,835952959,835952993,835953020,835953823,835953831,835953850,835954083,835954146,835955262,835955328,825301232,825301268,825301278,826769423,827680432,828166274,828188589,828200629,828204781,831312855,824527576,825235046,829779457,830908848,832283975,833349795,833418522,833685419,833688877,833692147,833734970,833735107,833735147,833822746,833823708,833888032,833991114,833991618,834016468,834071754,834076871,834623384,834714861,835836878,835851249,827190968,828609438,828621867,829706473,830609399,830626618,830630183,831727996,834476269,824976371,829584986,830874321,831089824,832641771,834569466,828443382,829507605,824501557,824932103,825036072,825040904,825048049,825048915,825062368,827482697,827680433,828063940,828159679,828178981,828188590,828200630,828204782,828208994,828212516,828213935,835050205,827732288,829049651,828333572,829249609,829651365,829981443,830073268,830440717,830697746,830722310,830918749,831487514,831487530,831487541,831487555,831487567,831487571,831487589,831487601,831493327,831493347,831658725,833043040,833092867,833093865,833097986,833098031,833098050,833098070,833099090,833099111,833099275,833099315,833099352,833099375,833099393,833100133,833100157,833252155,833353295,833542926,833760437,833767697,833790734,833847769,834043963,834043971,834092036,834099078,834130226,834130256,834130853,834173216,834173229,834173250,834173273,834173293,834173315,834173321,834173344,834173360,834173387,834173410,834173435,834173451,834173462,834173476,834173506,834174876,834174895,834174926,834174945,834174967,834174981,834175000,834175043,834175062,834175075,834175096,834175113,834175135,834175150,834175175,834175215,834175237,834237523,834578006,836336701,836648999,831327312,826963614,828510472,828519610,830483424,830815821,830845890,832107010,832249027,832330869,833218884,833766766,833806644,834056260,834056446,834060535,834066448,834074038,834076353,834099218,834126900,834146495,834300178,834310165,834487800,834488747,834506416,834539624,834549480,834562131,834562142,834562161,834562188,834562216,834562232,834562250,834668816,834676880,834702896,834766855,834769696,834786898,834849205,835310362,835421115,835823650,835852750,835853312,835893233,835894284,825266383,825869635,825869695,825874226,825881677,825881956,825882075,825882176,825883611,826842870,830811335,833338778,833338810,833338856,833778591,824750864,824850956,834516573,836547718,836547726,826762941,826768772,826775554,826781193,826798491,826802301,826862666,826864423,826866216,826868894,826869139,826870951,826871992,826872363,826872457,826873608,826881554,826917874,826919761,826929510,829571329,829573729,829914712,829914726,831750511,824911469,825136249,832455699,835335111,825087418,826233698,826615238,826621996,826629389,826633698,826638373,826640255,826766637,826775750,827021371,827026156,827110781,827121378,827165485,827170756,827300058,827346580,827364456,827569571,827576049,827595977,827646247,827646616,828166275,828166871,828174246,828181701,828188591,828967978,829008307,829032690,829829236,829850032,830563242,833933224,834559300,834564506,835347005,835624188,835624189,831109157,831109186,831109232,831109272,833559027,824606232,824633395,824633421,824633427,827129774,827131366,827252596,825492535,826991998,833456041,834430042,835608177,836420034,828688224,828688242,828688265,828688281,828688299,828688314,828688337,828688359,824629967,825242591,825246665,825706831,825725093,826150747,826704432,827227779,827242064,827961874,828304156,828310143,828314497,828328934,828332211,828336177,828336921,828833958,828842234,829452809,829452885,829912674,829915993,830438674,830934463,831462536,831472807,832366650,832408110,832966042,833293611,833300828,833301195,833304739,833309892,834060676,834660696,834663241,834676893,834682965,834694176,835190328,835192872,835639193,835642839,835645789,835647765,835655634,836068269,836083181,836985314,836985952,824804256,828519288,830043040,830311092,830640334,831589989,831980394,832044625,832498390,833164979,833933350,834352084,835212138,835483991,826150888,830410433,824585426,824589736,826062185,826124930,826191700,830436485,833120474,833120789,833234264,833304125,833842203,833842341,826403234,827702446,828029981,828537711,828750558,828762987,829158068,829163283,829163308,829163321,829163371,829251996,829254857,829411532,829411534,829578129,829578164,829578180,829578195,829578227,829679395,829748931,829748942,829748953,829748970,829748976,829779458,830753402,831676815,831676855,831676951,832317102,835364510,836268301,836268334,836268363,836675406,837083440,824980062,825954678,825954693,825958597,826453556,826458527,826582175,826582206,826968509,826980843,827017756,827601498,829084663,829107782,829108779,829213752,829213973,829841695,829851749,830034137,830756166,830834245,830858938,831817617,831818789,831835047,831835266,831835406,832252833,832258950,832259120,832263321,832263653,832347988,832350987,832393938,832968724,833285745,833367312,833406012,833406407,833407696,833523242,833537359,833550285,833555144,833555394,833556855,834094334,834098775,834099318,834199539,834653894,835012946,836374364,825381613,826618584,828387388,828593382,828809578,830097846,830986027,831557628,832394406,832751973,835270023,835994756,836292713,836446466,825486710,828394553,828394630,828394720,830400718,830400726,830400737,830400747,830400756,824678135,824740477,824756599,824800843,824801320,826300421,826349785,826436972,829971524,832918789,834049416,834049423,834049433,834049439,834049642,834050774,834050784,834050792,834050808,834050812,834050820,834050828,834050847,834050853,834051257,824985548,825005597,825383717,825411023,825418315,825441128,825464962,827901584,827961354,829169092,829615123,829661930,830249886,831177707,831586630,832854135,833379105,833661649,836312653,828272231,828272242,828272256,830156266,830156302,830156602,826617403,826618652,826619312,826717194,826720176,826856231,826860498,826871293,826871703,826872284,827994109,828331651,828370173,831367724,831724005,832525155,832594778,832595002,832596325,833338236,833351667,833510186,834797071,834797091,835052977,835055152,835290613,835297804,835312644,835312903,835437643,835809575,835977412,835977519,835977520,835985611,835994844,835994919,836856247,836859017,827601899,827602057,827602138,831066638,824833453,824833473,824834374,825300904,825322632,825322883,825330070,825343295,826535524,826545318,826545326,826554144,826554188,826554391,826783091,826791695,826791708,827093288,827093293,827125735,828328153,828500399,828833964,828834234,828843769,829452807,829452855,829642778,829642935,830036841,830041483,832494004,833302933,833303258,833303912,827389084,829190071,829747575,830569723,830575870,830580081,831023337,832985154,833491350,825285738,825296577,826147651,829272561,829272735,829434474,834252039,835038510,836244144,830280490,830289766,834417691,834419716,835369592,824709610,824716831,824716837,824716926,824717038,824717042,824717047,824717618,824717625,824717639,824717752,824717755,824717761,824932541,824932546,824932687,824932736,824932757,824967297,824967311,824967512,824967588,824967648,824967655,824967706,824967712,824967807,825010691,825309666,825396708,825408630,825770471,825788204,825789707,825789944,825789953,825789964,825789975,826060456,826111458,826218226,826453286,826487079,827308756,827464522,827464575,827464583,827904260,827909464,828080402,828181710,828228214,828271116,828416921,829129392,829197297,829211951,829376070,829378702,829536639,829557717,829613836,829616489,829642716,829689075,829689224,829971170,830602138,830610395,830621620,830622284,830877190,831083538,831084588,831421474,831426538,831430837,831691670,831710532,831713156,831844738,831854339,831862691,831992385,831998113,832013380,832059902,832071692,832071727,832071758,832276916,832276919,832276924,832277017,832277024,832277035,832277038,832277114,832845570,832864043,833045692,833103400,833125447,833175250,833176090,833247244,833247385,833247441,833787376,833804725,833889646,833970130,834000121,834005747,834200211,834339973,834529798,834742730,835175880,835258222,835413622,835874496,836196731,836300576,836312943,827986897,835754581,835754672,835754954,829679739,829683008,828585892,829613713,829903480,829989505,830708522,831446789,831568265,831568275,831568290,832230021,832302714,832665520,832698713,832701443,832749104,825668101,827996034,831590322,832189044,833097857,833097876,833098413,833098566,833098604,833098639,833098691,833098717,833098784,833098845,833098895,833098955,833098991,833099057,833099100,833099138,833099185,833099233,833099271,833099309,833099346,833100019,833100120,833100134,833100152,833100170,833100209,833100340,833100365,833100381,833100395,833100410,833100627,833100662,833100677,833100695,833100703,833100730,833100995,833101101,833101124,833101144,833101172,833101204,833101235,833101378,833101400,833101417,833101427,833101436,833101448,833101641,833101691,833101703,833101717,833101732,833101751,833745775,834021644,834935092,824724803,824727947,824753712,824753719,825151868,825891351,827866069,829475833,831924574,831924878,829588055,829588084,824819882,825271686,825271717,825271740,825271773,825271868,825271901,825271969,825272030,825921479,825962361,825962384,825962405,825962429,826090427,826097164,826097187,826098034,826098062,826102312,826102318,826102323,826349026,826883058,826886516,826888306,826888342,826888384,826888401,826888988,826889022,826889056,826889112,826889195,826889229,826894021,826896690,826897963,826898015,826898831,826898853,826898868,826898903,826986428,826986466,826998124,827155675,827155704,827213591,827213596,827213736,827256917,827684826,827685168,827685204,827685223,827750443,827750453,827750464,827750475,827750482,827750489,827750491,827750498,827750507,827778381,827778392,827778403,828622674,828624334,829293727,829381704,829492272,829602028,830161276,830161320,830162175,830162304,830162380,830356218,831042065,832277740,833372327,833432927,833613513,833660385,833660474,833661140,834081816,834621587,834885260,835137494,835210206,835236574,835267581,835271952,835318259,835328223,835468633,835468651,835468745,835585245,835589111,835680759,835705820,835711234,835713216,835771779,835794875,835803626,835810133,836013338,836014050,836014281,836014804,825927733,826571652,826915658,827331114,829288452,830142482,830862900,831666087,831666094,834265659,834866011,835855374,835855851,835855871,835855892,835855916,825175488,826745540,827122247,827299845,827851751,827979322,827979346,827979355,827979375,828315104,828315791,828438163,828438171,828438202,828438228,828439248,828953097,829043412,829088236,829877571,830269448,830654823,830892360,831270109,831526510,832305136,833318702,833415556,833415570,833977809,833977817,835222595,835483119,835554876,835554880,836071026,836071038,836977757,832836808,833344847,833344855,833344867,833344888,833344930,826720753,826731591,826990150,827276813,827472183,827851806,828029845,828033994,828554312,828763314,831825777,832214070,826651384,827188318,827260126,827260133,827260149,827260160,827375856,827469280,832744475,833464831,825122287,825122297,826812899,826820097,826879941,826932667,827191408,827367616,827470392,827470422,827488353,827488376,827488465,827880841,827906615,832552176,824896570,825200629,825200665,825200673,826899699,827054870,831433452,832039213,833892099,834251834,834584128,836195165,836452432,836452494,836454901,836454948,836454973,836455005,836455047,836455081,836455094,836455106,836455124,826508283,834184168,834184169,834184170,834184171,834184172,834184173,825077033,829604185,834558252,834995785,835006888,835010826,835034299,835048373,835052878,835056844,835067328,835078700,825084911,825090721,827845876,827847579,827849547,827850994,827851413,827877886,832528593,832531502,832531779,832535639,829695930,825385414,825385440,825385447,825386539,825386554,825386567,825386602,825386615,825712534,825712544,825712576,825712603,825712621,825712646,825890382,825890401,825890417,826398132,826398149,826398169,826398187,826955061,826955115,826955159,826955183,826955221,826955246,826955288,827366031,827366068,827366083,827366102,827973553,827973597,827973613,827974224,827988652,828378565,828527946,828527985,828528012,828528048,828528085,828528105,828528135,828528166,828535244,828539587,828552275,828552306,828779385,829102365,829534367,829534378,831010292,831010315,831010407,831010433,831010668,831010699,831995267,831995281,831995289,831995299,834359323,833329655,825244643,827934122,827934202,827934270,827935188,827935194,827935270,827935276,824913124,824925795,831205710,831520194,831849009,831854634,831866035,831871309,832100686,832317123,833964547,833983205,835028048,835039500,835160672,835345981,835355592,835368513,835380969,835396656,835491994,835576066,835579814,835613274,835964476,836061147,836061773,836065255,836852402,825570778,827580417,827815577,827817167,829521602,832118757,832266499,834821900,834821964,835124039,835124065,835124100,835124124,835124133,824799228,824799260,831725596,831725641,833811437,833811561,834298635,834314491,835296793,835296865,835296958,835297038,835681208,835685729,835700841,835709098,836196701,836208808,836294080,836336641,836952037,836953128,836958688,825334620,825337009,826466190,826469883,827646184,830661096,832781872,833827115,833830587,835005588,835901263,836604959,836606551,826355688,827512431,827513949,827514333,827589576,829471180,833012614,833391326,833391338,833391364,833391384,833391405,833759420,833759439,833759448,833759457,833759472,833759488,833759513,834210184,833080913,826949897,827216060,827224986,827232506,827238035,827238064,827238076,827238807,827238816,827256442,827256455,828798660,837059505,829339423,830409833,834621699,833503997,831973096,828020510,829256410,830756155,830960584,831367718,824757852,824833659,831282146,832000814,832622070,827816037,827816101,834511548,834537239,834537246,834537265,835555736,835555755,827493750,833552825,834185568,834267574,825081778,825081906,828519527,828519625,828519910,828519921,828520187,828520188,828520385,828521128,828523676,828532983,828544806,828604689,828978840,831013202,831013213,831013227,831013236,831013253,831013260,831013281,831013337,831057696,831057722,831057744,831120984,834539227,834539249,834539264,834587197,834587225,834587243,834587275,834587291,824644114,824524294,824538066,824538077,824538090,824538105,826648049,828601677,829681404,830032139,830435553,830440172,830969830,830969869,830983369,830983576,836131394,828023984,831989669,832403695,833047438,833757235,835250538,835250553,835250565,835250575,835250588,835250594,835250607,835250615,827925084,828108051,828108280,828108470,828108620,828510480,828518403,828531821,829245653,829292771,829292815,829292866,830796222,834351075,834949754,835312788,835313216,835314292,835338327,835341696,835341726,835343101,835343155,825642832,825652479,827796665,829905081,830424726,831446092,831447020,831878958,831879672,831879972,831880472,833502999,833503014,833504116,833504391,834051648,834053275,834053381,834053923,826490166,830551397,831352470,832193513,832193540,827289647,835998712,836023645,832429427,832430930,825559290,825559309,825559344,825559358,825559370,825559388,825559395,825559411,825559435,825559449,825559462,825559494,825559523,825559535,825559573,825559600,825559617,825559643,825559657,825559677,825559698,825559719,825559738,825559756,825559770,825559801,825559832,825559846,825559858,825559880,825559894,825559911,825559938,825559949,825559963,825559987,825560004,825560026,825560043,825560059,825560080,825560098,825560121,825560131,826643677,826643685,826643697,826643707,826643709,826643716,826643723,826643731,826643740,826643745,826643754,826643758,826643764,826643769,826643773,826643778,826643787,826643791,826643796,826643800,826643801,826643814,826643819,826643822,826643834,826643842,826643843,826643848,826643851,826643856,826643872,826643881,826643888,826643894,826643901,826643904,826643909,826643919,826643933,826643937,826643942,826643945,826643953,826643959,826643962,828649545,828649564,828649578,828649612,828649624,828649657,828649682,828649710,828649731,828649779,828649800,828649835,828649855,828649896,828649917,828649933,828649974,828650002,828650038,828650054,829828985,829829065,829829085,829829125,829829139,829829165,829829216,829829284,829829357,829829402,829829431,829829440,829829471,829829489,829829526,825264881,826507092,826710416,827311912,827746131,827844375,827990588,828220486,828221613,828290877,828735113,828735253,828749374,828796047,828799688,829844745,829844775,829844797,830431190,830431218,830496178,830595523,830982564,831363741,831363793,831363816,831369069,831369129,831369161,831369214,831369234,831369257,831369272,831369314,835287032,835994683,836002058,836708280,833467432,833467465,833467466,833467467,833467471,833467479,834289371,828402156,829748302,830558198,832688125,833151638,833640817,834258989,835334604,836257604,836506776,836506787,836953890,836954456,837010864,824523633,824523665,824523720,825003175,825003205,825033417,825033461,825218579,825500893,826146428,831741875,833789233,832300102,833553745,824908353,825128227,826583851,827131242,827797705,827817561,827817587,827817838,828145974,828145979,828145994,828244562,828269778,828274456,828456104,828456136,828456186,828583759,828642815,829524636,829524646,829524658,829547169,829684277,829684581,834063375,834173155,834173174,834216934,833096777,833288618,833291282,833291318,833560179,836141498,836230972,825290369,826086527,826345817,826392070,826392286,826765167,826765183,826765206,828578832,828891879,828910838,828912931,834490965,831818203,832449009,832628102,833319209,833975754,834724560,825135907,825135984,825144373,825275709,825276427,825330000,825373591,825401757,830674827,824829396,826352263,826375446,830070665,833213313,833213316,833213318,833213792,835505301,835581439,835727981,835729405,835730054,826738510,829668490,830757500,832041082,832041102,824750710,824754554,827102626,828981875,829125333,829125338,831760624,831760651,831760682,831760705,831760719,831760735,831760751,831760794,831760810,831760828,831760852,831760873,831760900,834809226,826190046,829437580,829437585,829988679,829988747,829988780,830125827,830128440,830162488,827030861,827152811,827713852,827744672,827749085,827749087,827768102,828015513,828018700,828236582,828239229,828242135,828265019,830334358,831251903,831280880,831338536,831348481,831371213,831385526,831388314,831412916,833433691,834578139,834578459,835996228,836053436,836063680,836063935,836064057,836064159,836064297,836064947,836065104,836065373,836435196,836437835,836454164,836456548,836456651,836459372,836467331,836482719,836483898,836483986,836487417,836488850,836488956,836489450,836490214,836492059,836494474,836500927,836501538,836502000,828909129,833074965,833498409,833768055,833776597,833816209,833832032,833835264,833871734,833894721,834277120,834306251,834451208,834714126,834833420,834841712,834855913,834916363,834919351,834924953,834941401,835121681,835122383,836123031,836131637,836159497,836507780,836512044,836522968,824599161,827124271,829912482,832276186,832276187,832426699,832427051,832577902,832671447,832926368,833263477,830534249,832253192,832290854,832973119,833070733,833070754,833071476,833076580,833102370,833568208,833707236,833712372,833921769,833934185,834513203,834812471,835283588,835303131,835315301,835316884,835316888,835700083,835702574,835704871,835715820,836119891,836119908,828234124,828260513,828881836,829360734,829361484,829367565,829377953,829379249,835205534,830740772,825272439,826266303,827093758,830494041,830509361,830519171,830559616,830575230,830582025,832150009,835704029,825861265,825876044,826002372,826291363,826301030,826308567,826438633,826795876,827303300,827332871,827347406,827374002,827381188,827494412,827498863,827516820,827519857,827545593,827548066,827781526,827839865,827839892,827840354,827864689,827866424,827867695,827894125,827897288,827898403,827920866,827922293,827941934,827941951,827942646,827947497,828387395,828387409,828496331,828614918,828884577,829544360,829585153,829585193,829975019,830628023,830716295,830919450,830919925,831012811,831018135,831018160,831018192,831018228,831018261,831108447,831108483,831108583,831108598,831108627,831108659,831108686,831229748,831527971,831528001,832891132,832891143,833144179,833296388,833486645,824883642,824883658,824883666,825039421,824870513,826938932,825529059,825551626,827092425,827619993,827628916,828178725,828180549,830502365,830517791,830523502,826339137,827446373,827736790,828483564,828772967,830212125,832220988,833339651,826860995,826895205,829123711,832467165,833573981,833629777,833629804,833629813,834233220,835505990,824736293,824741754,830045340,826451061,827982067,828051210,828393659,828697237,828697876,828699168,828700234,828782201,829949383,831368944,831605004,832510800,834103664,834143220,834152458,834159793,834552845,834552872,834552889,834552912,834708960,834890297,834904175,835004693,835005357,835217064,835259749,835293369,835497981,835645438,835645845,835646376,835646682,835647645,835648628,835650293,835651357,826775353,826775385,827192513,827193088,828535521,829039625,829039740,829440536,829535643,829540405,830208622,831229769,831691179,831826647,832524259,832537848,834173414,834340510,834474489,834609403,834652409,834741558,834994616,835073606,835080325,835167338,835252814,835479965,835564757,835601300,836020109,836020933,836351576,836405125,836769902,836864001,829879130,831366416,831589358,831589370,824583975,824620486,825180022,836643338,835229903,835240459,835356759,835365120,829752513,825759588,825781682,829563436,829569608,834829545,834837556,834887497,826858942,827367353,829041411,830299889,830707798,831055303,831322463,832225700,830658149,836263666,836263702,836267549,836267566,836267574,836268098,836268110,836268119,836580354,828624606,829927881,830347353,830347371,830347402,830347417,836189651,836189725,836429698,832476104,833068391,824602374,824636292,824671580,824671659,825168024,831609397,832637609,833744484,834347599,835823400,836072656,836269367,836292893,836292904,836292924,836292949,836292962,836292978,836292994,836293008,836293243,836293264,837044630,837072119,827824653,835416836,835438444,835644679,835645979,836278492,824757863,829483014,830465213,830865813,831507172,831556950,831558986,831559880,831560743,831635813,831681723,832060768,832060799,832100534,832252721,832256189,832257493,832359898,832516221,832524064,832528320,832530324,832622238,832632496,832632497,832632590,832632904,832633447,832711355,832962642,832976450,834390393,834611783,834611786,834967528,835267006,835267198,828656309,829074672,833585086,834817971,834834293,834841536,834862074,832124954,832190780,832191902,827336972,828555108,828616485,828731412,829697377,834723498,834801120,834803946,835836462,828501761,831318303,831326073,831326075,831333701,831666753,831687931,831687945,831792461,831815001,833277575,833288065,835084614,835092017,835234653,835236946,835237137,835750866,835756189,834912392,834926632,834939284,836075921,836186458,836250371,835808129,824860081,825887742,828310014,828311165,830390081,831240235,831899999,832327015,832328313,833474770,825665772,827553440,824481621,824706553,824857816,828290096,829254755,829691989,829692004,829902808,829903598,834880342,824705688,825063025,825745648,825961669,825978276,825987038,826068915,826086290,826189443,826205698,826286304,826319104,826399937,827029637,827288872,827322919,827410642,827411503,827775251,827779919,827780291,827962137,828547950,829074969,829156255,829219377,829761818,829921763,829989141,831886982,831967777,831968072,833613584,836556686,824499823,824734765,824774101,827565994,827613254,829111407,829151080,830107369,832947945,828045124,836513555,836513562,836513574,832672713,832909862,833232297,833523204,833528166,833533030,833542974,833545607,833553868,833556017,833556829,833563771,833575625,833598972,833653074,833679980,835687736,835776816,835829205,835907759,835964995,836521518,836543803,836571816,836770550,836907319,825923480,828782460,828782472,828782482,828782489,828782504,828782512,828782517,828782530,828782545,828785058,828785924,828912259,828918215,829219336,829536221,829536238,829536249,829536260,829536278,829536301,829594807,829603789,831006782,831007311,831007318,831007327,831007349,831007379,831007404,831007425,831007445,831007463,831007474,831569071,831878828,835186361,835186372,835186375,835186387,835186392,835186397,828040129,828055000,828055763,828084860,828090186,828109644,828111589,828112192,828114874,828114967,828119689,828131208,828142685,828149167,828149343,828157653,828159350,829672127,825656836,825660778,825661625,825896996,828116518,828129387,828860917,828863524,828912509,828919624,828924172,829380006,830984025,831030369,831843711,831846293,831903357,832133638,832648928,832656525,832656526,832670930,832696882,833266424,833280957,833281374,824563984,824573306,826350773,826464045,827574153,831599666,832187661,832188754,832188763,832188768,835701687,835704791,835708157,835720771,835723578,835796554,835798048,835800688,835801984,835900412,835914429,835954369,835970340,836366746,825003273,826560089,827228175,827228181,827228189,827228790,827228794,827229200,827229252,827229293,827229327,827229351,827229396,833343157,833377859,833486185,834502259,834637253,836214600,827128914,828431239,828943005,833054430,835917477,835927797,835938073,833361893,824674117,824711896,824728457,824760855,824833996,824834614,825934920,825934944,825934972,826856403,826859540,826859704,826871746,826874341,826879351,826883244,826885119,826885223,826922307,827275144,827275182,827275226,827275308,827291838,827291908,827297282,827297828,827304541,827316560,827338392,827338591,827345036,827345701,827352243,827355210,827360683,827364723,827478955,827484193,827523623,827627674,828928460,828984047,828999620,828999636,829005386,829013383,829086090,829104755,829105046,829108180,829128573,829132230,829133562,829134880,829134972,829196571,829196689,829201802,829204440,829206275,829206662,829221253,829227359,829246367,829250774,829288708,829724802,829724829,829725006,830434937,830434950,830434966,830435006,830439070,830441831,830478914,830478952,830479000,830479087,830479129,830479216,830479413,830479496,830479645,830566219,830566648,830567654,830732413,830887930,830889450,830889458,830889519,830889527,830951609,830951726,830951749,831048119,831150454,831174527,831174943,835730749,835730769,825367613,825367631,825367674,833758877,833758899,833758911,833758950,833758966,834237520,834239984,834991617,834992825,834992840,834992862,834992894,834992907,834992929,834992947,834992973,835881694,835881710,835881720,835881743,835881759,835881781,835881797,835907254,835910259,835916039,835916075,836749719,837019224,832809622,827489393,827590374,829909551,831371260,834738001,835185389,832683123,828627938,828628235,829646678,827205419,831006876,831009120,831009145,831083450,831083451,831083452,831083453,831083457,831083458,831083459,831083463,834807797,835309123,835309133,835376178,835411727,829527567,828559314,831530022,832606976,833784314,833784315,833886881,836295635,824981667,825732149,826229687,826229866,826229957,826230084,826230155,826230281,826230334,826230428,826230478,826230545,826230647,826230728,826230790,826230884,826230947,826230997,826231047,826231125,826700489,826700507,826700555,826700565,826808190,826808236,826808277,826808321,826844020,828556404,828956016,828956037,829864718,831413226,831413233,831413238,831413248,831413257,824779530,825180712,826157709,826517618,826575032,826736296,826766829,827463767,827526208,827817802,827817813,827817869,827858629,827892018,827931369,827931447,828297004,828387590,828627951,828628015,828629563,833430437,833434821,835249005,835253857,835255445,835255469,835255482,835366590,835980476,835980710,835980721,835980726,835980732,835980739,835980753,835980911,835980917,835980930,835980939,827304029,833460717,826821387,826821769,826821910,826821922,826821936,825002848,834280948,834971755,835821726,833716749,836297470,834609427,825784656,827730088,828971397,829126054,829530230,829531090,829531402,829531413,829531428,829531446,829531458,829678684,829744692,829744703,829744714,829744756,826142083,833462359,834435173,834476145,834476176,834476303,834476322,835599483,835613898,835624487,835624492,835624498,835624502,835624565,835624566,835624568,836034341,836036578,836043368,836043412,836043416,836043420,836043421,836043423,833293881,827713100,828001053,832202000,824976076,827972471,832878407,832879760,835652655,836044643,836044646,836046672,836050716,836052050,836052064,836052072,826586590,830134682,824651354,825261180,827565895,827657744,827674510,827677512,827678216,828335702,828501213,830108375,831217046,831511669,831817875,831822204,832308972,832856061,832860755,832861838,832863451,833990755,834580600,828099294,828205736,828503653,829227768,829227811,829227851,829227896,829227932,829228006,829228084,829228129,829229540,829229569,829229591,829229629,829230195,829230258,829230296,829264206,829411087,832788782,834457021,824496047,825294575,825294630,827699523,828643992,828720056,832205840,832248884,832619561,834844938,834844963,834845016,826210835,830438969,825415754,827605356,826981928,830127153,826857764,827558394,827148921,829398114,831305400,831306009,831306042,831306082,831306111,831306144,831449334,831449354,831449367,831449375,833391421,833818021,834221158,834221207,834221261,834221335,834221346,834221499,834239275,834354448,834599456,834599494,834616718,830952714,830957769,831165682,831313156,832773593,832779078,832789750,832791864,832811174,832816060,832827762,832828412,832828877,833484161,833486377,833567192,833581156,833619275,833699609,833761272,833878145,834280061,834304863,834433764,834868747,834869937,834900259,834901186,834943735,834946126,835115064,835115704,835116184,836142703,836778730,831283833,834188350,825129842,825129850,825129855,825129859,825129870,829252497,829993967,830648221,830648245,831373497,832068856,833373947,833482533,833486003,833597422,833624736,833624753,827061464,826868216,825180473,825183237,825387392,826289272,826858722,826858744,826858836,826858875,827331814,827331827,827331850,827331970,827499219,827499240,827499273,827499315,827549521,827549545,827549598,827550923,827577499,827580643,827581694,827581728,827582463,827593612,827602450,827603743,827606181,827607400,827607423,827632779,827801489,827801498,827801514,827801523,827801595,827867838,827867852,827867879,827867899,827868352,827898338,827898363,827939255,827939266,827942617,828295419,828295428,828388231,828388243,828388253,828463759,828463782,828463830,828465716,828466623,828466666,828566782,828760264,828842345,829516726,829586100,829586117,829589631,829589770,830918964,830918968,830918992,831664105,832015516,832520854,833070272,833098951,833286657,832952631,832961144,833085474,836150441,828559907,835793618,835950719,836294931,828282975,835387402,836055331,832979548,833857789,833899638,834422651,835870513,835874794,835924200,835936490,835951729,835952913,829174844,831929572,832063586,832097458,833844162,833884349,824639775,830217377,831889987,834892925,834892966,835096389,835096914,825374766,826382188,828220017,829631654,829817942,829817955,829817978,829818272,830162352,825850352,825885650,825903854,825913079,826040737,826049730,826049869,826378122,826422760,826443653,826574881,826584208,826588143,826592205,827688151,828604819,836427724,836427725,836429033,836431559,836432410,836434177,836437895,836438350,836439774,836442310,836453344,827904545,831081502,830219886,830465132,830786214,824614233,824651639,824653676,824658183,824659268,824660498,824680074,824681712,824929314,825066799,825537111,825538423,825538561,825660072,825666168,825684817,825706671,825734109,826160216,826173496,826203315,826587468,826593040,826600872,826602629,826683943,827274614,827290116,827317881,827537067,827572894,827842978,828383565,828391995,828392511,828392975,828395211,828395645,828424994,828426187,828727861,828728163,828730670,828736203,828844462,828848873,828849531,829369954,829370080,829373819,829466789,829475509,829487652,829533413,829636957,829638165,829977299,830435656,831453639,831503860,831504738,831514616,831887864,831915134,831917029,831929360,831959221,831966464,831966472,831966670,831981211,831981423,831982814,835354051,835354241,835354774,835354819,835356418,835679403,835689888,835999261,836017586,827043536,827051280,824622559,824622728,824650491,827244106,827817896,829479920,829954945,829990632,830083790,831442418,831900649,831900720,832336763,835656926,835657541,835657728,835658086,835658178,835658207,835659115,835711683,836007479,836097726,836153804,824720144,824721865,824836661,825450116,827269270,827296154,827839153,828681026,831422428,831691485,831691496,832792889,833384092,833391276,834803022,835853338,836257625,836257788,836602904,836726668,836734201,832509415,832731229,832916840,832922143,833192082,833193078,833299394,833356012,833371272,833497556,833499815,833500528,833505381,833505388,833512400,833764628,832277048,826374948,828223151,828245939,828245970,828245994,828413308,828484597,829073646,830791210,831306045,831474846,831623440,831626983,831838542,831838855,831838893,831838921,831840467,833967526,834001749,835202827,835203642,835220742,835222206,835228187,835718884,835885010,835927216,835928486,835928497,836174523,836676453,833807826,834001704,834001734,834084132,834084159,828115739,830263068,830550770,828032042,832270748,832284254,832287778,832561228,833861870,834309696,835802543,831743867,835467760,836829934,826649781,826930518,827754715,832849134,834016808,834056411,826368461,829837942,830953583,831369463,831664538,832348256,834062937,834188190,834188205,834188222,834188252,834188268,834188285,834188306,834188317,834188341,834188358,834188395,834188427,834188452,834188481,834188498,834188522,834188538,834188556,827749026,834452252,834687559,834843247,834856489,835112039,836110013,836115789,836125496,836219060,829685423,829854481,831343777,831343815,831343846,835596264,825612753,825869692,826427357,826739329,830744317,832692479,832704159,832780151,832797374,833076858,833167224,833224270,833237457,833252575,833372446,833402256,833827605,833961346,835082328,835458355,835827872,835829002,827429059,831801093,835616254,835916349,835935774,835936539,835936763,833196632,833400567,833961223,833961921,833962103,833962283,833962442,834028620,835070560,835445306,835477418,835481554,835492397,835492672,835492955,835493256,835498710,835579196,835584101,835817735,826167584,826169552,833795959,835844535,835848461,825827498,825927839,826697926,826733934,826809437,827246445,827294240,827347922,827379272,827417281,827420855,827425290,827452192,827777579,827777694,827911624,827911663,828343595,828349229,828356175,828389830,828420576,828957566,829492230,826603198,827970724,827970751,827970784,827970803,827970841,827970874,827970889,827971005,827971041,827971064,831619130,831619143,831619162,831619197,831619222,831619249,831619273,833327704,833352170,835395298,835488510,824542280,825126255,825703935,825982049,825982088,827749967,827781542,827877490,827878268,827878292,827878304,827878340,828190044,834893625,835353191,828005333,828006516,828010259,828014153,828045926,828084380,832366093,830366721,831366743,834813726,824543137,824577259,824580169,824798314,825492906,824739267,826083444,832134524,832587772,832588411,832588594,832595254,832595386,832598970,833359431,833368521,833375534,833384262,825418340,829342531,832561538,835290273,826255627,826256457,827706867,826062711,829681918,828217384,828742289,828742317,830732813,830733872,831011236,834375385,834621821,834621823,834621950,834871578,835697584,835697600,835697610,826511892,828953704,828985924,829089863,829156615,829200518,829490398,829613323,824759295,824798606,824893540,824893560,824893579,824893619,827828245,833417345,833417393,825251429,825251486,825251499,830040752,826866210,826883734,826885613,827003929,827009101,827477883,827683357,827714452,828057957,828186339,828198670,828893706,828918025,829187935,831068206,831229682,833968212,834652453,831601198,832521869,833238294,833700950,834255344,834584092,834584118,834584124,834808978,835025212,835140197,835140203,835140211,825895614,826384912,826384936,826384950,826385163,826713572,828226044,828226574,829189181,829642646,829644022,829644575,829644593,829644694,829644731,829644754,829972791,830403882,830408379,830974153,832199803,832322422,832325080,832325207,832325282,832441946,832442046,832915396,832915411,832915906,832936153,832994134,832995686,833024155,833025531,833261902,833500107,833500928,833529206,833592752,833604706,833617404,833716532,833716706,833717369,833717408,833717425,833748285,833941654,834092679,834094258,834124031,834139924,834150490,834155928,834292551,834302000,834336601,834336736,834340216,834368481,834370214,834370336,834370798,834629370,834630336,834772918,834772945,834772981,834801932,834808285,834908183,834919535,835427646,835429503,835429523,835429592,835429869,835429889,835429911,835792970,835793361,836208672,836208700,824645580,827776898,829947329,830742824,831325758,831490264,831496957,831497268,831654896,831664684,831678485,831765863,831821319,831822432,830330899,830881855,830889515,831122622,831124915,831146376,831153138,831787480,831797743,831802224,831806584,831809610,832091614,832250838,832261811,832270315,832270595,832287049,832510538,832849460,832853966,832855214,832855935,832857793,833112972,833116781,833165721,833334268,833385615,833418935,833737539,833737583,833814195,833856293,833979857,834310219,834323665,835133154,835346913,835359470,835391357,835391452,835422632,835624116,835625345,824968002,824968045,825243597,825393143,825504818,827006795,827109466,827172926,827303897,827346975,827589048,827636412,827653256,827779884,827913061,827923161,828221111,828236655,828236731,828329771,828390696,828390719,828390738,828560864,828569210,828599912,828599939,828600027,828622551,828623452,828751439,828914332,828924081,828924178,828924228,829061465,829065721,829068492,829084312,829357213,830368085,831520681,831522183,831590380,831593502,831594148,831619072,831856393,831861916,832041693,832559523,833001682,833408641,834846346,836937624,828564920,835416752,835422072,835428409,830174157,831614929,830493457,832350706,832350719,832350729,832350737,832350744,836067949,836084966,836085582,836086571,826223814,826530623,826532599,826533117,826540476,826550834,826561265,826595449,826677172,826677228,826677345,826677384,826677626,826677994,826684671,827061570,827082115,827085207,827135713,827152175,827262491,827262565,827263125,827263254,827264063,827279904,827520874,828338156,828408931,828577407,828578420,828864515,829684468,831803706,831866691,831866692,832126788,832132722,832460236,832482306,832485846,832828103,832828183,832828294,833152143,833152281,833175330,833175509,833175581,833423423,833593134,833593757,833595283,833595402,833595451,833595534,833596921,833597029,833597076,833597131,833971984,833972215,833972267,833972977,833973017,833973064,833973096,833973156,833973201,833973419,833973448,833973487,833973525,833973556,833973598,833973620,833973639,833982034,833989688,834694729,834694836,834694942,834694985,835386430,835567233,835567366,835573832,835696971,835826057,835826862,835827817,835827935,835829920,836060004,836060017,836060032,836060039,836060055,833430753,826620036,832840827,829291373,829859497,831137813,832234061,835453281,831084049,831265575,831265600,831265616,831265687,831265851,831265861,826292019,826301032,828490901,828497559,828503608,831530581,831585251,831597033,831615747,831618665,831678421,832397548,825294771,825294824,825295231,826315900,828841430,824985840,825411792,836849253,834344591,834344671,836287906,824638550,824800796,825722408,825738514,825739099,829611597,836143059,836143874,836143890,836143914,836210311,836210330,836210352,836210367,836323824,824993794,826351868,826352244,826374169,826393327,826395412,826431308,826441180,826461349,830122023,830122952,830125985,832882724,832883749,832892075,832892506,832901811,832904171,832926035,833344729,833425712,833430195,833430248,833453700,833815090,834023316,834023891,834024639,834025093,834025998,834025999,834026920,834027080,834028423,834028427,834028662,834029452,834030018,834031477,834032589,834032706,834032750,834033304,834033542,834034628,834035397,834035406,834035411,834035583,834036183,834036239,834036255,834036765,834531284,835156066,835170154,835172243,835172268,835173633,835175136,835175408,835176837,835177199,835177318,835496253,835497478,835497706,835551655,835590129,835841785,835847774,835855409,835855823,835857589,835862829,835864486,835878499,835880781,835881981,835898559,835906164,835914513,835935454,835935545,835943101,835951445,835954239,835955044,835955469,835963125,835974688,835980117,835982620,835990251,835990443,835991503,835996741,836000855,836001722,836029102,836034421,836037226,836037596,836038656,836038946,836039476,836039977,836485530,824493853,824839778,824960153,825084504,825428440,825428441,825466601,825483052,825501667,825543402,825935731,826063889,826093313,826093619,827041709,827432369,827632237,827655912,827729795,828051039,828166733,828221125,828226008,828231937,828238654,828253299,828341364,828346654,828350743,828351969,828358812,828371151,828392177,828400176,828401232,828496414,828496821,828497616,828501454,828505991,828513703,828515775,828520194,828520216,828523810,828527987,828530344,828531189,828558226,828615055,828615585,828636518,828670991,828693638,828757457,828786995,828790438,828793270,828795439,828795456,828984050,828992538,829008122,829009495,829095513,829151007,829151837,829152816,829156752,829215983,829216758,829231633,829344827,829351021,829360166,829360684,829367552,829683891,830174477,830192907,830193107,830193326,830335666,830354709,830361179,830361271,830361486,830363454,830366393,830366461,830368769,830372121,830372239,830373497,830373641,830531980,830533361,830533403,830537117,830539266,830546737,830597320,830599128,830603414,830603603,830607955,830608392,831385357,831404403,831415400,831415863,831416684,831419134,831419177,831491975,831495136,831495216,831495531,831497310,831502422,831520092,831522593,831524538,831525435,831579663,831590741,831591243,831591806,831593269,831593527,831595110,831855955,831866768,831869294,832024785,832648403,832724309,832731120,832752503,832781912,832825675,832827570,832839499,832841800,832843611,832853662,832858838,833058888,833063027,833063620,833065911,833075732,833076448,833081548,833081693,833082629,833084023,833084092,833089431,833099095,833104349,833104351,833115445,833115510,833130390,833251731,833252806,833252846,833254656,833256073,833256129,833264883,833266951,833269211,833274400,833274980,833275245,833277728,833428020,833428600,833429478,833440748,833440771,833440784,833441831,833442014,833442033,833442201,833443428,833444037,833445657,833454524,833505338,833505623,833509180,833510331,833511584,833511623,833515555,833526430,833532382,833535594,833537149,833550414,833814213,833981156,833990813,833992989,833994447,833996539,833999991,834003067,834003068,834298360,834360542,834382513,834395637,834416790,834578661,834579054,834583175,834588420,834591227,834596418,834596780,834599066,834599097,834599100,834599133,834599222,834767725,834818294,834833776,834846249,834847386,835472205,835485709,835564079,836000994,836012660,836012695,836015411,836015634,836397139,836398859,836415074,836419735,836422007,836426796,836426976,836430819,836430993,836435816,836438278,836440766,836466687,836468737,836468880,836746332,836746898,836747294,836749588,836757320,836757321,836925507,836925834,836925855,836925984,836927746,836928895,836931567,836932259,836943317,837027100,837027140,837027227,837027263,837027801,835580355,835889742,835962162,836004162,836004177,827888260,828956308,835441353,835519930,835554944,835934927,828709156,835123260,835126536,835126566,827949491,830487621,835728550,826835244,829644469,830867133,830437667,835708810,835990569,835990571,828668746,828711150,830572489,832924677,833233295,833233309,833368447,833368463,833368487,833368519,833368533,833368558,833636303,833722749,834144726,828537827,831946145,829714587,836224504,836224641,824715550,824719154,825313008,825457849,825501110,825510797,825510901,825516990,825542930,825596408,825909043,826057219,824719647,826799901,827881641,829599985,829601377,829601417,829601458,829601479,824871413,824871420,824871474,825240754,825251848,825251866,831854061,831855381,829959199,831575132,833506576,834377531,834390855,834680620,834686254,835247401,835208447,825315009,825315026,825315312,825315346,825315360,825345498,825345530,825345547,825345629,830485219,831028484,833998361,825312189,826792474,826803898,827395555,828262263,825543396,825543580,825545558,827537439,827537440,829679851,830002795,830002860,835670240,825544709,825549925,825771300,825959959,825963178,826857327,826860547,827400974,827536685,829684211,830013534,826225442,831542344,825545675,826970257,829652736,829685604,830016800,836774011,824734069,826057073,828439174,828677992,830024439,824727071,824729136,824742273,825543581,825796232,825797091,827456036,827456049,827456056,827537441,827703876,829650670,829650956,829652639,829652688,829653347,829653873,829679849,829679907,829680780,829681336,829683650,829686083,830002861,830015263,831998706,831998709,834175363,827101161,827818970,828401598,828512565,828512621,828513385,828596566,828826174,829430713,829458142,829525141,829525170,829525178,829733207,829786087,829786195,829786307,829786682,829984810,829984838,829984866,829984888,830618861,830704780,830705101,830705141,830705757,830705783,830706063,830706137,832408503,832408723,832904189,832904629,836341988,836343824,831205464,835509391,835509426,835509455,835787098,835787859,836711756,824712273,824795257,824798690,824810604,825686199,828396656,828422651,831944277,831968267,826609617,827979601,828147703,829330312,831570868,834721117,835452742,825990685,827913906,827937139,829134293,829535492,829649765,829702238,829702481,829767374,830733898,830861948,830988253,832060424,832559216,833438690,833715675,834618127,834698574,835119493,835207101,835223797,827258984,826612107,828954803,828954972,824773506,824774898,829140070,830113374,828529005,831239180,831239186,831239199,832197130,832197150,832197166,834837770,834837792,834837846,834837877,834838134,834838172,834838222,834838277,834838387,834839638,834839667,834839833,834839899,834839934,834839988,834840028,834840071,834840190,834840247,834840297,834840344,834840369,834840391,834840413,834840426,834840440,834840455,834840484,834840499,834840516,834840530,834840552,834840573,834840587,834840630,834840644,834845595,834847154,834847179,834847199,834847233,827022851,828308313,828457213,828457244,832239369,832239506,827807613,832343578,832348241,832350271,832374896,832378783,832909863,832927971,833491657,833519373,833587456,833589967,833598974,833643735,833655936,834292265,827466673,828433361,835744291,835981660,836566331,824650816,824650829,824650903,824671638,824758855,825204534,825204551,825204627,825204655,825204672,825268678,825313764,825313777,825313844,825313868,825313900,826416248,827313079,827327786,827327813,827327859,827417055,828926742,829204075,830280561,830318132,830456125,830573714,830703078,830703104,830703173,830703207,830958631,832718225,829042561,833556711,833581979,835243146,835942328,835954061,835970297,835971813,835981349,835988720,834847781,835216039,835217513,835580756,835580803,835580807,824612319,824612980,824614713,825276749,825284031,825601299,825788978,825792655,826058851,826268393,826274123,826543789,826843465,826852961,826855194,827220646,827220978,828459790,828475294,828931698,828933662,832875717,832876500,832877901,833346441,832906176,832906450,836069740,836075005,836076681,836084080,836100047,836104951,836106128,824788292,836039947,836040681,826721427,827604496,827604510,827604532,827604555,827604608,827604659,827604679,827604689,829945279,831590452,824799292,825784618,827877940,827975342,828048768,824707684,825247262,824882406,824882429,825251631,825251648,825251782,825251791,827415975,831393419,826559170,827684757,830169948,834593750,832186989,832403698,832404906,832409675,832410281,832773471,832776627,832776804,833581316,833813792,833823681,834211137,834215282,834396725,834397286,834421458,834661682,834669575,834670617,834743803,834744247,835068751,835232380,835232384,836045636,836046821,836110042,836317675,836344068,836693971,836694070,836694157,836921232,836921739,836921826,826324363,829908816,829920240,829920245,829920249,829920259,829920264,829920271,829920278,829920288,829920301,829920315,829920321,829920330,829920342,825360245,825360271,825360280,825360302,825360322,825360357,825360385,825360412,825360437,825360453,828788105,828788112,828788121,828788123,828788127,828788134,828788137,828788143,828788155,828788162,828788176,828788184,832222868,832222905,832222920,832222946,827401514,827467423,828580702,828583546,832113005,834827186,825086114,825102317,825572892,825584788,825586350,825594566,826059152,826064544,826074591,826083186,827175036,827179699,827186700,827190971,828215352,828243414,828251295,828255306,830336474,830345155,830880390,830884385,830884590,830890457,830892217,831429442,831863853,833437493,833438335,833439257,833441311,829891212,829891217,829891225,830123190,830125755,830125782,830125801,830125817,830746827,831541256,831541273,831541294,831541449,832489211,825478797,825479604,825479618,825479692,828560439,828560459,828560495,828560551,828560573,829974069,830204176,830204215,830204295,831123885,829456903,829456927,831076022,831726315,833805007,826384640,831193068,833659107,832107530,836888683,825213669,827595543,828762586,828811237,828827652,830767101,830842951,832894517,833510778,833515245,833517262,833518158,833529407,833530257,833530260,833593667,833597379,833620275,833641364,833643401,834220852,834221585,834227882,834232449,834232943,834232965,834235121,834374004,834374075,834374114,834374159,834374194,834374249,834374299,834404934,834405232,834405737,834405803,834406806,834406818,834406827,834407822,834408997,834409466,834414186,834416025,834417455,834651373,834651442,834651489,834651540,834652671,834652686,834652708,834654451,834656691,834660084,834663263,834665376,834693005,834693334,834713783,834714242,834715607,834718622,834721088,834750380,835903534,825009355,825396018,825406422,826691651,826797746,829354027,829354172,834513666,834514619,834516082,834518269,834534265,834534271,834537587,834558010,834558353,834561119,834561415,834600234,834600573,834600727,834849198,834851186,834853541,834853825,834856709,834860043,834881936,834894404,834899002,834900402,834910291,834914829,834915084,835152725,835153312,835154965,835155257,835157266,835158469,835485217,835489551,835492241,835511095,835511099,836020162,836020163,824571515,827212200,833432503,833440258,828491720,829840079,829840136,824688145,825243000,825549844,825746509,826047480,828637851,829024503,829131859,829131890,829148487,829261752,829515930,829515950,829516082,829516102,829516381,829516392,829516411,829535750,829592514,829592555,829592588,829699095,831251158,831443118,831443243,831443248,831443255,831443268,831587992,831962146,831980001,831980014,831980021,831980141,831980147,831980153,831980172,831984667,831994231,832101519,832179480,832666834,833071537,833073493,833077517,833079260,833094902,833100539,833771375,834164489,836492015,836492018,836506747,836506814,836506825,836506826,836668042,825244968,825250542,825345626,825387082,826721026,829443722,832998334,832998346,826232679,826577089,827720856,824744760,825828186,827923028,828559118,835066409,835797942,831628667,832109777,832110638,832110766,832113551,832113567,832113596,832113603,832113614,832113649,832673810,832673858,832674519,832674579,832674594,832674627,832674644,832674666,832674684,832674700,832674719,832674754,832674777,832675622,832675632,832675865,832675886,832675901,832675918,832675932,832675946,832676002,832676887,832676910,832676935,832676941,832676956,832676997,832677142,832677158,832677177,832677206,832677227,832677240,832677257,832677266,832677288,832689649,832689700,832689746,832689790,832689819,832689852,832689981,832689997,832690012,832690044,832690056,832690082,832690528,832690560,832690799,832693240,832693275,832693297,832693304,832693348,832693361,832693389,832693404,832693418,832693441,832693454,832693467,832693484,832693503,832694016,832694036,832694081,832694104,833602046,833602123,833603748,833603759,833603792,833603798,833603806,833603823,833603891,833603895,833603905,833603914,833603925,826398491,826821422,832128774,826492016,826492033,828796839,828796842,828796844,830679763,833599557,833739459,833965929,834578115,836110822,836194436,836291191,836383027,836411113,836537406,836666740,836708121,836708128,824772861,824824811,824869537,824898201,824934800,824982599,830724716,830724731,830724787,830725022,830725039,831628221,835751760,835763681,835861925,836189306,827276137,830682965,836675382,834731420,834756594,834837620,835203097,835937792,825022032,825649224,826558152,826753339,826780121,826781629,826911273,826919699,827004783,827048441,827225827,827231610,829231945,830117113,830214373,830424891,831903165,832332044,832333215,832889921,832891322,832891594,832895215,832895307,832896086,833330042,834790512,834790632,824608711,825015995,825387787,825389692,826933761,827805995,829040963,829040982,829040990,829041125,829041134,829041146,826385813,827819511,829010473,829999459,830084606,830491697,830500063,830500619,830500642,830500671,830506291,830509213,830519821,830552296,830561623,830568636,830666548,830747362,830759960,830818211,830904564,830904986,830915754,830915760,830915768,832108553,832144520,832152407,834188361,834403648,824887770,832159465,835537167,835667947,827591368,828261279,828589329,828628898,828993236,829552430,830206219,830875527,830878601,831418550,831418770,830602934,831400742,831400965,832346759,832482855,832484905,832545290,832550410,832550593,832550700,832551025,832556624,832556752,832556875,832557104,832648282,832649816,832650899,832652591,832747917,832755366,832755508,832836044,832837916,832838014,833039852,833040635,833041974,833042406,833043114,833043215,833043904,833197316,833197931,833198220,833205273,833206359,833206808,833509798,833509943,833510081,833515086,833515682,833516466,826721840,826787468,828073086,828091481,828395167,829935272,830588192,830941607,830950894,832666580,832722113,832912081,832941484,832999119,833020679,833150335,833188745,833283316,833419386,833493838,833495654,833495672,834893881,828042309,836580762,836668633,836668655,834828681,834828779,825586537,828811805,828811810,825113219,827866434,827866465,828135000,828248047,828885190,829532922,829936650,830462410,830562364,831206510,831266342,831293871,831474122,831574694,831581168,831753045,832245001,826115988,835878829,836092461,836092748,836095418,836095577,836095623,836095815,836095956,836096022,825707170,824705405,825292022,826971330,826974068,828885051,828906421,828910001,828913636,831173578,831177690,834887248,835134589,824468025,824468053,824468066,825196327,825201845,825202667,825202701,825202715,825202728,825202760,825317803,825453203,825648556,826028120,826028438,826028447,826098162,826098174,826182387,827718640,827718676,829320686,829320835,829684942,830455925,830990542,830992713,830996567,831134901,831137208,831137258,831178213,831198703,831198725,831200508,831200558,831200623,831200677,831200707,831200817,831200838,831200916,831200942,831200975,831201060,831201090,831201136,831201156,831201257,831201279,831201375,831201404,831520557,834700913,834700943,835136631,834741721,834741742,835134111,827522485,829603273,833468237,829979828,832002212,833620803,830279799,830928739,831103886,831201930,831201969,831201985,831202004,831310106,831455225,832158232,832306526,833482800,834310038,834589768,835378335,835938757,835938787,836044017,836316337,826624100,828913774,829880720,829881940,829882954,829883073,829885467,829885888,832821365,832916091,835030618,831947848,834710266,834894733,835778070,836157063,836169939,826101192,827723700,828740645,831715284,831970942,832234974,828680628,828680649,824999100,824999142,824999158,824999193,824999335,824999355,825000403,825000432,825002528,825002771,825002888,825005867,825005905,825005923,825005941,825006071,825010490,825011034,825380491,825392008,825392083,825392219,825392806,825392972,825393218,825393279,825393639,825393728,825394058,825512062,825512095,825512113,825512140,825517662,825517675,825517688,825517697,825533079,827813649,827817831,828569439,828573285,828579065,828579266,828579802,828579825,828579956,828579974,828579987,828580022,828580040,828580387,828582475,828585511,828585525,828585542,828585559,828586085,828586159,828586317,828586332,828586348,828586376,828590457,828606753,828617473,828973722,828997085,829026255,829026270,829026307,829026485,829061441,829354427,829358382,829711220,829711242,829712027,829712040,829712062,829716857,829809900,829813667,830405587,830405990,830407350,830407643,830409073,830466442,830709494,830752015,830856653,831018015,831026065,831026081,831026102,831026653,831026674,831026697,831028336,831028872,831028964,831029012,831029047,831029106,831029216,831029234,831029256,831195738,831336476,831360671,831472461,831496539,831496639,831496987,831524070,831569472,831590312,831635528,831635548,831637057,831637114,831637965,831637996,831640571,831640589,831640600,831640627,831640648,831640694,831640720,831640761,831641593,831641629,831641672,831641706,831641752,831642486,831642497,831642519,831642681,831642707,831642730,831642755,831642780,831642792,831642814,831642841,831642856,831643937,831643955,831644014,831644024,831644052,831644074,831644106,831644629,831647512,831647526,831660020,831901104,831901117,831901131,831901703,831901720,831901731,831901738,831908006,831909786,831909795,831909821,831910276,831910278,831910280,831910281,831910285,831910287,831910291,831910296,831924436,831931426,832005085,832005098,832017855,832021392,832038161,832939288,832939309,832939321,832939351,832939382,832939587,832939596,832939625,832940161,832940181,832940205,832940264,832940269,832940298,832940345,832940361,832940382,832940410,832940661,832940677,832940689,832940699,832940721,832940755,832940768,832940783,832940805,832940820,832940835,832940841,832940858,832940873,832940899,832940911,832940936,832940955,832940980,832940995,832941024,832941036,832941050,832941071,832941088,832941116,832941139,832941148,832941166,832941178,832941198,832941242,832941276,832941370,832941500,832941511,832941529,832941548,832941567,832941587,832941603,832941786,832941797,832941858,832941904,832941932,832943200,832945369,832945390,832945413,832945777,832945823,832945899,832945909,832945931,832945948,832945966,832949730,832949930,832950361,832950840,832951634,832951983,832953598,832953707,832954191,832954247,832954308,832954412,832954552,832961212,833031513,833090186,833090249,833095099,833095118,833095137,833095180,833095209,833221710,833222388,833223010,833224439,833225101,833226392,833907640,833907654,833930327,833930349,833931872,833931925,833933170,834048455,834055727,834074345,834094307,834119413,834136617,834136993,834137320,834315418,834315440,834315460,834315475,834315650,834315676,834315702,834316230,834316279,834316606,834316646,834316674,834316700,834316720,834316757,834317756,834317991,834320834,834320871,834320899,834320938,834330632,834334565,834336369,834336644,834348255,834348283,834348294,834348755,834348819,834348883,834348924,834348949,834348970,834348993,834349011,834353533,834353569,834353588,834353621,834353663,834353679,834353708,834354076,834354102,834354124,834354143,834354176,834354195,834358671,834358686,834358710,834358752,834358772,834622164,834622274,834636290,834699609,834699657,835367482,835457473,835457506,835457561,835471116,835792167,835801702,835802196,836319222,836726205,836728758,836257717,831814043,832347512,826902443,828224643,829315225,832538487,833596588,825887923,826361102,827599453,827970558,827970559,827981348,830057392,833558994,834942932,835792506,835793566,835794995,835797463,835798735,835801373,835807952,835812660,835814247,835818335,835820868,835822513,835824873,825050372,825420224,826389050,826796501,826957637,827950112,827970110,828361415,828415283,828632771,829488277,831304572,831463974,831466356,831466371,831725933,832747891,833260782,833732695,835265249,835318129,835883994,835944529,832138386,832159543,832740385,833884816,834784847,835086055,835430931,835454948,828196341,826799842,833313749,835821196,828936932,828937247,828937283,828937305,828937362,829847351,829847460,831038466,831215825,831530909,831566523,831587573,831587592,831587618,831587641,831587715,831587779,831587904,831588669,831588684,831588698,831615896,831964522,831967714,832033742,833154002,833154024,833154085,833154193,833154214,833154238,833686785,825519955,828224929,832909125,833795241,834112148,826473931,826473959,826474748,824672095,825500678,825793531,829225504,831493803,831701441,831702152,831704824,832597718,833342614,833342637,833392695,835082581,835082622,835082641,835082658,836090554,836090576,830869721,830869729,824535270,826144883,827628476,827628540,830820376,834442741,834442767,830230648,830278568,830349136,830353417,831932454,834888882,835526271,835545398,835702477,835702486,835702868,835722545,835730827,835820915,836660836,836660844,836679753,836679970,825872792,826685887,826685895,826685906,831840065,831840103,831840111,831840121,831840133,831840137,831840151,828563407,834154968,834155007,834241061,835091921,835110000,835110332,835701395,835717724,830314597,830321747,824722145,836212204,826467870,827780076,827859319,824619475,824624625,824628668,825155753,825159793,825309525,825785558,825929708,826172298,827566586,828520666,829923522,829933224,830792638,830951097,836046341,836100739,836116494,836116551,833499417,833511834,834097723,834109612,830647628,830819736,830856528,833130963,824518386,824542492,825819009,826067049,826309833,828087216,828100873,828122158,828125653,828405766,828405806,828407847,828511632,828528536,828539913,828541616,828542434,828542622,828544554,829349887,829354120,830218807,830740781,830740848,830740942,830745580,830745584,830865110,830866693,831520614,831520641,831525534,831526025,832017310,832017403,832017447,832019074,832159205,832167380,832195233,832198393,832198397,832222940,832257609,832257716,832281433,832554232,832565150,832565191,832611503,832792118,824997803,826057923,826544971,827004003,827004021,827022069,827285357,827365177,827407242,830006160,831510534,832297467,832297472,832297609,832297615,832297619,833817731,834107029,834107938,834138840,834141325,834147253,834168777,834952503,835011974,835012852,835024602,835025097,835025542,835026064,835026454,835026964,835029897,835030758,835031207,835032149,835468394,835471995,835476540,835480266,835481374,835526264,835556921,835566249,835810541,835824748,835825503,835953279,835955764,835995458,836432480,836438703,836438974,836446965,826624117,827120894,827124644,827125365,827125366,827125367,827132302,827132308,827136726,827136727,827139163,827143488,827146397,827147616,827158419,827158420,827158949,827159507,827160211,827161310,827163490,827165736,827166195,827355479,827356231,827358534,827358535,827365778,827367372,826433048,826433100,826435790,826435812,826789525,827032013,827037848,827038360,827038575,827038638,827038686,827038712,827038771,827038811,828910549,828910573,828910670,828910688,832770234,832770251,832770551,832770565,832770623,832770694,832770720,832770737,833308719,833318974,833319021,833319045,833544119,833844400,833863806,833868211,833889928,833921796,833947758,834657371,834662558,834673834,834681626,834689487,828299790,829631991,830411076,832683879,832684362,832685309,832685652,832686351,832686818,832687284,832687695,832688208,832693221,832693728,832699671,832700050,832714123,832741438,834878210,834878611,828617309,828638655,829631686,829840071,829840633,829844146,830024543,825467459,825467487,826641112,826647377,826753780,828782840,829515918,829515958,835614888,834111062,834112097,829547870,830014846,834485316,832558259,832560614,835681117,826546637,829771614,834923227,835994441,835995351,825500844,825505690,825505720,825505928,825636282,825636283,825636285,825636286,825636287,826625493,826625500,826625505,827796252,827798378,828574255,828574273,828574284,829258822,829258877,825374794,825374817,828092601,828615398,828937187,828937206,828954163,829142534,829301343,829535008,829535033,829535059,829535110,829698882,829698892,829847743,830113764,830301609,830322247,830344952,830348885,830348903,830348913,830349094,830349101,830349109,830349158,830349174,830349180,830349305,830349339,830349593,830349603,830349619,830349752,830703378,830703444,830703473,830960616,831371225,831371923,831371950,831371972,831371987,831372020,831372037,831372336,831449530,831449535,831449541,831449547,831449551,831449553,831449555,831449560,831550692,831587908,831587947,831587967,831587977,831666306,831744606,831748537,831761751,831761776,832113590,832124919,832125005,832125034,832125472,832125500,832125518,832136672,832212044,832212062,832212374,832212399,832212428,832212442,832212610,832212623,832212634,832212654,832212866,832212899,834991650,835113920,835571402,836465582,836465736,836475331,836557836,836602648,836634632,836661915,837036888,825480791,825880433,825881627,827124797,827124798,827124799,827124800,827124801,827124802,827670170,827681055,828125227,829681192,831413444,831494176,831751357,834251565,826635575,827332534,830091350,830215488,830215492,830215567,830217360,830230927,835701403,835720381,835720840,835721498,835722309,835728576,825401926,825504299,826843272,829315009,829395411,829406173,829406178,829410578,829410594,829410596,829410597,829410602,829410603,829410607,829410612,829421343,829421354,825960586,826392462,826761726,830452526,833925778,824734357,828039414,828223448,828225261,829343217,831093790,834617277,834624713,834626016,834639020,834646842,836507638,836507794,836508466,836511071,836935383,836938226,826633702,826712731,826925844,826945427,827323676,827332621,827343525,827858447,827885825,827888672,827945775,827963232,825916085,831672764,832171550,832663150,834361061,834945269,837051444,825694708,825694728,825694748,825694766,825694810,825694830,825694852,825694864,825694903,825694914,825694932,825696018,825696076,825696112,825696146,825696196,825696217,825696235,825696247,825696274,825696290,829926400,830464256,830925825,831582788,831584041,831586245,831677275,831785588,836513399,836522206,836578165,836578241,836578583,836580417,824489834,824531691,824690841,826345106,827033201,827049087,827055389,827057037,827088730,827131698,827132178,828106953,828626907,833846513,833905225,833922637,834656940,834682394,834683319,834683882,826842395,826292281,827393293,827393409,827504232,827628515,827628567,827652470,827838796,827838807,833998428,830538091,830542023,831330562,831353053,831584139,831747085,832211116,832211124,832211128,832211144,832262000,832533969,832534015,832536776,834386467,834386485,834386614,835409958,835416761,824687573,825075903,825106360,825106370,825106402,825106410,825156620,825181919,825761563,826672590,826673665,827335260,827781216,829452634,829963985,830081898,830081985,830082019,830549048,830612733,830612741,830867313,831108575,831344272,831500190,832011511,834044026,834044592,834044620,834044683,834044834,834044922,834044956,834675602,835707744,835707813,835707880,835707961,835708022,835708122,835708201,835708279,835708359,835708690,835708782,835760372,836231096,828441625,825180525,827470440,827626539,827713646,829504333,832359196,832508032,829989475,832064145,832064161,832064173,832064179,832078163,832728976,826595311,834609365,834609398,835521788,835521855,835522249,835522283,835522346,835522580,836987009,827001051,832576364,832577424,832593402,832948158,833142407,833149853,833156772,834354952,834655387,835632305,835634138,836054667,836055217,836055218,836056914,824489144,824579084,831759081,831759185,831759249,831760681,824669062,824885962,824885979,825115377,825115385,825115563,825238329,825713926,826213911,826213939,826213959,826214348,826214377,826214420,826725295,827162153,827163789,827163867,827163897,827830588,827830607,828428749,828428762,831936329,831936336,834119157,834119193,834119214,834119231,834455018,834723946,834724001,834724057,834724121,834725327,835216523,835221846,835230506,835235518,835271487,835277722,835287585,835287602,835287658,835287670,836165713,836174513,836199865,836199884,836199901,836199925,836200554,836269409,836285405,836287133,836287167,836287199,836287379,836670401,824505788,826056430,828757780,830079295,830826661,831091681,831156154,831496195,832171290,832215916,833479262,833878506,833920094,835202765,835206714,835226376,824537243,824848712,825855544,827384605,831200299,832992584,824540304,824623672,825272712,825306573,825466454,825715315,825717197,825724408,825960989,834516387,824657306,825218525,825218548,825218628,825218664,825218684,825218808,830583235,833919433,824646965,827446953,827774477,827774492,826779853,827501924,827501961,827501983,827531377,827569745,827577286,827582640,827776237,827784332,827794818,827794828,827794830,827794838,827794990,827795140,827795149,827795155,827795359,827795363,827795379,827795385,827795530,827795540,827795547,827795625,827795629,827795951,827795956,827795960,827795962,827795965,828226420,828306797,828322373,828323195,828332707,828332730,828332738,828332754,828334625,828407136,828407648,828407669,828407682,828408462,828408477,828408501,828408517,828573690,828573704,828573719,828579075,828579588,828579634,828579982,828580028,828811880,828811968,828811975,828811978,828812144,828970426,828970449,829274469,829419541,829420366,829420374,829420381,829422955,829423105,829426428,829432569,829434971,829435045,829435072,829435080,829435086,829436023,829520611,829528853,829530288,829626578,829658800,829935945,835687286,835697521,835697874,835738981,824690472,824729308,824762212,824788377,824828125,824908686,825304674,825304995,825710453,826750333,827301815,827412094,827412140,827412164,827412184,827412208,829192223,829192240,829192259,829192272,829192293,829192315,829192697,834299448,834302245,834367658,834368839,834631291,835754797,835754956,836269850,836317326,836677295,836677299,836686110,833327036,835851378,835851627,825170481,834011363,834011366,835724573,826052178,826052191,830306244,831778093,831778097,832842343,834908502,832682183,827770578,829043478,832872089,825021773,826066771,826028458,826028532,826520313,826520347,831755131,831829566,826047711,827130883,826047538,826047984,826491115,826537790,828729530,829767340,830955417,830955448,826347557,826782511,827234137,827234158,833823059,826990287,828133937,829264053,829268075,829964331,829964468,830494776,830500543,830500562,830500581,830614835,830631220,830640900,830735640,830819722,835564807,827176870,827713078,827713136,827713144,827713171,828001028,828078323,828105335,828670789,828954190,828954215,828954243,828954435,828959833,829259183,829564552,829564570,829564786,829843541,829843553,829843578,829843590,829843606,829843624,829843641,829843713,829843734,829843742,829843752,829843762,829843772,829843788,829843800,829843816,831533154,832056385,832074962,832074980,832074989,832075005,832075950,832075964,834240857,834580543,836479427,824751209,826184646,826204884,826403011,827022855,829678561,824734163,825536607,827154537,827156980,827157010,827676557,827942193,827942311,827942335,827942382,829091786,831503458,832031441,832575217,825108070,825565184,825565209,828037473,828037586,829607012,830945831,831518788,831827065,832951053,832953878,825232085,832152870,827289205,824546844,824546905,824595971,825443000,825460971,825522511,825597697,825597711,825597720,825597730,825597751,825598604,825598612,825598622,825598654,825598683,825611069,825621276,825739715,825745157,826146177,826267807,826360135,826360168,826360195,826360225,826575007,826585224,826651191,826651290,826651297,826651312,826651320,826651332,826651338,826651350,826651360,827169393,827213747,827881373,828056087,828078623,828492728,828493154,828580975,829086914,829090040,829090066,829090107,829090126,829090145,829090607,829679000,829714014,829723215,829723238,829723260,829734181,829734218,829734256,829734279,829734305,830094980,830095003,830182928,830182961,830182980,830326940,830326951,830326967,831630626,831631868,831635020,831635147,831635261,832246845,832370258,832660256,833251454,833350200,833705615,833706082,833706413,833951511,833971900,835634879,836108519,836451818,836453056,836453123,836453128,836453140,836453155,836454103,836454145,836454155,836475407,836535861,836661114,836670532,836670548,836670563,836670578,836670617,827597777,831734916,827063441,827063448,827625505,831942001,832787152,833621483,833862954,834064275,834559974,835295886,835325658,835456672,835786361,835790573,835790726,824614184,826226549,828719240,829262687,829265768,831496271,831685114,832829190,832829272,832829800,832831134,832833196,832833299,825952476,825952680,828425679,827691579,829058690,829359909,829359927,829359946,829359951,829360023,829360081,829637865,829637884,829637897,835884904,836051184,827679591,824825278,825746817,825755217,825794199,825817505,825831470,825832725,825964979,825981100,826007868,826012903,826304522,826309589,826309615,826323657,826378233,826928334,826928758,826991301,826995504,827059945,827107999,827139237,827520632,827556121,827562969,827576262,827597667,828152387,828157923,828177847,828689680,828689879,828691014,829756975,829816718,829826454,829832313,829832319,829832336,829832348,829832365,829832390,829832402,829832425,829832437,829832449,829833219,829833233,829833245,829833250,829833264,829833985,829833997,829834009,829834048,829834082,829834118,829834136,829834162,829834211,829834247,829834270,829834745,829834775,829834797,829834809,829834833,829834862,829834883,829834919,829834980,829835010,829835026,829835049,829835578,830828243,830832122,831029287,831029310,831329743,831368975,831759881,831869769,825096087,825807609,825807626,826293187,826344207,826345689,826345702,826345719,826345736,826345753,826345777,826345796,827357676,827641109,826727035,826728885,826754868,826925690,827004583,827220210,827220213,827378105,827519334,829336328,829421637,829421657,829421665,829421671,829725067,824815026,824820007,825184814,825581953,825912151,827567712,827567729,827567774,827567817,827567833,827567857,827567894,827567919,829173916,830888818,832478040,833856714,833856923,833856947,833856958,828850619,831403739,832275543,827823331,828562259,829930919,829986946,830403853,835213857,835658052,835669945,831248244,833145546,833145712,834928383,834928551,834928697,834929340,834929512,827874466,824836675,825053763,825054989,825187214,826587058,829986970,826345467,826615436,826619213,826623597,826625101,826626227,827355574,827697331,828199450,828971860,830291936,833411097,833411667,833460234,833463113,833463115,833463120,833463127,833463130,833463177,833463179,833463217,833463221,833463224,833463288,833463291,833463401,833463418,833463873,833463877,833463879,833463946,833463947,833463951,833463983,833463987,833463991,833464095,833464099,833922687,833923657,833924508,833925922,833992959,834001464,834001745,834008228,834008271,834930379,834932194,834934426,834946044,834946691,835021981,835225407,835226814,835250396,826054657,827051377,827076802,834575818,835073817,835101135,835102963,835542104,824701899,825889008,825889038,826354508,826444156,826444175,826856967,827022046,824922889,824922961,825017957,829990398,830014620,830014641,830014652,830014681,830014692,830014699,830014881,830015113,830015136,830015151,827689754,828299199,828299396,828300553,828354265,828354459,828354517,828354657,828354715,828354805,835621739,836032043,836032105,836032183,836032257,826089124,831843273,831843297,835176959,836027672,836383959,836384090,824941274,824716917,824792900,824792964,824792978,824793823,824793839,824793855,824793894,824793918,824808112,825314952,825316057,825321855,825321866,825321882,825321914,825321944,825321955,825768255,825962475,825962487,825962562,825962580,826399088,826399114,826399151,826886754,826886809,826886830,826886852,826886907,826886933,826886969,826887003,826887044,826887063,827342609,827342645,827342658,827890570,827987243,827987267,827987291,827987327,827987342,827987360,827987406,827987413,827987440,828940449,828940596,831193744,831195456,831195474,831195500,831195518,831195548,831195581,831195631,831195658,831195688,831195718,831195743,833791212,833813487,834060884,834062457,834062878,834063257,834189012,834189035,834189057,834189080,834189103,834189118,834189132,834189146,834189158,834189180,834189203,834189228,834189248,834189259,834189281,834189305,834189333,834191037,834191051,834191078,834191139,834191167,834191222,834191250,834191271,834191284,834191589,834191629,834191714,834191750,834191814,834191844,834191892,834191920,834192005,834192043,834192108,834192157,834192229,834192255,834192280,834192314,834192343,834192358,834192409,834192432,834192457,834192507,834192534,834192563,834192609,834192627,834192664,834192720,834192742,834192770,834192827,834192860,834192881,834192929,834192953,834192983,834193044,834193069,834193093,834193142,834193171,834193195,834338956,834615159,834981918,834981991,834982022,834982037,834982350,835326766,835328017,835328028,835328042,835328051,835328062,835328087,835328104,835328118,835328139,835328151,835328169,835328183,835436824,835437453,835438207,835438853,835439463,835443769,836261733,836261752,836261767,829809543,829809544,829831922,834630797,834633669,834635321,834635388,834636567,834643868,824498839,824514833,824916317,824916341,824916360,824916384,824916419,824916946,824916958,825960029,826088615,828126899,828181600,828559445,829536092,829536144,830175096,830636347,832820586,833407979,833415550,834151411,834893616,834894114,834895783,835170700,835170795,835530567,835604945,835606004,835609698,835611416,835613103,835672715,835672763,836018942,832502622,834110672,834110910,826667079,827321173,827453242,827774072,827835263,827835264,827836435,827836499,827855182,827855240,827858583,829236637,829463880,829468740,829507613,829769608,829790407,829827083,829975648,829977043,831595829,833199304,833896129,834624879,834634653,834645616,834660578,834661802,834662167,834666967,835348493,835349131,835875071,836392252,829940489,830438190,831595831,833859673,827942440,828740813,829250840,832988495,834378721,834382343,834489485,834495090,834495273,835641016,824613931,825461522,825897699,827264458,827264475,827264516,827264539,827264562,827264579,827264631,827264654,827264676,827264714,827264738,827264763,827264778,827264828,827264902,827264921,827264952,827267248,827267304,827267350,827267386,827976720,828353085,828365846,828365855,828365866,828365873,828365901,828365917,828365930,828365941,828365953,828365964,828365978,828365996,828647963,828647983,828648002,828648034,828648067,828648084,828648124,829338250,831929541,831929597,825245411,832520139,834936536,835319391,830549350,833050555,833911657,827037483,835493105,835664502,825621318,825750325,826373425,826415115,826861526,826861551,826972811,826972834,826972870,827394601,827504403,827504439,827504468,827504492,827504865,827504892,827567992,827605768,827605803,827606021,827696842,827835097,827925448,827925546,827925565,827925591,827926131,827926161,828060620,828060636,828060664,828246093,828926731,828926761,828926793,828926815,828926849,829044460,829046596,829066855,825556819,825583261,825647152,825744560,825897157,825897181,825907410,826076904,826097973,826098518,826540499,826562980,826564990,827024788,827252522,827491803,827683454,827895564,827904694,828369146,831124296,834063321,825465338,825976569,826918432,826950898,827156539,828694108,828754399,830854659,824964734,825044310,825216509,825323677,825323722,825323759,825323810,825323860,825323920,825323961,834648833,836265105,836265214,836265265,836298157,836602311,831089991,835750864,835780969,829493008,829992479,835265904,827017822,827164359,827402461,827873252,827884534,827905335,827907130,827923030,827923114,828879141,829334320,829449575,829450840,829807185,829826682,829956923,829978580,830285967,830423812,830425623,830446072,830800800,830803938,832157823,833203205,834193168,834675757,834989098,834990012,835307395,835367645,835874784,836158130,831882499,830641126,831175733,824505249,824722888,826418478,831632655,832176872,832176899,832176938,832178998,832179022,832179058,833126604,833889401,833889450,833889479,833889536,833889570,834945824,832853744,833327341,834451785,834534461,835055267,826029328,827783543,827960207,827960850,828043871,828043883,828043894,828043904,831076653,835210530,836230230,824758642,827249859,831121272,831280120,832777807,833080383,833517245,833583324,833583742,833586415,833639776,833937124,834273188,834273234,834278196,834334607,834433749,834706168,834707260,834711877,835007603,835011446,827794805,828548174,829560981,831153972,831901817,833309156,833550555,833639619,833732184,833861768,833869362,833876108,834768325,834803538,836315705,836323653,831328551,831328568,831328583,831329203,831329354,829409574,829410562,831387107,832117821,834540215,827622327,830363408,832985095,832988668,833646173,833751759,831205146,825972532,826250537,834317650,834317670,835804046,835804064,835804073,835804091,835804284,835804298,835804360,828865272,831223150,831749304,832267880,832620801,824721172,824721267,825248730,825455773,826391516,827247114,827273967,828349805,828363406,828678431,828851002,828854632,828854814,828858423,828863829,828887304,835379025,835379038,828916918,828936231,828961264,828961294,828961336,828961367,831231114,831839828,835263887,826197336,833023639,833343270,833343340,833343406,833436959,834001771,834464068,836265166,824984245,824997213,826523325,826523360,826938896,829062908,829089837,829862679,829862682,829862697,829862703,829862716,829862730,829862737,829862751,831909045,832111577,832709265,834461339,835322893,825524203,825586626,825588870,825590423,825674375,825885553,830054798,830056473,831495857,831499191,831499192,835712136,835839323,836218310,836234273,836651228,836652061,825123216,825887565,826061971,826237305,826343678,826667271,827019857,827061129,827069710,827128844,827566580,827594486,827597824,828611607,828796826,828892225,829140439,829369929,832338803,833804639,836325035,826411319,826438634,826783131,826791482,827402927,827423473,827625131,831027867,831081530,831339334,831764887,831766108,831771253,831782462,828190335,831234680,832159654,832920932,833817004,833822211,833841601,834403750,834403781,834403806,834403835,834403866,834403905,834403928,834403952,834403968,834403996,834404044,834404069,834404094,834404123,834404145,834404159,834404212,834404252,834404274,834474466,834879943,835422138,835424275,835866501,835874348,835874553,835874758,835875366,828064716,828617241,828856829,828860989,828865125,830265905,830327420,830430971,831187217,831311920,834688866,824850244,825085344,825584609,825812516,826487626,826487819,826514898,826970813,827012419,829943939,829943945,830275041,830490719,833191109,835242358,835288563,836514771,836514775,836514779,825894451,828519765,828519782,828519806,828519847,835213839,835316392,835394337,835549526,835549548,835549598,835549777,835553852,835554923,835555165,835556668,835558065,835559670,835559699,835559714,835559733,835680452,835801017,835862872,825763552,836080829,836080904,836690179,833550722,833550731,833550733,826244038,827098215,827340076,827340098,827340138,827340159,827340173,827340195,827340223,827340243,827340265,827340294,827340447,827340481,827340502,827340519,827340570,827340751,827340775,827340794,827340816,827340840,827340860,827340884,827340911,827340953,827340998,827341017,827341038,827341068,827341089,827341103,827341116,827341148,827341165,827602344,829786283,832744155,832744425,832744442,832744453,833392779,833416431,833416443,833420043,833848522,833850257,825538582,827020146,827020175,827673465,827674036,828475006,828480252,828524445,834433255,834538452,836963316,836963677,825982005,828574320,829764754,830876036,831146997,831149858,831434722,831436809,831437723,831671998,831763858,832146901,832815323,834270005,834987216,835111405,827498809,829748253,830615661,831018107,834688429,834688451,834688475,834688500,834688530,835386973,836279622,836279657,836279692,824968955,824589008,830972096,832778758,836851424,836857544,824746929,824747468,824939367,825048464,826030233,826382821,826877535,826877545,827024823,827024863,827346255,827641004,827997912,828224071,828425587,829854682,831465058,831688022,833367570,833369505,833595867,833654349,834586752,834659308,834665559,835968209,836167274,836168119,836168170,836169035,836169587,836171672,829013790,824972104,824978213,825321599,825343049,825782708,825790340,827587281,827587328,827587437,827587460,827778396,828080505,828219540,828667636,829046296,829051699,829172308,824805299,824989606,828554564,828559287,828559439,828902487,828920154,828921730,828928904,828947746,828949492,828960669,829141308,829147679,829222593,829722588,831032992,831033852,832537038,834976672,836729751,824504089,824980126,824980384,825076118,825076132,825076150,825076156,825343950,825450953,825495754,825498461,825499293,825500988,825501031,825501044,825501060,825503445,825503461,825503497,825503535,825503569,825503616,825503648,825503667,825503692,825503706,825503726,825503751,825503769,825503797,825503837,825503858,825503892,825503926,825504957,825505000,825505034,825505070,825505094,825505138,826254942,826254950,826254962,826254972,826254979,826255002,826255019,826255039,826255047,826255064,826795280,826916456,826916476,826916502,826916530,826935743,827496764,827496850,828006567,832382480,832435148,832435205,833115361,833115362,824534125,827190615,827246609,833138779,833139001,833139018,833139045,833139085,833139201,833139214,833139255,831937463,826849146,831630134,824954531,825884347,830933649,827293546,827190119,827300340,827863275,827866132,827866150,827868802,827896553,827931974,828895452,829447843,829453957,829460418,829469542,829484724,829512653,829975399,831084794,834625606,834626827,835271775,836149408,836159882,836232693,836232748,824590588,835909369,833648304,833648420,829339861,829585199,829589502,829593065,829593446,829596888,829606541,829677172,830217707,830220207,830220208,830226545,830228849,830668931,830856828,830921620,830921623,830921626,830921627,830921640,830921743,830946901,831778078,831888493,831889013,831889530,831889564,831891289,831893618,831895339,831917128,831919365,831927447,831927525,831927526,831928146,832797198,832875893,832876472,832878389,832878497,832878579,832878786,832879539,832880648,832880949,832881141,833134746,833148612,833601180,833601195,834279813,834279816,834279824,834280006,834280010,834406799,834588066,834588358,834589980,834592464,834843261,834847417,834850669,836087865,836087961,836088036,836088107,836088171,836088534,836088598,836088687,836088777,836089069,836089150,836089230,825625709,826293299,826350349,826484206,826582495,826582496,826630040,826644864,826819428,827215129,827466903,827690837,827724468,828012662,828029294,828217619,828230026,830080617,830089064,830133849,830158326,830222156,830225112,830386222,835566677,835574391,835589998,835980427,825538511,826048743,827036357,828020166,828093219,828739058,828932469,829343872,829871729,827919776,831350957,827757548,827888144,825679459,831472157,831472236,826383330,826492660,827324049,836487155,836487352,836670445,836670512,836670527,824835496,829987951,824575945,831539727,833280232,825613809,825614208,825617903,828209890,825637502,825637514,829865561,829867393,829872355,831441099,831782096,833421572,833422316,835327589,828395657,825208828,824922106,825764410,825977557,827535478,827729807,827791966,828371447,831121283,831578179,831600999,831852310,832199186,833053853,833300833,833332341,835738914,831785170,832073312,832073471,832074197,832074741,832736755,833048609,833049270,833054407,833054408,833054881,833059202,833074015,833379195,833379430,833504660,833505111,834106673,834224958,834226757,834226759,835227943,835228838,835229689,835253757,836994571,837005284,826075360,830035654,830285390,830757546,830757595,833479475,833566368,824497742,824497759,824497771,824497792,824912467,824912488,824912532,824912546,825419960,825419979,826542094,827622266,827622282,827622297,828480178,828480219,828480230,825105675,825105685,825105880,825105891,825105896,825105903,825105910,825105917,825105921,825105934,825105951,825106120,826304796,826304822,826304850,826305029,826305045,826305071,824680926,825296554,825768906,826350341,826351862,826523373,831382715,824756726,825374786,827031886,827314831,827814720,832114748,833151859,833249178,835331241,825446264,825848630,826297002,826308326,826327236,826544508,826544515,826544529,826544645,826544669,826544698,826632300,826637773,826637790,826639614,826784722,826811028,827569920,828570190,828658034,828674319,828674336,828674367,828773400,836670759,836670794,836670823,826200089,826246529,833152404,824833468,829021483,830571037,830860320,832148079,833889965,834085428,834085977,834092978,834094018,834096657,834101352,834141451,834145407,834146983,834157581,834165298,834165679,834189184,834189440,834190166,828589265,829593267,833787628,834063954,834157991,834191412,834191479,834338799,834338887,834338909,834338933,827628498,827628524,827628553,827628593,828230958,828230977,828261065,824867678,824867685,827069107,827122017,827126646,829654462,831104605,835801723,825620121,825646217,828136600,828618915,828717194,828741290,829041435,829082568,829156970,829184890,829585181,829967658,829967764,829967789,829967802,829967820,830056407,830067626,830067655,830127783,830173590,830182452,830212118,830216183,830724630,830797067,831401582,831696131,831696141,831696168,831696190,831697051,832125231,832196711,832220519,832284367,832784457,826685638,829944886,830032127,830401718,831464741,834405940,835049777,836410382,836666571,827444541,827821313,827979584,825225575,825726854,826323615,826610580,826610592,826610610,826616406,826670501,826670509,826729950,826739838,826882683,827180201,834793175,836122103,836275805,836303316,836433044,836445383,836462129,826922140,827381397,828862111,828866568,828867190,828872447,828926346,829113426,829119417,829119484,829124001,830057202,830291227,830960495,834293758,834293762,834296546,834296563,834610196,834610198,834770357,834770488,825382063,827039939,827148783,827166738,827172449,836633199,836633987,836705881,836714197,826558778,826595412,836295295,828238508,829683133,829683162,829683181,824756644,835816245,835816302,835816336,835816362,835816382,835819212,835819235,825119553,825601403,828804020,834018173,825346431,826718444,827091581,827134402,828394005,828394125,828560646,828187604,828746648,830036742,830059446,830059478,832780996,833076336,833094988,833108622,833172499,833716457,836722430,828993778,828994337,833544815,834755274,836641697,833599088,829045064,829086830,834639214,835121886,835121909,835121961,835122003,835122022,835122044,835122147,824726394,824732826,824790244,824801770,825392063,827028069,827028471,827186977,827186986,827187488,827187507,827187817,827187836,827187849,827272136,827317031,827317043,827317064,827317091,827317119,827317137,827317163,827881613,828747218,828935039,831353503,831385298,831851247,831851427,832110125,834130271,834224028,836102112,836269458,825682027,825686629,825692229,825695062,829759775,830953729,831131963,831951655,832081814,832081862,832081904,832081946,832081989,832082016,832082034,832082058,832082220,832082235,832082253,832082309,832928539,832928545,832928555,832928563,832928574,832928580,832928586,832928600,832928610,832928623,832928632,832928639,827373240,827496963,833420158,833420919,833420937,833420951,833420981,833420996,833421009,833421029,834434371,836107099,836654791,836655090,830838195,827414642,832415037,830616534,827288771,827297548,828262126,829323611,829329263,829329276,829887269,834552431,834552483,834552498,834552522,834552568,836197691,836692951,827478778,827768344,830343844,830379327,834222557,834222676,834790097,829238171,830264414,830561460,828726840,824582728,825392365,827716994,827716995,828754119,829543805,829845061,831079199,833807865,833814069,833814083,833814088,833983167,833983314,833988527,825234853,825234866,825234881,825234897,825234911,825234922,825234940,825320616,828544040,828657488,828681353,828716571,828716603,828716631,828716647,828716660,828911141,828964798,828964817,828964838,829173247,829244804,829276905,829317077,830108052,830108071,830108174,830422248,831743374,831806178,824727442,827628011,827912018,827912052,827912805,829316238,829536119,829909797,832736136,834912474,835923878,835982885,835984791,835985002,835986590,835988549,835988652,836022202,836022476,836024697,836024702,832563553,833481088,833485675,833485677,833485682,835804927,836099340,831367924,826909793,830652058,825274224,826755404,828316639,830453513,830636548,830636587,830637759,830637821,831313097,831938781,831943652,831946536,836383517,836594306,827695184,827725191,827729975,827738662,827745650,827957466,828093176,829257615,829268345,829274184,829309116,829334274,829352053,829369441,829530835,831578377,835657440,835829220,824480534,824499960,824725211,824876961,824995855,825027908,825521579,825524415,825528080,825528081,825528939,828634599,828652644,828653975,831761063,833356278,833970386,834501477,827129846,828773995,828816863,830894975,831423362,833980150,834537362,834537390,836031612,836509792,836509797,826296099,828652448,829277553,833646029,825204372,825204398,825235168,825235178,825235273,825235290,825235303,825235321,825388459,826261902,834468594,826682282,830660291,830958550,831235023,831704712,831768801,833715661,833850691,833853706,833853712,833853720,833853725,833853727,833853729,833853738,833853741,833853756,833853760,833853767,833853775,833853779,833853783,833853789,824842974,824945649,825493604,827547565,827873873,828572536,828573536,829038464,829197663,829774272,830017233,830673658,832060790,826614189,827655763,827720224,827733881,828476905,828587021,829005436,829006505,829069702,829424783,830075167,830547540,830893711,830900059,831351584,831420564,831526687,833974754,834342440,825474186,826596729,827267817,827568643,827568828,827568901,827568987,827569046,827569104,827628416,827628610,827628646,827628661,827628675,827628688,827628698,827718979,827721218,827902659,827902716,828471122,828490567,828578593,828578623,828578633,828669083,828771607,829137969,829317342,829843336,830063340,830898204,830900338,831226694,831301710,832271540,832551695,834211379,835047293,835303291,835321496,835576387,835585580,835753799,836028632,824979817,825107916,825442585,827119387,827749261,827750667,827902763,829088324,829257800,830057340,830057752,830893338,831043350,831049337,831798895,832121724,824570064,824570075,824570088,824950605,824956013,824978577,825005736,825025212,825064282,825064335,825064376,825088903,825091529,825096528,825098399,825100322,825100323,825100407,825100436,825101713,825102973,825108399,825108402,825108546,825108547,825108584,825111456,825111510,825119058,825474185,825500763,825500851,825570145,825582535,825601639,825848250,825849473,825919910,826579267,826590477,826592443,827167745,827686169,827763943,827936096,828653021,828666056,828750955,828752539,828753741,828754451,828754651,828758755,828759328,828759330,828762860,829827411,829846674,829848070,829848114,829870718,829926615,830313365,830343567,830345141,830900174,831040475,831335477,835150546,835392733,831572133,831766181,831766204,831766469,827123599,833566995,835846893,835848147,824652386,834266932,834994734,835383403,825319342,825322950,825322981,825323006,826517570,827660493,828947161,828947176,831645002,834034090,836294695,836453110,836479574,826986148,829890454,831070254,832068621,825274154,826495993,829026560,831335675,831335737,831335844,831335878,831335904,831335945,831335994,831336033,831504812,831593109,831624333,831717971,836465968,826030832,828555865,829277626,829792266,831350872,831350880,831350930,831959553,831959564,831959593,831959615,833897681,833897821,833956527,834084622,834732895,828359062,828371289,828371363,828891666,829211945,833833458,833833568,835097858,835124416,835728402,835732346,835740312,835740809,835817453,835832398,835858274,836086206,836131284,829441417,833416740,828869996,828871064,828879120,829148075,829210665,824815476,824923334,824923413,824923428,824923450,824923467,824923475,825052225,828932360,828932383,828932401,828932442,828932478,828932496,828933085,828933112,825458158,825491156,826962284,827731770,828268259,829371970,829381677,829809987,829836551,829842419,829843488,831071426,831210710,831258733,831621818,831622312,831626052,832072663,832274199,832274200,832278525,825978880,827386212,827488315,827488393,827488409,827584579,827584595,827584656,827584678,827711970,827714136,827804701,828926781,829030262,829030552,829030586,829030636,829030793,829108920,829397348,829399924,829399931,829399932,829399936,829399939,829399941,829399942,829399945,829399946,829399948,829399951,829399956,829399961,829399970,829399974,829838381,829851317,829851322,829851345,829851350,829851355,829851365,829851373,829851384,829851392,829851564,830156882,830156893,830156928,830156949,830156979,831091701,831335719,831335798,831468432,831643887,831660500,831791926,831999957,832023134,832121402,832245219,832249533,832251802,832551764,832552603,832557645,832727113,833001169,833001176,833001193,833001205,833001219,833001230,833001245,833001469,833001480,833001491,833001506,833001525,833001535,833001556,833001844,833001860,833001878,833002245,833002262,833002284,833002897,833002951,833002970,833003001,833003023,833281623,833437087,833442436,833450264,833450270,833450272,833450276,833450278,833450285,833450288,833450292,833450346,833450351,833450354,833450401,833450403,833450407,833450413,833450417,833450605,833450619,833450625,833450715,833450734,833450738,833705877,833705906,833705919,833705935,833705951,833706303,833706320,833706333,833706342,833706356,833706545,833706558,833706571,833706594,833706607,833707037,833707054,833707068,833707092,834019456,834272556,835268120,835713122,835713136,835713219,835713233,835713644,836356469,836638024,833220514,833225463,835052961,835055488,835077415,835155381,835155475,835302974,835305389,835306279,835773589,835776085,825930013,833311409,825292573,825379662,825379683,825379704,826344759,829304148,831911072,832309975,832310000,832950035,834291029,834732684,834960614,835066896,835074478,835233854,835246653,835246731,835246887,835246962,835581703,835789134,835928802,836063966,836063979,836064983,836064987,830345453,832212083,826314372,827162835,828770973,829851607,829851654,830156909,830609197,831680024,829270040,831117926,831129447,835293833,835298418,835310462,824570821,824570846,824570852,826797761,826799883,826799930,826800598,826802306,827125678,827128020,827128061,827730487,827730497,827730507,827730517,827927346,827931542,827931559,827931572,827931583,827931618,827931635,830011056,834290553,834290564,834290598,830865103,832218665,832366116,832412118,832417299,832423196,832428938,832437980,833051639,833184569,833335804,833561705,833575090,833743549,833768025,833846245,833846258,833846263,833846268,833846283,834201121,834243658,834243685,836418856,836418878,836418907,836418919,836638153,836638222,836638233,836638301,836638322,836639673,836639704,836639731,836639851,836639866,836639883,836639893,836639919,836639930,836642023,836642032,836642040,836642113,836643315,825299678,825299695,825299770,825299803,825299816,825299834,825299841,825299848,827031436,827360806,827360840,827360866,827360999,827361041,827361071,827361083,834687265,836166347,824965531,830345213,830616464,830998500,836460021,826391400,826614547,826491256,826506523,828385373,834417343,827815921,827815949,829271969,829516321,829516758,829637988,830089505,830456925,831271196,831708403,831868824,832051560,834001385,836483785,825977992,827004680,829832489,831191461,831209106,831209107,831214373,831217956,831229064,831242231,831657483,831658633,831669342,831677699,831677749,831678619,831682917,831692609,832189530,832430632,832691525,832692845,833045501,833070708,835977574,835977593,835977601,835977633,836056759,836056767,836056773,825819581,825886069,826411789,830128158,830128173,830128186,830128195,830128222,830128236,830128423,830128433,830128452,830128463,830128478,831352789,831978553,832295034,833119215,833150158,833242744,833802314,833802333,833802350,833802369,833803110,833817030,833817090,826132425,830416481,830416989,830452342,830908285,831494048,832100968,832100985,832100996,832101021,832101036,832101052,832101061,832101073,832101088,834106990,834610679,834610850,834610861,834610870,834610876,834610880,834610884,834610885,834610888,835409831,835410378,835410630,835410918,835411148,835411664,835411902,831704758,832365167,832660672,832953753,831328283,831328300,825040432,826035683,828903842,832265094,833667868,834611709,834611800,834795912,834532666,834532682,834532707,835982465,824582342,824582355,824582362,824582373,824582376,824582381,828805262,831405760,831405790,831457461,831677003,830336229,830351074,830713228,830852802,830861393,830867570,832073130,835997533,824509103,827035675,827575139,829203060,829425991,829851990,829991564,830006351,830155553,830431553,831408794,831495034,831495059,831527420,831527444,831527465,831527479,831527496,831527522,831527532,831618074,832110941,832289062,833996879,833996887,833996900,833996940,833996954,833996984,834932679,834982934,834984554,834984560,835146818,835570430,829667565,830957906,830921636,827740524,828547824,828547872,828547910,828547935,831235371,831235391,831235416,832160501,832160533,832160549,828562601,828562626,830785873,835244887,825163444,825163640,827479273,827774030,828660291,828980075,831315117,825662057,833644616,830968002,831102626,831137035,831137211,831137621,831137738,831137757,832672768,832672790,832672795,832672803,832672818,832672833,832672853,832672862,832672887,832673848,832673860,832673886,832673914,832673940,832673964,832673976,832674004,832674012,832690984,833141563,833199085,833264425,834310841,834668084,834838579,834923481,834971611,834971633,834971652,834971931,834971954,834971968,834971995,834972018,834972037,834972049,834983249,834983391,834983583,834983596,834983616,834983629,834983640,834983650,835920704,835920718,835920733,835920750,835920781,835920989,835921053,824967190,824987111,825187537,825279859,825521169,825522943,825526546,825824939,826097551,826422561,826422589,826674683,826674704,826674724,826953525,826954013,827069867,827236491,827559925,827669268,827682707,827795762,827809789,827986070,828184956,828187262,828530501,828538367,828933537,829000851,829064577,829095067,831550201,831795376,833482472,834611752,834938653,835067701,835067717,836041819,836041956,828638522,828638542,834418712,834609452,834609470,834863833,827831877,828365103,829143814,829394382,836652893,830825416,831774869,825396759,828429330,829261062,830141966,830143940,830145071,830146292,830149515,830335205,830340130,830340149,830341323,830341669,830342080,827302448,829227800,829228023,829284291,829527972,834536513,834536528,834536699,834785769,834922087,834970464,831853676,829670008,830162705,830311757,824538036,824538045,827815615,828717351,825977135,825977153,829265793,829793194,829836250,836434485,836441173,826735179,826735319,826735404,829000715,829001980,830253639,826602562,835152695,824579382,824604198,824617336,827517591,827546494,827658680,826155849,836568519,833942317,833944821,824564051,824564064,824568902,824568912,829593243,835191362,835269804,835271350,835275596,835275598,835276726,835279550,835282914,835290238,835296600,835297066,832810284,832942540,832983848,825576954,825578718,828022510,831003016,831168406,833200136,833200157,833525808,833553955,833553965,833618438,834818671,827881259,827882413,827885466,827885467,827894046,827900016,827900153,827902493,828163797,828171922,828181000,828186595,828188882,828189365,828189594,828404356,830966360,832270084,836262001,831392977,825681489,825684881,827680393,827720049,828266975,828426375,829755760,828918857,824986802,824986825,824991826,824993700,825818895,825828861,825971884,825989772,825991383,826007279,826676933,826783767,826784312,826926026,827259433,827274355,827334765,828863158,828886804,828887276,829025746,829126522,830405723,830414431,830651890,831574192,831637265,831670673,831708494,831885875,831896896,832336482,832339201,832343784,832808805,832935180,832952716,833377611,833666050,833966771,834047063,834308821,835464937,835471685,835948634,835948687,835948738,835950183,835950258,835950284,836205185,824547487,825110633,827161526,827389695,827437311,827439200,828455214,828498937,828705321,825519364,825595155,825596660,827813807,831464011,824727951,825107030,825108633,825108834,825128975,825137066,825137072,825137075,825752086,826067965,826289414,826289441,826289465,826289517,826289579,826289673,826289694,826290332,826290384,826290400,826290424,826290442,826290468,826290508,826290534,826290577,826293280,826293306,826293329,826293349,826293384,826293414,826293441,826293467,826293485,826293527,826293557,826295225,826295246,826295262,826295276,826295292,826493136,826493166,826493487,826493680,826540448,826545814,826561961,826561973,826562108,826576877,827009964,827067264,827130268,827544294,827629800,830064067,830348981,831220689,831614145,832316473,832579020,832656182,833263790,833367528,833367718,833483086,833504449,833514801,833602673,833606618,833607016,833607996,833608303,833608320,833608339,833608539,833608565,833608609,833608813,833609098,833609242,833609265,833609296,833609343,833610208,833649240,833689605,833689638,833689706,833689760,833689796,833689842,833696498,833696524,833721457,833721476,833721499,833721520,833721546,833721564,833721587,833744183,833744198,833744212,833835727,833835764,833835985,833836141,833836170,833836212,833919842,833920131,833996130,835818844,835818868,836050909,836051544,836052181,824806883,825035158,827053646,827057714,828876653,828884215,828884240,828884264,828884313,828884328,828884355,828884373,828884393,828884417,828884435,828884450,829035185,829507890,829507908,829507967,829508027,829508049,830747467,830747482,830747504,830747525,830747536,830747569,830747603,830747635,830747673,830747696,830747722,830747747,830747762,830747783,830747806,830747823,830747837,831568697,831568724,831568761,831568785,831568807,831568855,831568900,831568944,831568984,831569015,831569052,834084185,834084202,834084294,834084315,834674960,830014715,833712865,830012249,830026491,825941140,826331993,826681070,826694445,827571633,830339146,830689330,830905658,831019171,831098925,831126497,833402144,834323816,835295437,835366676,835606111,836002987,836006832,828809118,830941664,831215906,834709463,836257385,836595526,825407058,827964092,827987223,828422912,835836919,826095236,827451303,827773006,828498597,828499059,828499078,828611942,828624009,828624029,828716268,828717121,828717133,829839404,829839414,829839426,829839441,829839451,829839493,829839501,829839505,829839521,829839532,829839549,830176615,825353795,827839746,835275325,834454980,835764618,835766377,835771602,835799589,832277863,835005776,835015729,829916318,830669829,831510824,831676566,831905002,824874756,829078876,829661803,830873935,830879283,830881102,830882358,830889238,830890464,830890926,831112561,831114274,831115893,831120546,831120644,831124567,831128846,831129323,831149107,831787684,831788643,831788780,831791167,831791308,831798859,831800188,831802690,831804300,831805864,831810422,832250712,832251309,832260690,832260692,832261461,832267155,832267589,832269382,832272961,832274181,832281812,832849218,832852034,832853954,832854540,832857900,832858792,832859102,832859351,833112721,833112821,833126218,833128381,833129801,833159928,833163241,833163282,833165639,833166818,833168838,833171687,833172734,833696293,833700785,833700787,833891187,834282205,834284176,834287624,834287723,834318834,834323258,834324685,834375092,834924400,835119089,835362598,835362895,835365126,835381556,835422824,835623687,835624151,827659861,832207392,832301232,832308896,835605450,836154312,828506608,830706187,832325779,824996168,824996189,824996218,827366686,827477293,829150358,829422837,829423454,829491908,831882179,832427101,832601792,832613451,834191433,834212551,834216264,834298672,834345017,834624845,834640979,834641368,834722972,834724130,834753517,834761518,834761592,834789410,834790075,827747805,834022468,834038836,826791459,829459127,829461444,829641490,829903599,829903600,829903601,829903615,829903617,829903622,829903627,829903630,829903635,829903636,829903641,829903646,829903655,829903658,829903708,830070900,830071218,830071460,830075432,830075446,830075468,830075497,830075513,830075557,830075565,830075662,830075680,830075707,830440740,830443822,830443859,830702145,830813962,831271972,831271983,831272011,832677414,832677636,832677642,832677651,833496940,833570960,835628595,835677552,835812790,835836872,835836896,835845425,835845461,835845487,836398280,836398293,836398345,829071425,829875565,829877628,831133543,831328795,831394737,831744714,832198453,832468469,832479485,833185157,833869722,834751692,834768787,834773334,834779213,834866609,835112541,835161865,835255608,835319899,835411659,835440694,835495378,835893354,836017090,836018436,836019491,836022664,836025545,836025731,835242456,824472934,836446181,831808595,825428249,825456212,825470415,825487731,825490918,827437932,827634298,828117981,835632939,836054780,832337299,832337462,832337605,832341433,832343765,831783826,833677067,831597612,825290621,825496620,826391228,827051252,827061817,827979274,828868981,828898256,828898263,828898277,828898289,828898306,828899833,828899850,828912385,828912402,828912409,829201782,829207043,829207242,829207259,829207290,829207313,829207335,829207373,830191441,830211056,830211069,830211093,830215585,826820585,826077711,826481979,826772413,828953753,829006956,829125400,829720747,833060506,834001397,835494311,835494350,835494420,835494439,835834041,835834061,835834075,835834091,835834109,824488648,824997652,826462596,826482278,826911787,826932017,827255716,827255721,827262105,827319879,828872958,828885240,828900119,828907968,828915608,829034392,829631122,830407018,831370491,831511951,831512967,831512968,831524229,831625724,831654643,831654678,831662622,831683334,831708895,831715598,831715744,831717208,831767610,831796458,831879827,831880964,831881086,831885256,831916248,831930148,831930168,832333684,832439176,832522020,832524635,832544025,832561729,832568643,832568843,832569260,832569404,832896804,832896853,832903181,833170575,833471397,827338837,828216540,828216636,828216682,829301830,833752419,833752470,833752485,833752516,833859058,833859066,833859073,833859076,834066405,834694629,835104835,830680975,830681009,830681034,830681053,830681527,824690998,825409422,825900533,825922314,825928075,825972300,825981995,825982261,826071451,826175792,826180263,826338058,826406540,826407458,829168262,829443670,833800650,824921050,836716509,824799795,824804888,824851928,825020518,825020560,825434101,825932425,826018547,826082192,826084077,826227264,826273055,826273676,826276768,826277409,826341184,826442573,826826767,826876753,827014294,827036620,827317295,827977065,828449777,828449789,828449794,828449849,828449861,828540030,828632688,828635268,828760326,828873462,828963445,828963473,828963494,828963535,829308410,829411889,829427863,829582680,829675912,829770482,829802130,829820769,829838871,829875465,829875679,829875680,829876114,829883746,829918109,829961997,829961998,829961999,830031294,830312656,830326380,830363027,830419526,830421842,830440100,830465250,830542455,830611884,830652466,830670701,830761583,830874133,830878889,830880984,830904555,830904982,831000219,831010167,831020620,831108378,831108393,831108414,831108431,831108524,831108559,831108570,831108612,831146159,831154596,831220886,831314557,831375117,831375138,831375187,831406375,831409043,831434743,831459765,831470302,831470314,831470320,831470335,831470349,831472667,831472828,831472842,831472849,831472862,831502936,831514281,831514294,831525447,831526609,831545716,831573400,831642331,831760604,831798429,831821378,831846202,831850698,831850829,831852347,831852685,831852743,831852746,831854231,831855543,832129993,832141889,832189644,832189656,832189920,832189937,832195344,832307474,832326499,832326506,832344212,832350026,832366672,832515830,832699894,832760374,832782667,832835833,832892405,832892892,833359830,833396196,833396220,833517689,833535991,833538169,833541504,833542190,833958066,833965108,833965121,833965965,833966314,833966328,833966333,833967392,833971369,833971985,833972014,834006902,834034122,834035085,834035391,834047355,834047800,834049272,834050394,834115827,834129105,834231193,834239654,834360265,834608764,834608765,834608770,834613132,834613136,834617080,834620412,834805684,834805699,834805711,834805726,835013466,835013488,835013523,835013545,835013569,835013604,835013634,835013651,835013677,835013703,835013724,835013746,835013769,835250476,835253600,835254129,835254143,835254158,835254164,835254295,835254327,835254338,835254354,835254393,835254403,835256286,835257579,835258450,835258463,835258471,835258676,835258699,835258836,835259858,835259865,835259869,835361173,835362658,835656482,835656522,835657783,835663447,835688977,835716233,835741424,835741810,835741853,835745400,835746338,835746591,835746901,835747210,835747436,835747667,835747942,835748719,835748982,835749029,835749331,835749347,835749371,835749473,835749501,835749510,835749514,835749519,835749531,835749538,835749557,835749564,835749572,835749576,835749586,835749600,835751379,835751812,835751886,835780175,835786411,835786418,835819184,835819214,835819279,835280130,835758577,835759043,825830003,825830014,825830047,825830057,825830065,825830075,827017144,827127264,827131201,827339220,829821296,829822237,829822281,829822296,829822317,829822335,827935870,831263325,826686125,826686148,826686214,825520679,825523929,825551579,825742390,825982870,825998493,826037922,835234982,828022878,828294575,828299004,828618156,830086300,830206192,829647008,824651711,824874470,825366397,829637857,833728102,833758940,827195067,827200316,827736544,828270035,828279714,828284437,828285665,834114029,834187491,834201112,834328978,834347714,834348566,834359752,834360257,834366822,834367188,834963176,834996702,835018973,835411521,835415997,835416265,835431476,829557298,829561248,829575257,829603131,830094075,830096544,830109951,830110450,835604957,835605181,824503936,824539904,824539913,824943192,824946987,825214976,825244948,825244960,825246121,825352700,825386334,825386339,826093240,826094441,826919689,826919737,827115486,827115510,827120593,827410440,827412128,827415000,827415015,827415025,827415081,827415093,827415111,827415414,827415448,827415474,827726107,827727399,827727412,827727427,827727440,827727453,827727458,827727479,827727601,827727620,827727640,827758514,827758519,827758526,827758533,827758537,827758543,827884605,827884625,827884646,827884767,827884790,827884845,827884863,827884884,827937673,827937709,827937787,827937833,827937847,827937873,829554415,829841425,829841439,829841446,829841455,829841984,829842649,829842712,832576120,834003342,834003348,834142971,834806478,834806497,834806509,834806605,834806776,834806810,834892172,835136661,835136683,835136700,835136750,835569215,835707511,835707684,835707802,835707957,835710273,835710284,836052046,836053079,836053085,836055187,836055687,836604554,836659950,836660733,836663276,836663311,836663338,836663364,836663378,836663403,836920207,826049488,833594715,832017299,834960841,830141200,832556672,833773168,833792625,833898326,833950551,833953613,835765868,836056460,836494762,836494845,836495273,836495595,836514482,836514641,836570537,836620752,836624925,836625007,836625048,836638867,836677639,836687827,836689612,826325494,831329318,832286121,833180935,835359316,835403608,835459823,836043247,836043521,836044457,836044583,827252726,827252735,827264810,827264870,828564729,828823250,835807491,835808070,835808074,835808581,835813788,835816238,824817287,825204736,826184285,826312313,826651275,827285760,827453718,827692139,827729506,827829745,827829753,827829769,827829784,827829796,830606067,832090891,834836838,834841631,834841657,827575702,828160773,828705731,825933410,836080830,828330778,828778228,829353949,833990375,826816608,826817526,826871127,826934460,826934952,826944576,826944994,826952937,826953050,826954965,831997513,832517975,828084356,828084407,828084627,828194799,828194813,833513316,834946828,835401640,835479345,835482187,835902694,835953040,835959724,835976811,835979271,831242259,831410340,831415620,827286767,827485018,830791871,831304273,831342806,831918930,836653707,835251239,824742882,828864631,829207944,829212433,830320251,830792281,832093380,833085412,833404280,833654667,834143023,825682063,825682097,826030494,826071449,827601818,828550625,828551566,828551909,828554758,828554759,828827872,829107789,829318536,829322380,829431909,829433595,829437027,829551132,829551133,829924441,830390985,831896227,831902614,832324069,832843604,834044643,834452511,834614469,834614473,834810042,835425501,836050632,836050707,836505436,836505789,836506010,829888940,824748226,825218206,827663730,829306274,829619553,829801434,830030450,831063169,831244721,831247608,835408827,825475398,829019249,829658864,829773462,831050089,831464633,831487484,825288782,826377460,826392802,826422976,826423010,826423029,826423046,826423066,826545930,827639061,827640991,827722840,829184305,829690347,829690359,829690467,830205320,830236404,830255332,830255358,830255386,830255419,830255457,830255502,831580159,831580172,831580193,831580217,831580245,831580274,831580286,834047310,834348097,834348111,834348156,834348189,834393205,834590883,834591516,834775204,835196189,835202394,835203576,835376695,835692395,835697794,835704488,835704650,835706154,836505224,836505226,836505229,824812797,829607927,834813491,826217737,829816844,829816868,829816877,829978787,829980069,829980224,829980314,829980367,829980413,830064476,830064490,830156014,830166221,830697663,831746367,832475353,833264434,834113151,834676615,834676641,834918618,834992992,835223336,835225595,835306466,835307867,835314957,835314966,835314977,835315017,835315246,835315260,835315267,835315334,835316158,835316168,835316179,835316197,835317273,835317353,835319292,835319402,835319548,835319763,835319951,835319998,835320062,835320396,835320569,835320614,835329105,835330754,835330828,835331583,835331623,835533312,835533724,835533824,835533968,835543808,835602000,835602003,835602006,835602009,835602011,835602014,835602021,835602029,835602034,835602038,835712325,826990678,827918907,828899438,829058045,826343398,826343442,827218928,827218932,827237277,827237285,827237291,827237295,827237297,827237300,832010726,832898949,833420129,834209965,834209982,834209986,834210005,834210023,834210097,834210121,834210223,825320736,836817028,824487343,824488651,824520368,825033278,825384057,825384058,828182339,828285764,828654151,829257353,830429660,830562890,831063258,831153154,831365325,831365454,831366273,831625666,832438513,832558049,832692091,833687239,833717923,834037314,834045989,834055644,834613175,835336280,835356804,835374327,835883170,835885359,835918244,835927307,835942766,836208945,836504361,836504362,837017283,825021967,825354007,825441629,825474729,825941416,825956289,825976657,825991746,825997049,828179947,834402840,829545594,829969944,830028302,830389874,830920499,830936115,830979626,831002105,831017940,831017974,831480038,831480242,831653256,831653766,831823249,831902380,831912294,832403030,832429241,833529343,833934589,834838532,834932429,834948868,835110557,835197320,835200610,835289844,835290559,835810981,835811203,835885459,836062878,836067511,836079888,836079889,836190821,836213698,836223747,836539245,836556759,836561625,836571758,836572250,836584209,836637859,836638080,836642557,836722950,836977293,836977965,836979580,836981027,836987570,836988393,836991044,836993775,824476086,825487989,830459256,831346790,832065099,832360756,832376885,832385377,832474604,832508182,834845991,834846508,834846509,834851038,834861668,835033220,835894565,835896382,835896871,835897606,825641864,826339459,829727404,830082501,830101740,830111230,830118474,830401475,830416142,830795288,831599176,832953019,833080460,833081923,833369665,833369666,835536766,835954198,835990295,835994482,836045676,836045772,836045892,836045939,836046043,836046250,824714269,829214660,832831053,833940069,831630246,828698059,835857148,824631514,824631692,825109381,825166363,825166370,825261945,826531733,827474870,827657399,828051153,828051174,828051229,828051245,828051262,828051299,828189900,828683308,829259019,830175199,831448147,831448152,831448154,831448156,831448202,831448206,831448209,831448213,831448218,835974378,835984742,825081900,825413398,828829479,830174810,831323360,831823109,831950716,833164466,833168920,833204078,833622624,833634720,833635637,834287493,834306279,834341779,834343671,834429907,834630937,834739587,834756606,834764410,834773137,834773657,834773914,834774452,834775433,834781943,834787253,834787299,834798176,834800365,834807052,834965194,835038500,835432206,827384503,827386248,828359446,829743462,830073270,830162252,830162274,830167781,831951563,824874836,824883464,825910666,826911632,826977904,827165802,827166338,827166477,827176995,827243333,827265727,827361310,827425123,827425284,827426747,827434184,827439141,827963922,827965593,828069281,828069343,828070110,828072229,828521497,828639861,828890113,832492281,832551326,832999291,835138287,835139266,835139330,835970533,836061325,836061554,836061562,836061568,837003509,837109372,829311355,831820389,831918336,832420102,832830099,833153549,833247089,833247110,833247138,833247161,833255379,833374388,833415154,833513350,833598767,833866146,833866168,835411822,835521239,835541095,835541541,835541905,835542275,836378144,836557372,836557416,836557442,836601958,836636378,824533023,824554897,824569216,825088927,825221134,825845525,826412100,826560276,826716660,827388887,827433125,827464632,827976408,828364481,829711778,829711796,829711816,829711823,829711835,829933216,829935564,829935841,829939125,830211663,830228477,830245528,830249205,830257948,830275898,830364009,830364793,830365617,830369747,830447656,830641774,830763064,831582774,831742902,832843830,832868986,832871900,832872980,833405971,834873446,834880819,834903535,835649272,835653728,835654095,835660283,835673233,835674359,828890650,828892355,828892427,826137272,826425803,828834505,828922988,829136923,829157628,829157993,829158013,829158046,836953987,824594826,824594841,824595235,824735771,824920080,824920211,825060296,825060326,825060343,825060365,825060389,825060408,825060428,825060445,825209360,825209705,825209743,825209777,825209889,825210132,825210147,825210161,825210223,828796389,829033091,830780978,831271391,831284202,831604473,831872779,833393549,833393583,833681809,824484890,825163760,825297745,825297747,825305114,825309981,826147393,826149464,826154696,826156661,826664746,826666213,826667198,826667416,827002712,827015127,827015136,827229157,827230212,827232767,827253437,827255653,827794952,827795918,827799214,827818654,827818872,828224131,828300807,828301204,828301206,828301984,828301985,828302727,828305405,828313510,829107107,829107129,829107477,829107515,829114351,829114437,829114515,829116745,829119434,829119490,829119517,829119590,829119618,829120814,829962360,829965688,829965711,829965845,829965866,829965890,829965894,829999667,830010167,830145650,830145671,830145723,830992805,831293294,831393074,831393090,831393140,831630558,831886093,834061157,834181485,834194626,834196714,834265741,834612767,834741949,835655738,824713221,826090359,830617864,830875080,830875181,830875201,833403022,833460715,833768870,833780017,833965263,835134232,835138413,836297204,836297226,836297239,836297279,836382806,836512481,835401422,835409146,825810474,834217759,825038232,826009339,826042058,826300557,826461964,826598541,826854264,826976170,828283323,828896072,830112877,830210476,830589535,830648586,830992121,831278911,831977112,832076249,832528037,832974536,833921836,834343106,835077068,835337970,835338096,835338593,835338611,835340783,835341337,835341361,835344032,835348382,835348655,835350032,835353497,835357482,835361548,835363933,835363987,835363988,835377555,835387518,832444792,832737998,835277219,835303051,824482101,824482130,824482236,824483142,824483159,824483191,824663121,824663136,824663149,825102586,825252726,825291134,825291157,826977873,827004339,827004397,827004808,827004836,827430658,827496885,827881632,826350731,827068875,828845294,833589926,834064013,825620646,825169891,829551411,831917099,831930454,831930473,831457700,831460164,831477929,831477952,831935544,832339101,835225524,827470342,827546948,829588312,831132380,831137299,831142989,831151105,831346799,831741216,832771199,832997245,833327402,826469177,827243060,829411681,829479672,829538937,829709096,829769329,830000579,830001087,830095752,830155963,830182469,830225557,830384527,830386226,830393187,830394792,830425339,830425459,830432412,831748313,832621277,833847626,829412533,829414125,829414132,829414187,829414274,829414278,830109424,830131713,830132979,830393940,832125155,832126900,829840771,830046469,830216108,835200277,835440651,836379746,829700569,830479109,834825224,836678568,824939769,824939808,824939879,825087058,825096056,825266979,825391338,825603922,825604928,825605769,825608376,825701355,825703120,825704601,825705963,825708506,825713671,826453141,826453150,826453190,827230434,827254547,827254568,827288619,827351383,827481646,827481700,827481738,827482253,827588314,828323734,828377519,834018654,834202015,835712011,836094399,836095395,827766998,830697616,825305381,826323274,826427236,826481392,826481402,826481411,826481420,826481440,826481458,826483035,826483156,826492814,826746120,826746612,826747792,826747805,827170041,827413561,827438588,827570102,827570848,827612824,827612840,827612883,827612891,827612902,827612909,827614077,827614094,827614111,827614127,827614165,827614188,827614200,827618497,827619377,827619430,827619471,827619528,827619574,827619620,827619667,827619704,827989764,828636342,828636352,828636402,828636419,828636441,828636534,828636548,831776041,832102194,832370779,833274561,833275653,834895297,835263485,836165758,836576082,836725012,836923628,824531627,825397217,825864151,825873485,825886474,825886817,825957232,825987428,826036666,826132344,826390418,826433043,826763090,826809456,826901341,827239556,827246204,828157564,830032112,830766144,830766559,824509806,824843504,824861430,824865179,824865315,824865377,824865428,824866041,824866216,824866277,825163704,825279832,825279932,825279974,826437294,826911873,831388781,831395211,831409818,831409886,831411992,831412004,831412166,831412195,831412237,831412289,831412399,831412458,831448717,831448861,831449024,831449037,831449107,831449131,831449147,831449157,831869057,831869069,831869257,831869261,831869268,831869274,831869282,832458339,832458444,832458545,832458576,832458613,832458657,832458869,832459820,832884395,832884414,832884420,832884462,832884691,832884703,832884714,832884719,832884724,832884728,833331324,833335133,833692467,833692565,834013555,834013566,834013585,834072191,834072299,834076008,834076435,834320898,834321105,834321151,834321193,834321265,834322076,834322140,834322180,834322448,834322524,834322602,834323453,834334943,834335177,834335228,834335300,834338078,834356491,834356561,834356586,834356595,834356662,835141304,835766905,835766947,824583483,824587114,824596581,824767055,825081311,825082422,825082432,825122741,825136503,825313829,825316192,825322647,825322661,825322705,825322733,825470237,825496521,825531064,825531348,825533574,825533878,825563012,825574408,825582162,825585148,825617076,825623367,825625713,825626111,825629642,825632531,825635857,825955058,825957237,826016471,826023498,826093376,826125889,826210812,826246486,826606587,826608394,826626965,826657954,826717449,826785342,826848912,827198090,827213702,827213708,827213712,827213720,827213725,827213727,827213731,827213757,827213769,827213773,827213778,827213781,827213788,827213790,827213796,827213802,827397755,827547264,827764287,827764290,827764396,827764403,827764404,827764407,827764418,827764436,827764447,828087637,828285727,828285737,828285739,828285744,828285755,828285756,828285758,828509457,829900680,829912557,829912784,829913026,829913029,829913049,829913053,830003113,830150793,832510257,832896285,833244401,833464770,833464785,833464807,834020968,835215429,836044193,836044196,827895604,828059941,828059960,828415433,828415453,828415469,828415485,828415519,828415541,828415625,828547856,834257306,834257390,827655907,827923595,828006536,828204378,832991552,832994087,833262440,833265811,833269289,833271810,824742073,824867278,825283650,826646170,826766941,827238406,828134841,828135145,828247741,828247751,828247763,828274616,828467378,828535617,828535655,828535708,834390593,834022793,834022794,834036338,834480636,834484181,834494753,834495078,834507070,834507122,834510848,834511415,834511566,834520402,834526985,834527006,834527731,834608255,834608824,835893186,836395555,824501144,824501155,824501165,831495468,835381626,835387648,835944638,835948332,826153451,829938888,834609500,834609509,834609518,834807740,835134612,835134629,824880511,827148857,827885882,828576061,828755395,830512487,831057350,831703137,832709630,834214118,835944306,835945582,835965821,835967611,835996889,824651020,824958684,825064281,825069148,825849680,826011934,826153951,827751540,828762735,828765111,829137952,829138315,830895199,831320550,832410452,836680902,836915643,825595128,833989791,833989813,836736125,836778155,836933474,836939045,837046930,837048563,837111200,825326503,828539703,830214497,830214516,830214550,830425319,830425343,830425347,830822129,831735125,832323013,833590160,835710968,829751139,824891619,825225475,825226124,825226914,825226965,825227241,826713957,827303138,829413822,830414923,830666849,830666979,830667265,827656332,828704533,828704546,828956248,829024144,829130964,829131015,831886700,832256975,834162504,834162540,834700884,826444164,826029673,831640297,825335346,827848480,827849113,827964672,829115335,829266765,829266777,835521072,835522802,836044950,836050367,836051699,836263297,836350590,836370769,836386384,836550501,824606696,827362396,827569705,827649878,828198751,829429502,829433220,829438957,829442746,831495080,834958036,835891999,825962595,826775283,827158055,827158349,827158380,827160481,827160500,827160524,827161735,827842738,828090092,828090137,828090180,828090204,828090249,828090270,828090286,828877778,828924802,828947440,829090622,829119650,829492068,829564399,829735597,827590315,830520473,830709525,831027518,829746849,830176801,830176861,830176879,830955479,830955487,830955500,830955518,830955534,830955558,830955568,830955583,832844691,834343103,834343160,834343496,834343577,834752080,834796884,834796932,832366194,825034514,825260749,825339737,825340493,825964696,826364183,826660888,826661049,826661176,827902356,828145874,828217873,828498768,828498836,828498927,828498965,828499135,828610157,831156834,831969756,834750641,835073392,835440284,835462021,835462075,835873537,835883591,836212215,836485712,836529205,837085051,837085112,834148438,834355327,835767293,836238134,836256197,836298139,836298156,836429729,836454161,836454178,836600204,836636058,837212119,829788869,827791092,828200153,829126249,829594964,829759029,829772282,829878867,829934493,829934501,829988722,829989064,830018957,830018993,830061950,830177687,830260356,830937428,830990106,833272685,833341610,833344916,833893587,833926609,833938195,833938212,834023736,834229456,834229501,834229518,834229545,834229575,834239887,835836430,835854399,835905758,836418870,836418884,836639952,828914701,834946910,835737485,826568764,825518633,826383885,826983058,827146411,827702871,827776785,828173581,828698038,829372072,829606426,829606444,829606475,829851558,829851640,829852243,829852249,830125162,831050962,831074362,831099764,831102276,831428639,832043659,832916170,833199395,833869053,833869946,833973612,833975250,836056513,836056719,836056731,836056735,836056741,836056746,836056749,836056760,836056777,836056784,836056795,836056802,829283806,830786943,827552907,827552932,827552999,827553039,827553295,827553319,828707220,828961523,828963354,829009919,829800720,829939142,829982671,829982699,829982934,830011531,830754187,830772626,830773991,830955991,830988142,831027667,831103855,831103871,831110687,831110700,831110712,831110747,831110761,831141675,831878504,831878505,831878510,831878512,831878513,831878517,831878522,831878595,831878598,831878600,831878605,831878613,831878618,835930625,828461277,829066749,829066775,829066800,830131790,831227593,831227602,831227631,831227641,832382955,834248716,834960921,834974436,834974468,834974495,834974512,834592411,835422602,835422676,835422758,835422833,835422912,835423006,835427243,835427324,830613232,828371057,828371074,828371084,828371093,832746240,832808714,835663508,835858194,835860160,835860175,835860191,835860328,835861868,835861889,835862982,835862995,835863009,835872117,835872907,835872921,835872938,835873032,835999017,835999356,836100183,836100461,836100470,836101402,836106827,836106839,836106869,836107052,836227304,836230246,836230259,825655873,835341332,824640257,824675628,824684002,824716895,824720394,824820696,824825478,825694527,825710456,825734016,825735725,825749801,825760370,825792187,825792745,825823845,825827145,825840205,826002112,826002525,826172391,826177266,826347373,826593173,827184917,827869150,828393281,828397780,828704671,828823272,828829758,830341855,831011863,831925980,832802602,832804514,833367676,833378338,833881255,828439646,828558414,830843362,830913929,831541477,833238451,833238547,834946120,832225531,834435915,825106896,833615414,833664739,834183246,834184055,835421514,835428875,835429907,835436018,835994981,835996975,835998767,836000048,829105230,829105253,829105270,829154948,829154991,829155011,829155029,829155046,829157098,829157117,829157133,829157142,829176743,829266745,829266752,829266805,829266840,829266858,829266871,829266888,829266909,829266937,829266978,829267826,830344281,830344292,830344317,830344338,831293025,831998868,832098245,832147808,832360218,832373589,832373673,832375181,832377180,832467519,832467541,832467565,832467582,832467606,832467631,832831020,832833013,832857566,836315449,824543609,824637827,824639004,828013324,828035830,828134294,828647838,828653523,828822863,828822866,828822868,828868915,828873300,829048346,829343963,829345505,829855813,831683393,832246138,832247963,832250180,832250273,832256161,832275565,832276659,832290867,832736498,832739151,832739405,832747077,833360626,833362071,833363929,833397783,833403465,833483641,833483772,833483796,833483817,833483837,833483863,833483890,833483918,833483943,833483950,833484109,833484214,833599421,833599506,833599594,833599665,833599744,833599822,833599905,833599984,833600054,834039729,834039806,834041494,834548755,834549036,834556661,834556692,834565437,834568432,835296683,835297720,835461839,835490042,835492188,835503550,824625056,825378706,825557065,825558300,825558310,825558324,825558331,836443335,825444137,832370396,832375218,832377178,832391627,832406324,832406607,832411611,832418871,832436807,832439966,832448871,832454776,832482268,832483593,832485065,832526995,832607825,832616443,832785058,832835826,832852961,826762737,832428974,828643568,828643570,830238200,830294959,832463233,832963134,832963135,833131214,833291253,833297348,833874666,833885414,833913201,834358938,834995571,834995572,835336017,827271921,827631064,830438397,830438482,831005381,831761029,832311322,827012259,828029270,828029317,828029380,828029421,828029479,829687144,829953452,830255160,830752378,830800700,831971070,832589975,833251221,834825943,824835108,828516617,828685230,828868456,828868481,828984938,828984962,828985002,824954569,825429593,826213043,826227424,826262119,827151155,827165710,827743757,826198532,826198540,826204616,826250563,826398860,826398891,826398922,826399137,826399165,826399194,826399223,826399250,826399284,826399744,826399785,826399811,826400132,826400168,826400192,826400210,826400356,826400384,826400437,826400458,826401886,826401945,826402237,826637025,826962958,826963993,826964979,826975720,826985569,827066354,827266350,827266711,827269983,827270297,827276411,827277282,827282618,827290748,827311965,827355881,827362196,827365489,827365518,827367101,827367165,827367319,827422954,827423881,827432094,827433417,827433448,828019106,828068874,828069622,828571109,828571145,828571180,828571222,828571254,828571287,828571322,828571357,828571387,828571410,828572065,828572088,828572110,828572131,828572147,828572179,828572210,828572235,828572256,828572275,828572291,828572312,828572344,828572380,828572995,828573012,828573021,828573041,828573063,828573127,828573145,828573160,829660339,829660372,830984954,830984965,830984987,830985647,831662165,832029155,832041882,832166884,832166908,832166934,832166959,832166969,832166983,832167003,832167034,832167067,832167101,832167655,832167673,832167707,832167721,832167759,832167789,832167799,832167824,832167901,832167925,832167954,832167971,832168021,832385817,832626733,833122048,833161723,833169943,833579196,833579711,833580019,833580593,833581758,833581780,833581842,833581905,833581955,833581980,833582014,833582027,833582052,833585034,833585064,833585363,833585387,833585415,833585433,834995321,834995358,834995611,835012843,835012870,835012897,835016190,825469870,827252708,831077938,832672234,833347281,833351169,833716172,833732232,834249966,834253170,834725130,836813726,836837543,825152727,826241218,826696137,826729151,826959081,827055727,828445069,824473569,824476042,824626052,824633209,824634202,824641282,824650527,824653930,825577994,825578758,826001954,825691460,825693946,825694941,825698526,825701252,825746915,825746922,825747153,825747176,825747522,825747777,825747778,825748681,825749097,825749346,826288502,829612871,829613559,829650246,830145051,830145398,830146942,830157268,830300843,830302264,832200710,833228910,833339166,833339198,833339259,833339302,835760760,831071998,831115865,829996359,830149757,827168656,831172719,831172738,831172852,832858936,828555074,825690795,826102426,826102995,827515208,827515280,827776553,828210225,828393968,828843072,829908318,829908733,829908734,829908735,829908740,829908742,829908746,829908980,830006336,830374998,830375003,830375011,830375014,830375015,830375018,831656929,834427404,834427489,830887890,832830282,832830466,832835098,827967344,829179396,832618463,832620379,835774886,835776836,835777382,825036232,825852011,827115189,827138463,827149014,829654404,832961862,834558074,834558173,834558271,834558336,834558412,834558487,836128908,831166814,832061299,832625427,832953141,834474333,834550265,834783016,827862333,829039076,829311824,830796331,830796444,833498268,833499920,833500210,833505393,833506243,833507019,835659026,835672204,824761841,828285725,830947314,831448848,834221373,834221408,835208450,835278981,835284359,835325620,835329732,835471215,835474181,835475662,835900193,835907788,835911708,835912706,835914802,825299501,829068655,834816818,827108017,827874769,831875073,833578876,833586869,834167527,834171731,834886154,834889735,834896133,834900032,835577097,835704931,835705474,829644720,830113676,830536428,830738328,830771216,830971348,830977364,830979576,830979581,830979582,830983390,830993067,830993378,830993743,830993745,831003468,831003470,831040135,831040144,831043385,831532234,834255604,834684664,835188718,835258341,835550966,836060104,836408413,825645634,826728799,826738733,829194158,829194412,831500005,831573476,826167753,826715384,834989124,834989132,835259912,831280007,824969333,825148610,830436401,832892428,833307716,833797139,835884236,835884251,835884273,835884286,835884309,835884333,825001009,825586451,825750606,825860415,826003874,826041758,826062154,829746661,830575786,831083030,831083836,831128450,831232729,831252700,831258724,831262659,831264013,831275169,831296179,832119493,832138366,832142889,832164028,832170261,832176091,832200140,832440006,832535235,832565060,832566897,832586389,832598764,832638473,832641895,832687404,832760607,832762446,833027309,833053994,833182234,833226066,833231192,833234311,833243793,833332304,833336166,833520787,833526047,833536016,833542574,833557159,833560721,833618520,833665586,833719421,833745055,833751795,833758742,834044290,834046505,834048140,834052956,834206886,834240495,834423322,834423668,834886023,834912146,836010308,836635030,836662223,825094237,836013803,836014008,836016993,836020080,836021077,836022080,825978364,826131775,826132628,826133057,826133570,826936380,827225148,827232015,827247145,827251351,827263434,827511378,827512735,827553086,827558225,828019194,828291215,828294286,828295655,828309928,829463415,829499785,829599949,829603539,830010344,830020773,830033081,831458823,831459563,831466663,831466921,831469932,831487326,835190568,832526948,835414335,835417338,824572787,824632367,824633121,824645759,824658745,824688282,824688309,824688341,824714238,824716731,824716869,824717169,824737114,824737236,824737266,824737477,824750038,824834762,824866321,824866387,824866433,824875035,824891289,824902425,824902476,824902745,825043689,825044837,825085340,825155995,825156003,825156017,825201140,825201149,825201165,825201183,825218836,825218931,825218969,825219018,825219387,825222538,825240605,825268609,825280137,825280213,825280245,825287673,825287952,825291079,825300444,825300539,825481219,825818322,825818494,825818679,825818715,825818874,825818925,825818949,825821656,825821696,825821947,825822007,825822026,825822383,825822422,825822434,825822462,825824216,825870769,826155617,826155625,826155636,826155640,826156283,826173170,826199952,826215427,826343900,826343919,826343947,826343976,826344015,826369994,826370823,826380508,826412431,826412481,826425936,826425958,826425981,826687628,826701485,826831760,826899992,827236797,827236801,827236808,827236813,827236832,827236835,827240063,827518234,828004183,828617422,828688170,828726394,828752145,829077790,829077835,829098193,829104432,829295849,829726633,829793257,829838314,829838386,830026607,830026620,830026651,830026658,830026667,830027057,830027075,830027087,830027106,830027151,830027200,830122693,830124331,830155565,830162697,830162976,830292111,830292139,830832073,831315912,831326101,831425966,831429110,831432682,831434442,831584889,831594955,831807360,831862131,831864453,831864457,831869978,831981567,831993091,832000136,832001596,832030971,832162469,832489539,832524165,832626915,832626955,833157399,833278773,833328593,833418812,833426405,833439915,833440356,833448418,833469904,833469907,833469910,833479449,834006675,834022266,834036961,834046836,834161030,834209291,834610303,834610906,834612234,834891174,835583865,835583882,835583893,835583897,835743495,835805034,835805069,835805207,835817969,835830920,835831100,835833327,835838670,835869805,836085644,836085709,836120305,836268470,836278612,836427115,836565176,826259366,826260840,830261249,831684472,832127968,833129894,833215303,835167898,836008768,829596200,830100037,824508459,824529600,825057750,825139366,825623488,825638797,827780236,828291596,828298465,829462104,829687535,831446085,831447552,832477575,832935294,833315201,835170753,836295882,824612930,825068033,826454804,827224988,827224994,827412261,827412305,828697199,830151702,830192602,826769319,829403692,831069499,831448799,831471403,833699603,833795885,834624881,834625568,834662714,835186588,835186650,835187494,835188091,835628011,835636409,836045286,836045652,825077890,825853133,826076520,826076533,826543261,827725062,827725078,827725084,828505209,828574800,828744129,828877706,828924042,828940227,829219832,829327982,835460852,835461780,835462306,825082058,825082072,825082088,825082282,825082293,827087861,827604514,827668738,827993798,828204285,828222671,828418675,828484888,828486541,829780463,830270650,831263062,832523486,832549241,832978549,833033158,833414010,833741327,834931045,830641320,831711750,831740470,831778487,832348127,832390454,832593008,832593132,832593276,832593415,832593561,832593701,832750059,832774861,833109078,833289852,833807927,835136012,835376934,835540456,835545681,835823618,835823748,835823853,835824017,835824201,835824372,835824474,835842948,835843964,835845277,835892111,828726754,831528725,831528799,824488751,824570863,824571888,824603735,824604505,824653874,824670941,824718042,824747807,824803868,824806197,824842838,825234144,825266394,825271787,825321891,825324241,825325903,825327605,825330924,825435724,825450547,825460459,825460460,825477735,825606109,825848596,825895424,827635202,827648920,827659882,827671420,827683582,829813563,832148199,832150363,832150984,832154711,832168293,832183046,832191913,832203932,832225321,832228836,832281680,832281706,832285942,832290632,834234519,825556207,826091466,831067127,831131991,831519291,835117585,835137322,836165194,824734958,825021793,825069854,825285059,825401517,825545262,825546152,825604116,833717597,833718396,833733895,833819491,828945435,825895582,833211198,833211291,834147267,834980325,834983210,834983783,834990662,835384679,835385914,835397505,829421441,829421448,829421471,829422555,829422846,829422866,829422895,829423029,829423051,829423074,829423080,830401573,830469208,830469223,830469248,831989830,824652229,825580227,826056110,829000455,836279120,825241932,825660528,825741839,825772023,826170473,826644822,827116353,827226122,827230979,827234411,827240711,827773401,827773409,827790778,828597251,828808196,828917200,828917202,828917217,828962954,828962955,829082284,829484301,829937224,829937236,836284789,824548452,825076073,827713497,828891385,829216502,829320445,829623669,829923737,830146960,829169292,829794147,830172620,830341653,829816590,829947040,830150479,830150852,830152439,832717420,833551281,833574470,833771609,833771650,834122719,834270508,834270512,834270555,834328339,834328468,834328557,834328643,834328711,835523083,835523406,836380185,827527550,827736189,828009434,828369715,829345589,830874629,832575754,833010889,833050586,835405078,836197789,836703411,827202417,832214365,835937469,835944903,832920800,836310441,825533812,836006340,836008001,836008003,828053175,830595968,831159829,831996082,832008682,832014361,832087592,832626866,832667805,832729687,828129819,831451296,833859650,833891686,825747252,825585727,824746593,825548398,832235815,824624224,824713874,825650761,825742968,826270212,834011130,834011133,834011145,834221099,834221124,834221222,834221237,834221444,834607887,836537607,836890567,826511558,826636953,826798345,826810733,826922916,826924298,829280904,836697970,827334522,827337877,827338649,827340261,825784263,829613899,831360773,832386544,832391550,830096453,830539066,831060364,833779359,833779383,833779400,834419733,834419751,834421112,834421150,834421195,835360758,832091589,833942878,833942908,834788702,834791815,834791836,827223255,828110318,828404449,828870474,829073236,829677085,829678485,829775152,829983079,830733674,830854002,830854011,832419828,833306415,833306431,834104688,834788699,834788701,834797873,834987568,825170266,826951214,827062480,834826697,825009700,825012953,825388348,825398522,825523672,825541175,825543512,826301661,826316488,826556721,826621939,827643223,827680605,827698356,827704813,828484338,828663557,828668026,828670959,828718380,828728597,828728632,829033231,829108247,829128438,829185041,829250563,830654975,830655178,830710207,830715956,830727627,830806255,830838247,830853862,830855403,830999331,830999556,831000423,831001265,831106690,831168376,832287938,832509105,833418107,833680037,833702621,836830411,829882122,827064309,827067770,827069097,827069100,827077732,829422884,829918704,829918743,829918754,828201542,828221985,836549847,836549877,836549894,836549907,836580670,835706156,835728981,835730921,835749278,835749458,835749744,835750014,835750154,835750502,835750798,835750929,835751101,835751276,824810829,825585628,826211239,826211247,826211256,826211277,826211682,826471255,827162534,828109575,828449563,828982272,831045609,832029930,832093214,832240801,835410466,831136722,832734302,834176632,834989301,825319094,826904042,827280826,828151669,829311965,833164594,833164761,834450614,834854554,824655779,827644259,828680851,828749956,828751607,828755037,828771966,828772155,828774875,828781881,828797283,829910712,830390201,830390204,830789192,830789193,830792432,830913867,830922860,830926929,830928047,831015837,831068371,831068501,831083488,831096380,831117942,831118458,831119727,831149967,831154712,831159554,831204250,831209847,831210210,831229057,831237070,831237226,831248707,831264026,831264061,831354661,831446191,831450595,831685656,831728480,831796256,832096558,832178142,832229709,832233993,832261338,832281896,832282173,832284032,832555539,832568725,833050356,833053182,833275686,833275936,833301699,833303348,833386492,833386888,833386908,833387118,833387214,833387402,833387445,833387459,833387496,833387531,833388494,833388512,833397265,833480949,833482986,833490588,833492259,833499954,833502682,833582252,833582276,833583454,833974147,835020577,835020891,835032211,835057357,835543031,835543486,835551255,835578688,835578830,835578914,835578925,835897275,835916524,835917409,835917929,835922223,835928526,835928657,835928681,835928694,835928717,835928736,835928760,835928783,835928813,835928829,835928841,835928921,835928924,835928926,835928939,835928949,835928955,835928961,835928966,835928974,835929237,835929321,835929323,835929329,835929333,835929343,835929352,835929356,835929362,835929433,835929440,835929493,835929556,835929559,835929563,835929568,835929737,835929767,835929925,835929930,835929935,835929942,835929946,835929951,835929955,835929962,835929967,835929975,835929985,835930000,835930015,835930193,835930245,835930249,835930255,835930258,835930265,835930269,835930276,835930279,835930614,835930820,835930834,835930837,835930840,835930849,835930886,835931848,835932067,835932075,835932115,835932167,835932172,835932179,835932188,835932197,835932207,835932231,835932255,835932263,835932331,835932561,835932568,835932578,835932580,835932586,835932591,835932601,835932608,835932613,835932617,835932626,835932635,835932649,835932653,835932664,835932672,835932676,835932680,835932694,835932709,835932711,835932831,835932892,835932898,835932906,835933391,835933395,835933398,835933410,835933413,835933421,835933426,835933430,835933435,836016313,836051480,836323280,836352888,836352938,836396244,836397882,825295719,826609383,826674582,826674588,826674594,826693094,826899894,827026460,827570567,827789009,827926902,831943595,833809022,834169406,834544227,834544256,834544282,834544297,834544322,834583460,835119012,835314659,835314750,835466704,835584591,835880541,835937229,835937372,835937417,835937443,835937672,835985927,835986018,836257588,836506768,836506780,836594112,836596958,836596994,825001513,825023824,832318265,833272676,834586938,828102987,828266784,828266804,828266806,828266808,828266814,833243003,826712386,827429227,828852591,831202959,824658255,824733278,825049735,825304608,825304622,826707919,827299992,827803831,826305486,826347536,828511418,832024813,832099597,832110248,832603520,832631145,832641096,825643529,834633153,834633204,834633225,834633249,834633281,834633308,835223884,835316289,825093439,825796854,826467448,826477129,827373029,827373739,827376224,827376605,827376670,827376730,827376934,827376969,827581466,829654494,829654545,829654615,829654665,829655189,827384608,827899929,832357579,834701150,835703822,829161326,833107168,825909664,836986933,835217840,836028969,836028975,836028979,828095678,828095702,829011613,832185272,828569647,825102903,825693793,826504290,826507150,826508029,826509764,826509772,826509773,827837750,828181431,829481086,829654944,829987880,830158919,830297377,829426064,829533562,829533564,829533565,829628211,829643617,830105697,830175893,831080432,831210246,831227663,831229087,831251157,831263987,831346715,831348646,831454236,831455098,831455155,831462634,831469009,831469150,831522451,832047597,832047633,832380826,832381569,833834553,833853676,833873382,833874653,834068227,835205410,835207807,835214328,835283498,835295377,827489542,827610037,828500695,828693845,829573178,829828302,832613123,832613143,833510713,826517743,826518270,826518604,826518669,826518704,826518755,826518798,826518828,826871627,826873553,828234337,828939722,829472550,829472924,833551995,833553502,833598234,833682497,833689495,833690860,833692710,834137828,835377075,835441492,835967775,828844704,830522083,830522101,830522143,830522154,830522175,830522195,825628261,827253136,835749962,827513648,824812431,825072932,825282780,826029841,826029982,826033454,826033455,826424634,826424885,826425347,827042902,827043342,827270354,828089500,829724554,830208393,830549386,830557245,830558783,833142278,833744994,833820718,828951127,830334998,825158917,825179514,825226343,825420610,825434514,825548256,825554195,825565973,827110912,827147109,828205117,828649023,828303441,834055614,828740243,829993817,829993828,829993843,829993857,829993941,829993955,829993979,829993997,829994011,829994032,829994049,829994058,829994072,829994086,829994102,829994118,829994143,829994159,836276679,825767872,825819093,826943097,828319243,828324339,828355222,828401413,828511996,828519275,828774181,828849604,828857858,828876043,829044954,829045022,829136188,829351943,829493427,829508861,829508955,829670909,829690181,829700063,829751395,830117416,830117419,830117609,830132937,830149133,830186206,830261273,830671587,831235052,832761006,832761023,833100056,833111474,835861555,828579053,826299861,827037837,829367012,829370864,829370878,835885494,835896906,835930437,835932809,828695844,830890054,835610661,826684399,827488662,830090456,833267308,826646166,826646718,827210056,827246663,827761803,827763101,827765053,827765054,827765270,827772673,828283442,828793616,828796809,828801761,830384708,830386968,830798855,830845304,830873945,830901723,831888160,831890119,826322568,827426566,828636361,826463820,826549630,828050132,832080080,832085583,834443888,834457799,837002736,828598007,831681716,831835356,831838157,832272712,832463662,834613764,835429986,835836214,835836251,827896623,828224252,832866333,832866550,825540552,825542255,826868146,827207691,827220302,827222403,827225222,827225223,827225226,827225231,827518591,827764193,828223659,828807777,828807780,828808580,828808584,828808679,828808686,829110373,829149883,829902919,829903066,829903068,829903072,829991696,830021909,830030520,834366181,834495025,834556756,834556763,834605071,834609413,834609422,834956618,835164254,835164293,835164326,835164544,835164757,835301987,835343563,835462457,835609459,835612759,835750416,835822104,835927896,835960824,835965968,835965978,835966414,835976411,836021923,836033030,836033052,836033069,836034777,836034802,836034857,836034870,836034991,836035005,836035028,836035098,836636173,836636222,830298243,830317235,830317885,831281764,832783829,833076173,833084375,833170883,833815208,833824788,835070988,835071020,835852255,835861624,835873867,835880303,835881787,835514080,835561971,835738068,835750253,829305951,828937674,831651552,826523530,832098239,832390740,834154952,834316161,834624592,834625241,834630119,834806354,834807889,835411248,835444505,835485934,827113529,827238587,832453233,835132296,835212113,835484797,835491489,835491493,836080270,836080471,836080580,836080720,830246515,833445209,833530435,830350973,824852051,824881957,825799420,825810668,825880413,826049954,826312498,826339654,828079282,828129647,828134489,828134617,828410707,828626202,828628407,828662153,828740044,828740141,828740818,828740948,828741287,828741311,828741317,828741841,828741851,828742536,828742563,828746057,828746073,828746113,828746129,828768145,828768524,828768754,828768939,828971653,828971724,828971772,829189204,829875151,829875193,829875208,829875323,829923300,830649221,830649240,831101767,831278123,831463754,831868829,835310058,835310078,835310101,835311781,835311964,835311976,835311986,835312005,835312034,835312061,835312077,835312099,835312141,835312302,835312319,835312408,835312424,835314420,835314438,835314446,835314458,835314474,835314902,835314921,836339883,828219509,828219531,828219548,830258353,830359621,830374673,830827326,832640273,832647711,832647712,832653554,832654145,832655251,832655271,832656694,832809641,832810321,832817122,832869118,834581268,835154759,835161686,835162238,828178351,828967389,829652745,832786506,833621426,835147387,835158712,835218473,826258966,826295885,826304899,826526042,827042173,827349271,827526120,827641871,828062308,828516696,829050164,829440123,831683389,832848233,834452865,834713655,834717122,834717123,834717429,834721034,834724613,834731112,834735912,834737713,834738210,834740442,834741961,834746909,834751414,834756924,834757833,834760487,834765673,834771734,834776349,834781289,834786742,834790008,834796973,834798090,834799684,834809267,834816193,834816367,834816598,834819216,834821969,834824792,834825174,834825492,834825685,834828271,834829523,834831496,834836608,834989403,835025393,835029176,835189734,835190078,835190230,835190919,835191118,835240149,835241519,835241772,835242898,835280474,835280701,835306396,835306419,835772372,835856909,835857137,835857167,835857186,836554242,828383790,829654855,829964306,830173843,830180224,830224911,830338840,830365405,830365429,830365566,830376141,830376148,830376153,830376162,830376182,830376203,830896125,830896184,830998977,831814609,831819694,831826396,831831255,834899032,834899072,834902134,828603385,833703120,835005287,835005829,835006627,835006834,835007014,827918246,828411027,828429749,828903466,830655057,830832671,830867846,832713701,832771538,832823848,833261169,833299887,833299914,834730036,835470129,836026463,836026471,836026771,836027391,836027890,836028056,836028111,836028231,836028659,836029249,836029644,836029866,836032228,836033224,836094503,836094896,836095582,836095935,836096349,836098239,836101208,836101565,836101581,836101586,836101694,836358275,825301032,825585595,826144652,826444652,826444764,826444804,826444822,826444854,826520248,826520487,826582413,826705908,826705916,826705952,826705959,826706166,826706181,826710534,826713108,826713309,826713839,827029341,827029360,827685816,827685857,827685987,827686077,827686152,827978166,827978183,827978212,827978231,827978258,827978287,827978619,827987827,827987856,828214612,828214619,828214627,828214639,828214653,828214668,828214687,828214697,828214721,828214743,828214754,828214765,828214780,828214795,828214841,828214871,828215949,828215963,828215997,828216006,828216014,828216023,828216037,828216052,828216068,828394024,828500671,827338746,827534538,827796188,829882190,830009517,830019698,827419825,827531392,827534832,827608153,828687875,830318552,830548960,831542836,835039027,835910651,826732744,826836225,826947434,827095715,830177219,831660122,831667499,832276098,834503821,836270320,836842864,827094083,827094087,827094508,827176754,827280669,827283674,833092794,828599555,828698492,828994131,829020013,829199593,829339384,826449373,826526836,833872234,835686756,835897923,835905713,835914681,835919612,835920566,835924886,835926567,835962433,835971488,835975305,826607021,827783744,827783756,827783871,827784549,829765109,830552475,830579021,826765768,826768928,829561815,829597998,829598351,829599117,829857024,830191100,830206029,830207008,830246616,830246617,830246618,830481739,830541932,831228338,831691629,831693978,831694385,831694933,831847921,831985646,833430603,833954504,833954787,833954788,833955282,833991931,834001115,834001122,834014202,834533718,834535332,834535564,834561535,834914560,835118085,835118116,835136255,835297368,835305194,835589067,835596223,835599850,836016057,836017254,836017437,836908135,828031999,830894093,831203347,831204830,831278381,831303216,831514437,831548490,831548522,831548542,831897141,832551900,833858155,834040287,834811688,835331521,835332480,835367997,835487860,835503795,835506789,835507670,835508662,825806767,827296072,827558146,828813246,829052562,833477031,834165853,834201090,834265808,834619171,834735017,834803742,835500401,835628222,835641661,835645300,831975162,832096958,835027150,835039556,835257053,835282013,835282825,835285966,835320554,835320948,835323585,835331066,835350818,835362106,835369010,835372595,835394816,835496270,835541952,835546619,835561828,835562491,835568771,835575588,835576601,835577011,835578288,835795600,835805758,835834677,836268584,837275137,837275193,837275248,837275249,837275331,837275398,837275419,829717399,830733405,831067037,831230910,831230963,831622244,832113141,825024023,827482047,836208481,829918487,832333842,832333844,832333851,832333855,833569644,833678224,833858618,833893425,833893470,833893522,834909711,834910355,834910372,834910403,834910427,834910461,834910495,834910523,834911520,834911554,834911577,834911631,834911916,834911945,834911961,834911987,834912016,834912247,835858137,827964687,830429257,832309947,832667115,832605818,831377881,832635431,833688343,833782699,833797409,834153804,834409730,835462419,835884348,831458531,827903425,828242121,830946998,830950892,831109218,832168879,832528620,834652385,834864039,834996530,835273862,832128616,832162937,825138370,826832828,827602844,827720073,828940032,829525394,829727409,829890669,829903175,829903183,829976279,830867020,830867028,830867029,830867033,831014512,831014520,831014527,830699728,831643868,833624654,834993561,831022660,832725494,835585549,835699982,836903582,831206428,832062620,832323456,834894204,827344020,828715147,828723819,828888352,828904893,829387697,829391599,829392566,829393515,829514734,832641276,828154605,828947086,835103654,835107676,835204856,835637858,835647628,835659148,835661329,824966430,827311788,827620712,827623175,828360775,828360780,828655668,831024844,831142988,831767987,833818044,834590870,835113116,835397191,835849162,835855103,827699113,827705660,828085387,828090420,828115659,828195040,829292409,829530381,829530834,829532475,829609763,829613874,829637248,829890597,829340062,831233210,835337322,831587026,831590670,834888343,834888353,836133522,836348203,836348220,836348254,827967443,828560819,828858050,828862397,829357580,829426117,829606413,829609182,829825625,829843821,829988331,830829482,831317837,831359686,831400821,831558217,829423335,830423941,830517545,830650235,830826136,831809765,833687482,833915621,828410700,830517234,830573706,830574380,831411232,831520370,829585468,832091727,833152721,833349016,833353474,833435699,833857417,833918721,833922935,834957965,835026234,835419023,836391154,836989404,836989409,828939215,828939357,828939603,831867193,832646448,833039435,834973808,834975869,835447966,835449823,837269583,833409110,827336683,828200816,828470996,827197222,827197281,827210060,828290608,828290879,828329824,828334379,830611414,833448054,834908786,832640114,832675098,832675126,829543740,829543904,829544033,829544526,829843517,830180102,830229345,830243362,830783821,830803545,830811201,830872869,831362294,831383822,831413650,831493314,831497930,831540463,831553450,831575163,831583733,831628351,831760699,831760829,831774493,831817462,831823362,831833188,831848139,831890635,832104187,832155107,832174609,832465453,832467177,832714391,832722958,832737443,832737986,832738658,832744243,832819386,832821356,832829735,833182038,833212246,833272668,833359459,833365485,833379641,833391969,833407428,833421491,833430176,833432973,833433109,833447100,833447876,833588289,833713883,833735509,833769224,833777466,833864930,833883035,833921989,833922213,833953938,834089810,834392414,835264360,835265441,835302887,835305074,835451672,835460620,835543453,835560868,835560870,835976064,835789326,826162279,831408508,832011284,832100101,832348973,832528097,832530983,835104119,835699606,835700942,835705434,835708056,835757481,835770343,835781578,835863446,835870520,835895130,835942451,835946275,835950428,835953912,834309668,834310030,834312626,834476039,834768600,834844326,834897595,835122200,829056140,830151864,830151879,830960019,831187518,831193444,834201802,834262579,834262601,834262642,834635227,834679850,834011817,836515604,827756182,829751984,831050379,832250852,832588805,832820633,832192893,834453087,834453210,834453283,834453389,834453518,834453633,834454926,834455004,834458670,834480183,835853255,828752632,828918305,832788220,833649022,835174708,835386720,829311873,829997219,830857795,830944535,831037217,831159903,831812429,831813783,831813803,831813830,826292932,826492266,828187728,831798401,831799249,832805478,832846007,832848225,833259516,833604808,833604874,834005124,834005727,834017401,834017797,834020072,834229396,834475419,834584867,834842779,834844773,834845889,834941303,835171108,835171191,835171198,835287377,835388508,835464193,835464293,835464751,835464956,835468009,835481367,835516720,835516722,835518217,835526873,835530439,835531378,835540058,835540063,835551790,835772398,835772634,835772639,835772655,835772667,835772691,835772706,835772725,835772734,835772746,835778430,835779393,835779394,835784424,835784491,835785137,835789536,835794838,835818768,835818900,835839919,835839943,835839953,835839970,835839996,835840011,835840020,835840042,835840051,835840066,835840084,835853542,835853570,835853588,835853618,835853676,835861811,835861910,835882214,835587965,835606916,835622961,835622966,835622970,835622987,835626090,835626104,835626117,835626127,835626140,835626272,835626284,835626348,835626396,835628199,835628224,835628233,835628247,835628276,835628294,835628309,835628319,835629784,835632765,835634878,835636440,835829321,835701206,836313782,836638077,829880177,829880181,826608582,829757595,830159496,830800413,830800430,830800447,830800469,830800488,830801428,830801441,830802313,830805305,830954109,831048440,831048460,831049578,831049587,831049625,831050206,831071093,831758343,831849544,831849556,831850451,831850471,831850501,832045200,832368021,832368570,834665535,835254720,835687404,836092793,836094948,836095801,836098104,829719985,829935615,831169236,831528460,831684566,831684581,832267915,832813226,833815831,835812585,832445459,834011340,835418274,831758173,835452526,835454325,835456033,835457873,835460208,835462642,835464564,835466514,835470092,835471512,835490161,835491754,835493051,835494794,835496671,835506448,835507958,835510346,835513647,835517123,835519217,835520268,835541161,835541833,835542752,835543764,835544651,835546487,835547264,835547353,835547415,835547977,835549804,835551102,835551820,835553138,835554871,835555568,835555613,833553991,835145083,835793653,829179142,829235942,830825956,830840932,830841592,832192714,833342744,833846764,833879243,833883688,834503458,829733776,830897252,831092016,831105536,831197949,832560886,836034304,830701912,830707441,831164784,832542872,832633179,831588226,832772293,830331198,831742714,832105392,833254169,834070156,834390640,834539422,837017488,836429131,836430660,832818344,833434315,835293122,835294553,835989147,835991156,835992977,835995155,835997679,835997898,835998107,835999242,835999601,836000660,836003010,836003627,830565478,830572740,830572872,830574575,830583963,830587529,833495735,833496991,831084017,831556970,832816185,836864213,832829782,832569714,826195355,831853268,834781081,835233706,835504443,835668869,835708047,835712158,835745731,835745739,835745743,835745752,835745769,835746119,835746137,835746147,835746597,835746611,835746617,836067698,836067770,833972351,833972396,833972432,833972466,833972512,833415787,836539633,829901279,834610216,834610330,830960776,832214258,832214276,832628879,834821659,835180905,835425202,835506848,835516727,835521945,835606186,835606398,835982308,836013969,836444373,832025739,832085650,835304452,835836102,835885149,835915933,835925700,835954031,836003409,830582793,835224433,835322712,836169041,836515055,836515102,836515141,836515151,836515154,836515161,836515194,836515197,836515201,836515210,826280760,833768494,833979960,830693454,830709615,830713659,830798610,830832964,831155760,831283982,831936438,831936846,832004058,832111519,832111839,832111868,832112017,832112157,832112268,832112309,832112377,832112414,832112444,832112469,832112508,833390176,833390196,835280214,835888635,835892057,831950082,831976858,834379383,835488303,835550497,835820147,835845997,835846140,835846252,835846378,835846513,835846628,835846726,835846857,835846996,835847129,830092227,832290787,832312254,832313020,832544207,832877224,832882356,833451116,833872775,833891733,833894173,833917947,833924255,833954769,833971533,833989595,833999055,834024222,834221284,834428223,835278664,832763292,833310212,834170711,834193291,834427286,834665200,834690524,834996153,833971808,828376218,834538402,834987001,834987034,834987336,834987381,834987444,830790569,830790659,830790660,830802295,830802358,830834658,830834659,830846039,831161183,831163957,831410497,831537695,831837248,831837585,832754834,832762167,832764406,832786388,832791112,832791124,832791140,834371856,834372435,834372633,834456396,834514999,835215185,835351006,829274153,829274157,830955484,831177352,831883374,831883375,831043956,834848255,835901407,836575478,836575485,831841202,832001729,830388596,830918187,830918224,832465691,833866079,835412978,836935203,837115200,837130222,830988582,833031618,833505842,833505893,834146269,834656119,834664098,830931106,831824160,831824899,833875327,833876783,833943793,834111587,834145961,831377668,835389320,835411619,830152804,830312698,834102884,835197579,835647979,835655146,835765544,835863629,835864022,833276138,833421010,833421026,833421039,833421062,833421089,833421119,833735088,834260656,834301616,834431797,834447664,834447853,834447896,834494930,835002552,835422682,835741750,836128819,836160083,836921361,837108703,832282777,832284539,832530208,832996816,833372014,835442666,835441282,835444060,835444688,835448736,835450454,835454150,835455502,835461824,835468165,835471371,832785526,833030302,831066717,831067796,831072632,831099599,832329003,832330127,833139373,833139378,834625966,832966014,832968634,833558118,834719868,832588762,833360181,836433891,832655976,835759459,835794957,836276616,836276648,836276699,836276727,836284053,836290793,836319091,836440023,836700529,835162499,835162872,835363154,835458967,836479492,836481692,829691210,833070349,833072658,833088369,831594029,831786653,832463743,832497725,832504863,832619994,832730776,832730854,832730938,834930243,835308761,835643899,835718758,835751288,832845801,833073549,833098033,833337264,833349955,834451466,834591386,834938659,835004909,835364907,835529300,835529632,835529742,835529830,835929159,835929253,835929314,835929353,835929814,832421596,835718214,825942364,832125047,833004932,835148642,835784969,836001676,836183668,836403385,836410489,836454533,836455860,836466773,831561983,833626802,834237797,835297830,835343491,835428061,835428093,835428163,835428171,835349538,832455513,833180714,833304912,833509438,833884661,833937543,837018409,831632234,833497280,833550311,833717725,834165922,834193207,834288534,827346015,834393614,834671627,832037691,832039631,832112018,832630245,833265880,834202266,830028246,832339838,833794987,834399564,834399634,834481835,834485126,834510273,834510342,834510370,834510393,834510422,834510461,834931529,834963371,835107677,835128275,835128280,835128933,835191176,835195220,835263111,835397980,835636046,835648209,835650633,833184077,831992906,832967192,833120697,833553499,833712580,825545964,834224029,832395518,833399322,833399323,833430800,833617672,833660160,834120574,834405270,832098065,832329401,832450882,832463737,832464408,832464433,832772783,832915496,832968648,833239411,833306420,833495736,833495739,833498427,833539115,833609711,833628773,833634749,833635567,833641588,834077610,834078568,834078617,834078636,834105517,834106833,834106939,834106984,835474626,835677310,835691546,835691555,835691577,835691589,835695344,835716306,835722470,833214545,833247348,833247378,833247602,833498396,833499433,833499444,833609722,833630192,833630211,833630224,834072725,834072739,834072778,834072794,834072821,834072855,834074808,834078580,834078620,834078630,834078632,834199003,835001530,835396069,835979973,836443908,836649830,836649849,836649861,836649869,836649878,832620348,836380974,836384225,836416803,836923986,837182000,833301309,833576666,836413196,836413210,836429668,836766137,831412461,831786295,832281493,832983474,832989607,833001333,833209927,833231702,833252978,833270026,833271758,833275341,833275935,833277692,833278704,833285051,833285937,833286064,833290171,833294628,833295685,833300448,833300469,833300493,833300516,833300548,833300588,833415253,833436987,833438997,835283026,835489344,835510695,835531329,835540467,835560467,835584696,835752651,835890550,836013366,836027731,834410354,834416325,834842177,834875672,835250506,835365107,835432648,835554678,835686746,835906515,835928697,835940254,836103231,836109297,836109298,836110007,836112019,832640377,833180722,833249082,835255695,835545743,835545779,834138120,835391603,829468788,831337625,833254380,832759677,833314641,833333053,833333078,833333269,833635609,835053300,835440537,835873936,835876819,836695727,835693157,835693208,835735282,835735288,835735294,835735298,836058769,837083040,837083442,837099602,834363172,834173332,833328859,833521687,833528331,833539606,833897537,834509055,834509064,834951003,834951228,835028969,835039275,831210373,832868922,830910829,832515893,832626902,832958022,833077835,833174371,833180407,833216004,833224436,833224949,833225105,833231411,833248770,833248792,833267990,833370099,833370112,833370129,833382515,833409755,833410107,833410274,833415909,833415928,833420969,833425362,833426597,833431901,833434570,833436525,833438074,833618402,833618424,833618470,833618760,833620429,833620587,833624051,833624063,833624082,833630777,833630823,833630843,833630883,833630898,833632111,833661860,833663193,833667480,833679496,833695981,833696020,833711356,833718034,833747616,833748391,833748438,833748475,833752466,833765730,833776052,833777317,833791483,834138292,834145964,834148367,834171050,834184791,834184831,834185328,834185350,834185366,834185387,834185595,834185653,834185675,834185709,834186026,834187879,834226726,834235688,834253721,834259313,834303763,834306183,834306186,834321646,834322701,834466066,834466089,834466101,834466113,834466140,834466884,834466907,834466921,834493324,834493916,834494224,834520044,834537254,834539441,834541364,834551744,834553013,834575881,834575893,834577246,834577255,834577285,834577297,834577772,834577798,834588155,834590848,834590851,834593677,834601832,834729070,835639967,835639983,835642933,835643166,835643177,835643194,835643206,835643394,835643903,835643913,835643918,835644509,835644721,835644731,835644753,835644761,835644772,835644778,835644788,835644799,835645364,835645373,835645383,835645542,835645558,835645631,835645638,835645643,835645649,835645660,835645666,835645674,835645676,835645690,835645694,835645703,835645712,835646467,834038510,834347795,834604447,834604845,836225412,836279114,836307285,836492435,828298529,833337758,833872881,834178705,834816119,833550528,833554964,833558172,833784916,833795231,834941420,834944832,834952719,834955301,834959575,834963054,835022643,835023135,835085466,835698148,835698993,835699315,835699527,835700285,835700336,835701251,835703323,835707683,835721891,835722595,835727622,835870713,835870714,835870949,835872374,835875930,835882643,835886380,835886633,835889977,835906566,835909843,835915615,835919274,835919281,835920874,835923846,835928690,835933953,835935922,835937823,835941797,835955104,835955174,835957448,835960862,835961100,835961206,835963127,835970203,835974539,833778187,833949047,834437110,835810063,833832896,834555657,826149084,829633344,833212431,833212911,833213423,833238578,833238928,833239044,833239076,833239730,834161045,834270103,834309946,834309958,834310007,834310022,834310059,834310079,834310093,834310109,834354465,834637513,834637533,834637574,834637584,834649427,834686770,834849916,835378368,835378424,835378448,835378455,835209144,835683998,835685585,835867297,835871768,836084295,836090433,833021058,834700554,834700595,835939764,835939817,835939857,835562080,835564585,835567495,833403179,833409523,833781570,833802069,834105092,834143950,834144120,834145938,834166201,834231860,834236634,834928230,833422782,833283728,833865844,834930891,835392117,835394920,835394945,835394973,835429480,833124857,833359410,833601324,833641504,833774503,833778053,833780237,834061782,833707593,833875897,834428840,834433658,834433963,835179202,835179219,835179228,835290948,835731481,835929948,825972776,830397557,835169577,825608992,834011919,834042606,834054616,834070992,834320546,836256793,836259870,834715170,835991353,833434830,833498406,833626480,833626483,833630182,833630240,833630262,833634730,833634747,833646717,835722504,836247963,834066755,834066983,834937762,834964582,835034025,835036746,835040703,835042581,835078785,835079664,835080520,835082200,835082607,835083621,835084471,835085046,835104426,835106545,835107739,835113851,835115606,835117099,835118937,835121640,835123224,835289875,835291372,835395904,835698654,835699427,835704491,835707988,835714866,835722111,835724630,835728487,835730393,835730741,835739615,835744589,835750753,835873618,835873648,835874599,835884261,835884509,835884510,835884511,835886287,835889760,835889855,835927349,835928818,835933962,835934068,835937728,835937730,835938365,835938537,835940803,835941812,835958202,835958725,835959784,835961059,835961447,835971209,835972667,835975043,835976470,834601973,835151159,835789545,834715151,835698750,835698894,835700660,835743310,835744106,835750255,835830667,835830668,835897042,832244432,832249844,837046571,833840315,833843818,833854753,833879757,835525065,835525495,833527329,833642240,833678971,834648737,835178298,835179170,835618395,836695618,836698195,833658840,833689628,833767035,833767379,833768297,833768321,833769375,833769414,833773901,833773964,833776216,833783335,833789518,833789546,833811169,833811274,833811751,833819180,833819278,836342774,833768430,834109358,834161748,835111024,835111876,835112166,835119599,835120732,835525109,835525171,835534172,835552550,835553043,835757149,835757262,835758629,835758734,835764557,835766454,835768715,835772349,835774414,835778357,835781162,835785060,835786802,835798280,835809395,835819973,836800483,836803358,836803372,834594127,835547125,835895166,835967109,833454454,834537460,834904462,834914145,835116010,835136122,835609994,835611840,835943127,835974578,834177090,834198044,834541404,834821915,834846419,834877225,835884157,835115990,835631961,835887381,836114966,836512522,836624228,835803527,835914210,834055257,834078692,835895976,835900463,835911864,835915716,835920153,835935632,835945339,835945927,835945998,835946060,835946138,835946253,835946311,835946776,832895728,832897112,832897255,832897513,832906887,834922416,835384059,835385630,835405737,834532286,833313857,833319201,833322872,835213819,835241970,835242452,835242766,835244597,835446556,835472640,835472749,835474235,835474625,835487606,835489410,835497125,835536110,832463639,832490696,832502690,834287179,834504800,834524628,834525391,834537788,834569314,835289019,835299529,835299810,835299812,835299813,835303321,835308336,835316481,835644252,835644703,835646890,835647093,835647925,835649191,835649759,835652276,835702331,835703115,836110985,836111021,836114637,830497953,831813973,833259004,833460031,833671909,835704126,835706641,836084557,836098479,836112953,836113156,836114742,836184510,836184636,834269582,834426895,835913058,836059793,833390184,834601262,835034663,836705935,834482477,834743140,834743171,830465256,835117747,835117870,835118339,835122053,835149252,835154192,835159533,835160184,835248375,835249403,835688214,835758928,835855958,835998568,835468976,835515659,834795223,834795224,835275173,835435327,835710370,834829115,834844845,835754360,835754757,834861812,834867344,835050487,835106787,835157087,835160982,835165550,835170569,835557544,835561322,835575201,835598474,836000854,836018901,836021031,836024896,835958355,835959966,835035312,835021947,835045553,835075261,835116173,835160510,835562116,836022963,836023150,836023159,836024346,836024796,836042177,827346831,831607695,833255964,833264486,833501995,834367378,834829129,834829130,834829131,834829349,835472082,835472083,835472084,835472298,836015350,836015351,836015352,836015768,836410442,836430777,836430778,836431982,835265914,835295716,835296226,835393563,835394554,835414013,835489374,835496622,835547495,835197116,833179989,834575925,836521848,826031588,833433588,835470768,836111876,836125346,836125495,836248835,836305535,836318939,836419957,836435626,836570765,836604574,836659046,832954218,834700497,834700627,827711317,833490686,833701106,834084649,834795097,834908708,834962707,834983687,835213095,836564344,829885585,834279513,836452115,835313828,835314396,835322285,835518668,835528101,835538063,835553535,835577904,835578061,835873590,835886583,835888017,835894265,835897755,835910180,835922758,835928920,835929552,836053249,836053795,836053984,836056285,831186803,836117087,835974961,836218877,836266956,836727661,837141438,837298964,826670563,835077314,835140535,835524985,835536996,835708785,835721527,835725451,835918488,835667340,835667362,835667638,835667814,835672692,835672817,835729148,835743593,835747896,835750451,835763509,835763556,835763706,835776482,835792692,835793204,835794965,835869735,835903292,835905047,836054786,836057184,836057212,836059632,836070694,836089363,836143732,836149718,836153810,836161387,836161823,836171090,836172484,836173264,836178503,836178683,836184697,836184726,836418768,836423509,836443393,836444005,836447355,836461286,836474387,836510411,836512086,836512671,836518131,836524866,835729170,835639669,835643562,835645403,835645717,835646198,835646776,835647230,835648991,835649165,835922134,835924175,835854799,835856204,835906181,835914310,835918004,836293107,834710189,832458179,834016637,835997911,836173635,836601532,836097528,836274565,836783201,836805580,836188593,836188600,836188608,836188619,836263655,835152411,836439485,836609649,836609853,825733663,835991341,834945153,835955620,837003689,825423796,825905135,826019742,827153969,827164156,827213220,835824350,836823693,836875794,833801474,836705592,834090153,836723769,830220217,837096061,837110860,836187794,835056871,836420436,836421171,836422894,836423338,836561518,836572143,836572174,836572197,836778967,836782471,836847437,836874054,828516495,835272816,836149775,837064627,837064628,837137125,835756656,836381841,835098399,836521435,836840282,836868421,832519719,834252283,834959419,837118443,837202324,837220659
};*/
#define LIST_SIZE 5
unsigned int ownerguid[LIST_SIZE] = {
5,5,5,6,6
};
unsigned int itemguid[LIST_SIZE]  = {
103,105,109,76,78
};

ItemPrototype const* ChatHandler::GetItemProtoByGuid(unsigned int guid)
{
    QueryResult *result;

   result = CharacterDatabase.PQuery("SELECT data FROM item_instance WHERE guid = '%u'", guid);

    if(!result)
    {
        delete result;
        return NULL;
    }

    Field *fields = result->Fetch();
    delete result;

    Tokens tokens = StrSplit(fields[0].GetString(), " ");

    if(tokens.size() < 4)
    {
        PSendSysMessage("ChatHandler::GetItemProtoByGuid : Lolwtf. Data size incorrect");
        return NULL;
    }

    unsigned item_entry = atoi(tokens[4].c_str());

    return objmgr.GetItemPrototype(item_entry);
}

bool ChatHandler::HandleListOrphanedFor(const char* args) {
    if (!args || !*args)
        return false;

    char* cplayerguid = strtok((char*)args, " ");
    if (!cplayerguid)
        return false;

    unsigned int playerguid = atoi(cplayerguid);
    if (playerguid)
        return false;

    unsigned startindex = 0;

    for(int i = 0; i < LIST_SIZE; i++)
        if(ownerguid[i] == playerguid)
        {
            startindex = i;
            break;
        }

    if (!startindex)
    {
        PSendSysMessage("Personnage introuvable.");
        return true;
    }

    PSendSysMessage("Liste des objets 'orphelins' pour le joueur %u",ownerguid[startindex]);

    for (int i = 0; i < LIST_SIZE && ownerguid[i] == playerguid; i++)
    {
        if (ItemPrototype const* proto = GetItemProtoByGuid(itemguid[i])) 
            PSendSysMessage("guid : %u, template : %u (%s) x %u",itemguid[i], proto->ItemId, proto->Name1);
         else
            PSendSysMessage("Objet trouvable (guid : %u)", itemguid[i]);
    }

    return true;
}
bool ChatHandler::HandleListMostOrphaned(const char* args) {
    return true;
}
bool ChatHandler::ReturnOrphaned(const char* args) {
    return true;
}