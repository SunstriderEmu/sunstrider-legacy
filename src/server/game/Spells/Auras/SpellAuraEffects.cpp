#include "SpellAuraEffects.h"
#include "BattleGround.h"
#include "CellImpl.h"
#include "Common.h"
#include "DBCStores.h"
#include "GridNotifiersImpl.h"
#include "Item.h"
#include "Log.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Opcodes.h"
#include "OutdoorPvPMgr.h"
#include "Pet.h"
#include "Player.h"
#include "Spell.h"
#include "SpellMgr.h"
#include "ThreatManager.h"
#include "Unit.h"
#include "Util.h"
#include "WorldPacket.h"
#include "SpellDefines.h"
#include "Formulas.h"
#include "ScriptMgr.h"

//
// EFFECT HANDLER NOTES
//
// in aura handler there should be check for modes:
// AURA_EFFECT_HANDLE_REAL set
// AURA_EFFECT_HANDLE_SEND_FOR_CLIENT_MASK set
// AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK set - aura is recalculated or is just applied/removed - need to redo all things related to m_amount
// AURA_EFFECT_HANDLE_CHANGE_AMOUNT_SEND_FOR_CLIENT_MASK - logical or of above conditions
// AURA_EFFECT_HANDLE_STAT - set when stats are reapplied
// such checks will speedup trinity change amount/send for client operations
// because for change amount operation packets will not be send
// aura effect handlers shouldn't contain any AuraEffect or Aura object modifications

pAuraEffectHandler AuraEffectHandler[TOTAL_AURAS] =
{
    &AuraEffect::HandleNULL,                                      //  0 SPELL_AURA_NONE
    &AuraEffect::HandleBindSight,                                 //  1 SPELL_AURA_BIND_SIGHT
    &AuraEffect::HandleModPossess,                                //  2 SPELL_AURA_MOD_POSSESS
    &AuraEffect::HandlePeriodicDamage,                            //  3 SPELL_AURA_PERIODIC_DAMAGE
    &AuraEffect::HandleAuraDummy,                                 //  4 SPELL_AURA_DUMMY
    &AuraEffect::HandleModConfuse,                                //  5 SPELL_AURA_MOD_CONFUSE
    &AuraEffect::HandleModCharm,                                  //  6 SPELL_AURA_MOD_CHARM
    &AuraEffect::HandleModFear,                                   //  7 SPELL_AURA_MOD_FEAR
    &AuraEffect::HandlePeriodicHeal,                              //  8 SPELL_AURA_PERIODIC_HEAL
    &AuraEffect::HandleModAttackSpeed,                            //  9 SPELL_AURA_MOD_ATTACKSPEED
    &AuraEffect::HandleModThreat,                                 // 10 SPELL_AURA_MOD_THREAT
    &AuraEffect::HandleModTaunt,                                  // 11 SPELL_AURA_MOD_TAUNT
    &AuraEffect::HandleAuraModStun,                               // 12 SPELL_AURA_MOD_STUN
    &AuraEffect::HandleModDamageDone,                             // 13 SPELL_AURA_MOD_DAMAGE_DONE
    &AuraEffect::HandleNoImmediateEffect,                         // 14 SPELL_AURA_MOD_DAMAGE_TAKEN implemented in Unit::MeleeDamageBonus and Unit::SpellDamageBonusDone
    &AuraEffect::HandleNoImmediateEffect,                         // 15 SPELL_AURA_DAMAGE_SHIELD    implemented in Unit::DoAttackDamage
    &AuraEffect::HandleModStealth,                                // 16 SPELL_AURA_MOD_STEALTH
    &AuraEffect::HandleModStealthDetect,                          // 17 SPELL_AURA_MOD_STEALTH_DETECT
    &AuraEffect::HandleModInvisibility,                           // 18 SPELL_AURA_MOD_INVISIBILITY
    &AuraEffect::HandleModInvisibilityDetect,                     // 19 SPELL_AURA_MOD_INVISIBILITY_DETECTION
    &AuraEffect::HandleNoImmediateEffect,                         // 20 SPELL_AURA_OBS_MOD_HEALTH
    &AuraEffect::HandleNoImmediateEffect,                         // 21 SPELL_AURA_OBS_MOD_POWER
    &AuraEffect::HandleAuraModResistance,                         // 22 SPELL_AURA_MOD_RESISTANCE
    &AuraEffect::HandlePeriodicTriggerSpell,                      // 23 SPELL_AURA_PERIODIC_TRIGGER_SPELL implemented in AuraEffect::PeriodicTick
    &AuraEffect::HandlePeriodicEnergize,                          // 24 SPELL_AURA_PERIODIC_ENERGIZE
    &AuraEffect::HandleAuraModPacify,                             // 25 SPELL_AURA_MOD_PACIFY
    &AuraEffect::HandleAuraModRoot,                               // 26 SPELL_AURA_MOD_ROOT
    &AuraEffect::HandleAuraModSilence,                            // 27 SPELL_AURA_MOD_SILENCE
    &AuraEffect::HandleNoImmediateEffect,                         // 28 SPELL_AURA_REFLECT_SPELLS        implement in Unit::SpellHitResult
    &AuraEffect::HandleAuraModStat,                               // 29 SPELL_AURA_MOD_STAT
    &AuraEffect::HandleAuraModSkill,                              // 30 SPELL_AURA_MOD_SKILL
    &AuraEffect::HandleAuraModIncreaseSpeed,                      // 31 SPELL_AURA_MOD_INCREASE_SPEED
    &AuraEffect::HandleAuraModIncreaseMountedSpeed,               // 32 SPELL_AURA_MOD_INCREASE_MOUNTED_SPEED
    &AuraEffect::HandleAuraModDecreaseSpeed,                      // 33 SPELL_AURA_MOD_DECREASE_SPEED
    &AuraEffect::HandleAuraModIncreaseHealth,                     // 34 SPELL_AURA_MOD_INCREASE_HEALTH
    &AuraEffect::HandleAuraModIncreaseEnergy,                     // 35 SPELL_AURA_MOD_INCREASE_ENERGY
    &AuraEffect::HandleAuraModShapeshift,                         // 36 SPELL_AURA_MOD_SHAPESHIFT
    &AuraEffect::HandleAuraModEffectImmunity,                     // 37 SPELL_AURA_EFFECT_IMMUNITY
    &AuraEffect::HandleAuraModStateImmunity,                      // 38 SPELL_AURA_STATE_IMMUNITY
    &AuraEffect::HandleAuraModSchoolImmunity,                     // 39 SPELL_AURA_SCHOOL_IMMUNITY
    &AuraEffect::HandleAuraModDmgImmunity,                        // 40 SPELL_AURA_DAMAGE_IMMUNITY
    &AuraEffect::HandleAuraModDispelImmunity,                     // 41 SPELL_AURA_DISPEL_IMMUNITY
    &AuraEffect::HandleNoImmediateEffect,                         // 42 SPELL_AURA_PROC_TRIGGER_SPELL  implemented in AuraEffect::HandleProc
    &AuraEffect::HandleNoImmediateEffect,                         // 43 SPELL_AURA_PROC_TRIGGER_DAMAGE implemented in AuraEffect::HandleProc
    &AuraEffect::HandleAuraTrackCreatures,                        // 44 SPELL_AURA_TRACK_CREATURES
    &AuraEffect::HandleAuraTrackResources,                        // 45 SPELL_AURA_TRACK_RESOURCES
    &AuraEffect::HandleUnused,                                    // 46 SPELL_AURA_MOD_PARRY_SKILL    obsolete?
    &AuraEffect::HandleAuraModParryPercent,                       // 47 SPELL_AURA_MOD_PARRY_PERCENT
    &AuraEffect::HandleUnused,                                    // 48 SPELL_AURA_MOD_DODGE_SKILL    obsolete?
    &AuraEffect::HandleAuraModDodgePercent,                       // 49 SPELL_AURA_MOD_DODGE_PERCENT
    &AuraEffect::HandleUnused,                                    // 50 SPELL_AURA_MOD_BLOCK_SKILL    obsolete?
    &AuraEffect::HandleAuraModBlockPercent,                       // 51 SPELL_AURA_MOD_BLOCK_PERCENT
    &AuraEffect::HandleAuraModWeaponCritPercent,                  // 52 SPELL_AURA_MOD_WEAPON_CRIT_PERCENT
    &AuraEffect::HandleNoImmediateEffect,                         // 53 SPELL_AURA_PERIODIC_LEECH implemented in AuraEffect::PeriodicTick
    &AuraEffect::HandleModHitChance,                              // 54 SPELL_AURA_MOD_HIT_CHANCE
    &AuraEffect::HandleModSpellHitChance,                         // 55 SPELL_AURA_MOD_SPELL_HIT_CHANCE
    &AuraEffect::HandleAuraTransform,                             // 56 SPELL_AURA_TRANSFORM
    &AuraEffect::HandleModSpellCritChance,                        // 57 SPELL_AURA_MOD_SPELL_CRIT_CHANCE
    &AuraEffect::HandleAuraModIncreaseSwimSpeed,                  // 58 SPELL_AURA_MOD_INCREASE_SWIM_SPEED
    &AuraEffect::HandleNoImmediateEffect,                         // 59 SPELL_AURA_MOD_DAMAGE_DONE_CREATURE implemented in Unit::MeleeDamageBonus and Unit::SpellDamageBonusDone
    &AuraEffect::HandleAuraModPacifyAndSilence,                   // 60 SPELL_AURA_MOD_PACIFY_SILENCE
    &AuraEffect::HandleAuraModScale,                              // 61 SPELL_AURA_MOD_SCALE
    &AuraEffect::HandleNULL,                                      // 62 SPELL_AURA_PERIODIC_HEALTH_FUNNEL
    &AuraEffect::HandleUnused,                                    // 63 SPELL_AURA_PERIODIC_MANA_FUNNEL obsolete?
    &AuraEffect::HandleNoImmediateEffect,                         // 64 SPELL_AURA_PERIODIC_MANA_LEECH implemented in AuraEffect::PeriodicTick
    &AuraEffect::HandleModCastingSpeed,                           // 65 SPELL_AURA_MOD_CASTING_SPEED
    &AuraEffect::HandleFeignDeath,                                // 66 SPELL_AURA_FEIGN_DEATH
    &AuraEffect::HandleAuraModDisarm,                             // 67 SPELL_AURA_MOD_DISARM
    &AuraEffect::HandleAuraModStalked,                            // 68 SPELL_AURA_MOD_STALKED
    &AuraEffect::HandleSchoolAbsorb,                              // 69 SPELL_AURA_SCHOOL_ABSORB implemented in Unit::CalcAbsorbResist
    &AuraEffect::HandleUnused,                                    // 70 SPELL_AURA_EXTRA_ATTACKS      Useless, used by only one spell that has only visual effect
    &AuraEffect::HandleModSpellCritChanceShool,                   // 71 SPELL_AURA_MOD_SPELL_CRIT_CHANCE_SCHOOL
    &AuraEffect::HandleModPowerCostPCT,                           // 72 SPELL_AURA_MOD_POWER_COST_SCHOOL_PCT
    &AuraEffect::HandleModPowerCost,                              // 73 SPELL_AURA_MOD_POWER_COST_SCHOOL
    &AuraEffect::HandleNoImmediateEffect,                         // 74 SPELL_AURA_REFLECT_SPELLS_SCHOOL  implemented in Unit::SpellHitResult
    &AuraEffect::HandleNoImmediateEffect,                         // 75 SPELL_AURA_MOD_LANGUAGE
    &AuraEffect::HandleNoImmediateEffect,                         // 76 SPELL_AURA_FAR_SIGHT
    &AuraEffect::HandleModMechanicImmunity,                       // 77 SPELL_AURA_MECHANIC_IMMUNITY
    &AuraEffect::HandleAuraMounted,                               // 78 SPELL_AURA_MOUNTED
    &AuraEffect::HandleModDamagePercentDone,                      // 79 SPELL_AURA_MOD_DAMAGE_PERCENT_DONE
    &AuraEffect::HandleModPercentStat,                            // 80 SPELL_AURA_MOD_PERCENT_STAT
    &AuraEffect::HandleNoImmediateEffect,                         // 81 SPELL_AURA_SPLIT_DAMAGE_PCT
    &AuraEffect::HandleWaterBreathing,                            // 82 SPELL_AURA_WATER_BREATHING
    &AuraEffect::HandleModBaseResistance,                         // 83 SPELL_AURA_MOD_BASE_RESISTANCE
    &AuraEffect::HandleNoImmediateEffect,                         // 84 SPELL_AURA_MOD_REGEN implemented in Player::RegenerateHealth
    &AuraEffect::HandleModPowerRegen,                             // 85 SPELL_AURA_MOD_POWER_REGEN implemented in Player::Regenerate
    &AuraEffect::HandleChannelDeathItem,                          // 86 SPELL_AURA_CHANNEL_DEATH_ITEM
    &AuraEffect::HandleNoImmediateEffect,                         // 87 SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN implemented in Unit::MeleeDamageBonus and Unit::SpellDamageBonusDone
    &AuraEffect::HandleNoImmediateEffect,                         // 88 SPELL_AURA_MOD_HEALTH_REGEN_PERCENT
    &AuraEffect::HandleNoImmediateEffect,                         // 89 SPELL_AURA_PERIODIC_DAMAGE_PERCENT implemented in AuraEffect::PeriodicTick
    &AuraEffect::HandleUnused,                                    // 90 SPELL_AURA_MOD_RESIST_CHANCE  Useless
    &AuraEffect::HandleNoImmediateEffect,                         // 91 SPELL_AURA_MOD_STEALTH_DETECT_RANGE implemented in Creature::GetAggroRange
    &AuraEffect::HandlePreventFleeing,                            // 92 SPELL_AURA_PREVENTS_FLEEING
    &AuraEffect::HandleModUnattackable,                           // 93 SPELL_AURA_MOD_UNATTACKABLE
    &AuraEffect::HandleNoImmediateEffect,                         // 94 SPELL_AURA_INTERRUPT_REGEN implemented in Player::RegenerateAll
    &AuraEffect::HandleAuraGhost,                                 // 95 SPELL_AURA_GHOST
    &AuraEffect::HandleNoImmediateEffect,                         // 96 SPELL_AURA_SPELL_MAGNET implemented in Unit::GetMagicHitRedirectTarget
    &AuraEffect::HandleNoImmediateEffect,                         // 97 SPELL_AURA_MANA_SHIELD implemented in Unit::CalcAbsorbResist
    &AuraEffect::HandleAuraModSkill,                              // 98 SPELL_AURA_MOD_SKILL_TALENT
    &AuraEffect::HandleAuraModAttackPower,                        // 99 SPELL_AURA_MOD_ATTACK_POWER
    &AuraEffect::HandleUnused,                                    //100 SPELL_AURA_AURAS_VISIBLE obsolete? all player can see all auras now
    &AuraEffect::HandleModResistancePercent,                      //101 SPELL_AURA_MOD_RESISTANCE_PCT
    &AuraEffect::HandleNoImmediateEffect,                         //102 SPELL_AURA_MOD_MELEE_ATTACK_POWER_VERSUS implemented in Unit::MeleeDamageBonus
    &AuraEffect::HandleAuraModTotalThreat,                        //103 SPELL_AURA_MOD_TOTAL_THREAT
    &AuraEffect::HandleAuraWaterWalk,                             //104 SPELL_AURA_WATER_WALK
    &AuraEffect::HandleAuraFeatherFall,                           //105 SPELL_AURA_FEATHER_FALL
    &AuraEffect::HandleAuraHover,                                 //106 SPELL_AURA_HOVER
    &AuraEffect::HandleNoImmediateEffect,                         //107 SPELL_AURA_ADD_FLAT_MODIFIER implemented in AuraEffect::CalculateSpellMod()
    &AuraEffect::HandleNoImmediateEffect,                         //108 SPELL_AURA_ADD_PCT_MODIFIER implemented in AuraEffect::CalculateSpellMod()
    &AuraEffect::HandleNoImmediateEffect,                         //109 SPELL_AURA_ADD_TARGET_TRIGGER
    &AuraEffect::HandleModPowerRegenPCT,                          //110 SPELL_AURA_MOD_POWER_REGEN_PERCENT implemented in Player::Regenerate, Creature::Regenerate
    &AuraEffect::HandleNoImmediateEffect,                         //111 SPELL_AURA_ADD_CASTER_HIT_TRIGGER NOT IMPLEMENTED
    &AuraEffect::HandleNoImmediateEffect,                         //112 SPELL_AURA_OVERRIDE_CLASS_SCRIPTS
    &AuraEffect::HandleNoImmediateEffect,                         //113 SPELL_AURA_MOD_RANGED_DAMAGE_TAKEN implemented in Unit::MeleeDamageBonus
    &AuraEffect::HandleNoImmediateEffect,                         //114 SPELL_AURA_MOD_RANGED_DAMAGE_TAKEN_PCT implemented in Unit::MeleeDamageBonus
    &AuraEffect::HandleNoImmediateEffect,                         //115 SPELL_AURA_MOD_HEALING
    &AuraEffect::HandleNoImmediateEffect,                         //116 SPELL_AURA_MOD_REGEN_DURING_COMBAT
    &AuraEffect::HandleNoImmediateEffect,                         //117 SPELL_AURA_MOD_MECHANIC_RESISTANCE     implemented in Unit::MagicSpellHitResult
    &AuraEffect::HandleNoImmediateEffect,                         //118 SPELL_AURA_MOD_HEALING_PCT             implemented in Unit::SpellHealingBonus
    &AuraEffect::HandleUnused,                                    //119 SPELL_AURA_SHARE_PET_TRACKING useless
    &AuraEffect::HandleAuraUntrackable,                           //120 SPELL_AURA_UNTRACKABLE
    &AuraEffect::HandleAuraEmpathy,                               //121 SPELL_AURA_EMPATHY
    &AuraEffect::HandleModOffhandDamagePercent,                   //122 SPELL_AURA_MOD_OFFHAND_DAMAGE_PCT
    &AuraEffect::HandleModTargetResistance,                       //123 SPELL_AURA_MOD_TARGET_RESISTANCE
    &AuraEffect::HandleAuraModRangedAttackPower,                  //124 SPELL_AURA_MOD_RANGED_ATTACK_POWER
    &AuraEffect::HandleNoImmediateEffect,                         //125 SPELL_AURA_MOD_MELEE_DAMAGE_TAKEN implemented in Unit::MeleeDamageBonus
    &AuraEffect::HandleNoImmediateEffect,                         //126 SPELL_AURA_MOD_MELEE_DAMAGE_TAKEN_PCT implemented in Unit::MeleeDamageBonus
    &AuraEffect::HandleNoImmediateEffect,                         //127 SPELL_AURA_RANGED_ATTACK_POWER_ATTACKER_BONUS implemented in Unit::MeleeDamageBonus
    &AuraEffect::HandleModPossessPet,                             //128 SPELL_AURA_MOD_POSSESS_PET
    &AuraEffect::HandleAuraModIncreaseSpeed,                      //129 SPELL_AURA_MOD_SPEED_ALWAYS
    &AuraEffect::HandleAuraModIncreaseMountedSpeed,               //130 SPELL_AURA_MOD_MOUNTED_SPEED_ALWAYS
    &AuraEffect::HandleNoImmediateEffect,                         //131 SPELL_AURA_MOD_RANGED_ATTACK_POWER_VERSUS implemented in Unit::MeleeDamageBonus
    &AuraEffect::HandleAuraModIncreaseEnergyPercent,              //132 SPELL_AURA_MOD_INCREASE_ENERGY_PERCENT
    &AuraEffect::HandleAuraModIncreaseHealthPercent,              //133 SPELL_AURA_MOD_INCREASE_HEALTH_PERCENT
    &AuraEffect::HandleAuraModRegenInterrupt,                     //134 SPELL_AURA_MOD_MANA_REGEN_INTERRUPT
    &AuraEffect::HandleModHealingDone,                            //135 SPELL_AURA_MOD_HEALING_DONE
    &AuraEffect::HandleNoImmediateEffect,                         //136 SPELL_AURA_MOD_HEALING_DONE_PERCENT   implemented in Unit::SpellHealingBonus
    &AuraEffect::HandleModTotalPercentStat,                       //137 SPELL_AURA_MOD_TOTAL_STAT_PERCENTAGE
    &AuraEffect::HandleModMeleeSpeedPct,                                     //138 SPELL_AURA_MOD_MELEE_HASTE
    &AuraEffect::HandleForceReaction,                             //139 SPELL_AURA_FORCE_REACTION
    &AuraEffect::HandleAuraModRangedHaste,                        //140 SPELL_AURA_MOD_RANGED_HASTE
    &AuraEffect::HandleRangedAmmoHaste,                           //141 SPELL_AURA_MOD_RANGED_AMMO_HASTE
    &AuraEffect::HandleAuraModBaseResistancePCT,                  //142 SPELL_AURA_MOD_BASE_RESISTANCE_PCT
    &AuraEffect::HandleAuraModResistanceExclusive,                //143 SPELL_AURA_MOD_RESISTANCE_EXCLUSIVE
    &AuraEffect::HandleNoImmediateEffect,                         //144 SPELL_AURA_SAFE_FALL                  implemented in WorldSession::HandleMovementOpcodes
    &AuraEffect::HandleUnused,                                    //145 SPELL_AURA_CHARISMA obsolete?
    &AuraEffect::HandleUnused,                                    //146 SPELL_AURA_PERSUADED obsolete?
    &AuraEffect::HandleModMechanicImmunityMask,                   //147 SPELL_AURA_MECHANIC_IMMUNITY_MASK
    &AuraEffect::HandleAuraRetainComboPoints,                     //148 SPELL_AURA_RETAIN_COMBO_POINTS
    &AuraEffect::HandleNoImmediateEffect,                         //149 SPELL_AURA_RESIST_PUSHBACK
    &AuraEffect::HandleShieldBlockValuePercent,                   //150 SPELL_AURA_MOD_SHIELD_BLOCKVALUE_PCT
    &AuraEffect::HandleAuraTrackStealthed,                        //151 SPELL_AURA_TRACK_STEALTHED
    &AuraEffect::HandleNoImmediateEffect,                         //152 SPELL_AURA_MOD_STEALTH_DETECTED_RANGE implemented in Creature::GetAggroRange
    &AuraEffect::HandleNoImmediateEffect,                         //153 SPELL_AURA_SPLIT_DAMAGE_FLAT
    &AuraEffect::HandleModStealthLevel,                           //154 SPELL_AURA_MOD_STEALTH_LEVEL
    &AuraEffect::HandleNoImmediateEffect,                         //155 SPELL_AURA_MOD_WATER_BREATHING
    &AuraEffect::HandleNoImmediateEffect,                         //156 SPELL_AURA_MOD_REPUTATION_GAIN
    &AuraEffect::HandleNULL,                                      //157 SPELL_AURA_PET_DAMAGE_MULTI
    &AuraEffect::HandleShieldBlockValue,                          //158 SPELL_AURA_MOD_SHIELD_BLOCKVALUE
    &AuraEffect::HandleNoImmediateEffect,                         //159 SPELL_AURA_NO_PVP_CREDIT      only for Honorless Target spell
    &AuraEffect::HandleNoImmediateEffect,                         //160 SPELL_AURA_MOD_AOE_AVOIDANCE                 implemented in Unit::MagicSpellHitResult
    &AuraEffect::HandleNoImmediateEffect,                         //161 SPELL_AURA_MOD_HEALTH_REGEN_IN_COMBAT
    &AuraEffect::HandleNoImmediateEffect,                         //162 SPELL_AURA_POWER_BURN implemented in AuraEffect::PeriodicTick
    &AuraEffect::HandleNoImmediateEffect,                         //163 SPELL_AURA_MOD_CRIT_DAMAGE_BONUS
    &AuraEffect::HandleUnused,                                    //164 useless, only one test spell
    &AuraEffect::HandleNoImmediateEffect,                         //165 SPELL_AURA_MELEE_ATTACK_POWER_ATTACKER_BONUS implemented in Unit::MeleeDamageBonus
    &AuraEffect::HandleAuraModAttackPowerPercent,                 //166 SPELL_AURA_MOD_ATTACK_POWER_PCT
    &AuraEffect::HandleAuraModRangedAttackPowerPercent,           //167 SPELL_AURA_MOD_RANGED_ATTACK_POWER_PCT
    &AuraEffect::HandleNoImmediateEffect,                         //168 SPELL_AURA_MOD_DAMAGE_DONE_VERSUS            implemented in Unit::SpellDamageBonusDone, Unit::MeleeDamageBonus
    &AuraEffect::HandleNoImmediateEffect,                         //169 SPELL_AURA_MOD_CRIT_PERCENT_VERSUS           implemented in Unit::DealDamageBySchool, Unit::DoAttackDamage, Unit::SpellCriticalDamageBonus
    &AuraEffect::HandleNULL,                                      //170 SPELL_AURA_DETECT_AMORE       only for Detect Amore spell
    &AuraEffect::HandleAuraModIncreaseSpeed,                      //171 SPELL_AURA_MOD_SPEED_NOT_STACK
    &AuraEffect::HandleAuraModIncreaseMountedSpeed,               //172 SPELL_AURA_MOD_MOUNTED_SPEED_NOT_STACK
    &AuraEffect::HandleUnused,                                    //173 SPELL_AURA_ALLOW_CHAMPION_SPELLS  only for Proclaim Champion spell
    &AuraEffect::HandleModSpellDamagePercentFromStat,             //174 SPELL_AURA_MOD_SPELL_DAMAGE_OF_STAT_PERCENT  implemented in Unit::SpellBaseDamageBonusDone (by default intellect, dependent from SPELL_AURA_MOD_SPELL_HEALING_OF_STAT_PERCENT)
    &AuraEffect::HandleModSpellHealingPercentFromStat,            //175 SPELL_AURA_MOD_SPELL_HEALING_OF_STAT_PERCENT implemented in Unit::SpellBaseHealingBonusDone
    &AuraEffect::HandleSpiritOfRedemption,                        //176 SPELL_AURA_SPIRIT_OF_REDEMPTION   only for Spirit of Redemption spell, die at aura end
    &AuraEffect::HandleAOECharm,                                  //177 SPELL_AURA_AOE_CHARM
    &AuraEffect::HandleNoImmediateEffect,                         //178 SPELL_AURA_MOD_DEBUFF_RESISTANCE          implemented in Unit::MagicSpellHitResult
    &AuraEffect::HandleNoImmediateEffect,                         //179 SPELL_AURA_MOD_ATTACKER_SPELL_CRIT_CHANCE implemented in Unit::SpellCriticalDamageBonus
    &AuraEffect::HandleNoImmediateEffect,                         //180 SPELL_AURA_MOD_FLAT_SPELL_DAMAGE_VERSUS   implemented in Unit::SpellDamageBonusDone
    &AuraEffect::HandleUnused,                                    //181 SPELL_AURA_MOD_FLAT_SPELL_CRIT_DAMAGE_VERSUS unused
    &AuraEffect::HandleAuraModResistenceOfStatPercent,            //182 SPELL_AURA_MOD_RESISTANCE_OF_STAT_PERCENT
    &AuraEffect::HandleNULL,                                      //183 SPELL_AURA_MOD_CRITICAL_THREAT
    &AuraEffect::HandleNoImmediateEffect,                         //184 SPELL_AURA_MOD_ATTACKER_MELEE_HIT_CHANCE  implemented in Unit::RollMeleeOutcomeAgainst
    &AuraEffect::HandleNoImmediateEffect,                         //185 SPELL_AURA_MOD_ATTACKER_RANGED_HIT_CHANCE implemented in Unit::RollMeleeOutcomeAgainst
    &AuraEffect::HandleNoImmediateEffect,                         //186 SPELL_AURA_MOD_ATTACKER_SPELL_HIT_CHANCE  implemented in Unit::MagicSpellHitResult
    &AuraEffect::HandleNoImmediateEffect,                         //187 SPELL_AURA_MOD_ATTACKER_MELEE_CRIT_CHANCE  implemented in Unit::GetUnitCriticalChanceAgainst
    &AuraEffect::HandleNoImmediateEffect,                         //188 SPELL_AURA_MOD_ATTACKER_RANGED_CRIT_CHANCE implemented in Unit::GetUnitCriticalChanceAgainst
    &AuraEffect::HandleModRating,                                 //189 SPELL_AURA_MOD_RATING
    &AuraEffect::HandleNULL,                                      //190 SPELL_AURA_MOD_FACTION_REPUTATION_GAIN
    &AuraEffect::HandleAuraModUseNormalSpeed,                     //191 SPELL_AURA_USE_NORMAL_MOVEMENT_SPEED
    &AuraEffect::HandleModMeleeRangedSpeedPct,                    //192 SPELL_AURA_MOD_MELEE_RANGED_HASTE
    &AuraEffect::HandleModCombatSpeedPct,                         //193 SPELL_AURA_MELEE_SLOW (in fact combat (any type attack) speed pct)
    &AuraEffect::HandleUnused,                                    //194 SPELL_AURA_MOD_DEPRICATED_1 not used now (old SPELL_AURA_MOD_SPELL_DAMAGE_OF_INTELLECT)
    &AuraEffect::HandleUnused,                                    //195 SPELL_AURA_MOD_DEPRICATED_2 not used now (old SPELL_AURA_MOD_SPELL_HEALING_OF_INTELLECT)
    &AuraEffect::HandleNULL,                                      //196 SPELL_AURA_MOD_COOLDOWN
    &AuraEffect::HandleNoImmediateEffect,                         //197 SPELL_AURA_MOD_ATTACKER_SPELL_AND_WEAPON_CRIT_CHANCE implemented in Unit::SpellCriticalDamageBonus Unit::GetUnitCriticalChanceAgainst
    &AuraEffect::HandleUnused,                                    //198 SPELL_AURA_MOD_ALL_WEAPON_SKILLS
    &AuraEffect::HandleNoImmediateEffect,                         //199 SPELL_AURA_MOD_INCREASES_SPELL_PCT_TO_HIT  implemented in Unit::MagicSpellHitResult
    &AuraEffect::HandleNoImmediateEffect,                         //200 SPELL_AURA_MOD_XP_PCT implemented in Player::GiveXP
    &AuraEffect::HandleAuraAllowFlight,                           //201 SPELL_AURA_FLY                             this aura enable flight mode...
    &AuraEffect::HandleNoImmediateEffect,                         //202 SPELL_AURA_CANNOT_BE_DODGED                implemented in Unit::RollPhysicalOutcomeAgainst
    &AuraEffect::HandleNoImmediateEffect,                         //203 SPELL_AURA_MOD_ATTACKER_MELEE_CRIT_DAMAGE  implemented in Unit::CalculateMeleeDamage and Unit::CalculateSpellDamage
    &AuraEffect::HandleNoImmediateEffect,                         //204 SPELL_AURA_MOD_ATTACKER_RANGED_CRIT_DAMAGE implemented in Unit::CalculateMeleeDamage and Unit::CalculateSpellDamage
    &AuraEffect::HandleNULL,                                      //205 vulnerable to school dmg?
    &AuraEffect::HandleNULL,                                      //206 SPELL_AURA_MOD_INCREASE_VEHICLE_FLIGHT_SPEED
    &AuraEffect::HandleAuraModIncreaseFlightSpeed,                //207 SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED
    &AuraEffect::HandleAuraModIncreaseFlightSpeed,                //208 SPELL_AURA_MOD_INCREASE_FLIGHT_SPEED, used only in spell: Flight Form (Passive)
    &AuraEffect::HandleAuraModIncreaseFlightSpeed,                //209 SPELL_AURA_MOD_MOUNTED_FLIGHT_SPEED_ALWAYS
    &AuraEffect::HandleNULL,                                      //210 Commentator's Command
    &AuraEffect::HandleAuraModIncreaseFlightSpeed,                //211 SPELL_AURA_MOD_FLIGHT_SPEED_NOT_STACK
    &AuraEffect::HandleAuraModRangedAttackPowerOfStatPercent,     //212 SPELL_AURA_MOD_RANGED_ATTACK_POWER_OF_STAT_PERCENT
    &AuraEffect::HandleNoImmediateEffect,                         //213 SPELL_AURA_MOD_RAGE_FROM_DAMAGE_DEALT implemented in Player::RewardRage
    &AuraEffect::HandleNULL,                                      //214 Tamed Pet Passive
    &AuraEffect::HandleArenaPreparation,                          //215 SPELL_AURA_ARENA_PREPARATION
    &AuraEffect::HandleModCastingSpeed,                           //216 SPELL_AURA_HASTE_SPELLS
    &AuraEffect::HandleUnused,                                    //217                                   unused
    &AuraEffect::HandleAuraModRangedHaste,                        //218 SPELL_AURA_HASTE_RANGED
    &AuraEffect::HandleModManaRegen,                              //219 SPELL_AURA_MOD_MANA_REGEN_FROM_STAT
    &AuraEffect::HandleNULL,                                      //220 SPELL_AURA_MOD_RATING_FROM_STAT
    &AuraEffect::HandleModDetaunt,                                //221 SPELL_AURA_MOD_DETAUNT
    &AuraEffect::HandleUnused,                                    //222 unused
    &AuraEffect::HandleNULL,                                      //223 Cold Stare
    &AuraEffect::HandleUnused,                                    //224 unused
    &AuraEffect::HandleNoImmediateEffect,                         //225 SPELL_AURA_PRAYER_OF_MENDING
    &AuraEffect::HandleAuraPeriodicDummy,                         //226 SPELL_AURA_PERIODIC_DUMMY - Handled in AuraEffect::HandleAuraPeriodicDummy + AuraEffect::PeriodicTick
    &AuraEffect::HandleNoImmediateEffect,                         //227 SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE implemented in AuraEffect::PeriodicTick
    &AuraEffect::HandleNoImmediateEffect,                         //228 stealth detection
    &AuraEffect::HandleNULL,                                      //229 SPELL_AURA_MOD_AOE_DAMAGE_AVOIDANCE
    &AuraEffect::HandleAuraModIncreaseMaxHealth,                  //230 Commanding Shout
    &AuraEffect::HandleNULL,                                      //231
    &AuraEffect::HandleNoImmediateEffect,                         //232 SPELL_AURA_MECHANIC_DURATION_MOD           implement in Unit::ModSpellDuration
    &AuraEffect::HandleNULL,                                      //233 set model id to the one of the creature with id GetMiscValue()
    &AuraEffect::HandleNoImmediateEffect,                         //234 SPELL_AURA_MECHANIC_DURATION_MOD_NOT_STACK implement in Unit::ModSpellDuration
    &AuraEffect::HandleUnused,                                    //235 SPELL_AURA_MOD_DISPEL_RESIST               implement in Unit::MagicSpellHitResult
    &AuraEffect::HandleUnused,                                    //236 unused
    &AuraEffect::HandleModSpellDamagePercentFromAttackPower,      //237 SPELL_AURA_MOD_SPELL_DAMAGE_OF_ATTACK_POWER  implemented in Unit::SpellBaseDamageBonusDone
    &AuraEffect::HandleModSpellHealingPercentFromAttackPower,     //238 SPELL_AURA_MOD_SPELL_HEALING_OF_ATTACK_POWER implemented in Unit::SpellBaseHealingBonusDone
    &AuraEffect::HandleAuraModScale,                              //239 SPELL_AURA_MOD_SCALE_2 only in Noggenfogger Elixir (16595) before 2.3.0 aura 61
    &AuraEffect::HandleAuraModExpertise,                          //240 SPELL_AURA_MOD_EXPERTISE
    &AuraEffect::HandleForceMoveForward,                          //241 Forces the player to move forward
    &AuraEffect::HandleUnused,                                    //242 SPELL_AURA_MOD_SPELL_DAMAGE_FROM_HEALING
    &AuraEffect::HandleUnused,                                    //243 used by two test spells
    &AuraEffect::HandleComprehendLanguage,                        //244 Comprehend language
    &AuraEffect::HandleUnused,                                    //245 SPELL_AURA_MOD_DURATION_OF_MAGIC_EFFECTS
    &AuraEffect::HandleUnused,                                    //246 unused
    &AuraEffect::HandleAuraCloneCaster,                           //247 SPELL_AURA_CLONE_CASTER
    &AuraEffect::HandleNoImmediateEffect,                         //248 SPELL_AURA_MOD_COMBAT_RESULT_CHANCE         implemented in Unit::RollMeleeOutcomeAgainst
    &AuraEffect::HandleNULL,                                      //249
    &AuraEffect::HandleAuraModIncreaseHealth,                     //250 SPELL_AURA_MOD_INCREASE_HEALTH_2
    &AuraEffect::HandleNoImmediateEffect,                         //251 SPELL_AURA_MOD_ENEMY_DODGE
    &AuraEffect::HandleUnused,                                    //252 unused
    &AuraEffect::HandleUnused,                                    //253 unused
    &AuraEffect::HandleUnused,                                    //254 unused
    &AuraEffect::HandleUnused,                                    //255 unused
    &AuraEffect::HandleUnused,                                    //256 unused
    &AuraEffect::HandleUnused,                                    //257 unused
    &AuraEffect::HandleUnused,                                    //258 unused
    &AuraEffect::HandleAuraImmunityId,                            //259 SPELL_AURA_APPLY_IMMUNITY_ID - WM custom
    &AuraEffect::HandleAuraApplyExtraFlag,                        //260 SPELL_AURA_APPLY_EXTRA_FLAG - WM custom
    &AuraEffect::HandleNULL                                       //261 SPELL_AURA_261 some phased state (44856 spell)
};

