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

#include "Log.h"
#include "MapManager.h"
#include "ObjectMgr.h"
#include "SpellMgr.h"
#include "UpdateMask.h"
#include "World.h"
#include "WorldSession.h"
#include "Group.h"
#include "Guild.h"
#include "ArenaTeam.h"
#include "Transport.h"
#include "Language.h"
#include "GameEvent.h"
#include "Spell.h"
#include "Chat.h"
#include "AccountMgr.h"
#include "InstanceSaveMgr.h"
#include "SpellAuras.h"
#include "Util.h"
#include "WaypointManager.h"
#include "InstanceData.h" //for condition_instance_data

ScriptMapMap sQuestEndScripts;
ScriptMapMap sQuestStartScripts;
ScriptMapMap sSpellScripts;
ScriptMapMap sGameObjectScripts;
ScriptMapMap sEventScripts;
ScriptMapMap sWaypointScripts;

bool normalizePlayerName(std::string& name)
{
    if(name.empty())
        return false;

    wchar_t wstr_buf[MAX_INTERNAL_PLAYER_NAME+1];
    size_t wstr_len = MAX_INTERNAL_PLAYER_NAME;

    if(!Utf8toWStr(name,&wstr_buf[0],wstr_len))
        return false;

    wstr_buf[0] = wcharToUpper(wstr_buf[0]);
    for(size_t i = 1; i < wstr_len; ++i)
        wstr_buf[i] = wcharToLower(wstr_buf[i]);

    if(!WStrToUtf8(wstr_buf,wstr_len,name))
        return false;

    return true;
}

LanguageDesc lang_description[LANGUAGES_COUNT] =
{
    { LANG_ADDON,           0, 0                       },
    { LANG_UNIVERSAL,       0, 0                       },
    { LANG_ORCISH,        669, SKILL_LANG_ORCISH       },
    { LANG_DARNASSIAN,    671, SKILL_LANG_DARNASSIAN   },
    { LANG_TAURAHE,       670, SKILL_LANG_TAURAHE      },
    { LANG_DWARVISH,      672, SKILL_LANG_DWARVEN      },
    { LANG_COMMON,        668, SKILL_LANG_COMMON       },
    { LANG_DEMONIC,       815, SKILL_LANG_DEMON_TONGUE },
    { LANG_TITAN,         816, SKILL_LANG_TITAN        },
    { LANG_THALASSIAN,    813, SKILL_LANG_THALASSIAN   },
    { LANG_DRACONIC,      814, SKILL_LANG_DRACONIC     },
    { LANG_KALIMAG,       817, SKILL_LANG_OLD_TONGUE   },
    { LANG_GNOMISH,      7340, SKILL_LANG_GNOMISH      },
    { LANG_TROLL,        7341, SKILL_LANG_TROLL        },
    { LANG_GUTTERSPEAK, 17737, SKILL_LANG_GUTTERSPEAK  },
    { LANG_DRAENEI,     29932, SKILL_LANG_DRAENEI      },
    { LANG_ZOMBIE,          0, 0                       },
    { LANG_GNOMISH_BINARY,  0, 0                       },
    { LANG_GOBLIN_BINARY,   0, 0                       }
};

LanguageDesc const* GetLanguageDescByID(uint32 lang)
{
    for(int i = 0; i < LANGUAGES_COUNT; ++i)
    {
        if(uint32(lang_description[i].lang_id) == lang)
            return &lang_description[i];
    }

    return NULL;
}

ObjectMgr::ObjectMgr()
{
    m_hiCharGuid        = 1;
    m_hiCreatureGuid    = 1;
    m_hiTempCreatureGuidStart = 1;
    m_hiTempCreatureGuid = 1;
    m_hiCreatureRegularModeGuid = !sWorld->getConfig(CONFIG_GUIDDISTRIB_NEWMETHOD);
    m_hiPetGuid         = 1;
    m_hiItemGuid        = 1;
    m_hiGoGuid          = 1;
    m_hiTempGoGuidStart = 1;
    m_hiTempGoGuid = 1;
    m_hiGoRegularModeGuid = !sWorld->getConfig(CONFIG_GUIDDISTRIB_NEWMETHOD);
    m_hiDoGuid          = 1;
    m_hiCorpseGuid      = 1;
    m_hiTransportGuid   = 1;
    m_hiPetNumber       = 1;
    m_ItemTextId        = 1;
    m_mailid            = 1;
    m_guildId           = 1;
    m_arenaTeamId       = 1;
    m_auctionid         = 1;

    mGuildBankTabPrice.resize(GUILD_BANK_MAX_TABS);
    mGuildBankTabPrice[0] = 100;
    mGuildBankTabPrice[1] = 250;
    mGuildBankTabPrice[2] = 500;
    mGuildBankTabPrice[3] = 1000;
    mGuildBankTabPrice[4] = 2500;
    mGuildBankTabPrice[5] = 5000;

    // Only zero condition left, others will be added while loading DB tables
    mConditions.resize(1);
}

ObjectMgr::~ObjectMgr()
{
    for( QuestMap::iterator i = mQuestTemplates.begin( ); i != mQuestTemplates.end( ); ++i )
    {
        delete i->second;
    }
    mQuestTemplates.clear( );

    for( GossipTextMap::iterator i = mGossipText.begin( ); i != mGossipText.end( ); ++i )
    {
        delete i->second;
    }
    mGossipText.clear( );

    mAreaTriggers.clear();

    for(PetLevelInfoMap::iterator i = petInfo.begin( ); i != petInfo.end( ); ++i )
    {
        delete[] i->second;
    }
    petInfo.clear();

    // free only if loaded
    for (int class_ = 0; class_ < MAX_CLASSES; ++class_)
        delete[] playerClassInfo[class_].levelInfo;

    for (int race = 0; race < MAX_RACES; ++race)
        for (int class_ = 0; class_ < MAX_CLASSES; ++class_)
            delete[] playerInfo[race][class_].levelInfo;

    // free group and guild objects
    for (GroupSet::iterator itr = mGroupSet.begin(); itr != mGroupSet.end(); ++itr)
        delete (*itr);

    for (GuildMap::iterator itr = mGuildMap.begin(); itr != mGuildMap.end(); ++itr)
        delete itr->second;
    mGuildMap.clear();

    for (CacheVendorItemMap::iterator itr = m_mCacheVendorItemMap.begin(); itr != m_mCacheVendorItemMap.end(); ++itr)
        itr->second.Clear();

    for (CacheTrainerSpellMap::iterator itr = m_mCacheTrainerSpellMap.begin(); itr != m_mCacheTrainerSpellMap.end(); ++itr)
        itr->second.Clear();
}

Group * ObjectMgr::GetGroupByLeader(const uint64 &guid) const
{
    for(GroupSet::const_iterator itr = mGroupSet.begin(); itr != mGroupSet.end(); ++itr)
        if ((*itr)->GetLeaderGUID() == guid)
            return *itr;

    return NULL;
}

Guild * ObjectMgr::_GetGuildById(const uint32 GuildId) const
{
    GuildMap::const_iterator itr = mGuildMap.find(GuildId);
    if (itr != mGuildMap.end())
        return itr->second;

    return NULL;
}

Guild * ObjectMgr::GetGuildById(const uint32 GuildId)
{
    Guild *guild = _GetGuildById(GuildId);
    if (guild)
        return guild;

    guild = new Guild;
    if (guild->LoadGuildFromDB(GuildId)) {
        AddGuild(guild);
        return guild;
    }

    guild->Disband();
    delete guild;
    return NULL;
}

Guild * ObjectMgr::_GetGuildByName(const std::string& guildname) const
{
    for(GuildMap::const_iterator itr = mGuildMap.begin(); itr != mGuildMap.end(); ++itr)
        if (itr->second->GetName() == guildname)
            return itr->second;

    return NULL;
}

Guild * ObjectMgr::GetGuildByName(const std::string& guildname)
{
    Guild *guild = _GetGuildByName(guildname);
    if (guild)
        return guild;

    guild = new Guild;
    if (guild->LoadGuildFromDB(guildname)) {
        AddGuild(guild);
        return guild;
    }

    guild->Disband();
    delete guild;
    return NULL;
}

std::string ObjectMgr::_GetGuildNameById(const uint32 GuildId) const
{
    GuildMap::const_iterator itr = mGuildMap.find(GuildId);
    if (itr != mGuildMap.end())
        return itr->second->GetName();

    return "";
}

std::string ObjectMgr::GetGuildNameById(const uint32 GuildId)
{
    std::string gname = _GetGuildNameById(GuildId);
    if (gname.length() > 0)
        return gname;

    Guild *guild = new Guild;
    if (guild->LoadGuildFromDB(GuildId)) {
        AddGuild(guild);
        return guild->GetName();
    }

    guild->Disband();
    delete guild;
    return "";
}

Guild* ObjectMgr::_GetGuildByLeader(const uint64 &guid) const
{
    for(GuildMap::const_iterator itr = mGuildMap.begin(); itr != mGuildMap.end(); ++itr)
        if (itr->second->GetLeader() == guid)
            return itr->second;

    return NULL;
}

bool ObjectMgr::IsGuildLeader(const uint64 &guid) const
{
    QueryResult result = CharacterDatabase.PQuery("SELECT guildid FROM guild WHERE leaderguid=%u", uint32(guid));
    if (!result)
        return false;

    return true;
}

void ObjectMgr::AddGuild(Guild* guild)
{
    mGuildMap[guild->GetId()] = guild;
}

void ObjectMgr::RemoveGuild(uint32 Id)
{
    mGuildMap.erase(Id);
}

ArenaTeam* ObjectMgr::_GetArenaTeamById(const uint32 arenateamid) const
{
    ArenaTeamMap::const_iterator itr = mArenaTeamMap.find(arenateamid);
    if (itr != mArenaTeamMap.end())
        return itr->second;

    return NULL;
}

ArenaTeam* ObjectMgr::GetArenaTeamById(const uint32 arenateamid)
{
    ArenaTeam *team = _GetArenaTeamById(arenateamid);
    if (team)
        return team;

    team = new ArenaTeam;
    if (team->LoadArenaTeamFromDB(arenateamid)) {
        AddArenaTeam(team);
        return team;
    }

    delete team;
    return NULL;
}

ArenaTeam* ObjectMgr::_GetArenaTeamByName(const std::string& arenateamname) const
{
    for(ArenaTeamMap::const_iterator itr = mArenaTeamMap.begin(); itr != mArenaTeamMap.end(); ++itr)
        if (itr->second->GetName() == arenateamname)
            return itr->second;

    return NULL;
}

ArenaTeam* ObjectMgr::GetArenaTeamByName(const std::string& arenateamname)
{
    ArenaTeam *team = _GetArenaTeamByName(arenateamname);
    if (team)
        return team;

    team = new ArenaTeam;
    if (team->LoadArenaTeamFromDB(arenateamname)) {
        AddArenaTeam(team);
        return team;
    }

    delete team;
    return NULL;
}

ArenaTeam* ObjectMgr::_GetArenaTeamByCaptain(uint64 const& guid) const
{
    for(ArenaTeamMap::const_iterator itr = mArenaTeamMap.begin(); itr != mArenaTeamMap.end(); ++itr)
        if (itr->second->GetCaptain() == guid)
            return itr->second;

    return NULL;
}

bool ObjectMgr::IsArenaTeamCaptain(uint64 const& guid) const
{
    QueryResult result = CharacterDatabase.PQuery("SELECT arenateamid FROM arena_team WHERE captainguid=%u", uint32(guid));
    if (!result)
        return false;

    return true;
}

void ObjectMgr::AddArenaTeam(ArenaTeam* arenaTeam)
{
    mArenaTeamMap[arenaTeam->GetId()] = arenaTeam;
}

void ObjectMgr::RemoveArenaTeam(uint32 Id)
{
    mArenaTeamMap.erase(Id);
}

CreatureTemplate const* ObjectMgr::GetCreatureTemplate(uint32 entry)
{
    auto itr = _creatureTemplateStore.find(entry);
    if (itr != _creatureTemplateStore.end())
        return &(itr->second);

    return NULL;
}

GameObjectTemplate const* ObjectMgr::GetGameObjectTemplate(uint32 entry)
{
    auto itr = _gameObjectTemplateStore.find(entry);
    if (itr != _gameObjectTemplateStore.end())
        return &(itr->second);

    return NULL;
}

void ObjectMgr::LoadCreatureLocales()
{
    uint32 oldMSTime = GetMSTime();

    _creatureLocaleStore.clear();                              // need for reload case
//                                                    0        1                     3                      5                      7                      9                       11                    13                      15                
    QueryResult result = WorldDatabase.Query("SELECT entry,name_loc1,subname_loc1,name_loc2,subname_loc2,name_loc3,subname_loc3,name_loc4,subname_loc4,name_loc5,subname_loc5,name_loc6,subname_loc6,name_loc7,subname_loc7,name_loc8,subname_loc8 FROM locales_creature");

    if(!result)
    {
        TC_LOG_INFO("sql.sql",">> Loaded 0 creature locale strings. DB table `locales_creature` is empty.");
        return;
    }

    do
    {
        Field* fields = result->Fetch();

        uint32 entry = fields[0].GetUInt32();

        CreatureLocale& data = _creatureLocaleStore[entry];

        for (uint8 i = MAX_LOCALE; i > 0; --i)
        {
            LocaleConstant locale = (LocaleConstant) i;
            AddLocaleString(fields[1 + 2 * (i - 1)].GetString(), locale, data.Name);
            AddLocaleString(fields[1 + 2 * (i - 1) + 1].GetString(), locale, data.SubName);
        }
    } while (result->NextRow());

    TC_LOG_INFO("server.loading", ">> Loaded %u creature locale strings in %u ms", uint32(_creatureLocaleStore.size()), GetMSTimeDiffToNow(oldMSTime));
}

void ObjectMgr::LoadNpcOptionLocales()
{
    mNpcOptionLocaleMap.clear();                              // need for reload case

    QueryResult result = WorldDatabase.Query("SELECT entry,"
        "option_text_loc1,box_text_loc1,option_text_loc2,box_text_loc2,"
        "option_text_loc3,box_text_loc3,option_text_loc4,box_text_loc4,"
        "option_text_loc5,box_text_loc5,option_text_loc6,box_text_loc6,"
        "option_text_loc7,box_text_loc7,option_text_loc8,box_text_loc8 "
        "FROM locales_npc_option");

    if(!result)
    {
        TC_LOG_INFO("sql.sql",">> Loaded 0 npc_option locale strings. DB table `locales_npc_option` is empty.");
        return;
    }

    do
    {
        Field *fields = result->Fetch();

        uint32 entry = fields[0].GetUInt32();

        NpcOptionLocale& data = mNpcOptionLocaleMap[entry];

        for (uint8 i = TOTAL_LOCALES - 1; i > 0; --i)
        {
            LocaleConstant locale = (LocaleConstant) i;
            AddLocaleString(fields[1 + 2 * (i - 1)].GetString(), locale, data.OptionText);
            AddLocaleString(fields[1 + 2 * (i - 1) + 1].GetString(), locale, data.BoxText);
        }
    } while (result->NextRow());

    TC_LOG_INFO("server.loading", ">> Loaded " UI64FMTD " npc_option locale strings", mNpcOptionLocaleMap.size() );
}

void ObjectMgr::LoadCreatureTemplates()
{
    uint32 oldMSTime = GetMSTime();

    //                                                 0          1                2           3       4
    QueryResult result = WorldDatabase.Query("SELECT entry, difficulty_entry_1, modelid1, modelid2, modelid3, "
                                             //   5             7                    9               11              13
                                             "modelid4, name, subname, IconName, minlevel, maxlevel, exp, faction, npcflag, speed, "
                                             // 15
                                             "scale, rank, dmgschool, BaseAttackTime, RangeAttackTime, BaseVariance, RangeVariance, unit_class, unit_flags, dynamicflags, family,"
                                             //   
                                             "trainer_type, trainer_spell, trainer_class, trainer_race, type,"
                                             //  
                                             "type_flags, lootid, pickpocketloot, skinloot, resistance1, resistance2, resistance3, resistance4, resistance5, resistance6, spell1, "
                                             //
                                             "spell2, spell3, spell4, spell5, spell6, spell7, spell8, PetSpellDataId, mingold, maxgold, AIName, MovementType, "
                                             //           
                                             "InhabitType, HealthModifier, ManaModifier, ArmorModifier, DamageModifier, ExperienceModifier, RacialLeader, RegenHealth, equipment_id, "
                                             //   
                                             "mechanic_immune_mask, flags_extra, creature_template.ScriptName, creature_scripts.scriptname "
                                             "FROM creature_template LEFT JOIN creature_scripts ON entryorguid = entry;");

    if (!result)
    {
        TC_LOG_INFO("server.loading", ">> Loaded 0 creature template definitions. DB table `creature_template` is empty.");
        return;
    }

    _creatureTemplateStore.rehash(result->GetRowCount());
    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();
        LoadCreatureTemplate(fields);
        ++count;
    }
    while (result->NextRow());

    // Checking needs to be done after loading because of the difficulty self referencing
    for (CreatureTemplateContainer::const_iterator itr = _creatureTemplateStore.begin(); itr != _creatureTemplateStore.end(); ++itr)
        CheckCreatureTemplate(&itr->second);

    TC_LOG_INFO("server.loading", ">> Loaded %u creature definitions in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void ObjectMgr::LoadCreatureTemplate(Field* fields)
{
    uint8 f = 0;
    uint32 entry = fields[f++].GetUInt32();

    CreatureTemplate& creatureTemplate = _creatureTemplateStore[entry];

    creatureTemplate.Entry = entry;
    creatureTemplate.difficulty_entry_1 = fields[f++].GetUInt32();

    creatureTemplate.Modelid1         = fields[f++].GetUInt32();
    creatureTemplate.Modelid2         = fields[f++].GetUInt32();
    creatureTemplate.Modelid3         = fields[f++].GetUInt32();
    creatureTemplate.Modelid4         = fields[f++].GetUInt32();
    creatureTemplate.Name             = fields[f++].GetString();
    creatureTemplate.SubName          = fields[f++].GetString();
    creatureTemplate.IconName         = fields[f++].GetString();
    creatureTemplate.minlevel         = fields[f++].GetUInt8();
    creatureTemplate.maxlevel         = fields[f++].GetUInt8();
    creatureTemplate.expansion        = fields[f++].GetUInt32();
    creatureTemplate.faction          = fields[f++].GetUInt16();
    creatureTemplate.npcflag          = fields[f++].GetUInt32();
    creatureTemplate.speed            = fields[f++].GetFloat();
    creatureTemplate.scale            = fields[f++].GetFloat();
    creatureTemplate.rank             = fields[f++].GetUInt8();
    creatureTemplate.dmgschool        = uint32(fields[f++].GetInt8());
    creatureTemplate.baseattacktime   = fields[f++].GetUInt32();
    creatureTemplate.rangeattacktime  = fields[f++].GetUInt32();
    creatureTemplate.BaseVariance     = fields[f++].GetFloat();
    creatureTemplate.RangeVariance    = fields[f++].GetFloat();
    creatureTemplate.unit_class       = fields[f++].GetUInt8();
    creatureTemplate.unit_flags       = fields[f++].GetUInt32();
    creatureTemplate.dynamicflags     = fields[f++].GetUInt32();
    creatureTemplate.family           = fields[f++].GetUInt8();
    creatureTemplate.trainer_type     = fields[f++].GetUInt8();
    creatureTemplate.trainer_spell    = fields[f++].GetUInt32();
    creatureTemplate.trainer_class    = fields[f++].GetUInt8();
    creatureTemplate.trainer_race     = fields[f++].GetUInt8();
    creatureTemplate.type             = fields[f++].GetUInt8();
    creatureTemplate.type_flags       = fields[f++].GetUInt32();
    creatureTemplate.lootid           = fields[f++].GetUInt32();
    creatureTemplate.pickpocketLootId = fields[f++].GetUInt32();
    creatureTemplate.SkinLootId       = fields[f++].GetUInt32();

    for (uint8 i = SPELL_SCHOOL_HOLY; i < MAX_SPELL_SCHOOL; ++i)
        creatureTemplate.resistance[i] = fields[f++].GetInt16();

    for (uint8 i = 0; i < CREATURE_MAX_SPELLS; ++i)
        creatureTemplate.spells[i] = fields[f++].GetUInt32();

    creatureTemplate.PetSpellDataId = fields[f++].GetUInt32();
    creatureTemplate.mingold        = fields[f++].GetUInt32();
    creatureTemplate.maxgold        = fields[f++].GetUInt32();
    creatureTemplate.AIName         = fields[f++].GetString();
    creatureTemplate.MovementType   = fields[f++].GetUInt8();
    creatureTemplate.InhabitType    = fields[f++].GetUInt8();
    creatureTemplate.ModHealth      = fields[f++].GetFloat();
    creatureTemplate.ModMana        = fields[f++].GetFloat();
    creatureTemplate.ModArmor       = fields[f++].GetFloat();
    creatureTemplate.ModDamage      = fields[f++].GetFloat();
    creatureTemplate.ModExperience  = fields[f++].GetFloat();
    creatureTemplate.RacialLeader   = fields[f++].GetBool();
    creatureTemplate.RegenHealth    = fields[f++].GetBool();
    creatureTemplate.equipmentId    = fields[f++].GetUInt32();
    creatureTemplate.MechanicImmuneMask = fields[f++].GetUInt32();
    creatureTemplate.flags_extra        = fields[f++].GetUInt32();
    creatureTemplate.ScriptID           = GetScriptId(fields[f++].GetCString());
    creatureTemplate.scriptName         = fields[f++].GetString();
}

void ObjectMgr::CheckCreatureTemplate(CreatureTemplate const* cInfo)
{
    if(cInfo->difficulty_entry_1)
    {
        uint32 i = cInfo->Entry;
        CreatureTemplate const* heroicInfo = GetCreatureTemplate(cInfo->difficulty_entry_1);
        if(!heroicInfo)
        {
            TC_LOG_ERROR("sql.sql", "Creature (Entry: %u) have `difficulty_entry_1`=%u but the creature does not exist.", i, cInfo->difficulty_entry_1);
        }
        /*
        if(heroicEntries.find(i)!=heroicEntries.end())
        {
            TC_LOG_ERROR("sql.sql","Creature (Entry: %u) listed as heroic but have value in `heroic_entry`.",i);
        }
        if(heroicEntries.find(cInfo->difficulty_entry_1)!=heroicEntries.end())
        {
            TC_LOG_ERROR("sql.sql","Creature (Entry: %u) already listed as heroic for another entry.",cInfo->difficulty_entry_1);
        }
        if(hasHeroicEntries.find(cInfo->difficulty_entry_1)!=hasHeroicEntries.end())
        {
            TC_LOG_ERROR("sql.sql","Creature (Entry: %u) have `difficulty_entry_1`=%u but creature entry %u have heroic entry also.",i,cInfo->difficulty_entry_1,cInfo->difficulty_entry_1);
        }*/

        if(cInfo->npcflag != heroicInfo->npcflag)
        {
            TC_LOG_ERROR("sql.sql","Creature (Entry: %u) listed in `creature_template_substitution` has different `npcflag` in heroic mode.",i);
        }

        if(cInfo->trainer_class != heroicInfo->trainer_class)
        {
            TC_LOG_ERROR("sql.sql","Creature (Entry: %u) listed in `creature_template_substitution` has different `trainer_class` in heroic mode.",i);
        }

        if(cInfo->trainer_race != heroicInfo->trainer_race)
        {
            TC_LOG_ERROR("sql.sql","Creature (Entry: %u) listed in `creature_template_substitution` has different `race` in heroic mode.",i);
        }

        if(cInfo->trainer_type != heroicInfo->trainer_type)
        {
            TC_LOG_ERROR("sql.sql","Creature (Entry: %u) listed in `creature_template_substitution` has different `trainer_type` in heroic mode.",i);
        }

        if(cInfo->trainer_spell != heroicInfo->trainer_spell)
        {
            TC_LOG_ERROR("sql.sql","Creature (Entry: %u) listed in `creature_template_substitution` has different `trainer_spell` in heroic mode.",i);
        }
        /*
        hasHeroicEntries.insert(i);
        heroicEntries.insert(cInfo->difficulty_entry_1);**/
    }

    FactionTemplateEntry const* factionTemplate = sFactionTemplateStore.LookupEntry(cInfo->faction);
    if(!factionTemplate)
        TC_LOG_ERROR("sql.sql","Creature (Entry: %u) has non-existing faction template (%u)", cInfo->Entry, cInfo->faction);

    // check model ids, supplying and sending non-existent ids to the client might crash them
    if(cInfo->Modelid1 && !GetCreatureModelInfo(cInfo->Modelid1))
    {
        TC_LOG_ERROR("sql.sql","Creature (Entry: %u) has non-existing Modelid1 (%u), setting it to 0", cInfo->Entry, cInfo->Modelid1);
        const_cast<CreatureTemplate*>(cInfo)->Modelid1 = 0;
    }
    if(cInfo->Modelid2 && !GetCreatureModelInfo(cInfo->Modelid2))
    {
        TC_LOG_ERROR("sql.sql","Creature (Entry: %u) has non-existing Modelid2 (%u), setting it to 0", cInfo->Entry, cInfo->Modelid2);
        const_cast<CreatureTemplate*>(cInfo)->Modelid2 = 0;
    }
    if(cInfo->Modelid3 && !GetCreatureModelInfo(cInfo->Modelid3))
    {
        TC_LOG_ERROR("sql.sql","Creature (Entry: %u) has non-existing Modelid3 (%u), setting it to 0", cInfo->Entry, cInfo->Modelid3);
        const_cast<CreatureTemplate*>(cInfo)->Modelid3 = 0;
    }
    if(cInfo->Modelid4 && !GetCreatureModelInfo(cInfo->Modelid4))
    {
        TC_LOG_ERROR("sql.sql","Creature (Entry: %u) has non-existing Modelid4 (%u), setting it to 0", cInfo->Entry, cInfo->Modelid4);
        const_cast<CreatureTemplate*>(cInfo)->Modelid4 = 0;
    }

    if(cInfo->dmgschool >= MAX_SPELL_SCHOOL)
    {
        TC_LOG_ERROR("sql.sql","Creature (Entry: %u) has invalid spell school value (%u) in `dmgschool`",cInfo->Entry,cInfo->dmgschool);
        const_cast<CreatureTemplate*>(cInfo)->dmgschool = SPELL_SCHOOL_NORMAL;
    }

    if(cInfo->baseattacktime == 0)
        const_cast<CreatureTemplate*>(cInfo)->baseattacktime  = BASE_ATTACK_TIME;

    if(cInfo->rangeattacktime == 0)
        const_cast<CreatureTemplate*>(cInfo)->rangeattacktime = BASE_ATTACK_TIME;

    if((cInfo->npcflag & UNIT_NPC_FLAG_TRAINER) && cInfo->trainer_type >= MAX_TRAINER_TYPE)
        TC_LOG_ERROR("sql.sql","Creature (Entry: %u) has wrong trainer type %u",cInfo->Entry,cInfo->trainer_type);

    if(cInfo->InhabitType <= 0 || cInfo->InhabitType > INHABIT_ANYWHERE)
    {
        TC_LOG_ERROR("sql.sql","Creature (Entry: %u) has wrong value (%u) in `InhabitType`, creature will not correctly walk/swim/fly",cInfo->Entry,cInfo->InhabitType);
        const_cast<CreatureTemplate*>(cInfo)->InhabitType = INHABIT_ANYWHERE;
    }

    if(cInfo->PetSpellDataId)
    {
        CreatureSpellDataEntry const* spellDataId = sCreatureSpellDataStore.LookupEntry(cInfo->PetSpellDataId);
        if(!spellDataId)
            TC_LOG_ERROR("sql.sql","Creature (Entry: %u) has non-existing PetSpellDataId (%u)", cInfo->Entry, cInfo->PetSpellDataId);
    }

    if(cInfo->MovementType >= MAX_DB_MOTION_TYPE)
    {
        TC_LOG_ERROR("sql.sql","Creature (Entry: %u) has wrong movement generator type (%u), ignore and set to IDLE.",cInfo->Entry,cInfo->MovementType);
        const_cast<CreatureTemplate*>(cInfo)->MovementType = IDLE_MOTION_TYPE;
    }

    if(cInfo->equipmentId > 0)                          // 0 no equipment
    {
        if(!GetEquipmentInfo(cInfo->equipmentId))
        {
            TC_LOG_ERROR("sql.sql","Table `creature_template` have creature (Entry: %u) with equipment_id %u not found in table `creature_equip_template`, set to no equipment.", cInfo->Entry, cInfo->equipmentId);
            const_cast<CreatureTemplate*>(cInfo)->equipmentId = 0;
        }
    }

    /// if not set custom creature scale then load scale from CreatureDisplayInfo.dbc
    if(cInfo->scale <= 0.0f)
    {
        uint32 modelid = cInfo->GetFirstValidModelId();
        CreatureDisplayInfoEntry const* ScaleEntry = sCreatureDisplayInfoStore.LookupEntry(modelid);
        const_cast<CreatureTemplate*>(cInfo)->scale = ScaleEntry ? ScaleEntry->scale : 1.0f;
    }
}

void ObjectMgr::LoadCreatureAddons()
{
    uint32 oldMSTime = GetMSTime();

    //                                                0       1       2      3       4       5        6        7
    QueryResult result = WorldDatabase.Query("SELECT guid, path_id, mount, bytes1, bytes2, emote, moveflags, auras FROM creature_addon");

    if (!result)
    {
        TC_LOG_INFO("server.loading", ">> Loaded 0 creature addon definitions. DB table `creature_addon` is empty.");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        uint32 guid = fields[0].GetUInt32();

        CreatureData const* creData = GetCreatureData(guid);
        if (!creData)
        {
            TC_LOG_ERROR("sql.sql", "Creature (GUID: %u) does not exist but has a record in `creature_addon`", guid);
            continue;
        }

        CreatureAddon& creatureAddon = _creatureAddonStore[guid];

        creatureAddon.path_id = fields[1].GetUInt32();
        if (creData->movementType == WAYPOINT_MOTION_TYPE && !creatureAddon.path_id)
        {
            const_cast<CreatureData*>(creData)->movementType = IDLE_MOTION_TYPE;
            TC_LOG_ERROR("sql.sql", "Creature (GUID %u) has movement type set to WAYPOINT_MOTION_TYPE but no path assigned", guid);
        }

        creatureAddon.mount   = fields[2].GetUInt32();
        creatureAddon.bytes1  = fields[3].GetUInt32();
        creatureAddon.bytes2  = fields[4].GetUInt32();
        creatureAddon.emote   = fields[5].GetUInt32();
        creatureAddon.move_flags = fields[6].GetUInt32();

        Tokenizer tokens(fields[7].GetString(), ' ');
        uint8 i = 0;
        creatureAddon.auras.resize(tokens.size());
        for (Tokenizer::const_iterator itr = tokens.begin(); itr != tokens.end(); ++itr)
        {
            SpellEntry const* AdditionalSpellInfo = sSpellMgr->GetSpellInfo(uint32(atol(*itr)));
            if (!AdditionalSpellInfo)
            {
                TC_LOG_ERROR("sql.sql", "Creature (GUID: %u) has wrong spell %u defined in `auras` field in `creature_addon`.", guid, uint32(atol(*itr)));
                continue;
            }
#ifdef LICH_KING
            if (AdditionalSpellInfo->HasAura(SPELL_AURA_CONTROL_VEHICLE))
                TC_LOG_ERROR("sql.sql", "Creature (GUID: %u) has SPELL_AURA_CONTROL_VEHICLE aura %u defined in `auras` field in `creature_addon`.", guid, uint32(atol(*itr)));
#endif
            creatureAddon.auras[i++] = uint32(atol(*itr));
        }

        if (creatureAddon.mount)
        {
            if (!sCreatureDisplayInfoStore.LookupEntry(creatureAddon.mount))
            {
                TC_LOG_ERROR("sql.sql", "Creature (GUID: %u) has invalid displayInfoId (%u) for mount defined in `creature_addon`", guid, creatureAddon.mount);
                creatureAddon.mount = 0;
            }
        }

        /*if (!sEmotesStore.LookupEntry(creatureAddon.emote))
        {
            TC_LOG_ERROR("sql.sql", "Creature (GUID: %u) has invalid emote (%u) defined in `creature_addon`.", guid, creatureAddon.emote);
            creatureAddon.emote = 0;
        }*/

        if(mCreatureDataMap.find(guid)==mCreatureDataMap.end())
            TC_LOG_ERROR("sql.sql","Creature (GUID: %u) does not exist but has a record in `creature_addon`",guid);

        ++count;
    }
    while (result->NextRow());

    TC_LOG_INFO("server.loading", ">> Loaded %u creature addons in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

CreatureAddon const* ObjectMgr::GetCreatureAddon(uint32 lowguid)
{
    auto itr = _creatureAddonStore.find(lowguid);
    if (itr != _creatureAddonStore.end())
        return &(itr->second);

    return NULL;
}

void ObjectMgr::LoadCreatureTemplateAddons()
{
    uint32 oldMSTime = GetMSTime();

    //                                                0       1       2      3       4       5        6         7
    QueryResult result = WorldDatabase.Query("SELECT entry, path_id, mount, bytes1, bytes2, emote, moveflags, auras FROM creature_template_addon");

    if (!result)
    {
        TC_LOG_INFO("server.loading", ">> Loaded 0 creature template addon definitions. DB table `creature_template_addon` is empty.");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        uint32 entry = fields[0].GetUInt32();

        if (!sObjectMgr->GetCreatureTemplate(entry))
        {
            TC_LOG_ERROR("sql.sql", "Creature template (Entry: %u) does not exist but has a record in `creature_template_addon`", entry);
            continue;
        }

        CreatureAddon& creatureAddon = _creatureTemplateAddonStore[entry];

        creatureAddon.path_id = fields[1].GetUInt32();
        creatureAddon.mount   = fields[2].GetUInt32();
        creatureAddon.bytes1  = fields[3].GetUInt32();
        creatureAddon.bytes2  = fields[4].GetUInt32();
        creatureAddon.emote   = fields[5].GetUInt32();
        creatureAddon.move_flags = fields[6].GetUInt32();

        Tokenizer tokens(fields[7].GetString(), ' ');
        uint8 i = 0;
        creatureAddon.auras.resize(tokens.size());
        for (Tokenizer::const_iterator itr = tokens.begin(); itr != tokens.end(); ++itr)
        {
            SpellEntry const* AdditionalSpellInfo = sSpellMgr->GetSpellInfo(uint32(atol(*itr)));
            if (!AdditionalSpellInfo)
            {
                TC_LOG_ERROR("sql.sql", "Creature (Entry: %u) has wrong spell %u defined in `auras` field in `creature_template_addon`.", entry, uint32(atol(*itr)));
                continue;
            }
#ifdef LICH_KING
            if (AdditionalSpellInfo->HasAura(SPELL_AURA_CONTROL_VEHICLE))
                TC_LOG_ERROR("sql.sql", "Creature (Entry: %u) has SPELL_AURA_CONTROL_VEHICLE aura %u defined in `auras` field in `creature_template_addon`.", entry, uint32(atol(*itr)));
#endif
            creatureAddon.auras[i++] = uint32(atol(*itr));
        }

        if (creatureAddon.mount)
        {
            if (!sCreatureDisplayInfoStore.LookupEntry(creatureAddon.mount))
            {
                TC_LOG_ERROR("sql.sql", "Creature (Entry: %u) has invalid displayInfoId (%u) for mount defined in `creature_template_addon`", entry, creatureAddon.mount);
                creatureAddon.mount = 0;
            }
        }
        /*
        if (!sEmotesStore.LookupEntry(creatureAddon.emote))
        {
            TC_LOG_ERROR("sql.sql", "Creature (Entry: %u) has invalid emote (%u) defined in `creature_template_addon`.", entry, creatureAddon.emote);
            creatureAddon.emote = 0;
        }
        */
        ++count;
    }
    while (result->NextRow());

    TC_LOG_INFO("server.loading", ">> Loaded %u creature template addons in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

CreatureAddon const* ObjectMgr::GetCreatureTemplateAddon(uint32 entry)
{
    CreatureAddonContainer::const_iterator itr = _creatureTemplateAddonStore.find(entry);
    if (itr != _creatureTemplateAddonStore.end())
        return &(itr->second);

    return NULL;
}

EquipmentInfo const* ObjectMgr::GetEquipmentInfo(uint32 entry)
{
    EquipmentInfoContainer::const_iterator itr = _equipmentInfoStore.find(entry);
    if (itr == _equipmentInfoStore.end())
        return NULL;
    /*
    if (itr->second.empty())
        return NULL;

    if (id == -1) // select a random element
    {
        EquipmentInfoContainerInternal::const_iterator ritr = itr->second.begin();
        std::advance(ritr, urand(0u, itr->second.size() - 1));
        id = std::distance(itr->second.begin(), ritr) + 1;
        return &ritr->second;
    }
    else
    {
        EquipmentInfoContainerInternal::const_iterator itr2 = itr->second.find(id);
        if (itr2 != itr->second.end())
            return &itr2->second;
    }

    return nullptr;
    */
    return &itr->second;
}

void ObjectMgr::LoadEquipmentTemplates()
{
    uint32 oldMSTime = GetMSTime();

    //                                                 0     1        2           3            4           5           6            7          8           9         10
    QueryResult result = WorldDatabase.Query("SELECT entry, id, equipmodel1, equipmodel2, equipmodel3, equipinfo1, equipinfo2, equipinfo3, equipslot1, equipslot2, equipslot3 FROM creature_equip_template");

    if (!result)
    {
        TC_LOG_INFO("server.loading", ">> Loaded 0 creature equipment templates. DB table `creature_equip_template` is empty!");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        uint32 entry = fields[0].GetUInt32();

        /* re enable this when creature_equip_template has been updated to match creature_template
        if (!sObjectMgr->GetCreatureTemplate(entry))
        {
            TC_LOG_ERROR("sql.sql", "Creature template (Entry: %u) does not exist but has a record in `creature_equip_template`", entry);
            continue;
        }
        */
        uint8 id = fields[1].GetUInt8();
        
        EquipmentInfo& equipmentInfo = _equipmentInfoStore[entry];

        equipmentInfo.equipmodel[0] = fields[2].GetUInt32();
        equipmentInfo.equipmodel[1] = fields[3].GetUInt32();
        equipmentInfo.equipmodel[2] = fields[4].GetUInt32();
        equipmentInfo.equipinfo[0] = fields[5].GetUInt32();
        equipmentInfo.equipinfo[1] = fields[6].GetUInt32();
        equipmentInfo.equipinfo[2] = fields[7].GetUInt32();
        equipmentInfo.equipslot[0] = fields[8].GetUInt32();
        equipmentInfo.equipslot[1] = fields[9].GetUInt32();
        equipmentInfo.equipslot[2] = fields[10].GetUInt32();
        /*
        for (uint8 i = 0; i < MAX_EQUIPMENT_ITEMS; ++i)
        {
            if (!equipmentInfo.ItemEntry[i])
                continue;

            ItemEntry const* dbcItem = sItemStore.LookupEntry(equipmentInfo.ItemEntry[i]);

            if (!dbcItem)
            {
                TC_LOG_ERROR("sql.sql", "Unknown item (entry=%u) in creature_equip_template.itemEntry%u for entry = %u and id=%u, forced to 0.",
                    equipmentInfo.ItemEntry[i], i+1, entry, id);
                equipmentInfo.ItemEntry[i] = 0;
                continue;
            }

            if (dbcItem->InventoryType != INVTYPE_WEAPON &&
                dbcItem->InventoryType != INVTYPE_SHIELD &&
                dbcItem->InventoryType != INVTYPE_RANGED &&
                dbcItem->InventoryType != INVTYPE_2HWEAPON &&
                dbcItem->InventoryType != INVTYPE_WEAPONMAINHAND &&
                dbcItem->InventoryType != INVTYPE_WEAPONOFFHAND &&
                dbcItem->InventoryType != INVTYPE_HOLDABLE &&
                dbcItem->InventoryType != INVTYPE_THROWN &&
                dbcItem->InventoryType != INVTYPE_RANGEDRIGHT)
            {
                TC_LOG_ERROR("sql.sql", "Item (entry=%u) in creature_equip_template.itemEntry%u for entry = %u and id = %u is not equipable in a hand, forced to 0.",
                    equipmentInfo.ItemEntry[i], i+1, entry, id);
                equipmentInfo.ItemEntry[i] = 0;
            }
        }*/

        ++count;
    }
    while (result->NextRow());

    TC_LOG_INFO("server.loading", ">> Loaded %u equipment templates in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

ItemTemplate const* ObjectMgr::GetItemTemplate(uint32 entry)
{
    ItemTemplateContainer::const_iterator itr = _itemTemplateStore.find(entry);
    if (itr != _itemTemplateStore.end())
        return &(itr->second);

    return NULL;
}

CreatureModelInfo const* ObjectMgr::GetCreatureModelInfo(uint32 modelId)
{
    CreatureModelContainer::const_iterator itr = _creatureModelStore.find(modelId);
    if (itr != _creatureModelStore.end())
        return &(itr->second);

    return NULL;
}

uint32 ObjectMgr::ChooseDisplayId(uint32 team, const CreatureTemplate *cinfo, const CreatureData *data)
{
    // Load creature model (display id)
    uint32 display_id = 0;

    if (!data || data->displayid == 0) // use defaults from the template
    {
        display_id = cinfo->GetRandomValidModelId();
    } else display_id = data->displayid; // overwritten from creature data

    return display_id;
}

CreatureModelInfo const* ObjectMgr::GetCreatureModelRandomGender(uint32* displayID)
{
    CreatureModelInfo const* modelInfo = GetCreatureModelInfo(*displayID);
    if (!modelInfo)
        return NULL;

    // If a model for another gender exists, 50% chance to use it
    if (modelInfo->modelid_other_gender != 0 && urand(0, 1) == 0)
    {
        CreatureModelInfo const* minfo_tmp = GetCreatureModelInfo(modelInfo->modelid_other_gender);
        if (!minfo_tmp)
            TC_LOG_ERROR("sql.sql", "Model (Entry: %u) has modelid_other_gender %u not found in table `creature_model_info`. ", *displayID, modelInfo->modelid_other_gender);
        else
        {
            // Model ID changed
            *displayID = modelInfo->modelid_other_gender;
            return minfo_tmp;
        }
    }

    return modelInfo;
}

void ObjectMgr::LoadCreatureModelInfo()
{
    uint32 oldMSTime = GetMSTime();

    QueryResult result = WorldDatabase.Query("SELECT modelid, bounding_radius, combat_reach, gender, modelid_other_gender FROM creature_model_info");

    if (!result)
    {
        TC_LOG_INFO("server.loading", ">> Loaded 0 creature model definitions. DB table `creature_model_info` is empty.");
        return;
    }

    _creatureModelStore.rehash(result->GetRowCount());
    uint32 count = 0;

    do
    {
        Field* fields = result->Fetch();

        uint32 modelId = fields[0].GetUInt32();

        CreatureModelInfo& modelInfo = _creatureModelStore[modelId];

        modelInfo.bounding_radius      = fields[1].GetFloat();
        modelInfo.combat_reach         = fields[2].GetFloat();
        modelInfo.gender               = fields[3].GetUInt8();
        modelInfo.modelid_other_gender = fields[4].GetUInt32();

        // Checks

        if (!sCreatureDisplayInfoStore.LookupEntry(modelId))
            TC_LOG_ERROR("sql.sql", "Table `creature_model_info` has model for not existed display id (%u).", modelId);

        if (modelInfo.gender > GENDER_NONE)
        {
            TC_LOG_ERROR("sql.sql", "Table `creature_model_info` has wrong gender (%u) for display id (%u).", uint32(modelInfo.gender), modelId);
            modelInfo.gender = GENDER_MALE;
        }

        if (modelInfo.modelid_other_gender && !sCreatureDisplayInfoStore.LookupEntry(modelInfo.modelid_other_gender))
        {
            TC_LOG_ERROR("sql.sql", "Table `creature_model_info` has not existed alt.gender model (%u) for existed display id (%u).", modelInfo.modelid_other_gender, modelId);
            modelInfo.modelid_other_gender = 0;
        }

        if (modelInfo.combat_reach < 0.1f)
            modelInfo.combat_reach = DEFAULT_COMBAT_REACH;

        ++count;
    }
    while (result->NextRow());

    TC_LOG_INFO("server.loading", ">> Loaded %u creature model based info in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

bool ObjectMgr::CheckCreatureLinkedRespawn(uint32 guid, uint32 linkedGuid) const
{
    const CreatureData* const slave = GetCreatureData(guid);
    const CreatureData* const master = GetCreatureData(linkedGuid);
    
    if(!slave || !master) // they must have a corresponding entry in db
    {
        TC_LOG_ERROR("sql.sql","LinkedRespawn: Creature '%u' linking to '%u' which doesn't exist",guid,linkedGuid);
        return false;
    }

    const MapEntry* const map = sMapStore.LookupEntry(master->mapid);
        
    if(master->mapid != slave->mapid        // link only to same map
        && (!map || map->Instanceable()))   // or to unistanced world
    {
        TC_LOG_ERROR("sql.sql","LinkedRespawn: Creature '%u' linking to '%u' on an unpermitted map",guid,linkedGuid);
        return false;
    }

    if(!(master->spawnMask & slave->spawnMask)  // they must have a possibility to meet (normal/heroic difficulty)
        && (!map || map->Instanceable()))
    {
        TC_LOG_ERROR("sql.sql","LinkedRespawn: Creature '%u' linking to '%u' with not corresponding spawnMask",guid,linkedGuid);
        return false;
    }

    return true;
}

void ObjectMgr::LoadCreatureLinkedRespawn()
{
    mCreatureLinkedRespawnMap.clear();
    QueryResult result = WorldDatabase.Query("SELECT guid, linkedGuid FROM creature_linked_respawn ORDER BY guid ASC");

    if(!result)
    {
        TC_LOG_ERROR("sql.sql",">> Loaded 0 linked respawns. DB table `creature_linked_respawn` is empty.");
        return;
    }

    do
    {
        Field *fields = result->Fetch();

        uint32 guid = fields[0].GetUInt32();
        uint32 linkedGuid = fields[1].GetUInt32();

        if(CheckCreatureLinkedRespawn(guid,linkedGuid))
            mCreatureLinkedRespawnMap[guid] = linkedGuid;

    } while (result->NextRow());

    TC_LOG_INFO("server.loading", ">> Loaded " UI64FMTD " linked respawns", mCreatureLinkedRespawnMap.size());
}

bool ObjectMgr::SetCreatureLinkedRespawn(uint32 guid, uint32 linkedGuid)
{
    if(!guid)
        return false;

    if(!linkedGuid) // we're removing the linking
    {
        mCreatureLinkedRespawnMap.erase(guid);
        WorldDatabase.DirectPExecute("DELETE FROM `creature_linked_respawn` WHERE `guid` = '%u'",guid);
        return true;
    }

    if(CheckCreatureLinkedRespawn(guid,linkedGuid)) // we add/change linking
    {
        mCreatureLinkedRespawnMap[guid] = linkedGuid;
        WorldDatabase.DirectPExecute("REPLACE INTO `creature_linked_respawn`(`guid`,`linkedGuid`) VALUES ('%u','%u')",guid,linkedGuid);
        return true;
    }
    return false;
}

void ObjectMgr::LoadCreatures()
{
    uint32 count = 0;
    //                                                0              1   2    3
    QueryResult result = WorldDatabase.Query("SELECT creature.guid, id, map, modelid,"
    //   4             5           6           7           8            9              10         11
        "equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint,"
    //   12         13           14            15       16      17                18                         19                                                 
        "curhealth, curmana, MovementType, spawnMask, event, pool_id, creature_scripts.scriptname, COALESCE(creature_encounter_respawn.eventid, -1) "
        "FROM creature LEFT OUTER JOIN game_event_creature ON creature.guid = game_event_creature.guid "
        "LEFT OUTER JOIN creature_scripts ON creature.guid = -creature_scripts.entryorguid "
        "LEFT OUTER JOIN creature_encounter_respawn ON creature.guid = creature_encounter_respawn.guid "
        );

    if(!result)
    {
        TC_LOG_ERROR("server.loading",">> Loaded 0 creature. DB table `creature` is empty.");
        return;
    }

    // build single time for check creature data
    std::set<uint32> heroicCreatures;
    CreatureTemplateContainer const* ctc = sObjectMgr->GetCreatureTemplateStore();
    for (CreatureTemplateContainer::const_iterator itr = ctc->begin(); itr != ctc->end(); ++itr)
        if(CreatureTemplate const* cInfo = &itr->second)
            if(cInfo->difficulty_entry_1)
                heroicCreatures.insert(cInfo->difficulty_entry_1);

    do
    {
        Field *fields = result->Fetch();

        uint32 guid = fields[0].GetUInt32();

        CreatureData& data = mCreatureDataMap[guid];

        data.id             = fields[ 1].GetUInt32();
        data.mapid          = fields[ 2].GetUInt32();
        data.displayid      = fields[ 3].GetUInt32();
        data.equipmentId    = fields[ 4].GetUInt32();
        data.posX           = fields[ 5].GetFloat();
        data.posY           = fields[ 6].GetFloat();
        data.posZ           = fields[ 7].GetFloat();
        data.orientation    = fields[ 8].GetFloat();
        data.spawntimesecs  = fields[ 9].GetUInt32();
        data.spawndist      = fields[10].GetFloat();
        data.currentwaypoint= fields[11].GetUInt32();
        data.curhealth      = fields[12].GetUInt32();
        data.curmana        = fields[13].GetUInt32();
        data.movementType   = fields[14].GetUInt8();
        data.spawnMask      = fields[15].GetUInt8();
        int16 gameEvent     = fields[16].GetInt16();
        data.poolId         = fields[17].GetUInt32();

        std::string scriptname = fields[18].GetString();
        data.scriptName = scriptname;
        data.instanceEventId = fields[19].GetUInt32();

        CreatureTemplate const* cInfo = GetCreatureTemplate(data.id);
        if(!cInfo)
        {
            TC_LOG_ERROR("sql.sql","Table `creature` has creature (GUID: %u) with not existed creature entry %u, skipped.",guid,data.id );
            continue;
        }

        if(heroicCreatures.find(data.id)!=heroicCreatures.end())
        {
            TC_LOG_ERROR("sql.sql","Table `creature` has creature (GUID: %u) that listed as heroic template in `creature_template_substitution`, skipped.",guid );
            continue;
        }

        // -1 random, 0 no equipment,
        if (data.equipmentId != 0)
        {
            if(!GetEquipmentInfo(data.equipmentId))
            {
                TC_LOG_ERROR("sql.sql","Table `creature` have creature (Entry: %u) with equipment_id %u not found in table `creature_equip_template`, set to no equipment.", data.id, data.equipmentId);
                data.equipmentId = -1;
            }
        }

        if(data.spawndist < 0.0f)
        {
            TC_LOG_ERROR("sql.sql","Table `creature` have creature (GUID: %u Entry: %u) with `spawndist`< 0, set to 0.",guid,data.id );
            data.spawndist = 0.0f;
        }
        else if(data.movementType == RANDOM_MOTION_TYPE)
        {
            if(data.spawndist == 0.0f)
            {
                TC_LOG_ERROR("sql.sql","Table `creature` have creature (GUID: %u Entry: %u) with `MovementType`=1 (random movement) but with `spawndist`=0, replace by idle movement type (0).",guid,data.id );
                data.movementType = IDLE_MOTION_TYPE;
            }
        }
        else if(data.movementType == IDLE_MOTION_TYPE)
        {
            if(data.spawndist != 0.0f)
            {
                TC_LOG_ERROR("sql.sql","Table `creature` have creature (GUID: %u Entry: %u) with `MovementType`=0 (idle) have `spawndist`<>0, set to 0.",guid,data.id );
                data.spawndist = 0.0f;
            }
        }

        if (gameEvent==0)                                   // if not this is to be managed by GameEvent System or transports themselves
            AddCreatureToGrid(guid, &data);

        ++count;

    } while (result->NextRow());

    DeleteCreatureData(0);

    TC_LOG_INFO("server.loading", ">> Loaded " UI64FMTD " creatures", mCreatureDataMap.size());
}

void ObjectMgr::AddCreatureToGrid(uint32 guid, CreatureData const* data)
{
    uint8 mask = data->spawnMask;
    for(uint8 i = 0; mask != 0; i++, mask >>= 1)
    {
        if(mask & 1)
        {
            CellPair cell_pair = Trinity::ComputeCellPair(data->posX, data->posY);
            uint32 cell_id = (cell_pair.y_coord*TOTAL_NUMBER_OF_CELLS_PER_MAP) + cell_pair.x_coord;

            CellObjectGuids& cell_guids = mMapObjectGuids[MAKE_PAIR32(data->mapid,i)][cell_id];
            cell_guids.creatures.insert(guid);
        }
    }
}

void ObjectMgr::RemoveCreatureFromGrid(uint32 guid, CreatureData const* data)
{
    uint8 mask = data->spawnMask;
    for(uint8 i = 0; mask != 0; i++, mask >>= 1)
    {
        if(mask & 1)
        {
            CellPair cell_pair = Trinity::ComputeCellPair(data->posX, data->posY);
            uint32 cell_id = (cell_pair.y_coord*TOTAL_NUMBER_OF_CELLS_PER_MAP) + cell_pair.x_coord;

            CellObjectGuids& cell_guids = mMapObjectGuids[MAKE_PAIR32(data->mapid,i)][cell_id];
            cell_guids.creatures.erase(guid);
        }
    }
}

void ObjectMgr::LoadGameobjects()
{
    uint32 count = 0;

    //                                                0                1   2    3           4           5           6
    QueryResult result = WorldDatabase.Query("SELECT gameobject.guid, id, map, position_x, position_y, position_z, orientation,"
    //   7          8          9          10         11             12            13     14         15
        "rotation0, rotation1, rotation2, rotation3, spawntimesecs, animprogress, state, spawnMask, event "
        "FROM gameobject LEFT OUTER JOIN game_event_gameobject ON gameobject.guid = game_event_gameobject.guid");

    if(!result)
    {
        TC_LOG_ERROR("server.loading",">> Loaded 0 gameobjects. DB table `gameobject` is empty.");
        return;
    }

    do
    {
        Field *fields = result->Fetch();

        uint32 guid = fields[0].GetUInt32();

        GameObjectData& data = mGameObjectDataMap[guid];

        data.id             = fields[ 1].GetUInt32();
        data.mapid          = fields[ 2].GetUInt32();
        data.posX           = fields[ 3].GetFloat();
        data.posY           = fields[ 4].GetFloat();
        data.posZ           = fields[ 5].GetFloat();
        data.orientation    = fields[ 6].GetFloat();
        data.rotation0      = fields[ 7].GetFloat();
        data.rotation1      = fields[ 8].GetFloat();
        data.rotation2      = fields[ 9].GetFloat();
        data.rotation3      = fields[10].GetFloat();
        data.spawntimesecs  = fields[11].GetInt32();
        data.animprogress   = fields[12].GetUInt32();
        data.go_state       = fields[13].GetUInt32();
        data.ArtKit         = 0;
        data.spawnMask      = fields[14].GetUInt8();
        int16 gameEvent     = fields[15].GetInt16();

        GameObjectTemplate const* gInfo = GetGameObjectTemplate(data.id);
        if(!gInfo)
        {
            TC_LOG_ERROR("sql.sql","Table `gameobject` have gameobject (GUID: %u) with not existed gameobject entry %u, skipped.",guid,data.id );
            continue;
        }

        if (gameEvent==0)                                   // if not this is to be managed by GameEvent System
            AddGameobjectToGrid(guid, &data);

        ++count;

    } while (result->NextRow());

    TC_LOG_INFO("server.loading", ">> Loaded " UI64FMTD " gameobjects", mGameObjectDataMap.size());
}

void ObjectMgr::AddGameobjectToGrid(uint32 guid, GameObjectData const* data)
{
    assert(data);

    uint8 mask = data->spawnMask;
    for(uint8 i = 0; mask != 0; i++, mask >>= 1)
    {
        if(mask & 1)
        {
            CellPair cell_pair = Trinity::ComputeCellPair(data->posX, data->posY);
            uint32 cell_id = (cell_pair.y_coord*TOTAL_NUMBER_OF_CELLS_PER_MAP) + cell_pair.x_coord;

            CellObjectGuids& cell_guids = mMapObjectGuids[MAKE_PAIR32(data->mapid,i)][cell_id];
            cell_guids.gameobjects.insert(guid);
        }
    }
}

void ObjectMgr::RemoveGameobjectFromGrid(uint32 guid, GameObjectData const* data)
{
    uint8 mask = data->spawnMask;
    for(uint8 i = 0; mask != 0; i++, mask >>= 1)
    {
        if(mask & 1)
        {
            CellPair cell_pair = Trinity::ComputeCellPair(data->posX, data->posY);
            uint32 cell_id = (cell_pair.y_coord*TOTAL_NUMBER_OF_CELLS_PER_MAP) + cell_pair.x_coord;

            CellObjectGuids& cell_guids = mMapObjectGuids[MAKE_PAIR32(data->mapid,i)][cell_id];
            cell_guids.gameobjects.erase(guid);
        }
    }
}

void ObjectMgr::LoadCreatureRespawnTimes()
{
    uint32 count = 0;

    QueryResult result = WorldDatabase.Query("SELECT guid,respawntime,instance FROM creature_respawn");

    if(!result)
    {
        TC_LOG_INFO("sql.sql",">> Loaded 0 creature respawn time.");
        return;
    }

    do
    {
        Field *fields = result->Fetch();

        uint32 loguid       = fields[0].GetUInt32();
        uint64 respawn_time = fields[1].GetUInt64();
        uint32 instance     = fields[2].GetUInt32();

        mCreatureRespawnTimes[MAKE_PAIR64(loguid,instance)] = time_t(respawn_time);

        ++count;
    } while (result->NextRow());

    TC_LOG_INFO("server.loading", ">> Loaded " UI64FMTD " creature respawn times", mCreatureRespawnTimes.size());
}

void ObjectMgr::LoadGameobjectRespawnTimes()
{
    // remove outdated data
    WorldDatabase.DirectExecute("DELETE FROM gameobject_respawn WHERE respawntime <= UNIX_TIMESTAMP(NOW())");

    uint32 count = 0;

    QueryResult result = WorldDatabase.Query("SELECT guid,respawntime,instance FROM gameobject_respawn");

    if(!result)
    {
        TC_LOG_INFO("sql.sql",">> Loaded 0 gameobject respawn time.");
        TC_LOG_INFO("sql.sql"," ");
        return;
    }

    do
    {
        Field *fields = result->Fetch();

        uint32 loguid       = fields[0].GetUInt32();
        uint64 respawn_time = fields[1].GetUInt64();
        uint32 instance     = fields[2].GetUInt32();

        mGORespawnTimes[MAKE_PAIR64(loguid,instance)] = time_t(respawn_time);

        ++count;
    } while (result->NextRow());

    TC_LOG_INFO("server.loading", ">> Loaded " UI64FMTD " gameobject respawn times", mGORespawnTimes.size());
}

// name must be checked to correctness (if received) before call this function
uint64 ObjectMgr::GetPlayerGUIDByName(std::string name) const
{
    uint64 guid = 0;

    CharacterDatabase.EscapeString(name);

    // Player name safe to sending to DB (checked at login) and this function using
    QueryResult result = CharacterDatabase.PQuery("SELECT guid FROM characters WHERE name = '%s'", name.c_str());
    if(result)
    {
        guid = MAKE_NEW_GUID((*result)[0].GetUInt32(), 0, HIGHGUID_PLAYER);
    }

    return guid;
}

uint32 ObjectMgr::GetPlayerLowGUIDByName(std::string name) const
{
    uint32 guid = 0;
    
    CharacterDatabase.EscapeString(name);
    
    QueryResult result = CharacterDatabase.PQuery("SELECT guid FROM characters WHERE name = '%s'", name.c_str());
    if (result) {
        Field* fields = result->Fetch();
        guid = fields[0].GetUInt32();
    }
    
    return guid;
}

bool ObjectMgr::GetPlayerNameByGUID(const uint64 &guid, std::string &name) const
{
    // prevent DB access for online player
    if(Player* player = GetPlayer(guid))
    {
        name = player->GetName();
        return true;
    }

    QueryResult result = CharacterDatabase.PQuery("SELECT name FROM characters WHERE guid = '%u'", GUID_LOPART(guid));

    if(result)
    {
        name = (*result)[0].GetString();
        return true;
    }

    return false;
}

bool ObjectMgr::GetPlayerNameByLowGUID(uint32 guid, std::string &name) const
{
    // TODO: Add support to get Player* by low guid
    QueryResult result = CharacterDatabase.PQuery("SELECT name FROM characters WHERE guid = '%u'", guid);
    
    if (result) {
        Field* fields = result->Fetch();
        name = fields[0].GetString();
        
        return true;        
    }
    
    return false;
}

uint32 ObjectMgr::GetPlayerTeamByGUID(const uint64 &guid) const
{
    QueryResult result = CharacterDatabase.PQuery("SELECT race FROM characters WHERE guid = '%u'", GUID_LOPART(guid));

    if(result)
    {
        uint8 race = (*result)[0].GetUInt8();
        return Player::TeamForRace(race);
    }

    return 0;
}

uint32 ObjectMgr::GetPlayerAccountIdByGUID(const uint64 &guid) const
{
    QueryResult result = CharacterDatabase.PQuery("SELECT account FROM characters WHERE guid = '%u'", GUID_LOPART(guid));
    if(result)
    {
        uint32 acc = (*result)[0].GetUInt32();
        return acc;
    }

    return 0;
}

uint32 ObjectMgr::GetPlayerAccountIdByPlayerName(const std::string& name) const
{
    QueryResult result = CharacterDatabase.PQuery("SELECT account FROM characters WHERE name = '%s'", name.c_str());
    if(result)
    {
        uint32 acc = (*result)[0].GetUInt32();
        return acc;
    }

    return 0;
}

void ObjectMgr::LoadItemLocales()
{
    mItemLocaleMap.clear();                                 // need for reload case

    QueryResult result = WorldDatabase.Query("SELECT entry,name_loc1,description_loc1,name_loc2,description_loc2,name_loc3,description_loc3,name_loc4,description_loc4,name_loc5,description_loc5,name_loc6,description_loc6,name_loc7,description_loc7,name_loc8,description_loc8 FROM locales_item");

    if(!result)
    {
        TC_LOG_INFO("server.loading",">> Loaded 0 Item locale strings. DB table `locales_item` is empty.");
        TC_LOG_INFO("server.loading", " ");
        return;
    }
    do
    {
        Field *fields = result->Fetch();

        uint32 entry = fields[0].GetUInt32();

        ItemLocale& data = mItemLocaleMap[entry];

        for (uint8 i = TOTAL_LOCALES - 1; i > 0; --i)
        {
            LocaleConstant locale = (LocaleConstant) i;
            AddLocaleString(fields[1 + 2 * (i - 1)].GetString(), locale, data.Name);
            AddLocaleString(fields[1 + 2 * (i - 1) + 1].GetString(), locale, data.Description);
        }
    } while (result->NextRow());

    TC_LOG_INFO("server.loading", ">> Loaded " UI64FMTD " Item locale strings", mItemLocaleMap.size());
}
void ObjectMgr::LoadItemTemplates()
{
    uint32 oldMSTime = GetMSTime();

    //                                                 0      1       2               3              4        5        6       7       8            9        10        11
    QueryResult result = WorldDatabase.Query("SELECT entry, class, subclass, SoundOverrideSubclass, name, displayid, Quality, Flags, BuyCount, BuyPrice, SellPrice, InventoryType, "
    //                                              12                                                                                                        19
                                             "AllowableClass, AllowableRace, ItemLevel, RequiredLevel, RequiredSkill, RequiredSkillRank, requiredspell, requiredhonorrank, "
    //                                              20
                                             "RequiredCityRank, RequiredReputationFaction, RequiredReputationRank, maxcount, stackable, ContainerSlots, stat_type1, "
    //                                             27
                                             "stat_value1, stat_type2, stat_value2, stat_type3, stat_value3, stat_type4, stat_value4, stat_type5, stat_value5, stat_type6, "
    //                                          37
                                             "stat_value6, stat_type7, stat_value7, stat_type8, stat_value8, stat_type9, stat_value9, stat_type10, stat_value10, "
    //                                           46
                                             "dmg_min1, dmg_max1, dmg_type1, dmg_min2, dmg_max2, dmg_type2,  dmg_min3, dmg_max3, dmg_type3, dmg_min4, dmg_max4, dmg_type4, dmg_min5, dmg_max5, dmg_type5,"
    //                                          61
                                             "armor, holy_res, fire_res, "
    //                                            64
                                             "nature_res, frost_res, shadow_res, arcane_res, delay, ammo_type, RangedModRange, spellid_1, spelltrigger_1, spellcharges_1, "
    //                                             74
                                             "spellppmRate_1, spellcooldown_1, spellcategory_1, spellcategorycooldown_1, spellid_2, spelltrigger_2, spellcharges_2, "
    //                                             81
                                             "spellppmRate_2, spellcooldown_2, spellcategory_2, spellcategorycooldown_2, spellid_3, spelltrigger_3, spellcharges_3, "
    //                                             88
                                             "spellppmRate_3, spellcooldown_3, spellcategory_3, spellcategorycooldown_3, spellid_4, spelltrigger_4, spellcharges_4, "
    //                                              95
                                             "spellppmRate_4, spellcooldown_4, spellcategory_4, spellcategorycooldown_4, spellid_5, spelltrigger_5, spellcharges_5, "
    //                                             102
                                             "spellppmRate_5, spellcooldown_5, spellcategory_5, spellcategorycooldown_5, bonding, description, PageText, LanguageID, PageMaterial, "
    //                                            111
                                             "startquest, lockid, Material, sheath, RandomProperty, RandomSuffix, block, itemset, MaxDurability, area, Map, BagFamily, "
    //                                            123                          125                             127                               129               130
                                             "TotemCategory, socketColor_1, socketContent_1, socketColor_2, socketContent_2, socketColor_3, socketContent_3, socketBonus, "
    //                                            131                   132                 133                134        135
                                             "GemProperties, RequiredDisenchantSkill, ArmorDamageModifier, ScriptName, DisenchantID, "
    //                                           136         137         138         139 
                                             "FoodType, minMoneyLoot, maxMoneyLoot, Duration FROM item_template");

    if (!result)
    {
        TC_LOG_INFO("server.loading", ">> Loaded 0 item templates. DB table `item_template` is empty.");
        return;
    }

    _itemTemplateStore.rehash(result->GetRowCount());
    uint32 count = 0;
    bool enforceDBCAttributes = false; //sWorld->getBoolConfig(CONFIG_DBC_ENFORCE_ITEM_ATTRIBUTES);

    do
    {
        Field* fields = result->Fetch();

        uint32 entry = fields[0].GetUInt32();

        ItemTemplate& itemTemplate = _itemTemplateStore[entry];

        itemTemplate.ItemId                    = entry;
        itemTemplate.Class                     = uint32(fields[1].GetUInt8());
        itemTemplate.SubClass                  = uint32(fields[2].GetUInt8());
        itemTemplate.SoundOverrideSubclass     = int32(fields[3].GetInt8());
        itemTemplate.Name1                     = fields[4].GetString();
        itemTemplate.DisplayInfoID             = fields[5].GetUInt32();
        itemTemplate.Quality                   = uint32(fields[6].GetUInt8());
        itemTemplate.Flags                     = fields[7].GetUInt32();
        itemTemplate.BuyCount                  = uint32(fields[8].GetUInt8());
        itemTemplate.BuyPrice                  = int32(fields[9].GetInt64());
        itemTemplate.SellPrice                 = fields[10].GetUInt32();
        itemTemplate.InventoryType             = uint32(fields[11].GetUInt8());
        itemTemplate.AllowableClass            = fields[12].GetUInt32();
        itemTemplate.AllowableRace             = fields[13].GetUInt32();
        itemTemplate.ItemLevel                 = uint32(fields[14].GetUInt16());
        itemTemplate.RequiredLevel             = uint32(fields[15].GetUInt8());
        itemTemplate.RequiredSkill             = uint32(fields[16].GetUInt16());
        itemTemplate.RequiredSkillRank         = uint32(fields[17].GetUInt16());
        itemTemplate.RequiredSpell             = fields[18].GetUInt32();
        itemTemplate.RequiredHonorRank         = fields[19].GetUInt32();
        itemTemplate.RequiredCityRank          = fields[20].GetUInt32();
        itemTemplate.RequiredReputationFaction = uint32(fields[21].GetUInt16());
        itemTemplate.RequiredReputationRank    = uint32(fields[22].GetUInt16());
        itemTemplate.MaxCount                  = fields[23].GetUInt32();
        itemTemplate.Stackable                 = fields[24].GetUInt32();
        itemTemplate.ContainerSlots            = uint32(fields[25].GetUInt8());
#ifdef LICH_KING
        for (uint8 i = 0; i < itemTemplate.StatsCount; ++i)
        {
            itemTemplate.ItemStat[i].ItemStatType  = uint32(fields[26 + i*2].GetUInt8());
            itemTemplate.ItemStat[i].ItemStatValue = int32(fields[27 + i*2].GetInt16());
        }
        itemTemplate.ScalingStatDistribution = uint32(fields[48].GetUInt16());
        itemTemplate.ScalingStatValue        = fields[49].GetInt32();
#else
        for (uint8 i = 0; i < 10; ++i)
        {
            itemTemplate.ItemStat[i].ItemStatType  = uint32(fields[26 + i*2].GetUInt8());
            itemTemplate.ItemStat[i].ItemStatValue = int32(fields[27 + i*2].GetInt16());
        }
#endif

        for (uint8 i = 0; i < MAX_ITEM_PROTO_DAMAGES; ++i)
        {
            itemTemplate.Damage[i].DamageMin  = fields[46 + i*3].GetFloat();
            itemTemplate.Damage[i].DamageMax  = fields[47 + i*3].GetFloat();
            itemTemplate.Damage[i].DamageType = uint32(fields[48 + i*3].GetUInt8());
        }

        itemTemplate.Armor          = uint32(fields[61].GetUInt16());
        itemTemplate.HolyRes        = uint32(fields[62].GetUInt8());
        itemTemplate.FireRes        = uint32(fields[63].GetUInt8());
        itemTemplate.NatureRes      = uint32(fields[64].GetUInt8());
        itemTemplate.FrostRes       = uint32(fields[65].GetUInt8());
        itemTemplate.ShadowRes      = uint32(fields[66].GetUInt8());
        itemTemplate.ArcaneRes      = uint32(fields[67].GetUInt8());
        itemTemplate.Delay          = uint32(fields[68].GetUInt16());
        itemTemplate.AmmoType       = uint32(fields[69].GetUInt8());
        itemTemplate.RangedModRange = fields[70].GetFloat();

        for (uint8 i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
        {
            itemTemplate.Spells[i].SpellId               = fields[71 + i*7  ].GetInt32();
            itemTemplate.Spells[i].SpellTrigger          = uint32(fields[72 + i*7].GetUInt8());
            itemTemplate.Spells[i].SpellCharges          = int32(fields[73 + i*7].GetInt16());
            itemTemplate.Spells[i].SpellPPMRate          = fields[74 + i*7].GetFloat();
            itemTemplate.Spells[i].SpellCooldown         = fields[75 + i*7].GetInt32();
            itemTemplate.Spells[i].SpellCategory         = uint32(fields[76 + i*7].GetUInt16());
            itemTemplate.Spells[i].SpellCategoryCooldown = fields[77 + i*7].GetInt32();
        }

        itemTemplate.Bonding        = uint32(fields[106].GetUInt8());
        itemTemplate.Description    = fields[107].GetString();
        itemTemplate.PageText       = fields[108].GetUInt32();
        itemTemplate.LanguageID     = uint32(fields[109].GetUInt8());
        itemTemplate.PageMaterial   = uint32(fields[110].GetUInt8());
        itemTemplate.StartQuest     = fields[111].GetUInt32();
        itemTemplate.LockID         = fields[112].GetUInt32();
        itemTemplate.Material       = int32(fields[113].GetInt8());
        itemTemplate.Sheath         = uint32(fields[114].GetUInt8());
        itemTemplate.RandomProperty = fields[115].GetUInt32();
        itemTemplate.RandomSuffix   = fields[116].GetInt32();
        itemTemplate.Block          = fields[117].GetUInt32();
        itemTemplate.ItemSet        = fields[118].GetUInt32();
        itemTemplate.MaxDurability  = uint32(fields[119].GetUInt16());
        itemTemplate.Area           = fields[120].GetUInt32();
        itemTemplate.Map            = uint32(fields[121].GetUInt16());
        itemTemplate.BagFamily      = fields[123].GetUInt32();
        itemTemplate.TotemCategory  = fields[123].GetUInt32();

        for (uint8 i = 0; i < MAX_ITEM_PROTO_SOCKETS; ++i)
        {
            itemTemplate.Socket[i].Color   = uint32(fields[124 + i*2].GetUInt8());
            itemTemplate.Socket[i].Content = fields[125 + i*2].GetUInt32();
        }

        itemTemplate.socketBonus             = fields[130].GetUInt32();
        itemTemplate.GemProperties           = fields[131].GetUInt32();
        itemTemplate.RequiredDisenchantSkill = uint32(fields[132].GetInt16());
        itemTemplate.ArmorDamageModifier     = fields[133].GetFloat();
        itemTemplate.ScriptId                = sObjectMgr->GetScriptId(fields[134].GetCString());
        itemTemplate.DisenchantID            = fields[135].GetUInt32();
        itemTemplate.FoodType                = uint32(fields[136].GetUInt8());
        itemTemplate.MinMoneyLoot            = fields[137].GetUInt32();
        itemTemplate.MaxMoneyLoot            = fields[138].GetUInt32();
        itemTemplate.Duration                = fields[139].GetUInt32();

        // Checks

        ItemEntry const* dbcitem = sItemStore.LookupEntry(entry);

        if (dbcitem)
        {
#ifdef LICH_KING
            if (itemTemplate.Class != dbcitem->Class)
            {
                TC_LOG_ERROR("sql.sql", "Item (Entry: %u) does not have a correct class %u, must be %u .", entry, itemTemplate.Class, dbcitem->Class);
                if (enforceDBCAttributes)
                    itemTemplate.Class = dbcitem->Class;
            }

            if (itemTemplate.SoundOverrideSubclass != dbcitem->SoundOverrideSubclass)
            {
                TC_LOG_ERROR("sql.sql", "Item (Entry: %u) does not have a correct SoundOverrideSubclass (%i), must be %i .", entry, itemTemplate.SoundOverrideSubclass, dbcitem->SoundOverrideSubclass);
                if (enforceDBCAttributes)
                    itemTemplate.SoundOverrideSubclass = dbcitem->SoundOverrideSubclass;
            }
            if (itemTemplate.Material != dbcitem->Material)
            {
                TC_LOG_ERROR("sql.sql", "Item (Entry: %u) does not have a correct material (%i), must be %i .", entry, itemTemplate.Material, dbcitem->Material);
                if (enforceDBCAttributes)
                    itemTemplate.Material = dbcitem->Material;
            }
#endif
            if (itemTemplate.InventoryType != dbcitem->InventoryType)
            {
                TC_LOG_ERROR("sql.sql", "Item (Entry: %u) does not have a correct inventory type (%u), must be %u .", entry, itemTemplate.InventoryType, dbcitem->InventoryType);
                if (enforceDBCAttributes)
                    itemTemplate.InventoryType = dbcitem->InventoryType;
            }
            if (itemTemplate.DisplayInfoID != dbcitem->DisplayId)
            {
                TC_LOG_ERROR("sql.sql", "Item (Entry: %u) does not have a correct display id (%u), must be %u .", entry, itemTemplate.DisplayInfoID, dbcitem->DisplayId);
                if (enforceDBCAttributes)
                    itemTemplate.DisplayInfoID = dbcitem->DisplayId;
            }
            if (itemTemplate.Sheath != dbcitem->Sheath)
            {
                TC_LOG_ERROR("sql.sql", "Item (Entry: %u) does not have a correct sheathid (%u), must be %u .", entry, itemTemplate.Sheath, dbcitem->Sheath);
                if (enforceDBCAttributes)
                    itemTemplate.Sheath = dbcitem->Sheath;
            }

        }
        else
            TC_LOG_ERROR("sql.sql", "Item (Entry: %u) does not exist in item.dbc! (not correct id?).", entry);

        if (itemTemplate.Class >= MAX_ITEM_CLASS)
        {
            TC_LOG_ERROR("sql.sql", "Item (Entry: %u) has wrong Class value (%u)", entry, itemTemplate.Class);
            itemTemplate.Class = ITEM_CLASS_MISC;
        }

        if (itemTemplate.SubClass >= MaxItemSubclassValues[itemTemplate.Class])
        {
            TC_LOG_ERROR("sql.sql", "Item (Entry: %u) has wrong Subclass value (%u) for class %u", entry, itemTemplate.SubClass, itemTemplate.Class);
            itemTemplate.SubClass = 0;// exist for all item classes
        }

        if (itemTemplate.Quality >= MAX_ITEM_QUALITY)
        {
            TC_LOG_ERROR("sql.sql", "Item (Entry: %u) has wrong Quality value (%u)", entry, itemTemplate.Quality);
            itemTemplate.Quality = ITEM_QUALITY_NORMAL;
        }

#ifdef LICH_KING
        if (itemTemplate.Flags2 & ITEM_FLAGS_EXTRA_HORDE_ONLY)
        {
            if (FactionEntry const* faction = sFactionStore.LookupEntry(TEAM_HORDE))
                if ((itemTemplate.AllowableRace & faction->BaseRepRaceMask[0]) == 0)
                    TC_LOG_ERROR("sql.sql", "Item (Entry: %u) has value (%u) in `AllowableRace` races, not compatible with ITEM_FLAGS_EXTRA_HORDE_ONLY (%u) in Flags field, item cannot be equipped or used by these races.",
                        entry, itemTemplate.AllowableRace, ITEM_FLAGS_EXTRA_HORDE_ONLY);

            if (itemTemplate.Flags2 & ITEM_FLAGS_EXTRA_ALLIANCE_ONLY)
                TC_LOG_ERROR("sql.sql", "Item (Entry: %u) has value (%u) in `Flags2` flags (ITEM_FLAGS_EXTRA_ALLIANCE_ONLY) and ITEM_FLAGS_EXTRA_HORDE_ONLY (%u) in Flags field, this is a wrong combination.",
                    entry, ITEM_FLAGS_EXTRA_ALLIANCE_ONLY, ITEM_FLAGS_EXTRA_HORDE_ONLY);
        }
        else if (itemTemplate.Flags2 & ITEM_FLAGS_EXTRA_ALLIANCE_ONLY)
        {
            if (FactionEntry const* faction = sFactionStore.LookupEntry(TEAM_ALLIANCE))
                if ((itemTemplate.AllowableRace & faction->BaseRepRaceMask[0]) == 0)
                    TC_LOG_ERROR("sql.sql", "Item (Entry: %u) has value (%u) in `AllowableRace` races, not compatible with ITEM_FLAGS_EXTRA_ALLIANCE_ONLY (%u) in Flags field, item cannot be equipped or used by these races.",
                        entry, itemTemplate.AllowableRace, ITEM_FLAGS_EXTRA_ALLIANCE_ONLY);
        }
#endif

        if (itemTemplate.BuyCount <= 0)
        {
            TC_LOG_ERROR("sql.sql", "Item (Entry: %u) has wrong BuyCount value (%u), set to default(1).", entry, itemTemplate.BuyCount);
            itemTemplate.BuyCount = 1;
        }

        if (itemTemplate.InventoryType >= MAX_INVTYPE)
        {
            TC_LOG_ERROR("sql.sql", "Item (Entry: %u) has wrong InventoryType value (%u)", entry, itemTemplate.InventoryType);
            itemTemplate.InventoryType = INVTYPE_NON_EQUIP;
        }

        if (itemTemplate.RequiredSkill >= MAX_SKILL_TYPE)
        {
            TC_LOG_ERROR("sql.sql", "Item (Entry: %u) has wrong RequiredSkill value (%u)", entry, itemTemplate.RequiredSkill);
            itemTemplate.RequiredSkill = 0;
        }

        {
            // can be used in equip slot, as page read use in inventory, or spell casting at use
            bool req = itemTemplate.InventoryType != INVTYPE_NON_EQUIP || itemTemplate.PageText;
            if (!req)
                for (uint8 j = 0; j < MAX_ITEM_PROTO_SPELLS; ++j)
                {
                    if (itemTemplate.Spells[j].SpellId)
                    {
                        req = true;
                        break;
                    }
                }

            if (req)
            {
                if (!(itemTemplate.AllowableClass & CLASSMASK_ALL_PLAYABLE))
                    TC_LOG_ERROR("sql.sql", "Item (Entry: %u) does not have any playable classes (%u) in `AllowableClass` and can't be equipped or used.", entry, itemTemplate.AllowableClass);

                if (!(itemTemplate.AllowableRace & RACEMASK_ALL_PLAYABLE))
                    TC_LOG_ERROR("sql.sql", "Item (Entry: %u) does not have any playable races (%u) in `AllowableRace` and can't be equipped or used.", entry, itemTemplate.AllowableRace);
            }
        }

        if (itemTemplate.RequiredSpell && !sSpellMgr->GetSpellInfo(itemTemplate.RequiredSpell))
        {
            TC_LOG_ERROR("sql.sql", "Item (Entry: %u) has a wrong (non-existing) spell in RequiredSpell (%u)", entry, itemTemplate.RequiredSpell);
            itemTemplate.RequiredSpell = 0;
        }

        if (itemTemplate.RequiredReputationRank >= MAX_REPUTATION_RANK)
            TC_LOG_ERROR("sql.sql", "Item (Entry: %u) has wrong reputation rank in RequiredReputationRank (%u), item can't be used.", entry, itemTemplate.RequiredReputationRank);

        if (itemTemplate.RequiredReputationFaction)
        {
            if (!sFactionStore.LookupEntry(itemTemplate.RequiredReputationFaction))
            {
                TC_LOG_ERROR("sql.sql", "Item (Entry: %u) has wrong (not existing) faction in RequiredReputationFaction (%u)", entry, itemTemplate.RequiredReputationFaction);
                itemTemplate.RequiredReputationFaction = 0;
            }

            if (itemTemplate.RequiredReputationRank == MIN_REPUTATION_RANK)
                TC_LOG_ERROR("sql.sql", "Item (Entry: %u) has min. reputation rank in RequiredReputationRank (0) but RequiredReputationFaction > 0, faction setting is useless.", entry);
        }

        if (itemTemplate.ContainerSlots > MAX_BAG_SIZE)
        {
            TC_LOG_ERROR("sql.sql", "Item (Entry: %u) has too large value in ContainerSlots (%u), replace by hardcoded limit (%u).", entry, itemTemplate.ContainerSlots, MAX_BAG_SIZE);
            itemTemplate.ContainerSlots = MAX_BAG_SIZE;
        }
#ifdef LICH_KING
        if (itemTemplate.StatsCount > MAX_ITEM_PROTO_STATS)
        {
            TC_LOG_ERROR("sql.sql", "Item (Entry: %u) has too large value in statscount (%u), replace by hardcoded limit (%u).", entry, itemTemplate.StatsCount, MAX_ITEM_PROTO_STATS);
            itemTemplate.StatsCount = MAX_ITEM_PROTO_STATS;
        }
        for (uint8 j = 0; j < itemTemplate.StatsCount; ++j)
#else
        for (uint8 j = 0; j < MAX_ITEM_PROTO_STATS; ++j)
#endif
        {
            // for ItemStatValue != 0
            if (itemTemplate.ItemStat[j].ItemStatValue && itemTemplate.ItemStat[j].ItemStatType >= MAX_ITEM_MOD)
            {
                TC_LOG_ERROR("sql.sql", "Item (Entry: %u) has wrong (non-existing?) stat_type%d (%u)", entry, j+1, itemTemplate.ItemStat[j].ItemStatType);
                itemTemplate.ItemStat[j].ItemStatType = 0;
            }

            switch (itemTemplate.ItemStat[j].ItemStatType)
            {
                case ITEM_MOD_SPELL_HEALING_DONE:
                case ITEM_MOD_SPELL_DAMAGE_DONE:
                    TC_LOG_ERROR("sql.sql", "Item (Entry: %u) has deprecated stat_type%d (%u)", entry, j+1, itemTemplate.ItemStat[j].ItemStatType);
                    break;
                default:
                    break;
            }
        }

        for (uint8 j = 0; j < MAX_ITEM_PROTO_DAMAGES; ++j)
        {
            if (itemTemplate.Damage[j].DamageType >= MAX_SPELL_SCHOOL)
            {
                TC_LOG_ERROR("sql.sql", "Item (Entry: %u) has wrong dmg_type%d (%u)", entry, j+1, itemTemplate.Damage[j].DamageType);
                itemTemplate.Damage[j].DamageType = 0;
            }
        }

        // special format
        if ((itemTemplate.Spells[0].SpellId == 483) || (itemTemplate.Spells[0].SpellId == 55884))
        {
            // spell_1
            if (itemTemplate.Spells[0].SpellTrigger != ITEM_SPELLTRIGGER_ON_USE)
            {
                TC_LOG_ERROR("sql.sql", "Item (Entry: %u) has wrong item spell trigger value in spelltrigger_%d (%u) for special learning format", entry, 0+1, itemTemplate.Spells[0].SpellTrigger);
                itemTemplate.Spells[0].SpellId = 0;
                itemTemplate.Spells[0].SpellTrigger = ITEM_SPELLTRIGGER_ON_USE;
                itemTemplate.Spells[1].SpellId = 0;
                itemTemplate.Spells[1].SpellTrigger = ITEM_SPELLTRIGGER_ON_USE;
            }

            // spell_2 have learning spell
            if (itemTemplate.Spells[1].SpellTrigger != ITEM_SPELLTRIGGER_LEARN_SPELL_ID)
            {
                TC_LOG_ERROR("sql.sql", "Item (Entry: %u) has wrong item spell trigger value in spelltrigger_%d (%u) for special learning format.", entry, 1+1, itemTemplate.Spells[1].SpellTrigger);
                itemTemplate.Spells[0].SpellId = 0;
                itemTemplate.Spells[1].SpellId = 0;
                itemTemplate.Spells[1].SpellTrigger = ITEM_SPELLTRIGGER_ON_USE;
            }
            else if (!itemTemplate.Spells[1].SpellId)
            {
                TC_LOG_ERROR("sql.sql", "Item (Entry: %u) does not have an expected spell in spellid_%d in special learning format.", entry, 1+1);
                itemTemplate.Spells[0].SpellId = 0;
                itemTemplate.Spells[1].SpellTrigger = ITEM_SPELLTRIGGER_ON_USE;
            }
            else if (itemTemplate.Spells[1].SpellId != -1)
            {
                SpellEntry const* spellInfo = sSpellMgr->GetSpellInfo(itemTemplate.Spells[1].SpellId);
                if (!spellInfo /*&& !DisableMgr::IsDisabledFor(DISABLE_TYPE_SPELL, itemTemplate.Spells[1].SpellId, NULL) */)
                {
                    TC_LOG_ERROR("sql.sql", "Item (Entry: %u) has wrong (not existing) spell in spellid_%d (%d)", entry, 1+1, itemTemplate.Spells[1].SpellId);
                    itemTemplate.Spells[0].SpellId = 0;
                    itemTemplate.Spells[1].SpellId = 0;
                    itemTemplate.Spells[1].SpellTrigger = ITEM_SPELLTRIGGER_ON_USE;
                }
                // allowed only in special format
                else if ((itemTemplate.Spells[1].SpellId == 483) || (itemTemplate.Spells[1].SpellId == 55884))
                {
                    TC_LOG_ERROR("sql.sql", "Item (Entry: %u) has broken spell in spellid_%d (%d)", entry, 1+1, itemTemplate.Spells[1].SpellId);
                    itemTemplate.Spells[0].SpellId = 0;
                    itemTemplate.Spells[1].SpellId = 0;
                    itemTemplate.Spells[1].SpellTrigger = ITEM_SPELLTRIGGER_ON_USE;
                }
            }

            // spell_3*, spell_4*, spell_5* is empty
            for (uint8 j = 2; j < MAX_ITEM_PROTO_SPELLS; ++j)
            {
                if (itemTemplate.Spells[j].SpellTrigger != ITEM_SPELLTRIGGER_ON_USE)
                {
                    TC_LOG_ERROR("sql.sql", "Item (Entry: %u) has wrong item spell trigger value in spelltrigger_%d (%u)", entry, j+1, itemTemplate.Spells[j].SpellTrigger);
                    itemTemplate.Spells[j].SpellId = 0;
                    itemTemplate.Spells[j].SpellTrigger = ITEM_SPELLTRIGGER_ON_USE;
                }
                else if (itemTemplate.Spells[j].SpellId != 0)
                {
                    TC_LOG_ERROR("sql.sql", "Item (Entry: %u) has wrong spell in spellid_%d (%d) for learning special format", entry, j+1, itemTemplate.Spells[j].SpellId);
                    itemTemplate.Spells[j].SpellId = 0;
                }
            }
        }
        // normal spell list
        else
        {
            for (uint8 j = 0; j < MAX_ITEM_PROTO_SPELLS; ++j)
            {
                if (itemTemplate.Spells[j].SpellTrigger >= MAX_ITEM_SPELLTRIGGER || itemTemplate.Spells[j].SpellTrigger == ITEM_SPELLTRIGGER_LEARN_SPELL_ID)
                {
                    TC_LOG_ERROR("sql.sql", "Item (Entry: %u) has wrong item spell trigger value in spelltrigger_%d (%u)", entry, j+1, itemTemplate.Spells[j].SpellTrigger);
                    itemTemplate.Spells[j].SpellId = 0;
                    itemTemplate.Spells[j].SpellTrigger = ITEM_SPELLTRIGGER_ON_USE;
                }

                if (itemTemplate.Spells[j].SpellId && itemTemplate.Spells[j].SpellId != -1)
                {
                    SpellEntry const* spellInfo = sSpellMgr->GetSpellInfo(itemTemplate.Spells[j].SpellId);
                    if (!spellInfo /* && !DisableMgr::IsDisabledFor(DISABLE_TYPE_SPELL, itemTemplate.Spells[j].SpellId, NULL) */)
                    {
                        TC_LOG_ERROR("sql.sql", "Item (Entry: %u) has wrong (not existing) spell in spellid_%d (%d)", entry, j+1, itemTemplate.Spells[j].SpellId);
                        itemTemplate.Spells[j].SpellId = 0;
                    }
                    // allowed only in special format
                    else if ((itemTemplate.Spells[j].SpellId == 483) || (itemTemplate.Spells[j].SpellId == 55884))
                    {
                        TC_LOG_ERROR("sql.sql", "Item (Entry: %u) has broken spell in spellid_%d (%d)", entry, j+1, itemTemplate.Spells[j].SpellId);
                        itemTemplate.Spells[j].SpellId = 0;
                    }
                }
            }
        }

        if (itemTemplate.Bonding >= MAX_BIND_TYPE)
            TC_LOG_ERROR("sql.sql", "Item (Entry: %u) has wrong Bonding value (%u)", entry, itemTemplate.Bonding);

        if (itemTemplate.PageText && !GetPageText(itemTemplate.PageText))
            TC_LOG_ERROR("sql.sql", "Item (Entry: %u) has non existing first page (Id:%u)", entry, itemTemplate.PageText);

        if (itemTemplate.LockID && !sLockStore.LookupEntry(itemTemplate.LockID))
            TC_LOG_ERROR("sql.sql", "Item (Entry: %u) has wrong LockID (%u)", entry, itemTemplate.LockID);

        if (itemTemplate.Sheath >= MAX_SHEATHETYPE)
        {
            TC_LOG_ERROR("sql.sql", "Item (Entry: %u) has wrong Sheath (%u)", entry, itemTemplate.Sheath);
            itemTemplate.Sheath = SHEATHETYPE_NONE;
        }

        if (itemTemplate.RandomProperty)
        {
            // To be implemented later
            if (itemTemplate.RandomProperty == -1)
                itemTemplate.RandomProperty = 0;

            else if (!sItemRandomPropertiesStore.LookupEntry(GetItemEnchantMod(itemTemplate.RandomProperty)))
            {
                TC_LOG_ERROR("sql.sql", "Item (Entry: %u) has unknown (wrong or not listed in `item_enchantment_template`) RandomProperty (%u)", entry, itemTemplate.RandomProperty);
                itemTemplate.RandomProperty = 0;
            }
        }

        if (itemTemplate.RandomSuffix && !sItemRandomSuffixStore.LookupEntry(GetItemEnchantMod(itemTemplate.RandomSuffix)))
        {
            TC_LOG_ERROR("sql.sql", "Item (Entry: %u) has wrong RandomSuffix (%u)", entry, itemTemplate.RandomSuffix);
            itemTemplate.RandomSuffix = 0;
        }

        if (itemTemplate.ItemSet && !sItemSetStore.LookupEntry(itemTemplate.ItemSet))
        {
            TC_LOG_ERROR("sql.sql", "Item (Entry: %u) have wrong ItemSet (%u)", entry, itemTemplate.ItemSet);
            itemTemplate.ItemSet = 0;
        }

        if (itemTemplate.Area && !GetAreaEntryByAreaID(itemTemplate.Area))
            TC_LOG_ERROR("sql.sql", "Item (Entry: %u) has wrong Area (%u)", entry, itemTemplate.Area);

        if (itemTemplate.Map && !sMapStore.LookupEntry(itemTemplate.Map))
            TC_LOG_ERROR("sql.sql", "Item (Entry: %u) has wrong Map (%u)", entry, itemTemplate.Map);

        if (itemTemplate.BagFamily)
        {
            // check bits
            /*
            for (uint32 j = 0; j < sizeof(itemTemplate.BagFamily)*8; ++j)
            {
                uint32 mask = 1 << j;
                if ((itemTemplate.BagFamily & mask) == 0)
                    continue;

                ItemBagFamilyEntry const* bf = sItemBagFamilyStore.LookupEntry(j+1);
                if (!bf)
                {
                    TC_LOG_ERROR("sql.sql", "Item (Entry: %u) has bag family bit set not listed in ItemBagFamily.dbc, remove bit", entry);
                    itemTemplate.BagFamily &= ~mask;
                    continue;
                }

                if (BAG_FAMILY_MASK_CURRENCY_TOKENS & mask)
                {
                    CurrencyTypesEntry const* ctEntry = sCurrencyTypesStore.LookupEntry(itemTemplate.ItemId);
                    if (!ctEntry)
                    {
                        TC_LOG_ERROR("sql.sql", "Item (Entry: %u) has currency bag family bit set in BagFamily but not listed in CurrencyTypes.dbc, remove bit", entry);
                        itemTemplate.BagFamily &= ~mask;
                    }
                }
            }*/
        }

        if (itemTemplate.TotemCategory && !sTotemCategoryStore.LookupEntry(itemTemplate.TotemCategory))
            TC_LOG_ERROR("sql.sql", "Item (Entry: %u) has wrong TotemCategory (%u)", entry, itemTemplate.TotemCategory);

        for (uint8 j = 0; j < MAX_ITEM_PROTO_SOCKETS; ++j)
        {
            if (itemTemplate.Socket[j].Color && (itemTemplate.Socket[j].Color & SOCKET_COLOR_ALL) != itemTemplate.Socket[j].Color)
            {
                TC_LOG_ERROR("sql.sql", "Item (Entry: %u) has wrong socketColor_%d (%u)", entry, j+1, itemTemplate.Socket[j].Color);
                itemTemplate.Socket[j].Color = 0;
            }
        }

        if (itemTemplate.GemProperties && !sGemPropertiesStore.LookupEntry(itemTemplate.GemProperties))
            TC_LOG_ERROR("sql.sql", "Item (Entry: %u) has wrong GemProperties (%u)", entry, itemTemplate.GemProperties);

        if (itemTemplate.FoodType >= MAX_PET_DIET)
        {
            TC_LOG_ERROR("sql.sql", "Item (Entry: %u) has wrong FoodType value (%u)", entry, itemTemplate.FoodType);
            itemTemplate.FoodType = 0;
        }
        /*
        if (itemTemplate.ItemLimitCategory && !sItemLimitCategoryStore.LookupEntry(itemTemplate.ItemLimitCategory))
        {
            TC_LOG_ERROR("sql.sql", "Item (Entry: %u) has wrong LimitCategory value (%u)", entry, itemTemplate.ItemLimitCategory);
            itemTemplate.ItemLimitCategory = 0;
        }

        if (itemTemplate.HolidayId && !sHolidaysStore.LookupEntry(itemTemplate.HolidayId))
        {
            TC_LOG_ERROR("sql.sql", "Item (Entry: %u) has wrong HolidayId value (%u)", entry, itemTemplate.HolidayId);
            itemTemplate.HolidayId = 0;
        }

        if (itemTemplate.FlagsCu & ITEM_FLAGS_CU_DURATION_REAL_TIME && !itemTemplate.Duration)
        {
            TC_LOG_ERROR("sql.sql", "Item (Entry %u) has flag ITEM_FLAGS_CU_DURATION_REAL_TIME but it does not have duration limit", entry);
            itemTemplate.FlagsCu &= ~ITEM_FLAGS_CU_DURATION_REAL_TIME;
        }*/

        ++count;
    }
    while (result->NextRow());

    // Check if item templates for DBC referenced character start outfit are present
    std::set<uint32> notFoundOutfit;
    for (uint32 i = 1; i < sCharStartOutfitStore.GetNumRows(); ++i)
    {
        CharStartOutfitEntry const* entry = sCharStartOutfitStore.LookupEntry(i);
        if (!entry)
            continue;

        for (int j = 0; j < MAX_OUTFIT_ITEMS; ++j)
        {
            if (entry->ItemId[j] <= 0)
                continue;

            uint32 item_id = entry->ItemId[j];

            if (!GetItemTemplate(item_id))
                notFoundOutfit.insert(item_id);
        }
    }

    for (std::set<uint32>::const_iterator itr = notFoundOutfit.begin(); itr != notFoundOutfit.end(); ++itr)
        TC_LOG_ERROR("sql.sql", "Item (Entry: %u) does not exist in `item_template` but is referenced in `CharStartOutfit.dbc`", *itr);

    TC_LOG_INFO("server.loading", ">> Loaded %u item templates in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}
/*
void ObjectMgr::LoadItemTemplates()
{
    

    // check data correctness
    ItemTemplateContainer const* its = sObjectMgr->GetItemTemplateStore();
    for (ItemTemplateContainer::const_iterator itr = its->begin(); itr != its->end(); ++itr)
    {
        ItemTemplate const* proto = itr->second;
        ItemEntry const *dbcitem = sItemStore.LookupEntry(i);
        if(!proto)
        {
            // to many errors, and possible not all items really used in game
            //if (dbcitem)
                //TC_LOG_ERROR("sql.sql","Item (Entry: %u) doesn't exists in DB, but must exist.",i);
            //
            continue;
        }

        if(dbcitem)
        {
            if(proto->InventoryType != dbcitem->InventoryType)
            {
                TC_LOG_ERROR("sql.sql","Item (Entry: %u) not correct %u inventory type, must be %u (still using DB value).",i,proto->InventoryType,dbcitem->InventoryType);
                // It safe let use InventoryType from DB
            }

            if(proto->DisplayInfoID != dbcitem->DisplayId)
            {
                TC_LOG_ERROR("sql.sql","Item (Entry: %u) not correct %u display id, must be %u (using it).",i,proto->DisplayInfoID,dbcitem->DisplayId);
                const_cast<ItemTemplate*>(proto)->DisplayInfoID = dbcitem->DisplayId;
            }
            if(proto->Sheath != dbcitem->Sheath)
            {
                TC_LOG_ERROR("sql.sql","Item (Entry: %u) not correct %u sheath, must be %u  (using it).",i,proto->Sheath,dbcitem->Sheath);
                const_cast<ItemTemplate*>(proto)->Sheath = dbcitem->Sheath;
            }
        }

        if(proto->Class >= MAX_ITEM_CLASS)
        {
            TC_LOG_ERROR("sql.sql","Item (Entry: %u) has wrong Class value (%u)",i,proto->Class);
            const_cast<ItemTemplate*>(proto)->Class = ITEM_CLASS_MISC;
        }

        if(proto->SubClass >= MaxItemSubclassValues[proto->Class])
        {
            TC_LOG_ERROR("FIXME","Item (Entry: %u) has wrong Subclass value (%u) for class %u",i,proto->SubClass,proto->Class);
            const_cast<ItemTemplate*>(proto)->SubClass = 0;// exist for all item classes
        }

        if(proto->Quality >= MAX_ITEM_QUALITY)
        {
            TC_LOG_ERROR("FIXME","Item (Entry: %u) has wrong Quality value (%u)",i,proto->Quality);
            const_cast<ItemTemplate*>(proto)->Quality = ITEM_QUALITY_NORMAL;
        }

        if(proto->BuyCount <= 0)
        {
            TC_LOG_ERROR("FIXME","Item (Entry: %u) has wrong BuyCount value (%u), set to default(1).",i,proto->BuyCount);
            const_cast<ItemTemplate*>(proto)->BuyCount = 1;
        }

        if(proto->InventoryType >= MAX_INVTYPE)
        {
            TC_LOG_ERROR("FIXME","Item (Entry: %u) has wrong InventoryType value (%u)",i,proto->InventoryType);
            const_cast<ItemTemplate*>(proto)->InventoryType = INVTYPE_NON_EQUIP;
        }

        if(proto->RequiredSkill >= MAX_SKILL_TYPE)
        {
            TC_LOG_ERROR("FIXME","Item (Entry: %u) has wrong RequiredSkill value (%u)",i,proto->RequiredSkill);
            const_cast<ItemTemplate*>(proto)->RequiredSkill = 0;
        }

        if(!(proto->AllowableClass & CLASSMASK_ALL_PLAYABLE))
        {
            TC_LOG_ERROR("FIXME","Item (Entry: %u) not have in `AllowableClass` any playable classes (%u) and can't be equipped.",i,proto->AllowableClass);
        }

        if(!(proto->AllowableRace & RACEMASK_ALL_PLAYABLE))
        {
            TC_LOG_ERROR("FIXME","Item (Entry: %u) not have in `AllowableRace` any playable races (%u) and can't be equipped.",i,proto->AllowableRace);
        }

        if(proto->RequiredSpell && !sSpellMgr->GetSpellInfo(proto->RequiredSpell))
        {
            TC_LOG_ERROR("FIXME","Item (Entry: %u) have wrong (non-existed) spell in RequiredSpell (%u)",i,proto->RequiredSpell);
            const_cast<ItemTemplate*>(proto)->RequiredSpell = 0;
        }

        if(proto->RequiredReputationRank >= MAX_REPUTATION_RANK)
            TC_LOG_ERROR("FIXME","Item (Entry: %u) has wrong reputation rank in RequiredReputationRank (%u), item can't be used.",i,proto->RequiredReputationRank);

        if(proto->RequiredReputationFaction)
        {
            if(!sFactionStore.LookupEntry(proto->RequiredReputationFaction))
            {
                TC_LOG_ERROR("FIXME","Item (Entry: %u) has wrong (not existing) faction in RequiredReputationFaction (%u)",i,proto->RequiredReputationFaction);
                const_cast<ItemTemplate*>(proto)->RequiredReputationFaction = 0;
            }

            if(proto->RequiredReputationRank == MIN_REPUTATION_RANK)
                TC_LOG_ERROR("FIXME","Item (Entry: %u) has min. reputation rank in RequiredReputationRank (0) but RequiredReputationFaction > 0, faction setting is useless.",i);
        }
        else if(proto->RequiredReputationRank > MIN_REPUTATION_RANK)
            TC_LOG_ERROR("FIXME","Item (Entry: %u) has RequiredReputationFaction ==0 but RequiredReputationRank > 0, rank setting is useless.",i);

        if(proto->Stackable==0)
        {
            TC_LOG_ERROR("FIXME","Item (Entry: %u) has wrong value in stackable (%u), replace by default 1.",i,proto->Stackable);
            const_cast<ItemTemplate*>(proto)->Stackable = 1;
        }
        else if(proto->Stackable > 255)
        {
            TC_LOG_ERROR("FIXME","Item (Entry: %u) has too large value in stackable (%u), replace by hardcoded upper limit (255).",i,proto->Stackable);
            const_cast<ItemTemplate*>(proto)->Stackable = 255;
        }

        for (int j = 0; j < 10; j++)
        {
            // for ItemStatValue != 0
            if(proto->ItemStat[j].ItemStatValue && proto->ItemStat[j].ItemStatType >= MAX_ITEM_MOD)
            {
                TC_LOG_ERROR("FIXME","Item (Entry: %u) has wrong stat_type%d (%u)",i,j+1,proto->ItemStat[j].ItemStatType);
                const_cast<ItemTemplate*>(proto)->ItemStat[j].ItemStatType = 0;
            }
        }

        for (int j = 0; j < 5; j++)
        {
            if(proto->Damage[j].DamageType >= MAX_SPELL_SCHOOL)
            {
                TC_LOG_ERROR("FIXME","Item (Entry: %u) has wrong dmg_type%d (%u)",i,j+1,proto->Damage[j].DamageType);
                const_cast<ItemTemplate*>(proto)->Damage[j].DamageType = 0;
            }
        }

        // special format
        if(proto->Spells[0].SpellId == SPELL_ID_GENERIC_LEARN)
        {
            // spell_1
            if(proto->Spells[0].SpellTrigger != ITEM_SPELLTRIGGER_ON_USE)
            {
                TC_LOG_ERROR("FIXME","Item (Entry: %u) has wrong item spell trigger value in spelltrigger_%d (%u) for special learning format",i,0+1,proto->Spells[0].SpellTrigger);
                const_cast<ItemTemplate*>(proto)->Spells[0].SpellId = 0;
                const_cast<ItemTemplate*>(proto)->Spells[0].SpellTrigger = ITEM_SPELLTRIGGER_ON_USE;
                const_cast<ItemTemplate*>(proto)->Spells[1].SpellId = 0;
                const_cast<ItemTemplate*>(proto)->Spells[1].SpellTrigger = ITEM_SPELLTRIGGER_ON_USE;
            }

            // spell_2 have learning spell
            if(proto->Spells[1].SpellTrigger != ITEM_SPELLTRIGGER_LEARN_SPELL_ID)
            {
                TC_LOG_ERROR("FIXME","Item (Entry: %u) has wrong item spell trigger value in spelltrigger_%d (%u) for special learning format.",i,1+1,proto->Spells[1].SpellTrigger);
                const_cast<ItemTemplate*>(proto)->Spells[0].SpellId = 0;
                const_cast<ItemTemplate*>(proto)->Spells[1].SpellId = 0;
                const_cast<ItemTemplate*>(proto)->Spells[1].SpellTrigger = ITEM_SPELLTRIGGER_ON_USE;
            }
            else if(!proto->Spells[1].SpellId)
            {
                TC_LOG_ERROR("FIXME","Item (Entry: %u) not has expected spell in spellid_%d in special learning format.",i,1+1);
                const_cast<ItemTemplate*>(proto)->Spells[0].SpellId = 0;
                const_cast<ItemTemplate*>(proto)->Spells[1].SpellTrigger = ITEM_SPELLTRIGGER_ON_USE;
            }
            else
            {
                SpellEntry const* spellInfo = sSpellMgr->GetSpellInfo(proto->Spells[1].SpellId);
                if(!spellInfo)
                {
                    TC_LOG_ERROR("FIXME","Item (Entry: %u) has wrong (not existing) spell in spellid_%d (%u)",i,1+1,proto->Spells[1].SpellId);
                    const_cast<ItemTemplate*>(proto)->Spells[0].SpellId = 0;
                    const_cast<ItemTemplate*>(proto)->Spells[1].SpellId = 0;
                    const_cast<ItemTemplate*>(proto)->Spells[1].SpellTrigger = ITEM_SPELLTRIGGER_ON_USE;
                }
                // allowed only in special format
                else if(proto->Spells[1].SpellId==SPELL_ID_GENERIC_LEARN)
                {
                    TC_LOG_ERROR("FIXME","Item (Entry: %u) has broken spell in spellid_%d (%u)",i,1+1,proto->Spells[1].SpellId);
                    const_cast<ItemTemplate*>(proto)->Spells[0].SpellId = 0;
                    const_cast<ItemTemplate*>(proto)->Spells[1].SpellId = 0;
                    const_cast<ItemTemplate*>(proto)->Spells[1].SpellTrigger = ITEM_SPELLTRIGGER_ON_USE;
                }
            }

            // spell_3*,spell_4*,spell_5* is empty
            for (int j = 2; j < 5; j++)
            {
                if(proto->Spells[j].SpellTrigger != ITEM_SPELLTRIGGER_ON_USE)
                {
                    TC_LOG_ERROR("FIXME","Item (Entry: %u) has wrong item spell trigger value in spelltrigger_%d (%u)",i,j+1,proto->Spells[j].SpellTrigger);
                    const_cast<ItemTemplate*>(proto)->Spells[j].SpellId = 0;
                    const_cast<ItemTemplate*>(proto)->Spells[j].SpellTrigger = ITEM_SPELLTRIGGER_ON_USE;
                }
                else if(proto->Spells[j].SpellId != 0)
                {
                    TC_LOG_ERROR("FIXME","Item (Entry: %u) has wrong spell in spellid_%d (%u) for learning special format",i,j+1,proto->Spells[j].SpellId);
                    const_cast<ItemTemplate*>(proto)->Spells[j].SpellId = 0;
                }
            }
        }
        // normal spell list
        else
        {
            for (int j = 0; j < 5; j++)
            {
                if(proto->Spells[j].SpellTrigger >= MAX_ITEM_SPELLTRIGGER || proto->Spells[j].SpellTrigger == ITEM_SPELLTRIGGER_LEARN_SPELL_ID)
                {
                    TC_LOG_ERROR("FIXME","Item (Entry: %u) has wrong item spell trigger value in spelltrigger_%d (%u)",i,j+1,proto->Spells[j].SpellTrigger);
                    const_cast<ItemTemplate*>(proto)->Spells[j].SpellId = 0;
                    const_cast<ItemTemplate*>(proto)->Spells[j].SpellTrigger = ITEM_SPELLTRIGGER_ON_USE;
                }

                if(proto->Spells[j].SpellId)
                {
                    SpellEntry const* spellInfo = sSpellMgr->GetSpellInfo(proto->Spells[j].SpellId);
                    if(!spellInfo)
                    {
                        TC_LOG_ERROR("FIXME","Item (Entry: %u) has wrong (not existing) spell in spellid_%d (%u)",i,j+1,proto->Spells[j].SpellId);
                        const_cast<ItemTemplate*>(proto)->Spells[j].SpellId = 0;
                    }
                    // allowed only in special format
                    else if(proto->Spells[j].SpellId==SPELL_ID_GENERIC_LEARN)
                    {
                        TC_LOG_ERROR("FIXME","Item (Entry: %u) has broken spell in spellid_%d (%u)",i,j+1,proto->Spells[j].SpellId);
                        const_cast<ItemTemplate*>(proto)->Spells[j].SpellId = 0;
                    }
                }
            }
        }

        if(proto->Bonding >= MAX_BIND_TYPE)
            TC_LOG_ERROR("FIXME","Item (Entry: %u) has wrong Bonding value (%u)",i,proto->Bonding);

        if(proto->PageText && !sPageTextStore.LookupEntry<PageText>(proto->PageText))
            TC_LOG_ERROR("FIXME","Item (Entry: %u) has non existing first page (Id:%u)", i,proto->PageText);

        if(proto->LockID && !sLockStore.LookupEntry(proto->LockID))
            TC_LOG_ERROR("FIXME","Item (Entry: %u) has wrong LockID (%u)",i,proto->LockID);

        if(proto->Sheath >= MAX_SHEATHETYPE)
        {
            TC_LOG_ERROR("FIXME","Item (Entry: %u) has wrong Sheath (%u)",i,proto->Sheath);
            const_cast<ItemTemplate*>(proto)->Sheath = SHEATHETYPE_NONE;
        }

        if(proto->RandomProperty && !sItemRandomPropertiesStore.LookupEntry(GetItemEnchantMod(proto->RandomProperty)))
        {
            TC_LOG_ERROR("FIXME","Item (Entry: %u) has unknown (wrong or not listed in `item_enchantment_template`) RandomProperty (%u)",i,proto->RandomProperty);
            const_cast<ItemTemplate*>(proto)->RandomProperty = 0;
        }

        if(proto->RandomSuffix && !sItemRandomSuffixStore.LookupEntry(GetItemEnchantMod(proto->RandomSuffix)))
        {
            TC_LOG_ERROR("FIXME","Item (Entry: %u) has wrong RandomSuffix (%u)",i,proto->RandomSuffix);
            const_cast<ItemTemplate*>(proto)->RandomSuffix = 0;
        }

        if(proto->ItemSet && !sItemSetStore.LookupEntry(proto->ItemSet))
        {
            TC_LOG_ERROR("FIXME","Item (Entry: %u) have wrong ItemSet (%u)",i,proto->ItemSet);
            const_cast<ItemTemplate*>(proto)->ItemSet = 0;
        }

        if(proto->Area && !GetAreaEntryByAreaID(proto->Area))
            TC_LOG_ERROR("FIXME","Item (Entry: %u) has wrong Area (%u)",i,proto->Area);

        if(proto->Map && !sMapStore.LookupEntry(proto->Map))
            TC_LOG_ERROR("FIXME","Item (Entry: %u) has wrong Map (%u)",i,proto->Map);

        if(proto->TotemCategory && !sTotemCategoryStore.LookupEntry(proto->TotemCategory))
            TC_LOG_ERROR("FIXME","Item (Entry: %u) has wrong TotemCategory (%u)",i,proto->TotemCategory);

        for (int j = 0; j < 3; j++)
        {
            if(proto->Socket[j].Color && (proto->Socket[j].Color & SOCKET_COLOR_ALL) != proto->Socket[j].Color)
            {
                TC_LOG_ERROR("FIXME","Item (Entry: %u) has wrong socketColor_%d (%u)",i,j+1,proto->Socket[j].Color);
                const_cast<ItemTemplate*>(proto)->Socket[j].Color = 0;
            }
        }

        if(proto->GemProperties && !sGemPropertiesStore.LookupEntry(proto->GemProperties))
            TC_LOG_ERROR("FIXME","Item (Entry: %u) has wrong GemProperties (%u)",i,proto->GemProperties);

        if(proto->FoodType >= MAX_PET_DIET)
        {
            TC_LOG_ERROR("FIXME","Item (Entry: %u) has wrong FoodType value (%u)",i,proto->FoodType);
            const_cast<ItemTemplate*>(proto)->FoodType = 0;
        }
    }

    // this DBC used currently only for check item templates in DB.
    sItemStore.Clear();
}
*/
void ObjectMgr::LoadPetLevelInfo()
{
    // Loading levels data
    {
        //                                                 0               1      2   3     4    5    6    7     8    9
        QueryResult result  = WorldDatabase.Query("SELECT creature_entry, level, hp, mana, str, agi, sta, inte, spi, armor FROM pet_levelstats");

        uint32 count = 0;

        if (!result)
        {
            TC_LOG_ERROR("server.loading", "Error loading `pet_levelstats` table or empty table.");
            return;
        }
        do
        {
            Field* fields = result->Fetch();

            uint32 creature_id = fields[0].GetUInt32();
            if(!sObjectMgr->GetCreatureTemplate(creature_id))
            {
                TC_LOG_ERROR("FIXME","Wrong creature id %u in `pet_levelstats` table, ignoring.",creature_id);
                continue;
            }

            uint32 current_level = fields[1].GetUInt32();
            if(current_level > sWorld->getConfig(CONFIG_MAX_PLAYER_LEVEL))
            {
                if(current_level > STRONG_MAX_LEVEL)        // hardcoded level maximum
                    TC_LOG_ERROR("FIXME","Wrong (> %u) level %u in `pet_levelstats` table, ignoring.",STRONG_MAX_LEVEL,current_level);
                else
                    TC_LOG_DEBUG("FIXME","Unused (> MaxPlayerLevel in Trinityd.conf) level %u in `pet_levelstats` table, ignoring.",current_level);
                continue;
            }
            else if(current_level < 1)
            {
                TC_LOG_ERROR("FIXME","Wrong (<1) level %u in `pet_levelstats` table, ignoring.",current_level);
                continue;
            }

            PetLevelInfo*& pInfoMapEntry = petInfo[creature_id];

            if(pInfoMapEntry==NULL)
                pInfoMapEntry =  new PetLevelInfo[sWorld->getConfig(CONFIG_MAX_PLAYER_LEVEL)];

            // data for level 1 stored in [0] array element, ...
            PetLevelInfo* pLevelInfo = &pInfoMapEntry[current_level-1];

            pLevelInfo->health = fields[2].GetUInt16();
            pLevelInfo->mana   = fields[3].GetUInt16();
            pLevelInfo->armor  = fields[9].GetUInt16();

            for (int i = 0; i < MAX_STATS; i++)
                pLevelInfo->stats[i] = fields[i+4].GetUInt16();
            
            ++count;
        }
        while (result->NextRow());

        TC_LOG_INFO("server.loading", ">> Loaded %u level pet stats definitions", count );
    }

    // Fill gaps and check integrity
    for (PetLevelInfoMap::iterator itr = petInfo.begin(); itr != petInfo.end(); ++itr)
    {
        PetLevelInfo* pInfo = itr->second;

        // fatal error if no level 1 data
        if(!pInfo || pInfo[0].health == 0 )
        {
            TC_LOG_ERROR("sql.sql","Creature %u does not have pet stats data for Level 1!",itr->first);
            exit(1);
        }

        // fill level gaps
        for (uint32 level = 1; level < sWorld->getConfig(CONFIG_MAX_PLAYER_LEVEL); ++level)
        {
            if(pInfo[level].health == 0)
            {
                TC_LOG_ERROR("sql.sql","Creature %u has no data for Level %i pet stats data, using data of Level %i.",itr->first,level+1, level);
                pInfo[level] = pInfo[level-1];
            }
        }
    }
}

PetLevelInfo const* ObjectMgr::GetPetLevelInfo(uint32 creature_id, uint32 level) const
{
    if(level > sWorld->getConfig(CONFIG_MAX_PLAYER_LEVEL))
        level = sWorld->getConfig(CONFIG_MAX_PLAYER_LEVEL);

    PetLevelInfoMap::const_iterator itr = petInfo.find(creature_id);
    if(itr == petInfo.end())
        return NULL;

    return &itr->second[level-1];                           // data for level 1 stored in [0] array element, ...
}

void ObjectMgr::LoadPlayerInfo()
{
    // Load playercreate
    {
        //                                                0     1      2    3     4           5           6
        QueryResult result = WorldDatabase.Query("SELECT race, class, map, zone, position_x, position_y, position_z FROM playercreateinfo");

        uint32 count = 0;

        if (!result)
        {
            TC_LOG_ERROR( "server.loading","Error loading `playercreateinfo` table or empty table.");
            exit(1);
        }

        do
        {
            Field* fields = result->Fetch();

            uint32 current_race     = fields[0].GetUInt32();
            uint32 current_class    = fields[1].GetUInt32();
            uint32 mapId            = fields[2].GetUInt32();
            uint32 areaId           = fields[3].GetUInt32();
            float  positionX        = fields[4].GetFloat();
            float  positionY        = fields[5].GetFloat();
            float  positionZ        = fields[6].GetFloat();

            if(current_race >= MAX_RACES)
            {
                TC_LOG_ERROR("sql.sql","Wrong race %u in `playercreateinfo` table, ignoring.",current_race);
                continue;
            }

            ChrRacesEntry const* rEntry = sChrRacesStore.LookupEntry(current_race);
            if(!rEntry)
            {
                TC_LOG_ERROR("sql.sql","Wrong race %u in `playercreateinfo` table, ignoring.",current_race);
                continue;
            }

            if(current_class >= MAX_CLASSES)
            {
                TC_LOG_ERROR("sql.sql","Wrong class %u in `playercreateinfo` table, ignoring.",current_class);
                continue;
            }

            if(!sChrClassesStore.LookupEntry(current_class))
            {
                TC_LOG_ERROR("sql.sql","Wrong class %u in `playercreateinfo` table, ignoring.",current_class);
                continue;
            }

            // accept DB data only for valid position (and non instanceable)
            if( !MapManager::IsValidMapCoord(mapId,positionX,positionY,positionZ) )
            {
                TC_LOG_ERROR("sql.sql","Wrong home position for class %u race %u pair in `playercreateinfo` table, ignoring.",current_class,current_race);
                continue;
            }

            if( sMapStore.LookupEntry(mapId)->Instanceable() )
            {
                TC_LOG_ERROR("sql.sql","Home position in instanceable map for class %u race %u pair in `playercreateinfo` table, ignoring.",current_class,current_race);
                continue;
            }

            PlayerInfo* pInfo = &playerInfo[current_race][current_class];

            pInfo->mapId     = mapId;
            pInfo->areaId    = areaId;
            pInfo->positionX = positionX;
            pInfo->positionY = positionY;
            pInfo->positionZ = positionZ;

            pInfo->displayId_m = rEntry->model_m;
            pInfo->displayId_f = rEntry->model_f;

            ++count;
        }
        while (result->NextRow());

        TC_LOG_INFO("server.loading", ">> Loaded %u player create definitions", count );
    }

    // Load playercreate items
    {
        //                                                0     1      2       3
        QueryResult result = WorldDatabase.Query("SELECT race, class, itemid, amount FROM playercreateinfo_item");

        uint32 count = 0;

        if (!result)
        {
            TC_LOG_INFO( "server.loading",">> Loaded %u custom player create items", count );
        }
        else
        {
            do
            {
                Field* fields = result->Fetch();

                uint32 current_race = fields[0].GetUInt32();
                if(current_race >= MAX_RACES)
                {
                    TC_LOG_ERROR("sql.sql","Wrong race %u in `playercreateinfo_item` table, ignoring.",current_race);
                    continue;
                }

                uint32 current_class = fields[1].GetUInt32();
                if(current_class >= MAX_CLASSES)
                {
                    TC_LOG_ERROR("sql.sql","Wrong class %u in `playercreateinfo_item` table, ignoring.",current_class);
                    continue;
                }

                PlayerInfo* pInfo = &playerInfo[current_race][current_class];

                uint32 item_id = fields[2].GetUInt32();

                if(!GetItemTemplate(item_id))
                {
                    TC_LOG_ERROR("sql.sql","Item id %u (race %u class %u) in `playercreateinfo_item` table but not listed in `item_template`, ignoring.",item_id,current_race,current_class);
                    continue;
                }

                uint32 amount  = fields[3].GetUInt32();

                if(!amount)
                {
                    TC_LOG_ERROR("sql.sql","Item id %u (class %u race %u) have amount==0 in `playercreateinfo_item` table, ignoring.",item_id,current_race,current_class);
                    continue;
                }

                pInfo->item.push_back(PlayerCreateInfoItem( item_id, amount));

                ++count;
            }
            while(result->NextRow());

            TC_LOG_INFO("server.loading", ">> Loaded %u custom player create items", count );
        }
    }

    // Load playercreate spells
    {

        QueryResult result = NULL;
        if(sWorld->getConfig(CONFIG_START_ALL_SPELLS))
            result = WorldDatabase.Query("SELECT race, class, Spell, Active FROM playercreateinfo_spell_custom");
        else
            result = WorldDatabase.Query("SELECT race, class, Spell, Active FROM playercreateinfo_spell");

        uint32 count = 0;

        if (!result)
        {
            TC_LOG_ERROR("player.loading", "Error loading player starting spells or empty table.");
        }
        else
        {
            do
            {
                Field* fields = result->Fetch();

                uint32 current_race = fields[0].GetUInt32();
                if(current_race >= MAX_RACES)
                {
                    TC_LOG_ERROR("sql.sql","Wrong race %u in `playercreateinfo_spell` table, ignoring.",current_race);
                    continue;
                }

                uint32 current_class = fields[1].GetUInt32();
                if(current_class >= MAX_CLASSES)
                {
                    TC_LOG_ERROR("sql.sql","Wrong class %u in `playercreateinfo_spell` table, ignoring.",current_class);
                    continue;
                }

                PlayerInfo* pInfo = &playerInfo[current_race][current_class];
                pInfo->spell.push_back(CreateSpellPair(fields[2].GetUInt16(), fields[3].GetUInt8()));

                ++count;
            }
            while( result->NextRow() );

            TC_LOG_INFO( "server.loading",">> Loaded %u player create spells", count );
        }
    }

    // Load playercreate actions
    {
        //                                                0     1      2       3       4     5
        QueryResult result = WorldDatabase.Query("SELECT race, class, button, action, type, misc FROM playercreateinfo_action");

        uint32 count = 0;

        if (!result)
        {
            TC_LOG_ERROR( "player.loading","Error loading `playercreateinfo_action` table or empty table.");
        }
        else
        {
            do
            {
                Field* fields = result->Fetch();

                uint32 current_race = fields[0].GetUInt32();
                if(current_race >= MAX_RACES)
                {
                    TC_LOG_ERROR("sql.sql","Wrong race %u in `playercreateinfo_action` table, ignoring.",current_race);
                    continue;
                }

                uint32 current_class = fields[1].GetUInt32();
                if(current_class >= MAX_CLASSES)
                {
                    TC_LOG_ERROR("sql.sql","Wrong class %u in `playercreateinfo_action` table, ignoring.",current_class);
                    continue;
                }

                PlayerInfo* pInfo = &playerInfo[current_race][current_class];
                pInfo->action[0].push_back(fields[2].GetUInt16());
                pInfo->action[1].push_back(fields[3].GetUInt16());
                pInfo->action[2].push_back(fields[4].GetUInt16());
                pInfo->action[3].push_back(fields[5].GetUInt16());

                ++count;
            }
            while( result->NextRow() );

            TC_LOG_INFO("player.loading", ">> Loaded %u player create actions", count );
        }
    }

    // Loading levels data (class only dependent)
    {
        //                                                 0      1      2       3
        QueryResult result  = WorldDatabase.Query("SELECT class, level, basehp, basemana FROM player_classlevelstats");

        uint32 count = 0;

        if (!result)
        {
            TC_LOG_ERROR( "player.loading", "Error loading `player_classlevelstats` table or empty table.");
            exit(1);
        }

        do
        {
            Field* fields = result->Fetch();

            uint32 current_class = fields[0].GetUInt32();
            if(current_class >= MAX_CLASSES)
            {
                TC_LOG_ERROR("FIXME","Wrong class %u in `player_classlevelstats` table, ignoring.",current_class);
                continue;
            }

            uint32 current_level = fields[1].GetUInt32();
            if(current_level > sWorld->getConfig(CONFIG_MAX_PLAYER_LEVEL))
            {
                if(current_level > STRONG_MAX_LEVEL)        // hardcoded level maximum
                    TC_LOG_ERROR("sql.sql","Wrong (> %u) level %u in `player_classlevelstats` table, ignoring.",STRONG_MAX_LEVEL,current_level);
                else
                    TC_LOG_DEBUG("sql.sql","Unused (> MaxPlayerLevel in Trinityd.conf) level %u in `player_classlevelstats` table, ignoring.",current_level);
                continue;
            }

            PlayerClassInfo* pClassInfo = &playerClassInfo[current_class];

            if(!pClassInfo->levelInfo)
                pClassInfo->levelInfo = new PlayerClassLevelInfo[sWorld->getConfig(CONFIG_MAX_PLAYER_LEVEL)];

            PlayerClassLevelInfo* pClassLevelInfo = &pClassInfo->levelInfo[current_level-1];

            pClassLevelInfo->basehealth = fields[2].GetUInt16();
            pClassLevelInfo->basemana   = fields[3].GetUInt16();

            ++count;
        }
        while (result->NextRow());

        TC_LOG_INFO( "server.loading",">> Loaded %u level health/mana definitions", count );
    }

    // Fill gaps and check integrity
    for (int class_ = 0; class_ < MAX_CLASSES; ++class_)
    {
        // skip non existed classes
        if(!sChrClassesStore.LookupEntry(class_))
            continue;

        PlayerClassInfo* pClassInfo = &playerClassInfo[class_];

        // fatal error if no level 1 data
        if(!pClassInfo->levelInfo || pClassInfo->levelInfo[0].basehealth == 0 )
        {
            TC_LOG_ERROR("FIXME","Class %i Level 1 does not have health/mana data!",class_);
            exit(1);
        }

        // fill level gaps
        for (uint32 level = 1; level < sWorld->getConfig(CONFIG_MAX_PLAYER_LEVEL); ++level)
        {
            if(pClassInfo->levelInfo[level].basehealth == 0)
            {
                TC_LOG_ERROR("FIXME","Class %i Level %i does not have health/mana data. Using stats data of level %i.",class_,level+1, level);
                pClassInfo->levelInfo[level] = pClassInfo->levelInfo[level-1];
            }
        }
    }

    // Loading levels data (class/race dependent)
    {
        //                                                 0     1      2      3    4    5    6    7
        QueryResult result  = WorldDatabase.Query("SELECT race, class, level, str, agi, sta, inte, spi FROM player_levelstats");

        uint32 count = 0;

        if (!result)
        {
            TC_LOG_ERROR( "player.loading", "Error loading `player_levelstats` table or empty table.");
            exit(1);
        }

        do
        {
            Field* fields = result->Fetch();

            uint32 current_race = fields[0].GetUInt32();
            if(current_race >= MAX_RACES)
            {
                TC_LOG_ERROR("sql.sql","Wrong race %u in `player_levelstats` table, ignoring.",current_race);
                continue;
            }

            uint32 current_class = fields[1].GetUInt32();
            if(current_class >= MAX_CLASSES)
            {
                TC_LOG_ERROR("sql.sql","Wrong class %u in `player_levelstats` table, ignoring.",current_class);
                continue;
            }

            uint32 current_level = fields[2].GetUInt32();
            if(current_level > sWorld->getConfig(CONFIG_MAX_PLAYER_LEVEL))
            {
                if(current_level > STRONG_MAX_LEVEL)        // hardcoded level maximum
                    TC_LOG_ERROR("sql.sql","Wrong (> %u) level %u in `player_levelstats` table, ignoring.",STRONG_MAX_LEVEL,current_level);
                else
                    TC_LOG_DEBUG("sql.sql","Unused (> MaxPlayerLevel in Trinityd.conf) level %u in `player_levelstats` table, ignoring.",current_level);
                continue;
            }

            PlayerInfo* pInfo = &playerInfo[current_race][current_class];

            if(!pInfo->levelInfo)
                pInfo->levelInfo = new PlayerLevelInfo[sWorld->getConfig(CONFIG_MAX_PLAYER_LEVEL)];

            PlayerLevelInfo* pLevelInfo = &pInfo->levelInfo[current_level-1];

            for (int i = 0; i < MAX_STATS; i++)
            {
                pLevelInfo->stats[i] = fields[i+3].GetUInt8();
            }

            ++count;
        }
        while (result->NextRow());

        TC_LOG_INFO("server.loading", ">> Loaded %u level stats definitions", count );
    }

    // Fill gaps and check integrity
    for (int race = 0; race < MAX_RACES; ++race)
    {
        // skip non existed races
        if(!sChrRacesStore.LookupEntry(race))
            continue;

        for (int class_ = 0; class_ < MAX_CLASSES; ++class_)
        {
            // skip non existed classes
            if(!sChrClassesStore.LookupEntry(class_))
                continue;

            PlayerInfo* pInfo = &playerInfo[race][class_];

            // skip non loaded combinations
            if(!pInfo->displayId_m || !pInfo->displayId_f)
                continue;

            // skip expansion races if not playing with expansion
            if (sWorld->getConfig(CONFIG_EXPANSION) < 1 && (race == RACE_BLOODELF || race == RACE_DRAENEI))
                continue;

            // skip expansion classes if not playing with expansion
            if (sWorld->getConfig(CONFIG_EXPANSION) < 2 && class_ == CLASS_DEATH_KNIGHT)
                continue;

            // fatal error if no level 1 data
            if(!pInfo->levelInfo || pInfo->levelInfo[0].stats[0] == 0 )
            {
                TC_LOG_ERROR("FIXME","Race %i Class %i Level 1 does not have stats data!",race,class_);
                exit(1);
            }

            // fill level gaps
            for (uint32 level = 1; level < sWorld->getConfig(CONFIG_MAX_PLAYER_LEVEL); ++level)
            {
                if(pInfo->levelInfo[level].stats[0] == 0)
                {
                    TC_LOG_ERROR("FIXME","Race %i Class %i Level %i does not have stats data. Using stats data of level %i.",race,class_,level+1, level);
                    pInfo->levelInfo[level] = pInfo->levelInfo[level-1];
                }
            }
        }
    }
}

void ObjectMgr::GetPlayerClassLevelInfo(uint32 class_, uint32 level, PlayerClassLevelInfo* info) const
{
    if(level < 1 || class_ >= MAX_CLASSES)
        return;

    PlayerClassInfo const* pInfo = &playerClassInfo[class_];

    if(level > sWorld->getConfig(CONFIG_MAX_PLAYER_LEVEL))
        level = sWorld->getConfig(CONFIG_MAX_PLAYER_LEVEL);

    *info = pInfo->levelInfo[level-1];
}

void ObjectMgr::GetPlayerLevelInfo(uint32 race, uint32 class_, uint32 level, PlayerLevelInfo* info) const
{
    if(level < 1 || race   >= MAX_RACES || class_ >= MAX_CLASSES)
        return;

    PlayerInfo const* pInfo = &playerInfo[race][class_];
    if(pInfo->displayId_m==0 || pInfo->displayId_f==0)
        return;

    if(level <= sWorld->getConfig(CONFIG_MAX_PLAYER_LEVEL))
        *info = pInfo->levelInfo[level-1];
    else
        BuildPlayerLevelInfo(race,class_,level,info);
}

void ObjectMgr::BuildPlayerLevelInfo(uint8 race, uint8 _class, uint8 level, PlayerLevelInfo* info) const
{
    // base data (last known level)
    *info = playerInfo[race][_class].levelInfo[sWorld->getConfig(CONFIG_MAX_PLAYER_LEVEL)-1];

    for(int lvl = sWorld->getConfig(CONFIG_MAX_PLAYER_LEVEL)-1; lvl < level; ++lvl)
    {
        switch(_class)
        {
            case CLASS_WARRIOR:
                info->stats[STAT_STRENGTH]  += (lvl > 23 ? 2: (lvl > 1  ? 1: 0));
                info->stats[STAT_STAMINA]   += (lvl > 23 ? 2: (lvl > 1  ? 1: 0));
                info->stats[STAT_AGILITY]   += (lvl > 36 ? 1: (lvl > 6 && (lvl%2) ? 1: 0));
                info->stats[STAT_INTELLECT] += (lvl > 9 && !(lvl%2) ? 1: 0);
                info->stats[STAT_SPIRIT]    += (lvl > 9 && !(lvl%2) ? 1: 0);
                break;
            case CLASS_PALADIN:
                info->stats[STAT_STRENGTH]  += (lvl > 3  ? 1: 0);
                info->stats[STAT_STAMINA]   += (lvl > 33 ? 2: (lvl > 1 ? 1: 0));
                info->stats[STAT_AGILITY]   += (lvl > 38 ? 1: (lvl > 7 && !(lvl%2) ? 1: 0));
                info->stats[STAT_INTELLECT] += (lvl > 6 && (lvl%2) ? 1: 0);
                info->stats[STAT_SPIRIT]    += (lvl > 7 ? 1: 0);
                break;
            case CLASS_HUNTER:
                info->stats[STAT_STRENGTH]  += (lvl > 4  ? 1: 0);
                info->stats[STAT_STAMINA]   += (lvl > 4  ? 1: 0);
                info->stats[STAT_AGILITY]   += (lvl > 33 ? 2: (lvl > 1 ? 1: 0));
                info->stats[STAT_INTELLECT] += (lvl > 8 && (lvl%2) ? 1: 0);
                info->stats[STAT_SPIRIT]    += (lvl > 38 ? 1: (lvl > 9 && !(lvl%2) ? 1: 0));
                break;
            case CLASS_ROGUE:
                info->stats[STAT_STRENGTH]  += (lvl > 5  ? 1: 0);
                info->stats[STAT_STAMINA]   += (lvl > 4  ? 1: 0);
                info->stats[STAT_AGILITY]   += (lvl > 16 ? 2: (lvl > 1 ? 1: 0));
                info->stats[STAT_INTELLECT] += (lvl > 8 && !(lvl%2) ? 1: 0);
                info->stats[STAT_SPIRIT]    += (lvl > 38 ? 1: (lvl > 9 && !(lvl%2) ? 1: 0));
                break;
            case CLASS_PRIEST:
                info->stats[STAT_STRENGTH]  += (lvl > 9 && !(lvl%2) ? 1: 0);
                info->stats[STAT_STAMINA]   += (lvl > 5  ? 1: 0);
                info->stats[STAT_AGILITY]   += (lvl > 38 ? 1: (lvl > 8 && (lvl%2) ? 1: 0));
                info->stats[STAT_INTELLECT] += (lvl > 22 ? 2: (lvl > 1 ? 1: 0));
                info->stats[STAT_SPIRIT]    += (lvl > 3  ? 1: 0);
                break;
            case CLASS_SHAMAN:
                info->stats[STAT_STRENGTH]  += (lvl > 34 ? 1: (lvl > 6 && (lvl%2) ? 1: 0));
                info->stats[STAT_STAMINA]   += (lvl > 4 ? 1: 0);
                info->stats[STAT_AGILITY]   += (lvl > 7 && !(lvl%2) ? 1: 0);
                info->stats[STAT_INTELLECT] += (lvl > 5 ? 1: 0);
                info->stats[STAT_SPIRIT]    += (lvl > 4 ? 1: 0);
                break;
            case CLASS_MAGE:
                info->stats[STAT_STRENGTH]  += (lvl > 9 && !(lvl%2) ? 1: 0);
                info->stats[STAT_STAMINA]   += (lvl > 5  ? 1: 0);
                info->stats[STAT_AGILITY]   += (lvl > 9 && !(lvl%2) ? 1: 0);
                info->stats[STAT_INTELLECT] += (lvl > 24 ? 2: (lvl > 1 ? 1: 0));
                info->stats[STAT_SPIRIT]    += (lvl > 33 ? 2: (lvl > 2 ? 1: 0));
                break;
            case CLASS_WARLOCK:
                info->stats[STAT_STRENGTH]  += (lvl > 9 && !(lvl%2) ? 1: 0);
                info->stats[STAT_STAMINA]   += (lvl > 38 ? 2: (lvl > 3 ? 1: 0));
                info->stats[STAT_AGILITY]   += (lvl > 9 && !(lvl%2) ? 1: 0);
                info->stats[STAT_INTELLECT] += (lvl > 33 ? 2: (lvl > 2 ? 1: 0));
                info->stats[STAT_SPIRIT]    += (lvl > 38 ? 2: (lvl > 3 ? 1: 0));
                break;
            case CLASS_DRUID:
                info->stats[STAT_STRENGTH]  += (lvl > 38 ? 2: (lvl > 6 && (lvl%2) ? 1: 0));
                info->stats[STAT_STAMINA]   += (lvl > 32 ? 2: (lvl > 4 ? 1: 0));
                info->stats[STAT_AGILITY]   += (lvl > 38 ? 2: (lvl > 8 && (lvl%2) ? 1: 0));
                info->stats[STAT_INTELLECT] += (lvl > 38 ? 3: (lvl > 4 ? 1: 0));
                info->stats[STAT_SPIRIT]    += (lvl > 38 ? 3: (lvl > 5 ? 1: 0));
        }
    }
}

void ObjectMgr::LoadGuilds()
{
    Guild *newguild;
    uint32 count = 0;

    QueryResult result = CharacterDatabase.Query( "SELECT guildid FROM guild" );

    if( !result )
    {
        TC_LOG_INFO("server.loading", ">> Loaded %u guild definitions", count );
        return;
    }

    do
    {
        Field *fields = result->Fetch();

        ++count;

        newguild = new Guild;
        if(!newguild->LoadGuildFromDB(fields[0].GetUInt32()))
        {
            newguild->Disband();
            delete newguild;
            continue;
        }
        AddGuild(newguild);

    }while( result->NextRow() );

    TC_LOG_INFO("server.loading", ">> Loaded %u guild definitions", count );
}

void ObjectMgr::LoadArenaTeams()
{
    uint32 count = 0;

    QueryResult result = CharacterDatabase.Query( "SELECT arenateamid FROM arena_team" );

    if( !result )
    {
        TC_LOG_INFO("server.loading", ">> Loaded %u arenateam definitions", count );
        return;
    }

    do
    {
        Field *fields = result->Fetch();

        ++count;

        ArenaTeam *newarenateam = new ArenaTeam;
        if(!newarenateam->LoadArenaTeamFromDB(fields[0].GetUInt32()))
        {
            delete newarenateam;
            continue;
        }
        AddArenaTeam(newarenateam);
    }while( result->NextRow() );

    if(sWorld->getConfig(CONFIG_ARENA_NEW_TITLE_DISTRIB))
        sWorld->updateArenaLeadersTitles();

    TC_LOG_INFO("server.loading", ">> Loaded %u arenateam definitions", count );
}

void ObjectMgr::LoadGroups()
{
    // -- loading groups --
    Group *group = NULL;
    uint64 leaderGuid = 0;
    uint32 count = 0;
    //                                                     0         1              2           3           4              5      6      7      8      9      10     11     12     13      14          15
    QueryResult result = CharacterDatabase.Query("SELECT mainTank, mainAssistant, lootMethod, looterGuid, lootThreshold, icon1, icon2, icon3, icon4, icon5, icon6, icon7, icon8, isRaid, difficulty, leaderGuid FROM groups");

    if( !result )
    {
        TC_LOG_INFO("server.loading", ">> Loaded %u group definitions", count );
        return;
    }

    do
    {
        Field *fields = result->Fetch();
        ++count;
        leaderGuid = MAKE_NEW_GUID(fields[15].GetUInt32(),0,HIGHGUID_PLAYER);

        group = new Group;
        if(!group->LoadGroupFromDB(leaderGuid, result, false))
        {
            group->Disband();
            delete group;
            continue;
        }
        AddGroup(group);
    }while( result->NextRow() );

    TC_LOG_INFO("server.loading", ">> Loaded %u group definitions", count );

    // -- loading members --
    count = 0;
    group = NULL;
    leaderGuid = 0;
    //                                        0           1          2         3
    result = CharacterDatabase.Query("SELECT memberGuid, assistant, subgroup, leaderGuid FROM group_member ORDER BY leaderGuid");
    if (result)
    {
        do
        {
            Field *fields = result->Fetch();
            count++;
            leaderGuid = MAKE_NEW_GUID(fields[3].GetUInt32(), 0, HIGHGUID_PLAYER);
            if(!group || group->GetLeaderGUID() != leaderGuid)
            {
                group = GetGroupByLeader(leaderGuid);
                if(!group)
                {
                    TC_LOG_ERROR("FIXME","Incorrect entry in group_member table : no group with leader %d for member %d!", fields[3].GetUInt32(), fields[0].GetUInt32());
                    CharacterDatabase.PExecute("DELETE FROM group_member WHERE memberGuid = '%d'", fields[0].GetUInt32());
                    continue;
                }
            }

            if(!group->LoadMemberFromDB(fields[0].GetUInt32(), fields[2].GetUInt8(), fields[1].GetBool()))
            {
                TC_LOG_ERROR("FIXME","Incorrect entry in group_member table : member %d cannot be added to player %d's group!", fields[0].GetUInt32(), fields[3].GetUInt32());
                CharacterDatabase.PExecute("DELETE FROM group_member WHERE memberGuid = '%d'", fields[0].GetUInt32());
            }
        }while( result->NextRow() );
    }

    // clean groups
    // TODO: maybe delete from the DB before loading in this case
    for(GroupSet::iterator itr = mGroupSet.begin(); itr != mGroupSet.end();)
    {
        if((*itr)->GetMembersCount() < 2)
        {
            (*itr)->Disband();
            delete *itr;
            mGroupSet.erase(itr++);
        }
        else
            ++itr;
    }

    // -- loading instances --
    count = 0;
    group = NULL;
    leaderGuid = 0;
    result = CharacterDatabase.Query(
        //      0           1    2         3          4           5
        "SELECT leaderGuid, map, instance, permanent, difficulty, resettime, "
        // 6
        "(SELECT COUNT(*) FROM character_instance WHERE guid = leaderGuid AND instance = group_instance.instance AND permanent = 1 LIMIT 1) "
        "FROM group_instance LEFT JOIN instance ON instance = id ORDER BY leaderGuid"
    );

    if (result)
    {
        do
        {
            Field *fields = result->Fetch();
            count++;
            leaderGuid = MAKE_NEW_GUID(fields[0].GetUInt32(), 0, HIGHGUID_PLAYER);
            if(!group || group->GetLeaderGUID() != leaderGuid)
            {
                group = GetGroupByLeader(leaderGuid);
                if(!group)
                {
                    TC_LOG_ERROR("FIXME","Incorrect entry in group_instance table : no group with leader %d", fields[0].GetUInt32());
                    continue;
                }
            }

            InstanceSave *save = sInstanceSaveMgr->AddInstanceSave(fields[1].GetUInt32(), fields[2].GetUInt32(), fields[4].GetUInt8(), (time_t)fields[5].GetUInt64(), (fields[6].GetUInt32() == 0), true);
            group->BindToInstance(save, fields[3].GetBool(), true);
        }while( result->NextRow() );
    }

    TC_LOG_INFO("server.loading", ">> Loaded %u group-instance binds total", count );
    TC_LOG_INFO("server.loading", ">> Loaded %u group members total", count );
}

void ObjectMgr::LoadQuests()
{
    // For reload case
    for(auto itr : mQuestTemplates)
        delete itr.second;
    mQuestTemplates.clear();

    mExclusiveQuestGroups.clear();

    //                                               0                     1       2           3             4         5           6     7              8
    QueryResult result = WorldDatabase.Query("SELECT quest_template.entry, Method, ZoneOrSort, SkillOrClass, MinLevel, QuestLevel, Type, RequiredRaces, RequiredSkillValue,"
    //   9                    10                 11                     12                   13                     14                   15                16
        "RepObjectiveFaction, RepObjectiveValue, RequiredMinRepFaction, RequiredMinRepValue, RequiredMaxRepFaction, RequiredMaxRepValue, SuggestedPlayers, LimitTime,"
    //   17          18            19           20           21           22              23                24         25            26
        "QuestFlags, SpecialFlags, CharTitleId, PrevQuestId, NextQuestId, ExclusiveGroup, NextQuestInChain, SrcItemId, SrcItemCount, SrcSpell,"
    //   27     28       29          30               31                32       33              34              35              36
        "Title, Details, Objectives, OfferRewardText, RequestItemsText, EndText, ObjectiveText1, ObjectiveText2, ObjectiveText3, ObjectiveText4,"
    //   37          38          39          40          41             42             43             44
        "ReqItemId1, ReqItemId2, ReqItemId3, ReqItemId4, ReqItemCount1, ReqItemCount2, ReqItemCount3, ReqItemCount4,"
    //   45            46            47            48            49               50               51               52               53             54             54             55
        "ReqSourceId1, ReqSourceId2, ReqSourceId3, ReqSourceId4, ReqSourceCount1, ReqSourceCount2, ReqSourceCount3, ReqSourceCount4, ReqSourceRef1, ReqSourceRef2, ReqSourceRef3, ReqSourceRef4,"
    //   57                  58                  59                  60                  61                     62                     63                     64
        "ReqCreatureOrGOId1, ReqCreatureOrGOId2, ReqCreatureOrGOId3, ReqCreatureOrGOId4, ReqCreatureOrGOCount1, ReqCreatureOrGOCount2, ReqCreatureOrGOCount3, ReqCreatureOrGOCount4,"
    //   65             66             67             68
        "ReqSpellCast1, ReqSpellCast2, ReqSpellCast3, ReqSpellCast4,"
    //   69                70                71                72                73                74
        "RewChoiceItemId1, RewChoiceItemId2, RewChoiceItemId3, RewChoiceItemId4, RewChoiceItemId5, RewChoiceItemId6,"
    //   75                   76                   77                   78                   79                   80
        "RewChoiceItemCount1, RewChoiceItemCount2, RewChoiceItemCount3, RewChoiceItemCount4, RewChoiceItemCount5, RewChoiceItemCount6,"
    //   81          82          83          84          85             86             87             88
        "RewItemId1, RewItemId2, RewItemId3, RewItemId4, RewItemCount1, RewItemCount2, RewItemCount3, RewItemCount4,"
    //   89              90              91              92              93              94            95            96            97            98
        "RewRepFaction1, RewRepFaction2, RewRepFaction3, RewRepFaction4, RewRepFaction5, RewRepValue1, RewRepValue2, RewRepValue3, RewRepValue4, RewRepValue5,"
    //   99                 100            101               102       103           104                105               106         107     108    109
        "RewHonorableKills, RewOrReqMoney, RewMoneyMaxLevel, RewSpell, RewSpellCast, RewMailTemplateId, RewMailDelaySecs, PointMapId, PointX, PointY, PointOpt,"
    //   110            111            112           113              114            115            116                117                118                119
        "DetailsEmote1, DetailsEmote2, DetailsEmote3, DetailsEmote4,IncompleteEmote, CompleteEmote, OfferRewardEmote1, OfferRewardEmote2, OfferRewardEmote3, OfferRewardEmote4,"
    //   120          121             122
        "StartScript, CompleteScript, quest_bugs.entry"
        " FROM quest_template "
        " LEFT JOIN quest_bugs ON bugged = 1 and quest_bugs.entry = quest_template.entry "
        );
    if(result == NULL)
    {
        TC_LOG_INFO("server.loading", ">> Loaded 0 quests definitions" );
        TC_LOG_ERROR("server.loading","`quest_template` table is empty!");
        return;
    }
    
    // create multimap previous quest for each existed quest
    // some quests can have many previous maps set by NextQuestId in previous quest
    // for example set of race quests can lead to single not race specific quest
    do
    {
        Field *fields = result->Fetch();
        Quest* newQuest = new Quest(fields);
        mQuestTemplates[newQuest->GetQuestId()] = newQuest;
    } while( result->NextRow() );

    // Post processing
    for (QuestMap::iterator iter = mQuestTemplates.begin(); iter != mQuestTemplates.end(); ++iter)
    {
        Quest * qinfo = iter->second;

        // additional quest integrity checks (GO, creature_template and item_template must be loaded already)

        if( qinfo->GetQuestMethod() >= 3 )
        {
            TC_LOG_ERROR("sql.sql","Quest %u has `Method` = %u, expected values are 0, 1 or 2.",qinfo->GetQuestId(),qinfo->GetQuestMethod());
        }

        if (qinfo->QuestFlags & ~QUEST_TRINITY_FLAGS_DB_ALLOWED)
        {
            TC_LOG_ERROR("sql.sql","Quest %u has `SpecialFlags` = %u > max allowed value. Correct `SpecialFlags` to value <= %u",
                qinfo->GetQuestId(),qinfo->QuestFlags,QUEST_TRINITY_FLAGS_DB_ALLOWED >> 16);
            qinfo->QuestFlags &= QUEST_TRINITY_FLAGS_DB_ALLOWED;
        }

        if(qinfo->QuestFlags & QUEST_FLAGS_DAILY)
        {
            if(!(qinfo->QuestFlags & QUEST_TRINITY_FLAGS_REPEATABLE))
            {
                TC_LOG_ERROR("sql.sql","Daily Quest %u not marked as repeatable in `SpecialFlags`, added.",qinfo->GetQuestId());
                qinfo->QuestFlags |= QUEST_TRINITY_FLAGS_REPEATABLE;
            }
        }

        if(qinfo->QuestFlags & QUEST_FLAGS_AUTO_REWARDED)
        {
            // at auto-reward can be rewarded only RewChoiceItemId[0]
            for(int j = 1; j < QUEST_REWARD_CHOICES_COUNT; ++j )
            {
                if(uint32 id = qinfo->RewChoiceItemId[j])
                {
                    TC_LOG_ERROR("sql.sql","Quest %u has `RewChoiceItemId%d` = %u but item from `RewChoiceItemId%d` can't be rewarded with quest flag QUEST_FLAGS_AUTO_REWARDED.",
                        qinfo->GetQuestId(),j+1,id,j+1);
                    // no changes, quest ignore this data
                }
            }
        }

        // client quest log visual (area case)
        if( qinfo->ZoneOrSort > 0 )
        {
            if(!GetAreaEntryByAreaID(qinfo->ZoneOrSort))
            {
                TC_LOG_ERROR("sql.sql","Quest %u has `ZoneOrSort` = %u (zone case) but zone with this id does not exist.",
                    qinfo->GetQuestId(),qinfo->ZoneOrSort);
                // no changes, quest not dependent from this value but can have problems at client
            }
        }
        // client quest log visual (sort case)
        if( qinfo->ZoneOrSort < 0 )
        {
            QuestSortEntry const* qSort = sQuestSortStore.LookupEntry(-int32(qinfo->ZoneOrSort));
            if( !qSort )
            {
                TC_LOG_ERROR("sql.sql","Quest %u has `ZoneOrSort` = %i (sort case) but quest sort with this id does not exist.",
                    qinfo->GetQuestId(),qinfo->ZoneOrSort);
                // no changes, quest not dependent from this value but can have problems at client (note some may be 0, we must allow this so no check)
            }
            //check SkillOrClass value (class case).
            if( ClassByQuestSort(-int32(qinfo->ZoneOrSort)) )
            {
                // SkillOrClass should not have class case when class case already set in ZoneOrSort.
                if(qinfo->SkillOrClass < 0)
                {
                    TC_LOG_ERROR("sql.sql","Quest %u has `ZoneOrSort` = %i (class sort case) and `SkillOrClass` = %i (class case), redundant.",
                        qinfo->GetQuestId(),qinfo->ZoneOrSort,qinfo->SkillOrClass);
                }
            }
            //check for proper SkillOrClass value (skill case)
            if(int32 skill_id =  SkillByQuestSort(-int32(qinfo->ZoneOrSort)))
            {
                // skill is positive value in SkillOrClass
                if(qinfo->SkillOrClass != skill_id )
                {
                    TC_LOG_ERROR("sql.sql","Quest %u has `ZoneOrSort` = %i (skill sort case) but `SkillOrClass` does not have a corresponding value (%i).",
                        qinfo->GetQuestId(),qinfo->ZoneOrSort,skill_id);
                    //override, and force proper value here?
                }
            }
        }

        // SkillOrClass (class case)
        if( qinfo->SkillOrClass < 0 )
        {
            if(pow(2,11)-1 < qinfo->SkillOrClass) //mask for every class
            {
                TC_LOG_ERROR("sql.sql","Quest %u has invalid `SkillOrClass` = %i (class case)",
                    qinfo->GetQuestId(), qinfo->SkillOrClass);
            }
        }
        // SkillOrClass (skill case)
        if( qinfo->SkillOrClass > 0 )
        {
            if( !sSkillLineStore.LookupEntry(qinfo->SkillOrClass) )
            {
                TC_LOG_ERROR("sql.sql","Quest %u has `SkillOrClass` = %i (skill case) but skill (%i) does not exist",
                    qinfo->GetQuestId(), qinfo->SkillOrClass, qinfo->SkillOrClass);
            }
        }

        if( qinfo->RequiredSkillValue )
        {
            if( qinfo->RequiredSkillValue > sWorld->GetConfigMaxSkillValue() )
            {
                TC_LOG_ERROR("sql.sql","Quest %u has `RequiredSkillValue` = %u but max possible skill is %u, quest can't be done.",
                    qinfo->GetQuestId(),qinfo->RequiredSkillValue,sWorld->GetConfigMaxSkillValue());
                // no changes, quest can't be done for this requirement
            }

            if( qinfo->SkillOrClass <= 0 )
            {
                TC_LOG_ERROR("sql.sql","Quest %u has `RequiredSkillValue` = %u but `SkillOrClass` = %i (class case), value ignored.",
                    qinfo->GetQuestId(),qinfo->RequiredSkillValue,qinfo->SkillOrClass);
                // no changes, quest can't be done for this requirement (fail at wrong skill id)
            }
        }
        // else Skill quests can have 0 skill level, this is ok

        if(qinfo->RepObjectiveFaction && !sFactionStore.LookupEntry(qinfo->RepObjectiveFaction))
        {
            TC_LOG_ERROR("sql.sql","Quest %u has `RepObjectiveFaction` = %u but faction template %u does not exist, quest can't be done.",
                qinfo->GetQuestId(),qinfo->RepObjectiveFaction,qinfo->RepObjectiveFaction);
            // no changes, quest can't be done for this requirement
        }

        if(qinfo->RequiredMinRepFaction && !sFactionStore.LookupEntry(qinfo->RequiredMinRepFaction))
        {
            TC_LOG_ERROR("sql.sql","Quest %u has `RequiredMinRepFaction` = %u but faction template %u does not exist, quest can't be done.",
                qinfo->GetQuestId(),qinfo->RequiredMinRepFaction,qinfo->RequiredMinRepFaction);
            // no changes, quest can't be done for this requirement
        }

        if(qinfo->RequiredMaxRepFaction && !sFactionStore.LookupEntry(qinfo->RequiredMaxRepFaction))
        {
            TC_LOG_ERROR("sql.sql","Quest %u has `RequiredMaxRepFaction` = %u but faction template %u does not exist, quest can't be done.",
                qinfo->GetQuestId(),qinfo->RequiredMaxRepFaction,qinfo->RequiredMaxRepFaction);
            // no changes, quest can't be done for this requirement
        }

        if(qinfo->RequiredMinRepValue && qinfo->RequiredMinRepValue > Player::Reputation_Cap)
        {
            TC_LOG_ERROR("sql.sql","Quest %u has `RequiredMinRepValue` = %d but max reputation is %u, quest can't be done.",
                qinfo->GetQuestId(),qinfo->RequiredMinRepValue,Player::Reputation_Cap);
            // no changes, quest can't be done for this requirement
        }

        if(qinfo->RequiredMinRepValue && qinfo->RequiredMaxRepValue && qinfo->RequiredMaxRepValue <= qinfo->RequiredMinRepValue)
        {
            TC_LOG_ERROR("sql.sql","Quest %u has `RequiredMaxRepValue` = %d and `RequiredMinRepValue` = %d, quest can't be done.",
                qinfo->GetQuestId(),qinfo->RequiredMaxRepValue,qinfo->RequiredMinRepValue);
            // no changes, quest can't be done for this requirement
        }

        if(!qinfo->RepObjectiveFaction && qinfo->RepObjectiveValue > 0 )
        {
            TC_LOG_ERROR("sql.sql","Quest %u has `RepObjectiveValue` = %d but `RepObjectiveFaction` is 0, value has no effect",
                qinfo->GetQuestId(),qinfo->RepObjectiveValue);
            // warning
        }

        if(!qinfo->RequiredMinRepFaction && qinfo->RequiredMinRepValue > 0 )
        {
            TC_LOG_ERROR("sql.sql","Quest %u has `RequiredMinRepValue` = %d but `RequiredMinRepFaction` is 0, value has no effect",
                qinfo->GetQuestId(),qinfo->RequiredMinRepValue);
            // warning
        }

        if(!qinfo->RequiredMaxRepFaction && qinfo->RequiredMaxRepValue > 0 )
        {
            TC_LOG_ERROR("sql.sql","Quest %u has `RequiredMaxRepValue` = %d but `RequiredMaxRepFaction` is 0, value has no effect",
                qinfo->GetQuestId(),qinfo->RequiredMaxRepValue);
            // warning
        }

        if(qinfo->CharTitleId && !sCharTitlesStore.LookupEntry(qinfo->CharTitleId))
        {
            TC_LOG_ERROR("sql.sql","Quest %u has `CharTitleId` = %u but CharTitle Id %u does not exist, quest can't be rewarded with title.",
                qinfo->GetQuestId(),qinfo->GetCharTitleId(),qinfo->GetCharTitleId());
            qinfo->CharTitleId = 0;
            // quest can't reward this title
        }

        if(qinfo->SrcItemId)
        {
            if(!sObjectMgr->GetItemTemplate(qinfo->SrcItemId))
            {
                TC_LOG_ERROR("sql.sql","Quest %u has `SrcItemId` = %u but item with entry %u does not exist, quest can't be done.",
                    qinfo->GetQuestId(),qinfo->SrcItemId,qinfo->SrcItemId);
                qinfo->SrcItemId = 0;                       // quest can't be done for this requirement
            }
            else if(qinfo->SrcItemCount==0)
            {
                TC_LOG_ERROR("sql.sql","Quest %u has `SrcItemId` = %u but `SrcItemCount` = 0, set to 1 but need fix in DB.",
                    qinfo->GetQuestId(),qinfo->SrcItemId);
                qinfo->SrcItemCount = 1;                    // update to 1 for allow quest work for backward compatibility with DB
            }
        }
        else if(qinfo->SrcItemCount>0)
        {
            TC_LOG_ERROR("sql.sql","Quest %u has `SrcItemId` = 0 but `SrcItemCount` = %u, useless value.",
                qinfo->GetQuestId(),qinfo->SrcItemCount);
            qinfo->SrcItemCount=0;                          // no quest work changes in fact
        }

        if(qinfo->SrcSpell)
        {
            SpellEntry const* spellInfo = sSpellMgr->GetSpellInfo(qinfo->SrcSpell);
            if(!spellInfo)
            {
                TC_LOG_ERROR("sql.sql","Quest %u has `SrcSpell` = %u but spell %u doesn't exist, quest can't be done.",
                    qinfo->GetQuestId(),qinfo->SrcSpell,qinfo->SrcSpell);
                qinfo->SrcSpell = 0;                        // quest can't be done for this requirement
            }
            else if(!SpellMgr::IsSpellValid(spellInfo))
            {
                TC_LOG_ERROR("sql.sql","Quest %u has `SrcSpell` = %u but spell %u is broken, quest can't be done.",
                    qinfo->GetQuestId(),qinfo->SrcSpell,qinfo->SrcSpell);
                qinfo->SrcSpell = 0;                        // quest can't be done for this requirement
            }
        }

        for(int j = 0; j < QUEST_OBJECTIVES_COUNT; ++j )
        {
            uint32 id = qinfo->ReqItemId[j];
            if(id)
            {
                if(qinfo->ReqItemCount[j]==0)
                {
                    TC_LOG_ERROR("sql.sql","Quest %u has `ReqItemId%d` = %u but `ReqItemCount%d` = 0, quest can't be done.",
                        qinfo->GetQuestId(),j+1,id,j+1);
                    // no changes, quest can't be done for this requirement
                }

                qinfo->SetFlag(QUEST_TRINITY_FLAGS_DELIVER);

                if(!sObjectMgr->GetItemTemplate(id))
                {
                    TC_LOG_ERROR("sql.sql","Quest %u has `ReqItemId%d` = %u but item with entry %u does not exist, quest can't be done.",
                        qinfo->GetQuestId(),j+1,id,id);
                    qinfo->ReqItemCount[j] = 0;             // prevent incorrect work of quest
                }
            }
            else if(qinfo->ReqItemCount[j]>0)
            {
                TC_LOG_ERROR("sql.sql","Quest %u has `ReqItemId%d` = 0 but `ReqItemCount%d` = %u, quest can't be done.",
                    qinfo->GetQuestId(),j+1,j+1,qinfo->ReqItemCount[j]);
                qinfo->ReqItemCount[j] = 0;                 // prevent incorrect work of quest
            }
        }

        for(int j = 0; j < QUEST_SOURCE_ITEM_IDS_COUNT; ++j )
        {
            uint32 id = qinfo->ReqSourceId[j];
            if(id)
            {
                if(!sObjectMgr->GetItemTemplate(id))
                {
                    TC_LOG_ERROR("sql.sql","Quest %u has `ReqSourceId%d` = %u but item with entry %u does not exist, quest can't be done.",
                        qinfo->GetQuestId(),j+1,id,id);
                    // no changes, quest can't be done for this requirement
                }

                if(!qinfo->ReqSourceCount[j])
                {
                    TC_LOG_ERROR("sql.sql","Quest %u has `ReqSourceId%d` = %u but `ReqSourceCount%d` = 0, quest can't be done.",
                        qinfo->GetQuestId(),j+1,id,j+1);
                    qinfo->ReqSourceId[j] = 0;              // prevent incorrect work of quest
                }

                if(!qinfo->ReqSourceRef[j])
                {
                    TC_LOG_ERROR("sql.sql","Quest %u has `ReqSourceId%d` = %u but `ReqSourceRef%d` = 0, quest can't be done.",
                        qinfo->GetQuestId(),j+1,id,j+1);
                    qinfo->ReqSourceId[j] = 0;              // prevent incorrect work of quest
                }
            }
            else
            {
                if(qinfo->ReqSourceCount[j]>0)
                {
                    TC_LOG_ERROR("sql.sql","Quest %u has `ReqSourceId%d` = 0 but `ReqSourceCount%d` = %u.",
                        qinfo->GetQuestId(),j+1,j+1,qinfo->ReqSourceCount[j]);
                    // no changes, quest ignore this data
                }

                if(qinfo->ReqSourceRef[j]>0)
                {
                    TC_LOG_ERROR("sql.sql","Quest %u has `ReqSourceId%d` = 0 but `ReqSourceRef%d` = %u.",
                        qinfo->GetQuestId(),j+1,j+1,qinfo->ReqSourceRef[j]);
                    // no changes, quest ignore this data
                }
            }
        }

        for(int j = 0; j < QUEST_SOURCE_ITEM_IDS_COUNT; ++j )
        {
            uint32 ref = qinfo->ReqSourceRef[j];
            if(ref)
            {
                if(ref > QUEST_OBJECTIVES_COUNT)
                {
                    TC_LOG_ERROR("sql.sql","Quest %u has `ReqSourceRef%d` = %u but max value in `ReqSourceRef%d` is %u, quest can't be done.",
                        qinfo->GetQuestId(),j+1,ref,j+1,QUEST_OBJECTIVES_COUNT);
                    // no changes, quest can't be done for this requirement
                }
                else
                if(!qinfo->ReqItemId[ref-1] && !qinfo->ReqSpell[ref-1])
                {
                    TC_LOG_ERROR("sql.sql","Quest %u has `ReqSourceRef%d` = %u but `ReqItemId%u` = 0 and `ReqSpellCast%u` = 0, quest can't be done.",
                        qinfo->GetQuestId(),j+1,ref,ref,ref);
                    // no changes, quest can't be done for this requirement
                }
                else if(qinfo->ReqItemId[ref-1] && qinfo->ReqSpell[ref-1])
                {
                    TC_LOG_ERROR("sql.sql","Quest %u has `ReqItemId%u` = %u and `ReqSpellCast%u` = %u, quest can't have both fields <> 0, then can't be done.",
                        qinfo->GetQuestId(),ref,qinfo->ReqItemId[ref-1],ref,qinfo->ReqSpell[ref-1]);
                    // no changes, quest can't be done for this requirement
                    qinfo->ReqSourceId[j] = 0;              // prevent incorrect work of quest
                }
            }
        }

        for(int j = 0; j < QUEST_OBJECTIVES_COUNT; ++j )
        {
            uint32 id = qinfo->ReqSpell[j];
            if(id)
            {
                SpellEntry const* spellInfo = sSpellMgr->GetSpellInfo(id);
                if(!spellInfo)
                {
                    TC_LOG_ERROR("sql.sql","Quest %u has `ReqSpellCast%d` = %u but spell %u does not exist, quest can't be done.",
                        qinfo->GetQuestId(),j+1,id,id);
                    // no changes, quest can't be done for this requirement
                }

                if(!qinfo->ReqCreatureOrGOId[j])
                {
                    bool found = false;
                    for(int k = 0; k < 3; ++k)
                    {
                        if( spellInfo->Effect[k]==SPELL_EFFECT_QUEST_COMPLETE && uint32(spellInfo->EffectMiscValue[k])==qinfo->QuestId ||
                            spellInfo->Effect[k]==SPELL_EFFECT_SEND_EVENT)
                        {
                            found = true;
                            break;
                        }
                    }

                    if(found)
                    {
                        if(!qinfo->HasFlag(QUEST_TRINITY_FLAGS_EXPLORATION_OR_EVENT))
                        {
                            TC_LOG_ERROR("sql.sql","Spell (id: %u) have SPELL_EFFECT_QUEST_COMPLETE or SPELL_EFFECT_SEND_EVENT for quest %u and ReqCreatureOrGOId%d = 0, but quest not have flag QUEST_TRINITY_FLAGS_EXPLORATION_OR_EVENT. Quest flags or ReqCreatureOrGOId%d must be fixed, quest modified to enable objective.",spellInfo->Id,qinfo->QuestId,j+1,j+1);

                            // this will prevent quest completing without objective
                            const_cast<Quest*>(qinfo)->SetFlag(QUEST_TRINITY_FLAGS_EXPLORATION_OR_EVENT);
                        }
                    }
                    else
                    {
                        TC_LOG_ERROR("sql.sql","Quest %u has `ReqSpellCast%d` = %u and ReqCreatureOrGOId%d = 0 but spell %u does not have SPELL_EFFECT_QUEST_COMPLETE or SPELL_EFFECT_SEND_EVENT effect for this quest, quest can't be done.",
                            qinfo->GetQuestId(),j+1,id,j+1,id);
                        // no changes, quest can't be done for this requirement
                    }
                }
            }
        }

        for(int j = 0; j < QUEST_OBJECTIVES_COUNT; ++j )
        {
            int32 id = qinfo->ReqCreatureOrGOId[j];
            if(id < 0 && !GetGameObjectTemplate(-id))
            {
                TC_LOG_ERROR("sql.sql","Quest %u has `ReqCreatureOrGOId%d` = %i but gameobject %u does not exist, quest can't be done.",
                    qinfo->GetQuestId(),j+1,id,uint32(-id));
                qinfo->ReqCreatureOrGOId[j] = 0;            // quest can't be done for this requirement
            }

            if(id > 0 && !sObjectMgr->GetCreatureTemplate(id))
            {
                TC_LOG_ERROR("FIXME","Quest %u has `ReqCreatureOrGOId%d` = %i but creature with entry %u does not exist, quest can't be done.",
                    qinfo->GetQuestId(),j+1,id,uint32(id));
                qinfo->ReqCreatureOrGOId[j] = 0;            // quest can't be done for this requirement
            }

            if(id)
            {
                // In fact SpeakTo and Kill are quite same: either you can speak to mob:SpeakTo or you can't:Kill/Cast

                qinfo->SetFlag(QUEST_TRINITY_FLAGS_KILL_OR_CAST | QUEST_TRINITY_FLAGS_SPEAKTO);

                if(!qinfo->ReqCreatureOrGOCount[j])
                {
                    TC_LOG_ERROR("sql.sql","Quest %u has `ReqCreatureOrGOId%d` = %u but `ReqCreatureOrGOCount%d` = 0, quest can't be done.",
                        qinfo->GetQuestId(),j+1,id,j+1);
                    // no changes, quest can be incorrectly done, but we already report this
                }
            }
            else if(qinfo->ReqCreatureOrGOCount[j]>0)
            {
                TC_LOG_ERROR("sql.sql","Quest %u has `ReqCreatureOrGOId%d` = 0 but `ReqCreatureOrGOCount%d` = %u.",
                    qinfo->GetQuestId(),j+1,j+1,qinfo->ReqCreatureOrGOCount[j]);
                // no changes, quest ignore this data
            }
        }

        for(int j = 0; j < QUEST_REWARD_CHOICES_COUNT; ++j )
        {
            uint32 id = qinfo->RewChoiceItemId[j];
            if(id)
            {
                if(!sObjectMgr->GetItemTemplate(id))
                {
                    TC_LOG_ERROR("sql.sql","Quest %u has `RewChoiceItemId%d` = %u but item with entry %u does not exist, quest will not reward this item.",
                        qinfo->GetQuestId(),j+1,id,id);
                    qinfo->RewChoiceItemId[j] = 0;          // no changes, quest will not reward this
                }

                if(!qinfo->RewChoiceItemCount[j])
                {
                    TC_LOG_ERROR("sql.sql","Quest %u has `RewChoiceItemId%d` = %u but `RewChoiceItemCount%d` = 0, quest can't be done.",
                        qinfo->GetQuestId(),j+1,id,j+1);
                    // no changes, quest can't be done
                }
            }
            else if(qinfo->RewChoiceItemCount[j]>0)
            {
                TC_LOG_ERROR("sql.sql","Quest %u has `RewChoiceItemId%d` = 0 but `RewChoiceItemCount%d` = %u.",
                    qinfo->GetQuestId(),j+1,j+1,qinfo->RewChoiceItemCount[j]);
                // no changes, quest ignore this data
            }
        }

        for(int j = 0; j < QUEST_REWARDS_COUNT; ++j )
        {
            uint32 id = qinfo->RewItemId[j];
            if(id)
            {
                if(!sObjectMgr->GetItemTemplate(id))
                {
                    TC_LOG_ERROR("sql.sql","Quest %u has `RewItemId%d` = %u but item with entry %u does not exist, quest will not reward this item.",
                        qinfo->GetQuestId(),j+1,id,id);
                    qinfo->RewItemId[j] = 0;                // no changes, quest will not reward this item
                }

                if(!qinfo->RewItemCount[j])
                {
                    TC_LOG_ERROR("sql.sql","Quest %u has `RewItemId%d` = %u but `RewItemCount%d` = 0, quest will not reward this item.",
                        qinfo->GetQuestId(),j+1,id,j+1);
                    // no changes
                }
            }
            else if(qinfo->RewItemCount[j]>0)
            {
                TC_LOG_ERROR("sql.sql","Quest %u has `RewItemId%d` = 0 but `RewItemCount%d` = %u.",
                    qinfo->GetQuestId(),j+1,j+1,qinfo->RewItemCount[j]);
                // no changes, quest ignore this data
            }
        }

        for(int j = 0; j < QUEST_REPUTATIONS_COUNT; ++j)
        {
            if(qinfo->RewRepFaction[j])
            {
                if(!qinfo->RewRepValue[j])
                {
                    TC_LOG_ERROR("sql.sql","Quest %u has `RewRepFaction%d` = %u but `RewRepValue%d` = 0, quest will not reward this reputation.",
                        qinfo->GetQuestId(),j+1,qinfo->RewRepValue[j],j+1);
                    // no changes
                }

                if(!sFactionStore.LookupEntry(qinfo->RewRepFaction[j]))
                {
                    TC_LOG_ERROR("sql.sql","Quest %u has `RewRepFaction%d` = %u but raw faction (faction.dbc) %u does not exist, quest will not reward reputation for this faction.",
                        qinfo->GetQuestId(),j+1,qinfo->RewRepFaction[j] ,qinfo->RewRepFaction[j] );
                    qinfo->RewRepFaction[j] = 0;            // quest will not reward this
                }
            }
            else if(qinfo->RewRepValue[j]!=0)
            {
                TC_LOG_ERROR("sql.sql","Quest %u has `RewRepFaction%d` = 0 but `RewRepValue%d` = %u.",
                    qinfo->GetQuestId(),j+1,j+1,qinfo->RewRepValue[j]);
                // no changes, quest ignore this data
            }
        }

        if(qinfo->RewSpell)
        {
            SpellEntry const* spellInfo = sSpellMgr->GetSpellInfo(qinfo->RewSpell);

            if(!spellInfo)
            {
                TC_LOG_ERROR("sql.sql","Quest %u has `RewSpell` = %u but spell %u does not exist, spell removed as display reward.",
                    qinfo->GetQuestId(),qinfo->RewSpell,qinfo->RewSpell);
                qinfo->RewSpell = 0;                        // no spell reward will display for this quest
            }

            else if(!SpellMgr::IsSpellValid(spellInfo))
            {
                TC_LOG_ERROR("sql.sql","Quest %u has `RewSpell` = %u but spell %u is broken, quest can't be done.",
                    qinfo->GetQuestId(),qinfo->RewSpell,qinfo->RewSpell);
                qinfo->RewSpell = 0;                        // no spell reward will display for this quest
            }

        }

        if(qinfo->RewSpellCast)
        {
            SpellEntry const* spellInfo = sSpellMgr->GetSpellInfo(qinfo->RewSpellCast);

            if(!spellInfo)
            {
                TC_LOG_ERROR("sql.sql","Quest %u has `RewSpellCast` = %u but spell %u does not exist, quest will not have a spell reward.",
                    qinfo->GetQuestId(),qinfo->RewSpellCast,qinfo->RewSpellCast);
                qinfo->RewSpellCast = 0;                    // no spell will be casted on player
            }

            else if(!SpellMgr::IsSpellValid(spellInfo))
            {
                TC_LOG_ERROR("sql.sql","Quest %u has `RewSpellCast` = %u but spell %u is broken, quest can't be done.",
                    qinfo->GetQuestId(),qinfo->RewSpellCast,qinfo->RewSpellCast);
                qinfo->RewSpellCast = 0;                    // no spell will be casted on player
            }

        }

        if(qinfo->RewMailTemplateId)
        {
            if(!sMailTemplateStore.LookupEntry(qinfo->RewMailTemplateId))
            {
                TC_LOG_ERROR("sql.sql","Quest %u has `RewMailTemplateId` = %u but mail template  %u does not exist, quest will not have a mail reward.",
                    qinfo->GetQuestId(),qinfo->RewMailTemplateId,qinfo->RewMailTemplateId);
                qinfo->RewMailTemplateId = 0;               // no mail will send to player
                qinfo->RewMailDelaySecs = 0;                // no mail will send to player
            }
        }

        if(qinfo->NextQuestInChain)
        {
            if(mQuestTemplates.find(qinfo->NextQuestInChain) == mQuestTemplates.end())
            {
                TC_LOG_ERROR("sql.sql","Quest %u has `NextQuestInChain` = %u but quest %u does not exist, quest chain will not work.",
                    qinfo->GetQuestId(),qinfo->NextQuestInChain ,qinfo->NextQuestInChain );
                qinfo->NextQuestInChain = 0;
            }
            else
                mQuestTemplates[qinfo->NextQuestInChain]->prevChainQuests.push_back(qinfo->GetQuestId());
        }

        // fill additional data stores
        if(qinfo->PrevQuestId)
        {
            if (mQuestTemplates.find(abs(qinfo->GetPrevQuestId())) == mQuestTemplates.end())
            {
                TC_LOG_ERROR("sql.sql","Quest %d has PrevQuestId %i, but no such quest", qinfo->GetQuestId(), qinfo->GetPrevQuestId());
            }
            else
            {
                qinfo->prevQuests.push_back(qinfo->PrevQuestId);
            }
        }

        if(qinfo->NextQuestId)
        {
            if (mQuestTemplates.find(abs(qinfo->GetNextQuestId())) == mQuestTemplates.end())
            {
                TC_LOG_ERROR("sql.sql","Quest %d has NextQuestId %i, but no such quest", qinfo->GetQuestId(), qinfo->GetNextQuestId());
            }
            else
            {
                int32 signedQuestId = qinfo->NextQuestId < 0 ? -int32(qinfo->GetQuestId()) : int32(qinfo->GetQuestId());
                mQuestTemplates[abs(qinfo->GetNextQuestId())]->prevQuests.push_back(signedQuestId);
            }
        }

        if(qinfo->ExclusiveGroup)
            mExclusiveQuestGroups.insert(std::pair<int32, uint32>(qinfo->ExclusiveGroup, qinfo->GetQuestId()));
        if(qinfo->LimitTime)
            qinfo->SetFlag(QUEST_TRINITY_FLAGS_TIMED);
    }

    // check QUEST_TRINITY_FLAGS_EXPLORATION_OR_EVENT for spell with SPELL_EFFECT_QUEST_COMPLETE
    //for (uint32 i = 0; i < sSpellStore.GetNumRows(); ++i)
    for (std::map<uint32, SpellEntry*>::iterator itr = sObjectMgr->GetSpellStore()->begin(); itr != sObjectMgr->GetSpellStore()->end(); itr++)
    {
        uint32 i = itr->first;
        SpellEntry const *spellInfo = sSpellMgr->GetSpellInfo(i);
        if(!spellInfo)
            continue;

        for(int j = 0; j < 3; ++j)
        {
            if(spellInfo->Effect[j] != SPELL_EFFECT_QUEST_COMPLETE)
                continue;

            uint32 quest_id = spellInfo->EffectMiscValue[j];

            Quest const* quest = GetQuestTemplate(quest_id);

            // some quest referenced in spells not exist (outdated spells)
            if(!quest)
                continue;

            if(!quest->HasFlag(QUEST_TRINITY_FLAGS_EXPLORATION_OR_EVENT))
            {
                TC_LOG_ERROR("sql.sql","Spell (id: %u) have SPELL_EFFECT_QUEST_COMPLETE for quest %u , but quest not have flag QUEST_TRINITY_FLAGS_EXPLORATION_OR_EVENT. Quest flags must be fixed, quest modified to enable objective.",spellInfo->Id,quest_id);

                // this will prevent quest completing without objective
                const_cast<Quest*>(quest)->SetFlag(QUEST_TRINITY_FLAGS_EXPLORATION_OR_EVENT);
            }
        }
    }

    TC_LOG_INFO("server.loading", ">> Loaded " UI64FMTD " quests definitions", mQuestTemplates.size());
}

void ObjectMgr::LoadQuestLocales()
{
    mQuestLocaleMap.clear();                                // need for reload case

    QueryResult result = WorldDatabase.Query("SELECT entry,"
        "Title_loc1,Details_loc1,Objectives_loc1,OfferRewardText_loc1,RequestItemsText_loc1,EndText_loc1,ObjectiveText1_loc1,ObjectiveText2_loc1,ObjectiveText3_loc1,ObjectiveText4_loc1,"
        "Title_loc2,Details_loc2,Objectives_loc2,OfferRewardText_loc2,RequestItemsText_loc2,EndText_loc2,ObjectiveText1_loc2,ObjectiveText2_loc2,ObjectiveText3_loc2,ObjectiveText4_loc2,"
        "Title_loc3,Details_loc3,Objectives_loc3,OfferRewardText_loc3,RequestItemsText_loc3,EndText_loc3,ObjectiveText1_loc3,ObjectiveText2_loc3,ObjectiveText3_loc3,ObjectiveText4_loc3,"
        "Title_loc4,Details_loc4,Objectives_loc4,OfferRewardText_loc4,RequestItemsText_loc4,EndText_loc4,ObjectiveText1_loc4,ObjectiveText2_loc4,ObjectiveText3_loc4,ObjectiveText4_loc4,"
        "Title_loc5,Details_loc5,Objectives_loc5,OfferRewardText_loc5,RequestItemsText_loc5,EndText_loc5,ObjectiveText1_loc5,ObjectiveText2_loc5,ObjectiveText3_loc5,ObjectiveText4_loc5,"
        "Title_loc6,Details_loc6,Objectives_loc6,OfferRewardText_loc6,RequestItemsText_loc6,EndText_loc6,ObjectiveText1_loc6,ObjectiveText2_loc6,ObjectiveText3_loc6,ObjectiveText4_loc6,"
        "Title_loc7,Details_loc7,Objectives_loc7,OfferRewardText_loc7,RequestItemsText_loc7,EndText_loc7,ObjectiveText1_loc7,ObjectiveText2_loc7,ObjectiveText3_loc7,ObjectiveText4_loc7,"
        "Title_loc8,Details_loc8,Objectives_loc8,OfferRewardText_loc8,RequestItemsText_loc8,EndText_loc8,ObjectiveText1_loc8,ObjectiveText2_loc8,ObjectiveText3_loc8,ObjectiveText4_loc8"
        " FROM locales_quest"
        );

    if(!result)
    {
        TC_LOG_INFO("server.loading",">> Loaded 0 Quest locale strings. DB table `locales_quest` is empty.");
        return;
    }

    do
    {
        Field *fields = result->Fetch();

        uint32 entry = fields[0].GetUInt32();

        QuestLocale& data = mQuestLocaleMap[entry];

        for (uint8 i = MAX_LOCALE; i > 0; --i)
        {
            LocaleConstant locale = (LocaleConstant) i;

            AddLocaleString(fields[1 + 10 * (i - 1)].GetString(), locale, data.Title);
            AddLocaleString(fields[1 + 10 * (i - 1) + 1].GetString(), locale, data.Details);
            AddLocaleString(fields[1 + 10 * (i - 1) + 2].GetString(), locale, data.Objectives);
            AddLocaleString(fields[1 + 10 * (i - 1) + 3].GetString(), locale, data.OfferRewardText);
            AddLocaleString(fields[1 + 10 * (i - 1) + 4].GetString(), locale, data.RequestItemsText);
            AddLocaleString(fields[1 + 10 * (i - 1) + 5].GetString(), locale, data.EndText);

            for (uint8 k = 0; k < 4; ++k)
                AddLocaleString(fields[1 + 10 * (i - 1) + 6 + k].GetString(), locale, data.ObjectiveText[k]);
        }
    } while (result->NextRow());

    TC_LOG_INFO("server.loading", ">> Loaded " UI64FMTD " Quest locale strings", mQuestLocaleMap.size() );
}

void ObjectMgr::LoadPetCreateSpells()
{
    QueryResult result = WorldDatabase.Query("SELECT entry, Spell1, Spell2, Spell3, Spell4 FROM petcreateinfo_spell");
    if(!result)
    {
        TC_LOG_INFO("server.loading", ">> Loaded 0 pet create spells" );
        TC_LOG_ERROR("sql.sql","`petcreateinfo_spell` table is empty!");
        
        return;
    }

    uint32 count = 0;

    mPetCreateSpell.clear();

    do
    {
        Field *fields = result->Fetch();

        uint32 creature_id = fields[0].GetUInt32();

        if(!creature_id || !sObjectMgr->GetCreatureTemplate(creature_id))
            continue;

        PetCreateSpellEntry PetCreateSpell;
        for(int i = 0; i < 4; i++)
        {
            PetCreateSpell.spellid[i] = fields[i + 1].GetUInt32();

            if(PetCreateSpell.spellid[i] && !sSpellMgr->GetSpellInfo(PetCreateSpell.spellid[i]))
                TC_LOG_ERROR("sql.sql","Spell %u listed in `petcreateinfo_spell` does not exist",PetCreateSpell.spellid[i]);
        }

        mPetCreateSpell[creature_id] = PetCreateSpell;

        ++count;
    }
    while (result->NextRow());

    TC_LOG_INFO("server.loading", ">> Loaded %u pet create spells", count );
    
}

void ObjectMgr::LoadScripts(ScriptMapMap& scripts, char const* tablename)
{
    if(sWorld->IsScriptScheduled())                          // function don't must be called in time scripts use.
        return;

    TC_LOG_INFO("server.loading", "%s :", tablename);

    scripts.clear();                                        // need for reload support

    QueryResult result = WorldDatabase.PQuery( "SELECT id,delay,command,datalong,datalong2,dataint, x, y, z, o FROM %s", tablename );

    uint32 count = 0;

    if( !result )
    {
        TC_LOG_INFO("server.loading", ">> Loaded %u script definitions", count );
        
        return;
    }

    do
    {
        Field *fields = result->Fetch();
        ScriptInfo tmp;
        tmp.id        = fields[0].GetUInt32();
        tmp.delay     = fields[1].GetUInt32();
        tmp.command   = fields[2].GetUInt32();
        tmp.datalong  = fields[3].GetUInt32();
        tmp.datalong2 = fields[4].GetUInt32();
        tmp.dataint   = fields[5].GetInt32();
        tmp.x         = fields[6].GetFloat();
        tmp.y         = fields[7].GetFloat();
        tmp.z         = fields[8].GetFloat();
        tmp.o         = fields[9].GetFloat();

        // generic command args check
        switch(tmp.command)
        {
            case SCRIPT_COMMAND_TALK:
            {
                if(tmp.datalong > 3)
                {
                    TC_LOG_ERROR("sql.sql","Table `%s` has invalid talk type (datalong = %u) in SCRIPT_COMMAND_TALK for script id %u",tablename,tmp.datalong,tmp.id);
                    continue;
                }
                if(tmp.dataint==0)
                {
                    TC_LOG_ERROR("sql.sql","Table `%s` has invalid talk text id (dataint = %i) in SCRIPT_COMMAND_TALK for script id %u",tablename,tmp.dataint,tmp.id);
                    continue;
                }
                if(tmp.dataint < MIN_DB_SCRIPT_STRING_ID || tmp.dataint >= MAX_DB_SCRIPT_STRING_ID)
                {
                    TC_LOG_ERROR("sql.sql","Table `%s` has out of range text id (dataint = %i expected %u-%u) in SCRIPT_COMMAND_TALK for script id %u",tablename,tmp.dataint,MIN_DB_SCRIPT_STRING_ID,MAX_DB_SCRIPT_STRING_ID,tmp.id);
                    continue;
                }

                // if(!sObjectMgr->GetMangosStringLocale(tmp.dataint)) will checked after db_script_string loading
                break;
            }

            case SCRIPT_COMMAND_TELEPORT_TO:
            {
                if(!sMapStore.LookupEntry(tmp.datalong))
                {
                    TC_LOG_ERROR("sql.sql","Table `%s` has invalid map (Id: %u) in SCRIPT_COMMAND_TELEPORT_TO for script id %u",tablename,tmp.datalong,tmp.id);
                    continue;
                }

                if(!Trinity::IsValidMapCoord(tmp.x,tmp.y,tmp.z,tmp.o))
                {
                    TC_LOG_ERROR("sql.sql","Table `%s` has invalid coordinates (X: %f Y: %f) in SCRIPT_COMMAND_TELEPORT_TO for script id %u",tablename,tmp.x,tmp.y,tmp.id);
                    continue;
                }
                break;
            }

            case SCRIPT_COMMAND_TEMP_SUMMON_CREATURE:
            {
                if(!Trinity::IsValidMapCoord(tmp.x,tmp.y,tmp.z,tmp.o))
                {
                    TC_LOG_ERROR("sql.sql","Table `%s` has invalid coordinates (X: %f Y: %f) in SCRIPT_COMMAND_TEMP_SUMMON_CREATURE for script id %u",tablename,tmp.x,tmp.y,tmp.id);
                    continue;
                }

                if(!GetCreatureTemplate(tmp.datalong))
                {
                    TC_LOG_ERROR("sql.sql","Table `%s` has invalid creature (Entry: %u) in SCRIPT_COMMAND_TEMP_SUMMON_CREATURE for script id %u",tablename,tmp.datalong,tmp.id);
                    continue;
                }
                break;
            }

            case SCRIPT_COMMAND_RESPAWN_GAMEOBJECT:
            {
                GameObjectData const* data = GetGOData(tmp.datalong);
                if(!data)
                {
                    TC_LOG_ERROR("sql.sql","Table `%s` has invalid gameobject (GUID: %u) in SCRIPT_COMMAND_RESPAWN_GAMEOBJECT for script id %u",tablename,tmp.datalong,tmp.id);
                    continue;
                }

                GameObjectTemplate const* info = GetGameObjectTemplate(data->id);
                if(!info)
                {
                    TC_LOG_ERROR("sql.sql","Table `%s` has gameobject with invalid entry (GUID: %u Entry: %u) in SCRIPT_COMMAND_RESPAWN_GAMEOBJECT for script id %u",tablename,tmp.datalong,data->id,tmp.id);
                    continue;
                }

                if( info->type==GAMEOBJECT_TYPE_FISHINGNODE ||
                    info->type==GAMEOBJECT_TYPE_FISHINGHOLE ||
                    info->type==GAMEOBJECT_TYPE_DOOR        ||
                    info->type==GAMEOBJECT_TYPE_BUTTON      ||
                    info->type==GAMEOBJECT_TYPE_TRAP )
                {
                    TC_LOG_ERROR("sql.sql","Table `%s` have gameobject type (%u) unsupported by command SCRIPT_COMMAND_RESPAWN_GAMEOBJECT for script id %u",tablename,info->entry,tmp.id);
                    continue;
                }
                break;
            }
            case SCRIPT_COMMAND_OPEN_DOOR:
            case SCRIPT_COMMAND_CLOSE_DOOR:
            {
                GameObjectData const* data = GetGOData(tmp.datalong);
                if(!data)
                {
                    TC_LOG_ERROR("sql.sql","Table `%s` has invalid gameobject (GUID: %u) in %s for script id %u",tablename,tmp.datalong,(tmp.command==SCRIPT_COMMAND_OPEN_DOOR ? "SCRIPT_COMMAND_OPEN_DOOR" : "SCRIPT_COMMAND_CLOSE_DOOR"),tmp.id);
                    continue;
                }

                GameObjectTemplate const* info = GetGameObjectTemplate(data->id);
                if(!info)
                {
                    TC_LOG_ERROR("sql.sql","Table `%s` has gameobject with invalid entry (GUID: %u Entry: %u) in %s for script id %u",tablename,tmp.datalong,data->id,(tmp.command==SCRIPT_COMMAND_OPEN_DOOR ? "SCRIPT_COMMAND_OPEN_DOOR" : "SCRIPT_COMMAND_CLOSE_DOOR"),tmp.id);
                    continue;
                }

                if( info->type!=GAMEOBJECT_TYPE_DOOR)
                {
                    TC_LOG_ERROR("sql.sql","Table `%s` has gameobject type (%u) non supported by command %s for script id %u",tablename,info->entry,(tmp.command==SCRIPT_COMMAND_OPEN_DOOR ? "SCRIPT_COMMAND_OPEN_DOOR" : "SCRIPT_COMMAND_CLOSE_DOOR"),tmp.id);
                    continue;
                }

                break;
            }
            case SCRIPT_COMMAND_QUEST_EXPLORED:
            {
                Quest const* quest = GetQuestTemplate(tmp.datalong);
                if(!quest)
                {
                    TC_LOG_ERROR("sql.sql","Table `%s` has invalid quest (ID: %u) in SCRIPT_COMMAND_QUEST_EXPLORED in `datalong` for script id %u",tablename,tmp.datalong,tmp.id);
                    continue;
                }

                if(!quest->HasFlag(QUEST_TRINITY_FLAGS_EXPLORATION_OR_EVENT))
                {
                    TC_LOG_ERROR("sql.sql","Table `%s` has quest (ID: %u) in SCRIPT_COMMAND_QUEST_EXPLORED in `datalong` for script id %u, but quest not have flag QUEST_TRINITY_FLAGS_EXPLORATION_OR_EVENT in quest flags. Script command or quest flags wrong. Quest modified to require objective.",tablename,tmp.datalong,tmp.id);

                    // this will prevent quest completing without objective
                    const_cast<Quest*>(quest)->SetFlag(QUEST_TRINITY_FLAGS_EXPLORATION_OR_EVENT);

                    // continue; - quest objective requirement set and command can be allowed
                }

                if(float(tmp.datalong2) > DEFAULT_VISIBILITY_DISTANCE)
                {
                    TC_LOG_ERROR("sql.sql","Table `%s` has too large distance (%u) for exploring objective complete in `datalong2` in SCRIPT_COMMAND_QUEST_EXPLORED in `datalong` for script id %u",
                        tablename,tmp.datalong2,tmp.id);
                    continue;
                }

                if(tmp.datalong2 && float(tmp.datalong2) > DEFAULT_VISIBILITY_DISTANCE)
                {
                    TC_LOG_ERROR("sql.sql","Table `%s` has too large distance (%u) for exploring objective complete in `datalong2` in SCRIPT_COMMAND_QUEST_EXPLORED in `datalong` for script id %u, max distance is %f or 0 for disable distance check",
                        tablename,tmp.datalong2,tmp.id,DEFAULT_VISIBILITY_DISTANCE);
                    continue;
                }

                if(tmp.datalong2 && float(tmp.datalong2) < INTERACTION_DISTANCE)
                {
                    TC_LOG_ERROR("sql.sql","Table `%s` has too small distance (%u) for exploring objective complete in `datalong2` in SCRIPT_COMMAND_QUEST_EXPLORED in `datalong` for script id %u, min distance is %f or 0 for disable distance check",
                        tablename,tmp.datalong2,tmp.id,INTERACTION_DISTANCE);
                    continue;
                }

                break;
            }

            case SCRIPT_COMMAND_REMOVE_AURA:
            case SCRIPT_COMMAND_CAST_SPELL:
            {
                if(!sSpellMgr->GetSpellInfo(tmp.datalong))
                {
                    TC_LOG_ERROR("sql.sql","Table `%s` using non-existent spell (id: %u) in SCRIPT_COMMAND_REMOVE_AURA or SCRIPT_COMMAND_CAST_SPELL for script id %u",
                        tablename,tmp.datalong,tmp.id);
                    continue;
                }
                break;
            }
        }

        if (scripts.find(tmp.id) == scripts.end())
        {
            ScriptMap emptyMap;
            scripts[tmp.id] = emptyMap;
        }
        scripts[tmp.id].insert(std::pair<uint32, ScriptInfo>(tmp.delay, tmp));

        ++count;
    } while( result->NextRow() );

    TC_LOG_INFO("server.loading", ">> Loaded %u script definitions", count );
    
}

void ObjectMgr::LoadGameObjectScripts()
{
    LoadScripts(sGameObjectScripts,    "gameobject_scripts");

    // check ids
    for(ScriptMapMap::const_iterator itr = sGameObjectScripts.begin(); itr != sGameObjectScripts.end(); ++itr)
    {
        if(!GetGOData(itr->first))
            TC_LOG_ERROR("sql.sql","Table `gameobject_scripts` has not existing gameobject (GUID: %u) as script id",itr->first);
    }
}

void ObjectMgr::LoadQuestEndScripts()
{
    LoadScripts(sQuestEndScripts,  "quest_end_scripts");

    // check ids
    for(ScriptMapMap::const_iterator itr = sQuestEndScripts.begin(); itr != sQuestEndScripts.end(); ++itr)
    {
        if(!GetQuestTemplate(itr->first))
            TC_LOG_ERROR("sql.sql","Table `quest_end_scripts` has not existing quest (Id: %u) as script id",itr->first);
    }
}

void ObjectMgr::LoadQuestStartScripts()
{
    LoadScripts(sQuestStartScripts,"quest_start_scripts");

    // check ids
    for(ScriptMapMap::const_iterator itr = sQuestStartScripts.begin(); itr != sQuestStartScripts.end(); ++itr)
    {
        if(!GetQuestTemplate(itr->first))
            TC_LOG_ERROR("sql.sql","Table `quest_start_scripts` has not existing quest (Id: %u) as script id",itr->first);
    }
}

void ObjectMgr::LoadSpellScripts()
{
    LoadScripts(sSpellScripts, "spell_scripts");

    // check ids
    for(ScriptMapMap::const_iterator itr = sSpellScripts.begin(); itr != sSpellScripts.end(); ++itr)
    {
        SpellEntry const* spellInfo = sSpellMgr->GetSpellInfo(itr->first);

        if(!spellInfo)
        {
            TC_LOG_ERROR("sql.sql","Table `spell_scripts` has not existing spell (Id: %u) as script id",itr->first);
            continue;
        }

        //check for correct spellEffect
        bool found = false;
        for(int i=0; i<3; ++i)
        {
            // skip empty effects
            if( !spellInfo->Effect[i] )
                continue;

            if( spellInfo->Effect[i] == SPELL_EFFECT_SCRIPT_EFFECT )
            {
                found =  true;
                break;
            }
        }

        if(!found)
            TC_LOG_ERROR("sql.sql","Table `spell_scripts` has unsupported spell (Id: %u) without SPELL_EFFECT_SCRIPT_EFFECT (%u) spell effect",itr->first,SPELL_EFFECT_SCRIPT_EFFECT);
    }
}

void ObjectMgr::LoadEventScripts()
{
    LoadScripts(sEventScripts, "event_scripts");

    std::set<uint32> evt_scripts;
    // Load all possible script entries from gameobjects
    GameObjectTemplateContainer const* gotc = sObjectMgr->GetGameObjectTemplateStore();
    for (auto itr = gotc->begin(); itr != gotc->end(); ++itr)
    {
        if(GameObjectTemplate const* goInfo = &(itr->second))
        {
            switch(goInfo->type)
            {
                case GAMEOBJECT_TYPE_GOOBER:
                    if(goInfo->goober.eventId)
                        evt_scripts.insert(goInfo->goober.eventId);
                    break;
                case GAMEOBJECT_TYPE_CHEST:
                    if(goInfo->chest.eventId)
                        evt_scripts.insert(goInfo->chest.eventId);
                    break;
                default:
                    break;
            }
        }
    }
    // Load all possible script entries from spells
    //for(uint32 i = 1; i < sSpellStore.GetNumRows(); ++i)
    for (std::map<uint32, SpellEntry*>::iterator itr = sObjectMgr->GetSpellStore()->begin(); itr != sObjectMgr->GetSpellStore()->end(); itr++)
    {
        uint32 i = itr->first;
        SpellEntry const * spell = sSpellMgr->GetSpellInfo(i);
        if (spell)
        {
            for(int j=0; j<3; ++j)
            {
                if( spell->Effect[j] == SPELL_EFFECT_SEND_EVENT )
                {
                    if (spell->EffectMiscValue[j])
                        evt_scripts.insert(spell->EffectMiscValue[j]);
                }
            }
        }
    }
    // Then check if all scripts are in above list of possible script entries
    for(ScriptMapMap::const_iterator itr = sEventScripts.begin(); itr != sEventScripts.end(); ++itr)
    {
        std::set<uint32>::const_iterator itr2 = evt_scripts.find(itr->first);
        if (itr2 == evt_scripts.end())
            TC_LOG_ERROR("sql.sql","Table `event_scripts` has script (Id: %u) not referring to any gameobject_template type 10 data2 field or type 3 data6 field or any spell effect %u", itr->first, SPELL_EFFECT_SEND_EVENT);
    }
}

//Load WP Scripts
void ObjectMgr::LoadWaypointScripts()
{
    LoadScripts(sWaypointScripts, "waypoint_scripts");

    for(ScriptMapMap::const_iterator itr = sWaypointScripts.begin(); itr != sWaypointScripts.end(); ++itr)
    {
        QueryResult query = WorldDatabase.PQuery("SELECT * FROM `waypoint_scripts` WHERE `id` = %u", itr->first);
        if(query)
        {
            if (query->GetRowCount()) 
                continue;
        } //no query or no row count
        TC_LOG_ERROR("sql.sql","There is no waypoint which links to the waypoint script %u", itr->first);
    }
}

void ObjectMgr::LoadItemTexts()
{
    QueryResult result = CharacterDatabase.Query("SELECT id, text FROM item_text");

    uint32 count = 0;

    if( !result )
    {
        TC_LOG_INFO("server.loading", ">> Empty table item_text, loaded 0 item pages" );
        
        return;
    }

    Field* fields;
    do
    {
        fields = result->Fetch();

        mItemTexts[ fields[0].GetUInt32() ] = fields[1].GetString();

        ++count;

    } while ( result->NextRow() );

    TC_LOG_INFO("server.loading", ">> Loaded %u item texts", count );
    
}

void ObjectMgr::LoadPageTexts()
{
    uint32 oldMSTime = GetMSTime();

    //                                                 0      1       2
    QueryResult result = WorldDatabase.Query("SELECT entry, text, next_page FROM page_text");

    if (!result)
    {
        TC_LOG_INFO("server.loading", ">> Loaded 0 page texts. DB table `page_text` is empty!");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        PageText& pageText = _pageTextStore[fields[0].GetUInt32()];

        pageText.Text     = fields[1].GetString();
        pageText.NextPage = fields[2].GetUInt32();

        ++count;
    }
    while (result->NextRow());

    for (PageTextContainer::const_iterator itr = _pageTextStore.begin(); itr != _pageTextStore.end(); ++itr)
    {
        if (itr->second.NextPage)
        {
            PageTextContainer::const_iterator itr2 = _pageTextStore.find(itr->second.NextPage);
            if (itr2 == _pageTextStore.end())
                TC_LOG_ERROR("sql.sql", "Page text (Id: %u) has not existing next page (Id: %u)", itr->first, itr->second.NextPage);

        }
    }

    TC_LOG_INFO("server.loading", ">> Loaded %u page texts in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

PageText const* ObjectMgr::GetPageText(uint32 pageEntry)
{
    PageTextContainer::const_iterator itr = _pageTextStore.find(pageEntry);
    if (itr != _pageTextStore.end())
        return &(itr->second);

    return NULL;
}

void ObjectMgr::LoadPageTextLocales()
{
    uint32 oldMSTime = GetMSTime();

    mPageTextLocaleMap.clear();                             // need for reload case

    QueryResult result = WorldDatabase.Query("SELECT entry,text_loc1,text_loc2,text_loc3,text_loc4,text_loc5,text_loc6,text_loc7,text_loc8 FROM locales_page_text");

    if(!result)
    {
        TC_LOG_INFO("server.loading",">> Loaded 0 PageText locale strings. DB table `locales_page_text` is empty.");
        
        return;
    }

    do
    {
        Field *fields = result->Fetch();

        uint32 entry = fields[0].GetUInt32();

        PageTextLocale& data = mPageTextLocaleMap[entry];

        for (uint8 i = TOTAL_LOCALES - 1; i > 0; --i)
            AddLocaleString(fields[i].GetString(), LocaleConstant(i), data.Text);

    } while (result->NextRow());

    TC_LOG_INFO("server.loading", ">> Loaded " UI64FMTD " PageText locale strings in %u ms", mPageTextLocaleMap.size(), GetMSTimeDiffToNow(oldMSTime));
    
}

void ObjectMgr::LoadInstanceTemplate()
{
    uint32 oldMSTime = GetMSTime();
    //                                                0     1         2           4            5          6         7             8         9       10
    QueryResult result = WorldDatabase.Query("SELECT map, parent, maxPlayers, reset_delay, access_id, startLocX, startLocY, startLocZ, startLocO, script FROM instance_template");

    if (!result)
    {
        TC_LOG_INFO("server.loading", ">> Loaded 0 instance templates. DB table `instance_template` is empty!");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        uint16 mapID = fields[0].GetUInt16();

        if (!MapManager::IsValidMAP(mapID, true))
        {
            TC_LOG_ERROR("sql.sql", "ObjectMgr::LoadInstanceTemplate: bad mapid %d for template!", mapID);
            continue;
        }

        InstanceTemplate instanceTemplate;

        instanceTemplate.parent = fields[1].GetUInt32();
        instanceTemplate.maxPlayers = fields[2].GetUInt32();
        instanceTemplate.reset_delay = fields[3].GetUInt32();
        instanceTemplate.access_id = fields[4].GetUInt32();
        instanceTemplate.startLocX = fields[5].GetFloat();
        instanceTemplate.startLocY = fields[6].GetFloat();
        instanceTemplate.startLocZ = fields[7].GetFloat();
        instanceTemplate.startLocO = fields[8].GetFloat();
        instanceTemplate.script_id = sObjectMgr->GetScriptId(fields[9].GetCString());

        _instanceTemplateStore[mapID] = instanceTemplate;

        ++count;
    }
    while (result->NextRow());

    //correctness check
    for(auto i : _instanceTemplateStore)
    {
        InstanceTemplate* temp = &(i.second);
        if(!temp) 
            continue;

        const MapEntry* entry = sMapStore.LookupEntry(i.first);
        if(!entry)
            continue;  //should never happen if sMapStore isn't altered elswhere
        
        if(!entry->HasResetTime())
            continue;

        if(temp->reset_delay == 0)
        {
            // use defaults from the DBC
            if(entry->resetTimeHeroic)
            {
                temp->reset_delay = entry->resetTimeHeroic / DAY;
            }
            else if (entry->resetTimeRaid && entry->map_type == MAP_RAID)
            {
                temp->reset_delay = entry->resetTimeRaid / DAY;
            }
        }

        // the reset_delay must be at least one day
        temp->reset_delay = std::max((uint32)1, (uint32)(temp->reset_delay * sWorld->GetRate(RATE_INSTANCE_RESET_TIME)));
    }

    TC_LOG_INFO("server.loading", ">> Loaded " UI64FMTD " Instance Template definitions in %u ms", _instanceTemplateStore.size(), GetMSTimeDiffToNow(oldMSTime));
}

InstanceTemplate const* ObjectMgr::GetInstanceTemplate(uint32 mapID)
{
    InstanceTemplateContainer::const_iterator itr = _instanceTemplateStore.find(uint16(mapID));
    if (itr != _instanceTemplateStore.end())
        return &(itr->second);

    return NULL;
}

void ObjectMgr::LoadInstanceTemplateAddon()
{
    uint32 oldMSTime = GetMSTime();    
    QueryResult result = WorldDatabase.Query("SELECT map, forceHeroicEnabled FROM instance_template_addon");

    if (!result)
    {
        TC_LOG_INFO("server.loading", ">> Loaded 0 instance templates addon. DB table `instance_template_addon` is empty!");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        uint16 mapID = fields[0].GetUInt16();

        if (!MapManager::IsValidMAP(mapID, false))
        {
            TC_LOG_ERROR("sql.sql", "ObjectMgr::LoadInstanceTemplateAddon: bad mapid %d for template!", mapID);
            continue;
        }

        InstanceTemplate const* iTemp = GetInstanceTemplate(mapID);
        if (!iTemp)
        {
            TC_LOG_ERROR("sql.sql", "ObjectMgr::LoadInstanceTemplateAddon: bad mapid %d for template!", mapID);
            continue;
        }

        InstanceTemplateAddon instanceTemplateAddon;

        instanceTemplateAddon.forceHeroicEnabled = fields[1].GetBool();

        _instanceTemplateAddonStore[mapID] = instanceTemplateAddon;

        ++count;
    }
    while (result->NextRow());

    for(auto i : _instanceTemplateAddonStore)
    {
        const InstanceTemplateAddon* tempAddon = &(i.second);
        if(!tempAddon) 
            continue;
        if(!tempAddon->forceHeroicEnabled) 
            continue; //entry disabled

        InstanceTemplate* temp = (InstanceTemplate*)GetInstanceTemplate(i.first);
        if(!temp)
        {
            TC_LOG_ERROR("sql.sql","ObjectMgr::LoadInstanceTemplateAddon: bad mapid %d for template!", tempAddon->map);
            continue;
        }
        
        if(temp->reset_delay == 0) // instance has no reset delay, defaulting to 1 day
            temp->reset_delay = 1;
    }
    TC_LOG_INFO("server.loading", ">> Loaded " UI64FMTD " Instance Template Addons definitions in %u ms", _instanceTemplateAddonStore.size(), GetMSTimeDiffToNow(oldMSTime));
    
}

InstanceTemplateAddon const* ObjectMgr::GetInstanceTemplateAddon(uint32 mapID)
{
    InstanceTemplateAddonContainer::const_iterator itr = _instanceTemplateAddonStore.find(uint16(mapID));
    if (itr != _instanceTemplateAddonStore.end())
        return &(itr->second);

    return NULL;
}

void ObjectMgr::AddGossipText(GossipText *pGText)
{
    ASSERT( pGText->Text_ID );
    ASSERT( mGossipText.find(pGText->Text_ID) == mGossipText.end() );
    mGossipText[pGText->Text_ID] = pGText;
}

GossipText *ObjectMgr::GetGossipText(uint32 Text_ID)
{
    GossipTextMap::const_iterator itr;
    for (itr = mGossipText.begin(); itr != mGossipText.end(); ++itr)
    {
        if(itr->second->Text_ID == Text_ID)
            return itr->second;
    }
    return NULL;
}

void ObjectMgr::LoadGossipText()
{
    GossipText *pGText;
    QueryResult result = WorldDatabase.Query( "SELECT * FROM npc_text" );

    int count = 0;
    if( !result )
    {
        TC_LOG_INFO("server.loading", ">> Loaded %u npc texts", count );
        
        return;
    }

    int cic;

    do
    {
        ++count;
        cic = 0;

        Field *fields = result->Fetch();

        pGText = new GossipText;
        pGText->Text_ID    = fields[cic++].GetUInt32();

        for (int i=0; i< 8; i++)
        {
            pGText->Options[i].Text_0           = fields[cic++].GetString();
            pGText->Options[i].Text_1           = fields[cic++].GetString();

            pGText->Options[i].Language         = fields[cic++].GetUInt32();
            pGText->Options[i].Probability      = fields[cic++].GetFloat();

            pGText->Options[i].Emotes[0]._Delay  = fields[cic++].GetUInt32();
            pGText->Options[i].Emotes[0]._Emote  = fields[cic++].GetUInt32();

            pGText->Options[i].Emotes[1]._Delay  = fields[cic++].GetUInt32();
            pGText->Options[i].Emotes[1]._Emote  = fields[cic++].GetUInt32();

            pGText->Options[i].Emotes[2]._Delay  = fields[cic++].GetUInt32();
            pGText->Options[i].Emotes[2]._Emote  = fields[cic++].GetUInt32();
        }

        if ( !pGText->Text_ID ){
          delete pGText;
          continue;
        }

        AddGossipText( pGText );

    } while( result->NextRow() );

    TC_LOG_INFO("server.loading", ">> Loaded %u npc texts", count );
    
}

void ObjectMgr::LoadNpcTextLocales()
{
    mNpcTextLocaleMap.clear();                              // need for reload case

    QueryResult result = WorldDatabase.Query("SELECT entry,"
        "Text0_0_loc1,Text0_1_loc1,Text1_0_loc1,Text1_1_loc1,Text2_0_loc1,Text2_1_loc1,Text3_0_loc1,Text3_1_loc1,Text4_0_loc1,Text4_1_loc1,Text5_0_loc1,Text5_1_loc1,Text6_0_loc1,Text6_1_loc1,Text7_0_loc1,Text7_1_loc1,"
        "Text0_0_loc2,Text0_1_loc2,Text1_0_loc2,Text1_1_loc2,Text2_0_loc2,Text2_1_loc2,Text3_0_loc2,Text3_1_loc1,Text4_0_loc2,Text4_1_loc2,Text5_0_loc2,Text5_1_loc2,Text6_0_loc2,Text6_1_loc2,Text7_0_loc2,Text7_1_loc2,"
        "Text0_0_loc3,Text0_1_loc3,Text1_0_loc3,Text1_1_loc3,Text2_0_loc3,Text2_1_loc3,Text3_0_loc3,Text3_1_loc1,Text4_0_loc3,Text4_1_loc3,Text5_0_loc3,Text5_1_loc3,Text6_0_loc3,Text6_1_loc3,Text7_0_loc3,Text7_1_loc3,"
        "Text0_0_loc4,Text0_1_loc4,Text1_0_loc4,Text1_1_loc4,Text2_0_loc4,Text2_1_loc4,Text3_0_loc4,Text3_1_loc1,Text4_0_loc4,Text4_1_loc4,Text5_0_loc4,Text5_1_loc4,Text6_0_loc4,Text6_1_loc4,Text7_0_loc4,Text7_1_loc4,"
        "Text0_0_loc5,Text0_1_loc5,Text1_0_loc5,Text1_1_loc5,Text2_0_loc5,Text2_1_loc5,Text3_0_loc5,Text3_1_loc1,Text4_0_loc5,Text4_1_loc5,Text5_0_loc5,Text5_1_loc5,Text6_0_loc5,Text6_1_loc5,Text7_0_loc5,Text7_1_loc5,"
        "Text0_0_loc6,Text0_1_loc6,Text1_0_loc6,Text1_1_loc6,Text2_0_loc6,Text2_1_loc6,Text3_0_loc6,Text3_1_loc1,Text4_0_loc6,Text4_1_loc6,Text5_0_loc6,Text5_1_loc6,Text6_0_loc6,Text6_1_loc6,Text7_0_loc6,Text7_1_loc6,"
        "Text0_0_loc7,Text0_1_loc7,Text1_0_loc7,Text1_1_loc7,Text2_0_loc7,Text2_1_loc7,Text3_0_loc7,Text3_1_loc1,Text4_0_loc7,Text4_1_loc7,Text5_0_loc7,Text5_1_loc7,Text6_0_loc7,Text6_1_loc7,Text7_0_loc7,Text7_1_loc7, "
        "Text0_0_loc8,Text0_1_loc8,Text1_0_loc8,Text1_1_loc8,Text2_0_loc8,Text2_1_loc8,Text3_0_loc8,Text3_1_loc1,Text4_0_loc8,Text4_1_loc8,Text5_0_loc8,Text5_1_loc8,Text6_0_loc8,Text6_1_loc8,Text7_0_loc8,Text7_1_loc8 "
        " FROM locales_npc_text");

    if(!result)
    {
        TC_LOG_INFO("server.loading",">> Loaded 0 Quest locale strings. DB table `locales_npc_text` is empty.");
        
        return;
    }

    do
    {
        Field *fields = result->Fetch();

        uint32 entry = fields[0].GetUInt32();

        NpcTextLocale& data = mNpcTextLocaleMap[entry];

        for (uint8 i = MAX_LOCALE; i > 0; --i)
        {
            LocaleConstant locale = (LocaleConstant) i;
            for (uint8 j = 0; j < MAX_GOSSIP_TEXT_OPTIONS; ++j)
            {
                AddLocaleString(fields[1 + 8 * 2 * (i - 1) + 2 * j].GetString(), locale, data.Text_0[j]);
                AddLocaleString(fields[1 + 8 * 2 * (i - 1) + 2 * j + 1].GetString(), locale, data.Text_1[j]);
            }
        }
    } while (result->NextRow());

    TC_LOG_INFO("server.loading", ">> Loaded " UI64FMTD " NpcText locale strings", mNpcTextLocaleMap.size());
    
}

//not very fast function but it is called only once a day, or on starting-up
void ObjectMgr::ReturnOrDeleteOldMails(bool serverUp)
{
    time_t basetime = time(NULL);
    //delete all old mails without item and without body immediately, if starting server
    if (!serverUp)
        CharacterDatabase.PExecute("DELETE FROM mail WHERE expire_time < '" UI64FMTD "' AND has_items = '0' AND itemTextId = 0", (uint64)basetime);
    //                                                     0  1           2      3        4          5         6           7   8       9
    QueryResult result = CharacterDatabase.PQuery("SELECT id,messageType,sender,receiver,itemTextId,has_items,expire_time,cod,checked,mailTemplateId FROM mail WHERE expire_time < '" UI64FMTD "'", (uint64)basetime);
    if ( !result )
        return;                                             // any mails need to be returned or deleted
    Field *fields;
    //std::ostringstream delitems, delmails; //will be here for optimization
    //bool deletemail = false, deleteitem = false;
    //delitems << "DELETE FROM item_instance WHERE guid IN ( ";
    //delmails << "DELETE FROM mail WHERE id IN ( "
    do
    {
        fields = result->Fetch();
        Mail *m = new Mail;
        m->messageID = fields[0].GetUInt32();
        m->messageType = fields[1].GetUInt8();
        m->sender = fields[2].GetUInt32();
        m->receiver = fields[3].GetUInt32();
        m->itemTextId = fields[4].GetUInt32();
        bool has_items = fields[5].GetBool();
        m->expire_time = (time_t)fields[6].GetUInt64();
        m->deliver_time = 0;
        m->COD = fields[7].GetUInt32();
        m->checked = fields[8].GetUInt32();
        m->mailTemplateId = fields[9].GetInt16();

        Player *pl = 0;
        if (serverUp)
            pl = GetPlayer((uint64)m->receiver);
        if (pl && pl->m_mailsLoaded)
        {                                                   //this code will run very improbably (the time is between 4 and 5 am, in game is online a player, who has old mail
            //his in mailbox and he has already listed his mails )
            delete m;
            continue;
        }
        //delete or return mail:
        if (has_items)
        {
            QueryResult resultItems = CharacterDatabase.PQuery("SELECT item_guid,item_template FROM mail_items WHERE mail_id='%u'", m->messageID);
            if(resultItems)
            {
                do
                {
                    Field *fields2 = resultItems->Fetch();

                    uint32 item_guid_low = fields2[0].GetUInt32();
                    uint32 item_template = fields2[1].GetUInt32();

                    m->AddItem(item_guid_low, item_template);
                }
                while (resultItems->NextRow());
            }
            //if it is mail from AH, it shouldn't be returned, but deleted
            if (m->messageType != MAIL_NORMAL || (m->checked & (MAIL_CHECK_MASK_AUCTION | MAIL_CHECK_MASK_COD_PAYMENT | MAIL_CHECK_MASK_RETURNED)))
            {
                // mail open and then not returned
                for(std::vector<MailItemInfo>::iterator itr2 = m->items.begin(); itr2 != m->items.end(); ++itr2)
                    CharacterDatabase.PExecute("DELETE FROM item_instance WHERE guid = '%u'", itr2->item_guid);
            }
            else
            {
                //mail will be returned:
                CharacterDatabase.PExecute("UPDATE mail SET sender = '%u', receiver = '%u', expire_time = '" UI64FMTD "', deliver_time = '" UI64FMTD "',cod = '0', checked = '%u' WHERE id = '%u'", m->receiver, m->sender, (uint64)(basetime + 30*DAY), (uint64)basetime, MAIL_CHECK_MASK_RETURNED, m->messageID);
                delete m;
                continue;
            }
        }

        if (m->itemTextId)
            CharacterDatabase.PExecute("DELETE FROM item_text WHERE id = '%u'", m->itemTextId);

        //deletemail = true;
        //delmails << m->messageID << ", ";
        CharacterDatabase.PExecute("DELETE FROM mail WHERE id = '%u'", m->messageID);
        delete m;
    } while (result->NextRow());
}

void ObjectMgr::LoadQuestAreaTriggers()
{
    mQuestAreaTriggerMap.clear();                           // need for reload case

    QueryResult result = WorldDatabase.Query( "SELECT id,quest FROM areatrigger_involvedrelation" );

    uint32 count = 0;

    if( !result )
    {
        TC_LOG_INFO("server.loading", ">> Loaded %u quest trigger points", count );
        
        return;
    }

    do
    {
        ++count;

        Field *fields = result->Fetch();

        uint32 trigger_ID = fields[0].GetUInt32();
        uint32 quest_ID   = fields[1].GetUInt32();

        AreaTriggerEntry const* atEntry = sAreaTriggerStore.LookupEntry(trigger_ID);
        if(!atEntry)
        {
            TC_LOG_ERROR("sql.sql","Area trigger (ID:%u) does not exist in `AreaTrigger.dbc`.",trigger_ID);
            continue;
        }

        Quest const* quest = GetQuestTemplate(quest_ID);

        if(!quest)
        {
            TC_LOG_ERROR("sql.sql","Table `areatrigger_involvedrelation` has record (id: %u) for not existing quest %u",trigger_ID,quest_ID);
            continue;
        }

        if(!quest->HasFlag(QUEST_TRINITY_FLAGS_EXPLORATION_OR_EVENT))
        {
            TC_LOG_ERROR("sql.sql","Table `areatrigger_involvedrelation` has record (id: %u) for not quest %u, but quest not have flag QUEST_TRINITY_FLAGS_EXPLORATION_OR_EVENT. Trigger or quest flags must be fixed, quest modified to require objective.",trigger_ID,quest_ID);

            // this will prevent quest completing without objective
            const_cast<Quest*>(quest)->SetFlag(QUEST_TRINITY_FLAGS_EXPLORATION_OR_EVENT);

            // continue; - quest modified to required objective and trigger can be allowed.
        }

        mQuestAreaTriggerMap[trigger_ID] = quest_ID;

    } while( result->NextRow() );

    TC_LOG_INFO("server.loading", ">> Loaded %u quest trigger points", count );
    
}

void ObjectMgr::LoadTavernAreaTriggers()
{
    mTavernAreaTriggerSet.clear();                          // need for reload case

    QueryResult result = WorldDatabase.Query("SELECT id FROM areatrigger_tavern");

    uint32 count = 0;

    if( !result )
    {
        TC_LOG_INFO("server.loading", ">> Loaded %u tavern triggers", count );
        
        return;
    }

    do
    {
        ++count;

        Field *fields = result->Fetch();

        uint32 Trigger_ID      = fields[0].GetUInt32();

        AreaTriggerEntry const* atEntry = sAreaTriggerStore.LookupEntry(Trigger_ID);
        if(!atEntry)
        {
            TC_LOG_ERROR("FIXME","Area trigger (ID:%u) does not exist in `AreaTrigger.dbc`.",Trigger_ID);
            continue;
        }

        mTavernAreaTriggerSet.insert(Trigger_ID);
    } while( result->NextRow() );

    TC_LOG_INFO("server.loading", ">> Loaded %u tavern triggers", count );
    
}

void ObjectMgr::LoadAreaTriggerScripts()
{
    mAreaTriggerScripts.clear();                            // need for reload case
    QueryResult result = WorldDatabase.Query("SELECT entry, ScriptName FROM areatrigger_scripts");

    uint32 count = 0;

    if( !result )
    {
        TC_LOG_INFO("server.loading", ">> Loaded %u areatrigger scripts", count );
        
        return;
    }

    do
    {
        ++count;

        Field *fields = result->Fetch();

        uint32 Trigger_ID      = fields[0].GetUInt32();
        const char *scriptName = fields[1].GetCString();

        AreaTriggerEntry const* atEntry = sAreaTriggerStore.LookupEntry(Trigger_ID);
        if(!atEntry)
        {
            TC_LOG_ERROR("FIXME","Area trigger (ID:%u) does not exist in `AreaTrigger.dbc`.",Trigger_ID);
            continue;
        }
        mAreaTriggerScripts[Trigger_ID] = GetScriptId(scriptName);
    } while( result->NextRow() );

    TC_LOG_INFO("server.loading", ">> Loaded %u areatrigger scripts", count );
    
}

uint32 ObjectMgr::GetNearestTaxiNode(float x, float y, float z, uint32 mapid, uint32 team)
{
    bool found = false;
    float dist = 10000;
    uint32 id = 0;

    for (uint32 i = 1; i < sTaxiNodesStore.GetNumRows(); ++i)
    {
        TaxiNodesEntry const* node = sTaxiNodesStore.LookupEntry(i);

        if (!node || node->map_id != mapid || (!node->MountCreatureID[team == TEAM_ALLIANCE ? 1 : 0] && node->MountCreatureID[0] != 32981)) // dk flight
            continue;

        uint8  field   = (uint8)((i - 1) / 32);
        uint32 submask = 1<<((i-1)%32);

        // skip not taxi network nodes
        if ((sTaxiNodesMask[field] & submask) == 0)
            continue;

        float dist2 = (node->x - x)*(node->x - x)+(node->y - y)*(node->y - y)+(node->z - z)*(node->z - z);
        if (found)
        {
            if (dist2 < dist)
            {
                dist = dist2;
                id = i;
            }
        }
        else
        {
            found = true;
            dist = dist2;
            id = i;
        }
    }

    return id;
}

void ObjectMgr::GetTaxiPath( uint32 source, uint32 destination, uint32 &path, uint32 &cost)
{
    TaxiPathSetBySource::iterator src_i = sTaxiPathSetBySource.find(source);
    if(src_i==sTaxiPathSetBySource.end())
    {
        path = 0;
        cost = 0;
        return;
    }

    TaxiPathSetForSource& pathSet = src_i->second;

    TaxiPathSetForSource::iterator dest_i = pathSet.find(destination);
    if(dest_i==pathSet.end())
    {
        path = 0;
        cost = 0;
        return;
    }

    cost = dest_i->second.price;
    path = dest_i->second.ID;
}

uint32 ObjectMgr::GetTaxiMountDisplayId(uint32 id, uint32 team, bool allowed_alt_team /* = false */)
{
    uint32 mount_id = 0;

    // select mount creature id
    TaxiNodesEntry const* node = sTaxiNodesStore.LookupEntry(id);
    if (node)
    {
        uint32 mount_entry = 0;
        if (team == TEAM_ALLIANCE)
            mount_entry = node->MountCreatureID[1];
        else
            mount_entry = node->MountCreatureID[0];

        // Fix for Alliance not being able to use Acherus taxi
        // only one mount type for both sides
        if (mount_entry == 0 && allowed_alt_team)
        {
            // Simply reverse the selection. At least one team in theory should have a valid mount ID to choose.
            mount_entry = team == TEAM_ALLIANCE ? node->MountCreatureID[0] : node->MountCreatureID[1];
        }

        CreatureTemplate const* mount_info = GetCreatureTemplate(mount_entry);
        if (mount_info)
        {
            mount_id = mount_info->GetRandomValidModelId();
            if (!mount_id)
            {
                TC_LOG_ERROR("sql.sql", "No displayid found for the taxi mount with the entry %u! Can't load it!", mount_entry);
                return 0;
            }
        }
    }

    // minfo is not actually used but the mount_id was updated
    GetCreatureModelRandomGender(&mount_id);

    return mount_id;
}

void ObjectMgr::LoadGraveyardZones()
{
    mGraveYardMap.clear();                                  // need for reload case

    QueryResult result = WorldDatabase.Query("SELECT id,ghost_zone,faction FROM game_graveyard_zone");

    uint32 count = 0;

    if( !result )
    {
        TC_LOG_INFO("server.loading", ">> Loaded %u graveyard-zone links", count );
        
        return;
    }

    do
    {
        ++count;

        Field *fields = result->Fetch();

        uint32 safeLocId = fields[0].GetUInt32();
        uint32 zoneId = fields[1].GetUInt32();
        uint32 team   = fields[2].GetUInt32();

        WorldSafeLocsEntry const* entry = sWorldSafeLocsStore.LookupEntry(safeLocId);
        if(!entry)
        {
            TC_LOG_ERROR("sql.sql","Table `game_graveyard_zone` has record for not existing graveyard (WorldSafeLocs.dbc id) %u, skipped.",safeLocId);
            continue;
        }

        AreaTableEntry const *areaEntry = GetAreaEntryByAreaID(zoneId);
        if(!areaEntry)
        {
            TC_LOG_ERROR("sql.sql","Table `game_graveyard_zone` has record for not existing zone id (%u), skipped.",zoneId);
            continue;
        }

        if(areaEntry->parentArea != 0)
        {
            TC_LOG_ERROR("sql.sql","Table `game_graveyard_zone` has record subzone id (%u) instead of zone, skipped.",zoneId);
            continue;
        }

        if(team!=0 && team!=TEAM_HORDE && team!=TEAM_ALLIANCE)
        {
            TC_LOG_ERROR("sql.sql","Table `game_graveyard_zone` has record for non player faction (%u), skipped.",team);
            continue;
        }

        if(!AddGraveYardLink(safeLocId,zoneId,team,false))
            TC_LOG_ERROR("sql.sql","Table `game_graveyard_zone` has a duplicate record for Graveyard (ID: %u) and Zone (ID: %u), skipped.",safeLocId,zoneId);
    } while( result->NextRow() );

    TC_LOG_INFO("server.loading", ">> Loaded %u graveyard-zone links", count );
    
}

WorldSafeLocsEntry const *ObjectMgr::GetClosestGraveYard(float x, float y, float z, uint32 MapId, uint32 team)
{
    // search for zone associated closest graveyard
    uint32 zoneId = sMapMgr->GetZoneId(MapId,x,y,z);

    // Simulate std. algorithm:
    //   found some graveyard associated to (ghost_zone,ghost_map)
    //
    //   if mapId == graveyard.mapId (ghost in plain zone or city or battleground) and search graveyard at same map
    //     then check faction
    //   if mapId != graveyard.mapId (ghost in instance) and search any graveyard associated
    //     then check faction
    GraveYardMap::const_iterator graveLow  = mGraveYardMap.lower_bound(zoneId);
    GraveYardMap::const_iterator graveUp   = mGraveYardMap.upper_bound(zoneId);
    if(graveLow==graveUp)
    {
        TC_LOG_ERROR("FIXME","Table `game_graveyard_zone` incomplete: Zone %u Team %u does not have a linked graveyard.",zoneId,team);
        return NULL;
    }

    // at corpse map
    bool foundNear = false;
    float distNear;
    WorldSafeLocsEntry const* entryNear = NULL;

    // at entrance map for corpse map
    bool foundEntr = false;
    float distEntr;
    WorldSafeLocsEntry const* entryEntr = NULL;

    // some where other
    WorldSafeLocsEntry const* entryFar = NULL;

    MapEntry const* mapEntry = sMapStore.LookupEntry(MapId);

    for(GraveYardMap::const_iterator itr = graveLow; itr != graveUp; ++itr)
    {
        GraveYardData const& data = itr->second;

        WorldSafeLocsEntry const* entry = sWorldSafeLocsStore.LookupEntry(data.safeLocId);
        if(!entry)
        {
            TC_LOG_ERROR("sql.sql","Table `game_graveyard_zone` has record for not existing graveyard (WorldSafeLocs.dbc id) %u, skipped.",data.safeLocId);
            continue;
        }

        // skip enemy faction graveyard
        // team == 0 case can be at call from .neargrave
        if(data.team != 0 && team != 0 && data.team != team)
            continue;

        // find now nearest graveyard at other map
        if(MapId != entry->map_id)
        {
            // if find graveyard at different map from where entrance placed (or no entrance data), use any first
            if (!mapEntry || mapEntry->entrance_map < 0 || mapEntry->entrance_map != entry->map_id ||
                mapEntry->entrance_x == 0 && mapEntry->entrance_y == 0)
            {
                // not have any corrdinates for check distance anyway
                entryFar = entry;
                continue;
            }

            // at entrance map calculate distance (2D);
            float dist2 = (entry->x - mapEntry->entrance_x)*(entry->x - mapEntry->entrance_x)
                +(entry->y - mapEntry->entrance_y)*(entry->y - mapEntry->entrance_y);
            if(foundEntr)
            {
                if(dist2 < distEntr)
                {
                    distEntr = dist2;
                    entryEntr = entry;
                }
            }
            else
            {
                foundEntr = true;
                distEntr = dist2;
                entryEntr = entry;
            }
        }
        // find now nearest graveyard at same map
        else
        {
            float dist2 = (entry->x - x)*(entry->x - x)+(entry->y - y)*(entry->y - y)+(entry->z - z)*(entry->z - z);
            if(foundNear)
            {
                if(dist2 < distNear)
                {
                    distNear = dist2;
                    entryNear = entry;
                }
            }
            else
            {
                foundNear = true;
                distNear = dist2;
                entryNear = entry;
            }
        }
    }

    if(entryNear)
        return entryNear;

    if(entryEntr)
        return entryEntr;

    return entryFar;
}

GraveYardData const* ObjectMgr::FindGraveYardData(uint32 id, uint32 zoneId)
{
    GraveYardMap::const_iterator graveLow  = mGraveYardMap.lower_bound(zoneId);
    GraveYardMap::const_iterator graveUp   = mGraveYardMap.upper_bound(zoneId);

    for(GraveYardMap::const_iterator itr = graveLow; itr != graveUp; ++itr)
    {
        if(itr->second.safeLocId==id)
            return &itr->second;
    }

    return NULL;
}

bool ObjectMgr::AddGraveYardLink(uint32 id, uint32 zoneId, uint32 team, bool inDB)
{
    if(FindGraveYardData(id,zoneId))
        return false;

    // add link to loaded data
    GraveYardData data;
    data.safeLocId = id;
    data.team = team;

    mGraveYardMap.insert(GraveYardMap::value_type(zoneId,data));

    // add link to DB
    if(inDB)
    {
        WorldDatabase.PExecute("INSERT INTO game_graveyard_zone ( id,ghost_zone,faction) "
            "VALUES ('%u', '%u','%u')",id,zoneId,team);
    }

    return true;
}

void ObjectMgr::RemoveGraveYardLink(uint32 id, uint32 zoneId, uint32 team, bool inDB)
{
    GraveYardMap::iterator graveLow  = mGraveYardMap.lower_bound(zoneId);
    GraveYardMap::iterator graveUp   = mGraveYardMap.upper_bound(zoneId);
    if(graveLow==graveUp)
    {
        //TC_LOG_ERROR("FIXME","Table `game_graveyard_zone` incomplete: Zone %u Team %u does not have a linked graveyard.",zoneId,team);
        return;
    }

    bool found = false;

    GraveYardMap::iterator itr;

    for(itr = graveLow; itr != graveUp; ++itr)
    {
        GraveYardData & data = itr->second;

        // skip not matching safezone id
        if(data.safeLocId != id)
            continue;

        // skip enemy faction graveyard at same map (normal area, city, or battleground)
        // team == 0 case can be at call from .neargrave
        if(data.team != 0 && team != 0 && data.team != team)
            continue;

        found = true;
        break;
    }

    // no match, return
    if(!found)
        return;

    // remove from links
    mGraveYardMap.erase(itr);

    // remove link from DB
    if(inDB)
    {
        WorldDatabase.PExecute("DELETE FROM game_graveyard_zone WHERE id = '%u' AND ghost_zone = '%u' AND faction = '%u'",id,zoneId,team);
    }

    return;
}


void ObjectMgr::LoadAreaTriggerTeleports()
{
    mAreaTriggers.clear();                                  // need for reload case

    uint32 count = 0;

    //                                                0       1           2              3               4                   5                   6  
    QueryResult result = WorldDatabase.Query("SELECT id, access_id, target_map, target_position_x, target_position_y, target_position_z, target_orientation FROM areatrigger_teleport");
    if( !result )
    {
        TC_LOG_INFO("server.loading", ">> Loaded %u area trigger teleport definitions", count );
        
        return;
    }

    do
    {
        Field *fields = result->Fetch();

        ++count;

        uint32 Trigger_ID = fields[0].GetUInt32();

        AreaTrigger at;

        at.access_id                = fields[1].GetUInt32();
        at.target_mapId             = fields[2].GetUInt32();
        at.target_X                 = fields[3].GetFloat();
        at.target_Y                 = fields[4].GetFloat();
        at.target_Z                 = fields[5].GetFloat();
        at.target_Orientation       = fields[6].GetFloat();

        AreaTriggerEntry const* atEntry = sAreaTriggerStore.LookupEntry(Trigger_ID);
        if(!atEntry)
        {
            TC_LOG_ERROR("sql.sql","Area trigger (ID:%u) does not exist in `AreaTrigger.dbc`.",Trigger_ID);
            continue;
        }
        
        MapEntry const* mapEntry = sMapStore.LookupEntry(at.target_mapId);
        if(!mapEntry)
        {
            TC_LOG_ERROR("sql.sql","Area trigger (ID:%u) target map (ID: %u) does not exist in `Map.dbc`.",Trigger_ID,at.target_mapId);
            continue;
        }

        if(at.target_X==0 && at.target_Y==0 && at.target_Z==0)
        {
            TC_LOG_ERROR("sql.sql","Area trigger (ID:%u) target coordinates not provided.",Trigger_ID);
            continue;
        }

        mAreaTriggers[Trigger_ID] = at;

    } while( result->NextRow() );

    TC_LOG_INFO("server.loading", ">> Loaded %u area trigger teleport definitions", count );
    
}

void ObjectMgr::LoadAccessRequirements()
{
    mAccessRequirements.clear();                                  // need for reload case

    uint32 count = 0;

    //                                                0       1          2       3      4        5           6             7              8                   9                  10
    QueryResult result = WorldDatabase.Query("SELECT id, level_min, level_max, item, item2, heroic_key, heroic_key2, quest_done, quest_failed_text, heroic_quest_done, heroic_quest_failed_text FROM access_requirement");
    if( !result )
    {
        TC_LOG_INFO("server.loading", ">> Loaded %u access requirement definitions", count );
        
        return;
    }

    do
    {
        Field *fields = result->Fetch();

        ++count;

        uint32 requiremt_ID = fields[0].GetUInt32();

        AccessRequirement ar;

        ar.levelMin                 = fields[1].GetUInt8();
        ar.levelMax                 = fields[2].GetUInt32();
        ar.item                     = fields[3].GetUInt32();
        ar.item2                    = fields[4].GetUInt32();
        ar.heroicKey                = fields[5].GetUInt32();
        ar.heroicKey2               = fields[6].GetUInt32();
        ar.quest                    = fields[7].GetUInt32();
        ar.questFailedText          = fields[8].GetUInt32();
        ar.heroicQuest              = fields[9].GetUInt32();
        ar.heroicQuestFailedText    = fields[10].GetUInt32();

        if(ar.item)
        {
            ItemTemplate const *pProto = GetItemTemplate(ar.item);
            if(!pProto)
            {
                TC_LOG_ERROR("sql.sql","Key item %u does not exist for requirement %u, removing key requirement.", ar.item, requiremt_ID);
                ar.item = 0;
            }
        }

        if(ar.item2)
        {
            ItemTemplate const *pProto = GetItemTemplate(ar.item2);
            if(!pProto)
            {
                TC_LOG_ERROR("sql.sql","Second item %u does not exist for requirement %u, removing key requirement.", ar.item2, requiremt_ID);
                ar.item2 = 0;
            }
        }

        if(ar.heroicKey)
        {
            ItemTemplate const *pProto = GetItemTemplate(ar.heroicKey);
            if(!pProto)
            {
                TC_LOG_ERROR("sql.sql","Heroic key %u not exist for trigger %u, remove key requirement.", ar.heroicKey, requiremt_ID);
                ar.heroicKey = 0;
            }
        }

        if(ar.heroicKey2)
        {
            ItemTemplate const *pProto = GetItemTemplate(ar.heroicKey2);
            if(!pProto)
            {
                TC_LOG_ERROR("sql.sql","Second heroic key %u not exist for trigger %u, remove key requirement.", ar.heroicKey2, requiremt_ID);
                ar.heroicKey2 = 0;
            }
        }

        if(ar.heroicQuest)
        {
            if(!GetQuestTemplate(ar.heroicQuest))
            {
                TC_LOG_ERROR("sql.sql","Required Heroic Quest %u not exist for trigger %u, remove heroic quest done requirement.",ar.heroicQuest,requiremt_ID);
                ar.heroicQuest = 0;
            }
        }

        if(ar.quest)
        {
            if(!GetQuestTemplate(ar.quest))
            {
                TC_LOG_ERROR("sql.sql","Required Quest %u not exist for trigger %u, remove quest done requirement.",ar.quest,requiremt_ID);
                ar.quest = 0;
            }
        }

        if(ar.questFailedText)
        {
            if(!GetTrinityStringForDBCLocale(ar.questFailedText))
                TC_LOG_ERROR("sql.sql","access_requirement - text %u does not exist.",ar.questFailedText);
        }

        if(ar.heroicQuestFailedText)
        {
            if(!GetTrinityStringForDBCLocale(ar.heroicQuestFailedText))
                TC_LOG_ERROR("sql.sql","access_requirement - text %u does not exist.",ar.questFailedText);
        }

        mAccessRequirements[requiremt_ID] = ar;

    } while( result->NextRow() );

    TC_LOG_INFO("server.loading", ">> Loaded %u access requirement definitions", count );
    
}

AreaTrigger const* ObjectMgr::GetGoBackTrigger(uint32 Map) const
{
    const MapEntry *mapEntry = sMapStore.LookupEntry(Map);
    if(!mapEntry) return NULL;
    for (AreaTriggerMap::const_iterator itr = mAreaTriggers.begin(); itr != mAreaTriggers.end(); ++itr)
    {
        if(itr->second.target_mapId == mapEntry->entrance_map)
        {
            AreaTriggerEntry const* atEntry = sAreaTriggerStore.LookupEntry(itr->first);
            if(atEntry && atEntry->mapid == Map)
                return &itr->second;
        }
    }
    return NULL;
}

/**
 * Searches for the areatrigger which teleports players to the given map
 */
AreaTrigger const* ObjectMgr::GetMapEntranceTrigger(uint32 Map) const
{
    for (AreaTriggerMap::const_iterator itr = mAreaTriggers.begin(); itr != mAreaTriggers.end(); ++itr)
    {
        if(itr->second.target_mapId == Map)
        {
            AreaTriggerEntry const* atEntry = sAreaTriggerStore.LookupEntry(itr->first);
            if(atEntry)
                return &itr->second;
        }
    }
    return NULL;
}

void ObjectMgr::SetHighestGuids()
{
    QueryResult result = CharacterDatabase.Query( "SELECT MAX(guid) FROM characters" );
    if( result )
    {
        m_hiCharGuid = (*result)[0].GetUInt32()+1;
    }

    result = WorldDatabase.Query( "SELECT MAX(guid) FROM creature" );
    if( result )
    {
        m_hiCreatureGuid = (*result)[0].GetUInt32()+1;
        uint32 proportion = sWorld->getConfig(CONFIG_GUIDDISTRIB_PROPORTION);
        if (proportion > 95 || proportion < 50)
        {
            TC_LOG_INFO("server.loading","Invalid GuidDistribution.Proportion in conf, setting to 90");
            proportion = 90;
        }
        m_hiTempCreatureGuidStart = m_hiCreatureGuid + ((0x00FFFFFF-m_hiCreatureGuid) * (100-proportion))/100;
        m_hiTempCreatureGuid = m_hiTempCreatureGuidStart;
        TC_LOG_INFO("server.loading","Temporary creatures guid range initialized at %u", m_hiTempCreatureGuid);
    } else m_hiCreatureRegularModeGuid = true;

    // pet guids are not saved to DB, set to 0 (pet guid != pet id)
    m_hiPetGuid = 0;

    result = CharacterDatabase.Query( "SELECT MAX(guid) FROM item_instance" );
    if( result )
    {
        m_hiItemGuid = (*result)[0].GetUInt32()+1;
    }

    // Cleanup other tables from not existed guids (>=m_hiItemGuid)
    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    trans->PAppend("DELETE FROM character_inventory WHERE item >= '%u'", m_hiItemGuid);
    trans->PAppend("DELETE FROM mail_items WHERE item_guid >= '%u'", m_hiItemGuid);
    trans->PAppend("DELETE FROM auctionhouse WHERE itemguid >= '%u'", m_hiItemGuid);
    trans->PAppend("DELETE FROM guild_bank_item WHERE item_guid >= '%u'", m_hiItemGuid);
    CharacterDatabase.CommitTransaction(trans);

    result = WorldDatabase.Query("SELECT MAX(guid) FROM gameobject" );
    if( result )
    {
        m_hiGoGuid = (*result)[0].GetUInt32()+1;
        uint32 proportion = sWorld->getConfig(CONFIG_GUIDDISTRIB_PROPORTION);
        if (proportion > 95 || proportion < 50)
        {
            TC_LOG_INFO("server.loading","Invalid GuidDistribution.Proportion in conf, setting to 90");
            proportion = 90;
        }
        m_hiTempGoGuidStart = m_hiGoGuid + ((0x00FFFFFF-m_hiGoGuid) * (100-proportion))/100;
        m_hiTempGoGuid = m_hiTempGoGuidStart;
        TC_LOG_INFO("server.loading","Temporary gameobjects guid range initialized at %u", m_hiTempGoGuid);
    } else m_hiGoRegularModeGuid = true;

    result = CharacterDatabase.Query("SELECT MAX(id) FROM auctionhouse" );
    if( result )
    {
        m_auctionid = (*result)[0].GetUInt32()+1;
    }

    result = CharacterDatabase.Query( "SELECT MAX(id) FROM mail" );
    if( result )
    {
        m_mailid = (*result)[0].GetUInt32()+1;
    }

    result = CharacterDatabase.Query( "SELECT MAX(id) FROM item_text" );
    if( result )
    {
        m_ItemTextId = (*result)[0].GetUInt32()+1;
    }

    result = CharacterDatabase.Query( "SELECT MAX(guid) FROM corpse" );
    if( result )
    {
        m_hiCorpseGuid = (*result)[0].GetUInt32()+1;
    }

    result = CharacterDatabase.Query("SELECT MAX(arenateamid) FROM arena_team");
    if (result)
    {
        m_arenaTeamId = (*result)[0].GetUInt32()+1;
    }

    result = CharacterDatabase.Query( "SELECT MAX(guildid) FROM guild" );
    if (result)
    {
        m_guildId = (*result)[0].GetUInt32()+1;
    }
}

uint32 ObjectMgr::GenerateArenaTeamId()
{
    if(m_arenaTeamId>=0xFFFFFFFE)
    {
        TC_LOG_ERROR("server","Arena team ids overflow!! Can't continue, shutting down server. ");
        World::StopNow(ERROR_EXIT_CODE);
    }
    return m_arenaTeamId++;
}

uint32 ObjectMgr::GenerateGuildId()
{
    if(m_guildId>=0xFFFFFFFE)
    {
        TC_LOG_ERROR("server","Guild ids overflow!! Can't continue, shutting down server. ");
        World::StopNow(ERROR_EXIT_CODE);
    }
    return m_guildId++;
}

uint32 ObjectMgr::GenerateAuctionID()
{
    if(m_auctionid>=0xFFFFFFFE)
    {
        TC_LOG_ERROR("server","Auctions ids overflow!! Can't continue, shutting down server. ");
        World::StopNow(ERROR_EXIT_CODE);
    }
    return m_auctionid++;
}

uint32 ObjectMgr::GenerateMailID()
{
    if(m_mailid>=0xFFFFFFFE)
    {
        TC_LOG_ERROR("server","Mail ids overflow!! Can't continue, shutting down server. ");
        World::StopNow(ERROR_EXIT_CODE);
    }
    return m_mailid++;
}

uint32 ObjectMgr::GenerateItemTextID()
{
    if(m_ItemTextId>=0xFFFFFFFE)
    {
        TC_LOG_ERROR("server","Item text ids overflow!! Can't continue, shutting down server. ");
        World::StopNow(ERROR_EXIT_CODE);
    }
    return m_ItemTextId++;
}

uint32 ObjectMgr::CreateItemText(std::string text)
{
    uint32 newItemTextId = GenerateItemTextID();
    //insert new itempage to container
    mItemTexts[ newItemTextId ] = text;
    //save new itempage
    CharacterDatabase.EscapeString(text);
    //any Delete query needed, itemTextId is maximum of all ids
    std::ostringstream query;
    query << "INSERT INTO item_text (id,text) VALUES ( '" << newItemTextId << "', '" << text << "')";
    CharacterDatabase.Execute(query.str().c_str());         //needs to be run this way, because mail body may be more than 1024 characters
    return newItemTextId;
}

uint32 ObjectMgr::GenerateLowGuid(HighGuid guidhigh, bool temporary)
{
    switch(guidhigh)
    {
         case HIGHGUID_MO_TRANSPORT:
         case HIGHGUID_TRANSPORT:
             return m_hiTransportGuid++;
         case HIGHGUID_GAMEOBJECT:
            if(m_hiGoRegularModeGuid)
            {
                if(m_hiGoGuid >= 0x00FFFFFE)
                {
                    TC_LOG_ERROR("FIXME","Gameobject guid overflow!! Can't continue, shutting down server. ");
                    World::StopNow(ERROR_EXIT_CODE);
                }
                //TC_LOG_ERROR("FIXME","GenerateLowGuid : returning m_hiGoGuid = %u",m_hiGoGuid);
                return m_hiGoGuid++;
            } else {
                return AltGenerateLowGuid(HIGHGUID_GAMEOBJECT,temporary);
            }
        case HIGHGUID_UNIT:
            if(m_hiCreatureRegularModeGuid)
            {
                if(m_hiCreatureGuid >= 0x00FFFFFE)
                {
                    TC_LOG_ERROR("server","Creature guid overflow!! Can't continue, shutting down server. ");
                    World::StopNow(ERROR_EXIT_CODE);
                }
                //TC_LOG_ERROR("server","GenerateLowGuid : returning m_hiCreatureGuid = %u",m_hiCreatureGuid);
                return m_hiCreatureGuid++;
            } else {
                return AltGenerateLowGuid(HIGHGUID_UNIT,temporary);
            }
        case HIGHGUID_ITEM:
            if(m_hiItemGuid >= 0xFFFFFFFE)
            {
                TC_LOG_ERROR("server","Item guid overflow!! Can't continue, shutting down server. ");
                World::StopNow(ERROR_EXIT_CODE);
            }
            return m_hiItemGuid++;
        case HIGHGUID_PET:
            if(m_hiPetGuid >= 0x00FFFFFE)
            {
                TC_LOG_ERROR("server","Pet guid overflow!! Can't continue, shutting down server. ");
                World::StopNow(ERROR_EXIT_CODE);
            }
            return m_hiPetGuid++;
        case HIGHGUID_PLAYER:
            if(m_hiCharGuid >= 0xFFFFFFFE)
            {
                TC_LOG_ERROR("server","Players guid overflow!! Can't continue, shutting down server. ");
                World::StopNow(ERROR_EXIT_CODE);
            }
            return m_hiCharGuid++;
        case HIGHGUID_CORPSE:
            if(m_hiCorpseGuid >= 0xFFFFFFFE)
            {
                TC_LOG_ERROR("server","Corpse guid overflow!! Can't continue, shutting down server. ");
                World::StopNow(ERROR_EXIT_CODE);
            }
            return m_hiCorpseGuid++;
        case HIGHGUID_DYNAMICOBJECT:
            if(m_hiDoGuid >= 0xFFFFFFFE)
            {
                TC_LOG_ERROR("server","DynamicObject guid overflow!! Can't continue, shutting down server. ");
                World::StopNow(ERROR_EXIT_CODE);
            }
            return m_hiDoGuid++;
        default:
            ASSERT(0);
    }

    ASSERT(0);
    return 0;
}

uint32 ObjectMgr::AltGenerateLowGuid(uint32 type, bool& temporary)
{
    uint32* baseguid;
    uint32* tempguid;
    uint32* tempstartguid;
    bool* regularmode;

    switch(type)
    {
    case HIGHGUID_UNIT:
        baseguid = &m_hiCreatureGuid;
        tempguid = &m_hiTempCreatureGuid;
        tempstartguid = &m_hiTempCreatureGuidStart;
        regularmode = &m_hiCreatureRegularModeGuid;
        break;
    case HIGHGUID_GAMEOBJECT:
        baseguid = &m_hiGoGuid;
        tempguid = &m_hiTempGoGuid;
        tempstartguid = &m_hiTempGoGuidStart;
        regularmode = &m_hiGoRegularModeGuid;
        break;
    default:
        return 0;
    }
    
    if (temporary)
    {
        if(*tempguid >= 0x00FFFFFE)
        {
           TC_LOG_ERROR("server","ERROR : AltGenerateLowGuid(%i) : Temporary guid range appears to be full. Can't continue, shutting down server. Sorry, it's really flooded and there's nothing I can do without my bucket they stole.",type);
           World::StopNow(ERROR_EXIT_CODE);
        }

        return (*tempguid)++;
    } else {
        if ((*baseguid) +1 >= *tempstartguid) 
        {
                TC_LOG_ERROR("server","AltGenerateLowGuid(%i) : Base guid range is full. Reverting to old guid distribution mode, new objects will now use the same range of guid.",type);
                *regularmode = true;
                *baseguid = *tempguid;
        } 
        return (*baseguid)++;
    }
}

bool ObjectMgr::IsInTemporaryGuidRange(uint32 type, uint32 guid)
{
    bool* regularmode;
    uint32* tempstartguid;

    switch(type)
    {
    case HIGHGUID_UNIT:
        regularmode = &m_hiCreatureRegularModeGuid;
        tempstartguid = &m_hiTempCreatureGuidStart;
        break;
    case HIGHGUID_GAMEOBJECT:
        regularmode = &m_hiGoRegularModeGuid;
        tempstartguid = &m_hiTempGoGuidStart;
        break;
    default:
        return false;
    }

    if (*regularmode)
        return false;
    else
        return (guid >= *tempstartguid);  
}

void ObjectMgr::LoadGameObjectLocales()
{
    mGameObjectLocaleMap.clear();                           // need for reload case

    QueryResult result = WorldDatabase.Query("SELECT entry,"
        "name_loc1,name_loc2,name_loc3,name_loc4,name_loc5,name_loc6,name_loc7,name_loc8,"
        "castbarcaption_loc1,castbarcaption_loc2,castbarcaption_loc3,castbarcaption_loc4,"
        "castbarcaption_loc5,castbarcaption_loc6,castbarcaption_loc7,castbarcaption_loc8 FROM locales_gameobject");

    if(!result)
    {
        TC_LOG_INFO("server.loading",">> Loaded 0 gameobject locale strings. DB table `locales_gameobject` is empty.");
        return;
    }

    do
    {
        Field *fields = result->Fetch();

        uint32 entry = fields[0].GetUInt32();

        GameObjectLocale& data = mGameObjectLocaleMap[entry];

        for (uint8 i = TOTAL_LOCALES - 1; i > 0; --i)
        {
            AddLocaleString(fields[i].GetString(), LocaleConstant(i), data.Name);
            AddLocaleString(fields[i + (TOTAL_LOCALES - 1)].GetString(), LocaleConstant(i), data.CastBarCaption);
        }
    } while (result->NextRow());

    TC_LOG_INFO("server.loading", ">> Loaded " UI64FMTD " gameobject locale strings", mGameObjectLocaleMap.size());
    
}

void ObjectMgr::LoadGameObjectTemplate()
{
    uint32 oldMSTime = GetMSTime();

    //                                                 0      1      2        3          4            5       6      7      8    9      10     11     12            14             16            18              20 
    QueryResult result = WorldDatabase.Query("SELECT entry, type, displayId, name, castBarCaption, faction, flags, size, data0, data1, data2, data3, data4, data5, data6, data7, data8, data9, data10, data11, data12, "
    //                                          21               23             25              27                      30              32        33
                                             "data13, data14, data15, data16, data17, data18, data19, data20, data21, data22, data23, AIName, ScriptName "
                                             "FROM gameobject_template");

    if (!result)
    {
        TC_LOG_INFO("server.loading", ">> Loaded 0 gameobject definitions. DB table `gameobject_template` is empty.");
        return;
    }

    _gameObjectTemplateStore.rehash(result->GetRowCount());
    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        uint32 entry = fields[0].GetUInt32();

        GameObjectTemplate& got = _gameObjectTemplateStore[entry];

        got.entry          = entry;
        got.type           = uint32(fields[1].GetUInt8());
        got.displayId      = fields[2].GetUInt32();
        got.name           = fields[3].GetString();
        got.castBarCaption = fields[4].GetString();
        got.faction        = uint32(fields[5].GetUInt16());
        got.flags          = fields[6].GetUInt32();
        got.size           = fields[7].GetFloat();

        for (uint8 i = 0; i < MAX_GAMEOBJECT_DATA; ++i)
            got.raw.data[i] = fields[8 + i].GetInt32(); // data1 and data6 can be -1

        got.AIName = fields[32].GetString();
        got.ScriptId = GetScriptId(fields[33].GetCString());

        switch(got.type)
        {
            case GAMEOBJECT_TYPE_DOOR:                      //0
            {
                if(got.door.lockId)
                {
                    if(!sLockStore.LookupEntry(got.door.lockId))
                        TC_LOG_ERROR("sql.sql","Gameobject (Entry: %u GoType: %u) have data1=%u but lock (Id: %u) not found.",
                            entry,got.type,got.door.lockId,got.door.lockId);
                }
                break;
            }
            case GAMEOBJECT_TYPE_BUTTON:                    //1
            {
                if(got.button.lockId)
                {
                    if(!sLockStore.LookupEntry(got.button.lockId))
                        TC_LOG_ERROR("sql.sql","Gameobject (Entry: %u GoType: %u) have data1=%u but lock (Id: %u) not found.",
                            entry,got.type,got.button.lockId,got.button.lockId);
                }
                break;
            }
            case GAMEOBJECT_TYPE_CHEST:                     //3
            {
                if(got.chest.lockId)
                {
                    if(!sLockStore.LookupEntry(got.chest.lockId))
                        TC_LOG_ERROR("sql.sql","Gameobject (Entry: %u GoType: %u) have data0=%u but lock (Id: %u) not found.",
                            entry,got.type,got.chest.lockId,got.chest.lockId);
                }
                if(got.chest.linkedTrapId)              // linked trap
                {
                    if(GameObjectTemplate const* trapInfo = GetGameObjectTemplate(got.chest.linkedTrapId))
                    {
                        if(trapInfo->type!=GAMEOBJECT_TYPE_TRAP)
                            TC_LOG_ERROR("sql.sql","Gameobject (Entry: %u GoType: %u) have data7=%u but GO (Entry %u) have not GAMEOBJECT_TYPE_TRAP (%u) type.",
                                entry,got.type,got.chest.linkedTrapId,got.chest.linkedTrapId,GAMEOBJECT_TYPE_TRAP);
                    }
                    /* disable check for while
                    else
                        TC_LOG_ERROR("sql.sql","Gameobject (Entry: %u GoType: %u) have data2=%u but trap GO (Entry %u) not exist in `gameobject_template`.",
                            id,got.type,got.chest.linkedTrapId,got.chest.linkedTrapId);
                    */
                }
                break;
            }
            case GAMEOBJECT_TYPE_TRAP:                      //6
            {
                /* disable check for while
                if(got.trap.spellId)                    // spell
                {
                    if(!sSpellMgr->GetSpellInfo(got.trap.spellId))
                        TC_LOG_ERROR("FIXME","Gameobject (Entry: %u GoType: %u) have data3=%u but Spell (Entry %u) not exist.",
                            id,got.type,got.trap.spellId,got.trap.spellId);
                }
                */
                break;
            }
            case GAMEOBJECT_TYPE_CHAIR:                     //7
                if(got.chair.height > 2)
                {
                    TC_LOG_ERROR("sql.sql","Gameobject (Entry: %u GoType: %u) have data1=%u but correct chair height in range 0..2.",
                        entry,got.type,got.chair.height);

                    // prevent client and server unexpected work
                    got.chair.height = 0;
                }
                break;
            case GAMEOBJECT_TYPE_SPELL_FOCUS:               //8
            {
                if(got.spellFocus.focusId)
                {
                    if(!sSpellFocusObjectStore.LookupEntry(got.spellFocus.focusId))
                        TC_LOG_ERROR("sql.sql","Gameobject (Entry: %u GoType: %u) have data0=%u but SpellFocus (Id: %u) not exist.",
                            entry,got.type,got.spellFocus.focusId,got.spellFocus.focusId);
                }

                if(got.spellFocus.linkedTrapId)         // linked trap
                {
                    if(GameObjectTemplate const* trapInfo = GetGameObjectTemplate(got.spellFocus.linkedTrapId))
                    {
                        if(trapInfo->type!=GAMEOBJECT_TYPE_TRAP)
                            TC_LOG_ERROR("sql.sql","Gameobject (Entry: %u GoType: %u) have data2=%u but GO (Entry %u) have not GAMEOBJECT_TYPE_TRAP (%u) type.",
                                entry,got.type,got.spellFocus.linkedTrapId,got.spellFocus.linkedTrapId,GAMEOBJECT_TYPE_TRAP);
                    }
                    /* disable check for while
                    else
                        TC_LOG_ERROR("sql.sql","Gameobject (Entry: %u GoType: %u) have data2=%u but trap GO (Entry %u) not exist in `gameobject_template`.",
                            id,got.type,got.spellFocus.linkedTrapId,got.spellFocus.linkedTrapId);
                    */
                }
                break;
            }
            case GAMEOBJECT_TYPE_GOOBER:                    //10
            {
                if (got.goober.pageId)                  // pageId
                {
                    if (!GetPageText(got.goober.pageId))
                        TC_LOG_ERROR("sql.sql", "GameObject (Entry: %u GoType: %u) have data7=%u but PageText (Entry %u) not exist.",
                        entry, got.type, got.goober.pageId, got.goober.pageId);
                }
                /* disable check for while
                if(got.goober.spellId)                  // spell
                {
                    if(!sSpellMgr->GetSpellInfo(got.goober.spellId))
                        TC_LOG_ERROR("sql.sql","Gameobject (Entry: %u GoType: %u) have data2=%u but Spell (Entry %u) not exist.",
                            id,got.type,got.goober.spellId,got.goober.spellId);
                }
                */
                if(got.goober.linkedTrapId)             // linked trap
                {
                    if(GameObjectTemplate const* trapInfo = GetGameObjectTemplate(got.goober.linkedTrapId))
                    {
                        if(trapInfo->type!=GAMEOBJECT_TYPE_TRAP)
                            TC_LOG_ERROR("sql.sql","Gameobject (Entry: %u GoType: %u) have data12=%u but GO (Entry %u) have not GAMEOBJECT_TYPE_TRAP (%u) type.",
                                entry,got.type,got.goober.linkedTrapId,got.goober.linkedTrapId,GAMEOBJECT_TYPE_TRAP);
                    }
                    /* disable check for while
                    else
                        TC_LOG_ERROR("sql.sql","Gameobject (Entry: %u GoType: %u) have data12=%u but trap GO (Entry %u) not exist in `gameobject_template`.",
                            id,got.type,got.goober.linkedTrapId,got.goober.linkedTrapId);
                    */
                }
                break;
            }
            case GAMEOBJECT_TYPE_MO_TRANSPORT:              //15
            {
                if(got.moTransport.taxiPathId)
                {
                    if(got.moTransport.taxiPathId >= sTaxiPathNodesByPath.size() || sTaxiPathNodesByPath[got.moTransport.taxiPathId].empty())
                        TC_LOG_ERROR("sql.sql","Gameobject (Entry: %u GoType: %u) have data0=%u but TaxiPath (Id: %u) not exist.",
                            entry,got.type,got.moTransport.taxiPathId,got.moTransport.taxiPathId);
                }
                if (uint32 transportMap = got.moTransport.mapID)
                    _transportMaps.insert(transportMap);
                break;
            }
            case GAMEOBJECT_TYPE_SUMMONING_RITUAL:          //18
            {
                /* disabled
                if(got.summoningRitual.spellId)
                {
                    if(!sSpellMgr->GetSpellInfo(got.summoningRitual.spellId))
                        TC_LOG_ERROR("sql.sql","Gameobject (Entry: %u GoType: %u) have data1=%u but Spell (Entry %u) not exist.",
                            id,got.type,got.summoningRitual.spellId,got.summoningRitual.spellId);
                }
                */
                break;
            }
            case GAMEOBJECT_TYPE_SPELLCASTER:               //22
            {
                if(got.spellcaster.spellId)             // spell
                {
                    if(!sSpellMgr->GetSpellInfo(got.spellcaster.spellId))
                        TC_LOG_ERROR("sql.sql","Gameobject (Entry: %u GoType: %u) have data3=%u but Spell (Entry %u) not exist.",
                            entry,got.type,got.spellcaster.spellId,got.spellcaster.spellId);
                }
                break;
            }
        }
        count++;
    }
    while (result->NextRow());

    TC_LOG_INFO("server.loading", ">> Loaded %u game object templates in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void ObjectMgr::LoadExplorationBaseXP()
{
    uint32 count = 0;
    QueryResult result = WorldDatabase.Query("SELECT level,basexp FROM exploration_basexp");

    if( !result )
    {
        TC_LOG_INFO("server.loading", ">> Loaded %u BaseXP definitions", count );
        
        return;
    }

    do
    {
        Field *fields = result->Fetch();
        uint32 level  = fields[0].GetUInt32();
        uint32 basexp = fields[1].GetUInt32();
        mBaseXPTable[level] = basexp;
        ++count;
    }
    while (result->NextRow());

    TC_LOG_INFO("server.loading", ">> Loaded %u BaseXP definitions", count );
    
}

uint32 ObjectMgr::GetBaseXP(uint32 level)
{
    return mBaseXPTable[level] ? mBaseXPTable[level] : 0;
}

void ObjectMgr::LoadPetNames()
{
    uint32 count = 0;
    QueryResult result = WorldDatabase.Query("SELECT word,entry,half FROM pet_name_generation");

    if( !result )
    {
        TC_LOG_INFO("server.loading", ">> Loaded %u pet name parts", count );
        
        return;
    }

    do
    {
        Field *fields = result->Fetch();
        std::string word = fields[0].GetString();
        uint32 entry     = fields[1].GetUInt32();
        bool   half      = fields[2].GetBool();
        if(half)
            PetHalfName1[entry].push_back(word);
        else
            PetHalfName0[entry].push_back(word);
        ++count;
    }
    while (result->NextRow());

    TC_LOG_INFO("server.loading", ">> Loaded %u pet name parts", count );
    
}

void ObjectMgr::LoadPetNumber()
{
    QueryResult result = CharacterDatabase.Query("SELECT MAX(id) FROM character_pet");
    if(result)
    {
        Field *fields = result->Fetch();
        m_hiPetNumber = fields[0].GetUInt32()+1;
    }

    TC_LOG_INFO("server.loading", ">> Loaded the max pet number: %d", m_hiPetNumber-1);
    
}

std::string ObjectMgr::GeneratePetName(uint32 entry)
{
    std::vector<std::string> & list0 = PetHalfName0[entry];
    std::vector<std::string> & list1 = PetHalfName1[entry];

    if(list0.empty() || list1.empty())
    {
        CreatureTemplate const *cinfo = GetCreatureTemplate(entry);
        std::string petname = GetPetName(cinfo->family, sWorld->GetDefaultDbcLocale());
        if(petname.empty())
            petname = cinfo->Name;
        return std::string(petname);
    }

    return *(list0.begin()+urand(0, list0.size()-1)) + *(list1.begin()+urand(0, list1.size()-1));
}

uint32 ObjectMgr::GeneratePetNumber()
{
    return ++m_hiPetNumber;
}

void ObjectMgr::LoadCorpses()
{
    uint32 count = 0;
    //                                                     0           1           2           3            4    5     6     7            8         10
    QueryResult result = CharacterDatabase.Query("SELECT position_x, position_y, position_z, orientation, map, data, time, corpse_type, instance, guid FROM corpse WHERE corpse_type <> 0");

    if( !result )
    {
        TC_LOG_INFO("server.loading", ">> Loaded %u corpses", count );
        
        return;
    }

    do
    {
        Field *fields = result->Fetch();

        uint32 guid = fields[result->GetFieldCount()-1].GetUInt32();

        Corpse *corpse = new Corpse;
        if(!corpse->LoadFromDB(guid,fields))
        {
            delete corpse;
            continue;
        }

        sObjectAccessor->AddCorpse(corpse);

        ++count;
    }
    while (result->NextRow());

    TC_LOG_INFO("server.loading", ">> Loaded %u corpses", count );
    
}

void ObjectMgr::LoadReputationOnKill()
{
    uint32 count = 0;

    //                                                0            1                     2
    QueryResult result = WorldDatabase.Query("SELECT creature_id, RewOnKillRepFaction1, RewOnKillRepFaction2,"
    //   3             4             5                   6             7             8                   9
        "IsTeamAward1, MaxStanding1, RewOnKillRepValue1, IsTeamAward2, MaxStanding2, RewOnKillRepValue2, TeamDependent "
        "FROM creature_onkill_reputation");

    if(!result)
    {
        TC_LOG_ERROR("sql.sql",">> Loaded 0 creature award reputation definitions. DB table `creature_onkill_reputation` is empty.");
        
        return;
    }

    do
    {
        Field *fields = result->Fetch();

        uint32 creature_id = fields[0].GetUInt32();

        ReputationOnKillEntry repOnKill;
        repOnKill.repfaction1          = fields[1].GetUInt32();
        repOnKill.repfaction2          = fields[2].GetUInt32();
        repOnKill.is_teamaward1        = fields[3].GetBool();
        repOnKill.reputation_max_cap1  = fields[4].GetUInt32();
        repOnKill.repvalue1            = fields[5].GetInt32();
        repOnKill.is_teamaward2        = fields[6].GetBool();
        repOnKill.reputation_max_cap2  = fields[7].GetUInt32();
        repOnKill.repvalue2            = fields[8].GetInt32();
        repOnKill.team_dependent       = fields[9].GetUInt8();

        if(!GetCreatureTemplate(creature_id))
        {
            TC_LOG_ERROR("sql.sql","Table `creature_onkill_reputation` have data for not existed creature entry (%u), skipped",creature_id);
            continue;
        }

        if(repOnKill.repfaction1)
        {
            FactionEntry const *factionEntry1 = sFactionStore.LookupEntry(repOnKill.repfaction1);
            if(!factionEntry1)
            {
                TC_LOG_ERROR("sql.sql","Faction (faction.dbc) %u does not exist but is used in `creature_onkill_reputation`",repOnKill.repfaction1);
                continue;
            }
        }

        if(repOnKill.repfaction2)
        {
            FactionEntry const *factionEntry2 = sFactionStore.LookupEntry(repOnKill.repfaction2);
            if(!factionEntry2)
            {
                TC_LOG_ERROR("sql.sql","Faction (faction.dbc) %u does not exist but is used in `creature_onkill_reputation`",repOnKill.repfaction2);
                continue;
            }
        }

        mRepOnKill[creature_id] = repOnKill;

        ++count;
    } while (result->NextRow());

    TC_LOG_INFO("server.loading",">> Loaded %u creature award reputation definitions", count);
    
}

void ObjectMgr::LoadWeatherZoneChances()
{
    uint32 count = 0;

    //                                                0     1                   2                   3                    4                   5                   6                    7                 8                 9                  10                  11                  12
    QueryResult result = WorldDatabase.Query("SELECT zone, spring_rain_chance, spring_snow_chance, spring_storm_chance, summer_rain_chance, summer_snow_chance, summer_storm_chance, fall_rain_chance, fall_snow_chance, fall_storm_chance, winter_rain_chance, winter_snow_chance, winter_storm_chance FROM game_weather");

    if(!result)
    {
        TC_LOG_ERROR("server.loading",">> Loaded 0 weather definitions. DB table `game_weather` is empty.");
        return;
    }

    do
    {
        Field *fields = result->Fetch();

        uint32 zone_id = fields[0].GetUInt32();

        WeatherZoneChances& wzc = mWeatherZoneMap[zone_id];

        for(int season = 0; season < WEATHER_SEASONS; ++season)
        {
            wzc.data[season].rainChance  = fields[season * (MAX_WEATHER_TYPE-1) + 1].GetUInt32();
            wzc.data[season].snowChance  = fields[season * (MAX_WEATHER_TYPE-1) + 2].GetUInt32();
            wzc.data[season].stormChance = fields[season * (MAX_WEATHER_TYPE-1) + 3].GetUInt32();

            if(wzc.data[season].rainChance > 100)
            {
                wzc.data[season].rainChance = 25;
                TC_LOG_ERROR("sql.sql","Weather for zone %u season %u has wrong rain chance > 100%%",zone_id,season);
            }

            if(wzc.data[season].snowChance > 100)
            {
                wzc.data[season].snowChance = 25;
                TC_LOG_ERROR("sql.sql","Weather for zone %u season %u has wrong snow chance > 100%%", zone_id, season);
            }

            if(wzc.data[season].stormChance > 100)
            {
                wzc.data[season].stormChance = 25;
                TC_LOG_ERROR("sql.sql","Weather for zone %u season %u has wrong storm chance > 100%%",zone_id,season);
            }
        }

        ++count;
    } while (result->NextRow());

    TC_LOG_INFO("server.loading",">> Loaded %u weather definitions", count);
}

void ObjectMgr::SaveCreatureRespawnTime(uint32 loguid, uint32 instance, time_t t)
{
    _creatureRespawnTimeLock.lock();
    mCreatureRespawnTimes[MAKE_PAIR64(loguid,instance)] = t;
    _creatureRespawnTimeLock.unlock();

    SQLTransaction trans = WorldDatabase.BeginTransaction();
    trans->PAppend("DELETE FROM creature_respawn WHERE guid = '%u' AND instance = '%u'", loguid, instance);
    if (t)
        trans->PAppend("INSERT INTO creature_respawn VALUES ( '%u', '" UI64FMTD "', '%u' )", loguid, uint64(t), instance);
    WorldDatabase.CommitTransaction(trans);
}

void ObjectMgr::DeleteCreatureData(uint32 guid)
{
    // remove mapid*cellid -> guid_set map
    CreatureData const* data = GetCreatureData(guid);
    if (data)
        RemoveCreatureFromGrid(guid, data);

    mCreatureDataMap.erase(guid);
}

void ObjectMgr::SaveGORespawnTime(uint32 loguid, uint32 instance, time_t t)
{
    if(!loguid) 
        return;

    _goRespawnTimeLock.lock();
    mGORespawnTimes[MAKE_PAIR64(loguid,instance)] = t;
    _goRespawnTimeLock.unlock();

    SQLTransaction trans = WorldDatabase.BeginTransaction();
    trans->PAppend("DELETE FROM gameobject_respawn WHERE guid = '%u' AND instance = '%u'", loguid, instance);
    if (t)
        trans->PAppend("INSERT INTO gameobject_respawn VALUES ( '%u', '" UI64FMTD "', '%u' )", loguid, uint64(t), instance);
    WorldDatabase.CommitTransaction(trans);
}

void ObjectMgr::DeleteRespawnTimeForInstance(uint32 instance)
{
    RespawnTimes::iterator next;

    _goRespawnTimeLock.lock();
    for(RespawnTimes::iterator itr = mGORespawnTimes.begin(); itr != mGORespawnTimes.end(); itr = next)
    {
        next = itr;
        ++next;

        if(GUID_HIPART(itr->first)==instance)
            mGORespawnTimes.erase(itr);
    }
    _goRespawnTimeLock.unlock();

    _creatureRespawnTimeLock.lock();
    for(RespawnTimes::iterator itr = mCreatureRespawnTimes.begin(); itr != mCreatureRespawnTimes.end(); itr = next)
    {
        next = itr;
        ++next;

        if(GUID_HIPART(itr->first)==instance)
            mCreatureRespawnTimes.erase(itr);
    }
    _creatureRespawnTimeLock.unlock();

    SQLTransaction trans = WorldDatabase.BeginTransaction();
    trans->PAppend("DELETE FROM creature_respawn WHERE instance = '%u'", instance);
    trans->PAppend("DELETE FROM gameobject_respawn WHERE instance = '%u'", instance);
    WorldDatabase.CommitTransaction(trans);
}

void ObjectMgr::DeleteGOData(uint32 guid)
{
    // remove mapid*cellid -> guid_set map
    GameObjectData const* data = GetGOData(guid);
    if(data)
        RemoveGameobjectFromGrid(guid, data);

    mGameObjectDataMap.erase(guid);
}

void ObjectMgr::AddCorpseCellData(uint32 mapid, uint32 cellid, uint32 player_guid, uint32 instance)
{
    // corpses are always added to spawn mode 0 and they are spawned by their instance id
    CellObjectGuids& cell_guids = mMapObjectGuids[MAKE_PAIR32(mapid,0)][cellid];
    cell_guids.corpses[player_guid] = instance;
}

void ObjectMgr::DeleteCorpseCellData(uint32 mapid, uint32 cellid, uint32 player_guid)
{
    // corpses are always added to spawn mode 0 and they are spawned by their instance id
    CellObjectGuids& cell_guids = mMapObjectGuids[MAKE_PAIR32(mapid,0)][cellid];
    cell_guids.corpses.erase(player_guid);
}

void ObjectMgr::LoadQuestRelationsHelper(QuestRelations& map,char const* table)
{
    map.clear();                                            // need for reload case

    uint32 count = 0;

    QueryResult result = WorldDatabase.PQuery("SELECT id,quest FROM %s",table);

    if(!result)
    {
        TC_LOG_ERROR("sql.sql",">> Loaded 0 quest relations from %s. DB table `%s` is empty.",table,table);
        
        return;
    }

    do
    {
        Field *fields = result->Fetch();

        uint32 id    = fields[0].GetUInt32();
        uint32 quest = fields[1].GetUInt32();

        if(mQuestTemplates.find(quest) == mQuestTemplates.end())
        {
            TC_LOG_ERROR("sql.sql","Table `%s: Quest %u listed for entry %u does not exist.",table,quest,id);
            continue;
        }

        map.insert(QuestRelations::value_type(id,quest));

        ++count;
    } while (result->NextRow());

    TC_LOG_INFO("server.loading",">> Loaded %u quest relations from %s", count,table);
    
}

void ObjectMgr::LoadGameobjectQuestStarters()
{
    LoadQuestRelationsHelper(mGOQuestRelations,"gameobject_queststarter");

    for(QuestRelations::iterator itr = mGOQuestRelations.begin(); itr != mGOQuestRelations.end(); ++itr)
    {
        GameObjectTemplate const* goInfo = GetGameObjectTemplate(itr->first);
        if(!goInfo)
            TC_LOG_ERROR("sql.sql","Table `gameobject_queststarter` have data for not existed gameobject entry (%u) and existed quest %u",itr->first,itr->second);
        else if(goInfo->type != GAMEOBJECT_TYPE_QUESTGIVER)
            TC_LOG_ERROR("sql.sql","Table `gameobject_queststarter` have data gameobject entry (%u) for quest %u, but GO is not GAMEOBJECT_TYPE_QUESTGIVER",itr->first,itr->second);
    }
}

void ObjectMgr::LoadGameobjectQuestEnders()
{
    LoadQuestRelationsHelper(mGOQuestInvolvedRelations,"gameobject_questender");

    for(QuestRelations::iterator itr = mGOQuestInvolvedRelations.begin(); itr != mGOQuestInvolvedRelations.end(); ++itr)
    {
        GameObjectTemplate const* goInfo = GetGameObjectTemplate(itr->first);
        if(!goInfo)
            TC_LOG_ERROR("sql.sql","Table `gameobject_questender` have data for not existed gameobject entry (%u) and existed quest %u",itr->first,itr->second);
        else if(goInfo->type != GAMEOBJECT_TYPE_QUESTGIVER)
            TC_LOG_ERROR("sql.sql","Table `gameobject_questender` have data gameobject entry (%u) for quest %u, but GO is not GAMEOBJECT_TYPE_QUESTGIVER",itr->first,itr->second);
    }
}

void ObjectMgr::LoadCreatureQuestStarters()
{
    LoadQuestRelationsHelper(mCreatureQuestRelations,"creature_queststarter");

    for(QuestRelations::iterator itr = mCreatureQuestRelations.begin(); itr != mCreatureQuestRelations.end(); ++itr)
    {
        CreatureTemplate const* cInfo = GetCreatureTemplate(itr->first);
        if(!cInfo)
            TC_LOG_ERROR("sql.sql","Table `creature_queststarter` have data for not existed creature entry (%u) and existed quest %u",itr->first,itr->second);
        else if(!(cInfo->npcflag & UNIT_NPC_FLAG_QUESTGIVER))
            TC_LOG_ERROR("sql.sql","Table `creature_queststarter` has creature entry (%u) for quest %u, but npcflag does not include UNIT_NPC_FLAG_QUESTGIVER",itr->first,itr->second);
    }
}

void ObjectMgr::LoadCreatureQuestEnders()
{
    LoadQuestRelationsHelper(mCreatureQuestInvolvedRelations,"creature_questender");

    for(QuestRelations::iterator itr = mCreatureQuestInvolvedRelations.begin(); itr != mCreatureQuestInvolvedRelations.end(); ++itr)
    {
        CreatureTemplate const* cInfo = GetCreatureTemplate(itr->first);
        if(!cInfo)
            TC_LOG_ERROR("sql.sql","Table `creature_questender` have data for not existed creature entry (%u) and existed quest %u",itr->first,itr->second);
        else if(!(cInfo->npcflag & UNIT_NPC_FLAG_QUESTGIVER))
            TC_LOG_ERROR("sql.sql","Table `creature_questender` has creature entry (%u) for quest %u, but npcflag does not include UNIT_NPC_FLAG_QUESTGIVER",itr->first,itr->second);
    }
}

void ObjectMgr::LoadReservedPlayersNames()
{
    m_ReservedNames.clear();                                // need for reload case

    QueryResult result = WorldDatabase.Query("SELECT name FROM reserved_name");

    uint32 count = 0;

    if( !result )
    {
        TC_LOG_INFO("server.loading", ">> Loaded %u reserved player names", count );
        
        return;
    }

    Field* fields;
    do
    {
        fields = result->Fetch();
        std::string name= fields[0].GetString();
        if(normalizePlayerName(name))
        {
            m_ReservedNames.insert(name);
            ++count;
        }
    } while ( result->NextRow() );

    TC_LOG_INFO("server.loading", ">> Loaded %u reserved player names", count );
    
}

enum LanguageType
{
    LT_BASIC_LATIN    = 0x0000,
    LT_EXTENDEN_LATIN = 0x0001,
    LT_CYRILLIC       = 0x0002,
    LT_EAST_ASIA      = 0x0004,
    LT_ANY            = 0xFFFF
};

static LanguageType GetRealmLanguageType(bool create)
{
    switch(sWorld->getConfig(CONFIG_REALM_ZONE))
    {
        case REALM_ZONE_UNKNOWN:                            // any language
        case REALM_ZONE_DEVELOPMENT:
        case REALM_ZONE_TEST_SERVER:
        case REALM_ZONE_QA_SERVER:
            return LT_ANY;
        case REALM_ZONE_UNITED_STATES:                      // extended-Latin
        case REALM_ZONE_OCEANIC:
        case REALM_ZONE_LATIN_AMERICA:
        case REALM_ZONE_ENGLISH:
        case REALM_ZONE_GERMAN:
        case REALM_ZONE_FRENCH:
        case REALM_ZONE_SPANISH:
            return LT_EXTENDEN_LATIN;
        case REALM_ZONE_KOREA:                              // East-Asian
        case REALM_ZONE_TAIWAN:
        case REALM_ZONE_CHINA:
            return LT_EAST_ASIA;
        case REALM_ZONE_RUSSIAN:                            // Cyrillic
            return LT_CYRILLIC;
        default:
            return create ? LT_BASIC_LATIN : LT_ANY;        // basic-Latin at create, any at login
    }
}

bool isValidString(std::wstring wstr, uint32 strictMask, bool numericOrSpace, bool create = false)
{
    if(strictMask==0)                                       // any language, ignore realm
    {
        if(isExtendedLatinString(wstr,numericOrSpace))
            return true;
        if(isCyrillicString(wstr,numericOrSpace))
            return true;
        if(isEastAsianString(wstr,numericOrSpace))
            return true;
        return false;
    }

    if(strictMask & 0x2)                                    // realm zone specific
    {
        LanguageType lt = GetRealmLanguageType(create);
        if(lt & LT_EXTENDEN_LATIN)
            if(isExtendedLatinString(wstr,numericOrSpace))
                return true;
        if(lt & LT_CYRILLIC)
            if(isCyrillicString(wstr,numericOrSpace))
                return true;
        if(lt & LT_EAST_ASIA)
            if(isEastAsianString(wstr,numericOrSpace))
                return true;
    }

    if(strictMask & 0x1)                                    // basic Latin
    {
        if(isBasicLatinString(wstr,numericOrSpace))
            return true;
    }

    return false;
}

ResponseCodes ObjectMgr::CheckPlayerName( const std::string& name, bool create )
{
    std::wstring wname;
    if(!Utf8toWStr(name,wname))
        return CHAR_NAME_INVALID_CHARACTER;

    if( wname.size() > MAX_PLAYER_NAME)
        return CHAR_NAME_TOO_LONG;

    if(wname.size() < 2)
        return CHAR_NAME_TOO_SHORT;

    uint32 strictMask = sWorld->getConfig(CONFIG_STRICT_PLAYER_NAMES);

    if(!isValidString(wname,strictMask,false,create))
        return CHAR_NAME_MIXED_LANGUAGES;

    wstrToLower(wname);
    for (size_t i = 2; i < wname.size(); ++i)
        if (wname[i] == wname[i-1] && wname[i] == wname[i-2])
            return CHAR_NAME_THREE_CONSECUTIVE;

    return CHAR_NAME_SUCCESS;
}

bool ObjectMgr::IsValidCharterName( const std::string& name )
{
    std::wstring wname;
    if(!Utf8toWStr(name,wname))
        return false;

    if(wname.size() < 1)
        return false;

    if (wname.size() > MAX_CHARTER_NAME)
        return false;

    uint32 strictMask = sWorld->getConfig(CONFIG_STRICT_CHARTER_NAMES);

    return isValidString(wname,strictMask,true);
}

PetNameInvalidReason ObjectMgr::CheckPetName( const std::string& name )
{
    std::wstring wname;
    if(!Utf8toWStr(name,wname))
        return PET_NAME_INVALID;

    if(wname.size() < 1)
        return PET_NAME_INVALID;

    if (wname.size() > MAX_PET_NAME)
        return PET_NAME_TOO_LONG;

    uint32 strictMask = sWorld->getConfig(CONFIG_STRICT_PET_NAMES);

    if (!isValidString(wname, strictMask, false))
        return PET_NAME_MIXED_LANGUAGES;

    return PET_NAME_SUCCESS;
}

void ObjectMgr::LoadBattleMastersEntry()
{
    mBattleMastersMap.clear();                              // need for reload case

    QueryResult result = WorldDatabase.Query( "SELECT entry,bg_template FROM battlemaster_entry" );

    uint32 count = 0;

    if( !result )
    {
        TC_LOG_INFO("server.loading", ">> Loaded 0 battlemaster entries - table is empty!" );
        
        return;
    }

    do
    {
        ++count;

        Field *fields = result->Fetch();

        uint32 entry = fields[0].GetUInt32();
        uint32 bgTypeId  = fields[1].GetUInt32();

        mBattleMastersMap[entry] = bgTypeId;

    } while( result->NextRow() );

    TC_LOG_INFO("server.loading", ">> Loaded %u battlemaster entries", count );
    
}

void ObjectMgr::LoadGameObjectForQuests()
{
    uint32 oldMSTime = GetMSTime();

    _gameObjectForQuestStore.clear();                         // need for reload case

    if (sObjectMgr->GetGameObjectTemplateStore()->empty())
    {
        TC_LOG_INFO("server.loading", ">> Loaded 0 GameObjects for quests");
        return;
    }

    uint32 count = 0;

    
    // collect GO entries for GO that must activated
    GameObjectTemplateContainer const* gotc = sObjectMgr->GetGameObjectTemplateStore();
    for (GameObjectTemplateContainer::const_iterator itr = gotc->begin(); itr != gotc->end(); ++itr)
    {
        switch(itr->second.type)
        {
            // scan GO chest with loot including quest items
            case GAMEOBJECT_TYPE_CHEST:
            {
                uint32 loot_id = GameObject::GetLootId(&itr->second);

                // find quest loot for GO
                if(LootTemplates_Gameobject.HaveQuestLootFor(loot_id))
                {
                    _gameObjectForQuestStore.insert(itr->second.entry);
                    ++count;
                }
                break;
            }
            case GAMEOBJECT_TYPE_GOOBER:
            {
                if(itr->second.goober.questId)                  //quests objects
                {
                    _gameObjectForQuestStore.insert(itr->second.entry);
                    count++;
                }
                break;
            }
            default:
                break;
        }
    }

    TC_LOG_INFO("server.loading", ">> Loaded %u GameObject for quests", count );
    
}

bool ObjectMgr::LoadTrinityStrings(WorldDatabaseWorkerPool& db, char const* table, int32 min_value, int32 max_value)
{
    // cleanup affected map part for reloading case
    for(TrinityStringLocaleMap::iterator itr = mTrinityStringLocaleMap.begin(); itr != mTrinityStringLocaleMap.end();)
    {
        if(itr->first >= min_value && itr->first <= max_value)
        {
            TrinityStringLocaleMap::iterator itr2 = itr;
            ++itr;
            mTrinityStringLocaleMap.erase(itr2);
        }
        else
            ++itr;
    }

    QueryResult result = db.PQuery("SELECT entry,content_default,content_loc1,content_loc2,content_loc3,content_loc4,content_loc5,content_loc6,content_loc7,content_loc8 FROM %s",table);

    if(!result)
    {
        if(min_value == MIN_TRINITY_STRING_ID)               // error only in case internal strings
            TC_LOG_ERROR("sql.sql",">> Loaded 0 trinity strings. DB table `%s` is empty. Cannot continue.",table);
        else
            TC_LOG_INFO("sql.sql",">> Loaded 0 string templates. DB table `%s` is empty.",table);
        return false;
    }

    uint32 count = 0;

    do
    {
        Field *fields = result->Fetch();

        int32 entry = fields[0].GetInt32();

        if(entry==0)
        {
            TC_LOG_ERROR("sql.sql","Table `%s` contain reserved entry 0, ignored.",table);
            continue;
        }
        else if(entry < min_value || entry > max_value)
        {
            int32 start = min_value > 0 ? min_value : max_value;
            int32 end   = min_value > 0 ? max_value : min_value;
            TC_LOG_ERROR("sql.sql","Table `%s` contain entry %i out of allowed range (%d - %d), ignored.",table,entry,start,end);
            continue;
        }

        TrinityStringLocale& data = mTrinityStringLocaleMap[entry];

        if(data.Content.size() > 0)
        {
            TC_LOG_ERROR("sql.sql","Table `%s` contain data for already loaded entry  %i (from another table?), ignored.",table,entry);
            continue;
        }

        data.Content.resize(1);
        ++count;

        for (int8 i = MAX_LOCALE; i >= 0; --i)
            AddLocaleString(fields[i + 1].GetString(), LocaleConstant(i), data.Content);

    } while (result->NextRow());

    
    if(min_value == MIN_TRINITY_STRING_ID)               // internal Trinity strings
        TC_LOG_INFO("server.loading", ">> Loaded %u Trinity strings from table %s", count,table);
    else
        TC_LOG_INFO("server.loading", ">> Loaded %u string templates from %s", count,table);

    return true;
}

const char *ObjectMgr::GetTrinityString(int32 entry, LocaleConstant locale_idx) const
{
    // locale_idx==-1 -> default, locale_idx >= 0 in to idx+1
    // Content[0] always exist if exist TrinityStringLocale
    if(TrinityStringLocale const *msl = GetTrinityStringLocale(entry))
    {
        if(msl->Content.size() > locale_idx+1 && !msl->Content[locale_idx+1].empty())
            return msl->Content[locale_idx+1].c_str();
        else
            return msl->Content[0].c_str();
    }

    if(entry > 0)
        TC_LOG_ERROR("sql.sql","Entry %i not found in `trinity_string` table.",entry);
    else
        TC_LOG_ERROR("sql.sql","Trinity string entry %i not found in DB.",entry);

    return "<error>";
}

void ObjectMgr::LoadSpellDisabledEntrys()
{
    m_DisabledPlayerSpells.clear();                                // need for reload case
    m_DisabledCreatureSpells.clear();
    m_DisabledPetSpells.clear();
    QueryResult result = WorldDatabase.Query("SELECT entry, disable_mask FROM spell_disabled");

    uint32 total_count = 0;

    if( !result )
    {
        TC_LOG_INFO("server.loading", ">> Loaded %u disabled spells", total_count );
        
        return;
    }

    Field* fields;
    do
    {
        fields = result->Fetch();
        uint32 spellid = fields[0].GetUInt32();
        if(!sSpellMgr->GetSpellInfo(spellid))
        {
            TC_LOG_ERROR("sql.sql","Spell entry %u from `spell_disabled` doesn't exist in dbc, ignoring.",spellid);
            continue;
        }
        uint32 disable_mask = fields[1].GetUInt32();
        if(disable_mask & SPELL_DISABLE_PLAYER)
            m_DisabledPlayerSpells.insert(spellid);
        if(disable_mask & SPELL_DISABLE_CREATURE)
            m_DisabledCreatureSpells.insert(spellid);
        if(disable_mask & SPELL_DISABLE_PET)
            m_DisabledPetSpells.insert(spellid);
        ++total_count;
   } while ( result->NextRow() );

    TC_LOG_INFO("server.loading", ">> Loaded %u disabled spells from `spell_disabled`", total_count);
    
}

void ObjectMgr::LoadFishingBaseSkillLevel()
{
    mFishingBaseForArea.clear();                            // for reload case

    uint32 count = 0;
    QueryResult result = WorldDatabase.Query("SELECT entry,skill FROM skill_fishing_base_level");

    if( !result )
    {
        TC_LOG_ERROR("sql.sql",">> Loaded `skill_fishing_base_level`, table is empty!");
        
        return;
    }

    do
    {
        Field *fields = result->Fetch();
        uint32 entry  = fields[0].GetUInt32();
        int32 skill   = fields[1].GetInt32();

        AreaTableEntry const* fArea = GetAreaEntryByAreaID(entry);
        if(!fArea)
        {
            TC_LOG_ERROR("sql.sql","AreaId %u defined in `skill_fishing_base_level` does not exist",entry);
            continue;
        }

        mFishingBaseForArea[entry] = skill;
        ++count;
    }
    while (result->NextRow());

    TC_LOG_INFO("server.loading", ">> Loaded %u areas for fishing base skill level", count );
    
}

// Searches for the same condition already in Conditions store
// Returns Id if found, else adds it to Conditions and returns Id
uint16 ObjectMgr::GetConditionId( ConditionType condition, uint32 value1, uint32 value2 )
{
    PlayerCondition lc = PlayerCondition(condition, value1, value2);
    for (uint16 i=0; i < mConditions.size(); ++i)
    {
        if (lc == mConditions[i])
            return i;
    }

    mConditions.push_back(lc);

    if(mConditions.size() > 0xFFFF)
    {
        TC_LOG_ERROR("sql.sql","Conditions store overflow! Current and later loaded conditions will ignored!");
        return 0;
    }

    return mConditions.size() - 1;
}

bool ObjectMgr::CheckDeclinedNames( std::wstring mainpart, DeclinedName const& names )
{
    for(int i =0; i < MAX_DECLINED_NAME_CASES; ++i)
    {
        std::wstring wname;
        if(!Utf8toWStr(names.name[i],wname))
            return false;

        if(mainpart!=GetMainPartOfName(wname,i+1))
            return false;
    }
    return true;
}

uint32 ObjectMgr::GetAreaTriggerScriptId(uint32 trigger_id)
{
    AreaTriggerScriptMap::const_iterator i = mAreaTriggerScripts.find(trigger_id);
    if(i!= mAreaTriggerScripts.end())
        return i->second;
    return 0;
}

// Checks if player meets the condition
bool PlayerCondition::Meets(Player const * player) const
{
    if( !player )
        return false;                                       // player not present, return false

    switch (condition)
    {
        case CONDITION_NONE:
            return true;                                    // empty condition, always met
        case CONDITION_AURA:
            return player->HasAura(value1, value2);
        case CONDITION_ITEM:
            return player->HasItemCount(value1, value2);
        case CONDITION_ITEM_EQUIPPED:
            return player->GetItemOrItemWithGemEquipped(value1) != NULL;
        case CONDITION_ZONEID:
            return player->GetZoneId() == value1;
        case CONDITION_REPUTATION_RANK:
        {
            FactionEntry const* faction = sFactionStore.LookupEntry(value1);
            return faction && player->GetReputationRank(faction) >= value2;
        }
        case CONDITION_TEAM:
            return player->GetTeam() == value1;
        case CONDITION_SKILL:
            return player->HasSkill(value1) && player->GetBaseSkillValue(value1) >= value2;
        case CONDITION_QUESTREWARDED:
            return player->GetQuestRewardStatus(value1);
        case CONDITION_QUESTTAKEN:
        {
            QuestStatus status = player->GetQuestStatus(value1);
            return (status == QUEST_STATUS_INCOMPLETE);
        }
        case CONDITION_AD_COMMISSION_AURA:
        {
            Unit::AuraMap const& auras = player->GetAuras();
            for(Unit::AuraMap::const_iterator itr = auras.begin(); itr != auras.end(); ++itr)
                if((itr->second->GetSpellProto()->Attributes & 0x1000010) && itr->second->GetSpellProto()->SpellVisual==3580)
                    return true;
            return false;
        }
        case CONDITION_NO_AURA:
            return !player->HasAura(value1, value2);
        case CONDITION_ACTIVE_EVENT:
            return gameeventmgr.IsActiveEvent(value1);
        case CONDITION_INSTANCE_DATA:
        {
            Map *map = player->GetMap();
            if(map && map->IsDungeon() && ((InstanceMap*)map)->GetInstanceData())
                return ((InstanceMap*)map)->GetInstanceData()->GetData(value1) == value2;
        }
        default:
            return false;
    }
}

// Verification of condition values validity
bool PlayerCondition::IsValid(ConditionType condition, uint32 value1, uint32 value2)
{
    if( condition >= MAX_CONDITION)                         // Wrong condition type
    {
        TC_LOG_ERROR("condition","Condition has bad type of %u, skipped ", condition );
        return false;
    }

    switch (condition)
    {
        case CONDITION_AURA:
        {
            if(!sSpellMgr->GetSpellInfo(value1))
            {
                TC_LOG_ERROR("condition","Aura condition requires to have non existing spell (Id: %d), skipped", value1);
                return false;
            }
            if(value2 > 2)
            {
                TC_LOG_ERROR("condition","Aura condition requires to have non existing effect index (%u) (must be 0..2), skipped", value2);
                return false;
            }
            break;
        }
        case CONDITION_ITEM:
        {
            ItemTemplate const *proto = sObjectMgr->GetItemTemplate(value1);
            if(!proto)
            {
                TC_LOG_ERROR("condition","Item condition requires to have non existing item (%u), skipped", value1);
                return false;
            }
            break;
        }
        case CONDITION_ITEM_EQUIPPED:
        {
            ItemTemplate const *proto = sObjectMgr->GetItemTemplate(value1);
            if(!proto)
            {
                TC_LOG_ERROR("condition","ItemEquipped condition requires to have non existing item (%u) equipped, skipped", value1);
                return false;
            }
            break;
        }
        case CONDITION_ZONEID:
        {
            AreaTableEntry const* areaEntry = GetAreaEntryByAreaID(value1);
            if(!areaEntry)
            {
                TC_LOG_ERROR("condition","Zone condition requires to be in non existing area (%u), skipped", value1);
                return false;
            }
            if(areaEntry->parentArea != 0)
            {
                TC_LOG_ERROR("condition","Zone condition requires to be in area (%u) which is a subzone but zone expected, skipped", value1);
                return false;
            }
            break;
        }
        case CONDITION_REPUTATION_RANK:
        {
            FactionEntry const* factionEntry = sFactionStore.LookupEntry(value1);
            if(!factionEntry)
            {
                TC_LOG_ERROR("condition","Reputation condition requires to have reputation non existing faction (%u), skipped", value1);
                return false;
            }
            break;
        }
        case CONDITION_TEAM:
        {
            if (value1 != TEAM_ALLIANCE && value1 != TEAM_HORDE)
            {
                TC_LOG_ERROR("condition","Team condition specifies unknown team (%u), skipped", value1);
                return false;
            }
            break;
        }
        case CONDITION_SKILL:
        {
            SkillLineEntry const *pSkill = sSkillLineStore.LookupEntry(value1);
            if (!pSkill)
            {
                TC_LOG_ERROR("condition","Skill condition specifies non-existing skill (%u), skipped", value1);
                return false;
            }
            if (value2 < 1 || value2 > sWorld->GetConfigMaxSkillValue() )
            {
                TC_LOG_ERROR("condition","Skill condition specifies invalid skill value (%u), skipped", value2);
                return false;
            }
            break;
        }
        case CONDITION_QUESTREWARDED:
        case CONDITION_QUESTTAKEN:
        {
            Quest const *Quest = sObjectMgr->GetQuestTemplate(value1);
            if (!Quest)
            {
                TC_LOG_ERROR("condition","Quest condition specifies non-existing quest (%u), skipped", value1);
                return false;
            }
            if(value2)
                TC_LOG_ERROR("condition","Quest condition has useless data in value2 (%u)!", value2);
            break;
        }
        case CONDITION_AD_COMMISSION_AURA:
        {
            if(value1)
                TC_LOG_ERROR("condition","Quest condition has useless data in value1 (%u)!", value1);
            if(value2)
                TC_LOG_ERROR("condition","Quest condition has useless data in value2 (%u)!", value2);
            break;
        }
        case CONDITION_NO_AURA:
        {
            if(!sSpellMgr->GetSpellInfo(value1))
            {
                TC_LOG_ERROR("condition","Aura condition requires to have non existing spell (Id: %d), skipped", value1);
                return false;
            }
            if(value2 > 2)
            {
                TC_LOG_ERROR("condition","Aura condition requires to have non existing effect index (%u) (must be 0..2), skipped", value2);
                return false;
            }
            break;
        }
        case CONDITION_ACTIVE_EVENT:
        {
            GameEvent::GameEventDataMap const& events = gameeventmgr.GetEventMap();
            if(value1 >=events.size() || !events[value1].isValid())
            {
                TC_LOG_ERROR("condition","Active event condition requires existed event id (%u), skipped", value1);
                return false;
            }
            break;
        }
        case CONDITION_INSTANCE_DATA:
            //TODO: need some check
            break;
    }
    return true;
}

SkillRangeType GetSkillRangeType(SkillLineEntry const *pSkill, bool racial)
{
    switch(pSkill->categoryId)
    {
        case SKILL_CATEGORY_LANGUAGES: return SKILL_RANGE_LANGUAGE;
        case SKILL_CATEGORY_WEAPON:
            if(pSkill->id!=SKILL_FIST_WEAPONS)
                return SKILL_RANGE_LEVEL;
            else
                return SKILL_RANGE_MONO;
        case SKILL_CATEGORY_ARMOR:
        case SKILL_CATEGORY_CLASS:
            if(pSkill->id != SKILL_POISONS && pSkill->id != SKILL_LOCKPICKING)
                return SKILL_RANGE_MONO;
            else
                return SKILL_RANGE_LEVEL;
        case SKILL_CATEGORY_SECONDARY:
        case SKILL_CATEGORY_PROFESSION:
            // not set skills for professions and racial abilities
            if(IsProfessionSkill(pSkill->id))
                return SKILL_RANGE_RANK;
            else if(racial)
                return SKILL_RANGE_NONE;
            else
                return SKILL_RANGE_MONO;
        default:
        case SKILL_CATEGORY_ATTRIBUTES:                     //not found in dbc
        case SKILL_CATEGORY_NOT_DISPLAYED:                  //only GENEREC(DND)
            return SKILL_RANGE_NONE;
    }
}

void ObjectMgr::LoadGameTele()
{
    m_GameTeleMap.clear();                                  // for reload case

    uint32 count = 0;
    QueryResult result = WorldDatabase.Query("SELECT id, position_x, position_y, position_z, orientation, map, name FROM game_tele");

    if( !result )
    {
        TC_LOG_ERROR("server.loading",">> Loaded `game_tele`, table is empty!");
        
        return;
    }

    do
    {
        Field *fields = result->Fetch();

        uint32 id         = fields[0].GetUInt32();

        GameTele gt;

        gt.position_x     = fields[1].GetFloat();
        gt.position_y     = fields[2].GetFloat();
        gt.position_z     = fields[3].GetFloat();
        gt.orientation    = fields[4].GetFloat();
        gt.mapId          = fields[5].GetUInt32();
        gt.name           = fields[6].GetString();

        if(!MapManager::IsValidMapCoord(gt.mapId,gt.position_x,gt.position_y,gt.position_z,gt.orientation))
        {
            TC_LOG_ERROR("sql.sql","Wrong position for id %u (name: %s) in `game_tele` table, ignoring.",id,gt.name.c_str());
            continue;
        }

        if(!Utf8toWStr(gt.name,gt.wnameLow))
        {
            TC_LOG_ERROR("sql.sql","Wrong UTF8 name for id %u in `game_tele` table, ignoring.",id);
            continue;
        }

        wstrToLower( gt.wnameLow );

        m_GameTeleMap[id] = gt;

        ++count;
    }
    while (result->NextRow());

    TC_LOG_INFO("server.loading", ">> Loaded %u game tele's", count );
    
}

GameTele const* ObjectMgr::GetGameTele(const std::string& name) const
{
    // explicit name case
    std::wstring wname;
    if(!Utf8toWStr(name,wname))
        return nullptr;

    // converting string that we try to find to lower case
    wstrToLower( wname );

    // Alternative first GameTele what contains wnameLow as substring in case no GameTele location found
    const GameTele* alt = nullptr;
    for(GameTeleMap::const_iterator itr = m_GameTeleMap.begin(); itr != m_GameTeleMap.end(); ++itr)
    {
        if(itr->second.wnameLow == wname)
            return &itr->second;
        else if (alt == nullptr && itr->second.wnameLow.find(wname) != std::wstring::npos)
            alt = &itr->second;
    }

    return alt;
}

bool ObjectMgr::AddGameTele(GameTele& tele)
{
    // find max id
    uint32 new_id = 0;
    for(GameTeleMap::const_iterator itr = m_GameTeleMap.begin(); itr != m_GameTeleMap.end(); ++itr)
        if(itr->first > new_id)
            new_id = itr->first;

    // use next
    ++new_id;

    if(!Utf8toWStr(tele.name,tele.wnameLow))
        return false;

    wstrToLower( tele.wnameLow );

    m_GameTeleMap[new_id] = tele;

    WorldDatabase.PExecute("INSERT INTO game_tele (id,position_x,position_y,position_z,orientation,map,name) VALUES (%u,%f,%f,%f,%f,%d,'%s')",
        new_id,tele.position_x,tele.position_y,tele.position_z,tele.orientation,tele.mapId,tele.name.c_str());
        
    return true;
}

bool ObjectMgr::DeleteGameTele(const std::string& name)
{
    // explicit name case
    std::wstring wname;
    if(!Utf8toWStr(name,wname))
        return false;

    // converting string that we try to find to lower case
    wstrToLower( wname );

    for(GameTeleMap::iterator itr = m_GameTeleMap.begin(); itr != m_GameTeleMap.end(); ++itr)
    {
        if(itr->second.wnameLow == wname)
        {
            WorldDatabase.PExecute("DELETE FROM game_tele WHERE name = '%s'",itr->second.name.c_str());
            m_GameTeleMap.erase(itr);
            return true;
        }
    }

    return false;
}

void ObjectMgr::LoadTrainerSpell()
{
    // For reload case
    for (CacheTrainerSpellMap::iterator itr = m_mCacheTrainerSpellMap.begin(); itr != m_mCacheTrainerSpellMap.end(); ++itr)
        itr->second.Clear();
    m_mCacheTrainerSpellMap.clear();

    std::set<uint32> skip_trainers;

    QueryResult result = WorldDatabase.Query("SELECT entry, spell,spellcost,reqskill,reqskillvalue,reqlevel FROM npc_trainer");

    if( !result )
    {
        TC_LOG_ERROR("server.loading",">> Loaded `npc_trainer`, table is empty!");
        
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        uint32 entry  = fields[0].GetUInt32();
        uint32 spell  = fields[1].GetUInt32();

        CreatureTemplate const* cInfo = GetCreatureTemplate(entry);

        if(!cInfo)
        {
            TC_LOG_ERROR("sql.sql","Table `npc_trainer` have entry for not existed creature template (Entry: %u), ignore", entry);
            continue;
        }

        if(!(cInfo->npcflag & UNIT_NPC_FLAG_TRAINER))
        {
            if(skip_trainers.count(entry) == 0)
            {
                TC_LOG_ERROR("sql.sql","Table `npc_trainer` have data for not creature template (Entry: %u) without trainer flag, ignore", entry);
                skip_trainers.insert(entry);
            }
            continue;
        }

        SpellEntry const *spellinfo = sSpellMgr->GetSpellInfo(spell);
        if(!spellinfo)
        {
            TC_LOG_ERROR("sql.sql","Table `npc_trainer` for Trainer (Entry: %u ) has non existing spell %u, ignore", entry,spell);
            continue;
        }

        if(!SpellMgr::IsSpellValid(spellinfo))
        {
            TC_LOG_ERROR("sql.sql","Table `npc_trainer` for Trainer (Entry: %u) has broken learning spell %u, ignore", entry, spell);
            continue;
        }

        TrainerSpell* pTrainerSpell = new TrainerSpell();
        pTrainerSpell->spell         = spell;
        pTrainerSpell->spellcost     = fields[2].GetUInt32();
        pTrainerSpell->reqskill      = fields[3].GetUInt32();
        pTrainerSpell->reqskillvalue = fields[4].GetUInt32();
        pTrainerSpell->reqlevel      = fields[5].GetUInt32();

        if(!pTrainerSpell->reqlevel)
            pTrainerSpell->reqlevel = spellinfo->SpellLevel;


        TrainerSpellData& data = m_mCacheTrainerSpellMap[entry];

        if(SpellMgr::IsProfessionSpell(spell))
            data.trainerType = 2;

        data.spellList.push_back(pTrainerSpell);
        ++count;

    } while (result->NextRow());

    TC_LOG_INFO("server.loading", ">> Loaded Trainers %d", count );
    
}

void ObjectMgr::LoadVendors()
{
    // For reload case
    for (CacheVendorItemMap::iterator itr = m_mCacheVendorItemMap.begin(); itr != m_mCacheVendorItemMap.end(); ++itr)
        itr->second.Clear();
    m_mCacheVendorItemMap.clear();

    std::set<uint32> skip_vendors;

    QueryResult result = WorldDatabase.Query("SELECT entry, item, maxcount, incrtime, ExtendedCost FROM npc_vendor");
    if( !result )
    {
        TC_LOG_ERROR("server.loading",">> Loaded `npc_vendor`, table is empty!");
        
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        uint32 entry        = fields[0].GetUInt32();
        uint32 item_id      = fields[1].GetUInt32();
        ItemTemplate const* proto = sObjectMgr->GetItemTemplate(item_id);
        if(!proto)
        {
            TC_LOG_ERROR("sql.sql","Table `npc_vendor` for Vendor (Entry: %u) have in item list non-existed item (%u), ignore",entry,item_id);
            continue;
        }
        uint32 maxcount     = fields[2].GetUInt32();
        uint32 incrtime     = fields[3].GetUInt32();
        uint32 ExtendedCost = fields[4].GetUInt32();

        if(!IsVendorItemValid(entry,proto,maxcount,incrtime,ExtendedCost,NULL,&skip_vendors))
            continue;

        VendorItemData& vList = m_mCacheVendorItemMap[entry];

        vList.AddItem(proto,maxcount,incrtime,ExtendedCost);
        ++count;

    } while (result->NextRow());

    TC_LOG_INFO("server.loading", ">> Loaded %d Vendors ", count );
    
}

void ObjectMgr::LoadNpcTextId()
{
    m_mCacheNpcTextIdMap.clear();

    QueryResult result = WorldDatabase.Query("SELECT npc_guid, textid FROM npc_gossip");
    if( !result )
    {
        TC_LOG_ERROR("server.loading",">> Loaded `npc_gossip`, table is empty!");
        
        return;
    }

    uint32 count = 0;
    uint32 guid,textid;
    do
    {
        Field* fields = result->Fetch();

        guid   = fields[0].GetUInt32();
        textid = fields[1].GetUInt32();

        if (!GetCreatureData(guid))
        {
            TC_LOG_ERROR("sql.sql","Table `npc_gossip` have not existed creature (GUID: %u) entry, ignore. ",guid);
            continue;
        }
        if (!GetGossipText(textid))
        {
            TC_LOG_ERROR("sql.sql","Table `npc_gossip` for creature (GUID: %u) have wrong Textid (%u), ignore. ", guid, textid);
            continue;
        }

        m_mCacheNpcTextIdMap[guid] = textid ;
        ++count;

    } while (result->NextRow());

    TC_LOG_INFO("server.loading", ">> Loaded %d NpcTextId ", count );
    
}

void ObjectMgr::LoadNpcOptions()
{
    m_mCacheNpcOptionList.clear();                          // For reload case

    QueryResult result = WorldDatabase.Query(
        //      0  1         2       3    4      5         6     7           8
        "SELECT id,gossip_id,npcflag,icon,action,box_money,coded,option_text,box_text "
        "FROM npc_option");

    if( !result )
    {
        TC_LOG_ERROR("server.loading",">> Loaded `npc_option`, table is empty!");
        
        return;
    }

    uint32 count = 0;

    do
    {
        Field* fields = result->Fetch();

        GossipOption go;
        go.Id               = fields[0].GetUInt32();
        go.GossipId         = fields[1].GetUInt32();
        go.NpcFlag          = fields[2].GetUInt32();
        go.Icon             = fields[3].GetUInt32();
        go.Action           = fields[4].GetUInt32();
        go.BoxMoney         = fields[5].GetUInt32();
        go.Coded            = fields[6].GetUInt8()!=0;
        go.OptionText       = fields[7].GetString();
        go.BoxText          = fields[8].GetString();

        m_mCacheNpcOptionList.push_back(go);

        ++count;

    } while (result->NextRow());

    TC_LOG_INFO("server.loading", ">> Loaded %d npc_option entries", count );
    
}

void ObjectMgr::AddVendorItem( uint32 entry, ItemTemplate const *proto, uint32 maxcount, uint32 incrtime, uint32 extendedcost, bool savetodb)
{
    VendorItemData& vList = m_mCacheVendorItemMap[entry];
    vList.AddItem(proto,maxcount,incrtime,extendedcost);

    if(savetodb) WorldDatabase.PExecute("INSERT INTO npc_vendor (entry,item,maxcount,incrtime,extendedcost) VALUES('%u','%u','%u','%u','%u')",entry, proto->ItemId, maxcount,incrtime,extendedcost);
}

bool ObjectMgr::RemoveVendorItem( uint32 entry, ItemTemplate const *proto, bool savetodb)
{
    CacheVendorItemMap::iterator  iter = m_mCacheVendorItemMap.find(entry);
    if(iter == m_mCacheVendorItemMap.end())
        return false;

    if(!iter->second.FindItem(proto->ItemId))
        return false;

    iter->second.RemoveItem(proto->ItemId);
    if(savetodb) WorldDatabase.PExecute("DELETE FROM npc_vendor WHERE entry='%u' AND item='%u'",entry, proto->ItemId);
    return true;
}

bool ObjectMgr::IsVendorItemValid( uint32 vendor_entry, ItemTemplate const *proto, uint32 maxcount, uint32 incrtime, uint32 ExtendedCost, Player* pl, std::set<uint32>* skip_vendors, uint32 ORnpcflag ) const
{
    CreatureTemplate const* cInfo = sObjectMgr->GetCreatureTemplate(vendor_entry);
    if(!cInfo)
    {
        if(pl)
            ChatHandler(pl).SendSysMessage(LANG_COMMAND_VENDORSELECTION);
        else
            TC_LOG_ERROR("sql.sql","Table `(game_event_)npc_vendor` have data for not existed creature template (Entry: %u), ignore", vendor_entry);
        return false;
    }

    if(!((cInfo->npcflag | ORnpcflag) & UNIT_NPC_FLAG_VENDOR))
    {
        if(!skip_vendors || skip_vendors->count(vendor_entry)==0)
        {
            if(pl)
                ChatHandler(pl).SendSysMessage(LANG_COMMAND_VENDORSELECTION);
            else
                TC_LOG_ERROR("sql.sql","Table `(game_event_)npc_vendor` have data for not creature template (Entry: %u) without vendor flag, ignore", vendor_entry);

            if(skip_vendors)
                skip_vendors->insert(vendor_entry);
        }
        return false;
    }

    if(ExtendedCost && !sItemExtendedCostStore.LookupEntry(ExtendedCost))
    {
        if(pl)
            ChatHandler(pl).PSendSysMessage(LANG_EXTENDED_COST_NOT_EXIST,ExtendedCost);
        else
            TC_LOG_ERROR("sql.sql","Table `(game_event_)npc_vendor` have Item (Entry: %u) with wrong ExtendedCost (%u) for vendor (%u), ignore",proto->ItemId,ExtendedCost,vendor_entry);
        return false;
    }

    if(maxcount > 0 && incrtime == 0)
    {
        if(pl)
            ChatHandler(pl).PSendSysMessage("MaxCount!=0 (%u) but IncrTime==0", maxcount);
        else
            TC_LOG_ERROR( "sql.sql","Table `(game_event_)npc_vendor` has `maxcount` (%u) for item %u of vendor (Entry: %u) but `incrtime`=0, ignore", maxcount, proto->ItemId, vendor_entry);
        return false;
    }
    else if(maxcount==0 && incrtime > 0)
    {
        if(pl)
            ChatHandler(pl).PSendSysMessage("MaxCount==0 but IncrTime<>=0");
        else
            TC_LOG_ERROR( "FIXME","Table `(game_event_)npc_vendor` has `maxcount`=0 for item %u of vendor (Entry: %u) but `incrtime`<>0, ignore", proto->ItemId, vendor_entry);
        return false;
    }

    VendorItemData const* vItems = GetNpcVendorItemList(vendor_entry);
    if(!vItems)
        return true;                                        // later checks for non-empty lists

    if(vItems->FindItem(proto->ItemId))
    {
        if(pl)
            ChatHandler(pl).PSendSysMessage(LANG_ITEM_ALREADY_IN_LIST,proto->ItemId);
        else
            TC_LOG_ERROR("sql.sql", "Table `(game_event_)npc_vendor` has duplicate items %u for vendor (Entry: %u), ignore", proto->ItemId, vendor_entry);
        return false;
    }

    if(vItems->GetItemCount() >= MAX_VENDOR_ITEMS)
    {
        if(pl)
            ChatHandler(pl).SendSysMessage(LANG_COMMAND_ADDVENDORITEMITEMS);
        else
            TC_LOG_ERROR("sql.sql", "Table `npc_vendor` has too many items (%u >= %i) for vendor (Entry: %u), ignore", vItems->GetItemCount(), MAX_VENDOR_ITEMS, vendor_entry);
        return false;
    }

    return true;
}

void ObjectMgr::LoadScriptNames()
{
    m_scriptNames.push_back("");
    QueryResult result = WorldDatabase.Query(
      "SELECT DISTINCT(ScriptName) FROM creature_template WHERE ScriptName <> '' "
      "UNION "
      "SELECT DISTINCT(ScriptName) FROM gameobject_template WHERE ScriptName <> '' "
      "UNION "
      "SELECT DISTINCT(ScriptName) FROM item_template WHERE ScriptName <> '' "
      "UNION "
      "SELECT DISTINCT(ScriptName) FROM areatrigger_scripts WHERE ScriptName <> '' "
      "UNION "
      "SELECT DISTINCT(script) FROM instance_template WHERE script <> ''");
    
    //OnEvent Changes
    m_scriptNames.push_back("scripted_on_events"); 
    
    if(result)
    {
        do
        {
            m_scriptNames.push_back((*result)[0].GetString());
        } while (result->NextRow());
    }

    std::sort(m_scriptNames.begin(), m_scriptNames.end());
    
    // New system
    QueryResult resultnew = WorldDatabase.Query("SELECT entryorguid, scriptname FROM creature_scripts");
    Field* fields;
    if (resultnew) {
        do {
            fields = resultnew->Fetch();
            
            int32 entryorguid = fields[0].GetInt32();
            std::string scriptname = fields[1].GetString();
            
            if (entryorguid > 0)    // By entry
                m_creatureScriptsByEntry[uint32(entryorguid)] = scriptname;
            else                    // By GUID
                m_creatureScriptsByGUID[uint32(-entryorguid)] = scriptname;
        } while (resultnew->NextRow());
    }
    else
        TC_LOG_DEBUG("server.loading","ObjectMgr::LoadScriptNames: Table `creature_scripts` is empty!");
}

uint32 ObjectMgr::GetScriptId(const char *name)
{
    // use binary search to find the script name in the sorted vector
    // assume "" is the first element
    if(!name) return 0;
    ScriptNameMap::const_iterator itr =
        std::lower_bound(m_scriptNames.begin(), m_scriptNames.end(), name);
    if(itr == m_scriptNames.end() || *itr != name) return 0;
    return itr - m_scriptNames.begin();
}

void ObjectMgr::CheckScripts(ScriptMapMap const& scripts,std::set<int32>& ids)
{
    for(ScriptMapMap::const_iterator itrMM = scripts.begin(); itrMM != scripts.end(); ++itrMM)
    {
        for(ScriptMap::const_iterator itrM = itrMM->second.begin(); itrM != itrMM->second.end(); ++itrM)
        {
            if(itrM->second.dataint)
            {
                if(!GetTrinityStringLocale (itrM->second.dataint))
                    TC_LOG_ERROR("sql.sql", "Table `db_script_string` has not existed string id  %u", itrM->first);

                if(ids.count(itrM->second.dataint))
                    ids.erase(itrM->second.dataint);
            }
        }
    }
}

void ObjectMgr::LoadDbScriptStrings()
{
    LoadTrinityStrings(WorldDatabase,"db_script_string",MIN_DB_SCRIPT_STRING_ID,MAX_DB_SCRIPT_STRING_ID);

    std::set<int32> ids;

    for(int32 i = MIN_DB_SCRIPT_STRING_ID; i < MAX_DB_SCRIPT_STRING_ID; ++i)
        if(GetTrinityStringLocale(i))
            ids.insert(i);

    CheckScripts(sQuestEndScripts,ids);
    CheckScripts(sQuestStartScripts,ids);
    CheckScripts(sSpellScripts,ids);
    CheckScripts(sGameObjectScripts,ids);
    CheckScripts(sEventScripts,ids);

    CheckScripts(sWaypointScripts,ids);

    for(std::set<int32>::const_iterator itr = ids.begin(); itr != ids.end(); ++itr)
        TC_LOG_ERROR("sql.sql","Table `db_script_string` has unused string id  %u", *itr);
}

// Functions for scripting access
uint32 GetAreaTriggerScriptId(uint32 trigger_id)
{
    return sObjectMgr->GetAreaTriggerScriptId(trigger_id);
}

bool LoadTrinityStrings(WorldDatabaseWorkerPool& db, char const* table,int32 start_value, int32 end_value)
{
    if(start_value >= 0 || start_value <= end_value)        // start/end reversed for negative values
    {
        TC_LOG_ERROR("server.loading","Table '%s' attempt loaded with invalid range (%d - %d), use (%d - %d) instead.",table,start_value,end_value,-1,std::numeric_limits<int32>::min());
        start_value = -1;
        end_value = std::numeric_limits<int32>::min();
    }

    // for scripting localized strings allowed use _only_ negative entries
    return sObjectMgr->LoadTrinityStrings(db,table,end_value,start_value);
}

uint32 GetScriptId(const char *name)
{
    return sObjectMgr->GetScriptId(name);
}

ObjectMgr::ScriptNameMap & GetScriptNames()
{
    return sObjectMgr->GetScriptNames();
}

CreatureTemplate const* GetCreatureTemplateStore(uint32 entry)
{
    return sObjectMgr->GetCreatureTemplate(entry);
}

Quest const* GetQuestTemplateStore(uint32 entry)
{
    return sObjectMgr->GetQuestTemplate(entry);
}

uint64 ObjectMgr::GenerateGMTicketId()
{
  return ++m_GMticketid;
}

void ObjectMgr::LoadGMTickets()
{
  m_GMTicketList.clear();

  QueryResult result = CharacterDatabase.Query( "SELECT `guid`, `playerGuid`, `message`, `createtime`, `map`, `posX`, `posY`, `posZ`, `timestamp`, `closed`, `assignedto`, `comment` FROM `gm_tickets`" );

  if(!result)
  {
    TC_LOG_INFO("server.loading"," \n>> GM Tickets table is empty, no tickets were loaded.\n" );
    
    return;
  }

  uint16 count = 0;
  GM_Ticket *ticket;
  do
  {
    Field *fields = result->Fetch();
    ticket = new GM_Ticket;
    ticket->guid = fields[0].GetUInt64();
    ticket->playerGuid = fields[1].GetUInt64();
    ticket->message = fields[2].GetString();
    ticket->createtime = fields[3].GetUInt64();
    ticket->map = fields[4].GetUInt32();
    ticket->pos_x = fields[5].GetFloat();
    ticket->pos_y = fields[6].GetFloat();
    ticket->pos_z = fields[7].GetFloat();
    ticket->timestamp = fields[8].GetUInt64();
    ticket->closed = fields[9].GetUInt64();
    ticket->assignedToGM = fields[10].GetUInt64();
    ticket->comment = fields[11].GetString();
    ++count;

    m_GMTicketList.push_back(ticket);

  } while( result->NextRow() );

  result = CharacterDatabase.PQuery("SELECT MAX(`guid`) from `gm_tickets`");
  m_GMticketid = (*result)[0].GetUInt64(); 

  TC_LOG_INFO("server.loading",">>> %u GM Tickets loaded from the database.", count);
  
}

void ObjectMgr::AddOrUpdateGMTicket(GM_Ticket &ticket, bool create)
{
  if(create)
    m_GMTicketList.push_back(&ticket);

  _AddOrUpdateGMTicket(ticket);    
}

void ObjectMgr::_AddOrUpdateGMTicket(GM_Ticket &ticket)
{
    std::string msg(ticket.message);
    std::string comment(ticket.comment); 
    CharacterDatabase.EscapeString(msg);
    CharacterDatabase.EscapeString(comment);
    std::ostringstream ss;
    ss << "REPLACE INTO `gm_tickets` (`guid`, `playerGuid`, `message`, `createtime`, `map`, `posX`, `posY`, `posZ`, `timestamp`, `closed`, `assignedto`, `comment`) VALUES('";
    ss << ticket.guid << "', '";
    ss << ticket.playerGuid << "', '";
    ss << msg << "', '" ;
    ss << ticket.createtime << "', '";
    ss << ticket.map << "', '";
    ss << ticket.pos_x << "', '";
    ss << ticket.pos_y << "', '";
    ss << ticket.pos_z << "', '";
    ss << ticket.timestamp << "', '";
    ss << ticket.closed << "', '";
    ss << ticket.assignedToGM << "', '";
    ss << comment << "');";

    CharacterDatabase.Execute(ss.str().c_str());
}

void ObjectMgr::RemoveGMTicket(GM_Ticket *ticket, int64 source, bool permanently)
{
  for(GmTicketList::iterator i = m_GMTicketList.begin(); i != m_GMTicketList.end(); ++i)
    if((*i)->guid == ticket->guid) 
    {
      if(permanently)
      {
        CharacterDatabase.PExecute("DELETE FROM `gm_tickets` WHERE `guid` = '%u'", ticket->guid);
        i = m_GMTicketList.erase(i);
        ticket = NULL;
        return;
      }
      (*i)->closed = source;
      _AddOrUpdateGMTicket(*(*i)); 
    }
}

void ObjectMgr::RemoveGMTicket(uint64 ticketGuid, int64 source, bool permanently)
{
  GM_Ticket *ticket = GetGMTicket(ticketGuid);
  assert( ticket );
  RemoveGMTicket(ticket, source, permanently);
}

void ObjectMgr::LoadSpellTemplates()
{
    uint32 count = 0;
    uint32 id;
    Field* fields;
    
    QueryResult result = WorldDatabase.Query("SELECT entry, category, dispel, mechanic, attributes, attributesEx, attributesEx2, attributesEx3, attributesEx4, attributesEx5, attributesEx6, "
        "stances, stancesNot, targets, targetCreatureType, requiresSpellFocus, facingCasterFlags, casterAuraState, targetAuraState, casterAuraStateNot, targetAuraStateNot, castingTimeIndex, recoveryTime, "
        "categoryRecoveryTime, interruptFlags, auraInterruptFlags, channelInterruptFlags, ProcFlags, procChance, procCharges, MaxLevel, BaseLevel, SpellLevel, durationIndex, powerType, ManaCost, "
        "ManaCostPerlevel, manaPerSecond, ManaPerSecondPerLevel, rangeIndex, speed, stackAmount, totem1, totem2, reagent1, reagent2, reagent3, reagent4, reagent5, reagent6, reagent7, "
        "reagent8, reagentCount1, reagentCount2, reagentCount3, reagentCount4, reagentCount5, reagentCount6, reagentCount7, reagentCount8, equippedItemClass, equippedItemSubClassMask, "
        "equippedItemInventoryTypeMask, effect1, effect2, effect3, effectDieSides1, effectDieSides2, effectDieSides3, effectBaseDice1, effectBaseDice2, effectBaseDice3, effectDicePerLevel1, "
        "effectDicePerLevel2, effectDicePerLevel3, effectRealPointsPerLevel1, effectRealPointsPerLevel2, effectRealPointsPerLevel3, effectBasePoints1, effectBasePoints2, effectBasePoints3, effectMechanic1, "
        "effectMechanic2, effectMechanic3, effectImplicitTargetA1, effectImplicitTargetA2, effectImplicitTargetA3, effectImplicitTargetB1, effectImplicitTargetB2, effectImplicitTargetB3, effectRadiusIndex1, "
        "effectRadiusIndex2, effectRadiusIndex3, effectApplyAuraName1, effectApplyAuraName2, effectApplyAuraName3, effectAmplitude1, effectAmplitude2, effectAmplitude3, effectMultipleValue1, "
        "effectMultipleValue2, effectMultipleValue3, effectChainTarget1, effectChainTarget2, effectChainTarget3, effectItemType1, effectItemType2, effectItemType3, effectMiscValue1, effectMiscValue2, "
        "effectMiscValue3, effectMiscValueB1, effectMiscValueB2, effectMiscValueB3, effectTriggerSpell1, effectTriggerSpell2, effectTriggerSpell3, effectPointsPerComboPoint1, effectPointsPerComboPoint2, "
        "effectPointsPerComboPoint3, spellVisual, spellIconID, activeIconID, spellName1, spellName2, spellName3, spellName4, spellName5, spellName6, spellName7, spellName8, "
        "spellName9, spellName10, spellName11, spellName12, spellName13, spellName14, spellName15, spellName16, rank1, rank2, rank3, rank4, rank5, rank6, rank7, rank8, rank9, rank10, "
        "rank11, rank12, rank13, rank14, rank15, rank16, ManaCostPercentage, startRecoveryCategory, startRecoveryTime, "
        "maxTargetLevel, spellFamilyName, spellFamilyFlags1, spellFamilyFlags2, maxAffectedTargets, dmgClass, preventionType, dmgMultiplier1, dmgMultiplier2, dmgMultiplier3, "
        "totemCategory1, totemCategory2, areaId, schoolMask FROM spell_template ORDER BY entry");
        
    if (!result) {
        TC_LOG_INFO("server.loading","Table spell_template is empty!");
        return;
    }

    do {
        fields = result->Fetch();
        id = fields[0].GetUInt32();        
        std::map<uint32, SpellEntry*>::iterator itr = spellTemplates.find(id);
        SpellEntry* spell = NULL;
        if (itr != spellTemplates.end()) { // Already existing
            spell = itr->second;
        }
        else {
            spell = new SpellEntry();
        }

        spell->Id = fields[0].GetUInt32();
        spell->Category = fields[1].GetUInt32();
        spell->Dispel = fields[2].GetUInt32();
        spell->Mechanic = fields[3].GetUInt32();
        spell->Attributes = fields[4].GetUInt32();
        spell->AttributesEx = fields[5].GetUInt32();
        spell->AttributesEx2 = fields[6].GetUInt32();
        spell->AttributesEx3 = fields[7].GetUInt32();
        spell->AttributesEx4 = fields[8].GetUInt32();
        spell->AttributesEx5 = fields[9].GetUInt32();
        spell->AttributesEx6 = fields[10].GetUInt32();
        spell->Stances = fields[11].GetUInt32();
        spell->StancesNot = fields[12].GetUInt32();
        spell->Targets = fields[13].GetUInt32();
        spell->TargetCreatureType = fields[14].GetUInt32();
        spell->RequiresSpellFocus = fields[15].GetUInt32();
        spell->FacingCasterFlags = fields[16].GetUInt32();
        spell->CasterAuraState = fields[17].GetUInt32();
        spell->TargetAuraState = fields[18].GetUInt32();
        spell->CasterAuraStateNot = fields[19].GetUInt32();
        spell->TargetAuraStateNot = fields[20].GetUInt32();
        spell->CastingTimeIndex = fields[21].GetUInt32();
        spell->RecoveryTime = fields[22].GetUInt32();
        spell->CategoryRecoveryTime = fields[23].GetUInt32();
        spell->InterruptFlags = fields[24].GetUInt32();
        spell->AuraInterruptFlags = fields[25].GetUInt32();
        spell->ChannelInterruptFlags = fields[26].GetUInt32();
        spell->ProcFlags = fields[27].GetUInt32();
        spell->procChance = fields[28].GetUInt32();
        spell->procCharges = fields[29].GetUInt32();
        spell->MaxLevel = fields[30].GetUInt32();
        spell->BaseLevel = fields[31].GetUInt32();
        spell->SpellLevel = fields[32].GetUInt32();
        spell->DurationIndex = fields[33].GetUInt32();
        spell->powerType = fields[34].GetUInt32();
        spell->ManaCost = fields[35].GetUInt32();
        spell->ManaCostPerlevel = fields[36].GetUInt32();
        spell->manaPerSecond = fields[37].GetUInt32();
        spell->ManaPerSecondPerLevel = fields[38].GetUInt32();
        spell->rangeIndex = fields[39].GetUInt32();
        spell->speed = fields[40].GetFloat();
        spell->StackAmount = fields[41].GetUInt32();
        spell->Totem[0] = fields[42].GetUInt32();
        spell->Totem[1] = fields[43].GetUInt32();
        for (uint8 i = 0; i < 8; i++) {
            spell->Reagent[i] = fields[i+44].GetInt32();
            spell->ReagentCount[i] = fields[i+52].GetUInt32();
        }
        spell->EquippedItemClass = fields[60].GetInt32();
        spell->EquippedItemSubClassMask = fields[61].GetInt32();
        spell->EquippedItemInventoryTypeMask = fields[62].GetInt32();
        for (uint8 i = 0; i < 3; i++) {
            spell->Effect[i] = fields[i+63].GetUInt32();
            spell->EffectDieSides[i] = fields[i+66].GetInt32();
            spell->EffectBaseDice[i] = fields[i+69].GetInt32();
            spell->EffectDicePerLevel[i] = fields[i+72].GetFloat();
            spell->EffectRealPointsPerLevel[i] = fields[i+75].GetFloat();
            spell->EffectBasePoints[i] = fields[i+78].GetInt32();
            spell->EffectMechanic[i] = fields[i+81].GetUInt32();
            spell->EffectImplicitTargetA[i] = fields[i+84].GetUInt32();
            spell->EffectImplicitTargetB[i] = fields[i+87].GetUInt32();
            spell->EffectRadiusIndex[i] = fields[i+90].GetUInt32();
            spell->EffectApplyAuraName[i] = fields[i+93].GetUInt32();
            spell->EffectAmplitude[i] = fields[i+96].GetUInt32();
            spell->EffectValueMultiplier[i] = fields[i+99].GetFloat();
            spell->EffectChainTarget[i] = fields[i+102].GetUInt32();
            spell->EffectItemType[i] = fields[i+105].GetUInt32();
            spell->EffectMiscValue[i] = fields[i+108].GetInt32();
            spell->EffectMiscValueB[i] = fields[i+111].GetInt32();
            spell->EffectTriggerSpell[i] = fields[i+114].GetUInt32();
            spell->EffectPointsPerComboPoint[i] = fields[i+117].GetFloat();
        }
        spell->SpellVisual = fields[120].GetUInt32();
        spell->SpellIconID = fields[121].GetUInt32();
        spell->activeIconID = fields[122].GetUInt32();
        for (uint8 i = 0; i < 16; i++) {
            strcpy(spell->SpellName[i], fields[i+123].GetCString());
            if (strcmp(spell->SpellName[i], "0.000000") == 0)
                strcpy(spell->SpellName[i], "");
            strcpy(spell->Rank[i], fields[i+139].GetCString());
            if (strcmp(spell->Rank[i], "0.000000") == 0)
                strcpy(spell->Rank[i], "");
        }
        spell->ManaCostPercentage = fields[155].GetUInt32();
        spell->StartRecoveryCategory = fields[156].GetUInt32();
        spell->StartRecoveryTime = fields[157].GetUInt32();
        spell->MaxTargetLevel = fields[158].GetUInt32();
        spell->SpellFamilyName = fields[159].GetUInt32();
        uint32 col200 = fields[160].GetUInt32();
        uint32 col201 = fields[161].GetUInt32();
        spell->SpellFamilyFlags = ((uint64)col201 << 32) | (uint64)col200;
        spell->MaxAffectedTargets = fields[162].GetUInt32();
        spell->DmgClass = fields[163].GetUInt32();
        spell->PreventionType = fields[164].GetUInt32();
        for (uint8 i = 0; i < 3; i++)
            spell->EffectDamageMultiplier[i] = fields[i+165].GetFloat();
        spell->TotemCategory[0] = fields[168].GetUInt32();
        spell->TotemCategory[1] = fields[169].GetUInt32();
        spell->AreaId = fields[170].GetUInt32();
        spell->SchoolMask = fields[171].GetUInt32();

        spellTemplates[id] = spell;
        maxSpellId = id;
        count++;
    } while (result->NextRow());
    
    for (std::map<uint32, SpellEntry*>::iterator itr = spellTemplates.begin(); itr != spellTemplates.end(); itr++) {
        SpellEntry const * spell = itr->second;
        if(spell && spell->Category)
            sSpellsByCategoryStore[spell->Category].insert(itr->first);

        // DBC not support uint64 fields but SpellEntry have SpellFamilyFlags mapped at 2 uint32 fields
        // uint32 field already converted to bigendian if need, but must be swapped for correct uint64 bigendian view
        #if TRINITY_ENDIAN == TRINITY_BIGENDIAN
        std::swap(*((uint32*)(&spell->SpellFamilyFlags)),*(((uint32*)(&spell->SpellFamilyFlags))+1));
        #endif
    }

    for (uint32 j = 0; j < sSkillLineAbilityStore.GetNumRows(); ++j) {
        SkillLineAbilityEntry const *skillLine = sSkillLineAbilityStore.LookupEntry(j);

        if(!skillLine)
            continue;
        SpellEntry const* spellInfo = sSpellMgr->GetSpellInfo(skillLine->spellId);
        if(spellInfo && (spellInfo->Attributes & 0x1D0) == 0x1D0) {
            for (unsigned int i = 1; i < sCreatureFamilyStore.GetNumRows(); ++i)
            {
                CreatureFamilyEntry const* cFamily = sCreatureFamilyStore.LookupEntry(i);
                if(!cFamily)
                    continue;

                if(skillLine->skillId != cFamily->skillLine[0] && skillLine->skillId != cFamily->skillLine[1])
                    continue;

                sPetFamilySpellsStore[i].insert(spellInfo->Id);
            }
        }
    }
    
    TC_LOG_INFO("server.loading",">> Loaded %u spell templates.", count);
    
}

SpellEntry* ObjectMgr::GetSpellTemplate(uint32 id)
{
    std::map<uint32, SpellEntry*>::const_iterator itr = spellTemplates.find(id);
    if (itr != spellTemplates.end())
        return itr->second;
        
    return NULL;
}

void ObjectMgr::LoadAreaFlagsOverridenData()
{
    AreaTableEntry* areaEntry;
    uint32 count = 0;
    for (uint32 areaflag = 0; areaflag < sAreaStore.GetNumRows(); ++areaflag) {
        areaEntry = (AreaTableEntry*) sAreaStore.LookupEntry(areaflag);
        if (!areaEntry)
            continue;
        
        /*switch (areaEntry->ID) {
        case 1637: // Orgrimmar
            areaEntry->flags |= AREA_FLAG_OUTSIDE;
            ++count;
            break;
        }*/
    }
    
    TC_LOG_INFO("server.loading",">> Loaded %u overriden data.", count);
    
}

void ObjectMgr::LoadFactionChangeItems()
{
    factionchange_items.clear();
    
    QueryResult result = WorldDatabase.Query("SELECT alliance_id, horde_id FROM player_factionchange_items");

    if (!result)
    {
        TC_LOG_INFO("server.loading",">> Loaded 0 faction change items. DB table `player_factionchange_items` is empty.");
        
        return;
    }

    uint32 count = 0;

    do
    {
        Field* fields = result->Fetch();

        uint32 alliance = fields[0].GetUInt32();
        uint32 horde = fields[1].GetUInt32();

        // TODO: add item template validation
        /*if (!GetItemTemplate(alliance))
            TC_LOG_ERROR("sql.sql","Item %u referenced in `player_factionchange_items` does not exist, pair skipped!", alliance);
        else if (!GetItemTemplate(horde))
            TC_LOG_ERROR("sql.sql","Item %u referenced in `player_factionchange_items` does not exist, pair skipped!", horde);
        else*/
            factionchange_items[alliance] = horde;

        ++count;
    }
    while (result->NextRow());

    TC_LOG_INFO("server.loading",">> Loaded %u faction change items", count);
    
}

void ObjectMgr::LoadFactionChangeSpells()
{
    factionchange_spells.clear();
    
    QueryResult result = WorldDatabase.Query("SELECT alliance_id, horde_id FROM player_factionchange_spells");

    if (!result)
    {
        TC_LOG_ERROR("server.loading",">> Loaded 0 faction change spells. DB table `player_factionchange_spells` is empty.");
        
        return;
    }

    uint32 count = 0;

    do
    {
        Field* fields = result->Fetch();

        uint32 alliance = fields[0].GetUInt32();
        uint32 horde = fields[1].GetUInt32();

        if (!sSpellMgr->GetSpellInfo(alliance))
            TC_LOG_ERROR("sql.sql","Spell %u referenced in `player_factionchange_spells` does not exist, skipped!", alliance);
        else if (!sSpellMgr->GetSpellInfo(horde))
            TC_LOG_ERROR("sql.sql","Spell %u referenced in `player_factionchange_spells` does not exist, skipped!", horde);
        else
            factionchange_spells[alliance] = horde;

        ++count;
    }
    while (result->NextRow());

    TC_LOG_INFO("server.loading",">> Loaded %u faction change spells", count);
    
}

void ObjectMgr::LoadFactionChangeTitles()
{
    factionchange_titles.clear();
    
    QueryResult result = WorldDatabase.Query("SELECT alliance_id, horde_id FROM player_factionchange_titles");

    if (!result)
    {
        TC_LOG_ERROR("server.loading",">> Loaded 0 faction change titles. DB table `player_factionchange_titles` is empty.");
        
        return;
    }

    uint32 count = 0;

    do
    {
        Field* fields = result->Fetch();

        uint32 alliance = fields[0].GetUInt32();
        uint32 horde = fields[1].GetUInt32();

        factionchange_titles[alliance] = horde;

        ++count;
    }
    while (result->NextRow());

    TC_LOG_INFO("server.loading",">> Loaded %u faction change titles", count);
    
}

void ObjectMgr::LoadFactionChangeQuests()
{
    factionchange_quests.clear();
    
    QueryResult result = WorldDatabase.Query("SELECT alliance_id, horde_id FROM player_factionchange_quests");

    if (!result)
    {
        TC_LOG_ERROR("server.loading",">> Loaded 0 faction change quest. DB table `player_factionchange_quests` is empty.");
        
        return;
    }

    uint32 count = 0;

    do
    {
        Field* fields = result->Fetch();

        uint32 alliance = fields[0].GetUInt32();
        uint32 horde = fields[1].GetUInt32();

        factionchange_quests[alliance] = horde;

        ++count;
    }
    while (result->NextRow());

    TC_LOG_INFO("server.loading",">> Loaded %u faction change quests", count);
    
}

void ObjectMgr::LoadFactionChangeReputGeneric()
{
    factionchange_reput_generic.clear();
    
    QueryResult result = WorldDatabase.Query("SELECT alliance_id, horde_id FROM player_factionchange_reputations_generic");

    if (!result)
    {
        TC_LOG_INFO("server.loading",">> Loaded 0 faction change reputations (generic). DB table `player_factionchange_reputations_generic` is empty.");
        
        return;
    }

    uint32 count = 0;

    do
    {
        Field* fields = result->Fetch();

        uint32 alliance = fields[0].GetUInt32();
        uint32 horde = fields[1].GetUInt32();

        // TODO: add item template validation
        /*if (!GetItemTemplate(alliance))
            TC_LOG_ERROR("sql.sql","Item %u referenced in `player_factionchange_items` does not exist, pair skipped!", alliance);
        else if (!GetItemTemplate(horde))
            TC_LOG_ERROR("sql.sql","Item %u referenced in `player_factionchange_items` does not exist, pair skipped!", horde);
        else*/
            factionchange_reput_generic[alliance] = horde;

        ++count;
    }
    while (result->NextRow());

    TC_LOG_INFO("server.loading",">> Loaded %u faction change reputations (generic)", count);
    
}

void ObjectMgr::AddLocaleString(std::string const& s, LocaleConstant locale, StringVector& data)
{
    if (!s.empty())
    {
        if (data.size() <= size_t(locale))
            data.resize(locale + 1);

        data[locale] = s;
    }
}

CreatureBaseStats const* ObjectMgr::GetCreatureBaseStats(uint8 level, uint8 unitClass)
{
    CreatureBaseStatsContainer::const_iterator it = _creatureBaseStatsStore.find(MAKE_PAIR16(level, unitClass));

    if (it != _creatureBaseStatsStore.end())
        return &(it->second);

    struct DefaultCreatureBaseStats : public CreatureBaseStats
    {
        DefaultCreatureBaseStats()
        {
            BaseArmor = 1;
            for (uint8 j = 0; j < MAX_EXPANSIONS; ++j)
            {
                BaseHealth[j] = 1;
                BaseDamage[j] = 0.0f;
            }
            BaseMana = 0;
            AttackPower = 0;
            RangedAttackPower = 0;
        }
    };
    static const DefaultCreatureBaseStats defStats;
    return &defStats;
}

void ObjectMgr::LoadCreatureClassLevelStats()
{
    uint32 oldMSTime = GetMSTime();

    QueryResult result = WorldDatabase.Query("SELECT level, class, basehp0, basehp1, basehp2, basemana, basearmor, attackpower, rangedattackpower, damage_base, damage_exp1, damage_exp2 FROM creature_classlevelstats");

    if (!result)
    {
        TC_LOG_INFO("server.loading", ">> Loaded 0 creature base stats. DB table `creature_classlevelstats` is empty.");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        uint8 Level = fields[0].GetUInt8();
        uint8 Class = fields[1].GetUInt8();

        if (!Class || ((1 << (Class - 1)) & CLASSMASK_ALL_CREATURES) == 0)
            TC_LOG_ERROR("sql.sql", "Creature base stats for level %u has invalid class %u", Level, Class);

        CreatureBaseStats stats;

        for (uint8 i = 0; i < MAX_EXPANSIONS; ++i)
        {
            stats.BaseHealth[i] = fields[2 + i].GetUInt16();

            if (stats.BaseHealth[i] == 0)
            {
                TC_LOG_ERROR("sql.sql", "Creature base stats for class %u, level %u has invalid zero base HP[%u] - set to 1", Class, Level, i);
                stats.BaseHealth[i] = 1;
            }

            stats.BaseDamage[i] = fields[9 + i].GetFloat();
            if (stats.BaseDamage[i] < 0.0f)
            {
                TC_LOG_ERROR("sql.sql", "Creature base stats for class %u, level %u has invalid negative base damage[%u] - set to 0.0", Class, Level, i);
                stats.BaseDamage[i] = 0.0f;
            }
        }

        stats.BaseMana = fields[5].GetUInt16();
        stats.BaseArmor = fields[6].GetUInt16();

        stats.AttackPower = fields[7].GetUInt16();
        stats.RangedAttackPower = fields[8].GetUInt16();

        _creatureBaseStatsStore[MAKE_PAIR16(Level, Class)] = stats;

        ++count;
    }
    while (result->NextRow());

    CreatureTemplateContainer const* ctc = sObjectMgr->GetCreatureTemplateStore();
    for (CreatureTemplateContainer::const_iterator itr = ctc->begin(); itr != ctc->end(); ++itr)
    {
        for (uint16 lvl = itr->second.minlevel; lvl <= itr->second.maxlevel; ++lvl)
        {
            if (_creatureBaseStatsStore.find(MAKE_PAIR16(lvl, itr->second.unit_class)) == _creatureBaseStatsStore.end())
                TC_LOG_ERROR("sql.sql", "Missing base stats for creature class %u level %u", itr->second.unit_class, lvl);
        }
    }

    TC_LOG_INFO("server.loading", ">> Loaded %u creature base stats in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}