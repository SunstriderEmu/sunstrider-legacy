
#ifndef __SPELL_H
#define __SPELL_H

#include "GridDefines.h"
#include "PathGenerator.h"
#include "Position.h"
#include "DBCEnums.h"
#include "ConditionMgr.h"

namespace WorldPackets
{
    namespace Spells
    {
        struct SpellTargetData;
        struct SpellAmmo;
        struct SpellCastData;
    }
}
class Unit;
class UnitAura;
class Player;
class GameObject;
class DynamicObject;
class DynObjAura;
class Aura;
class Item;
class Object;
enum SpellCastResult : int;
class SpellScript;
class SpellEvent;
enum LootType : uint8;
enum SpellCastTargetFlags : uint32;
class SpellImplicitTargetInfo;
enum SpellTargetCheckTypes : uint8;
enum SpellTargetObjectTypes : uint8;
enum SpellTargetSelectionCategories : uint8;
enum CurrentSpellTypes : uint8;

#define SPELL_CHANNEL_UPDATE_INTERVAL 1000
#define MAX_SPELL_RANGE_TOLERANCE 3.0f
#define TRAJECTORY_MISSILE_SIZE 3.0f

enum SpellCastFlags
{
    CAST_FLAG_NONE               = 0x00000000,
    CAST_FLAG_PENDING            = 0x00000001, // stucks spell highlight
    CAST_FLAG_UNKNOWN_2          = 0x00000002,
    CAST_FLAG_UNKNOWN_3          = 0x00000004,
    CAST_FLAG_UNKNOWN_4          = 0x00000008,              // ignore AOE visual
    CAST_FLAG_UNKNOWN_5          = 0x00000010,
    CAST_FLAG_AMMO               = 0x00000020,              // Projectiles visual
    CAST_FLAG_UNKNOWN_7          = 0x00000040,
    CAST_FLAG_UNKNOWN_8          = 0x00000080,
    CAST_FLAG_UNKNOWN_9          = 0x00000100,
    //not sure when does bc stops here
#ifdef LICH_KING
    CAST_FLAG_UNKNOWN_10         = 0x00000200,
    CAST_FLAG_UNKNOWN_11         = 0x00000400,
    CAST_FLAG_POWER_LEFT_SELF    = 0x00000800,
    CAST_FLAG_UNKNOWN_13         = 0x00001000,
    CAST_FLAG_UNKNOWN_14         = 0x00002000,
    CAST_FLAG_UNKNOWN_15         = 0x00004000,
    CAST_FLAG_UNKNOWN_16         = 0x00008000,
    CAST_FLAG_UNKNOWN_17         = 0x00010000,
    CAST_FLAG_ADJUST_MISSILE     = 0x00020000,
    CAST_FLAG_NO_GCD             = 0x00040000,              // no GCD for spell casts from charm/summon (vehicle spells is an example)
    CAST_FLAG_VISUAL_CHAIN       = 0x00080000,
    CAST_FLAG_UNKNOWN_21         = 0x00100000,
    CAST_FLAG_RUNE_LIST          = 0x00200000,
    CAST_FLAG_UNKNOWN_23         = 0x00400000,
    CAST_FLAG_UNKNOWN_24         = 0x00800000,
    CAST_FLAG_UNKNOWN_25         = 0x01000000,
    CAST_FLAG_UNKNOWN_26         = 0x02000000,
    CAST_FLAG_IMMUNITY           = 0x04000000,
    CAST_FLAG_UNKNOWN_28         = 0x08000000,
    CAST_FLAG_UNKNOWN_29         = 0x10000000,
    CAST_FLAG_UNKNOWN_30         = 0x20000000,
    CAST_FLAG_UNKNOWN_31         = 0x40000000,
    CAST_FLAG_UNKNOWN_32         = 0x80000000
#endif
};

enum SpellRangeFlag
{
    SPELL_RANGE_DEFAULT             = 0,
    SPELL_RANGE_MELEE               = 1,     //melee
    SPELL_RANGE_RANGED              = 2,     //hunter range and ranged weapon
};

enum SpellNotifyPushType
{
    PUSH_NONE           = 0,
    PUSH_IN_FRONT,
    PUSH_IN_FRONT_180,
    PUSH_IN_BACK,
    PUSH_IN_LINE,
    PUSH_SRC_CENTER,
    PUSH_DST_CENTER,
    PUSH_CASTER_CENTER, //this is never used in grid search
    PUSH_CHAIN,
};

bool IsQuestTameSpell(uint32 spellId);

struct SpellDestination
{
    SpellDestination();
    SpellDestination(float x, float y, float z, float orientation = 0.0f, uint32 mapId = MAPID_INVALID);
    SpellDestination(Position const& pos);
    SpellDestination(WorldObject const& wObj);

    void Relocate(Position const& pos);
    void RelocateOffset(Position const& offset);

    WorldLocation _position;
    //no transport handling in spell destination for BC client
#ifdef LICH_KING
    ObjectGuid _transportGUID;
    Position _transportOffset;
#endif
};