AuraEffect::AuraEffect(Aura* base, uint8 effIndex, int32 *baseAmount, Unit* caster) :
    m_base(base), m_spellInfo(base->GetSpellInfo()),
    m_baseAmount(baseAmount ? *baseAmount : m_spellInfo->Effects[effIndex].BasePoints),
    _amount(), m_spellmod(nullptr), _periodicTimer(0), _amplitude(0), _ticksDone(0), m_effIndex(effIndex),
    m_canBeRecalculated(true), m_isPeriodic(false)
{
    CalculatePeriodic(caster, true, false);

    _amount = CalculateAmount(caster);

    CalculateSpellMod();
}

AuraEffect::~AuraEffect()
{
    delete m_spellmod;
}

template <typename Container>
void AuraEffect::GetTargetList(Container& targetContainer) const
{
    Aura::ApplicationMap const& targetMap = GetBase()->GetApplicationMap();
    // remove all targets which were not added to new list - they no longer deserve area aura
    for (auto appIter = targetMap.begin(); appIter != targetMap.end(); ++appIter)
    {
        if (appIter->second->HasEffect(GetEffIndex()))
            targetContainer.push_back(appIter->second->GetTarget());
    }
}

template <typename Container>
void AuraEffect::GetApplicationList(Container& applicationContainer) const
{
    Aura::ApplicationMap const& targetMap = GetBase()->GetApplicationMap();
    for (auto appIter = targetMap.begin(); appIter != targetMap.end(); ++appIter)
    {
        if (appIter->second->HasEffect(GetEffIndex()))
            applicationContainer.push_back(appIter->second);
    }
}

int32 AuraEffect::CalculateAmount(Unit* caster)
{
    int32 amount = m_spellInfo->Effects[m_effIndex].CalcValue(caster, &m_baseAmount, nullptr);

    // check item enchant aura cast
    if (!amount && caster)
        if (ObjectGuid itemGUID = GetBase()->GetCastItemGUID())
            if (Player* playerCaster = caster->ToPlayer())
                if (Item* castItem = playerCaster->GetItemByGuid(itemGUID))
                    if (castItem->GetItemSuffixFactor())
                    {
                        ItemRandomSuffixEntry const* item_rand_suffix = sItemRandomSuffixStore.LookupEntry(abs(castItem->GetItemRandomPropertyId()));
                        if (item_rand_suffix)
                        {
                            for (int k = 0; k < MAX_ITEM_ENCHANTMENT_EFFECTS; k++)
                            {
                                SpellItemEnchantmentEntry const* pEnchant = sSpellItemEnchantmentStore.LookupEntry(item_rand_suffix->enchant_id[k]);
                                if (pEnchant)
                                {
                                    for (int t = 0; t < MAX_ITEM_ENCHANTMENT_EFFECTS; t++)
                                        if (pEnchant->spellid[t] == m_spellInfo->Id)
                                    {
                                        amount = uint32((item_rand_suffix->prefix[k]*castItem->GetItemSuffixFactor()) / 10000);
                                        break;
                                    }
                                }

                                if (amount)
                                    break;
                            }
                        }
                    }

    // custom amount calculations go here
    switch (GetAuraType())
    {
        // crowd control auras
        case SPELL_AURA_MOD_CONFUSE:
        case SPELL_AURA_MOD_FEAR:
        case SPELL_AURA_MOD_STUN:
        case SPELL_AURA_MOD_ROOT:
        case SPELL_AURA_TRANSFORM:
            m_canBeRecalculated = false;
            if (!m_spellInfo->ProcFlags)
                break;
            amount = int32(GetBase()->GetUnitOwner()->CountPctFromMaxHealth(10));
#ifdef LICH_KING
            if (caster)
            {
                // Glyphs increasing damage cap
                Unit::Unit::AuraEffectList const& overrideClassScripts = caster->GetAuraEffectsByType(SPELL_AURA_OVERRIDE_CLASS_SCRIPTS);
                for (Unit::Unit::AuraEffectList::const_iterator itr = overrideClassScripts.begin(); itr != overrideClassScripts.end(); ++itr)
                {
                    if ((*itr)->IsAffectedOnSpell(m_spellInfo))
                    {
                        // Glyph of Fear, Glyph of Frost nova and similar auras
                        if ((*itr)->GetMiscValue() == 7801)
                        {
                            AddPct(amount, (*itr)->GetAmount());
                            break;
                        }
                    }
                }
            }
#endif
            break;
        case SPELL_AURA_SCHOOL_ABSORB:
        case SPELL_AURA_MANA_SHIELD:
            m_canBeRecalculated = false;
            break;
        default:
            break;
    }

    if (caster)
    {
        switch (GetAuraType())
        {
            case SPELL_AURA_PERIODIC_DAMAGE:
            case SPELL_AURA_PERIODIC_LEECH:
                if (GetBase()->GetType() == UNIT_AURA_TYPE)
                    amount = caster->SpellDamageBonusDone(GetBase()->GetUnitOwner(), GetSpellInfo(), amount, DOT, GetEffIndex(), GetBase()->GetDonePct());
                break;
            case SPELL_AURA_PERIODIC_HEAL:
                if (GetBase()->GetType() == UNIT_AURA_TYPE)
                    amount = caster->SpellHealingBonusDone(GetBase()->GetUnitOwner(), GetSpellInfo(), amount, DOT, GetEffIndex(), GetBase()->GetDonePct());
                break;
            default:
                break;
        }
    }

    //OLD HACKS, todo: move to scripts
    {
        switch (m_spellInfo->Id)
        {
        case 44505: // "Drink Fel Infusion"
            if (m_effIndex == 0)
                amount = 500;
            break;
        case 34161: // "Wild Growth"
            if (m_effIndex == 0)
                amount = 25;
            break;
        case 20007: // Holy Strength
            int32 level_diff = caster->GetLevel() - 60;
            int32 multiplier = 4;

            if (level_diff > 0)
                amount -= multiplier * level_diff;
            break;
        }

        switch (GetAuraType())
        {
        case SPELL_AURA_RANGED_ATTACK_POWER_ATTACKER_BONUS:
            // Expose Weakness
            if (m_spellInfo->Id == 34501)
                _amount = (int32)((float)m_spellInfo->Effects[m_effIndex].BasePoints*caster->GetStat(STAT_AGILITY) / 100.0f);
            break;
        case SPELL_AURA_PERIODIC_DAMAGE:
            switch (m_spellInfo->SpellFamilyName)
            {
            case SPELLFAMILY_GENERIC:
            {
                // Pounce Bleed
                if (m_spellInfo->SpellIconID == 147 && m_spellInfo->HasVisual(0))
                {
                    // $AP*0.18/6 bonus per tick
                    if (caster)
                        _amount += int32(caster->GetTotalAttackPowerValue(BASE_ATTACK) * 3 / 100);
                    break;
                }
                if (m_spellInfo->Id == 40953) {
                    _amount = 1388 + rand() % 225;
                }
                break;
            }
            case SPELLFAMILY_WARRIOR:
            {
                // Rend
                if (m_spellInfo->SpellFamilyFlags & 0x0000000000000020LL)
                {
                    // 0.00743*(($MWB+$mwb)/2+$AP/14*$MWS) bonus per tick
                    if (caster)
                    {
                        float ap = caster->GetTotalAttackPowerValue(BASE_ATTACK);
                        int32 mws = caster->GetAttackTime(BASE_ATTACK);
                        float mwb_min = caster->GetWeaponDamageRange(BASE_ATTACK, MINDAMAGE);
                        float mwb_max = caster->GetWeaponDamageRange(BASE_ATTACK, MAXDAMAGE);
                        // WARNING! in 3.0 multiplier 0.00743f change to 0.6
                        _amount += int32(((mwb_min + mwb_max) / 2 + ap * mws / 14000)*0.00743f);
                    }
                    break;
                }
                break;
            }
            case SPELLFAMILY_DRUID:
            {
                // Rake
                if (m_spellInfo->SpellFamilyFlags & 0x0000000000001000LL)
                {
                    // $AP*0.06/3 bonus per tick
                    if (caster)
                        _amount += int32(caster->GetTotalAttackPowerValue(BASE_ATTACK) * 2 / 100);
                    break;
                }
                // Lacerate
                if (m_spellInfo->SpellFamilyFlags & 0x000000010000000000LL)
                {
                    // $AP*0.05/5 bonus per tick
                    if (caster)
                        _amount += int32(caster->GetTotalAttackPowerValue(BASE_ATTACK) / 100);
                    break;
                }
                // Rip
                if (m_spellInfo->SpellFamilyFlags & 0x000000000000800000LL)
                {
                    // $AP * min(0.06*$cp, 0.24)/6 [Yes, there is no difference, whether 4 or 5 CPs are being used]
                    if (caster && caster->GetTypeId() == TYPEID_PLAYER)
                    {
                        uint8 cp = (caster->ToPlayer())->GetComboPoints();

                        // Idol of Feral Shadows. Cant be handled as SpellMod in SpellAura:Dummy due its dependency from CPs
                        Unit::AuraEffectList const& dummyAuras = caster->GetAuraEffectsByType(SPELL_AURA_DUMMY);
                        for (auto dummyAura : dummyAuras)
                        {
                            if (dummyAura->GetId() == 34241)
                            {
                                _amount += cp * dummyAura->GetAmount();
                                break;
                            }
                        }

                        if (cp > 4) cp = 4;
                        _amount += int32(caster->GetTotalAttackPowerValue(BASE_ATTACK) * cp / 100);
                    }
                    break;
                }
                break;
            }
            case SPELLFAMILY_ROGUE:
            {
                // Rupture
                if (m_spellInfo->SpellFamilyFlags & 0x000000000000100000LL)
                {
                    // Dmg/tick = $AP*min(0.01*$cp, 0.03) [Like Rip: only the first three CP increase the contribution from AP]
                    if (caster && caster->GetTypeId() == TYPEID_PLAYER)
                    {
                        uint8 cp = (caster->ToPlayer())->GetComboPoints();
                        if (cp > 3) cp = 3;
                        _amount += int32(caster->GetTotalAttackPowerValue(BASE_ATTACK) * cp / 100);
                    }
                    break;
                }
                // Garrote
                if (m_spellInfo->SpellFamilyFlags & 0x000000000000000100LL)
                {
                    // $AP*0.18/6 bonus per tick
                    if (caster)
                        _amount += int32(caster->GetTotalAttackPowerValue(BASE_ATTACK) * 3 / 100);
                    break;
                }
                break;
            }
            case SPELLFAMILY_HUNTER:
            {
                // Serpent Sting
                if (m_spellInfo->SpellFamilyFlags & 0x0000000000004000LL)
                {
                    // $RAP*0.1/5 bonus per tick
                    if (caster) {
                        _amount += int32(caster->GetTotalAttackPowerValue(RANGED_ATTACK) * 10 / 500);
                        //_amount += int32(m_target->GetTotalAuraModifier(SPELL_AURA_RANGED_ATTACK_POWER_ATTACKER_BONUS) * 10 / 500);
                    }
                    break;
                }
                // Immolation Trap
                if (m_spellInfo->SpellFamilyFlags & 0x0000000000000004LL && m_spellInfo->SpellIconID == 678)
                {
                    // $RAP*0.1/5 bonus per tick
                    if (caster)
                        _amount += int32(caster->GetTotalAttackPowerValue(RANGED_ATTACK) * 10 / 500);
                    break;
                }
                break;
            }
            case SPELLFAMILY_PALADIN:
            {
                // Consecration
                if (m_spellInfo->SpellFamilyFlags & 0x0000000000000020LL)
                {
                    if (Unit* caster2 = GetCaster())
                    {
                        Unit::AuraEffectList const& classScripts = caster2->GetAuraEffectsByType(SPELL_AURA_OVERRIDE_CLASS_SCRIPTS);
                        for (auto classScript : classScripts)
                        {
                            int32 tickcount = m_spellInfo->GetDuration() / m_spellInfo->Effects[m_effIndex].Amplitude;
                            switch (classScript->GetMiscValue())
                            {
                            case 5147:                  // Improved Consecration - Libram of the Eternal Rest
                            {
                                _amount += classScript->GetAmount() / tickcount;
                                break;
                            }
                            }
                        }
                    }
                    break;
                }
                break;
            }
            default:
                break;
            }
            break;
        case SPELL_AURA_PERIODIC_HEAL:

            switch (m_spellInfo->SpellFamilyName)
            {
            case SPELLFAMILY_DRUID:
            {
                // Rejuvenation
                if (m_spellInfo->SpellFamilyFlags & 0x0000000000000010LL)
                {
                    if (Unit* caster = GetCaster())
                    {
                        Unit::AuraEffectList const& classScripts = caster->GetAuraEffectsByType(SPELL_AURA_OVERRIDE_CLASS_SCRIPTS);
                        for (auto classScript : classScripts)
                        {
                            switch (classScript->GetMiscValue())
                            {
                                case 4953:                          // Increased Rejuvenation Healing - Harold's Rejuvenating Broach Aura
                                case 4415:                          // Increased Rejuvenation Healing - Idol of Rejuvenation Aura
                                {
                                    _amount += classScript->GetAmount() / GetTotalTicks();
                                    break;
                                }
                            }
                        }
                    }
                }
            }
            }
            break;
        }
    }

    GetBase()->CallScriptEffectCalcAmountHandlers(this, amount, m_canBeRecalculated);
    amount *= GetBase()->GetStackAmount();
    return amount;
}

void AuraEffect::ChangeAmount(int32 newAmount, bool mark, bool onStackOrReapply)
{
    // Reapply if amount change
    uint8 handleMask = 0;
    if (newAmount != GetAmount())
        handleMask |= AURA_EFFECT_HANDLE_CHANGE_AMOUNT;
    if (onStackOrReapply)
        handleMask |= AURA_EFFECT_HANDLE_REAPPLY;

    if (!handleMask)
        return;

    std::vector<AuraApplication*> effectApplications;
    GetApplicationList(effectApplications);

    for (AuraApplication* aurApp : effectApplications)
    {
        aurApp->GetTarget()->_RegisterAuraEffect(this, false);
        HandleEffect(aurApp, handleMask, false);
    }

    if (handleMask & AURA_EFFECT_HANDLE_CHANGE_AMOUNT)
    {
        if (!mark)
            _amount = newAmount;
        else
            SetAmount(newAmount);
        CalculateSpellMod();
    }

    for (AuraApplication* aurApp : effectApplications)
    {
        if (aurApp->GetRemoveMode() != AURA_REMOVE_NONE)
            continue;

        aurApp->GetTarget()->_RegisterAuraEffect(this, true);
        HandleEffect(aurApp, handleMask, true);
    }
}

void AuraEffect::HandleEffect(AuraApplication* aurApp, uint8 mode, bool apply)
{
    // check if call is correct, we really don't want using bitmasks here (with 1 exception)
    ASSERT(mode == AURA_EFFECT_HANDLE_REAL
        || mode == AURA_EFFECT_HANDLE_SEND_FOR_CLIENT
        || mode == AURA_EFFECT_HANDLE_CHANGE_AMOUNT
        || mode == AURA_EFFECT_HANDLE_STAT
        || mode == AURA_EFFECT_HANDLE_SKILL
        || mode == AURA_EFFECT_HANDLE_REAPPLY
        || mode == (AURA_EFFECT_HANDLE_CHANGE_AMOUNT | AURA_EFFECT_HANDLE_REAPPLY));

    // register/unregister effect in lists in case of real AuraEffect apply/remove
    // registration/unregistration is done always before real effect handling (some effect handlers code is depending on this)
    if (mode & AURA_EFFECT_HANDLE_REAL)
        aurApp->GetTarget()->_RegisterAuraEffect(this, apply);

    // real aura apply/remove, handle modifier
    if (mode & AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK)
        ApplySpellMod(aurApp->GetTarget(), apply);

    // call scripts helping/replacing effect handlers
    bool prevented = false;
    if (apply)
        prevented = GetBase()->CallScriptEffectApplyHandlers(this, aurApp, (AuraEffectHandleModes)mode);
    else
        prevented = GetBase()->CallScriptEffectRemoveHandlers(this, aurApp, (AuraEffectHandleModes)mode);

    // check if script events have removed the aura already
    if (apply && aurApp->GetRemoveMode())
        return;

    // call default effect handler if it wasn't prevented
    if (!prevented)
        (*this.*AuraEffectHandler[GetAuraType()])(aurApp, mode, apply);

    // check if the default handler reemoved the aura
    if (apply && aurApp->GetRemoveMode())
        return;

    // call scripts triggering additional events after apply/remove
    if (apply)
        GetBase()->CallScriptAfterEffectApplyHandlers(this, aurApp, (AuraEffectHandleModes)mode);
    else
        GetBase()->CallScriptAfterEffectRemoveHandlers(this, aurApp, (AuraEffectHandleModes)mode);
}

void AuraEffect::HandleEffect(Unit* target, uint8 mode, bool apply)
{
    AuraApplication* aurApp = GetBase()->GetApplicationOfTarget(target->GetGUID());
    ASSERT(aurApp);
    HandleEffect(aurApp, mode, apply);
}

uint32 AuraEffect::GetTotalTicks() const
{
    uint32 totalTicks = 0;
    if (_amplitude && !GetBase()->IsPermanent())
    {
        totalTicks = static_cast<uint32>(GetBase()->GetMaxDuration() / _amplitude);
        if (m_spellInfo->HasAttribute(SPELL_ATTR5_START_PERIODIC_AT_APPLY))
            ++totalTicks;
    }

    return totalTicks;
}


void AuraEffect::ResetPeriodic(bool resetPeriodicTimer /*= false*/)
{
    _ticksDone = 0;
    if (resetPeriodicTimer)
    {
        _periodicTimer = 0;
        // Start periodic on next tick or at aura apply
        if (m_spellInfo->HasAttribute(SPELL_ATTR5_START_PERIODIC_AT_APPLY))
            _periodicTimer = _amplitude;
    }
}

void AuraEffect::CalculatePeriodic(Unit* caster, bool resetPeriodicTimer /*= true*/, bool load /*= false*/)
{
    _amplitude = m_spellInfo->Effects[m_effIndex].Amplitude;

    // prepare periodics
    switch (GetAuraType())
    {
    case SPELL_AURA_PERIODIC_DAMAGE:
    case SPELL_AURA_PERIODIC_HEAL:
    case SPELL_AURA_OBS_MOD_HEALTH:
    case SPELL_AURA_PERIODIC_TRIGGER_SPELL:
    case SPELL_AURA_PERIODIC_ENERGIZE:
    case SPELL_AURA_PERIODIC_LEECH:
    case SPELL_AURA_PERIODIC_HEALTH_FUNNEL:
    case SPELL_AURA_PERIODIC_MANA_LEECH:
    case SPELL_AURA_PERIODIC_DAMAGE_PERCENT:
    case SPELL_AURA_POWER_BURN:
    case SPELL_AURA_PERIODIC_DUMMY:
    case SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE:
#ifdef LICH_KING
    case SPELL_AURA_MOD_ATTACK_POWER_OF_ARMOR:
#endif
        m_isPeriodic = true;
        break;
    //sun: these spells have 0 as amplitude (on BC at least)
    case SPELL_AURA_MOD_REGEN:
    case SPELL_AURA_MOD_MANA_REGEN_FROM_STAT:
    case SPELL_AURA_MOD_POWER_REGEN:
        m_isPeriodic = true;
        _amplitude = 5000;
        break;
    case SPELL_AURA_OBS_MOD_POWER: //sun: some spells have 0, such as 25990
        m_isPeriodic = true;
        if(_amplitude == 0)
            _amplitude = 1000;
    default:
        break;
    }

    //GetBase()->CallScriptEffectCalcPeriodicHandlers(this, m_isPeriodic, _amplitude);

    if (!m_isPeriodic)
        return;

    Player* modOwner = caster ? caster->GetSpellModOwner() : nullptr;
    // Apply casting time mods
    if (_amplitude)
    {
        // Apply periodic time mod
        if (modOwner)
            modOwner->ApplySpellMod(GetId(), SPELLMOD_ACTIVATION_TIME, _amplitude);

        if (caster)
        {
            // Haste modifies periodic time of channeled spells
            if (m_spellInfo->IsChanneled())
                caster->ModSpellDurationTime(m_spellInfo, _amplitude);
#ifdef LICH_KING
            // and periodic time of auras affected by SPELL_AURA_PERIODIC_HASTE
            else if (caster->HasAuraTypeWithAffectMask(SPELL_AURA_PERIODIC_HASTE, m_spellInfo) || m_spellInfo->HasAttribute(SPELL_ATTR5_HASTE_AFFECT_DURATION))
                _amplitude = int32(_amplitude * caster->GetFloatValue(UNIT_MOD_CAST_SPEED));
#endif
        }
    }
    else // prevent infinite loop on Update
        m_isPeriodic = false;

    if (load) // aura loaded from db
    {
        if (_amplitude && !GetBase()->IsPermanent())
        {
            uint32 elapsedTime = GetBase()->GetMaxDuration() - GetBase()->GetDuration();
            _ticksDone = elapsedTime / uint32(_amplitude);
            _periodicTimer = elapsedTime % uint32(_amplitude);
        }

        if (m_spellInfo->HasAttribute(SPELL_ATTR5_START_PERIODIC_AT_APPLY))
            ++_ticksDone;
    }
    else // aura just created or reapplied
    {
        // reset periodic timer on aura create or reapply
        // we don't reset periodic timers when aura is triggered by proc
        ResetPeriodic(resetPeriodicTimer);
    }
}

bool AuraEffect::IsAffectedOnSpell(SpellInfo const* spell) const
{
    if (!spell)
        return false;

    // Check family name and EffectClassMask (sun: SpellClassMask does not exists on BC but we fill this fake field with spell_affect table)
    uint64 familyClassMask = m_spellInfo->Effects[m_effIndex].SpellClassMask;
#ifndef LICH_KING
    //sun: BC is using this field as non existent SpellClassMask on BC?
    if (!familyClassMask)
        familyClassMask = m_spellInfo->Effects[m_effIndex].ItemType; 
#endif
    if (!spell->IsAffected(m_spellInfo->SpellFamilyName, familyClassMask))
        return false;

    return true;
}

void AuraEffect::HandlePeriodicDamageAurasTick(Unit* target, Unit* caster, uint32 &realDamage)
{
    if (!target->IsAlive())
        return;

    if (!caster)
        return;

    if (GetSpellInfo()->Effects[GetEffIndex()].Effect == SPELL_EFFECT_PERSISTENT_AREA_AURA &&
        caster->SpellHitResult(target, GetSpellInfo(), false) != SPELL_MISS_NONE)
        return;

    if (target->GetEntry() != 25653 || GetId() != 45848)
    {
        // Check for immune (not use charges)
        if (target->HasUnitState(UNIT_STATE_ISOLATED) || target->IsImmunedToDamage(GetSpellInfo()))
        {
            SendTickImmune(target, caster);
            return;
        }
    }

    // some auras remove at specific health level or more
    if (GetAuraType() == SPELL_AURA_PERIODIC_DAMAGE)
    {
        switch (GetId())
        {
        case 43093: case 31956: case 38801:
        case 35321: case 38363: case 39215:
            if (target->GetHealth() == target->GetMaxHealth())
            {
                target->RemoveAurasDueToSpell(GetId());
                return;
            }
            break;
        case 38772:
        {
            uint32 percent =
                GetEffIndex() < 2 && GetSpellInfo()->Effects[GetEffIndex()].Effect == SPELL_EFFECT_DUMMY ?
                caster->CalculateSpellDamage(target, GetSpellInfo(), GetEffIndex() + 1, &GetSpellInfo()->Effects[GetEffIndex() + 1].BasePoints) :
                100;
            if (target->GetHealth() * 100 >= target->GetMaxHealth()*percent)
            {
                target->RemoveAurasDueToSpell(GetId());
                return;
            }
            break;
        }
        case 41337:// aura of anger
        {
            Unit::AuraEffectList const& mMod = target->GetAuraEffectsByType(SPELL_AURA_MOD_DAMAGE_PERCENT_DONE);
            for (auto i : mMod)
            {
                if (i->GetId() == 41337)
                {
                    i->SetAmount(GetAmount() + 5);
                    break;
                }
            }
            SetAmount(100 * GetTickNumber());
            break;
        }
        case 41351:
            if (target && target->ToCreature() && target->ToCreature()->IsBelowHPPercent(50.0f))
                target->RemoveAurasDueToSpell(41351);

            break;
        default:
            break;
        }
    }

    CleanDamage cleanDamage = CleanDamage(0, BASE_ATTACK, MELEE_HIT_NORMAL);

    // ignore negative values (can be result apply spellmods to aura damage
    uint32 damage = std::max(GetAmount(), 0);

    // Script Hook For HandlePeriodicDamageAurasTick -- Allow scripts to change the Damage pre class mitigation calculations
    //sScriptMgr->ModifyPeriodicDamageAurasTick(target, caster, damage);

    if (GetAuraType() == SPELL_AURA_PERIODIC_DAMAGE)
    {
        // leave only target depending bonuses, rest is handled in calculate amount
        if (GetBase()->GetType() == DYNOBJ_AURA_TYPE)
            damage = caster->SpellDamageBonusDone(target, GetSpellInfo(), damage, DOT, GetEffIndex(), {}, GetBase()->GetStackAmount());

        // Curse of Agony damage-per-tick calculation
        if (GetSpellInfo()->SpellFamilyName == SPELLFAMILY_WARLOCK && (GetSpellInfo()->SpellFamilyFlags & 0x0000000000000400LL) && GetSpellInfo()->SpellIconID == 544)
        {
            uint32 totalTick = GetTotalTicks();
            // 1..4 ticks, 1/2 from normal tick damage
            if (GetTickNumber() <= totalTick / 3)
               damage = damage / 2;
            // 9..12 ticks, 3/2 from normal tick damage
            else if (GetTickNumber() > totalTick * 2 / 3)
                damage += (damage + 1) / 2;           // +1 prevent 0.5 damage possible lost at 1..4 ticks
                                                        // 5..8 ticks have normal tick damage
        }
        // Illidan Agonizing Flames
        else if (GetSpellInfo()->Id == 40932)
        {
            // 1200 - 1200 - 1200 - 2400 - 2400 - 2400 - 3600 - 3600 - 3600 - 4800 - 4800 - 4800
            uint32 totalTick = GetTotalTicks();
            if (GetTickNumber() <= totalTick / 4)
                damage = damage * 2 / 5;
            else if (GetTickNumber() <= totalTick / 2)
                damage = damage * 4 / 5;
            else if (GetTickNumber() <= totalTick * 3 / 4)
                damage = damage * 6 / 5;
            else
                damage = damage * 8 / 5;
        }
        // Curse of Boundless Agony (Kalecgos/Sathrovarr)
        else if (GetSpellInfo()->Id == 45032 || GetSpellInfo()->Id == 45034)
        {
            uint32 exp = (GetTickNumber() - 1) / 5; // Integral division...!
            damage = 100 * (1 << exp);
            switch (exp)
            {
            case 0:
                target->CastSpell(target, 45083, true);
                break;
            case 1:
                target->CastSpell(target, 45084, true);
                break;
            default:
                target->CastSpell(target, 45085, true);
                break;
            }
        }
        // Burn (Brutallus)
        else if (GetSpellInfo()->Id == 46394)
        {
            damage += GetTickNumber() * 60 > 3600 ? 3600 : GetTickNumber() * 60;
        }
    }
    else // ceil obtained value, it may happen that 10 ticks for 10% damage may not kill owner
        damage = uint32(ceil(CalculatePct<float, float>(target->GetMaxHealth(), damage)));

    damage = target->SpellDamageBonusTaken(caster, GetSpellInfo(), damage, DOT);

    //As of 2.2 resilience reduces damage from DoT ticks as much as the chance to not be critically hit
    //Reduce dot damage from resilience for players
    if (target->GetTypeId() == TYPEID_PLAYER)
        damage -= (target->ToPlayer())->GetDotDamageReduction(damage);

    realDamage = damage;

    DamageInfo damageInfo(caster, target, damage, GetSpellInfo(), GetSpellInfo()->GetSchoolMask(), DOT, BASE_ATTACK);
    Unit::CalcAbsorbResist(damageInfo);
    damage = damageInfo.GetDamage();

    uint32 absorb = damageInfo.GetAbsorb();
    uint32 resist = damageInfo.GetResist();

    TC_LOG_DEBUG("spells", "PeriodicTick: %u (TypeId: %u) attacked %u (TypeId: %u) for %u dmg inflicted by %u abs is %u",
        GetCasterGUID().GetCounter(), GuidHigh2TypeId(GetCasterGUID().GetHigh()), target->GetGUID().GetCounter(), target->GetTypeId(), damage, GetId(), damageInfo.GetAbsorb());

    Unit::DealDamageMods(target, damage, &absorb);

    // Shadow Word: Death backfire damage hackfix
    if (GetId() == 32409 && caster->ToPlayer())
    {
        damage = caster->ToPlayer()->m_swdBackfireDmg;
        caster->ToPlayer()->m_swdBackfireDmg = 0;
        absorb = 0;
        resist = 0;
    }

    SpellPeriodicAuraLogInfo pInfo(this, damage, absorb, resist, 0.0f);
    target->SendPeriodicAuraLog(&pInfo);

    // Set trigger flag
    uint32 procAttacker = PROC_FLAG_DONE_PERIODIC;
    uint32 procVictim = PROC_FLAG_TAKEN_PERIODIC;
    uint32 hitMask = damageInfo.GetHitMask();
    if (damage)
    {
#ifdef LICH_KING
        hitMask |= crit ? PROC_HIT_CRITICAL : PROC_HIT_NORMAL;
#else
        hitMask |= PROC_HIT_NORMAL;
#endif
        procVictim |= PROC_FLAG_TAKEN_DAMAGE;
    }

    Unit::DealDamage(caster, target, damage, &cleanDamage, DOT, GetSpellInfo()->GetSchoolMask(), GetSpellInfo(), true);
    Unit::ProcSkillsAndAuras(caster, target, procAttacker, procVictim, PROC_SPELL_TYPE_DAMAGE, PROC_SPELL_PHASE_NONE, hitMask, nullptr, &damageInfo, nullptr);
}

