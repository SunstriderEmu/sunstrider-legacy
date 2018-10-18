#ifndef _SPELLINFO_H
#define _SPELLINFO_H

#include "SharedDefines.h"
#include "DBCStructure.h"

#include <boost/container/flat_set.hpp>

enum AuraType : unsigned int;
enum SpellCastResult : int;
class SpellInfo;
class Spell;
struct Condition;
class Unit;
class WorldObject;
class Item;
struct SpellChainNode;
struct SpellModifier;
class Player;
struct SpellDurationEntry;
struct SpellRangeEntry;
struct SpellRadiusEntry;
struct SpellCastTimesEntry;

enum SpellCustomAttributes
{
    SPELL_ATTR0_CU_ENCHANT_PROC                  = 0x00000001,
    SPELL_ATTR0_CU_CONE_BACK                     = 0x00000002,
    SPELL_ATTR0_CU_CONE_LINE                     = 0x00000004,
    SPELL_ATTR0_CU_SHARE_DAMAGE                  = 0x00000008,
    SPELL_ATTR0_CU_AURA_HOT                      = 0x00000010,
    SPELL_ATTR0_CU_AURA_DOT                      = 0x00000020,
    SPELL_ATTR0_CU_AURA_CC                       = 0x00000040,
    SPELL_ATTR0_CU_CAN_CRIT                      = 0x00000080,
    SPELL_ATTR0_CU_DIRECT_DAMAGE                 = 0x00000100,
    SPELL_ATTR0_CU_PICKPOCKET                    = 0x00000200,
    SPELL_ATTR0_CU_LINK_CAST                     = 0x00000400,
    SPELL_ATTR0_CU_LINK_HIT                      = 0x00000800,
    SPELL_ATTR0_CU_LINK_AURA                     = 0x00001000,
    SPELL_ATTR0_CU_LINK_REMOVE                   = 0x00002000,
    SPELL_ATTR0_CU_ROLLING_PERIODIC              = 0x00004000,
    SPELL_ATTR0_CU_IGNORE_ARMOR                  = 0x00008000,
    SPELL_ATTR0_CU_SAME_STACK_DIFF_CASTERS       = 0x00010000,
    //reuse                                      = 0x00020000,
    SPELL_ATTR0_CU_THREAT_GOES_TO_CURRENT_CASTER = 0x00040000,     // Instead of original caster
    SPELL_ATTR0_CU_NEEDS_AMMO_DATA               = 0x00080000,
    //reuse                                      = 0x00100000,
    SPELL_ATTR0_CU_REMOVE_ON_INSTANCE_ENTER      = 0x00200000,     // Auras removed when target enters an instance
    SPELL_ATTR0_CU_AOE_CANT_TARGET_SELF          = 0x00400000,
    SPELL_ATTR0_CU_CONE_180                      = 0x00800000,
    SPELL_ATTR0_CU_BINARY_SPELL                  = 0x01000000,

    SPELL_ATTR0_CU_NEGATIVE_EFF0                 = 0x02000000,
    SPELL_ATTR0_CU_NEGATIVE_EFF1                 = 0x04000000,
    SPELL_ATTR0_CU_NEGATIVE_EFF2                 = 0x08000000,

    SPELL_ATTR0_CU_NEGATIVE                      = SPELL_ATTR0_CU_NEGATIVE_EFF0 | SPELL_ATTR0_CU_NEGATIVE_EFF1 | SPELL_ATTR0_CU_NEGATIVE_EFF2
};

