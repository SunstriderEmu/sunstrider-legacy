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

#include "Common.h"
#include "SharedDefines.h"
#include "Database/DatabaseEnv.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Opcodes.h"
#include "Log.h"
#include "UpdateMask.h"
#include "World.h"
#include "ObjectMgr.h"
#include "SpellMgr.h"
#include "Player.h"
#include "SkillExtraItems.h"
#include "Unit.h"
#include "CreatureAI.h"
#include "CreatureAIImpl.h"
#include "Spell.h"
#include "DynamicObject.h"
#include "SpellAuras.h"
#include "Group.h"
#include "UpdateData.h"
#include "MapManager.h"
#include "ObjectAccessor.h"
#include "SharedDefines.h"
#include "Pet.h"
#include "GameObject.h"
#include "GossipDef.h"
#include "Creature.h"
#include "Totem.h"
#include "CreatureAI.h"
#include "BattleGround.h"
#include "BattleGroundEY.h"
#include "BattleGroundWS.h"
#include "OutdoorPvPMgr.h"
#include "VMapFactory.h"
#include "Language.h"
#include "SocialMgr.h"
#include "Util.h"
#include "TemporarySummon.h"
#include "ScriptCalls.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "../scripts/ScriptMgr.h"
#include "GameObjectAI.h"
#include "CreatureAINew.h"

#include "precompiled.h"

pEffect SpellEffects[TOTAL_SPELL_EFFECTS]=
{
    &Spell::EffectNULL,                                     //  0
    &Spell::EffectInstaKill,                                //  1 SPELL_EFFECT_INSTAKILL
    &Spell::EffectSchoolDMG,                                //  2 SPELL_EFFECT_SCHOOL_DAMAGE
    &Spell::EffectDummy,                                    //  3 SPELL_EFFECT_DUMMY
    &Spell::EffectUnused,                                   //  4 SPELL_EFFECT_PORTAL_TELEPORT          unused
    &Spell::EffectTeleportUnits,                            //  5 SPELL_EFFECT_TELEPORT_UNITS
    &Spell::EffectApplyAura,                                //  6 SPELL_EFFECT_APPLY_AURA
    &Spell::EffectEnvironmentalDMG,                         //  7 SPELL_EFFECT_ENVIRONMENTAL_DAMAGE
    &Spell::EffectPowerDrain,                               //  8 SPELL_EFFECT_POWER_DRAIN
    &Spell::EffectHealthLeech,                              //  9 SPELL_EFFECT_HEALTH_LEECH
    &Spell::EffectHeal,                                     // 10 SPELL_EFFECT_HEAL
    &Spell::EffectBind,                                     // 11 SPELL_EFFECT_BIND
    &Spell::EffectNULL,                                     // 12 SPELL_EFFECT_PORTAL
    &Spell::EffectUnused,                                   // 13 SPELL_EFFECT_RITUAL_BASE              unused
    &Spell::EffectUnused,                                   // 14 SPELL_EFFECT_RITUAL_SPECIALIZE        unused
    &Spell::EffectUnused,                                   // 15 SPELL_EFFECT_RITUAL_ACTIVATE_PORTAL   unused
    &Spell::EffectQuestComplete,                            // 16 SPELL_EFFECT_QUEST_COMPLETE
    &Spell::EffectWeaponDmg,                                // 17 SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL
    &Spell::EffectResurrect,                                // 18 SPELL_EFFECT_RESURRECT
    &Spell::EffectAddExtraAttacks,                          // 19 SPELL_EFFECT_ADD_EXTRA_ATTACKS
    &Spell::EffectUnused,                                   // 20 SPELL_EFFECT_DODGE                    one spell: Dodge
    &Spell::EffectUnused,                                   // 21 SPELL_EFFECT_EVADE                    one spell: Evade (DND)
    &Spell::EffectParry,                                    // 22 SPELL_EFFECT_PARRY
    &Spell::EffectBlock,                                    // 23 SPELL_EFFECT_BLOCK                    one spell: Block
    &Spell::EffectCreateItem,                               // 24 SPELL_EFFECT_CREATE_ITEM
    &Spell::EffectUnused,                                   // 25 SPELL_EFFECT_WEAPON
    &Spell::EffectUnused,                                   // 26 SPELL_EFFECT_DEFENSE                  one spell: Defense
    &Spell::EffectPersistentAA,                             // 27 SPELL_EFFECT_PERSISTENT_AREA_AURA
    &Spell::EffectSummonType,                               // 28 SPELL_EFFECT_SUMMON
    &Spell::EffectMomentMove,                               // 29 SPELL_EFFECT_LEAP
    &Spell::EffectEnergize,                                 // 30 SPELL_EFFECT_ENERGIZE
    &Spell::EffectWeaponDmg,                                // 31 SPELL_EFFECT_WEAPON_PERCENT_DAMAGE
    &Spell::EffectTriggerMissileSpell,                      // 32 SPELL_EFFECT_TRIGGER_MISSILE
    &Spell::EffectOpenLock,                                 // 33 SPELL_EFFECT_OPEN_LOCK
    &Spell::EffectSummonChangeItem,                         // 34 SPELL_EFFECT_SUMMON_CHANGE_ITEM
    &Spell::EffectApplyAreaAura,                            // 35 SPELL_EFFECT_APPLY_AREA_AURA_PARTY
    &Spell::EffectLearnSpell,                               // 36 SPELL_EFFECT_LEARN_SPELL
    &Spell::EffectUnused,                                   // 37 SPELL_EFFECT_SPELL_DEFENSE            one spell: SPELLDEFENSE (DND)
    &Spell::EffectDispel,                                   // 38 SPELL_EFFECT_DISPEL
    &Spell::EffectUnused,                                   // 39 SPELL_EFFECT_LANGUAGE
    &Spell::EffectDualWield,                                // 40 SPELL_EFFECT_DUAL_WIELD
    &Spell::EffectSummonWild,                               // 41 SPELL_EFFECT_SUMMON_WILD
    &Spell::EffectSummonGuardian,                           // 42 SPELL_EFFECT_SUMMON_GUARDIAN
    &Spell::EffectTeleUnitsFaceCaster,                      // 43 SPELL_EFFECT_TELEPORT_UNITS_FACE_CASTER
    &Spell::EffectLearnSkill,                               // 44 SPELL_EFFECT_SKILL_STEP
    &Spell::EffectAddHonor,                                 // 45 SPELL_EFFECT_ADD_HONOR                honor/pvp related
    &Spell::EffectNULL,                                     // 46 SPELL_EFFECT_SPAWN                    we must spawn pet there
    &Spell::EffectTradeSkill,                               // 47 SPELL_EFFECT_TRADE_SKILL
    &Spell::EffectUnused,                                   // 48 SPELL_EFFECT_STEALTH                  one spell: Base Stealth
    &Spell::EffectUnused,                                   // 49 SPELL_EFFECT_DETECT                   one spell: Detect
    &Spell::EffectTransmitted,                              // 50 SPELL_EFFECT_TRANS_DOOR
    &Spell::EffectUnused,                                   // 51 SPELL_EFFECT_FORCE_CRITICAL_HIT       unused
    &Spell::EffectUnused,                                   // 52 SPELL_EFFECT_GUARANTEE_HIT            one spell: zzOLDCritical Shot
    &Spell::EffectEnchantItemPerm,                          // 53 SPELL_EFFECT_ENCHANT_ITEM
    &Spell::EffectEnchantItemTmp,                           // 54 SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY
    &Spell::EffectTameCreature,                             // 55 SPELL_EFFECT_TAMECREATURE
    &Spell::EffectSummonPet,                                // 56 SPELL_EFFECT_SUMMON_PET
    &Spell::EffectLearnPetSpell,                            // 57 SPELL_EFFECT_LEARN_PET_SPELL
    &Spell::EffectWeaponDmg,                                // 58 SPELL_EFFECT_WEAPON_DAMAGE
    &Spell::EffectOpenSecretSafe,                           // 59 SPELL_EFFECT_OPEN_LOCK_ITEM
    &Spell::EffectProficiency,                              // 60 SPELL_EFFECT_PROFICIENCY
    &Spell::EffectSendEvent,                                // 61 SPELL_EFFECT_SEND_EVENT
    &Spell::EffectPowerBurn,                                // 62 SPELL_EFFECT_POWER_BURN
    &Spell::EffectThreat,                                   // 63 SPELL_EFFECT_THREAT
    &Spell::EffectTriggerSpell,                             // 64 SPELL_EFFECT_TRIGGER_SPELL
    &Spell::EffectUnused,                                   // 65 SPELL_EFFECT_HEALTH_FUNNEL            unused
    &Spell::EffectUnused,                                   // 66 SPELL_EFFECT_POWER_FUNNEL             unused
    &Spell::EffectHealMaxHealth,                            // 67 SPELL_EFFECT_HEAL_MAX_HEALTH
    &Spell::EffectInterruptCast,                            // 68 SPELL_EFFECT_INTERRUPT_CAST
    &Spell::EffectDistract,                                 // 69 SPELL_EFFECT_DISTRACT
    &Spell::EffectPull,                                     // 70 SPELL_EFFECT_PULL                     one spell: Distract Move
    &Spell::EffectPickPocket,                               // 71 SPELL_EFFECT_PICKPOCKET
    &Spell::EffectAddFarsight,                              // 72 SPELL_EFFECT_ADD_FARSIGHT
    &Spell::EffectSummonPossessed,                          // 73 SPELL_EFFECT_SUMMON_POSSESSED
    &Spell::EffectSummonTotem,                              // 74 SPELL_EFFECT_SUMMON_TOTEM
    &Spell::EffectHealMechanical,                           // 75 SPELL_EFFECT_HEAL_MECHANICAL          one spell: Mechanical Patch Kit
    &Spell::EffectSummonObjectWild,                         // 76 SPELL_EFFECT_SUMMON_OBJECT_WILD
    &Spell::EffectScriptEffect,                             // 77 SPELL_EFFECT_SCRIPT_EFFECT
    &Spell::EffectUnused,                                   // 78 SPELL_EFFECT_ATTACK
    &Spell::EffectSanctuary,                                // 79 SPELL_EFFECT_SANCTUARY
    &Spell::EffectAddComboPoints,                           // 80 SPELL_EFFECT_ADD_COMBO_POINTS
    &Spell::EffectUnused,                                   // 81 SPELL_EFFECT_CREATE_HOUSE             one spell: Create House (TEST)
    &Spell::EffectNULL,                                     // 82 SPELL_EFFECT_BIND_SIGHT
    &Spell::EffectDuel,                                     // 83 SPELL_EFFECT_DUEL
    &Spell::EffectStuck,                                    // 84 SPELL_EFFECT_STUCK
    &Spell::EffectSummonPlayer,                             // 85 SPELL_EFFECT_SUMMON_PLAYER
    &Spell::EffectActivateObject,                           // 86 SPELL_EFFECT_ACTIVATE_OBJECT
    &Spell::EffectSummonTotem,                              // 87 SPELL_EFFECT_SUMMON_TOTEM_SLOT1
    &Spell::EffectSummonTotem,                              // 88 SPELL_EFFECT_SUMMON_TOTEM_SLOT2
    &Spell::EffectSummonTotem,                              // 89 SPELL_EFFECT_SUMMON_TOTEM_SLOT3
    &Spell::EffectSummonTotem,                              // 90 SPELL_EFFECT_SUMMON_TOTEM_SLOT4
    &Spell::EffectUnused,                                   // 91 SPELL_EFFECT_THREAT_ALL               one spell: zzOLDBrainwash
    &Spell::EffectEnchantHeldItem,                          // 92 SPELL_EFFECT_ENCHANT_HELD_ITEM
    &Spell::EffectUnused,                                   // 93 SPELL_EFFECT_SUMMON_PHANTASM
    &Spell::EffectSelfResurrect,                            // 94 SPELL_EFFECT_SELF_RESURRECT
    &Spell::EffectSkinning,                                 // 95 SPELL_EFFECT_SKINNING
    &Spell::EffectCharge,                                   // 96 SPELL_EFFECT_CHARGE
    &Spell::EffectSummonCritter,                            // 97 SPELL_EFFECT_SUMMON_CRITTER
    &Spell::EffectKnockBack,                                // 98 SPELL_EFFECT_KNOCK_BACK
    &Spell::EffectDisEnchant,                               // 99 SPELL_EFFECT_DISENCHANT
    &Spell::EffectInebriate,                                //100 SPELL_EFFECT_INEBRIATE
    &Spell::EffectFeedPet,                                  //101 SPELL_EFFECT_FEED_PET
    &Spell::EffectDismissPet,                               //102 SPELL_EFFECT_DISMISS_PET
    &Spell::EffectReputation,                               //103 SPELL_EFFECT_REPUTATION
    &Spell::EffectSummonObject,                             //104 SPELL_EFFECT_SUMMON_OBJECT_SLOT1
    &Spell::EffectSummonObject,                             //105 SPELL_EFFECT_SUMMON_OBJECT_SLOT2
    &Spell::EffectSummonObject,                             //106 SPELL_EFFECT_SUMMON_OBJECT_SLOT3
    &Spell::EffectSummonObject,                             //107 SPELL_EFFECT_SUMMON_OBJECT_SLOT4
    &Spell::EffectDispelMechanic,                           //108 SPELL_EFFECT_DISPEL_MECHANIC
    &Spell::EffectSummonDeadPet,                            //109 SPELL_EFFECT_SUMMON_DEAD_PET
    &Spell::EffectDestroyAllTotems,                         //110 SPELL_EFFECT_DESTROY_ALL_TOTEMS
    &Spell::EffectDurabilityDamage,                         //111 SPELL_EFFECT_DURABILITY_DAMAGE
    &Spell::EffectSummonDemon,                              //112 SPELL_EFFECT_SUMMON_DEMON
    &Spell::EffectResurrectNew,                             //113 SPELL_EFFECT_RESURRECT_NEW
    &Spell::EffectTaunt,                                    //114 SPELL_EFFECT_ATTACK_ME
    &Spell::EffectDurabilityDamagePCT,                      //115 SPELL_EFFECT_DURABILITY_DAMAGE_PCT
    &Spell::EffectSkinPlayerCorpse,                         //116 SPELL_EFFECT_SKIN_PLAYER_CORPSE       one spell: Remove Insignia, bg usage, required special corpse flags...
    &Spell::EffectSpiritHeal,                               //117 SPELL_EFFECT_SPIRIT_HEAL              one spell: Spirit Heal
    &Spell::EffectSkill,                                    //118 SPELL_EFFECT_SKILL                    professions and more
    &Spell::EffectApplyAreaAura,                            //119 SPELL_EFFECT_APPLY_AREA_AURA_PET
    &Spell::EffectUnused,                                   //120 SPELL_EFFECT_TELEPORT_GRAVEYARD       one spell: Graveyard Teleport Test
    &Spell::EffectWeaponDmg,                                //121 SPELL_EFFECT_NORMALIZED_WEAPON_DMG
    &Spell::EffectUnused,                                   //122 SPELL_EFFECT_122                      unused
    &Spell::EffectSendTaxi,                                 //123 SPELL_EFFECT_SEND_TAXI                taxi/flight related (misc value is taxi path id)
    &Spell::EffectPlayerPull,                               //124 SPELL_EFFECT_PLAYER_PULL              opposite of knockback effect (pulls player twoard caster)
    &Spell::EffectModifyThreatPercent,                      //125 SPELL_EFFECT_MODIFY_THREAT_PERCENT
    &Spell::EffectStealBeneficialBuff,                      //126 SPELL_EFFECT_STEAL_BENEFICIAL_BUFF    spell steal effect?
    &Spell::EffectProspecting,                              //127 SPELL_EFFECT_PROSPECTING              Prospecting spell
    &Spell::EffectApplyAreaAura,                            //128 SPELL_EFFECT_APPLY_AREA_AURA_FRIEND
    &Spell::EffectApplyAreaAura,                            //129 SPELL_EFFECT_APPLY_AREA_AURA_ENEMY
    &Spell::EffectRedirectThreat,                           //130 SPELL_EFFECT_REDIRECT_THREAT
    &Spell::EffectUnused,                                   //131 SPELL_EFFECT_131                      used in some test spells
    &Spell::EffectNULL,                                     //132 SPELL_EFFECT_PLAY_MUSIC               sound id in misc value
    &Spell::EffectUnlearnSpecialization,                    //133 SPELL_EFFECT_UNLEARN_SPECIALIZATION   unlearn profession specialization
    &Spell::EffectKillCredit,                               //134 SPELL_EFFECT_KILL_CREDIT              misc value is creature entry
    &Spell::EffectNULL,                                     //135 SPELL_EFFECT_CALL_PET
    &Spell::EffectHealPct,                                  //136 SPELL_EFFECT_HEAL_PCT
    &Spell::EffectEnergisePct,                              //137 SPELL_EFFECT_ENERGIZE_PCT
    &Spell::EffectNULL,                                     //138 SPELL_EFFECT_138                      Leap
    &Spell::EffectUnused,                                   //139 SPELL_EFFECT_139                      unused
    &Spell::EffectForceCast,                                //140 SPELL_EFFECT_FORCE_CAST
    &Spell::EffectForceCastWithValue,                       //141 SPELL_EFFECT_FORCE_CAST_WITH_VALUE
    &Spell::EffectTriggerSpellWithValue,                    //142 SPELL_EFFECT_TRIGGER_SPELL_WITH_VALUE
    &Spell::EffectApplyAreaAura,                            //143 SPELL_EFFECT_APPLY_AREA_AURA_OWNER
    &Spell::EffectKnockBack,                                //144 SPELL_EFFECT_KNOCK_BACK_2             Spectral Blast
    &Spell::EffectPlayerPull,                               //145 SPELL_EFFECT_145                      Black Hole Effect
    &Spell::EffectUnused,                                   //146 SPELL_EFFECT_146                      unused
    &Spell::EffectQuestFail,                                //147 SPELL_EFFECT_QUEST_FAIL               quest fail
    &Spell::EffectUnused,                                   //148 SPELL_EFFECT_148                      unused
    &Spell::EffectNULL,                                     //149 SPELL_EFFECT_149                      swoop
    &Spell::EffectUnused,                                   //150 SPELL_EFFECT_150                      unused
    &Spell::EffectTriggerRitualOfSummoning,                 //151 SPELL_EFFECT_TRIGGER_SPELL_2
    &Spell::EffectNULL,                                     //152 SPELL_EFFECT_152                      summon Refer-a-Friend
    &Spell::EffectNULL,                                     //153 SPELL_EFFECT_CREATE_PET               misc value is creature entry
};

void Spell::EffectNULL(uint32 /*i*/)
{
    
}

void Spell::EffectUnused(uint32 /*i*/)
{
    // NOT USED BY ANY SPELL OR USELESS OR IMPLEMENTED IN DIFFERENT WAY IN MANGOS
}

void Spell::EffectResurrectNew(uint32 i)
{
    if(!unitTarget || unitTarget->isAlive())
        return;

    if(unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    if(!unitTarget->IsInWorld())
        return;

    Player* pTarget = (unitTarget->ToPlayer());

    if(pTarget->isRessurectRequested())       // already have one active request
        return;

    uint32 health = damage;
    uint32 mana = m_spellInfo->EffectMiscValue[i];
    pTarget->setResurrectRequestData(m_caster->GetGUID(), m_caster->GetMapId(), m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), health, mana);
    SendResurrectRequest(pTarget);
}

