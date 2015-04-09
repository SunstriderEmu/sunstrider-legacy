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

#include "SpellInfo.h"
#include "DBCStores.h"
#include "SpellAuraDefines.h"

SpellImplicitTargetInfo::SpellImplicitTargetInfo(uint32 target)
{
    _target = Targets(target);
}

bool SpellImplicitTargetInfo::IsArea() const
{
    switch (_target)
    {
    case TARGET_UNIT_DEST_AREA_ENEMY:
    case TARGET_UNIT_SRC_AREA_ENEMY:
    case TARGET_UNIT_DEST_AREA_ALLY:
    case TARGET_UNIT_SRC_AREA_ALLY:
    case TARGET_UNIT_DEST_AREA_ENTRY:
    case TARGET_UNIT_SRC_AREA_ENTRY:
    case TARGET_UNIT_DEST_AREA_PARTY:
    case TARGET_UNIT_SRC_AREA_PARTY:
    case TARGET_UNIT_LASTTARGET_AREA_PARTY:
    case TARGET_UNIT_CASTER_AREA_PARTY:
    case TARGET_UNIT_CONE_ENEMY:
    case TARGET_UNIT_CONE_ALLY:
    case TARGET_UNIT_CONE_ENEMY_UNKNOWN:
    case TARGET_UNIT_CASTER_AREA_RAID:
        return true;
    default:
        return false;
    }
    //return GetSelectionCategory() == TARGET_SELECT_CATEGORY_AREA || GetSelectionCategory() == TARGET_SELECT_CATEGORY_CONE;
}

Targets SpellImplicitTargetInfo::GetTarget() const
{
    return _target;
}

SpellEffectInfo::SpellEffectInfo(SpellEntry const* spellEntry, SpellInfo const* spellInfo, uint8 effIndex)
{
    _spellInfo = spellInfo;
    _effIndex = effIndex;
    Effect = spellEntry->Effect[effIndex];
    ApplyAuraName = spellEntry->EffectApplyAuraName[effIndex];
    Amplitude = spellEntry->EffectAmplitude[effIndex];
    DieSides = spellEntry->EffectDieSides[effIndex];
#ifndef LICH_KING
    BaseDice = spellEntry->EffectBaseDice[effIndex];
    DicePerLevel = spellEntry->EffectDicePerLevel[effIndex];
#endif
    RealPointsPerLevel = spellEntry->EffectRealPointsPerLevel[effIndex];
    BasePoints = spellEntry->EffectBasePoints[effIndex];
    PointsPerComboPoint = spellEntry->EffectPointsPerComboPoint[effIndex];
    ValueMultiplier = spellEntry->EffectValueMultiplier[effIndex];
    DamageMultiplier = spellEntry->EffectDamageMultiplier[effIndex];
#ifdef LICH_KING
    BonusMultiplier = spellEntry->EffectBonusMultiplier[effIndex];
#endif
    MiscValue = spellEntry->EffectMiscValue[effIndex];
    MiscValueB = spellEntry->EffectMiscValueB[effIndex];
    Mechanic = Mechanics(spellEntry->EffectMechanic[effIndex]);
    TargetA = SpellImplicitTargetInfo(spellEntry->EffectImplicitTargetA[effIndex]);
    TargetB = SpellImplicitTargetInfo(spellEntry->EffectImplicitTargetB[effIndex]);
    RadiusEntry = spellEntry->EffectRadiusIndex[effIndex] ? sSpellRadiusStore.LookupEntry(spellEntry->EffectRadiusIndex[effIndex]) : NULL;
    ChainTarget = spellEntry->EffectChainTarget[effIndex];
    ItemType = spellEntry->EffectItemType[effIndex];
    TriggerSpell = spellEntry->EffectTriggerSpell[effIndex];
#ifdef LICH_KING
    SpellClassMask = spellEntry->EffectSpellClassMask[effIndex];
#endif
    //TODO SPELLINFO ImplicitTargetConditions = NULL;
}

bool SpellEffectInfo::IsEffect() const
{
    return Effect != 0;
}