enum SpellCastTargetFlags : uint32
{
    TARGET_FLAG_NONE            = 0x00000000,
    TARGET_FLAG_UNUSED_1        = 0x00000001,               // not used
    TARGET_FLAG_UNIT            = 0x00000002,               // pguid
    TARGET_FLAG_UNIT_RAID       = 0x00000004,               // not sent, used to validate target (if raid member)
    TARGET_FLAG_UNIT_PARTY      = 0x00000008,               // not sent, used to validate target (if party member)
    TARGET_FLAG_ITEM            = 0x00000010,               // pguid
    TARGET_FLAG_SOURCE_LOCATION = 0x00000020,               // 3 float
    TARGET_FLAG_DEST_LOCATION   = 0x00000040,               // 3 float
    TARGET_FLAG_UNIT_ENEMY      = 0x00000080,               // not sent, used to validate target (if enemy)
    TARGET_FLAG_UNIT_ALLY       = 0x00000100,               // not sent, used to validate target (if ally)
    TARGET_FLAG_CORPSE_ENEMY    = 0x00000200,               // pguid
    TARGET_FLAG_UNIT_DEAD       = 0x00000400,               // not sent, used to validate target (if dead creature)
    TARGET_FLAG_GAMEOBJECT      = 0x00000800,               // pguid
    TARGET_FLAG_TRADE_ITEM      = 0x00001000,               // pguid
    TARGET_FLAG_STRING          = 0x00002000,               // string
    TARGET_FLAG_GAMEOBJECT_ITEM = 0x00004000,               // not sent, used with TARGET_GAMEOBJECT_ITEM_TARGET
    TARGET_FLAG_CORPSE_ALLY     = 0x00008000,               // pguid
    TARGET_FLAG_UNIT_MINIPET    = 0x00010000,                // pguid, used to validate target (if non combat pet)
#ifdef LICH_KING
    TARGET_FLAG_GLYPH_SLOT      = 0x00020000,               // used in glyph spells
    TARGET_FLAG_DEST_TARGET     = 0x00040000,               // sometimes appears with DEST_TARGET spells (may appear or not for a given spell)
    TARGET_FLAG_UNUSED20        = 0x00080000,               // uint32 counter, loop { vec3 - screen position (?), guid }, not used so far
    TARGET_FLAG_UNIT_PASSENGER  = 0x00100000,               // guessed, used to validate target (if vehicle passenger)
#endif

    TARGET_FLAG_UNIT_MASK = TARGET_FLAG_UNIT | TARGET_FLAG_UNIT_RAID | TARGET_FLAG_UNIT_PARTY
        | TARGET_FLAG_UNIT_ENEMY | TARGET_FLAG_UNIT_ALLY | TARGET_FLAG_UNIT_DEAD | TARGET_FLAG_UNIT_MINIPET 
#ifdef LICH_KING
        | TARGET_FLAG_UNIT_PASSENGER
#endif
    ,
    TARGET_FLAG_GAMEOBJECT_MASK = TARGET_FLAG_GAMEOBJECT | TARGET_FLAG_GAMEOBJECT_ITEM,
    TARGET_FLAG_CORPSE_MASK = TARGET_FLAG_CORPSE_ALLY | TARGET_FLAG_CORPSE_ENEMY,
    TARGET_FLAG_ITEM_MASK = TARGET_FLAG_TRADE_ITEM | TARGET_FLAG_ITEM | TARGET_FLAG_GAMEOBJECT_ITEM
};

enum SpellTargetSelectionCategories : uint8
{
    TARGET_SELECT_CATEGORY_NYI,
    TARGET_SELECT_CATEGORY_DEFAULT,
    TARGET_SELECT_CATEGORY_CHANNEL,
    TARGET_SELECT_CATEGORY_NEARBY,
    TARGET_SELECT_CATEGORY_CONE,
    TARGET_SELECT_CATEGORY_AREA,
    TARGET_SELECT_CATEGORY_TRAJ,
};

enum SpellTargetReferenceTypes
{
    TARGET_REFERENCE_TYPE_NONE,
    TARGET_REFERENCE_TYPE_CASTER,
    TARGET_REFERENCE_TYPE_TARGET,
    TARGET_REFERENCE_TYPE_LAST,
    TARGET_REFERENCE_TYPE_SRC,
    TARGET_REFERENCE_TYPE_DEST,
};

enum SpellTargetObjectTypes : uint8
{
    TARGET_OBJECT_TYPE_NONE = 0,
    TARGET_OBJECT_TYPE_SRC,
    TARGET_OBJECT_TYPE_DEST,
    TARGET_OBJECT_TYPE_UNIT,
    TARGET_OBJECT_TYPE_UNIT_AND_DEST,
    TARGET_OBJECT_TYPE_GOBJ,
    TARGET_OBJECT_TYPE_GOBJ_ITEM,
    TARGET_OBJECT_TYPE_ITEM,
    TARGET_OBJECT_TYPE_CORPSE,
    // only for effect target type
    TARGET_OBJECT_TYPE_CORPSE_ENEMY,
    TARGET_OBJECT_TYPE_CORPSE_ALLY,
};

