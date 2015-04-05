/* Copyright (C) 2006 - 2008 TrinityScript <https://scriptdev2.svn.sourceforge.net/>
 * This program is free software licensed under GPL version 2
 * Please see the included DOCS/LICENSE.TXT for more information */

#include "ConfigMgr.h"
#include "DatabaseEnv.h"
#include "DBCStores.h"
#include "ObjectMgr.h"
#include "Policies/SingletonImp.h"
#include "Spell.h"
#include "ConfigMgr.h"
#include "ScriptMgr.h"
#include "GridNotifiers.h"
#include "Player.h"
#include "GossipDef.h"
#include "ScriptLoader.h"

class CreatureScript;

#define _FULLVERSION "TrinityScript"

#ifndef _TRINITY_SCRIPT_CONFIG
# define _TRINITY_SCRIPT_CONFIG  "worldserver.conf"
#endif // _TRINITY_SCRIPT_CONFIG

//*** Global data ***
int num_sc_scripts;
Script *m_scripts[MAX_SCRIPTS];

// String text additional data, used in TextMap
struct StringTextData
{
    uint32 SoundId;
    uint8  Type;
    uint32 Language;
    uint32 Emote;
};

#define TEXT_SOURCE_RANGE   -1000000                        //the amount of entries each text source has available

// Text Maps
std::unordered_map<int32, StringTextData> TextMap;

//*** End Global data ***

void FillSpellSummary();
void LoadOverridenSQLData();
void LoadOverridenDBCData();

// -------------------
void ScriptMgr::LoadDatabase()
{
    //***Preform all DB queries here***
    QueryResult result;

    // Drop Existing Text Map, only done once and we are ready to add data from multiple sources.
    TextMap.clear();

    // Load Script Text
    TC_LOG_INFO("server.loading","TSCR: Loading Script Texts...");
    LoadTrinityStrings(WorldDatabase,"script_texts",TEXT_SOURCE_RANGE,1+(TEXT_SOURCE_RANGE*2));

    // Gather Additional data from Script Texts
    result = WorldDatabase.PQuery("SELECT entry, sound, type, language, emote FROM script_texts");

    TC_LOG_INFO("FIXME","TSCR: Loading Script Texts additional data...");
    if (result)
    {
        uint32 count = 0;

        do
        {
            Field* fields = result->Fetch();
            StringTextData temp;

            int32 i             = fields[0].GetInt32();
            temp.SoundId        = fields[1].GetInt32();
            temp.Type           = fields[2].GetInt32();
            temp.Language       = fields[3].GetInt32();
            temp.Emote          = fields[4].GetInt32();

            if (i >= 0)
            {
                error_db_log("TSCR: Entry %i in table `script_texts` is not a negative value.",i);
                continue;
            }

            if (i > TEXT_SOURCE_RANGE || i <= TEXT_SOURCE_RANGE*2)
            {
                error_db_log("TSCR: Entry %i in table `script_texts` is out of accepted entry range for table.",i);
                continue;
            }

            if (temp.SoundId)
            {
                if (!GetSoundEntriesStore()->LookupEntry(temp.SoundId))
                    error_db_log("TSCR: Entry %i in table `script_texts` has soundId %u but sound does not exist.",i,temp.SoundId);
            }

            if (!GetLanguageDescByID(temp.Language))
                error_db_log("TSCR: Entry %i in table `script_texts` using Language %u but Language does not exist.",i,temp.Language);

            if (temp.Type >= CHAT_TYPE_END)
                error_db_log("TSCR: Entry %i in table `script_texts` has Type %u but this Chat Type does not exist.",i,temp.Type);

            TextMap[i] = temp;
            ++count;
        } while (result->NextRow());

        TC_LOG_INFO("FIXME","\n>> TSCR: Loaded %u additional Script Texts data.", count);
    }else
    {
        TC_LOG_INFO("FIXME","\n>> Loaded 0 additional Script Texts data. DB table `script_texts` is empty.");
    }
}

struct TSpellSummary {
    uint8 Targets;                                          // set of enum SelectTarget
    uint8 Effects;                                          // set of enum SelectEffect
}extern *SpellSummary;

void ScriptsFree()
{
    // Free Spell Summary
    delete []SpellSummary;

    // Free resources before library unload
    for(int i=0;i<num_sc_scripts;i++)
        delete m_scripts[i];

    num_sc_scripts = 0;
}

ScriptMgr::ScriptMgr()
{
    
}

