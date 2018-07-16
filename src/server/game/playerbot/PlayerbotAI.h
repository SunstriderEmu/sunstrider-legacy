#ifndef PLAYERBOT_AI_H
#define PLAYERBOT_AI_H

#include "PlayerbotMgr.h"
#include "PlayerbotAIBase.h"
#include "strategy/AiObjectContext.h"
#include "strategy/Engine.h"
#include "strategy/ExternalEventHelper.h"
#include "ChatFilter.h"
#include "PlayerbotSecurity.h"
#include "ChatHelper.h"
#include "Chat.h"
#include <stack>

class Player;
class PlayerbotMgr;
class TestPlayer;

using namespace std;
using namespace ai;

bool IsAlliance(uint8 race);

class PlayerbotChatHandler: protected ChatHandler
{
public:
    explicit PlayerbotChatHandler(Player* pMasterPlayer);
    void sysmessage(std::string str) { SendSysMessage(str.c_str()); }
    uint32 extractQuestId(std::string str);
    uint32 extractSpellId(std::string str)
    {
        char* source = (char*)str.c_str();
        return extractSpellIdFromLink(source);
    }
};

namespace ai
{
    class MinValueCalculator {
    public:
        MinValueCalculator(float def = 0.0f) {
            param = nullptr;
            minValue = def;
        }

    public:
        void probe(float value, void* p) {
            if (!param || minValue >= value) {
                minValue = value;
                param = p;
            }
        }

    public:
        void* param;
        float minValue;
    };
};

enum BotState
{
    BOT_STATE_COMBAT = 0,
    BOT_STATE_NON_COMBAT = 1,
    BOT_STATE_DEAD = 2
};

#define BOT_STATE_MAX 3

class PacketHandlingHelper
{
public:
    void AddHandler(uint16 opcode, std::string handler);
    void Handle(ExternalEventHelper &helper);
    void AddPacket(const WorldPacket& packet);

private:
    map<uint16, std::string> handlers;
    stack<WorldPacket> queue;
};

class ChatCommandHolder
{
public:
    ChatCommandHolder(std::string command, Player* owner = nullptr, uint32 type = CHAT_MSG_WHISPER) : command(std::move(command)), owner(owner), type(type) {}
    ChatCommandHolder(ChatCommandHolder const& other)
    {
        this->command = other.command;
        this->owner = other.owner;
        this->type = other.type;
    }

public:
    std::string GetCommand() { return command; }
    Player* GetOwner() { return owner; }
    uint32 GetType() { return type; }

private:
    std::string command;
    Player* owner;
    uint32 type;
};

class TC_GAME_API PlayerbotAI : public PlayerbotAIBase
{
public:
    PlayerbotAI();
    PlayerbotAI(Player* bot);
    virtual ~PlayerbotAI();

public:
    void UpdateAI(uint32 elapsed) override;
    void UpdateAIInternal(uint32 elapsed) override;
    string HandleRemoteCommand(std::string command);
    void HandleCommand(uint32 type, const std::string& text, Player& fromPlayer);
    void HandleBotOutgoingPacket(const WorldPacket& packet);
    void HandleMasterIncomingPacket(const WorldPacket& packet);
    void HandleMasterOutgoingPacket(const WorldPacket& packet);
    void HandleTeleportAck();
    void ChangeEngine(BotState type);
    void DoNextAction();
    void DoSpecificAction(std::string name);
    void ChangeStrategy(std::string name, BotState type);
    bool ContainsStrategy(StrategyType type);
    bool HasStrategy(std::string name, BotState type);
    void ResetStrategies();
    void ReInitCurrentEngine();
    void Reset();
    bool IsTank(Player* player);
    bool IsHeal(Player* player);
    bool IsRanged(Player* player);
    Creature* GetCreature(ObjectGuid guid);
    Unit* GetUnit(ObjectGuid guid);
    GameObject* GetGameObject(ObjectGuid guid);
    bool TellMaster(std::ostringstream &stream, PlayerbotSecurityLevel securityLevel = PLAYERBOT_SECURITY_ALLOW_ALL) { return TellMaster(stream.str(), securityLevel); }
    bool TellMaster(std::string text, PlayerbotSecurityLevel securityLevel = PLAYERBOT_SECURITY_ALLOW_ALL);
    bool TellMasterNoFacing(std::string text, PlayerbotSecurityLevel securityLevel = PLAYERBOT_SECURITY_ALLOW_ALL);
    void SpellInterrupted(uint32 spellid);
    int32 CalculateGlobalCooldown(uint32 spellid);
    void InterruptSpell();
    void RemoveAura(std::string name);
    void RemoveShapeshift();
    void WaitForSpellCast(Spell *spell);

    virtual bool CanCastSpell(std::string name, Unit* target);
    virtual bool CastSpell(std::string name, Unit* target);
    virtual bool HasAura(std::string spellName, Unit* player);
    virtual bool HasAnyAuraOf(Unit* player, ...);

    virtual bool IsInterruptableSpellCasting(Unit* player, std::string spell);
    virtual bool HasAuraToDispel(Unit* player, uint32 dispelType);
    bool CanCastSpell(uint32 spellid, Unit* target, bool checkHasSpell = true);

    bool HasAura(uint32 spellId, const Unit* player);
    bool CastSpell(uint32 spellId, Unit* target);
    bool canDispel(const SpellInfo* entry, uint32 dispelType);

    uint32 GetEquipGearScore(Player* player, bool withBags, bool withBank);
     

