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

#ifndef __SPELL_H
#define __SPELL_H

#include "GridDefines.h"
#include "PathGenerator.h"

class Unit;
class Player;
class GameObject;
class Aura;
enum SpellCastResult : int;
class SpellScript;

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
    uint64 _transportGUID;
    Position _transportOffset;
#endif
};

class SpellCastTargets
{
public:
    SpellCastTargets();
    ~SpellCastTargets();

    void Read(ByteBuffer& data, Unit* caster);
    void Write(ByteBuffer& data);

    uint32 GetTargetMask() const { return m_targetMask; }
    void SetTargetMask(uint32 newMask) { m_targetMask = newMask; }

    void SetTargetFlag(SpellCastTargetFlags flag) { m_targetMask |= flag; }

    uint64 GetUnitTargetGUID() const;
    Unit* GetUnitTarget() const;
    void SetUnitTarget(Unit* target);

    uint64 GetGOTargetGUID() const;
    GameObject* GetGOTarget() const;
    void SetGOTarget(GameObject* target);

    uint64 GetCorpseTargetGUID() const;
    Corpse* GetCorpseTarget() const;

    WorldObject* GetObjectTarget() const;
    uint64 GetObjectTargetGUID() const;
    void RemoveObjectTarget();

    uint64 GetItemTargetGUID() const { return m_itemTargetGUID; }
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

    // Xinef: Channel data
    void SetObjectTargetChannel(uint64 targetGUID);
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
    uint64 m_objectTargetGUID;
    uint64 m_itemTargetGUID;
    uint32 m_itemTargetEntry;

    SpellDestination m_src;
    SpellDestination m_dst;

    float m_elevation, m_speed;
    std::string m_strTarget;

    // Xinef: Save channel data
    SpellDestination m_dstChannel;
    uint64 m_objectTargetGUIDChannel;
};

struct SpellValue
{
    explicit SpellValue(SpellInfo const *proto)
    {
        for (uint32 i = 0; i < MAX_SPELL_EFFECTS; ++i)
            EffectBasePoints[i] = proto->Effects[i].BasePoints;

        RadiusMod = 1.0f;
        MaxAffectedTargets = proto->MaxAffectedTargets;
    }

    uint32 MaxAffectedTargets;
    int32 EffectBasePoints[MAX_SPELL_EFFECTS];
    float RadiusMod;
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


// special structure containing data for channel target spells
struct ChannelTargetData
{
    ChannelTargetData(uint64 cguid, const SpellDestination* dst) : channelGUID(cguid)
    {
        if (dst)
            spellDst = *dst;
    }

    uint64 channelGUID;
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

class Spell
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
        void EffectSummon(uint32 i);
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
        void EffectPlayerPull(uint32 i);
        void EffectDispelMechanic(uint32 i);
        void EffectSummonDeadPet(uint32 i);
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
        void EffectEnergisePct(uint32 i);
        void EffectTriggerSpellWithValue(uint32 i);
        void EffectTriggerRitualOfSummoning(uint32 i);
        void EffectKillCredit(uint32 i);
        void EffectQuestFail(uint32 i);
        void EffectRedirectThreat(uint32 i);
        void EffectPlaySound(uint32 i);
        void EffectPlayMusic(uint32 i);
        void EffectForceCastWithValue(uint32 i);

        Spell(Unit* Caster, SpellInfo const *info, bool triggered, uint64 originalCasterGUID = 0, Spell** triggeringContainer = NULL, bool skipCheck = false);
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

        uint32 GetSearcherTypeMask(SpellTargetObjectTypes objType, ConditionList* condList);
        template<class SEARCHER> void SearchTargets(SEARCHER& searcher, uint32 containerMask, Unit* referer, Position const* pos, float radius);

        WorldObject* SearchNearbyTarget(float range, SpellTargetObjectTypes objectType, SpellTargetCheckTypes selectionType, ConditionList* condList = NULL);
        void SearchAreaTargets(std::list<WorldObject*>& targets, float range, Position const* position, Unit* referer, SpellTargetObjectTypes objectType, SpellTargetCheckTypes selectionType, ConditionList* condList);
        void SearchChainTargets(std::list<WorldObject*>& targets, uint32 chainTargets, WorldObject* target, SpellTargetObjectTypes objectType, SpellTargetCheckTypes selectType, SpellTargetSelectionCategories selectCategory, ConditionList* condList, bool isChainHeal);

