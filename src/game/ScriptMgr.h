/* Copyright (C) 2008 Trinity <http://www.trinitycore.org/>
 *
 * Thanks to the original authors: ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 *
 * This program is free software licensed under GPL version 2
 * Please see the included DOCS/LICENSE.TXT for more information */

#ifndef SCRIPTMGR_H
#define SCRIPTMGR_H

#include "QuestDef.h"
#include "ObjectMgr.h"

class WorldSocket;
class WorldSession;
class WorldPacket;

#ifndef _TRINITY_SCRIPT_CONFIG
# define _TRINITY_SCRIPT_CONFIG  "worldserver.conf"
#endif // _TRINITY_SCRIPT_CONFIG

class Player;
class Creature;
class CreatureAI;
class GameObject;
class GameObjectAI;
class InstanceScript;
class Quest;
class Item;
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
struct OLDScript;
struct SpellSummary;
class ScriptMgr;

#define MAX_SCRIPTS         5000                            //72 bytes each (approx 351kb)
#define VISIBLE_RANGE       (166.0f)                        //MAX visible range (size of grid)
#define DEFAULT_TEXT        "<missing text>"

class ScriptObject
{
    friend class ScriptMgr;

public:

    // Do not override this in scripts; it should be overridden by the various script type classes. It indicates
    // whether or not this script type must be assigned in the database.
    virtual bool IsDatabaseBound() const { return false; }

    const std::string& GetName() const { return _name; }

protected:

    ScriptObject(const char* name)
        : _name(std::string(name))
    {
    }

    virtual ~ScriptObject()
    {
    }

private:

    const std::string _name;
};

template<class TObject> class UpdatableScript
{
protected:

    UpdatableScript()
    {
    }

public:

    virtual void OnUpdate(TObject* /*obj*/, uint32 /*diff*/) { }
};

struct OLDScript
{
public:
    std::string const& GetName() { return Name; }

    OLDScript()
    {}

    std::string Name;

    //Methods to be scripted
    bool(*OnGossipHello)(Player*, Creature*) = nullptr;
    bool(*OnQuestAccept)(Player*, Creature*, Quest const*) = nullptr;
    bool(*OnGossipSelect)(Player*, Creature*, uint32, uint32) = nullptr;
    bool(*OnGossipSelectCode)(Player*, Creature*, uint32, uint32, const char*) = nullptr;
    bool(*OnQuestSelect)(Player*, Creature*, Quest const*) = nullptr;
    bool(*OnQuestComplete)(Player*, Creature*, Quest const*) = nullptr;
    uint32(*pGetDialogStatus)(Player*, Creature*) = nullptr;
    bool(*OnQuestReward)(Player*, Creature*, Quest const*, uint32) = nullptr;
    bool(*OnReceiveEmote)(Player*, Creature*, uint32 emote) = nullptr;
    bool(*OnEffectDummyCreature)(Unit*, uint32, uint32, Creature*) = nullptr;

    CreatureAI* (*GetAI)(Creature*) = nullptr;
};

template<class TMap> class MapScript : public UpdatableScript<TMap>
{
    MapEntry const* _mapEntry;

protected:

    MapScript(uint32 mapId)
        : _mapEntry(sMapStore.LookupEntry(mapId))
    {
        if (!_mapEntry)
            TC_LOG_ERROR("scripts","Invalid MapScript for %u; no such map ID.", mapId);
    }

public:

    // Gets the MapEntry structure associated with this script. Can return NULL.
    MapEntry const* GetEntry() { return _mapEntry; }

    // Called when the map is created.
    virtual void OnCreate(TMap* /*map*/) { }

    // Called just before the map is destroyed.
    virtual void OnDestroy(TMap* /*map*/) { }

    // Called when a grid map is loaded.
    virtual void OnLoadGridMap(TMap* /*map*/, GridMap* /*gmap*/, uint32 /*gx*/, uint32 /*gy*/) { }

    // Called when a grid map is unloaded.
    virtual void OnUnloadGridMap(TMap* /*map*/, GridMap* /*gmap*/, uint32 /*gx*/, uint32 /*gy*/) { }

    // Called when a player enters the map.
    virtual void OnPlayerEnter(TMap* /*map*/, Player* /*player*/) { }

    // Called when a player leaves the map.
    virtual void OnPlayerLeave(TMap* /*map*/, Player* /*player*/) { }

    // Called on every map update tick.
    virtual void OnUpdate(TMap* /*map*/, uint32 /*diff*/) { }
};

class WorldMapScript : public ScriptObject, public MapScript<Map>
{
protected:

    WorldMapScript(const char* name, uint32 mapId);
};

class InstanceMapScript : public ScriptObject, public MapScript<InstanceMap>
{
protected:

    InstanceMapScript(const char* name, uint32 mapId);

public:

    bool IsDatabaseBound() const { return true; }

    // Gets an InstanceScript object for this instance.
    virtual InstanceScript* GetInstanceScript(InstanceMap* /*map*/) const { return NULL; }
};

class BattlegroundMapScript : public ScriptObject, public MapScript<BattlegroundMap>
{
protected:

    BattlegroundMapScript(const char* name, uint32 mapId);
};

class ItemScript : public ScriptObject
{
protected:

    ItemScript(const char* name);

public:

    bool IsDatabaseBound() const { return true; }

    // Called when a player accepts a quest from the item.
    virtual bool OnQuestAccept(Player* /*player*/, Item* /*item*/, Quest const* /*quest*/) { return false; }

    // Called when a player uses the item.
    virtual bool OnUse(Player* /*player*/, Item* /*item*/, SpellCastTargets const& /*targets*/) { return false; }

    // Called when the item expires (is destroyed).
    virtual bool OnExpire(Player* /*player*/, ItemTemplate const* /*proto*/) { return false; }
};

class CreatureScript : public ScriptObject, public UpdatableScript<Creature>
{
    //make ScriptMgr friend so that we can create CreatureScript's from OLDScript from there (to be removed when OLDScript is discarded)
    friend class ScriptMgr;

protected:

    CreatureScript(const char* name);
    ~CreatureScript();

public:

    bool IsDatabaseBound() const { return true; }

    // Called when a player opens a gossip dialog with the creature.
    virtual bool OnGossipHello(Player* player, Creature* creature) // { return false; }
    {
        if (baseScript && baseScript->OnGossipHello)
            return baseScript->OnGossipHello(player, creature);
        return false;
    }

    // Called when a player selects a gossip item in the creature's gossip menu.
    virtual bool OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action) // { return false; }
    {
        if (baseScript && baseScript->OnGossipSelect)
            return baseScript->OnGossipSelect(player, creature, sender, action);
        return false;
    }

    // Called when a player selects a gossip with a code in the creature's gossip menu.
    virtual bool OnGossipSelectCode(Player* player, Creature* creature, uint32 sender, uint32 action, const char* code) // { return false; }
    {
        if (baseScript && baseScript->OnGossipSelectCode)
            return baseScript->OnGossipSelectCode(player, creature, sender, action, code);
        return false;
    }

    // Called when a player accepts a quest from the creature.
    virtual bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest) // { return false; }
    {
        if (baseScript && baseScript->OnQuestAccept)
            return baseScript->OnQuestAccept(player, creature, quest);
        return false;
    }

    // Called when a player selects a quest in the creature's quest menu.
    virtual bool OnQuestSelect(Player* player, Creature* creature, Quest const* quest) //{ return false; }
    {
        if (baseScript && baseScript->OnQuestSelect)
            return baseScript->OnQuestSelect(player, creature, quest);
        return false;
    }

    // Called when a player completes a quest with the creature.
    virtual bool OnQuestComplete(Player* player, Creature* creature, Quest const* quest) //{ return false; }
    {
        if (baseScript && baseScript->OnQuestComplete)
            return baseScript->OnQuestComplete(player, creature, quest);
        return false;
    }

    // Called when a player selects a quest reward.
    virtual bool OnQuestReward(Player* player, Creature* creature, Quest const* quest, uint32 opt) //{ return false; }
    {
        if (baseScript && baseScript->OnQuestReward)
            return baseScript->OnQuestReward(player, creature, quest, opt);
        return false;
    }

    //OLD! Only here for backward compat, if you're scripting a new creature override CreatureAI::ReceiveEmote instead
    virtual bool OnReceiveEmote(Player* player, Creature* creature, uint32 emote) //{ return false; }
    {
        if (baseScript && baseScript->OnReceiveEmote)
            return baseScript->OnReceiveEmote(player, creature, emote);
        return false;
    }

    virtual bool OnEffectDummyCreature(Unit* caster, uint32 spellid, uint32 effIndex, Creature* targetCreature) // { return false; }
    {
        if (baseScript && baseScript->OnEffectDummyCreature)
            return baseScript->OnEffectDummyCreature(caster, spellid, effIndex, targetCreature);
        return false;
    }

    // Called when the dialog status between a player and the creature is requested.
    virtual uint32 GetDialogStatus(Player* player, Creature* creature) //{ return DIALOG_STATUS_SCRIPTED_NO_STATUS; }
    {
        if (baseScript && baseScript->pGetDialogStatus)
            return baseScript->pGetDialogStatus(player, creature);
        return DIALOG_STATUS_SCRIPTED_NO_STATUS;
    }

    // Called when a CreatureAI object is needed for the creature.
    virtual CreatureAI* GetAI(Creature* creature) const //{ return NULL; }
    {
        if (baseScript && baseScript->GetAI)
            return baseScript->GetAI(creature);
        return nullptr;
    }

    //Backward compat function, OLDScript used to create this script, this is stored here because we use it's member functions
    OLDScript* baseScript;
};

