/* Copyright (C) 2008 Trinity <http://www.trinitycore.org/>
 *
 * Thanks to the original authors: ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 *
 * This program is free software licensed under GPL version 2
 * Please see the included DOCS/LICENSE.TXT for more information */

#ifndef SCRIPTMGR_H
#define SCRIPTMGR_H

#include "Common.h"
#include "CreatureScript.h"
#include "SpellScript.h"
#include "Platform/CompilerDefs.h"
#include "Database/DBCStructure.h"

#ifndef _TRINITY_SCRIPT_CONFIG
# define _TRINITY_SCRIPT_CONFIG  "worldserver.conf"
#endif _TRINITY_SCRIPT_CONFIG

class Player;
class Creature;
class CreatureAI;
class CreatureAINew;
class SpellScriptWrapper;
class InstanceData;
class Quest;
class Item;
class GameObject;
class SpellCastTargets;
class Map;
class Unit;
class WorldObject;

#define MAX_SCRIPTS         5000                            //72 bytes each (approx 351kb)
#define VISIBLE_RANGE       (166.0f)                        //MAX visible range (size of grid)
#define DEFAULT_TEXT        "<Trinity Script Text Entry Missing!>"

struct Script
{
    Script() :
        pGossipHello(NULL), pQuestAccept(NULL), pGossipSelect(NULL), pGossipSelectWithCode(NULL),
        pQuestSelect(NULL), pQuestComplete(NULL), pNPCDialogStatus(NULL), pGODialogStatus(NULL), pChooseReward(NULL),
        pItemHello(NULL), pGOHello(NULL), pAreaTrigger(NULL), pItemQuestAccept(NULL), pGOQuestAccept(NULL),
        pGOChooseReward(NULL), pReceiveEmote(NULL), pItemUse(NULL), GetAI(NULL), GetInstanceData(NULL), pEffectDummyCreature(NULL)
    {}

    std::string Name;

    //Methods to be scripted
    void (*pOnLogin             )(Player*);
    void (*pOnLogout            )(Player*);
    void (*pOnPVPKill           )(Player*, Player*);
    bool (*pGossipHello         )(Player*, Creature*);
    bool (*pQuestAccept         )(Player*, Creature*, Quest const* );
    bool (*pGossipSelect        )(Player*, Creature*, uint32 , uint32 );
    bool (*pGossipSelectWithCode)(Player*, Creature*, uint32 , uint32 , const char* );
    bool (*pGOSelect            )(Player*, GameObject*, uint32 , uint32 );
    bool (*pGOSelectWithCode    )(Player*, GameObject*, uint32 , uint32 , const char* );
    bool (*pQuestSelect         )(Player*, Creature*, Quest const* );
    bool (*pQuestComplete       )(Player*, Creature*, Quest const* );
    uint32 (*pNPCDialogStatus   )(Player*, Creature* );
    uint32 (*pGODialogStatus    )(Player*, GameObject * _GO );
    bool (*pChooseReward        )(Player*, Creature*, Quest const*, uint32 );
    bool (*pItemHello           )(Player*, Item*, Quest const* );
    bool (*pGOHello             )(Player*, GameObject* );
    bool (*pAreaTrigger         )(Player*, AreaTriggerEntry const* );
    bool (*pItemQuestAccept     )(Player*, Item *, Quest const* );
    bool (*pGOQuestAccept       )(Player*, GameObject*, Quest const* );
    bool (*pGOChooseReward      )(Player*, GameObject*, Quest const*, uint32 );
    bool (*pReceiveEmote        )(Player*, Creature*, uint32 );
    bool (*pItemUse             )(Player*, Item*, SpellCastTargets const& );
    bool (*pEffectDummyCreature )(Unit*, uint32, uint32, Creature*);

    CreatureAI* (*GetAI)(Creature*);
    InstanceData* (*GetInstanceData)(Map*);

    void RegisterSelf();
};

class ScriptMgr
{
    public:
        ScriptMgr();
        ~ScriptMgr();
        
        void ScriptsInit(char const* cfg_file);
        void LoadDatabase();
        char const* ScriptsVersion();    

        std::string GetConfigValueStr(char const* option);
        int32 GetConfigValueInt32(char const* option);
        float GetConfigValueFloat(char const* option);