void Spell::EffectInstaKill(uint32 /*i*/)
{
    if( !unitTarget || !unitTarget->isAlive() )
        return;

    // Demonic Sacrifice
    if(m_spellInfo->Id==18788 && unitTarget->GetTypeId()==TYPEID_UNIT)
    {
        uint32 entry = unitTarget->GetEntry();
        uint32 spellID;
        switch(entry)
        {
            case   416: spellID=18789; break;               //imp
            case   417: spellID=18792; break;               //fellhunter
            case  1860: spellID=18790; break;               //void
            case  1863: spellID=18791; break;               //succubus
            case 17252: spellID=35701; break;               //fellguard
            default:
                sLog.outError("EffectInstaKill: Unhandled creature entry (%u) case.",entry);
                return;
        }

        m_caster->CastSpell(m_caster,spellID,true);
    }

    if(m_caster==unitTarget)                                // prevent interrupt message
        finish();

    WorldPacket data(SMSG_SPELLINSTAKILLLOG, 8+8+4);
    data << uint64(m_caster->GetGUID());
    data << uint64(unitTarget->GetGUID());
    data << uint32(m_spellInfo->Id);
    m_caster->SendMessageToSet(&data, true);
    
    m_caster->DealDamage(unitTarget, unitTarget->GetHealth(), NULL, NODAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
}

void Spell::EffectEnvironmentalDMG(uint32 i)
{
    uint32 absorb = 0;
    uint32 resist = 0;

    // Note: this hack with damage replace required until GO casting not implemented
    // environment damage spells already have around enemies targeting but this not help in case not existed GO casting support
    // currently each enemy selected explicitly and self cast damage, we prevent apply self casted spell bonuses/etc
    damage = m_spellInfo->EffectBasePoints[i]+m_spellInfo->EffectBaseDice[i];

    m_caster->CalcAbsorbResist(m_caster,GetSpellSchoolMask(m_spellInfo), SPELL_DIRECT_DAMAGE, damage, &absorb, &resist, m_spellInfo->Id);

    m_caster->SendSpellNonMeleeDamageLog(m_caster, m_spellInfo->Id, damage, GetSpellSchoolMask(m_spellInfo), absorb, resist, false, 0, false);
    if(m_caster->GetTypeId() == TYPEID_PLAYER)
        (m_caster->ToPlayer())->EnvironmentalDamage(DAMAGE_FIRE,damage);
}

void Spell::EffectSchoolDMG(uint32 effect_idx)
{
    /*if (m_spellInfo->SpellFamilyName == 3 && m_spellInfo->SpellFamilyFlags == 0x400000) { // Pyro
        if (m_caster->ToPlayer()) {
            if (m_caster->HasAura(12043))
                m_caster->RemoveAurasDueToSpell(12043);
            m_caster->ToPlayer()->AddGlobalCooldown(m_spellInfo, this);
        }
    }*/
}

void Spell::SpellDamageSchoolDmg(uint32 effect_idx)
{
    bool addBonusDamage = true;
    if( unitTarget && unitTarget->isAlive())
    {
        switch(m_spellInfo->SpellFamilyName)
        {
            case SPELLFAMILY_GENERIC:
            {
                //Gore
                if(m_spellInfo->SpellIconID == 2269 )
                {
                  damage+= (uint32)unitTarget->GetMap()->rand32()%2 ? damage : 0;
                }

                // Meteor like spells (divided damage to targets)
                if(m_customAttr & SPELL_ATTR_CU_SHARE_DAMAGE)
                {
                    uint32 count = 0;
                    for(std::list<TargetInfo>::iterator ihit= m_UniqueTargetInfo.begin();ihit != m_UniqueTargetInfo.end();++ihit)
                        if(ihit->effectMask & (1<<effect_idx))
                            ++count;

                    damage /= count;                    // divide to all targets
                }

                switch(m_spellInfo->Id)                     // better way to check unknown
                {
                    case 35354: //Hand of Death
                    {
                        if(unitTarget && unitTarget->HasAura(38528,0)) //Protection of Elune
                        {
                            damage = 0;
                        }
                        break;
                    }
                    // Saber lash immune (Shahraz)
                    case 40810:
                    {
                        unitTarget->CastSpell(unitTarget, 43690, true);
                        break;
                    }
                    case 45189: //Dawnblade Attack
                    {
                        damage = 0;
                        break;
                    }
                    // percent from health with min
                    case 25599:                             // Thundercrash
                    {
                        damage = unitTarget->GetHealth() / 2;
                        if(damage < 200)
                            damage = 200;
                        break;
                    }
                    // arcane charge. must only affect demons (also undead?)
                    case 45072:
                    {
                        if(unitTarget->GetCreatureType() != CREATURE_TYPE_DEMON
                            && unitTarget->GetCreatureType() != CREATURE_TYPE_UNDEAD)
                            return;
                        break;
                    }
                    // gruul's shatter
                    case 33671:
                    {
                        // don't damage self and only *ALIVE* players
                        if(unitTarget->GetGUID() == m_caster->GetGUID() || unitTarget->GetTypeId() != TYPEID_PLAYER || !unitTarget->isAlive())
                            return;

                        float radius = GetSpellRadius(sSpellRadiusStore.LookupEntry(m_spellInfo->EffectRadiusIndex[0]));
                        if(!radius) return;
                        float distance = m_caster->GetDistance2d(unitTarget);
                        damage = (distance > radius ) ? 0 : (int32)(m_spellInfo->EffectBasePoints[0]*((radius - distance)/radius));
                        break;
                    }
                    // Cataclysmic Bolt
                    case 38441:
                        damage = unitTarget->GetMaxHealth() / 2;
                        break;
                    // Illidan Flame Burst
                    case 41126:
                    {
                        if (!unitTarget || !unitTarget->isAlive())
                            return;
                            
                        break;
                    }
                    case 31944:
                    {
                        if (!unitTarget)
                            return;
                            
                        if (m_damage > unitTarget->GetHealth()) {
                            if (Creature* archimonde = unitTarget->FindCreatureInGrid(17968, 100.0f, true))
                                archimonde->AI()->KilledUnit(unitTarget);
                        }
                        break;
                    }
                    case 46768:
                    {
                        if (!unitTarget)
                            return;
                            
                        if (!unitTarget->HasAura(45345, 0)) {
                            if (unitTarget->HasAura(45347, 0)) {
                                unitTarget->RemoveAurasDueToSpell(45347);
                                unitTarget->CastSpell(unitTarget, 45345, true);
                            }
                            else
                                unitTarget->CastSpell(unitTarget, 45348, true);
                        }
                        break;
                    }
                    case 37363:
                    {
                        if (!unitTarget)
                            return;

                        if (!m_caster->IsWithinMeleeRange(unitTarget))
                            return;

                        break;
                    }
                    case 45779:
                    {
                        if (!unitTarget)
                            return;

                        if (Unit* summoner = m_caster->GetSummoner())
                            if (unitTarget == summoner)
                                return;

                        break;
                    }
                    case 45664:
                        damage = damage/2;
                        break;
                    case 37433:
                    	if (!unitTarget)
                    	    return;

                    	if (unitTarget->isPet())
                    		return;

                    	break;
                }
                break;
            }

            case SPELLFAMILY_MAGE:
            {
                // Arcane Blast
                if(m_spellInfo->SpellFamilyFlags & 0x20000000LL)
                {
                    m_caster->CastSpell(m_caster,36032,true);
                }
                break;
            }
            case SPELLFAMILY_WARRIOR:
            {
                // Bloodthirst
                if(m_spellInfo->SpellFamilyFlags & 0x40000000000LL)
                {
                    damage = uint32(damage * (m_caster->GetTotalAttackPowerValue(BASE_ATTACK, unitTarget)) / 100);
                }
                // Shield Slam
                else if(m_spellInfo->SpellFamilyFlags & 0x100000000LL)
                    damage += int32(m_caster->GetShieldBlockValue());
                // Victory Rush
                else if(m_spellInfo->SpellFamilyFlags & 0x10000000000LL)
                {
                    damage = uint32(damage * m_caster->GetTotalAttackPowerValue(BASE_ATTACK, unitTarget) / 100);
                    m_caster->ModifyAuraState(AURA_STATE_WARRIOR_VICTORY_RUSH, false);
                }
                break;
            }
            case SPELLFAMILY_WARLOCK:
            {
                // Incinerate Rank 1 & 2
                if((m_spellInfo->SpellFamilyFlags & 0x00004000000000LL) && m_spellInfo->SpellIconID==2128)
                {
                    // Incinerate does more dmg (dmg*0.25) if the target is Immolated.
                    if(unitTarget->HasAuraState(AURA_STATE_IMMOLATE))
                        damage += int32(damage*0.25);
                }

                // Conflagrate - consumes immolate
                if (m_spellInfo->TargetAuraState == AURA_STATE_IMMOLATE)
                {
                    bool found = false;
                    // for caster applied auras only
                    Unit::AuraList const &mPeriodic = unitTarget->GetAurasByType(SPELL_AURA_PERIODIC_DAMAGE);
                    for(Unit::AuraList::const_iterator i = mPeriodic.begin(); i != mPeriodic.end(); ++i)
                    {
                        if( (*i)->GetSpellProto()->SpellFamilyName == SPELLFAMILY_WARLOCK && ((*i)->GetSpellProto()->SpellFamilyFlags & 4) &&
                            (*i)->GetCasterGUID()==m_caster->GetGUID() )
                        {
                            unitTarget->RemoveAurasByCasterSpell((*i)->GetId(), m_caster->GetGUID());
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        SendCastResult(SPELL_FAILED_TARGET_AURASTATE);
                        if (m_caster->GetTypeId() == TYPEID_PLAYER) {
                            m_caster->ToPlayer()->RemoveSpellCooldown(m_spellInfo->Id);
                            
                            WorldPacket data(SMSG_CLEAR_COOLDOWN, (4+8));
                            data << uint32(m_spellInfo->Id);
                            data << uint64(m_caster->GetGUID());
                            (m_caster->ToPlayer())->GetSession()->SendPacket(&data);
                        }
                        return;
                    }
                }
                break;
            }
            case SPELLFAMILY_DRUID:
            {
                // L5 Arcane Charge
				if (m_spellInfo->Id == 41360)
                {
					damage = unitTarget->GetMaxHealth();
                    addBonusDamage = false;
                    break;
                }
                // Ferocious Bite
                else if((m_spellInfo->SpellFamilyFlags & 0x000800000) && m_spellInfo->SpellVisual==6587)
                {
                    // converts each extra point of energy into ($f1+$AP/630) additional damage
                    float multiple = m_caster->GetTotalAttackPowerValue(BASE_ATTACK, unitTarget) / 630 + m_spellInfo->DmgMultiplier[effect_idx];
                    damage += int32(m_caster->GetPower(POWER_ENERGY) * multiple);
                    m_caster->SetPower(POWER_ENERGY,0);
                }
                // Rake
                else if(m_spellInfo->SpellFamilyFlags & 0x0000000000001000LL)
                {
                    damage += int32(m_caster->GetTotalAttackPowerValue(BASE_ATTACK, unitTarget) / 100);
                }
                // Swipe
                else if(m_spellInfo->SpellFamilyFlags & 0x0010000000000000LL)
                {
                    damage += int32(m_caster->GetTotalAttackPowerValue(BASE_ATTACK, unitTarget)*0.08f);
                }
                // Starfire
                else if ( m_spellInfo->SpellFamilyFlags & 0x0004LL )
                {
                    Unit::AuraList const& m_OverrideClassScript = m_caster->GetAurasByType(SPELL_AURA_OVERRIDE_CLASS_SCRIPTS);
                    for(Unit::AuraList::const_iterator i = m_OverrideClassScript.begin(); i != m_OverrideClassScript.end(); ++i)
                    {
                        // Starfire Bonus (caster)
                        switch((*i)->GetModifier()->m_miscvalue)
                        {
                            case 5481:                      // Nordrassil Regalia - bonus
                            {
                                Unit::AuraList const& m_periodicDamageAuras = unitTarget->GetAurasByType(SPELL_AURA_PERIODIC_DAMAGE);
                                for(Unit::AuraList::const_iterator itr = m_periodicDamageAuras.begin(); itr != m_periodicDamageAuras.end(); ++itr)
                                {
                                    // Moonfire or Insect Swarm (target debuff from any casters)
                                    if ( (*itr)->GetSpellProto()->SpellFamilyFlags & 0x00200002LL )
                                    {
                                        int32 mod = (*i)->GetModifier()->m_amount;
                                        damage += damage*mod/100;
                                        break;
                                    }
                                }
                                break;
                            }
                            case 5148:                      //Improved Starfire - Ivory Idol of the Moongoddes Aura
                            {
                                damage += (*i)->GetModifier()->m_amount;
                                break;
                            }
                        }
                    }
                }
                //Mangle Bonus for the initial damage of Lacerate and Rake
                if((m_spellInfo->SpellFamilyFlags==0x0000000000001000LL && m_spellInfo->SpellIconID==494) ||
                    (m_spellInfo->SpellFamilyFlags==0x0000010000000000LL && m_spellInfo->SpellIconID==2246))
                {
                    Unit::AuraList const& mDummyAuras = unitTarget->GetAurasByType(SPELL_AURA_DUMMY);
                    for(Unit::AuraList::const_iterator i = mDummyAuras.begin(); i != mDummyAuras.end(); ++i)
                        if((*i)->GetSpellProto()->SpellFamilyFlags & 0x0000044000000000LL && (*i)->GetSpellProto()->SpellFamilyName==SPELLFAMILY_DRUID)
                        {
                            damage = int32(damage*(100.0f+(*i)->GetModifier()->m_amount)/100.0f);
                            break;
                        }
                }
                break;
            }
            case SPELLFAMILY_ROGUE:
            {
                // Envenom
                if(m_caster->GetTypeId()==TYPEID_PLAYER && (m_spellInfo->SpellFamilyFlags & 0x800000000LL))
                {
                    // consume from stack dozes not more that have combo-points
                    if(uint32 combo = (m_caster->ToPlayer())->GetComboPoints())
                    {
                        // count consumed deadly poison doses at target
                        uint32 doses = 0;

                        // remove consumed poison doses
                        Unit::AuraList const& auras = unitTarget->GetAurasByType(SPELL_AURA_PERIODIC_DAMAGE);
                        for(Unit::AuraList::const_iterator itr = auras.begin(); itr!=auras.end(); ++itr)
                        {
                            // Deadly poison (only attacker applied)
                            if( (*itr)->GetSpellProto()->SpellFamilyName==SPELLFAMILY_ROGUE && ((*itr)->GetSpellProto()->SpellFamilyFlags & 0x10000) &&
                                (*itr)->GetSpellProto()->SpellVisual==5100 && (*itr)->GetCasterGUID()==m_caster->GetGUID() )
                            {
                                doses = (*itr)->GetStackAmount();
                                
                                if ( doses > combo )
									doses = combo;

                                for (int j=0;j<doses;j++)
                                    unitTarget->RemoveSingleAuraFromStack((*itr)->GetId(), (*itr)->GetEffIndex());

                                break;
                            }
                        }

                        damage *= doses;
                        damage += int32((m_caster->ToPlayer())->GetTotalAttackPowerValue(BASE_ATTACK, unitTarget) * 0.03f * doses);

                        // Eviscerate and Envenom Bonus Damage (item set effect)
                        if(m_caster->GetDummyAura(37169))
                            damage += (m_caster->ToPlayer())->GetComboPoints()*40;
                    }
                }
                // Eviscerate
                else if((m_spellInfo->SpellFamilyFlags & 0x00020000LL) && m_caster->GetTypeId()==TYPEID_PLAYER)
                {
                    if(uint32 combo = (m_caster->ToPlayer())->GetComboPoints())
                    {
                        damage += int32(m_caster->GetTotalAttackPowerValue(BASE_ATTACK, unitTarget) * combo * 0.03f);

                        // Eviscerate and Envenom Bonus Damage (item set effect)
                        if(m_caster->GetDummyAura(37169))
                            damage += combo*40;
                    }
                }
                break;
            }
            case SPELLFAMILY_HUNTER:
            {
                // Mongoose Bite
                if((m_spellInfo->SpellFamilyFlags & 0x000000002) && m_spellInfo->SpellVisual==342)
                {
                    damage += int32(m_caster->GetTotalAttackPowerValue(BASE_ATTACK, unitTarget)*0.2);
                }
                // Arcane Shot
                else if((m_spellInfo->SpellFamilyFlags & 0x00000800) && m_spellInfo->maxLevel > 0)
                {
                    damage += int32(m_caster->GetTotalAttackPowerValue(RANGED_ATTACK, unitTarget)*0.15);
                }
                // Steady Shot
                else if(m_spellInfo->SpellFamilyFlags & 0x100000000LL)
                {
                  int32 base = m_caster->GetMap()->irand((int32)m_caster->GetWeaponDamageRange(RANGED_ATTACK, MINDAMAGE),(int32)m_caster->GetWeaponDamageRange(RANGED_ATTACK, MAXDAMAGE));
                    damage += int32(float(base)/m_caster->GetAttackTime(RANGED_ATTACK)*2800 + m_caster->GetTotalAttackPowerValue(RANGED_ATTACK, unitTarget)*0.2f);

                    bool found = false;

                    // check dazed affect
                    Unit::AuraList const& decSpeedList = unitTarget->GetAurasByType(SPELL_AURA_MOD_DECREASE_SPEED);
                    for(Unit::AuraList::const_iterator iter = decSpeedList.begin(); iter != decSpeedList.end(); ++iter)
                    {
                        if((*iter)->GetSpellProto()->SpellIconID==15 && (*iter)->GetSpellProto()->Dispel==0)
                        {
                            found = true;
                            break;
                        }
                    }

                    //TODO: should this be put on taken but not done?
                    if(found)
                        damage += m_spellInfo->EffectBasePoints[1];
                }
                //Explosive Trap Effect
                else if(m_spellInfo->SpellFamilyFlags & 0x00000004)
                {
                    damage += int32(m_caster->GetTotalAttackPowerValue(RANGED_ATTACK, unitTarget)*0.1);
                }
                break;
            }
            case SPELLFAMILY_PALADIN:
            {
                //Judgement of Vengeance
                if((m_spellInfo->SpellFamilyFlags & 0x800000000LL) && m_spellInfo->SpellIconID==2292)
                {
                    uint32 stacks = 0;
                    Unit::AuraList const& auras = unitTarget->GetAurasByType(SPELL_AURA_PERIODIC_DAMAGE);
                    for(Unit::AuraList::const_iterator itr = auras.begin(); itr!=auras.end(); ++itr)
                        if((*itr)->GetId() == 31803 && (*itr)->GetCasterGUID()==m_caster->GetGUID())
                            stacks = (*itr)->GetStackAmount();
                    if(!stacks)
                        //No damage if the target isn't affected by this
                        damage = -1;
                    else
                        damage *= stacks;
                }
                break;
            }
            case SPELLFAMILY_SHAMAN:
            {
                // Lightning and Chain Lightning
                if (m_spellInfo->SpellFamilyFlags & 0x0003LL)
                {
                    Unit::AuraList const& m_OverrideClassScript = m_caster->GetAurasByType(SPELL_AURA_OVERRIDE_CLASS_SCRIPTS);
                    for(Unit::AuraList::const_iterator i = m_OverrideClassScript.begin(); i != m_OverrideClassScript.end(); ++i)
                    {
                        // Increased Lightning Damage
                        switch((*i)->GetModifier()->m_miscvalue)
                        {
                            case 4554:
                            case 5142:
							case 6008:
                            {
                                damage += (*i)->GetModifier()->m_amount;
                                break;
                            }
                        }
                    }
                }
                break;
            }
        }

        if(m_originalCaster && damage > 0 && addBonusDamage)
            damage = m_originalCaster->SpellDamageBonus(unitTarget, m_spellInfo, (uint32)damage, SPELL_DIRECT_DAMAGE);

        m_damage += damage;
    }
}

void Spell::EffectDummy(uint32 i)
{
    if(!unitTarget && !gameObjTarget && !itemTarget)
        return;

    uint32 spell_id = 0;
    int32 bp = 0;

    // selection by spell family
    switch(m_spellInfo->SpellFamilyName)
    {
        case SPELLFAMILY_GENERIC:
        {
            switch(m_spellInfo->Id )
            {
                // Blazerunner Dispel
                case 14247:
                {
                    if (unitTarget && unitTarget->GetTypeId() == TYPEID_UNIT && unitTarget->GetEntry() == 9376)
                        unitTarget->RemoveAurasDueToSpell(13913);
                    break;
                }
                // Bethor's Potion
                case 7669:
                {
                    m_caster->RemoveAurasDueToSpell(7656);
                    break;
                }
                // Self Force Bow
                case 25716:
                {
                    m_caster->HandleEmoteCommand(EMOTE_ONESHOT_BOW);
                    break;
                }
                // Elune's Candle
                case 26374:
                {
                    m_caster->CastSpell(unitTarget, RAND(26622, 26623, 26624, 26625, 26636, 26649), true);
                    break;
                }
                // Melodious Rapture (quest 6661)
                case 21050:
                {
                    if (unitTarget && unitTarget->ToCreature() && unitTarget->GetEntry() == 13016) {
                        if (m_caster->ToPlayer() && m_caster->ToPlayer()->GetQuestStatus(6661) == QUEST_STATUS_INCOMPLETE) {
                            unitTarget->ToCreature()->UpdateEntry(13017);
                            unitTarget->setFaction(m_caster->getFaction());
                            unitTarget->GetMotionMaster()->MoveFollow(m_caster, PET_FOLLOW_DIST, PET_FOLLOW_ANGLE);
                            m_caster->ToPlayer()->KilledMonster(13017, unitTarget->GetGUID());
                        }
                    }
                    
                    break;
                }
                // Placing Smokey's Explosives (quest 6041)
                case 19250:
                {
                    if (gameObjTarget && m_caster->ToPlayer()) {
                        if (i == 0)
                            m_caster->ToPlayer()->KilledMonster(12247, 0);
                    }
                    
                    break;
                }
                // Bucket lands
                case 42339:
                {
                    if (unitTarget && unitTarget->GetTypeId() == TYPEID_PLAYER) {
                        ItemPosCountVec dest;
                        uint8 msg = unitTarget->ToPlayer()->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, 32971, 1);
                        if (msg == EQUIP_ERR_OK) {
                            Item* item = unitTarget->ToPlayer()->StoreNewItem(dest, 32971, true);
                            unitTarget->ToPlayer()->SendNewItem(item, 1, true, false);
                        }
                    }
                    
                    break;
                }
                // Goblin Bomb
                case 23134:
                {
                    if (Creature *bomb = m_caster->SummonCreature(8937, m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), m_caster->GetOrientation(), TEMPSUMMON_DEAD_DESPAWN, 0))
                        bomb->setFaction(m_caster->getFaction());
                    
                    break;
                }
                // Druid Signal
                case 38782:
                {
                    if (m_caster->GetAreaId() == 3831)
                        if (Creature *druid = m_caster->SummonCreature(22423, m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), m_caster->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 45000)) {
                            druid->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                            druid->CastSpell(druid, 39158, false);
                        }
                    break;
                }
                // Expose Razorthorn Root
                case 44935:
                {
                    if (m_caster->GetTypeId() == TYPEID_UNIT && m_caster->GetEntry() == 24922) {
                        if (GameObject* root = m_caster->FindGOInGrid(187073, 15.0f)) {
                            if (!root->isSpawned())
                                break;
                            m_caster->GetMotionMaster()->MovePoint(0, root->GetPositionX(), root->GetPositionY(), root->GetPositionZ());
                            m_caster->SummonGameObject(187072, root->GetPositionX(), root->GetPositionY(), root->GetPositionZ(), root->GetOrientation(), 0, 0, 0, 0, (root->GetRespawnTime()-time(NULL)));
                            root->SetLootState(GO_JUST_DEACTIVATED);
                        }
                        else
                            sLog.outError("EffectDummy: Spell 44935 not casted by the right creature.");
                        break;
                    }
                    break;
                }
                // Wrath of the Astromancer
                case 42784:
                {
                    uint32 count = 0;
                    for(std::list<TargetInfo>::iterator ihit= m_UniqueTargetInfo.begin();ihit != m_UniqueTargetInfo.end();++ihit)
                        if(ihit->effectMask & (1<<i))
                            ++count;

                    damage = 12000; // maybe wrong value
                    damage /= count;

                    SpellEntry const *spellInfo = spellmgr.LookupSpell(42784);

                     // now deal the damage
                    for(std::list<TargetInfo>::iterator ihit= m_UniqueTargetInfo.begin();ihit != m_UniqueTargetInfo.end();++ihit)
                        if(ihit->effectMask & (1<<i))
                            {
                                Unit* casttarget = Unit::GetUnit((*unitTarget), ihit->targetGUID);
                                if(casttarget)
                                    m_caster->DealDamage(casttarget, damage, NULL, SPELL_DIRECT_DAMAGE, SPELL_SCHOOL_MASK_ARCANE, spellInfo, false);
                            }
                    break;
                }
                case 45226:
                {
                    if (m_caster->GetTypeId() == TYPEID_PLAYER) {
                        (m_caster->ToPlayer())->PlaySound(4654, true);
                        (m_caster->ToPlayer())->CastSpell(m_caster, 44762, true);
                    }
                    ((Player*)m_caster)->PlaySound(4654, false);
                    break;
                }
                //Six Demon Bag (thx ArcEmu devs :D)
                case 14537:
                {
                    if (!unitTarget || !unitTarget->isAlive())
                        return;

                    m_caster->CastSpell(unitTarget, RAND(8401,8408,930,118,1680,10159) ,true);
                }break;
                // Gnomish Shrink Ray
                case 13006:
                {
                    if (!unitTarget || !unitTarget->isAlive())
                        return;

                    if(m_caster->GetTypeId() != TYPEID_PLAYER)
                        return;
                        
                    Unit *target = rand()%2 ? unitTarget : m_caster;

                    m_caster->CastSpell(target,rand()%2 ? 13003 : 13004,true,NULL);
                    return;
                }
                // Gnomish Universal Remote
                case 8344:
                {
                    if (!unitTarget || !unitTarget->isAlive())
                        return;

                    if(m_caster->GetTypeId() != TYPEID_PLAYER)
                        return;
                        
                    switch (rand()%5) {
                    case 0:
                    case 1:
                    case 2:
                        m_caster->CastSpell(unitTarget,8345,true,NULL);
                        break;
                    case 3:
                        m_caster->CastSpell(unitTarget,8346,true,NULL);
                        break;
                    case 4:
                        m_caster->CastSpell(unitTarget,8347,true,NULL);
                        break;
                    }
                    
                    return;
                }
                // Encapsulate Voidwalker
                case 29364:
                {
                    if (!unitTarget || unitTarget->GetTypeId() != TYPEID_UNIT || (unitTarget->ToCreature())->isPet()) return;

                    Creature* creatureTarget = unitTarget->ToCreature();
                    GameObject* pGameObj = new GameObject;

                    if (!creatureTarget || !pGameObj) return;

                    if (!pGameObj->Create(objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT,true), 181574, creatureTarget->GetMap(),
                        creatureTarget->GetPositionX(), creatureTarget->GetPositionY(), creatureTarget->GetPositionZ(),
                        creatureTarget->GetOrientation(), 0, 0, 0, 0, 100, 1))
                    {
                        delete pGameObj;
                        return;
                    }

                    pGameObj->SetRespawnTime(0);
                    pGameObj->SetOwnerGUID(m_caster->GetGUID());
                    //pGameObj->SetUInt32Value(GAMEOBJECT_LEVEL, m_caster->getLevel());
                    pGameObj->SetSpellId(m_spellInfo->Id);

                    MapManager::Instance().GetMap(creatureTarget->GetMapId(), pGameObj)->Add(pGameObj);

                    WorldPacket data(SMSG_GAMEOBJECT_SPAWN_ANIM_OBSOLETE, 8);
                    data << uint64(pGameObj->GetGUID());
                    m_caster->SendMessageToSet(&data,true);

                    return;
                }
                // Demon Broiled Surprise
                case 43723:
                {
                    if (!unitTarget || unitTarget->isAlive() || unitTarget->GetTypeId() != TYPEID_UNIT ||
                        (unitTarget->ToCreature())->isPet()) return;

                    Player *player = m_caster->ToPlayer();

                    if (!player) return;

                    player->CastSpell(unitTarget, 43753, true);

                    if (player->GetQuestStatus(11379) == QUEST_STATUS_INCOMPLETE && unitTarget->GetEntry() == 19973)
                        player->CastedCreatureOrGO(19973, unitTarget->GetGUID(), 43723);

                    return;
                }
                case 8063:                                  // Deviate Fish
                {
                    if(m_caster->GetTypeId() != TYPEID_PLAYER)
                        return;

                    uint32 spell_id = 0;
                    switch(m_caster->GetMap()->urand(1,5))
                    {
                        case 1: spell_id = 8064; break;     // Sleepy
                        case 2: spell_id = 8065; break;     // Invigorate
                        case 3: spell_id = 8066; break;     // Shrink
                        case 4: spell_id = 8067; break;     // Party Time!
                        case 5: spell_id = 8068; break;     // Healthy Spirit
                    }
                    m_caster->CastSpell(m_caster,spell_id,true,NULL);
                    return;
                }
                case 8213:                                  // Savory Deviate Delight
                {
                    if(m_caster->GetTypeId() != TYPEID_PLAYER)
                        return;

                    uint32 spell_id = 0;
                    switch(m_caster->GetMap()->urand(1,2))
                    {
                        // Flip Out - ninja
                        case 1: spell_id = (m_caster->getGender() == GENDER_MALE ? 8219 : 8220); break;
                        // Yaaarrrr - pirate
                        case 2: spell_id = (m_caster->getGender() == GENDER_MALE ? 8221 : 8222); break;
                    }
                    m_caster->CastSpell(m_caster,spell_id,true,NULL);
                    return;
                }
                case 8593:                                  // Symbol of life (restore creature to life)
                case 31225:                                 // Shimmering Vessel (restore creature to life)
                {
                    if(!unitTarget || unitTarget->GetTypeId()!=TYPEID_UNIT)
                        return;
                    (unitTarget->ToCreature())->setDeathState(JUST_ALIVED);
                    return;
                }
                case 12162:                                 // Deep wounds
                case 12850:                                 // (now good common check for this spells)
                case 12868:
                {
                    if(!unitTarget)
                        return;

                    float damage;
                    // DW should benefit of attack power, damage percent mods etc.
                    // TODO: check if using offhand damage is correct and if it should be divided by 2
                    if (m_caster->haveOffhandWeapon() && m_caster->getAttackTimer(BASE_ATTACK) > m_caster->getAttackTimer(OFF_ATTACK))
                        damage = (m_caster->GetFloatValue(UNIT_FIELD_MINOFFHANDDAMAGE) + m_caster->GetFloatValue(UNIT_FIELD_MAXOFFHANDDAMAGE))/2;
                    else
                        damage = (m_caster->GetFloatValue(UNIT_FIELD_MINDAMAGE) + m_caster->GetFloatValue(UNIT_FIELD_MAXDAMAGE))/2;

                    switch (m_spellInfo->Id)
                    {
                        case 12850: damage *= 0.2f; break;
                        case 12162: damage *= 0.4f; break;
                        case 12868: damage *= 0.6f; break;
                        default:
                            sLog.outError("Spell::EffectDummy: Spell %u not handled in DW",m_spellInfo->Id);
                            return;
                    };

                    int32 deepWoundsDotBasePoints0 = int32(damage / 4);
                    m_caster->CastCustomSpell(unitTarget, 12721, &deepWoundsDotBasePoints0, NULL, NULL, true, NULL);
                    return;
                }
                case 12975:                                 //Last Stand
                {
                    int32 healthModSpellBasePoints0 = int32(m_caster->GetMaxHealth()*0.3);
                    m_caster->CastCustomSpell(m_caster, 12976, &healthModSpellBasePoints0, NULL, NULL, true, NULL);
                    return;
                }
                case 13120:                                 // net-o-matic
                {
                    if(!unitTarget)
                        return;

                    uint32 spell_id = 0;

                    uint32 roll = m_caster->GetMap()->urand(0, 99);

                    if(roll < 2)                            // 2% for 30 sec self root (off-like chance unknown)
                        spell_id = 16566;
                    else if(roll < 4)                       // 2% for 20 sec root, charge to target (off-like chance unknown)
                        spell_id = 13119;
                    else                                    // normal root
                        spell_id = 13099;

                    m_caster->CastSpell(unitTarget,spell_id,true,NULL);
                    return;
                }
                case 13280:                                 // Gnomish Death Ray
                {
                    if (!unitTarget)
                        return;
                        
                    if (urand(0, 99) < 15)
                        m_caster->CastSpell(m_caster, 13493, true, NULL);    // failure
                    else
                        m_caster->CastSpell(unitTarget, 13279, true, NULL);
                        
                    return;
                }
                case 13567:                                 // Dummy Trigger
                {
                    // can be used for different aura triggering, so select by aura
                    if(!m_triggeredByAuraSpell || !unitTarget)
                        return;

                    switch(m_triggeredByAuraSpell->Id)
                    {
                        case 26467:                         // Persistent Shield
                            m_caster->CastCustomSpell(unitTarget, 26470, &damage, NULL, NULL, true);
                            break;
                        default:
                            sLog.outError("EffectDummy: Non-handled case for spell 13567 for triggered aura %u",m_triggeredByAuraSpell->Id);
                            break;
                    }
                    return;
                }
                case 14185:                                 // Preparation Rogue
                {
                    if(m_caster->GetTypeId()!=TYPEID_PLAYER)
                        return;

                    //immediately finishes the cooldown on certain Rogue abilities
                    const PlayerSpellMap& sp_list = (m_caster->ToPlayer())->GetSpellMap();
                    for (PlayerSpellMap::const_iterator itr = sp_list.begin(); itr != sp_list.end(); ++itr)
                    {
                        uint32 classspell = itr->first;
                        SpellEntry const *spellInfo = spellmgr.LookupSpell(classspell);

                        if (spellInfo->SpellFamilyName == SPELLFAMILY_ROGUE && (spellInfo->SpellFamilyFlags & 0x26000000860LL))
                        {
                            (m_caster->ToPlayer())->RemoveSpellCooldown(classspell);

                            WorldPacket data(SMSG_CLEAR_COOLDOWN, (4+8));
                            data << uint32(classspell);
                            data << uint64(m_caster->GetGUID());
                            (m_caster->ToPlayer())->GetSession()->SendPacket(&data);
                        }
                    }
                    return;
                }
                case 15998:                                 // Capture Worg Pup
                case 29435:                                 // Capture Female Kaliri Hatchling
                {
                    if(!unitTarget || unitTarget->GetTypeId() != TYPEID_UNIT)
                        return;

                    Creature* creatureTarget = unitTarget->ToCreature();
                    creatureTarget->setDeathState(JUST_DIED);
                    creatureTarget->RemoveCorpse();
                    creatureTarget->SetHealth(0);           // just for nice GM-mode view
                    return;
                }
                case 16589:                                 // Noggenfogger Elixir
                {
                    if(m_caster->GetTypeId()!=TYPEID_PLAYER)
                        return;

                    uint32 spell_id = 0;
                    switch(m_caster->GetMap()->urand(1,3))
                    {
                        case 1: spell_id = 16595; break;
                        case 2: spell_id = 16593; break;
                        default:spell_id = 16591; break;
                    }

                    m_caster->CastSpell(m_caster,spell_id,true,NULL);
                    return;
                }
                case 17251:                                 // Spirit Healer Res
                {
                    if(!unitTarget || !m_originalCaster)
                        return;

                    if(m_originalCaster->GetTypeId() == TYPEID_PLAYER)
                    {
                        WorldPacket data(SMSG_SPIRIT_HEALER_CONFIRM, 8);
                        data << unitTarget->GetGUID();
                        (m_originalCaster->ToPlayer())->GetSession()->SendPacket( &data );
                    }
                    return;
                }
                case 17271:                                 // Test Fetid Skull
                {
                    if(!itemTarget && m_caster->GetTypeId()!=TYPEID_PLAYER)
                        return;

                    uint32 spell_id = roll_chance_i(50) ? 17269 : 17270;

                    m_caster->CastSpell(m_caster,spell_id,true,NULL);
                    return;
                }
                case 20577:                                 // Cannibalize
                    if (unitTarget)
                        m_caster->CastSpell(m_caster,20578,false,NULL);
                    return;
                case 23019:                                 // Crystal Prison Dummy DND
                {
                    if(!m_caster->ToCreature() || (m_caster->ToCreature() && m_caster->ToCreature()->GetEntry() != 12396))
                        return;

                    Creature* creatureTarget = m_caster->ToCreature();
                    if(creatureTarget->isPet())
                        return;

                    creatureTarget->setDeathState(JUST_DIED);
                    creatureTarget->RemoveCorpse();
                    creatureTarget->SetHealth(0);                   // just for nice GM-mode view

                    GameObject* Crystal_Prison = m_caster->SummonGameObject(179644, creatureTarget->GetPositionX(), creatureTarget->GetPositionY(), creatureTarget->GetPositionZ(), creatureTarget->GetOrientation(), 0, 0, 0, 0, creatureTarget->GetRespawnTime()-time(NULL));
                    WorldPacket data(SMSG_GAMEOBJECT_SPAWN_ANIM_OBSOLETE, 8);
                    data << uint64(Crystal_Prison->GetGUID());
                    m_caster->SendMessageToSet(&data,true);

                    return;
                }
                case 23074:                                 // Arc. Dragonling
                    if (!m_CastItem) return;
                    m_caster->CastSpell(m_caster,19804,true,m_CastItem);
                    return;
                case 23075:                                 // Mithril Mechanical Dragonling
                    if (!m_CastItem) return;
                    m_caster->CastSpell(m_caster,12749,true,m_CastItem);
                    return;
                case 23076:                                 // Mechanical Dragonling
                    if (!m_CastItem) return;
                    m_caster->CastSpell(m_caster,4073,true,m_CastItem);
                    return;
                case 23133:                                 // Gnomish Battle Chicken
                    if (!m_CastItem) return;
                    m_caster->CastSpell(m_caster,13166,true,m_CastItem);
                    return;
                case 23138:
                {
                    if(!unitTarget)
                        return;

                    float x = unitTarget->GetPositionX();
                    float y = unitTarget->GetPositionY();
                    float z = unitTarget->GetPositionZ();

                    m_caster->Relocate(x, y, z);
                    m_caster->SendMonsterMove(x, y, z, 0);
                    m_caster->CastSpell(unitTarget, 19712, false);
                    if (m_caster->ToCreature())
                        if (m_caster->ToCreature()->getAI())
                            m_caster->ToCreature()->getAI()->doResetThreat();
                    return;
                }
                case 23448:                                 // Ultrasafe Transporter: Gadgetzan - backfires
                {
                  int32 r = m_caster->GetMap()->irand(0, 119);
                    if ( r < 20 )                           // 1/6 polymorph
                        m_caster->CastSpell(m_caster,23444,true);
                    else if ( r < 100 )                     // 4/6 evil twin
                        m_caster->CastSpell(m_caster,23445,true);
                    else                                    // 1/6 miss the target
                        m_caster->CastSpell(m_caster,36902,true);
                    return;
                }
                case 23453:                                 // Ultrasafe Transporter: Gadgetzan
                    if ( roll_chance_i(50) )                // success
                        m_caster->CastSpell(m_caster,23441,true);
                    else                                    // failure
                        m_caster->CastSpell(m_caster,23446,true);
                    return;
                case 23645:                                 // Hourglass Sand
                    m_caster->RemoveAurasDueToSpell(23170);
                    return;
                case 23725:                                 // Gift of Life (warrior bwl trinket)
                    m_caster->CastSpell(m_caster,23782,true);
                    m_caster->CastSpell(m_caster,23783,true);
                    return;
                case 24930:                                 // Hallow's End Candy
                    if(m_caster->GetTypeId()!=TYPEID_PLAYER)
                        return;

                    switch(m_caster->GetMap()->irand(0,3))
                    {
                    case 0:
                        m_caster->CastSpell(m_caster,24927,true); // Ghost
                        break;
                    case 1:
                        m_caster->CastSpell(m_caster,24926,true); // Pirate
                        if(m_caster->getGender() == GENDER_MALE)
                        {
                            m_caster->CastSpell(m_caster,44743,true);
                        }
                        else
                        {
                            m_caster->CastSpell(m_caster,44742,true);
                        }
                        break;
                    case 2:
                        m_caster->CastSpell(m_caster,24925,true); // Skeleton
                        break;
                    case 3:
                        m_caster->CastSpell(m_caster,24924,true); // Huge and Orange
                        break;
                    }
                    return;
                case 25860:                                 // Reindeer Transformation
                {
                    if (!m_caster->HasAuraType(SPELL_AURA_MOUNTED))
                        return;

                    float flyspeed = m_caster->GetSpeedRate(MOVE_FLIGHT);
                    float speed = m_caster->GetSpeedRate(MOVE_RUN);

                    m_caster->RemoveSpellsCausingAura(SPELL_AURA_MOUNTED);

                    //5 different spells used depending on mounted speed and if mount can fly or not
                    if (flyspeed >= 4.1f)
                        m_caster->CastSpell(m_caster, 44827, true); //310% flying Reindeer
                    else if (flyspeed >= 3.8f)
                        m_caster->CastSpell(m_caster, 44825, true); //280% flying Reindeer
                    else if (flyspeed >= 1.6f)
                        m_caster->CastSpell(m_caster, 44824, true); //60% flying Reindeer
                    else if (speed >= 2.0f)
                        m_caster->CastSpell(m_caster, 25859, true); //100% ground Reindeer
                    else
                        m_caster->CastSpell(m_caster, 25858, true); //60% ground Reindeer

                    return;
                }
                //case 26074:                               // Holiday Cheer
                //    return; -- implemented at client side
                case 28006:                                 // Arcane Cloaking
                {
                    if( unitTarget->GetTypeId() == TYPEID_PLAYER )
                        m_caster->CastSpell(unitTarget,29294,true);
                    return;
                }
                case 28730:                                 // Arcane Torrent (Mana)
                {
                    Aura * dummy = m_caster->GetDummyAura(28734);
                    if (dummy)
                    {
                        int32 bp = damage * dummy->GetStackAmount();
                        m_caster->CastCustomSpell(m_caster, 28733, &bp, NULL, NULL, true);
                        m_caster->RemoveAurasDueToSpell(28734);
                    }
                    return;
                }
                //this spell has no effect in dbc (?!), using item_script instead.
                case 29200:                                 // Purify Helboar Meat
                {
                    if( m_caster->GetTypeId() != TYPEID_PLAYER )
                        return;

                    uint32 spell_id = roll_chance_i(50) ? 29277 : 29278;

                    m_caster->CastSpell(m_caster,spell_id,true,NULL);
                    return;
                }
                case 29858:                                 // Soulshatter
                    if (unitTarget && unitTarget->CanHaveThreatList()
                        && unitTarget->getThreatManager().getThreat(m_caster) > 0.0f)
                        m_caster->CastSpell(unitTarget,32835,true);
                    return;
                case 30458:                                 // Nigh Invulnerability
                    if (!m_CastItem) return;
                    if(roll_chance_i(86))                   // success
                        m_caster->CastSpell(m_caster, 30456, true, m_CastItem);
                    else                                    // backfire in 14% casts
                        m_caster->CastSpell(m_caster, 30457, true, m_CastItem);
                    return;
                case 30507:                                 // Poultryizer
                    if (!m_CastItem) return;
                    if(roll_chance_i(80))                   // success
                        m_caster->CastSpell(unitTarget, 30501, true, m_CastItem);
                    else                                    // backfire 20%
                        m_caster->CastSpell(unitTarget, 30504, true, m_CastItem);
                    return;
                case 33060:                                         // Make a Wish
                {
                    if(m_caster->GetTypeId()!=TYPEID_PLAYER)
                        return;

                    uint32 spell_id = 0;

                    switch(m_caster->GetMap()->urand(1,5))
                    {
                        case 1: spell_id = 33053; break;
                        case 2: spell_id = 33057; break;
                        case 3: spell_id = 33059; break;
                        case 4: spell_id = 33062; break;
                        case 5: spell_id = 33064; break;
                    }

                    m_caster->CastSpell(m_caster,spell_id,true,NULL);
                    return;
                }
                case 35686:                                 // Electro-Shock (Electro-Shock Goodness!)
                {
                    if (!unitTarget)
                        return;
                    
                    if (unitTarget->GetTypeId() == TYPEID_UNIT && unitTarget->GetEntry() == 20778)
                    {
                        Creature *cr = NULL;
                        uint8 spawnNum = 6 + rand()%3;
                        for (uint8 i = 0; i < spawnNum; i++) 
                        {
                            cr = unitTarget->SummonCreature(20805, unitTarget->GetPositionX(), unitTarget->GetPositionY(), unitTarget->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 0);
                            if (cr && cr->IsAIEnabled) {
                                cr->AI()->AttackStart(m_caster);
                                if (cr->getAI())
                                    cr->getAI()->attackStart(m_caster);
                            }
                        }
                        m_caster->Kill(unitTarget, false); // Just for the "burst" animation on death....
                        ( unitTarget->ToCreature() )->RemoveCorpse();
                    }
                    if (unitTarget->GetTypeId() == TYPEID_UNIT && unitTarget->GetEntry() == 20501)
                    {
                        Creature *cr = NULL;
                        uint8 spawnNum = 6 + rand()%3;
                        for (uint8 i = 0; i < spawnNum; i++) 
                        {
                            cr = unitTarget->SummonCreature(20806, unitTarget->GetPositionX(), unitTarget->GetPositionY(), unitTarget->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 0);
                            if (cr && cr->IsAIEnabled) {
                                cr->AI()->AttackStart(m_caster);
                                if (cr->getAI())
                                    cr->getAI()->attackStart(m_caster);
                            }
                        }
                        m_caster->Kill(unitTarget, false); // Just for the "burst" animation on death....
                        ( unitTarget->ToCreature() )->RemoveCorpse();
                    }
                    
                    return;
                }
                case 35745:
                {
                    uint32 spell_id;
                    switch(m_caster->GetAreaId())
                    {
                        case 3900: spell_id = 35743; break;
                        case 3742: spell_id = 35744; break;
                        default: return;
                    }

                    m_caster->CastSpell(m_caster,spell_id,true);
                    return;
                }
                case 37674:                                 // Chaos Blast
                {
                    if(!unitTarget)
                        return;

                    int32 basepoints0 = 100;
                    m_caster->CastCustomSpell(unitTarget,37675,&basepoints0,NULL,NULL,true);
                    return;
                }
                case 40109:                                 // Knockdown Fel Cannon: The Bolt
                {
                    unitTarget->CastSpell(unitTarget, 40075, true);
                    return;
                }
                case 40802:                                 // Mingo's Fortune Generator (Mingo's Fortune Giblets)
                {
                    // selecting one from Bloodstained Fortune item
                    uint32 newitemid;
                    switch(m_caster->GetMap()->urand(1,20))
                    {
                        case 1:  newitemid = 32688; break;
                        case 2:  newitemid = 32689; break;
                        case 3:  newitemid = 32690; break;
                        case 4:  newitemid = 32691; break;
                        case 5:  newitemid = 32692; break;
                        case 6:  newitemid = 32693; break;
                        case 7:  newitemid = 32700; break;
                        case 8:  newitemid = 32701; break;
                        case 9:  newitemid = 32702; break;
                        case 10: newitemid = 32703; break;
                        case 11: newitemid = 32704; break;
                        case 12: newitemid = 32705; break;
                        case 13: newitemid = 32706; break;
                        case 14: newitemid = 32707; break;
                        case 15: newitemid = 32708; break;
                        case 16: newitemid = 32709; break;
                        case 17: newitemid = 32710; break;
                        case 18: newitemid = 32711; break;
                        case 19: newitemid = 32712; break;
                        case 20: newitemid = 32713; break;
                        default:
                            return;
                    }

                    DoCreateItem(i,newitemid);
                    return;
                }
                case 47129:                                 // Totemic Beacon (Midsummer Fire Festival)
                {
                    if (i != 0)
                        return;

                    float fDestX, fDestY, fDestZ;
                    m_caster->GetNearPoint(m_caster, fDestX, fDestY, fDestZ, m_caster->GetObjectSize(), 30.0f, 0.0f);
                    if (Creature* pWolf = m_caster->SummonCreature(25324, fDestX, fDestY, fDestZ, 0.0f, TEMPSUMMON_TIMED_DESPAWN, 60000))
                        pWolf->GetMotionMaster()->MoveFollow(m_caster, PET_FOLLOW_DIST, pWolf->GetAngle(m_caster)); 
                    return;
                }
                // Demon Broiled Surprise
                /* FIX ME: Required for correct work implementing implicit target 7 (in pair (22,7))
                case 43723:
                {
                    if (m_caster->GetTypeId() != TYPEID_PLAYER)
                        return;

                    (m_caster->ToPlayer())->CastSpell(unitTarget, 43753, true);
                    return;
                }
                */
                case 46292:                                 // Cataclysm Breath
                {
                    if(!unitTarget)
                        return;

                    uint32 Disease[2] = {46294, 46300};
                    uint32 Curse[2] = {46297, 46298};
                    uint32 Magic[2] = {46299, 46295};
                    uint32 Poison[2] = {46293, 46296};
                   
                    uint8 rands[4];
                   
                    // Select 4 random spells
                    for (uint8 i = 0; i < 4; ++i)
                        rands[i] = rand() % 2;
 
                    m_caster->CastSpell(unitTarget, Disease[rands[0]], true);
                    m_caster->CastSpell(unitTarget, Curse[rands[1]], true);
                    m_caster->CastSpell(unitTarget, Magic[rands[2]], true);
                    m_caster->CastSpell(unitTarget, Poison[rands[3]], true);
                   
                    // Flip the 4 previous random numbers
                    for (uint8 i = 0; i < 4; ++i)
                        rands[i] = (rands[i] + 1) % 2;
                   
                    // Select one of the non-executed spells
                    uint32 Dispell5[4] = { Disease[rands[0]], Curse[rands[1]], Magic[rands[2]], Poison[rands[3]] };
                    m_caster->CastSpell(unitTarget, Dispell5[rand() % 4], true);

                    return;
                }
                case 44875:                                 // Complete Raptor Capture
                {
                    if(!unitTarget || unitTarget->GetTypeId() != TYPEID_UNIT)
                        return;

                    Creature* creatureTarget = unitTarget->ToCreature();

                    creatureTarget->setDeathState(JUST_DIED);
                    creatureTarget->RemoveCorpse();
                    creatureTarget->SetHealth(0);           // just for nice GM-mode view

                    //cast spell Raptor Capture Credit
                    if(Unit * pCaster = m_originalCaster)
                        pCaster->CastSpell(pCaster, 42337, true);
                    return;
                }
                case 37573:                                 //Temporal Phase Modulator
                {
                    if(!unitTarget)
                        return;

                    TemporarySummon* tempSummon = dynamic_cast<TemporarySummon*>(unitTarget);
                    if(!tempSummon)
                        return;

                    uint32 health = tempSummon->GetHealth();
                    const uint32 entry_list[6] = {21821, 21820, 21817};

                    float x = tempSummon->GetPositionX();
                    float y = tempSummon->GetPositionY();
                    float z = tempSummon->GetPositionZ();
                    float o = tempSummon->GetOrientation();

                    tempSummon->UnSummon();

                    Creature* pCreature = m_caster->SummonCreature(entry_list[m_caster->GetMap()->urand(0, 2)], x, y, z, o,TEMPSUMMON_TIMED_OR_DEAD_DESPAWN,180000);
                    if (!pCreature)
                        return;

                    pCreature->SetHealth(health);

                    if(pCreature->IsAIEnabled) {
                        pCreature->AI()->AttackStart(m_caster);
                        if (pCreature->getAI())
                            pCreature->getAI()->attackStart(m_caster);
                    }

                    return;
                }
                case 34665:                                 //Administer Antidote
                {
                    if(!unitTarget || m_caster->GetTypeId() != TYPEID_PLAYER )
                        return;

                    if(!unitTarget)
                        return;

                    TemporarySummon* tempSummon = dynamic_cast<TemporarySummon*>(unitTarget);
                    if(!tempSummon)
                        return;

                    uint32 health = tempSummon->GetHealth();

                    float x = tempSummon->GetPositionX();
                    float y = tempSummon->GetPositionY();
                    float z = tempSummon->GetPositionZ();
                    float o = tempSummon->GetOrientation();
                    tempSummon->UnSummon();

                    Creature* pCreature = m_caster->SummonCreature(16992, x, y, z, o,TEMPSUMMON_TIMED_OR_DEAD_DESPAWN,180000);
                    if (!pCreature)
                        return;

                    pCreature->SetHealth(health);
                    (m_caster->ToPlayer())->KilledMonster(16992,pCreature->GetGUID());

                    if(pCreature->IsAIEnabled) {
                        pCreature->AI()->AttackStart(m_caster);
                        if (pCreature->getAI())
                            pCreature->getAI()->attackStart(m_caster);
                    }

                    return;
                }
                case 34063:                                 //Soul Mirror
                {
                    if(!unitTarget || m_caster->GetTypeId() != TYPEID_PLAYER )
                        return;

                    Creature* pCreature = m_caster->SummonCreature(19480, unitTarget->GetPositionX(), unitTarget->GetPositionY(), unitTarget->GetPositionZ(), unitTarget->GetOrientation(),TEMPSUMMON_TIMED_OR_DEAD_DESPAWN,180000);
                    if (!pCreature)
                        return;

                    unitTarget->Kill(unitTarget, false);
                    if (unitTarget->GetTypeId() == TYPEID_UNIT)
                        (unitTarget->ToCreature())->RemoveCorpse();

                    if(pCreature->IsAIEnabled) {
                        pCreature->AI()->AttackStart(m_caster);
                        if (pCreature->getAI())
                            pCreature->getAI()->attackStart(m_caster);
                    }

                    return;
                }
                case 45030:                                 // Impale Emissary
                {
                    // Emissary of Hate Credit
                    m_caster->CastSpell(m_caster, 45088, true);
                    return;
                }
                case 50243:                                 // Teach Language
                {
                    if(m_caster->GetTypeId() != TYPEID_PLAYER)
                        return;

                    // spell has a 1/3 chance to trigger one of the below
                    if(roll_chance_i(66))
                        return;
                    if((m_caster->ToPlayer())->GetTeam() == ALLIANCE)
                    {
                        // 1000001 - gnomish binary
                        m_caster->CastSpell(m_caster, 50242, true);
                    }
                    else
                    {
                        // 01001000 - goblin binary
                        m_caster->CastSpell(m_caster, 50246, true);
                    }

                    return;
                }
                case 51582:                                 //Rocket Boots Engaged (Rocket Boots Xtreme and Rocket Boots Xtreme Lite)
                {
                    if(m_caster->GetTypeId() != TYPEID_PLAYER)
                        return;

                    if(BattleGround* bg = (m_caster->ToPlayer())->GetBattleGround())
                        bg->EventPlayerDroppedFlag(m_caster->ToPlayer());

                    m_caster->CastSpell(m_caster, 30452, true, NULL);
                    return;
                }
                case 41170:
                {
                    if(unitTarget->GetTypeId() != TYPEID_PLAYER)
                        return;
                        
                    unitTarget->CastSpell(unitTarget, 6945, true);
                    return;
                }
                case 41248:
                {
                    int32 heal = damage;
                    if (heal <= 0)
                        return;

                    m_caster->CastCustomSpell(m_caster, 41249, &heal, NULL, NULL, true, NULL);
                    return;
                }
                case 38920:
                {
                    if (Player* player = m_caster->GetCharmerOrOwnerPlayerOrPlayerItself()) {
                        if (Creature* west = m_caster->FindCreatureInGrid(22348, 12.0f, true))
                            player->KilledMonster(22348, 0);
                        else if (Creature* center = m_caster->FindCreatureInGrid(22350, 12.0f, true))
                            player->KilledMonster(22350, 0);
                        else if (Creature* east = m_caster->FindCreatureInGrid(22351, 12.0f, true))
                            player->KilledMonster(22351, 0);
                    }
                    
                    return;
                }
                case 25952:
                {
                    if (unitTarget && unitTarget->ToCreature())
                        unitTarget->ToCreature()->DisappearAndDie();
                        
                    return;
                }
                case 42287:
                {
                    if (gameObjTarget)
                        gameObjTarget->SetLootState(GO_JUST_DEACTIVATED);
                        
                    if (m_caster)
                        m_caster->CastSpell(m_caster, 42288, false);
                        
                    return;
                }
                case 28441:
                {
                    if (!unitTarget || unitTarget->GetTypeId() != TYPEID_UNIT)
                        return;

                    if (unitTarget->ToCreature()->isPet())
                        return;
                        
                    if (unitTarget->ToCreature()->GetReactState() == REACT_PASSIVE)
                        return;
                    
                    if (unitTarget->GetEntry() != 3976 && unitTarget->GetEntry() != 4542) {
                        unitTarget->GetMotionMaster()->MoveIdle();
                        unitTarget->SetFacing(0, m_caster);
                        unitTarget->SendMovementFlagUpdate();
                        unitTarget->ToCreature()->SetReactState(REACT_PASSIVE);
                        unitTarget->CastSpell(unitTarget, 39656, true);
                    }

                    return;
                }
                // Egg Explosion
                case 19593:
                {
                    if (!unitTarget || !m_caster)
                        return;
                    int32 bp = 45000 - (unitTarget->GetDistance(m_caster) * 2500);
                    m_caster->CastCustomSpell(unitTarget, 5255, &bp, NULL, NULL, true);
                    
                    return;
                }
            }

            //All IconID Check in there
            switch(m_spellInfo->SpellIconID)
            {
                // Berserking (troll racial traits)
                case 1661:
                {
                    uint32 healthPerc = uint32((float(m_caster->GetHealth())/m_caster->GetMaxHealth())*100);
                    int32 melee_mod = 10;
                    int32 spell_mod = 12;
                    if (healthPerc <= 40) {
                        melee_mod = 30;
                        spell_mod = 40;
                    }
                    if (healthPerc < 100 && healthPerc > 40) {
                        melee_mod = 10+(100-healthPerc)/3;
                        spell_mod = melee_mod + 1;
                    }

                    int32 hasteModBasePoints0 = melee_mod;          // (EffectBasePoints[0]+1)-1+(5-melee_mod) = (melee_mod-1+1)-1+5-melee_mod = 5-1
                    int32 hasteModBasePoints1 = (5-melee_mod);
                    int32 hasteModBasePoints2 = spell_mod;

                    // FIXME: custom spell required this aura state by some unknown reason, we not need remove it anyway
                    m_caster->ModifyAuraState(AURA_STATE_BERSERKING,true);
                    m_caster->CastCustomSpell(m_caster,26635,&hasteModBasePoints0,&hasteModBasePoints1,&hasteModBasePoints2,true,NULL);
                    return;
                }
            }
            break;
        }
        case SPELLFAMILY_MAGE:
            switch(m_spellInfo->Id )
            {
                case 11958:                                 // Cold Snap
                {
                    if(m_caster->GetTypeId()!=TYPEID_PLAYER)
                        return;

                    // immediately finishes the cooldown on Frost spells
                    const PlayerSpellMap& sp_list = (m_caster->ToPlayer())->GetSpellMap();
                    for (PlayerSpellMap::const_iterator itr = sp_list.begin(); itr != sp_list.end(); ++itr)
                    {
                        if (itr->second->state == PLAYERSPELL_REMOVED)
                            continue;

                        uint32 classspell = itr->first;
                        SpellEntry const *spellInfo = spellmgr.LookupSpell(classspell);

                        if( spellInfo->SpellFamilyName == SPELLFAMILY_MAGE &&
                            (GetSpellSchoolMask(spellInfo) & SPELL_SCHOOL_MASK_FROST) &&
                            spellInfo->Id != 11958 && GetSpellRecoveryTime(spellInfo) > 0 )
                        {
                            (m_caster->ToPlayer())->RemoveSpellCooldown(classspell);

                            WorldPacket data(SMSG_CLEAR_COOLDOWN, (4+8));
                            data << uint32(classspell);
                            data << uint64(m_caster->GetGUID());
                            (m_caster->ToPlayer())->GetSession()->SendPacket(&data);
                        }
                    }
                    return;
                }
                case 32826:
                {
                    if ( unitTarget && unitTarget->GetTypeId() == TYPEID_UNIT )
                    {
                        //Polymorph Cast Visual Rank 1
                        const uint32 spell_list[6] = {32813, 32816, 32817, 32818, 32819, 32820};
                        unitTarget->CastSpell( unitTarget, spell_list[m_caster->GetMap()->urand(0, 5)], true);
                    }
                    return;
                }
                case 30610:
                {
                    if (!unitTarget)
                        return;
                        
                    /*unitTarget->CastSpell(unitTarget, RAND(30605, 30606, 30607, 30608, 30609), true);
                    return;*/
                    spell_id = RAND(30605, 30606, 30607, 30608, 30609);
                    break;
                }
            }
            break;
        case SPELLFAMILY_WARRIOR:
            // Charge
            if(m_spellInfo->SpellFamilyFlags & 0x1 && m_spellInfo->SpellVisual == 867)
            {
                int32 chargeBasePoints0 = damage;
                m_caster->CastCustomSpell(m_caster,34846,&chargeBasePoints0,NULL,NULL,true);
                return;
            }
            // Execute
            if(m_spellInfo->SpellFamilyFlags & 0x20000000)
            {
                if(!unitTarget)
                    return;

                spell_id = 20647;
                bp = damage+int32(m_caster->GetPower(POWER_RAGE) * m_spellInfo->DmgMultiplier[i]);
                m_caster->SetPower(POWER_RAGE,0);
                break;
            }
            if(m_spellInfo->Id==21977)                      //Warrior's Wrath
            {
                if(!unitTarget)
                    return;

                m_caster->CastSpell(unitTarget,21887,true); // spell mod
                return;
            }
            break;
        case SPELLFAMILY_WARLOCK:
            //Life Tap (only it have this with dummy effect)
            if (m_spellInfo->SpellFamilyFlags == 0x40000)
            {
                float cost = damage;

                if(Player* modOwner = m_caster->GetSpellModOwner())
                    modOwner->ApplySpellMod(m_spellInfo->Id, SPELLMOD_COST, cost,this);

                int32 dmg = m_caster->SpellDamageBonus(m_caster, m_spellInfo,uint32(cost > 0 ? cost : 0), SPELL_DIRECT_DAMAGE);

                if(int32(m_caster->GetHealth()) > dmg)
                {
                    // Shouldn't Appear in Combat Log
                    m_caster->ModifyHealth(-dmg);

                    int32 mana = dmg;

                    Unit::AuraList const& auraDummy = m_caster->GetAurasByType(SPELL_AURA_DUMMY);
                    for(Unit::AuraList::const_iterator itr = auraDummy.begin(); itr != auraDummy.end(); ++itr)
                    {
                        // only Imp. Life Tap have this in combination with dummy aura
                        if((*itr)->GetSpellProto()->SpellFamilyName==SPELLFAMILY_WARLOCK && (*itr)->GetSpellProto()->SpellIconID == 208)
                            mana = ((*itr)->GetModifier()->m_amount + 100)* mana / 100;
                    }

                    m_caster->CastCustomSpell(m_caster,31818,&mana,NULL,NULL,true,NULL);

                    // Mana Feed
                    int32 manaFeedVal = m_caster->CalculateSpellDamage(m_spellInfo,1, m_spellInfo->EffectBasePoints[1],m_caster);
                    manaFeedVal = manaFeedVal * mana / 100;
                    if(manaFeedVal > 0)
                        m_caster->CastCustomSpell(m_caster,32553,&manaFeedVal,NULL,NULL,true,NULL);
                }
                else
                    SendCastResult(SPELL_FAILED_FIZZLE);
                return;
            }
            break;
        case SPELLFAMILY_PRIEST:
            switch(m_spellInfo->Id )
            {
                case 28598:                                 // Touch of Weakness triggered spell
                {
                    if(!unitTarget || !m_triggeredByAuraSpell)
                        return;

                    uint32 spellid = 0;
                    switch(m_triggeredByAuraSpell->Id)
                    {
                        case 2652:  spellid =  2943; break; // Rank 1
                        case 19261: spellid = 19249; break; // Rank 2
                        case 19262: spellid = 19251; break; // Rank 3
                        case 19264: spellid = 19252; break; // Rank 4
                        case 19265: spellid = 19253; break; // Rank 5
                        case 19266: spellid = 19254; break; // Rank 6
                        case 25461: spellid = 25460; break; // Rank 7
                        default:
                            sLog.outError("Spell::EffectDummy: Spell 28598 triggered by unhandled spell %u",m_triggeredByAuraSpell->Id);
                            return;
                    }
                    m_caster->CastSpell(unitTarget, spellid, true, NULL);
                    return;
                }
            }
            break;
        case SPELLFAMILY_DRUID:
            switch(m_spellInfo->Id )
            {
                case 5420:                                  // Tree of Life passive
                {
                    // Tree of Life area effect
                    int32 health_mod = int32(m_caster->GetStat(STAT_SPIRIT)/4);
                    m_caster->CastCustomSpell(m_caster,34123,&health_mod,NULL,NULL,true,NULL);
                    return;
                }
            }
            break;
        case SPELLFAMILY_ROGUE:
            switch(m_spellInfo->Id )
            {
                case 31231:                                 // Cheat Death
                {
                    m_caster->CastSpell(m_caster,45182,true);
                    return;
                }
                case 5938:                                  // Shiv
                {
                    if(m_caster->GetTypeId() != TYPEID_PLAYER)
                        return;

                    Player *pCaster = (m_caster->ToPlayer());

                    Item *item = pCaster->GetWeaponForAttack(OFF_ATTACK);
                    if(!item)
                        return;

                    // all poison enchantments is temporary
                    uint32 enchant_id = item->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT);
                    if(!enchant_id)
                        return;

                    SpellItemEnchantmentEntry const *pEnchant = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
                    if(!pEnchant)
                        return;

                    for (int s=0;s<3;s++)
                    {
                        if(pEnchant->type[s]!=ITEM_ENCHANTMENT_TYPE_COMBAT_SPELL)
                            continue;

                        SpellEntry const* combatEntry = spellmgr.LookupSpell(pEnchant->spellid[s]);
                        if(!combatEntry || combatEntry->Dispel != DISPEL_POISON)
                            continue;

                        m_caster->CastSpell(unitTarget, combatEntry, true, item);
                    }

                    m_caster->CastSpell(unitTarget, 5940, true);
                    return;
                }
            }
            break;
        case SPELLFAMILY_HUNTER:
            // Kill command
            if(m_spellInfo->SpellFamilyFlags & 0x00080000000000LL)
            {
                if(m_caster->getClass()!=CLASS_HUNTER)
                    return;

                // clear hunter crit aura state
                m_caster->ModifyAuraState(AURA_STATE_HUNTER_CRIT_STRIKE,false);

                // additional damage from pet to pet target
                Pet* pet = m_caster->GetPet();
                if(!pet || !pet->getVictim())
                    return;

                uint32 spell_id = 0;
                switch (m_spellInfo->Id)
                {
                case 34026: spell_id = 34027; break;        // rank 1
                default:
                    sLog.outError("Spell::EffectDummy: Spell %u not handled in KC",m_spellInfo->Id);
                    return;
                }

                pet->CastSpell(pet->getVictim(), spell_id, true);
                return;
            }

            switch(m_spellInfo->Id)
            {
                case 781:                                 // Disengage
                {
                    if (m_caster->GetTypeId() != TYPEID_PLAYER)
                        return;
                        
                    WorldPacket data(SMSG_MOVE_KNOCK_BACK, 50);
                    data.append(m_caster->GetPackGUID());
                    data << getMSTime();
                    data << float(cosf(m_caster->GetOrientation()+M_PI));
                    data << float(sinf(m_caster->GetOrientation()+M_PI));
                    data << float(15);
                    data << float(-7.0f);
                    (m_caster->ToPlayer())->GetSession()->SendPacket(&data);
                    return;
                }
                case 23989:                                 //Readiness talent
                {
                    if(m_caster->GetTypeId()!=TYPEID_PLAYER)
                        return;

                    //immediately finishes the cooldown for hunter abilities
                    const PlayerSpellMap& sp_list = (m_caster->ToPlayer())->GetSpellMap();
                    for (PlayerSpellMap::const_iterator itr = sp_list.begin(); itr != sp_list.end(); ++itr)
                    {
                        uint32 classspell = itr->first;
                        SpellEntry const *spellInfo = spellmgr.LookupSpell(classspell);

                        if (spellInfo->SpellFamilyName == SPELLFAMILY_HUNTER && spellInfo->Id != 23989 && GetSpellRecoveryTime(spellInfo) > 0 )
                        {
                            (m_caster->ToPlayer())->RemoveSpellCooldown(classspell);

                            WorldPacket data(SMSG_CLEAR_COOLDOWN, (4+8));
                            data << uint32(classspell);
                            data << uint64(m_caster->GetGUID());
                            (m_caster->ToPlayer())->GetSession()->SendPacket(&data);
                        }
                    }
                    return;
                }
                case 37506:                                 // Scatter Shot
                {
                    if (m_caster->GetTypeId()!=TYPEID_PLAYER)
                        return;

                    // break Auto Shot and autohit
                    m_caster->InterruptSpell(CURRENT_AUTOREPEAT_SPELL);
                    m_caster->AttackStop();
                    (m_caster->ToPlayer())->SendAttackSwingCancelAttack();
                    return;
                }
            }
            break;
        case SPELLFAMILY_PALADIN:
            switch(m_spellInfo->SpellIconID)
            {
                case  156:                                  // Holy Shock
                {
                    if(!unitTarget)
                        return;

                    int hurt = 0;
                    int heal = 0;

                    switch(m_spellInfo->Id)
                    {
                        case 20473: hurt = 25912; heal = 25914; break;
                        case 20929: hurt = 25911; heal = 25913; break;
                        case 20930: hurt = 25902; heal = 25903; break;
                        case 27174: hurt = 27176; heal = 27175; break;
                        case 33072: hurt = 33073; heal = 33074; break;
                        default:
                            sLog.outError("Spell::EffectDummy: Spell %u not handled in HS",m_spellInfo->Id);
                            return;
                    }

                    if(m_caster->IsFriendlyTo(unitTarget))
                        m_caster->CastSpell(unitTarget, heal, true, 0);
                    else
                        m_caster->CastSpell(unitTarget, hurt, true, 0);

                    return;
                }
                case 561:                                   // Judgement of command
                {
                    if(!unitTarget)
                        return;

                    uint32 spell_id = m_spellInfo->EffectBasePoints[i]+1;//m_currentBasePoints[i]+1;
                    SpellEntry const* spell_proto = spellmgr.LookupSpell(spell_id);
                    if(!spell_proto)
                        return;

                    if( !unitTarget->hasUnitState(UNIT_STAT_STUNNED) && m_caster->GetTypeId()==TYPEID_PLAYER)
                    {
                        // decreased damage (/2) for non-stunned target.
                        SpellModifier *mod = new SpellModifier;
                        mod->op = SPELLMOD_DAMAGE;
                        mod->value = -50;
                        mod->type = SPELLMOD_PCT;
                        mod->spellId = m_spellInfo->Id;
                        mod->effectId = i;
                        mod->lastAffected = NULL;
                        mod->mask = 0x0000020000000000LL;
                        mod->charges = 0;

                        (m_caster->ToPlayer())->AddSpellMod(mod, true);
                        m_caster->CastSpell(unitTarget,spell_proto,true,NULL);
                                                            // mod deleted
                        (m_caster->ToPlayer())->AddSpellMod(mod, false);
                    }
                    else
                        m_caster->CastSpell(unitTarget,spell_proto,true,NULL);

                    return;
                }
            }

            switch(m_spellInfo->Id)
            {
                case 31789:                                 // Righteous Defense (step 1)
                {
                    // 31989 -> dummy effect (step 1) + dummy effect (step 2) -> 31709 (taunt like spell for each target)

                    // non-standard cast requirement check
                    if (!unitTarget || unitTarget->getAttackers().empty())
                    {
                        // clear cooldown at fail
                        if(m_caster->GetTypeId()==TYPEID_PLAYER)
                        {
                            (m_caster->ToPlayer())->RemoveSpellCooldown(m_spellInfo->Id);

                            WorldPacket data(SMSG_CLEAR_COOLDOWN, (4+8));
                            data << uint32(m_spellInfo->Id);
                            data << uint64(m_caster->GetGUID());
                            (m_caster->ToPlayer())->GetSession()->SendPacket(&data);
                        }

                        SendCastResult(SPELL_FAILED_TARGET_AFFECTING_COMBAT);
                        return;
                    }

                    // Righteous Defense (step 2) (in old version 31980 dummy effect)
                    // Clear targets for eff 1
                    for(std::list<TargetInfo>::iterator ihit= m_UniqueTargetInfo.begin();ihit != m_UniqueTargetInfo.end();++ihit)
                        ihit->effectMask &= ~(1<<1);

                    // not empty (checked)
                    Unit::AttackerSet const& attackers = unitTarget->getAttackers();

                    // chance to be selected from list
                    float chance = 100.0f/attackers.size();
                    uint32 count=0;
                    for(Unit::AttackerSet::const_iterator aItr = attackers.begin(); aItr != attackers.end() && count < 3; ++aItr)
                    {
                        if(!roll_chance_f(chance))
                            continue;
                        ++count;
                        AddUnitTarget((*aItr), 1);
                    }

                    // now let next effect cast spell at each target.
                    return;
                }
                case 37877:                                 // Blessing of Faith
                {
                    if(!unitTarget)
                        return;

                    uint32 spell_id = 0;
                    switch(unitTarget->getClass())
                    {
                        case CLASS_DRUID:   spell_id = 37878; break;
                        case CLASS_PALADIN: spell_id = 37879; break;
                        case CLASS_PRIEST:  spell_id = 37880; break;
                        case CLASS_SHAMAN:  spell_id = 37881; break;
                        default: return;                    // ignore for not healing classes
                    }

                    m_caster->CastSpell(m_caster,spell_id,true);
                    return;
                }
            }
            break;
        case SPELLFAMILY_SHAMAN:
            //Shaman Rockbiter Weapon
            if (m_spellInfo->SpellFamilyFlags == 0x400000)
            {
                uint32 spell_id = 0;
                switch(m_spellInfo->Id)
                {
                    case  8017: spell_id = 36494; break;    // Rank 1
                    case  8018: spell_id = 36750; break;    // Rank 2
                    case  8019: spell_id = 36755; break;    // Rank 3
                    case 10399: spell_id = 36759; break;    // Rank 4
                    case 16314: spell_id = 36763; break;    // Rank 5
                    case 16315: spell_id = 36766; break;    // Rank 6
                    case 16316: spell_id = 36771; break;    // Rank 7
                    case 25479: spell_id = 36775; break;    // Rank 8
                    case 25485: spell_id = 36499; break;    // Rank 9
                    default:
                        sLog.outError("Spell::EffectDummy: Spell %u not handled in RW",m_spellInfo->Id);
                        return;
                }

                SpellEntry const *spellInfo = spellmgr.LookupSpell( spell_id );

                if(!spellInfo)
                {
                    sLog.outError("WORLD: unknown spell id %i\n", spell_id);
                    return;
                }

                if(m_caster->GetTypeId() != TYPEID_PLAYER)
                    return;

                for(int i = BASE_ATTACK; i <= OFF_ATTACK; ++i)
                {
                    if(Item* item = (m_caster->ToPlayer())->GetWeaponForAttack(WeaponAttackType(i)))
                    {
                        if(item->IsFitToSpellRequirements(m_spellInfo))
                        {
                            Spell *spell = new Spell(m_caster, spellInfo, true);

                            // enchanting spell selected by calculated damage-per-sec in enchanting effect
                            // at calculation applied affect from Elemental Weapons talent
                            // real enchantment damage-1
                            spell->m_currentBasePoints[1] = damage-1;

                            SpellCastTargets targets;
                            targets.setItemTarget( item );
                            spell->prepare(&targets);
                        }
                    }
                }
                return;
            }

            if(m_spellInfo->Id == 39610)                    // Mana-Tide Totem effect
            {
                if(!unitTarget || unitTarget->getPowerType() != POWER_MANA)
                    return;

                // Regenerate 6% of Total Mana Every 3 secs
                int32 EffectBasePoints0 = unitTarget->GetMaxPower(POWER_MANA)  * damage / 100;
                m_caster->CastCustomSpell(unitTarget,39609,&EffectBasePoints0,NULL,NULL,true,NULL,NULL,m_originalCasterGUID);
                return;
            }

            // Flametongue Weapon Proc
            if(m_spellInfo->SpellFamilyFlags &0x0000000000200000LL)  
            {
                if(m_caster->GetTypeId()!=TYPEID_PLAYER)
                    return;

                if(!m_CastItem || !m_CastItem->IsEquipped())
                    return;

                WeaponAttackType attType=BASE_ATTACK;
                if (m_CastItem->GetSlot() == EQUIPMENT_SLOT_OFFHAND)
                    attType=OFF_ATTACK;

                float wspeed = m_caster->GetAttackTime(attType)/1000.0f;
                if (wspeed > 4.0) wspeed = 4.0f;
                if (wspeed < 1.3) wspeed = 1.3f;
                int32 EffectBasePoints0 = uint32(damage*wspeed/100.f);
                m_caster->CastCustomSpell(unitTarget,10444,&EffectBasePoints0,NULL,NULL,true);
                return;
            }
            // Flametongue Totem Proc
            if(m_spellInfo->SpellFamilyFlags &0x0000000400000000LL)  
            {
                if(m_caster->GetTypeId()!=TYPEID_PLAYER)
                    return;

                float wspeed = m_caster->GetAttackTime(BASE_ATTACK)/1000.0f;
                if (wspeed > 4.0) wspeed = 4.0f;
                if (wspeed < 1.3) wspeed = 1.3f;
                int32 EffectBasePoints0 = uint32(damage*wspeed/100.f);
                m_caster->CastCustomSpell(unitTarget,16368,&EffectBasePoints0,NULL,NULL,true);
                return;
            }

            break;
    }

    //spells triggered by dummy effect should not miss
    if(spell_id)
    {
        SpellEntry const *spellInfo = spellmgr.LookupSpell( spell_id );

        if(!spellInfo)
        {
            sLog.outError("EffectDummy of spell %u: triggering unknown spell id %i\n", m_spellInfo->Id, spell_id);
            return;
        }

        Spell* spell = new Spell(m_caster, spellInfo, true, m_originalCasterGUID, NULL, true);
        if(bp) spell->m_currentBasePoints[0] = bp;
        SpellCastTargets targets;
        targets.setUnitTarget(unitTarget);
        spell->prepare(&targets);
    }

    // pet auras
    if(PetAura const* petSpell = spellmgr.GetPetAura(m_spellInfo->Id))
    {
        m_caster->AddPetAura(petSpell);
        return;
    }
    
    if (unitTarget && unitTarget->GetTypeId() == TYPEID_UNIT)
		sScriptMgr.EffectDummyCreature(m_caster, m_spellInfo->Id, i, unitTarget->ToCreature());

    if (unitTarget && unitTarget->ToCreature() && unitTarget->ToCreature()->IsAIEnabled)
        unitTarget->ToCreature()->AI()->sOnDummyEffect(m_caster, m_spellInfo->Id, i);
}