        //return SpellCastResult
        uint32 prepare(SpellCastTargets const* targets, Aura* triggeredByAura = NULL);
        void cancel();
        void update(uint32 difftime);
        void cast(bool skipCheck = false);
        void finish(bool ok = true, bool cancelChannel = true);
        void TakePower();
        void TakeAmmo();
        void TakeReagents();
        void TakeCastItem();
        void TriggerSpell();
        SpellCastResult CheckCast(bool strict);
        SpellCastResult PetCanCast(Unit* target);
        bool CanAutoCast(Unit* target);

        // handlers
        void handle_immediate();
        uint64 handle_delayed(uint64 t_offset);
        // handler helpers
        void _handle_immediate_phase();
        void _handle_finish_phase();

        SpellCastResult CheckItems();
        SpellCastResult CheckRange(bool strict);
        SpellCastResult CheckPower();
        SpellCastResult CheckCasterAuras() const;

        int32 CalculateDamage(uint8 i, Unit* target) { return m_caster->CalculateSpellDamage(m_spellInfo,i,m_currentBasePoints[i],target); }
        int32 CalculatePowerCost();

        bool HaveTargetsForEffect(uint8 effect) const;
        void Delayed();
        void DelayedChannel();
        inline uint32 getState() const { return m_spellState; }
        void setState(uint32 state) { m_spellState = state; }

        void DoCreateItem(uint32 i, uint32 itemtype);

        void WriteSpellGoTargets( WorldPacket * data );
        void WriteAmmoToPacket( WorldPacket * data );

        Unit* SelectMagnetTarget();
        void HandleHitTriggerAura();
        bool CheckTarget(Unit* target, uint32 eff);

        bool CheckEffectTarget(Unit const* target, uint32 eff) const;
        void CheckSrc() { if(!m_targets.HasSrc()) m_targets.SetSrc(m_caster); }
        void CheckDst() { if(!m_targets.HasDst()) m_targets.SetDst(m_caster); }

        void SendCastResult(SpellCastResult result);
        void SendSpellStart();
        void SendSpellGo();
        void SendSpellCooldown();
        void SendLogExecute();
        void SendInterrupted(uint8 result);
        void SendChannelUpdate(uint32 time);
        void SendChannelUpdate(uint32 time, uint32 spellId); //only update if channeling given spell
        void SendChannelStart(uint32 duration);
        void SendResurrectRequest(Player* target);
        void SendPlaySpellVisual(uint32 SpellID);

        void HandleEffects(Unit *pUnitTarget,Item *pItemTarget,GameObject *pGOTarget,uint32 i, SpellEffectHandleMode mode);
        void HandleFlatThreat();
        //void HandleAddAura(Unit* Target);

        const SpellInfo* const m_spellInfo;
        int32 m_currentBasePoints[3];                       // cache SpellInfo::EffectBasePoints and use for set custom base points
        Item* m_CastItem;
        uint64 m_castItemGUID;
        uint8 m_cast_count;
        SpellCastTargets m_targets;
        bool m_skipHitCheck;

        int32 GetCastTime() const { return m_casttime; }
        bool IsAutoRepeat() const { return m_autoRepeat; }
        void SetAutoRepeat(bool rep) { m_autoRepeat = rep; }
        void ReSetTimer() { m_timer = m_casttime > 0 ? m_casttime : 0; }
        bool IsNextMeleeSwingSpell() const
        {
            return m_spellInfo->Attributes & (SPELL_ATTR0_ON_NEXT_SWING_1|SPELL_ATTR0_ON_NEXT_SWING_2);
        }
        static bool IsNextMeleeSwingSpell(SpellInfo const* spellInfo)
        {
            return spellInfo && spellInfo->Attributes & (SPELL_ATTR0_ON_NEXT_SWING_1|SPELL_ATTR0_ON_NEXT_SWING_2);
        }
        bool IsTriggered() const { return /*TC spells _triggeredCastFlags & TRIGGERED_FULL_MASK; */ m_IsTriggeredSpell; };
        bool IsChannelActive() const { return m_caster->GetUInt32Value(UNIT_CHANNEL_SPELL) != 0; }
        bool IsAutoActionResetSpell() const { return !m_IsTriggeredSpell && (m_spellInfo->InterruptFlags & SPELL_INTERRUPT_FLAG_AUTOATTACK); }

        bool IsDeletable() const { return !m_referencedFromCurrentSpell && !m_executedCurrently; }
        void SetReferencedFromCurrent(bool yes) { m_referencedFromCurrentSpell = yes; }
        bool IsInterruptable() const { return !m_executedCurrently; }
        void SetExecutedCurrently(bool yes) { m_executedCurrently = yes; }
        uint64 GetDelayStart() const { return m_delayStart; }
        void SetDelayStart(uint64 m_time) { m_delayStart = m_time; }
        uint64 GetDelayMoment() const { return m_delayMoment; }
        uint64 GetDelayTrajectory() const { return m_delayTrajectory; }