bool SpellEffectInfo::IsEffect(SpellEffects effectName) const
{
    return Effect == uint32(effectName);
}

bool SpellEffectInfo::IsAura() const
{
    return (IsUnitOwnedAuraEffect() || Effect == SPELL_EFFECT_PERSISTENT_AREA_AURA) && ApplyAuraName != 0;
}

bool SpellEffectInfo::IsAura(AuraType aura) const
{
    return IsAura() && ApplyAuraName == uint32(aura);
}

bool SpellEffectInfo::IsTargetingArea() const
{
    return TargetA.IsArea() || TargetB.IsArea();
}

bool SpellEffectInfo::IsAreaAuraEffect() const
{
    if (Effect == SPELL_EFFECT_APPLY_AREA_AURA_PARTY ||
#ifdef LICH_KING
        Effect == SPELL_EFFECT_APPLY_AREA_AURA_RAID ||
#endif
        Effect == SPELL_EFFECT_APPLY_AREA_AURA_FRIEND ||
        Effect == SPELL_EFFECT_APPLY_AREA_AURA_ENEMY ||
        Effect == SPELL_EFFECT_APPLY_AREA_AURA_PET ||
        Effect == SPELL_EFFECT_APPLY_AREA_AURA_OWNER)
        return true;

    return false;
}

bool SpellEffectInfo::IsFarUnitTargetEffect() const
{
    return (Effect == SPELL_EFFECT_SUMMON_PLAYER)
#ifdef LICH_KING
        || (Effect == SPELL_EFFECT_SUMMON_RAF_FRIEND)
#endif
        || (Effect == SPELL_EFFECT_RESURRECT)
        || (Effect == SPELL_EFFECT_RESURRECT_NEW)
        || (Effect == SPELL_EFFECT_SKIN_PLAYER_CORPSE);
}

bool SpellEffectInfo::IsFarDestTargetEffect() const
{
    return Effect == SPELL_EFFECT_TELEPORT_UNITS;
}

bool SpellEffectInfo::IsUnitOwnedAuraEffect() const
{
    return IsAreaAuraEffect() || Effect == SPELL_EFFECT_APPLY_AURA;
}