enum SpellTargetCheckTypes : uint8
{
    TARGET_CHECK_DEFAULT,
    TARGET_CHECK_ENTRY,
    TARGET_CHECK_ENEMY,
    TARGET_CHECK_ALLY,
    TARGET_CHECK_PARTY,
    TARGET_CHECK_RAID,
    TARGET_CHECK_RAID_CLASS, //LK
    TARGET_CHECK_PASSENGER,  //LK
    TARGET_CHECK_CORPSE,
};

enum SpellTargetDirectionTypes
{
    TARGET_DIR_NONE,
    TARGET_DIR_FRONT,
    TARGET_DIR_BACK,
    TARGET_DIR_RIGHT,
    TARGET_DIR_LEFT,
    TARGET_DIR_FRONT_RIGHT,
    TARGET_DIR_BACK_RIGHT,
    TARGET_DIR_BACK_LEFT,
    TARGET_DIR_FRONT_LEFT,
    TARGET_DIR_RANDOM,
    TARGET_DIR_ENTRY,
};

enum SpellEffectImplicitTargetTypes
{
    EFFECT_IMPLICIT_TARGET_NONE = 0,
    EFFECT_IMPLICIT_TARGET_EXPLICIT,
    EFFECT_IMPLICIT_TARGET_CASTER,
};

// Spell clasification
enum SpellSpecificType
{
    SPELL_SPECIFIC_NORMAL                = 0,
    SPELL_SPECIFIC_SEAL                  = 1,
    SPELL_SPECIFIC_BLESSING              = 2,
    SPELL_SPECIFIC_AURA                  = 3,
    SPELL_SPECIFIC_STING                 = 4,
    SPELL_SPECIFIC_CURSE                 = 5,
    SPELL_SPECIFIC_ASPECT                = 6,
    SPELL_SPECIFIC_TRACKER               = 7,
    SPELL_SPECIFIC_WARLOCK_ARMOR         = 8,
    SPELL_SPECIFIC_MAGE_ARMOR            = 9,
    SPELL_SPECIFIC_ELEMENTAL_SHIELD      = 10,
    SPELL_SPECIFIC_MAGE_POLYMORPH        = 11,
    SPELL_SPECIFIC_POSITIVE_SHOUT        = 12,
    SPELL_SPECIFIC_JUDGEMENT             = 13,
    SPELL_SPECIFIC_BATTLE_ELIXIR         = 14,
    SPELL_SPECIFIC_GUARDIAN_ELIXIR       = 15,
    SPELL_SPECIFIC_FLASK_ELIXIR          = 16,
    SPELL_SPECIFIC_WARLOCK_CORRUPTION    = 17,
    SPELL_SPECIFIC_WELL_FED              = 18,
    SPELL_SPECIFIC_DRINK                 = 19,
    SPELL_SPECIFIC_FOOD                  = 20,
    SPELL_SPECIFIC_CHARM                 = 21,
    SPELL_SPECIFIC_WARRIOR_ENRAGE        = 22,
    //REUSE          = 23,
    SPELL_SPECIFIC_DRUID_MANGLE          = 24,
    SPELL_SPECIFIC_FOOD_AND_DRINK        = 25,
    SPELL_SPECIFIC_SCROLL                = 26,
    SPELL_SPECIFIC_MAGE_ARCANE_BRILLANCE = 27,
    SPELL_SPECIFIC_PRIEST_DIVINE_SPIRIT  = 28,
};

enum SpellEffectMask
{
    SPELL_EFFECT_MASK_NONE = 0x0,
    SPELL_EFFECT_MASK_1    = 0x1,
    SPELL_EFFECT_MASK_2    = 0x2,
    SPELL_EFFECT_MASK_3    = 0x4,

    SPELL_EFFECT_MASK_ALL  = SPELL_EFFECT_MASK_1 | SPELL_EFFECT_MASK_2 | SPELL_EFFECT_MASK_3,
};

uint32 GetTargetFlagMask(SpellTargetObjectTypes objType);

class TC_GAME_API SpellImplicitTargetInfo
{
private:
    Targets _target;

public:
    SpellImplicitTargetInfo() : _target(Targets(0)) { }
    SpellImplicitTargetInfo(uint32 target);