        bool IsNeedSendToClient() const;

        CurrentSpellTypes GetCurrentContainer();

        Unit* GetCaster() const { return m_caster; }
        Unit* GetOriginalCaster() const { return m_originalCaster; }
        int32 GetPowerCost() const { return m_powerCost; }
        SpellInfo const* GetSpellInfo() const { return m_spellInfo; }

        bool UpdatePointers();                              // must be used at call Spell code after time delay (non triggered spell cast/update spell call/etc)

        bool IsAffectedBy(SpellInfo const *spellInfo, uint32 effectId);

        bool CheckTargetCreatureType(Unit* target) const;

        void AddTriggeredSpell(SpellInfo const* spell) { m_TriggerSpells.push_back(spell); }

        void CleanupTargetList();

        void SetSpellValue(SpellValueMod mod, int32 value);
        
        void setRemoveReflect() { m_removeReflect = true; }

        bool DoesApplyAuraName(uint32 name);

        void LoadScripts();

    protected:
        bool HasGlobalCooldown();
        void TriggerGlobalCooldown();
        void CancelGlobalCooldown();

        void SendLoot(uint64 guid, LootType loottype);

        Unit* const m_caster;

        SpellValue* const m_spellValue;

        uint64 m_originalCasterGUID;                        // real source of cast (aura caster/etc), used for spell targets selection
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
        uint64 m_delayTrajectory;							// Xinef: Trajectory delay
        bool m_immediateHandled;                            // were immediate actions handled? (used by delayed spells only)

        // These vars are used in both delayed spell system and modified immediate spell system
        bool m_referencedFromCurrentSpell;                  // mark as references to prevent deleted and access by dead pointers
        bool m_executedCurrently;                           // mark as executed to prevent deleted and access by dead pointers
        bool m_needSpellLog;                                // need to send spell log?
        uint8 m_applyMultiplierMask;                        // by effect: damage multiplier needed?

        float m_damageMultipliers[MAX_SPELL_EFFECTS];                       // by effect: damage multiplier

        // Current targets, to be used in SpellEffects (MUST BE USED ONLY IN SPELL EFFECTS)
        Unit* unitTarget;
        Item* itemTarget;
        GameObject* gameObjTarget;
        WorldLocation* destTarget;

        int32 damage;
        SpellEffectHandleMode effectHandleMode;

        // this is set in Spell Hit, but used in Apply Aura handler
        DiminishingLevels m_diminishLevel;
        DiminishingGroup m_diminishGroup;

        // -------------------------------------------
        GameObject* focusObject;

        // Damage and healing in effects need just calculate
        int32 m_damage;           // Damge   in effects count here
        int32 m_healing;          // Healing in effects count here
        int32 m_healthLeech;      // Health leech in effects for all targets count here

        //******************************************
        // Spell trigger system
        //******************************************
        bool   m_canTrigger;                  // Can start trigger (m_IsTriggeredSpell can`t use for this)
        void   prepareDataForTriggerSystem();
        void   prepareHitProcData(uint32& procAttacker, uint32& procVictim, bool hostileTarget);
        bool   m_removeReflect;

        //*****************************************
        // Spell target subsystem
        //*****************************************
        // Targets store structures and data
        struct TargetInfo
        {
            uint64 targetGUID;
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
            uint64 targetGUID;
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
        void DoSpellHitOnUnit(Unit *unit, uint32 effectMask);
        void DoAllEffectOnTarget(GOTargetInfo *target);
        void DoAllEffectOnTarget(ItemTargetInfo *target);
        bool UpdateChanneledTargetList();
        bool IsValidDeadOrAliveTarget(Unit const* target) const;
        void HandleLaunchPhase();
        void DoAllEffectOnLaunchTarget(TargetInfo& targetInfo, float* multiplier, bool firstTarget);

        void PrepareTargetProcessing();
        void FinishTargetProcessing();

        bool _scriptsLoaded;
        void CallScriptBeforeCastHandlers();
        void CallScriptOnCastHandlers();
        void CallScriptAfterCastHandlers();
        SpellCastResult CallScriptCheckCastHandlers();
        void PrepareScriptHitHandlers();
        bool CallScriptEffectHandlers(SpellEffIndex effIndex, SpellEffectHandleMode mode);
        void CallScriptBeforeHitHandlers();
        void CallScriptOnHitHandlers();
        void CallScriptAfterHitHandlers();
        void CallScriptObjectAreaTargetSelectHandlers(std::list<WorldObject*>& targets, SpellEffIndex effIndex, SpellImplicitTargetInfo const& targetType);
        void CallScriptObjectTargetSelectHandlers(WorldObject*& target, SpellEffIndex effIndex, SpellImplicitTargetInfo const& targetType);
        void CallScriptDestinationTargetSelectHandlers(SpellDestination& target, SpellEffIndex effIndex, SpellImplicitTargetInfo const& targetType);
        bool CheckScriptEffectImplicitTargets(uint32 effIndex, uint32 effIndexToCheck);
        std::list<SpellScript*> m_loadedScripts;
        // -------------------------------------------