void Spell::EffectTriggerSpellWithValue(uint32 i)
{
    uint32 triggered_spell_id = m_spellInfo->EffectTriggerSpell[i];

    // normal case
    SpellEntry const *spellInfo = spellmgr.LookupSpell( triggered_spell_id );

    if(!spellInfo)
    {
        sLog.outError("EffectTriggerSpellWithValue of spell %u: triggering unknown spell id %i\n", m_spellInfo->Id,triggered_spell_id);
        return;
    }

    int32 bp = damage;
    m_caster->CastCustomSpell(unitTarget,triggered_spell_id,&bp,&bp,&bp,true,NULL,NULL,m_originalCasterGUID);
}

void Spell::EffectTriggerRitualOfSummoning(uint32 i)
{
    uint32 triggered_spell_id = m_spellInfo->EffectTriggerSpell[i];
    SpellEntry const *spellInfo = spellmgr.LookupSpell( triggered_spell_id );

    if (!spellInfo) {
        sLog.outError("EffectTriggerRitualOfSummoning of spell %u: triggering unknown spell id %i", m_spellInfo->Id,triggered_spell_id);
        return;
    }

    finish();
    Spell *spell = new Spell(m_caster, spellInfo, true);

    SpellCastTargets targets;
    targets.setUnitTarget( unitTarget);
    spell->prepare(&targets);

    m_caster->SetCurrentCastedSpell(spell);
    spell->m_selfContainer = &(m_caster->m_currentSpells[spell->GetCurrentContainer()]);

    TakeReagents();
}

void Spell::EffectForceCast(uint32 i)
{
    if( !unitTarget )
        return;

    uint32 triggered_spell_id = m_spellInfo->EffectTriggerSpell[i];

    // normal case
    SpellEntry const *spellInfo = spellmgr.LookupSpell( triggered_spell_id );

    if(!spellInfo)
    {
        sLog.outError("EffectForceCast of spell %u: triggering unknown spell id %i", m_spellInfo->Id,triggered_spell_id);
        return;
    }

    switch (m_spellInfo->Id)
    {
        case 45442:
            if (!m_caster->getVictim())
                return;

            m_caster->getVictim()->CastSpell(m_caster->getVictim(), triggered_spell_id, true, NULL, NULL, m_originalCasterGUID);
            break;
        default:
            unitTarget->CastSpell(unitTarget,spellInfo,true,NULL,NULL,m_originalCasterGUID);
            break;
    }
}

void Spell::EffectTriggerSpell(uint32 i)
{
    // Prevent triggering spells here if spell has a charge effect (handled in ChargeMovementGenerator)
    if (sWorld.getConfig(CONFIG_CHARGEMOVEGEN) && (m_spellInfo->Effect[0] == SPELL_EFFECT_CHARGE || m_spellInfo->Effect[1] == SPELL_EFFECT_CHARGE || m_spellInfo->Effect[2] == SPELL_EFFECT_CHARGE))
        return;
    
    uint32 triggered_spell_id = m_spellInfo->EffectTriggerSpell[i];

    // special cases
    switch(triggered_spell_id)
    {
		// Enrage
		case 37975:
		{
			unitTarget = m_caster;
			break;
		}
        // Vanish
        case 18461:
        {
            m_caster->RemoveSpellsCausingAura(SPELL_AURA_MOD_ROOT);
            m_caster->RemoveSpellsCausingAura(SPELL_AURA_MOD_DECREASE_SPEED);
            m_caster->RemoveSpellsCausingAura(SPELL_AURA_MOD_STALKED);

            // if this spell is given to NPC it must handle rest by it's own AI
            if ( m_caster->GetTypeId() != TYPEID_PLAYER )
                return;

            // get highest rank of the Stealth spell
            bool found = false;
            SpellEntry const *spellInfo;
            const PlayerSpellMap& sp_list = (m_caster->ToPlayer())->GetSpellMap();
            for (PlayerSpellMap::const_iterator itr = sp_list.begin(); itr != sp_list.end(); ++itr)
            {
                // only highest rank is shown in spell book, so simply check if shown in spell book
                if(!itr->second->active || itr->second->disabled || itr->second->state == PLAYERSPELL_REMOVED)
                    continue;

                spellInfo = spellmgr.LookupSpell(itr->first);
                if (!spellInfo)
                    continue;

                if (spellInfo->SpellFamilyName == SPELLFAMILY_ROGUE && spellInfo->SpellFamilyFlags & SPELLFAMILYFLAG_ROGUE_STEALTH)
                {
                    found=true;
                    break;
                }
            }

            // no Stealth spell found
            if (!found)
                return;

            // reset cooldown on it if needed
            if((m_caster->ToPlayer())->HasSpellCooldown(spellInfo->Id))
                (m_caster->ToPlayer())->RemoveSpellCooldown(spellInfo->Id);

            m_TriggerSpells.push_back(spellInfo);
            return;
        }
        // just skip
        case 23770:                                         // Sayge's Dark Fortune of *
            // not exist, common cooldown can be implemented in scripts if need.
            return;
        // Brittle Armor - (need add max stack of 24575 Brittle Armor)
        case 29284:
        {
            const SpellEntry *spell = spellmgr.LookupSpell(24575);
            if (!spell)
                return;

            for (int i=0; i < spell->StackAmount; ++i)
                m_caster->CastSpell(unitTarget,spell->Id, true, m_CastItem, NULL, m_originalCasterGUID);
            return;
        }
        // Mercurial Shield - (need add max stack of 26464 Mercurial Shield)
        case 29286:
        {
            const SpellEntry *spell = spellmgr.LookupSpell(26464);
            if (!spell)
                return;

            for (int i=0; i < spell->StackAmount; ++i)
                m_caster->CastSpell(unitTarget,spell->Id, true, m_CastItem, NULL, m_originalCasterGUID);
            return;
        }
        // Righteous Defense
        case 31980:
        {
            m_caster->CastSpell(unitTarget, 31790, true,m_CastItem,NULL,m_originalCasterGUID);
            return;
        }
        // Cloak of Shadows
        case 35729 :
        {
            uint32 dispelMask = GetDispellMask(DISPEL_ALL);
            Unit::AuraMap& Auras = m_caster->GetAuras();
            for(Unit::AuraMap::iterator iter = Auras.begin(); iter != Auras.end(); ++iter)
            {
                // remove all harmful spells on you...
                SpellEntry const* spell = iter->second->GetSpellProto();
                if((spell->DmgClass == SPELL_DAMAGE_CLASS_MAGIC // only affect magic spells
                    || ((1<<spell->Dispel) & dispelMask))
                    // ignore positive and passive auras
                    && !iter->second->IsPositive() && !iter->second->IsPassive())
                {
                    m_caster->RemoveAurasDueToSpell(spell->Id);
                    iter = Auras.begin();
                }
            }
            return;
        }
        // Priest Shadowfiend (34433) need apply mana gain trigger aura on pet
        case 41967:
        {
            if (Unit *pet = m_caster->GetPet()) {
                if (m_caster->getVictim())
                    pet->SendAttackStart(m_caster->getVictim());
                pet->CastSpell(pet, 28305, true);
            }
            return;
        }
    }

    // normal case
    SpellEntry const *spellInfo = spellmgr.LookupSpell( triggered_spell_id );

    if(!spellInfo)
    {
        sLog.outError("EffectTriggerSpell of spell %u: triggering unknown spell id %i", m_spellInfo->Id,triggered_spell_id);
        return;
    }

    // some triggered spells require specific equipment
    if(spellInfo->EquippedItemClass >=0 && m_caster->GetTypeId()==TYPEID_PLAYER)
    {
        // main hand weapon required
        if(spellInfo->AttributesEx3 & SPELL_ATTR_EX3_MAIN_HAND)
        {
            Item* item = (m_caster->ToPlayer())->GetWeaponForAttack(BASE_ATTACK);

            // skip spell if no weapon in slot or broken
            if(!item || item->IsBroken() )
                return;

            // skip spell if weapon not fit to triggered spell
            if(!item->IsFitToSpellRequirements(spellInfo))
                return;
        }

        // offhand hand weapon required
        if(spellInfo->AttributesEx3 & SPELL_ATTR_EX3_REQ_OFFHAND)
        {
            Item* item = (m_caster->ToPlayer())->GetWeaponForAttack(OFF_ATTACK);

            // skip spell if no weapon in slot or broken
            if(!item || item->IsBroken() )
                return;

            // skip spell if weapon not fit to triggered spell
            if(!item->IsFitToSpellRequirements(spellInfo))
                return;
        }
    }

    // some triggered spells must be casted instantly (for example, if next effect case instant kill caster)
    /*bool instant = false;
    for(uint32 j = i+1; j < 3; ++j)
    {
        if(m_spellInfo->EffectImplicitTargetA[j] == TARGET_UNIT_CASTER
            && (m_spellInfo->Effect[j]==SPELL_EFFECT_INSTAKILL))
        {
            instant = true;
            break;
        }
    }

    if(instant)
    {*/
    if (unitTarget)
        m_caster->CastSpell(unitTarget, spellInfo, true, m_CastItem, NULL, m_originalCasterGUID);
    /*}
    else
        m_TriggerSpells.push_back(spellInfo);
    */
}

void Spell::EffectTriggerMissileSpell(uint32 effect_idx)
{
    uint32 triggered_spell_id = m_spellInfo->EffectTriggerSpell[effect_idx];

    // normal case
    SpellEntry const *spellInfo = spellmgr.LookupSpell( triggered_spell_id );

    if(!spellInfo)
    {
        sLog.outError("EffectTriggerMissileSpell of spell %u (eff: %u): triggering unknown spell id %u",
            m_spellInfo->Id,effect_idx,triggered_spell_id);
        return;
    }
    
    // some triggered spells require specific equipment
    if(spellInfo->EquippedItemClass >=0 && m_caster->GetTypeId()==TYPEID_PLAYER)
    {
        // main hand weapon required
        if(spellInfo->AttributesEx3 & SPELL_ATTR_EX3_MAIN_HAND)
        {
            Item* item = (m_caster->ToPlayer())->GetWeaponForAttack(BASE_ATTACK);

            // skip spell if no weapon in slot or broken
            if(!item || item->IsBroken() )
                return;

            // skip spell if weapon not fit to triggered spell
            if(!item->IsFitToSpellRequirements(spellInfo))
                return;
        }

        // offhand hand weapon required
        if(spellInfo->AttributesEx3 & SPELL_ATTR_EX3_REQ_OFFHAND)
        {
            Item* item = (m_caster->ToPlayer())->GetWeaponForAttack(OFF_ATTACK);

            // skip spell if no weapon in slot or broken
            if(!item || item->IsBroken() )
                return;

            // skip spell if weapon not fit to triggered spell
            if(!item->IsFitToSpellRequirements(spellInfo))
                return;
        }
    }

    if (m_CastItem)
        DEBUG_LOG("WORLD: cast Item spellId - %i", spellInfo->Id);

    Spell *spell = new Spell(m_caster, spellInfo, true, m_originalCasterGUID );

    SpellCastTargets targets;
    targets.setDestination(m_targets.m_destX,m_targets.m_destY,m_targets.m_destZ);
    spell->m_CastItem = m_CastItem;
    spell->prepare(&targets, NULL);
}