void AuraEffect::HandlePeriodicHealthLeechAuraTick(Unit* m_target, Unit* caster, uint32 &realDamage) const
{
    if (!m_target->IsAlive())
        return;

    // dynobj auras must always have a caster
    if (GetSpellInfo()->Effects[GetEffIndex()].Effect == SPELL_EFFECT_PERSISTENT_AREA_AURA &&
        ASSERT_NOTNULL(caster)->SpellHitResult(m_target, GetSpellInfo(), false) != SPELL_MISS_NONE)
        return;

    // Check for immune
    if (m_target->HasUnitState(UNIT_STATE_ISOLATED) || m_target->IsImmunedToDamage(GetSpellInfo()))
    {
        SendTickImmune(m_target, caster);
        return;
    }

    CleanDamage cleanDamage = CleanDamage(0, BASE_ATTACK, MELEE_HIT_NORMAL);

    uint32 pdamage = std::max(GetAmount(), 0);

    // Script Hook For HandlePeriodicDamageAurasTick -- Allow scripts to change the Damage pre class mitigation calculations
    //sScriptMgr->ModifyPeriodicDamageAurasTick(target, caster, pdamage);

    if (GetBase()->GetType() == DYNOBJ_AURA_TYPE)
        pdamage = ASSERT_NOTNULL(caster)->SpellDamageBonusDone(m_target, GetSpellInfo(), pdamage, DOT, GetEffIndex());
    pdamage = m_target->SpellDamageBonusTaken(caster, GetSpellInfo(), pdamage, DOT);

    // talent Soul Siphon add bonus to Drain Life spells
    if (caster && GetSpellInfo()->SpellFamilyName == SPELLFAMILY_WARLOCK && (GetSpellInfo()->SpellFamilyFlags & 0x8))
    {
        // find talent max bonus percentage
        Unit::AuraEffectList const& mClassScriptAuras = caster->GetAuraEffectsByType(SPELL_AURA_OVERRIDE_CLASS_SCRIPTS);
        for (auto mClassScriptAura : mClassScriptAuras)
        {
            if (mClassScriptAura->GetMiscValue() == 4992 || mClassScriptAura->GetMiscValue() == 4993)
            {
                if (mClassScriptAura->GetEffIndex() != 1)
                {
                    TC_LOG_ERROR("FIXME", "Expected spell %u structure change, need code update", mClassScriptAura->GetId());
                    break;
                }

                // effect 1 m_amount
                int32 maxPercent = mClassScriptAura->GetAmount();
                // effect 0 m_amount
                int32 stepPercent = caster->CalculateSpellDamage(caster, mClassScriptAura->GetSpellInfo(), 0, &mClassScriptAura->GetSpellInfo()->Effects[0].BasePoints);

                // count affliction effects and calc additional damage in percentage
                int32 modPercent = 0;
                auto& victimAuras = m_target->GetAppliedAuras();
                for (const auto & victimAura : victimAuras)
                {
                    Aura* aura = victimAura.second->GetBase();
                    if (victimAura.second->IsPositive())continue;
                    SpellInfo const* m_spell = aura->GetSpellInfo();
                    if (m_spell->SpellFamilyName != SPELLFAMILY_WARLOCK)
                        continue;

                    SkillLineAbilityMapBounds skill_bounds = sSpellMgr->GetSkillLineAbilityMapBounds(m_spell->Id);
                    for (auto _spell_idx = skill_bounds.first; _spell_idx != skill_bounds.second; ++_spell_idx)
                    {
                        if (_spell_idx->second->skillId == SKILL_AFFLICTION)
                        {
                            modPercent += stepPercent;
                            if (modPercent >= maxPercent)
                            {
                                modPercent = maxPercent;
                                break;
                            }
                        }
                    }
                }
                pdamage += (pdamage*modPercent / 100);
                break;
            }
        }
    }

    //As of 2.2 resilience reduces damage from DoT ticks as much as the chance to not be critically hit
    // Reduce dot damage from resilience for players
    if (!GetSpellInfo()->HasAttribute(SPELL_ATTR4_FIXED_DAMAGE) && m_target->GetTypeId() == TYPEID_PLAYER)
        pdamage -= (m_target->ToPlayer())->GetDotDamageReduction(pdamage);

    DamageInfo damageInfo(caster, m_target, pdamage, GetSpellInfo(), GetSpellInfo()->GetSchoolMask(), DOT, BASE_ATTACK);
    Unit::CalcAbsorbResist(damageInfo);

    uint32 absorb = damageInfo.GetAbsorb();
    uint32 resist = damageInfo.GetResist();

    if (m_target->GetHealth() < pdamage)
        pdamage = uint32(m_target->GetHealth());

    TC_LOG_DEBUG("spells", "PeriodicTick: %u (TypeId: %u) health leech of %u (TypeId: %u) for %u dmg inflicted by %u abs is %u",
        GetCasterGUID().GetCounter(), GuidHigh2TypeId(GetCasterGUID().GetHigh()), m_target->GetGUID().GetCounter(), m_target->GetTypeId(), pdamage, GetId(), absorb);

    // SendSpellNonMeleeDamageLog expects non-absorbed/non-resisted damage
    if (caster)
        caster->SendSpellNonMeleeDamageLog(m_target, GetId(), pdamage, GetSpellInfo()->GetSchoolMask(), absorb, resist, false, 0);

    Unit* target = m_target;                        // aura can be deleted in DealDamage
    SpellInfo const* spellProto = GetSpellInfo();
    float  multiplier = spellProto->Effects[GetEffIndex()].CalcValueMultiplier(caster);

    realDamage = pdamage;

    // Set trigger flag
    uint32 procAttacker = PROC_FLAG_DONE_PERIODIC;
    uint32 procVictim = PROC_FLAG_TAKEN_PERIODIC;
    uint32 hitMask = damageInfo.GetHitMask();
    if (pdamage)
    {
#ifdef LICH_KING
        hitMask |= crit ? PROC_HIT_CRITICAL : PROC_HIT_NORMAL;
#else
        hitMask |= PROC_HIT_NORMAL;
#endif
        procVictim |= PROC_FLAG_TAKEN_DAMAGE;
    }

    int32 new_damage = Unit::DealDamage(caster, target, pdamage, &cleanDamage, DOT, spellProto->GetSchoolMask(), spellProto, false);
    Unit::ProcSkillsAndAuras(caster, target, procAttacker, procVictim, PROC_SPELL_TYPE_DAMAGE, PROC_SPELL_PHASE_NONE, hitMask, nullptr, &damageInfo, nullptr);

    // process caster heal from now on (must be in world)
    if (!caster || !caster->IsAlive())
        return;

    if (!target->IsAlive() && caster->IsNonMeleeSpellCast(false))
    {
        for (uint32 i = CURRENT_FIRST_NON_MELEE_SPELL; i < CURRENT_MAX_SPELL; i++)
        {
            if (caster->m_currentSpells[i] && caster->m_currentSpells[i]->m_spellInfo->Id == spellProto->Id)
                caster->m_currentSpells[i]->cancel();
        }
    }

    uint32 heal = caster->SpellHealingBonusDone(caster, spellProto, uint32(new_damage * multiplier), DOT, GetEffIndex(), { }, GetBase()->GetStackAmount());
    heal = caster->SpellHealingBonusTaken(caster, GetSpellInfo(), heal, DOT);

    HealInfo healInfo(caster, caster, heal, GetSpellInfo(), GetSpellInfo()->GetSchoolMask());
    caster->HealBySpell(healInfo, false);

    caster->GetThreatManager().ForwardThreatForAssistingMe(caster, healInfo.GetEffectiveHeal() * 0.5f, spellProto);
    Unit::ProcSkillsAndAuras(caster, caster, PROC_FLAG_DONE_PERIODIC, PROC_FLAG_TAKEN_PERIODIC, PROC_SPELL_TYPE_HEAL, PROC_SPELL_PHASE_NONE, hitMask, nullptr, nullptr, &healInfo);
}

void AuraEffect::HandlePeriodicHealthFunnelAuraTick(Unit* target, Unit* caster, uint32 &realDamage) const
{
    //NYI
}

void AuraEffect::HandlePeriodicHealAurasTick(Unit* target, Unit* caster, uint32 &realDamage) const
{
    if (!target->IsAlive())
        return;

    if (!caster)
        return;

    // heal for caster damage (must be alive)
    if (target != caster && GetSpellInfo()->HasVisual(163) && !caster->IsAlive()) //Health Funnel
        return;

    // ignore negative values (can be result apply spellmods to aura damage
    uint32 damage = std::max(GetAmount(), 0);

    // Script Hook For HandlePeriodicHealAurasTick -- Allow scripts to change the Damage pre class mitigation calculations
    //sScriptMgr->ModifyPeriodicDamageAurasTick(target, caster, damage);

    if (GetAuraType() == SPELL_AURA_OBS_MOD_HEALTH)
        damage = uint32(target->CountPctFromMaxHealth(int32(damage)));
    else
    {
        // dynobj auras must always have a caster
        if (GetBase()->GetType() == DYNOBJ_AURA_TYPE)
            damage = ASSERT_NOTNULL(caster)->SpellHealingBonusDone(target, GetSpellInfo(), damage, DOT, GetEffIndex(), {}, GetBase()->GetStackAmount());
    }

    damage = target->SpellHealingBonusTaken(caster, GetSpellInfo(), damage, DOT);

    TC_LOG_DEBUG("spells", "PeriodicTick: %u (TypeId: %u) heal of %u (TypeId: %u) for %u health inflicted by %u",
        GetCasterGUID().GetCounter(), GuidHigh2TypeId(GetCasterGUID().GetHigh()), target->GetGUID().GetCounter(), target->GetTypeId(), damage, GetId());

    uint32 heal = damage;
    realDamage = heal;

    HealInfo healInfo(caster, target, heal, GetSpellInfo(), GetSpellInfo()->GetSchoolMask());
    Unit::DealHeal(healInfo);

    SpellPeriodicAuraLogInfo pInfo(this, heal, 0, 0, 0.0f);
    target->SendPeriodicAuraLog(&pInfo);

    if (caster)
        target->GetThreatManager().ForwardThreatForAssistingMe(caster, healInfo.GetEffectiveHeal() * 0.5f, GetSpellInfo());

    bool haveCastItem = GetBase()->GetCastItemGUID() != 0;

    // heal for caster damage
    if (target != caster && GetSpellInfo()->HasVisual(163)) //Health Funnel
    {
        uint32 dmg = GetSpellInfo()->ManaPerSecond;
        if (caster->GetHealth() <= dmg && caster->GetTypeId() == TYPEID_PLAYER)
        {
            caster->RemoveAurasDueToSpell(GetId());

            // finish current generic/channeling spells, don't affect autorepeat
            if (caster->m_currentSpells[CURRENT_GENERIC_SPELL])
            {
                caster->m_currentSpells[CURRENT_GENERIC_SPELL]->finish();
            }
            if (caster->m_currentSpells[CURRENT_CHANNELED_SPELL])
            {
                caster->m_currentSpells[CURRENT_CHANNELED_SPELL]->SendChannelUpdate(0);
                caster->m_currentSpells[CURRENT_CHANNELED_SPELL]->finish();
            }
        }
        else
        {
            caster->SendSpellNonMeleeDamageLog(caster, GetId(), healInfo.GetEffectiveHeal(), GetSpellInfo()->GetSchoolMask(), 0, 0, false, 0, false);

            CleanDamage cleanDamage = CleanDamage(0, BASE_ATTACK, MELEE_HIT_NORMAL);
            Unit::DealDamage(caster, caster, healInfo.GetEffectiveHeal(), &cleanDamage, NODAMAGE, GetSpellInfo()->GetSchoolMask(), GetSpellInfo(), true);
        }
    }

    // %-based heal - does not proc auras
    if (GetAuraType() == SPELL_AURA_OBS_MOD_HEALTH)
        return;

    uint32 procAttacker = PROC_FLAG_DONE_PERIODIC;
    uint32 procVictim = PROC_FLAG_TAKEN_PERIODIC;
#ifdef LICH_KING
    uint32 hitMask = crit ? PROC_HIT_CRITICAL : PROC_HIT_NORMAL;
#else
    uint32 hitMask = PROC_HIT_NORMAL;
#endif

    // ignore item heals
    if (!haveCastItem)
        Unit::ProcSkillsAndAuras(caster, target, procAttacker, procVictim, PROC_SPELL_TYPE_HEAL, PROC_SPELL_PHASE_NONE, hitMask, nullptr, nullptr, &healInfo);
}

void AuraEffect::HandlePeriodicManaLeechAuraTick(Unit* m_target, Unit* caster, uint32 &realDamage) const
{
    Powers powerType = Powers(GetMiscValue());

    if (!caster || !caster->IsAlive() || !m_target->IsAlive() || m_target->GetPowerType() != powerType)
        return;

    if (m_target->HasUnitState(UNIT_STATE_ISOLATED) || m_target->IsImmunedToDamage(GetSpellInfo()))
    {
        SendTickImmune(m_target, caster);
        return;
    }

    if (GetSpellInfo()->Effects[GetEffIndex()].Effect == SPELL_EFFECT_PERSISTENT_AREA_AURA &&
        caster->SpellHitResult(m_target, GetSpellInfo(), false) != SPELL_MISS_NONE)
        return;

    // ignore non positive values (can be result apply spellmods to aura damage
    uint32 pdamage = GetAmount() > 0 ? GetAmount() : 0;

    TC_LOG_DEBUG("spells", "PeriodicTick: %u (TypeId: %u) power leech of %u (TypeId: %u) for %u dmg inflicted by %u",
        GetCasterGUID().GetCounter(), GuidHigh2TypeId(GetCasterGUID().GetHigh()), m_target->GetGUID().GetCounter(), m_target->GetTypeId(), pdamage, GetId());

    Powers power = Powers(GetMiscValue());

    int32 drain_amount = m_target->GetPower(power) > pdamage ? pdamage : m_target->GetPower(power);

    // resilience reduce mana draining effect at spell crit damage reduction (added in 2.4)
    if (power == POWER_MANA && m_target->GetTypeId() == TYPEID_PLAYER)
        drain_amount -= (m_target->ToPlayer())->GetSpellCritDamageReduction(drain_amount);

    m_target->ModifyPower(power, -drain_amount);
    realDamage = pdamage;

    float gain_multiplier = 0;

    if (caster->GetMaxPower(power) > 0)
    {
        gain_multiplier *= GetSpellInfo()->Effects[GetEffIndex()].CalcValueMultiplier(caster);
    }

    SpellPeriodicAuraLogInfo pInfo(this, drain_amount, 0, 0, gain_multiplier);
    m_target->SendPeriodicAuraLog(&pInfo);

    int32 gain_amount = int32(drain_amount*gain_multiplier);

    if (gain_amount)
    {
        int32 gain = caster->ModifyPower(power, gain_amount);
        m_target->GetThreatManager().AddThreat(caster, float(gain) * 0.5f, GetSpellInfo());
    }

    //no procs?

    // Mark of Kaz'rogal
    if (GetId() == 31447 && m_target->GetPower(power) == 0)
    {
        m_target->CastSpell(m_target, 31463, this);
        m_target->RemoveAura(GetId());
    }

    // Mark of Kazzak
    if (GetId() == 32960)
    {
        int32 modifier = (m_target->GetPower(power) * 0.05f);
        m_target->ModifyPower(power, -modifier);

        if (m_target->GetPower(power) == 0)
        {
            m_target->CastSpell(m_target, 32961, this);
            // Remove aura
            m_target->RemoveAura(GetId());
        }
    }

    m_target->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_TAKE_DAMAGE, m_spellInfo ? m_spellInfo->Id : 0);
}

void AuraEffect::HandleObsModPowerAuraTick(Unit* target, Unit* caster, uint32 &realDamage) const
{
    Powers powerType;
#ifdef LICH_KING
    //no such spells on BC
    if (GetMiscValue() == POWER_ALL)
        powerType = target->GetPowerType();
    else
#endif
        powerType = Powers(GetMiscValue());

    if (!target->IsAlive() || !target->GetMaxPower(powerType))
        return;

    if (target->HasUnitState(UNIT_STATE_ISOLATED))
    {
        SendTickImmune(target, caster);
        return;
    }

    // ignore non positive values (can be result apply spellmods to aura damage
    uint32 mod = GetAmount() > 0 ? GetAmount() : 0;

    uint32 amount = uint32(target->GetMaxPower(powerType) * mod / 100);

    TC_LOG_DEBUG("spells", "PeriodicTick: %u (TypeId: %u) energize %u (TypeId: %u) for %u mana inflicted by %u",
        GetCasterGUID().GetCounter(), GuidHigh2TypeId(GetCasterGUID().GetHigh()), target->GetGUID().GetCounter(), target->GetTypeId(), amount, GetId());

    SpellPeriodicAuraLogInfo pInfo(this, amount, 0, 0, 0.0f);
    target->SendPeriodicAuraLog(&pInfo);

    realDamage = amount;

    int32 gain = target->ModifyPower(powerType, amount);

    if (caster)
        target->GetThreatManager().ForwardThreatForAssistingMe(caster, float(gain) * 0.5f, GetSpellInfo());
}

void AuraEffect::HandlePeriodicEnergizeAuraTick(Unit* target, Unit* caster, uint32 &realDamage) const
{
    Powers powerType = Powers(GetMiscValue());

    if (target->GetTypeId() == TYPEID_PLAYER && target->GetPowerType() != powerType
#ifdef LICH_KING
        && !m_spellInfo->HasAttribute(SPELL_ATTR7_CAN_RESTORE_SECONDARY_POWER)
#endif
        )
        return;

    if (target->HasUnitState(UNIT_STATE_ISOLATED))
    {
        SendTickImmune(target, caster);
        return;
    }

    // don't regen when permanent aura target has full power
    if (GetBase()->IsPermanent() && target->GetPower(powerType) == target->GetMaxPower(powerType))
        return;

    // ignore negative values (can be result apply spellmods to aura damage)
    int32 amount = std::max(GetAmount(), 0);

    TC_LOG_DEBUG("spells", "PeriodicTick: %u (TypeId: %u) energize %u (TypeId: %u) for %u dmg inflicted by %u",
        GetCasterGUID().GetCounter(), GuidHigh2TypeId(GetCasterGUID().GetHigh()), target->GetGUID().GetCounter(), target->GetTypeId(), amount, GetId());

    SpellPeriodicAuraLogInfo pInfo(this, amount, 0, 0, 0.0f);
    target->SendPeriodicAuraLog(&pInfo);

    int32 gain = target->ModifyPower(powerType, amount);
    realDamage = amount;

    if (caster)
        target->GetThreatManager().ForwardThreatForAssistingMe(caster, float(gain) * 0.5f, GetSpellInfo(), true); //to confirm : threat from energize spells is not subject to threat modifiers
}

void AuraEffect::HandlePeriodicPowerBurnAuraTick(Unit* target, Unit* caster, uint32 &realDamage) const
{
    Powers powerType = Powers(GetMiscValue());

    if (!caster || !target->IsAlive() || target->GetPowerType() != powerType)
        return;

    // Check for immune
    if (target->HasUnitState(UNIT_STATE_ISOLATED) || target->IsImmunedToDamage(GetSpellInfo()))
    {
        SendTickImmune(target, caster);
        return;
    }

    // ignore negative values (can be result apply spellmods to aura damage
    int32 pdamage = std::max(GetAmount(), 0);

    // resilience reduce mana draining effect at spell crit damage reduction (added in 2.4)
    if (powerType == POWER_MANA && target->GetTypeId() == TYPEID_PLAYER)
        pdamage -= (target->ToPlayer())->GetSpellCritDamageReduction(pdamage);

    uint32 gain = uint32(-target->ModifyPower(powerType, -pdamage));
    gain = uint32(gain * GetSpellInfo()->Effects[GetEffIndex()].CalcValueMultiplier(caster));
    realDamage = gain;

    SpellInfo const* spellProto = GetSpellInfo();
    //maybe has to be sent different to client, but not by SMSG_PERIODICAURALOG
    SpellNonMeleeDamage damageInfo(caster, target, spellProto->Id, spellProto->SchoolMask);
    //no SpellDamageBonusDone for burn mana
    caster->CalculateSpellDamageTaken(&damageInfo, gain, spellProto);

    Unit::DealDamageMods(damageInfo.target, damageInfo.damage, &damageInfo.absorb);

    caster->SendSpellNonMeleeDamageLog(&damageInfo);

    // Set trigger flag
    uint32 procAttacker = PROC_FLAG_DONE_PERIODIC;
    uint32 procVictim = PROC_FLAG_TAKEN_PERIODIC;
    uint32 hitMask = createProcHitMask(&damageInfo, SPELL_MISS_NONE);
    uint32 spellTypeMask = PROC_SPELL_TYPE_NO_DMG_HEAL;
    if (damageInfo.damage)
    {
        procVictim |= PROC_FLAG_TAKEN_DAMAGE;
        spellTypeMask |= PROC_SPELL_TYPE_DAMAGE;
    }

    caster->DealSpellDamage(&damageInfo, true);

    DamageInfo dotDamageInfo(damageInfo, DOT, BASE_ATTACK, hitMask);
    Unit::ProcSkillsAndAuras(caster, target, procAttacker, procVictim, spellTypeMask, PROC_SPELL_PHASE_NONE, hitMask, nullptr, &dotDamageInfo, nullptr);
}

