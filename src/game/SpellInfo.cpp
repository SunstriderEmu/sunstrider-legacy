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
    BonusMultiplier = spellEntry->Effects[effIndex].BonusMultiplier;
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

SpellSpecificType SpellInfo::GetSpellSpecific() const
{
    switch(SpellFamilyName)
    {
        case SPELLFAMILY_GENERIC:
        {
            //food/drink
            if (AuraInterruptFlags & AURA_INTERRUPT_FLAG_NOT_SEATED)
            {
                for(int i = 0; i < 3; i++)
                    if( Effects[i].ApplyAuraName==SPELL_AURA_MOD_POWER_REGEN)
                        return SPELL_DRINK;
                    else if ( Effects[i].ApplyAuraName==SPELL_AURA_MOD_REGEN)
                        return SPELL_FOOD;
            }
            // this may be a hack
            else if((HasAttribute(SPELL_ATTR2_FOOD))
                && !Category->Id)
                return SPELL_WELL_FED;

            switch(Id)
            {
                case 12880: // warrior's Enrage rank 1
                case 14201: //           Enrage rank 2
                case 14202: //           Enrage rank 3
                case 14203: //           Enrage rank 4
                case 14204: //           Enrage rank 5
                case 12292: //             Death Wish
                    return SPELL_WARRIOR_ENRAGE;
                break;
                default: break;
            }
            break;
        }
        case SPELLFAMILY_MAGE:
        {
            // family flags 18(Molten), 25(Frost/Ice), 28(Mage)
            if (SpellFamilyFlags & 0x12040000)
                return SPELL_MAGE_ARMOR;

            if ((SpellFamilyFlags & 0x1000000) && Effects[0].ApplyAuraName==SPELL_AURA_MOD_CONFUSE)
                return SPELL_MAGE_POLYMORPH;

            break;
        }
        case SPELLFAMILY_WARRIOR:
        {
            if (SpellFamilyFlags & 0x00008000010000LL)
                return SPELL_POSITIVE_SHOUT;
            // Sunder Armor (vs Expose Armor)
            if (SpellFamilyFlags & 0x00000000004000LL)
                return SPELL_ARMOR_REDUCE;
            break;
        }
        case SPELLFAMILY_WARLOCK:
        {
            // only warlock curses have this
            if (Dispel == DISPEL_CURSE)
                return SPELL_CURSE;

            // family flag 37 (only part spells have family name)
            if (SpellFamilyFlags & 0x2000000000LL)
                return SPELL_WARLOCK_ARMOR;

            //seed of corruption and corruption
            if (SpellFamilyFlags & 0x1000000002LL)
                return SPELL_WARLOCK_CORRUPTION;
            break;
        }
        case SPELLFAMILY_HUNTER:
        {
            // only hunter stings have this
            if (Dispel == DISPEL_POISON)
                return SPELL_STING;

            break;
        }
        case SPELLFAMILY_PALADIN:
        {
            //Collection of all the seal family flags. No other paladin spell has any of those.
            if( SpellFamilyFlags & 0x4000A000200LL );
                return SPELL_SEAL;

            if (SpellFamilyFlags & 0x10000100LL)
                return SPELL_BLESSING;

            if ((SpellFamilyFlags & 0x00000820180400LL) && (HasAttribute(SPELL_ATTR3_UNK9)))
                return SPELL_JUDGEMENT;

            for (int i = 0; i < 3; i++)
            {
                // only paladin auras have this
                if (Effects[i].Effect == SPELL_EFFECT_APPLY_AREA_AURA_PARTY)
                    return SPELL_AURA;
            }
            break;
        }
        case SPELLFAMILY_SHAMAN:
        {
            if (SpellFamilyFlags & 0x42000000400LL 
                || Id == 23552)
                return SPELL_ELEMENTAL_SHIELD;

            break;
        }
        case SPELLFAMILY_DRUID:
        {
            // Mangle
            if (SpellFamilyFlags & 0x0000044000000000LL)
                return SPELL_DRUID_MANGLE;
        
            break;
        }
        case SPELLFAMILY_ROGUE:
        {
            // Expose Armor (vs Sunder Armor)
            if (SpellFamilyFlags & 0x00000000080000LL)
                return SPELL_ARMOR_REDUCE;
            break;
        }
        case SPELLFAMILY_POTION:
            return GetSpellElixirSpecific();
    }

    // only warlock armor/skin have this (in additional to family cases)
    if (HasVisual(130) && SpellIconID == 89)
    {
        return SPELL_WARLOCK_ARMOR;
    }

    // only hunter aspects have this (but not all aspects in hunter family)
    if( ActiveIconID == 122 && (GetSchoolMask() & SPELL_SCHOOL_MASK_NATURE) &&
        (Attributes & 0x50000) != 0 && (Attributes & 0x9000010) == 0)
    {
        return SPELL_ASPECT;
    }

    for(int i = 0; i < 3; ++i)
    {
        if(Effects[i].Effect == SPELL_EFFECT_APPLY_AURA)
        {
            switch(Effects[i].ApplyAuraName)
            {
                case SPELL_AURA_TRACK_CREATURES:
                case SPELL_AURA_TRACK_RESOURCES:
                case SPELL_AURA_TRACK_STEALTHED:
                    return SPELL_TRACKER;
                case SPELL_AURA_MOD_CHARM:
                case SPELL_AURA_MOD_POSSESS_PET:
                case SPELL_AURA_MOD_POSSESS:
                    return SPELL_CHARM;
            }
        }
    }

    // elixirs can have different families, but potion most ofc.
    if(SpellSpecificType sp = GetSpellElixirSpecific())
        return sp;

    return SPELL_NORMAL;
}

