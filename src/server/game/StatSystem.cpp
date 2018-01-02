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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "Unit.h"
#include "Player.h"
#include "Pet.h"
#include "Creature.h"
#include "SharedDefines.h"
#include "SpellAuras.h"

 /*#######################################
 ########                         ########
 ########    UNIT STAT SYSTEM     ########
 ########                         ########
 #######################################*/

void Unit::UpdateAllResistances()
{
    for (uint8 i = SPELL_SCHOOL_NORMAL; i < MAX_SPELL_SCHOOL; ++i)
        UpdateResistances(i);
}


void Unit::UpdateDamagePhysical(WeaponAttackType attType)
{
    float minDamage = 0.0f;
    float maxDamage = 0.0f;

    CalculateMinMaxDamage(attType, false, true, minDamage, maxDamage);

    switch (attType)
    {
    case BASE_ATTACK:
    default:
        SetStatFloatValue(UNIT_FIELD_MINDAMAGE, minDamage);
        SetStatFloatValue(UNIT_FIELD_MAXDAMAGE, maxDamage);
        break;
    case OFF_ATTACK:
        SetStatFloatValue(UNIT_FIELD_MINOFFHANDDAMAGE, minDamage);
        SetStatFloatValue(UNIT_FIELD_MAXOFFHANDDAMAGE, maxDamage);
        break;
    case RANGED_ATTACK:
        SetStatFloatValue(UNIT_FIELD_MINRANGEDDAMAGE, minDamage);
        SetStatFloatValue(UNIT_FIELD_MAXRANGEDDAMAGE, maxDamage);
        break;
    }
}

void Unit::HandleStatFlatModifier(UnitMods unitMod, UnitModifierFlatType modifierType, float amount, bool apply)
{
    if (unitMod >= UNIT_MOD_END || modifierType >= MODIFIER_TYPE_FLAT_END)
    {
        TC_LOG_ERROR("entities.unit", "ERROR in HandleStatFlatModifier(): non-existing UnitMods or wrong UnitModifierType!");
        return;
    }

    if (!amount)
        return;

    switch (modifierType)
    {
    case BASE_VALUE:
    case TOTAL_VALUE:
        m_auraFlatModifiersGroup[unitMod][modifierType] += apply ? amount : -amount;
        break;
    default:
        break;
    }

    UpdateUnitMod(unitMod);
}

void Unit::ApplyStatPctModifier(UnitMods unitMod, UnitModifierPctType modifierType, float pct)
{
    if (unitMod >= UNIT_MOD_END || modifierType >= MODIFIER_TYPE_PCT_END)
    {
        TC_LOG_ERROR("entities.unit", "ERROR in ApplyStatPctModifier(): non-existing UnitMods or wrong UnitModifierType!");
        return;
    }

    if (!pct)
        return;

    switch (modifierType)
    {
    case BASE_PCT:
    case TOTAL_PCT:
        AddPct(m_auraPctModifiersGroup[unitMod][modifierType], pct);
        break;
    default:
        break;
    }

    UpdateUnitMod(unitMod);
}

void Unit::SetStatFlatModifier(UnitMods unitMod, UnitModifierFlatType modifierType, float val)
{
    if (m_auraFlatModifiersGroup[unitMod][modifierType] == val)
        return;

    m_auraFlatModifiersGroup[unitMod][modifierType] = val;
    UpdateUnitMod(unitMod);
}

void Unit::SetStatPctModifier(UnitMods unitMod, UnitModifierPctType modifierType, float val)
{
    if (m_auraPctModifiersGroup[unitMod][modifierType] == val)
        return;

    m_auraPctModifiersGroup[unitMod][modifierType] = val;
    UpdateUnitMod(unitMod);
}

float Unit::GetFlatModifierValue(UnitMods unitMod, UnitModifierFlatType modifierType) const
{
    if (unitMod >= UNIT_MOD_END || modifierType >= MODIFIER_TYPE_FLAT_END)
    {
        TC_LOG_ERROR("entities.unit", "attempt to access non-existing modifier value from UnitMods!");
        return 0.0f;
    }

    return m_auraFlatModifiersGroup[unitMod][modifierType];
}

float Unit::GetPctModifierValue(UnitMods unitMod, UnitModifierPctType modifierType) const
{
    if (unitMod >= UNIT_MOD_END || modifierType >= MODIFIER_TYPE_PCT_END)
    {
        TC_LOG_ERROR("entities.unit", "attempt to access non-existing modifier value from UnitMods!");
        return 0.0f;
    }

    return m_auraPctModifiersGroup[unitMod][modifierType];
}

void Unit::UpdateUnitMod(UnitMods unitMod)
{
    if (!CanModifyStats())
        return;

    switch (unitMod)
    {
    case UNIT_MOD_STAT_STRENGTH:
    case UNIT_MOD_STAT_AGILITY:
    case UNIT_MOD_STAT_STAMINA:
    case UNIT_MOD_STAT_INTELLECT:
    case UNIT_MOD_STAT_SPIRIT:         UpdateStats(GetStatByAuraGroup(unitMod));  break;

    case UNIT_MOD_ARMOR:               UpdateArmor();           break;
    case UNIT_MOD_HEALTH:              UpdateMaxHealth();       break;

    case UNIT_MOD_MANA:
    case UNIT_MOD_RAGE:
    case UNIT_MOD_FOCUS:
    case UNIT_MOD_ENERGY:
    case UNIT_MOD_HAPPINESS:
#ifdef LICH_KING
    case UNIT_MOD_RUNE:
    case UNIT_MOD_RUNIC_POWER:          
#endif
                                       UpdateMaxPower(GetPowerTypeByAuraGroup(unitMod));          break;

    case UNIT_MOD_RESISTANCE_HOLY:
    case UNIT_MOD_RESISTANCE_FIRE:
    case UNIT_MOD_RESISTANCE_NATURE:
    case UNIT_MOD_RESISTANCE_FROST:
    case UNIT_MOD_RESISTANCE_SHADOW:
    case UNIT_MOD_RESISTANCE_ARCANE:   UpdateResistances(GetSpellSchoolByAuraGroup(unitMod));      break;

    case UNIT_MOD_ATTACK_POWER:        UpdateAttackPowerAndDamage();         break;
    case UNIT_MOD_ATTACK_POWER_RANGED: UpdateAttackPowerAndDamage(true);     break;

    case UNIT_MOD_DAMAGE_MAINHAND:     UpdateDamagePhysical(BASE_ATTACK);    break;
    case UNIT_MOD_DAMAGE_OFFHAND:      UpdateDamagePhysical(OFF_ATTACK);     break;
    case UNIT_MOD_DAMAGE_RANGED:       UpdateDamagePhysical(RANGED_ATTACK);  break;

    default:
        break;
    }
}

void Unit::UpdateDamageDoneMods(WeaponAttackType attackType)
{
    UnitMods unitMod;
    switch (attackType)
    {
    case BASE_ATTACK:
        unitMod = UNIT_MOD_DAMAGE_MAINHAND;
        break;
    case OFF_ATTACK:
        unitMod = UNIT_MOD_DAMAGE_OFFHAND;
        break;
    case RANGED_ATTACK:
        unitMod = UNIT_MOD_DAMAGE_RANGED;
        break;
    default:
        ABORT();
        break;
    }

    float amount = GetTotalAuraModifier(SPELL_AURA_MOD_DAMAGE_DONE, [&](AuraEffect const* aurEff) -> bool
    {
        if (!(aurEff->GetMiscValue() & SPELL_SCHOOL_MASK_NORMAL))
            return false;

        return CheckAttackFitToAuraRequirement(attackType, aurEff);
    });

    SetStatFlatModifier(unitMod, TOTAL_VALUE, amount);

    /* old Sun 
    if (GetTypeId() == TYPEID_PLAYER)
    {

        //apply damage to already equipped weapon
        for (UnitMods mod = UNIT_MOD_DAMAGE_MAINHAND; mod <= UNIT_MOD_DAMAGE_RANGED; mod = (UnitMods)(mod + 1))
            ToPlayer()->HandleStatFlatModifier(mod, TOTAL_VALUE, float(GetModifierValue()), apply);
    }
    */
}