class GameObjectScript : public ScriptObject, public UpdatableScript<GameObject>
{
protected:

    GameObjectScript(const char* name);

public:

    bool IsDatabaseBound() const { return true; }

    // Called when a player opens a gossip dialog with the gameobject.
    virtual bool OnGossipHello(Player* /*player*/, GameObject* /*go*/) { return false; }

    // Called when a player selects a gossip item in the gameobject's gossip menu.
    virtual bool OnGossipSelect(Player* /*player*/, GameObject* /*go*/, uint32 /*sender*/, uint32 /*action*/) { return false; }

    // Called when a player selects a gossip with a code in the gameobject's gossip menu.
    virtual bool OnGossipSelectCode(Player* /*player*/, GameObject* /*go*/, uint32 /*sender*/, uint32 /*action*/, const char* /*code*/) { return false; }

    // Called when a player accepts a quest from the gameobject.
    virtual bool OnQuestAccept(Player* /*player*/, GameObject* /*go*/, Quest const* /*quest*/) { return false; }

    // Called when a player selects a quest reward.
    virtual bool OnQuestReward(Player* /*player*/, GameObject* /*go*/, Quest const* /*quest*/, uint32 /*opt*/) { return false; }

    // Called when the dialog status between a player and the gameobject is requested.
    virtual uint32 GetDialogStatus(Player* /*player*/, GameObject* /*go*/) { return DIALOG_STATUS_SCRIPTED_NO_STATUS; }

    // Called when the game object is destroyed (destructible buildings only).
    virtual void OnDestroyed(GameObject* /*go*/, Player* /*player*/) { }

    // Called when the game object is damaged (destructible buildings only).
    virtual void OnDamaged(GameObject* /*go*/, Player* /*player*/) { }

    // Called when the game object loot state is changed.
    virtual void OnLootStateChanged(GameObject* /*go*/, uint32 /*state*/, Unit* /*unit*/) { }

    // Called when the game object state is changed.
    virtual void OnGameObjectStateChanged(GameObject* /*go*/, uint32 /*state*/) { }

    // Called when a GameObjectAI object is needed for the gameobject.
    virtual GameObjectAI* GetAI(GameObject* /*go*/) const { return NULL; }
};

class AreaTriggerScript : public ScriptObject
{
protected:

    AreaTriggerScript(const char* name);

public:

    bool IsDatabaseBound() const { return true; }

    // Called when the area trigger is activated by a player.
    virtual bool OnTrigger(Player* /*player*/, AreaTriggerEntry const* /*trigger*/) { return false; }
};