    bool IsArea() const;
    bool IsProximityBasedAoe() const;

    SpellTargetSelectionCategories GetSelectionCategory() const;
    SpellTargetReferenceTypes GetReferenceType() const;
    SpellTargetObjectTypes GetObjectType() const;
    SpellTargetCheckTypes GetCheckType() const;
    SpellTargetDirectionTypes GetDirectionType() const;
    float CalcDirectionAngle() const;

    Targets GetTarget() const;
    uint32 GetExplicitTargetMask(bool& srcSet, bool& dstSet) const;

private:
    struct StaticData
    {
        SpellTargetObjectTypes ObjectType;    // type of object returned by target type
        SpellTargetReferenceTypes ReferenceType; // defines which object is used as a reference when selecting target
        SpellTargetSelectionCategories SelectionCategory;
        SpellTargetCheckTypes SelectionCheckType; // defines selection criteria
        SpellTargetDirectionTypes DirectionType; // direction for cone and dest targets
    };
    static StaticData _data[TOTAL_SPELL_TARGETS];
};

class TC_GAME_API SpellEffectInfo
{
    friend class SpellMgr;

    SpellInfo const* _spellInfo;
    uint8 _effIndex;
public:
    uint32    Effect;
    uint32    ApplyAuraName;
    uint32    Amplitude;
    int32     DieSides;
    int32     BaseDice; //not LK, present only on some test spells, ignore this
    float     DicePerLevel; //not LK, not different from 0 on any spells, ignore this
    float     RealPointsPerLevel;
    int32     BasePoints;
    float     PointsPerComboPoint;
    float     ValueMultiplier;
    float     DamageMultiplier;
#ifdef LICH_KING
    float     BonusMultiplier;
#endif
    int32     MiscValue;
    int32     MiscValueB;
    Mechanics Mechanic;
    SpellImplicitTargetInfo TargetA;
    SpellImplicitTargetInfo TargetB;
    SpellRadiusEntry const* RadiusEntry;
    uint32    ChainTarget;
    uint32    ItemType;
    uint32    TriggerSpell;
#ifdef LICH_KING
    //not on bc, we use spell_affect table instead
    flag96    SpellClassMask;
#else
    uint64    SpellClassMask; //fake field for BC, contains spell_affect table data
#endif
    std::vector<Condition*>* ImplicitTargetConditions;

    SpellEffectInfo() : _spellInfo(nullptr), _effIndex(0), Effect(0), ApplyAuraName(0), Amplitude(0), DieSides(0),
        RealPointsPerLevel(0), BasePoints(0), PointsPerComboPoint(0), ValueMultiplier(0), DamageMultiplier(0),
        MiscValue(0), MiscValueB(0), Mechanic(MECHANIC_NONE), RadiusEntry(nullptr), ChainTarget(0),
        ItemType(0), TriggerSpell(0), SpellClassMask(0), ImplicitTargetConditions(nullptr),
#ifdef LICH_KING
        BonusMultiplier(0)
#else
        BaseDice(0), DicePerLevel(0)
#endif    
    {}
    SpellEffectInfo(SpellEntry const* spellEntry, SpellInfo const* spellInfo, uint8 effIndex);

    bool IsEffect() const;
    bool IsEffect(SpellEffects effectName) const;
    bool IsAura() const;
    bool IsAura(AuraType aura) const;
    bool IsTargetingArea() const;
    bool IsAreaAuraEffect() const;
    bool IsUnitOwnedAuraEffect() const;

    int32 CalcValue(WorldObject  const* caster = nullptr, int32 const* basePoints = nullptr) const;
    int32 CalcBaseValue(int32 value) const;
    float CalcValueMultiplier(WorldObject * caster, Spell* spell = nullptr) const;
    float CalcDamageMultiplier(WorldObject * caster, Spell* spell = nullptr) const;

    bool HasRadius() const;
    //always use GetSpellModOwner() for caster
    float CalcRadius(WorldObject * caster = nullptr, Spell* = nullptr) const;

    uint32 GetProvidedTargetMask() const;
    uint32 GetMissingTargetMask(bool srcSet = false, bool destSet = false, uint32 mask = 0) const;

    SpellEffectImplicitTargetTypes GetImplicitTargetType() const;
    SpellTargetObjectTypes GetUsedTargetObjectType() const;
private:

    struct StaticData
    {
    SpellEffectImplicitTargetTypes ImplicitTargetType; // defines what target can be added to effect target list if there's no valid target type provided for effect
    SpellTargetObjectTypes UsedTargetObjectType; // defines valid target object type for spell effect
    };
    static StaticData _data[TOTAL_SPELL_EFFECTS];
};

struct TC_GAME_API SpellDiminishInfo
{
    DiminishingGroup DiminishGroup = DIMINISHING_NONE;
    DiminishingReturnsType DiminishReturnType = DRTYPE_NONE;
#ifdef LICH_KING
    DiminishingLevels DiminishMaxLevel = DIMINISHING_LEVEL_IMMUNE;
#endif
    int32 DiminishDurationLimit = 0;
};

struct TC_GAME_API ImmunityInfo
{
    uint32 SchoolImmuneMask = 0;
    uint32 ApplyHarmfulAuraImmuneMask = 0;
    uint32 MechanicImmuneMask = 0;
    uint32 DispelImmune = 0;
    uint32 DamageSchoolMask = 0;

    boost::container::flat_set<AuraType> AuraTypeImmune;
    boost::container::flat_set<SpellEffects> SpellEffectImmune;
};

class TC_GAME_API SpellInfo
{
    friend class SpellMgr;

public:
    uint32 Id;
    //can be null
    SpellCategoryEntry const* Category;
    uint32 Dispel;
    Mechanics Mechanic;
    uint32 Attributes;
    uint32 AttributesEx;
    uint32 AttributesEx2;
    uint32 AttributesEx3;
    uint32 AttributesEx4;
    uint32 AttributesEx5;
    uint32 AttributesEx6;
    uint32 AttributesEx7; // LK field, not commented out so we can still use it if we want
    uint32 AttributesCu;
    uint32 Stances;
    uint32 StancesNot;
    uint32 Targets;
    uint32 TargetCreatureType;
    uint32 RequiresSpellFocus;
    uint32 FacingCasterFlags;
    AuraStateType CasterAuraState;
    AuraStateType TargetAuraState;
    AuraStateType CasterAuraStateNot;
    AuraStateType TargetAuraStateNot;
#ifdef LICH_KING
    uint32 CasterAuraSpell;
    uint32 TargetAuraSpell;
    uint32 ExcludeCasterAuraSpell;
    uint32 ExcludeTargetAuraSpell;
#endif
    //can be null
    SpellCastTimesEntry const* CastTimeEntry;
    uint32 RecoveryTime;
    uint32 CategoryRecoveryTime;
    uint32 StartRecoveryCategory;
    uint32 StartRecoveryTime;
    uint32 InterruptFlags;
    uint32 AuraInterruptFlags;
    uint32 ChannelInterruptFlags;
    uint32 ProcFlags;
    uint32 ProcChance;
    uint32 ProcCharges;
    uint32 MaxLevel;
    uint32 BaseLevel; // = min level
    uint32 SpellLevel;
    //can be null
    SpellDurationEntry const* DurationEntry;
    uint32 PowerType;
    uint32 ManaCost;
    uint32 ManaCostPerlevel;
    uint32 ManaPerSecond;
    uint32 ManaPerSecondPerLevel;
    uint32 ManaCostPercentage;
    //uint32 RuneCostID; //LK
    //can be null
    SpellRangeEntry const* RangeEntry;
    float  Speed;
    uint32 StackAmount;
    uint32 Totem[2];
    int32  Reagent[MAX_SPELL_REAGENTS];
    uint32 ReagentCount[MAX_SPELL_REAGENTS];
    int32  EquippedItemClass;
    int32  EquippedItemSubClassMask;
    int32  EquippedItemInventoryTypeMask;
    //spell effects later
    uint32 TotemCategory[2];
#ifdef LICH_KING
    uint32 SpellVisual[2];
#else
    uint32 SpellVisual;
#endif
    uint32 SpellIconID;
    uint32 ActiveIconID;
    uint32 Priority;
    char* SpellName[16];
    char* Rank[16];
    uint32 MaxTargetLevel;
    uint32 MaxAffectedTargets;
    uint32 SpellFamilyName;
#ifdef LICH_KING
    flag96 SpellFamilyFlags;
#else
    uint64 SpellFamilyFlags;
#endif
    uint32 DmgClass;
    uint32 PreventionType;
#ifdef LICH_KING
    int32  AreaGroupId;
#else
    uint32 AreaId;
#endif
    uint32 SchoolMask;
    SpellEffectInfo Effects[MAX_SPELL_EFFECTS];
    uint32 ExplicitTargetMask;
    SpellChainNode const* ChainEntry;