ScriptMgr::~ScriptMgr()
{
    
}

void ScriptMgr::ScriptsInit(char const* cfg_file)
{
    bool CanLoadDB = true;
    
    //Trinity Script startup
    TC_LOG_INFO("server.loading"," _____     _       _ _         ____            _       _");
    TC_LOG_INFO("server.loading","|_   _| __(_)_ __ (_) |_ _   _/ ___|  ___ _ __(_)_ __ | |_ ");
    TC_LOG_INFO("server.loading","  | || '__| | '_ \\| | __| | | \\___ \\ / __| \'__| | \'_ \\| __|");
    TC_LOG_INFO("server.loading","  | || |  | | | | | | |_| |_| |___) | (__| |  | | |_) | |_ ");
    TC_LOG_INFO("server.loading","  |_||_|  |_|_| |_|_|\\__|\\__, |____/ \\___|_|  |_| .__/ \\__|");
    TC_LOG_INFO("server.loading","                          |___/                  |_|        ");
    TC_LOG_INFO("server.loading","Trinity Script initializing %s\n", _FULLVERSION);

    TC_LOG_INFO("server.loading"," ");

    //Load database (must be called after TScriptConfig.SetSource). In case it failed, no need to even try load.
    if (CanLoadDB)
        LoadDatabase();

    TC_LOG_INFO("server.loading","TSCR: Loading C++ scripts\n");

    for(int i=0;i<MAX_SCRIPTS;i++)
        m_scripts[i]=NULL;

    FillSpellSummary();

    AddScripts();

    // -------------------

    TC_LOG_INFO("FIXME",">> Loaded %i C++ Scripts.", num_sc_scripts);

    TC_LOG_INFO("FIXME",">> Load Overriden SQL Data.");
    LoadOverridenSQLData();
    TC_LOG_INFO("FIXME",">> Load Overriden DBC Data.");
    LoadOverridenDBCData();
}

//*********************************
//*** Functions used globally ***

void DoScriptText(int32 textEntry, WorldObject* pSource, Unit* target)
{
    if (!pSource)
    {
        error_log("TSCR: DoScriptText entry %i, invalid Source pointer.",textEntry);
        return;
    }

    if (textEntry >= 0)
    {
        error_log("TSCR: DoScriptText with source entry %u (TypeId=%u, guid=%u) attempts to process text entry %i, but text entry must be negative.",pSource->GetEntry(),pSource->GetTypeId(),pSource->GetGUIDLow(),textEntry);
        return;
    }

    std::unordered_map<int32, StringTextData>::iterator i = TextMap.find(textEntry);

    if (i == TextMap.end())
    {
        error_log("TSCR: DoScriptText with source entry %u (TypeId=%u, guid=%u) could not find text entry %i.",pSource->GetEntry(),pSource->GetTypeId(),pSource->GetGUIDLow(),textEntry);
        return;
    }

    if((*i).second.SoundId)
    {
        if( GetSoundEntriesStore()->LookupEntry((*i).second.SoundId) )
        {
            pSource->SendPlaySound((*i).second.SoundId, false);
        }
        else
            error_log("TSCR: DoScriptText entry %i tried to process invalid sound id %u.",textEntry,(*i).second.SoundId);
    }

    if((*i).second.Emote)
    {
        if (pSource->GetTypeId() == TYPEID_UNIT || pSource->GetTypeId() == TYPEID_PLAYER)
        {
            ((Unit*)pSource)->HandleEmoteCommand((*i).second.Emote);
        }
        else
            error_log("TSCR: DoScriptText entry %i tried to process emote for invalid TypeId (%u).",textEntry,pSource->GetTypeId());
    }

    switch((*i).second.Type)
    {
        case CHAT_TYPE_SAY:
            pSource->MonsterSay(textEntry, (*i).second.Language, target ? target->GetGUID() : 0);
            break;
        case CHAT_TYPE_YELL:
            pSource->MonsterYell(textEntry, (*i).second.Language, target ? target->GetGUID() : 0);
            break;
        case CHAT_TYPE_TEXT_EMOTE:
            pSource->MonsterTextEmote(textEntry, target ? target->GetGUID() : 0);
            break;
        case CHAT_TYPE_BOSS_EMOTE:
            pSource->MonsterTextEmote(textEntry, target ? target->GetGUID() : 0, true);
            break;
        case CHAT_TYPE_WHISPER:
            {
                if (target && target->GetTypeId() == TYPEID_PLAYER)
                    pSource->MonsterWhisper(textEntry, target->GetGUID());
                else error_log("TSCR: DoScriptText entry %i cannot whisper without target unit (TYPEID_PLAYER).", textEntry);
            }break;
        case CHAT_TYPE_BOSS_WHISPER:
            {
                if (target && target->GetTypeId() == TYPEID_PLAYER)
                    pSource->MonsterWhisper(textEntry, target->GetGUID(), true);
                else error_log("TSCR: DoScriptText entry %i cannot whisper without target unit (TYPEID_PLAYER).", textEntry);
            }break;
    }
}

