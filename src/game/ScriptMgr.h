/* Copyright (C) 2008 Trinity <http://www.trinitycore.org/>
 *
 * Thanks to the original authors: ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 *
 * This program is free software licensed under GPL version 2
 * Please see the included DOCS/LICENSE.TXT for more information */

#ifndef SCRIPTMGR_H
#define SCRIPTMGR_H

#include "CreatureScript.h"

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
class InstanceScript;
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
class SpellInfo;
class Spell;
struct AreaTriggerEntry;
struct AreaTableEntry;
enum ShutdownExitCode : int;
enum ShutdownMask : int;

#define MAX_SCRIPTS         5000                            //72 bytes each (approx 351kb)
#define VISIBLE_RANGE       (166.0f)                        //MAX visible range (size of grid)
#define DEFAULT_TEXT        "<Trinity Script Text Entry Missing!>"

struct Script
{
    Script()
    {}

    std::string Name;

    //Methods to be scripted
    void (*pOnLogin             )(Player*)                                                = nullptr;
    void (*pOnLogout            )(Player*)                                                = nullptr;
    void (*pOnPVPKill           )(Player*, Player*)                                       = nullptr;
    bool (*pOnGossipHello       )(Player*, Creature*)                                     = nullptr;
    bool (*pOnQuestAccept       )(Player*, Creature*, Quest const* )                      = nullptr;
    bool (*pOnGossipSelect      )(Player*, Creature*, uint32 , uint32 )                   = nullptr;
    bool (*pOnGossipSelectCode  )(Player*, Creature*, uint32 , uint32 , const char* )     = nullptr;
    bool (*pGOOnGossipSelect    )(Player*, GameObject*, uint32 , uint32 )                 = nullptr;
    bool (*pGOOnGossipSelectCode)(Player*, GameObject*, uint32 , uint32 , const char* )   = nullptr;
    bool (*pQuestSelect         )(Player*, Creature*, Quest const* )                      = nullptr;
    bool (*pQuestComplete       )(Player*, Creature*, Quest const* )                      = nullptr;
    uint32 (*pGetDialogStatus   )(Player*, Creature* )                                    = nullptr;
    uint32 (*pGOGetDialogStatus )(Player*, GameObject * _GO )                             = nullptr;
    bool (*pOnQuestReward       )(Player*, Creature*, Quest const*, uint32 )              = nullptr;
    bool (*pItemHello           )(Player*, Item*, Quest const* )                          = nullptr;
    bool (*pGOOnGossipHello     )(Player*, GameObject* )                                  = nullptr;
    bool (*pAreaTrigger         )(Player*, AreaTriggerEntry const* )                      = nullptr;
    bool (*pItemQuestAccept     )(Player*, Item *, Quest const* )                         = nullptr;
    bool (*pGoOnUse             )(Player*, GameObject*)                                   = nullptr;
    bool (*pGOOnQuestAccept     )(Player*, GameObject*, Quest const* )                    = nullptr;
    bool (*pGOOnQuestReward     )(Player*, GameObject*, Quest const*, uint32 )            = nullptr;
    bool (*pReceiveEmote        )(Player*, Creature*, uint32 )                            = nullptr;
    bool (*pItemUse             )(Player*, Item*, SpellCastTargets const& )               = nullptr;
    bool (*pEffectDummyCreature )(Unit*, uint32, uint32, Creature*)                       = nullptr;

    CreatureAI* (*GetAI)(Creature*)                                                       = nullptr;
    InstanceScript* (*GetInstanceData)(Map*)                                              = nullptr;
};

struct TSpellSummary {
    uint8 Targets;                                          // set of enum SelectTarget
    uint8 Effects;                                          // set of enum SelectEffect
};

class ScriptMgr
{
    private:
        ScriptMgr();
        ~ScriptMgr();

        void ClearScripts();
    public:
        static ScriptMgr* instance()
        {
            static ScriptMgr instance;
            return &instance;
        }

        void ScriptsInit(char const* cfg_file);
        void LoadDatabase();
        char const* ScriptsVersion();    

        /* Add given script to m_script list if the script is used in database. DO NOT use script afterwards as this function may delete it.
          Pointer script* will be freed upon ScriptMgr deletion, or immediately if registering failed
        **/
        void RegisterScript(Script*& script);
        void FillSpellSummary();

        TSpellSummary* GetSpellSummary() { return spellSummary; }

