
#ifndef __SPELL_H
#define __SPELL_H

#include "GridDefines.h"
#include "PathGenerator.h"
#include "Position.h"

class Unit;
class Player;
class GameObject;
class Aura;
enum SpellCastResult : int;
class SpellScript;
class SpellEvent;
enum LootType : uint8;

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

namespace Trinity
{
    struct SpellNotifierCreatureAndPlayer;
}

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
    void Write(ByteBuffer& data);

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

    bool HasSrc() const { return GetTargetMask() & TARGET_FLAG_SOURCE_LOCATION; }
    bool HasDst() const { return GetTargetMask() & TARGET_FLAG_DEST_LOCATION; }
    bool HasTraj() const { return m_speed != 0; }

    float GetElevation() const { return m_elevation; }
    void SetElevation(float elevation) { m_elevation = elevation; }
    float GetSpeed() const { return m_speed; }
    void SetSpeed(float speed) { m_speed = speed; }

    float GetDist2d() const { return m_src._position.GetExactDist2d(&m_dst._position); }
    float GetSpeedXY() const { return m_speed * cos(m_elevation); }
    float GetSpeedZ() const { return m_speed * sin(m_elevation); }

    void Update(Unit* caster);
    void OutDebug() const;

    // sunwell: Channel data
    void SetObjectTargetChannel(ObjectGuid targetGUID);
    void SetDstChannel(SpellDestination const& spellDest);
    WorldObject* GetObjectTargetChannel(Unit* caster) const;
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
    friend struct Trinity::SpellNotifierCreatureAndPlayer;
    friend class SpellScript;
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
        void EffectApplyAreaAura(uint32 i);
        void EffectSummonType(uint32 i);
        //void EffectSummon(uint32 i);
        void EffectLearnSpell(uint32 i);
        void EffectDispel(uint32 i);
        void EffectDualWield(uint32 i);
        void EffectPickPocket(uint32 i);
        void EffectAddFarsight(uint32 i);
        void EffectSummonPossessed(uint32 i);
        void EffectSummonWild(uint32 i);
        void EffectSummonGuardian(uint32 i);
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
        void EffectSummonTotem(uint32 i);
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
        void EffectSummonCritter(uint32 i);
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
        void EffectSummonDemon(uint32 i);
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

        Spell(Unit* Caster, SpellInfo const *info, TriggerCastFlags triggerFlags, ObjectGuid originalCasterGUID = ObjectGuid::Empty, Spell** triggeringContainer = nullptr, bool skipCheck = false);
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
        template<class SEARCHER> void SearchTargets(SEARCHER& searcher, uint32 containerMask, Unit* referer, Position const* pos, float radius);

        WorldObject* SearchNearbyTarget(float range, SpellTargetObjectTypes objectType, SpellTargetCheckTypes selectionType, ConditionContainer* condList = nullptr);
        void SearchAreaTargets(std::list<WorldObject*>& targets, float range, Position const* position, Unit* referer, SpellTargetObjectTypes objectType, SpellTargetCheckTypes selectionType, ConditionContainer* condList);
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

        int32 CalculateDamage(uint8 i, Unit* target) { return m_caster->CalculateSpellDamage(nullptr, m_spellInfo,i, &m_currentBasePoints[i]); }

        bool HaveTargetsForEffect(uint8 effect) const;
        void Delayed();
        void DelayedChannel();
        inline uint32 getState() const { return m_spellState; }
        void setState(uint32 state) { m_spellState = state; }

        void DoCreateItem(uint32 i, uint32 itemtype);

        void WriteSpellGoTargets( WorldPacket * data );
        void WriteAmmoToPacket( WorldPacket * data );

        bool CheckTarget(Unit* target, uint32 eff);

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
        void SendResurrectRequest(Player* target);

        void HandleEffects(Unit *pUnitTarget,Item *pItemTarget,GameObject *pGOTarget,uint32 i, SpellEffectHandleMode mode);
        void HandleFlatThreat();
        //void HandleAddAura(Unit* Target);

        const SpellInfo* const m_spellInfo;
        int32 m_currentBasePoints[3];                       // cache SpellInfo::EffectBasePoints and use for set custom base points
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
        bool IsChannelActive() const { return m_caster->GetUInt32Value(UNIT_CHANNEL_SPELL) != 0; }
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

        Unit* GetCaster() const { return m_caster; }
        Unit* GetOriginalCaster() const { return m_originalCaster; }
        SpellInfo const* GetSpellInfo() const { return m_spellInfo; }
        int32 GetPowerCost() const { return m_powerCost; }

        bool UpdatePointers();                              // must be used at call Spell code after time delay (non triggered spell cast/update spell call/etc)

        bool CheckTargetCreatureType(Unit* target) const;

        void CleanupTargetList();

        void SetSpellValue(SpellValueMod mod, int32 value);
        
        bool DoesApplyAuraName(uint32 name);
        
    protected:
        bool HasGlobalCooldown();
        void TriggerGlobalCooldown();
        void CancelGlobalCooldown();

        void SendLoot(ObjectGuid guid, LootType loottype);
        std::pair<float, float> GetMinMaxRange(bool strict);

        Unit* const m_caster;

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
        // this is set in Spell Hit, but used in Apply Aura handler
        DiminishingLevels m_diminishLevel;
        
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
        struct TargetInfo
        {
            ObjectGuid targetGUID;
            uint64 timeDelay;
            SpellMissInfo missCondition:8;
            SpellMissInfo reflectResult:8;
            uint8  effectMask:8;
            bool   processed:1;
            bool   alive:1;
            bool   crit:1;
            bool   scaleAura:1;
            int32  damage;
        };
        std::list<TargetInfo> m_UniqueTargetInfo;
        uint8 m_channelTargetEffectMask;                        // Mask req. alive targets

        struct GOTargetInfo
        {
            ObjectGuid targetGUID;
            uint64 timeDelay;
            uint8  effectMask:8;
            bool   processed:1;
        };
        std::list<GOTargetInfo> m_UniqueGOTargetInfo;

        struct ItemTargetInfo
        {
            Item  *item;
            uint8 effectMask;
        };
        std::list<ItemTargetInfo> m_UniqueItemInfo;
        
        SpellDestination m_destTargets[MAX_SPELL_EFFECTS];

        void AddUnitTarget(Unit* target, uint32 effectMask, bool checkIfValid = true, bool implicit = true);
        void AddGOTarget(GameObject* target, uint32 effectMask);
        void AddItemTarget(Item* item, uint32 effectMask);
        void AddDestTarget(SpellDestination const& dest, uint32 effIndex);

        void DoAllEffectOnTarget(TargetInfo *target);
        SpellMissInfo DoSpellHitOnUnit(Unit *unit, uint32 effectMask);
        void DoAllEffectOnTarget(GOTargetInfo *target);
        void DoAllEffectOnTarget(ItemTargetInfo *target);
        void DoTriggersOnSpellHit(Unit* unit, uint8 effMask);
        bool UpdateChanneledTargetList();
        bool IsValidDeadOrAliveTarget(Unit const* target) const;
        void HandleLaunchPhase();
        void DoAllEffectOnLaunchTarget(TargetInfo& targetInfo, float* multiplier, bool firstTarget);

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
        void PrepareScriptHitHandlers();
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

        uint32 m_spellState;
        uint32 m_timer;
        SpellEvent* _spellEvent;

        float m_castPositionX;
        float m_castPositionY;
        float m_castPositionZ;
        float m_castOrientation;
        TriggerCastFlags _triggeredCastFlags;;

        // if need this can be replaced by Aura copy
        // we can't store original aura link to prevent access to deleted auras
        // and in same time need aura data and after aura deleting.
        SpellInfo const* m_triggeredByAuraSpell;
        // used in effects handlers
        Aura* m_spellAura;

        uint8 m_auraScaleMask;

        ByteBuffer* m_effectExecuteData[MAX_SPELL_EFFECTS];

        // sunwell:
        bool _spellTargetsSelected;

        PathGenerator* m_preGeneratedPath;
};