SpellSpecificType SpellInfo::GetSpellElixirSpecific() const
{
    uint32 mask = sSpellMgr->GetSpellElixirMask(Id);
    if (mask & ELIXIR_SHATTRATH_MASK)
        return SPELL_FLASK_ELIXIR;
    else if((mask & ELIXIR_FLASK_MASK)==ELIXIR_FLASK_MASK)
        return SPELL_FLASK_ELIXIR;
    else if(mask & ELIXIR_BATTLE_MASK)
        return SPELL_BATTLE_ELIXIR;
    else if(mask & ELIXIR_GUARDIAN_MASK)
        return SPELL_GUARDIAN_ELIXIR;
    else
        return SPELL_NORMAL;
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
    PowerType = spellEntry->PowerType;
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

    LoadCustomAttributes();
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

bool SpellInfo::CanBeUsedInCombat() const
{
    return !HasAttribute(SPELL_ATTR0_CANT_USED_IN_COMBAT);
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
    switch(Id)
    {
        case 40214:                                     // Dragonmaw Illusion
        case 35480: case 35481: case 35482:             // Human Illusion
        case 35483: case 39824:                         // Human Illusion
        case 17619:
        case 37128:                                     // Doomwalker's Mark of Death
            return true;
    }

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

float SpellInfo::GetMinRange(bool /* positive */) const
{
    if (!RangeEntry)
        return 0.0f;
    /* LK
    if (positive)
        return RangeEntry->minRangeFriend;
    else
        return RangeEntry->minRangeHostile; */
    return RangeEntry->minRange;
}

float SpellInfo::GetMaxRange(bool /* positive */, Unit* caster, Spell* spell) const
{
    if (!RangeEntry)
        return 0.0f;
    float range;
    /* LK
    if (positive)
        range = RangeEntry->maxRangeFriend;
    else
        range = RangeEntry->maxRangeHostile; */
    range = RangeEntry->maxRange;
    if (caster)
        if (Player* modOwner = caster->GetSpellModOwner())
            modOwner->ApplySpellMod(Id, SPELLMOD_RANGE, range, spell);
    return range;
}

bool SpellInfo::IsPassive() const
{
    return HasAttribute(SPELL_ATTR0_PASSIVE);
}

AuraStateType SpellInfo::GetAuraState() const
{
    if (GetSpellSpecific() == SPELL_SEAL)
        return AURA_STATE_JUDGEMENT;

    // Conflagrate aura state
    if (SpellFamilyName == SPELLFAMILY_WARLOCK && (SpellFamilyFlags & SPELLFAMILYFLAG_WARLOCK_IMMOLATE))
        return AURA_STATE_CONFLAGRATE;

    // Swiftmend state on Regrowth & Rejuvenation
    if (SpellFamilyName == SPELLFAMILY_DRUID
        && (SpellFamilyFlags & (SPELLFAMILYFLAG_DRUIDE_REJUVENATION | SPELLFAMILYFLAG_DRUIDE_REGROWTH)))
        return AURA_STATE_SWIFTMEND;

    // Deadly poison
    if (SpellFamilyName == SPELLFAMILY_ROGUE && (SpellFamilyFlags & SPELLFAMILYFLAG_ROGUE_DEADLYPOISON))
        return AURA_STATE_DEADLY_POISON;

    // Faerie Fire (druid versions)
    if ((SpellFamilyName == SPELLFAMILY_DRUID &&
        SpellFamilyFlags & SPELLFAMILYFLAG_DRUIDE_FAERIEFIRE)
        || Id == 35325) //glowing blood (why ?)
        return AURA_STATE_FAERIE_FIRE;

    // Forbearance and Weakened Soul
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; i++)
    {
        if (Effects[i].ApplyAuraName == SPELL_AURA_MECHANIC_IMMUNITY)
        {
            if (Effects[i].MiscValue == MECHANIC_INVULNERABILITY)
                return AURA_STATE_FORBEARANCE;
            if (Effects[i].MiscValue == MECHANIC_SHIELD)
                return AURA_STATE_WEAKENED_SOUL;
        }
    }

    if (SpellFamilyName == SPELLFAMILY_WARRIOR
        && (SpellFamilyFlags == SPELLFAMILYFLAG_WARRIOR_VICTORYRUSH))
        return AURA_STATE_WARRIOR_VICTORY_RUSH;

    switch (Id)
    {
    case 41425: // Hypothermia
        return AURA_STATE_HYPOTHERMIA;
    case 32216: // Victorious
        return AURA_STATE_WARRIOR_VICTORY_RUSH;
    case 1661: // Berserking (troll racial traits)
        return AURA_STATE_BERSERKING;
    default:
        break;
    }

    return AURA_STATE_NONE;
}

bool SpellEffectInfo::HasRadius() const
{
    return RadiusEntry != NULL;
}

float SpellEffectInfo::CalcRadius(Unit* caster, Spell* spell) const
{
    if (!HasRadius())
        return 0.0f;

    float radius = RadiusEntry->RadiusMin;
    if (caster)
    {
        //LK radius += RadiusEntry->RadiusPerLevel * caster->getLevel();
        radius = std::min(radius, RadiusEntry->RadiusMax);
        if (Player* modOwner = caster->GetSpellModOwner())
            modOwner->ApplySpellMod(_spellInfo->Id, SPELLMOD_RADIUS, radius, spell);
    }

    return radius;
}










void SpellInfo::LoadCustomAttributes()
{
    bool auraSpell = true;
    for(uint32 j = 0; j < 3; ++j)
    {
        if(Effects[j].Effect)
            if(Effects[j].Effect != SPELL_EFFECT_APPLY_AURA
            || sSpellMgr->SpellTargetType[Effects[j].TargetA.GetTarget()] != TARGET_TYPE_UNIT_TARGET)
            //ignore target party for now
            {
                auraSpell = false;
                break;
            }
    }
    if(auraSpell)
        AttributesCu |= SPELL_ATTR0_CU_AURA_SPELL;

    for(uint32 j = 0; j < 3; ++j)
    {
        switch(Effects[j].ApplyAuraName)
        {
            case SPELL_AURA_PERIODIC_DAMAGE:
            case SPELL_AURA_PERIODIC_DAMAGE_PERCENT:
            case SPELL_AURA_PERIODIC_LEECH:
                AttributesCu |= SPELL_ATTR0_CU_AURA_DOT;
                break;
            case SPELL_AURA_PERIODIC_HEAL:
            case SPELL_AURA_OBS_MOD_HEALTH:
                AttributesCu |= SPELL_ATTR0_CU_AURA_HOT;
                break;
            case SPELL_AURA_MOD_ROOT:
            //    AttributesCu |= SPELL_ATTR0_CU_AURA_CC;
                AttributesCu |= SPELL_ATTR0_CU_MOVEMENT_IMPAIR;
                break;
            case SPELL_AURA_MOD_DECREASE_SPEED:
                AttributesCu |= SPELL_ATTR0_CU_MOVEMENT_IMPAIR;
                break;
            case SPELL_AURA_MOD_POSSESS:
            case SPELL_AURA_MOD_CONFUSE:
            case SPELL_AURA_MOD_CHARM:
            case SPELL_AURA_MOD_FEAR:
            case SPELL_AURA_MOD_STUN:
                AttributesCu |= SPELL_ATTR0_CU_AURA_CC;
                AttributesCu &= ~SPELL_ATTR0_CU_MOVEMENT_IMPAIR;
                break;
            default:
                break;
        }

        switch(Effects[j].Effect)
        {
            case SPELL_EFFECT_SCHOOL_DAMAGE:
            case SPELL_EFFECT_WEAPON_DAMAGE:
            case SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL:
            case SPELL_EFFECT_NORMALIZED_WEAPON_DMG:
            case SPELL_EFFECT_WEAPON_PERCENT_DAMAGE:
            case SPELL_EFFECT_HEAL:
                AttributesCu |= SPELL_ATTR0_CU_DIRECT_DAMAGE;
                break;
            case SPELL_EFFECT_CHARGE:
                if(!Speed && !SpellFamilyName)
                    Speed = SPEED_CHARGE;
                AttributesCu |= SPELL_ATTR0_CU_CHARGE;
                break;
        }
    }

    if(SpellVisual == 3879)
        AttributesCu |= SPELL_ATTR0_CU_CONE_BACK;

    switch (Id)
    {
         case 27003:
            AttributesCu |= SPELL_ATTR0_CU_IGNORE_ARMOR;
            break;
        case 31041: // Roar's Mangle
            AttributesCu |= SPELL_ATTR0_CU_IGNORE_ARMOR;
            break;
        case 33745: // Rank 1 Lacerate (Druid)
            AttributesCu |= SPELL_ATTR0_CU_IGNORE_ARMOR;
            break;
        case 379: //earth shield heal effect
        case 33076: //Prayer of mending
            AttributesCu |= SPELL_ATTR0_CU_THREAT_GOES_TO_CURRENT_CASTER;
            break;
        case 26029: // dark glare
        case 37433: // spout
        case 43140: case 43215: // flame breath
            AttributesCu |= SPELL_ATTR0_CU_CONE_LINE;
            break;
        case 24340: case 26558: case 28884:     // Meteor
        case 36837: case 38903: case 41276:     // Meteor
        case 26789:                             // Shard of the Fallen Star
        case 31436:                             // Malevolent Cleave
        case 35181:                             // Dive Bomb
        case 40810: case 43267: case 43268:     // Saber Lash
        case 42384:                             // Brutal Swipe
            AttributesCu |= SPELL_ATTR0_CU_SHARE_DAMAGE;
            break;
        case 45150:                             // Meteor Slash
            AttributesCu |= SPELL_ATTR0_CU_SHARE_DAMAGE;
            break;
        case 12723: // Sweeping Strikes proc
            AttributesCu |= SPELL_ATTR0_CU_IGNORE_ARMOR;
            break;
        case 40327: // Atrophy
            AttributesCu |= SPELL_ATTR0_CU_SAME_STACK_DIFF_CASTERS;
            break;
        case 40902: //SPELL_AKAMA_SOUL_RETRIEVE
            AttributesCu |= SPELL_ATTR0_CU_CAN_CHANNEL_DEAD_TARGET;
            break;
        case 45236:
            AttributesCu |= SPELL_ATTR0_CU_IGNORE_ARMOR;
            break;
        case 45232:
            AttributesCu |= SPELL_ATTR0_CU_IGNORE_ARMOR;
            break;
        case 45256:
            AttributesCu |= SPELL_ATTR0_CU_IGNORE_ARMOR;
            break;
        case 45329: // Sacrolash Show nova
            AttributesCu |= SPELL_ATTR0_CU_IGNORE_ARMOR;
            break;
        case 45347: // Sacrolash SPELL_DARK_TOUCHED
            AttributesCu |= SPELL_ATTR0_CU_SAME_STACK_DIFF_CASTERS;
            AttributesCu |= SPELL_ATTR0_CU_IGNORE_ARMOR;
            break;
        case 46771: // SPELL_FLAME_SEAR
            AttributesCu |= SPELL_ATTR0_CU_IGNORE_ARMOR;
            break;
        case 15258:
        case 22959:
        case 12579:
        case 13218:
        case 13222:
        case 13223:
        case 13224:
        case 27189:
        case 33878:
        case 33986:
        case 33987:
        case 33876:
        case 33982:
        case 33983:
        case 43299:
        case 43114:
        case 7386:
        case 7405:
        case 8380:
        case 11596:
        case 11597:
        case 25225:
        case 40520: //shadow of akama slow aura from canalisations (spell 40401)
            AttributesCu |= SPELL_ATTR0_CU_SAME_STACK_DIFF_CASTERS;
            break;
        case 1120:
        case 8288:
        case 8289:
        case 11675:
        case 27217:
        case 41083: //Illidan's shadow demons Paralyze
        case 39123: //purple beam
            AttributesCu |= SPELL_ATTR0_CU_ONE_STACK_PER_CASTER_SPECIAL;
            break;
        case 44335:
            AttributesCu |= SPELL_ATTR0_CU_SAME_STACK_DIFF_CASTERS;
            break;
        case 45271:
            AttributesCu |= SPELL_ATTR0_CU_IGNORE_ARMOR;
            AttributesCu |= SPELL_ATTR0_CU_SAME_STACK_DIFF_CASTERS;
            break;
         case 40845: //Fury
            AttributesCu |= SPELL_ATTR0_CU_SAME_STACK_DIFF_CASTERS;
            break;
        case 41173:
        case 41093:
        case 41084:
        case 34580: //Impale
            AttributesCu |= SPELL_ATTR0_CU_IGNORE_ARMOR;
            break;
        case 33619: // Reflective Shield
            AttributesCu |= SPELL_ATTR0_CU_CANT_BREAK_CC;
            break;
        case 45248:
            AttributesCu |= SPELL_ATTR0_CU_IGNORE_ARMOR;
            break;
        case 39968: //najentus spine explosion
            AttributesCu |= SPELL_ATTR0_CU_AOE_CANT_TARGET_SELF;
            break;
        case 23735:
        case 23736:
        case 23737:
        case 23738:
        case 23766:
        case 23767:
        case 23768:
        case 23769:
            AttributesCu |= SPELL_ATTR0_CU_REMOVE_ON_INSTANCE_ENTER;
            break;
        case 26102: // Sandblast (Ouro)
        case 19272:
        case 21333:
        case 36654:
        case 38813:
        case 38814:
            AttributesCu |= SPELL_ATTR0_CU_CONE_180;
            break;
        case 45770:
        case 19516:
            AttributesCu |= SPELL_ATTR0_CU_SAME_STACK_DIFF_CASTERS;
            break;
    }
}