    public:

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

    public: /* MapScript */
         void OnMapUpdate(Map* map, uint32 diff);

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
        void OnPlayerMoneyLimit(Player* player, int32& amount);
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
        bool OnSpellCast(Unit *pUnitTarget, Item *pItemTarget, GameObject *pGoTarget, uint32 i, SpellInfo const *spell);
        void OnServerStartup();
        void OnServerShutdown();
        void OnAreaChange(Player *pPlayer, AreaTableEntry const *pArea);
        bool OnItemClick (Player *pPlayer, Item *pItem);
        bool OnItemOpen (Player *pPlayer, Item *pItem);
        bool OnGoClick (Player *pPlayer, GameObject *pGameObject);
        bool OnGossipHello (Player * pPlayer, Creature* pCreature);
        bool OnGossipSelect(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction);
        bool OnGossipSelectCode(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction, const char* sCode);
        bool OnGossipSelect(Player* pPlayer, GameObject* pGO, uint32 uiSender, uint32 uiAction);
        bool OnGossipSelectCode(Player* pPlayer, GameObject* pGO, uint32 uiSender, uint32 uiAction, const char* sCode);
        bool QuestAccept(Player* pPlayer, Creature* pCreature, Quest const* pQuest);
        bool QuestSelect(Player* pPlayer, Creature* pCreature, Quest const* pQuest);
        bool QuestComplete(Player* pPlayer, Creature* pCreature, Quest const* pQuest);
        bool ChooseReward(Player* pPlayer, Creature* pCreature, Quest const* pQuest, uint32 opt);
        uint32 GetDialogStatus(Player* pPlayer, Creature* pCreature);
        uint32 GetDialogStatus(Player* pPlayer, GameObject* pGO);
        bool ItemHello(Player* pPlayer, Item* pItem, Quest const* pQuest);
        bool ItemQuestAccept(Player* pPlayer, Item* pItem, Quest const* pQuest);
        bool OnGossipHello(Player* pPlayer, GameObject* pGO);
        bool OnQuestAccept(Player* pPlayer, GameObject* pGO, Quest const* pQuest);
        bool OnQuestReward(Player* pPlayer, GameObject* pGO, Quest const* pQuest, uint32 opt);
        bool OnUse(Player* pPlayer, GameObject* pGO);
        bool AreaTrigger(Player* pPlayer,AreaTriggerEntry const* atEntry);
        //use scriptId to override script, else this uses script in database
        CreatureAI* GetAI(Creature* pCreature, uint32 scriptId = 0);
        bool ItemUse(Player* pPlayer, Item* pItem, SpellCastTargets const& targets);
        //bool ItemExpire(Player* pPlayer, ItemTemplate const * pItemProto);
        //bool EffectDummyCreature(Unit *caster, uint32 spellId, uint32 effIndex, Creature *crTarget);
        //bool EffectDummyGameObj(Unit *caster, uint32 spellId, uint32 effIndex, GameObject *gameObjTarget);
        //bool EffectDummyItem(Unit *caster, uint32 spellId, uint32 effIndex, Item *itemTarget);
        InstanceScript* CreateInstanceData(Map *map);
        bool ReceiveEmote(Player *player, Creature *_Creature, uint32 emote);
        bool EffectDummyCreature(Unit *caster, uint32 spellId, uint32 effIndex, Creature *crTarget);
        
        
        //script system by thumsoul
        CreatureAINew* getAINew(Creature* creature);
        //script system by thumsoul
        void addScript(CreatureScript* cscript) { m_creatureScripts[cscript->getName()] = cscript; }
        
    private:
        typedef std::map<std::string, CreatureScript*> CreatureScriptMap;
        CreatureScriptMap m_creatureScripts;

        int num_sc_scripts;
        Script* m_scripts[MAX_SCRIPTS] = { nullptr };

        //summary array
        TSpellSummary* spellSummary;
};

//Generic scripting text function
void DoScriptText(int32 textEntry, Unit* pSource, Unit* target = NULL);

#if COMPILER == COMPILER_GNU
#define FUNC_PTR(name,callconvention,returntype,parameters)    typedef returntype(*name)parameters __attribute__ ((callconvention));
#else
#define FUNC_PTR(name, callconvention, returntype, parameters)    typedef returntype(callconvention *name)parameters;
#endif


#define sScriptMgr ScriptMgr::instance()
#endif