void Spell::EffectTeleportUnits(uint32 i)
{
    if(!unitTarget || unitTarget->isInFlight())
        return;
        
    // HackCrashFix
    if (m_spellInfo->Id == 46020 && unitTarget->GetMapId() != 580)
        return;

    // If not exist data for dest location - return
    if(!m_targets.HasDst())
    {
        sLog.outError( "Spell::EffectTeleportUnits - does not have destination for spell ID %u\n", m_spellInfo->Id );
        return;
    }
    // Init dest coordinates
    int32 mapid = m_targets.m_mapId;
    if(mapid < 0) mapid = (int32)unitTarget->GetMapId();
    float x = m_targets.m_destX;
    float y = m_targets.m_destY;
    float z = m_targets.m_destZ;

    float orientation = 0.0f;

    // Teleport and Transform kj outro
    // Orientation exist in db but is not used wtf?!
    if (m_spellInfo->Id == 46473)
    	orientation = 6.22f;
    else
        orientation = m_targets.getUnitTarget() ? m_targets.getUnitTarget()->GetOrientation() : unitTarget->GetOrientation();

    // Teleport
    if(unitTarget->GetTypeId() == TYPEID_PLAYER)
        (unitTarget->ToPlayer())->TeleportTo(mapid, x, y, z, orientation, TELE_TO_NOT_LEAVE_COMBAT | TELE_TO_NOT_UNSUMMON_PET | (unitTarget==m_caster ? TELE_TO_SPELL : 0));
    else
    {
        MapManager::Instance().GetMap(mapid, m_caster)->CreatureRelocation(unitTarget->ToCreature(), x, y, z, orientation);
        WorldPacket data;
        unitTarget->BuildTeleportAckMsg(&data, x, y, z, orientation);
        unitTarget->SendMessageToSet(&data, false);
        unitTarget->BuildHeartBeatMsg(&data);
        unitTarget->SendMessageToSet(&data,true);
    }

    // post effects for TARGET_DST_DB
    switch ( m_spellInfo->Id )
    {
        // Dimensional Ripper - Everlook
        case 23442:
        {
          int32 r = m_caster->GetMap()->irand(0, 119);
            if ( r >= 70 )                                  // 7/12 success
            {
                if ( r < 100 )                              // 4/12 evil twin
                    m_caster->CastSpell(m_caster,23445,true);
                else                                        // 1/12 fire
                    m_caster->CastSpell(m_caster,23449,true);
            }
            return;
        }
        // Ultrasafe Transporter: Toshley's Station
        case 36941:
        {
            if ( roll_chance_i(50) )                        // 50% success
            {
              int32 rand_eff = m_caster->GetMap()->urand(1,7);
                switch ( rand_eff )
                {
                    case 1:
                        // soul split - evil
                        m_caster->CastSpell(m_caster,36900,true);
                        break;
                    case 2:
                        // soul split - good
                        m_caster->CastSpell(m_caster,36901,true);
                        break;
                    case 3:
                        // Increase the size
                        m_caster->CastSpell(m_caster,36895,true);
                        break;
                    case 4:
                        // Decrease the size
                        m_caster->CastSpell(m_caster,36893,true);
                        break;
                    case 5:
                    // Transform
                    {
                        if ((m_caster->ToPlayer())->GetTeam() == ALLIANCE )
                            m_caster->CastSpell(m_caster,36897,true);
                        else
                            m_caster->CastSpell(m_caster,36899,true);
                        break;
                    }
                    case 6:
                        // chicken
                        m_caster->CastSpell(m_caster,36940,true);
                        break;
                    case 7:
                        // evil twin
                        m_caster->CastSpell(m_caster,23445,true);
                        break;
                }
            }
            return;
        }
        // Dimensional Ripper - Area 52
        case 36890:
        {
            if ( roll_chance_i(50) )                        // 50% success
            {
              int32 rand_eff = m_caster->GetMap()->urand(1,7);
                switch ( rand_eff )
                {
                    case 1:
                        // soul split - evil
                        m_caster->CastSpell(m_caster,36900,true);
                        break;
                    case 2:
                        // soul split - good
                        m_caster->CastSpell(m_caster,36901,true);
                        break;
                    case 3:
                        // Increase the size
                        m_caster->CastSpell(m_caster,36895,true);
                        break;
                    case 4:
                    // Transform
                    {
                        if ((m_caster->ToPlayer())->GetTeam() == ALLIANCE )
                            m_caster->CastSpell(m_caster,36897,true);
                        else
                            m_caster->CastSpell(m_caster,36899,true);
                        break;
                    }
                    case 5: // Fire
                    {
                        m_caster->CastSpell(m_caster, 36892, true);
                        break;
                    }
                    case 6: // Decrease size
                    {
                        m_caster->CastSpell(m_caster, 36893, true);
                        break;
                    }
                    case 7: // Chicken
                    {
                        m_caster->CastSpell(m_caster, 36940, true);
                        break;
                    }
                }
            }
            return;
        }
    }
}

void Spell::EffectApplyAura(uint32 i)
{
    if(!unitTarget)
        return;

    // Intervention shouldn't be used in a bg in preparation phase (possibility to get out of starting area with that spell)
    if (m_spellInfo->Id == 3411 && m_caster->HasAura(44521))     // Preparation
        return;

    if (m_spellInfo->Id == 10803 || m_spellInfo->Id == 10804)
        unitTarget->RemoveSpellsCausingAura(SPELL_AURA_MOUNTED);

    SpellImmuneList const& list = unitTarget->m_spellImmune[IMMUNITY_STATE];
    for(SpellImmuneList::const_iterator itr = list.begin(); itr != list.end(); ++itr)
        if(itr->type == m_spellInfo->EffectApplyAuraName[i])
            return;

    // ghost spell check, allow apply any auras at player loading in ghost mode (will be cleanup after load)
    if( !unitTarget->isAlive() && m_spellInfo->Id != 20584 && m_spellInfo->Id != 8326 &&
        (unitTarget->GetTypeId()!=TYPEID_PLAYER || !(unitTarget->ToPlayer())->GetSession()->PlayerLoading()) )
        return;

    Unit* caster = m_originalCasterGUID ? m_originalCaster : m_caster;
    if(!caster)
        return;
            
    if (m_spellInfo->Id == 40880 || m_spellInfo->Id == 40882 || m_spellInfo->Id == 40883 || m_spellInfo->Id == 40891 || m_spellInfo->Id == 40896 || m_spellInfo->Id == 40897)   // Sharaz prismatic auras
        unitTarget = caster;
    
    // Some level depends spells
    int multiplier = 0;
    int level_diff = 0;
    switch (m_spellInfo->Id)
    {
        // Holy Strength
        case 20007:
            level_diff = m_caster->getLevel() - 60;
            multiplier = 4;
            break;
        default:
            break;
    }

    if (level_diff > 0)
        damage -= multiplier * level_diff;

    Aura* Aur = CreateAura(m_spellInfo, i, &damage, unitTarget, caster, m_CastItem);

    // Now Reduce spell duration using data received at spell hit
    int32 duration = Aur->GetAuraMaxDuration();
    
    // Shahraz: immunity to teleport
    if (m_spellInfo->Id == 43690)
        duration *= 2;
    
    if(!IsPositiveSpell(m_spellInfo->Id))
    {
        unitTarget->ApplyDiminishingToDuration(m_diminishGroup,duration,caster,m_diminishLevel);
        Aur->setDiminishGroup(m_diminishGroup);
    }

    //mod duration of channeled aura by spell haste
    if (IsChanneledSpell(m_spellInfo))
        caster->ModSpellCastTime(m_spellInfo, duration, this);

    // if Aura removed and deleted, do not continue.
    if(duration== 0 && !(Aur->IsPermanent()))
    {
        delete Aur;
        return;
    }

    if(duration != Aur->GetAuraMaxDuration())
    {
        Aur->SetAuraMaxDuration(duration);
        Aur->SetAuraDuration(duration);
    }
    
    if (Aur->GetId() == 45582)
        Aur->SetAuraDuration(Aur->GetAuraMaxDuration()*0.5f);

    bool added = unitTarget->AddAura(Aur);

    // Aura not added and deleted in AddAura call;
    if (!added)
        return;

    // found crash at character loading, broken pointer to Aur...
    // Aur was deleted in AddAura()...
    if(!Aur)
        return;

    // TODO Make a way so it works for every related spell!
    if(unitTarget->GetTypeId()==TYPEID_PLAYER ||( unitTarget->GetTypeId()==TYPEID_UNIT && (unitTarget->ToCreature())->isPet() ) )              // Negative buff should only be applied on players
    {
        uint32 spellId = 0;
        if(m_spellInfo->CasterAuraStateNot==AURA_STATE_WEAKENED_SOUL || m_spellInfo->TargetAuraStateNot==AURA_STATE_WEAKENED_SOUL)
            spellId = 6788;                                 // Weakened Soul
        else if(m_spellInfo->CasterAuraStateNot==AURA_STATE_FORBEARANCE || m_spellInfo->TargetAuraStateNot==AURA_STATE_FORBEARANCE)
            spellId = 25771;                                // Forbearance
        else if(m_spellInfo->CasterAuraStateNot==AURA_STATE_HYPOTHERMIA)
            spellId = 41425;                                // Hypothermia
        else if (m_spellInfo->Mechanic == MECHANIC_BANDAGE) // Bandages
            spellId = 11196;                                // Recently Bandaged
        else if( (m_spellInfo->AttributesEx & 0x20) && (m_spellInfo->AttributesEx2 & 0x20000) )
            spellId = 23230;                                // Blood Fury - Healing Reduction

        SpellEntry const *AdditionalSpellInfo = spellmgr.LookupSpell(spellId);
        if (AdditionalSpellInfo)
        {
            // applied at target by target
            Aura* AdditionalAura = CreateAura(AdditionalSpellInfo, 0, NULL, unitTarget,unitTarget, 0);
            unitTarget->AddAura(AdditionalAura);
        }
    }
    
    if (m_spellInfo->Id == 34219)                                 // Quest 10190
    {
        if (!unitTarget || unitTarget->GetTypeId() != TYPEID_UNIT || m_caster->GetTypeId() != TYPEID_PLAYER)
            return;
            
        Creature* cTarget = unitTarget->ToCreature();
        
        if (cTarget->GetHealth() <= cTarget->GetMaxHealth()/5)
        {
            (m_caster->ToPlayer())->KilledMonster(19595, 0);
            cTarget->DealDamage(cTarget, cTarget->GetMaxHealth(), NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NONE, NULL, false);
            cTarget->RemoveCorpse();
        }
    }
    
    // Remove Stealth on Druid/Warrior shout
    switch (m_spellInfo->SpellFamilyName)
    {
	    case SPELLFAMILY_WARRIOR:
            if (m_spellInfo->SpellFamilyFlags & 0x0000002000020000LL)
				 unitTarget->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_TALK, 0, false);
            break;
	    case SPELLFAMILY_DRUID:
            if (m_spellInfo->SpellFamilyFlags & 0x0000000000000408LL)
			    unitTarget->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_TALK, 0, false);
            break;
    }

    // Prayer of Mending (jump animation), we need formal caster instead original for correct animation
    if( m_spellInfo->SpellFamilyName == SPELLFAMILY_PRIEST && (m_spellInfo->SpellFamilyFlags & 0x00002000000000LL))
        m_caster->CastSpell(unitTarget, 41637, true, NULL, Aur, m_originalCasterGUID);
        
    /*if (caster->IsHostileTo(unitTarget) && !(m_spellInfo->AttributesEx3 & SPELL_ATTR_EX3_NO_INITIAL_AGGRO))
        unitTarget->RemoveSpellsCausingAura(SPELL_AURA_MOD_STEALTH);*/
}

void Spell::EffectUnlearnSpecialization( uint32 i )
{
    if(!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    Player *_player = unitTarget->ToPlayer();
    uint32 spellToUnlearn = m_spellInfo->EffectTriggerSpell[i];

    _player->removeSpell(spellToUnlearn);
}

void Spell::EffectPowerDrain(uint32 i)
{
    if(m_spellInfo->EffectMiscValue[i] < 0 || m_spellInfo->EffectMiscValue[i] >= MAX_POWERS)
        return;

    Powers drain_power = Powers(m_spellInfo->EffectMiscValue[i]);

    if(!unitTarget)
        return;
    if(!unitTarget->isAlive())
        return;
    if(unitTarget->getPowerType() != drain_power)
        return;
    if(damage < 0)
        return;

    uint32 curPower = unitTarget->GetPower(drain_power);

    //add spell damage bonus
    damage=m_caster->SpellDamageBonus(unitTarget,m_spellInfo,uint32(damage),SPELL_DIRECT_DAMAGE);

    // resilience reduce mana draining effect at spell crit damage reduction (added in 2.4)
    uint32 power = damage;
    if ( drain_power == POWER_MANA && unitTarget->GetTypeId() == TYPEID_PLAYER )
        power -= (unitTarget->ToPlayer())->GetSpellCritDamageReduction(power);

    int32 new_damage;
    if(curPower < power)
        new_damage = curPower;
    else
        new_damage = power;
        
    if (m_spellInfo->Id == 27526) {
        if (m_caster->GetTypeId() == TYPEID_PLAYER && m_caster->getVictim())
            unitTarget = m_caster->getVictim();
    }
    
    unitTarget->ModifyPower(drain_power,-new_damage);

    if(drain_power == POWER_MANA)
    {
        float manaMultiplier = m_spellInfo->EffectMultipleValue[i];
        if(manaMultiplier==0)
            manaMultiplier = 1;

        if(Player *modOwner = m_caster->GetSpellModOwner())
            modOwner->ApplySpellMod(m_spellInfo->Id, SPELLMOD_MULTIPLE_VALUE, manaMultiplier);

        int32 gain = int32(new_damage*manaMultiplier);

        m_caster->ModifyPower(POWER_MANA,gain);
        //send log
        m_caster->SendEnergizeSpellLog(m_caster, m_spellInfo->Id,gain,POWER_MANA);
    }
}

void Spell::EffectSendEvent(uint32 EffectIndex)
{
    if (m_caster->GetTypeId() == TYPEID_PLAYER && (m_caster->ToPlayer())->InBattleGround())
    {
        BattleGround* bg = (m_caster->ToPlayer())->GetBattleGround();
        if(bg && bg->GetStatus() == STATUS_IN_PROGRESS)
        {
            switch(m_spellInfo->Id)
            {
                case 23333:                                 // Pickup Horde Flag
                    /*do not uncomment .
                    if(bg->GetTypeID()==BATTLEGROUND_WS)
                        bg->EventPlayerClickedOnFlag(m_caster->ToPlayer(), gameObjTarget);
                    sLog.outDebug("Send Event Horde Flag Picked Up");
                    break;
                    /* not used :
                    case 23334:                                 // Drop Horde Flag
                        if(bg->GetTypeID()==BATTLEGROUND_WS)
                            bg->EventPlayerDroppedFlag(m_caster->ToPlayer());
                        sLog.outDebug("Drop Horde Flag");
                        break;
                    */
                case 23335:                                 // Pickup Alliance Flag
                    /*do not uncomment ... (it will cause crash, because of null targetobject!) anyway this is a bad way to call that event, because it would cause recursion
                    if(bg->GetTypeID()==BATTLEGROUND_WS)
                        bg->EventPlayerClickedOnFlag(m_caster->ToPlayer(), gameObjTarget);
                    sLog.outDebug("Send Event Alliance Flag Picked Up");
                    break;
                    /* not used :
                    case 23336:                                 // Drop Alliance Flag
                        if(bg->GetTypeID()==BATTLEGROUND_WS)
                            bg->EventPlayerDroppedFlag(m_caster->ToPlayer());
                        sLog.outDebug("Drop Alliance Flag");
                        break;
                    case 23385:                                 // Alliance Flag Returns
                        if(bg->GetTypeID()==BATTLEGROUND_WS)
                            bg->EventPlayerClickedOnFlag(m_caster->ToPlayer(), gameObjTarget);
                        sLog.outDebug("Alliance Flag Returned");
                        break;
                    case 23386:                                   // Horde Flag Returns
                        if(bg->GetTypeID()==BATTLEGROUND_WS)
                            bg->EventPlayerClickedOnFlag(m_caster->ToPlayer(), gameObjTarget);
                        sLog.outDebug("Horde Flag Returned");
                        break;*/
                case 34976:
                    /*
                    if(bg->GetTypeID()==BATTLEGROUND_EY)
                        bg->EventPlayerClickedOnFlag(m_caster->ToPlayer(), gameObjTarget);
                    */
                    break;
                default:
                    sLog.outError("Unknown spellid %u in BG event", m_spellInfo->Id);
                    break;
            }
        }
    }
    
    //special cases TODO: switch + improve event_scripts system
    if (m_spellInfo->Id == 31949 && m_caster->GetTypeId() == TYPEID_PLAYER)
        (m_caster->ToPlayer())->CompleteQuest(9816);
    else if (m_spellInfo->Id == 34140 && m_caster->GetTypeId() == TYPEID_PLAYER && (m_caster->ToPlayer())->GetQuestStatus(10305) == QUEST_STATUS_INCOMPLETE)
        (m_caster->ToPlayer())->KilledMonster(19547, 0);
    else if (m_spellInfo->Id == 30098 && m_caster->GetTypeId() == TYPEID_PLAYER && (m_caster->ToPlayer())->GetQuestStatus(9444) == QUEST_STATUS_INCOMPLETE)
        (m_caster->ToPlayer())->CompleteQuest(9444);
    else if (m_spellInfo->Id == 24325) {
        ScriptedInstance *pInstance = ((ScriptedInstance*)m_caster->GetInstanceData());
        if (pInstance && (pInstance->GetData(29) == 1 || pInstance->GetData(29) == 3)) // Ghazranka has been down, don't spawn it another time
            return;
    }
    else if (m_spellInfo->Id == 32408 && (m_caster->ToPlayer()))
		(m_caster->ToPlayer())->KilledMonster(18395, 0);
    else if (m_spellInfo->Id == 40328) {
        Creature *soulgrinder = m_caster->FindNearestCreature(23019, 10.0f, true);
        if (soulgrinder) {      // Fear all ghosts
            /*CellPair pair(Trinity::ComputeCellPair(m_caster->GetPositionX(), m_caster->GetPositionY()));
            Cell cell(pair);
            cell.data.Part.reserved = ALL_DISTRICT;
            cell.SetNoCreate();
            std::list<Creature*> ogres;

            Trinity::AllCreaturesOfEntryInRange check(m_caster, 24039, 100.0f);
            Trinity::CreatureListSearcher<Trinity::AllCreaturesOfEntryInRange> searcher(ogres, check);
            TypeContainerVisitor<Trinity::CreatureListSearcher<Trinity::AllCreaturesOfEntryInRange>, GridTypeMapContainer> visitor(searcher);

            cell.Visit(pair, visitor, *m_caster->GetMap());
            
            for (std::list<Creature*>::iterator itr = ogres.begin(); itr != ogres.end(); itr++)
                (*itr)->CastSpell((*itr), 39914, true);*/
            m_caster->CastSpell(m_caster, 39914, false);
        }
        else                   // Summon Soulgrinder
            m_caster->SummonCreature(23019, 3535.181641, 5590.692871, 0.183175, 3.915725, TEMPSUMMON_DEAD_DESPAWN, 0);
    }
    else if (m_spellInfo->Id == 34142 && m_caster->GetTypeId() == TYPEID_PLAYER)
        m_caster->ToPlayer()->CompleteQuest(10306);
    else if (m_spellInfo->Id == 24706 && m_caster->ToPlayer())
        m_caster->ToPlayer()->KilledMonster(15415, 0);
    else if (m_spellInfo->Id == 39223 && m_caster->ToPlayer()) {
        if (Creature* rexxar = m_caster->SummonCreature(22448, m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), m_caster->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000)) {
            if (Creature* goc = m_caster->SummonCreature(20555, 3741.64, 5384.42, -4.98528, 1.89685, TEMPSUMMON_DEAD_DESPAWN, 60000))
                goc->AI()->AttackStart(rexxar);
        }
    }
    else if (m_spellInfo->Id == 20737 && m_caster->GetTypeId() == TYPEID_PLAYER)
        m_caster->ToPlayer()->SummonCreature(12918, m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), m_caster->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
    else if (m_spellInfo->Id == 42338 && m_caster->GetTypeId() == TYPEID_PLAYER)
        m_caster->ToPlayer()->KilledMonster(23727, 0);
    
    sWorld.ScriptsStart(sEventScripts, m_spellInfo->EffectMiscValue[EffectIndex], m_caster, focusObject);
}

void Spell::EffectPowerBurn(uint32 i)
{
    if(m_spellInfo->EffectMiscValue[i] < 0 || m_spellInfo->EffectMiscValue[i] >= MAX_POWERS)
        return;

    Powers powertype = Powers(m_spellInfo->EffectMiscValue[i]);

    if(!unitTarget)
        return;
    if(!unitTarget->isAlive())
        return;
    if(unitTarget->getPowerType()!=powertype)
        return;
    if(damage < 0)
        return;

    int32 curPower = int32(unitTarget->GetPower(powertype));

    // resilience reduce mana draining effect at spell crit damage reduction (added in 2.4)
    uint32 power = damage;
    if ( powertype == POWER_MANA && unitTarget->GetTypeId() == TYPEID_PLAYER )
        power -= (unitTarget->ToPlayer())->GetSpellCritDamageReduction(power);

    int32 new_damage = (curPower < power) ? curPower : power;

    unitTarget->ModifyPower(powertype,-new_damage);
    float multiplier = m_spellInfo->EffectMultipleValue[i];

    if(Player *modOwner = m_caster->GetSpellModOwner())
        modOwner->ApplySpellMod(m_spellInfo->Id, SPELLMOD_MULTIPLE_VALUE, multiplier);

    new_damage = int32(new_damage*multiplier);
    //m_damage+=new_damage; should not apply spell bonus
    //TODO: no log
    //unitTarget->ModifyHealth(-new_damage);
    if(m_originalCaster)
        m_originalCaster->DealDamage(unitTarget, new_damage);
}

void Spell::EffectHeal( uint32 /*i*/ )
{
}

void Spell::SpellDamageHeal(uint32 /*i*/)
{
    // Some level depends spells
    int multiplier = 0;
    int level_diff = 0;
    switch (m_spellInfo->Id)
    {
        // Holy Strength
        case 20007:
            level_diff = m_caster->getLevel() - 60;
            multiplier = int(0.04*damage);
            break;
        default:
            break;
    }

    if (level_diff > 0)
        damage -= multiplier * level_diff;

    if( unitTarget && unitTarget->isAlive() && damage >= 0)
    {
        // Try to get original caster
        Unit *caster = m_originalCasterGUID ? m_originalCaster : m_caster;

        // Skip if m_originalCaster not available
        if (!caster)
            return;

        int32 addhealth = damage;

        // Vessel of the Naaru (Vial of the Sunwell trinket)
        if (m_spellInfo->Id == 45064)
        {
            // Amount of heal - depends from stacked Holy Energy
            int damageAmount = 0;
            Unit::AuraList const& mDummyAuras = m_caster->GetAurasByType(SPELL_AURA_DUMMY);
            for(Unit::AuraList::const_iterator i = mDummyAuras.begin();i != mDummyAuras.end(); ++i)
                if((*i)->GetId() == 45062)
                    damageAmount+=(*i)->GetModifierValue();
            if (damageAmount)
                m_caster->RemoveAurasDueToSpell(45062);

            addhealth += damageAmount;
        }
        // Swiftmend - consumes Regrowth or Rejuvenation
        else if (m_spellInfo->TargetAuraState == AURA_STATE_SWIFTMEND && unitTarget->HasAuraState(AURA_STATE_SWIFTMEND))
        {
            Unit::AuraList const& RejorRegr = unitTarget->GetAurasByType(SPELL_AURA_PERIODIC_HEAL);
            // find most short by duration
            Aura *targetAura = NULL;
            for(Unit::AuraList::const_iterator i = RejorRegr.begin(); i != RejorRegr.end(); ++i)
            {
                if((*i)->GetSpellProto()->SpellFamilyName == SPELLFAMILY_DRUID
                    && ((*i)->GetSpellProto()->SpellFamilyFlags == 0x40 || (*i)->GetSpellProto()->SpellFamilyFlags == 0x10) )
                {
                    if(!targetAura || (*i)->GetAuraDuration() < targetAura->GetAuraDuration())
                        targetAura = *i;
                }
            }

            if(!targetAura)
            {
                sLog.outError("Target(GUID:" I64FMTD ") has aurastate AURA_STATE_SWIFTMEND but no matching aura.", unitTarget->GetGUID());
                return;
            }

            int32 tickheal = targetAura->GetModifierValuePerStack();
            if(Unit* auraCaster = targetAura->GetCaster())
                tickheal = auraCaster->SpellHealingBonus(targetAura->GetSpellProto(), tickheal, DOT, unitTarget);
            //int32 tickheal = targetAura->GetSpellProto()->EffectBasePoints[idx] + 1;
            //It is said that talent bonus should not be included
            //int32 tickheal = targetAura->GetModifierValue();
            int32 tickcount = 0;
            if(targetAura->GetSpellProto()->SpellFamilyName == SPELLFAMILY_DRUID)
            {
                switch(targetAura->GetSpellProto()->SpellFamilyFlags)//TODO: proper spellfamily for 3.0.x
                {
                    case 0x10:  tickcount = 4;  break; // Rejuvenation
                    case 0x40:  tickcount = 6;  break; // Regrowth
                }
            }
            addhealth += tickheal * tickcount;
            unitTarget->RemoveAurasByCasterSpell(targetAura->GetId(), targetAura->GetCasterGUID());

            //addhealth += tickheal * tickcount;
            //addhealth = caster->SpellHealingBonus(m_spellInfo, addhealth,HEAL, unitTarget);
        }
        else
            addhealth = caster->SpellHealingBonus(m_spellInfo, addhealth,HEAL, unitTarget);

        m_damage -= addhealth;
    }
}

void Spell::EffectHealPct( uint32 /*i*/ )
{
    if( unitTarget && unitTarget->isAlive() && damage >= 0)
    {
        // Try to get original caster
        Unit *caster = m_originalCasterGUID ? m_originalCaster : m_caster;

        // Skip if m_originalCaster not available
        if (!caster)
            return;
            
        if (m_spellInfo->Id == 39703)
            unitTarget = caster;

        uint32 addhealth = unitTarget->GetMaxHealth() * damage / 100;
        caster->SendHealSpellLog(unitTarget, m_spellInfo->Id, addhealth, false);

        int32 gain = unitTarget->ModifyHealth( int32(addhealth) );
        unitTarget->getHostilRefManager().threatAssist(m_caster, float(gain) * 0.5f, m_spellInfo);

        if(caster->GetTypeId()==TYPEID_PLAYER)
            if(BattleGround *bg = (caster->ToPlayer())->GetBattleGround())
                bg->UpdatePlayerScore((caster->ToPlayer()), SCORE_HEALING_DONE, gain);
    }
}

void Spell::EffectHealMechanical( uint32 /*i*/ )
{
    // Mechanic creature type should be correctly checked by targetCreatureType field
    if( unitTarget && unitTarget->isAlive() && damage >= 0)
    {
        // Try to get original caster
        Unit *caster = m_originalCasterGUID ? m_originalCaster : m_caster;

        // Skip if m_originalCaster not available
        if (!caster)
            return;

        uint32 addhealth = caster->SpellHealingBonus(m_spellInfo, uint32(damage), HEAL, unitTarget);
        caster->SendHealSpellLog(unitTarget, m_spellInfo->Id, addhealth, false);
        unitTarget->ModifyHealth( int32(damage) );
    }
}

void Spell::EffectHealthLeech(uint32 i)
{
    if(!unitTarget)
        return;
    if(!unitTarget->isAlive())
        return;

    if(damage < 0)
        return;

    float multiplier = m_spellInfo->EffectMultipleValue[i];

    if(Player *modOwner = m_caster->GetSpellModOwner())
        modOwner->ApplySpellMod(m_spellInfo->Id, SPELLMOD_MULTIPLE_VALUE, multiplier);

    int32 new_damage = int32(damage*multiplier);
    uint32 curHealth = unitTarget->GetHealth();
    new_damage = m_caster->SpellNonMeleeDamageLog(unitTarget, m_spellInfo->Id, new_damage, m_IsTriggeredSpell, true);
    if(curHealth < new_damage)
        new_damage = curHealth;

    if(m_caster->isAlive())
    {
        new_damage = m_caster->SpellHealingBonus(m_spellInfo, new_damage, HEAL, m_caster);

        m_caster->ModifyHealth(new_damage);

        if(m_caster->GetTypeId() == TYPEID_PLAYER)
            m_caster->SendHealSpellLog(m_caster, m_spellInfo->Id, uint32(new_damage));
    }
//    m_healthLeech+=tmpvalue;
//    m_damage+=new_damage;
}