class TC_GAME_API SpellCastTargets
{
public:
    SpellCastTargets();
    ~SpellCastTargets();

    void Read(ByteBuffer& data, Unit* caster);
    void Write(WorldPackets::Spells::SpellTargetData& data);

    uint32 GetTargetMask() const { return m_targetMask; }
    void SetTargetMask(uint32 newMask) { m_targetMask = newMask; }

    void SetTargetFlag(SpellCastTargetFlags flag) { m_targetMask |= flag; }

    ObjectGuid GetOrigUnitTargetGUID() const;
    void SetOrigUnitTarget(Unit* target);

    ObjectGuid GetUnitTargetGUID() const;
    Unit* GetUnitTarget() const;
    void SetUnitTarget(Unit* target);

    ObjectGuid GetGOTargetGUID() const;
    GameObject* GetGOTarget() const;
    void SetGOTarget(GameObject* target);

    ObjectGuid GetCorpseTargetGUID() const;
    Corpse* GetCorpseTarget() const;

    WorldObject* GetObjectTarget() const;
    ObjectGuid GetObjectTargetGUID() const;
    void RemoveObjectTarget();

    ObjectGuid GetItemTargetGUID() const { return m_itemTargetGUID; }
    Item* GetItemTarget() const { return m_itemTarget; }
    uint32 GetItemTargetEntry() const { return m_itemTargetEntry; }
    void SetItemTarget(Item* item);
    void SetTradeItemTarget(Player* caster);
    void UpdateTradeSlotItem();

    SpellDestination const* GetSrc() const;
    Position const* GetSrcPos() const;
    void SetSrc(float x, float y, float z);
    void SetSrc(Position const& pos);
    void SetSrc(WorldObject const& wObj);
    void ModSrc(Position const& pos);
    void RemoveSrc();

    SpellDestination const* GetDst() const;
    WorldLocation const* GetDstPos() const;
    void SetDst(float x, float y, float z, float orientation, uint32 mapId = MAPID_INVALID);
    void SetDst(Position const& pos);
    void SetDst(WorldObject const& wObj);
    void SetDst(SpellDestination const& spellDest);
    void SetDst(SpellCastTargets const& spellTargets);
    void ModDst(Position const& pos);
    void ModDst(SpellDestination const& spellDest);
    void RemoveDst();

    bool HasSrc() const;
    bool HasDst() const;
    bool HasTraj() const { return m_speed != 0; }

    float GetElevation() const { return m_elevation; }
    void SetElevation(float elevation) { m_elevation = elevation; }
    float GetSpeed() const { return m_speed; }
    void SetSpeed(float speed) { m_speed = speed; }

    float GetDist2d() const { return m_src._position.GetExactDist2d(&m_dst._position); }
    float GetSpeedXY() const { return m_speed * cos(m_elevation); }
    float GetSpeedZ() const { return m_speed * sin(m_elevation); }

    void Update(WorldObject* caster);

    // sunwell: Channel data
    void SetObjectTargetChannel(ObjectGuid targetGUID);
    void SetDstChannel(SpellDestination const& spellDest);
    WorldObject* GetObjectTargetChannel(WorldObject* caster) const;
    bool HasDstChannel() const;
    SpellDestination const* GetDstChannel() const;

private:
    uint32 m_targetMask;

    // objects (can be used at spell creating and after Update at casting)
    WorldObject* m_objectTarget;
    Item* m_itemTarget;

    // object GUID/etc, can be used always
    ObjectGuid m_origObjectTargetGUID;
    ObjectGuid m_objectTargetGUID;
    ObjectGuid m_itemTargetGUID;
    uint32 m_itemTargetEntry;

    SpellDestination m_src;
    SpellDestination m_dst;

    float m_elevation, m_speed;
    std::string m_strTarget;

    // sunwell: Save channel data
    /* sun: Changed from TC logic, we now use this var instead of UNIT_FIELD_CHANNEL_OBJECT for channel targets
    because UNIT_FIELD_CHANNEL_OBJECT may be cleared prematurely in some cases where spells finishes before
    a SPELL_AURA_PERIODIC_TRIGGER_SPELL aura does */
    SpellDestination m_dstChannel;
    ObjectGuid m_objectTargetGUIDChannel;
};

struct SpellValue
{
    explicit SpellValue(SpellInfo const* spellInfo);

    uint32  MaxAffectedTargets;
    int32   EffectBasePoints[MAX_SPELL_EFFECTS];
    float   RadiusMod;
    uint8   AuraStackAmount;
    float   CriticalChance;
};

enum SpellState
{
    SPELL_STATE_NULL      = 0,
    SPELL_STATE_PREPARING = 1,
    SPELL_STATE_CASTING   = 2,
    SPELL_STATE_FINISHED  = 3,
    SPELL_STATE_IDLE      = 4,
    SPELL_STATE_DELAYED   = 5
};