Creature* SelectCreatureInGrid(Creature* origin, uint32 entry, float range)
{
    Creature* pCreature = NULL;

    CellPair pair(Trinity::ComputeCellPair(origin->GetPositionX(), origin->GetPositionY()));
    Cell cell(pair);
    cell.data.Part.reserved = ALL_DISTRICT;
    cell.SetNoCreate();

    Trinity::NearestCreatureEntryWithLiveStateInObjectRangeCheck creature_check(*origin, entry, true, range);
    Trinity::CreatureLastSearcher<Trinity::NearestCreatureEntryWithLiveStateInObjectRangeCheck> searcher(pCreature, creature_check);

    TypeContainerVisitor<Trinity::CreatureLastSearcher<Trinity::NearestCreatureEntryWithLiveStateInObjectRangeCheck>, GridTypeMapContainer> creature_searcher(searcher);

    cell.Visit(pair, creature_searcher, *(origin->GetMap()));

    return pCreature;
}

//*********************************
//*** Functions used internally ***

void Script::RegisterSelf()
{
    int id = GetScriptId(Name.c_str());
    if(id)
    {
        m_scripts[id] = this;
        ++num_sc_scripts;
    }
    else
        TC_LOG_ERROR("FIXME","TrinityScript: RegisterSelf, but script named %s does not have ScriptName assigned in database.",(this)->Name.c_str());
}

//********************************
//*** Functions to be Exported ***


void ScriptMgr::OnAddPassenger(Transport* transport, Player* player)
{
    ASSERT(transport);
    ASSERT(player);

   /* GET_SCRIPT(TransportScript, transport->GetScriptId(), tmpscript);
    tmpscript->OnAddPassenger(transport, player); */
}

void ScriptMgr::OnAddCreaturePassenger(Transport* transport, Creature* creature)
{
    ASSERT(transport);
    ASSERT(creature);

    /*GET_SCRIPT(TransportScript, transport->GetScriptId(), tmpscript);
    tmpscript->OnAddCreaturePassenger(transport, creature);*/
}

void ScriptMgr::OnRemovePassenger(Transport* transport, Player* player)
{
    ASSERT(transport);
    ASSERT(player);

    /*GET_SCRIPT(TransportScript, transport->GetScriptId(), tmpscript);
    tmpscript->OnRemovePassenger(transport, player);*/
}

void ScriptMgr::OnTransportUpdate(Transport* transport, uint32 diff)
{
    ASSERT(transport);

    /*GET_SCRIPT(TransportScript, transport->GetScriptId(), tmpscript);
    tmpscript->OnUpdate(transport, diff);*/
}

void ScriptMgr::OnRelocate(Transport* transport, uint32 waypointId, uint32 mapId, float x, float y, float z)
{
/*    GET_SCRIPT(TransportScript, transport->GetScriptId(), tmpscript);
    tmpscript->OnRelocate(transport, waypointId, mapId, x, y, z); */
}

void ScriptMgr::OnPlayerLogin(Player *pPlayer, bool /* firstLogin */)
{
    Script *tmpscript = m_scripts[GetScriptId("scripted_on_events")];
    if (!tmpscript || !tmpscript->pOnLogin) 
        return;

    tmpscript->pOnLogin(pPlayer);
}

void ScriptMgr::OnSocketOpen(std::shared_ptr<WorldSocket> socket)
{
    ASSERT(socket);

    //FOREACH_SCRIPT(ServerScript)->OnSocketOpen(socket);
}

void ScriptMgr::OnSocketClose(std::shared_ptr<WorldSocket> socket)
{
    ASSERT(socket);

    //FOREACH_SCRIPT(ServerScript)->OnSocketClose(socket);
}

void ScriptMgr::OnPlayerSave(Player* player)
{
   // FOREACH_SCRIPT(PlayerScript)->OnSave(player);
}