    // sunstrider addition for testing: Called at any Damage to any victim (before damage apply)
    //virtual void DamageDealt(Unit* /*victim*/, uint32& /*damage*/, DamageEffectType /*damageType*/) { }
    //virtual void DamageTaken(Unit *done_by, uint32 & /*damage*/, DamageEffectType /*damageType*/) { }
    virtual void CastedDamageSpell(Unit const* target, SpellNonMeleeDamage damageInfo, SpellMissInfo missInfo, bool crit) const { }
    virtual void CastedHealingSpell(Unit const* target, uint32 healing, uint32 realGain, uint32 spellID, SpellMissInfo missInfo, bool crit) const { }
    virtual void PeriodicTick(Unit const* target, int32 amount, uint32 spellID) const { }
    virtual void DoneWhiteDamage(Unit const* target, CalcDamageInfo const* damageInfo) const { }
    virtual void SpellDamageDealt(Unit const* target, uint32 damage, uint32 spellID) const { };

private:
    void _fillGearScoreData(Player *player, Item* item, std::vector<uint32>* gearScore, uint32& twoHandScore);

public:
    Player* GetBot() { return bot; }
    Player* GetMaster() { return master; }
    void SetMaster(Player* _master) { master = _master; }
    AiObjectContext* GetAiObjectContext() { return aiObjectContext; }
    ChatHelper* GetChatHelper() { return &chatHelper; }
    bool IsOpposing(Player* player);
    static bool IsOpposing(uint8 race1, uint8 race2);
    PlayerbotSecurity* GetSecurity() { return &security; }

protected:
    Player* bot;
    Player* master;
    uint32 accountId;
    AiObjectContext* aiObjectContext;
    Engine* currentEngine;
    Engine* engines[BOT_STATE_MAX];
    BotState currentState;
    ChatHelper chatHelper;
    stack<ChatCommandHolder> chatCommands;
    PacketHandlingHelper botOutgoingPacketHandlers;
    PacketHandlingHelper masterIncomingPacketHandlers;
    PacketHandlingHelper masterOutgoingPacketHandlers;
    CompositeChatFilter chatFilter;
    PlayerbotSecurity security;
};

class TC_GAME_API PlayerbotTestingAI : public PlayerbotAI
{
public:
    PlayerbotTestingAI(Player* bot);
    virtual ~PlayerbotTestingAI() {}

    void UpdateAIInternal(uint32 elapsed) override;
    virtual void CastedDamageSpell(Unit const* target, SpellNonMeleeDamage damageInfo, SpellMissInfo missInfo, bool crit) const override;
    virtual void CastedHealingSpell(Unit const* target, uint32 healing, uint32 realGain, uint32 spellID, SpellMissInfo missInfo, bool crit) const override;
    virtual void PeriodicTick(Unit const* target, int32 amount, uint32 spellID) const override;
    virtual void DoneWhiteDamage(Unit const* target, CalcDamageInfo const* damageInfo) const override;
    virtual void SpellDamageDealt(Unit const* target, uint32 damage, uint32 spellID) const override;

    void ResetSpellCounters();

    struct SpellDamageDoneInfo
    {
        SpellDamageDoneInfo(uint32 spellID, SpellNonMeleeDamage damageInfo, SpellMissInfo missInfo, bool crit) :
            spellID(spellID), damageInfo(std::move(damageInfo)), missInfo(missInfo), crit(crit)
        {}

        uint32 spellID;
        SpellNonMeleeDamage damageInfo;
        SpellMissInfo missInfo;
        bool crit;
    };

    struct MeleeDamageDoneInfo
    {
        MeleeDamageDoneInfo(CalcDamageInfo const* damageInfo) :
            damageInfo(*damageInfo)
        {}

        CalcDamageInfo damageInfo;
    };

    struct HealingDoneInfo
    {
        uint32 spellID;
        uint32 healing; //healing done by spell
        uint32 realGain; //real gain, ex: a unit at full health will gain 0 health from healing
        SpellMissInfo missInfo;
        bool crit;
    };

    struct TickInfo
    {
        uint32 spellID;
        int32 amount;
    };

    //Get DoT total damage, resists and absorbs ignored. Also returns how much ticks were found
    std::pair<int32 /*totalDmg*/, uint32 /*tickCount*/> GetDotDamage(Unit const* to, uint32 spellID) const;
    //Get spells damage info for this caster on given target. Note that ranged attacks are also spells
    std::vector<SpellDamageDoneInfo> const* GetSpellDamageDoneInfo(Unit const* target) const;
    //Get healing spells info for this caster on given target.
    std::vector<HealingDoneInfo> const* GetHealingDoneInfo(Unit const* target) const;
    //Return main and offhand damages for this caster on given target. Does NOT includes ranged damage, those are in GetSpellDamageDoneInfo
    std::vector<MeleeDamageDoneInfo> const* GetMeleeDamageDoneInfo(Unit const* target) const;

private:
   
    mutable std::unordered_map<ObjectGuid /*targetGUID*/, std::vector<SpellDamageDoneInfo>> spellDamageDone;
    mutable std::unordered_map<ObjectGuid /*targetGUID*/, std::vector<MeleeDamageDoneInfo>> meleeDamageDone;
    mutable std::unordered_map<ObjectGuid /*targetGUID*/, std::vector<HealingDoneInfo>> healingDone;
    mutable std::unordered_map<ObjectGuid /*targetGUID*/, std::vector<TickInfo>> ticksDone;

};

#endif