enum SpellEffectHandleMode
{
    SPELL_EFFECT_HANDLE_LAUNCH, //process on launch for caster. Ex: spell triggering another spell
    SPELL_EFFECT_HANDLE_LAUNCH_TARGET, //process on launch, for each target. Ex: damage and heal calculations
    SPELL_EFFECT_HANDLE_HIT, //process on hit for caster. Ex: Summons
    SPELL_EFFECT_HANDLE_HIT_TARGET, //process on hit, for each target affected. Ex: Aura apply
};

typedef std::vector<std::pair<uint32, ObjectGuid>> DispelList;

// special structure containing data for channel target spells
struct ChannelTargetData
{
    ChannelTargetData(ObjectGuid cguid, const SpellDestination* dst) : channelGUID(cguid)
    {
        if (dst)
            spellDst = *dst;
    }

    ObjectGuid channelGUID;
    SpellDestination spellDst;
};

enum ReplenishType
{
    REPLENISH_UNDEFINED = 0,
    REPLENISH_HEALTH    = 20,
    REPLENISH_MANA      = 21,
    REPLENISH_RAGE      = 22
};

enum SpellTargets
{
    SPELL_TARGETS_ALLY,
    SPELL_TARGETS_ENEMY,
    SPELL_TARGETS_ENTRY,
    SPELL_TARGETS_CHAINHEAL,
};

class TC_GAME_API Spell
{
    friend class SpellScript;
    friend class TestCase;

    public:

        void EffectNULL(uint32 );
        void EffectUnused(uint32 );
        void EffectDistract(uint32 i);
        void EffectPull(uint32 i);
        void EffectSchoolDMG(uint32 i);
        void EffectEnvironmentalDMG(uint32 i);
        void EffectInstaKill(uint32 i);
        void EffectDummy(uint32 i);
        void EffectTeleportUnits(uint32 i);
        void EffectApplyAura(uint32 i);
        void EffectSendEvent(uint32 i);
        void EffectPowerBurn(uint32 i);
        void EffectPowerDrain(uint32 i);
        void EffectHeal(uint32 i);
        void EffectBind(uint32 i);
        void EffectHealthLeech(uint32 i);
        void EffectQuestComplete(uint32 i);
        void EffectCreateItem(uint32 i);
        void EffectPersistentAA(uint32 i);
        void EffectEnergize(uint32 i);
        void EffectOpenLock(uint32 i);
        void EffectSummonChangeItem(uint32 i);
        void EffectOpenSecretSafe(uint32 i);
        void EffectProficiency(uint32 i);
        void EffectSummonType(uint32 i);
        void _EffectSummonType(uint32 effIndex, SummonPropertiesEntry const* properties);
        //void EffectSummon(uint32 i);
        void EffectLearnSpell(uint32 i);
        void EffectDispel(uint32 i);
        void EffectDualWield(uint32 i);
        void EffectPickPocket(uint32 i);
        void EffectAddFarsight(uint32 i);
        void EffectHealMechanical(uint32 i);
        void EffectTeleUnitsFaceCaster(uint32 i);
        void EffectLearnSkill(uint32 i);
        void EffectAddHonor(uint32 i);
        void EffectTradeSkill(uint32 i);
        void EffectEnchantItemPerm(uint32 i);
        void EffectEnchantItemTmp(uint32 i);
        void EffectTameCreature(uint32 i);
        void EffectSummonPet(uint32 i);
        void EffectLearnPetSpell(uint32 i);
        void EffectWeaponDmg(uint32 i);
        void EffectForceCast(uint32 i);
        void EffectTriggerSpell(uint32 i);
        void EffectTriggerMissileSpell(uint32 i);
        void EffectThreat(uint32 i);
        void EffectHealMaxHealth(uint32 i);
        void EffectInterruptCast(uint32 i);
        void EffectSummonObjectWild(uint32 i);
        void EffectScriptEffect(uint32 i);
        void EffectSanctuary(uint32 i);
        void EffectAddComboPoints(uint32 i);
        void EffectDuel(uint32 i);
        void EffectStuck(uint32 i);
        void EffectSummonPlayer(uint32 i);
        void EffectActivateObject(uint32 i);
        void EffectEnchantHeldItem(uint32 i);
        void EffectSummonObject(uint32 i);
        void EffectResurrect(uint32 i);
        void EffectParry(uint32 i);
        void EffectBlock(uint32 i);
        void EffectLeap(uint32 i);
        void EffectTransmitted(uint32 i);
        void EffectDisEnchant(uint32 i);
        void EffectInebriate(uint32 i);
        void EffectFeedPet(uint32 i);
        void EffectDismissPet(uint32 i);
        void EffectReputation(uint32 i);
        void EffectSelfResurrect(uint32 i);
        void EffectSkinning(uint32 i);
        void EffectCharge(uint32 i);
        void EffectProspecting(uint32 i);
        void EffectSendTaxi(uint32 i);
        void EffectKnockBack(uint32 i);
        void EffectPullTowards(uint32 i);
        void EffectDispelMechanic(uint32 i);
        void EffectResurrectPet(uint32 i);
        void EffectDestroyAllTotems(uint32 i);
        void EffectDurabilityDamage(uint32 i);
        void EffectSkill(uint32 i);
        void EffectTaunt(uint32 i);
        void EffectDurabilityDamagePCT(uint32 i);
        void EffectModifyThreatPercent(uint32 i);
        void EffectResurrectNew(uint32 i);
        void EffectAddExtraAttacks(uint32 i);
        void EffectSpiritHeal(uint32 i);
        void EffectSkinPlayerCorpse(uint32 i);
        void EffectStealBeneficialBuff(uint32 i);
        void EffectUnlearnSpecialization(uint32 i);
        void EffectHealPct(uint32 i);
        void EffectEnergizePct(uint32 i);
        void EffectTriggerRitualOfSummoning(uint32 i);
        void EffectKillCredit(uint32 i);
        void EffectQuestFail(uint32 i);
        void EffectRedirectThreat(uint32 i);
        void EffectPlaySound(uint32 i);
        void EffectPlayMusic(uint32 i);
        void EffectForceCastWithValue(uint32 i);

