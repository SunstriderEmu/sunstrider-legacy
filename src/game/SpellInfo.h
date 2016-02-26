/*
* Copyright (C) 2008-2014 TrinityCore <http://www.trinitycore.org/>
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the
* Free Software Foundation; either version 2 of the License, or (at your
* option) any later version.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
* more details.
*
* You should have received a copy of the GNU General Public License along
* with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _SPELLINFO_H
#define _SPELLINFO_H

#include "SharedDefines.h"
#include "DBCStructure.h"
#include "Util.h"

enum AuraType;
class SpellInfo;
class Spell;
struct Condition;

enum SpellCustomAttributes
{
    //SPELL_ATTR_CU_PLAYERS_ONLY      0x00000001,
    SPELL_ATTR_CU_CONE_BACK                     = 0x00000002,
    SPELL_ATTR_CU_CONE_LINE                     = 0x00000004,
    SPELL_ATTR_CU_SHARE_DAMAGE                  = 0x00000008,
    SPELL_ATTR_CU_AURA_HOT                      = 0x00000010,
    SPELL_ATTR_CU_AURA_DOT                      = 0x00000020,
    SPELL_ATTR_CU_AURA_CC                       = 0x00000040,
    SPELL_ATTR_CU_AURA_SPELL                    = 0x00000080,
    SPELL_ATTR_CU_DIRECT_DAMAGE                 = 0x00000100,
    SPELL_ATTR_CU_CHARGE                        = 0x00000200,
    SPELL_ATTR_CU_LINK_CAST                     = 0x00000400,
    SPELL_ATTR_CU_LINK_HIT                      = 0x00000800,
    SPELL_ATTR_CU_LINK_AURA                     = 0x00001000,
    SPELL_ATTR_CU_LINK_REMOVE                   = 0x00002000,
    SPELL_ATTR_CU_MOVEMENT_IMPAIR               = 0x00004000,
    SPELL_ATTR_CU_IGNORE_ARMOR                  = 0x00008000,
    SPELL_ATTR_CU_SAME_STACK_DIFF_CASTERS       = 0x00010000,
    SPELL_ATTR_CU_ONE_STACK_PER_CASTER_SPECIAL  = 0x00020000,
    SPELL_ATTR_CU_THREAT_GOES_TO_CURRENT_CASTER = 0x00040000,     // Instead of original caster
    SPELL_ATTR_CU_CANT_BREAK_CC                 = 0x00080000,     // Damage done by these spells won't break crowd controls
    SPELL_ATTR_CU_PUT_ONLY_CASTER_IN_COMBAT     = 0x00100000,
    SPELL_ATTR_CU_REMOVE_ON_INSTANCE_ENTER      = 0x00200000,     // Auras removed when target enters an instance
    SPELL_ATTR_CU_AOE_CANT_TARGET_SELF          = 0x00400000,
    SPELL_ATTR_CU_CONE_180                      = 0x00800000,
    SPELL_ATTR_CU_CAN_CHANNEL_DEAD_TARGET       = 0x01000000,

    SPELL_ATTR_CU_NEGATIVE_EFF0                 = 0x02000000,
    SPELL_ATTR_CU_NEGATIVE_EFF1                 = 0x04000000,
    SPELL_ATTR_CU_NEGATIVE_EFF2                 = 0x08000000,

    SPELL_ATTR_CU_NEGATIVE                      = SPELL_ATTR_CU_NEGATIVE_EFF0 | SPELL_ATTR_CU_NEGATIVE_EFF1 | SPELL_ATTR_CU_NEGATIVE_EFF2
};

enum SpellCastTargetFlags
{
    TARGET_FLAG_SELF            = 0x00000000,
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

enum SpellTargetSelectionCategories
{
    TARGET_SELECT_CATEGORY_NYI,
    TARGET_SELECT_CATEGORY_DEFAULT,
    TARGET_SELECT_CATEGORY_CHANNEL,
    TARGET_SELECT_CATEGORY_NEARBY,
    TARGET_SELECT_CATEGORY_CONE,
    TARGET_SELECT_CATEGORY_AREA
};

enum SpellTargetReferenceTypes
{
    TARGET_REFERENCE_TYPE_NONE,
    TARGET_REFERENCE_TYPE_CASTER,
    TARGET_REFERENCE_TYPE_TARGET,
    TARGET_REFERENCE_TYPE_LAST,
    TARGET_REFERENCE_TYPE_SRC,
    TARGET_REFERENCE_TYPE_DEST
};

enum SpellTargetObjectTypes
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
    TARGET_OBJECT_TYPE_CORPSE_ALLY
};

enum SpellTargetCheckTypes
{
    TARGET_CHECK_DEFAULT,
    TARGET_CHECK_ENTRY,
    TARGET_CHECK_ENEMY,
    TARGET_CHECK_ALLY,
    TARGET_CHECK_PARTY,
    TARGET_CHECK_RAID,
    TARGET_CHECK_RAID_CLASS,
    TARGET_CHECK_PASSENGER
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
    TARGET_DIR_ENTRY
};

enum SpellEffectImplicitTargetTypes
{
    EFFECT_IMPLICIT_TARGET_NONE = 0,
    EFFECT_IMPLICIT_TARGET_EXPLICIT,
    EFFECT_IMPLICIT_TARGET_CASTER
};

// Spell clasification
enum SpellSpecificType
{
    SPELL_NORMAL            = 0,
    SPELL_SEAL              = 1,
    SPELL_BLESSING          = 2,
    SPELL_AURA              = 3,
    SPELL_STING             = 4,
    SPELL_CURSE             = 5,
    SPELL_ASPECT            = 6,
    SPELL_TRACKER           = 7,
    SPELL_WARLOCK_ARMOR     = 8,
    SPELL_MAGE_ARMOR        = 9,
    SPELL_ELEMENTAL_SHIELD  = 10,
    SPELL_MAGE_POLYMORPH    = 11,
    SPELL_POSITIVE_SHOUT    = 12,
    SPELL_JUDGEMENT         = 13,
    SPELL_BATTLE_ELIXIR     = 14,
    SPELL_GUARDIAN_ELIXIR   = 15,
    SPELL_FLASK_ELIXIR      = 16,
    SPELL_WARLOCK_CORRUPTION= 17,
    SPELL_WELL_FED          = 18,
    SPELL_DRINK             = 19,
    SPELL_FOOD              = 20,
    SPELL_CHARM             = 21,
    SPELL_WARRIOR_ENRAGE    = 22,
    SPELL_ARMOR_REDUCE      = 23,
    SPELL_DRUID_MANGLE      = 24
};

enum SpellEffectMask
{
    SPELL_EFFECT_MASK_NONE = 0x0,
    SPELL_EFFECT_MASK_1    = 0x1,
    SPELL_EFFECT_MASK_2    = 0x2,
    SPELL_EFFECT_MASK_3    = 0x4,

    SPELL_EFFECT_MASK_ALL  = SPELL_EFFECT_MASK_1 | SPELL_EFFECT_MASK_2 | SPELL_EFFECT_MASK_3,
};

class SpellImplicitTargetInfo
{
private:
    Targets _target;

public:
    SpellImplicitTargetInfo() : _target(Targets(0)) { }
    SpellImplicitTargetInfo(uint32 target);

    bool IsArea() const;
    
    //THOSE ARE NOT YET USED
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

class SpellEffectInfo
{
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
    flag96    SpellClassMask;
#endif
    std::list<Condition*>* ImplicitTargetConditions;

    SpellEffectInfo() : _spellInfo(NULL), _effIndex(0), Effect(0), ApplyAuraName(0), Amplitude(0), DieSides(0),
        RealPointsPerLevel(0), BasePoints(0), PointsPerComboPoint(0), ValueMultiplier(0), DamageMultiplier(0),
        MiscValue(0), MiscValueB(0), Mechanic(MECHANIC_NONE), RadiusEntry(NULL), ChainTarget(0),
        ItemType(0), TriggerSpell(0), /*ImplicitTargetConditions(NULL) */
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
    bool IsFarUnitTargetEffect() const;
    bool IsFarDestTargetEffect() const;
    bool IsUnitOwnedAuraEffect() const;

    bool HasRadius() const;
    //always use GetSpellModOwner() for caster
    float CalcRadius(Unit* caster = NULL, Spell* = NULL) const;