    SpellInfo(SpellEntry const* spellEntry);
    ~SpellInfo();

    uint32 GetCategory() const;
    bool HasEffectByEffectMask(SpellEffects effect, SpellEffectMask effectMask = SPELL_EFFECT_MASK_ALL) const;
    bool HasEffect(SpellEffects effect, uint8 effectIndex = 0) const;
    bool HasAura(AuraType aura) const;
    bool HasAuraEffect(AuraType aura) const;
    bool HasAreaAuraEffect() const;
    bool HasAnyAura() const;
    bool HasOnlyDamageEffects() const;

    inline bool HasAttribute(SpellAttr0 attribute) const { return !!(Attributes & attribute); }
    inline bool HasAttribute(SpellAttr1 attribute) const { return !!(AttributesEx & attribute); }
    inline bool HasAttribute(SpellAttr2 attribute) const { return !!(AttributesEx2 & attribute); }
    inline bool HasAttribute(SpellAttr3 attribute) const { return !!(AttributesEx3 & attribute); }
    inline bool HasAttribute(SpellAttr4 attribute) const { return !!(AttributesEx4 & attribute); }
    inline bool HasAttribute(SpellAttr5 attribute) const { return !!(AttributesEx5 & attribute); }
    inline bool HasAttribute(SpellAttr6 attribute) const { return !!(AttributesEx6 & attribute); }
    inline bool HasAttribute(SpellCustomAttributes customAttribute) const { return !!(AttributesCu & customAttribute); }

    bool IsAffectingArea() const;
    bool IsTargetingArea() const;
    bool NeedsExplicitUnitTarget() const;
    bool NeedsToBeTriggeredByCaster(SpellInfo const* triggeringSpell, uint8 effIndex = MAX_SPELL_EFFECTS) const;
    bool IsAreaAuraEffect() const;
    bool IsChanneled() const;
    bool IsMoveAllowedChannel() const;
    bool IsNextMeleeSwingSpell() const;
    bool NeedsComboPoints() const;
    bool IsBreakingStealth() const;
    bool IsDeathPersistent() const;
    bool IsRequiringDeadTarget() const;
    bool IsAllowingDeadTarget() const;
    bool IsGroupBuff() const;
    bool IsValidDeadOrAliveTarget(Unit const* target) const;
    bool HasVisual(uint32 visual) const;
    bool CanBeUsedInCombat() const;
    bool IsPassive() const;
    bool IsAutocastable() const;
    bool IsStackableWithRanks() const;
    bool IsPassiveStackableWithRanks() const;
    bool IsMultiSlotAura() const;
    bool IsStackableOnOneSlotWithDifferentCasters() const;
	bool IsCooldownStartedOnEvent() const;
    bool IsChannelCategorySpell() const;
    bool IsBinarySpell() const;

    bool IsProfessionOrRiding() const;
    bool IsAbilityLearnedWithProfession() const;

    /** Some spells, such as dispells, can be positive or negative depending on target */
    bool IsPositive() const;
    /** Some effects, such as dispells, can be positive or negative depending on target */
    bool IsPositiveEffect(uint8 effIndex, bool hostileTarget = false) const;

    uint32 CalcCastTime(Spell* spell = nullptr) const;
    uint32 GetRecoveryTime() const;

    int32 CalcPowerCost(WorldObject const* caster, SpellSchoolMask schoolMask, Spell* spell = nullptr) const;

    bool IsRanked() const;
    uint8 GetRank() const;
    SpellInfo const* GetFirstRankSpell() const;
    SpellInfo const* GetLastRankSpell() const;
    SpellInfo const* GetNextRankSpell() const;
    SpellInfo const* GetPrevRankSpell() const;
    SpellInfo const* GetAuraRankForLevel(uint8 level) const;
    bool IsRankOf(SpellInfo const* spellInfo) const;
    bool IsDifferentRankOf(SpellInfo const* spellInfo) const;
    bool IsHighRankOf(SpellInfo const* spellInfo) const;