        typedef std::unordered_set<Aura*> UsedSpellMods;

        Spell(WorldObject* caster, SpellInfo const *info, TriggerCastFlags triggerFlags, ObjectGuid originalCasterGUID = ObjectGuid::Empty, Spell** triggeringContainer = nullptr, bool skipCheck = false);
        ~Spell();

        void InitExplicitTargets(SpellCastTargets const& targets);
        void SelectExplicitTargets();

        void SelectSpellTargets();
        void SelectEffectImplicitTargets(SpellEffIndex effIndex, SpellImplicitTargetInfo const& targetType, uint32& processedEffectMask);
        void SelectImplicitChannelTargets(SpellEffIndex effIndex, SpellImplicitTargetInfo const& targetType);
        void SelectImplicitNearbyTargets(SpellEffIndex effIndex, SpellImplicitTargetInfo const& targetType, uint32 effMask);
        void SelectImplicitConeTargets(SpellEffIndex effIndex, SpellImplicitTargetInfo const& targetType, uint32 effMask);
        void SelectImplicitAreaTargets(SpellEffIndex effIndex, SpellImplicitTargetInfo const& targetType, uint32 effMask);
        void SelectImplicitCasterDestTargets(SpellEffIndex effIndex, SpellImplicitTargetInfo const& targetType);
        void SelectImplicitTargetDestTargets(SpellEffIndex effIndex, SpellImplicitTargetInfo const& targetType);
        void SelectImplicitDestDestTargets(SpellEffIndex effIndex, SpellImplicitTargetInfo const& targetType);
        void SelectImplicitCasterObjectTargets(SpellEffIndex effIndex, SpellImplicitTargetInfo const& targetType);
        void SelectImplicitTargetObjectTargets(SpellEffIndex effIndex, SpellImplicitTargetInfo const& targetType);
        void SelectImplicitChainTargets(SpellEffIndex effIndex, SpellImplicitTargetInfo const& targetType, WorldObject* target, uint32 effMask);
        void SelectImplicitTrajTargets(SpellEffIndex effIndex, SpellImplicitTargetInfo const& targetType);

        void SelectEffectTypeImplicitTargets(uint8 effIndex);

        uint32 GetSearcherTypeMask(SpellTargetObjectTypes objType, ConditionContainer* condList);
        template<class SEARCHER> void SearchTargets(SEARCHER& searcher, uint32 containerMask, WorldObject* referer, Position const* pos, float radius);

        WorldObject* SearchNearbyTarget(float range, SpellTargetObjectTypes objectType, SpellTargetCheckTypes selectionType, ConditionContainer* condList = nullptr);
        void SearchAreaTargets(std::list<WorldObject*>& targets, float range, Position const* position, WorldObject* referer, SpellTargetObjectTypes objectType, SpellTargetCheckTypes selectionType, ConditionContainer* condList);
        void SearchChainTargets(std::list<WorldObject*>& targets, uint32 chainTargets, WorldObject* target, SpellTargetObjectTypes objectType, SpellTargetCheckTypes selectType, SpellTargetSelectionCategories selectCategory, ConditionContainer* condList, bool isChainHeal);

        inline uint32 prepare(Unit* const target, AuraEffect const* triggeredByAura = nullptr)
        {
            SpellCastTargets targets;
            targets.SetUnitTarget(target);
            return prepare(targets, triggeredByAura);
        }
        //return SpellCastResult
        uint32 prepare(SpellCastTargets const& targets, AuraEffect const* triggeredByAura = nullptr);
        void cancel();
        void update(uint32 difftime);
        void cast(bool skipCheck = false);
        void _cast(bool skipCheck = false);
        void finish(bool ok = true, bool cancelChannel = true);
        void TakePower();
        void TakeAmmo();
        void TakeReagents();
        void TakeCastItem();
        SpellCastResult CheckCast(bool strict, uint32* param1 = nullptr, uint32* param2 = nullptr);
        SpellCastResult CheckPetCast(Unit* target);
        bool CanAutoCast(Unit* target);