void Spell::DoCreateItem(uint32 i, uint32 itemtype)
{
    if (!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    Player* player = unitTarget->ToPlayer();

    uint32 newitemid = itemtype;
    ItemPrototype const *pProto = objmgr.GetItemPrototype( newitemid );
    if(!pProto)
    {
        player->SendEquipError( EQUIP_ERR_ITEM_NOT_FOUND, NULL, NULL );
        return;
    }

    uint32 num_to_add;

    // TODO: maybe all this can be replaced by using correct calculated `damage` value
    if(pProto->Class != ITEM_CLASS_CONSUMABLE || m_spellInfo->SpellFamilyName != SPELLFAMILY_MAGE)
    {
        num_to_add = damage;
        /*int32 basePoints = m_currentBasePoints[i];
        int32 randomPoints = m_spellInfo->EffectDieSides[i];
        if (randomPoints)
            num_to_add = basePoints + m_caster->GetMap()->irand(1, randomPoints);
        else
            num_to_add = basePoints + 1;*/
    }
    else if (pProto->MaxCount == 1)
        num_to_add = 1;
    else if(player->getLevel() >= m_spellInfo->spellLevel)
    {
        num_to_add = damage;
        /*int32 basePoints = m_currentBasePoints[i];
        float pointPerLevel = m_spellInfo->EffectRealPointsPerLevel[i];
        num_to_add = basePoints + 1 + uint32((player->getLevel() - m_spellInfo->spellLevel)*pointPerLevel);*/
    }
    else
        num_to_add = 2;

    if (num_to_add < 1)
        num_to_add = 1;
    if (num_to_add > pProto->Stackable)
        num_to_add = pProto->Stackable;

    // init items_count to 1, since 1 item will be created regardless of specialization
    int items_count=1;
    // the chance to create additional items
    float additionalCreateChance=0.0f;
    // the maximum number of created additional items
    uint8 additionalMaxNum=0;
    // get the chance and maximum number for creating extra items
    if ( canCreateExtraItems(player, m_spellInfo->Id, additionalCreateChance, additionalMaxNum) )
    {
        // roll with this chance till we roll not to create or we create the max num
        while ( roll_chance_f(additionalCreateChance) && items_count<=additionalMaxNum )
            ++items_count;
    }

    // really will be created more items
    num_to_add *= items_count;

    // can the player store the new item?
    ItemPosCountVec dest;
    uint32 no_space = 0;
    uint8 msg = player->CanStoreNewItem( NULL_BAG, NULL_SLOT, dest, newitemid, num_to_add, &no_space );
    if( msg != EQUIP_ERR_OK )
    {
        // convert to possible store amount
        if( msg == EQUIP_ERR_INVENTORY_FULL || msg == EQUIP_ERR_CANT_CARRY_MORE_OF_THIS )
            num_to_add -= no_space;
        else
        {
            // if not created by another reason from full inventory or unique items amount limitation
            player->SendEquipError( msg, NULL, NULL );
            return;
        }
    }

    if(num_to_add)
    {
        // create the new item and store it
        Item* pItem = player->StoreNewItem( dest, newitemid, true, Item::GenerateItemRandomPropertyId(newitemid));

        // was it successful? return error if not
        if(!pItem)
        {
            player->SendEquipError( EQUIP_ERR_ITEM_NOT_FOUND, NULL, NULL );
            return;
        }

        // set the "Crafted by ..." property of the item
        if( pItem->GetProto()->Class != ITEM_CLASS_CONSUMABLE && pItem->GetProto()->Class != ITEM_CLASS_QUEST)
            pItem->SetUInt32Value(ITEM_FIELD_CREATOR,player->GetGUIDLow());

        // send info to the client
        if(pItem)
            player->SendNewItem(pItem, num_to_add, true, true);

        // we succeeded in creating at least one item, so a levelup is possible
        player->UpdateCraftSkill(m_spellInfo->Id);
    }
}

void Spell::EffectCreateItem(uint32 i)
{
    DoCreateItem(i,m_spellInfo->EffectItemType[i]);
}

void Spell::EffectPersistentAA(uint32 i)
{
    float radius = GetSpellRadius(sSpellRadiusStore.LookupEntry(m_spellInfo->EffectRadiusIndex[i]));
    if(Player* modOwner = m_originalCaster->GetSpellModOwner())
        modOwner->ApplySpellMod(m_spellInfo->Id, SPELLMOD_RADIUS, radius);

    Unit *caster = m_caster->GetEntry() == WORLD_TRIGGER ? m_originalCaster : m_caster;
    int32 duration = GetSpellDuration(m_spellInfo);
    DynamicObject* dynObj = new DynamicObject;
    if(!dynObj->Create(objmgr.GenerateLowGuid(HIGHGUID_DYNAMICOBJECT), caster, m_spellInfo->Id, i, m_targets.m_destX, m_targets.m_destY, m_targets.m_destZ, duration, radius))
    {
        delete dynObj;
        return;
    }
    dynObj->SetUInt32Value(OBJECT_FIELD_TYPE, 65);
    dynObj->SetUInt32Value(DYNAMICOBJECT_BYTES, 0x01eeeeee);
    caster->AddDynObject(dynObj);
    dynObj->GetMap()->Add(dynObj);
}

void Spell::EffectEnergize(uint32 i)
{
    if(!unitTarget)
        return;
    if(!unitTarget->isAlive())
        return;

    if(m_spellInfo->EffectMiscValue[i] < 0 || m_spellInfo->EffectMiscValue[i] >= MAX_POWERS)
        return;

    // Some level depends spells
    int multiplier = 0;
    int level_diff = 0;
    switch (m_spellInfo->Id)
    {
        // Restore Energy
        case 9512:
            level_diff = m_caster->getLevel() - 40;
            multiplier = 2;
            break;
        // Blood Fury
        case 24571:
            level_diff = m_caster->getLevel() - 60;
            multiplier = 10;
            break;
        // Burst of Energy
        case 24532:
            level_diff = m_caster->getLevel() - 60;
            multiplier = 4;
            break;
        // Mana gems with item 30720
        case 5405:
        case 10052:
        case 10057:
        case 10058:
        case 27103:
            if (m_caster->HasAura(37447))
                m_caster->CastSpell(m_caster, 37445, true);
            break;
        //Elune's Touch (30% AP)
        case 33926:
            damage = m_caster->GetTotalAttackPowerValue(BASE_ATTACK, unitTarget) * 30 / 100;
            break;
        default:
            break;
    }

    if (level_diff > 0)
        damage -= multiplier * level_diff;

    if(damage < 0)
        return;

    Powers power = Powers(m_spellInfo->EffectMiscValue[i]);

    if(unitTarget->GetMaxPower(power) == 0)
        return;

    unitTarget->ModifyPower(power,damage);
    m_caster->SendEnergizeSpellLog(unitTarget, m_spellInfo->Id, damage, power);

    // Mad Alchemist's Potion
    if (m_spellInfo->Id == 45051)
    {
        // find elixirs on target
        uint32 elixir_mask = 0;
        Unit::AuraMap& Auras = unitTarget->GetAuras();
        for(Unit::AuraMap::iterator itr = Auras.begin(); itr != Auras.end(); ++itr)
        {
            uint32 spell_id = itr->second->GetId();
            if(uint32 mask = spellmgr.GetSpellElixirMask(spell_id))
                elixir_mask |= mask;
        }

        // get available elixir mask any not active type from battle/guardian (and flask if no any)
        elixir_mask = (elixir_mask & ELIXIR_FLASK_MASK) ^ ELIXIR_FLASK_MASK;

        // get all available elixirs by mask and spell level
        std::vector<uint32> elixirs;
        SpellElixirMap const& m_spellElixirs = spellmgr.GetSpellElixirMap();
        for(SpellElixirMap::const_iterator itr = m_spellElixirs.begin(); itr != m_spellElixirs.end(); ++itr)
        {
            if (itr->second & elixir_mask)
            {
                if (itr->second & (ELIXIR_UNSTABLE_MASK | ELIXIR_SHATTRATH_MASK))
                    continue;

                SpellEntry const *spellInfo = spellmgr.LookupSpell(itr->first);
                if (spellInfo && (spellInfo->spellLevel < m_spellInfo->spellLevel || spellInfo->spellLevel > unitTarget->getLevel()))
                    continue;

                elixirs.push_back(itr->first);
            }
        }

        if (!elixirs.empty())
        {
            // cast random elixir on target
          uint32 rand_spell = m_caster->GetMap()->urand(0,elixirs.size()-1);
            m_caster->CastSpell(unitTarget,elixirs[rand_spell],true,m_CastItem);
        }
    }
}

void Spell::EffectEnergisePct(uint32 i)
{
    if(!unitTarget)
        return;
    if(!unitTarget->isAlive())
        return;

    if(m_spellInfo->EffectMiscValue[i] < 0 || m_spellInfo->EffectMiscValue[i] >= MAX_POWERS)
        return;

    Powers power = Powers(m_spellInfo->EffectMiscValue[i]);

    uint32 maxPower = unitTarget->GetMaxPower(power);
    if(maxPower == 0)
        return;

    uint32 gain = damage * maxPower / 100;
    unitTarget->ModifyPower(power, gain);
    m_caster->SendEnergizeSpellLog(unitTarget, m_spellInfo->Id, damage, power);
}

void Spell::SendLoot(uint64 guid, LootType loottype)
{
    Player* player = m_caster->ToPlayer();
    if (!player)
        return;

    if (gameObjTarget)
    {
        if (sScriptMgr.GOHello(player, gameObjTarget))
            return;
            
        gameObjTarget->AI()->GossipHello(player);

        switch (gameObjTarget->GetGoType())
        {
            case GAMEOBJECT_TYPE_DOOR:
            case GAMEOBJECT_TYPE_BUTTON:
                gameObjTarget->UseDoorOrButton();
                sWorld.ScriptsStart(sGameObjectScripts, gameObjTarget->GetDBTableGUIDLow(), player, gameObjTarget);
                return;

            case GAMEOBJECT_TYPE_QUESTGIVER:
                // start or end quest
                player->PrepareQuestMenu(guid);
                player->SendPreparedQuest(guid);
                return;

            case GAMEOBJECT_TYPE_SPELL_FOCUS:
                // triggering linked GO
                if(uint32 trapEntry = gameObjTarget->GetGOInfo()->spellFocus.linkedTrapId)
                    gameObjTarget->TriggeringLinkedGameObject(trapEntry,m_caster);
                return;

            case GAMEOBJECT_TYPE_GOOBER:
                // goober_scripts can be triggered if the player don't have the quest
                if (gameObjTarget->GetGOInfo()->goober.eventId)
                {
                    sWorld.ScriptsStart(sEventScripts, gameObjTarget->GetGOInfo()->goober.eventId, player, gameObjTarget);
                }

                // cast goober spell
                if (gameObjTarget->GetGOInfo()->goober.questId)
                    ///Quest require to be active for GO using
                    if(player->GetQuestStatus(gameObjTarget->GetGOInfo()->goober.questId) != QUEST_STATUS_INCOMPLETE)
                        return;

                sScriptMgr.GOHello(player, gameObjTarget);
                sWorld.ScriptsStart(sGameObjectScripts, gameObjTarget->GetDBTableGUIDLow(), player, gameObjTarget);

                gameObjTarget->AddUniqueUse(player);
                gameObjTarget->SetLootState(GO_JUST_DEACTIVATED);

                //TODO? Objective counting called without spell check but with quest objective check
                // if send spell id then this line will duplicate to spell casting call (double counting)
                // So we or have this line and not required in quest_template have reqSpellIdN
                // or must remove this line and required in DB have data in quest_template have reqSpellIdN for all quest using cases.
                player->CastedCreatureOrGO(gameObjTarget->GetEntry(), gameObjTarget->GetGUID(), 0);

                // triggering linked GO
                if(uint32 trapEntry = gameObjTarget->GetGOInfo()->goober.linkedTrapId)
                    gameObjTarget->TriggeringLinkedGameObject(trapEntry,m_caster);

                return;

            case GAMEOBJECT_TYPE_CHEST:
                // TODO: possible must be moved to loot release (in different from linked triggering)
                if (gameObjTarget->GetGOInfo()->chest.eventId)
                {
                    sWorld.ScriptsStart(sEventScripts, gameObjTarget->GetGOInfo()->chest.eventId, player, gameObjTarget);
                }

                // triggering linked GO
                if(uint32 trapEntry = gameObjTarget->GetGOInfo()->chest.linkedTrapId)
                    gameObjTarget->TriggeringLinkedGameObject(trapEntry,m_caster);

                // Don't return, let loots been taken
        }
    }

    // Send loot
    player->SendLoot(guid, loottype);
}

void Spell::EffectOpenLock(uint32 /*i*/)
{
    if(!m_caster || m_caster->GetTypeId() != TYPEID_PLAYER)
    {
        sLog.outError( "WORLD: Open Lock - No Player Caster!");
        return;
    }

    Player* player = m_caster->ToPlayer();

    LootType loottype = LOOT_CORPSE;
    uint32 lockId = 0;
    uint64 guid = 0;
    
    if (m_spellInfo->Id == 18762 && m_caster->GetTypeId() == TYPEID_PLAYER)
        m_caster->ToPlayer()->KilledMonster(11937, 0);

    // Get lockId
    if(gameObjTarget)
    {
        GameObjectInfo const* goInfo = gameObjTarget->GetGOInfo();
        // Arathi Basin banner opening !
        if( goInfo->type == GAMEOBJECT_TYPE_BUTTON && goInfo->button.noDamageImmune ||
            goInfo->type == GAMEOBJECT_TYPE_GOOBER && goInfo->goober.losOK )
        {
            //isAllowUseBattleGroundObject() already called in CanCast()
            // in battleground check
            if(BattleGround *bg = player->GetBattleGround())
            {
                // check if it's correct bg
                if(bg->GetTypeID() == BATTLEGROUND_AB || bg->GetTypeID() == BATTLEGROUND_AV)
                    bg->EventPlayerClickedOnFlag(player, gameObjTarget);
                return;
            }
        }
        else if (goInfo->type == GAMEOBJECT_TYPE_FLAGSTAND)
        {
            //isAllowUseBattleGroundObject() already called in CanCast()
            // in battleground check
            if(BattleGround *bg = player->GetBattleGround())
            {
                if(bg->GetTypeID() == BATTLEGROUND_EY)
                    bg->EventPlayerClickedOnFlag(player, gameObjTarget);
                return;
            }
        }
        // handle outdoor pvp object opening, return true if go was registered for handling
        // these objects must have been spawned by outdoorpvp!
        else if(gameObjTarget->GetGOInfo()->type == GAMEOBJECT_TYPE_GOOBER && sOutdoorPvPMgr.HandleOpenGo(player, gameObjTarget->GetGUID()))
            return;
        lockId = gameObjTarget->GetLockId();
        guid = gameObjTarget->GetGUID();
    }
    else if(itemTarget)
    {
        lockId = itemTarget->GetProto()->LockID;
        guid = itemTarget->GetGUID();
    }
    else
    {
        sLog.outError( "WORLD: Open Lock - No GameObject/Item Target!");
        return;
    }

    if(!lockId)                                             // possible case for GO and maybe for items.
    {
        SendLoot(guid, loottype);
        return;
    }

    // Get LockInfo
    LockEntry const *lockInfo = sLockStore.LookupEntry(lockId);

    if (!lockInfo)
    {
        sLog.outError( "Spell::EffectOpenLock: %s [guid = %u] has an unknown lockId: %u!",
            (gameObjTarget ? "gameobject" : "item"), GUID_LOPART(guid), lockId);
        SendCastResult(SPELL_FAILED_BAD_TARGETS);
        return;
    }

    // check key
    for(int i = 0; i < 5; ++i)
    {
        // type==1 This means lockInfo->key[i] is an item
        if(lockInfo->keytype[i]==LOCK_KEY_ITEM && lockInfo->key[i] && m_CastItem && m_CastItem->GetEntry()==lockInfo->key[i])
        {
            if (gameObjTarget && gameObjTarget->GetEntry() == 184849) // Mechanar Cache of the Legion
                gameObjTarget->setManualUnlocked();

            SendLoot(guid, loottype);
            return;
        }
    }

    uint32 SkillId = 0;
    // Check and skill-up skill
    if( m_spellInfo->Effect[1] == SPELL_EFFECT_SKILL )
        SkillId = m_spellInfo->EffectMiscValue[1];
                                                            // pickpocketing spells
    else if( m_spellInfo->EffectMiscValue[0] == LOCKTYPE_PICKLOCK )
        SkillId = SKILL_LOCKPICKING;

    // skill bonus provided by casting spell (mostly item spells)
    uint32 spellSkillBonus = uint32(damage/*m_currentBasePoints[0]+1*/);

    uint32 reqSkillValue = lockInfo->requiredminingskill;

    if(lockInfo->requiredlockskill)                         // required pick lock skill applying
    {
        if(SkillId != SKILL_LOCKPICKING)                    // wrong skill (cheating?)
        {
            SendCastResult(SPELL_FAILED_FIZZLE);
            return;
        }

        reqSkillValue = lockInfo->requiredlockskill;
    }
    else if(SkillId == SKILL_LOCKPICKING)                   // apply picklock skill to wrong target
    {
        SendCastResult(SPELL_FAILED_BAD_TARGETS);
        return;
    }

    if ( SkillId )
    {
        loottype = LOOT_SKINNING;
        if ( player->GetSkillValue(SkillId) + spellSkillBonus < reqSkillValue )
        {
            SendCastResult(SPELL_FAILED_LOW_CASTLEVEL);
            return;
        }

        // update skill if really known
        uint32 SkillValue = player->GetPureSkillValue(SkillId);
        if(SkillValue)                                      // non only item base skill
        {
            if(gameObjTarget)
            {
                // Allow one skill-up until respawned
                if ( !gameObjTarget->IsInSkillupList( player->GetGUIDLow() ) &&
                    player->UpdateGatherSkill(SkillId, SkillValue, reqSkillValue) )
                    gameObjTarget->AddToSkillupList( player->GetGUIDLow() );
            }
            else if(itemTarget)
            {
                // Do one skill-up
                uint32 SkillValue = player->GetPureSkillValue(SkillId);
                player->UpdateGatherSkill(SkillId, SkillValue, reqSkillValue);
            }
        }
    }

    SendLoot(guid, loottype);
}

void Spell::EffectSummonChangeItem(uint32 i)
{
    if(m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    Player *player = m_caster->ToPlayer();

    // applied only to using item
    if(!m_CastItem)
        return;

    // ... only to item in own inventory/bank/equip_slot
    if(m_CastItem->GetOwnerGUID()!=player->GetGUID())
        return;

    uint32 newitemid = m_spellInfo->EffectItemType[i];
    if(!newitemid)
        return;

    uint16 pos = m_CastItem->GetPos();

    Item *pNewItem = Item::CreateItem( newitemid, 1, player);
    if( !pNewItem )
        return;

    for(uint8 i= PERM_ENCHANTMENT_SLOT; i<=TEMP_ENCHANTMENT_SLOT; ++i)
    {
        if(m_CastItem->GetEnchantmentId(EnchantmentSlot(i)))
            pNewItem->SetEnchantment(EnchantmentSlot(i), m_CastItem->GetEnchantmentId(EnchantmentSlot(i)), m_CastItem->GetEnchantmentDuration(EnchantmentSlot(i)), m_CastItem->GetEnchantmentCharges(EnchantmentSlot(i)));
    }

    if(m_CastItem->GetUInt32Value(ITEM_FIELD_DURABILITY) < m_CastItem->GetUInt32Value(ITEM_FIELD_MAXDURABILITY))
    {
        double loosePercent = 1 - m_CastItem->GetUInt32Value(ITEM_FIELD_DURABILITY) / double(m_CastItem->GetUInt32Value(ITEM_FIELD_MAXDURABILITY));
        player->DurabilityLoss(pNewItem, loosePercent);
    }

    if( player->IsInventoryPos( pos ) )
    {
        ItemPosCountVec dest;
        uint8 msg = player->CanStoreItem( m_CastItem->GetBagSlot(), m_CastItem->GetSlot(), dest, pNewItem, true );
        if( msg == EQUIP_ERR_OK )
        {
            player->DestroyItem(m_CastItem->GetBagSlot(), m_CastItem->GetSlot(),true);

            // prevent crash at access and unexpected charges counting with item update queue corrupt
            if(m_CastItem==m_targets.getItemTarget())
                m_targets.setItemTarget(NULL);

            m_CastItem = NULL;

            player->StoreItem( dest, pNewItem, true);
            return;
        }
    }
    else if( player->IsBankPos ( pos ) )
    {
        ItemPosCountVec dest;
        uint8 msg = player->CanBankItem( m_CastItem->GetBagSlot(), m_CastItem->GetSlot(), dest, pNewItem, true );
        if( msg == EQUIP_ERR_OK )
        {
            player->DestroyItem(m_CastItem->GetBagSlot(), m_CastItem->GetSlot(),true);

            // prevent crash at access and unexpected charges counting with item update queue corrupt
            if(m_CastItem==m_targets.getItemTarget())
                m_targets.setItemTarget(NULL);

            m_CastItem = NULL;

            player->BankItem( dest, pNewItem, true);
            return;
        }
    }
    else if( player->IsEquipmentPos ( pos ) )
    {
        uint16 dest;
        uint8 msg = player->CanEquipItem( m_CastItem->GetSlot(), dest, pNewItem, true );
        if( msg == EQUIP_ERR_OK )
        {
            player->DestroyItem(m_CastItem->GetBagSlot(), m_CastItem->GetSlot(),true);

            // prevent crash at access and unexpected charges counting with item update queue corrupt
            if(m_CastItem==m_targets.getItemTarget())
                m_targets.setItemTarget(NULL);

            m_CastItem = NULL;

            player->EquipItem( dest, pNewItem, true);
            player->AutoUnequipOffhandIfNeed();
            return;
        }
    }

    // fail
    delete pNewItem;
}

void Spell::EffectOpenSecretSafe(uint32 i)
{
    EffectOpenLock(i);                                      //no difference for now
}

void Spell::EffectProficiency(uint32 /*i*/)
{
    if (!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;
    Player *p_target = unitTarget->ToPlayer();

    uint32 subClassMask = m_spellInfo->EquippedItemSubClassMask;
    if(m_spellInfo->EquippedItemClass == 2 && !(p_target->GetWeaponProficiency() & subClassMask))
    {
        p_target->AddWeaponProficiency(subClassMask);
        p_target->SendProficiency(uint8(0x02),p_target->GetWeaponProficiency());
    }
    if(m_spellInfo->EquippedItemClass == 4 && !(p_target->GetArmorProficiency() & subClassMask))
    {
        p_target->AddArmorProficiency(subClassMask);
        p_target->SendProficiency(uint8(0x04),p_target->GetArmorProficiency());
    }
}

void Spell::EffectApplyAreaAura(uint32 i)
{
    if(!unitTarget)
        return;
    if(!unitTarget->isAlive())
        return;

    AreaAura* Aur = new AreaAura(m_spellInfo, i, &damage, unitTarget, m_caster, m_CastItem);
    unitTarget->AddAura(Aur);
}

void Spell::EffectSummonType(uint32 i)
{
    switch(m_spellInfo->EffectMiscValueB[i])
    {
        case SUMMON_TYPE_GUARDIAN:
            EffectSummonGuardian(i);
            break;
        case SUMMON_TYPE_POSESSED:
        case SUMMON_TYPE_POSESSED2:
        case SUMMON_TYPE_POSESSED3:
            EffectSummonPossessed(i);
            break;
        case SUMMON_TYPE_WILD:
            EffectSummonWild(i);
            break;
        case SUMMON_TYPE_DEMON:
            EffectSummonDemon(i);
            break;
        case SUMMON_TYPE_SUMMON:
            EffectSummon(i);
            break;
        case SUMMON_TYPE_CRITTER:
        case SUMMON_TYPE_CRITTER2:
        case SUMMON_TYPE_CRITTER3:
            EffectSummonCritter(i);
            break;
        case SUMMON_TYPE_TOTEM_SLOT1:
        case SUMMON_TYPE_TOTEM_SLOT2:
        case SUMMON_TYPE_TOTEM_SLOT3:
        case SUMMON_TYPE_TOTEM_SLOT4:
        case SUMMON_TYPE_TOTEM:
            EffectSummonTotem(i);
            break;
        case SUMMON_TYPE_UNKNOWN1:
        case SUMMON_TYPE_UNKNOWN3:
        case SUMMON_TYPE_UNKNOWN4:
        case SUMMON_TYPE_UNKNOWN5:
            break;
        default:
            sLog.outError("EffectSummonType: Unhandled summon type %u", m_spellInfo->EffectMiscValueB[i]);
            break;
    }
}

void Spell::EffectSummon(uint32 i)
{
    uint32 pet_entry = m_spellInfo->EffectMiscValue[i];
    if(!pet_entry)
        return;

    if(!m_originalCaster || m_originalCaster->GetTypeId() != TYPEID_PLAYER)
    {
        EffectSummonWild(i);
        return;
    }

    Player *owner = m_originalCaster->ToPlayer();

    if(owner->GetPetGUID())
        return;

    // Summon in dest location
    float x,y,z;
    if(m_targets.m_targetMask & TARGET_FLAG_DEST_LOCATION)
    {
        x = m_targets.m_destX;
        y = m_targets.m_destY;
        z = m_targets.m_destZ;
    }
    else
        m_caster->GetClosePoint(x,y,z,owner->GetObjectSize());

    Pet *spawnCreature = owner->SummonPet(pet_entry, x, y, z, m_caster->GetOrientation(), SUMMON_PET, GetSpellDuration(m_spellInfo));
    if(!spawnCreature)
        return;

    spawnCreature->SetUInt32Value(UNIT_NPC_FLAGS, 0);
    spawnCreature->SetUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP, 0);
    spawnCreature->SetUInt32Value(UNIT_CREATED_BY_SPELL, m_spellInfo->Id);

    /*std::string name = owner->GetName();
    name.append(petTypeSuffix[spawnCreature->getPetType()]);*/
    spawnCreature->SetName(spawnCreature->GetNameForLocaleIdx(owner->GetSession()->GetSessionDbLocaleIndex()));

    spawnCreature->SetReactState( REACT_DEFENSIVE );
}

void Spell::EffectLearnSpell(uint32 i)
{
    if(!unitTarget)
        return;

    if(unitTarget->GetTypeId() != TYPEID_PLAYER)
    {
        if(m_caster->GetTypeId() == TYPEID_PLAYER)
            EffectLearnPetSpell(i);

        return;
    }

    Player *player = unitTarget->ToPlayer();

    uint32 spellToLearn = (m_spellInfo->Id==SPELL_ID_GENERIC_LEARN) ? damage : m_spellInfo->EffectTriggerSpell[i];
    player->learnSpell(spellToLearn);
}

void Spell::EffectDispel(uint32 i)
{
    if(!unitTarget)
        return;

    // Fill possible dispel list
    std::vector <Aura *> dispel_list;
    if (unitTarget->IsHostileTo(m_caster) && (m_spellInfo->SpellVisual != 3299 && m_spellInfo->SpellIconID != 218) /* Arcane Shot */)   // TODO: Better fix would be if unitTarget is creature, then add CombatStart
    {
        if (unitTarget->ToCreature() && !(unitTarget->ToCreature()->isPet()))
            unitTarget->ToCreature()->AI()->AttackStart(m_caster);
        m_caster->SetInCombatWith(unitTarget);
        unitTarget->SetInCombatWith(m_caster);
        if (unitTarget->IsSitState())
            unitTarget->SetStandState(UNIT_STAND_STATE_STAND);
    }

    // Create dispel mask by dispel type
    uint32 dispel_type = m_spellInfo->EffectMiscValue[i];
    uint32 dispelMask  = GetDispellMask( DispelType(dispel_type) );
    Unit::AuraMap const& auras = unitTarget->GetAuras();
    for(Unit::AuraMap::const_iterator itr = auras.begin(); itr != auras.end(); ++itr)
    {
        Aura *aur = (*itr).second;
        if (aur && (1<<aur->GetSpellProto()->Dispel) & dispelMask)
        {
            if(aur->GetSpellProto()->Dispel == DISPEL_MAGIC)
            {
                bool positive = true;
                if (!aur->IsPositive())
                    positive = false;
                else
                    positive = (aur->GetSpellProto()->AttributesEx & SPELL_ATTR_EX_NEGATIVE)==0;

                // do not remove positive auras if friendly target
                //               negative auras if non-friendly target
                if(positive == unitTarget->IsFriendlyTo(m_caster))
                    continue;
            }
            // Add every aura stack to dispel list
            for(uint32 stack_amount = 0; stack_amount < aur->GetStackAmount(); ++stack_amount)
                dispel_list.push_back(aur);
        }
    }
    // Ok if exist some buffs for dispel try dispel it
    if (!dispel_list.empty())
    {
        std::list < std::pair<uint32,uint64> > success_list;// (spell_id,casterGuid)
        std::list < uint32 > fail_list;                     // spell_id
        int32 list_size = dispel_list.size();
        // dispel N = damage buffs (or while exist buffs for dispel)
        for (int32 count=0; count < damage && list_size > 0; ++count)
        {
            // Random select buff for dispel
          Aura *aur = dispel_list[m_caster->GetMap()->urand(0, list_size-1)];

            SpellEntry const* spellInfo = aur->GetSpellProto();
            // Base dispel chance
            // TODO: possible chance depend from spell level??
            int32 miss_chance = 0;
            // Apply dispel mod from aura caster
            if (Unit *caster = aur->GetCaster())
            {
                if ( Player* modOwner = caster->GetSpellModOwner() )
                    modOwner->ApplySpellMod(spellInfo->Id, SPELLMOD_RESIST_DISPEL_CHANCE, miss_chance, this);
            }
            // Try dispel
            if (roll_chance_i(miss_chance))
                fail_list.push_back(aur->GetId());
            else
                success_list.push_back(std::pair<uint32,uint64>(aur->GetId(),aur->GetCasterGUID()));
            // Remove buff from list for prevent doubles
            for (std::vector<Aura *>::iterator j = dispel_list.begin(); j != dispel_list.end(); )
            {
                Aura *dispelled = *j;
                if (dispelled->GetId() == aur->GetId() && dispelled->GetCasterGUID() == aur->GetCasterGUID())
                {
                    j = dispel_list.erase(j);
                    --list_size;
                    break;
                }
                else
                    ++j;
            }
        }
        // Send success log and really remove auras
        if (!success_list.empty())
        {
            int32 count = success_list.size();
            WorldPacket data(SMSG_SPELLDISPELLOG, 8+8+4+1+4+count*5);
            data.append(unitTarget->GetPackGUID());         // Victim GUID
            data.append(m_caster->GetPackGUID());           // Caster GUID
            data << uint32(m_spellInfo->Id);                // dispel spell id
            data << uint8(0);                               // not used
            data << uint32(count);                          // count
            for (std::list<std::pair<uint32,uint64> >::iterator j = success_list.begin(); j != success_list.end(); ++j)
            {
                SpellEntry const* spellInfo = spellmgr.LookupSpell(j->first);
                data << uint32(spellInfo->Id);              // Spell Id
                data << uint8(0);                           // 0 - dispelled !=0 cleansed
                if(spellInfo->StackAmount!= 0)
                {
                    //Why are Aura's Removed by EffIndex? Auras should be removed as a whole.....
                    unitTarget->RemoveSingleAuraFromStackByDispel(spellInfo->Id);
                }
                else
                unitTarget->RemoveAurasDueToSpellByDispel(spellInfo->Id, j->second, m_caster);
             }
            m_caster->SendMessageToSet(&data, true);

            // On succes dispel
            // Devour Magic
            if (m_spellInfo->SpellFamilyName == SPELLFAMILY_WARLOCK && m_spellInfo->Category == 12)
            {
                uint32 heal_spell = 0;
                switch (m_spellInfo->Id)
                {
                    case 19505: heal_spell = 19658; break;
                    case 19731: heal_spell = 19732; break;
                    case 19734: heal_spell = 19733; break;
                    case 19736: heal_spell = 19735; break;
                    case 27276: heal_spell = 27278; break;
                    case 27277: heal_spell = 27279; break;
                    default:
                        sLog.outError("Spell for Devour Magic %d not handled in Spell::EffectDispel", m_spellInfo->Id);
                        break;
                }
                if (heal_spell)
                    m_caster->CastSpell(m_caster, heal_spell, true);
            }
        }
        // Send fail log to client
        if (!fail_list.empty())
        {
            // Failed to dispell
            WorldPacket data(SMSG_DISPEL_FAILED, 8+8+4+4*fail_list.size());
            data << uint64(m_caster->GetGUID());            // Caster GUID
            data << uint64(unitTarget->GetGUID());          // Victim GUID
            data << uint32(m_spellInfo->Id);                // dispel spell id
            for (std::list< uint32 >::iterator j = fail_list.begin(); j != fail_list.end(); ++j)
                data << uint32(*j);                         // Spell Id
            m_caster->SendMessageToSet(&data, true);
        }
    }
}

void Spell::EffectDualWield(uint32 /*i*/)
{
    unitTarget->SetCanDualWield(true);
    if(unitTarget->GetTypeId() == TYPEID_UNIT)
        (unitTarget->ToCreature())->UpdateDamagePhysical(OFF_ATTACK);
}

void Spell::EffectPull(uint32 /*i*/)
{
    // TODO: create a proper pull towards distract spell center for distract
}

void Spell::EffectDistract(uint32 /*i*/)
{
    // Check for possible target
    if (!unitTarget || unitTarget->isInCombat())
        return;

    // target must be OK to do this
    if( unitTarget->hasUnitState(UNIT_STAT_CONFUSED | UNIT_STAT_STUNNED | UNIT_STAT_FLEEING ) )
        return;

    float angle = unitTarget->GetAngle(m_targets.m_destX, m_targets.m_destY);

    if ( unitTarget->GetTypeId() == TYPEID_PLAYER )
    {
        // For players just turn them
        WorldPacket data;
        (unitTarget->ToPlayer())->BuildTeleportAckMsg(&data, unitTarget->GetPositionX(), unitTarget->GetPositionY(), unitTarget->GetPositionZ(), angle);
        (unitTarget->ToPlayer())->GetSession()->SendPacket( &data );
        (unitTarget->ToPlayer())->SetPosition(unitTarget->GetPositionX(), unitTarget->GetPositionY(), unitTarget->GetPositionZ(), angle, false);
    }
    else
    {
        // Set creature Distracted, Stop it, And turn it
        unitTarget->SetOrientation(angle);
        unitTarget->StopMoving();
        unitTarget->GetMotionMaster()->MoveDistract(damage*1000);
    }
}

void Spell::EffectPickPocket(uint32 /*i*/)
{
    if( m_caster->GetTypeId() != TYPEID_PLAYER )
        return;

    // victim must be creature and attackable
    if( !unitTarget || unitTarget->GetTypeId() != TYPEID_UNIT || m_caster->IsFriendlyTo(unitTarget) )
        return;

    // victim have to be alive and humanoid or undead
    if( unitTarget->isAlive() && (unitTarget->GetCreatureTypeMask() &CREATURE_TYPEMASK_HUMANOID_OR_UNDEAD) != 0)
    {
        int32 chance = 10 + int32(m_caster->getLevel()) - int32(unitTarget->getLevel());

        if (chance > m_caster->GetMap()->irand(0, 19))
        {
            // Stealing successful
            //sLog.outDebug("Sending loot from pickpocket");
            (m_caster->ToPlayer())->SendLoot(unitTarget->GetGUID(),LOOT_PICKPOCKETING);
        }
        else
        {
            // Reveal action + get attack
            m_caster->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_TALK);
            if ((unitTarget->ToCreature())->IsAIEnabled) {
                (unitTarget->ToCreature())->AI()->AttackStart(m_caster);
                if (unitTarget->ToCreature()->getAI())
                    unitTarget->ToCreature()->getAI()->attackStart(m_caster);
            }
        }
    }
}

void Spell::EffectAddFarsight(uint32 i)
{
    if (m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    float radius = GetSpellRadius(sSpellRadiusStore.LookupEntry(m_spellInfo->EffectRadiusIndex[i]));
    int32 duration = GetSpellDuration(m_spellInfo);
    DynamicObject* dynObj = new DynamicObject;
    if(!dynObj->Create(objmgr.GenerateLowGuid(HIGHGUID_DYNAMICOBJECT), m_caster, m_spellInfo->Id, 4, m_targets.m_destX, m_targets.m_destY, m_targets.m_destZ, duration, radius))
    {
        delete dynObj;
        return;
    }
    dynObj->SetUInt32Value(OBJECT_FIELD_TYPE, 65);
    dynObj->SetUInt32Value(DYNAMICOBJECT_BYTES, 0x80000002);
    m_caster->AddDynObject(dynObj);

    dynObj->setActive(true);    //must before add to map to be put in world container
    dynObj->GetMap()->Add(dynObj); //grid will also be loaded

    // Need to update visibility of object for client to accept farsight guid
    (m_caster->ToPlayer())->UpdateVisibilityOf(dynObj);
    (m_caster->ToPlayer())->SetFarsightTarget(dynObj);
}

void Spell::EffectSummonWild(uint32 i)
{
    uint32 creature_entry = m_spellInfo->EffectMiscValue[i];
    if(!creature_entry)
        return;
    
    // Hack
    if (m_spellInfo->Id == 25823)
        creature_entry = 18342;

    uint32 level = m_caster->getLevel();

    // level of creature summoned using engineering item based at engineering skill level
    if(m_caster->GetTypeId()==TYPEID_PLAYER && m_CastItem)
    {
        ItemPrototype const *proto = m_CastItem->GetProto();
        if(proto && proto->RequiredSkill == SKILL_ENGINERING)
        {
            uint16 skill202 = (m_caster->ToPlayer())->GetSkillValue(SKILL_ENGINERING);
            if(skill202)
            {
                level = skill202/5;
            }
        }
    }

    // select center of summon position
    float center_x = m_targets.m_destX;
    float center_y = m_targets.m_destY;
    float center_z = m_targets.m_destZ;

    float radius = GetSpellRadius(sSpellRadiusStore.LookupEntry(m_spellInfo->EffectRadiusIndex[i]));

    int32 amount = damage > 0 ? damage : 1;

    for(int32 count = 0; count < amount; ++count)
    {
        float px, py, pz;
        // If dest location if present
        if (m_targets.m_targetMask & TARGET_FLAG_DEST_LOCATION)
        {
            // Summon 1 unit in dest location
            if (count == 0)
            {
                px = m_targets.m_destX;
                py = m_targets.m_destY;
                pz = m_targets.m_destZ;
            }
            // Summon in random point all other units if location present
            else
                m_caster->GetRandomPoint(center_x,center_y,center_z,radius,px,py,pz);
        }
        // Summon if dest location not present near caster
        else
            m_caster->GetClosePoint(px,py,pz,3.0f);

        int32 duration = GetSpellDuration(m_spellInfo);

        TempSummonType summonType = (duration == 0) ? TEMPSUMMON_DEAD_DESPAWN : TEMPSUMMON_TIMED_DESPAWN;

        if(m_originalCaster)
        {
            Creature* Charmed = m_originalCaster->SummonCreature(creature_entry,px,py,pz,m_caster->GetOrientation(),summonType,duration);
            if (Charmed)
                Charmed->SetSummoner(m_caster);
            if (creature_entry == 12922 || creature_entry == 8996) //Summoned Imp/Voidwalker by many NPCs, they're all level 46, even if summoner has a different level
            {
                Charmed->SetLevel(m_originalCaster->getLevel());
                Charmed->setFaction(m_originalCaster->getFaction());
            }
        }
        else
        {
            Creature* Charmed = m_caster->SummonCreature(creature_entry,px,py,pz,m_caster->GetOrientation(),summonType,duration);
            if (Charmed)
                Charmed->SetSummoner(m_caster);
            if (creature_entry == 12922 || creature_entry == 8996) //Summoneded Imp/Voidwalker by many NPCs, they're all level 46, even if summoner has a different level
            {
                Charmed->SetLevel(m_caster->getLevel());
                Charmed->setFaction(m_caster->getFaction());
                if (Charmed->isPet())
                    Charmed->ToPet()->InitStatsForLevel(Charmed->getLevel());
            }
        }
            
    }
    
    //handle special cases
    switch(m_spellInfo->Id)
    {
        //quest 9711
        case 31333:
        {
            if (m_caster->GetTypeId() == TYPEID_PLAYER)
                (m_caster->ToPlayer())->AreaExploredOrEventHappens(9711);
            break;
        }
        case 26286:
        case 26291:
        case 26292:
        case 26294:
        case 26295:
        case 26293:
            if (m_caster->GetTypeId() == TYPEID_PLAYER)
                m_caster->ToPlayer()->KilledMonster(15893, 0);
            break;
        case 26333:
        case 26334:
        case 26336:
        case 26337:
        case 26338:
        case 26335:
            if (m_caster->GetTypeId() == TYPEID_PLAYER)
                m_caster->ToPlayer()->KilledMonster(15893, 0);
            break;
        case 26516:
        case 26517:
        case 26518:
        case 26519:
        case 26488:
        case 26490:
        case 26325:
        case 26304:
        case 26329:
        case 26328:
        case 26327:
        case 26326:
            if (m_caster->GetTypeId() == TYPEID_PLAYER)
                m_caster->ToPlayer()->KilledMonster(15894, 0);
            break;
        default:
            break;
    }
}

void Spell::EffectSummonGuardian(uint32 i)
{
    uint32 pet_entry = m_spellInfo->EffectMiscValue[i];
    if(!pet_entry)
        return;

    // Jewelery statue case (totem like)
    if(m_spellInfo->SpellIconID==2056)
    {
        EffectSummonTotem(i);
        return;
    }

    Player *caster = NULL;
    if(m_originalCaster)
    {
        if(m_originalCaster->GetTypeId() == TYPEID_PLAYER)
            caster = m_originalCaster->ToPlayer();
        else if((m_originalCaster->ToCreature())->isTotem())
            caster = m_originalCaster->GetCharmerOrOwnerPlayerOrPlayerItself();
    }

    if(!caster)
    {
        EffectSummonWild(i);
        return;
    }

    // set timer for unsummon
    int32 duration = GetSpellDuration(m_spellInfo);

    // Search old Guardian only for players (if casted spell not have duration or cooldown)
    // FIXME: some guardians have control spell applied and controlled by player and anyway player can't summon in this time
    //        so this code hack in fact
    if(duration <= 0 || GetSpellRecoveryTime(m_spellInfo)==0)
        if(caster->HasGuardianWithEntry(pet_entry))
            return;                                         // find old guardian, ignore summon

    // in another case summon new
    uint32 level = caster->getLevel();

    // level of pet summoned using engineering item based at engineering skill level
    if(m_CastItem)
    {
        ItemPrototype const *proto = m_CastItem->GetProto();
        if(proto && proto->RequiredSkill == SKILL_ENGINERING)
        {
            uint16 skill202 = caster->GetSkillValue(SKILL_ENGINERING);
            if(skill202)
            {
                level = skill202/5;
            }
        }
    }

    // select center of summon position
    float center_x = m_targets.m_destX;
    float center_y = m_targets.m_destY;
    float center_z = m_targets.m_destZ;

    float radius = GetSpellRadius(sSpellRadiusStore.LookupEntry(m_spellInfo->EffectRadiusIndex[i]));

    int32 amount = damage > 0 ? damage : 1;

    for(int32 count = 0; count < amount; ++count)
    {
        float px, py, pz;
        // If dest location if present
        if (m_targets.m_targetMask & TARGET_FLAG_DEST_LOCATION)
        {
            // Summon 1 unit in dest location
            if (count == 0)
            {
                px = m_targets.m_destX;
                py = m_targets.m_destY;
                pz = m_targets.m_destZ;
            }
            // Summon in random point all other units if location present
            else
                m_caster->GetRandomPoint(center_x,center_y,center_z,radius,px,py,pz);
        }
        // Summon if dest location not present near caster
        else
            m_caster->GetClosePoint(px,py,pz,m_caster->GetObjectSize());

        Pet *spawnCreature = caster->SummonPet(m_spellInfo->EffectMiscValue[i], px, py, pz, m_caster->GetOrientation(), GUARDIAN_PET, duration);
        if(!spawnCreature)
            return;

        spawnCreature->SetUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP,0);
        spawnCreature->SetUInt32Value(UNIT_CREATED_BY_SPELL, m_spellInfo->Id);
        spawnCreature->SetFlag(UNIT_FIELD_FLAGS,UNIT_FLAG_PVP_ATTACKABLE);
        if (spawnCreature->GetEntry() == 2673) {        // Target dummy
            spawnCreature->AddAura(42176, spawnCreature);
            spawnCreature->addUnitState(UNIT_STAT_ROOT);
        }
    }
}

void Spell::EffectSummonPossessed(uint32 i)
{
    uint32 entry = m_spellInfo->EffectMiscValue[i];
    if(!entry)
        return;

    if(m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    uint32 level = m_caster->getLevel();

    float x, y, z;
    m_caster->GetClosePoint(x, y, z, DEFAULT_WORLD_OBJECT_SIZE);

    int32 duration = GetSpellDuration(m_spellInfo);

    Pet* pet = (m_caster->ToPlayer())->SummonPet(entry, x, y, z, m_caster->GetOrientation(), POSSESSED_PET, duration);
    if(!pet)
        return;

    pet->SetUInt32Value(UNIT_CREATED_BY_SPELL, m_spellInfo->Id);
    pet->SetCharmedOrPossessedBy(m_caster, true);
}

void Spell::EffectTeleUnitsFaceCaster(uint32 i)
{
    if(!unitTarget)
        return;

    if(unitTarget->isInFlight())
        return;

    uint32 mapid = m_caster->GetMapId();
    float dis = GetSpellRadius(sSpellRadiusStore.LookupEntry(m_spellInfo->EffectRadiusIndex[i]));

    float fx,fy,fz;
    m_caster->GetClosePoint(fx,fy,fz,unitTarget->GetObjectSize(),dis);

    if(unitTarget->GetTypeId() == TYPEID_PLAYER)
        (unitTarget->ToPlayer())->TeleportTo(mapid, fx, fy, fz, -m_caster->GetOrientation(), TELE_TO_NOT_LEAVE_COMBAT | TELE_TO_NOT_UNSUMMON_PET | (unitTarget==m_caster ? TELE_TO_SPELL : 0));
    else
        unitTarget->GetMap()->CreatureRelocation(unitTarget->ToCreature(), fx, fy, fz, -m_caster->GetOrientation());
}

void Spell::EffectLearnSkill(uint32 i)
{
    if(unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    if(damage < 0)
        return;

    uint32 skillid =  m_spellInfo->EffectMiscValue[i];
    uint16 skillval = (unitTarget->ToPlayer())->GetPureSkillValue(skillid);
    (unitTarget->ToPlayer())->SetSkill(skillid, skillval?skillval:1, damage*75);
}

void Spell::EffectAddHonor(uint32 /*i*/)
{
    if(unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    // 2.4.3 honor-spells don't scale with level and won't be casted by an item
	// also we must use damage+1 (spelldescription says +25 honor but damage is only 24)
    (unitTarget->ToPlayer())->RewardHonor(NULL, 1, damage + 1);
}

void Spell::EffectTradeSkill(uint32 /*i*/)
{
    if(unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;
    // uint32 skillid =  m_spellInfo->EffectMiscValue[i];
    // uint16 skillmax = (unitTarget->ToPlayer())->(skillid);
    // (unitTarget->ToPlayer())->SetSkill(skillid,skillval?skillval:1,skillmax+75);
}

void Spell::EffectEnchantItemPerm(uint32 i)
{
    if(m_caster->GetTypeId() != TYPEID_PLAYER)
        return;
    if (!itemTarget)
        return;

    Player* p_caster = m_caster->ToPlayer();

    p_caster->UpdateCraftSkill(m_spellInfo->Id);

    if (m_spellInfo->EffectMiscValue[i])
    {
        uint32 enchant_id = m_spellInfo->EffectMiscValue[i];

        SpellItemEnchantmentEntry const *pEnchant = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
        if(!pEnchant)
            return;

        // item can be in trade slot and have owner diff. from caster
        Player* item_owner = itemTarget->GetOwner();
        if(!item_owner)
            return;

        if(item_owner!=p_caster && p_caster->GetSession()->GetSecurity() > SEC_PLAYER && sWorld.getConfig(CONFIG_GM_LOG_TRADE) )
        {
            sLog.outCommand(p_caster->GetSession()->GetAccountId(),"GM %s (Account: %u) enchanting(perm): %s (Entry: %d) for player: %s (Account: %u)",
                p_caster->GetName(),p_caster->GetSession()->GetAccountId(),
                itemTarget->GetProto()->Name1,itemTarget->GetEntry(),
                item_owner->GetName(),item_owner->GetSession()->GetAccountId());
        }

        // remove old enchanting before applying new if equipped
        item_owner->ApplyEnchantment(itemTarget,PERM_ENCHANTMENT_SLOT,false);

        itemTarget->SetEnchantment(PERM_ENCHANTMENT_SLOT, enchant_id, 0, 0);

        // add new enchanting if equipped
        item_owner->ApplyEnchantment(itemTarget,PERM_ENCHANTMENT_SLOT,true);

		// update trade window for show enchantment for caster in trade window               
		if (m_targets.m_targetMask & TARGET_FLAG_TRADE_ITEM)                       
			p_caster->GetSession()->SendUpdateTrade();
    }
}

void Spell::EffectEnchantItemTmp(uint32 i)
{
    if(m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    Player* p_caster = m_caster->ToPlayer();

    if(!itemTarget)
        return;

    uint32 enchant_id = m_spellInfo->EffectMiscValue[i];

    // Shaman Rockbiter Weapon
    if(i==0 && m_spellInfo->Effect[1]==SPELL_EFFECT_DUMMY)
    {
        int32 enchnting_damage = CalculateDamage(1, NULL);//+1;

        // enchanting id selected by calculated damage-per-sec stored in Effect[1] base value
        // with already applied percent bonus from Elemental Weapons talent
        // Note: damage calculated (correctly) with rounding int32(float(v)) but
        // RW enchantments applied damage int32(float(v)+0.5), this create  0..1 difference sometime
        switch(enchnting_damage)
        {
            // Rank 1
            case  2: enchant_id =   29; break;              //  0% [ 7% ==  2, 14% == 2, 20% == 2]
            // Rank 2
            case  4: enchant_id =    6; break;              //  0% [ 7% ==  4, 14% == 4]
            case  5: enchant_id = 3025; break;              // 20%
            // Rank 3
            case  6: enchant_id =    1; break;              //  0% [ 7% ==  6, 14% == 6]
            case  7: enchant_id = 3027; break;              // 20%
            // Rank 4
            case  9: enchant_id = 3032; break;              //  0% [ 7% ==  6]
            case 10: enchant_id =  503; break;              // 14%
            case 11: enchant_id = 3031; break;              // 20%
            // Rank 5
            case 15: enchant_id = 3035; break;              // 0%
            case 16: enchant_id = 1663; break;              // 7%
            case 17: enchant_id = 3033; break;              // 14%
            case 18: enchant_id = 3034; break;              // 20%
            // Rank 6
            case 28: enchant_id = 3038; break;              // 0%
            case 29: enchant_id =  683; break;              // 7%
            case 31: enchant_id = 3036; break;              // 14%
            case 33: enchant_id = 3037; break;              // 20%
            // Rank 7
            case 40: enchant_id = 3041; break;              // 0%
            case 42: enchant_id = 1664; break;              // 7%
            case 45: enchant_id = 3039; break;              // 14%
            case 48: enchant_id = 3040; break;              // 20%
            // Rank 8
            case 49: enchant_id = 3044; break;              // 0%
            case 52: enchant_id = 2632; break;              // 7%
            case 55: enchant_id = 3042; break;              // 14%
            case 58: enchant_id = 3043; break;              // 20%
            // Rank 9
            case 62: enchant_id = 2633; break;              // 0%
            case 66: enchant_id = 3018; break;              // 7%
            case 70: enchant_id = 3019; break;              // 14%
            case 74: enchant_id = 3020; break;              // 20%
            default:
                sLog.outError("Spell::EffectEnchantItemTmp: Damage %u not handled in S'RW",enchnting_damage);
                return;
        }
    }
    
    // Override wrong DBC values
    /*switch (enchant_id) {
    case 45396: enchant_id = 45403; break;
    case 45398: enchant_id = 45401; break;
    default:    break;
    }*/

    if (!enchant_id)
    {
        sLog.outError("Spell %u Effect %u (SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY) have 0 as enchanting id",m_spellInfo->Id,i);
        return;
    }

    SpellItemEnchantmentEntry const *pEnchant = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
    if(!pEnchant)
    {
        sLog.outError("Spell %u Effect %u (SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY) have not existed enchanting id %u ",m_spellInfo->Id,i,enchant_id);
        return;
    }

    // select enchantment duration
    uint32 duration;

    // rogue family enchantments exception by duration
    if(m_spellInfo->Id==38615)
        duration = 1800;                                    // 30 mins
    // other rogue family enchantments always 1 hour (some have spell damage=0, but some have wrong data in EffBasePoints)
    else if(m_spellInfo->SpellFamilyName==SPELLFAMILY_ROGUE)
        duration = 3600;                                    // 1 hour
    // shaman family enchantments
    else if(m_spellInfo->SpellFamilyName==SPELLFAMILY_SHAMAN)
        duration = 1800;                                    // 30 mins
    // other cases with this SpellVisual already selected
    else if(m_spellInfo->SpellVisual==215)
        duration = 1800;                                    // 30 mins
    // some fishing pole bonuses
    else if(m_spellInfo->SpellVisual==563)
        duration = 600;                                     // 10 mins
    // shaman rockbiter enchantments
    else if(m_spellInfo->SpellVisual==0)
        duration = 1800;                                    // 30 mins
    else if(m_spellInfo->Id==29702)
        duration = 300;                                     // 5 mins
    else if(m_spellInfo->Id==37360)
        duration = 300;                                     // 5 mins
    // default case
    else
        duration = 3600;                                    // 1 hour

    // item can be in trade slot and have owner diff. from caster
    Player* item_owner = itemTarget->GetOwner();
    if(!item_owner)
        return;

    if(item_owner!=p_caster && p_caster->GetSession()->GetSecurity() > SEC_PLAYER && sWorld.getConfig(CONFIG_GM_LOG_TRADE) )
    {
        sLog.outCommand(p_caster->GetSession()->GetAccountId(),"GM %s (Account: %u) enchanting(temp): %s (Entry: %d) for player: %s (Account: %u)",
            p_caster->GetName(),p_caster->GetSession()->GetAccountId(),
            itemTarget->GetProto()->Name1,itemTarget->GetEntry(),
            item_owner->GetName(),item_owner->GetSession()->GetAccountId());
    }

    // remove old enchanting before applying new if equipped
    item_owner->ApplyEnchantment(itemTarget,TEMP_ENCHANTMENT_SLOT,false);

    itemTarget->SetEnchantment(TEMP_ENCHANTMENT_SLOT, enchant_id, duration*1000, 0);

    // add new enchanting if equipped
    item_owner->ApplyEnchantment(itemTarget,TEMP_ENCHANTMENT_SLOT,true);

	// update trade window for show enchantment for caster in trade window       
	if (m_targets.m_targetMask & TARGET_FLAG_TRADE_ITEM)               
		p_caster->GetSession()->SendUpdateTrade(); 
}

void Spell::EffectTameCreature(uint32 /*i*/)
{
    if(m_caster->GetPetGUID())
        return;

    if(!unitTarget)
        return;

    if(unitTarget->GetTypeId() == TYPEID_PLAYER)
        return;

    Creature* creatureTarget = unitTarget->ToCreature();

    if(creatureTarget->isPet())
        return;

    if(m_caster->getClass() != CLASS_HUNTER)
        return;

    // cast finish successfully
    //SendChannelUpdate(0);
    finish();

    Pet* pet = m_caster->CreateTamedPetFrom(creatureTarget,m_spellInfo->Id);
    if(!pet) return;

    // kill original creature
    creatureTarget->setDeathState(JUST_DIED);
    creatureTarget->RemoveCorpse();
    creatureTarget->SetHealth(0);                       // just for nice GM-mode view

    // prepare visual effect for levelup
    pet->SetUInt32Value(UNIT_FIELD_LEVEL,creatureTarget->getLevel()-1);

    // add to world
    pet->GetMap()->Add(pet->ToCreature());

    // visual effect for levelup
    pet->SetUInt32Value(UNIT_FIELD_LEVEL,creatureTarget->getLevel());

    // caster have pet now
    m_caster->SetPet(pet);

    if(m_caster->GetTypeId() == TYPEID_PLAYER)
    {
        pet->SavePetToDB(PET_SAVE_AS_CURRENT);
        (m_caster->ToPlayer())->PetSpellInitialize();
    }
}

void Spell::EffectSummonPet(uint32 i)
{
    Unit *owner = NULL;
    if(m_originalCaster)
    {
        if (m_originalCaster->GetTypeId() == TYPEID_PLAYER)
            owner = m_originalCaster->ToPlayer();
        else if ((m_originalCaster->ToCreature())->isTotem())
            owner = m_originalCaster->GetCharmerOrOwnerPlayerOrPlayerItself();
        else if (m_originalCaster->ToCreature())
            owner = m_originalCaster;
    }

    if(!owner)
    {
        EffectSummonWild(i);
        return;
    }

    uint32 petentry = m_spellInfo->EffectMiscValue[i];

    Pet *OldSummon = owner->GetPet();

    // if pet requested type already exist
    if( OldSummon )
    {
        if(petentry == 0 || OldSummon->GetEntry() == petentry)
        {
            // pet in corpse state can't be summoned
            if( OldSummon->isDead() )
                return;

            OldSummon->GetMap()->Remove(OldSummon->ToCreature(),false);
            OldSummon->SetMapId(owner->GetMapId());

            float px, py, pz;
            owner->GetClosePoint(px, py, pz, OldSummon->GetObjectSize());

            OldSummon->Relocate(px, py, pz, OldSummon->GetOrientation());
            owner->GetMap()->Add(OldSummon->ToCreature());
            if(m_spellInfo->Id == 688 /*imp*/ || m_spellInfo->Id == 697 /*void walker*/ || m_spellInfo->Id == 691 /*felhunter*/ || m_spellInfo->Id == 712 /*succubus*/)
                OldSummon->SetHealth(OldSummon->GetMaxHealth());

            if(owner->GetTypeId() == TYPEID_PLAYER && OldSummon->isControlled() )
            {
                (owner->ToPlayer())->PetSpellInitialize();
            }
            return;
        }

        if(owner->GetTypeId() == TYPEID_PLAYER)
            (owner->ToPlayer())->RemovePet(OldSummon,(OldSummon->getPetType()==HUNTER_PET ? PET_SAVE_AS_DELETED : PET_SAVE_NOT_IN_SLOT),false);
        else
            return;
    }

    float x, y, z;
    owner->GetClosePoint(x, y, z, owner->GetObjectSize());
    Pet* pet = NULL;
    if (owner->GetTypeId() == TYPEID_PLAYER)
        pet = owner->ToPlayer()->SummonPet(petentry, x, y, z, owner->GetOrientation(), SUMMON_PET, 0);
    else
        pet = owner->SummonPet(petentry, x, y, z, owner->GetOrientation(), 0); // FIXME: Check if duration should always be 0 for creatures
        
    if(!pet)
        return;

    if(m_caster->GetTypeId() == TYPEID_UNIT)
    {
        if ( (m_caster->ToCreature())->isTotem() || owner->GetTypeId() == TYPEID_UNIT)
            pet->SetReactState(REACT_AGGRESSIVE);
        else
            pet->SetReactState(REACT_DEFENSIVE);
    }
    
    //Set health of warlock pets full when summoned
    if(m_caster->GetTypeId() == TYPEID_PLAYER)
    {
        if(m_spellInfo->Id == 688 /*imp*/ || m_spellInfo->Id == 697 /*void walker*/ || m_spellInfo->Id == 691 /*felhunter*/ || m_spellInfo->Id == 712 /*succubus*/)
            pet->SetHealth(pet->GetMaxHealth());
    }

    pet->SetUInt32Value(UNIT_CREATED_BY_SPELL, m_spellInfo->Id);

    // this enables popup window (pet dismiss, cancel), hunter pet additional flags set later
    pet->SetUInt32Value(UNIT_FIELD_FLAGS,UNIT_FLAG_PVP_ATTACKABLE);
    pet->SetUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP, time(NULL));

    // generate new name for summon pet
    std::string new_name=objmgr.GeneratePetName(petentry);
    if(!new_name.empty())
        pet->SetName(new_name);
}

void Spell::EffectLearnPetSpell(uint32 i)
{
    if(m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    Player *_player = m_caster->ToPlayer();

    Pet *pet = _player->GetPet();
    if(!pet)
        return;
    if(!pet->isAlive())
        return;

    SpellEntry const *learn_spellproto = spellmgr.LookupSpell(m_spellInfo->EffectTriggerSpell[i]);
    if(!learn_spellproto)
        return;

    pet->SetTP(pet->m_TrainingPoints - pet->GetTPForSpell(learn_spellproto->Id));
    pet->learnSpell(learn_spellproto->Id);

    pet->SavePetToDB(PET_SAVE_AS_CURRENT);
    _player->PetSpellInitialize();
}

void Spell::EffectTaunt(uint32 /*i*/)
{
    // this effect use before aura Taunt apply for prevent taunt already attacking target
    // for spell as marked "non effective at already attacking target"
    if(!unitTarget || !unitTarget->CanHaveThreatList()
        || unitTarget->getVictim() == m_caster)
    {
        SendCastResult(SPELL_FAILED_DONT_REPORT);
        return;
    }

    // Also use this effect to set the taunter's threat to the taunted creature's highest value
    if(unitTarget->getThreatManager().getCurrentVictim())
    {
        float myThreat = unitTarget->getThreatManager().getThreat(m_caster);
        float itsThreat = unitTarget->getThreatManager().getCurrentVictim()->getThreat();
        if(itsThreat > myThreat)
            unitTarget->getThreatManager().addThreat(m_caster, itsThreat - myThreat);
    }

    //Set aggro victim to caster
    if( !unitTarget->getThreatManager().getOnlineContainer().empty() )
        if(HostilReference* forcedVictim = unitTarget->getThreatManager().getOnlineContainer().getReferenceByTarget(m_caster))
            unitTarget->getThreatManager().setCurrentVictim(forcedVictim);

    if((unitTarget->ToCreature())->IsAIEnabled) {
        (unitTarget->ToCreature())->AI()->AttackStart(m_caster);
        if (unitTarget->ToCreature()->getAI())
            unitTarget->ToCreature()->getAI()->attackStart(m_caster);
    }
}

void Spell::EffectWeaponDmg(uint32 i)
{
}

void Spell::SpellDamageWeaponDmg(uint32 i)
{
    if(!unitTarget)
        return;
    if(!unitTarget->isAlive())
        return;

    // multiple weapon dmg effect workaround
    // execute only the last weapon damage
    // and handle all effects at once
    for (int j = 0; j < 3; j++)
    {
        switch(m_spellInfo->Effect[j])
        {
            case SPELL_EFFECT_WEAPON_DAMAGE:
            case SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL:
            case SPELL_EFFECT_NORMALIZED_WEAPON_DMG:
            case SPELL_EFFECT_WEAPON_PERCENT_DAMAGE:
                if (j < i)                                  // we must calculate only at last weapon effect
                    return;
            break;
        }
    }

    // some spell specific modifiers
    //float weaponDamagePercentMod = 1.0f;                    // applied to weapon damage (and to fixed effect damage bonus if customBonusDamagePercentMod not set
    float totalDamagePercentMod  = 1.0f;                    // applied to final bonus+weapon damage
    int32 fixed_bonus = 0;
    int32 spell_bonus = 0;                                  // bonus specific for spell

    switch(m_spellInfo->SpellFamilyName)
    {
        case SPELLFAMILY_WARRIOR:
        {
            // Devastate bonus and sunder armor refresh
            if(m_spellInfo->SpellVisual == 671 && m_spellInfo->SpellIconID == 1508)
            {
                uint32 stack = 0;

                Unit::AuraList const& list = unitTarget->GetAurasByType(SPELL_AURA_MOD_RESISTANCE);
                for(Unit::AuraList::const_iterator itr=list.begin();itr!=list.end();++itr)
                {
                    SpellEntry const *proto = (*itr)->GetSpellProto();
                    if(proto->SpellFamilyName == SPELLFAMILY_WARRIOR
                        && proto->SpellFamilyFlags == SPELLFAMILYFLAG_WARRIOR_SUNDERARMOR)
                    {
                        int32 duration = GetSpellDuration(proto);
                        (*itr)->SetAuraDuration(duration);
                        (*itr)->UpdateAuraDuration();
                        stack = (*itr)->GetStackAmount();
                        break;
                    }
                }

                for(int j = 0; j < 3; j++)
                {
                    if(m_spellInfo->Effect[j] == SPELL_EFFECT_NORMALIZED_WEAPON_DMG)
                    {
                        fixed_bonus += (stack - 1) * CalculateDamage(j, unitTarget);
                        break;
                    }
                }

                if(stack < 5)
                {
                    // get highest rank of the Sunder Armor spell
                    const PlayerSpellMap& sp_list = (m_caster->ToPlayer())->GetSpellMap();
                    for (PlayerSpellMap::const_iterator itr = sp_list.begin(); itr != sp_list.end(); ++itr)
                    {
                        // only highest rank is shown in spell book, so simply check if shown in spell book
                        if(!itr->second->active || itr->second->disabled || itr->second->state == PLAYERSPELL_REMOVED)
                            continue;

                        SpellEntry const *spellInfo = spellmgr.LookupSpell(itr->first);
                        if (!spellInfo)
                            continue;

                        if (spellInfo->SpellFamilyFlags == SPELLFAMILYFLAG_WARRIOR_SUNDERARMOR
                            && spellInfo->Id != m_spellInfo->Id
                            && spellInfo->SpellFamilyName == SPELLFAMILY_WARRIOR)
                        {
                            m_caster->CastSpell(unitTarget, spellInfo, true);
                            break;
                        }
                    }
                }
            }
            break;
        }
        case SPELLFAMILY_ROGUE:
        {
            // Hemorrhage
            if(m_spellInfo->SpellFamilyFlags & 0x2000000)
            {
                if(m_caster->GetTypeId()==TYPEID_PLAYER)
                    (m_caster->ToPlayer())->AddComboPoints(unitTarget, 1);
            }
            // Mutilate (for each hand)
            else if(m_spellInfo->SpellFamilyFlags & 0x600000000LL)
            {
                Unit::AuraMap const& auras = unitTarget->GetAuras();
                for(Unit::AuraMap::const_iterator itr = auras.begin(); itr!=auras.end(); ++itr)
                {
                    if(itr->second->GetSpellProto()->Dispel == DISPEL_POISON)
                    {
                        totalDamagePercentMod *= 1.5f;          // 150% if poisoned
                        break;
                    }
                }
            }
            break;
        }
        case SPELLFAMILY_PALADIN:
        {
            // Seal of Command - receive benefit from Spell Damage and Healing
            if(m_spellInfo->SpellFamilyFlags & 0x00000002000000LL)
            {
                spell_bonus += int32(0.20f*m_caster->SpellBaseDamageBonus(GetSpellSchoolMask(m_spellInfo)));
                spell_bonus += int32(0.29f*m_caster->SpellBaseDamageBonusForVictim(GetSpellSchoolMask(m_spellInfo), unitTarget));
            }
            break;
        }
        case SPELLFAMILY_SHAMAN:
        {
            // Skyshatter Harness item set bonus
            // Stormstrike
            if(m_spellInfo->SpellFamilyFlags & 0x001000000000LL)
            {
                Unit::AuraList const& m_OverrideClassScript = m_caster->GetAurasByType(SPELL_AURA_OVERRIDE_CLASS_SCRIPTS);
                for(Unit::AuraList::const_iterator i = m_OverrideClassScript.begin(); i != m_OverrideClassScript.end(); ++i)
                {
                    // Stormstrike AP Buff
                    if ( (*i)->GetModifier()->m_miscvalue == 5634 )
                    {
                        m_caster->CastSpell(m_caster,38430,true,NULL,*i);
                        break;
                    }
                }
            }
            break;
        }
        case SPELLFAMILY_DRUID:
        {
            // Mangle (Cat): CP
            if(m_spellInfo->SpellFamilyFlags==0x0000040000000000LL)
            {
                if(m_caster->GetTypeId()==TYPEID_PLAYER)
                    (m_caster->ToPlayer())->AddComboPoints(unitTarget,1);
            }
            break;
        }
    }

    bool normalized = false;
    float weaponDamagePercentMod = 1.0;
    for (int j = 0; j < 3; ++j)
    {
        switch(m_spellInfo->Effect[j])
        {
            case SPELL_EFFECT_WEAPON_DAMAGE:
            case SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL:
                fixed_bonus += CalculateDamage(j,unitTarget);
                break;
            case SPELL_EFFECT_NORMALIZED_WEAPON_DMG:
                fixed_bonus += CalculateDamage(j,unitTarget);
                normalized = true;
                break;
            case SPELL_EFFECT_WEAPON_PERCENT_DAMAGE:
                weaponDamagePercentMod *= float(CalculateDamage(j,unitTarget)) / 100.0f;
            default:
                break;                                      // not weapon damage effect, just skip
        }
    }

    // apply to non-weapon bonus weapon total pct effect, weapon total flat effect included in weapon damage
    if(fixed_bonus || spell_bonus)
    {
        UnitMods unitMod;
        switch(m_attackType)
        {
            default:
            case BASE_ATTACK:   unitMod = UNIT_MOD_DAMAGE_MAINHAND; break;
            case OFF_ATTACK:    unitMod = UNIT_MOD_DAMAGE_OFFHAND;  break;
            case RANGED_ATTACK: unitMod = UNIT_MOD_DAMAGE_RANGED;   break;
        }

        float weapon_total_pct  = m_caster->GetModifierValue(unitMod, TOTAL_PCT);

        if(fixed_bonus)
            fixed_bonus = int32(fixed_bonus * weapon_total_pct);
        if(spell_bonus)
            spell_bonus = int32(spell_bonus * weapon_total_pct);
    }

    int32 weaponDamage = m_caster->CalculateDamage(m_attackType, normalized, m_spellInfo);

    // Sequence is important
    for (int j = 0; j < 3; ++j)
    {
        // We assume that a spell have at most one fixed_bonus
        // and at most one weaponDamagePercentMod
        switch(m_spellInfo->Effect[j])
        {
            case SPELL_EFFECT_WEAPON_DAMAGE:
            case SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL:
            case SPELL_EFFECT_NORMALIZED_WEAPON_DMG:
                weaponDamage += fixed_bonus;
                break;
            case SPELL_EFFECT_WEAPON_PERCENT_DAMAGE:
                weaponDamage = int32(weaponDamage * weaponDamagePercentMod);
            default:
                break;                                      // not weapon damage effect, just skip
        }
    }

    // only for Seal of Command
    if(spell_bonus)
        weaponDamage += spell_bonus;

    // only for Mutilate
    if(totalDamagePercentMod != 1.0f)
        weaponDamage = int32(weaponDamage * totalDamagePercentMod);

    // prevent negative damage
    uint32 eff_damage = uint32(weaponDamage > 0 ? weaponDamage : 0);

    // Add melee damage bonuses (also check for negative)
    m_caster->MeleeDamageBonus(unitTarget, &eff_damage, m_attackType, m_spellInfo);
    m_damage+= eff_damage;

    // take ammo
    if(m_attackType == RANGED_ATTACK && m_caster->GetTypeId() == TYPEID_PLAYER)
    {
        Item *pItem = (m_caster->ToPlayer())->GetWeaponForAttack( RANGED_ATTACK );

        // wands don't have ammo
        if(!pItem  || pItem->IsBroken() || pItem->GetProto()->SubClass==ITEM_SUBCLASS_WEAPON_WAND)
            return;

        if( pItem->GetProto()->InventoryType == INVTYPE_THROWN )
        {
            if(pItem->GetMaxStackCount()==1)
            {
                // decrease durability for non-stackable throw weapon
                (m_caster->ToPlayer())->DurabilityPointLossForEquipSlot(EQUIPMENT_SLOT_RANGED);
            }
            else
            {
                // decrease items amount for stackable throw weapon
                uint32 count = 1;
                (m_caster->ToPlayer())->DestroyItemCount( pItem, count, true);
            }
        }
        else if(uint32 ammo = (m_caster->ToPlayer())->GetUInt32Value(PLAYER_AMMO_ID))
            (m_caster->ToPlayer())->DestroyItemCount(ammo, 1, true);
    }
}

void Spell::EffectThreat(uint32 /*i*/)
{
    if(!unitTarget || !unitTarget->isAlive() || !m_caster->isAlive())
        return;

    if(!unitTarget->CanHaveThreatList())
        return;

    unitTarget->AddThreat(m_caster, float(damage));
}

void Spell::EffectHealMaxHealth(uint32 /*i*/)
{
    if(!unitTarget)
        return;
    if(!unitTarget->isAlive())
        return;

    uint32 addhealth = unitTarget->GetMaxHealth() - unitTarget->GetHealth();

    // Lay on Hands
    if(m_spellInfo->SpellFamilyName == SPELLFAMILY_PALADIN && m_spellInfo->SpellFamilyFlags & 0x0000000000008000)
    {
        if(!m_originalCaster)
            return;
        addhealth = addhealth > m_originalCaster->GetMaxHealth() ? m_originalCaster->GetMaxHealth() : addhealth;
        uint32 LoHamount = unitTarget->GetHealth() + m_originalCaster->GetMaxHealth();
        LoHamount = LoHamount > unitTarget->GetMaxHealth() ? unitTarget->GetMaxHealth() : LoHamount;
        unitTarget->SetHealth(LoHamount);
    }
    else
        unitTarget->SetHealth(unitTarget->GetMaxHealth());

    if(m_originalCaster)
        m_originalCaster->SendHealSpellLog(unitTarget, m_spellInfo->Id, addhealth, false);
}

void Spell::EffectInterruptCast(uint32 i)
{
    if(!unitTarget)
        return;
    if(!unitTarget->isAlive())
        return;

    // TODO: not all spells that used this effect apply cooldown at school spells
    // also exist case: apply cooldown to interrupted cast only and to all spells
    for (uint32 i = CURRENT_FIRST_NON_MELEE_SPELL; i < CURRENT_MAX_SPELL; i++)
    {
        if (unitTarget->m_currentSpells[i])
        {
            // check if we can interrupt spell
            if ( (unitTarget->m_currentSpells[i]->getState() == SPELL_STATE_CASTING || (unitTarget->m_currentSpells[i]->getState() == SPELL_STATE_PREPARING && unitTarget->m_currentSpells[i]->GetCastTime() > 0.0f)) && unitTarget->m_currentSpells[i]->m_spellInfo->InterruptFlags & SPELL_INTERRUPT_FLAG_INTERRUPT && unitTarget->m_currentSpells[i]->m_spellInfo->PreventionType == SPELL_PREVENTION_TYPE_SILENCE )
            {
                if(m_originalCaster)
                {
                    int32 duration = m_originalCaster->CalculateSpellDuration(m_spellInfo, i, unitTarget);
                    unitTarget->ProhibitSpellSchool(GetSpellSchoolMask(unitTarget->m_currentSpells[i]->m_spellInfo), duration/*GetSpellDuration(m_spellInfo)*/);
                }
                unitTarget->InterruptSpell(i,false);
            }
        }
    }
}

void Spell::EffectSummonObjectWild(uint32 i)
{
    uint32 gameobject_id = m_spellInfo->EffectMiscValue[i];

    GameObject* pGameObj = new GameObject;

    WorldObject* target = focusObject;
    if( !target )
        target = m_caster;

    float x,y,z;
    if(m_targets.m_targetMask & TARGET_FLAG_DEST_LOCATION)
    {
        x = m_targets.m_destX;
        y = m_targets.m_destY;
        z = m_targets.m_destZ;
    }
    else
        m_caster->GetClosePoint(x,y,z,DEFAULT_WORLD_OBJECT_SIZE);

    Map *map = target->GetMap();

    if(!pGameObj->Create(objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT,true), gameobject_id, map,
        x, y, z, target->GetOrientation(), 0, 0, 0, 0, 100, 1))
    {
        delete pGameObj;
        return;
    }

    int32 duration = GetSpellDuration(m_spellInfo);
    pGameObj->SetRespawnTime(duration > 0 ? duration/1000 : 0);
    pGameObj->SetSpellId(m_spellInfo->Id);

    if(pGameObj->GetGoType() != GAMEOBJECT_TYPE_FLAGDROP)   // make dropped flag clickable for other players (not set owner guid (created by) for this)...
        m_caster->AddGameObject(pGameObj);
    map->Add(pGameObj);

    if(pGameObj->GetMapId() == 489 && pGameObj->GetGoType() == GAMEOBJECT_TYPE_FLAGDROP)  //WS
    {
        if(m_caster->GetTypeId() == TYPEID_PLAYER)
        {
            Player *pl = m_caster->ToPlayer();
            BattleGround* bg = (m_caster->ToPlayer())->GetBattleGround();
            if(bg && bg->GetTypeID()==BATTLEGROUND_WS && bg->GetStatus() == STATUS_IN_PROGRESS)
            {
                 uint32 team = ALLIANCE;

                 if(pl->GetTeam() == team)
                     team = HORDE;

                ((BattleGroundWS*)bg)->SetDroppedFlagGUID(pGameObj->GetGUID(),team);
            }
        }
    }

    if(pGameObj->GetMapId() == 566 && pGameObj->GetGoType() == GAMEOBJECT_TYPE_FLAGDROP)  //EY
    {
        if(m_caster->GetTypeId() == TYPEID_PLAYER)
        {
            BattleGround* bg = (m_caster->ToPlayer())->GetBattleGround();
            if(bg && bg->GetTypeID()==BATTLEGROUND_EY && bg->GetStatus() == STATUS_IN_PROGRESS)
            {
                ((BattleGroundEY*)bg)->SetDroppedFlagGUID(pGameObj->GetGUID());
            }
        }
    }

    if(uint32 linkedEntry = pGameObj->GetLinkedGameObjectEntry())
    {
        GameObject* linkedGO = new GameObject;
        if(linkedGO->Create(objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT,true), linkedEntry, map,
            x, y, z, target->GetOrientation(), 0, 0, 0, 0, 100, 1))
        {
            linkedGO->SetRespawnTime(duration > 0 ? duration/1000 : 0);
            linkedGO->SetSpellId(m_spellInfo->Id);

            m_caster->AddGameObject(linkedGO);
            map->Add(linkedGO);
        }
        else
        {
            delete linkedGO;
            linkedGO = NULL;
            return;
        }
    }
    
    if (m_spellInfo->Id == 19588)
        m_caster->SummonCreature(11560, m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), m_caster->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 60000);
}

void Spell::EffectScriptEffect(uint32 effIndex)
{
    // TODO: we must implement hunter pet summon at login there (spell 6962)

    // by spell id
    switch(m_spellInfo->Id)
    {
        // Power of the Blue Flight
        case 45833:
            if (!unitTarget)
                return;

            unitTarget->CastSpell(unitTarget, 45836, true, NULL, NULL, unitTarget->GetGUID());
            return;
        // Sinister Reflection
        case 45892:
        {
        	Unit *target = NULL;
        	if (!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
        	{
        		if (m_caster->ToCreature())
        		{
        			if (m_caster->ToCreature()->getAI())
        			{
        				if (Unit *unit = m_caster->ToCreature()->getAI()->selectUnit(SELECT_TARGET_RANDOM, 0, 100.0f, true))
        					target = unit;
        			}
        		}
        	}
        	else
        		target = unitTarget;

        	if (target)
        	{
                // Summon 4 clones of the same player
                for (uint8 i = 0; i < 4; ++i)
            	    target->CastSpell(target, 45891, true, NULL, NULL, target->GetGUID());

                target->CastSpell(target, 45785, true);
        	}
            return;
        }
        // Copy Weapon
        case 45785:
        {
        	if (!unitTarget)
        		return;

            if (unitTarget->GetEntry() != 25708)
                return;

            Item* mainItem = NULL;
            Item* offItem = NULL;
            Item* rangedItem = NULL;
            if (mainItem = m_caster->ToPlayer()->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND))
                unitTarget->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY + 0, mainItem->GetProto()->DisplayInfoID);

            if (offItem = m_caster->ToPlayer()->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND))
                unitTarget->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY + 1, offItem->GetProto()->DisplayInfoID);

            if (rangedItem = m_caster->ToPlayer()->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED))
                unitTarget->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY + 2, rangedItem->GetProto()->DisplayInfoID);

            break;
        }
        // Chilling burst
        case 46541:
            unitTarget->AddAura(46458, unitTarget);
            break;
		// Plant Kil'sorrow Banner (quest 9931)
		case 32314:
			if (Player *plr = m_caster->ToPlayer())
				plr->KilledMonster(18393, 0);
			break;
        // Unban Azaloth (quest 10637 && 10688)
        case 37834:
            if (m_caster->GetTypeId() == TYPEID_PLAYER)
            {
                if ((m_caster->ToPlayer())->GetQuestStatus(10637) == QUEST_STATUS_INCOMPLETE)
                    (m_caster->ToPlayer())->CompleteQuest(10637);
                else if ((m_caster->ToPlayer())->GetQuestStatus(10688) == QUEST_STATUS_INCOMPLETE)
                    (m_caster->ToPlayer())->KilledMonster(21892, 0);
            }
            break;
        // Grillok's Eye Quest Credit
        case 38530:
            if(unitTarget->GetTypeId() == TYPEID_PLAYER)
            {
                (unitTarget->ToPlayer())->CompleteQuest(10813);
                unitTarget->RemoveAurasDueToSpell(38495);
            }
            break;
        // PX-238 Winter Wondervolt TRAP
        case 26275:
        {
            if( unitTarget->HasAura(26272,0)
             || unitTarget->HasAura(26157,0)
             || unitTarget->HasAura(26273,0)
             || unitTarget->HasAura(26274,0))
                return;

            uint32 iTmpSpellId;

            switch(m_caster->GetMap()->urand(0,3))
            {
                case 0:
                    iTmpSpellId = 26272;
                    break;
                case 1:
                    iTmpSpellId = 26157;
                    break;
                case 2:
                    iTmpSpellId = 26273;
                    break;
                case 3:
                    iTmpSpellId = 26274;
                    break;
            }

            unitTarget->CastSpell(unitTarget, iTmpSpellId, true);

            return;
        }

        // Bending Shinbone
        case 8856:
        {
            if(!itemTarget && m_caster->GetTypeId()!=TYPEID_PLAYER)
                return;

            uint32 spell_id = 0;
            switch(m_caster->GetMap()->urand(1,5))
            {
                case 1:  spell_id = 8854; break;
                default: spell_id = 8855; break;
            }

            m_caster->CastSpell(m_caster,spell_id,true,NULL);
            return;
        }

        // Healthstone creating spells
        case  6201:
        case  6202:
        case  5699:
        case 11729:
        case 11730:
        case 27230:
        {
            uint32 itemtype;
            uint32 rank = 0;
            Unit::AuraList const& mDummyAuras = unitTarget->GetAurasByType(SPELL_AURA_DUMMY);
            for(Unit::AuraList::const_iterator i = mDummyAuras.begin();i != mDummyAuras.end(); ++i)
            {
                if((*i)->GetId() == 18692)
                {
                    rank = 1;
                    break;
                }
                else if((*i)->GetId() == 18693)
                {
                    rank = 2;
                    break;
                }
            }

            static uint32 const itypes[6][3] = {
                { 5512,19004,19005},                        // Minor Healthstone
                { 5511,19006,19007},                        // Lesser Healthstone
                { 5509,19008,19009},                        // Healthstone
                { 5510,19010,19011},                        // Greater Healthstone
                { 9421,19012,19013},                        // Major Healthstone
                {22103,22104,22105}                         // Master Healthstone
            };

            switch(m_spellInfo->Id)
            {
                case  6201: itemtype=itypes[0][rank];break; // Minor Healthstone
                case  6202: itemtype=itypes[1][rank];break; // Lesser Healthstone
                case  5699: itemtype=itypes[2][rank];break; // Healthstone
                case 11729: itemtype=itypes[3][rank];break; // Greater Healthstone
                case 11730: itemtype=itypes[4][rank];break; // Major Healthstone
                case 27230: itemtype=itypes[5][rank];break; // Master Healthstone
                default:
                    return;
            }
            
            DoCreateItem( effIndex, itemtype );
            return;
        }
        // Brittle Armor - need remove one 24575 Brittle Armor aura
        case 24590:
            unitTarget->RemoveSingleAuraFromStack(24575, 0);
            unitTarget->RemoveSingleAuraFromStack(24575, 1);
            return;
        // Mercurial Shield - need remove one 26464 Mercurial Shield aura
        case 26465:
            unitTarget->RemoveSingleAuraFromStack(26464, 0);
            return;
        // Orb teleport spells
        case 25140:
        case 25143:
        case 25650:
        case 25652:
        case 29128:
        case 29129:
        case 35376:
        case 35727:
        {
            if(!unitTarget)
                return;

            uint32 spellid;
            switch(m_spellInfo->Id)
            {
                case 25140: spellid =  32571; break;
                case 25143: spellid =  32572; break;
                case 25650: spellid =  30140; break;
                case 25652: spellid =  30141; break;
                case 29128: spellid =  32568; break;
                case 29129: spellid =  32569; break;
                case 35376: spellid =  25649; break;
                case 35727: spellid =  35730; break;
                default:
                    return;
            }

            unitTarget->CastSpell(unitTarget,spellid,false);
            return;
        }

        // Shadow Flame (All script effects, not just end ones to prevent player from dodging the last triggered spell)
        case 22539:
        case 22972:
        case 22975:
        case 22976:
        case 22977:
        case 22978:
        case 22979:
        case 22980:
        case 22981:
        case 22982:
        case 22983:
        case 22984:
        case 22985:
        {
            if(!unitTarget || !unitTarget->isAlive())
                return;

            // Onyxia Scale Cloak
            if(unitTarget->GetDummyAura(22683))
                return;

            // Shadow Flame
            m_caster->CastSpell(unitTarget, 22682, true);
            return;
        }
        break;
        // Hallowed Wand
        // Random Costume
        case 24720:
        {
            if(!unitTarget || !unitTarget->isAlive())
                return;

            uint32 spellId;
            switch((uint32)m_caster->GetMap()->rand32()%7)
            {
            case 0: spellId = 24717; break; // Pirate Costume
            case 1: spellId = 24741; break; // Wisp Costume
            case 2: spellId = 24724; break; // Skeleton Costume
            case 3: spellId = 24719; break; // Leper Gnome Costume
            case 4: spellId = 24718; break; // Ninja Costume
            case 5: spellId = 24737; break; // Ghost Costume
            case 6: spellId = 24733; break; // Bat Costume
            }
            m_caster->CastSpell(unitTarget, spellId, true);
        }
        break;
        // Pirate Costume
        case 24717:
        {
            if(!unitTarget || !unitTarget->isAlive())
                return;

            if(unitTarget->getGender() == GENDER_MALE)
                m_caster->CastSpell(unitTarget,24708,true);
            else
                m_caster->CastSpell(unitTarget,24709,true);
        }
        break;
        // Ninja Costume
        case 24718:
        {
            if(!unitTarget || !unitTarget->isAlive())
                return;

            if(unitTarget->getGender() == GENDER_MALE)
                m_caster->CastSpell(unitTarget,24711,true);
            else
                m_caster->CastSpell(unitTarget,24710,true);
        }
        break;
        // Leper Gnome Costume
        case 24719:
        {
            if(!unitTarget || !unitTarget->isAlive())
                return;

            if(unitTarget->getGender() == GENDER_MALE)
                m_caster->CastSpell(unitTarget,24712,true);
            else
                m_caster->CastSpell(unitTarget,24713,true);
        }
        break;
        // Ghost Costume
        case 24737:
        {
            if(!unitTarget || !unitTarget->isAlive())
                return;

            if(unitTarget->getGender() == GENDER_MALE)
                m_caster->CastSpell(unitTarget,24735,true);
            else
                m_caster->CastSpell(unitTarget,24736,true);
        }
        break;
        // Summon Black Qiraji Battle Tank
        case 26656:
        {
            if(!unitTarget)
                return;
                
            if(unitTarget->isInCombat())
            {
                SendCastResult(SPELL_FAILED_TARGET_IN_COMBAT);
                return;
            }

            // Prevent stacking of mounts
            unitTarget->RemoveSpellsCausingAura(SPELL_AURA_MOUNTED);

            // Two separate mounts depending on area id (allows use both in and out of specific instance)
            if (unitTarget->GetAreaId() == 3428)
                unitTarget->CastSpell(unitTarget, 25863, false);
            else
                unitTarget->CastSpell(unitTarget, 26655, false);
            break;
        }
        // Piccolo of the Flaming Fire
        case 17512:
        {
            if(!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
                return;
            unitTarget->HandleEmoteCommand(EMOTE_STATE_DANCE);
            break;
        }
        // Netherbloom
        case 28702:
        {
            if(!unitTarget)
                return;
            // 25% chance of casting a random buff
            if(roll_chance_i(75))
                return;

            // triggered spells are 28703 to 28707
            // Note: some sources say, that there was the possibility of
            //       receiving a debuff. However, this seems to be removed by a patch.
            const uint32 spellid = 28703;

            // don't overwrite an existing aura
            for(uint8 i=0; i<5; i++)
                if(unitTarget->HasAura(spellid+i, 0))
                    return;
            unitTarget->CastSpell(unitTarget, spellid+m_caster->GetMap()->urand(0, 4), true);
            break;
        }

        // Nightmare Vine
        case 28720:
        {
            if(!unitTarget)
                return;
            // 25% chance of casting Nightmare Pollen
            if(roll_chance_i(75))
                return;
            unitTarget->CastSpell(unitTarget, 28721, true);
            break;
        }

        // Mirren's Drinking Hat
        case 29830:
        {
            uint32 item = 0;
            switch ( m_caster->GetMap()->urand(1,6) )
            {
                case 1: case 2: case 3: item = 23584; break;// Loch Modan Lager
                case 4: case 5:         item = 23585; break;// Stouthammer Lite
                case 6:                 item = 23586; break;// Aerie Peak Pale Ale
            }
            if (item)
                DoCreateItem(effIndex,item);
            break;
        }
        // Improved Sprint
        case 30918:
        {
            // Removes snares and roots.
            uint32 mechanic_mask = (1<<MECHANIC_ROOT) | (1<<MECHANIC_SNARE);
            Unit::AuraMap& Auras = unitTarget->GetAuras();
            for(Unit::AuraMap::iterator iter = Auras.begin(), next; iter != Auras.end(); iter = next)
            {
                next = iter;
                ++next;
                Aura *aur = iter->second;
                if (!aur->IsPositive())             //only remove negative spells
                {
                    // check for mechanic mask
                    if(GetSpellMechanicMask(aur->GetSpellProto(), aur->GetEffIndex()) & mechanic_mask)
                    {
                        unitTarget->RemoveAurasDueToSpell(aur->GetId());
                        if(Auras.empty())
                            break;
                        else
                            next = Auras.begin();
                    }
                }
            }
            break;
        }

        // Plant Warmaul Ogre Banner
        case 32307:
        {
            Player *p_caster = dynamic_cast<Player*>(m_caster);
            if (!p_caster)
                break;
            Creature *cTarget = dynamic_cast<Creature*>(unitTarget);
            if (!cTarget)
                break;
            p_caster->KilledMonster(18388, cTarget->GetGUID());
            cTarget->setDeathState(CORPSE);
            cTarget->RemoveCorpse();
            break;
        }

        // Goblin Weather Machine
        case 46203:
        {
            if(!unitTarget)
                return;

            uint32 spellId;
            switch((uint32)m_caster->GetMap()->rand32()%4)
            {
                case 0:
                    spellId=46740;
                    break;
                case 1:
                    spellId=46739;
                    break;
                case 2:
                    spellId=46738;
                    break;
                case 3:
                    spellId=46736;
                    break;
            }
            unitTarget->CastSpell(unitTarget, spellId, true);
            break;
        }
        case 48025:                                     // Headless Horseman's Mount
        {
                if(!unitTarget)
                    return;

                if(unitTarget)
                {
                    switch((unitTarget->ToPlayer())->GetBaseSkillValue(762))
                    {
                    case 75: unitTarget->CastSpell(unitTarget, 51621, true); break;;
                    case 150: unitTarget->CastSpell(unitTarget, 48024, true); break;
                    case 225: 
                        if (GetVirtualMapForMapAndZone(m_caster->GetMapId(),m_caster->GetZoneId()) == 530)
                            unitTarget->CastSpell(unitTarget, 51617, true);
                        else
                            unitTarget->CastSpell(unitTarget, 48024, true);
                        break;
                    case 300: 
                        if (GetVirtualMapForMapAndZone(m_caster->GetMapId(),m_caster->GetZoneId()) == 530)
                            unitTarget->CastSpell(unitTarget, 48023, true);
                        else
                            unitTarget->CastSpell(unitTarget, 48024, true);
                        break;
                    default: break;
                    }
                }
                break;
        }
        case 47977:                                     // Magic Broom
        {
            if(!unitTarget)
                return;

            if(unitTarget)
            {
                switch((unitTarget->ToPlayer())->GetBaseSkillValue(762))
                {
                case 75: unitTarget->CastSpell(unitTarget, 42680, true); break;;
                case 150: case 225: case 300: unitTarget->CastSpell(unitTarget, 42683, true); break;
                default: break;
                }
            }
            break;
        }
        // Fog of corruption
        case 45714:
        {
            // Find Felmyst
            CellPair pair(Trinity::ComputeCellPair(unitTarget->GetPositionX(), unitTarget->GetPositionY()));
            Cell cell(pair);
            cell.data.Part.reserved = ALL_DISTRICT;
            cell.SetNoCreate();
            std::list<Creature*> Felmyst;

            Trinity::AllCreaturesOfEntryInRange check(unitTarget, 25038, 50.0f);
            Trinity::CreatureListSearcher<Trinity::AllCreaturesOfEntryInRange> searcher(Felmyst, check);
            TypeContainerVisitor<Trinity::CreatureListSearcher<Trinity::AllCreaturesOfEntryInRange>, GridTypeMapContainer> visitor(searcher);

            cell.Visit(pair, visitor, *m_caster->GetMap(), *m_caster, 50.0f);
            // Let him MC this target
            if(!Felmyst.empty())
                (*Felmyst.begin())->CastSpell(unitTarget, damage, true );
            return;
        }
        // Burn 
        case 45151:
        {
            // Already Burning
            if (unitTarget->HasAura(46394, 0))
                return;

            if (unitTarget->isPet())
                return;

            if (m_originalCaster)
                unitTarget->CastSpell(unitTarget, 46394 , true, 0, 0, m_originalCasterGUID);
            return;
        }
        // Quest 11026 Kill Credit
        case 40828:
        {
            if (Unit *summoner = ((TemporarySummon*)m_caster)->GetSummoner()) {
                if (summoner->ToPlayer())
                    summoner->ToPlayer()->KilledMonster(23327, m_caster->GetGUID());
            }
            return;
        }
        case 39649:
        {
            for (uint8 i = 0; i < 8; i++)
                m_caster->CastSpell(unitTarget, 41159, true);
                
            return;
        }
        case 41072:
        {
            m_caster->CastSpell(unitTarget, 41065, true);
            return;
        }
        case 32580:
        {
            if (Creature* bunny = m_caster->FindCreatureInGrid(21352, 20.0f, true)) {
                switch (m_caster->GetAreaId()) {
                case 3776:
                {
                    if (Creature* spirit = bunny->SummonCreature(21452, bunny->GetPositionX(), bunny->GetPositionY(), bunny->GetPositionZ(), bunny->GetOrientation(), TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 120000))
                        spirit->AI()->AttackStart(m_caster);
                    break;
                }
                case 3773:
                {
                    if (Creature* spirit = bunny->SummonCreature(21446, bunny->GetPositionX(), bunny->GetPositionY(), bunny->GetPositionZ(), bunny->GetOrientation(), TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 120000))
                        spirit->AI()->AttackStart(m_caster);
                    break;
                }
                }
            }
            
            return;
        }
        case 43723:
        {
            m_caster->CastSpell(m_caster, 43753, false);
            return;
        }
        case 26541:
        case 26532:
        case 26528:
        case 26469:
        {
            if (!m_caster->ToPlayer())
                return;
            
            uint32 petEntry = 0;
            uint32 triggeredSpellId = 0;
            switch (m_spellInfo->Id) {
            case 26541:
                petEntry = 15705;
                triggeredSpellId = 26536;
                break;
            case 26532:
                petEntry = 15698;
                triggeredSpellId = 26533;
                break;
            case 26528:
                petEntry = 15706;
                triggeredSpellId = 26529;
                break;
            case 26469:
                petEntry = 15710;
                triggeredSpellId = 26045;
                break;                
            }
                
            Pet* old_critter = m_caster->ToPlayer()->GetMiniPet();
            // for same pet just despawn
            if (old_critter && old_critter->GetEntry() == petEntry) {
                m_caster->ToPlayer()->RemoveMiniPet();
                return;
            }

            // check reagent
            if (!m_caster->ToPlayer()->HasItemCount(17202, 1, false))
                return;

            // despawn old pet before summon new
            if(old_critter)
                m_caster->ToPlayer()->RemoveMiniPet();
            
            m_caster->ToPlayer()->CastSpell(m_caster, triggeredSpellId, false);

            return;
        }
        case 26393:
        {
            if (unitTarget)
                unitTarget->CastSpell(unitTarget, 26394, true);
                
            return;
        }
        case 349:
        {
            if (m_caster->ToPlayer()->GetBattleGround())
            {
                m_caster->ToPlayer()->GetSession()->SendAreaTriggerMessage("Vous avez �t� exclu du champ de bataille pour inactivit�.");
                m_caster->ToPlayer()->LeaveBattleground();
            }
            return;
        }
    }

    if(!unitTarget || !unitTarget->isAlive()) // can we remove this check?
    {
        sLog.outError("Spell %u in EffectScriptEffect does not have unitTarget", m_spellInfo->Id);
        return;
    }

    switch(m_spellInfo->Id)
    {
        // Dreaming Glory
        case 28698: unitTarget->CastSpell(unitTarget, 28694, true); break;
        // Needle Spine
        //case 39835: unitTarget->CastSpell(unitTarget, 39968, true); break;
        // Draw Soul
        case 40904: unitTarget->CastSpell(m_caster, 40903, true); break;
        // Flame Crash
        //case 41126: unitTarget->CastSpell(unitTarget, 41131, true); break;
        case 45151: 
        {
            if (unitTarget->HasAura(46394, 0))
                return;

            if (unitTarget->isPet())
                return;

            unitTarget->CastSpell(unitTarget, 46394, true); break;
        }
        case 41931:
        {
            int bag=19;
            int slot=0;
            Item* item = NULL;

            while (bag < 256)
            {
                item = (m_caster->ToPlayer())->GetItemByPos(bag,slot);
                if (item && item->GetEntry() == 38587) break;
                slot++;
                if (slot == 39)
                {
                    slot = 0;
                    bag++;
                }
            }
            if (bag < 256)
            {
                if ((m_caster->ToPlayer())->GetItemByPos(bag,slot)->GetCount() == 1) (m_caster->ToPlayer())->RemoveItem(bag,slot,true);
                else (m_caster->ToPlayer())->GetItemByPos(bag,slot)->SetCount((m_caster->ToPlayer())->GetItemByPos(bag,slot)->GetCount()-1);
                // Spell 42518 (Braufest - Gratisprobe des Braufest herstellen)
                m_caster->CastSpell(m_caster,42518,true);
                return;
            }
        }
        // Force Cast - Portal Effect: Sunwell Isle
        case 44876: unitTarget->CastSpell(unitTarget, 44870, true); break;
        // spell of Brutallus - Stomp
        case 45185:
        {
            if(unitTarget->HasAura(46394, 0)) // spell of Brutallus - Burn
                unitTarget->RemoveAurasDueToSpell(46394);
            break;
        }
        // Negative Energy
        case 46289: m_caster->CastSpell(unitTarget, 46285, true); break;
        //5,000 Gold
        case 46642:
        {
            if(unitTarget->GetTypeId() == TYPEID_PLAYER)
                (unitTarget->ToPlayer())->ModifyMoney(50000000);
            break;
        }
    }

    if( m_spellInfo->SpellFamilyName == SPELLFAMILY_PALADIN )
    {
        switch(m_spellInfo->SpellFamilyFlags)
        {
            // Judgement
            case 0x800000:
            {
                uint32 spellId2 = 0;

                // all seals have aura dummy
                Unit::AuraList const& m_dummyAuras = m_caster->GetAurasByType(SPELL_AURA_DUMMY);
                for(Unit::AuraList::const_iterator itr = m_dummyAuras.begin(); itr != m_dummyAuras.end(); ++itr)
                {
                    SpellEntry const *spellInfo = (*itr)->GetSpellProto();

                    // search seal (all seals have judgement's aura dummy spell id in 2 effect
                    if ( !spellInfo || !IsSealSpell((*itr)->GetSpellProto()) || (*itr)->GetEffIndex() != 2 )
                        continue;

                    // must be calculated base at raw base points in spell proto, GetModifier()->m_value for S.Righteousness modified by SPELLMOD_DAMAGE
                    spellId2 = (*itr)->GetSpellProto()->EffectBasePoints[2]+1;

                    if(spellId2 <= 1)
                        continue;

                    // found, remove seal
                    m_caster->RemoveAurasDueToSpell((*itr)->GetId());

                    // Sanctified Judgement
                    Unit::AuraList const& m_auras = m_caster->GetAurasByType(SPELL_AURA_DUMMY);
                    for(Unit::AuraList::const_iterator i = m_auras.begin(); i != m_auras.end(); ++i)
                    {
                        if ((*i)->GetSpellProto()->SpellIconID == 205 && (*i)->GetSpellProto()->Attributes == 0x01D0LL)
                        {
                            int32 chance = (*i)->GetModifier()->m_amount;
                            if ( roll_chance_i(chance) )
                            {
                                int32 mana = spellInfo->manaCost;
                                if (!mana)
                                    mana = spellInfo->ManaCostPercentage * m_caster->GetCreateMana() / 100;
                                if ( Player* modOwner = m_caster->GetSpellModOwner() )
                                    modOwner->ApplySpellMod(spellInfo->Id, SPELLMOD_COST, mana);
                                mana = int32(mana* 0.8f);
                                m_caster->CastCustomSpell(m_caster,31930,&mana,NULL,NULL,true,NULL,*i);
                            }
                            break;
                        }
                    }

                    break;
                }

                m_caster->CastSpell(unitTarget,spellId2,true);
                return;
            }
        }
    }

    // normal DB scripted effect
    sWorld.ScriptsStart(sSpellScripts, m_spellInfo->Id, m_caster, unitTarget);
}