    // Diminishing Returns interaction with spells
    DiminishingGroup GetDiminishingReturnsGroupForSpell(bool triggered) const;
    int32 GetDiminishingReturnsLimitDuration(bool triggered) const;
    DiminishingLevels GetDiminishingReturnsMaxLevel(bool triggered) const;
    DiminishingReturnsType GetDiminishingReturnsGroupType(bool triggered) const;

    bool CanDispelAura(SpellInfo const* auraSpellInfo) const;
    bool CanPierceImmuneAura(SpellInfo const* auraSpellInfo) const;

    int32 GetDuration() const;
    int32 GetMaxDuration() const;

    uint32 GetMaxTicks() const;

    bool IsRangedWeaponSpell() const;
    bool IsAutoRepeatRangedSpell() const;
    bool HasInitialAggro() const;

    WeaponAttackType GetAttackType() const;

    bool IsItemFitToSpellRequirements(Item const* item) const;

    //familyFlags = override spell family flags
    bool IsAffected(uint32 familyName, uint64 const& familyFlags) const;

    bool IsAffectedBySpellMods() const;
    bool IsAffectedBySpellMod(SpellModifier const* mod) const;

    SpellSchoolMask GetSchoolMask() const;
    uint32 GetAllEffectsMechanicMask() const;
    uint32 GetEffectMechanicMask(uint8 effIndex) const;
    uint32 GetSpellMechanicMaskByEffectMask(SpellEffectMask effectMask) const;
    Mechanics GetEffectMechanic(uint8 effIndex) const;
    bool HasAnyEffectMechanic() const;
    uint32 GetDispelMask() const;
    static uint32 GetDispelMask(DispelType type);
    uint32 GetExplicitTargetMask() const;

    AuraStateType GetAuraState() const;
    SpellSpecificType GetSpellSpecific() const;
    SpellSpecificType GetSpellElixirSpecific() const;

    float GetMinRange(bool positive = false) const;
    //always use GetSpellModOwner() for caster
    float GetMaxRange(bool positive = false, WorldObject* caster = nullptr, Spell* spell = nullptr) const;

    bool IsSingleTarget() const;
    bool IsAuraExclusiveBySpecificWith(SpellInfo const* spellInfo) const;
    bool IsAuraExclusiveBySpecificPerCasterWith(SpellInfo const* spellInfo) const;

    SpellCastResult CheckTarget(WorldObject  const* caster, WorldObject const* target, bool implicit = true, Spell const* spell = nullptr) const;
    SpellCastResult CheckExplicitTarget(WorldObject  const* caster, WorldObject const* target, Item const* itemTarget = nullptr) const;
    SpellCastResult CheckLocation(uint32 map_id, uint32 zone_id, uint32 area_id, Player const* player = nullptr, bool strict = true) const;
    bool CheckTargetCreatureType(Unit const* target) const;

    // spell immunities
    void ApplyAllSpellImmunitiesTo(Unit* target, uint8 effIndex, bool apply) const;
    void ApplyDispelImmune(Unit* target, DispelType dispelType, bool apply) const;
    void ApplyMechanicImmune(Unit* target, uint32 mechanicImmunityMask, bool apply) const;
    void ApplySchoolImmune(Unit* target, uint32 schoolMask, bool apply) const;
    bool CanSpellProvideImmunityAgainstAura(SpellInfo const* auraSpellInfo) const;
    bool SpellCancelsAuraEffect(SpellInfo const* auraSpellInfo, uint8 auraEffIndex) const;

    uint32 GetAllowedMechanicMask() const;

private:
    void _LoadImmunityInfo();
    void _LoadSpellDiminishInfo();
    void _InitializeExplicitTargetMask();

    uint32 _GetExplicitTargetMask() const;

    void _InitializeSpellPositivity();

    // unloading helpers
    void _UnloadImplicitTargetConditionLists();

    SpellDiminishInfo _diminishInfoNonTriggered;
    SpellDiminishInfo _diminishInfoTriggered;

    uint32 _allowedMechanicMask;

    ImmunityInfo _immunityInfo[MAX_SPELL_EFFECTS];
};

#endif // _SPELLINFO_