        // handlers
        void handle_immediate();
        uint64 handle_delayed(uint64 t_offset);
        // handler helpers
        void _handle_immediate_phase();
        void _handle_finish_phase();

        SpellCastResult CheckItems(uint32* param1, uint32* param2);
        SpellCastResult CheckRange(bool strict);
        SpellCastResult CheckPower();
        SpellCastResult CheckCasterAuras(uint32* param1) const;

        int32 CalculateDamage(uint8 effIndex) const;

        bool CheckSpellCancelsAuraEffect(AuraType auraType, uint32* param1) const;
        bool CheckSpellCancelsCharm(uint32* param1) const;
        bool CheckSpellCancelsStun(uint32* param1) const;
        bool CheckSpellCancelsSilence(uint32* param1) const;
        bool CheckSpellCancelsPacify(uint32* param1) const;
        bool CheckSpellCancelsFear(uint32* param1) const;
        bool CheckSpellCancelsConfuse(uint32* param1) const;

        void Delayed();
        void DelayedChannel();
        inline uint32 getState() const { return m_spellState; }
        void setState(uint32 state) { m_spellState = state; }

        void DoCreateItem(uint32 i, uint32 itemtype);

        void UpdateSpellCastDataTargets(WorldPackets::Spells::SpellCastData& data);
        void UpdateSpellCastDataAmmo(WorldPackets::Spells::SpellAmmo& ammo);

        bool CheckEffectTarget(Unit const* target, uint32 eff) const;
        void CheckSrc() { if(!m_targets.HasSrc()) m_targets.SetSrc(m_caster); }
        void CheckDst() { if(!m_targets.HasDst()) m_targets.SetDst(m_caster); }

		static void WriteCastResultInfo(WorldPacket& data, Player* caster, SpellInfo const* spellInfo, uint8 castCount, SpellCastResult result, /*SpellCustomErrors customError,*/ uint32* param1 = nullptr, uint32* param2 = nullptr);
		static void SendCastResult(Player* caster, SpellInfo const* spellInfo, uint8 castCount, SpellCastResult result, /*SpellCustomErrors customError = SPELL_CUSTOM_ERROR_NONE,*/ uint32* param1 = nullptr, uint32* param2 = nullptr);
		void SendCastResult(SpellCastResult result, uint32* param1 = nullptr, uint32* param2 = nullptr) const;
        void SendSpellStart();
        void SendSpellGo();
        void SendSpellCooldown();
        void SendLogExecute();
        void ExecuteLogEffectTakeTargetPower(uint8 effIndex, Unit* target, uint32 powerType, uint32 powerTaken, float gainMultiplier);
        void ExecuteLogEffectExtraAttacks(uint8 effIndex, Unit* victim, uint32 attCount);
        void ExecuteLogEffectInterruptCast(uint8 effIndex, Unit* victim, uint32 spellId);
        void ExecuteLogEffectDurabilityDamage(uint8 effIndex, Unit* victim, int32 itemId, int32 slot);
        void ExecuteLogEffectOpenLock(uint8 effIndex, Object* obj);
        void ExecuteLogEffectCreateItem(uint8 effIndex, uint32 entry);
        void ExecuteLogEffectDestroyItem(uint8 effIndex, uint32 entry);
        void ExecuteLogEffectSummonObject(uint8 effIndex, WorldObject* obj);
        void ExecuteLogEffectUnsummonObject(uint8 effIndex, WorldObject* obj);
        void ExecuteLogEffectResurrect(uint8 effIndex, Unit* target);
        void SendInterrupted(uint8 result);
        void SendChannelUpdate(uint32 time);
        void SendChannelUpdate(uint32 time, uint32 spellId); //only update if channeling given spell
        void SendChannelStart(uint32 duration);
        ObjectGuid GetStartChannelTarget() const;
        void SendResurrectRequest(Player* target);

        void HandleEffects(Unit *pUnitTarget,Item *pItemTarget,GameObject *pGOTarget,uint32 i, SpellEffectHandleMode mode);
        void HandleFlatThreat();
        //void HandleAddAura(Unit* Target);

        const SpellInfo* const m_spellInfo;
        Item* m_CastItem;
        ObjectGuid m_castItemGUID;
        uint32 m_castItemEntry;
        uint8 m_cast_count;
        SpellCastTargets m_targets;
        bool m_skipCheck;

        UsedSpellMods m_appliedMods;

        int32 GetCastTime() const { return m_casttime; }
        bool IsAutoRepeat() const { return m_autoRepeat; }
        void SetAutoRepeat(bool rep) { m_autoRepeat = rep; }
        void ReSetTimer() { m_timer = m_casttime > 0 ? m_casttime : 0; }
        bool IsTriggered() const;
        bool IsIgnoringCooldowns() const;
        bool IsFocusDisabled() const;
        bool IsProcDisabled() const;
        bool IsChannelActive() const;
        bool IsAutoActionResetSpell() const;
        bool IsPositive() const;

        bool IsTriggeredByAura(SpellInfo const* auraSpellInfo) const { return (auraSpellInfo == m_triggeredByAuraSpell); }