    //event handlers
        void OnLogin(Player *pPlayer);
        void OnLogout(Player *pPlayer);
        void OnPVPKill(Player *killer, Player *killed);
        bool OnSpellCast (Unit *pUnitTarget, Item *pItemTarget, GameObject *pGoTarget, uint32 i, SpellEntry const *spell);
        uint32 OnGetXP(Player *pPlayer, uint32 amount);
        uint32 OnGetMoney(Player *pPlayer, int32 amount);
        bool OnPlayerChat(Player *pPlayer, const char *text);
        void OnServerStartup();
        void OnServerShutdown();
        void OnAreaChange(Player *pPlayer, AreaTableEntry const *pArea);
        bool OnItemClick (Player *pPlayer, Item *pItem);
        bool OnItemOpen (Player *pPlayer, Item *pItem);
        bool OnGoClick (Player *pPlayer, GameObject *pGameObject);
        void OnCreatureKill (Player *pPlayer, Creature *pCreature);
        bool GossipHello (Player * pPlayer, Creature* pCreature);
        bool GossipSelect(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction);
        bool GossipSelectWithCode(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction, const char* sCode);
        bool GOSelect(Player* pPlayer, GameObject* pGO, uint32 uiSender, uint32 uiAction);
        bool GOSelectWithCode(Player* pPlayer, GameObject* pGO, uint32 uiSender, uint32 uiAction, const char* sCode);
        bool QuestAccept(Player* pPlayer, Creature* pCreature, Quest const* pQuest);
        bool QuestSelect(Player* pPlayer, Creature* pCreature, Quest const* pQuest);
        bool QuestComplete(Player* pPlayer, Creature* pCreature, Quest const* pQuest);
        bool ChooseReward(Player* pPlayer, Creature* pCreature, Quest const* pQuest, uint32 opt);
        uint32 NPCDialogStatus(Player* pPlayer, Creature* pCreature);
        uint32 GODialogStatus(Player* pPlayer, GameObject* pGO);
        bool ItemHello(Player* pPlayer, Item* pItem, Quest const* pQuest);
        bool ItemQuestAccept(Player* pPlayer, Item* pItem, Quest const* pQuest);
        bool GOHello(Player* pPlayer, GameObject* pGO);
        bool GOQuestAccept(Player* pPlayer, GameObject* pGO, Quest const* pQuest);
        bool GOChooseReward(Player* pPlayer, GameObject* pGO, Quest const* pQuest, uint32 opt);
        bool AreaTrigger(Player* pPlayer,AreaTriggerEntry const* atEntry);
        CreatureAI* GetAI(Creature* pCreature);
        bool ItemUse(Player* pPlayer, Item* pItem, SpellCastTargets const& targets);
        //bool ItemExpire(Player* pPlayer, ItemPrototype const * pItemProto);
        //bool EffectDummyCreature(Unit *caster, uint32 spellId, uint32 effIndex, Creature *crTarget);
        //bool EffectDummyGameObj(Unit *caster, uint32 spellId, uint32 effIndex, GameObject *gameObjTarget);
        //bool EffectDummyItem(Unit *caster, uint32 spellId, uint32 effIndex, Item *itemTarget);
        InstanceData* CreateInstanceData(Map *map);
        bool ReceiveEmote(Player *player, Creature *_Creature, uint32 emote);
        bool EffectDummyCreature(Unit *caster, uint32 spellId, uint32 effIndex, Creature *crTarget);
        
        CreatureAINew* getAINew(Creature* creature);
        SpellScript* getSpellScript(Spell* spell);
        
        void addScript(CreatureScript* cscript) { m_creatureScripts[cscript->getName()] = cscript; }
        void addScript(SpellScriptWrapper* sscript) { m_spellScripts[sscript->getName()] = sscript; }
        
    private:
        typedef std::map<std::string, CreatureScript*> CreatureScriptMap;
        CreatureScriptMap m_creatureScripts;
        typedef std::map<std::string, SpellScriptWrapper*> SpellScriptMap;
        SpellScriptMap m_spellScripts;
};

//Generic scripting text function
void DoScriptText(int32 textEntry, WorldObject* pSource, Unit* target = NULL);

Creature* SelectCreatureInGrid(Creature* origin, uint32 entry, float range);

#if COMPILER == COMPILER_GNU
#define FUNC_PTR(name,callconvention,returntype,parameters)    typedef returntype(*name)parameters __attribute__ ((callconvention));
#else
#define FUNC_PTR(name, callconvention, returntype, parameters)    typedef returntype(callconvention *name)parameters;
#endif
/*
#ifdef WIN32
  #define TRINITY_DLL_EXPORT extern "C" __declspec(dllexport)
#elif defined( __GNUC__ )
#define TRINITY_DLL_EXPORT extern "C"
#else
#define TRINITY_DLL_EXPORT extern "C" export
#endif
*/
#define sScriptMgr Trinity::Singleton<ScriptMgr>::Instance()
#endif