void AuraEffect::HandlePeriodicTriggerSpellAuraTick(Unit* target, Unit* caster, uint32 &realDamage)
{
    uint32 triggerSpellId = GetSpellInfo()->Effects[GetEffIndex()].TriggerSpell;
    SpellInfo const* triggeredSpellInfo = sSpellMgr->GetSpellInfo(triggerSpellId);
    //Unit* triggerCaster = triggeredSpellInfo ? (triggeredSpellInfo->NeedsToBeTriggeredByCaster(m_spellInfo) ? caster : target) : caster;
    Unit* triggerCaster = caster; //sun: why should be use target as caster? this breaks spell such as Spore Cloud (34168). Experimently disabled previous line
    ObjectGuid originalCasterGUID = GetCasterGUID();
    //old hacks time
    {
        if (triggeredSpellInfo)
        {
            // Spell exist but require custom code
            // (Some of those did have serverside spells we didn't have at the time, but we need to check each one to remove the hacks here)
            if (triggerCaster)
            {
                switch (m_spellInfo->Id)
                {
                // Lightning Shield (The Earthshatterer set trigger after cast Lighting Shield)
                case 28820:
                {
                    // Need remove self if Lightning Shield not active
                    auto& auras = target->GetAppliedAuras();
                    for (auto const& aura : auras)
                    {
                        SpellInfo const* spell = aura.second->GetBase()->GetSpellInfo();
                        if (spell->SpellFamilyName == SPELLFAMILY_SHAMAN &&
                            spell->SpellFamilyFlags & 0x0000000000000400L) //lighning shield
                            return;
                    }
                    target->RemoveAurasDueToSpell(28820);
                    return;
                }
                // Totemic Mastery (Skyshatter Regalia (Shaman Tier 6) - bonus)
                case 38443:
                {
                    if (!caster)
                        return;

                    bool all = true;
                    for (uint64 i : caster->m_SummonSlot)
                    {
                        if (!i)
                        {
                            all = false;
                            break;
                        }
                    }

                    if (all)
                        caster->CastSpell(caster, 38437, true);
                    else
                        caster->RemoveAurasDueToSpell(38437);
                    return;
                }

                // Cat Form
                // triggered_spell_id not set and unknown effect triggered in this case, ignoring for while
                case 768:
                    return;
                    // Frenzied Regeneration
                case 22842:
                case 22895:
                case 22896:
                case 26999:
                {
                    int32 LifePerRage = GetAmount();

                    int32 lRage = target->GetPower(POWER_RAGE);
                    if (lRage > 100)                                     // rage stored as rage*10
                        lRage = 100;
                    target->ModifyPower(POWER_RAGE, -lRage);

                    int32 FRTriggerBasePoints = int32(lRage*LifePerRage / 10);
                    CastSpellExtraArgs args;
                    args.TriggerFlags = TRIGGERED_FULL_MASK;
                    args.AddSpellBP0(FRTriggerBasePoints);
                    args.SetTriggeringAura(this);
                    target->CastSpell(target, 22845, args);
                    return;
                }
                // Doom
                case 31347:
                {
                    target->CastSpell(target, 31350, TRIGGERED_FULL_MASK);
                    Unit::DealDamage(target, target, target->GetHealth(), nullptr, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, nullptr, false);
                    return;
                }
                // Spellcloth
                case 31373:
                {
                    if (!caster)
                        return;
                    // Summon Elemental after create item
                    caster->SummonCreature(17870, 0, 0, 0, caster->GetOrientation(), TEMPSUMMON_DEAD_DESPAWN, 0);
                    return;
                }
                // Flame Quills
                case 34229:
                {
                    if (!caster)
                        return;
                    // cast 24 spells 34269-34289, 34314-34316
                    for (uint32 spell_id = 34269; spell_id != 34290; ++spell_id)
                        caster->CastSpell(target, spell_id, TRIGGERED_FULL_MASK);
                    for (uint32 spell_id = 34314; spell_id != 34317; ++spell_id)
                        caster->CastSpell(target, spell_id, TRIGGERED_FULL_MASK);
                    return;
                }
                // Eye of Grillok
                case 38495:
                {
                    target->CastSpell(target, 38530, TRIGGERED_FULL_MASK);
                    return;
                }
                // Absorb Eye of Grillok (Zezzak's Shard)
                case 38554:
                {
                    if (!caster)
                        return;

                    if (target->GetTypeId() != TYPEID_UNIT)
                        return;

                    caster->CastSpell(caster, 38495, TRIGGERED_FULL_MASK);

                    Creature* creatureTarget = target->ToCreature();

                    creatureTarget->SetDeathState(JUST_DIED);
                    creatureTarget->RemoveCorpse();
                    creatureTarget->SetHealth(0);       // just for nice GM-mode view
                    return;
                }
                // Inferno
                case 39346:
                {
                    CastSpellExtraArgs args;
                    args.TriggerFlags = TRIGGERED_FULL_MASK;
                    args.AddSpellBP0(int32(GetAmount()));
                    args.SetTriggeringAura(this);
                    target->CastSpell(target, 31969, args);

                    return;
                }
                // Shadow Inferno
                case 39645:
                {
                    CastSpellExtraArgs args;
                    args.TriggerFlags = TRIGGERED_FULL_MASK;
                    args.AddSpellBP0(int32(GetAmount()));
                    args.SetTriggeringAura(this);
                    target->CastSpell(target, 39646, args);
                    return;
                }
                // Tear of Azzinoth Summon Channel - it's not really supposed to do anything,and this only prevents the console spam
                case 39857: triggerSpellId = 39856; break;
                    // Prismatic Shield
                case 40879:
                {
                    switch (rand() % 6)
                    {
                    case 0: triggerSpellId = 40880; break;
                    case 1: triggerSpellId = 40882; break;
                    case 2: triggerSpellId = 40883; break;
                    case 3: triggerSpellId = 40891; break;
                    case 4: triggerSpellId = 40896; break;
                    case 5: triggerSpellId = 40897; break;
                    }
                }break;
                // Aura of Desire
                case 41350:
                {
                    Unit::AuraEffectList const& mMod = target->GetAuraEffectsByType(SPELL_AURA_MOD_INCREASE_ENERGY_PERCENT);
                    for (auto i : mMod)
                    {
                        if (i->GetId() == 41350)
                        {
                            i->SetAmount(GetAmount() - 5);
                            break;
                        }
                    }
                }break;
                //                    // Dementia
                case 41404:
                {
                    triggerSpellId = (rand() % 2) ? 41406 : 41409;
                }break;
                // Personalized Weather
                case 46736: triggerSpellId = 46737; break;
                case 29528: triggerSpellId = 28713; break;
                // Feed Captured Animal
                case 29917: triggerSpellId = 29916; break;
                // Extract Gas
                case 30427:
                {
                    if (!caster)
                        return;

                    // move loot to player inventory and despawn target
                    if (caster->GetTypeId() == TYPEID_PLAYER &&
                        target->GetTypeId() == TYPEID_UNIT &&
                        (target->ToCreature())->GetCreatureTemplate()->type == CREATURE_TYPE_GAS_CLOUD)
                    {
                        Player* player = caster->ToPlayer();
                        Creature* creature = target->ToCreature();
                        // missing lootid has been reported on startup - just return
                        if (!creature->GetCreatureTemplate()->SkinLootId)
                        {
                            return;
                        }
                        Loot *loot = &creature->loot;
                        loot->clear();
                        loot->FillLoot(creature->GetCreatureTemplate()->SkinLootId, LootTemplates_Skinning, player, true);
                        for (uint8 i = 0; i < loot->items.size(); i++)
                        {
                            LootItem *item = loot->LootItemInSlot(i, player);
                            ItemPosCountVec dest;
                            uint8 msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, item->itemid, item->count);
                            if (msg == EQUIP_ERR_OK)
                            {
                                Item * newitem = player->StoreNewItem(dest, item->itemid, true, item->randomPropertyId);

                                player->SendNewItem(newitem, uint32(item->count), false, false, true);
                            }
                            else
                                player->SendEquipError(msg, nullptr, nullptr);
                        }
                        creature->SetDeathState(JUST_DIED);
                        creature->RemoveCorpse();
                        creature->SetHealth(0);         // just for nice GM-mode view
                    }
                    return;
                    break;
                }
                // Nitrous Boost
                case 27746:
                {
                    if (target->GetPower(POWER_MANA) >= 10)
                    {
                        target->ModifyPower(POWER_MANA, -10);
                        // target->SendEnergizeSpellLog(target, (uint32) 27746, -10, POWER_MANA); // DEBUG, problem with -10
                    }
                    else
                    {
                        target->RemoveAurasDueToSpell(27746);
                        return;
                    }
                } break;
                // Restoration
                case 23493:
                {
                    if (!caster)
                        return;
                    int32 heal = target->GetMaxHealth() / 10;
                    HealInfo healInfo(target, target, heal, GetSpellInfo(), GetSpellInfo()->GetSchoolMask());
                    caster->HealBySpell(healInfo);

                    int32 mana = target->GetMaxPower(POWER_MANA);
                    if (mana)
                    {
                        mana /= 10;
                        target->ModifyPower(POWER_MANA, mana);
                        target->SendEnergizeSpellLog(target, 23493, mana, POWER_MANA);
                    }
                    break;
                }
                // Brood Affliction: Bronze
                case 23170:
                {
                    target->CastSpell(target, 23171, this);
                    return;
                }
                // Thaumaturgy Channel
                case 9712: triggerSpellId = 21029; break;
                    // Firestone Passive (1-5 ranks)
                case 758:
                case 17945:
                case 17947:
                case 17949:
                case 27252:
                {
                    if (!caster || caster->GetTypeId() != TYPEID_PLAYER)
                        return;
                    Item* item = (caster->ToPlayer())->GetWeaponForAttack(BASE_ATTACK);
                    if (!item)
                        return;
                    uint32 enchant_id = 0;
                    switch (GetId())
                    {
                    case   758: enchant_id = 1803; break;   // Rank 1
                    case 17945: enchant_id = 1823; break;   // Rank 2
                    case 17947: enchant_id = 1824; break;   // Rank 3
                    case 17949: enchant_id = 1825; break;   // Rank 4
                    case 27252: enchant_id = 2645; break;   // Rank 5
                    default:
                        return;
                    }
                    // remove old enchanting before applying new
                    (caster->ToPlayer())->ApplyEnchantment(item, TEMP_ENCHANTMENT_SLOT, false);
                    item->SetEnchantment(TEMP_ENCHANTMENT_SLOT, enchant_id, _amplitude + 1000, 0);
                    // add new enchanting
                    (caster->ToPlayer())->ApplyEnchantment(item, TEMP_ENCHANTMENT_SLOT, true);
                    return;
                }
                // Curse of Idiocy
                case 1010:
                {
                    // TODO: spell casted by result in correct way mostly
                    // BUT:
                    // 1) target show casting at each triggered cast: target don't must show casting animation for any triggered spell
                    //      but must show affect apply like item casting
                    // 2) maybe aura must be replace by new with accumulative stat mods instead stacking

                    // prevent cast by triggered auras
                    if (caster == target)
                        return;

                    // stop triggering after each affected stats lost > 90
                    int32 intellectLoss = 0;
                    int32 spiritLoss = 0;

                    Unit::AuraEffectList const& mModStat = target->GetAuraEffectsByType(SPELL_AURA_MOD_STAT);
                    for (auto i : mModStat)
                    {
                        if (i->GetId() == 1010)
                        {
                            switch (i->GetMiscValue())
                            {
                            case STAT_INTELLECT: intellectLoss += i->GetAmount(); break;
                            case STAT_SPIRIT:    spiritLoss += i->GetAmount(); break;
                            default: break;
                            }
                        }
                    }

                    if (intellectLoss <= -90 && spiritLoss <= -90)
                        return;

                    triggerCaster = target;
                    originalCasterGUID.Clear();
                    break;
                }
                // Mana Tide
                case 16191:
                {
                    if (!caster)
                        return;
                    CastSpellExtraArgs args;
                    args.TriggerFlags = TRIGGERED_FULL_MASK;
                    args.AddSpellBP0(int32(GetAmount()));
                    args.SetTriggeringAura(this);
                    args.SetOriginalCaster(originalCasterGUID);
                    caster->CastSpell(target, triggerSpellId, args);
                    return;
                }
                // Warlord's Rage
                case 31543:
                {
                    return;
                }
                // Negative Energy Periodic - Muru
                case 46284:
                {
                    if (!caster)
                        return;
                    CastSpellExtraArgs args;
                    args.TriggerFlags = TRIGGERED_FULL_MASK;
                    args.AddSpellMod(SPELLVALUE_MAX_TARGETS, int32(GetTickNumber() / 15 + 1));
                    args.SetTriggeringAura(this);
                    args.SetOriginalCaster(originalCasterGUID);
                    caster->CastSpell(nullptr, triggerSpellId, args);
                    return;
                }
                case 46680:
                    if (!caster)
                        return;
                    if (caster->ToCreature())
                        if (caster->ToCreature()->AI())
                            if (Unit* victim = caster->ToCreature()->AI()->SelectTarget(SELECT_TARGET_RANDOM, 0))
                            {
                                CastSpellExtraArgs args;
                                args.TriggerFlags = TRIGGERED_FULL_MASK;
                                args.SetTriggeringAura(this);
                                args.SetOriginalCaster(originalCasterGUID);
                                target->CastSpell(victim, triggerSpellId, args);
                            }
                    return;
                case 45921:
                    if (!caster)
                        return;

                    if (caster->ToCreature())
                        if (caster->ToCreature()->AI())
                            if (Unit* victim = caster->ToCreature()->AI()->SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                            {
                                CastSpellExtraArgs args;
                                args.TriggerFlags = TRIGGERED_FULL_MASK;
                                args.SetTriggeringAura(this);
                                args.SetOriginalCaster(originalCasterGUID);
                                caster->CastSpell(victim, triggerSpellId, args);
                            }
                    break;
                }

                //TC triggerCaster->CastSpell(target, triggerSpellId, this);
                TC_LOG_DEBUG("spells", "AuraEffect::HandlePeriodicTriggerSpellAuraTick: Spell %u Trigger %u", GetId(), triggeredSpellInfo->Id);
            }
        }
        else
        {
            /* SELECT st.entry, st.spellName1, st.effectTriggerSpell1,  st2.entry
            FROM spell_template st
            LEFT JOIN spell_template st2 ON st.effectTriggerSpell1 = st2.entry
            WHERE st.effectApplyAuraName1 = 23
            AND st2.entry IS NULL
            */
            switch (GetSpellInfo()->SpellFamilyName)
            {
            case SPELLFAMILY_GENERIC:
            {
                switch (GetSpellInfo()->Id)
                {
                    // Quake
                case 30576: triggerSpellId = 30571; break;
                case 29768:
                {
                    CastSpellExtraArgs args;
                    args.TriggerFlags = TRIGGERED_FULL_MASK;
                    args.AddSpellBP0(int32(200 * GetTickNumber()));
                    args.SetTriggeringAura(this);
                    target->CastSpell(target, 29766, args);

                    return;
                }
                default:
                    break;
                }
                break;
            }
            default:
                TC_LOG_WARN("spells", "AuraEffect::HandlePeriodicTriggerSpellAuraTick: Spell %u has non-existent spell %u in EffectTriggered[%d] and is therefore not triggered.", GetId(), triggerSpellId, GetEffIndex());
                return;
            }
        }
    }

    SpellCastTargets targets;
    targets.SetUnitTarget(target);
    if (triggeredSpellInfo && triggeredSpellInfo->IsChannelCategorySpell() && GetBase()->GetChannelTargetData())
    {
        targets.SetDstChannel(GetBase()->GetChannelTargetData()->spellDst);
        targets.SetObjectTargetChannel(GetBase()->GetChannelTargetData()->channelGUID);
    }

    CastSpellExtraArgs args(this);
    args.SetOriginalCaster(originalCasterGUID);
#ifdef LICH_KING
    //don't know what to do with this on BC but this attribute was already existing. Auras can't crit so this attributes is probably not exact at TC.
    if (GetSpellInfo()->HasAttribute(SPELL_ATTR4_INHERIT_CRIT_FROM_AURA))
        args.AddSpellMod(SPELLVALUE_CRIT_CHANCE, int32(GetBase()->GetCritChance() * 100.0f)); // @todo: ugly x100 remove when basepoints are double
#endif

    triggerCaster->CastSpell(targets, triggerSpellId, args);
}

void AuraEffect::HandlePeriodicTriggerSpellWithValueAuraTick(Unit* target, Unit* caster, uint32 &realDamage) const
{
    uint32 triggerSpellId = GetSpellInfo()->Effects[m_effIndex].TriggerSpell;
    if (SpellInfo const* triggeredSpellInfo = sSpellMgr->GetSpellInfo(triggerSpellId))
    {
        //if (Unit* triggerCaster = triggeredSpellInfo->NeedsToBeTriggeredByCaster(m_spellInfo) ? caster : target)
        if (Unit* triggerCaster = caster) //sun: see HandlePeriodicTriggerSpellAuraTick
        {
            CastSpellExtraArgs args(this);
            for (uint32 i = 0; i < MAX_SPELL_EFFECTS; ++i)
                args.AddSpellMod(SpellValueMod(SPELLVALUE_BASE_POINT0 + i), GetAmount());
#ifdef LICH_KING
            //don't know what to do with this on BC but this attribute was already existing. Auras can't crit so this attributes is probably not exact at TC.
            if (GetSpellInfo()->HasAttribute(SPELL_ATTR4_INHERIT_CRIT_FROM_AURA))
                args.AddSpellMod(SPELLVALUE_CRIT_CHANCE, int32(GetBase()->GetCritChance() * 100.0f)); // @todo: ugly x100 remove when basepoints are double
#endif
            triggerCaster->CastSpell(target, triggerSpellId, args);
            TC_LOG_DEBUG("spells", "AuraEffect::HandlePeriodicTriggerSpellWithValueAuraTick: Spell %u Trigger %u", GetId(), triggeredSpellInfo->Id);
        }
    }
    else
        TC_LOG_WARN("spells", "AuraEffect::HandlePeriodicTriggerSpellWithValueAuraTick: Spell %u has non-existent spell %u in EffectTriggered[%d] and is therefore not triggered.", GetId(), triggerSpellId, GetEffIndex());
}

void AuraEffect::PeriodicTick(AuraApplication* aurApp, Unit* caster)
{
    bool prevented = GetBase()->CallScriptEffectPeriodicHandlers(this, aurApp);
    if (prevented)
        return;

    Unit* target = aurApp->GetTarget();

    uint32 realDamage = 0; //tick value before reduction, for testing purposes

    switch (GetAuraType())
    {
    case SPELL_AURA_PERIODIC_DAMAGE:
    case SPELL_AURA_PERIODIC_DAMAGE_PERCENT:
    {
        HandlePeriodicDamageAurasTick(target, caster, realDamage);
        break;
    }
    case SPELL_AURA_PERIODIC_LEECH:
    {
        HandlePeriodicHealthLeechAuraTick(target, caster, realDamage);
        break;
    }
    case SPELL_AURA_PERIODIC_HEALTH_FUNNEL:
        HandlePeriodicHealthFunnelAuraTick(target, caster, realDamage);
        break;
    case SPELL_AURA_OBS_MOD_HEALTH:
        if (target->GetHealth() >= target->GetMaxHealth())
            return;
    case SPELL_AURA_PERIODIC_HEAL:
        HandlePeriodicHealAurasTick(target, caster, realDamage);
        break;
    case SPELL_AURA_PERIODIC_MANA_LEECH:
    {
        HandlePeriodicManaLeechAuraTick(target, caster, realDamage);
        break;
    }
    case SPELL_AURA_PERIODIC_ENERGIZE:
    {
        HandlePeriodicEnergizeAuraTick(target, caster, realDamage);
        break;
    }
    case SPELL_AURA_OBS_MOD_POWER:
    {
        HandleObsModPowerAuraTick(target, caster, realDamage);
        break;
    }
    case SPELL_AURA_POWER_BURN:
    {
        HandlePeriodicPowerBurnAuraTick(target, caster, realDamage);
        break;
    }
    // Here tick dummy auras
    case SPELL_AURA_DUMMY:                              // some spells have dummy aura
    case SPELL_AURA_PERIODIC_DUMMY:
    {
        //should be moved to scripts
        PeriodicDummyTick(aurApp, caster, realDamage);
        break;
    }
    case SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE:
        HandlePeriodicTriggerSpellWithValueAuraTick(target, caster, realDamage);
        break;
    break;
    case SPELL_AURA_MOD_REGEN:
        if (GetSpellInfo()->AuraInterruptFlags & AURA_INTERRUPT_FLAG_NOT_SEATED)
            target->HandleEmoteCommand(EMOTE_ONESHOT_EAT);
        break;
    case SPELL_AURA_MOD_POWER_REGEN:
    {
        Powers pt = target->GetPowerType();
        if (pt != Powers(GetMiscValue()))
            return;

        if (GetSpellInfo()->AuraInterruptFlags & AURA_INTERRUPT_FLAG_NOT_SEATED)
            target->HandleEmoteCommand(EMOTE_ONESHOT_EAT);
        else if (GetId() == 20577)
            target->HandleEmoteCommand(EMOTE_STATE_CANNIBALIZE);

        // Warrior talent, gain 1 rage every 3 seconds while in combat
        // Anger Menagement
        // amount = 1+ 16 = 17 = 3,4*5 = 10,2*5/3 
        // so 17 is rounded amount for 5 sec tick grow ~ 1 range grow in 3 sec
        if (pt == POWER_RAGE && target->IsInCombat())
        {
            target->ModifyPower(pt, _amount * 3 / 5);
        }
    }
    break;
    case SPELL_AURA_PERIODIC_TRIGGER_SPELL:
        HandlePeriodicTriggerSpellAuraTick(target, caster, realDamage);
        break;
    default:
        break;
    }

#ifdef TESTS
    if (caster && caster->GetTypeId() == TYPEID_PLAYER)
        if (auto playerBotAI = caster->ToPlayer()->GetPlayerbotAI())
            playerBotAI->PeriodicTick(target, realDamage, GetId());
#endif
}

void AuraEffect::PeriodicDummyTick(AuraApplication* aurApp, Unit* caster, uint32 &realDamage)
{
    Unit* m_target = aurApp->GetTarget();
    SpellInfo const* spell = GetSpellInfo();
    switch (spell->Id)
    {
        // Drink
    case 430:
    case 431:
    case 432:
    case 1133:
    case 1135:
    case 1137:
    case 10250:
    case 22734:
    case 27089:
    case 34291:
    case 43706:
    case 46755:
    {
        if (m_target->GetTypeId() != TYPEID_PLAYER)
            return;
        // Search SPELL_AURA_MOD_POWER_REGEN aura for this spell and add bonus
        Unit::AuraEffectList const& aura = m_target->GetAuraEffectsByType(SPELL_AURA_MOD_POWER_REGEN);
        for (auto i : aura)
        {
            if (i->GetId() == GetId())
            {
                Battleground *bg = (m_target->ToPlayer())->GetBattleground();
                if (!bg || !bg->IsArena())
                {
                    // default case - not in arena
                    m_isPeriodic = false;
                    if (GetTickNumber() == 1)
                        i->SetAmount(_amount);

                    (m_target->ToPlayer())->UpdateManaRegen();
                    return;
                }
                //**********************************************/
                // This feature uses only in arenas
                //**********************************************/
                // Here need increase mana regen per tick (6 second rule)
                // on 0 tick -   0  (handled in 2 second)
                // on 1 tick - 166% (handled in 4 second)
                // on 2 tick - 133% (handled in 6 second)
                // Not need update after 4 tick
                if (GetTickNumber() > 4)
                    return;
                // Apply bonus for 1 - 4 tick
                switch (GetTickNumber())
                {
                case 1:   // 0%
                    i->SetAmount(0);
                    break;
                case 2:   // 166%
                    i->SetAmount(_amount * 5 / 3);
                    break;
                case 3:   // 133%
                    i->SetAmount(_amount * 4 / 3);
                    break;
                default:  // 100% - normal regen
                    i->SetAmount(_amount);
                    break;
                }
                (m_target->ToPlayer())->UpdateManaRegen();
                return;
            }
        }
        return;
    }
    //        // Panda
    //        case 19230: break;
    //        // Master of Subtlety
    //        case 31666: break;
    //        // Gossip NPC Periodic - Talk
    //        case 33208: break;
    //        // Gossip NPC Periodic - Despawn
    //        case 33209: break;
    //        // Force of Nature
    //        case 33831: break;
    // Aspect of the Viper
    case 34074:
    {
        if (m_target->GetTypeId() != TYPEID_PLAYER)
            return;
        // Should be manauser
        if (m_target->GetPowerType() != POWER_MANA)
            return;
        if (!caster)
            return;
        // Regen amount is max (100% from spell) on 21% or less mana and min on 92.5% or greater mana (20% from spell)
        int mana = m_target->GetPower(POWER_MANA);
        int max_mana = m_target->GetMaxPower(POWER_MANA);
        int32 base_regen = caster->CalculateSpellDamage(m_target, m_spellInfo, m_effIndex, &m_spellInfo->Effects[1].BasePoints);
        float regen_pct = 1.20f - 1.1f * mana / max_mana;
        if (regen_pct > 1.0f) 
            regen_pct = 1.0f;
        else if (regen_pct < 0.2f) 
            regen_pct = 0.2f;
        _amount = int32(base_regen * regen_pct);
        (m_target->ToPlayer())->UpdateManaRegen();
        return;
    }
    //        // Steal Weapon
    //        case 36207: break;
    //        // Simon Game START timer, (DND)
    //        case 39993: break;
    //        // Harpooner's Mark
    //        case 40084: break;
    //        // Knockdown Fel Cannon: break; The Aggro Burst
    //        case 40119: break;
    //        // Old Mount Spell
    //        case 40154: break;
    //        // Magnetic Pull
    //        case 40581: break;
    //        // Ethereal Ring: break; The Bolt Burst
    //        case 40801: break;
    //        // Crystal Prison
    //        case 40846: break;
    //        // Copy Weapon
    //        case 41054: break;
    //        // Ethereal Ring Visual, Lightning Aura
    //        case 41477: break;
    //        // Ethereal Ring Visual, Lightning Aura (Fork)
    //        case 41525: break;
    //        // Ethereal Ring Visual, Lightning Jumper Aura
    //        case 41567: break;
    //        // No Man's Land
    //        case 41955: break;
    //        // Headless Horseman - Fire
    //        case 42074: break;
    //        // Headless Horseman - Visual - Large Fire
    //        case 42075: break;
    //        // Headless Horseman - Start Fire, Periodic Aura
    //        case 42140: break;
    //        // Ram Speed Boost
    //        case 42152: break;
    //        // Headless Horseman - Fires Out Victory Aura
    //        case 42235: break;
    //        // Pumpkin Life Cycle
    //        case 42280: break;
    //        // Brewfest Request Chick Chuck Mug Aura
    //        case 42537: break;
    //        // Squashling
    //        case 42596: break;
    //        // Headless Horseman Climax, Head: Periodic
    //        case 42603: break;
    //        // Fire Bomb
    //        case 42621: break;
    //        // Headless Horseman - Conflagrate, Periodic Aura
    //        case 42637: break;
    //        // Headless Horseman - Create Pumpkin Treats Aura
    //        case 42774: break;
    //        // Headless Horseman Climax - Summoning Rhyme Aura
    //        case 42879: break;
    //        // Tricky Treat
    //        case 42919: break;
    //        // Giddyup!
    //        case 42924: break;
    //        // Ram - Trot
    //        case 42992: break;
    //        // Ram - Canter
    //        case 42993: break;
    //        // Ram - Gallop
    //        case 42994: break;
    //        // Ram Level - Neutral
    //        case 43310: break;
    //        // Headless Horseman - Maniacal Laugh, Maniacal, Delayed 17
    //        case 43884: break;
    //        // Headless Horseman - Maniacal Laugh, Maniacal, other, Delayed 17
    //        case 44000: break;
    //        // Energy Feedback
    //        case 44328: break;
    //        // Romantic Picnic
    //        case 45102: break;
    //        // Romantic Picnic
    //        case 45123: break;
    //        // Looking for Love
    //        case 45124: break;
    //        // Kite - Lightning Strike Kite Aura
    //        case 45197: break;
    //        // Rocket Chicken
    //        case 45202: break;
    //        // Copy Offhand Weapon
    //        case 45205: break;
    //        // Upper Deck - Kite - Lightning Periodic Aura
    //        case 45207: break;
    //        // Kite -Sky  Lightning Strike Kite Aura
    //        case 45251: break;
    //        // Ribbon Pole Dancer Check Aura
    //        case 45390: break;
    //        // Holiday - Midsummer, Ribbon Pole Periodic Visual
    //        case 45406: break;
    //        // Parachute
    //        case 45472: break;
    //        // Alliance Flag, Extra Damage Debuff
    //        case 45898: break;
    //        // Horde Flag, Extra Damage Debuff
    //        case 45899: break;
    //        // Ahune - Summoning Rhyme Aura
    //        case 45926: break;
    //        // Ahune - Slippery Floor
    //        case 45945: break;
    //        // Ahune's Shield
    //        case 45954: break;
    //        // Nether Vapor Lightning
    //        case 45960: break;
    //        // Darkness
    //        case 45996: break;
    // Summon Blood Elves Periodic
    case 46041:
    {
        m_target->CastSpell(m_target, 46037, this);
        m_target->CastSpell(m_target, roll_chance_i(50) ? 46038 : 46039, this);
        m_target->CastSpell(m_target, 46040, this);
        return;
    }
    break;
    //        // Transform Visual Missile Periodic
    //        case 46205: break;
    //        // Find Opening Beam End
    //        case 46333: break;
    //        // Ice Spear Control Aura
    //        case 46371: break;
    //        // Hailstone Chill
    //        case 46458: break;
    //        // Hailstone Chill, Internal
    //        case 46465: break;
    //        // Chill, Internal Shifter
    //        case 46549: break;
    //        // Summon Ice Spear Knockback Delayer
    //        case 46878: break;
    //        // Burninate Effect
    //        case 47214: break;
    //        // Fizzcrank Practice Parachute
    //        case 47228: break;
    //        // Send Mug Control Aura
    //        case 47369: break;
    //        // Direbrew's Disarm (precast)
    //        case 47407: break;
    //        // Mole Machine Port Schedule
    //        case 47489: break;
    //        // Mole Machine Portal Schedule
    //        case 49466: break;
    //        // Drink Coffee
    //        case 49472: break;
    //        // Listening to Music
    //        case 50493: break;
    //        // Love Rocket Barrage
    //        case 50530: break;
    default:
        break;
    }
}

void AuraEffect::SendTickImmune(Unit* target, Unit* caster) const
{
    if (caster)
        caster->SendSpellDamageImmune(target, m_spellInfo->Id);
}

bool AuraEffect::CheckEffectProc(AuraApplication* aurApp, ProcEventInfo& eventInfo) const
{
    bool result = GetBase()->CallScriptCheckEffectProcHandlers(this, aurApp, eventInfo);
    if (!result)
        return false;

    SpellInfo const* spellInfo = eventInfo.GetSpellInfo();
    switch (GetAuraType())
    {
    case SPELL_AURA_MOD_CONFUSE:
    case SPELL_AURA_MOD_FEAR:
    case SPELL_AURA_MOD_STUN:
    case SPELL_AURA_MOD_ROOT:
    case SPELL_AURA_TRANSFORM:
    {
        DamageInfo* damageInfo = eventInfo.GetDamageInfo();
        if (!damageInfo || !damageInfo->GetDamage())
            return false;

        // Spell own damage at apply won't break CC
        if (spellInfo && spellInfo == GetSpellInfo())
        {
            Aura* aura = GetBase();
            // called from spellcast, should not have ticked yet
            if (aura->GetDuration() == aura->GetMaxDuration())
                return false;
        }
        break;
    }
    case SPELL_AURA_MECHANIC_IMMUNITY:
    case SPELL_AURA_MOD_MECHANIC_RESISTANCE:
        // compare mechanic
        if (!spellInfo || static_cast<int32>(spellInfo->Mechanic) != GetMiscValue())
            return false;
        break;
#ifdef LICH_KING
    case SPELL_AURA_MOD_CASTING_SPEED_NOT_STACK:
        // skip melee hits and instant cast spells
        if (!spellInfo || !spellInfo->CalcCastTime())
            return false;
        break;
    case SPELL_AURA_MOD_DAMAGE_FROM_CASTER:
        // Compare casters
        if (GetCasterGUID() != eventInfo.GetActor()->GetGUID())
            return false;
        break;
#endif
    case SPELL_AURA_MOD_POWER_COST_SCHOOL:
    case SPELL_AURA_MOD_POWER_COST_SCHOOL_PCT:
        // Skip melee hits and spells with wrong school or zero cost
        if (!spellInfo || (!spellInfo->ManaCost && !spellInfo->ManaCostPercentage) || // Cost Check
            !(spellInfo->GetSchoolMask() & GetMiscValue())) // School Check
            return false;
        break;
    case SPELL_AURA_REFLECT_SPELLS_SCHOOL:
        // Skip melee hits and spells with wrong school
        if (!spellInfo || !(spellInfo->GetSchoolMask() & GetMiscValue()))
            return false;
        break;
    case SPELL_AURA_PROC_TRIGGER_SPELL:
#ifdef LICH_KING
    case SPELL_AURA_PROC_TRIGGER_SPELL_WITH_VALUE:
#endif
    {
        // Don't proc extra attacks while already processing extra attack spell
        uint32 triggerSpellId = GetSpellInfo()->Effects[GetEffIndex()].TriggerSpell;
        if (SpellInfo const* triggeredSpellInfo = sSpellMgr->GetSpellInfo(triggerSpellId))
            if (aurApp->GetTarget()->m_extraAttacks && triggeredSpellInfo->HasEffect(SPELL_EFFECT_ADD_EXTRA_ATTACKS))
                return false;
        break;
    }
    case SPELL_AURA_MOD_SPELL_CRIT_CHANCE:
        // skip spells that can't crit
        if (!spellInfo || !spellInfo->HasAttribute(SPELL_ATTR0_CU_CAN_CRIT))
            return false;
        break;
    default:
        break;
    }

    return result;
}

void AuraEffect::HandleProc(AuraApplication* aurApp, ProcEventInfo& eventInfo)
{
    bool prevented = GetBase()->CallScriptEffectProcHandlers(this, aurApp, eventInfo);
    if (prevented)
        return;

    switch (GetAuraType())
    {
    // CC Auras which use their amount to drop
    // Are there any more auras which need this?
    case SPELL_AURA_MOD_CONFUSE:
    case SPELL_AURA_MOD_FEAR:
    case SPELL_AURA_MOD_STUN:
    case SPELL_AURA_MOD_ROOT:
    case SPELL_AURA_TRANSFORM:
        HandleBreakableCCAuraProc(aurApp, eventInfo);
        break;
    case SPELL_AURA_DUMMY:
    case SPELL_AURA_PROC_TRIGGER_SPELL:
        HandleProcTriggerSpellAuraProc(aurApp, eventInfo);
        break;
    case SPELL_AURA_PROC_TRIGGER_DAMAGE:
        HandleProcTriggerDamageAuraProc(aurApp, eventInfo);
        break;
#ifdef LICH_KING
    case SPELL_AURA_PROC_TRIGGER_SPELL_WITH_VALUE:
        HandleProcTriggerSpellWithValueAuraProc(aurApp, eventInfo);
        break;
    case SPELL_AURA_RAID_PROC_FROM_CHARGE:
        HandleRaidProcFromChargeAuraProc(aurApp, eventInfo);
        break;
    case SPELL_AURA_RAID_PROC_FROM_CHARGE_WITH_VALUE:
        HandleRaidProcFromChargeWithValueAuraProc(aurApp, eventInfo);
        break;
#else
    case SPELL_AURA_PRAYER_OF_MENDING:
        HandleRaidProcFromChargeWithValueAuraProc(aurApp, eventInfo);
#endif
    default:
        break;
    }

    GetBase()->CallScriptAfterEffectProcHandlers(this, aurApp, eventInfo);
}

void AuraEffect::CleanupTriggeredSpells(Unit* target)
{
    if (m_spellInfo->SpellFamilyName == SPELLFAMILY_WARRIOR && m_spellInfo->SpellFamilyFlags & 0x0000001000000020LL)
    {
        // Blood Frenzy remove
        if (!target->HasAuraTypeWithFamilyFlags(SPELL_AURA_PERIODIC_DAMAGE, SPELLFAMILY_WARRIOR, (0x0000001000000020LL & ~m_spellInfo->SpellFamilyFlags)))
        {
            target->RemoveAurasDueToSpell(30069);
            target->RemoveAurasDueToSpell(30070);
            return;
        }
    }

    // Corruption/Seed of Corruption/Curse of Agony - check if shadow embrace should be removed
    if (m_spellInfo->SpellFamilyName == SPELLFAMILY_WARLOCK && m_spellInfo->SpellFamilyFlags & 0x0000001000000402LL)
    {
        bool canRemove = true;
        auto& auraMap = target->GetAppliedAuras();
        for (auto itr = auraMap.begin(); itr != auraMap.end() && canRemove; ++itr) 
        {
            SpellInfo const* proto = itr->second->GetBase()->GetSpellInfo();
            if (proto && proto->SpellFamilyName == SPELLFAMILY_WARLOCK && proto->SpellFamilyFlags & 0x0000001000000402LL) 
            {
                if (GetId() != itr->second->GetBase()->GetId() && itr->second->GetBase()->GetCaster() && GetCaster() && itr->second->GetBase()->GetCaster() == GetCaster())
                    canRemove = false;
            }
        }

        if (canRemove) 
        {
            Unit::AuraEffectList const& auras = target->GetAuraEffectsByType(SPELL_AURA_MOD_DAMAGE_PERCENT_DONE);
            for (auto itr = auras.begin(); itr != auras.end();) 
            {
                SpellInfo const* itr_spell = (*itr)->GetSpellInfo();
                if (itr_spell && itr_spell->SpellFamilyName == SPELLFAMILY_WARLOCK && (itr_spell->SpellFamilyFlags & 0x0000000080000000LL)) 
                {
                    target->RemoveAurasDueToSpell(itr_spell->Id);
                    itr = auras.begin();
                }
                else {
                    itr++;
                }
            }
        }
    }

    uint32 tSpellId = m_spellInfo->Effects[GetEffIndex()].TriggerSpell;
    if (!tSpellId)
        return;

    SpellInfo const* tProto = sSpellMgr->GetSpellInfo(tSpellId);
    if (!tProto)
        return;

    if (tProto->GetDuration() != -1)
        return;

    // needed for spell 43680, maybe others
    // TODO: is there a spell flag, which can solve this in a more sophisticated way?
    if (m_spellInfo->Effects[GetEffIndex()].ApplyAuraName == SPELL_AURA_PERIODIC_TRIGGER_SPELL &&
        (uint32)m_spellInfo->GetDuration() == m_spellInfo->Effects[GetEffIndex()].Amplitude)
        return;
    target->RemoveAurasDueToSpell(tSpellId);
}