        bool IsDeletable() const { return !m_referencedFromCurrentSpell && !m_executedCurrently; }
        void SetReferencedFromCurrent(bool yes) { m_referencedFromCurrentSpell = yes; }
        bool IsInterruptable() const { return !m_executedCurrently; }
        void SetExecutedCurrently(bool yes) { m_executedCurrently = yes; }
        uint64 GetDelayStart() const { return m_delayStart; }
        void SetDelayStart(uint64 m_time) { m_delayStart = m_time; }
        uint64 GetDelayMoment() const { return m_delayMoment; }
        uint64 GetDelayTrajectory() const { return m_delayTrajectory; }
        uint64 CalculateDelayMomentForDst() const;
        void RecalculateDelayMomentForDst();

        bool IsNeedSendToClient() const;

        CurrentSpellTypes GetCurrentContainer();

        WorldObject* GetCaster() const { return m_caster; }
        Unit* GetOriginalCaster() const { return m_originalCaster; }
        SpellInfo const* GetSpellInfo() const { return m_spellInfo; }
        int32 GetPowerCost() const { return m_powerCost; }

        bool UpdatePointers();                              // must be used at call Spell code after time delay (non triggered spell cast/update spell call/etc)

        void CleanupTargetList();

        void SetSpellValue(SpellValueMod mod, int32 value);
        
        bool DoesApplyAuraName(uint32 name);

        TriggerCastFlags GetTriggerCastFlags() const { return _triggeredCastFlags; }
        SpellDestination GetSpellDestination(SpellEffIndex effIndex) const { return m_destTargets[effIndex]; }

        SpellMissInfo GetForceHitResult() const { return _forceHitResult; }
        //once set, cannot be changed
        void SetForceHitResult(SpellMissInfo result);

    protected:
        bool HasGlobalCooldown();
        void TriggerGlobalCooldown();
        void CancelGlobalCooldown();

        void SendLoot(ObjectGuid guid, LootType loottype);
        std::pair<float, float> GetMinMaxRange(bool strict);

        WorldObject* const m_caster;

        SpellValue* const m_spellValue;

        ObjectGuid m_originalCasterGUID;                    // real source of cast (aura caster/etc), used for spell targets selection
                                                            // e.g. damage around area spell trigered by victim aura and damage enemies of aura caster
        Unit* m_originalCaster;                             // cached pointer for m_originalCaster, updated at Spell::UpdatePointers()

        Spell** m_selfContainer;                            // pointer to our spell container (if applicable)
        Spell** m_triggeringContainer;                      // pointer to container with spell that has triggered us

        //Spell data
        SpellSchoolMask m_spellSchoolMask;                  // Spell school (can be overwrite for some spells (wand shoot for example)
        WeaponAttackType m_attackType;                      // For weapon based attack
        int32 m_powerCost;                                  // Calculated spell cost     initialized only in Spell::prepare
        int32 m_casttime;                                   // Calculated spell cast time initialized only in Spell::prepare
        bool m_canReflect;                                  // can reflect this spell?
        bool m_autoRepeat;

        uint8 m_delayAtDamageCount;
        int32 GetNextDelayAtDamageMsTime() { return m_delayAtDamageCount < 5 ? 1000 - (m_delayAtDamageCount++)* 200 : 200; }

        // Delayed spells system
        uint64 m_delayStart;                                // time of spell delay start, filled by event handler, zero = just started
        uint64 m_delayMoment;                               // moment of next delay call, used internally
        uint64 m_delayTrajectory;                            // sunwell: Trajectory delay
        bool m_immediateHandled;                            // were immediate actions handled? (used by delayed spells only)

        // These vars are used in both delayed spell system and modified immediate spell system
        bool m_referencedFromCurrentSpell;                  // mark as references to prevent deleted and access by dead pointers
        bool m_executedCurrently;                           // mark as executed to prevent deleted and access by dead pointers
        bool m_needComboPoints;
        uint8 m_applyMultiplierMask;                        // by effect: damage multiplier needed?

        float m_damageMultipliers[MAX_SPELL_EFFECTS];                       // by effect: damage multiplier

        // Current targets, to be used in SpellEffects (MUST BE USED ONLY IN SPELL EFFECTS)
        Unit* unitTarget;
        Item* itemTarget;
        GameObject* gameObjTarget;
        WorldLocation* destTarget;

        int32 damage;
        SpellMissInfo targetMissInfo;
        SpellEffectHandleMode effectHandleMode;
        
        // -------------------------------------------
        GameObject* focusObject;

        // Damage and healing in effects need just calculate
        int32 m_damage;           // Damge   in effects count here
        int32 m_healing;          // Healing in effects count here

        // ******************************************
        // Spell trigger system
        // ******************************************
        uint32 m_procAttacker;                // Attacker trigger flags
        uint32 m_procVictim;                  // Victim   trigger flags
        uint32 m_hitMask;
        void   prepareDataForTriggerSystem();