void ScriptMgr::OnPasswordChange(uint32 accountId)
{
   // FOREACH_SCRIPT(AccountScript)->OnPasswordChange(accountId);
}

void ScriptMgr::OnPacketReceive(WorldSession* session, WorldPacket const& packet)
{
   /* if (SCR_REG_LST(ServerScript).empty())
        return;

    WorldPacket copy(packet);
    FOREACH_SCRIPT(ServerScript)->OnPacketReceive(session, copy);*/
}

void ScriptMgr::OnPacketSend(WorldSession* session, WorldPacket const& packet)
{
    ASSERT(session);

   /* if (SCR_REG_LST(ServerScript).empty())
        return;

    WorldPacket copy(packet);
    FOREACH_SCRIPT(ServerScript)->OnPacketSend(session, copy);*/
}


void ScriptMgr::OnNetworkStart()
{
 //   FOREACH_SCRIPT(ServerScript)->OnNetworkStart();
}

void ScriptMgr::OnNetworkStop()
{
  //  FOREACH_SCRIPT(ServerScript)->OnNetworkStop();
}

void ScriptMgr::OnEmailChange(uint32 accountId)
{
   // FOREACH_SCRIPT(AccountScript)->OnEmailChange(accountId);
}

void ScriptMgr::OnFailedEmailChange(uint32 accountId)
{
    //FOREACH_SCRIPT(AccountScript)->OnFailedEmailChange(accountId);
}

void ScriptMgr::OnFailedPasswordChange(uint32 accountId)
{
    //FOREACH_SCRIPT(AccountScript)->OnFailedPasswordChange(accountId);
}

void ScriptMgr::OnAccountLogin(uint32 accountId)
{
    //FOREACH_SCRIPT(AccountScript)->OnAccountLogin(accountId);
}

void ScriptMgr::OnFailedAccountLogin(uint32 accountId)
{
    //FOREACH_SCRIPT(AccountScript)->OnFailedAccountLogin(accountId);
}

void ScriptMgr::OnPlayerLogout(Player *pPlayer)
{
    Script *tmpscript = m_scripts[GetScriptId("scripted_on_events")];
    if (!tmpscript || !tmpscript->pOnLogout) return;
    tmpscript->pOnLogout(pPlayer);
}


void ScriptMgr::OnPVPKill(Player *killer, Player *killed)
{
    Script *tmpscript = m_scripts[GetScriptId("scripted_on_events")];
    if (!tmpscript || !tmpscript->pOnPVPKill) return;
    tmpscript->pOnPVPKill(killer, killed);
}

void ScriptMgr::OnUnknownPacketReceive(WorldSession* session, WorldPacket const& packet)
{
    ASSERT(session);

    /*if (SCR_REG_LST(ServerScript).empty())
        return;

    WorldPacket copy(packet);
    FOREACH_SCRIPT(ServerScript)->OnUnknownPacketReceive(session, copy);*/
}

void ScriptMgr::OnPlayerCreate(Player* player)
{
    //FOREACH_SCRIPT(PlayerScript)->OnCreate(player);
}

char const* ScriptMgr::ScriptsVersion()
{
    return "Default Trinity scripting library";
}

void ScriptMgr::Unload()
{

}

bool ScriptMgr::GossipHello ( Player * player, Creature *_Creature )
{
    player->PlayerTalkClass->ClearMenus();
    Script *tmpscript = m_scripts[_Creature->GetScriptId()];
    if(!tmpscript || !tmpscript->pGossipHello)
        return false;

    player->PlayerTalkClass->ClearMenus();
    return tmpscript->pGossipHello(player,_Creature);
    
    CreatureScript* tmpscript2 = m_creatureScripts[_Creature->getScriptName()];
    if(tmpscript2 && tmpscript2->pGossipHello) 
        return tmpscript2->pGossipHello(player,_Creature);
}


bool ScriptMgr::GossipSelect( Player *player, Creature *_Creature, uint32 sender, uint32 action )
{
    Script *tmpscript = m_scripts[_Creature->GetScriptId()];
    if(!tmpscript || !tmpscript->pGossipSelect)
        return false;
    
    player->PlayerTalkClass->ClearMenus();
    return tmpscript->pGossipSelect(player,_Creature,sender,action);
     
    CreatureScript* tmpscript2 = m_creatureScripts[_Creature->getScriptName()];
    if(tmpscript2 && tmpscript2->pGossipSelect) 
        return tmpscript2->pGossipSelect(player,_Creature,sender,action);
 
    return false;
}