void AuraEffect::HandleShapeshiftBoosts(Unit* target, bool apply) const
{
    uint32 spellId = 0;
    uint32 spellId2 = 0;
    uint32 HotWSpellId = 0;

    switch (GetMiscValue())
    {
    case FORM_CAT:
        spellId = 3025;
        HotWSpellId = 24900;
        break;
    case FORM_TREE:
        spellId = 5420;
        break;
    case FORM_TRAVEL:
        spellId = 5419;
        break;
    case FORM_AQUA:
        spellId = 5421;
        break;
    case FORM_BEAR:
        spellId = 1178;
        spellId2 = 21178;
        HotWSpellId = 24899;
        break;
    case FORM_DIREBEAR:
        spellId = 9635;
        spellId2 = 21178;
        HotWSpellId = 24899;
        break;
    case FORM_BATTLESTANCE:
        spellId = 21156;
        break;
    case FORM_DEFENSIVESTANCE:
        spellId = 7376;
        break;
    case FORM_BERSERKERSTANCE:
        spellId = 7381;
        break;
    case FORM_MOONKIN:
        spellId = 24905;
        // aura from effect trigger spell
        spellId2 = 24907;
        break;
    case FORM_FLIGHT:
        spellId = 33948;
        break;
    case FORM_FLIGHT_EPIC:
        spellId = 40122;
        spellId2 = 40121;
        break;
    case FORM_SPIRITOFREDEMPTION:
        spellId = 27792;
        spellId2 = 27795;                               // must be second, this important at aura remove to prevent to early iterator invalidation.
        break;
    case FORM_GHOSTWOLF:
    case FORM_AMBIENT:
    case FORM_GHOUL:
    case FORM_SHADOW:
    case FORM_STEALTH:
    case FORM_CREATURECAT:
    case FORM_CREATUREBEAR:
        spellId = 0;
        break;
    }

    uint32 form = GetMiscValue() - 1;

    if (apply)
    {
        if (spellId)
            target->CastSpell(target, spellId, this);

        if (spellId2)
            target->CastSpell(target, spellId2, this);

        if (target->GetTypeId() == TYPEID_PLAYER)
        {
            const PlayerSpellMap& sp_list = (target->ToPlayer())->GetSpellMap();
            for (const auto & itr : sp_list)
            {
                if (itr.second->state == PLAYERSPELL_REMOVED) continue;
                if (itr.first == spellId || itr.first == spellId2) continue;
                SpellInfo const *spellInfo = sSpellMgr->GetSpellInfo(itr.first);
                if (!spellInfo || !(spellInfo->Attributes & ((1 << 6) | (1 << 7)))) continue;
                if (spellInfo->Stances & (1 << form))
                    target->CastSpell(target, itr.first, this);
            }
            //LotP
            if ((target->ToPlayer())->HasSpell(17007))
            {
                SpellInfo const *spellInfo = sSpellMgr->GetSpellInfo(24932);
                if (spellInfo && spellInfo->Stances & (1 << form))
                    target->CastSpell(target, 24932, this);
            }
            // HotW
            if (HotWSpellId)
            {
                Unit::AuraEffectList const& mModTotalStatPct = target->GetAuraEffectsByType(SPELL_AURA_MOD_TOTAL_STAT_PERCENTAGE);
                for (auto i : mModTotalStatPct)
                {
                    if (i->GetSpellInfo()->SpellIconID == 240 && i->GetMiscValue() == 3)
                    {
                        int32 HotWMod = i->GetAmount();
                        if (GetMiscValue() == FORM_CAT)
                            HotWMod /= 2;

                        CastSpellExtraArgs args;
                        args.TriggerFlags = TRIGGERED_FULL_MASK;
                        args.AddSpellBP0(int32(HotWMod));
                        args.SetTriggeringAura(this);
                        target->CastSpell(target, HotWSpellId, this);
                        break;
                    }
                }
            }
        }
    }
    else
    {
        if(spellId)
            target->RemoveAurasDueToSpell(spellId);
        if(spellId2)
            target->RemoveAurasDueToSpell(spellId2);

        Unit::AuraEffectList const& shapeshifts = target->GetAuraEffectsByType(SPELL_AURA_MOD_SHAPESHIFT);
        AuraEffect const* newAura = nullptr;
        // Iterate through all the shapeshift auras that the target has, if there is another aura with SPELL_AURA_MOD_SHAPESHIFT, then this aura is being removed due to that one being applied
        for (AuraEffect const* aurEff : shapeshifts)
        {
            if (aurEff != this)
            {
                newAura = aurEff;
                break;
            }
        }

        Unit::AuraApplicationMap& tAuras = target->GetAppliedAuras();
        for (auto itr = tAuras.begin(); itr != tAuras.end();)
        {
            // Use the new aura to see on what stance the target will be
            uint64 newStance = newAura ? (UI64LIT(1) << (newAura->GetMiscValue() - 1)) : 0;

            // If the stances are not compatible with the spell, remove it
            if (itr->second->GetBase()->IsRemovedOnShapeLost(target) && !(itr->second->GetBase()->GetSpellInfo()->Stances & newStance))
                target->RemoveAura(itr);
            else
                ++itr;
        }
    }
}

void AuraEffect::CalculateSpellMod()
{
    switch (GetAuraType())
    {
    case SPELL_AURA_ADD_FLAT_MODIFIER:
    case SPELL_AURA_ADD_PCT_MODIFIER:
        if (!m_spellmod)
        {
            if (GetMiscValue() >= MAX_SPELLMOD)
                break;

            m_spellmod = new SpellModifier(GetBase());
            m_spellmod->op = SpellModOp(GetMiscValue());

            m_spellmod->type = SpellModType(GetAuraType());    // SpellModType value == spell aura types
            m_spellmod->spellId = GetId();

            m_spellmod->mask = GetSpellInfo()->Effects[GetEffIndex()].SpellClassMask;
            if(!m_spellmod->mask)
                m_spellmod->mask = GetSpellInfo()->Effects[m_effIndex].ItemType;
        }
        m_spellmod->value = GetAmount();
        break;
    default:
        break;
    }

    GetBase()->CallScriptEffectCalcSpellModHandlers(this, m_spellmod);
}

void AuraEffect::ApplySpellMod(Unit* target, bool apply)
{
    if (!m_spellmod || target->GetTypeId() != TYPEID_PLAYER)
        return;

    target->ToPlayer()->AddSpellMod(m_spellmod, apply);

    // Auras with charges do not mod amount of passive auras
    if (GetBase()->IsUsingCharges())
        return;

    // reapply some passive spells after add/remove related spellmods
    // Warning: it is a dead loop if 2 auras each other amount-shouldn't happen
    switch (GetMiscValue())
    {
    case SPELLMOD_ALL_EFFECTS:
    case SPELLMOD_EFFECT1:
    case SPELLMOD_EFFECT2:
    case SPELLMOD_EFFECT3:
    {
        ObjectGuid guid = target->GetGUID();

        Unit::AuraApplicationMap & auras = target->GetAppliedAuras();
        for (Unit::AuraApplicationMap::iterator iter = auras.begin(); iter != auras.end(); ++iter)
        {
            Aura* aura = iter->second->GetBase();
            // only passive and permament auras-active auras should have amount set on spellcast and not be affected
            // if aura is cast by others, it will not be affected
            if ((aura->IsPassive() || aura->IsPermanent()) && aura->GetCasterGUID() == guid && aura->GetSpellInfo()->IsAffectedBySpellMod(m_spellmod))
            {
                if (GetMiscValue() == SPELLMOD_ALL_EFFECTS)
                {
                    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
                    {
                        if (AuraEffect* aurEff = aura->GetEffect(i))
                            aurEff->RecalculateAmount();
                    }
                }
                else if (GetMiscValue() == SPELLMOD_EFFECT1)
                {
                    if (AuraEffect* aurEff = aura->GetEffect(0))
                        aurEff->RecalculateAmount();
                }
                else if (GetMiscValue() == SPELLMOD_EFFECT2)
                {
                    if (AuraEffect* aurEff = aura->GetEffect(1))
                        aurEff->RecalculateAmount();
                }
                else //if (modOp == SPELLMOD_EFFECT3)
                {
                    if (AuraEffect* aurEff = aura->GetEffect(2))
                        aurEff->RecalculateAmount();
                }
            }
        }
    }
    default:
        break;
    }

    //old hacks time
    if (apply && m_spellmod)
    {
        uint64 spellFamilyMask = m_spellmod->mask;
        // Spiritual Attunement hack
        if (GetSpellInfo()->SpellFamilyName == SPELLFAMILY_PALADIN && (spellFamilyMask & 0x0000100000000000LL))
        {
            if (target->HasAura(31785)) // rank 1
            {
                target->RemoveAurasDueToSpell(31785);
                target->CastSpell(target, 31785, true);
            }
            if (target->HasAura(33776)) // rank 2
            {
                target->RemoveAurasDueToSpell(33776);
                target->CastSpell(target, 33776, true);
            }
        }
    }
}

void AuraEffect::Update(uint32 diff, Unit* caster)
{
    if (!m_isPeriodic || (GetBase()->GetDuration() < 0 && !GetBase()->IsPassive() && !GetBase()->IsPermanent()))
        return;

    uint32 totalTicks = GetTotalTicks();

    _periodicTimer += diff;
    while (_periodicTimer >= _amplitude)
    {
        _periodicTimer -= _amplitude;

        if (!GetBase()->IsPermanent() && (_ticksDone + 1) > totalTicks)
            break;

        ++_ticksDone;

        GetBase()->CallScriptEffectUpdatePeriodicHandlers(this);

        std::vector<AuraApplication*> effectApplications;
        GetApplicationList(effectApplications);

        // tick on targets of effects
        for (AuraApplication* aurApp : effectApplications)
            PeriodicTick(aurApp, caster);
    }
}


/*********************************************************/
/***                  AURA EFFECTS                     ***/
/*********************************************************/

void AuraEffect::HandleAuraDummy(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_REAPPLY)))
        return;

    Unit* m_target = aurApp->GetTarget();
    Unit* caster = GetCaster();

    // pet auras
    /* TC
    if (m_target->GetTypeId() == TYPEID_PLAYER && (mode & AURA_EFFECT_HANDLE_REAL))
    {
        if (PetAura const* petSpell = sSpellMgr->GetPetAura(GetId(), m_effIndex))
        {
            if (apply)
                target->ToPlayer()->AddPetAura(petSpell);
            else
                target->ToPlayer()->RemovePetAura(petSpell);
        }
    }*/

    if (mode & (AURA_EFFECT_HANDLE_REAL | AURA_EFFECT_HANDLE_REAPPLY))
    {
        // AT APPLY
        if (apply)
        {
            switch (GetId())
            {
            case 1515:                                      // Tame beast
                                                            // FIX_ME: this is 2.0.12 threat effect replaced in 2.1.x by dummy aura, must be checked for correctness
                if (caster && m_target->CanHaveThreatList())
                    m_target->GetThreatManager().AddThreat(caster, 10.0f);
                return;
            case 10909: //Mind Vision
                if (caster->HasAura(14751))
                    caster->RemoveAurasDueToSpell(14751);
                break;
            case 13139:                                     // net-o-matic
                                                            // root to self part of (root_target->charge->root_self sequence
                if (caster)
                    caster->CastSpell(caster, 13138, this);
                return;
            case 34520: //Elemental Power Extractor
            {
                if (!m_target || m_target->GetTypeId() != TYPEID_UNIT || /*!m_target->IsDead() || */caster->GetTypeId() != TYPEID_PLAYER)
                    return;

                if (m_target->GetEntry() == 18881 || m_target->GetEntry() == 18865)
                {
                    /* GameObject* elemPower = */ caster->ToPlayer()->SummonGameObject(183933, Position(m_target->GetPositionX(), m_target->GetPositionY(), (m_target->GetPositionZ() + 2.5f), m_target->GetOrientation()), G3D::Quat(), (m_target->ToCreature())->GetRespawnTime() - time(nullptr));
                    //elemPower->SetLootState(GO_READY);
                }
                return;
            }
            case 32014: //archimonde bump
            {
                GetBase()->SetDuration(GetBase()->GetDuration() * 3);
                if (m_target->HasAura(31970))       // Archimonde fear, remove Fear before bump
                    m_target->RemoveAurasDueToSpell(31970);
                return;
            }
            /*
            Working with spell_target and SmartAI
            case 32146: //Liquid Fire
            {
            if (!m_target && !caster->GetVictim())
            return;

            Creature* cTarget = caster->FindNearestCreature(18240, 5, true);
            if ((caster->ToPlayer())->GetQuestStatus(9874) == QUEST_STATUS_INCOMPLETE && cTarget)
            {
            (caster->ToPlayer())->KilledMonsterCredit(18240, 0);
            cTarget->ForcedDespawn();
            }

            return;
            }
            */
            case 37096:                                     // Blood Elf Disguise
                if (caster)
                {
                    uint32 castSpellId = 0;
                    switch (caster->GetGender())
                    {
                    case GENDER_FEMALE: castSpellId = 37095; break;
                    case GENDER_MALE: castSpellId = 37093; break;
                    default: break;
                    }
                    if (castSpellId)
                        caster->CastSpell(m_target, castSpellId, this);
                }
                return;
            case 39850:                                     // Rocket Blast
                if (roll_chance_i(20))                       // backfire stun
                    m_target->CastSpell(m_target, 51581, this);
                return;
            case 43873:                                     // Headless Horseman Laugh
                if (caster->GetTypeId() == TYPEID_PLAYER)
                    (caster->ToPlayer())->PlayDirectSound(11965);
                return;
            case 46354:                                     // Blood Elf Illusion
                if (caster)
                {
                    uint32 castSpellId = 0;
                    switch (caster->GetGender())
                    {
                    case GENDER_FEMALE: castSpellId = 46356; break;
                    case GENDER_MALE:   castSpellId = 46355; break;
                    default:
                        break;
                    }
                    if (castSpellId)
                        caster->CastSpell(m_target, castSpellId, this);
                }
                return;
            case 46699:                                     // Requires No Ammo
                if (m_target->GetTypeId() == TYPEID_PLAYER)
                    (m_target->ToPlayer())->RemoveAmmo();      // not use ammo and not allow use
                return;
            }
        }
        // AT REMOVE
        else
        {
            if ((IsQuestTameSpell(GetId())) && caster && caster->IsAlive() && m_target->IsAlive())
            {
                uint32 finalSpelId = 0;
                switch (GetId())
                {
                case 19548: finalSpelId = 19597; break;
                case 19674: finalSpelId = 19677; break;
                case 19687: finalSpelId = 19676; break;
                case 19688: finalSpelId = 19678; break;
                case 19689: finalSpelId = 19679; break;
                case 19692: finalSpelId = 19680; break;
                case 19693: finalSpelId = 19684; break;
                case 19694: finalSpelId = 19681; break;
                case 19696: finalSpelId = 19682; break;
                case 19697: finalSpelId = 19683; break;
                case 19699: finalSpelId = 19685; break;
                case 19700: finalSpelId = 19686; break;
                case 30646: finalSpelId = 30647; break;
                case 30653: finalSpelId = 30648; break;
                case 30654: finalSpelId = 30652; break;
                case 30099: finalSpelId = 30100; break;
                case 30102: finalSpelId = 30103; break;
                case 30105: finalSpelId = 30104; break;
                }

                if (finalSpelId)
                    caster->CastSpell(m_target, finalSpelId, this);
                return;
            }

            switch (GetId())
            {
            case 27243: //seed of corruption hackkkk
            {
                if (m_target->GetHealth() == 0) //we died before the seed could explode
                {
                    if (Unit* caster2 = ObjectAccessor::GetUnit(*m_target, GetCasterGUID()))
                        caster2->CastSpell(m_target, 27285, true); //explosion spell
                }
            }
            case 2584:                                     // Waiting to Resurrect
            {
                // Waiting to resurrect spell cancel, we must remove player from resurrect queue
                if (m_target->GetTypeId() == TYPEID_PLAYER)
                    if (Battleground *bg = (m_target->ToPlayer())->GetBattleground())
                        bg->RemovePlayerFromResurrectQueue(m_target->GetGUID());
                return;
            }
            case 28169:                                     // Mutating Injection
            {
                // Mutagen Explosion
                m_target->CastSpell(m_target, 28206, this);
                // Poison Cloud
                m_target->CastSpell(m_target, 28240, this);
                return;
            }
            case 36730:                                     // Flame Strike
            {
                m_target->CastSpell(m_target, 36731, this);
                return;
            }
            case 44191:                                     // Flame Strike
            {
                if (m_target->GetMap()->IsDungeon())
                {
                    uint32 spellId = m_target->GetMap()->IsHeroic() ? 46163 : 44190;

                    m_target->CastSpell(m_target, spellId, this);
                }
                return;
            }
            case 45934:                                     // Dark Fiend
            {
                // Kill target if dispelled
                if (aurApp->GetRemoveMode() == AURA_REMOVE_BY_ENEMY_SPELL)
                    Unit::DealDamage(m_target, m_target, m_target->GetHealth(), nullptr, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, nullptr, false);
                return;
            }
            case 46308:                                     // Burning Winds - casted only at creatures at spawn
            {
                m_target->CastSpell(m_target, 47287, this);
                return;
            }
            case 34477:                                     // Misdirection
            {
                aurApp->GetTarget()->GetThreatManager().UnregisterRedirectThreat(GetId()); //SPELL_HUNTER_MISDIRECTION
                return;
            }
            case 40830:
            {
                if (GetCaster() && aurApp->GetRemoveMode() == AURA_REMOVE_BY_DEATH)
                    GetCaster()->CastSpell(GetCaster(), 40828, TRIGGERED_NONE);
            }
            case 34367: // quest 10204
            {
                if (caster && caster->GetTypeId() == TYPEID_PLAYER && aurApp->GetRemoveMode() == AURA_REMOVE_BY_DEFAULT)
                    caster->ToPlayer()->AreaExploredOrEventHappens(10204);

                return;
            }
            case 39238: // quest 10929
            {
                if (caster && caster->GetTypeId() == TYPEID_PLAYER) {
                    float x, y, z;
                    caster->GetRandomPoint(caster, 10.0f, x, y, z);
                    if (Creature* summoned = caster->SummonCreature(((rand() % 2) ? 22482 : 22483), x, y, z, 0, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 120000))
                        summoned->AI()->AttackStart(caster);
                }

                return;
            }
            case 39246: // quest 10930
            {
                if (caster && caster->GetTypeId() == TYPEID_PLAYER) {
                    Creature* clefthoof = caster->FindNearestCreature(22105, 30.0f, false);
                    if (!clefthoof)
                        return;

                    float x, y, z;
                    uint32 entry = 0, amount = 0;

                    uint32 randomSpawn = urand(0, 15);
                    if (randomSpawn < 1) {
                        entry = 22038;
                        amount = 1;
                    }
                    else if (randomSpawn < 8) {
                        entry = 22482;
                        amount = 3;
                    }
                    else {
                        entry = 22483;
                        amount = 3;
                    }

                    for (int i = 0; i < amount; i++) {
                        caster->GetRandomPoint(caster, 10.0f, x, y, z);
                        if (Creature* summoned = caster->SummonCreature(entry, x, y, z, 0, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 120000))
                            summoned->AI()->AttackStart(caster);
                    }
                }

                return;
            }
            case 30019: // Karazhan Chess event: Control piece
            {
                if (caster && caster->GetTypeId() == TYPEID_PLAYER) {
                    if (Unit* charmed = caster->ToPlayer()->GetCharm()) {
                        charmed->RemoveCharmedBy(caster);
                        charmed->RemoveAura(30019);
                        caster->CastSpell(caster, 30529, true);
                    }
                }

                return;
            }
            case 46605: // Kil Jaeden : Darkness of a Thousand Souls
            {
                if (!GetCaster())
                    return;

                GetCaster()->CastSpell(GetCaster(), GetSpellInfo()->Effects[2].BasePoints, true);
                return;
            }
            }
        }
    }

    // AT APPLY & REMOVE

    switch (m_spellInfo->SpellFamilyName)
    {
    case SPELLFAMILY_GENERIC:
    {
        switch (GetId())
        {
        // Recently Bandaged
        case 11196:
            m_target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, GetMiscValue(), apply);
            break;
        case 46230: //Muru black hole bump
        {
            m_target->ApplySpellImmune(0, IMMUNITY_ID, 46264, apply); //immune to void zone (p2)
            return;
        }
        case 45043: // Power circle (Muru trinket)
            if (caster)
            {
                if (apply)
                    caster->CastSpell(caster, 45044, true); //45044 = 320 spell power aura
                else
                    caster->RemoveAurasDueToSpell(45044);
            }
            return;
        return;
        case 16093:                                  // Self Visual - Sleep Until Cancelled (DND)
        {
            if (apply)
            {
                m_target->SetStandState(UNIT_STAND_STATE_DEAD);
                m_target->AddUnitState(UNIT_STATE_ROOT);
            }
            else
            {
                m_target->ClearUnitState(UNIT_STATE_ROOT);
                m_target->SetStandState(UNIT_STAND_STATE_STAND);
            }

            return;
        }
        case 24658: // Unstable Power
        {
            uint32 spellId = 24659;
            if (apply)
            {
                const SpellInfo *spell = sSpellMgr->GetSpellInfo(spellId);
                if (!spell)
                    return;
                for (int i = 0; i < spell->StackAmount; ++i)
                    caster->CastSpell(m_target, spell->Id, GetCasterGUID());
                return;
            }
            m_target->RemoveAurasDueToSpell(spellId);
        }
        return;
        case 24661: // Restless Strength
        {
            uint32 spellId = 24662;
            if (apply)
            {
                const SpellInfo *spell = sSpellMgr->GetSpellInfo(spellId);
                if (!spell)
                    return;
                for (int i = 0; i < spell->StackAmount; ++i)
                    caster->CastSpell(m_target, spell->Id, GetCasterGUID());
                return;
            }
            m_target->RemoveAurasDueToSpell(spellId);
        }
        return;
        case 40133: //Summon Fire Elemental
            if (caster)
            {
                Unit *owner = caster->GetOwner();
                if (owner && owner->GetTypeId() == TYPEID_PLAYER)
                {
                    if (apply)
                        owner->CastSpell(owner, 8985, true);
                    else
                        (owner->ToPlayer())->RemovePet(nullptr, PET_SAVE_NOT_IN_SLOT, true);
                }
            }
            return;
        case 40132: //Summon Earth Elemental
            if (caster)
            {
                Unit *owner = caster->GetOwner();
                if (owner && owner->GetTypeId() == TYPEID_PLAYER)
                {
                    if (apply)
                        owner->CastSpell(owner, 19704, true);
                    else
                        (owner->ToPlayer())->RemovePet(nullptr, PET_SAVE_NOT_IN_SLOT, true);
                }
            }
            return;
        }
        break;
    }
    case SPELLFAMILY_MAGE:
    {
        break;
    }
    case SPELLFAMILY_DRUID:
    {
        // Lifebloom
        if (GetSpellInfo()->SpellFamilyFlags & 0x1000000000LL)
        {
            // Do final heal for real !apply
            if (!apply && (mode & AURA_EFFECT_HANDLE_REAL))
            {
                if (GetBase()->GetDuration() <= 0 || aurApp->GetRemoveMode() == AURA_REMOVE_BY_ENEMY_SPELL)
                {
                    // final heal
                    if (m_target->IsInWorld())
                    {
                        int32 amount = _amount;
                        if(caster)
                            amount = caster->SpellHealingBonusDone(m_target, GetSpellInfo(), _amount, HEAL, GetEffIndex());

                        CastSpellExtraArgs args;
                        args.TriggerFlags = TRIGGERED_FULL_MASK;
                        args.AddSpellBP0(int32(amount));
                        args.SetTriggeringAura(this);
                        args.SetOriginalCaster(GetCasterGUID());
                        m_target->CastSpell(m_target, 33778, args);
                    }
                }
            }
            return;
        }

        // Predatory Strikes
        if (m_target->GetTypeId() == TYPEID_PLAYER && GetSpellInfo()->SpellIconID == 1563)
        {
            (m_target->ToPlayer())->UpdateAttackPowerAndDamage();
            return;
        }
    }
    case SPELLFAMILY_SHAMAN:
    {
        // Sentry Totem
        if (GetId() == 6495 && caster->GetTypeId() == TYPEID_PLAYER)
        {
            if (apply)
            {
                ObjectGuid guid = caster->m_SummonSlot[3];
                if (guid)
                {
                    Creature *totem = ObjectAccessor::GetCreature(*caster, guid);
                    if (totem && totem->IsTotem())
                        totem->AddPlayerToVision(caster->ToPlayer());
                }
            }
            else
                (caster->ToPlayer())->StopCastingBindSight();
            return;
        }
        break;
    }
    }

    // pet auras
    if (PetAura const* petSpell = sSpellMgr->GetPetAura(GetId()))
    {
        if (apply)
            m_target->AddPetAura(petSpell);
        else
            m_target->RemovePetAura(petSpell);
        return;
    }
}

void AuraEffect::HandleAuraPeriodicDummy(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    Unit* m_target = aurApp->GetTarget();
    SpellInfo const* spell = GetSpellInfo();
    switch (spell->SpellFamilyName)
    {
    case SPELLFAMILY_ROGUE:
    {
        // Master of Subtlety
        if (spell->Id == 31666 && !apply && (mode & AURA_EFFECT_HANDLE_REAL))
        {
            m_target->RemoveAurasDueToSpell(31665);
            break;
        }
        break;
    }
    case SPELLFAMILY_HUNTER:
    {
        // Aspect of the Viper
        if (spell->SpellFamilyFlags & 0x0004000000000000LL)
        {
            // Update regen on remove
            if (!apply && m_target->GetTypeId() == TYPEID_PLAYER)
                (m_target->ToPlayer())->UpdateManaRegen();
            break;
        }
        break;
    }
    }
}

void AuraEffect::HandleAuraMounted(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_SEND_FOR_CLIENT_MASK))
        return;

    Unit* m_target = aurApp->GetTarget();
    if (apply)
    {
        CreatureTemplate const* ci = sObjectMgr->GetCreatureTemplate(GetMiscValue());
        if (!ci)
        {
            TC_LOG_ERROR("FIXME", "AuraMounted: `creature_template`='%u' not found in database (only need it modelid)", GetMiscValue());
            return;
        }

        uint32 display_id = ObjectMgr::ChooseDisplayId(ci);
        sObjectMgr->GetCreatureModelRandomGender(display_id);

        //m_target->RemoveAurasByType(SPELL_AURA_MOUNTED);
        bool flying = false;
        for (const auto & Effect : m_spellInfo->Effects) {
            if (Effect.ApplyAuraName == SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED)
                flying = true;
        }
        m_target->Mount(display_id, flying);
    }
    else
    {
        m_target->Dismount();
    }
}

void AuraEffect::HandleAuraWaterWalk(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_SEND_FOR_CLIENT_MASK))
        return;

    Unit* m_target = aurApp->GetTarget();
    if (!apply)
    {
        // do not remove unit flag if there are more than this auraEffect of that kind on unit on unit
        if (m_target->HasAuraType(GetAuraType()))
            return;
    }

    m_target->SetWaterWalking(apply);
}

void AuraEffect::HandleAuraFeatherFall(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    // only at real add/remove aura
    if (!(mode & AURA_EFFECT_HANDLE_SEND_FOR_CLIENT_MASK))
        return;

    Unit* m_target = aurApp->GetTarget();
    if (!apply)
    {
        // do not remove unit flag if there are more than this auraEffect of that kind on unit on unit
        if (m_target->HasAuraType(GetAuraType()))
            return;
    }

    m_target->SetFeatherFall(apply);

    if (!apply && m_target->GetTypeId() == TYPEID_PLAYER)
        m_target->ToPlayer()->SetFallInformation(0, m_target->GetPositionZ());
}

void AuraEffect::HandleAuraHover(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_SEND_FOR_CLIENT_MASK))
        return;

    Unit* m_target = aurApp->GetTarget();
    if (!apply)
    {
        // do not remove unit flag if there are more than this auraEffect of that kind on unit on unit
        if (m_target->HasAuraType(GetAuraType()))
            return;
    }

    m_target->SetHover(apply);

    if (!apply && m_target->GetTypeId() == TYPEID_PLAYER)
        m_target->ToPlayer()->SetFallInformation(0, m_target->GetPositionZ());
}

void AuraEffect::HandleWaterBreathing(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_SEND_FOR_CLIENT_MASK))
        return;

    Unit* m_target = aurApp->GetTarget();
    // update timers in client
    if (m_target->GetTypeId() == TYPEID_PLAYER && m_target->IsInWorld())
        m_target->ToPlayer()->UpdateMirrorTimers();
}

void AuraEffect::HandleAuraModShapeshift(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK))
        return;

    Unit* m_target = aurApp->GetTarget();
    Powers PowerType = POWER_MANA;
    ShapeshiftForm form = ShapeshiftForm(GetMiscValue());
    uint32 modelid = m_target->GetModelForForm(form);
    switch (form)
    {
    case FORM_CAT:
        PowerType = POWER_ENERGY;
        break;
    case FORM_BEAR:
    case FORM_DIREBEAR:
    case FORM_BATTLESTANCE:
    case FORM_BERSERKERSTANCE:
    case FORM_DEFENSIVESTANCE:
        PowerType = POWER_RAGE;
        break;
    case FORM_SPIRITOFREDEMPTION:
    case FORM_SHADOW:
    case FORM_STEALTH:
    case FORM_FLIGHT_EPIC:
    case FORM_FLIGHT:
    case FORM_TRAVEL:
    case FORM_AQUA:
    case FORM_GHOSTWOLF:
    case FORM_MOONKIN:
    case FORM_TREE:
    case FORM_NONE:
    case FORM_CREATUREBEAR: //sunstrider: several quest spells used by npc
    case FORM_CREATURECAT: //sunstrider: several quest spells used by npc
        break;
    default:
        TC_LOG_ERROR("FIXME", "Auras: Unknown Shapeshift Type: %u for spell %u", form, GetId());
    }

    // remove polymorph before changing display id to keep new display id
    switch (form)
    {
    case FORM_CAT:
    case FORM_TREE:
    case FORM_TRAVEL:
    case FORM_AQUA:
    case FORM_BEAR:
    case FORM_DIREBEAR:
    case FORM_FLIGHT_EPIC:
    case FORM_FLIGHT:
    case FORM_MOONKIN:
        // remove movement affects
        m_target->RemoveAurasByShapeShift();

        // and polymorphic affects
        if (m_target->IsPolymorphed())
            m_target->RemoveAurasDueToSpell(m_target->GetTransForm());
        break;
    default:
        break;
    }

    if (apply)
    {
        // remove other shapeshift before applying a new one
        m_target->RemoveAurasByType(SPELL_AURA_MOD_SHAPESHIFT, ObjectGuid::Empty, GetBase());

        m_target->SetByteValue(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_SHAPESHIFT_FORM, form);

        if (modelid > 0)
            m_target->SetDisplayId(modelid);

        if (PowerType != POWER_MANA)
        {
            // reset power to default values only at power change
            if (m_target->GetPowerType() != PowerType)
                m_target->SetPowerType(PowerType);

            switch (form)
            {
            case FORM_CAT:
            case FORM_BEAR:
            case FORM_DIREBEAR:
            {
                // get furor proc chance
                uint32 FurorChance = 0;
                Unit::AuraEffectList const& mDummy = m_target->GetAuraEffectsByType(SPELL_AURA_DUMMY);
                for (auto i : mDummy)
                {
                    if (i->GetSpellInfo()->SpellIconID == 238)
                    {
                        FurorChance = i->GetAmount();
                        break;
                    }
                }

                if (FurorChance)
                {
                    uint32 castSpellId = 0;
                    if (GetMiscValue() == FORM_CAT)
                    {
                        m_target->SetPower(POWER_ENERGY, 0);
                        if (urand(1, 100) <= FurorChance)
                            castSpellId = 17099;
                    }
                    else
                    {
                        m_target->SetPower(POWER_RAGE, 0);
                        if (urand(1, 100) <= FurorChance)
                            castSpellId = 17057;
                    }
                    if (castSpellId)
                        m_target->CastSpell(m_target, castSpellId, this);
                }
                break;
            }
            case FORM_BATTLESTANCE:
            case FORM_DEFENSIVESTANCE:
            case FORM_BERSERKERSTANCE:
            {
                uint32 Rage_val = 0;
                // Stance mastery + Tactical mastery (both passive, and last have aura only in defense stance, but need apply at any stance switch)
                if (m_target->GetTypeId() == TYPEID_PLAYER)
                {
                    PlayerSpellMap const& sp_list = (m_target->ToPlayer())->GetSpellMap();
                    for (const auto & itr : sp_list)
                    {
                        if (itr.second->state == PLAYERSPELL_REMOVED) continue;
                        SpellInfo const *spellInfo = sSpellMgr->GetSpellInfo(itr.first);
                        if (spellInfo && spellInfo->SpellFamilyName == SPELLFAMILY_WARRIOR && spellInfo->SpellIconID == 139)
                            Rage_val += m_target->CalculateSpellDamage(m_target, spellInfo, 0, &spellInfo->Effects[0].BasePoints) * 10;
                    }
                }

                if (m_target->GetPower(POWER_RAGE) > Rage_val)
                    m_target->SetPower(POWER_RAGE, Rage_val);
                break;
            }
            default:
                break;
            }
        }

        m_target->m_ShapeShiftFormSpellId = GetId();
        m_target->m_form = form;
    }
    else
    {
        m_target->SetShapeshiftForm(FORM_NONE);
        if (m_target->GetClass() == CLASS_DRUID)
        {
            m_target->SetPowerType(POWER_MANA);
            m_target->RemoveAurasByShapeShift();
        }
        m_target->m_ShapeShiftFormSpellId = 0;
        m_target->m_form = FORM_NONE;

        if (modelid > 0)
            m_target->RestoreDisplayId();

        switch (form)
        {
            // Nordrassil Harness - bonus
        case FORM_BEAR:
        case FORM_DIREBEAR:
        case FORM_CAT:
        {
            if (AuraEffect* dummy = m_target->GetDummyAura(37315))
                m_target->CastSpell(m_target, 37316, dummy);
            break;
        }
        // Nordrassil Regalia - bonus
        case FORM_MOONKIN:
        {
            if (AuraEffect* dummy = m_target->GetDummyAura(37324))
                m_target->CastSpell(m_target, 37325, dummy);
            break;
        }
        default:
            break;
        }
    }

    // adding/removing linked auras
    // add/remove the shapeshift aura's boosts
    HandleShapeshiftBoosts(m_target, apply);

    if (m_target->GetTypeId() == TYPEID_PLAYER)
        (m_target->ToPlayer())->InitDataForForm();

    // force update as too quick shapeshifting and back
    // causes the value to stay the same serverside
    // causes issues clientside (player gets stuck)
    m_target->ForceValuesUpdateAtIndex(UNIT_FIELD_BYTES_2);
}