void Unit::UpdateAllDamageDoneMods()
{
    for (uint8 i = BASE_ATTACK; i < MAX_ATTACK; ++i)
        UpdateDamageDoneMods(WeaponAttackType(i));
}

void Unit::UpdateDamagePctDoneMods(WeaponAttackType attackType)
{
    float factor;
    UnitMods unitMod;
    switch (attackType)
    {
    case BASE_ATTACK:
        factor = 1.0f;
        unitMod = UNIT_MOD_DAMAGE_MAINHAND;
        break;
    case OFF_ATTACK:
        // off hand has 50% penalty
        factor = 0.5f;
        unitMod = UNIT_MOD_DAMAGE_OFFHAND;
        break;
    case RANGED_ATTACK:
        factor = 1.0f;
        unitMod = UNIT_MOD_DAMAGE_RANGED;
        break;
    default:
        ABORT();
        break;
    }

    factor *= GetTotalAuraMultiplier(SPELL_AURA_MOD_DAMAGE_PERCENT_DONE, [attackType, this](AuraEffect const* aurEff) -> bool
    {
        /*sunstrider: partially disabled check, we need to check wands too*/
        if (attackType != RANGED_ATTACK && !(aurEff->GetMiscValue() & SPELL_SCHOOL_MASK_NORMAL))
            return false;

        return CheckAttackFitToAuraRequirement(attackType, aurEff);
    });

    if (attackType == OFF_ATTACK)
        factor *= GetTotalAuraMultiplier(SPELL_AURA_MOD_OFFHAND_DAMAGE_PCT, std::bind(&Unit::CheckAttackFitToAuraRequirement, this, attackType, std::placeholders::_1));

    SetStatPctModifier(unitMod, TOTAL_PCT, factor);

    /* Old sun
        // apply generic physical damage bonuses including wand case
        // import me 

        // For show in client
        if(m_target->GetTypeId() == TYPEID_PLAYER)
        {
            //apply damage to already equipped weapon
            for(UnitMods mod = UNIT_MOD_DAMAGE_MAINHAND; mod <= UNIT_MOD_DAMAGE_RANGED; mod = (UnitMods)(mod +1))
                m_target->ToPlayer()->HandleStatModifier(mod, TOTAL_PCT, float(GetModifierValue()),apply);
        }
    */
}

void Unit::UpdateAllDamagePctDoneMods()
{
    for (uint8 i = BASE_ATTACK; i < MAX_ATTACK; ++i)
        UpdateDamagePctDoneMods(WeaponAttackType(i));
}

/*#######################################
########                         ########
########   PLAYERS STAT SYSTEM   ########
########                         ########
#######################################*/

bool Player::UpdateStats(Stats stat)
{
    if(stat > STAT_SPIRIT)
        return false;

    // value = ((base_value * base_pct) + total_value) * total_pct
    float value  = GetTotalStatValue(stat);

    SetStat(stat, int32(value));

    if(stat == STAT_STAMINA || stat == STAT_INTELLECT)
    {
        Pet *pet = GetPet();
        if(pet)
            pet->UpdateStats(stat);
    }

    switch(stat)
    {
        case STAT_STRENGTH:
            UpdateAttackPowerAndDamage();
            UpdateShieldBlockValue();
            break;
        case STAT_AGILITY:
            UpdateArmor();
            UpdateAttackPowerAndDamage(true);
            if(GetClass() == CLASS_ROGUE || GetClass() == CLASS_HUNTER || (GetClass() == CLASS_DRUID && m_form == FORM_CAT))
                UpdateAttackPowerAndDamage();

            UpdateAllCritPercentages();
            UpdateDodgePercentage();
            break;

        case STAT_STAMINA:   UpdateMaxHealth(); break;
        case STAT_INTELLECT:
            UpdateMaxPower(POWER_MANA);
            UpdateAllSpellCritChances();
            UpdateAttackPowerAndDamage(true);               //SPELL_AURA_MOD_RANGED_ATTACK_POWER_OF_STAT_PERCENT, only intellect currently
            UpdateArmor();                                  //SPELL_AURA_MOD_RESISTANCE_OF_INTELLECT_PERCENT, only armor currently
            break;

        case STAT_SPIRIT:
            break;

        default:
            break;
    }
    UpdateSpellDamageAndHealingBonus();
    UpdateManaRegen();
    return true;
}

void Player::UpdateSpellDamageAndHealingBonus()
{
    // Magic damage modifiers implemented in Unit::SpellDamageBonusDone
    // This information for client side use only
    // Get healing bonus for all schools
    SetStatInt32Value(PLAYER_FIELD_MOD_HEALING_DONE_POS, SpellBaseHealingBonusDone(SPELL_SCHOOL_MASK_ALL));
    // Get damage bonus for all schools
    for(int i = SPELL_SCHOOL_HOLY; i < MAX_SPELL_SCHOOL; i++)
        SetStatInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS+i, SpellBaseDamageBonusDone(SpellSchoolMask(1 << i)));
}

bool Player::UpdateAllStats()
{
    for (uint8 i = STAT_STRENGTH; i < MAX_STATS; i++)
    {
        float value = GetTotalStatValue(Stats(i));
        SetStat(Stats(i), (int32)value);
    }

    UpdateAttackPowerAndDamage();
    UpdateAttackPowerAndDamage(true);
    UpdateArmor();
    UpdateMaxHealth();

    for(uint8 i = POWER_MANA; i < MAX_POWERS; i++)
        UpdateMaxPower(Powers(i));

    UpdateAllCritPercentages();
    UpdateAllSpellCritChances();
    UpdateDefenseBonusesMod();
    UpdateShieldBlockValue();
    UpdateSpellDamageAndHealingBonus();
    UpdateManaRegen();
    UpdateExpertise(BASE_ATTACK);
    UpdateExpertise(OFF_ATTACK);
#ifdef LICH_KING
    RecalculateRating(CR_ARMOR_PENETRATION);
#endif
    UpdateAllResistances();

    return true;
}

void Player::UpdateResistances(uint32 school)
{
    if(school > SPELL_SCHOOL_NORMAL)
    {
        float value  = GetTotalAuraModValue(UnitMods(UNIT_MOD_RESISTANCE_START + school));
        SetResistance(SpellSchools(school), int32(value));

        Pet *pet = GetPet();
        if(pet)
            pet->UpdateResistances(school);
    }
    else
        UpdateArmor();
}

void Player::UpdateArmor()
{
    float value = 0.0f;
    UnitMods unitMod = UNIT_MOD_ARMOR;

    value  = GetFlatModifierValue(unitMod, BASE_VALUE);    // base armor (from items)
    value *= GetPctModifierValue(unitMod, BASE_PCT);            // armor percent from items
    
    // Enrage
    if (GetAura(5229,0))
    {
        switch(m_form)
        {
            case FORM_BEAR:
                value *= 0.84f;
                break;
            case FORM_DIREBEAR:
                value *= 0.73f;
                break;
            default:
                break;
        }
    }
    
    value += GetStat(STAT_AGILITY) * 2.0f;                  // armor bonus from stats
    value += GetFlatModifierValue(unitMod, TOTAL_VALUE);

    //add dynamic flat mods
    AuraList const& mResbyIntellect = GetAurasByType(SPELL_AURA_MOD_RESISTANCE_OF_STAT_PERCENT);
    for(auto i : mResbyIntellect)
    {
        Modifier const* mod = i->GetModifier();
        if(mod->m_miscvalue & SPELL_SCHOOL_MASK_NORMAL)
            value += int32(GetStat(Stats(i->GetMiscBValue())) * i->GetModifierValue() / 100.0f);
    }

    value *= GetPctModifierValue(unitMod, TOTAL_PCT);

    SetArmor(int32(value));

    Pet *pet = GetPet();
    if(pet)
        pet->UpdateArmor();
}