namespace Trinity
{
    struct SpellNotifierCreatureAndPlayer
    {
        std::list<Unit*> *i_data;
        Spell &i_spell;
        const uint32& i_push_type;
        float i_radius, i_radiusSq;
        SpellTargets i_TargetType;
        Unit* i_caster;
        uint32 i_entry;
        float i_x, i_y, i_z;

        SpellNotifierCreatureAndPlayer(Spell &spell, std::list<Unit*> &data, float radius, const uint32 &type,
            SpellTargets TargetType = SPELL_TARGETS_ENEMY, uint32 entry = 0, float x = 0, float y = 0, float z = 0)
            : i_data(&data), i_spell(spell), i_push_type(type), i_radius(radius), i_radiusSq(radius*radius)
            , i_TargetType(TargetType), i_entry(entry), i_x(x), i_y(y), i_z(z)
        {
            i_caster = spell.GetCaster();
        }

        template<class T> inline void Visit(GridRefManager<T>  &m)
        {
            assert(i_data);

            if(!i_caster)
                return;

            for(typename GridRefManager<T>::iterator itr = m.begin(); itr != m.end(); ++itr)
            {
                if(!itr->GetSource()->IsAlive())
                    continue;

                if (itr->GetSource()->GetTypeId() == TYPEID_PLAYER)
                {
                    if ((itr->GetSource()->ToPlayer())->IsInFlight())
                        continue;

                    if ((itr->GetSource()->ToPlayer())->isSpectator())
                        continue;
                } else {
                    if(i_spell.m_spellInfo->HasAttribute(SPELL_ATTR3_ONLY_TARGET_PLAYERS))
                        continue;
                }

                switch (i_TargetType)
                {
                    case SPELL_TARGETS_ALLY:
                        if(!itr->GetSource()->IsAttackableByAOE())
                            continue;

                        if(!i_caster->IsFriendlyTo( itr->GetSource()))
                            continue;

                        if(i_spell.m_spellInfo->HasAttribute(SPELL_ATTR0_CU_AOE_CANT_TARGET_SELF) && i_caster == itr->GetSource())
                            continue;

                        break;
                    case SPELL_TARGETS_ENEMY:
                    {
                        if(!itr->GetSource()->IsAttackableByAOE())
                            continue;

                        Unit* check = i_caster->GetCharmerOrOwnerOrSelf();

                        if( check->GetTypeId()==TYPEID_PLAYER )
                        {
                            if (check->IsFriendlyTo( itr->GetSource() ))
                                continue;
                        }
                        else
                        {
                            if (!check->IsHostileTo( itr->GetSource() ))
                                continue;
                        }
                        break;
                    }
                    case SPELL_TARGETS_ENTRY:
                    {
                        if(itr->GetSource()->GetEntry()!= i_entry)
                            continue;
                        break;
                    }
                    default:
                        continue;
                }

                switch(i_push_type)
                {
                    case PUSH_IN_FRONT:
                        if(i_caster->IsWithinDistInMap( itr->GetSource(), i_radius))
                        {
                            if(i_caster->isInFront((Unit*)(itr->GetSource()), M_PI/3 ))
                                i_data->push_back(itr->GetSource());
                        }
                        break;
                    case PUSH_IN_BACK:
                        if(i_caster->IsWithinDistInMap( itr->GetSource(), i_radius))
                        {
                            if(i_caster->isInBack((Unit*)(itr->GetSource()), M_PI/3 ))
                                i_data->push_back(itr->GetSource());
                        }
                        break;
                    case PUSH_IN_LINE:
                        if(i_caster->IsWithinDistInMap( itr->GetSource(), i_radius))
                        {
                            if(i_caster->HasInLine(itr->GetSource(), itr->GetSource()->GetCombatReach(), i_caster->GetCombatReach()))
                                i_data->push_back(itr->GetSource());
                        }
                        break;
                    case PUSH_IN_FRONT_180:
                        if(i_caster->IsWithinDistInMap( itr->GetSource(), i_radius))
                        {
                            if(i_caster->isInFront((Unit*)(itr->GetSource()), M_PI ))
                                i_data->push_back(itr->GetSource());
                        }
                        break;
                    default:
                        if(i_TargetType != SPELL_TARGETS_ENTRY && i_push_type == PUSH_SRC_CENTER && i_caster) // if caster then check distance from caster to target (because of model collision)
                        {
                            if(i_caster->IsWithinDistInMap( itr->GetSource(), i_radius, true) )
                                i_data->push_back(itr->GetSource());
                        }
                        else
                        {
                            if((itr->GetSource()->GetDistanceSq(i_x, i_y, i_z) < i_radiusSq))
                                i_data->push_back(itr->GetSource());
                        }
                        break;
                }
            }
        }