void AuraEffect::HandleAuraTransform(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_SEND_FOR_CLIENT_MASK))
        return;

    Unit* m_target = aurApp->GetTarget();
    if (apply)
    {
        m_target->SetTransForm(GetId());
        // special case (spell specific functionality)
        if (GetMiscValue() == 0)
        {
            // player applied only
            if (m_target->GetTypeId() != TYPEID_PLAYER)
                return;

            switch (GetId())
            {
                // Orb of Deception
            case 16739:
            {
                uint32 orb_model = m_target->GetNativeDisplayId();
                switch (orb_model)
                {
                    // Troll Female
                case 1479: m_target->SetDisplayId(10134); break;
                    // Troll Male
                case 1478: m_target->SetDisplayId(10135); break;
                    // Tauren Male
                case 59:   m_target->SetDisplayId(10136); break;
                    // Human Male
                case 49:   m_target->SetDisplayId(10137); break;
                    // Human Female
                case 50:   m_target->SetDisplayId(10138); break;
                    // Orc Male
                case 51:   m_target->SetDisplayId(10139); break;
                    // Orc Female
                case 52:   m_target->SetDisplayId(10140); break;
                    // Dwarf Male
                case 53:   m_target->SetDisplayId(10141); break;
                    // Dwarf Female
                case 54:   m_target->SetDisplayId(10142); break;
                    // NightElf Male
                case 55:   m_target->SetDisplayId(10143); break;
                    // NightElf Female
                case 56:   m_target->SetDisplayId(10144); break;
                    // Undead Female
                case 58:   m_target->SetDisplayId(10145); break;
                    // Undead Male
                case 57:   m_target->SetDisplayId(10146); break;
                    // Tauren Female
                case 60:   m_target->SetDisplayId(10147); break;
                    // Gnome Male
                case 1563: m_target->SetDisplayId(10148); break;
                    // Gnome Female
                case 1564: m_target->SetDisplayId(10149); break;
                    // BloodElf Female
                case 15475: m_target->SetDisplayId(17830); break;
                    // BloodElf Male
                case 15476: m_target->SetDisplayId(17829); break;
                    // Draenei Female
                case 16126: m_target->SetDisplayId(17828); break;
                    // Draenei Male
                case 16125: m_target->SetDisplayId(17827); break;
                default: break;
                }
                break;
            }
            // Dread Corsair - Pirate Day
            case 50531:
            case 50517:
            {
                uint32 model = m_target->GetNativeDisplayId();
                switch (model)
                {
                    // Troll Female
                case 1479: m_target->SetDisplayId(25052); break;
                    // Troll Male
                case 1478: m_target->SetDisplayId(25041); break;
                    // Tauren Male
                case 59:   m_target->SetDisplayId(25040); break;
                    // Human Male
                case 49:   m_target->SetDisplayId(25037); break;
                    // Human Female
                case 50:   m_target->SetDisplayId(25048); break;
                    // Orc Male
                case 51:   m_target->SetDisplayId(25039); break;
                    // Orc Female
                case 52:   m_target->SetDisplayId(25050); break;
                    // Dwarf Male
                case 53:   m_target->SetDisplayId(25034); break;
                    // Dwarf Female
                case 54:   m_target->SetDisplayId(25045); break;
                    // NightElf Male
                case 55:   m_target->SetDisplayId(25038); break;
                    // NightElf Female
                case 56:   m_target->SetDisplayId(25049); break;
                    // Undead Female
                case 58:   m_target->SetDisplayId(25053); break;
                    // Undead Male
                case 57:   m_target->SetDisplayId(25042); break;
                    // Tauren Female
                case 60:   m_target->SetDisplayId(25051); break;
                    // Gnome Male
                case 1563: m_target->SetDisplayId(25035); break;
                    // Gnome Female
                case 1564: m_target->SetDisplayId(25046); break;
                    // BloodElf Female
                case 15475: m_target->SetDisplayId(25043); break;
                    // BloodElf Male
                case 15476: m_target->SetDisplayId(25032); break;
                    // Draenei Female
                case 16126: m_target->SetDisplayId(25044); break;
                    // Draenei Male
                case 16125: m_target->SetDisplayId(25033); break;
                default: break;
                }
                break;
            }
            // Murloc costume
            case 42365: m_target->SetDisplayId(21723); break;
            default: break;
            }
        }
        else
        {
            CreatureTemplate const * ci = sObjectMgr->GetCreatureTemplate(GetMiscValue());
            if (!ci)
            {
                //pig pink ^_^
                m_target->SetDisplayId(16358);
                TC_LOG_ERROR("spells", "Auras: unknown creature id = %d (only need its modelid) Form Spell Aura Transform in Spell ID = %d", GetMiscValue(), GetId());
            }
            else
            {
                // Will use the default model here
                if (uint32 modelid = ObjectMgr::ChooseDisplayId(ci))
                    m_target->SetDisplayId(modelid);

                // Dragonmaw Illusion (set mount model also)
                if (GetId() == 42016 && m_target->GetMountID() && !m_target->GetAuraEffectsByType(SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED).empty())
                    m_target->SetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID, 16314);
            }
        }

        // polymorph case
        if ((mode & AURA_EFFECT_HANDLE_REAL) && m_target->GetTypeId() == TYPEID_PLAYER && m_target->IsPolymorphed())
        {
            // for players, start regeneration after 1s (in polymorph fast regeneration case)
            // only if caster is Player (after patch 2.4.2)
            if (GetCasterGUID().IsPlayer())
                m_target->ToPlayer()->setRegenTimerCount(1 * IN_MILLISECONDS);

            //dismount polymorphed target (after patch 2.4.2)
            if (m_target->IsMounted())
                m_target->RemoveAurasByType(SPELL_AURA_MOUNTED);
        }
    }
    else
    {
        // HandleEffect(this, AURA_EFFECT_HANDLE_SEND_FOR_CLIENT, true) will reapply it if need
        if (m_target->GetTransForm() == GetId())
            m_target->SetTransForm(0);

        m_target->RestoreDisplayId();

        // Dragonmaw Illusion (restore mount model)
        if (GetId() == 42016 && m_target->GetMountID() == 16314)
        {
            if (!m_target->GetAuraEffectsByType(SPELL_AURA_MOUNTED).empty())
            {
                uint32 cr_id = m_target->GetAuraEffectsByType(SPELL_AURA_MOUNTED).front()->GetMiscValue();
                if (CreatureTemplate const* ci = sObjectMgr->GetCreatureTemplate(cr_id))
                {
                    uint32 team = 0;
                    if (m_target->GetTypeId() == TYPEID_PLAYER)
                        team = (m_target->ToPlayer())->GetTeam();

                    uint32 display_id = ObjectMgr::ChooseDisplayId(ci);
                    sObjectMgr->GetCreatureModelRandomGender(display_id);

                    m_target->SetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID, display_id);
                }
            }
        }
    }

    m_target->GetThreatManager().UpdateOnlineStates(true, false);
}

void AuraEffect::HandleForceReaction(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK))
        return;

    Unit* m_target = aurApp->GetTarget();
    if (m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    Player* player = m_target->ToPlayer();

    uint32 faction_id = GetMiscValue();
    uint32 faction_rank = _amount;

    if (apply)
        player->m_forcedReactions[faction_id] = ReputationRank(faction_rank);
    else
        player->m_forcedReactions.erase(faction_id);

    WorldPacket data;
    data.Initialize(SMSG_SET_FORCED_REACTIONS, 4 + player->m_forcedReactions.size()*(4 + 4));
    data << uint32(player->m_forcedReactions.size());
    for (ForcedReactions::const_iterator itr = player->m_forcedReactions.begin(); itr != player->m_forcedReactions.end(); ++itr)
    {
        data << uint32(itr->first);                         // faction_id (Faction.dbc)
        data << uint32(itr->second);                        // reputation rank
    }
    player->SendDirectMessage(&data);
}

void AuraEffect::HandleAuraModSkill(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_SKILL)))
        return;

    Unit* m_target = aurApp->GetTarget();
    if (m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    uint32 prot = GetMiscValue();
    int32 points = GetAmount();

    (m_target->ToPlayer())->ModifySkillBonus(prot, (apply ? points : -points), GetAuraType() == SPELL_AURA_MOD_SKILL_TALENT);
    if (prot == SKILL_DEFENSE)
        (m_target->ToPlayer())->UpdateDefenseBonusesMod();
}

void AuraEffect::HandleChannelDeathItem(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    if (apply || aurApp->GetRemoveMode() != AURA_REMOVE_BY_DEATH)
        return;

    Unit* caster = GetCaster();
    Player* plCaster = caster->ToPlayer();
    Unit* victim = aurApp->GetTarget();
    if (!caster || caster->GetTypeId() != TYPEID_PLAYER || !victim)
        return;

    //we cannot check removemode = death
    //talent will remove the caster's aura->interrupt channel->remove victim aura
    //if (victim->GetHealth() > 0)
        //return;

    SpellInfo const *spellInfo = GetSpellInfo();
    if (spellInfo->Effects[m_effIndex].ItemType == 0)
        return;

    Creature *cr = victim->ToCreature();

    // Soul Shard only from non-grey units
    if (spellInfo->Effects[m_effIndex].ItemType == 6265 && plCaster)
    {
        if (!plCaster->IsHonorOrXPTarget(victim) ||
            (cr && !cr->isTappedBy(plCaster)))
            return;
    }

    ItemPosCountVec dest;
    uint8 msg = (caster->ToPlayer())->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, spellInfo->Effects[m_effIndex].ItemType, 1);
    if (msg != EQUIP_ERR_OK)
    {
        (caster->ToPlayer())->SendEquipError(msg, nullptr, nullptr);
        return;
    }

    Item* newitem = (caster->ToPlayer())->StoreNewItem(dest, spellInfo->Effects[m_effIndex].ItemType, true);
    (caster->ToPlayer())->SendNewItem(newitem, 1, true, false);
}

void AuraEffect::HandleBindSight(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();
    Unit* caster = GetCaster();
    if (!caster || caster->GetTypeId() != TYPEID_PLAYER)
        return;

    caster->ToPlayer()->SetViewpoint(target, apply);
}

void AuraEffect::HandleAuraTrackCreatures(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_SEND_FOR_CLIENT_MASK))
        return;

    Unit* m_target = aurApp->GetTarget();
    if (m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    if (apply)
        m_target->SetFlag(PLAYER_TRACK_CREATURES, uint32(1) << (GetMiscValue() - 1));
    else
        m_target->RemoveFlag(PLAYER_TRACK_CREATURES, uint32(1) << (GetMiscValue() - 1));
}

void AuraEffect::HandleAuraTrackResources(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_SEND_FOR_CLIENT_MASK))
        return;

    Unit* m_target = aurApp->GetTarget();
    if (m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    if (apply)
        m_target->SetFlag(PLAYER_TRACK_RESOURCES, uint32(1) << (GetMiscValue() - 1));
    else
        m_target->RemoveFlag(PLAYER_TRACK_RESOURCES, uint32(1) << (GetMiscValue() - 1));
}

void AuraEffect::HandleAuraTrackStealthed(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_SEND_FOR_CLIENT_MASK))
        return;

    Unit* m_target = aurApp->GetTarget();
    if (m_target->GetTypeId() == TYPEID_PLAYER)
    {
        if ((mode & AURA_EFFECT_HANDLE_REAL))
            m_target->UpdateObjectVisibility(); //update current vision
    }
    else {
        return;
    }

    if (!apply)
    {
        // do not remove unit flag if there are more than this auraEffect of that kind on unit on unit
        if (m_target->HasAuraType(GetAuraType()))
            return;
    }

    m_target->ApplyModFlag(PLAYER_FIELD_BYTES, PLAYER_FIELD_BYTE_TRACK_STEALTHED, apply);
}

void AuraEffect::HandleModStealthLevel(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK))
        return;

    Unit* target = aurApp->GetTarget();
    StealthType type = StealthType(GetMiscValue());

    //TC has not *5 here. Here I assume having 1 "stealth level" is being considered 1 level higher on stealth detection calculation
    //Improved stealth for rogue and druid add 15 yards in total (15 points). Items with Increases your effective stealth level by 1 have 5.

    if (apply)
        target->m_stealth.AddValue(type, GetAmount() * 5);
    else
        target->m_stealth.AddValue(type, -GetAmount() * 5);

    target->UpdateObjectVisibility(); //update visibility for nearby units
}

void AuraEffect::HandleAuraModScale(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_CHANGE_AMOUNT_SEND_FOR_CLIENT_MASK))
        return;

    Unit* m_target = aurApp->GetTarget();
    float scale = m_target->GetObjectScale();
    ApplyPercentModFloatVar(scale, float(GetAmount()), apply);
    m_target->SetObjectScale(scale);
}

void AuraEffect::HandleModPossess(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* caster = GetCaster();
    if (caster && caster->GetTypeId() == TYPEID_UNIT)
    {
        HandleModCharm(aurApp, mode, apply);
        return;
    }

    Unit* m_target = aurApp->GetTarget();
    if (apply)
    {
        if (m_target->GetLevel() > _amount)
            return;

        if (m_target == caster)
            TC_LOG_ERROR("spells", "HandleModPossess: unit %s (typeId %u, entry %u) tried to charm itself", caster->GetGUID().ToString().c_str(), caster->GetTypeId(), caster->GetEntry());
        else
            m_target->SetCharmedBy(caster, CHARM_TYPE_POSSESS, aurApp);
    }
    else
    {
        m_target->RemoveCharmedBy(caster);

        // Spiritual Vengeance
        if (GetId() == 40268)
        {
            m_target->RemoveAurasDueToSpell(40282);
            m_target->SetDeathState(JUST_DIED);
            if (m_target->GetTypeId() == TYPEID_UNIT)
                (m_target->ToCreature())->RemoveCorpse();
            if (caster)
                caster->SetDeathState(JUST_DIED);
        }
    }
}

void AuraEffect::HandleModPossessPet(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* caster = GetCaster();
    if (!caster || caster->GetTypeId() != TYPEID_PLAYER)
        return;

    Unit* m_target = aurApp->GetTarget();
    if (m_target->GetTypeId() != TYPEID_UNIT || !m_target->IsPet())
        return;

    Pet* pet = m_target->ToPet();

    if (apply)
    {
        if (caster->GetPet() != pet)
            return;

        // Must clear current motion or pet leashes back to owner after a few yards
        //  when under spell 'Eyes of the Beast'
        pet->GetMotionMaster()->Clear();
        pet->SetCharmedBy(caster, CHARM_TYPE_POSSESS, aurApp);
    }
    else
    {
        pet->RemoveCharmedBy(caster);

        if (!pet->IsWithinDistInMap(caster, pet->GetMap()->GetVisibilityRange()))
            pet->Remove(PET_SAVE_NOT_IN_SLOT, true);
        else
        {
            // Reinitialize the pet bar and make the pet come back to the owner
            (caster->ToPlayer())->PetSpellInitialize();

            // Follow owner only if not fighting or owner didn't click "stay" at new location
            // This may be confusing because pet bar shows "stay" when under the spell but it retains
            //  the "follow" flag. Player MUST click "stay" while under the spell.
            if (!pet->GetVictim() && !pet->GetCharmInfo()->HasCommandState(COMMAND_STAY))
            {
                pet->GetMotionMaster()->MoveFollow(caster, PET_FOLLOW_DIST, pet->GetFollowAngle());
            }
        }
    }
}

void AuraEffect::HandleModCharm(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* caster = GetCaster();
    Unit* m_target = aurApp->GetTarget();
    if (apply)
    {
        if (int32(m_target->GetLevel()) > _amount)
            return;

        if (GetId() == 1098 || GetId() == 11725 || GetId() == 11726)
        {
            if (m_target->ToCreature()->IsPet())
                return;
        }

        m_target->SetCharmedBy(caster, CHARM_TYPE_CHARM, aurApp);
    }
    else
        m_target->RemoveCharmedBy(caster);
}

void AuraEffect::HandleModConfuse(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* m_target = aurApp->GetTarget();
    m_target->SetControlled(apply, UNIT_STATE_CONFUSED);
    m_target->GetThreatManager().UpdateOnlineStates(true, false);
}

void AuraEffect::HandleModFear(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* m_target = aurApp->GetTarget();
    // HACK / Archimonde: if player has Air Burst, don't apply fear
    if (apply && m_target->HasAura(32014))
        return;

    m_target->SetControlled(apply, UNIT_STATE_FLEEING);
    m_target->GetThreatManager().UpdateOnlineStates(true, false);
}

void AuraEffect::HandleFeignDeath(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* m_target = aurApp->GetTarget();
    if (m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    m_target->ResetFeignDeathDetected();
    if (apply)
    {
        //code adapted from CMangos
        bool disengage = true;
        if (apply && m_target->GetCharmerOrOwnerPlayerOrPlayerItself())
        {
            const Unit::AttackerSet& attackers = m_target->GetAttackers();
            for (Unit::AttackerSet::const_iterator itr = attackers.begin(); itr != attackers.end(); ++itr)
            {
                Unit* opponent = (*itr);
                if (opponent && !opponent->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED))
                {
                    if (m_target->MagicSpellHitResult(opponent, GetSpellInfo()) != SPELL_MISS_NONE)
                    {
                        m_target->FeignDeathDetected(opponent);
                        // m_target->SendSpellMiss(opponent, GetSpellInfo()->Id, SPELL_MISS_RESIST); //does not work, need fix
                        disengage = false;
                        break;
                    }
                }
            }
        }

        if (disengage)
        {
            std::list<Unit*> targets;
            Trinity::AnyUnfriendlyUnitInObjectRangeCheck u_check(m_target, m_target, m_target->GetMap()->GetVisibilityRange());
            Trinity::UnitListSearcher<Trinity::AnyUnfriendlyUnitInObjectRangeCheck> searcher(m_target, targets, u_check);
            Cell::VisitAllObjects(m_target, searcher, m_target->GetMap()->GetVisibilityRange());

            /* first pass, interrupt spells and check for units attacking the misdirection target */
            for (auto & target : targets)
            {
                if (!target->HasUnitState(UNIT_STATE_CASTING))
                    continue;

                for (uint32 i = CURRENT_FIRST_NON_MELEE_SPELL; i < CURRENT_MAX_SPELL; i++)
                {
                    if (target->GetCurrentSpell(i)
                        && target->GetCurrentSpell(i)->m_targets.GetUnitTargetGUID() == m_target->GetGUID())
                    {
                        target->InterruptSpell(CurrentSpellTypes(i), false);
                    }
                }
            }

            if (m_target->GetMap()->IsDungeon()) // feign death does not remove combat in dungeons
            {
                m_target->AttackStop();
                if (Player* targetPlayer = m_target->ToPlayer())
                    targetPlayer->SendAttackSwingCancelAttack();
            }
            else
                m_target->CombatStop(false, false);

            if (Creature* creature = m_target->ToCreature())
                creature->SetReactState(REACT_PASSIVE);
        }
        else 
        {
            if (Player* p = m_target->GetCharmerOrOwnerPlayerOrPlayerItself())
                p->SendFeignDeathResisted();

            if (m_target->GetTypeId() == TYPEID_PLAYER)
                static_cast<Player*>(m_target)->SendAttackSwingCancelAttack();

            m_target->AttackStop();
        }

        m_target->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_IMMUNE_OR_LOST_SELECTION); //drop flag in bg

                                                                                               // prevent interrupt message
        if (GetBase()->GetCasterGUID() == m_target->GetGUID() && m_target->GetCurrentSpell(CURRENT_GENERIC_SPELL))
            m_target->GetCurrentSpell(CURRENT_GENERIC_SPELL)->finish();
        m_target->InterruptNonMeleeSpells(true);

        // stop handling the effect if it was removed by linked event
        if (aurApp->GetRemoveMode())
            return;

        m_target->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNK_29);  // blizz like 2.0.x
        m_target->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH);   // blizz like 2.0.x
        m_target->SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_DEAD); // blizz like 2.0.x
        m_target->AddUnitState(UNIT_STATE_DIED);
    }
    else
    {
        m_target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNK_29); // blizz like 2.0.x
        m_target->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH);  // blizz like 2.0.x
        m_target->RemoveFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_DEAD);  // blizz like 2.0.x
        m_target->ClearUnitState(UNIT_STATE_DIED);

        //reset react state
        if (Creature* creature = m_target->ToCreature())
            creature->InitializeReactState();

        if (Map* map = m_target->GetMap()) {
            if (m_target->ToPlayer()) {
                float x, y, z;
                m_target->GetPosition(x, y, z);
                map->PlayerRelocation(m_target->ToPlayer(), x, y, z, m_target->GetOrientation());
            }
        }
    }
    m_target->GetThreatManager().UpdateOnlineStates(true, false);
}

void AuraEffect::HandleAuraModDisarm(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* m_target = aurApp->GetTarget();
    if (!apply && m_target->HasAuraType(SPELL_AURA_MOD_DISARM))
        return;

    // not sure for it's correctness
    if (apply)
        m_target->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISARMED);
    else {
        if (m_target->HasAuraType(GetAuraType()))
            return;
        m_target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISARMED);
    }

    if (m_target->GetTypeId() == TYPEID_PLAYER)
    {
        Player* player = m_target->ToPlayer();
        // main-hand attack speed already set to special value for feral form already and don't must change and reset at remove.
        if (player->IsInFeralForm())
            return;

        if (Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND))
        {
            WeaponAttackType const attackType = Player::GetAttackBySlot(EQUIPMENT_SLOT_MAINHAND);
#ifdef LICH_KING
            HandleOffHand  & ranged
#endif

                player->ApplyItemDependentAuras(item, !apply);
            if (attackType != MAX_ATTACK)
            {
                player->_ApplyWeaponDamage(EQUIPMENT_SLOT_MAINHAND, item->GetTemplate(), /*NULL,*/ !apply);
                if (!apply) // apply case already handled on item dependent aura removal (if any)
                    player->UpdateWeaponDependentAuras(attackType);
            }
        }

        if (apply)
            m_target->SetAttackTime(BASE_ATTACK, BASE_ATTACK_TIME);
        else
            (m_target->ToPlayer())->SetRegularAttackTime();
    }
    else
    {
        // creature does not have equipment
        if (apply && !(m_target->ToCreature())->GetCurrentEquipmentId())
            return;
    }

    m_target->UpdateDamagePhysical(BASE_ATTACK);
}

void AuraEffect::HandleAuraModStun(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* m_target = aurApp->GetTarget();
    if (Unit *caster = GetCaster()) {
        // Handle Prohibit school effect before applying stun, or m_target is not casting anymore and prohibit fails
        if (GetId() == 22570 && apply && caster->HasAura(44835))
            caster->CastSpell(m_target, 32747, true);
    }

    if (!apply && m_target->HasAuraType(GetAuraType()))
        return;

    m_target->SetControlled(apply, UNIT_STATE_STUNNED);
    m_target->GetThreatManager().UpdateOnlineStates(true, false);
}

void AuraEffect::HandleModStealthDetect(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK))
        return;

    Unit* target = aurApp->GetTarget();
    StealthType type = StealthType(GetMiscValue());

    if (apply)
    {
        target->m_stealthDetect.AddFlag(type);
        target->m_stealthDetect.AddValue(type, GetAmount());
    }
    else
    {
        if (!target->HasAuraType(SPELL_AURA_MOD_STEALTH_DETECT))
            target->m_stealthDetect.DelFlag(type);

        target->m_stealthDetect.AddValue(type, -GetAmount());
    }

    // call functions which may have additional effects after chainging state of unit
    target->UpdateObjectVisibility();
}

void AuraEffect::HandleModStealth(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_CHANGE_AMOUNT_SEND_FOR_CLIENT_MASK))
        return;

    Unit* target = aurApp->GetTarget();
    StealthType type = StealthType(GetMiscValue());

    if (apply)
    {
        if (target->GetTypeId() == TYPEID_PLAYER)
        {
            target->m_stealth.AddFlag(type);
            target->m_stealth.AddValue(type, GetAmount());

            target->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_IMMUNE_OR_LOST_SELECTION);  // drop flag at stealth in bg
        }

        target->SetStandFlags(UNIT_STAND_FLAGS_CREEP);
        if (target->GetTypeId() == TYPEID_PLAYER)
            target->SetByteFlag(PLAYER_FIELD_BYTES2, PLAYER_FIELD_BYTES_2_OFFSET_AURA_VISION, PLAYER_FIELD_BYTE2_STEALTH);
    }
    else
    {
        target->m_stealth.AddValue(type, -GetAmount());

        // if last SPELL_AURA_MOD_STEALTH and no GM invisibility
        if (!target->HasAuraType(SPELL_AURA_MOD_STEALTH))
        {
            target->m_stealth.DelFlag(type);

            target->RemoveStandFlags(UNIT_STAND_FLAGS_CREEP);
            if (target->GetTypeId() == TYPEID_PLAYER)
                target->RemoveByteFlag(PLAYER_FIELD_BYTES2, PLAYER_FIELD_BYTES_2_OFFSET_AURA_VISION, PLAYER_FIELD_BYTE2_STEALTH);
        }
    }

    target->UpdateObjectVisibility(); //update visibility for nearby units

    // Master of Subtlety HACK
    Unit::AuraEffectList const& mDummyAuras = target->GetAuraEffectsByType(SPELL_AURA_DUMMY);
    for (auto mDummyAura : mDummyAuras)
    {
        if (mDummyAura->GetSpellInfo()->SpellIconID == 2114)
        {
            if (apply)
            {
                int32 bp = mDummyAura->GetAmount();
                CastSpellExtraArgs args;
                args.TriggerFlags = TRIGGERED_FULL_MASK;
                args.AddSpellBP0(int32(bp));
                target->CastSpell(target, 31665, args);
            }
            else
                target->CastSpell(target, 31666, true);
            break;
        }
    }
}

void AuraEffect::HandleModInvisibility(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_CHANGE_AMOUNT_SEND_FOR_CLIENT_MASK))
        return;

    InvisibilityType type = InvisibilityType(GetMiscValue());

    Unit* m_target = aurApp->GetTarget();
    if (apply)
    {
        //        m_target->m_invisibilityMask |= (1 << GetMiscValue());

        m_target->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_IMMUNE_OR_LOST_SELECTION); // also drop flag at invisibiliy in bg

        if ((mode & AURA_EFFECT_HANDLE_REAL) && m_target->GetTypeId() == TYPEID_PLAYER)
            m_target->SetByteFlag(PLAYER_FIELD_BYTES2, PLAYER_FIELD_BYTES_2_OFFSET_AURA_VISION, PLAYER_FIELD_BYTE2_INVISIBILITY_GLOW);

        m_target->m_invisibility.AddFlag(type);
        m_target->m_invisibility.AddValue(type, GetAmount());
    }
    else
    {
        if (!m_target->HasAuraType(SPELL_AURA_MOD_INVISIBILITY))
        {
            // if not have different invisibility auras.
            // remove glow vision
            if ((mode & AURA_EFFECT_HANDLE_REAL) && m_target->GetTypeId() == TYPEID_PLAYER)
                m_target->RemoveByteFlag(PLAYER_FIELD_BYTES2, PLAYER_FIELD_BYTES_2_OFFSET_AURA_VISION, PLAYER_FIELD_BYTE2_INVISIBILITY_GLOW);

            m_target->m_invisibility.DelFlag(type);
        }
        else {
            bool found = false;
            Unit::Unit::AuraEffectList const& invisAuras = m_target->GetAuraEffectsByType(SPELL_AURA_MOD_INVISIBILITY);
            for (Unit::Unit::AuraEffectList::const_iterator i = invisAuras.begin(); i != invisAuras.end(); ++i)
            {
                if (GetMiscValue() == (*i)->GetMiscValue())
                {
                    found = true;
                    break;
                }
            }
            if (!found)
                m_target->m_invisibility.DelFlag(type);
        }

        m_target->m_invisibility.AddValue(type, -GetAmount());
    }

    if (apply && (mode & AURA_EFFECT_HANDLE_REAL))
        m_target->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_IMMUNE_OR_LOST_SELECTION);

    m_target->UpdateObjectVisibility();

    // recalculate value at modifier remove (current aura already removed)
    /*
    m_target->m_invisibilityMask = 0;
    Unit::AuraEffectList const& auras = m_target->GetAuraEffectsByType(SPELL_AURA_MOD_INVISIBILITY);
    for(auto itr = auras.begin(); itr != auras.end(); ++itr)
    m_target->m_invisibilityMask |= (1 << GetMiscValue());
    */
    // only at real aura remove and if not have different invisibility auras.
}

void AuraEffect::HandleModInvisibilityDetect(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK))
        return;

    InvisibilityType type = InvisibilityType(GetMiscValue());

    Unit* m_target = aurApp->GetTarget();
    if (apply)
    {
        m_target->m_invisibilityDetect.AddFlag(type);
        m_target->m_invisibilityDetect.AddValue(type, GetAmount());
    }
    else
    {
        if (!m_target->HasAuraType(SPELL_AURA_MOD_INVISIBILITY_DETECTION))
            m_target->m_invisibilityDetect.DelFlag(type);

        m_target->m_invisibilityDetect.AddValue(type, -GetAmount());
    }

    // call functions which may have additional effects after chainging state of unit
    m_target->UpdateObjectVisibility();
}

void AuraEffect::HandleAuraModRoot(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    // only at real add/remove aura
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* m_target = aurApp->GetTarget();
    if (!apply && m_target->HasAuraType(GetAuraType()))
        return;

    m_target->SetControlled(apply, UNIT_STATE_ROOT);
    m_target->GetThreatManager().UpdateOnlineStates(true, false);
}

void AuraEffect::HandleAuraModSilence(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    // only at real add/remove aura
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* m_target = aurApp->GetTarget();
    if (apply)
    {
        m_target->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SILENCED);
        // Stop cast only spells vs PreventionType == SPELL_PREVENTION_TYPE_SILENCE
        for (uint32 i = CURRENT_MELEE_SPELL; i < CURRENT_MAX_SPELL; i++)
        {
            Spell* currentSpell = m_target->m_currentSpells[i];
            if (currentSpell && currentSpell->m_spellInfo->PreventionType == SPELL_PREVENTION_TYPE_SILENCE)
            {
                uint32 state = currentSpell->getState();
                // Stop spells on prepare or casting state
                if (state == SPELL_STATE_PREPARING || state == SPELL_STATE_CASTING)
                {
                    currentSpell->cancel();
                }
            }
        }

        switch (GetId())
        {
            // Arcane Torrent (Energy)
        case 25046:
        {
            Unit * caster = GetCaster();
            if (!caster)
                return;

            // Search Mana Tap auras on caster
            AuraEffect* dummy = caster->GetDummyAura(28734);
            if (dummy)
            {
                int32 bp = dummy->GetBase()->GetStackAmount() * 10;
                CastSpellExtraArgs args;
                args.TriggerFlags = TRIGGERED_FULL_MASK;
                args.AddSpellBP0(int32(bp));
                m_target->CastSpell(m_target, 25048, args);
                caster->RemoveAurasDueToSpell(28734);
            }
        }
        }
    }
    else
    {
        // Real remove called after current aura remove from lists, check if other similar auras active
        if (m_target->HasAuraType(SPELL_AURA_MOD_SILENCE))
            return;

        m_target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SILENCED);
    }
}