private:
    /*
    struct StaticData
    {
    SpellEffectImplicitTargetTypes ImplicitTargetType; // defines what target can be added to effect target list if there's no valid target type provided for effect
    SpellTargetObjectTypes UsedTargetObjectType; // defines valid target object type for spell effect
    };
    static StaticData _data[TOTAL_SPELL_EFFECTS]; */
};

class SpellInfo
{
public:
    uint32 Id;
    //can be null
    SpellCategoryEntry const* Category;
    uint32 Dispel;
    uint32 Mechanic;
    uint32 Attributes;
    uint32 AttributesEx;
    uint32 AttributesEx2;
    uint32 AttributesEx3;
    uint32 AttributesEx4;
    uint32 AttributesEx5;
    uint32 AttributesEx6;
    uint32 AttributesEx7; //LK
    uint32 AttributesCu;
    uint32 Stances;
    uint32 StancesNot;
    uint32 Targets;
    uint32 TargetCreatureType;
    uint32 RequiresSpellFocus;
    uint32 FacingCasterFlags;
    uint32 CasterAuraState;
    uint32 TargetAuraState;
    uint32 CasterAuraStateNot;
    uint32 TargetAuraStateNot;
    /* LK
    uint32 CasterAuraSpell;
    uint32 TargetAuraSpell;
    uint32 ExcludeCasterAuraSpell;
    uint32 ExcludeTargetAuraSpell;
    */
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
    /* TODO SPELLINFO
    uint32 ExplicitTargetMask;
    SpellChainNode const* ChainEntry;
    */