float Player::GetHealthBonusFromStamina() const
{
    float stamina = GetStat(STAT_STAMINA);

    float baseStam = stamina < 20 ? stamina : 20;
    float moreStam = stamina - baseStam;

    return baseStam + (moreStam*10.0f);
}

float Player::GetManaBonusFromIntellect() const
{
    float intellect = GetStat(STAT_INTELLECT);

    float baseInt = intellect < 20 ? intellect : 20;
    float moreInt = intellect - baseInt;

    return baseInt + (moreInt*15.0f);
}

void Player::UpdateMaxHealth()
{
    UnitMods unitMod = UNIT_MOD_HEALTH;
    
    float value = GetFlatModifierValue(unitMod, BASE_VALUE) + GetCreateHealth();
    value *= GetPctModifierValue(unitMod, BASE_PCT);
    value += GetFlatModifierValue(unitMod, TOTAL_VALUE) + GetHealthBonusFromStamina();
    value *= GetPctModifierValue(unitMod, TOTAL_PCT);

    SetMaxHealth((uint32)value);
}

void Player::UpdateMaxPower(Powers power)
{
    UnitMods unitMod = UnitMods(UNIT_MOD_POWER_START + power);

    float bonusPower = (power == POWER_MANA) ? GetManaBonusFromIntellect() : 0;
    
    float value = GetFlatModifierValue(unitMod, BASE_VALUE) + GetCreatePowers(power);
    value *= GetPctModifierValue(unitMod, BASE_PCT);
    value += GetFlatModifierValue(unitMod, TOTAL_VALUE) + bonusPower;
    value *= GetPctModifierValue(unitMod, TOTAL_PCT);

    SetMaxPower(power, uint32(value));
}

void Player::UpdateAttackPowerAndDamage(bool ranged )
{
    float val2 = 0.0f;
    float level = float(GetLevel());

    UnitMods unitMod = ranged ? UNIT_MOD_ATTACK_POWER_RANGED : UNIT_MOD_ATTACK_POWER;

    uint16 index = UNIT_FIELD_ATTACK_POWER;
    uint16 index_mod = UNIT_FIELD_ATTACK_POWER_MODS;
    uint16 index_mult = UNIT_FIELD_ATTACK_POWER_MULTIPLIER;

    // For -10 and -20 reduction, see http://wowwiki.wikia.com/wiki/Attack_power?oldid=1715193 
    if(ranged)
    {
        index = UNIT_FIELD_RANGED_ATTACK_POWER;
        index_mod = UNIT_FIELD_RANGED_ATTACK_POWER_MODS;
        index_mult = UNIT_FIELD_RANGED_ATTACK_POWER_MULTIPLIER;

        switch(GetClass())
        {
            case CLASS_HUNTER: 
                val2 = level * 2.0f + GetStat(STAT_AGILITY) - 10.0f;    
                break;
            case CLASS_ROGUE:  
                val2 = level        + GetStat(STAT_AGILITY) - 10.0f;    
                break;
            case CLASS_WARRIOR:
                val2 = level        + GetStat(STAT_AGILITY) - 10.0f;    
                break;
            case CLASS_DRUID:
                switch(m_form)
                {
                    case FORM_CAT:
                    case FORM_BEAR:
                    case FORM_DIREBEAR:
                        val2 = 0.0f; break;
                    default:
                        val2 = GetStat(STAT_AGILITY) - 10.0f; break;
                }
                break;
            default: val2 = GetStat(STAT_AGILITY) - 10.0f; break;
        }
    }
    else
    {
        switch(GetClass())
        {
            case CLASS_WARRIOR:
                val2 = level * 3.0f + GetStat(STAT_STRENGTH) * 2.0f - 20.0f;
                break;
            case CLASS_PALADIN:
                val2 = level * 3.0f + GetStat(STAT_STRENGTH) * 2.0f - 20.0f;
                break;
#ifdef LICH_KING
            case CLASS_DEATH_KNIGHT:
                val2 = level * 3.0f + GetStat(STAT_STRENGTH) * 2.0f - 20.0f;
                break;
#endif
            case CLASS_ROGUE:
                val2 = level * 2.0f + GetStat(STAT_STRENGTH) + GetStat(STAT_AGILITY) - 20.0f;
                break;
            case CLASS_HUNTER:
                val2 = level * 2.0f + GetStat(STAT_STRENGTH) + GetStat(STAT_AGILITY) - 20.0f;
                break;
            case CLASS_SHAMAN:
                val2 = level * 2.0f + GetStat(STAT_STRENGTH) + GetStat(STAT_AGILITY) - 20.0f;
                break;
            case CLASS_DRUID:
            {
                //Check if Predatory Strikes is skilled
                float mLevelMult = 0.0;
                switch(m_form)
                {
                    case FORM_CAT:
                    case FORM_BEAR:
                    case FORM_DIREBEAR:
                    case FORM_MOONKIN:
                    {
                        Unit::AuraList const& mDummy = GetAurasByType(SPELL_AURA_DUMMY);
                        for(auto itr : mDummy)
                        {
                            // Predatory Strikes
                            if (itr->GetSpellInfo()->SpellIconID == 1563)
                            {
                                mLevelMult = itr->GetModifier()->m_amount / 100.0f;
                                break;
                            }
                        }
                        break;
                    }
                }

                switch(m_form)
                {
                    case FORM_CAT:
                        val2 = GetLevel()*(mLevelMult+2.0f) + GetStat(STAT_STRENGTH)*2.0f + GetStat(STAT_AGILITY) - 20.0f; break; // http://wowwiki.wikia.com/wiki/Cat_Form?oldid=1572074
                    case FORM_BEAR:
                    case FORM_DIREBEAR:
                        val2 = GetLevel()*(mLevelMult+3.0f) + GetStat(STAT_STRENGTH)*2.0f - 20.0f; break; //http://wowwiki.wikia.com/wiki/Dire_Bear_Form?oldid=1371376
                    case FORM_MOONKIN:
                        val2 = GetLevel()*(mLevelMult+1.5f) + GetStat(STAT_STRENGTH)*2.0f - 20.0f; break; //http://wowwiki.wikia.com/wiki/Moonkin_Form?oldid=1624337
                    default:
                        val2 = GetStat(STAT_STRENGTH)*2.0f - 20.0f; break;
                }
                break;
            }
            case CLASS_MAGE:    val2 =              GetStat(STAT_STRENGTH)                         - 10.0f; break;
            case CLASS_PRIEST:  val2 =              GetStat(STAT_STRENGTH)                         - 10.0f; break;
            case CLASS_WARLOCK: val2 =              GetStat(STAT_STRENGTH)                         - 10.0f; break;
        }
    }

    SetStatFlatModifier(unitMod, BASE_VALUE, val2);

    float base_attPower = GetFlatModifierValue(unitMod, BASE_VALUE) * GetPctModifierValue(unitMod, BASE_PCT);
    float attPowerMod = GetFlatModifierValue(unitMod, TOTAL_VALUE);

    //add dynamic flat mods
    if( ranged && (GetClassMask() & CLASSMASK_WAND_USERS)==0)
    {
        AuraList const& mRAPbyIntellect = GetAurasByType(SPELL_AURA_MOD_RANGED_ATTACK_POWER_OF_STAT_PERCENT);
        for(auto i : mRAPbyIntellect)
            attPowerMod += int32(GetStat(Stats(i->GetModifier()->m_miscvalue)) * i->GetModifierValue() / 100.0f);
    }
#ifdef LICH_KING
    else
    {
        AuraEffectList const& mAPbyStat = GetAuraEffectsByType(SPELL_AURA_MOD_ATTACK_POWER_OF_STAT_PERCENT);
        for (AuraEffectList::const_iterator i = mAPbyStat.begin(); i != mAPbyStat.end(); ++i)
            attPowerMod += CalculatePct(GetStat(Stats((*i)->GetMiscValue())), (*i)->GetAmount());

        AuraEffectList const& mAPbyArmor = GetAuraEffectsByType(SPELL_AURA_MOD_ATTACK_POWER_OF_ARMOR);
        for (AuraEffectList::const_iterator iter = mAPbyArmor.begin(); iter != mAPbyArmor.end(); ++iter)
            // always: ((*i)->GetModifier()->m_miscvalue == 1 == SPELL_SCHOOL_MASK_NORMAL)
            attPowerMod += int32(GetArmor() / (*iter)->GetAmount());
    }
#endif

    float attPowerMultiplier = GetPctModifierValue(unitMod, TOTAL_PCT) - 1.0f;

    SetUInt32Value(index, (uint32)base_attPower);           //UNIT_FIELD_(RANGED)_ATTACK_POWER field
    SetUInt32Value(index_mod, (uint32)attPowerMod);         //UNIT_FIELD_(RANGED)_ATTACK_POWER_MODS field
    SetFloatValue(index_mult, attPowerMultiplier);          //UNIT_FIELD_(RANGED)_ATTACK_POWER_MULTIPLIER field

    //automatically update weapon damage after attack power modification
    Pet *pet = GetPet();                                //update pet's AP
    if(ranged)
    {
        UpdateDamagePhysical(RANGED_ATTACK);

        if(pet && pet->IsHunterPet()) // At ranged attack change for hunter pet
            pet->UpdateAttackPowerAndDamage();
    }
    else
    {
        UpdateDamagePhysical(BASE_ATTACK);
        if(CanDualWield() && HaveOffhandWeapon())           //allow update offhand damage only if player knows DualWield Spec and has equipped offhand weapon
            UpdateDamagePhysical(OFF_ATTACK);
        if(GetClass() == CLASS_SHAMAN 
#ifdef LICH_KING
            || getClass() == CLASS_PALADIN) 
#endif
            )                      // mental quickness
            UpdateSpellDamageAndHealingBonus();

#ifdef LICH_KING
        if (pet && pet->IsPetGhoul()) // At melee attack power change for DK pet
            pet->UpdateAttackPowerAndDamage();

        Guardian* guardian = GetGuardianPet();
        if (guardian && guardian->IsSpiritWolf()) // At melee attack power change for Shaman feral spirit
            guardian->UpdateAttackPowerAndDamage();
#endif
    }
}