SpellInfo::SpellInfo(SpellEntry const* spellEntry)
{
    Id = spellEntry->Id;
    Category = spellEntry->Category ? sSpellCategoryStore.LookupEntry(spellEntry->Category) : NULL;
    Dispel = spellEntry->Dispel;
    Mechanic = spellEntry->Mechanic;
    Attributes = spellEntry->Attributes;
    AttributesEx = spellEntry->AttributesEx;
    AttributesEx2 = spellEntry->AttributesEx2;
    AttributesEx3 = spellEntry->AttributesEx3;
    AttributesEx4 = spellEntry->AttributesEx4;
    AttributesEx5 = spellEntry->AttributesEx5;
    AttributesEx6 = spellEntry->AttributesEx6;
#ifdef LICH_KING
    AttributesEx7 = spellEntry->AttributesEx7;
#else
    AttributesEx7 = 0;
#endif
    AttributesCu = 0;
    Stances = spellEntry->Stances;
    StancesNot = spellEntry->StancesNot;
    Targets = spellEntry->Targets;
    TargetCreatureType = spellEntry->TargetCreatureType;
    RequiresSpellFocus = spellEntry->RequiresSpellFocus;
    FacingCasterFlags = spellEntry->FacingCasterFlags;
    CasterAuraState = spellEntry->CasterAuraState;
    TargetAuraState = spellEntry->TargetAuraState;
    CasterAuraStateNot = spellEntry->CasterAuraStateNot;
    TargetAuraStateNot = spellEntry->TargetAuraStateNot;
#ifdef LICH_KING
    CasterAuraSpell = spellEntry->casterAuraSpell;
    TargetAuraSpell = spellEntry->targetAuraSpell;
    ExcludeCasterAuraSpell = spellEntry->excludeCasterAuraSpell;
    ExcludeTargetAuraSpell = spellEntry->excludeTargetAuraSpell;
#else
    CasterAuraSpell = 0;
    TargetAuraSpell = 0;
    ExcludeCasterAuraSpell = 0;
    ExcludeTargetAuraSpell = 0;
#endif
    CastTimeEntry = spellEntry->CastingTimeIndex ? sSpellCastTimesStore.LookupEntry(spellEntry->CastingTimeIndex) : NULL;
    RecoveryTime = spellEntry->RecoveryTime;
    CategoryRecoveryTime = spellEntry->CategoryRecoveryTime;
    StartRecoveryCategory = spellEntry->StartRecoveryCategory;
    StartRecoveryTime = spellEntry->StartRecoveryTime;
    InterruptFlags = spellEntry->InterruptFlags;
    AuraInterruptFlags = spellEntry->AuraInterruptFlags;
    ChannelInterruptFlags = spellEntry->ChannelInterruptFlags;
    ProcFlags = spellEntry->ProcFlags;
    ProcChance = spellEntry->procChance;
    ProcCharges = spellEntry->procCharges;
    MaxLevel = spellEntry->MaxLevel;
    BaseLevel = spellEntry->BaseLevel;
    SpellLevel = spellEntry->SpellLevel;
    DurationEntry = spellEntry->DurationIndex ? sSpellDurationStore.LookupEntry(spellEntry->DurationIndex) : NULL;
    PowerType = spellEntry->powerType;
    ManaCost = spellEntry->ManaCost;
    ManaCostPerlevel = spellEntry->ManaCostPerlevel;
    ManaPerSecond = spellEntry->manaPerSecond;
    ManaPerSecondPerLevel = spellEntry->ManaPerSecondPerLevel;
    ManaCostPercentage = spellEntry->ManaCostPercentage;
#ifdef LICH_KING
    RuneCostID = spellEntry->runeCostID;
#else
    RuneCostID = 0;
#endif
    RangeEntry = spellEntry->rangeIndex ? sSpellRangeStore.LookupEntry(spellEntry->rangeIndex) : NULL;
    Speed = spellEntry->speed;
    StackAmount = spellEntry->StackAmount;
    for (uint8 i = 0; i < 2; ++i)
        Totem[i] = spellEntry->Totem[i];

    for (uint8 i = 0; i < MAX_SPELL_REAGENTS; ++i)
    {
        Reagent[i] = spellEntry->Reagent[i];
        ReagentCount[i] = spellEntry->ReagentCount[i];
    }

    EquippedItemClass = spellEntry->EquippedItemClass;
    EquippedItemSubClassMask = spellEntry->EquippedItemSubClassMask;
    EquippedItemInventoryTypeMask = spellEntry->EquippedItemInventoryTypeMask;
    for (uint8 i = 0; i < 2; ++i)
        TotemCategory[i] = spellEntry->TotemCategory[i];

    for (uint8 i = 0; i < 2; ++i)
#ifdef LICH_KING
        SpellVisual[i] = spellEntry->SpellVisual[i];
#else
        SpellVisual = spellEntry->SpellVisual;
#endif

    SpellIconID = spellEntry->SpellIconID;
    ActiveIconID = spellEntry->activeIconID;
    for (uint8 i = 0; i < 16; ++i)
        SpellName[i] = spellEntry->SpellName[i];

    for (uint8 i = 0; i < 16; ++i)
        Rank[i] = spellEntry->Rank[i];

    MaxTargetLevel = spellEntry->MaxTargetLevel;
    MaxAffectedTargets = spellEntry->MaxAffectedTargets;
    SpellFamilyName = spellEntry->SpellFamilyName;
    SpellFamilyFlags = spellEntry->SpellFamilyFlags;
    DmgClass = spellEntry->DmgClass;
    PreventionType = spellEntry->PreventionType;
#ifdef LICH_KING
    AreaGroupId = spellEntry->AreaGroupId;
#else
    AreaId = spellEntry->AreaId;
#endif
    SchoolMask = spellEntry->SchoolMask;
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        Effects[i] = SpellEffectInfo(spellEntry, this, i);

    /* TODO SPELLINFO
    ChainEntry = NULL;
    ExplicitTargetMask = 0; */
}

