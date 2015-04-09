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

class SpellImplicitTargetInfo
{
private:
    Targets _target;

public:
    SpellImplicitTargetInfo() : _target(Targets(0)) { }
    SpellImplicitTargetInfo(uint32 target);

    bool IsArea() const;
    /*
    SpellTargetSelectionCategories GetSelectionCategory() const;
    SpellTargetReferenceTypes GetReferenceType() const;
    SpellTargetObjectTypes GetObjectType() const;
    SpellTargetCheckTypes GetCheckType() const;
    SpellTargetDirectionTypes GetDirectionType() const;
    float CalcDirectionAngle() const;
    */
    Targets GetTarget() const;
    uint32 GetExplicitTargetMask(bool& srcSet, bool& dstSet) const;

    /*
    private:
    struct StaticData
    {
    SpellTargetObjectTypes ObjectType;    // type of object returned by target type
    SpellTargetReferenceTypes ReferenceType; // defines which object is used as a reference when selecting target
    SpellTargetSelectionCategories SelectionCategory;
    SpellTargetCheckTypes SelectionCheckType; // defines selection criteria
    SpellTargetDirectionTypes DirectionType; // direction for cone and dest targets
    };
    static StaticData _data[TOTAL_SPELL_TARGETS];*/
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
    int32     BaseDice; //not LK
    float     DicePerLevel; //not LK
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
    //TODO Spellinfo std::list<Condition*>* ImplicitTargetConditions;

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
    // LK --
    uint32 CasterAuraSpell;
    uint32 TargetAuraSpell;
    uint32 ExcludeCasterAuraSpell;
    uint32 ExcludeTargetAuraSpell;
    // -- LK
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
    uint32 BaseLevel;
    uint32 SpellLevel;
    SpellDurationEntry const* DurationEntry;
    uint32 PowerType;
    uint32 ManaCost;
    uint32 ManaCostPerlevel;
    uint32 ManaPerSecond;
    uint32 ManaPerSecondPerLevel;
    uint32 ManaCostPercentage;
    uint32 RuneCostID; //LK
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
    bool HasEffect(SpellEffects effect) const;
    bool HasAura(AuraType aura) const;
    bool HasAreaAuraEffect() const;

    bool IsAffectingArea() const;
    bool IsTargetingArea() const;
    bool IsAreaAuraEffect() const;
    bool IsChanneled() const;
    bool NeedsComboPoints() const;
    bool IsBreakingStealth() const;
    bool IsDeathPersistent() const;
    bool HasVisual(uint32 visual) const;

    SpellSchoolMask GetSchoolMask() const;
    uint32 GetAllEffectsMechanicMask() const;
    uint32 GetEffectMechanicMask(uint8 effIndex) const;
    uint32 GetSpellMechanicMaskByEffectMask(uint32 effectMask) const;
    Mechanics GetEffectMechanic(uint8 effIndex) const;
    bool HasAnyEffectMechanic() const;
};

#endif // _SPELLINFO_H