    SpellInfo(SpellEntry const* spellEntry);
    ~SpellInfo();

    uint32 GetCategory() const;
    /** -1 for all indexes */
    bool HasEffectByEffectMask(SpellEffects effect, SpellEffectMask effectMask = SPELL_EFFECT_MASK_ALL) const;
    bool HasEffect(SpellEffects effect, uint8 effectIndex = 0) const;
    bool HasAuraEffect(AuraType aura) const;
    bool HasAreaAuraEffect() const;

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
    bool IsAreaAuraEffect() const;
    bool IsChanneled() const;
    bool NeedsComboPoints() const;
    bool IsBreakingStealth() const;
    bool IsDeathPersistent() const;
    bool HasVisual(uint32 visual) const;
    bool CanBeUsedInCombat() const;
    bool IsPassive() const;

    /** Some spells, such as dispells, can be positive or negative depending on target */
    bool IsPositive(bool hostileTarget = false) const;
    /** Some effects, such as dispells, can be positive or negative depending on target */
    bool IsPositiveEffect(uint8 effIndex, bool hostileTarget = false) const;
    /* Internal check, will try to deduce result from spell effects + lots of hardcoded id's
    Use "deep" to enable recursive search in triggered spells
    */
    bool _IsPositiveEffect(uint32 effIndex, bool deep = true) const;
    bool _IsPositiveSpell() const;

    uint32 CalcCastTime(Spell* spell = nullptr) const;
    uint32 GetRecoveryTime() const;

    int32 GetDuration() const;
    int32 GetMaxDuration() const;

    uint32 GetMaxTicks() const;

    bool IsRangedWeaponSpell() const;
    bool IsAutoRepeatRangedSpell() const;
    bool HasInitialAggro() const;

    SpellSchoolMask GetSchoolMask() const;
    uint32 GetAllEffectsMechanicMask() const;
    uint32 GetEffectMechanicMask(uint8 effIndex) const;
    uint32 GetSpellMechanicMaskByEffectMask(SpellEffectMask effectMask) const;
    Mechanics GetEffectMechanic(uint8 effIndex) const;
    bool HasAnyEffectMechanic() const;

    AuraStateType GetAuraState() const;
    SpellSpecificType GetSpellSpecific() const;
    SpellSpecificType GetSpellElixirSpecific() const;

    float GetMinRange(bool positive = false) const;
    //always use GetSpellModOwner() for caster
    float GetMaxRange(bool positive = false, Unit* caster = NULL, Spell* spell = NULL) const;

    bool IsSingleTarget() const;
private:
    //apply SpellCustomAttributes. Some custom attributes are also added in SpellMgr::LoadSpellLinked()
    void LoadCustomAttributes();
    static bool _IsPositiveTarget(uint32 targetA, uint32 targetB);

    // unloading helpers
    void _UnloadImplicitTargetConditionLists();
};

#endif // _SPELLINFO_H