SpellInfo::~SpellInfo()
{
    //TODO Spellinfo _UnloadImplicitTargetConditionLists();
}

// checks if spell targets are selected from area, doesn't include spell effects in check (like area wide auras for example)
bool SpellInfo::IsTargetingArea() const
{
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        if (Effects[i].IsEffect() && Effects[i].IsTargetingArea())
            return true;
    return false;
}

bool SpellInfo::IsAffectingArea() const
{
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        if (Effects[i].IsEffect() && (Effects[i].IsTargetingArea() || Effects[i].IsEffect(SPELL_EFFECT_PERSISTENT_AREA_AURA) || Effects[i].IsAreaAuraEffect()))
            return true;
    return false;
}

bool SpellInfo::HasAreaAuraEffect() const
{
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        if (Effects[i].IsAreaAuraEffect())
            return true;
    return false;
}

uint32 SpellInfo::GetCategory() const
{
    return Category ? Category->Id : 0;
}


bool SpellInfo::HasEffect(SpellEffects effect) const
{
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        if (Effects[i].IsEffect(effect))
            return true;
    return false;
}

bool SpellInfo::HasAura(AuraType aura) const
{
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        if (Effects[i].IsAura(aura))
            return true;
    return false;
}

bool SpellInfo::IsBreakingStealth() const
{
    return !(AttributesEx & SPELL_ATTR1_NOT_BREAK_STEALTH);
}

bool SpellInfo::IsChanneled() const
{
    return (AttributesEx & (SPELL_ATTR1_CHANNELED_1 | SPELL_ATTR1_CHANNELED_2)) != 0;
}

bool SpellInfo::NeedsComboPoints() const
{
    return (AttributesEx & (SPELL_ATTR1_REQ_COMBO_POINTS1 | SPELL_ATTR1_REQ_COMBO_POINTS2)) != 0;
}

bool SpellInfo::IsDeathPersistent() const
{
    return (AttributesEx3 & SPELL_ATTR3_DEATH_PERSISTENT) != 0;
}

bool SpellInfo::HasVisual(uint32 visual) const
{
#ifdef LICH_KING
    for (uint8 i = 0; i < 2; i++)
        if (SpellVisual[i] == visual)
            return true;

    return false;
#else
    return SpellVisual == visual;
#endif
}

SpellSchoolMask SpellInfo::GetSchoolMask() const
{
    return SpellSchoolMask(SchoolMask);
}

uint32 SpellInfo::GetAllEffectsMechanicMask() const
{
    uint32 mask = 0;
    if (Mechanic)
        mask |= 1 << Mechanic;
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        if (Effects[i].IsEffect() && Effects[i].Mechanic)
            mask |= 1 << Effects[i].Mechanic;
    return mask;
}

uint32 SpellInfo::GetEffectMechanicMask(uint8 effIndex) const
{
    uint32 mask = 0;
    if (Mechanic)
        mask |= 1 << Mechanic;
    if (Effects[effIndex].IsEffect() && Effects[effIndex].Mechanic)
        mask |= 1 << Effects[effIndex].Mechanic;
    return mask;
}

uint32 SpellInfo::GetSpellMechanicMaskByEffectMask(uint32 effectMask) const
{
    uint32 mask = 0;
    if (Mechanic)
        mask |= 1 << Mechanic;
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        if ((effectMask & (1 << i)) && Effects[i].Mechanic)
            mask |= 1 << Effects[i].Mechanic;
    return mask;
}

Mechanics SpellInfo::GetEffectMechanic(uint8 effIndex) const
{
    if (Effects[effIndex].IsEffect() && Effects[effIndex].Mechanic)
        return Mechanics(Effects[effIndex].Mechanic);
    if (Mechanic)
        return Mechanics(Mechanic);
    return MECHANIC_NONE;
}

bool SpellInfo::HasAnyEffectMechanic() const
{
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        if (Effects[i].Mechanic)
            return true;
    return false;
}