void Spell::EffectSanctuary(uint32 /*i*/)
{
    if(!unitTarget)
        return;

    std::list<Unit*> targets;
    Trinity::AnyUnfriendlyUnitInObjectRangeCheck u_check(unitTarget, unitTarget, m_caster->GetMap()->GetVisibilityDistance());
    Trinity::UnitListSearcher<Trinity::AnyUnfriendlyUnitInObjectRangeCheck> searcher(targets, u_check);
    unitTarget->VisitNearbyObject(m_caster->GetMap()->GetVisibilityDistance(), searcher);
    for(std::list<Unit*>::iterator iter = targets.begin(); iter != targets.end(); ++iter)
    {
        if(!(*iter)->hasUnitState(UNIT_STAT_CASTING))
            continue;

        for(uint32 i = CURRENT_FIRST_NON_MELEE_SPELL; i < CURRENT_MAX_SPELL; i++)
        {
            if((*iter)->m_currentSpells[i]
            && (*iter)->m_currentSpells[i]->m_targets.getUnitTargetGUID() == unitTarget->GetGUID())
            {
                (*iter)->InterruptSpell(i, false);
            }
        }
    }

    unitTarget->CombatStop();
    unitTarget->getHostilRefManager().deleteReferences();   // stop all fighting
    // Vanish allows to remove all threat and cast regular stealth so other spells can be used
    if(m_spellInfo->SpellFamilyName == SPELLFAMILY_ROGUE && (m_spellInfo->SpellFamilyFlags & SPELLFAMILYFLAG_ROGUE_VANISH))
    {
        m_caster->RemoveSpellsCausingAura(SPELL_AURA_MOD_ROOT);
    }
}