void Player::UpdateShieldBlockValue()
{
    SetUInt32Value(PLAYER_SHIELD_BLOCK, GetShieldBlockValue());
}

bool Player::HasWand() const
{
    Item* weapon = GetWeaponForAttack(RANGED_ATTACK, true);
    if (!weapon)
        return false;

    return weapon->GetTemplate()->SubClass == ITEM_SUBCLASS_WEAPON_WAND;
}

void Player::CalculateMinMaxDamage(WeaponAttackType attType, bool normalized, bool addTotalPct, float& minDamage, float& maxDamage, Unit* target)
{
    UnitMods unitMod;

    switch (attType)
    {
        case BASE_ATTACK:
        default:
            unitMod = UNIT_MOD_DAMAGE_MAINHAND;
            break;
        case OFF_ATTACK:
            unitMod = UNIT_MOD_DAMAGE_OFFHAND;
            break;
        case RANGED_ATTACK:
            unitMod = UNIT_MOD_DAMAGE_RANGED;
            break;
    }

    float weaponMinDamage = GetWeaponDamageRange(attType, MINDAMAGE);
    float weaponMaxDamage = GetWeaponDamageRange(attType, MAXDAMAGE);
    bool wandCase = attType == RANGED_ATTACK  && HasWand(); //wand case, do not use attack power

    float const attackPowerMod = wandCase ? 0.0f : std::max(GetAPMultiplier(attType, normalized), 0.25f);
    
    float baseValue  = GetFlatModifierValue(unitMod, BASE_VALUE) + GetTotalAttackPowerValue(attType) / 14.0f * attackPowerMod;
    float basePct    = GetPctModifierValue(unitMod, BASE_PCT);
    float totalValue = GetFlatModifierValue(unitMod, TOTAL_VALUE);
    float totalPct   = addTotalPct ? GetPctModifierValue(unitMod, TOTAL_PCT) : 1.0f;

    if (IsInFeralForm()) // check if player is druid and in cat or bear forms
    {
        uint8 lvl = std::min(GetLevel(), uint32(60)); //not 100% sure about this but it seems feral damage from vanilla to wotlk has not changed
        //sunstrider formula:
        //exact for cat lvl 70 at least, regeverse engeneered from videos and screenshots, https://docs.google.com/spreadsheets/d/1zcB4S_1186JVIaJhPU0tPiJF8Bjje5BJm5lPMtwHkMc
        //some data from here: https://www.youtube.com/watch?v=uK8Ufk9wsc4 (mind the halaa +5% buff)
        //Also needs more data for level scaling
        float const baseDps = 14 + lvl * 0.5f; //This is an out of hat formula to match the correct cat 44 base dps at lvl 60/70 and ~24 bear dps at lvl 18.
        float const variance = 0.5f;
        weaponMinDamage = baseDps * attackPowerMod;
        weaponMaxDamage = baseDps * (1.0f + variance) * attackPowerMod;

        //Needs more data for bear, I'm using the same formula as for cat for now
        //from https://www.youtube.com/watch?v=U6lVRkJlTzI&feature=youtu.be&t=31s : lvl 18 druid, has 92 ap, 40 min, 53 max
        /* Old TC formula // Proven to be wrong by data
        weaponMinDamage = lvl * 0.85f * attackPowerMod;
        weaponMaxDamage = lvl * 1.25f * attackPowerMod;
        */
    }
    else if (!CanUseAttackType(attType)) // check if player not in form but still can't use (disarm case)
    {
        // cannot use ranged/off attack, set values to 0
        if (attType != BASE_ATTACK)
        {
            minDamage = 0;
            maxDamage = 0;
            return;
        }
        weaponMinDamage = BASE_MINDAMAGE;
        weaponMaxDamage = BASE_MAXDAMAGE;
    }
    else if (!HasWand() && attType == RANGED_ATTACK) // add ammo DPS to ranged damage
    {
        weaponMinDamage += GetAmmoDPS() * attackPowerMod;
        weaponMaxDamage += GetAmmoDPS() * attackPowerMod;
    }

    minDamage = ((weaponMinDamage + baseValue) * basePct + totalValue) * totalPct;
    maxDamage = ((weaponMaxDamage + baseValue) * basePct + totalValue) * totalPct;
}

void Player::UpdateDefenseBonusesMod()
{
    UpdateBlockPercentage();
    UpdateParryPercentage();
    UpdateDodgePercentage();
}

void Player::UpdateBlockPercentage()
{
    // No block
    float value = 0.0f;
    if(CanBlock())
    {
        // Base value
        value = 5.0f;
        // Modify value from defense skill
        value += (int32(GetDefenseSkillValue()) - int32(GetMaxSkillValueForLevel())) * 0.04f;
        // Increase from SPELL_AURA_MOD_BLOCK_PERCENT aura
        value += GetTotalAuraModifier(SPELL_AURA_MOD_BLOCK_PERCENT);
        // Increase from rating
        value += GetRatingBonusValue(CR_BLOCK);
        value = value < 0.0f ? 0.0f : value;
    }
    SetStatFloatValue(PLAYER_BLOCK_PERCENTAGE, value);
}