bool ScriptMgr::GossipSelectWithCode( Player *player, Creature *_Creature, uint32 sender, uint32 action, const char* sCode )
{
    Script *tmpscript = m_scripts[_Creature->GetScriptId()];
     if(!tmpscript || !tmpscript->pGossipSelectWithCode)
         return false;
     
    player->PlayerTalkClass->ClearMenus();
    return tmpscript->pGossipSelectWithCode(player,_Creature,sender,action,sCode);
     
    CreatureScript* tmpscript2 = m_creatureScripts[_Creature->GetScriptName()];
    if(tmpscript2 && tmpscript2->pGossipSelectWithCode) 
        return tmpscript2->pGossipSelectWithCode(player,_Creature,sender,action,sCode);

    return false;
}


bool ScriptMgr::GOSelect( Player *player, GameObject *_GO, uint32 sender, uint32 action )
{
    if(!_GO)
    return false;

    Script *tmpscript = m_scripts[_GO->GetGOInfo()->ScriptId];
    if(!tmpscript || !tmpscript->pGOSelect) return false;

    player->PlayerTalkClass->ClearMenus();
    return tmpscript->pGOSelect(player,_GO,sender,action);
}


bool ScriptMgr::GOSelectWithCode( Player *player, GameObject *_GO, uint32 sender, uint32 action, const char* sCode )
{
    if(!_GO)
    return false;

    Script *tmpscript = m_scripts[_GO->GetGOInfo()->ScriptId];
    if(!tmpscript || !tmpscript->pGOSelectWithCode) return false;

    player->PlayerTalkClass->ClearMenus();
    return tmpscript->pGOSelectWithCode(player,_GO,sender,action,sCode);
}


bool ScriptMgr::QuestAccept( Player *player, Creature *_Creature, Quest const *_Quest )
{
    Script *tmpscript = m_scripts[_Creature->GetScriptId()];
    if (!tmpscript || !tmpscript->pQuestAccept) return false;

    player->PlayerTalkClass->ClearMenus();
    return tmpscript->pQuestAccept(player,_Creature,_Quest);
}


bool ScriptMgr::QuestSelect( Player *player, Creature *_Creature, Quest const *_Quest )
{
    Script *tmpscript = m_scripts[_Creature->GetScriptId()];
    if (!tmpscript || !tmpscript->pQuestSelect) return false;

    player->PlayerTalkClass->ClearMenus();
    return tmpscript->pQuestSelect(player,_Creature,_Quest);
}


bool ScriptMgr::QuestComplete( Player *player, Creature *_Creature, Quest const *_Quest )
{
    Script *tmpscript = m_scripts[_Creature->GetScriptId()];
    if (!tmpscript || !tmpscript->pQuestComplete) return false;

    player->PlayerTalkClass->ClearMenus();
    return tmpscript->pQuestComplete(player,_Creature,_Quest);
}


bool ScriptMgr::ChooseReward( Player *player, Creature *_Creature, Quest const *_Quest, uint32 opt )
{
    Script *tmpscript = m_scripts[_Creature->GetScriptId()];
    if (!tmpscript || !tmpscript->pChooseReward) return false;

    player->PlayerTalkClass->ClearMenus();
    return tmpscript->pChooseReward(player,_Creature,_Quest,opt);
}


uint32 ScriptMgr::NPCDialogStatus( Player *player, Creature *_Creature )
{
    Script *tmpscript = m_scripts[_Creature->GetScriptId()];
    if (!tmpscript || !tmpscript->pNPCDialogStatus) return 100;

    player->PlayerTalkClass->ClearMenus();
    return tmpscript->pNPCDialogStatus(player,_Creature);
}


uint32 ScriptMgr::GODialogStatus( Player *player, GameObject *_GO )
{
    Script *tmpscript = m_scripts[_GO->GetGOInfo()->ScriptId];
    if (!tmpscript || !tmpscript->pGODialogStatus) return 100;

    player->PlayerTalkClass->ClearMenus();
    return tmpscript->pGODialogStatus(player,_GO);
}


bool ScriptMgr::ItemHello( Player *player, Item *_Item, Quest const *_Quest )
{
    Script *tmpscript = m_scripts[_Item->GetProto()->ScriptId];
    if (!tmpscript || !tmpscript->pItemHello) return false;

    player->PlayerTalkClass->ClearMenus();
    return tmpscript->pItemHello(player,_Item,_Quest);
}