        #ifdef WIN32
        template<> inline void Visit(CorpseMapType & ) {}
        template<> inline void Visit(GameObjectMapType & ) {}
        template<> inline void Visit(DynamicObjectMapType & ) {}
        #endif
    };

    #ifndef WIN32
    template<> inline void SpellNotifierCreatureAndPlayer::Visit(CorpseMapType& ) {}
    template<> inline void SpellNotifierCreatureAndPlayer::Visit(GameObjectMapType& ) {}
    template<> inline void SpellNotifierCreatureAndPlayer::Visit(DynamicObjectMapType& ) {}
    #endif
}

namespace Trinity
{
    struct TC_GAME_API WorldObjectSpellTargetCheck
    {
        Unit* _caster;
        Unit* _referer;
        SpellInfo const* _spellInfo;
        SpellTargetCheckTypes _targetSelectionType;
        ConditionSourceInfo* _condSrcInfo;
        ConditionContainer* _condList;

        WorldObjectSpellTargetCheck(Unit* caster, Unit* referer, SpellInfo const* spellInfo,
            SpellTargetCheckTypes selectionType, ConditionContainer* condList);
        ~WorldObjectSpellTargetCheck();
        bool operator()(WorldObject* target);
    };

    struct TC_GAME_API WorldObjectSpellNearbyTargetCheck : public WorldObjectSpellTargetCheck
    {
        float _range;
        Position const* _position;
        WorldObjectSpellNearbyTargetCheck(float range, Unit* caster, SpellInfo const* spellInfo,
            SpellTargetCheckTypes selectionType, ConditionContainer* condList);
        bool operator()(WorldObject* target);
    };