void Player::UpdateCritPercentage(WeaponAttackType attType)
{
    BaseModGroup modGroup;
    uint16 index;
    CombatRating cr;

    switch(attType)
    {
        case OFF_ATTACK:
            modGroup = OFFHAND_CRIT_PERCENTAGE;
            index = PLAYER_OFFHAND_CRIT_PERCENTAGE;
            cr = CR_CRIT_MELEE;
            break;
        case RANGED_ATTACK:
            modGroup = RANGED_CRIT_PERCENTAGE;
            index = PLAYER_RANGED_CRIT_PERCENTAGE;
            cr = CR_CRIT_RANGED;
            break;
        case BASE_ATTACK:
        default:
            modGroup = CRIT_PERCENTAGE;
            index = PLAYER_CRIT_PERCENTAGE;
            cr = CR_CRIT_MELEE;
            break;
    }

    float value = GetBaseModValue(modGroup, FLAT_MOD) + GetBaseModValue(modGroup, PCT_MOD) + GetRatingBonusValue(cr);

    // Modify crit from weapon skill and maximized defense skill of same level victim difference
    value += (int32(GetWeaponSkillValue(attType)) - int32(GetMaxSkillValueForLevel())) * 0.04f;

    value = std::max(0.0f, value);
    SetStatFloatValue(index, value);
}

void Player::UpdateAllCritPercentages()
{
    float value = GetMeleeCritFromAgility();

    SetBaseModPctValue(CRIT_PERCENTAGE, value);
    SetBaseModPctValue(OFFHAND_CRIT_PERCENTAGE, value);
    SetBaseModPctValue(RANGED_CRIT_PERCENTAGE, value);

    UpdateCritPercentage(BASE_ATTACK);
    UpdateCritPercentage(OFF_ATTACK);
    UpdateCritPercentage(RANGED_ATTACK);
}

void Player::UpdateParryPercentage()
{
    // No parry
    float value = 0.0f;
    if (CanParry())
    {
        // Base parry
        value  = 5.0f;
        // Modify value from defense skill
        value += (int32(GetDefenseSkillValue()) - int32(GetMaxSkillValueForLevel())) * 0.04f;
        // Parry from SPELL_AURA_MOD_PARRY_PERCENT aura
        value += GetTotalAuraModifier(SPELL_AURA_MOD_PARRY_PERCENT);
        // Parry from rating
        value += GetRatingBonusValue(CR_PARRY);
        value = value < 0.0f ? 0.0f : value;
    }
    SetStatFloatValue(PLAYER_PARRY_PERCENTAGE, value);
}

void Player::UpdateDodgePercentage()
{
    // Dodge from agility
    float value = GetDodgeFromAgility();
    // Modify value from defense skill
    value += (int32(GetDefenseSkillValue()) - int32(GetMaxSkillValueForLevel())) * 0.04f;
    // Dodge from SPELL_AURA_MOD_DODGE_PERCENT aura
    value += GetTotalAuraModifier(SPELL_AURA_MOD_DODGE_PERCENT);
    // Dodge from rating
    value += GetRatingBonusValue(CR_DODGE);
    value = value < 0.0f ? 0.0f : value;
    SetStatFloatValue(PLAYER_DODGE_PERCENTAGE, value);
}

void Player::UpdateSpellCritChance(uint32 school)
{
    // For normal school set zero crit chance
    if(school == SPELL_SCHOOL_NORMAL)
    {
        SetFloatValue(PLAYER_SPELL_CRIT_PERCENTAGE1, 0.0f);
        return;
    }
    // For others recalculate it from:
    float crit = 0.0f;
    // Crit from Intellect
    crit += GetSpellCritFromIntellect();
    // Increase crit from SPELL_AURA_MOD_SPELL_CRIT_CHANCE
    crit += GetTotalAuraModifier(SPELL_AURA_MOD_SPELL_CRIT_CHANCE);
    // Increase crit by school from SPELL_AURA_MOD_SPELL_CRIT_CHANCE_SCHOOL
    crit += GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_SPELL_CRIT_CHANCE_SCHOOL, 1<<school);
    // Increase crit from spell crit ratings
    crit += GetRatingBonusValue(CR_CRIT_SPELL);

    // Store crit value
    SetFloatValue(PLAYER_SPELL_CRIT_PERCENTAGE1 + school, crit);
}

void Player::UpdateAllSpellCritChances()
{
    for (int i = SPELL_SCHOOL_NORMAL; i < MAX_SPELL_SCHOOL; i++)
        UpdateSpellCritChance(i);
}

void Player::UpdateExpertise(WeaponAttackType attack)
{
    if(attack==RANGED_ATTACK)
        return;

    int32 expertise = int32(GetRatingBonusValue(CR_EXPERTISE));

    Item *weapon = GetWeaponForAttack(attack);

    AuraList const& expAuras = GetAurasByType(SPELL_AURA_MOD_EXPERTISE);
    for(auto expAura : expAuras)
    {
        // item neutral spell
        if(expAura->GetSpellInfo()->EquippedItemClass == -1)
            expertise += expAura->GetModifierValue();
        // item dependent spell
        else if(weapon && weapon->IsFitToSpellRequirements(expAura->GetSpellInfo()))
            expertise += expAura->GetModifierValue();
    }

    if(expertise < 0)
        expertise = 0;

    switch(attack)
    {
        case BASE_ATTACK: SetUInt32Value(PLAYER_EXPERTISE, expertise);         break;
        case OFF_ATTACK:  SetUInt32Value(PLAYER_OFFHAND_EXPERTISE, expertise); break;
        default: break;
    }
}

void Player::UpdateManaRegen()
{
    float Intellect = GetStat(STAT_INTELLECT);
    // Mana regen from spirit and intellect
    float power_regen = sqrt(Intellect) * OCTRegenMPPerSpirit();
    // Apply PCT bonus from SPELL_AURA_MOD_POWER_REGEN_PERCENT aura on spirit base regen
    power_regen *= GetTotalAuraMultiplierByMiscValue(SPELL_AURA_MOD_POWER_REGEN_PERCENT, POWER_MANA);

    // Mana regen from SPELL_AURA_MOD_POWER_REGEN aura
    float power_regen_mp5 = GetTotalAuraModifierByMiscValue(SPELL_AURA_MOD_POWER_REGEN, POWER_MANA) / 5.0f;

    // Get bonus from SPELL_AURA_MOD_MANA_REGEN_FROM_STAT aura
    AuraList const& regenAura = GetAurasByType(SPELL_AURA_MOD_MANA_REGEN_FROM_STAT);
    for(auto i : regenAura)
    {
        Modifier const* mod = i->GetModifier();
        power_regen_mp5 += GetStat(Stats(mod->m_miscvalue)) * i->GetModifierValue() / 500.0f;
    }

    // Bonus from some dummy auras
    AuraList const& mDummyAuras = GetAurasByType(SPELL_AURA_PERIODIC_DUMMY);
    for(auto mDummyAura : mDummyAuras)
        if(mDummyAura->GetId() == 34074)                          // Aspect of the Viper
        {
            power_regen_mp5 += mDummyAura->GetModifier()->m_amount * Intellect / 500.0f;
            // Add regen bonus from level in this dummy
            power_regen_mp5 += GetLevel() * 35 / 100;
        }

    // Set regen rate in cast state apply only on spirit based regen
    int32 modManaRegenInterrupt = GetTotalAuraModifier(SPELL_AURA_MOD_MANA_REGEN_INTERRUPT);
    if (modManaRegenInterrupt > 100)
        modManaRegenInterrupt = 100;
    SetStatFloatValue(PLAYER_FIELD_MOD_MANA_REGEN_INTERRUPT, power_regen_mp5 + power_regen * modManaRegenInterrupt / 100.0f);

    SetStatFloatValue(PLAYER_FIELD_MOD_MANA_REGEN, power_regen_mp5 + power_regen);
}