        //List For Triggered Spells
        typedef std::vector<SpellInfo const*> TriggerSpells;
        TriggerSpells m_TriggerSpells;
        typedef std::vector< std::pair<SpellInfo const*, int32> > ChanceTriggerSpells;
        ChanceTriggerSpells m_ChanceTriggerSpells;

        uint32 m_spellState;
        uint32 m_timer;

        float m_castPositionX;
        float m_castPositionY;
        float m_castPositionZ;
        float m_castOrientation;
        bool m_IsTriggeredSpell;

        // if need this can be replaced by Aura copy
        // we can't store original aura link to prevent access to deleted auras
        // and in same time need aura data and after aura deleting.
        SpellInfo const* m_triggeredByAuraSpell;

        uint8 m_auraScaleMask;

        // xinef:
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

                        if(i_spell.m_spellInfo->HasAttribute(SPELL_ATTR_CU_AOE_CANT_TARGET_SELF) && i_caster == itr->GetSource())
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
                            if(i_caster->HasInLine(itr->GetSource(), i_caster->GetObjectSize()))
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
    struct WorldObjectSpellTargetCheck
    {
        Unit* _caster;
        Unit* _referer;
        SpellInfo const* _spellInfo;
        SpellTargetCheckTypes _targetSelectionType;
        ConditionSourceInfo* _condSrcInfo;
        ConditionList* _condList;

        WorldObjectSpellTargetCheck(Unit* caster, Unit* referer, SpellInfo const* spellInfo,
            SpellTargetCheckTypes selectionType, ConditionList* condList);
        ~WorldObjectSpellTargetCheck();
        bool operator()(WorldObject* target);
    };

    struct WorldObjectSpellNearbyTargetCheck : public WorldObjectSpellTargetCheck
    {
        float _range;
        Position const* _position;
        WorldObjectSpellNearbyTargetCheck(float range, Unit* caster, SpellInfo const* spellInfo,
            SpellTargetCheckTypes selectionType, ConditionList* condList);
        bool operator()(WorldObject* target);
    };

    struct WorldObjectSpellAreaTargetCheck : public WorldObjectSpellTargetCheck
    {
        float _range;
        Position const* _position;
        WorldObjectSpellAreaTargetCheck(float range, Position const* position, Unit* caster,
            Unit* referer, SpellInfo const* spellInfo, SpellTargetCheckTypes selectionType, ConditionList* condList);
        bool operator()(WorldObject* target);
    };

    struct WorldObjectSpellConeTargetCheck : public WorldObjectSpellAreaTargetCheck
    {
        float _coneAngle;
        WorldObjectSpellConeTargetCheck(float coneAngle, float range, Unit* caster,
            SpellInfo const* spellInfo, SpellTargetCheckTypes selectionType, ConditionList* condList);
        bool operator()(WorldObject* target);
    };

    struct WorldObjectSpellTrajTargetCheck : public WorldObjectSpellAreaTargetCheck
    {
        WorldObjectSpellTrajTargetCheck(float range, Position const* position, Unit* caster,
            SpellInfo const* spellInfo, SpellTargetCheckTypes selectionType, ConditionList* condList);
        bool operator()(WorldObject* target);
    };
}

typedef void(Spell::*pEffect)(uint32 i);

//update spell. Reschedule itself if necessary
class SpellEvent : public BasicEvent
{
    public:
        SpellEvent(Spell* spell);
        virtual ~SpellEvent();

        virtual bool Execute(uint64 e_time, uint32 p_time);
        virtual void Abort(uint64 e_time);
        virtual bool IsDeletable() const;
    protected:
        Spell* m_Spell;
};

//procs for reflects + remove reflect
class ReflectEvent : public BasicEvent
{
public:
    ReflectEvent(uint64 casterGUID, uint64 targetGUID, const SpellInfo* spellInfo) : _casterGUID(casterGUID), _targetGUID(targetGUID), _spellInfo(spellInfo) { }
    virtual bool Execute(uint64 e_time, uint32 p_time);

protected:
    uint64 _casterGUID;
    uint64 _targetGUID;
    const SpellInfo* _spellInfo;
};


#endif

