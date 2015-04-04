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
#include "DBCStructure.h"
#include "World.h"

class WorldSocket;
class WorldSession;
class WorldPacket;

#ifndef _TRINITY_SCRIPT_CONFIG
# define _TRINITY_SCRIPT_CONFIG  "worldserver.conf"
#endif // _TRINITY_SCRIPT_CONFIG

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
class Transport;
class Guild;
class Group;

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
    private:
        ScriptMgr();
        ~ScriptMgr();
    public:
        static ScriptMgr* instance()
        {
            static ScriptMgr instance;
            return &instance;
        }

        void ScriptsInit(char const* cfg_file);
        void LoadDatabase();
        char const* ScriptsVersion();    

    public: /* Unloading */

        void Unload();

        std::string GetConfigValueStr(char const* option);
        int32 GetConfigValueInt32(char const* option);
        float GetConfigValueFloat(char const* option);

    public: /* AccountScript */

        void OnAccountLogin(uint32 accountId);
        void OnFailedAccountLogin(uint32 accountId);
        void OnEmailChange(uint32 accountId);
        void OnFailedEmailChange(uint32 accountId);
        void OnPasswordChange(uint32 accountId);
        void OnFailedPasswordChange(uint32 accountId);
       
    public: /* GuildScript */

        void OnGuildAddMember(Guild* guild, Player* player, uint8& plRank);
        void OnGuildRemoveMember(Guild* guild, Player* player, bool isDisbanding, bool isKicked);
        void OnGuildMOTDChanged(Guild* guild, const std::string& newMotd);
        void OnGuildInfoChanged(Guild* guild, const std::string& newInfo);
        void OnGuildCreate(Guild* guild, Player* leader, const std::string& name);
        void OnGuildDisband(Guild* guild);
        void OnGuildMemberWitdrawMoney(Guild* guild, Player* player, uint32 &amount, bool isRepair);
        void OnGuildMemberDepositMoney(Guild* guild, Player* player, uint32 &amount);
        void OnGuildItemMove(Guild* guild, Player* player, Item* pItem, bool isSrcBank, uint8 srcContainer, uint8 srcSlotId,
            bool isDestBank, uint8 destContainer, uint8 destSlotId);
        void OnGuildEvent(Guild* guild, uint8 eventType, uint32 playerGuid1, uint32 playerGuid2, uint8 newRank);
        void OnGuildBankEvent(Guild* guild, uint8 eventType, uint8 tabId, uint32 playerGuid, uint32 itemOrMoney, uint16 itemStackCount, uint8 destTabId);

    public: /* GroupScript */

        void OnGroupAddMember(Group* group, uint64 guid);
        void OnGroupInviteMember(Group* group, uint64 guid);
//        void OnGroupRemoveMember(Group* group, uint64 guid, RemoveMethod method, uint64 kicker, const char* reason);
        void OnGroupChangeLeader(Group* group, uint64 newLeaderGuid, uint64 oldLeaderGuid);
        void OnGroupDisband(Group* group);

    public: /* TransportScript */

        void OnAddPassenger(Transport* transport, Player* player);
        void OnAddCreaturePassenger(Transport* transport, Creature* creature);
        void OnRemovePassenger(Transport* transport, Player* player);
        void OnTransportUpdate(Transport* transport, uint32 diff);
        void OnRelocate(Transport* transport, uint32 waypointId, uint32 mapId, float x, float y, float z);

    public: /* PlayerScript */

        void OnPVPKill(Player* killer, Player* killed);
        void OnCreatureKill(Player* killer, Creature* killed);
        void OnPlayerKilledByCreature(Creature* killer, Player* killed);
        void OnPlayerLevelChanged(Player* player, uint8 oldLevel);
        void OnPlayerFreeTalentPointsChanged(Player* player, uint32 newPoints);
        void OnPlayerTalentsReset(Player* player, bool noCost);
        void OnPlayerMoneyChanged(Player* player, int32& amount);
        void OnGivePlayerXP(Player* player, uint32& amount, Unit* victim);
        void OnPlayerReputationChange(Player* player, uint32 factionID, int32& standing, bool incremental);
        void OnPlayerDuelRequest(Player* target, Player* challenger);
        void OnPlayerDuelStart(Player* player1, Player* player2);
        void OnPlayerDuelEnd(Player* winner, Player* loser, DuelCompleteType type);
        /*void OnPlayerChat(Player* player, uint32 type, uint32 lang, std::string& msg);
        void OnPlayerChat(Player* player, uint32 type, uint32 lang, std::string& msg, Player* receiver);
        void OnPlayerChat(Player* player, uint32 type, uint32 lang, std::string& msg, Group* group);
        void OnPlayerChat(Player* player, uint32 type, uint32 lang, std::string& msg, Guild* guild);
        void OnPlayerChat(Player* player, uint32 type, uint32 lang, std::string& msg, Channel* channel);*/
        void OnPlayerEmote(Player* player, uint32 emote);
        void OnPlayerTextEmote(Player* player, uint32 textEmote, uint32 emoteNum, uint64 guid);
        void OnPlayerSpellCast(Player* player, Spell* spell, bool skipCheck);
        void OnPlayerLogin(Player* player, bool firstLogin);
        void OnPlayerLogout(Player* player);
        void OnPlayerCreate(Player* player);
        void OnPlayerDelete(uint64 guid, uint32 accountId);
        void OnPlayerFailedDelete(uint64 guid, uint32 accountId);
        void OnPlayerSave(Player* player);
//        void OnPlayerBindToInstance(Player* player, Difficulty difficulty, uint32 mapid, bool permanent);
        void OnPlayerUpdateZone(Player* player, uint32 newZone, uint32 newArea);
        bool OnPlayerChat(Player *pPlayer, const char *text); //old, to replace

    public: /* ServerScript */

        void OnNetworkStart();
        void OnNetworkStop();
        void OnSocketOpen(std::shared_ptr<WorldSocket> socket);
        void OnSocketClose(std::shared_ptr<WorldSocket> socket);
        void OnPacketReceive(WorldSession* session, WorldPacket const& packet);
        void OnPacketSend(WorldSession* session, WorldPacket const& packet);
        void OnUnknownPacketReceive(WorldSession* session, WorldPacket const& packet);

    public: /* WorldScript */

        void OnOpenStateChange(bool open) {}
        void OnConfigLoad(bool reload) {}
        void OnMotdChange(std::string& newMotd) {}
        void OnShutdownInitiate(ShutdownExitCode code, ShutdownMask mask) {}
        void OnShutdownCancel() {}
        void OnWorldUpdate(uint32 diff) {}
        void OnStartup() {}
        void OnShutdown() {}

    //event handlers
        bool OnSpellCast (Unit *pUnitTarget, Item *pItemTarget, GameObject *pGoTarget, uint32 i, SpellEntry const *spell);
        void OnServerStartup();
        void OnServerShutdown();
        void OnAreaChange(Player *pPlayer, AreaTableEntry const *pArea);
        bool OnItemClick (Player *pPlayer, Item *pItem);
        bool OnItemOpen (Player *pPlayer, Item *pItem);
        bool OnGoClick (Player *pPlayer, GameObject *pGameObject);
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
        //bool ItemExpire(Player* pPlayer, ItemTemplate const * pItemProto);
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
#define sScriptMgr ScriptMgr::instance()
#endif