void Player::_ApplyAllStatBonuses()
{
    SetCanModifyStats(false);

    _ApplyAllAuraMods();
    _ApplyAllItemMods();

    SetCanModifyStats(true);

    UpdateAllStats();
}

void Player::_RemoveAllStatBonuses()
{
    SetCanModifyStats(false);

    _RemoveAllItemMods();
    _RemoveAllAuraMods();

    SetCanModifyStats(true);

    UpdateAllStats();
}

/*#######################################
########                         ########
########    MOBS STAT SYSTEM     ########
########                         ########
#######################################*/

bool Creature::UpdateStats(Stats /*stat*/)
{
    return true;
}

bool Creature::UpdateAllStats()
{
    UpdateMaxHealth();
    UpdateAttackPowerAndDamage();
    UpdateAttackPowerAndDamage(true);

    for(int i = POWER_MANA; i < MAX_POWERS; ++i)
        UpdateMaxPower(Powers(i));

    UpdateAllResistances(); 

    return true;
}

void Creature::UpdateResistances(uint32 school)
{
    if(school > SPELL_SCHOOL_NORMAL)
    {
        float value  = GetTotalAuraModValue(UnitMods(UNIT_MOD_RESISTANCE_START + school));
        SetResistance(SpellSchools(school), int32(value));
    }
    else
        UpdateArmor();
}

void Creature::UpdateArmor()
{
    float value = GetTotalAuraModValue(UNIT_MOD_ARMOR);
    SetArmor(int32(value));
}

void Creature::UpdateMaxHealth()
{
    float value = GetTotalAuraModValue(UNIT_MOD_HEALTH);
    SetMaxHealth((uint32)value);
}

void Creature::UpdateMaxPower(Powers power)
{
    UnitMods unitMod = UnitMods(UNIT_MOD_POWER_START + power);

    float value  = GetTotalAuraModValue(unitMod);
    SetMaxPower(power, uint32(value));
}

void Creature::UpdateAttackPowerAndDamage(bool ranged)
{
    UnitMods unitMod = ranged ? UNIT_MOD_ATTACK_POWER_RANGED : UNIT_MOD_ATTACK_POWER;

    uint16 index = UNIT_FIELD_ATTACK_POWER;
    uint16 indexMod = UNIT_FIELD_ATTACK_POWER_MODS;
    uint16 indexMulti = UNIT_FIELD_ATTACK_POWER_MULTIPLIER;

    if (ranged)
    {
        index = UNIT_FIELD_RANGED_ATTACK_POWER;
        indexMod = UNIT_FIELD_RANGED_ATTACK_POWER_MODS;
        indexMulti = UNIT_FIELD_RANGED_ATTACK_POWER_MULTIPLIER;
    }
    float baseAttackPower       = GetFlatModifierValue(unitMod, BASE_VALUE) * GetPctModifierValue(unitMod, BASE_PCT);
    float attackPowerMod        = GetFlatModifierValue(unitMod, TOTAL_VALUE);
    float attackPowerMultiplier = GetPctModifierValue(unitMod, TOTAL_PCT) - 1.0f;

    SetInt32Value(index, uint32(baseAttackPower));      // UNIT_FIELD_(RANGED)_ATTACK_POWER
    SetInt32Value(indexMod, uint32(attackPowerMod));    // UNIT_FIELD_(RANGED)_ATTACK_POWER_MODS
    SetFloatValue(indexMulti, attackPowerMultiplier);   // UNIT_FIELD_(RANGED)_ATTACK_POWER_MULTIPLIER

    // automatically update weapon damage after attack power modification
    if (ranged)
        UpdateDamagePhysical(RANGED_ATTACK);
    else
    {
        UpdateDamagePhysical(BASE_ATTACK);
        UpdateDamagePhysical(OFF_ATTACK);
    }
}


void Creature::CalculateMinMaxDamage(WeaponAttackType attType, bool normalized, bool addTotalPct, float& minDamage, float& maxDamage, Unit* target)
{
    float variance = 1.0f;
    UnitMods unitMod;
    switch (attType)
    {
        case BASE_ATTACK:
        default:
            variance = GetCreatureTemplate()->BaseVariance;
            unitMod = UNIT_MOD_DAMAGE_MAINHAND;
            break;
        case OFF_ATTACK:
            variance = GetCreatureTemplate()->BaseVariance;
            unitMod = UNIT_MOD_DAMAGE_OFFHAND;
            break;
        case RANGED_ATTACK:
            variance = GetCreatureTemplate()->RangeVariance;
            unitMod = UNIT_MOD_DAMAGE_RANGED;
            break;
    }

    if (attType == OFF_ATTACK && !HaveOffhandWeapon())
    {
        minDamage = 0.0f;
        maxDamage = 0.0f;
        return;
    }

    float weaponMinDamage = GetWeaponDamageRange(attType, MINDAMAGE);
    float weaponMaxDamage = GetWeaponDamageRange(attType, MAXDAMAGE);

    if (!CanUseAttackType(attType)) // disarm case
    {
        weaponMinDamage = 0.0f;
        weaponMaxDamage = 0.0f;
    }

    /*Custom formula here :
     $minDamage = ( $baseStats["damagebase"] + ($attackpower / 14.0 * $attackSpeedMulti) ) * $creatureModifiers["damage"];
     $maxDamage = $minDamage * (1 + variance);

     weapon min/max damage being already set in Creature::SelectLevel(), formula is a bit changed here, variance being applied 
     to base damage and to attack power separately.

     This formula isn't verified

     $minDamage = ( mindmg + ($attackpower / 14.0 * $attackSpeedMulti ) ) * $creatureModifiers["damage"];
     $maxDamage = ( maxdmg + ($attackpower / 14.0 * $attackSpeedMulti * (1+variance) ) ) * $creatureModifiers["damage"];

     */

    
    float attackPower      = GetTotalAttackPowerValue(attType);
    float attackSpeedMulti = GetAPMultiplier(attType, normalized);
    float baseValue        = GetFlatModifierValue(unitMod, BASE_VALUE) + (attackPower / 14.0f) * variance;
    float basePct          = GetPctModifierValue(unitMod, BASE_PCT) * attackSpeedMulti;
    float totalValue       = GetFlatModifierValue(unitMod, TOTAL_VALUE);
    float totalPct         = addTotalPct ? GetPctModifierValue(unitMod, TOTAL_PCT) : 1.0f;
    float dmgMultiplier    = GetCreatureTemplate()->ModDamage; // = ModDamage * _GetDamageMod(rank);

    minDamage = ((weaponMinDamage + baseValue) * dmgMultiplier * basePct + totalValue) * totalPct;
    maxDamage = ((weaponMaxDamage + baseValue * (1.0f + variance)) * dmgMultiplier * basePct + totalValue) * totalPct;
}

/*#######################################
########                         ########
########    PETS STAT SYSTEM     ########
########                         ########
#######################################*/

#define ENTRY_IMP               416
#define ENTRY_VOIDWALKER        1860
#define ENTRY_SUCCUBUS          1863
#define ENTRY_FELHUNTER         417
#define ENTRY_FELGUARD          17252
#define ENTRY_WATER_ELEMENTAL   510
#define ENTRY_TREANT            1964
#define ENTRY_FIRE_ELEMENTAL    15438
#define ENTRY_SHADOWFIEND       19668