        //*****************************************
        // Spell target subsystem
        //*****************************************
        // Targets store structures and data
        struct TargetInfoBase
        {
            virtual void PreprocessTarget(Spell* /*spell*/) { }
            virtual void DoTargetSpellHit(Spell* spell, uint8 effIndex) = 0;
            virtual void DoDamageAndTriggers(Spell* /*spell*/) { }

            uint8 EffectMask = 0;
        protected:
            TargetInfoBase() { }
            virtual ~TargetInfoBase() { }
        };

        struct TargetInfo : public TargetInfoBase
        { 
           void PreprocessTarget(Spell* spell) override;
           void DoTargetSpellHit(Spell* spell, uint8 effIndex) override;
           void DoDamageAndTriggers(Spell* spell) override;

            ObjectGuid TargetGUID;
            uint64 TimeDelay = 0ULL;
            int32  Damage = 0;
            int32  Healing = 0;

            SpellMissInfo MissCondition = SPELL_MISS_NONE;
            SpellMissInfo ReflectResult = SPELL_MISS_NONE;

            bool   IsAlive = false;
            bool   IsCrit = false;
            bool   ScaleAura = false;

            // info set at PreprocessTarget, used by DoTargetSpellHit
            DiminishingGroup DRGroup = DIMINISHING_NONE;
            int32 AuraDuration = 0;
            SpellInfo const* AuraSpellInfo = nullptr;
            int32  AuraBasePoints[MAX_SPELL_EFFECTS] = {};
            bool   Positive = true;
            UnitAura* HitAura = nullptr;

            Unit const* GetSpellHitTarget() const { return _spellHitTarget; }

        private:
            Unit* _spellHitTarget = nullptr; // changed for example by reflect
            bool _enablePVP = false;         // need to enable PVP at DoDamageAndTriggers?
        };
        std::vector<TargetInfo> m_UniqueTargetInfo;
        uint8 m_channelTargetEffectMask;                        // Mask req. alive targets

        struct GOTargetInfo : public TargetInfoBase
        {
            void DoTargetSpellHit(Spell* spell, uint8 effIndex) override;

            ObjectGuid TargetGUID;
            uint64 TimeDelay = 0ULL;
        };
        std::vector<GOTargetInfo> m_UniqueGOTargetInfo;

        struct ItemTargetInfo : public TargetInfoBase
        {
            void DoTargetSpellHit(Spell* spell, uint8 effIndex) override;

            Item* TargetItem = nullptr;
        };
        std::vector<ItemTargetInfo> m_UniqueItemInfo;
        
        template <class Container>
        void DoProcessTargetContainer(Container& targetContainer);

        SpellDestination m_destTargets[MAX_SPELL_EFFECTS];

        void AddUnitTarget(Unit* target, uint32 effectMask, bool checkIfValid = true, bool implicit = true);
        void AddGOTarget(GameObject* target, uint32 effectMask);
        void AddItemTarget(Item* item, uint32 effectMask);
        void AddDestTarget(SpellDestination const& dest, uint32 effIndex);

        SpellMissInfo PreprocessSpellHit(Unit* unit, bool scaleAura, TargetInfo& targetInfo);
        void DoSpellEffectHit(Unit* unit, uint8 effIndex, TargetInfo& targetInfo);

        void DoTriggersOnSpellHit(Unit* unit, uint8 effMask);
        bool UpdateChanneledTargetList();
        bool IsValidDeadOrAliveTarget(Unit const* target) const;
        void HandleLaunchPhase();
        void DoEffectOnLaunchTarget(TargetInfo& targetInfo, float multiplier, uint8 effIndex);

        void PrepareTargetProcessing();
        void FinishTargetProcessing();

        // spell execution log
        void InitEffectExecuteData(uint8 effIndex);
        void AssertEffectExecuteData() const;

        void LoadScripts();
        void CallScriptBeforeCastHandlers();
        void CallScriptOnCastHandlers();
        void CallScriptAfterCastHandlers();
        SpellCastResult CallScriptCheckCastHandlers();
        bool CallScriptEffectHandlers(SpellEffIndex effIndex, SpellEffectHandleMode mode, int32& damage);
        void CallScriptBeforeHitHandlers();
        void CallScriptOnHitHandlers();
        void CallScriptAfterHitHandlers();
        void CallScriptObjectAreaTargetSelectHandlers(std::list<WorldObject*>& targets, SpellEffIndex effIndex, SpellImplicitTargetInfo const& targetType);
        void CallScriptObjectTargetSelectHandlers(WorldObject*& target, SpellEffIndex effIndex, SpellImplicitTargetInfo const& targetType);
        void CallScriptDestinationTargetSelectHandlers(SpellDestination& target, SpellEffIndex effIndex, SpellImplicitTargetInfo const& targetType);
        bool CheckScriptEffectImplicitTargets(SpellEffIndex effIndex, SpellEffIndex effIndexToCheck);
        std::vector<SpellScript*> m_loadedScripts;
        // -------------------------------------------

        //List For Triggered Spells
        struct HitTriggerSpell
        {
            HitTriggerSpell(SpellInfo const* spellInfo, SpellInfo const* auraSpellInfo, int32 procChance) :
                triggeredSpell(spellInfo), triggeredByAura(auraSpellInfo), chance(procChance) { }