void Spell::EffectAddComboPoints(uint32 /*i*/)
{
    if (!unitTarget)
        return;

    if (m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    if (damage <= 0)
        return;
        
    if (m_spellInfo->Id == 15250) {
        (m_caster->ToPlayer())->AddComboPoints(unitTarget, damage, true);
        return;
    }

    (m_caster->ToPlayer())->AddComboPoints(unitTarget, damage);
}

void Spell::EffectDuel(uint32 i)
{
    if(!m_caster || !unitTarget || m_caster->GetTypeId() != TYPEID_PLAYER || unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    Player *caster = m_caster->ToPlayer();
    Player *target = unitTarget->ToPlayer();

    // caster or target already have requested duel
    if( caster->duel || target->duel || !target->GetSocial() || target->GetSocial()->HasIgnore(caster->GetGUIDLow()) )
        return;

    // Players can only fight a duel with each other outside (=not inside dungeons and not in capital cities)
    // Don't have to check the target's map since you cannot challenge someone across maps
    if(caster->GetMap()->Instanceable())
    {
        SendCastResult(SPELL_FAILED_NO_DUELING);            // Dueling isn't allowed here
        return;
    }

    AreaTableEntry const* casterAreaEntry = GetAreaEntryByAreaID(caster->GetZoneId());
    if(casterAreaEntry && (casterAreaEntry->flags & AREA_FLAG_CAPITAL) )
    {
        SendCastResult(SPELL_FAILED_NO_DUELING);            // Dueling isn't allowed here
        return;
    }

    AreaTableEntry const* targetAreaEntry = GetAreaEntryByAreaID(target->GetZoneId());
    if(targetAreaEntry && (targetAreaEntry->flags & AREA_FLAG_CAPITAL) )
    {
        SendCastResult(SPELL_FAILED_NO_DUELING);            // Dueling isn't allowed here
        return;
    }

    //CREATE DUEL FLAG OBJECT
    GameObject* pGameObj = new GameObject;

    uint32 gameobject_id = m_spellInfo->EffectMiscValue[i];

    Map *map = m_caster->GetMap();
    if(!pGameObj->Create(objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT,true), gameobject_id, map,
        m_caster->GetPositionX()+(unitTarget->GetPositionX()-m_caster->GetPositionX())/2 ,
        m_caster->GetPositionY()+(unitTarget->GetPositionY()-m_caster->GetPositionY())/2 ,
        m_caster->GetPositionZ(),
        m_caster->GetOrientation(), 0, 0, 0, 0, 0, 1))
    {
        delete pGameObj;
        return;
    }

    pGameObj->SetUInt32Value(GAMEOBJECT_FACTION, m_caster->getFaction() );
    pGameObj->SetUInt32Value(GAMEOBJECT_LEVEL, m_caster->getLevel()+1 );
    int32 duration = GetSpellDuration(m_spellInfo);
    pGameObj->SetRespawnTime(duration > 0 ? duration/1000 : 0);
    pGameObj->SetSpellId(m_spellInfo->Id);

    m_caster->AddGameObject(pGameObj);
    map->Add(pGameObj);
    //END

    // Send request
    WorldPacket data(SMSG_DUEL_REQUESTED, 16);
    data << pGameObj->GetGUID();
    data << caster->GetGUID();
    caster->GetSession()->SendPacket(&data);
    target->GetSession()->SendPacket(&data);

    // create duel-info
    DuelInfo *duel   = new DuelInfo;
    duel->initiator  = caster;
    duel->opponent   = target;
    duel->startTime  = 0;
    duel->startTimer = 0;
    caster->duel     = duel;

    DuelInfo *duel2   = new DuelInfo;
    duel2->initiator  = caster;
    duel2->opponent   = caster;
    duel2->startTime  = 0;
    duel2->startTimer = 0;
    target->duel      = duel2;

    caster->SetUInt64Value(PLAYER_DUEL_ARBITER,pGameObj->GetGUID());
    target->SetUInt64Value(PLAYER_DUEL_ARBITER,pGameObj->GetGUID());
}

void Spell::EffectStuck(uint32 /*i*/)
{
    if(!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    if(!sWorld.getConfig(CONFIG_CAST_UNSTUCK))
        return;

    Player* pTarget = unitTarget->ToPlayer();

    if(pTarget->isInFlight())
        return;

    // Stuck spell trigger Hearthstone cooldown
    SpellEntry const *spellInfo = spellmgr.LookupSpell(8690);
    if(!spellInfo)
        return;
    Spell spell(pTarget,spellInfo,true,0);
    spell.SendSpellCooldown();
    
    pTarget->Kill(pTarget, false);
    // Teleport to nearest graveyard
    pTarget->RepopAtGraveyard();
}

void Spell::EffectSummonPlayer(uint32 /*i*/)
{
    if(!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    // Evil Twin (ignore player summon, but hide this for summoner)
    if(unitTarget->GetDummyAura(23445))
        return;

    float x,y,z;
    //m_caster->GetClosePoint(x,y,z,unitTarget->GetObjectSize());
    m_caster->GetPosition(x, y, z);

    (unitTarget->ToPlayer())->SetSummonPoint(m_caster->GetMapId(),x,y,z);

    WorldPacket data(SMSG_SUMMON_REQUEST, 8+4+4);
    data << uint64(m_caster->GetGUID());                    // summoner guid
    data << uint32(m_caster->GetZoneId());                  // summoner zone
    data << uint32(MAX_PLAYER_SUMMON_DELAY*1000);           // auto decline after msecs
    (unitTarget->ToPlayer())->GetSession()->SendPacket(&data);
}

static ScriptInfo generateActivateCommand()
{
    ScriptInfo si;
    si.command = SCRIPT_COMMAND_ACTIVATE_OBJECT;
    return si;
}

void Spell::EffectActivateObject(uint32 effect_idx)
{
    if(!gameObjTarget)
        return;

    static ScriptInfo activateCommand = generateActivateCommand();

    int32 delay_secs = m_spellInfo->EffectMiscValue[effect_idx];

    sWorld.ScriptCommandStart(activateCommand, delay_secs, m_caster, gameObjTarget);
}

void Spell::EffectSummonTotem(uint32 i)
{
    uint8 slot = 0;
    switch(m_spellInfo->EffectMiscValueB[i])
    {
        case SUMMON_TYPE_TOTEM_SLOT1: slot = 0; break;
        case SUMMON_TYPE_TOTEM_SLOT2: slot = 1; break;
        case SUMMON_TYPE_TOTEM_SLOT3: slot = 2; break;
        case SUMMON_TYPE_TOTEM_SLOT4: slot = 3; break;
        // Battle standard case
        case SUMMON_TYPE_TOTEM:       slot = 254; break;
        // jewelery statue case, like totem without slot
        case SUMMON_TYPE_GUARDIAN:    slot = 255; break;
        default: return;
    }

    if(slot < MAX_TOTEM)
    {
        uint64 guid = m_caster->m_TotemSlot[slot];
        if(guid != 0)
        {
            Creature *OldTotem = ObjectAccessor::GetCreature(*m_caster, guid);
            if(OldTotem && OldTotem->isTotem())
                ((Totem*)OldTotem)->UnSummon();
        }
    }

    uint32 team = 0;
    if (m_caster->GetTypeId()==TYPEID_PLAYER)
        team = (m_caster->ToPlayer())->GetTeam();

    Totem* pTotem = new Totem;

    if(!pTotem->Create(objmgr.GenerateLowGuid(HIGHGUID_UNIT,true), m_caster->GetMap(), m_spellInfo->EffectMiscValue[i], team ))
    {
        delete pTotem;
        return;
    }

    float angle = slot < MAX_TOTEM ? M_PI/MAX_TOTEM - (slot*2*M_PI/MAX_TOTEM) : 0;

    float cx,cy,cz;
    float dx,dy,dz;
    float angle2 = unitTarget->GetOrientation();
    uint32 mapid = m_caster->GetMapId();
    unitTarget->GetPosition(cx,cy,cz);
    m_caster->GetPosition(dx,dy,dz);

    //Check use of vamps//
    bool useVmap = false;
    bool swapZone = true;
    VMAP::IVMapManager* vmgr = VMAP::VMapFactory::createOrGetVMapManager();
    if(vmgr->isHeightCalcEnabled() && vmgr->isLineOfSightCalcEnabled() )
    {
        useVmap = true;

        const int itr = int(0.0f/0.1f);
        const float _dx = 0.5f * cos(angle2+angle);
        const float _dy = 0.5f * sin(angle2+angle);
        dx = cx;
        dy = cy;

        //Going foward 0.5f until max distance(added totem size to max distance)
        for(float i=0.0f; i<4.5f; i+=0.5f)
        {
            //unitTarget->GetNearPoint2D(dx,dy,i,angle);
            dx += _dx;
            dy += _dy;
            Trinity::NormalizeMapCoord(dx);
            Trinity::NormalizeMapCoord(dy);
            dz = MapManager::Instance().GetMap(mapid, unitTarget)->GetHeight(dx, dy, cz, useVmap);
           
            //Prevent climbing and go around object maybe 2.0f is to small? use 3.0f?
            if( (dz-cz) < 5.0f && (dz-cz) > -5.0f && (unitTarget->IsWithinLOS(dx, dy, dz)))
            {
                //No climb, the z differenze between this and prev step is ok. Store this destination for future use or check.
                cx = dx;
                cy = dy;
                cz = dz;
            }
            else
            {
                //Something wrong with los or z differenze... maybe we are going from outer world inside a building or viceversa
                if(swapZone)
                {
                    //so... change use of vmap and go back 1 step backward and recheck again.
                    swapZone = false;
                    useVmap = !useVmap;
                    //i-=0.5f;
                    --i;
                    dx -= _dx;
                    dy -= _dy;
                }
                else
                {
                    //bad recheck result... so break this and use last good coord for totem spawn...
                    dz += 0.5f;
                    break;
                }
            }
        }
        //minus totem size added earlier to max distance so totem doesn't visually hide inside textures
        dx -= _dx;
        dy -= _dy;
    }
    else {
        if (m_caster->ToPlayer() && m_caster->ToPlayer()->InArena())
            m_caster->GetClosePoint(dx,dy,dz,pTotem->GetObjectSize()/10,0.1f,angle);
        else
            m_caster->GetClosePoint(dx,dy,dz,pTotem->GetObjectSize(),2.0f,angle);
    }

    if( fabs( dz - m_caster->GetPositionZ() ) > 5 )
        dz = m_caster->GetPositionZ();

    pTotem->Relocate(dx, dy, dz, m_caster->GetOrientation());

    if(slot < MAX_TOTEM)
        m_caster->m_TotemSlot[slot] = pTotem->GetGUID();
    else if (slot == 254)
        m_caster->m_TotemSlot254 = pTotem->GetGUID();

    pTotem->SetOwner(m_caster->GetGUID());
    pTotem->SetTypeBySummonSpell(m_spellInfo);              // must be after Create call where m_spells initilized

    int32 duration=GetSpellDuration(m_spellInfo);
    if(Player* modOwner = m_caster->GetSpellModOwner())
        modOwner->ApplySpellMod(m_spellInfo->Id,SPELLMOD_DURATION, duration);
    pTotem->SetDuration(duration);

    if (damage)                                             // if not spell info, DB values used
    {
        pTotem->SetMaxHealth(damage);
        pTotem->SetHealth(damage);
    }

    pTotem->SetUInt32Value(UNIT_CREATED_BY_SPELL,m_spellInfo->Id);
    pTotem->SetFlag(UNIT_FIELD_FLAGS,UNIT_FLAG_PVP_ATTACKABLE);

    pTotem->ApplySpellImmune(m_spellInfo->Id,IMMUNITY_STATE,SPELL_AURA_MOD_FEAR,true);
    pTotem->ApplySpellImmune(m_spellInfo->Id,IMMUNITY_STATE,SPELL_AURA_TRANSFORM,true);
    // Naj'entus Needle Spine
    pTotem->ApplySpellImmune(0, IMMUNITY_ID, 39968, true);
    pTotem->ApplySpellImmune(0, IMMUNITY_ID, 39835, true);
    pTotem->ApplySpellImmune(0, IMMUNITY_ID, 45661, true);
    pTotem->ApplySpellImmune(0, IMMUNITY_ID, 45662, true);
    pTotem->ApplySpellImmune(0, IMMUNITY_ID, 45665, true);

    if(slot < MAX_TOTEM && m_caster->GetTypeId() == TYPEID_PLAYER)
    {
        WorldPacket data(SMSG_TOTEM_CREATED, 1+8+4+4);
        data << uint8(slot);
        data << uint64(pTotem->GetGUID());
        data << uint32(duration);
        data << uint32(m_spellInfo->Id);
        (m_caster->ToPlayer())->SendDirectMessage(&data);
    }

    pTotem->Summon(m_caster);
}

void Spell::EffectEnchantHeldItem(uint32 i)
{
    // this is only item spell effect applied to main-hand weapon of target player (players in area)
    if(!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    Player* item_owner = unitTarget->ToPlayer();
    Item* item = item_owner->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);

    if(!item )
        return;

    // must be equipped
    if(!item ->IsEquipped())
        return;

    if (m_spellInfo->EffectMiscValue[i])
    {
        uint32 enchant_id = m_spellInfo->EffectMiscValue[i];
        int32 duration = GetSpellDuration(m_spellInfo);          //Try duration index first ..
        if(!duration)
            duration = damage;//+1;            //Base points after ..
        if(!duration)
            duration = 10;                                  //10 seconds for enchants which don't have listed duration

        SpellItemEnchantmentEntry const *pEnchant = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
        if(!pEnchant)
            return;

        // Always go to temp enchantment slot
        EnchantmentSlot slot = TEMP_ENCHANTMENT_SLOT;

        // Enchantment will not be applied if a different one already exists
        if(item->GetEnchantmentId(slot) && item->GetEnchantmentId(slot) != enchant_id)
            return;

        // Apply the temporary enchantment
        item->SetEnchantment(slot, enchant_id, duration*1000, 0);
        item_owner->ApplyEnchantment(item,slot,true);
    }
}

void Spell::EffectDisEnchant(uint32 /*i*/)
{
    if(m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    Player* p_caster = m_caster->ToPlayer();
    if(!itemTarget || !itemTarget->GetProto()->DisenchantID)
        return;

    p_caster->UpdateCraftSkill(m_spellInfo->Id);

    (m_caster->ToPlayer())->SendLoot(itemTarget->GetGUID(),LOOT_DISENCHANTING);

    // item will be removed at disenchanting end
}

void Spell::EffectInebriate(uint32 /*i*/)
{
    if(!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    Player *player = unitTarget->ToPlayer();
    uint16 currentDrunk = player->GetDrunkValue();
    uint16 drunkMod = damage * 256;
    if (currentDrunk + drunkMod > 0xFFFF)
        currentDrunk = 0xFFFF;
    else
        currentDrunk += drunkMod;
    player->SetDrunkValue(currentDrunk, m_CastItem?m_CastItem->GetEntry():0);
}

void Spell::EffectFeedPet(uint32 i)
{
    if(m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    Player *_player = m_caster->ToPlayer();

    if(!itemTarget)
        return;

    Pet *pet = _player->GetPet();
    if(!pet)
        return;

    if(!pet->isAlive())
        return;

    int32 benefit = pet->GetCurrentFoodBenefitLevel(itemTarget->GetProto()->ItemLevel);
    if(benefit <= 0)
        return;

    uint32 count = 1;
    _player->DestroyItemCount(itemTarget,count,true);
    // TODO: fix crash when a spell has two effects, both pointed at the same item target

    m_caster->CastCustomSpell(m_caster,m_spellInfo->EffectTriggerSpell[i],&benefit,NULL,NULL,true);
}

void Spell::EffectDismissPet(uint32 /*i*/)
{
    if(m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    Pet* pet = m_caster->GetPet();

    // not let dismiss dead pet
    if(!pet||!pet->isAlive())
        return;

    (m_caster->ToPlayer())->RemovePet(pet,PET_SAVE_NOT_IN_SLOT);
}

void Spell::EffectSummonObject(uint32 i)
{
    uint32 go_id = m_spellInfo->EffectMiscValue[i];

    uint8 slot = 0;
    switch(m_spellInfo->Effect[i])
    {
        case SPELL_EFFECT_SUMMON_OBJECT_SLOT1: slot = 0; break;
        case SPELL_EFFECT_SUMMON_OBJECT_SLOT2: slot = 1; break;
        case SPELL_EFFECT_SUMMON_OBJECT_SLOT3: slot = 2; break;
        case SPELL_EFFECT_SUMMON_OBJECT_SLOT4: slot = 3; break;
        default: return;
    }

    uint64 guid = m_caster->m_ObjectSlot[slot];
    if(guid != 0)
    {
        GameObject* obj = NULL;
        if( m_caster )
            obj = ObjectAccessor::GetGameObject(*m_caster, guid);

        if(obj) obj->Delete();
        m_caster->m_ObjectSlot[slot] = 0;
    }

    GameObject* pGameObj = new GameObject;

    float rot2 = sin(m_caster->GetOrientation()/2);
    float rot3 = cos(m_caster->GetOrientation()/2);

    float x,y,z;
    // If dest location if present
    if (m_targets.m_targetMask & TARGET_FLAG_DEST_LOCATION)
    {
        x = m_targets.m_destX;
        y = m_targets.m_destY;
        z = m_targets.m_destZ;
    }
    // Summon in random point all other units if location present
    else
        m_caster->GetClosePoint(x,y,z,DEFAULT_WORLD_OBJECT_SIZE);

    Map *map = m_caster->GetMap();
    if(!pGameObj->Create(objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT,true), go_id, map, x, y, z, m_caster->GetOrientation(), 0, 0, rot2, rot3, 0, 1))
    {
        delete pGameObj;
        return;
    }

    //pGameObj->SetUInt32Value(GAMEOBJECT_LEVEL,m_caster->getLevel());
    int32 duration = GetSpellDuration(m_spellInfo);
    pGameObj->SetRespawnTime(duration > 0 ? duration/1000 : 0);
    pGameObj->SetSpellId(m_spellInfo->Id);
    m_caster->AddGameObject(pGameObj);

    map->Add(pGameObj);
    WorldPacket data(SMSG_GAMEOBJECT_SPAWN_ANIM_OBSOLETE, 8);
    data << pGameObj->GetGUID();
    m_caster->SendMessageToSet(&data,true);

    m_caster->m_ObjectSlot[slot] = pGameObj->GetGUID();
}

void Spell::EffectResurrect(uint32 /*effIndex*/)
{
    if(!unitTarget)
        return;
    if(unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    if(unitTarget->isAlive())
        return;
    if(!unitTarget->IsInWorld())
        return;

    switch (m_spellInfo->Id)
    {
        // Defibrillate (Goblin Jumper Cables) have 33% chance on success
        case 8342:
            if (roll_chance_i(67))
            {
                m_caster->CastSpell(m_caster, 8338, true, m_CastItem);
                return;
            }
            break;
        // Defibrillate (Goblin Jumper Cables XL) have 50% chance on success
        case 22999:
            if (roll_chance_i(50))
            {
                m_caster->CastSpell(m_caster, 23055, true, m_CastItem);
                return;
            }
            break;
        default:
            break;
    }

    Player* pTarget = (unitTarget->ToPlayer());

    if(pTarget->isRessurectRequested())       // already have one active request
        return;

    uint32 health = pTarget->GetMaxHealth() * damage / 100;
    uint32 mana   = pTarget->GetMaxPower(POWER_MANA) * damage / 100;

    pTarget->setResurrectRequestData(m_caster->GetGUID(), m_caster->GetMapId(), m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), health, mana);
    SendResurrectRequest(pTarget);
}

void Spell::EffectAddExtraAttacks(uint32 /*i*/)
{
    if(!unitTarget || !unitTarget->isAlive())
        return;

    //if( unitTarget->m_extraAttacks )
    //    return;

    Unit *victim = unitTarget->getVictim();

    // attack prevented
    // fixme, some attacks may not target current victim, this is right now not handled
    if (!victim || !unitTarget->IsWithinMeleeRange(victim) || !unitTarget->HasInArc( 2*M_PI/3, victim ))
        return;

    // Only for proc/log informations
    unitTarget->m_extraAttacks = damage;
    // Need to send log before attack is made
    SendLogExecute();
    m_needSpellLog = false;

    unitTarget->AttackerStateUpdate(victim, BASE_ATTACK, true);
}

void Spell::EffectParry(uint32 /*i*/)
{
    if (unitTarget->GetTypeId() == TYPEID_PLAYER)
    {
        (unitTarget->ToPlayer())->SetCanParry(true);
    }
}

void Spell::EffectBlock(uint32 /*i*/)
{
    if (unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    (unitTarget->ToPlayer())->SetCanBlock(true);
}

void Spell::EffectMomentMove(uint32 i)
{
    if(unitTarget->isInFlight())
        return;

    if(!m_targets.HasDst())
        return;

    uint32 mapid = m_caster->GetMapId();
    float dist = GetSpellRadius(sSpellRadiusStore.LookupEntry(m_spellInfo->EffectRadiusIndex[i]));
    float x,y,z;
    float destx,desty,destz,ground,floor;
    float orientation = unitTarget->GetOrientation(), step;

    unitTarget->GetPosition(x,y,z);
    destx = x + dist * cos(orientation);
    desty = y + dist * sin(orientation);
    ground = unitTarget->GetMap()->GetHeight(destx,desty,MAX_HEIGHT,true);
    floor = unitTarget->GetMap()->GetHeight(destx,desty,z, true);
    destz = fabs(ground - z) <= fabs(floor - z) ? ground:floor;

    bool col = VMAP::VMapFactory::createOrGetVMapManager()->getObjectHitPos(mapid,x,y,z+1.5f,destx,desty,destz+1.5f,destx,desty,destz,-0.5f);
    if(col)    // We had a collision!
    {
        destx -= 0.6 * cos(orientation);
        desty -= 0.6 * sin(orientation);
        dist = sqrt((x-destx)*(x-destx) + (y-desty)*(y-desty));
    }
    
    // Check dynamic collision
    col = unitTarget->GetMap()->getObjectHitPos(0, x, y, z+0.5f, destx, desty, destz+0.5f, destx, desty, destz, -0.5f);

    // Collided with a gameobject
    if (col)
    {
        destx -= CONTACT_DISTANCE * cos(orientation);
        desty -= CONTACT_DISTANCE * sin(orientation);
        dist = sqrt((x - destx)*(x - destx) + (y - desty)*(y - desty));
    }

    step = dist/10.0f;
    
    int j = 0;
    for(j; j<10 ;j++)
    {
        if(fabs(z - destz) > 6)
        {
            destx -= step * cos(orientation);
            desty -= step * sin(orientation);
            ground = unitTarget->GetMap()->GetHeight(destx,desty,MAX_HEIGHT,true);
            floor = unitTarget->GetMap()->GetHeight(destx,desty,z, true);
            destz = fabs(ground - z) <= fabs(floor - z) ? ground:floor;
        }
        else
            break;
    }
    if(j == 9)
    {
        return;
    }

    if(unitTarget->GetTypeId() == TYPEID_PLAYER)
      (unitTarget->ToPlayer())->TeleportTo(mapid, destx, desty, destz/*+0.07531f*/, unitTarget->GetOrientation(), TELE_TO_NOT_LEAVE_COMBAT | TELE_TO_NOT_UNSUMMON_PET | (unitTarget==m_caster ? TELE_TO_SPELL : 0));
    else if (unitTarget->ToCreature()) {
        //unitTarget->GetMap()->CreatureRelocation(unitTarget->ToCreature(), destx, desty, destz,unitTarget->GetOrientation());
        unitTarget->ToCreature()->Relocate(destx,desty,destz+0.07531f);
        unitTarget->ToCreature()->SendMonsterMove(x, y, z, 0);
    }

}

void Spell::EffectReputation(uint32 i)
{
    if(!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    Player *_player = unitTarget->ToPlayer();

    int32  rep_change = damage;//+1;           // field store reputation change -1

    uint32 faction_id = m_spellInfo->EffectMiscValue[i];

    FactionEntry const* factionEntry = sFactionStore.LookupEntry(faction_id);

    if(!factionEntry)
        return;

    _player->ModifyFactionReputation(factionEntry,rep_change);
}

void Spell::EffectQuestComplete(uint32 i)
{
    if(m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    Player *_player = m_caster->ToPlayer();

    uint32 quest_id = m_spellInfo->EffectMiscValue[i];
    _player->AreaExploredOrEventHappens(quest_id);
}

void Spell::EffectSelfResurrect(uint32 i)
{
    if(!unitTarget || unitTarget->isAlive())
        return;
    if(unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;
    if(!unitTarget->IsInWorld())
        return;

    uint32 health = 0;
    uint32 mana = 0;

    // flat case
    if(damage < 0)
    {
        health = uint32(-damage);
        mana = m_spellInfo->EffectMiscValue[i];
    }
    // percent case
    else
    {
        health = uint32(damage/100.0f*unitTarget->GetMaxHealth());
        if(unitTarget->GetMaxPower(POWER_MANA) > 0)
            mana = uint32(damage/100.0f*unitTarget->GetMaxPower(POWER_MANA));
    }

    Player *plr = (unitTarget->ToPlayer());
    plr->ResurrectPlayer(0.0f);

    plr->SetHealth( health );
    plr->SetPower(POWER_MANA, mana );
    plr->SetPower(POWER_RAGE, 0 );
    plr->SetPower(POWER_ENERGY, plr->GetMaxPower(POWER_ENERGY) );

    plr->SpawnCorpseBones();

    plr->SaveToDB();
}

void Spell::EffectSkinning(uint32 /*i*/)
{
    if(unitTarget->GetTypeId() != TYPEID_UNIT )
        return;
    if(!m_caster || m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    Creature* creature = unitTarget->ToCreature();
    int32 targetLevel = creature->getLevel();

    uint32 skill = creature->GetCreatureInfo()->GetRequiredLootSkill();

    (m_caster->ToPlayer())->SendLoot(creature->GetGUID(),LOOT_SKINNING);
    creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE);

    int32 reqValue = targetLevel < 10 ? 0 : targetLevel < 20 ? (targetLevel-10)*10 : targetLevel*5;

    int32 skillValue = (m_caster->ToPlayer())->GetPureSkillValue(skill);

    // Double chances for elites
    (m_caster->ToPlayer())->UpdateGatherSkill(skill, skillValue, reqValue, creature->isElite() ? 2 : 1 );
}

void Spell::EffectCharge(uint32 i)
{
    if(!m_caster)
        return;
        
    if (m_caster->hasUnitState(UNIT_STAT_CHARGE|UNIT_STAT_CHARGE_MOVE))
        return;
    
    if (m_caster->ToPlayer()) {    
        if (BattleGround * bg = (m_caster->ToPlayer())->GetBattleGround()) {
            if (bg->GetStatus() == STATUS_WAIT_JOIN)
                return;
        }
    }

    Unit *target = m_targets.getUnitTarget();
    if(!target)
        return;
        
    uint32 triggeredSpellId = 0, triggeredSpellId2 = 0;
    switch (i) {
    case 0:
        if (m_spellInfo->Effect[1] == SPELL_EFFECT_TRIGGER_SPELL)
            triggeredSpellId = m_spellInfo->EffectTriggerSpell[1];
        if (m_spellInfo->Effect[2] == SPELL_EFFECT_TRIGGER_SPELL)
            triggeredSpellId = m_spellInfo->EffectTriggerSpell[2];
        break;
    case 1:
        if (m_spellInfo->Effect[0] == SPELL_EFFECT_TRIGGER_SPELL)
            triggeredSpellId = m_spellInfo->EffectTriggerSpell[0];
        if (m_spellInfo->Effect[2] == SPELL_EFFECT_TRIGGER_SPELL)
            triggeredSpellId = m_spellInfo->EffectTriggerSpell[2];
        break;
    case 2:
        if (m_spellInfo->Effect[0] == SPELL_EFFECT_TRIGGER_SPELL)
            triggeredSpellId = m_spellInfo->EffectTriggerSpell[0];
        if (m_spellInfo->Effect[1] == SPELL_EFFECT_TRIGGER_SPELL)
            triggeredSpellId = m_spellInfo->EffectTriggerSpell[1];
        break;
    }

    /*if (sWorld.getConfig(CONFIG_CHARGEMOVEGEN))
        m_caster->GetMotionMaster()->MoveCharge(target, triggeredSpellId, triggeredSpellId2);
    else {*/
        float x, y, z;
        target->GetContactPoint(m_caster, x, y, z);
        m_caster->GetMotionMaster()->MoveCharge(x, y, z);
    //}

    // not all charge effects used in negative spells
    if ( !IsPositiveSpell(m_spellInfo->Id) && m_caster->GetTypeId() == TYPEID_PLAYER)
        m_caster->Attack(target, true);
}

void Spell::EffectSummonCritter(uint32 i)
{
    if(m_caster->GetTypeId() != TYPEID_PLAYER)
        return;
    Player* player = m_caster->ToPlayer();

    uint32 pet_entry = m_spellInfo->EffectMiscValue[i];
    if(!pet_entry)
        return;

    Pet* old_critter = player->GetMiniPet();

    // for same pet just despawn
    if(old_critter && old_critter->GetEntry() == pet_entry)
    {
        player->RemoveMiniPet();
        return;
    }

    // despawn old pet before summon new
    if(old_critter)
        player->RemoveMiniPet();

    // summon new pet
    Pet* critter = new Pet(MINI_PET);

    Map *map = m_caster->GetMap();
    uint32 pet_number = objmgr.GeneratePetNumber();
    if(!critter->Create(objmgr.GenerateLowGuid(HIGHGUID_PET),
        map, pet_entry, pet_number))
    {
        sLog.outError("Spell::EffectSummonCritter, spellid %u: no such creature entry %u", m_spellInfo->Id, pet_entry);
        delete critter;
        return;
    }

    float x,y,z;
    // If dest location if present
    if (m_targets.m_targetMask & TARGET_FLAG_DEST_LOCATION)
    {
         x = m_targets.m_destX;
         y = m_targets.m_destY;
         z = m_targets.m_destZ;
     }
     // Summon if dest location not present near caster
     else
         m_caster->GetClosePoint(x,y,z,critter->GetObjectSize());

    critter->Relocate(x,y,z,m_caster->GetOrientation());

    if(!critter->IsPositionValid())
    {
        sLog.outError("ERROR: Pet (guidlow %d, entry %d) not summoned. Suggested coordinates isn't valid (X: %f Y: %f)",
            critter->GetGUIDLow(), critter->GetEntry(), critter->GetPositionX(), critter->GetPositionY());
        delete critter;
        return;
    }

    critter->SetOwnerGUID(m_caster->GetGUID());
    critter->SetCreatorGUID(m_caster->GetGUID());
    critter->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE,m_caster->getFaction());
    critter->SetUInt32Value(UNIT_CREATED_BY_SPELL, m_spellInfo->Id);

    critter->AIM_Initialize();
    critter->InitPetCreateSpells();                         // e.g. disgusting oozeling has a create spell as critter...
    critter->SetMaxHealth(1);
    critter->SetHealth(1);
    critter->SetLevel(1);

    // set timer for unsummon
    int32 duration = GetSpellDuration(m_spellInfo);
    if(duration > 0)
        critter->SetDuration(duration);

    /*std::string name = player->GetName();
    name.append(petTypeSuffix[critter->getPetType()]);*/
    critter->SetName(critter->GetNameForLocaleIdx(player->GetSession()->GetSessionDbLocaleIndex()));
    player->SetMiniPet(critter);

    map->Add(critter->ToCreature());
}

void Spell::EffectKnockBack(uint32 i)
{
    if(!unitTarget)
        return;

    // Effect only works on players
    if(unitTarget->GetTypeId()!=TYPEID_PLAYER)
        return;

    float x, y;
    if(m_targets.m_targetMask & TARGET_FLAG_DEST_LOCATION)
    {
        x = m_targets.m_destX;
        y = m_targets.m_destY;
    }
    else
    {
        x = m_caster->GetPositionX();
        y = m_caster->GetPositionY();
    }

    float dx = unitTarget->GetPositionX() - x;
    float dy = unitTarget->GetPositionY() - y;
    float vcos, vsin;
    if(dx < 0.001f && dy < 0.001f)
    {
        float angle = m_caster->GetMap()->rand_norm()*2*M_PI;
        vcos = cos(angle);
        vsin = sin(angle);
    }
    else
    {
        float dist = sqrt((dx*dx) + (dy*dy));
        vcos = dx / dist;
        vsin = dy / dist;
    }

    switch (m_spellInfo->Id)
    {
        case 37363:
        {
            if (!m_caster->IsWithinMeleeRange(unitTarget))
                return;

            break;
        }
    }

    WorldPacket data(SMSG_MOVE_KNOCK_BACK, (8+4+4+4+4+4));
    data.append(unitTarget->GetPackGUID());
    data << uint32(0);                                      // Sequence
    data << float(vcos);                                    // x direction
    data << float(vsin);                                    // y direction
    data << float(m_spellInfo->EffectMiscValue[i])/10;      // Horizontal speed
    data << float(damage/-10);                              // Z Movement speed (vertical)

    (unitTarget->ToPlayer())->GetSession()->SendPacket(&data);
}

void Spell::EffectSendTaxi(uint32 i)
{
    if(!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    TaxiPathEntry const* entry = sTaxiPathStore.LookupEntry(m_spellInfo->EffectMiscValue[i]);
    if(!entry)
        return;

    std::vector<uint32> nodes;

    nodes.resize(2);
    nodes[0] = entry->from;
    nodes[1] = entry->to;

    uint32 mountid = 0;
    switch(m_spellInfo->Id)
    {
        case 31606:       //Stormcrow Amulet
            if (m_caster->GetTypeId() == TYPEID_PLAYER)
            {
                Player* pPlayer = m_caster->ToPlayer();
                pPlayer->AreaExploredOrEventHappens(9718);
                
            }
            mountid = 17447;
            break;
        case 45071:      //Quest - Sunwell Daily - Dead Scar Bombing Run
        case 45113:      //Quest - Sunwell Daily - Ship Bombing Run
        case 45353:      //Quest - Sunwell Daily - Ship Bombing Run Return
            mountid = 22840;
            break;
        case 34905:      //Stealth Flight
            mountid = 6851;
            break;
        case 41533:      //Fly of the Netherwing
        case 41540:      //Fly of the Netherwing
            mountid = 23468;
            break;
    }

    (unitTarget->ToPlayer())->ActivateTaxiPathTo(nodes,mountid);

}

void Spell::EffectPlayerPull(uint32 i)
{
    if(!unitTarget || !m_caster)
        return;

    // Effect only works on players
    if(unitTarget->GetTypeId()!=TYPEID_PLAYER)
        return;

    // Hack, effect scripted on black hole's script
    if (m_spellInfo->Id == 46230)
        return;

    float vsin = sin(unitTarget->GetAngle(m_caster));
    float vcos = cos(unitTarget->GetAngle(m_caster));

    WorldPacket data(SMSG_MOVE_KNOCK_BACK, (8+4+4+4+4+4));
    data.append(unitTarget->GetPackGUID());
    data << uint32(0);                                      // Sequence
    data << float(vcos);                                    // x direction
    data << float(vsin);                                    // y direction
                                                            // Horizontal speed
    data << float(damage ? damage : unitTarget->GetDistance2d(m_caster));
    data << float(m_spellInfo->EffectMiscValue[i])/-10;     // Z Movement speed

    (unitTarget->ToPlayer())->GetSession()->SendPacket(&data);
}

void Spell::EffectDispelMechanic(uint32 i)
{
    if(!unitTarget)
        return;

    uint32 mechanic = m_spellInfo->EffectMiscValue[i];
    
    // Luffa (http://www.wowhead.com/spell=23595): "Removes one Bleed effect.  Will not work on bleeds cast by enemies over level 60."
    if (mechanic == MECHANIC_BLEED && m_spellInfo->Id == 23595 && m_caster && m_caster->ToPlayer() && m_caster->ToPlayer()->getLevel() > 60)
        return;

    Unit::AuraMap& Auras = unitTarget->GetAuras();
    for(Unit::AuraMap::iterator iter = Auras.begin(), next; iter != Auras.end(); iter = next)
    {
        next = iter;
        ++next;
        SpellEntry const *spell = spellmgr.LookupSpell(iter->second->GetSpellProto()->Id);
        if(spell->Mechanic == mechanic || spell->EffectMechanic[iter->second->GetEffIndex()] == mechanic)
        {
            unitTarget->RemoveAurasDueToSpell(spell->Id);
            if(Auras.empty())
                break;
            else
                next = Auras.begin();
        }
    }
    return;
}

void Spell::EffectSummonDeadPet(uint32 /*i*/)
{
    if(m_caster->GetTypeId() != TYPEID_PLAYER)
        return;
    Player *_player = m_caster->ToPlayer();
    Pet *pet = _player->GetPet();
    if(!pet)
        return;
    if(pet->isAlive())
        return;
    if(damage < 0)
        return;

    float x,y,z;
    _player->GetPosition(x, y, z);
    _player->GetMap()->CreatureRelocation(pet, x, y, z, _player->GetOrientation());

    pet->SetUInt32Value(UNIT_DYNAMIC_FLAGS, 0);
    pet->RemoveFlag (UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE);
    pet->setDeathState( ALIVE );
    pet->clearUnitState(UNIT_STAT_ALL_STATE);
    pet->SetHealth( uint32(pet->GetMaxHealth()*(float(damage)/100)));

    pet->AIM_Initialize();

    // _player->PetSpellInitialize(); -- action bar not removed at death and not required send at revive
    pet->SavePetToDB(PET_SAVE_AS_CURRENT);
}

void Spell::EffectDestroyAllTotems(uint32 /*i*/)
{
    float mana = 0;
    for(int slot = 0;  slot < MAX_TOTEM; ++slot)
    {
        if(!m_caster->m_TotemSlot[slot])
            continue;

        Creature* totem = ObjectAccessor::GetCreature(*m_caster,m_caster->m_TotemSlot[slot]);
        if(totem && totem->isTotem())
        {
            uint32 spell_id = totem->GetUInt32Value(UNIT_CREATED_BY_SPELL);
            SpellEntry const* spellInfo = spellmgr.LookupSpell(spell_id);
            if(spellInfo)
                mana += spellInfo->manaCost * damage / 100;
            ((Totem*)totem)->UnSummon();
        }
    }

    int32 gain = m_caster->ModifyPower(POWER_MANA,int32(mana));
    m_caster->SendEnergizeSpellLog(m_caster, m_spellInfo->Id, gain, POWER_MANA);
}

void Spell::EffectDurabilityDamage(uint32 i)
{
    if(!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    int32 slot = m_spellInfo->EffectMiscValue[i];

    // FIXME: some spells effects have value -1/-2
    // Possibly its mean -1 all player equipped items and -2 all items
    if(slot < 0)
    {
        (unitTarget->ToPlayer())->DurabilityPointsLossAll(damage,(slot < -1));
        return;
    }

    // invalid slot value
    if(slot >= INVENTORY_SLOT_BAG_END)
        return;

    if(Item* item = (unitTarget->ToPlayer())->GetItemByPos(INVENTORY_SLOT_BAG_0,slot))
        (unitTarget->ToPlayer())->DurabilityPointsLoss(item,damage);
}

void Spell::EffectDurabilityDamagePCT(uint32 i)
{
    if(!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    int32 slot = m_spellInfo->EffectMiscValue[i];

    // FIXME: some spells effects have value -1/-2
    // Possibly its mean -1 all player equipped items and -2 all items
    if(slot < 0)
    {
        (unitTarget->ToPlayer())->DurabilityLossAll(double(damage)/100.0f,(slot < -1));
        return;
    }

    // invalid slot value
    if(slot >= INVENTORY_SLOT_BAG_END)
        return;

    if(damage <= 0)
        return;

    if(Item* item = (unitTarget->ToPlayer())->GetItemByPos(INVENTORY_SLOT_BAG_0,slot))
        (unitTarget->ToPlayer())->DurabilityLoss(item,double(damage)/100.0f);
}

void Spell::EffectModifyThreatPercent(uint32 /*effIndex*/)
{
    if(!unitTarget)
        return;

    unitTarget->getThreatManager().modifyThreatPercent(m_caster, damage);
}

void Spell::EffectTransmitted(uint32 effIndex)
{
    uint32 name_id = m_spellInfo->EffectMiscValue[effIndex];

    GameObjectInfo const* goinfo = objmgr.GetGameObjectInfo(name_id);

    if (!goinfo)
    {
        sLog.outErrorDb("Gameobject (Entry: %u) not exist and not created at spell (ID: %u) cast",name_id, m_spellInfo->Id);
        return;
    }

    float fx,fy,fz;

    if(m_targets.m_targetMask & TARGET_FLAG_DEST_LOCATION)
    {
        fx = m_targets.m_destX;
        fy = m_targets.m_destY;
        fz = m_targets.m_destZ;
    }
    //FIXME: this can be better check for most objects but still hack
    else if(m_spellInfo->EffectRadiusIndex[effIndex] && m_spellInfo->speed==0)
    {
        float dis = GetSpellRadius(sSpellRadiusStore.LookupEntry(m_spellInfo->EffectRadiusIndex[effIndex]));
        m_caster->GetClosePoint(fx,fy,fz,DEFAULT_WORLD_OBJECT_SIZE, dis);
    }
    else
    {
        float min_dis = GetSpellMinRange(sSpellRangeStore.LookupEntry(m_spellInfo->rangeIndex));
        float max_dis = GetSpellMaxRange(sSpellRangeStore.LookupEntry(m_spellInfo->rangeIndex));
        float dis = m_caster->GetMap()->rand_norm() * (max_dis - min_dis) + min_dis;

        m_caster->GetClosePoint(fx,fy,fz,DEFAULT_WORLD_OBJECT_SIZE, dis);
    }

    Map *cMap = m_caster->GetMap();

    if(goinfo->type==GAMEOBJECT_TYPE_FISHINGNODE)
    {
        LiquidData liqData;
        uint32 area_id = m_caster->GetAreaId();
        // Hack for SSC
        if (area_id != 3607)
        {
       	    if ( !cMap->IsInWater(fx, fy, fz + 1.f/* -0.5f */, &liqData))             // Hack to prevent fishing bobber from failing to land on fishing hole
            { // but this is not proper, we really need to ignore not materialized objects
                SendCastResult(SPELL_FAILED_NOT_HERE);
                SendChannelUpdate(0);
                return;
            }

            // replace by water level in this case
            fz = liqData.level;
        }
        else
        {
            GameObject* gob = m_caster->FindNearestGameObject(184956, 20.0f);
            if (!gob)
            {
                SendCastResult(SPELL_FAILED_NOT_HERE);
                SendChannelUpdate(0);
                return;
            }
            // Hack for SSC
            fz = -20.0f;
        }
        
    }
    // if gameobject is summoning object, it should be spawned right on caster's position
    else if(goinfo->type==GAMEOBJECT_TYPE_SUMMONING_RITUAL)
    {
        m_caster->GetPosition(fx,fy,fz);
    }

    GameObject* pGameObj = new GameObject;

    if(!pGameObj->Create(objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT,true), name_id, cMap,
        fx, fy, fz, m_caster->GetOrientation(), 0, 0, 0, 0, 100, 1))
    {
        delete pGameObj;
        return;
    }

    int32 duration = GetSpellDuration(m_spellInfo);

    switch(goinfo->type)
    {
        case GAMEOBJECT_TYPE_FISHINGNODE:
        {
            m_caster->SetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT,pGameObj->GetGUID());
                                                            // Orientation3
            pGameObj->SetFloatValue(GAMEOBJECT_ROTATION + 2, 0.88431775569915771 );
                                                            // Orientation4
            pGameObj->SetFloatValue(GAMEOBJECT_ROTATION + 3, -0.4668855369091033 );
            m_caster->AddGameObject(pGameObj);              // will removed at spell cancel

            // end time of range when possible catch fish (FISHING_BOBBER_READY_TIME..GetDuration(m_spellInfo))
            // start time == fish-FISHING_BOBBER_READY_TIME (0..GetDuration(m_spellInfo)-FISHING_BOBBER_READY_TIME)
            int32 lastSec;
            switch(m_caster->GetMap()->urand(0, 3))
            {
                case 0: lastSec =  3; break;
                case 1: lastSec =  7; break;
                case 2: lastSec = 13; break;
                case 3: lastSec = 17; break;
            }

            duration = duration - lastSec*1000 + FISHING_BOBBER_READY_TIME*1000;
            break;
        }
        case GAMEOBJECT_TYPE_SUMMONING_RITUAL:
        {
            if(m_caster->GetTypeId()==TYPEID_PLAYER)
            {
                pGameObj->AddUniqueUse(m_caster->ToPlayer());
                m_caster->AddGameObject(pGameObj);          // will removed at spell cancel
            }
            break;
        }
        case GAMEOBJECT_TYPE_FISHINGHOLE:
        case GAMEOBJECT_TYPE_CHEST:
        default:
        {
            break;
        }
    }

    pGameObj->SetRespawnTime(duration > 0 ? duration/1000 : 0);

    pGameObj->SetOwnerGUID(m_caster->GetGUID() );

    //pGameObj->SetUInt32Value(GAMEOBJECT_LEVEL, m_caster->getLevel() );
    pGameObj->SetSpellId(m_spellInfo->Id);

    DEBUG_LOG("AddObject at SpellEfects.cpp EffectTransmitted\n");
    //m_caster->AddGameObject(pGameObj);
    //m_ObjToDel.push_back(pGameObj);

    cMap->Add(pGameObj);

    WorldPacket data(SMSG_GAMEOBJECT_SPAWN_ANIM_OBSOLETE, 8);
    data << uint64(pGameObj->GetGUID());
    m_caster->SendMessageToSet(&data,true);

    if(uint32 linkedEntry = pGameObj->GetLinkedGameObjectEntry())
    {
        GameObject* linkedGO = new GameObject;
        if(linkedGO->Create(objmgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT,true), linkedEntry, cMap,
            fx, fy, fz, m_caster->GetOrientation(), 0, 0, 0, 0, 100, 1))
        {
            linkedGO->SetRespawnTime(duration > 0 ? duration/1000 : 0);
            //linkedGO->SetUInt32Value(GAMEOBJECT_LEVEL, m_caster->getLevel() );
            linkedGO->SetSpellId(m_spellInfo->Id);
            linkedGO->SetOwnerGUID(m_caster->GetGUID() );

            linkedGO->GetMap()->Add(linkedGO);
        }
        else
        {
            delete linkedGO;
            linkedGO = NULL;
            return;
        }
    }
}

void Spell::EffectProspecting(uint32 /*i*/)
{
    if(m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    Player* p_caster = m_caster->ToPlayer();
    if(!itemTarget || !(itemTarget->GetProto()->BagFamily & BAG_FAMILY_MASK_MINING_SUPP))
        return;

    if(itemTarget->GetCount() < 5)
        return;

    if( sWorld.getConfig(CONFIG_SKILL_PROSPECTING))
    {
        uint32 SkillValue = p_caster->GetPureSkillValue(SKILL_JEWELCRAFTING);
        uint32 reqSkillValue = itemTarget->GetProto()->RequiredSkillRank;
        p_caster->UpdateGatherSkill(SKILL_JEWELCRAFTING, SkillValue, reqSkillValue);
    }

    (m_caster->ToPlayer())->SendLoot(itemTarget->GetGUID(), LOOT_PROSPECTING);
}

void Spell::EffectSkill(uint32 /*i*/)
{
    // TODO
}

void Spell::EffectSummonDemon(uint32 i)
{
    float px = m_targets.m_destX;
    float py = m_targets.m_destY;
    float pz = m_targets.m_destZ;

    Creature* Charmed = m_caster->SummonCreature(m_spellInfo->EffectMiscValue[i], px, py, pz, m_caster->GetOrientation(),TEMPSUMMON_TIMED_OR_DEAD_DESPAWN,3600000);
    if (!Charmed)
        return;

    // might not always work correctly, maybe the creature that dies from CoD casts the effect on itself and is therefore the caster?
    Charmed->SetLevel(m_caster->getLevel());

    // TODO: Add damage/mana/hp according to level

    if (m_spellInfo->EffectMiscValue[i] == 89)              // Inferno summon
    {
        // Enslave demon effect, without mana cost and cooldown
        m_caster->CastSpell(Charmed, 20882, true);          // FIXME: enslave does not scale with level, level 62+ minions cannot be enslaved

        // Inferno effect
        Charmed->CastSpell(Charmed, 22703, true, 0);
    }
    else if (m_spellInfo->EffectMiscValue[i] == 23369) {
        if (m_caster->getVictim()) {
            Charmed->AI()->AttackStart(m_caster->getVictim());
            if (Charmed->getAI())
                Charmed->getAI()->attackStart(m_caster->getVictim());
        }
    }
}

/* There is currently no need for this effect. We handle it in BattleGround.cpp
   If we would handle the resurrection here, the spiritguide would instantly disappear as the
   player revives, and so we wouldn't see the spirit heal visual effect on the npc.
   This is why we use a half sec delay between the visual effect and the resurrection itself */
void Spell::EffectSpiritHeal(uint32 /*i*/)
{
    /*
    if(!unitTarget || unitTarget->isAlive())
        return;
    if(unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;
    if(!unitTarget->IsInWorld())
        return;

    //m_spellInfo->EffectBasePoints[i]; == 99 (percent?)
    //(unitTarget->ToPlayer())->setResurrect(m_caster->GetGUID(), unitTarget->GetPositionX(), unitTarget->GetPositionY(), unitTarget->GetPositionZ(), unitTarget->GetMaxHealth(), unitTarget->GetMaxPower(POWER_MANA));
    (unitTarget->ToPlayer())->ResurrectPlayer(1.0f);
    (unitTarget->ToPlayer())->SpawnCorpseBones();
    */
}

// remove insignia spell effect
void Spell::EffectSkinPlayerCorpse(uint32 /*i*/)
{
    if ( (m_caster->GetTypeId() != TYPEID_PLAYER) || (unitTarget->GetTypeId() != TYPEID_PLAYER) || (unitTarget->isAlive()) )
        return;

    (unitTarget->ToPlayer())->RemovedInsignia( m_caster->ToPlayer() );
}

void Spell::EffectStealBeneficialBuff(uint32 i)
{
    if(!unitTarget || unitTarget==m_caster)                 // can't steal from self
        return;

    std::vector <Aura *> steal_list;
    // Create dispel mask by dispel type
    uint32 dispelMask  = GetDispellMask( DispelType(m_spellInfo->EffectMiscValue[i]) );
    Unit::AuraMap const& auras = unitTarget->GetAuras();
    for(Unit::AuraMap::const_iterator itr = auras.begin(); itr != auras.end(); ++itr)
    {
        Aura *aur = (*itr).second;
        if (aur && (1<<aur->GetSpellProto()->Dispel) & dispelMask)
        {
            // Need check for passive? this
            if (aur->IsPositive() && !aur->IsPassive() && !(aur->GetSpellProto()->AttributesEx4 & SPELL_ATTR_EX4_NOT_STEALABLE))
                steal_list.push_back(aur);
        }
    }
    // Ok if exist some buffs for dispel try dispel it
    if (!steal_list.empty())
    {
        std::list < std::pair<uint32,uint64> > success_list;
        int32 list_size = steal_list.size();
        // dispel N = damage buffs (or while exist buffs for dispel)
        for (int32 count=0; count < damage && list_size > 0; ++count)
        {
            // Random select buff for dispel
            Aura *aur = steal_list[m_caster->GetMap()->urand(0, list_size-1)];
            // Not use chance for steal
            // TODO possible need do it
            success_list.push_back( std::pair<uint32,uint64>(aur->GetId(),aur->GetCasterGUID()));

            // Remove buff from list for prevent doubles
            for (std::vector<Aura *>::iterator j = steal_list.begin(); j != steal_list.end(); )
            {
                Aura *stealed = *j;
                if (stealed->GetId() == aur->GetId() && stealed->GetCasterGUID() == aur->GetCasterGUID())
                {
                    j = steal_list.erase(j);
                    --list_size;
                }
                else
                    ++j;
            }
        }
        // Really try steal and send log
        if (!success_list.empty())
        {
            int32 count = success_list.size();
            WorldPacket data(SMSG_SPELLSTEALLOG, 8+8+4+1+4+count*5);
            data.append(unitTarget->GetPackGUID());  // Victim GUID
            data.append(m_caster->GetPackGUID());    // Caster GUID
            data << uint32(m_spellInfo->Id);         // dispel spell id
            data << uint8(0);                        // not used
            data << uint32(count);                   // count
            for (std::list<std::pair<uint32,uint64> >::iterator j = success_list.begin(); j != success_list.end(); ++j)
            {
                SpellEntry const* spellInfo = spellmgr.LookupSpell(j->first);
                data << uint32(spellInfo->Id);       // Spell Id
                data << uint8(0);                    // 0 - steals !=0 transfers
                unitTarget->RemoveAurasDueToSpellBySteal(spellInfo->Id, j->second, m_caster);
            }
            m_caster->SendMessageToSet(&data, true);
        }
    }
}

void Spell::EffectKillCredit(uint32 i)
{
    if(!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    (unitTarget->ToPlayer())->KilledMonster(m_spellInfo->EffectMiscValue[i], 0);
}

void Spell::EffectQuestFail(uint32 i)
{
    if(!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    (unitTarget->ToPlayer())->FailQuest(m_spellInfo->EffectMiscValue[i]);
}

void Spell::EffectBind(uint32 i)
{
    if (!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    Player* player = (Player*)unitTarget;

    uint32 area_id;
    WorldLocation loc;
    player->GetPosition(loc);
    area_id = player->GetAreaId();

    player->SetHomebindToLocation(loc, area_id);

    // binding
    WorldPacket data(SMSG_BINDPOINTUPDATE, (4+4+4+4+4));
    data << float(loc.m_positionX);
    data << float(loc.m_positionY);
    data << float(loc.m_positionZ);
    data << uint32(loc.m_mapId);
    data << uint32(area_id);
    player->SendDirectMessage(&data);

    DEBUG_LOG("New Home Position X is %f", loc.m_positionX);
    DEBUG_LOG("New Home Position Y is %f", loc.m_positionY);
    DEBUG_LOG("New Home Position Z is %f", loc.m_positionZ);
    DEBUG_LOG("New Home MapId is %u", loc.m_mapId);
    DEBUG_LOG("New Home AreaId is %u", area_id);

    // zone update
    data.Initialize(SMSG_PLAYERBOUND, 8+4);
    data << uint64(player->GetGUID());
    data << uint32(area_id);
    player->SendDirectMessage(&data);
}

void Spell::EffectRedirectThreat(uint32 /*i*/)
{
    if(unitTarget) {
        m_caster->SetReducedThreatPercent(100, unitTarget->GetGUID());
        m_caster->SetLastMisdirectionTargetGUID(unitTarget->GetGUID());
    }
}

void Spell::EffectForceCastWithValue(uint32 i)
{
    if (!unitTarget)
        return;

    uint32 triggered_spell_id = m_spellInfo->EffectTriggerSpell[i];

    // normal case
    SpellEntry const* spellInfo = spellmgr.LookupSpell(triggered_spell_id);

    if (!spellInfo)
    {
        sLog.outError("EffectForceCastWithValue of spell %u: triggering unknown spell id %i", m_spellInfo->Id, triggered_spell_id);
        return;
    }
    
    int32 bp = damage;
    unitTarget->CastCustomSpell(m_caster, spellInfo->Id, &bp, &bp, &bp, true);
}