// Manages registration, loading, and execution of scripts.
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
        void RegisterOLDScript(OLDScript*& script);
        void FillSpellSummary();

        void IncrementScriptCount() { ++_scriptCount; }
        uint32 GetScriptCount() const { return _scriptCount; }

    public:

        std::string GetConfigValueStr(char const* option);
        int32 GetConfigValueInt32(char const* option);
        float GetConfigValueFloat(char const* option);

    public: /* AccountScript */

        /*
        void OnAccountLogin(uint32 accountId);
        void OnFailedAccountLogin(uint32 accountId);
        void OnEmailChange(uint32 accountId);
        void OnFailedEmailChange(uint32 accountId);
        void OnPasswordChange(uint32 accountId);
        void OnFailedPasswordChange(uint32 accountId);
       */

    public: /* GuildScript */

        /*
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
        */

    public: /* GroupScript */

        /*
        void OnGroupAddMember(Group* group, uint64 guid);
        void OnGroupInviteMember(Group* group, uint64 guid);
//        void OnGroupRemoveMember(Group* group, uint64 guid, RemoveMethod method, uint64 kicker, const char* reason);
        void OnGroupChangeLeader(Group* group, uint64 newLeaderGuid, uint64 oldLeaderGuid);
        void OnGroupDisband(Group* group);
        */

    public: /* MapScript */
        void OnCreateMap(Map* map);
        void OnDestroyMap(Map* map);
        void OnLoadGridMap(Map* map, GridMap* gmap, uint32 gx, uint32 gy);
        void OnUnloadGridMap(Map* map, GridMap* gmap, uint32 gx, uint32 gy);
        void OnPlayerEnterMap(Map* map, Player* player);
        void OnPlayerLeaveMap(Map* map, Player* player);
        void OnMapUpdate(Map* map, uint32 diff);

    public: /* InstanceMapScript */

        InstanceScript* CreateInstanceScript(InstanceMap* map);

    public: /* TransportScript */

        void OnAddPassenger(Transport* transport, Player* player);
        void OnAddCreaturePassenger(Transport* transport, Creature* creature);
        void OnRemovePassenger(Transport* transport, Player* player);
        void OnTransportUpdate(Transport* transport, uint32 diff);
        void OnRelocate(Transport* transport, uint32 waypointId, uint32 mapId, float x, float y, float z);

    public: /* PlayerScript */

        //void OnPVPKill(Player* killer, Player* killed);
        //void OnCreatureKill(Player* killer, Creature* killed);
        //void OnPlayerKilledByCreature(Creature* killer, Player* killed);
        //void OnPlayerLevelChanged(Player* player, uint8 oldLevel);
        //void OnPlayerFreeTalentPointsChanged(Player* player, uint32 newPoints);
        //void OnPlayerTalentsReset(Player* player, bool noCost);
        //void OnPlayerMoneyChanged(Player* player, int32& amount);
        //void OnPlayerMoneyLimit(Player* player, int32& amount);
        //void OnGivePlayerXP(Player* player, uint32& amount, Unit* victim);
        //void OnPlayerReputationChange(Player* player, uint32 factionID, int32& standing, bool incremental);
        //void OnPlayerDuelRequest(Player* target, Player* challenger);
        //void OnPlayerDuelStart(Player* player1, Player* player2);
        //void OnPlayerDuelEnd(Player* winner, Player* loser, DuelCompleteType type);
        /*void OnPlayerChat(Player* player, uint32 type, uint32 lang, std::string& msg);
        void OnPlayerChat(Player* player, uint32 type, uint32 lang, std::string& msg, Player* receiver);
        void OnPlayerChat(Player* player, uint32 type, uint32 lang, std::string& msg, Group* group);
        void OnPlayerChat(Player* player, uint32 type, uint32 lang, std::string& msg, Guild* guild);
        void OnPlayerChat(Player* player, uint32 type, uint32 lang, std::string& msg, Channel* channel);*/
        /*void OnPlayerEmote(Player* player, uint32 emote);
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
        */
    public: /* ServerScript */

        /*
        void OnNetworkStart();
        void OnNetworkStop();
        void OnSocketOpen(std::shared_ptr<WorldSocket> socket);
        void OnSocketClose(std::shared_ptr<WorldSocket> socket);
        void OnPacketReceive(WorldSession* session, WorldPacket const& packet);
        void OnPacketSend(WorldSession* session, WorldPacket const& packet);
        void OnUnknownPacketReceive(WorldSession* session, WorldPacket const& packet);
        */
    public: /* WorldScript */
        /*
        void OnOpenStateChange(bool open) {}
        void OnConfigLoad(bool reload) {}
        void OnMotdChange(std::string& newMotd) {}
        void OnShutdownInitiate(ShutdownExitCode code, ShutdownMask mask) {}
        void OnShutdownCancel() {}
        void OnWorldUpdate(uint32 diff) {}
        void OnStartup() {}
        void OnShutdown() {}
        */
    public: /* ItemScript */

        bool OnQuestAccept(Player* player, Item* item, Quest const* quest);
        bool OnItemUse(Player* player, Item* item, SpellCastTargets const& targets);
        bool OnItemExpire(Player* player, ItemTemplate const* proto);

    public: /* CreatureScript */

        bool OnGossipHello(Player* player, Creature* creature);
        bool OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action);
        bool OnGossipSelectCode(Player* player, Creature* creature, uint32 sender, uint32 action, const char* code);
        bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest);
        bool OnQuestSelect(Player* player, Creature* creature, Quest const* quest);
        bool OnQuestComplete(Player* player, Creature* creature, Quest const* quest);
        bool OnQuestReward(Player* player, Creature* creature, Quest const* quest, uint32 opt);
        uint32 GetDialogStatus(Player* player, Creature* creature);
        CreatureAI* GetCreatureAI(Creature* creature);
        void OnCreatureUpdate(Creature* creature, uint32 diff);
        //!Only here for backward compat, if you're scripting a new creature override CreatureAI::ReceiveEmote instead
        bool ReceiveEmote(Player *player, Creature *_Creature, uint32 emote);
        bool EffectDummyCreature(Unit *caster, uint32 spellId, uint32 effIndex, Creature *crTarget);

    public: /* GameObjectScript */

        bool OnGossipHello(Player* player, GameObject* go);
        bool OnGossipSelect(Player* player, GameObject* go, uint32 sender, uint32 action);
        bool OnGossipSelectCode(Player* player, GameObject* go, uint32 sender, uint32 action, const char* code);
        bool OnQuestAccept(Player* player, GameObject* go, Quest const* quest);
        bool OnQuestReward(Player* player, GameObject* go, Quest const* quest, uint32 opt);
        uint32 GetDialogStatus(Player* player, GameObject* go);
        void OnGameObjectDestroyed(GameObject* go, Player* player);
        void OnGameObjectDamaged(GameObject* go, Player* player);
        void OnGameObjectLootStateChanged(GameObject* go, uint32 state, Unit* unit);
        void OnGameObjectStateChanged(GameObject* go, uint32 state);
        void OnGameObjectUpdate(GameObject* go, uint32 diff);
        GameObjectAI* GetGameObjectAI(GameObject* go);

    public: /* AreaTriggerScript */

        bool OnAreaTrigger(Player* player, AreaTriggerEntry const* trigger);

    private:

        uint32 _scriptCount;

        OLDScript* m_scripts[MAX_SCRIPTS] = { nullptr };
};