            SpellInfo const* triggeredSpell;
            SpellInfo const* triggeredByAura;
            // uint8 triggeredByEffIdx          This might be needed at a later stage - No need known for now
            int32 chance;
        };
        bool CanExecuteTriggersOnHit(uint8 effMask, SpellInfo const* triggeredByAura = nullptr) const;
        void PrepareTriggersExecutedOnHit();
        typedef std::vector<HitTriggerSpell> HitTriggerSpellList;
        HitTriggerSpellList m_hitTriggerSpells;

		// effect helpers
		void SummonGuardian(uint32 effectIndex, uint32 entry, SummonPropertiesEntry const* properties, uint32 numSummons);

        SpellCastResult CanOpenLock(uint32 effIndex, uint32 lockid, SkillType& skillid, int32& reqSkillValue, int32& skillValue);

        uint32 m_spellState;
        int32 m_timer; //remaining cast time
        SpellEvent* _spellEvent;

        float m_castPositionX;
        float m_castPositionY;
        float m_castPositionZ;
        float m_castOrientation;
        TriggerCastFlags _triggeredCastFlags;

        // if need this can be replaced by Aura copy
        // we can't store original aura link to prevent access to deleted auras
        // and in same time need aura data and after aura deleting.
        SpellInfo const* m_triggeredByAuraSpell;
        // used in effects handlers
        Unit* _unitCaster; //renamed from TC unitCaster to _unitCaster, until they fixes all their variables hiding this class member (couldn't fix the warnings and then update code from TC safely)
        UnitAura* _spellAura;
        DynObjAura* _dynObjAura;

        uint8 m_auraScaleMask;

        ByteBuffer* m_effectExecuteData[MAX_SPELL_EFFECTS];

        // sunwell:
        bool _spellTargetsSelected;

        PathGenerator* m_preGeneratedPath;
        // We need to keep this variable in spell to allow applying it for channels or for when cast finish
        SpellMissInfo _forceHitResult;
        uint32 GetChannelStartDuration() const;
};

namespace Trinity
{
    struct TC_GAME_API WorldObjectSpellTargetCheck
    {
    protected:
        WorldObject* _caster;
        WorldObject* _referer;
        SpellInfo const* _spellInfo;
        SpellTargetCheckTypes _targetSelectionType;
        ConditionSourceInfo* _condSrcInfo;
        ConditionContainer const* _condList;

        WorldObjectSpellTargetCheck(WorldObject* caster, WorldObject* referer, SpellInfo const* spellInfo,
            SpellTargetCheckTypes selectionType, ConditionContainer const* condList);
        ~WorldObjectSpellTargetCheck();

        bool operator()(WorldObject* target) const;
    };

    struct TC_GAME_API WorldObjectSpellNearbyTargetCheck : public WorldObjectSpellTargetCheck
    {
        float _range;
        Position const* _position;
        WorldObjectSpellNearbyTargetCheck(float range, WorldObject* caster, SpellInfo const* spellInfo,
            SpellTargetCheckTypes selectionType, ConditionContainer const* condList);

        bool operator()(WorldObject* target);
    };

    struct TC_GAME_API WorldObjectSpellAreaTargetCheck : public WorldObjectSpellTargetCheck
    {
        float _range;
        Position const* _position;
        WorldObjectSpellAreaTargetCheck(float range, Position const* position, WorldObject* caster,
            WorldObject* referer, SpellInfo const* spellInfo, SpellTargetCheckTypes selectionType, ConditionContainer const* condList);

        bool operator()(WorldObject* target) const;
    };

    struct TC_GAME_API WorldObjectSpellConeTargetCheck : public WorldObjectSpellAreaTargetCheck
    {
        float _coneAngle;
        WorldObjectSpellConeTargetCheck(float coneAngle, float range, WorldObject* caster,
            SpellInfo const* spellInfo, SpellTargetCheckTypes selectionType, ConditionContainer const* condList);

        bool operator()(WorldObject* target) const;
    };

    struct TC_GAME_API WorldObjectSpellTrajTargetCheck : public WorldObjectSpellTargetCheck
    {
        float _range;
        Position const* _position;
        WorldObjectSpellTrajTargetCheck(float range, Position const* position, WorldObject* caster,
            SpellInfo const* spellInfo, SpellTargetCheckTypes selectionType, ConditionContainer const* condList);

        bool operator()(WorldObject* target) const;
    };
}

typedef void(Spell::*SpellEffectHandlerFn)(uint32 i);

//update spell. Reschedule itself if necessary
class TC_GAME_API SpellEvent : public BasicEvent
{
    friend class TestCase; //allow testCase to affect the events list

    public:
        SpellEvent(Spell* spell);
        ~SpellEvent() override;

        bool Execute(uint64 e_time, uint32 p_time) override;
        void Abort(uint64 e_time) override;
        bool IsDeletable() const override;
    protected:
        Spell* m_Spell;
};

#endif