bool Guardian::UpdateStats(Stats stat)
{
    if(stat >= MAX_STATS)
        return false;

    // value = ((base_value * base_pct) + total_value) * total_pct
    float value  = GetTotalStatValue(stat);
    float ownersBonus = 0.0f;

    Unit *owner = GetOwner();
#ifdef LICH_KING
    if ((IsPetGhoul() || IsRisenAlly()) && (stat == STAT_STAMINA || stat == STAT_STRENGTH))
    {
        if (stat == STAT_STAMINA)
            mod = 0.3f; // Default Owner's Stamina scale
        else
            mod = 0.7f; // Default Owner's Strength scale

                        // Check just if owner has Ravenous Dead since it's effect is not an aura
        AuraEffect const* aurEff = owner->GetAuraEffect(SPELL_AURA_MOD_TOTAL_STAT_PERCENTAGE, SPELLFAMILY_DEATHKNIGHT, 3010, 0);
        if (aurEff)
        {
            SpellInfo const* spellInfo = aurEff->GetSpellInfo();                                                 // Then get the SpellProto and add the dummy effect value
            AddPct(mod, spellInfo->Effects[EFFECT_1].CalcValue());                                              // Ravenous Dead edits the original scale
        }
        // Glyph of the Ghoul
        aurEff = owner->GetAuraEffect(58686, 0);
        if (aurEff)
            mod += CalculatePct(1.0f, aurEff->GetAmount());                                                    // Glyph of the Ghoul adds a flat value to the scale mod
        ownersBonus = float(owner->GetStat(stat)) * mod;
        value += ownersBonus;
    } else
#endif
    if ( stat == STAT_STAMINA )
    {
        //1 stamina gives 0.45 stamina untalented (erroneously reported as 0.3 stamina in the Hunter's stamina tooltip), or 0.63 stamina with 2/2 Wild Hunt (LK))s
        float mod = 0.45f;
#ifdef LICH_KING
        if (IsPet())
        {
            PetSpellMap::const_iterator itr = (ToPet()->m_spells.find(62758)); // Wild Hunt rank 1
            if (itr == ToPet()->m_spells.end())
                itr = ToPet()->m_spells.find(62762);                            // Wild Hunt rank 2

            if (itr != ToPet()->m_spells.end())                                 // If pet has Wild Hunt
            {
                SpellInfo const* spellInfo = sSpellMgr->AssertSpellInfo(itr->first); // Then get the SpellProto and add the dummy effect value
                AddPct(mod, spellInfo->Effects[EFFECT_0].CalcValue());
            }
        }
#endif
        ownersBonus = float(owner->GetStat(stat)) * mod;
        value += ownersBonus;
    }
                                                            //warlock's and mage's pets gain 30% of owner's intellect
    else if ( stat == STAT_INTELLECT )
    {
        if (owner->GetClass() == CLASS_WARLOCK || owner->GetClass() == CLASS_MAGE)
        {
            ownersBonus = CalculatePct(owner->GetStat(stat), 30);
            value += ownersBonus;
        }
    }

    SetStat(stat, int32(value));
    m_statFromOwner[stat] = ownersBonus;
    UpdateStatBuffMod(stat);

    switch(stat)
    {
        case STAT_STRENGTH:         UpdateAttackPowerAndDamage();        break;
        case STAT_AGILITY:          UpdateArmor();                       break;
        case STAT_STAMINA:          UpdateMaxHealth();                   break;
        case STAT_INTELLECT:        UpdateMaxPower(POWER_MANA);          break;
        case STAT_SPIRIT:
        default:
            break;
    }

    return true;
}

bool Guardian::UpdateAllStats()
{
    for (int i = STAT_STRENGTH; i < MAX_STATS; i++)
        UpdateStats(Stats(i));

    for(int i = POWER_MANA; i < MAX_POWERS; i++)
        UpdateMaxPower(Powers(i));

    UpdateAllResistances();

    return true;
}

void Guardian::UpdateResistances(uint32 school)
{
    if(school > SPELL_SCHOOL_NORMAL)
    {
        float value  = GetTotalAuraModValue(UnitMods(UNIT_MOD_RESISTANCE_START + school));

        // hunter and warlock pets gain 40% of owner's resistance
        if (IsPet())
            value += float(CalculatePct(m_owner->GetResistance(SpellSchools(school)), 40));

        SetResistance(SpellSchools(school), int32(value));
    }
    else
        UpdateArmor();
}

void Guardian::UpdateArmor()
{
    float value = 0.0f;
    float bonus_armor = 0.0f;
    UnitMods unitMod = UNIT_MOD_ARMOR;

    // hunter and warlock pets gain 35% of owner's armor value
    if(IsPet())
        bonus_armor = float(CalculatePct(m_owner->GetArmor(), 35));

    value  = GetFlatModifierValue(unitMod, BASE_VALUE);
    value *= GetPctModifierValue(unitMod, BASE_PCT);
    value += GetStat(STAT_AGILITY) * 2.0f;
    value += GetFlatModifierValue(unitMod, TOTAL_VALUE) + bonus_armor;
    value *= GetPctModifierValue(unitMod, TOTAL_PCT);

    SetArmor(int32(value));
}

void Guardian::UpdateMaxHealth()
{
    UnitMods unitMod = UNIT_MOD_HEALTH;
    float stamina = GetStat(STAT_STAMINA) - GetCreateStat(STAT_STAMINA);

        float multiplicator;
    switch (GetEntry())
    {
        case ENTRY_IMP:         multiplicator = 8.4f;   break;
        case ENTRY_VOIDWALKER:  multiplicator = 11.0f;  break;
        case ENTRY_SUCCUBUS:    multiplicator = 9.1f;   break;
        case ENTRY_FELHUNTER:   multiplicator = 9.5f;   break;
        case ENTRY_FELGUARD:    multiplicator = 11.0f;  break;
#ifdef LICH_KING
        case ENTRY_BLOODWORM:   multiplicator = 1.0f;   break;
#endif
        default:                multiplicator = 10.0f;  break;
    }

    float value = GetFlatModifierValue(unitMod, BASE_VALUE) + GetCreateHealth();
    value *= GetPctModifierValue(unitMod, BASE_PCT);
    value += GetFlatModifierValue(unitMod, TOTAL_VALUE) + stamina * multiplicator;
    value *= GetPctModifierValue(unitMod, TOTAL_PCT);

    SetMaxHealth((uint32)value);
}

void Guardian::UpdateMaxPower(Powers power)
{
    UnitMods unitMod = UnitMods(UNIT_MOD_POWER_START + power);

    float addValue = (power == POWER_MANA) ? GetStat(STAT_INTELLECT) - GetCreateStat(STAT_INTELLECT) : 0.0f;

    float multiplicator;

    switch (GetEntry())
    {
        case ENTRY_IMP:         multiplicator = 4.95f;  break;
        case ENTRY_VOIDWALKER:
        case ENTRY_SUCCUBUS:
        case ENTRY_FELHUNTER:
        case ENTRY_FELGUARD:    multiplicator = 11.5f;  break;
        default:                multiplicator = 15.0f;  break;
    }

    float value = GetFlatModifierValue(unitMod, BASE_VALUE) + GetCreatePowers(power);
    value *= GetPctModifierValue(unitMod, BASE_PCT);
    value += GetFlatModifierValue(unitMod, TOTAL_VALUE) + addValue * multiplicator;
    value *= GetPctModifierValue(unitMod, TOTAL_PCT);

    SetMaxPower(power, uint32(value));
}