void AuraEffect::HandleModThreat(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK))
        return;

    aurApp->GetTarget()->GetThreatManager().UpdateMySpellSchoolModifiers();
}

void AuraEffect::HandleAuraModTotalThreat(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK))
        return;

    Unit* m_target = aurApp->GetTarget();
    if (!m_target->IsAlive() || m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    Unit* caster = GetCaster();
    if (caster || caster->IsAlive())
        caster->GetThreatManager().UpdateMyTempModifiers();
}

void AuraEffect::HandleModTaunt(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    // only at real add/remove aura
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* m_target = aurApp->GetTarget();
    if (!m_target->IsAlive() || !m_target->CanHaveThreatList())
        return;

    m_target->GetThreatManager().TauntUpdate();
}

void AuraEffect::HandleModDetaunt(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* m_target = aurApp->GetTarget();
    if (!m_target)
        return;

    Unit* caster = GetCaster();
    if (!caster || !caster->IsAlive() || !m_target->IsAlive() || !caster->CanHaveThreatList())
        return;

    caster->GetThreatManager().TauntUpdate();
}

/*********************************************************/
/***                  MODIFY SPEED                     ***/
/*********************************************************/
void AuraEffect::HandleAuraModIncreaseSpeed(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK))
        return;

    Unit* m_target = aurApp->GetTarget();
    m_target->UpdateSpeed(MOVE_RUN);
}

void AuraEffect::HandleAuraModIncreaseMountedSpeed(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    HandleAuraModIncreaseSpeed(aurApp, mode, apply);
}

void AuraEffect::HandleAuraModIncreaseFlightSpeed(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_CHANGE_AMOUNT_SEND_FOR_CLIENT_MASK))
        return;

    Unit* m_target = aurApp->GetTarget();
    // Enable Fly mode for flying mounts
    if (GetAuraType() == SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED)
    {
        WorldPacket data;
        if (apply)
        {
            ((Player*)m_target)->SetFlying(true);
            data.Initialize(SMSG_MOVE_SET_CAN_FLY, 12);
        }
        else
        {
            data.Initialize(SMSG_MOVE_UNSET_CAN_FLY, 12);
            ((Player*)m_target)->SetFlying(false);
        }

        data << m_target->GetPackGUID();
        data << uint32(0);                                      // unknown
        m_target->SendMessageToSet(&data, true);

        //Players on flying mounts must be immune to polymorph
        if (m_target->GetTypeId() == TYPEID_PLAYER)
            m_target->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_POLYMORPH, apply);

        // Dragonmaw Illusion (overwrite mount model, mounted aura already applied)
        if (apply && m_target->HasAura(42016) && m_target->GetMountID())
            m_target->SetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID, 16314);
    }

    m_target->UpdateSpeed(MOVE_FLIGHT);
}

void AuraEffect::HandleAuraModIncreaseSwimSpeed(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK))
        return;

    Unit* m_target = aurApp->GetTarget();
    m_target->UpdateSpeed(MOVE_SWIM);
}

void AuraEffect::HandleAuraModDecreaseSpeed(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK))
        return;

    Unit* m_target = aurApp->GetTarget();
    m_target->UpdateSpeed(MOVE_RUN);
    m_target->UpdateSpeed(MOVE_SWIM);
    m_target->UpdateSpeed(MOVE_FLIGHT);
    m_target->UpdateSpeed(MOVE_RUN_BACK);
    m_target->UpdateSpeed(MOVE_SWIM_BACK);
    m_target->UpdateSpeed(MOVE_FLIGHT_BACK);
}

void AuraEffect::HandleAuraModUseNormalSpeed(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    // all applied/removed only at real aura add/remove
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* m_target = aurApp->GetTarget();
    m_target->UpdateSpeed(MOVE_RUN);
    m_target->UpdateSpeed(MOVE_SWIM);
    m_target->UpdateSpeed(MOVE_FLIGHT);
}

/*********************************************************/
/***                     IMMUNITY                      ***/
/*********************************************************/

void AuraEffect::HandleModMechanicImmunity(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* m_target = aurApp->GetTarget();
    GetSpellInfo()->ApplyAllSpellImmunitiesTo(m_target, GetEffIndex(), apply);

    //HACKS TIME
    // Bestial Wrath
    if (GetSpellInfo()->SpellFamilyName == SPELLFAMILY_HUNTER && GetSpellInfo()->Id == 19574)
    {
        // The Beast Within cast on owner if talent present
        if (Unit* owner = m_target->GetOwner())
        {
            // Search talent
            Unit::AuraEffectList const& m_dummyAuras = owner->GetAuraEffectsByType(SPELL_AURA_DUMMY);
            for (auto m_dummyAura : m_dummyAuras)
            {
                if (m_dummyAura->GetSpellInfo()->SpellIconID == 2229)
                {
                    if (apply)
                        owner->CastSpell(owner, 34471, this);
                    else
                        owner->RemoveAurasDueToSpell(34471);
                    break;
                }
            }
        }
    }

    // The Beast Within and Bestial Wrath - immunity
    if (GetId() == 19574 || GetId() == 34471)
    {
        if (apply)
        {
            m_target->CastSpell(m_target, 24395, true);
            m_target->CastSpell(m_target, 24396, true);
            m_target->CastSpell(m_target, 24397, true);
            m_target->CastSpell(m_target, 26592, true);
        }
        else
        {
            m_target->RemoveAurasDueToSpell(24395);
            m_target->RemoveAurasDueToSpell(24396);
            m_target->RemoveAurasDueToSpell(24397);
            m_target->RemoveAurasDueToSpell(26592);
        }
    }
}

void AuraEffect::HandleAuraModEffectImmunity(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* m_target = aurApp->GetTarget();
    GetSpellInfo()->ApplyAllSpellImmunitiesTo(m_target, GetEffIndex(), apply);

    // when removing flag aura, handle flag drop
    Player* player = m_target->ToPlayer();
    if (!apply && player && (GetSpellInfo()->AuraInterruptFlags & AURA_INTERRUPT_FLAG_IMMUNE_OR_LOST_SELECTION))
    {
        if (player->InBattleground())
        {
            if (Battleground* bg = player->GetBattleground())
                bg->EventPlayerDroppedFlag(player);
        }
        else
            sOutdoorPvPMgr->HandleDropFlag(player, GetSpellInfo()->Id);
    }
}

void AuraEffect::HandleAuraModStateImmunity(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;
    Unit* m_target = aurApp->GetTarget();
    GetSpellInfo()->ApplyAllSpellImmunitiesTo(m_target, GetEffIndex(), apply);
}

void AuraEffect::HandleAuraModSchoolImmunity(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* m_target = aurApp->GetTarget();
    GetSpellInfo()->ApplyAllSpellImmunitiesTo(m_target, GetEffIndex(), apply);

    if (apply && GetMiscValue() == SPELL_SCHOOL_MASK_NORMAL)
        m_target->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_IMMUNE_OR_LOST_SELECTION); //drop flag in bg

    if (GetSpellInfo()->Mechanic == MECHANIC_BANISH)
    {
        if (apply)
            m_target->AddUnitState(UNIT_STATE_ISOLATED);
        else
        {   //remove the state if there isn't any other banish aura left
            bool banishFound = false;
            auto auras = m_target->GetAuraEffectsByType(GetAuraType());
            for (auto aura : auras)
            {
                if (aura->GetSpellInfo()->Mechanic == MECHANIC_BANISH)
                {
                    banishFound = true;
                    break;
                }
            }

            if (!banishFound)
                m_target->ClearUnitState(UNIT_STATE_ISOLATED);
        }
    }

    if (apply && GetMiscValue() == SPELL_SCHOOL_MASK_NORMAL)
        m_target->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_IMMUNE_OR_LOST_SELECTION);

    // remove all flag auras (they are positive, but they must be removed when you are immune)
    if (GetSpellInfo()->HasAttribute(SPELL_ATTR1_DISPEL_AURAS_ON_IMMUNITY)
        && GetSpellInfo()->HasAttribute(SPELL_ATTR2_DAMAGE_REDUCED_SHIELD))
        m_target->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_IMMUNE_OR_LOST_SELECTION);

    /*
    if ((mode & AURA_EFFECT_HANDLE_REAL) && apply && GetSpellInfo()->HasAttribute(SPELL_ATTR1_DISPEL_AURAS_ON_IMMUNITY))
    {
        bool hostileTarget = GetCaster() ? GetCaster()->IsFriendlyTo(m_target) : false;
        if (GetSpellInfo()->IsPositive(hostileTarget))                        //Only positive immunity removes auras
        {
            uint32 school_mask = GetMiscValue();
            Unit::AuraMap& Auras = m_target->GetAuras();
            for (Unit::AuraMap::iterator iter = Auras.begin(), next; iter != Auras.end(); iter = next)
            {
                next = iter;
                ++next;
                SpellInfo const *spell = iter->second->GetSpellInfo();
                if ((spell->GetSchoolMask() & school_mask)  //Check for school mask
                    && !(spell->Attributes & SPELL_ATTR0_UNAFFECTED_BY_INVULNERABILITY)   //Spells unaffected by invulnerability
                    && !iter->second->IsPositive()          //Don't remove positive spells
                    && !spell->IsPassive()                  //Don't remove passive auras
                    && spell->Id != GetId()                 //Don't remove self
                    && spell->Id != 12042                   //Don't remove Arcane Power, don't know why it got removed...
                    && spell->Id != 37441                   // Improved Arcane Blast
                    && spell->Id != 16067                   // Arcane Blast (all ranks)
                    && spell->Id != 18091
                    && spell->Id != 20883
                    && spell->Id != 30451
                    && spell->Id != 35927
                    && spell->Id != 36032
                    && spell->Id != 38881
                    && spell->Id != 33786)
                {
                    m_target->RemoveAurasDueToSpell(spell->Id);
                    if (Auras.empty())
                        break;
                    else
                        next = Auras.begin();
                }
            }
        }
    }*/
    m_target->GetThreatManager().UpdateOnlineStates(true, false);
}

void AuraEffect::HandleAuraModDmgImmunity(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;
    Unit* m_target = aurApp->GetTarget();
    GetSpellInfo()->ApplyAllSpellImmunitiesTo(m_target, GetEffIndex(), apply);
    m_target->GetThreatManager().UpdateOnlineStates(true, false);
}

void AuraEffect::HandleAuraModDispelImmunity(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    // all applied/removed only at real aura add/remove
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* target = aurApp->GetTarget();
    GetSpellInfo()->ApplyAllSpellImmunitiesTo(target, GetEffIndex(), apply);
}

void AuraEffect::HandleAuraModStalked(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_SEND_FOR_CLIENT_MASK))
        return;

    Unit* m_target = aurApp->GetTarget();
    // used by spells: Hunter's Mark, Mind Vision, Syndicate Tracker (MURP) DND
    if (apply)
        m_target->SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_TRACK_UNIT);
    else {
        // do not remove unit flag if there are more than this auraEffect of that kind on unit on unit
        if (!m_target->HasAuraType(GetAuraType()))
            m_target->RemoveFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_TRACK_UNIT);
    }
}

/*********************************************************/
/***                   PERIODIC                        ***/
/*********************************************************/

void AuraEffect::HandlePeriodicTriggerSpell(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    // all applied/removed only at real aura add/remove
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* m_target = aurApp->GetTarget();

    //hack time, todo: move to scripts
    {
        // Curse of the Plaguebringer
        if (!apply && m_spellInfo->Id == 29213 && aurApp->GetRemoveMode() != AURA_REMOVE_BY_ENEMY_SPELL)
            // Cast Wrath of the Plaguebringer if not dispelled
            m_target->CastSpell(m_target, 29214, this);

        // Wrath of the Astromancer
        else if (!apply && m_spellInfo->Id == 42783)
            m_target->CastSpell(m_target, 42787, this);
        // Murmur's Touch (Shockwave)
        else if (!apply && m_spellInfo->Id == 38794)
            m_target->CastSpell(m_target, 33686, this);
    }
}

void AuraEffect::HandlePeriodicEnergize(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    Unit* m_target = aurApp->GetTarget();
    if ((mode & AURA_EFFECT_HANDLE_REAL))
    {
        //Todo hack: move to script
        if (m_spellInfo->Id == 5229)
            m_target->UpdateArmor();
    }
}

void AuraEffect::HandlePeriodicHeal(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    Unit* m_target = aurApp->GetTarget();
    // only at real apply
    if ((mode & AURA_EFFECT_HANDLE_REAL) && apply && GetSpellInfo()->Mechanic == MECHANIC_BANDAGE)
    {
        //todo hack, move to script
        // provided m_target as original caster to prevent apply aura caster selection for this negative buff
        CastSpellExtraArgs args;
        args.TriggerFlags = TRIGGERED_FULL_MASK;
        args.SetTriggeringAura(this);
        args.SetOriginalCaster(m_target->GetGUID());
        m_target->CastSpell(m_target, 11196, args);
    }
}

void AuraEffect::HandlePeriodicDamage(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    // spells required only Real aura add/remove
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* m_target = aurApp->GetTarget();
    Unit* caster = GetCaster();

    //old hacks, remove this function when they're removed. Todo: Move to scripts
    switch (m_spellInfo->SpellFamilyName)
    {
    case SPELLFAMILY_GENERIC:
    {
        if (m_spellInfo->Id == 41171) 
        {
            if (m_target && m_target->GetHealth() <= _amount)
                m_target->CastSpell(m_target, 41174, true);
        }
        // Curse of Boundless Agony (Sunwell - Kalecgos)
        else if ((m_spellInfo->Id == 45032 || m_spellInfo->Id == 45034) && !apply) 
        {
            if (caster && aurApp->GetRemoveMode() == AURA_REMOVE_BY_ENEMY_SPELL && caster->GetMapId() == 580)
                m_target->CastSpell(m_target, 45034, true);
        }
        break;
    }
    default:
        break;
    }
}

/*********************************************************/
/***                  MODIFY STATS                     ***/
/*********************************************************/

/********************************/
/***        RESISTANCE        ***/
/********************************/

void AuraEffect::HandleAuraModResistanceExclusive(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* m_target = aurApp->GetTarget();
    for (int8 x = SPELL_SCHOOL_NORMAL; x < MAX_SPELL_SCHOOL; x++)
    {
        if (GetMiscValue() & (1 << x))
        {
            int32 amount = m_target->GetMaxPositiveAuraModifierByMiscMask(SPELL_AURA_MOD_RESISTANCE_EXCLUSIVE, 1 << x, this);
            if (amount < GetAmount())
            {
                float value = float(GetAmount() - amount);
                m_target->HandleStatFlatModifier(UnitMods(UNIT_MOD_RESISTANCE_START + x), BASE_VALUE, value, apply);
                if (m_target->GetTypeId() == TYPEID_PLAYER || m_target->IsPet())
                    m_target->UpdateResistanceBuffModsMod(SpellSchools(x));
            }

        }
    }
}

void AuraEffect::HandleAuraModResistance(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;
    Unit* m_target = aurApp->GetTarget();
    for (int8 x = SPELL_SCHOOL_NORMAL; x < MAX_SPELL_SCHOOL; x++)
    {
        if (GetMiscValue() & (1 << x))
        {
            m_target->HandleStatFlatModifier(UnitMods(UNIT_MOD_RESISTANCE_START + x), TOTAL_VALUE, float(GetAmount()), apply);
            if (m_target->GetTypeId() == TYPEID_PLAYER || m_target->IsPet())
                m_target->UpdateResistanceBuffModsMod(SpellSchools(x));
        }
    }
}

void AuraEffect::HandleAuraModBaseResistancePCT(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* m_target = aurApp->GetTarget();
    // only players have base stats
    if (m_target->GetTypeId() != TYPEID_PLAYER)
    {
        //pets only have base armor
        if (m_target->IsPet() && (GetMiscValue() & SPELL_SCHOOL_MASK_NORMAL))
        {
            if (apply)
                m_target->ApplyStatPctModifier(UNIT_MOD_ARMOR, BASE_PCT, float(GetAmount()));
            else
            {
                float amount = m_target->GetTotalAuraMultiplierByMiscMask(SPELL_AURA_MOD_BASE_RESISTANCE_PCT, SPELL_SCHOOL_MASK_NORMAL);
                m_target->SetStatPctModifier(UNIT_MOD_ARMOR, BASE_PCT, amount);
            }
        }
    }
    else
    {
        for (uint8 x = SPELL_SCHOOL_NORMAL; x < MAX_SPELL_SCHOOL; ++x)
        {
            if (GetMiscValue() & (1 << x))
            {
                if (apply)
                    m_target->ApplyStatPctModifier(UnitMods(UNIT_MOD_RESISTANCE_START + x), BASE_PCT, float(GetAmount()));
                else
                {
                    float amount = m_target->GetTotalAuraMultiplierByMiscMask(SPELL_AURA_MOD_BASE_RESISTANCE_PCT, 1 << x);
                    m_target->SetStatPctModifier(UnitMods(UNIT_MOD_RESISTANCE_START + x), BASE_PCT, amount);
                }
            }
        }
    }
}

void AuraEffect::HandleModResistancePercent(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;
    Unit* m_target = aurApp->GetTarget();
    for (int8 i = SPELL_SCHOOL_NORMAL; i < MAX_SPELL_SCHOOL; i++)
    {
        if (GetMiscValue() & int32(1 << i))
        {
            float amount = m_target->GetTotalAuraMultiplierByMiscMask(SPELL_AURA_MOD_RESISTANCE_PCT, 1 << i);
            if (m_target->GetPctModifierValue(UnitMods(UNIT_MOD_RESISTANCE_START + i), TOTAL_PCT) == amount)
                continue;

            m_target->SetStatPctModifier(UnitMods(UNIT_MOD_RESISTANCE_START + i), TOTAL_PCT, amount);
            if (m_target->GetTypeId() == TYPEID_PLAYER || m_target->IsPet())
                m_target->UpdateResistanceBuffModsMod(SpellSchools(i));
        }
    }
}

void AuraEffect::HandleModBaseResistance(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;
    Unit* m_target = aurApp->GetTarget();
    // only players have base stats
    if (m_target->GetTypeId() != TYPEID_PLAYER)
    {
        //only pets have base stats
        if ((m_target->ToCreature())->IsPet() && (GetMiscValue() & SPELL_SCHOOL_MASK_NORMAL))
            m_target->HandleStatFlatModifier(UNIT_MOD_ARMOR, TOTAL_VALUE, float(GetAmount()), apply);
    }
    else
    {
        for (int i = SPELL_SCHOOL_NORMAL; i < MAX_SPELL_SCHOOL; i++)
            if (GetMiscValue() & (1 << i))
                m_target->HandleStatFlatModifier(UnitMods(UNIT_MOD_RESISTANCE_START + i), TOTAL_VALUE, float(GetAmount()), apply);
    }
}

/********************************/
/***           STAT           ***/
/********************************/

void AuraEffect::HandleAuraModStat(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;
    //-1 = all stats (such as mark of the wild)
    if (GetMiscValue() < -1 || GetMiscValue() > 4)
    {
        TC_LOG_ERROR("spells", "WARNING: Spell %u effect %u have unsupported misc value (%i) for SPELL_AURA_MOD_STAT ", GetId(), GetEffIndex(), GetMiscValue());
        return;
    }

    Unit* target = aurApp->GetTarget();
    int32 spellGroupVal = target->GetHighestExclusiveSameEffectSpellGroupValue(this, SPELL_AURA_MOD_STAT, true, GetMiscValue());
    if (abs(spellGroupVal) >= abs(GetAmount()))
        return;

    for (int32 i = STAT_STRENGTH; i < MAX_STATS; i++)
    {
        // -1 or -2 is all stats (misc < -2 checked in function beginning)
        if (GetMiscValue() < 0 || GetMiscValue() == i)
        {
            if (spellGroupVal)
            {
                target->HandleStatFlatModifier(UnitMods(UNIT_MOD_STAT_START + i), TOTAL_VALUE, float(spellGroupVal), !apply);
                if (target->GetTypeId() == TYPEID_PLAYER || target->IsPet())
                    target->UpdateStatBuffMod(Stats(i));
            }

            target->HandleStatFlatModifier(UnitMods(UNIT_MOD_STAT_START + i), TOTAL_VALUE, float(GetAmount()), apply);
            if (target->GetTypeId() == TYPEID_PLAYER || target->IsPet())
                target->UpdateStatBuffMod(Stats(i));
        }
    }
}