bool ScriptMgr::ItemQuestAccept( Player *player, Item *_Item, Quest const *_Quest )
{
    Script *tmpscript = m_scripts[_Item->GetProto()->ScriptId];
    if (!tmpscript || !tmpscript->pItemQuestAccept) return false;

    player->PlayerTalkClass->ClearMenus();
    return tmpscript->pItemQuestAccept(player,_Item,_Quest);
}


bool ScriptMgr::GOHello( Player *player, GameObject *_GO )
{
    Script *tmpscript = m_scripts[_GO->GetGOInfo()->ScriptId];
    if (!tmpscript || !tmpscript->pGOHello) return false;

    player->PlayerTalkClass->ClearMenus();
    return tmpscript->pGOHello(player,_GO);
}


bool ScriptMgr::GOQuestAccept( Player *player, GameObject *_GO, Quest const *_Quest )
{
    Script *tmpscript = m_scripts[_GO->GetGOInfo()->ScriptId];
    if (!tmpscript || !tmpscript->pGOQuestAccept) return false;

    player->PlayerTalkClass->ClearMenus();
    return tmpscript->pGOQuestAccept(player,_GO,_Quest);
}


bool ScriptMgr::GOChooseReward( Player *player, GameObject *_GO, Quest const *_Quest, uint32 opt )
{
    Script *tmpscript = m_scripts[_GO->GetGOInfo()->ScriptId];
    if (!tmpscript || !tmpscript->pGOChooseReward) return false;

    player->PlayerTalkClass->ClearMenus();
    return tmpscript->pGOChooseReward(player,_GO,_Quest,opt);
}


bool ScriptMgr::AreaTrigger( Player *player, AreaTriggerEntry const* atEntry)
{
    Script *tmpscript = m_scripts[GetAreaTriggerScriptId(atEntry->id)];
    if (!tmpscript || !tmpscript->pAreaTrigger) return false;

    return tmpscript->pAreaTrigger(player, atEntry);
}

CreatureAI* ScriptMgr::GetAI(Creature *_Creature)
{
    Script *tmpscript = m_scripts[_Creature->GetScriptId()];
    if (!tmpscript || !tmpscript->GetAI) return NULL;
    
    return tmpscript->GetAI(_Creature);
}

CreatureAINew* ScriptMgr::getAINew(Creature* creature)
{
    CreatureScriptMap::const_iterator iter = m_creatureScripts.find(creature->getScriptName().c_str());
    if (iter == m_creatureScripts.end())
        return NULL;
    
    return iter->second->getAI(creature);
}

SpellScript* ScriptMgr::getSpellScript(Spell* spell)
{
    if (!spell)
        return NULL;
    
    SpellScriptMap::const_iterator iter = m_spellScripts.find(sObjectMgr->getSpellScriptName(spell->m_spellInfo->Id));
    if (iter == m_spellScripts.end())
        return NULL;
        
    return iter->second->getScript(spell);
}

bool ScriptMgr::ItemUse( Player *player, Item* _Item, SpellCastTargets const& targets)
{
    Script *tmpscript = m_scripts[_Item->GetProto()->ScriptId];
    if (!tmpscript || !tmpscript->pItemUse) return false;

    return tmpscript->pItemUse(player,_Item,targets);
}


bool ScriptMgr::ReceiveEmote( Player *player, Creature *_Creature, uint32 emote )
{
    Script *tmpscript = m_scripts[_Creature->GetScriptId()];
    if (!tmpscript || !tmpscript->pReceiveEmote) return false;

    return tmpscript->pReceiveEmote(player, _Creature, emote);
}

bool ScriptMgr::EffectDummyCreature(Unit *caster, uint32 spellId, uint32 effIndex, Creature *crTarget)
{
    Script *tmpscript = m_scripts[crTarget->GetScriptId()];

    if (!tmpscript || !tmpscript->pEffectDummyCreature) return false;

    return tmpscript->pEffectDummyCreature(caster, spellId, effIndex, crTarget);
}

InstanceData* ScriptMgr::CreateInstanceData(Map *map)
{
    if (!map->IsDungeon()) return NULL;

    Script *tmpscript = m_scripts[((InstanceMap*)map)->GetScriptId()];
    if (!tmpscript || !tmpscript->GetInstanceData) return NULL;

    return tmpscript->GetInstanceData(map);
}