void Guardian::UpdateAttackPowerAndDamage(bool ranged)
{
    if(ranged)
        return;

    float val = 0.0f;
    float bonusAP = 0.0f;
    UnitMods unitMod = UNIT_MOD_ATTACK_POWER;

    if(GetEntry() == 416)                                   // imp's attack power
        val = GetStat(STAT_STRENGTH) - 10.0f;
    else
        val = 2 * GetStat(STAT_STRENGTH) - 20.0f;

    Unit* owner = GetOwner();
    if( owner && owner->GetTypeId()==TYPEID_PLAYER)
    {
        if(IsHunterPet())                      //hunter pets benefit from owner's attack power
        {
            float mod = 1.0f;                                                 //Hunter contribution modifier
            //... TC 
#ifdef LICH_KING
            if (IsPet())
            {
                PetSpellMap::const_iterator itr = ToPet()->m_spells.find(62758);    //Wild Hunt rank 1
                if (itr == ToPet()->m_spells.end())
                    itr = ToPet()->m_spells.find(62762);                            //Wild Hunt rank 2

                if (itr != ToPet()->m_spells.end())                                 // If pet has Wild Hunt
                {
                    SpellInfo const* sProto = sSpellMgr->AssertSpellInfo(itr->first); // Then get the SpellProto and add the dummy effect value
                    mod += CalculatePct(1.0f, sProto->Effects[1].CalcValue());
                }
            }
#endif
            bonusAP = owner->GetTotalAttackPowerValue(RANGED_ATTACK) * 0.22f * mod;
            //TC if (AuraEffect* aurEff = owner->GetAuraEffectOfRankedSpell(34453, EFFECT_1, owner->GetGUID())) // Animal Handler
            if (AuraEffect* aurEff = owner->GetAuraOfRankedSpell(34453, owner->GetGUID(), ObjectGuid::Empty, EFFECT_1_MASK)) // Animal Handler
            {
                AddPct(bonusAP, aurEff->GetAmount());
                AddPct(val, aurEff->GetAmount());
            }
            SetBonusDamage( int32(owner->GetTotalAttackPowerValue(RANGED_ATTACK) * 0.125f));
        }
#ifdef LICH_KING
        else if (IsPetGhoul() || IsRisenAlly()) //ghouls benefit from deathknight's attack power (may be summon pet or not)
        {
            bonusAP = owner->GetTotalAttackPowerValue(BASE_ATTACK) * 0.22f;
            SetBonusDamage(int32(owner->GetTotalAttackPowerValue(BASE_ATTACK) * 0.1287f));
        }
        else if (IsSpiritWolf()) //wolf benefit from shaman's attack power
        {
            float dmg_multiplier = 0.31f;
            if (m_owner->GetAuraEffect(63271, 0)) // Glyph of Feral Spirit
                dmg_multiplier = 0.61f;
            bonusAP = owner->GetTotalAttackPowerValue(BASE_ATTACK) * dmg_multiplier;
            SetBonusDamage(int32(owner->GetTotalAttackPowerValue(BASE_ATTACK) * dmg_multiplier));
        }
#endif
        //demons benefit from warlocks shadow or fire damage
        else if (IsPet())
        {
            int32 fire  = int32(owner->GetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_FIRE)) - owner->GetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_NEG + SPELL_SCHOOL_FIRE);
            int32 shadow = int32(owner->GetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW)) - owner->GetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_NEG + SPELL_SCHOOL_SHADOW);
            int32 maximum  = (fire > shadow) ? fire : shadow;
            if(maximum < 0)
                maximum = 0;
            SetBonusDamage( int32(maximum * 0.15f));
            bonusAP = maximum * 0.57f;
        }
        //water elementals benefit from mage's frost damage
        else if (GetEntry() == ENTRY_WATER_ELEMENTAL)
        {
            int32 frost = int32(owner->GetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_FROST)) - owner->GetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_NEG + SPELL_SCHOOL_FROST);
            if(frost < 0)
                frost = 0;
            SetBonusDamage( int32(frost * 0.4f));
        }
    }

    SetStatFlatModifier(UNIT_MOD_ATTACK_POWER, BASE_VALUE, val + bonusAP);

    //in BASE_VALUE of UNIT_MOD_ATTACK_POWER for creatures we store data of meleeattackpower field in DB

    float base_attPower = GetFlatModifierValue(unitMod, BASE_VALUE) * GetPctModifierValue(unitMod, BASE_PCT);
    float attPowerMod = GetFlatModifierValue(unitMod, TOTAL_VALUE);
    float attPowerMultiplier = GetPctModifierValue(unitMod, TOTAL_PCT) - 1.0f;

    //UNIT_FIELD_(RANGED)_ATTACK_POWER field
    SetUInt32Value(UNIT_FIELD_ATTACK_POWER, (uint32)base_attPower);
    //UNIT_FIELD_(RANGED)_ATTACK_POWER_MODS field
    SetUInt32Value(UNIT_FIELD_ATTACK_POWER_MODS, (uint32)attPowerMod);
    //UNIT_FIELD_(RANGED)_ATTACK_POWER_MULTIPLIER field
    SetFloatValue(UNIT_FIELD_ATTACK_POWER_MULTIPLIER, attPowerMultiplier);

    //automatically update weapon damage after attack power modification
    UpdateDamagePhysical(BASE_ATTACK);
}

void Guardian::UpdateDamagePhysical(WeaponAttackType attType)
{
    if(attType > BASE_ATTACK)
        return;

    float bonusDamage = 0.0f;
    if(Unit* owner = GetOwner())
    {
        //force of nature
        if(GetEntry() == ENTRY_TREANT)
        {
            int32 spellDmg = int32(owner->GetUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_NATURE)) - owner->GetUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_NEG + SPELL_SCHOOL_NATURE);
            if(spellDmg > 0)
                bonusDamage = spellDmg * 0.09f;
        }
        //greater fire elemental
        else if(GetEntry() == ENTRY_FIRE_ELEMENTAL)
        {
            if(Unit* shaman = owner->GetOwner())
            {
                int32 spellDmg = int32(shaman->GetUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_FIRE)) - shaman->GetUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_NEG + SPELL_SCHOOL_FIRE);
                if(spellDmg > 0)
                    bonusDamage = spellDmg * 0.4f;
            }
        }
    }

    UnitMods unitMod = UNIT_MOD_DAMAGE_MAINHAND;

    float att_speed = float(GetAttackTime(BASE_ATTACK))/1000.0f;

    float base_value  = GetFlatModifierValue(unitMod, BASE_VALUE) + GetTotalAttackPowerValue(attType) / 14.0f * att_speed + bonusDamage;
    float base_pct    = GetPctModifierValue(unitMod, BASE_PCT);
    float total_value = GetFlatModifierValue(unitMod, TOTAL_VALUE);
    float total_pct   = GetPctModifierValue(unitMod, TOTAL_PCT);

    float weapon_mindamage = GetWeaponDamageRange(BASE_ATTACK, MINDAMAGE);
    float weapon_maxdamage = GetWeaponDamageRange(BASE_ATTACK, MAXDAMAGE);

    float mindamage = ((base_value + weapon_mindamage) * base_pct + total_value) * total_pct;
    float maxdamage = ((base_value + weapon_maxdamage) * base_pct + total_value) * total_pct;

    //  Pet's base damage changes depending on happiness
    if (IsHunterPet())
    {
        switch(ToPet()->GetHappinessState())
        {
            case HAPPY:
                // 125% of normal damage
                mindamage = mindamage * 1.25;
                maxdamage = maxdamage * 1.25;
                break;
            case CONTENT:
                // 100% of normal damage, nothing to modify
                break;
            case UNHAPPY:
                // 75% of normal damage
                mindamage = mindamage * 0.75;
                maxdamage = maxdamage * 0.75;
                break;
        }
    }

    SetStatFloatValue(UNIT_FIELD_MINDAMAGE, mindamage);
    SetStatFloatValue(UNIT_FIELD_MAXDAMAGE, maxdamage);
}

void Guardian::SetBonusDamage(int32 damage)
{
    m_bonusSpellDamage = damage;
#ifdef LICH_KING
    if (GetOwner()->GetTypeId() == TYPEID_PLAYER)
        GetOwner()->SetUInt32Value(PLAYER_PET_SPELL_POWER, damage);
#endif
}