void AuraEffect::HandleModPercentStat(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;
    if (GetMiscValue() < -1 || GetMiscValue() > 4)
    {
        TC_LOG_ERROR("spells", "WARNING: Misc Value for SPELL_AURA_MOD_PERCENT_STAT not valid");
        return;
    }

    Unit* m_target = aurApp->GetTarget();
    // only players have base stats
    if (m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    for (int32 i = STAT_STRENGTH; i < MAX_STATS; ++i)
    {
        if (GetMiscValue() == i || GetMiscValue() == -1)
        {
            if (apply)
                m_target->ApplyStatPctModifier(UnitMods(UNIT_MOD_STAT_START + i), BASE_PCT, float(GetAmount()));
            else
            {
                float amount = m_target->GetTotalAuraMultiplier(SPELL_AURA_MOD_PERCENT_STAT, [i](AuraEffect const* aurEff) -> bool
                {
                    if (aurEff->GetMiscValue() == i || aurEff->GetMiscValue() == -1)
                        return true;
                    return false;
                });
                m_target->SetStatPctModifier(UnitMods(UNIT_MOD_STAT_START + i), BASE_PCT, amount);
            }
        }
    }
}

void AuraEffect::HandleModSpellDamagePercentFromStat(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* m_target = aurApp->GetTarget();
    if (m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    // Magic damage modifiers implemented in Unit::SpellDamageBonusDone
    // This information for client side use only
    // Recalculate bonus
    (m_target->ToPlayer())->UpdateSpellDamageAndHealingBonus();
}

void AuraEffect::HandleModSpellHealingPercentFromStat(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* m_target = aurApp->GetTarget();
    if (m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    // Recalculate bonus
    (m_target->ToPlayer())->UpdateSpellDamageAndHealingBonus();
}

void AuraEffect::HandleModSpellDamagePercentFromAttackPower(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* m_target = aurApp->GetTarget();
    if (m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    // Magic damage modifiers implemented in Unit::SpellDamageBonusDone
    // This information for client side use only
    // Recalculate bonus
    (m_target->ToPlayer())->UpdateSpellDamageAndHealingBonus();
}

void AuraEffect::HandleModSpellHealingPercentFromAttackPower(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;
    Unit* m_target = aurApp->GetTarget();
    if (m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    // Recalculate bonus
    (m_target->ToPlayer())->UpdateSpellDamageAndHealingBonus();
}

void AuraEffect::HandleModHealingDone(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;
    Unit* m_target = aurApp->GetTarget();
    if (m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    // implemented in Unit::SpellHealingBonusDone
    // this information is for client side only
    (m_target->ToPlayer())->UpdateSpellDamageAndHealingBonus();
}

void AuraEffect::HandleModTotalPercentStat(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;
    if (GetMiscValue() < -1 || GetMiscValue() > 4)
    {
        TC_LOG_ERROR("spells", "WARNING: Misc Value for SPELL_AURA_MOD_TOTAL_STAT_PERCENTAGE not valid");
        return;
    }

    Unit* m_target = aurApp->GetTarget();
    //save current and max HP before applying aura
    float healthPct = m_target->GetHealthPct();
    bool zeroHealth = !m_target->IsAlive();

    for (int32 i = STAT_STRENGTH; i < MAX_STATS; i++)
    {
        if (GetMiscValue() == i || GetMiscValue() == -1) // affect the same stats
        {
            float amount = m_target->GetTotalAuraMultiplier(SPELL_AURA_MOD_TOTAL_STAT_PERCENTAGE, [i](AuraEffect const* aurEff) -> bool
            {
                if (aurEff->GetMiscValue() == i || aurEff->GetMiscValue() == -1)
                    return true;
                return false;
            });

            if (m_target->GetPctModifierValue(UnitMods(UNIT_MOD_STAT_START + i), TOTAL_PCT) == amount)
                continue;

            m_target->SetStatPctModifier(UnitMods(UNIT_MOD_STAT_START + i), TOTAL_PCT, amount);
            if (m_target->GetTypeId() == TYPEID_PLAYER || m_target->IsPet())
                m_target->UpdateStatBuffMod(Stats(i));
        }
    }

    // recalculate current HP/MP after applying aura modifications (only for spells with SPELL_ATTR0_ABILITY 0x00000010 flag)
    // this check is total bullshit i think
    if ((GetMiscValue() == STAT_STAMINA || GetMiscValue() == -1) && GetSpellInfo()->HasAttribute(SPELL_ATTR0_ABILITY))
        m_target->SetHealth(std::max<uint32>(CalculatePct(m_target->GetMaxHealth(), healthPct), (zeroHealth ? 0 : 1)));

}

void AuraEffect::HandleAuraModResistenceOfStatPercent(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;
    Unit* m_target = aurApp->GetTarget();
    if (m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    if (GetMiscValue() != SPELL_SCHOOL_MASK_NORMAL)
    {
        // support required adding replace UpdateArmor by loop by UpdateResistence at intellect update
        // and include in UpdateResistence same code as in UpdateArmor for aura mod apply.
        TC_LOG_ERROR("FIXME", "Aura SPELL_AURA_MOD_RESISTANCE_OF_STAT_PERCENT(182) need adding support for non-armor resistances!");
        return;
    }

    // Recalculate Armor
    m_target->UpdateArmor();
}

/********************************/
/***      HEAL & ENERGIZE     ***/
/********************************/

void AuraEffect::HandleModPowerRegen(AuraApplication const* aurApp, uint8 mode, bool apply) const       // Drinking, Symbol of Hope, Water Shield, ...
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    if (target->GetTypeId() != TYPEID_PLAYER)
        return;

    if ((mode & AURA_EFFECT_HANDLE_REAL) && GetMiscValue() == POWER_MANA)
        (target->ToPlayer())->UpdateManaRegen();
#ifdef LICH_KING
    else if (GetMiscValue() == POWER_RUNE)
        target->ToPlayer()->UpdateRuneRegen(RuneType(GetMiscValueB()));
#endif
    // other powers are not immediate effects - implemented in Player::Regenerate, Creature::Regenerate
}

void AuraEffect::HandleModPowerRegenPCT(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    HandleModPowerRegen(aurApp, mode, apply);
}

void AuraEffect::HandleModManaRegen(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* m_target = aurApp->GetTarget();
    if (m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    //Note: an increase in regen does NOT cause threat.
    (m_target->ToPlayer())->UpdateManaRegen();
}

void AuraEffect::HandleComprehendLanguage(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_SEND_FOR_CLIENT_MASK))
        return;
    Unit* m_target = aurApp->GetTarget();
    if (apply)
        m_target->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_COMPREHEND_LANG);
    else {
        if (m_target->HasAuraType(GetAuraType()))
            return;

        m_target->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_COMPREHEND_LANG);
    }
}

void AuraEffect::HandleAuraModIncreaseHealth(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;
    Unit* m_target = aurApp->GetTarget();
    if (apply)
    {
        m_target->HandleStatFlatModifier(UNIT_MOD_HEALTH, TOTAL_VALUE, float(GetAmount()), apply);
        m_target->ModifyHealth(GetAmount());
    }
    else
    {
        if (m_target->GetHealth() > 0)
        {
            int32 value = std::min<int32>(m_target->GetHealth() - 1, GetAmount());  //shouldn't it be GetAmount ?
            m_target->ModifyHealth(-value);
        }

        m_target->HandleStatFlatModifier(UNIT_MOD_HEALTH, TOTAL_VALUE, float(GetAmount()), apply);
    }

    //HACK
    if (!apply && GetId() == 30421 && !m_target->HasAura(30421))
        m_target->AddAura(38637, m_target);
}

void  AuraEffect::HandleAuraModIncreaseMaxHealth(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;
    Unit* m_target = aurApp->GetTarget();
    float healthPercentage = m_target->GetHealthPct();

    m_target->HandleStatFlatModifier(UNIT_MOD_HEALTH, TOTAL_VALUE, float(GetAmount()), apply);

    // Unit will keep hp% after MaxHealth being modified if unit is alive.
    if (m_target->GetHealth() > 0)
    {
        uint32 newHealth = std::max<uint32>(std::round(m_target->CountPctFromMaxHealth(healthPercentage)), 1);
        m_target->SetHealth(newHealth);
    }
}

void AuraEffect::HandleAuraModIncreaseEnergy(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;
    Unit* m_target = aurApp->GetTarget();
    Powers powerType = m_target->GetPowerType();
    if (powerType != Powers(GetMiscValue()))
        return;

    UnitMods unitMod = UnitMods(UNIT_MOD_POWER_START + powerType);

    m_target->HandleStatFlatModifier(unitMod, TOTAL_VALUE, float(GetAmount()), apply);
}

void AuraEffect::HandleAuraModIncreaseEnergyPercent(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;
    Unit* m_target = aurApp->GetTarget();
    Powers powerType = m_target->GetPowerType();
    if (powerType != Powers(GetMiscValue()))
        return;

    UnitMods unitMod = UnitMods(UNIT_MOD_POWER_START + powerType);

    // Save old powers for further calculation
    int32 oldPower = int32(m_target->GetPower(powerType));
    int32 oldMaxPower = int32(m_target->GetMaxPower(powerType));

    if (apply)
    {
        float amount = float(GetAmount());
        m_target->ApplyStatPctModifier(unitMod, TOTAL_PCT, amount);
    }
    else
    {
        float amount = m_target->GetTotalAuraMultiplierByMiscValue(SPELL_AURA_MOD_INCREASE_ENERGY_PERCENT, GetMiscValue());
        m_target->SetStatPctModifier(unitMod, TOTAL_PCT, amount);
    }

    // Calculate the current power change
    int32 change = m_target->GetMaxPower(powerType) - oldMaxPower;
    change = (oldPower + change) - m_target->GetPower(powerType);
    m_target->ModifyPower(powerType, change);
}

void AuraEffect::HandleAuraModIncreaseHealthPercent(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;
    Unit* m_target = aurApp->GetTarget();
    // Unit will keep hp% after MaxHealth being modified if unit is alive.
    float percent = m_target->GetHealthPct();
    if (apply)
        m_target->ApplyStatPctModifier(UNIT_MOD_HEALTH, TOTAL_PCT, float(GetAmount()));
    else
    {
        float amount = m_target->GetTotalAuraMultiplier(SPELL_AURA_MOD_INCREASE_HEALTH_PERCENT);
        m_target->SetStatPctModifier(UNIT_MOD_HEALTH, TOTAL_PCT, amount);
    }

    // Unit will keep hp% after MaxHealth being modified if unit is alive.
    if (m_target->GetHealth() > 0)
    {
        uint32 newHealth = std::max<uint32>(m_target->CountPctFromMaxHealth(int32(percent)), 1);
        m_target->SetHealth(newHealth);
    }
}

/********************************/
/***          FIGHT           ***/
/********************************/

void AuraEffect::HandleAuraModParryPercent(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;
    Unit* m_target = aurApp->GetTarget();
    if (m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    (m_target->ToPlayer())->UpdateParryPercentage();
}

void AuraEffect::HandleAuraModDodgePercent(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;
    Unit* m_target = aurApp->GetTarget();
    if (m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    (m_target->ToPlayer())->UpdateDodgePercentage();
    //TC_LOG_ERROR("FIXME","BONUS DODGE CHANCE: + %f", float(_amount));
}

void AuraEffect::HandleAuraModBlockPercent(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;
    Unit* m_target = aurApp->GetTarget();
    if (m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    (m_target->ToPlayer())->UpdateBlockPercentage();
    //TC_LOG_ERROR("FIXME","BONUS BLOCK CHANCE: + %f", float(_amount));
}

void AuraEffect::HandleAuraModRegenInterrupt(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    HandleModManaRegen(aurApp, mode, apply);
}

void AuraEffect::HandleAuraModWeaponCritPercent(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;
    Player* target = aurApp->GetTarget()->ToPlayer();
    if (!target)
        return;

    target->UpdateAllWeaponDependentCritAuras();
}

void AuraEffect::HandleModHitChance(AuraApplication const* aurApp, uint8 mode, bool /*apply*/) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    // handle stack rules
    if (target->GetTypeId() == TYPEID_PLAYER)
    {
        target->ToPlayer()->UpdateMeleeHitChances();
        target->ToPlayer()->UpdateRangedHitChances();
    }
    else
    {
        float value = target->GetTotalAuraModifier(SPELL_AURA_MOD_HIT_CHANCE);
        target->m_modMeleeHitChance = value;
        target->m_modRangedHitChance = value;
    }
}

void AuraEffect::HandleModSpellHitChance(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    if (target->GetTypeId() == TYPEID_PLAYER)
        target->ToPlayer()->UpdateSpellHitChances();
    else
        target->m_modSpellHitChance += (apply) ? GetAmount() : (-GetAmount());
}

void AuraEffect::HandleModSpellCritChance(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* m_target = aurApp->GetTarget();
    if (m_target->GetTypeId() == TYPEID_PLAYER)
        (m_target->ToPlayer())->UpdateAllSpellCritChances();
    else
        m_target->m_baseSpellCritChance += apply ? GetAmount() : -GetAmount();
}

void AuraEffect::HandleModSpellCritChanceShool(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* m_target = aurApp->GetTarget();
    if (m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    for (int school = SPELL_SCHOOL_NORMAL; school < MAX_SPELL_SCHOOL; ++school)
        if (GetMiscValue() & (1 << school))
            (m_target->ToPlayer())->UpdateSpellCritChance(school);
}

/********************************/
/***         ATTACK SPEED     ***/
/********************************/

void AuraEffect::HandleModCastingSpeed(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;
    Unit* m_target = aurApp->GetTarget();
    m_target->ApplyCastTimePercentMod(GetAmount(), apply);
}

void AuraEffect::HandleModMeleeRangedSpeedPct(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;
    Unit* m_target = aurApp->GetTarget();
    m_target->ApplyAttackTimePercentMod(BASE_ATTACK, GetAmount(), apply);
    m_target->ApplyAttackTimePercentMod(OFF_ATTACK, GetAmount(), apply);
    m_target->ApplyAttackTimePercentMod(RANGED_ATTACK, GetAmount(), apply);
}

void AuraEffect::HandleModCombatSpeedPct(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();
    int32 spellGroupVal = target->GetHighestExclusiveSameEffectSpellGroupValue(this, SPELL_AURA_MELEE_SLOW);
    if (abs(spellGroupVal) >= abs(GetAmount()))
        return;

    if (spellGroupVal)
    {
        target->ApplyCastTimePercentMod(float(spellGroupVal), !apply);
        target->ApplyAttackTimePercentMod(BASE_ATTACK, float(spellGroupVal), !apply);
        target->ApplyAttackTimePercentMod(OFF_ATTACK, float(spellGroupVal), !apply);
        target->ApplyAttackTimePercentMod(RANGED_ATTACK, float(spellGroupVal), !apply);
    }
    target->ApplyCastTimePercentMod(float(GetAmount()), apply);
    target->ApplyAttackTimePercentMod(BASE_ATTACK, float(GetAmount()), apply);
    target->ApplyAttackTimePercentMod(OFF_ATTACK, float(GetAmount()), apply);
    target->ApplyAttackTimePercentMod(RANGED_ATTACK, float(GetAmount()), apply);
}

void AuraEffect::HandleModAttackSpeed(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();

    target->ApplyAttackTimePercentMod(BASE_ATTACK, float(GetAmount()), apply);
    target->UpdateDamagePhysical(BASE_ATTACK);
}

void AuraEffect::HandleModMeleeSpeedPct(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();
    int32 spellGroupVal = target->GetHighestExclusiveSameEffectSpellGroupValue(this, SPELL_AURA_MOD_MELEE_HASTE);
    if (abs(spellGroupVal) >= abs(GetAmount()))
        return;

    if (spellGroupVal)
    {
        target->ApplyAttackTimePercentMod(BASE_ATTACK, float(spellGroupVal), !apply);
        target->ApplyAttackTimePercentMod(OFF_ATTACK, float(spellGroupVal), !apply);
    }
    target->ApplyAttackTimePercentMod(BASE_ATTACK, float(GetAmount()), apply);
    target->ApplyAttackTimePercentMod(OFF_ATTACK, float(GetAmount()), apply);
}

void AuraEffect::HandleAuraModRangedHaste(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();
    target->ApplyAttackTimePercentMod(RANGED_ATTACK, (float)GetAmount(), apply);
}

void AuraEffect::HandleRangedAmmoHaste(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* target = aurApp->GetTarget();
    if (target->GetTypeId() != TYPEID_PLAYER)
        return;

    target->ApplyAttackTimePercentMod(RANGED_ATTACK, (float)GetAmount(), apply);
}

/********************************/
/***        ATTACK POWER      ***/
/********************************/

void AuraEffect::HandleAuraModAttackPower(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;
    Unit* m_target = aurApp->GetTarget();
    m_target->HandleStatFlatModifier(UNIT_MOD_ATTACK_POWER, TOTAL_VALUE, float(GetAmount()), apply);

    if (m_target->ToCreature() && m_target->GetEntry() == 15687)    // Moroes
        return;
}

void AuraEffect::HandleAuraModRangedAttackPower(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;
    Unit* m_target = aurApp->GetTarget();
    if ((m_target->GetClassMask() & CLASSMASK_WAND_USERS) != 0)
        return;

    m_target->HandleStatFlatModifier(UNIT_MOD_ATTACK_POWER_RANGED, TOTAL_VALUE, float(GetAmount()), apply);
}

void AuraEffect::HandleAuraModAttackPowerPercent(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;
    Unit* m_target = aurApp->GetTarget();
    //UNIT_FIELD_ATTACK_POWER_MULTIPLIER = multiplier - 1
    if (apply)
        m_target->ApplyStatPctModifier(UNIT_MOD_ATTACK_POWER, TOTAL_PCT, float(GetAmount()));
    else
    {
        float amount = m_target->GetTotalAuraMultiplier(SPELL_AURA_MOD_ATTACK_POWER_PCT);
        m_target->SetStatPctModifier(UNIT_MOD_ATTACK_POWER, TOTAL_PCT, amount);
    }
}

void AuraEffect::HandleAuraModRangedAttackPowerPercent(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* m_target = aurApp->GetTarget();
    if ((m_target->GetClassMask() & CLASSMASK_WAND_USERS) != 0)
        return;

    //UNIT_FIELD_RANGED_ATTACK_POWER_MULTIPLIER = multiplier - 1
    if (apply)
        m_target->ApplyStatPctModifier(UNIT_MOD_ATTACK_POWER_RANGED, TOTAL_PCT, float(GetAmount()));
    else
    {
        float amount = m_target->GetTotalAuraMultiplier(SPELL_AURA_MOD_RANGED_ATTACK_POWER_PCT);
        m_target->SetStatPctModifier(UNIT_MOD_ATTACK_POWER_RANGED, TOTAL_PCT, amount);
    }
}

void AuraEffect::HandleAuraModRangedAttackPowerOfStatPercent(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* m_target = aurApp->GetTarget();
    if (m_target->GetTypeId() == TYPEID_PLAYER && (m_target->GetClassMask() & CLASSMASK_WAND_USERS) != 0)
        return;

    if (GetMiscValue() != STAT_INTELLECT)
    {
        // support required adding UpdateAttackPowerAndDamage calls at stat update
        TC_LOG_ERROR("FIXME", "Aura SPELL_AURA_MOD_RANGED_ATTACK_POWER_OF_STAT_PERCENT (212) need support non-intellect stats!");
        return;
    }

    // Recalculate bonus
    (m_target->ToPlayer())->UpdateAttackPowerAndDamage(true);
}

/********************************/
/***        DAMAGE BONUS      ***/
/********************************/
void AuraEffect::HandleModDamageDone(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;
    // GetMiscValue() is bitmask of spell schools
    // 1 ( 0-bit ) - normal school damage (SPELL_SCHOOL_MASK_NORMAL)
    // 126 - full bitmask all magic damages (SPELL_SCHOOL_MASK_MAGIC) including wands
    // 127 - full bitmask any damages
    //
    // mods must be applied base at equipped weapon class and subclass comparison
    // with spell->EquippedItemClass and  EquippedItemSubClassMask and EquippedItemInventoryTypeMask
    // GetMiscValue() comparison with item generated damage types

    Unit* m_target = aurApp->GetTarget();
    if ((GetMiscValue() & SPELL_SCHOOL_MASK_NORMAL))
    {
        m_target->UpdateAllDamageDoneMods();
    }

    // Magic damage modifiers implemented in Unit::SpellDamageBonusDone
    // This information for client side use only
    if (m_target->GetTypeId() == TYPEID_PLAYER)
    {
        uint16 baseField = GetAmount() >= 0 ? PLAYER_FIELD_MOD_DAMAGE_DONE_POS : PLAYER_FIELD_MOD_DAMAGE_DONE_NEG;
        for (uint16 i = SPELL_SCHOOL_NORMAL; i < MAX_SPELL_SCHOOL; ++i)
            if (GetMiscValue() & (1 << i))
                m_target->ApplyModInt32Value(baseField + i, GetAmount(), apply);

        if (Guardian* pet = m_target->GetGuardianPet())
            pet->UpdateAttackPowerAndDamage();
    }
}

void AuraEffect::HandleModDamagePercentDone(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;
    // GetMiscValue() is bitmask of spell schools
    // 1 ( 0-bit ) - normal school damage (SPELL_SCHOOL_MASK_NORMAL)
    // 126 - full bitmask all magic damages (SPELL_SCHOOL_MASK_MAGIC) including wand
    // 127 - full bitmask any damages
    //
    // mods must be applied base at equipped weapon class and subclass comparison
    // with spell->EquippedItemClass and  EquippedItemSubClassMask and EquippedItemInventoryTypeMask
    // GetMiscValue() comparison with item generated damage types

    //sunstrider: removed condition, we need to update wands too. // if((GetMiscValue() & SPELL_SCHOOL_MASK_NORMAL))
    //{
    Unit* m_target = aurApp->GetTarget();
    m_target->UpdateAllDamagePctDoneMods();
    //}

    // Magic damage percent modifiers implemented in Unit::SpellDamageBonusDone
    // Send info to client
    if (m_target->GetTypeId() == TYPEID_PLAYER)
        for (int i = SPELL_SCHOOL_HOLY; i < MAX_SPELL_SCHOOL; ++i)
            if (GetMiscValue() & (1 << i))
            {
                // only aura type modifying PLAYER_FIELD_MOD_DAMAGE_DONE_PCT
                float amount = m_target->GetTotalAuraMultiplierByMiscMask(SPELL_AURA_MOD_DAMAGE_PERCENT_DONE, 1 << i);
                m_target->SetFloatValue(PLAYER_FIELD_MOD_DAMAGE_DONE_PCT + i, amount);
            }

    //Netherspite (Karazhan) nether beam UGLY HACK
    if ((mode & AURA_EFFECT_HANDLE_REAL) && !apply && GetId() == 30423 && !m_target->HasAura(30423) && !m_target->HasAura(30463))
        m_target->AddAura(38639, m_target);
}

void AuraEffect::HandleModOffhandDamagePercent(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* m_target = aurApp->GetTarget();
    m_target->UpdateDamagePctDoneMods(OFF_ATTACK);
}

/********************************/
/***        POWER COST        ***/
/********************************/

void AuraEffect::HandleModPowerCostPCT(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK))
        return;

    Unit* m_target = aurApp->GetTarget();
    float amount = GetAmount() / 100.0f;
    for (int i = 0; i < MAX_SPELL_SCHOOL; ++i)
        if (GetMiscValue() & (1 << i))
            m_target->ApplyModSignedFloatValue(UNIT_FIELD_POWER_COST_MULTIPLIER + i, amount, apply);

    if (!apply && GetId() == 30422 && !m_target->HasAura(30422))
        m_target->AddAura(38638, m_target);
}

void AuraEffect::HandleModPowerCost(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK))
        return;

    Unit* m_target = aurApp->GetTarget();
    for (int i = 0; i < MAX_SPELL_SCHOOL; ++i)
        if (GetMiscValue() & (1 << i))
            m_target->ApplyModInt32Value(UNIT_FIELD_POWER_COST_MODIFIER + i, GetAmount(), apply);
}

/*********************************************************/
/***                    OTHERS                         ***/
/*********************************************************/

void AuraEffect::HandleAuraEmpathy(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;
    Unit* m_target = aurApp->GetTarget();
    if (m_target->GetTypeId() != TYPEID_UNIT)
        return;

    if (!apply)
    {
        // do not remove unit flag if there are more than this auraEffect of that kind on unit on unit
        if (m_target->HasAuraType(GetAuraType()))
            return;
    }

    if (m_target->GetCreatureType() == CREATURE_TYPE_BEAST)
        m_target->ApplyModUInt32Value(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_SPECIALINFO, apply);
}

void AuraEffect::HandleAuraUntrackable(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_SEND_FOR_CLIENT_MASK))
        return;
    Unit* m_target = aurApp->GetTarget();
    if (apply)
        m_target->SetByteFlag(UNIT_FIELD_BYTES_1, UNIT_BYTES_1_OFFSET_VIS_FLAG, UNIT_STAND_FLAGS_UNTRACKABLE);
    else {
        // do not remove unit flag if there are more than this auraEffect of that kind on unit on unit
        if (m_target->HasAuraType(GetAuraType()))
            return;

        m_target->RemoveByteFlag(UNIT_FIELD_BYTES_1, UNIT_BYTES_1_OFFSET_VIS_FLAG, UNIT_STAND_FLAGS_UNTRACKABLE);
    }
}

void AuraEffect::HandleAuraModPacify(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_SEND_FOR_CLIENT_MASK))
        return;
    Unit* m_target = aurApp->GetTarget();
    if (apply)
        m_target->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED);
    else {
        // do not remove unit flag if there are more than this auraEffect of that kind on unit on unit
        if (m_target->HasAuraType(GetAuraType()))
            return;

        m_target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED);
    }
}

void AuraEffect::HandleAuraModPacifyAndSilence(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_SEND_FOR_CLIENT_MASK))
        return;

    HandleAuraModPacify(aurApp, mode, apply);
    HandleAuraModSilence(aurApp, mode, apply);
}

void AuraEffect::HandleAuraGhost(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_SEND_FOR_CLIENT_MASK))
        return;
    Unit* m_target = aurApp->GetTarget();
    if (m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    if (apply)
    {
        m_target->SetFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST);
        m_target->m_serverSideVisibility.SetValue(SERVERSIDE_VISIBILITY_GHOST, GHOST_VISIBILITY_GHOST);
        m_target->m_serverSideVisibilityDetect.SetValue(SERVERSIDE_VISIBILITY_GHOST, GHOST_VISIBILITY_GHOST);
    }
    else
    {
        if (m_target->HasAuraType(GetAuraType()))
            return;

        m_target->RemoveFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST);
        m_target->m_serverSideVisibility.SetValue(SERVERSIDE_VISIBILITY_GHOST, GHOST_VISIBILITY_ALIVE);
        m_target->m_serverSideVisibilityDetect.SetValue(SERVERSIDE_VISIBILITY_GHOST, GHOST_VISIBILITY_ALIVE);
    }
}

void AuraEffect::HandleAuraAllowFlight(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_SEND_FOR_CLIENT_MASK))
        return;

    Unit* m_target = aurApp->GetTarget();
    if (!apply)
    {
        // do not remove unit flag if there are more than this auraEffect of that kind on unit on unit
        if (m_target->HasAuraType(GetAuraType()) || m_target->HasAuraType(SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED))
            return;
    }

    m_target->SetFlying(apply);
}

void AuraEffect::HandleModRating(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;

    Unit* m_target = aurApp->GetTarget();
    if (m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    for (uint32 rating = 0; rating < MAX_COMBAT_RATING; ++rating)
        if (GetMiscValue() & (1 << rating))
            (m_target->ToPlayer())->ApplyRatingMod(CombatRating(rating), GetAmount(), apply);
}

void AuraEffect::HandleForceMoveForward(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_SEND_FOR_CLIENT_MASK))
        return;
    Unit* m_target = aurApp->GetTarget();
    if (m_target->GetTypeId() != TYPEID_PLAYER)
        return;
    if (apply)
        m_target->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FORCE_MOVE);
    else {
        // do not remove unit flag if there are more than this auraEffect of that kind on unit on unit
        if (m_target->HasAuraType(GetAuraType()))
            return;

        m_target->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FORCE_MOVE);
    }
}

void AuraEffect::HandleAuraModExpertise(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;
    Unit* m_target = aurApp->GetTarget();
    if (m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    (m_target->ToPlayer())->UpdateExpertise(BASE_ATTACK);
    (m_target->ToPlayer())->UpdateExpertise(OFF_ATTACK);
}

void AuraEffect::HandleModTargetResistance(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;
    // applied to damage as HandleNoImmediateEffect in Unit::CalcAbsorbResist and Unit::CalcArmorReducedDamage

    Unit* m_target = aurApp->GetTarget();
    // show armor penetration
    if (m_target->GetTypeId() == TYPEID_PLAYER && (GetMiscValue() & SPELL_SCHOOL_MASK_NORMAL))
        m_target->ApplyModInt32Value(PLAYER_FIELD_MOD_TARGET_PHYSICAL_RESISTANCE, GetAmount(), apply);

    // show as spell penetration only full spell penetration bonuses (all resistances except armor and holy)
    if (m_target->GetTypeId() == TYPEID_PLAYER && (GetMiscValue() & SPELL_SCHOOL_MASK_SPELL) == SPELL_SCHOOL_MASK_SPELL)
        m_target->ApplyModInt32Value(PLAYER_FIELD_MOD_TARGET_RESISTANCE, GetAmount(), apply);
}


void AuraEffect::HandleShieldBlockValue(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;
    Player* target = aurApp->GetTarget()->ToPlayer();
    if (!target)
        return;

    target->HandleBaseModFlatValue(SHIELD_BLOCK_VALUE, float(GetAmount()), apply);
}

void AuraEffect::HandleShieldBlockValuePercent(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & (AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK | AURA_EFFECT_HANDLE_STAT)))
        return;
    Player* target = aurApp->GetTarget()->ToPlayer();
    if (!target)
        return;

    if (apply)
        target->ApplyBaseModPctValue(SHIELD_BLOCK_VALUE, float(GetAmount()));
    else
    {
        float amount = target->GetTotalAuraMultiplier(SPELL_AURA_MOD_SHIELD_BLOCKVALUE_PCT);
        target->SetBaseModPctValue(SHIELD_BLOCK_VALUE, amount);
    }
}

void AuraEffect::HandleAuraRetainComboPoints(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    // spells required only Real aura add/remove
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* m_target = aurApp->GetTarget();
    if (m_target->GetTypeId() != TYPEID_PLAYER)
        return;

    Player *target = m_target->ToPlayer();

    // combo points was added in SPELL_EFFECT_ADD_COMBO_POINTS handler
    // remove only if aura expire by time (in case combo points amount change aura removed without combo points lost)
    if (!apply && GetBase()->GetDuration() == 0 && target->GetComboTarget())
        if (Unit* unit = ObjectAccessor::GetUnit(*m_target, target->GetComboTarget()))
            target->AddComboPoints(unit, -GetAmount());
}

void AuraEffect::HandleModUnattackable(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_SEND_FOR_CLIENT_MASK))
        return;
    Unit* m_target = aurApp->GetTarget();
    // do not remove unit flag if there are more than this auraEffect of that kind on unit on unit
    if (!apply && m_target->HasAuraType(SPELL_AURA_MOD_UNATTACKABLE))
        return;

    if (apply)
    {
        if (m_target->GetMap()->IsDungeon())
        {
            m_target->AttackStop();
            if (Player* targetPlayer = m_target->ToPlayer())
                targetPlayer->SendAttackSwingCancelAttack();
        }
        else
            m_target->CombatStop();
        m_target->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_IMMUNE_OR_LOST_SELECTION); //drop flag in bg
    }

    m_target->ApplyModFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE, apply);
}

void AuraEffect::HandleSpiritOfRedemption(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    // spells required only Real aura add/remove
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* m_target = aurApp->GetTarget();
    // prepare spirit state
    if (apply)
    {
        if (m_target->GetTypeId() == TYPEID_PLAYER)
        {
            // disable breath/etc timers
            (m_target->ToPlayer())->StopMirrorTimers();

            // cancel current spell
            m_target->InterruptNonMeleeSpells(true);

            // set stand state (expected in this form)
            if (!m_target->IsStandState())
                m_target->SetStandState(PLAYER_STATE_NONE);

            m_target->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_IMMUNE_OR_LOST_SELECTION);
        }

        m_target->SetHealth(1);
    }
    // die at aura end
    else {
        m_target->SetDeathState(JUST_DIED);
        if (m_target->GetTypeId() == TYPEID_PLAYER) {
            if (m_target->ToPlayer()->InBattleground()) {
                if (Battleground *bg = m_target->ToPlayer()->GetBattleground()) {
                    if (Player* killer = ObjectAccessor::FindPlayer(m_target->ToPlayer()->GetSpiritRedemptionKiller()))
                        bg->HandleKillPlayer(m_target->ToPlayer(), killer);
                }
            }
            (m_target->ToPlayer())->SetSpiritRedeptionKiller(ObjectGuid::Empty);
        }
    }
    m_target->ApplySpellImmune(GetId(), IMMUNITY_SCHOOL, SPELL_SCHOOL_MASK_NORMAL, apply);;
    m_target->CombatStop();
}

void AuraEffect::HandleSchoolAbsorb(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    //todo: move to scripts
    Unit* m_target = aurApp->GetTarget();
    // Shadow of Death just expired
    if (!apply && m_spellInfo->Id == 40251 && m_target->GetTypeId() == TYPEID_PLAYER)
    {
        m_target->RemoveAllAurasOnDeath();
        if (Unit* caster = GetCaster()) {
            if (caster->IsDead())
                return;

            // We summon ghost
            Creature* ghost = caster->SummonCreature(23109, m_target->GetPositionX(), m_target->GetPositionY(), m_target->GetPositionZ(), 0, TEMPSUMMON_MANUAL_DESPAWN, 0);
            if (!ghost)
                return;

            // remove pet if any
            (m_target->ToPlayer())->RemovePet(nullptr, PET_SAVE_AS_CURRENT, true);

            // immunity for body
            m_target->CastSpell(m_target, 40282, true);

            // Possess him
            m_target->CastSpell(ghost, 40268, true);

            // not attackable
            ghost->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);

            //summon shadowy constructs
            if (Creature* construct = caster->SummonCreature(23111, m_target->GetPositionX() + 2, m_target->GetPositionY() + 2, m_target->GetPositionZ() + 2, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 15000))
                construct->SetDisableGravity(true);
            if (Creature* construct = caster->SummonCreature(23111, m_target->GetPositionX() + 2, m_target->GetPositionY() - 2, m_target->GetPositionZ() + 2, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 15000))
                construct->SetDisableGravity(true);
            if (Creature* construct = caster->SummonCreature(23111, m_target->GetPositionX() - 2, m_target->GetPositionY() + 2, m_target->GetPositionZ() + 2, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 15000))
                construct->SetDisableGravity(true);
            if (Creature* construct = caster->SummonCreature(23111, m_target->GetPositionX() - 2, m_target->GetPositionY() - 2, m_target->GetPositionZ() + 2, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 15000))
                construct->SetDisableGravity(true);
        }

        return;
    }
}

void AuraEffect::HandlePreventFleeing(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* m_target = aurApp->GetTarget();
    m_target->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_FEAR, apply);

    /*
    // Since patch 3.0.2 this mechanic no longer affects fear effects. It will ONLY prevent humanoids from fleeing due to low health.
    if (!apply || target->HasAuraType(SPELL_AURA_MOD_FEAR))
    return;
    */

    if (apply && m_target->HasAuraType(SPELL_AURA_MOD_FEAR))
        m_target->SetControlled(false, UNIT_STATE_FLEEING);

    m_target->GetThreatManager().UpdateOnlineStates(true, false);
}

void AuraEffect::HandleArenaPreparation(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* m_target = aurApp->GetTarget();
    if (apply)
        m_target->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PREPARATION);
    else {
        // do not remove unit flag if there are more than this auraEffect of that kind on unit on unit
        if (m_target->HasAuraType(GetAuraType()))
            return;

        m_target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PREPARATION);
    }
}

void AuraEffect::HandleAOECharm(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* caster = GetCaster();
    Unit* m_target = aurApp->GetTarget();

    if (apply)
        m_target->SetCharmedBy(caster, CHARM_TYPE_CONVERT, aurApp);
    else
        m_target->RemoveCharmedBy(caster);
}

void AuraEffect::HandleModMechanicImmunityMask(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* m_target = aurApp->GetTarget();
    GetSpellInfo()->ApplyAllSpellImmunitiesTo(m_target, GetEffIndex(), apply);
}

void AuraEffect::HandleAuraCloneCaster(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_SEND_FOR_CLIENT_MASK))
        return;

    Unit* m_target = aurApp->GetTarget();
    if (apply)
    {
        Unit* caster = GetCaster();
        if (!caster || caster == m_target)
            return;

        switch (m_spellInfo->Id)
        {
        case 45785:
            if (m_target->GetEntry() != 25708)
                return;
            break;
        }
        m_target->SetDisplayId(caster->GetDisplayId());
        m_target->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_MIRROR_IMAGE);
    }
    else
    {
        m_target->SetDisplayId(m_target->GetNativeDisplayId());
        m_target->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_MIRROR_IMAGE);
    }
}

void AuraEffect::HandleAuraImmunityId(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* m_target = aurApp->GetTarget();
    if (!m_target || !GetMiscValue())
        return;

    m_target->ApplySpellImmune(0, IMMUNITY_ID, GetMiscValue(), apply);
}

void AuraEffect::HandleAuraApplyExtraFlag(AuraApplication const* aurApp, uint8 mode, bool apply) const
{
    if (!(mode & AURA_EFFECT_HANDLE_REAL))
        return;

    Unit* m_target = aurApp->GetTarget();
    if (!m_target || !(m_target->ToPlayer()))
        return;

    switch (GetMiscValue())
    {
    case PLAYER_EXTRA_DUEL_AREA:
        m_target->ToPlayer()->SetDuelArea(apply);
        m_target->ToPlayer()->UpdateZone(m_target->GetZoneId(), m_target->GetAreaId());
        break;
    default:
        TC_LOG_ERROR("spells", "HandleAuraApplyExtraFlag, flag %u not handled", GetMiscValue());
        break;
    }
}

void AuraEffect::HandleBreakableCCAuraProc(AuraApplication* aurApp, ProcEventInfo& eventInfo)
{
    int32 const damageLeft = GetAmount() - static_cast<int32>(eventInfo.GetDamageInfo()->GetDamage());

    if (damageLeft <= 0)
        aurApp->GetTarget()->RemoveAura(aurApp);
    else
        SetAmount(damageLeft);
}

void AuraEffect::HandleProcTriggerSpellAuraProc(AuraApplication* aurApp, ProcEventInfo& eventInfo)
{
    Unit* triggerCaster = aurApp->GetTarget();
    Unit* triggerTarget = eventInfo.GetProcTarget();

    uint32 triggered_spell_id = GetSpellInfo()->Effects[GetEffIndex()].TriggerSpell;
    Item* castItem = GetBase()->GetCastItemGUID() && GetBase()->GetCaster()->GetTypeId() == TYPEID_PLAYER
        ? (GetBase()->GetCaster()->ToPlayer())->GetItemByGuid(GetBase()->GetCastItemGUID()) : nullptr;

    if (SpellInfo const* triggeredSpellInfo = sSpellMgr->GetSpellInfo(triggered_spell_id))
    {
        TC_LOG_DEBUG("spells", "AuraEffect::HandleProcTriggerSpellAuraProc: Triggering spell %u from aura %u proc", triggeredSpellInfo->Id, GetId());

        CastSpellExtraArgs args(this);
        if (castItem)
            args.SetCastItem(castItem);

        if(!triggerTarget)  //may not be an error, but worth a debug log
            TC_LOG_DEBUG("spells", "AuraEffect::HandleProcTriggerSpellAuraProc: Triggering spell %u from aura proc %u has no target", triggeredSpellInfo->Id, GetId());

        triggerCaster->CastSpell(triggerTarget, triggeredSpellInfo->Id, args);
    }
    else
        TC_LOG_ERROR("spells", "AuraEffect::HandleProcTriggerSpellAuraProc: Could not trigger spell %u from aura %u proc, because the spell does not have an entry in Spell.dbc.", triggered_spell_id, GetId());
}

void AuraEffect::HandleProcTriggerDamageAuraProc(AuraApplication* aurApp, ProcEventInfo& eventInfo)
{
    if (!aurApp)
        return;

    Unit* target = aurApp->GetTarget();
    Unit* triggerTarget = eventInfo.GetProcTarget();
    if (triggerTarget->HasUnitState(UNIT_STATE_ISOLATED) || triggerTarget->IsImmunedToDamage(GetSpellInfo()))
    {
        SendTickImmune(triggerTarget, target);
        return;
    }

    SpellNonMeleeDamage damageInfo(target, triggerTarget, GetId(), GetSpellInfo()->SchoolMask);
    uint32 damage = target->SpellDamageBonusDone(triggerTarget, GetSpellInfo(), GetAmount(), SPELL_DIRECT_DAMAGE, GetEffIndex(), {});
    damage = triggerTarget->SpellDamageBonusTaken(target, GetSpellInfo(), damage, SPELL_DIRECT_DAMAGE);
    target->CalculateSpellDamageTaken(&damageInfo, damage, GetSpellInfo());
    Unit::DealDamageMods(damageInfo.target, damageInfo.damage, &damageInfo.absorb);
    target->SendSpellNonMeleeDamageLog(&damageInfo);
    TC_LOG_DEBUG("spells", "AuraEffect::HandleProcTriggerDamageAuraProc: Triggering %u spell damage from aura %u proc", damage, GetId());
    target->DealSpellDamage(&damageInfo, true);
}

void AuraEffect::HandleRaidProcFromChargeWithValueAuraProc(AuraApplication* aurApp, ProcEventInfo& /*eventInfo*/)
{
    Unit* target = aurApp->GetTarget();

    // Currently only Prayer of Mending
    if (!(GetSpellInfo()->SpellFamilyName == SPELLFAMILY_PRIEST && GetSpellInfo()->SpellFamilyFlags & 0x2000000000))
    {
        TC_LOG_DEBUG("spells", "AuraEffect::HandleRaidProcFromChargeWithValueAuraProc: received not handled spell: %u", GetId());
        return;
    }
    uint32 triggerSpellId = 33110;

    int32 jumps = GetBase()->GetCharges();

    // current aura expire on proc finish
    GetBase()->SetCharges(0);
    GetBase()->SetUsingCharges(true); //needed for the aura to properly expire

    CastSpellExtraArgs args(this);
    args.OriginalCaster = GetCasterGUID(); 
    args.AddSpellMod(SPELLVALUE_BASE_POINT0, GetAmount());

    // next target selection
    if (jumps > 0)
    {
        if (Unit* caster = GetCaster())
        {
            float radius = GetSpellInfo()->Effects[GetEffIndex()].CalcRadius(caster);

            if (Unit* triggerTarget = target->GetNextRandomRaidMemberOrPet(radius))
            {
                target->CastSpell(triggerTarget, GetId(), args);
                if (Aura* aura = triggerTarget->GetAura(GetId(), GetCasterGUID()))
                    aura->SetCharges(jumps);
            }
        }
    }

    TC_LOG_DEBUG("spells", "AuraEffect::HandleRaidProcFromChargeWithValueAuraProc: Triggering spell %u from aura %u proc", triggerSpellId, GetId());
    target->CastSpell(target, triggerSpellId, args);
}

template TC_GAME_API void AuraEffect::GetTargetList(std::list<Unit*>&) const;
template TC_GAME_API void AuraEffect::GetTargetList(std::deque<Unit*>&) const;
template TC_GAME_API void AuraEffect::GetTargetList(std::vector<Unit*>&) const;

template TC_GAME_API void AuraEffect::GetApplicationList(std::list<AuraApplication*>&) const;
template TC_GAME_API void AuraEffect::GetApplicationList(std::deque<AuraApplication*>&) const;
template TC_GAME_API void AuraEffect::GetApplicationList(std::vector<AuraApplication*>&) const;