//Generic scripting text function
void DoScriptText(int32 textEntry, Unit* pSource, Unit* target = NULL);

#define sScriptMgr ScriptMgr::instance()

template<class TScript>
class ScriptRegistry
{
public:

    typedef std::map<uint32, TScript*> ScriptMap;
    typedef typename ScriptMap::iterator ScriptMapIterator;

    // The actual list of scripts. This will be accessed concurrently, so it must not be modified
    // after server startup.
    static ScriptMap ScriptPointerList;

    static void AddScript(TScript* const script)
    {
        ASSERT(script);

        // See if the script is using the same memory as another script. If this happens, it means that
        // someone forgot to allocate new memory for a script.
        for (ScriptMapIterator it = ScriptPointerList.begin(); it != ScriptPointerList.end(); ++it)
        {
            if (it->second == script)
            {
                TC_LOG_ERROR("scripts", "Script '%s' has same memory pointer as '%s'.",
                    script->GetName().c_str(), it->second->GetName().c_str());

                return;
            }
        }

        if (script->IsDatabaseBound())
        {
            // Get an ID for the script. An ID only exists if it's a script that is assigned in the database
            // through a script name (or similar).
            uint32 id = sObjectMgr->GetScriptId(script->GetName().c_str());
            if (id)
            {
                // Try to find an existing script.
                bool existing = false;
                for (ScriptMapIterator it = ScriptPointerList.begin(); it != ScriptPointerList.end(); ++it)
                {
                    // If the script names match...
                    if (it->second->GetName() == script->GetName())
                    {
                        // ... It exists.
                        existing = true;
                        break;
                    }
                }

                // If the script isn't assigned -> assign it!
                if (!existing)
                {
                    ScriptPointerList[id] = script;
                    sScriptMgr->IncrementScriptCount();
                }
                else
                {
                    // If the script is already assigned -> delete it!
                    TC_LOG_ERROR("scripts", "Script '%s' already assigned with the same script name, so the script can't work.",
                        script->GetName().c_str());

                    ASSERT(false); // Error that should be fixed ASAP.
                }
            }
            else
            {
                // The script uses a script name from database, but isn't assigned to anything.
                if (script->GetName().find("Smart") == std::string::npos)
                    TC_LOG_ERROR("sqL.sql", "Script named '%s' does not have a script name assigned in database.",
                        script->GetName().c_str());
            }
        }
        else
        {
            // We're dealing with a code-only script; just add it.
            ScriptPointerList[_scriptIdCounter++] = script;
            sScriptMgr->IncrementScriptCount();
        }
    }

    // Gets a script by its ID (assigned by ObjectMgr).
    static TScript* GetScriptById(uint32 id)
    {
        ScriptMapIterator it = ScriptPointerList.find(id);
        if (it != ScriptPointerList.end())
            return it->second;

        return NULL;
    }

private:

    // Counter used for code-only scripts.
    static uint32 _scriptIdCounter;
};

#endif