    struct TC_GAME_API WorldObjectSpellAreaTargetCheck : public WorldObjectSpellTargetCheck
    {
        float _range;
        Position const* _position;
        WorldObjectSpellAreaTargetCheck(float range, Position const* position, Unit* caster,
            Unit* referer, SpellInfo const* spellInfo, SpellTargetCheckTypes selectionType, ConditionContainer* condList);
        bool operator()(WorldObject* target);
    };

    struct TC_GAME_API WorldObjectSpellConeTargetCheck : public WorldObjectSpellAreaTargetCheck
    {
        float _coneAngle;
        WorldObjectSpellConeTargetCheck(float coneAngle, float range, Unit* caster,
            SpellInfo const* spellInfo, SpellTargetCheckTypes selectionType, ConditionContainer* condList);
        bool operator()(WorldObject* target);
    };

    struct TC_GAME_API WorldObjectSpellTrajTargetCheck : public WorldObjectSpellTargetCheck
    {
        float _range;
        Position const* _position;
        WorldObjectSpellTrajTargetCheck(float range, Position const* position, Unit* caster,
            SpellInfo const* spellInfo, SpellTargetCheckTypes selectionType, ConditionContainer* condList);
        bool operator()(WorldObject* target);
    };
}

typedef void(Spell::*pEffect)(uint32 i);

//update spell. Reschedule itself if necessary
class TC_GAME_API SpellEvent : public BasicEvent
{
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

